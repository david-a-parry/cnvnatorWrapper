// @(#)root/rootxx-cocoa.mm:$Id$
// Author: Timur Pocheptsov   22/01/2014

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Rootxx-cocoa                                                         //
//                                                                      //
// Cocoa based routines used to display the splash screen for rootx,    //
// the root front-end program.                                          //
//                                                                      //
// WaitChildGeneric/StayUp/-main (ROOTWaitpidThread) copy-pasted/based  //
// on rootx.cxx/rootxx.cxx (author - Fons Rademakers).                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <fstream>
#include <cstdlib>
#include <csignal>
#include <string>
#include <cerrno>
#include <list>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pwd.h>

#include <Cocoa/Cocoa.h>

#include "CocoaUtils.h"
#include "RConfigure.h"
#include "RVersion.h"

//
//'root' with Cocoa is a quite special application. In principle, it's a background process,
//but it can create a GUI (in our case it's a simple splash-screen - a window with a ROOT's logo
//and "Credits" info. The first version was converting its process into foreground, which
//leads to some undesired effects like ... lost keyboard focus. Now this process never explicitly
//converted into foreground and [NSApp activateAppIgnorinxxxx] is never called.
//Instead, special window style is used - NSNonactivatingPanelMask (splash-screen's class derives NSPanel)
//and I still have to specify activation policy (to avoid NSInternalInconsistencyException on window creation).
//Also, I set a window level to NSFloatingPanelWindow (otherwise window is not visible).
//This behavior is still different from how X11 based root works on Mac, but is very close.
//


namespace ROOT {
namespace ROOTX {

//This had internal linkage before, now must be accessible from rootx-cocoa.mm.
extern int gChildpid;

}
}


namespace {

NSString * const gConception = @"Conception:  Rene Brun, Fons Rademakers\n\n";
NSString * const gLeadDevelopers = @"Lead Developers:  Rene Brun, Philippe Canal, Fons Rademakers\n\n";
//Ok, and poor little me.
NSString * const gRootDevelopers = @"Core Engineering:  Bertrand Bellenot, Olivier Couet, Gerardo Ganis,"
                                    "Andrei Gheata, Lorenzo Moneta, Axel Naumann, "
                                    "Paul Russo, Matevz Tadel, Timur Pocheptsov\n\n";
NSString * const gRootDocumentation = @"Documentation:  Ilka Antcheva\n\n";

bool showAboutInfo = false;

}

//ROOTSplashScreenView: content view for our panel (splash screen window)
//with a background (ROOT's logo) + scrollview and textview to show info
//about contributors.

@interface ROOTSplashScreenView : NSView
@end

@implementation ROOTSplashScreenView {
   NSImage *backgroundImage;
   NSScrollView *scrollView;
   NSTextView *textView;
}

//_________________________________________________________________
- (id) initWithImage : (NSImage *) image text : (NSAttributedString *) textToScroll aboutMode : (BOOL) about
{
   assert(image != nil && "initWithImage:text:, parameter 'image' is nil");
   assert(textToScroll != nil && "initWithImage:text:, parameter 'textToScroll' is nil");
   
   using ROOT::MacOSX::Util::NSScopeGuard;
   
   const CGSize imageSize = image.size;
   
   //minimal sizes required by text view's position (which is 'hardcoded' and
   //must be the same as in rootxx (X11 version).
   assert(imageSize.width >= 300 && imageSize.height >= 285 &&
          "initWithImage:text:, unexpected background image sizes");

   self = [super initWithFrame : CGRectMake(0, 0, imageSize.width, imageSize.height)];
   
   if (self) {
      //Let's create our child views.
      backgroundImage = [image retain];

      //In rootxx it's: x == 15, y == 155, w == 285, h == 130 (top-left corner).
      //In Cocoa it's counted from the bottom left: y = height - yX11 - h
      CGRect scrollRect = CGRectMake(15., imageSize.height - 285., 285., 130.);
      scrollView = [[NSScrollView alloc] initWithFrame : scrollRect];
      [self addSubview : scrollView];
      
      scrollRect.origin = CGPoint();
      textView = [[NSTextView alloc] initWithFrame : scrollRect];
      [textView setEditable : NO];
      
      [[textView textStorage] setAttributedString : textToScroll];
      
      [scrollView setDocumentView : textView];
      [scrollView setBorderType : NSNoBorder];


      if (about) {
         [textView setTextContainerInset : CGSizeMake(0., scrollRect.size.height)];
         [textView scrollPoint : NSMakePoint(0., scrollRect.size.height)];
      }

      //Hehehehe.
      [scrollView setDrawsBackground : NO];
      [textView setDrawsBackground : NO];
   }
   
   return self;
}

