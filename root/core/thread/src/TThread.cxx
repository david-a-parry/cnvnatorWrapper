// @(#)root/thread:$Id$
// Author: Fons Rademakers   02/07/97

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TThread                                                              //
//                                                                      //
// This class implements threads. A thread is an execution environment  //
// much lighter than a process. A single process can have multiple      //
// threads. The actual work is done via the TThreadImp class (either    //
// TPosixThread or TWin32Thread).                                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "RConfigure.h"

#include "TThread.h"
#include "TThreadImp.h"
#include "TThreadFactory.h"
#include "TROOT.h"
#include "TApplication.h"
#include "TVirtualPad.h"
#include "TMethodCall.h"
#include "TTimeStamp.h"
#include "TInterpreter.h"
#include "TError.h"
#include "Varargs.h"
#include "ThreadLocalStorage.h"

TThreadImp     *TThread::fgThreadImp = 0;
Long_t          TThread::fgMainId = 0;
TThread        *TThread::fgMain = 0;
TMutex         *TThread::fgMainMutex;
char  *volatile TThread::fgXAct = 0;
TMutex         *TThread::fgXActMutex = 0;
TCondition     *TThread::fgXActCondi = 0;
void **volatile TThread::fgXArr = 0;
volatile Int_t  TThread::fgXAnb = 0;
volatile Int_t  TThread::fgXArt = 0;

static void CINT_alloc_lock()   { gGlobalMutex->Lock(); }
static void CINT_alloc_unlock() { gGlobalMutex->UnLock(); }

static TMutex  *gMainInternalMutex = 0;

static void ThreadInternalLock() { if (gMainInternalMutex) gMainInternalMutex->Lock(); }
static void ThreadInternalUnLock() { if (gMainInternalMutex) gMainInternalMutex->UnLock(); }

static Bool_t fgIsTearDown(kFALSE);

//------------------------------------------------------------------------------

// Set gGlobalMutex to 0 when Thread library gets unloaded
class TThreadTearDownGuard {
public:
   TThreadTearDownGuard() { fgIsTearDown = kFALSE; }
   ~TThreadTearDownGuard() {
      // Note: we could insert here a wait for all thread to be finished.
      // this is questionable though as we need to balance between fixing a
      // user error (the thread was let lose and the caller did not explicit wait)
      // and the risk that we can not terminate a failing process.

      fgIsTearDown = kTRUE;
      TVirtualMutex *m = gGlobalMutex;
      gGlobalMutex = 0;
      delete m;
      TThreadImp *imp = TThread::fgThreadImp;
      TThread::fgThreadImp = 0;
      delete imp;
   }
};
static TThreadTearDownGuard gTearDownGuard;

//------------------------------------------------------------------------------

class TJoinHelper {
private:
   TThread    *fT;
   TThread    *fH;
   void      **fRet;
   Long_t      fRc;
   TMutex     *fM;
   TCondition *fC;
   Bool_t      fJoined;

   static void* JoinFunc(void *p);

public:
   TJoinHelper(TThread *th, void **ret);
   ~TJoinHelper();

   Int_t Join();
};

//______________________________________________________________________________
TJoinHelper::TJoinHelper(TThread *th, void **ret)
   : fT(th), fRet(ret), fRc(0), fM(new TMutex), fC(new TCondition(fM)), fJoined(kFALSE)
{
   // Constructor of Thread helper class.

   fH = new TThread("JoinHelper", JoinFunc, this);
}

//______________________________________________________________________________
TJoinHelper::~TJoinHelper()
{
   // Destructor.

   delete fC;
   delete fM;
   delete fH;
}

//______________________________________________________________________________
void* TJoinHelper::JoinFunc(void *p)
{
   // Static method which runs in a separate thread to handle thread
   // joins without blocking the main thread.
   // Return a value (zero) so that it makes a joinable thread.

   TJoinHelper *jp = (TJoinHelper*)p;

   jp->fRc = jp->fT->Join(jp->fRet);

   jp->fM->Lock();
   jp->fJoined = kTRUE;
   jp->fC->Signal();
   jp->fM->UnLock();

   TThread::Exit(0);

   return 0;
}

