// @(#)root/gpad:$Id$
// Author: Rene Brun  19/02/2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TView.h"
#include "TROOT.h"
#include "TPluginManager.h"

ClassImp(TView)

//______________________________________________________________________________
/* Begin_Html
See TView3D
End_Html */

//____________________________________________________________________________
TView::TView(const TView& tv) :
   TObject(tv),
   TAttLine(tv)
{
   // Copy constructor.
}

//____________________________________________________________________________
TView *TView::CreateView(Int_t system, const Double_t *rmin, const Double_t *rmax) 
{
   // Create a concrete default 3-d view via the plug-in manager
   
   TView *view = 0;
   TPluginHandler *h;
   if ((h = gROOT->GetPluginManager()->FindHandler("TView"))) {
      if (h->LoadPlugin() == -1)
         return 0;
      view = (TView*)h->ExecPlugin(3,system,rmin,rmax);
   }
   return view;
}