//_________________________________________________________________
- (void) dealloc
{
   [backgroundImage release];
   [textView release];
   [scrollView release];
   
   [super dealloc];
}

//Background image.

//_________________________________________________________________
- (void) drawRect : (NSRect) rect
{
#pragma unused(rect)
   assert(backgroundImage != nil && "drawRect:, backgroundImage is nil");
   
   CGRect frame = self.frame;
   frame.origin = CGPoint();

   const CGSize imageSize = backgroundImage.size;
   [backgroundImage drawInRect : frame
                    fromRect : CGRectMake(0., 0., imageSize.width, imageSize.height)
                    operation : NSCompositeSourceOver
                    fraction : 1.];
   
   //Let's now draw a version.
   if (NSString * const version = [NSString stringWithFormat : @"Version %s", ROOT_RELEASE]) {
      if (NSFont * const font = [NSFont fontWithName : @"Helvetica" size : 11.]) {
         NSDictionary * dict = [NSDictionary dictionaryWithObject : font forKey : NSFontAttributeName];
         [version drawAtPoint : CGPointMake(15., 15.) withAttributes : dict];
      }
   }
}

//Animation.

//_________________________________________________________________
- (void) scrollText
{
   const CGFloat scrollAmountPixels = 1.;
   // How far have we scrolled so far?
   const CGFloat currentScrollAmount = [scrollView documentVisibleRect].origin.y;
   
   if (currentScrollAmount + scrollAmountPixels >= textView.frame.size.height - scrollView.frame.size.height)
      [textView scrollPoint : NSMakePoint(0., 0.)];//Make a "loop".
   else {
      [textView scrollPoint : NSMakePoint(0., currentScrollAmount + scrollAmountPixels)];
   }
   // If anything overlaps the text we just scrolled, it won’t get redraw by the
   // scrolling, so force everything in that part of the panel to redraw.
   NSRect scrollViewFrame = [scrollView bounds];
   scrollViewFrame = [self convertRect : scrollViewFrame fromView : scrollView];
   // Redraw everything which overlaps it.
   [self setNeedsDisplayInRect : scrollViewFrame];
}

@end

//To be set from a signal handler.
namespace {

bool popupDone = false;

}


//Our "top-level-window" - borderless panel.

@interface ROOTSplashScreenPanel : NSPanel
@end

@implementation ROOTSplashScreenPanel

//Events and behaviour.

//_________________________________________________________________
- (BOOL) canBecomeMainWindow
{
   return NO;//YES;
}

//_________________________________________________________________
- (BOOL) canBecomeKeyWindow
{
   return NO;//YES;
}

//_________________________________________________________________
- (void) sendEvent : (NSEvent *) theEvent
{
   if ([theEvent type] == NSKeyDown) {
      popupDone = true;
      return;
   }

   if ([theEvent type] == NSLeftMouseDown || [theEvent type] == NSRightMouseDown) {
      popupDone = true;
      return;
   }

   [super sendEvent : theEvent];
}

@end