//______________________________________________________________________________
Int_t TJoinHelper::Join()
{
   // Thread join function.

   fM->Lock();
   fH->Run();

   while (kTRUE) {
      // TimedWaitRelative will release the mutex (i.e. equivalent to fM->Unlock),
      // then block on the condition variable.  Upon return it will lock the mutex.
      int r = fC->TimedWaitRelative(100);  // 100 ms

      // From the man page from pthread_ond_timedwait:

      // When using condition variables there is always a Boolean predicate
      // involving shared variables associated with each condition wait that
      // is true if the thread should proceed. Spurious wakeups from the
      // pthread_cond_timedwait() or pthread_cond_wait() functions may occur.
      // Since the return from pthread_cond_timedwait() or pthread_cond_wait()
      // does not imply anything about the value of this predicate, the
      // predicate should be re-evaluated upon such return.

      if (r == 0 || r == 1) {
         // If we received the signal or timed out, let's check the value
         if (fJoined) break;
      } else {
         // If any other error occured, there is no point in trying again
         break;
      }

      gSystem->ProcessEvents();
   }

   fM->UnLock();

   // And wait for the help to finish to avoid the risk that it is still
   // running when the main tread is finished (and the thread library unloaded!)
   TThread::fgThreadImp->Join(fH, 0);

   return fRc;
}


//------------------------------------------------------------------------------

ClassImp(TThread)


//______________________________________________________________________________
TThread::TThread(VoidRtnFunc_t fn, void *arg, EPriority pri)
   : TNamed("<anon>", "")
{
   // Create a thread. Specify the function or static class method
   // to be executed by the thread and a pointer to the argument structure.
   // The user function should return a void*. To start the thread call Run().

   fDetached  = kFALSE;
   fFcnVoid   = 0;
   fFcnRetn   = fn;
   fPriority  = pri;
   fThreadArg = arg;
   Constructor();
   fNamed     = kFALSE;
}

//______________________________________________________________________________
TThread::TThread(VoidFunc_t fn, void *arg, EPriority pri)
   : TNamed("<anon>", "")
{
   // Create a detached thread. Specify the function or static class method
   // to be executed by the thread and a pointer to the argument structure.
   // To start the thread call Run().

   fDetached  = kTRUE;
   fFcnRetn   = 0;
   fFcnVoid   = fn;
   fPriority  = pri;
   fThreadArg = arg;
   Constructor();
   fNamed     = kFALSE;
}

//______________________________________________________________________________
TThread::TThread(const char *thname, VoidRtnFunc_t fn, void *arg,
                 EPriority pri) : TNamed(thname, "")
{
   // Create thread with a name. Specify the function or static class method
   // to be executed by the thread and a pointer to the argument structure.
   // The user function should return a void*. To start the thread call Run().

   fDetached  = kFALSE;
   fFcnVoid   = 0;
   fFcnRetn   = fn;
   fPriority  = pri;
   fThreadArg = arg;
   Constructor();
   fNamed     = kTRUE;
}

//______________________________________________________________________________
TThread::TThread(const char *thname, VoidFunc_t fn, void *arg,
                 EPriority pri) : TNamed(thname, "")
{
   // Create a detached thread with a name. Specify the function or static
   // class method to be executed by the thread and a pointer to the argument
   // structure. To start the thread call Run().

   fDetached  = kTRUE;
   fFcnRetn   = 0;
   fFcnVoid   = fn;
   fPriority  = pri;
   fThreadArg = arg;
   Constructor();
   fNamed     = kTRUE;
}

//______________________________________________________________________________
TThread::TThread(Long_t id)
{
   // Create a TThread for a already running thread.

   fDetached  = kTRUE;
   fFcnRetn   = 0;
   fFcnVoid   = 0;
   fPriority  = kNormalPriority;
   fThreadArg = 0;
   Constructor();

   // Changing the id must be protected as it will be look at by multiple
   // threads (see TThread::GetThread)
   ThreadInternalLock();
   fNamed     = kFALSE;
   fId = (id ? id : SelfId());
   fState = kRunningState;
   ThreadInternalUnLock();

   if (gDebug)
      Info("TThread::TThread", "TThread attached to running thread");
}

//______________________________________________________________________________
void TThread::Initialize()
{
   // Initialize the Thread package. This initializes the TThread and ROOT
   // global mutexes to make parts of ROOT thread safe/aware. This call is
   // implicit in case a TThread is created.

   Init();
}

