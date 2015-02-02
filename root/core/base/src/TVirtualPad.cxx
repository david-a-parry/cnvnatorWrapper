// @(#)root/base:$Id$
// Author: Rene Brun   05/12/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TVirtualPad.h"
#include "X3DBuffer.h"
#include "TClass.h"
#include "TThreadSlots.h"

//______________________________________________________________________________
//
//  TVirtualPad is an abstract base class for the Pad and Canvas classes.
//

Size3D gVarSize3D;
Size3D* gFuncSize3D(){ return &gVarSize3D; }

Int_t (*gThreadXAR)(const char *xact, Int_t nb, void **ar, Int_t *iret) = 0;

//______________________________________________________________________________
TVirtualPad *&TVirtualPad::Pad()
{
   // Return the current pad for the current thread.

   static TVirtualPad *currentPad = 0;
   if (!gThreadTsd)
      return currentPad;
   else
      return *(TVirtualPad**)(*gThreadTsd)(&currentPad,ROOT::kPadThreadSlot);
}

ClassImp(TVirtualPad)

//______________________________________________________________________________
TVirtualPad::TVirtualPad() : TAttPad()
{
   // VirtualPad default constructor

   fResizing = kFALSE;
}

//______________________________________________________________________________
TVirtualPad::TVirtualPad(const char *, const char *, Double_t,
           Double_t, Double_t, Double_t, Color_t color, Short_t , Short_t)
          : TAttPad()
{
   // VirtualPad constructor

   fResizing = kFALSE;

   SetFillColor(color);
   SetFillStyle(1001);
}

//______________________________________________________________________________
TVirtualPad::~TVirtualPad()
{
   // VirtualPad destructor

}

//______________________________________________________________________________
void TVirtualPad::Streamer(TBuffer &R__b)
{
   // Stream an object of class TVirtualPad.

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v > 1) {
         R__b.ReadClassBuffer(TVirtualPad::Class(), this, R__v, R__s, R__c);
         return;
      }
      //====process old versions before automatic schema evolution
      TObject::Streamer(R__b);
      TAttLine::Streamer(R__b);
      TAttFill::Streamer(R__b);
      TAttPad::Streamer(R__b);
      //====end of old versions

   } else {
      R__b.WriteClassBuffer(TVirtualPad::Class(),this);
   }
}

//______________________________________________________________________________
Bool_t TVirtualPad::PadInSelectionMode() const
{
   // Should always return false unless you have non-standard picking.

   return kFALSE;
}

//______________________________________________________________________________
Bool_t TVirtualPad::PadInHighlightMode() const
{
   // Should always return false, unless you can highlight selected object in pad.

   return kFALSE;
}

//______________________________________________________________________________
void TVirtualPad::PushTopLevelSelectable(TObject * /*object*/)
{
   // Does nothing, unless you implement your own picking.
   // When complex object containing sub-objects (which can be picked)
   // is painted in a pad, this "top-level" object is pushed into 
   // the selectables stack.
}

//______________________________________________________________________________
void TVirtualPad::PushSelectableObject(TObject * /*object*/)
{
   // Does nothing, unless you implement your own picking.
   // "Complete" object, or part of complex object, which
   // can be picked.
}

//______________________________________________________________________________
void TVirtualPad::PopTopLevelSelectable()
{
   // Does nothing, unless you implement your own picking.
   // Remove top level selectable and all its' children.
}

//______________________________________________________________________________
TPickerStackGuard::TPickerStackGuard(TObject *obj)
{
   // Scope-guards ctor, pushe the object on stack.

   gPad->PushTopLevelSelectable(obj);
}

//______________________________________________________________________________
TPickerStackGuard::~TPickerStackGuard()
{
   // Guard does out of scope, pop object from stack.

   gPad->PopTopLevelSelectable();
}