//
//Waitpid is a blocking call and we're a foreground process (a 'normal' Cocoa application).
//Waitpid with WNOHUNG return immediately and does not solve our problems.
//Here's the trick: create a background thread executing ... waitpid
//and performing its special selector on a main thread (when status changed).
//

namespace {

//Timer callbacks 'fire' custom NSEvents:
enum CustomEventSource {//make it enum class when C++11 is here.
   kScrollTimer = 1,
   kSignalTimer = 2,
   kWaitpidThread = 3
};

}

@interface ROOTWaitpidThread : NSThread {
   int status;
   int result;
   bool normalExit;
}

- (int) getStatus;

@end

@implementation ROOTWaitpidThread

//_________________________________________________________________
- (id) init
{
   if (self = [super init]) {
      status = 0;
      result = 0;
      normalExit = false;
   }
   
   return self;
}

//_________________________________________________________________
- (int) getStatus
{
   return status;
}

//_________________________________________________________________
- (void) main
{
   using ROOT::ROOTX::gChildpid;
   
   do {
      while ((result = ::waitpid(gChildpid, &status, WUNTRACED) < 0)) {
         if (errno != EINTR)
            break;
         errno = 0;
      }

      if (WIFEXITED(status) || WIFSIGNALED(status)) {
         [self performSelectorOnMainThread : @selector(exitEventLoop) withObject : nil waitUntilDone : NO];
         return;
      }

      if (WIFSTOPPED(status)) {         // child got ctlr-Z
         ::raise(SIGTSTP);                // stop also parent
         ::kill(gChildpid, SIGCONT);       // if parent wakes up, wake up child
      }
   } while (WIFSTOPPED(status));

   normalExit = true;
   [self performSelectorOnMainThread : @selector(exitEventLoop) withObject : nil waitUntilDone : NO];

}

//_________________________________________________________________
- (void) exitEventLoop
{
   //assert - we are on a main thread.
   NSEvent * const timerEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                                 timestamp : 0. windowNumber : 0 context : nil subtype : 0 data1 : kWaitpidThread data2 : 0];
   [NSApp postEvent : timerEvent atStart : NO];
}

@end


@interface ROOTSplashScreenAppDelegate : NSObject<NSApplicationDelegate>
@end

@implementation ROOTSplashScreenAppDelegate

//_________________________________________________________________
- (void) applicationDidResignActive : (NSNotification *) aNotification
{
#pragma unused(aNotification)
   popupDone = true;
}

@end

namespace {

volatile sig_atomic_t popdown = 0;

ROOTSplashScreenPanel *splashScreen = nil;
//Top-level autorelease pool.
NSAutoreleasePool * topLevelPool = nil;

//We use a signal timer to check:
//if we got a SIGUSR1 (see rootx.cxx) or
//if we have to remove a splash-screen after a delay.
CFRunLoopTimerRef signalTimer = 0;
const CFTimeInterval signalInterval = 0.1;

timeval popupCreationTime;
const CFTimeInterval splashScreenDelayInSec = 2.;//4 seconds in rootx.cxx, I'm gonna use 2.

//Timer for a scroll animation.
CFRunLoopTimerRef scrollTimer = 0;
const CFTimeInterval scrollInterval = 0.1;

//Aux. functions:
bool InitCocoa();
bool InitTimers();
void RunEventLoop();
void RunEventLoopInBackground();
bool StayUp();
bool CreateSplashscreen(bool about);
void SetSplashscreenPosition();
//Non GUI aux. function.
NSAttributedString *CreateTextToScroll(bool about);
bool ReadContributors(std::list<std::string> & contributors);

}//unnamed namespace.