//______________________________________________________________________________
Bool_t TThread::IsInitialized()
{
   // Return true, if the TThread objects have been initialize. If false,
   // the process is (from ROOT's point of view) single threaded.

   if (fgThreadImp)
      return kTRUE;
   return kFALSE;
}

//______________________________________________________________________________
void TThread::Init()
{
   // Initialize global state and variables once.

   if (fgThreadImp || fgIsTearDown) return;

   fgThreadImp = gThreadFactory->CreateThreadImp();
   gMainInternalMutex = new TMutex(kTRUE);

   fgMainId    = fgThreadImp->SelfId();
   fgMainMutex = new TMutex(kTRUE);
   gThreadTsd  = TThread::Tsd;
   gThreadXAR  = TThread::XARequest;


   // Create the single global mutex
   gGlobalMutex = new TMutex(kTRUE);
   gCint->SetAlloclockfunc(CINT_alloc_lock);
   gCint->SetAllocunlockfunc(CINT_alloc_unlock);

   //To avoid deadlocks, gCintMutex and gROOTMutex need
   // to point at the same instance
   {
     R__LOCKGUARD(gGlobalMutex);
     if (!gCINTMutex) {
       gCINTMutex = gGlobalMutex->Factory(kTRUE);
     }
     gROOTMutex = gCINTMutex;
   }
}

//______________________________________________________________________________
void TThread::Constructor()
{
   // Common thread constructor.

   fHolder = 0;
   fClean  = 0;
   fState  = kNewState;

   fId = -1;
   fHandle= 0;
   if (!fgThreadImp) Init();

   SetComment("Constructor: MainInternalMutex Locking");
   ThreadInternalLock();
   SetComment("Constructor: MainInternalMutex Locked");
   memset(fTsd, 0, ROOT::kMaxThreadSlot*sizeof(void*));
   // In order for the thread 'gDirectory' value to be properly
   // initialized we need to set it now (otherwise it default
   // to zero which is 'unexpected')
   // We initialize it to gROOT rather than gDirectory, since
   // TFile are currently expected to not be shared by two threads.
   fTsd[ROOT::kDirectoryThreadSlot] = gROOT;

   if (fgMain) fgMain->fPrev = this;
   fNext = fgMain; fPrev = 0; fgMain = this;

   ThreadInternalUnLock();
   SetComment();

   // thread is set up in initialisation routine or Run().
}

//______________________________________________________________________________
TThread::~TThread()
{
   // Cleanup the thread.

   if (gDebug)
      Info("TThread::~TThread", "thread deleted");

   // Disconnect thread instance

   SetComment("Destructor: MainInternalMutex Locking");
   ThreadInternalLock();
   SetComment("Destructor: MainInternalMutex Locked");

   if (fPrev) fPrev->fNext = fNext;
   if (fNext) fNext->fPrev = fPrev;
   if (fgMain == this) fgMain = fNext;

   ThreadInternalUnLock();
   SetComment();
   if (fHolder) *fHolder = 0;
}

//______________________________________________________________________________
Int_t TThread::Delete(TThread *&th)
{
   // Static method to delete the specified thread.
   // Returns -1 in case the thread was running and has been killed. Returns
   // 0 in case the thread has been Delete and Cleaned up. The th pointer is
   // not valid anymore in that case.

   if (!th) return 0;
   th->fHolder = &th;

   if (th->fState == kRunningState) {     // Cancel if running
      th->fState = kDeletingState;

      if (gDebug)
         th->Info("TThread::Delete", "deleting thread");

      th->Kill();
      return -1;
   }

   CleanUp();
   return 0;
}

//______________________________________________________________________________
Int_t TThread::Exists()
{
   // Static method to check if threads exist.
   // Returns the number of running threads.

   ThreadInternalLock();

   Int_t num = 0;
   for (TThread *l = fgMain; l; l = l->fNext)
      num++; //count threads

   ThreadInternalUnLock();

   return num;
}

//______________________________________________________________________________
void TThread::SetPriority(EPriority pri)
{
   // Set thread priority.

   fPriority = pri;
}

//______________________________________________________________________________
TThread *TThread::GetThread(Long_t id)
{
   // Static method to find a thread by id.

   TThread *myTh;

   ThreadInternalLock();

   for (myTh = fgMain; myTh && (myTh->fId != id); myTh = myTh->fNext) { }

   ThreadInternalUnLock();

   return myTh;
}

