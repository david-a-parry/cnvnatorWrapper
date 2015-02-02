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
// TPosixThread                                                         //
//                                                                      //
// This class provides an interface to the posix thread routines.       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TPosixThread.h"


ClassImp(TPosixThread)


//______________________________________________________________________________
Int_t TPosixThread::Run(TThread *th)
{
   // Create a pthread. Returns 0 on success, otherwise an error number will
   // be returned.

   int det;
   pthread_t id;
   pthread_attr_t *attr = new pthread_attr_t;

   pthread_attr_init(attr);

   // Set detach state
   det = (th->fDetached) ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE;

   pthread_attr_setdetachstate(attr, det);

   // See e.g. https://developer.apple.com/library/mac/#qa/qa1419/_index.html
   // MacOS has only 512k of stack per thread; Linux has 2MB.
   const size_t requiredStackSize = 1024*1024*2;
   size_t stackSize = 0;
   if (!pthread_attr_getstacksize(attr, &stackSize)
       && stackSize < requiredStackSize) {
      pthread_attr_setstacksize(attr, requiredStackSize);
   }
   int ierr = pthread_create(&id, attr, &TThread::Function, th);
   if (!ierr) th->fId = (Long_t) id;

   pthread_attr_destroy(attr);
   delete attr;

   return ierr;
}

//______________________________________________________________________________
Int_t TPosixThread::Join(TThread *th, void **ret)
{
   // Join  suspends  the  execution  of the calling thread until the
   // thread identified by th terminates, either by  calling  pthread_exit
   // or by being cancelled. Returns 0 on success, otherwise an error number will
   // be returned.

   return pthread_join((pthread_t) th->fId, ret);
}

//______________________________________________________________________________
Int_t TPosixThread::Exit(void *ret)
{
   // Terminates the execution of the calling thread. Return 0.

   pthread_exit(ret);
   return 0;
}

//______________________________________________________________________________
Int_t TPosixThread::Kill(TThread *th)
{
   // Cancellation is the mechanism by which a thread can terminate the
   // execution of another thread. Returns 0 on success, otherwise an error
   // number will be returned.

   return pthread_cancel((pthread_t) th->fId);
}

//______________________________________________________________________________
Int_t TPosixThread::SetCancelOff()
{
   // Turn off the cancellation state of the calling thread. Returns 0 on
   // success, otherwise an error number will be returned.

   return pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
}

//______________________________________________________________________________
Int_t TPosixThread::SetCancelOn()
{
   // Turn on the cancellation state of the calling thread. Returns 0 on
   // success, otherwise an error number will be returned.

   return pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
}

//______________________________________________________________________________
Int_t TPosixThread::SetCancelAsynchronous()
{
   // Set the cancellation response type of the calling thread to
   // asynchronous, i.e. cancel as soon as the cancellation request
   // is received.

   return pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
}

//______________________________________________________________________________
Int_t TPosixThread::SetCancelDeferred()
{
   // Set the cancellation response type of the calling thread to
   // deferred, i.e. cancel only at next cancellation point.
   // Returns 0 on success, otherwise an error number will be returned.

   return pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, 0);
}

//______________________________________________________________________________
Int_t TPosixThread::CancelPoint()
{
   // Introduce an explicit cancellation point. Returns 0.

   int istate;
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &istate);
   pthread_testcancel();
   pthread_setcancelstate(istate, 0);

   return 0;
}

//______________________________________________________________________________
Int_t TPosixThread::CleanUpPush(void **main, void *free, void *arg)
{
   // Add thread cleanup function.

   // pthread_cleanup_push(free, arg);
   if (!free) Error("CleanUpPush", "cleanup rountine = 0");
   new TPosixThreadCleanUp(main, free, arg);
   return 0;
}

//______________________________________________________________________________
Int_t TPosixThread::CleanUpPop(void **main,Int_t exe)
{
   // Pop thread cleanup function from stack.

   //  pthread_cleanup_pop(exe); // happy pthread future

   if (!main || !*main) return 1;
   TPosixThreadCleanUp *l = (TPosixThreadCleanUp*)(*main);
   if (!l->fRoutine) Error("CleanUpPop", "cleanup routine = 0");
   if (exe && l->fRoutine) ((void (*)(void*))(l->fRoutine))(l->fArgument);
   *main = l->fNext;  delete l;
   return 0;
}

//______________________________________________________________________________
Int_t TPosixThread::CleanUp(void **main)
{
   // Default thread cleanup routine.

   if (gDebug > 0)
      Info("Cleanup", "cleanup 0x%lx", (Long_t)*main);
   while (!CleanUpPop(main, 1)) { }
   return 0;
}

//______________________________________________________________________________
Long_t TPosixThread::SelfId()
{
   // Return the thread identifier for the calling thread.

   return (Long_t) pthread_self();
}

//   Clean Up section. PTHREAD implementations of cleanup after cancel are
//   too different and often too bad. Temporary I invent my own bicycle.
//                                                              V.Perev.

//______________________________________________________________________________
TPosixThreadCleanUp::TPosixThreadCleanUp(void **main, void *routine, void *arg)
{
   //cleanup function
   fNext = (TPosixThreadCleanUp*)*main;
   fRoutine = routine; fArgument = arg;
   *main  = this;
}