//Platform-specific (OS X) versions of PopupLogo, WaitLogo, PopdownLogo, CloseDisplay.
//_________________________________________________________________
void PopupLogo(bool about)
{
   if (!InitCocoa()) {
      //TODO: diagnostic.
      return;
   }

   //0. For StayUp to check when we should hide our splash-screen.
   if (gettimeofday(&popupCreationTime, 0) == -1) {
      //TODO: check errno and issue a message,
      //we need a valid popup creation time.
      return;
   }

   if (!CreateSplashscreen(about)) {
      //TODO: diagnostic.
      return;
   }
   
   showAboutInfo = about;
   SetSplashscreenPosition();
   [splashScreen setLevel : NSFloatingWindowLevel];
   [splashScreen makeKeyAndOrderFront : nil];
}

//_________________________________________________________________
void PopdownLogo()
{
   //This function is called from the signal handler.
   popdown = 1;
}

//_________________________________________________________________
void WaitLogo()
{
   if (!splashScreen)
      //TODO: diagnostic.
      return;
   
   ROOTSplashScreenAppDelegate *delegate = nil;
   
   if (showAboutInfo) {
      delegate = [[ROOTSplashScreenAppDelegate alloc] init];
      [[NSApplication sharedApplication] setDelegate : delegate];
   }

   RunEventLoop();
   
   //Cleanup.
   [splashScreen orderOut : nil];
   [splashScreen release];
   splashScreen = nil;

   if (showAboutInfo) {
      [NSApp setDelegate : nil];
      [delegate release];
   }
}

//_________________________________________________________________
void CloseDisplay()
{
   //Noop.
}

//Aux. functions.

extern "C" {

//_________________________________________________________________
void ROOTSplashscreenTimerCallback(CFRunLoopTimerRef timer, void *info)
{
#pragma unused(info)
   if (timer == signalTimer) {
      NSEvent * const timerEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                                    timestamp : 0. windowNumber : 0 context : nil subtype : 0 data1 : kSignalTimer data2 : 0];
      [NSApp postEvent : timerEvent atStart : NO];
   } else {
      NSEvent * const timerEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                                    timestamp : 0. windowNumber : 0 context : nil subtype : 0 data1 : kScrollTimer data2 : 0];
      [NSApp postEvent : timerEvent atStart : NO];
   }
}

}//extern "C"