//______________________________________________________________________________
TThread *TThread::GetThread(const char *name)
{
   // Static method to find a thread by name.

   TThread *myTh;

   ThreadInternalLock();

   for (myTh = fgMain; myTh && (strcmp(name, myTh->GetName())); myTh = myTh->fNext) { }

   ThreadInternalUnLock();

   return myTh;
}

//______________________________________________________________________________
TThread *TThread::Self()
{
   // Static method returning pointer to current thread.

   TTHREAD_TLS(TThread*) self = 0;

   if (!self || fgIsTearDown) {
      if (fgIsTearDown) self = 0;
      self = GetThread(SelfId());
   }
   return self;
}


//______________________________________________________________________________
Long_t TThread::Join(void **ret)
{
   // Join this thread.

   if (fId == -1) {
      Error("Join", "thread not running");
      return -1;
   }

   if (fDetached) {
      Error("Join", "cannot join detached thread");
      return -1;
   }

   if (SelfId() != fgMainId)
      return fgThreadImp->Join(this, ret);

   // do not block the main thread, use helper thread
   TJoinHelper helper(this, ret);

   return helper.Join();
}

//______________________________________________________________________________
Long_t TThread::Join(Long_t jid, void **ret)
{
   // Static method to join a thread by id.

   TThread *myTh = GetThread(jid);

   if (!myTh) {
      ::Error("TThread::Join", "cannot find thread 0x%lx", jid);
      return -1L;
   }

   return myTh->Join(ret);
}

//______________________________________________________________________________
Long_t TThread::SelfId()
{
   // Static method returning the id for the current thread.

   if (fgIsTearDown) return -1;
   if (!fgThreadImp) Init();

   return fgThreadImp->SelfId();
}

//______________________________________________________________________________
Int_t TThread::Run(void *arg)
{
   // Start the thread. This starts the static method TThread::Function()
   // which calls the user function specified in the TThread ctor with
   // the arg argument. Returns 0 on success, otherwise an error number will
   // be returned.

   if (arg) fThreadArg = arg;

   SetComment("Run: MainInternalMutex locking");
   ThreadInternalLock();
   SetComment("Run: MainMutex locked");

   int iret = fgThreadImp->Run(this);

   fState = iret ? kInvalidState : kRunningState;

   if (gDebug)
      Info("TThread::Run", "thread run requested");

   ThreadInternalUnLock();
   SetComment();
   return iret;
}

//______________________________________________________________________________
Int_t TThread::Kill()
{
   // Kill this thread. Returns 0 on success, otherwise an error number will
   // be returned.

   if (fState != kRunningState && fState != kDeletingState) {
      if (gDebug)
         Warning("TThread::Kill", "thread is not running");
      return 13;
   } else {
      if (fState == kRunningState ) fState = kCancelingState;
      return fgThreadImp->Kill(this);
   }
}

//______________________________________________________________________________
Int_t TThread::Kill(Long_t id)
{
   // Static method to kill the thread by id. Returns 0 on success, otherwise
   // an error number will be returned.

   TThread *th = GetThread(id);
   if (th) {
      return fgThreadImp->Kill(th);
   } else  {
      if (gDebug)
         ::Warning("TThread::Kill(Long_t)", "thread 0x%lx not found", id);
      return 13;
   }
}

//______________________________________________________________________________
Int_t TThread::Kill(const char *name)
{
   // Static method to kill thread by name. Returns 0 on success, otherwise
   // an error number will be returned.

   TThread *th = GetThread(name);
   if (th) {
      return fgThreadImp->Kill(th);
   } else  {
      if (gDebug)
         ::Warning("TThread::Kill(const char*)", "thread %s not found", name);
      return 13;
   }
}

//______________________________________________________________________________
Int_t TThread::SetCancelOff()
{
   // Static method to turn off thread cancellation. Returns 0 on success,
   // otherwise an error number will be returned.

   return fgThreadImp ? fgThreadImp->SetCancelOff() : -1;
}

//______________________________________________________________________________
Int_t TThread::SetCancelOn()
{
   // Static method to turn on thread cancellation. Returns 0 on success,
   // otherwise an error number will be returned.

   return fgThreadImp ? fgThreadImp->SetCancelOn() : -1;
}

