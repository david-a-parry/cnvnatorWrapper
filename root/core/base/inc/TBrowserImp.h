// @(#)root/base:$Id$
// Author: Fons Rademakers   15/11/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#ifndef ROOT_TBrowserImp
#define ROOT_TBrowserImp

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TBrowserImp                                                          //
//                                                                      //
// ABC describing GUI independent browser implementation protocol.      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif

class TBrowser;
class TGMainFrame;

class TBrowserImp {

protected:
   TBrowser  *fBrowser;     //TBrowser associated with this implementation
   Bool_t     fShowCycles;  //Show object cycle numbers in browser

   TBrowserImp(const TBrowserImp& br)
     : fBrowser(br.fBrowser), fShowCycles(br.fShowCycles) { }
   TBrowserImp& operator=(const TBrowserImp& br)
     {if(this!=&br) {fBrowser=br.fBrowser; fShowCycles=br.fShowCycles;}
     return *this;}

public:
   TBrowserImp(TBrowser *b=0) : fBrowser(b), fShowCycles(kFALSE) { }
   TBrowserImp(TBrowser *b, const char *title, UInt_t width, UInt_t height, Option_t *opt = "");
   TBrowserImp(TBrowser *b, const char *title, Int_t x, Int_t y, UInt_t width, UInt_t height, Option_t *opt = "");
   virtual ~TBrowserImp() { }

   virtual void      Add(TObject *, const char *, Int_t) { }
   virtual void      AddCheckBox(TObject *, Bool_t = kFALSE) { }
   virtual void      CheckObjectItem(TObject *, Bool_t = kFALSE) { }
   virtual void      RemoveCheckBox(TObject *) { }
   virtual void      BrowseObj(TObject *) { }
   TBrowser         *Browser() const { return fBrowser; }
   virtual void      CloseTabs() { }
   virtual void      ExecuteDefaultAction(TObject *) { }
   virtual void      Iconify() { }
   virtual void      RecursiveRemove(TObject *) { }
   virtual void      Refresh(Bool_t = kFALSE) { }
   virtual void      Show() { }
   virtual void      SetDrawOption(Option_t *option="");
   virtual Option_t *GetDrawOption() const { return 0; }

   virtual Long_t    ExecPlugin(const char *, const char *, const char *, Int_t, Int_t) { return 0; }
   virtual void      SetStatusText(const char *, Int_t) { }
   virtual void      StartEmbedding(Int_t, Int_t) { }
   virtual void      StopEmbedding(const char *) { }

   virtual TGMainFrame *GetMainFrame() const { return 0; }

   virtual TBrowser *GetBrowser() const      { return fBrowser; }
   virtual void      SetBrowser(TBrowser *b) { fBrowser = b; }

   ClassDef(TBrowserImp,0)  //ABC describing browser implementation protocol
};

inline TBrowserImp::TBrowserImp(TBrowser *, const char *, UInt_t, UInt_t, Option_t *)
   : fBrowser(0), fShowCycles(kFALSE) { }
inline TBrowserImp::TBrowserImp(TBrowser *, const char *, Int_t, Int_t, UInt_t, UInt_t, Option_t *)
   : fBrowser(0), fShowCycles(kFALSE) { }

#endif