namespace {

//_________________________________________________________________
bool InitCocoa()
{
   if (!topLevelPool) {
      [[NSApplication sharedApplication] setActivationPolicy : NSApplicationActivationPolicyAccessory];//Do I need this?
      topLevelPool = [[NSAutoreleasePool alloc] init];
   }
   
   return true;
}

//_________________________________________________________________
bool InitTimers()
{
   assert(scrollTimer == 0 && "InitTimers, scrollTimer was initialized already");
   assert(signalTimer == 0 && "InitTimers, signalTimer was initialized already");

   using ROOT::MacOSX::Util::CFScopeGuard;
   
   CFScopeGuard<CFRunLoopTimerRef> guard1(CFRunLoopTimerCreate(kCFAllocatorDefault,//allocator
                                                               CFAbsoluteTimeGetCurrent() + signalInterval,//fireDate
                                                               signalInterval,//interval in seconds(?)
                                                               0,//flags - not used
                                                               0,//order - not used
                                                               ROOTSplashscreenTimerCallback,
                                                               0//info
                                                               ));
   if (!guard1.Get())
      return false;

   //Scroll animation.
   if (showAboutInfo) {
      CFScopeGuard<CFRunLoopTimerRef> guard2(CFRunLoopTimerCreate(kCFAllocatorDefault,
                                                                  CFAbsoluteTimeGetCurrent() + splashScreenDelayInSec,
                                                                  scrollInterval,
                                                                  0,
                                                                  0,
                                                                  ROOTSplashscreenTimerCallback,
                                                                  0
                                                                  ));
      if (!guard2.Get())
         return false;

      //TODO: refactor CFScopeGuard to return the pointer from Release.
      scrollTimer = guard2.Get();
      guard2.Release();
   }

   signalTimer = guard1.Get();
   guard1.Release();

   return true;
}

//_________________________________________________________________
void AttachTimers()
{
   assert(signalTimer != 0 && "AttachTimer, invalid signalTimer (null)");
   CFRunLoopAddTimer(CFRunLoopGetMain(), signalTimer, kCFRunLoopCommonModes);
   
   if (showAboutInfo) {
      assert(scrollTimer != 0 && "AttachTimer, invalid scrollTimer (null)");
      CFRunLoopAddTimer(CFRunLoopGetMain(), scrollTimer, kCFRunLoopCommonModes);
   }
}

//_________________________________________________________________
void RemoveTimers()
{
   assert(signalTimer != 0 && "RemoveTimers, signalTimer is null");
   
   CFRunLoopRemoveTimer(CFRunLoopGetMain(), signalTimer, kCFRunLoopCommonModes);
   CFRunLoopTimerInvalidate(signalTimer);
   //TODO: test if I also have to call release!!!
   signalTimer = 0;
   
   if (showAboutInfo) {
      assert(scrollTimer != 0 && "RemoveTimers, scrollTimer is null");
      CFRunLoopRemoveTimer(CFRunLoopGetMain(), scrollTimer, kCFRunLoopCommonModes);
      CFRunLoopTimerInvalidate(scrollTimer);
      //TODO: test if I also have to call release!!!
      scrollTimer = 0;
   }
}

//_________________________________________________________________
void RunEventLoop()
{
   //Kind of event loop.
   
   if (!splashScreen)
      //TODO: diagnostic.
      return;
   
   if (!InitTimers())
      return;
   
   AttachTimers();
   
   popupDone = false;

   while (!popupDone) {
      //Here we (possibly) suspend waiting for event.
      using ROOT::MacOSX::Util::NSScopeGuard;
      const NSScopeGuard<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
      
      if (NSEvent * const event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : [NSDate distantFuture] inMode : NSDefaultRunLoopMode dequeue : YES]) {
         //Let's first check the type:
         if (event.type == NSApplicationDefined) {//One of our timers 'fired'.
            if (event.data1 == kSignalTimer) {
               popupDone = !showAboutInfo && !StayUp() && popdown;
            } else if (showAboutInfo) {
               assert([[splashScreen contentView] isKindOfClass : [ROOTSplashScreenView class]] &&
                      "RunEventLoop, splashScreen.contentView has a wrong type");
               [(ROOTSplashScreenView *)[splashScreen contentView] scrollText];
            }
         } else
            [NSApp sendEvent : event];
      }
   }
   
   RemoveTimers();
   //Empty the queue (hehehe, this makes me feel ... uneasy :) ).
   while ([NSApp nextEventMatchingMask : NSAnyEventMask untilDate : nil inMode : NSDefaultRunLoopMode dequeue : YES]);
}

//_________________________________________________________________
void WaitChildGeneric()
{
   //If things with a waitpid thread went wrong, this function is called.
   //Wait till child (i.e. ROOT) is finished. From rootx.cxx.

   using ROOT::ROOTX::gChildpid;
   
   int status = 0;

   do {
      while (::waitpid(gChildpid, &status, WUNTRACED) < 0) {
         if (errno != EINTR)
            break;
         errno = 0;
      }

      if (WIFEXITED(status))
         std::exit(WEXITSTATUS(status));

      if (WIFSIGNALED(status))
         std::exit(WTERMSIG(status));

      if (WIFSTOPPED(status)) {         // child got ctlr-Z
         ::raise(SIGTSTP);                // stop also parent
         ::kill(gChildpid, SIGCONT);       // if parent wakes up, wake up child
      }
   } while (WIFSTOPPED(status));

   std::exit(0);
}