//______________________________________________________________________________
Int_t TThread::SetCancelAsynchronous()
{
   // Static method to set the cancellation response type of the calling thread
   // to asynchronous, i.e. cancel as soon as the cancellation request
   // is received.

   return fgThreadImp ? fgThreadImp->SetCancelAsynchronous() : -1;
}

//______________________________________________________________________________
Int_t TThread::SetCancelDeferred()
{
   // Static method to set the cancellation response type of the calling thread
   // to deferred, i.e. cancel only at next cancellation point.
   // Returns 0 on success, otherwise an error number will be returned.

   return fgThreadImp ? fgThreadImp->SetCancelDeferred() : -1;
}

//______________________________________________________________________________
Int_t TThread::CancelPoint()
{
   // Static method to set a cancellation point. Returns 0 on success, otherwise
   // an error number will be returned.

   return fgThreadImp ? fgThreadImp->CancelPoint() : -1;
}

//______________________________________________________________________________
Int_t TThread::CleanUpPush(void *free, void *arg)
{
   // Static method which pushes thread cleanup method on stack.
   // Returns 0 in case of success and -1 in case of error.

   TThread *th = Self();
   if (th)
      return fgThreadImp->CleanUpPush(&(th->fClean), free, arg);
   return -1;
}

//______________________________________________________________________________
Int_t TThread::CleanUpPop(Int_t exe)
{
   // Static method which pops thread cleanup method off stack.
   // Returns 0 in case of success and -1 in case of error.

   TThread *th = Self();
   if (th)
      return fgThreadImp->CleanUpPop(&(th->fClean), exe);
   return -1;
}

//______________________________________________________________________________
Int_t TThread::CleanUp()
{
   // Static method to cleanup the calling thread.

   TThread *th = Self();
   if (!th) return 13;

   fgThreadImp->CleanUp(&(th->fClean));
   fgMainMutex->CleanUp();
   if (fgXActMutex)
      fgXActMutex->CleanUp();

   gMainInternalMutex->CleanUp();

   if (th->fHolder)
      delete th;

   return 0;
}

//______________________________________________________________________________
void TThread::AfterCancel(TThread *th)
{
   // Static method which is called after the thread has been canceled.

   if (th) {
      th->fState = kCanceledState;
      if (gDebug)
         th->Info("TThread::AfterCancel", "thread is canceled");
   } else
      ::Error("TThread::AfterCancel", "zero thread pointer passed");
}

//______________________________________________________________________________
Int_t TThread::Exit(void *ret)
{
   // Static method which terminates the execution of the calling thread.

   return fgThreadImp ? fgThreadImp->Exit(ret) : -1;
}

//______________________________________________________________________________
Int_t TThread::Sleep(ULong_t secs, ULong_t nanos)
{
   // Static method to sleep the calling thread.

   UInt_t ms = UInt_t(secs * 1000) + UInt_t(nanos / 1000000);
   if (gSystem) gSystem->Sleep(ms);
   return 0;
}

//______________________________________________________________________________
Int_t TThread::GetTime(ULong_t *absSec, ULong_t *absNanoSec)
{
   // Static method to get the current time. Returns
   // the number of seconds.

   TTimeStamp t;
   if (absSec)     *absSec     = t.GetSec();
   if (absNanoSec) *absNanoSec = t.GetNanoSec();
   return t.GetSec();
}

//______________________________________________________________________________
Int_t TThread::Lock()
{
   // Static method to lock the main thread mutex.

   return (fgMainMutex ? fgMainMutex->Lock() : 0);
}

//______________________________________________________________________________
Int_t TThread::TryLock()
{
   // Static method to try to lock the main thread mutex.

   return (fgMainMutex ? fgMainMutex->TryLock() : 0);
}

//______________________________________________________________________________
Int_t TThread::UnLock()
{
   // Static method to unlock the main thread mutex.

   return (fgMainMutex ? fgMainMutex->UnLock() : 0);
}

//______________________________________________________________________________
void *TThread::Function(void *ptr)
{
   // Static method which is called by the system thread function and
   // which in turn calls the actual user function.

   TThread *th;
   void *ret, *arg;

   TThreadCleaner dummy;

   th = (TThread *)ptr;

   // Default cancel state is OFF
   // Default cancel type  is DEFERRED
   // User can change it by call SetCancelOn() and SetCancelAsynchronous()
   SetCancelOff();
   SetCancelDeferred();
   CleanUpPush((void *)&AfterCancel, th);  // Enable standard cancelling function

   if (gDebug)
      th->Info("TThread::Function", "thread is running");

   arg = th->fThreadArg;
   th->fState = kRunningState;

   if (th->fDetached) {
      //Detached, non joinable thread
      (th->fFcnVoid)(arg);
      ret = 0;
      th->fState = kFinishedState;
   } else {
      //UnDetached, joinable thread
      ret = (th->fFcnRetn)(arg);
      th->fState = kTerminatedState;
   }

   CleanUpPop(1);     // Disable standard canceling function

   if (gDebug)
      th->Info("TThread::Function", "thread has finished");

   TThread::Exit(ret);

   return ret;
}

//______________________________________________________________________________
void TThread::Ps()
{
   // Static method listing the existing threads.

   TThread *l;
   int i;

   if (!fgMain) {
      ::Info("TThread::Ps", "no threads have been created");
      return;
   }

   ThreadInternalLock();

   int num = 0;
   for (l = fgMain; l; l = l->fNext)
      num++;

   char cbuf[256];
   printf("     Thread                   State\n");
   for (l = fgMain; l; l = l->fNext) { // loop over threads
      memset(cbuf, ' ', sizeof(cbuf));
      snprintf(cbuf, sizeof(cbuf), "%3d  %s:0x%lx", num--, l->GetName(), l->fId);
      i = strlen(cbuf);
      if (i < 30)
         cbuf[i] = ' ';
      cbuf[30] = 0;
      printf("%30s", cbuf);

      switch (l->fState) {   // print states
         case kNewState:        printf("Idle       "); break;
         case kRunningState:    printf("Running    "); break;
         case kTerminatedState: printf("Terminated "); break;
         case kFinishedState:   printf("Finished   "); break;
         case kCancelingState:  printf("Canceling  "); break;
         case kCanceledState:   printf("Canceled   "); break;
         case kDeletingState:   printf("Deleting   "); break;
         default:               printf("Invalid    ");
      }
      if (l->fComment[0]) printf("  // %s", l->fComment);
      printf("\n");
   }  // end of loop

   ThreadInternalUnLock();
}

//______________________________________________________________________________
void **TThread::Tsd(void *dflt, Int_t k)
{
   // Static method returning a pointer to thread specific data container
   // of the calling thread.
   // k should be between 0 and kMaxUserThreadSlot for user application.
   // (and between kMaxUserThreadSlot and kMaxThreadSlot for ROOT libraries).
   // See ROOT::EThreadSlotReservation

   TThread *th = TThread::Self();

   if (!th) {   //Main thread
      return (void**)dflt;
   } else {
      return &(th->fTsd[k]);
   }
}

//______________________________________________________________________________
void TThread::Printf(const char *va_(fmt), ...)
{
   // Static method providing a thread safe printf. Appends a newline.

   va_list ap;
   va_start(ap,va_(fmt));

   Int_t buf_size = 2048;
   char *buf;

again:
   buf = new char[buf_size];

   int n = vsnprintf(buf, buf_size, va_(fmt), ap);
   // old vsnprintf's return -1 if string is truncated new ones return
   // total number of characters that would have been written
   if (n == -1 || n >= buf_size) {
      buf_size *= 2;
      delete [] buf;
      goto again;
   }

   va_end(ap);

   void *arr[2];
   arr[1] = (void*) buf;
   if (XARequest("PRTF", 2, arr, 0)) return;

   printf("%s\n", buf);
   fflush(stdout);

   delete [] buf;
}