//_________________________________________________________________
void RunEventLoopInBackground()
{
   using ROOT::MacOSX::Util::NSScopeGuard;
   
   if (!InitCocoa()) {
      //It's a serious bug and must be either reported or handled in a different way.
      return WaitChildGeneric();
   }

   int status = 0;
   
   {//Block to force a scope guard's lifetime.
   const NSScopeGuard<ROOTWaitpidThread> thread([[ROOTWaitpidThread alloc] init]);
   if (!thread.Get()) {
      //TODO: diagnostic.
      return WaitChildGeneric();
   } else {
      [thread.Get() start];

      while (true) {
         //Autorelease pool?
         using ROOT::MacOSX::Util::NSScopeGuard;
         const NSScopeGuard<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);

         if (NSEvent * const event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : [NSDate distantFuture] inMode : NSDefaultRunLoopMode dequeue : YES]) {
            if (event.type == NSApplicationDefined && event.data1 == kWaitpidThread) {
               [thread.Get() cancel];
               status = [thread.Get() getStatus];
               break;
            }
            else
               [NSApp sendEvent : event];
         }
      }
      
      [NSApp hide : nil];//deactivate?
   }
   }//to force thread release.

   if (WIFEXITED(status))
      std::exit(WEXITSTATUS(status));
   if (WIFSIGNALED(status))
      std::exit(WTERMSIG(status));
}

//_________________________________________________________________
bool StayUp()
{
   //Taken from rootxx.cxx.
   const int splashScreenDelay = int(splashScreenDelayInSec * 1000);

   timeval ctv = {};
   timeval dtv = {};
   timeval tv = {};
   timeval ptv = popupCreationTime;

   tv.tv_sec  = splashScreenDelay / 1000;
   tv.tv_usec = (splashScreenDelay % 1000) * 1000;

   gettimeofday(&ctv, 0);
   if ((dtv.tv_usec = ctv.tv_usec - ptv.tv_usec) < 0) {
      dtv.tv_usec += 1000000;
      ptv.tv_sec++;
   }

   dtv.tv_sec = ctv.tv_sec - ptv.tv_sec;
   if ((ctv.tv_usec = tv.tv_usec - dtv.tv_usec) < 0) {
      ctv.tv_usec += 1000000;
      dtv.tv_sec++;
   }
   
   ctv.tv_sec = tv.tv_sec - dtv.tv_sec;

   if (ctv.tv_sec < 0)
      return false;

   return true;
}

//_________________________________________________________________
bool CreateSplashscreen(bool about)
{
   //Try to create NSImage out of Splash.gif, create NSPanel
   //with ROOTSplashscreenView as its content view + our background image + text in a scroll view.

   using ROOT::MacOSX::Util::NSScopeGuard;

   //0. Text to show.
   const NSScopeGuard<NSAttributedString> textToScroll(CreateTextToScroll(about));
   if (!textToScroll.Get())
      //Diagnostic was issued by CreateTextToScroll (TODO though).
      return false;

   //1. Image for splash screen's background.
#ifdef ROOTICONPATH
   const std::string fileName(std::string(ROOTICONPATH) + "/Splash.gif");
#else
   const char * const env = std::getenv("ROOTSYS");
   if (!env) {
      //TODO: diagnostic.
      return false;
   }
   
   const std::string fileName(std::string(env) + "/icons/Splash.gif");
#endif

   const NSScopeGuard<NSString> nsStringGuard([[NSString alloc] initWithFormat : @"%s", fileName.c_str()]);
   if (!nsStringGuard.Get()) {
      //TODO: diagnostic.
      return false;
   }
   
   const NSScopeGuard<NSImage> imageGuard([[NSImage alloc] initWithContentsOfFile : nsStringGuard.Get()]);
   if (!imageGuard.Get()) {
      //TODO: diagnostic.
      return false;
   }
   
   const CGSize imageSize = imageGuard.Get().size;
   //These sizes are from X11 version, they are related to the geometry of a scroll view.
   if (imageSize.width < 300 || imageSize.height < 285) {
      //TODO: diagnostic.
      return false;
   }
   
   //2. Splash-screen ('panel' + its content view).
   NSScopeGuard<ROOTSplashScreenPanel> splashGuard([[ROOTSplashScreenPanel alloc]
                                                    initWithContentRect : CGRectMake(0, 0, imageSize.width, imageSize.height)
                                                    styleMask : NSNonactivatingPanelMask
                                                    backing : NSBackingStoreBuffered
                                                    defer : NO]);
   if (!splashGuard.Get()) {
      //TODO: diagnostic.
      return false;
   }

   const NSScopeGuard<ROOTSplashScreenView> viewGuard([[ROOTSplashScreenView alloc] initWithImage : imageGuard.Get() text : textToScroll.Get() aboutMode : about]);
   if (!viewGuard.Get()) {
      //TODO: diagnostic.
      return false;
   }
   
   [splashGuard.Get() setContentView : viewGuard.Get()];
   splashScreen = splashGuard.Get();
   splashGuard.Release();
   
   return true;
}

//_________________________________________________________________
void SetSplashscreenPosition()
{
   assert(splashScreen != nil && "SetSplashscreenPosition, splashScreen is nil");
   
   //Set the splash-screen's position (can it be wrong for a multi-head setup?)
   //TODO: check with a secondary display.
   if (NSScreen * const screen = [NSScreen mainScreen]) {
      const NSRect screenFrame = screen.frame;
      const CGSize splashSize = splashScreen.frame.size;

      const CGPoint origin = CGPointMake(screenFrame.origin.x + screenFrame.size.width / 2 - splashSize.width / 2,
                                         screenFrame.origin.y + screenFrame.size.height / 2 - splashSize.height / 2);

      [splashScreen setFrameOrigin : origin];
   }//else - is it possible? TODO: diagnostic.
}

//Aux. "non-GUI" functions.

//_________________________________________________________________
bool AddDeveloperInfo(NSMutableAttributedString *textToScroll)
{
   assert(textToScroll != nil && "AddDeveloperInfo, parameter 'textToScroll' is nil");
   
   using ROOT::MacOSX::Util::NSScopeGuard;

   NSScopeGuard<NSAttributedString> newString([[NSAttributedString alloc] initWithString : gConception]);
   if (!newString.Get())
      //TODO: diagnostic.
      return false;
   [textToScroll appendAttributedString : newString.Get()];
   
   newString.Reset([[NSAttributedString alloc] initWithString : gLeadDevelopers]);
   if (!newString.Get())
      //TODO: diagnostic.
      return false;
   [textToScroll appendAttributedString : newString.Get()];
   
   newString.Reset([[NSAttributedString alloc] initWithString : gRootDevelopers]);
   if (!newString.Get())
      //TODO: diagnostic.
      return false;
   [textToScroll appendAttributedString : newString.Get()];
   
   newString.Reset([[NSAttributedString alloc] initWithString : gRootDocumentation]);
   if (!newString.Get())
      //TODO: diagnostic.
      return false;
   [textToScroll appendAttributedString : newString.Get()];
   
   return true;
}

//_________________________________________________________________
void AddContributorsInfo(NSMutableAttributedString *textToScroll)
{
   //TODO: diagnostic and error handling + problems with non-ascii.

   assert(textToScroll != nil && "AddContributorsInfo, parameter 'textToScroll' is nil");
   //
   std::list<std::string> contributors;
   ReadContributors(contributors);

   if (contributors.size()) {//Add more lines here.
      using ROOT::MacOSX::Util::NSScopeGuard;
      
      NSScopeGuard<NSAttributedString> newString([[NSAttributedString alloc] initWithString : @"Contributors:  "]);
      if (!newString.Get())
         return;
      [textToScroll appendAttributedString : newString.Get()];
   
      std::list<std::string>::const_iterator it = contributors.begin(), end = contributors.end(), begin = contributors.begin();
      for (; it != end; ++it) {
         //Quite ugly :( NSString/NSAttributedString ARE ugly.
         NSString * const nsFromC = [NSString stringWithFormat : it != begin ? @", %s" : @"%s", it->c_str()];
         newString.Reset([[NSAttributedString alloc] initWithString : nsFromC]);
         if (newString.Get())
            [textToScroll appendAttributedString : newString.Get()];
      }
      
      newString.Reset([[NSAttributedString alloc] initWithString :
                      @"\n\nOur sincere thanks and apologies to anyone who deserves credit but fails to appear in this list."]);
   }
}