//______________________________________________________________________________
void TThread::ErrorHandler(int level, const char *location, const char *fmt,
                           va_list ap) const
{
   // Thread specific error handler function.
   // It calls the user set error handler in the main thread.

   Int_t buf_size = 2048;
   char *buf, *bp;

again:
   buf = new char[buf_size];

   int n = vsnprintf(buf, buf_size, fmt, ap);
   // old vsnprintf's return -1 if string is truncated new ones return
   // total number of characters that would have been written
   if (n == -1 || n >= buf_size) {
      buf_size *= 2;
      delete [] buf;
      goto again;
   }
   if (level >= kSysError && level < kFatal) {
      char *buf1 = new char[buf_size + strlen(gSystem->GetError()) + 5];
      sprintf(buf1, "%s (%s)", buf, gSystem->GetError());
      bp = buf1;
      delete [] buf;
   } else
      bp = buf;

   void *arr[4];
   arr[1] = (void*) Long_t(level);
   arr[2] = (void*) location;
   arr[3] = (void*) bp;
   if (XARequest("ERRO", 4, arr, 0)) return;

   if (level != kFatal)
      ::GetErrorHandler()(level, level >= gErrorAbortLevel, location, bp);
   else
      ::GetErrorHandler()(level, kTRUE, location, bp);

   delete [] bp;
}

//______________________________________________________________________________
void TThread::DoError(int level, const char *location, const char *fmt,
                      va_list va) const
{
   // Interface to ErrorHandler. User has to specify the class name as
   // part of the location, just like for the global Info(), Warning() and
   // Error() functions.

   char *loc = 0;

   if (location) {
      loc = new char[strlen(location) + strlen(GetName()) + 32];
      sprintf(loc, "%s %s:0x%lx", location, GetName(), fId);
   } else {
      loc = new char[strlen(GetName()) + 32];
      sprintf(loc, "%s:0x%lx", GetName(), fId);
   }

   ErrorHandler(level, loc, fmt, va);

   delete [] loc;
}

//______________________________________________________________________________
Int_t TThread::XARequest(const char *xact, Int_t nb, void **ar, Int_t *iret)
{
   // Static method used to allow commands to be executed by the main thread.

   if (!gApplication || !gApplication->IsRunning()) return 0;

   // The first time, create the related static vars
   if (!fgXActMutex && gGlobalMutex) {
      gGlobalMutex->Lock();
      if (!fgXActMutex) {
         fgXActMutex = new TMutex(kTRUE);
         fgXActCondi = new TCondition;
         new TThreadTimer;
      }
      gGlobalMutex->UnLock();
   }

   TThread *th = Self();
   if (th && th->fId != fgMainId) {   // we are in the thread
      th->SetComment("XARequest: XActMutex Locking");
      fgXActMutex->Lock();
      th->SetComment("XARequest: XActMutex Locked");

      TConditionImp *condimp = fgXActCondi->fConditionImp;
      TMutexImp *condmutex = fgXActCondi->GetMutex()->fMutexImp;

      // Lock now, so the XAction signal will wait
      // and never come before the wait
      condmutex->Lock();

      fgXAnb = nb;
      fgXArr = ar;
      fgXArt = 0;
      fgXAct = (char*) xact;
      th->SetComment(fgXAct);

      if (condimp) condimp->Wait();
      condmutex->UnLock();

      if (iret) *iret = fgXArt;
      fgXActMutex->UnLock();
      th->SetComment();
      return 1997;
   } else            //we are in the main thread
      return 0;
}