//_________________________________________________________________
void AddUserInfo(NSMutableAttributedString *textToScroll)
{
   //TODO: Fix - probably, it does not work with non-ascii? (if pw_gecos/pw_name can be mb strings?).

   assert(textToScroll != nil && "AddUserInfo, parameter 'textToScroll' is nil");

   if (const passwd * const pwd = getpwuid(getuid())) {
      std::string name;
      
      if (pwd->pw_gecos) {
         const char * const comma = std::strchr(pwd->pw_gecos, ',');
         if (!comma)
            name = pwd->pw_gecos;
         else if (pwd->pw_gecos - comma)
            name.assign(pwd->pw_gecos, pwd->pw_gecos - comma);
      }
      
      if (!name.length() && pwd->pw_name)
         name = pwd->pw_name;
      
      if (!name.length())
         return;
      
      using ROOT::MacOSX::Util::NSScopeGuard;
      NSString * const nsFromC = [NSString stringWithFormat : @"\n\nExtra special thanks go to %s, one of our favorite users.", name.c_str()];
      const NSScopeGuard<NSAttributedString> newString([[NSAttributedString alloc] initWithString : nsFromC]);

      if (newString.Get())
         [textToScroll appendAttributedString : newString.Get()];
   }
}

//_________________________________________________________________
NSAttributedString *CreateTextToScroll(bool about)
{
   using ROOT::MacOSX::Util::NSScopeGuard;

   //the resulting string.
   NSScopeGuard<NSMutableAttributedString> textToScroll([[NSMutableAttributedString alloc] init]);
   if (!textToScroll.Get())
      //TODO: diagnostic.
      return nil;

   if (!AddDeveloperInfo(textToScroll.Get()))
      return nil;

   //Read contributors.
   if (about) {
      AddContributorsInfo(textToScroll.Get());
      AddUserInfo(textToScroll.Get());
   }

   if (NSFont * const font = [NSFont fontWithName : @"Helvetica" size : 11.]) {
      NSDictionary * dict = [NSDictionary dictionaryWithObject : font forKey : NSFontAttributeName];
      [textToScroll.Get() setAttributes : dict range : NSMakeRange(0, textToScroll.Get().length)];
   }

   NSAttributedString * const result = textToScroll.Get();
   textToScroll.Release();

   return result;
}

//_________________________________________________________________
bool ReadContributors(std::list<std::string> & contributors)
{
   //TODO: Fix - does not work with something except ascii.

#ifdef ROOTDOCDIR
   const std::string fileName(std::string(ROOTDOCDIR) + "/CREDITS");
#else
   const char * const env = std::getenv("ROOTSYS");
   if (!env)
      //TODO: diagnostic?
      return false;
   
   const std::string fileName(std::string(env) + "/README/CREDITS");
#endif

   std::ifstream inputFile(fileName.c_str());
   if (!inputFile)
      return false;

   std::list<std::string> tmp;
   std::string line(200, ' ');
   while (std::getline(inputFile, line)) {
      if (line.length() > 3) {
         if (line[0] == 'N' && line[1] == ':' && line[2] == ' ')
            //Let's hope this substring is not a pile of whitespaces.
            tmp.push_back(line.substr(3, line.length() - 3));
      }
   }
   
   tmp.swap(contributors);
   
   return true;
}

}//unnamed namespace.


namespace ROOT {
namespace ROOTX {

//This is a 'replacement' version.
//_________________________________________________________________
void WaitChild()
{
   RunEventLoopInBackground();
}

}
}