//______________________________________________________________________________
void TThread::XAction()
{
   // Static method called via the thread timer to execute in the main
   // thread certain commands. This to avoid sophisticated locking and
   // possible deadlocking.

   TConditionImp *condimp = fgXActCondi->fConditionImp;
   TMutexImp *condmutex = fgXActCondi->GetMutex()->fMutexImp;
   condmutex->Lock();

   char const acts[] = "PRTF CUPD CANV CDEL PDCD METH ERRO";
   enum { kPRTF = 0, kCUPD = 5, kCANV = 10, kCDEL = 15,
          kPDCD = 20, kMETH = 25, kERRO = 30 };
   int iact = strstr(acts, fgXAct) - acts;
   char *cmd = 0;

   switch (iact) {

      case kPRTF:
         printf("%s\n", (const char*)fgXArr[1]);
         fflush(stdout);
         break;

      case kERRO:
         {
            int level = (int)Long_t(fgXArr[1]);
            const char *location = (const char*)fgXArr[2];
            char *mess = (char*)fgXArr[3];
            if (level != kFatal)
               GetErrorHandler()(level, level >= gErrorAbortLevel, location, mess);
            else
               GetErrorHandler()(level, kTRUE, location, mess);
            delete [] mess;
         }
         break;

      case kCUPD:
         //((TCanvas *)fgXArr[1])->Update();
         cmd = Form("((TCanvas *)0x%lx)->Update();",(Long_t)fgXArr[1]);
         gROOT->ProcessLine(cmd);
         break;

      case kCANV:

         switch(fgXAnb) { // Over TCanvas constructors

            case 2:
               //((TCanvas*)fgXArr[1])->Constructor();
               cmd = Form("((TCanvas *)0x%lx)->Constructor();",(Long_t)fgXArr[1]);
               gROOT->ProcessLine(cmd);
               break;

            case 5:
               //((TCanvas*)fgXArr[1])->Constructor(
               //                 (char*)fgXArr[2],
               //                 (char*)fgXArr[3],
               //                *((Int_t*)(fgXArr[4])));
               cmd = Form("((TCanvas *)0x%lx)->Constructor((char*)0x%lx,(char*)0x%lx,*((Int_t*)(0x%lx)));",(Long_t)fgXArr[1],(Long_t)fgXArr[2],(Long_t)fgXArr[3],(Long_t)fgXArr[4]);
               gROOT->ProcessLine(cmd);
               break;
            case 6:
               //((TCanvas*)fgXArr[1])->Constructor(
               //                 (char*)fgXArr[2],
               //                 (char*)fgXArr[3],
               //                *((Int_t*)(fgXArr[4])),
               //                *((Int_t*)(fgXArr[5])));
               cmd = Form("((TCanvas *)0x%lx)->Constructor((char*)0x%lx,(char*)0x%lx,*((Int_t*)(0x%lx)),*((Int_t*)(0x%lx)));",(Long_t)fgXArr[1],(Long_t)fgXArr[2],(Long_t)fgXArr[3],(Long_t)fgXArr[4],(Long_t)fgXArr[5]);
               gROOT->ProcessLine(cmd);
               break;

            case 8:
               //((TCanvas*)fgXArr[1])->Constructor(
               //                 (char*)fgXArr[2],
               //                 (char*)fgXArr[3],
               //               *((Int_t*)(fgXArr[4])),
               //               *((Int_t*)(fgXArr[5])),
               //               *((Int_t*)(fgXArr[6])),
               //               *((Int_t*)(fgXArr[7])));
               cmd = Form("((TCanvas *)0x%lx)->Constructor((char*)0x%lx,(char*)0x%lx,*((Int_t*)(0x%lx)),*((Int_t*)(0x%lx)),*((Int_t*)(0x%lx)),*((Int_t*)(0x%lx)));",(Long_t)fgXArr[1],(Long_t)fgXArr[2],(Long_t)fgXArr[3],(Long_t)fgXArr[4],(Long_t)fgXArr[5],(Long_t)fgXArr[6],(Long_t)fgXArr[7]);
               gROOT->ProcessLine(cmd);
               break;

         }
         break;

      case kCDEL:
         //((TCanvas*)fgXArr[1])->Destructor();
         cmd = Form("((TCanvas *)0x%lx)->Destructor();",(Long_t)fgXArr[1]);
         gROOT->ProcessLine(cmd);
         break;

      case kPDCD:
         ((TVirtualPad*) fgXArr[1])->Divide( *((Int_t*)(fgXArr[2])),
                                  *((Int_t*)(fgXArr[3])),
                                  *((Float_t*)(fgXArr[4])),
                                  *((Float_t*)(fgXArr[5])),
                                  *((Int_t*)(fgXArr[6])));
         break;
      case kMETH:
         ((TMethodCall *) fgXArr[1])->Execute((void*)(fgXArr[2]),(const char*)(fgXArr[3]));
         break;

      default:
         ::Error("TThread::XAction", "wrong case");
   }

   fgXAct = 0;
   if (condimp) condimp->Signal();
   condmutex->UnLock();
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TThreadTimer                                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________
TThreadTimer::TThreadTimer(Long_t ms) : TTimer(ms, kTRUE)
{
   // Create thread timer.

   gSystem->AddTimer(this);
}

//______________________________________________________________________________
Bool_t TThreadTimer::Notify()
{
   // Periodically execute the TThread::XAxtion() method in the main thread.

   if (TThread::fgXAct) { TThread::XAction(); }
   Reset();

   return kFALSE;
}


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//  TThreadCleaner                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________
TThreadCleaner::~TThreadCleaner()
{
   // Call user clean up routines.

   TThread::CleanUp();
}
