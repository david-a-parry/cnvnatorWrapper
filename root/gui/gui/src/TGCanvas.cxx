// @(#)root/gui:$Id$
// Author: Fons Rademakers   11/01/98

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
/**************************************************************************

    This source is based on Xclass95, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

    Xclass95 is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

**************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TGCanvas and TGViewPort and TGContainer                              //
//                                                                      //
// A TGCanvas is a frame containing two scrollbars (a horizontal and    //
// a vertical) and a viewport. The viewport acts as the window through  //
// which we look at the contents of the container frame.                //
//                                                                      //
// A TGContainer frame manages a content area. It can display and       //
// control a hierarchy of multi-column items, and provides the ability  //
// to add new items at any time. By default it doesn't map subwindows   //
// which are items of the container. In this case subwindow must        //
// provide DrawCopy method, see for example TGLVEntry class.            //
// It is also possible to use option which allow to map subwindows.     //
// This option has much slower drawing speed in case of more than 1000  //
// items placed in container. To activate this option the fMapSubwindows//
// data member must be set to kTRUE (for example TTVLVContainer class)  //
//                                                                      //
//   The TGContainer class can handle the keys:                         //
//                                                                      //
//    o  F7, Ctnrl-F - activate search dialog                           //
//    o  F3, Ctnrl-G - continue search                                  //
//    o  End - go to the last item in container                         //
//    o  Home - go to the first item in container                       //
//    o  PageUp,PageDown,arrow keys - navigate inside container         //
//    o  Return/Enter - equivalent to double click of the mouse button  //
//    o  Contrl-A - select/activate all items.                          //
//    o  Space - invert selection.                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TGCanvas.h"
#include "TGListView.h"
#include "TGScrollBar.h"
#include "TTimer.h"
#include "KeySymbols.h"
#include "TSystem.h"
#include "TGTextEditDialogs.h"
#include "TGMsgBox.h"
#include "TGResourcePool.h"
#include "TList.h"
#include "TClass.h"
#include "TGListView.h"
#include "TGMimeTypes.h"
#include "TKey.h"
#include "TKeyMapFile.h"
#include "TGDNDManager.h"
#include "Riostream.h"
#include "RConfigure.h"
#include <stdlib.h>


TGGC *TGContainer::fgLineGC = 0;

const Int_t kAutoScrollFudge = 10;
const Int_t kAcceleration[kAutoScrollFudge+1] = {1,1,1,2,3,4,6,7,8,16,32};
const Int_t kKeyboardTime = 700;

ClassImp(TGCanvas)
ClassImp(TGViewPort)
ClassImp(TGContainer)


//______________________________________________________________________________
class TGContainerKeyboardTimer : public TTimer {

private:
   TGContainer   *fContainer;
public:
   TGContainerKeyboardTimer(TGContainer *t) : TTimer(kKeyboardTime) { fContainer = t; }
   Bool_t Notify();
};

//______________________________________________________________________________
Bool_t TGContainerKeyboardTimer::Notify()
{
   // single shot timer

   fContainer->SearchPattern();
   Reset();
   if (gSystem) gSystem->RemoveTimer(this);
   return kFALSE;
}

//______________________________________________________________________________
class TGContainerScrollTimer : public TTimer {

private:
   TGContainer   *fContainer;
public:
   TGContainerScrollTimer(TGContainer *t) : TTimer(50) { fContainer = t; }
   Bool_t Notify();
};

//______________________________________________________________________________
Bool_t TGContainerScrollTimer::Notify()
{
   // on-timeout

   fContainer->OnAutoScroll();
   Reset();
   return kFALSE;
}


//______________________________________________________________________________
TGViewPort::TGViewPort(const TGWindow *p, UInt_t w, UInt_t h,
                       UInt_t options, ULong_t back) :
    TGCompositeFrame(p, w, h, options, back)
{
   // Create a viewport object.

   fContainer = 0;
   fX0 = fY0  = 0;

   AddInput(kStructureNotifyMask);
   SetWindowName();
   fEditDisabled = kEditDisable | kEditDisableGrab;
}

//______________________________________________________________________________
void TGViewPort::SetContainer(TGFrame *f)
{
   // Add container frame to the viewport. We must make sure that the added
   // container is at least a TGCompositeFrame (TGCanvas::AddFrame depends
   // on it).

   if (!f) {
      RemoveFrame(fContainer);
      fContainer = 0;
      return;
   }

   if (!fContainer) {
      fContainer = f;
      AddFrame(f, 0);
      fContainer->SetEditDisabled(fContainer->GetEditDisabled() | kEditDisableGrab);

      if (fContainer->InheritsFrom(TGContainer::Class())) {
         ((TGContainer*)fContainer)->fViewPort = this;
         if (fParent->InheritsFrom(TGCanvas::Class())) {
            ((TGContainer*)fContainer)->fCanvas = (TGCanvas*)fParent;
         }
      }
   }
}

//______________________________________________________________________________
void TGViewPort::SetHPos(Int_t xpos)
{
   // Moves content of container frame in horizontal direction.

   Int_t diff;

   if (!fContainer) return;

   if (!fContainer->InheritsFrom(TGContainer::Class())) {
      fContainer->Move(fX0 = xpos, fY0);
      return;
   } else {
      if (((TGContainer*)fContainer)->fMapSubwindows) {
         fContainer->Move(fX0 = xpos, fY0);
         return;
      }
   }

   if (-xpos < 0) return;
   else diff = xpos - fX0;

   if (!diff) return;

   fX0 = xpos;

#if defined(R__HAS_COCOA)
   //In the current version of cocoa back-end, it's very expensive
   //to read window's pixels, skip "optimization".
   ((TGContainer*)fContainer)->DrawRegion(0, 0, fWidth, fHeight);
#else
   UInt_t adiff = TMath::Abs(diff);

   if (adiff < fWidth) {
      if (diff < 0) {
         gVirtualX->CopyArea(fContainer->GetId(), fContainer->GetId(), GetWhiteGC()(),
                              adiff, 0, fWidth - adiff, fHeight, 0, 0);
         adiff += 20;   // draw larger region
         ((TGContainer*)fContainer)->DrawRegion(fWidth - adiff, 0, adiff, fHeight);
      } else {
         gVirtualX->CopyArea(fContainer->GetId(), fContainer->GetId(), GetWhiteGC()(),
                              0, 0, fWidth - adiff, fHeight, adiff, 0);
         adiff += 20;   // draw larger region
         ((TGContainer*)fContainer)->DrawRegion(0, 0, adiff, fHeight);
      }
   } else {
      ((TGContainer*)fContainer)->DrawRegion(0, 0, fWidth, fHeight);
   }
#endif
}

//______________________________________________________________________________
void TGViewPort::SetVPos(Int_t ypos)
{
   // Moves content of container frame in vertical direction.

   Int_t diff;

   if (!fContainer) return;

   // for backward comatibility
   if (!fContainer->InheritsFrom(TGContainer::Class())) {
      fContainer->Move(fX0, fY0 = ypos);
      return;
   } else {
      if (((TGContainer*)fContainer)->fMapSubwindows) {
         fContainer->Move(fX0, fY0 = ypos);
         return;
      }
   }

   if (-ypos < 0) return;
   else diff = ypos - fY0;

   if (!diff) return;

   fY0 = ypos;

#if defined(R__HAS_COCOA)
   //In the current version of cocoa back-end, it's very expensive
   //to read window's pixels, skip "optimization".
   ((TGContainer*)fContainer)->DrawRegion(0, 0, fWidth, fHeight);
#else
   UInt_t adiff = TMath::Abs(diff);

   if (adiff < fHeight) {
      if (diff < 0) {
         gVirtualX->CopyArea(fContainer->GetId(), fContainer->GetId(), GetWhiteGC()(),
                              0, adiff, fWidth, fHeight - adiff, 0, 0);
         adiff += 20;   // draw larger region
         ((TGContainer*)fContainer)->DrawRegion(0, fHeight - adiff, fWidth, adiff);
      } else {
         gVirtualX->CopyArea(fContainer->GetId(), fContainer->GetId(), GetWhiteGC()(),
                              0, 0, fWidth, fHeight - adiff, 0, adiff);
         adiff += 20;   // draw larger region
         ((TGContainer*)fContainer)->DrawRegion(0, 0, fWidth, adiff);
      }
   } else {
      ((TGContainer*)fContainer)->DrawRegion(0, 0, fWidth, fHeight);
   }
#endif
}

//______________________________________________________________________________
void TGViewPort::SetPos(Int_t xpos, Int_t ypos)
{
   // Goto new position.

   if (!fContainer) return;

   SetHPos(fX0 = xpos);
   SetVPos(fY0 = ypos);
}

//______________________________________________________________________________
Bool_t TGViewPort::HandleConfigureNotify(Event_t *event)
{
   // Handle resize events.

   if (!fContainer->InheritsFrom(TGContainer::Class())) {
      TGFrame::HandleConfigureNotify(event);
      return kTRUE;
   }

   TGContainer *cont = (TGContainer*)fContainer;

   // protection 
   if ((event->fWidth > 32768) || (event->fHeight  > 32768)) {
      return kFALSE;
   }

   cont->DrawRegion(event->fX, event->fY, event->fWidth, event->fHeight);

   return kTRUE;
}

//______________________________________________________________________________
TGContainer::TGContainer(const TGWindow *p, UInt_t w, UInt_t h,
                             UInt_t options, ULong_t back) :
   TGCompositeFrame(p, w, h, options, back)
{
   // Create a canvas container. This is the (large) frame that contains
   // all the list items. It will be shown through a TGViewPort (which is
   // created by the TGCanvas).

   fViewPort = 0;
   fBdown = kFALSE;
   fMsgWindow  = p;
   fDragging   = kFALSE;
   fTotal = fSelected = 0;
   fMapSubwindows = kFALSE;
   fOnMouseOver = kFALSE;
   fLastActiveEl = 0;
   fLastDir = kTRUE;
   fLastCase = kTRUE;
   fLastSubstring = kFALSE;
   fLastName = "";
   fKeyTimer = new TGContainerKeyboardTimer(this);
   fScrollTimer = new TGContainerScrollTimer(this);
   fKeyTimerActive = kFALSE;
   fScrolling = kFALSE;
   fCanvas = 0;
   fExposedRegion.Empty();

   gVirtualX->GrabButton(fId, kAnyButton, kAnyModifier,
                         kButtonPressMask | kButtonReleaseMask |
                         kPointerMotionMask, kNone, kNone);

   AddInput(kKeyPressMask | kPointerMotionMask);
   SetWindowName();

   SetWindowAttributes_t wattr;
   wattr.fMask = kWAWinGravity | kWABitGravity;
   wattr.fBitGravity = 1; // NorthWestGravity
   wattr.fWinGravity = 1;
   gVirtualX->ChangeWindowAttributes(fId, &wattr);

   fEditDisabled = kEditDisableGrab | kEditDisableBtnEnable;
}

//______________________________________________________________________________
TGContainer::TGContainer(TGCanvas *p, UInt_t options, ULong_t back) :
   TGCompositeFrame(p->GetViewPort(), p->GetWidth(), p->GetHeight(), options, back)
{
   // Create a canvas container. This is the (large) frame that contains
   // all the list items. It will be shown through a TGViewPort (which is
   // created by the TGCanvas).

   fViewPort = 0;
   fBdown = kFALSE;
   fMsgWindow  = p->GetViewPort();
   fCanvas = p;
   fCanvas->GetViewPort()->SetContainer(this);
   p->GetViewPort()->SetBackgroundColor(back);

   fDragging = kFALSE;
   fTotal = fSelected = 0;
   fMapSubwindows = kFALSE;
   fOnMouseOver = kFALSE;
   fLastActiveEl = 0;
   fLastDir = kTRUE;
   fLastCase = kTRUE;
   fLastSubstring = kFALSE;
   fLastName = "";
   fKeyTimer = new TGContainerKeyboardTimer(this);
   fScrollTimer = new TGContainerScrollTimer(this);
   fKeyTimerActive = kFALSE;
   fScrolling = kFALSE;
   fExposedRegion.Empty();

   gVirtualX->GrabButton(fId, kAnyButton, kAnyModifier,
                         kButtonPressMask | kButtonReleaseMask |
                         kPointerMotionMask, kNone, kNone);

   AddInput(kKeyPressMask | kPointerMotionMask);
   SetWindowName();

   SetWindowAttributes_t wattr;
   wattr.fMask = kWAWinGravity | kWABitGravity;
   wattr.fBitGravity = 1; // NorthWestGravity
   wattr.fWinGravity = 1;
   gVirtualX->ChangeWindowAttributes(fId, &wattr);

   fEditDisabled = kEditDisableGrab | kEditDisableBtnEnable;
}

//______________________________________________________________________________
TGContainer::~TGContainer()
{
   // Delete canvas container.

   if (TGSearchDialog::SearchDialog()) {
      TQObject::Disconnect(TGSearchDialog::SearchDialog(), 0, this);
   }

   delete fScrollTimer;
   fScrollTimer = 0;

   delete fKeyTimer;
   fKeyTimer = 0;
}

//______________________________________________________________________________
void TGContainer::Layout()
{
   // Layout container entries.

   TGCompositeFrame::Layout();
   TGLayoutManager *lm = GetLayoutManager();

   // clear content if positions of subframes changed after layout
   if (lm && lm->IsModified()) ClearViewPort();
}

//______________________________________________________________________________
void TGContainer::CurrentChanged(Int_t x, Int_t y)
{
   // Emit signal when current position changed.

   Long_t args[2];

   args[0] = x;
   args[1] = y;

   Emit("CurrentChanged(Int_t,Int_t)",args);
}

//______________________________________________________________________________
void TGContainer::CurrentChanged(TGFrame* f)
{
   // Emit signal when current selected frame changed.

   Emit("CurrentChanged(TGFrame*)", (Long_t)f);
}

//______________________________________________________________________________
void TGContainer::KeyPressed(TGFrame *frame, UInt_t keysym, UInt_t mask)
{
   // Signal emitted when keyboard key pressed
   //
   // frame - activated frame
   // keysym - defined in "KeySymbols.h"
   // mask - modifier key mask, defined in "GuiTypes.h"
   //
   // const Mask_t kKeyShiftMask   = BIT(0);
   // const Mask_t kKeyLockMask    = BIT(1);
   // const Mask_t kKeyControlMask = BIT(2);
   // const Mask_t kKeyMod1Mask    = BIT(3);   // typically the Alt key
   // const Mask_t kButton1Mask    = BIT(8);
   // const Mask_t kButton2Mask    = BIT(9);
   // const Mask_t kButton3Mask    = BIT(10);
   // const Mask_t kButton4Mask    = BIT(11);
   // const Mask_t kButton5Mask    = BIT(12);
   // const Mask_t kAnyModifier    = BIT(15);

   Long_t args[3];
   args[0] = (Long_t)frame;
   args[1] = (Long_t)keysym;
   args[2] = (Long_t)mask;
   Emit("KeyPressed(TGFrame*,UInt_t,UInt_t)", args);
   SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_KEY), keysym, mask);
}

//______________________________________________________________________________
void TGContainer::ReturnPressed(TGFrame* f)
{
   // Signal emitted when Return/Enter key pressed.
   // It's equivalent to "double click" of mouse button.

   Emit("ReturnPressed(TGFrame*)", (Long_t)f);
}

//______________________________________________________________________________
void TGContainer::SpacePressed(TGFrame* f)
{
   // Signal emitted when space key pressed.
   // Pressing space key inverts selection.

   Emit("SpacePressed(TGFrame*)", (Long_t)f);
}

//______________________________________________________________________________
void TGContainer::OnMouseOver(TGFrame* f)
{
   // Signal emitted when pointer is over entry.

   if (!fOnMouseOver) Emit("OnMouseOver(TGFrame*)", (Long_t)f);
   fOnMouseOver = kTRUE;
}

//______________________________________________________________________________
void TGContainer::Clicked(TGFrame *entry, Int_t btn)
{
   // Emit Clicked() signal.

   Long_t args[2];

   args[0] = (Long_t)entry;
   args[1] = btn;

   Emit("Clicked(TGFrame*,Int_t)", args);
}

//______________________________________________________________________________
void TGContainer::Clicked(TGFrame *entry, Int_t btn, Int_t x, Int_t y)
{
   // Emit Clicked() signal.

   Long_t args[4];

   args[0] = (Long_t)entry;
   args[1] = btn;
   args[2] = x;
   args[3] = y;

   Emit("Clicked(TGFrame*,Int_t,Int_t,Int_t)", args);
}

//______________________________________________________________________________
void TGContainer::DoubleClicked(TGFrame *entry, Int_t btn)
{
   // Emit DoubleClicked() signal.

   Long_t args[2];

   args[0] = (Long_t)entry;
   args[1] = btn;

   Emit("DoubleClicked(TGFrame*,Int_t)", args);
}

//______________________________________________________________________________
void TGContainer::DoubleClicked(TGFrame *entry, Int_t btn, Int_t x, Int_t y)
{
   // Emit DoubleClicked() signal.

   Long_t args[4];

   args[0] = (Long_t)entry;
   args[1] = btn;
   args[2] = x;
   args[3] = y;

   Emit("DoubleClicked(TGFrame*,Int_t,Int_t,Int_t)", args);
}

//______________________________________________________________________________
void TGContainer::SelectAll()
{
   // Select all items in the container.
   // SelectAll() signal emitted.

   TIter next(fList);
   TGFrameElement *el;
   TGFrame *fr;
   TGPosition pos = GetPagePosition();

   while ((el = (TGFrameElement *) next())) {
      fr = el->fFrame;
      if (!fr->IsActive()) {
         ActivateItem(el);
      }
   }
   fSelected = fTotal;
   SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED),
                  fTotal, fSelected);

   Emit("SelectAll()");
}

//______________________________________________________________________________
void TGContainer::UnSelectAll()
{
   // Unselect all items in the container.

   TIter next(fList);
   TGFrameElement *el;
   TGPosition pos = GetPagePosition();
   TGFrame *fr;

   while ((el = (TGFrameElement *) next())) {
      fr = el->fFrame;
      if (fr->IsActive()) {
         DeActivateItem(el);
      }
   }
   fLastActiveEl = 0;
   fSelected = 0;

   SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED),
                  fTotal, fSelected);

   Emit("UnSelectAll()");
}

//______________________________________________________________________________
void TGContainer::InvertSelection()
{
   // Invert the selection, all selected items become unselected and
   // vice versa.

   int selected = 0;

   TIter next(fList);
   TGFrameElement *el;

   while ((el = (TGFrameElement *) next())) {
      if (!el->fFrame->IsActive()) {
         ActivateItem(el);
         ++selected;
      } else {
         DeActivateItem(el);
      }
   }
   ClearViewPort();  // full redraw
   fSelected = selected;

   SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED),
                  fTotal, fSelected);

   Emit("InvertSelection()");
}

//______________________________________________________________________________
void TGContainer::RemoveAll()
{
   // Remove all items from the container.

   TGFrameElement *el;
   TIter next(fList);

   while ((el = (TGFrameElement *) next())) {
      el->fFrame->DestroyWindow();
      delete el->fFrame;
      fList->Remove(el);
      delete el;
   }
   fLastActiveEl = 0;
   fSelected = fTotal = 0;
   ClearViewPort();  // full redraw
}

//______________________________________________________________________________
void TGContainer::RemoveItem(TGFrame *item)
{
   // Remove item from container.

   TGFrameElement *el;
   TIter next(fList);
   while ((el = (TGFrameElement *) next())) {
      if (item == el->fFrame) {
         if (fLastActiveEl && item == fLastActiveEl->fFrame) fLastActiveEl = 0;
         item->DestroyWindow();
         delete item;
         fList->Remove(el);
         delete el;
         break;
      }
   }
   ClearViewPort();  // fill redraw
}

//______________________________________________________________________________
const TGFrame *TGContainer::GetNextSelected(void **current)
{
   // Return the next selected item. If the "current" pointer is 0, the first
   // selected item will be returned.

   TGFrame *f;
   TObjLink *lnk = (TObjLink *) *current;

   lnk = (lnk == 0) ? fList->FirstLink() : lnk->Next();
   while (lnk) {
      f = (TGFrame *) ((TGFrameElement *) lnk->GetObject())->fFrame;
      if (f->IsActive()) {
         *current = (void *) lnk;
         return f;
      }
      lnk = lnk->Next();
   }
   return 0;
}

//______________________________________________________________________________
void TGContainer::ActivateItem(TGFrameElement *el)
{
   // Activate item.

   TGFrame *fr = el->fFrame;
   fr->Activate(kTRUE);

   if (fLastActiveEl != el) {
      fLastActiveEl = el;
      CurrentChanged(fLastActiveEl->fFrame->GetX(), fLastActiveEl->fFrame->GetY());
      CurrentChanged(fLastActiveEl->fFrame);
      fSelected++;
   }

   if (!fSelected) fSelected = 1;

   SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED), fTotal, fSelected);

   TGPosition pos = GetPagePosition();
   DrawRegion(fr->GetX() - pos.fX, fr->GetY() - pos.fY, fr->GetWidth(), fr->GetHeight());
}

//______________________________________________________________________________
void TGContainer::DeActivateItem(TGFrameElement *el)
{
   // DeActivate item.

   TGFrame *fr = el->fFrame;
   fr->Activate(kFALSE);
   SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED), fTotal, fSelected);

   TGPosition pos = GetPagePosition();
   DrawRegion(fr->GetX() - pos.fX, fr->GetY() - pos.fY, fr->GetWidth(), fr->GetHeight());
}

//______________________________________________________________________________
TGPosition TGContainer::GetPagePosition() const
{
   // Returns page position.

   TGPosition ret;
   if (!fViewPort) return ret;

   ret.fX = -fViewPort->GetHPos();
   ret.fY = -fViewPort->GetVPos();

   return ret;
}

//______________________________________________________________________________
TGDimension TGContainer::GetPageDimension() const
{
   // Returns page dimension.

   TGDimension ret;
   if (!fViewPort) return ret;

   ret.fWidth = fViewPort->GetWidth();
   ret.fHeight = fViewPort->GetHeight();
   return ret;
}

//______________________________________________________________________________
void TGContainer::SetPagePosition(const TGPosition& pos)
{
   // Set page position.

   if (!fViewPort) return;
   fViewPort->SetPos(pos.fX, pos.fY);
}

//______________________________________________________________________________
void TGContainer::SetPagePosition(Int_t x, Int_t y)
{
   // Set page position.

   if (!fViewPort) return;
   fViewPort->SetPos(x, y);
}

//______________________________________________________________________________
void TGContainer::SetPageDimension(const TGDimension& dim)
{
   // Set page dimension.

   if (!fViewPort) return;
   fViewPort->Resize(dim);
}

//______________________________________________________________________________
void TGContainer::SetPageDimension(UInt_t w, UInt_t h)
{
   // Set page dimension.

   if (!fViewPort) return;
   fViewPort->Resize(w, h);
}

//______________________________________________________________________________
void TGContainer::DoRedraw()
{
   // Redraw content of container in the viewport region.
#ifdef R__HAS_COCOA
   DrawRegion(0, 0, GetWidth(), GetHeight());
#else
   if (!fExposedRegion.IsEmpty()) {
      DrawRegion(fExposedRegion.fX, fExposedRegion.fY, 
                 fExposedRegion.fW, fExposedRegion.fH);
      
      fExposedRegion.Empty();
   }
#endif
}

//______________________________________________________________________________
void TGContainer::DrawRegion(Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   // Draw a region of container in viewport.
   // x, y, w, h are position and dimension of area to be 
   // redrawn in viewport coordinates.

   static GContext_t gcBg = 0;
   Pixmap_t pixmap = 0;

   if (!fViewPort) return;
   // sanity checks
   if ((x > (Int_t)fViewPort->GetWidth()) || (y > (Int_t)fViewPort->GetHeight())) {
      return;
   }
   x = x < 0 ? 0 : x;
   y = y < 0 ? 0 : y;

   w = x + w > fViewPort->GetWidth() ? fViewPort->GetWidth() - x : w;
   h = y + h > fViewPort->GetHeight() ? fViewPort->GetHeight() - y :  h;

   if (((Int_t)w < 1) || ((Int_t)h < 1)) {
      return;
   }

   if (!fMapSubwindows) {
      pixmap = gVirtualX->CreatePixmap(fId, w, h);

      if (!gcBg) {
         GCValues_t gcValues;
         gcValues.fForeground = fBackground;
         gcValues.fBackground = fBackground;
         gcValues.fGraphicsExposures = kTRUE;
         gcValues.fMask = kGCForeground | kGCBackground | kGCGraphicsExposures;
         gcBg = gVirtualX->CreateGC(fId, &gcValues);
      }

      gVirtualX->SetForeground(gcBg, fBackground);
      gVirtualX->FillRectangle(pixmap, gcBg, 0, 0, w, h);
   }

   TGPosition pos = GetPagePosition();

   // translate coordinates in viewport into coordinates in container 
   Int_t xx = pos.fX + x;
   Int_t yy = pos.fY + y;

   TIter next(fList);
   TGFrameElement *el;

   while ((el = (TGFrameElement *) next())) {
      if ((Int_t(el->fFrame->GetY()) > yy - (Int_t)el->fFrame->GetHeight()) &&
          (Int_t(el->fFrame->GetX()) > xx - (Int_t)el->fFrame->GetWidth()) &&
          (Int_t(el->fFrame->GetY()) < yy + Int_t(h + el->fFrame->GetHeight())) &&
          (Int_t(el->fFrame->GetX()) < xx + Int_t(w + el->fFrame->GetWidth()))) {

         // draw either in container window or in double-buffer
         if (!fMapSubwindows) {
            Int_t fx = el->fFrame->GetX() - xx;
            Int_t fy = el->fFrame->GetY() - yy;
            el->fFrame->DrawCopy(pixmap, fx, fy);
         } else {
            fClient->NeedRedraw(el->fFrame);
         }
      }
   }

   if (fMapSubwindows) return;

   gVirtualX->CopyArea(pixmap, fId, gcBg, 0, 0, w, h, x, y);
   gVirtualX->DeletePixmap(pixmap);
   gVirtualX->Update(kFALSE);
}

//______________________________________________________________________________
void TGContainer::ClearViewPort()
{
   // Clear view port and redraw full content

   if (!fViewPort) return;
   fExposedRegion.fW = fViewPort->GetWidth();
   fExposedRegion.fH = fViewPort->GetHeight();
   fExposedRegion.fX = fExposedRegion.fY = 0;
   fClient->NeedRedraw(this);
}

//______________________________________________________________________________
Bool_t TGContainer::HandleExpose(Event_t *event)
{
   // Handle expose events. Do not use double buffer.

   if (fMapSubwindows) return TGCompositeFrame::HandleExpose(event);

   if (event->fWindow == GetId()) {
      TGPosition pos(event->fX, event->fY);
      TGDimension dim(event->fWidth, event->fHeight);
      TGRectangle rect(pos, dim);

      if (fExposedRegion.IsEmpty()) {
         fExposedRegion = rect;
      } else {
         fExposedRegion.Merge(rect);
      }

      fClient->NeedRedraw(this);
   } else {
      TGCompositeFrame::HandleExpose(event);
   }

   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGContainer::HandleButton(Event_t *event)
{
   // Handle mouse button event in container.

   Int_t total, selected, page = 0;

   TGPosition pos = GetPagePosition();
   TGDimension dim = GetPageDimension();
   Int_t newpos;
   page = dim.fHeight/4;

   if (event->fCode == kButton4) {
      //scroll up
      newpos = pos.fY - page;
      if (newpos < 0) newpos = 0;
      fCanvas->SetVsbPosition(newpos);
      return kTRUE;
   }
   if (event->fCode == kButton5) {
      // scroll down
      newpos = fCanvas->GetVsbPosition() + page;
      fCanvas->SetVsbPosition(newpos);
      return kTRUE;
   }

   Int_t xx = pos.fX + event->fX; // translate coordinates
   Int_t yy = pos.fY + event->fY;

   if (event->fType == kButtonPress) {
      gVirtualX->SetInputFocus(fId);

      fXp = pos.fX + event->fX;
      fYp = pos.fY + event->fY;

      fXDND = event->fX;
      fYDND = event->fY;
      fBdown = kTRUE;

      UnSelectAll();
      total = selected = 0;

      TGFrameElement *el;
      TIter next(fList);
      Bool_t select_frame = kFALSE;

      while ((el = (TGFrameElement *) next())) {
         select_frame = kFALSE;

         if (!fMapSubwindows) {
            if ((Int_t(el->fFrame->GetY()) + (Int_t)el->fFrame->GetHeight() > yy ) &&
               (Int_t(el->fFrame->GetX()) + (Int_t)el->fFrame->GetWidth() > xx ) &&
               (Int_t(el->fFrame->GetY()) < yy) &&
               (Int_t(el->fFrame->GetX()) < xx))  {
               select_frame = kTRUE;
            }
         } else {
            if (el->fFrame->GetId() == (Window_t)event->fUser[0]) {
               select_frame = kTRUE;
            }
         }

         if (select_frame) {
            selected++;
            ActivateItem(el);
            Clicked(el->fFrame, event->fCode);
            Clicked(el->fFrame, event->fCode, event->fXRoot, event->fYRoot);
         }
         total++;
      }

      if (fTotal != total || fSelected != selected) {
         fTotal = total;
         fSelected = selected;
         SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED),
                     fTotal, fSelected);
      }

      if ( selected == 0 ) {
         fDragging = kTRUE;
         fX0 = fXf = fXp;
         fY0 = fYf = fYp;
         gVirtualX->DrawRectangle(fId, GetLineGC()(), fX0-pos.fX, fY0-pos.fY, 
                                  fXf-fX0, fYf-fY0);
      }
   }

   if (event->fType == kButtonRelease) {
      gVirtualX->SetInputFocus(fId);

      fBdown = kFALSE;
      if (fDragging) {
         fDragging = kFALSE;
         fScrolling = kFALSE;

         if (gSystem) gSystem->RemoveTimer(fScrollTimer);
         gVirtualX->DrawRectangle(fId, GetLineGC()(), fX0-pos.fX, fY0-pos.fY,
                                  fXf-fX0, fYf-fY0);
         ClearViewPort();

      } else {
         SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_ITEMCLICK),
                     event->fCode, (event->fYRoot << 16) | event->fXRoot);
      }
   }
   DoRedraw();
   return kTRUE;
}

//______________________________________________________________________________
const TGPicture *TGContainer::GetObjPicture(TGFrame *f)
{
   // Retrieve icons associated with class "name". Association is made
   // via the user's ~/.root.mimes file or via $ROOTSYS/etc/root.mimes.

   TObject *obj = 0;
   TClass *cl;
   const TGPicture *pic=0;
   const char *iconname = 0;

   if (f->InheritsFrom("TGLVEntry")) {
      obj = (TObject *)((TGLVEntry *)f)->GetUserData();
      if (obj) {
         if (obj->IsA() == TKey::Class()) {
            cl = TClass::GetClass(((TKey *)obj)->GetClassName());
         } else if (obj->IsA() == TKeyMapFile::Class()) {
            cl = TClass::GetClass(((TKeyMapFile *)obj)->GetTitle());
         } else {
            cl = obj->IsA();
         }
         const char *name = obj->GetIconName();
         if (((name == 0) || (strlen(name) == 0)) && (cl != 0))
            name = cl->GetName();
         iconname = ((name != 0) && (strlen(name) > 0)) ? name : obj->GetName();

         if (obj->IsA()->InheritsFrom("TGeoVolume")) {
            iconname = obj->GetIconName() ? obj->GetIconName() : obj->IsA()->GetName();
         }
         pic = fClient->GetMimeTypeList()->GetIcon(iconname, kFALSE);
      }
   }
   if (pic == 0) {
      if (obj && obj->IsFolder()) {
         pic = fClient->GetPicture("folder_s.xpm");
      } else {
         pic = fClient->GetPicture("doc_s.xpm");
      }
   }
   return pic;
}

//______________________________________________________________________________
void TGContainer::SetDragPixmap(const TGPicture *p)
{
   // Set drag window pixmaps and hotpoint.
   
   Pixmap_t pic, mask;
   TGPicture *selpic = new TGSelectedPicture(gClient, p);
   pic  = selpic->GetPicture();
   mask = selpic->GetMask();

   if (gDNDManager) {
      gDNDManager->SetDragPixmap(pic, mask, p->GetWidth()/2, 2+p->GetHeight()/2);
   } else {
      gVirtualX->DeletePixmap(pic);
      gVirtualX->DeletePixmap(mask);
   }
}

//______________________________________________________________________________
Bool_t TGContainer::HandleDoubleClick(Event_t *event)
{
   // Handle double click mouse event.

   TGFrameElement *el;
   TIter next(fList);

   TGPosition pos = GetPagePosition();

   Int_t xx = pos.fX + event->fX; // translate coordinates
   Int_t yy = pos.fY + event->fY;

   Bool_t select_frame = kFALSE;

   while ((el = (TGFrameElement *) next())) {
      select_frame = kFALSE;

      if (!fMapSubwindows) {
         if ((Int_t(el->fFrame->GetY()) + (Int_t)el->fFrame->GetHeight() > yy) &&
            (Int_t(el->fFrame->GetX()) + (Int_t)el->fFrame->GetWidth() > xx) &&
            (Int_t(el->fFrame->GetY()) < yy) &&
            (Int_t(el->fFrame->GetX()) < xx))  {
            select_frame = kTRUE;
         }
      } else {
         if (el->fFrame->GetId() == (Window_t)event->fUser[0]) {
            select_frame = kTRUE;
         }
      }

      if (select_frame) {
         SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_ITEMDBLCLICK),
                     event->fCode, (event->fYRoot << 16) | event->fXRoot);

         DoubleClicked(el->fFrame, event->fCode);
         DoubleClicked(el->fFrame, event->fCode, event->fXRoot, event->fYRoot);
         return kTRUE;
      }
   }
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGContainer::HandleMotion(Event_t *event)
{
   // Handle mouse motion events.

   int xf0, yf0, xff, yff, total, selected;

   TGPosition pos = GetPagePosition();
   TGDimension dim = GetPageDimension();
   Int_t x = pos.fX + event->fX;
   Int_t y = pos.fY + event->fY;
   TGFrameElement *el = 0;
   TGFrame *f = 0;
   fOnMouseOver = kFALSE;

   Bool_t wasScrolling = fScrolling;

   if (gDNDManager->IsDragging()) {
      gDNDManager->Drag(event->fXRoot, event->fYRoot,
                        TGDNDManager::GetDNDActionCopy(), event->fTime);
   } 
   else if (fDragging) {

      gVirtualX->DrawRectangle(fId, GetLineGC()(), fX0-pos.fX, fY0-pos.fY,
                               fXf-fX0, fYf-fY0);
      fX0 =  TMath::Min(fXp,x);
      fY0 =  TMath::Min(fYp,y);
      fXf =  TMath::Max(fXp,x);
      fYf =  TMath::Max(fYp,y);

      total = selected = 0;

      if (event->fX > Int_t(dim.fWidth) - kAutoScrollFudge) {
         //fCanvas->SetHsbPosition(x - dim.fWidth);
         fScrolling = kTRUE;
      } else if (event->fX < kAutoScrollFudge) {
         //fCanvas->SetHsbPosition(x);
         fScrolling = kTRUE;
      } else if (event->fY > Int_t(dim.fHeight) - kAutoScrollFudge) {
         //fCanvas->SetVsbPosition(y - dim.fHeight);
         fScrolling = kTRUE;
      } else if (event->fY < kAutoScrollFudge) {
         //fCanvas->SetVsbPosition(y);
         fScrolling = kTRUE;
      }
      else {
         fScrolling = kFALSE;
      }

      TIter next(fList);

      while ((el = (TGFrameElement *) next())) {
         f = el->fFrame;
         ++total;
         xf0 = f->GetX() + (f->GetWidth() >> 3);
         yf0 = f->GetY() + (f->GetHeight() >> 3);
         xff = xf0 + f->GetWidth() - (f->GetWidth() >> 2);
         yff = yf0 + f->GetHeight() - (f->GetHeight() >> 2);

         if (((xf0 > fX0 && xf0 < fXf) ||
              (xff > fX0 && xff < fXf)) &&
             ((yf0 > fY0 && yf0 < fYf) ||
              (yff > fY0 && yff < fYf))) {
            if (!el->fFrame->IsActive())
               ActivateItem(el);
            gVirtualX->SetCursor(fId, gVirtualX->CreateCursor(kHand));
            OnMouseOver(f);
            ++selected;
         } else {
            if (el->fFrame->IsActive())
               DeActivateItem(el);
         }
      }

      if (fTotal != total || fSelected != selected) {
         fTotal = total;
         fSelected = selected;
         SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED),
                     fTotal, fSelected);
      }
      gVirtualX->DrawRectangle(fId, GetLineGC()(), fX0-pos.fX, fY0-pos.fY,
                               fXf-fX0, fYf-fY0);
   } 
   else {
      TGFrame *over_frame = 0;

      TIter next(fList);

      while ((el = (TGFrameElement *) next())) {
         if (!fMapSubwindows) {
            if ((Int_t(el->fFrame->GetY()) + (Int_t)el->fFrame->GetHeight() > y) &&
               (Int_t(el->fFrame->GetX()) + (Int_t)el->fFrame->GetWidth() > x) &&
               (Int_t(el->fFrame->GetY()) < y) &&
               (Int_t(el->fFrame->GetX()) < x))  {
               over_frame = el->fFrame;
               break;
            }
         } else {
            if (el->fFrame->GetId() == (Window_t)event->fUser[0]) {
               over_frame = el->fFrame;
               break;
            }
         }
      }
      if (over_frame) {
         if (!gDNDManager->IsDragging()) {
            if (fBdown && ((abs(event->fX - fXDND) > 2) || (abs(event->fY - fYDND) > 2))) {
               if (gDNDManager && over_frame->IsDNDSource()) {
                  const TGPicture *drag_pic = GetObjPicture(over_frame);
                  if (drag_pic) SetDragPixmap(drag_pic);
                  gDNDManager->StartDrag(over_frame, event->fXRoot, event->fYRoot);
               }
            }
         }
         if (gDNDManager->IsDragging()) {
            gDNDManager->Drag(event->fXRoot, event->fYRoot,
                              TGDNDManager::GetDNDActionCopy(), event->fTime);
         } else {
            OnMouseOver(over_frame);
            gVirtualX->SetCursor(fId, gVirtualX->CreateCursor(kHand));
         }
      } else {
         gVirtualX->SetCursor(fId, gVirtualX->CreateCursor(kPointer));
      }
   }

   if (!wasScrolling && fScrolling) {
      if (gSystem) {
         fScrollTimer->Reset();
         gSystem->AddTimer(fScrollTimer);
      }
   }

   return kTRUE;
}

//______________________________________________________________________________
Bool_t TGContainer::HandleKey(Event_t *event)
{
   // The key press event handler converts a key press to some line editor
   // action.

   char   input[10];
   Int_t  n;
   UInt_t keysym;

   if (event->fType == kGKeyPress) {
      gVirtualX->LookupString(event, input, sizeof(input), keysym);
      n = strlen(input);

      KeyPressed(fLastActiveEl?fLastActiveEl->fFrame:0, keysym, event->fState);

      switch ((EKeySym)keysym) {
         case kKey_Enter:
         case kKey_Return:
            // treat 'Enter' and 'Return' as a double click
            SendMessage(GetMessageWindow(), MK_MSG(kC_CONTAINER, kCT_ITEMDBLCLICK),
                              kButton1, (event->fYRoot << 16) | event->fXRoot);
            if (fLastActiveEl) ReturnPressed(fLastActiveEl->fFrame);
            break;
         case kKey_Shift:
         case kKey_Control:
         case kKey_Meta:
         case kKey_Alt:
         case kKey_CapsLock:
         case kKey_NumLock:
         case kKey_ScrollLock:
            return kTRUE;
         case kKey_Space:
            if (fLastActiveEl) {
               fLastActiveEl->fFrame->Activate(!fLastActiveEl->fFrame->IsActive());
               SpacePressed(fLastActiveEl->fFrame);
            }
            break;
         default:
         break;
      }

      if (event->fState & kKeyControlMask) {   // Cntrl key modifier pressed
         switch((EKeySym)keysym & ~0x20) {   // treat upper and lower the same
            case kKey_A:
               SelectAll();
               break;
            case kKey_B:
               LineLeft();
               break;
            case kKey_C:
               return kTRUE;
            case kKey_D:
               break;
            case kKey_E:
               End();
               break;
            case kKey_F:
               Search();
               break;
            case kKey_G:
               RepeatSearch();
               break;
            case kKey_H:
               LineLeft();
               break;
            case kKey_K:
               End();
               break;
            case kKey_U:
               Home();
               break;
            case kKey_V:
            case kKey_Y:
               return kTRUE;
            case kKey_X:
               return kTRUE;
            default:
               return kTRUE;
         }
      }
      if (n && keysym >= 32 && keysym < 127 &&     // printable keys
         !(event->fState & kKeyControlMask) &&
          (EKeySym)keysym != kKey_Delete &&
          (EKeySym)keysym != kKey_Backspace) {

         if (fKeyTimerActive) {
            fKeyInput += input;
         } else {
            fKeyInput = input;
            fKeyTimerActive = kTRUE;
            fKeyTimer->Reset();
            if (gSystem) gSystem->AddTimer(fKeyTimer);
         }
      } else {

         switch ((EKeySym)keysym) {
            case kKey_F3:
               RepeatSearch();
               break;
            case kKey_F5:
               Layout();
               break;
            case kKey_F7:
               Search();
               break;
            case kKey_Left:
               LineLeft(event->fState & kKeyShiftMask);
               break;
            case kKey_Right:
               LineRight(event->fState & kKeyShiftMask);
               break;
            case kKey_Up:
               LineUp(event->fState & kKeyShiftMask);
               break;
            case kKey_Down:
               LineDown(event->fState & kKeyShiftMask);
               break;
            case kKey_PageUp:
               PageUp(event->fState & kKeyShiftMask);
               break;
            case kKey_PageDown:
               PageDown(event->fState & kKeyShiftMask);
               break;
            case kKey_Home:
               Home(event->fState & kKeyShiftMask);
               break;
            case kKey_End:
               End(event->fState & kKeyShiftMask);
               break;
            default:
               break;
         }
      }
   }
   DoRedraw();
   return kTRUE;
}

//______________________________________________________________________________
TGFrame *TGContainer::FindFrameByName(const char *name)
{
   // Find frame by name.

   if (!IsMapped()) return 0;

   Bool_t direction = kTRUE;
   Bool_t caseSensitive = kFALSE;
   Bool_t subString = kFALSE;

   if (gTQSender && (gTQSender == TGSearchDialog::SearchDialog())) {
      caseSensitive = TGSearchDialog::SearchDialog()->GetType()->fCaseSensitive;
      direction = TGSearchDialog::SearchDialog()->GetType()->fDirection;
   }
   TString sname(name);
   if (sname.Contains("*")) {
      subString = kTRUE;
      sname.ReplaceAll("*", "");
   }

   TGFrameElement *fe = (TGFrameElement*)FindItem(sname.Data(), direction, 
                                                  caseSensitive, subString);
   if (!fe) {  // find again
      if (fLastActiveEl) DeActivateItem(fLastActiveEl);
      fLastActiveEl = 0;
      fe = (TGFrameElement*)FindItem(fLastName, fLastDir, fLastCase, fLastSubstring);

      if (!fe) {
         if (gTQSender && (gTQSender == TGSearchDialog::SearchDialog())) {
            TString msg = "Couldn't find \"" + fLastName + '\"';
            gVirtualX->Bell(20);
            new TGMsgBox(fClient->GetDefaultRoot(), fCanvas, "Container", msg.Data(),
                          kMBIconExclamation, kMBOk, 0);
         }
         return 0;
      } else {
         if (fLastActiveEl) DeActivateItem(fLastActiveEl);
         ActivateItem(fe);
         AdjustPosition();
         return fe->fFrame;
      }
   } else {
      if (fLastActiveEl) DeActivateItem(fLastActiveEl);
      ActivateItem(fe);
      AdjustPosition();
      return fe->fFrame;
   }
   return 0;
}

//______________________________________________________________________________
void TGContainer::Search(Bool_t close)
{
   // Invokes search dialog. Looks for item with the entered name.

   static TGSearchType *srch = 0;
   Int_t ret = 0;

   if (!srch) srch = new TGSearchType;
   srch->fClose = close;
   srch->fBuffer = 0;

   if (!close) {
      if (!TGSearchDialog::SearchDialog()) {
         TGSearchDialog::SearchDialog() = new TGSearchDialog(fClient->GetDefaultRoot(), 
                                                             fCanvas, 400, 150, srch, &ret);
      }
      TGSearchDialog::SearchDialog()->Connect("TextEntered(char *)", "TGContainer", this,
                                              "FindFrameByName(char *)");
      TGSearchDialog::SearchDialog()->MapRaised();
   } else {
      new TGSearchDialog(fClient->GetDefaultRoot(), fCanvas, 400, 150, srch, &ret);
      if (ret) {
         FindFrameByName(srch->fBuffer);
      }
   }
}

//______________________________________________________________________________
void TGContainer::OnAutoScroll()
{
   // Autoscroll while close to & beyond  The Wall

   TGFrameElement *el = 0;
   TGFrame *f = 0;
   int xf0, yf0, xff, yff, total, selected;

   TGDimension dim = GetPageDimension();
   TGPosition pos = GetPagePosition();

   Window_t  dum1, dum2;
   Event_t   ev;
   ev.fType    = kButtonPress;
   Int_t x,y;

   // Autoscroll while close to the wall
   Int_t dx = 0;
   Int_t dy = 0;

   // Where's the cursor?
   gVirtualX->QueryPointer(fId,dum1,dum2,ev.fXRoot,ev.fYRoot,x,y,ev.fState);

   // Figure scroll amount x
   if (x < kAutoScrollFudge)
      dx = kAutoScrollFudge - x;
   else if ((Int_t)dim.fWidth-kAutoScrollFudge <= x)
      dx = dim.fWidth - kAutoScrollFudge - x;

   // Figure scroll amount y
   if (y < kAutoScrollFudge)
      dy = kAutoScrollFudge - y;
   else if ((Int_t)dim.fHeight - kAutoScrollFudge <= y)
      dy = dim.fHeight - kAutoScrollFudge - y;

   if (dx || dy) {
      if (dx) dx /= 5;
      if (dy) dy /= 5;
      Int_t adx = TMath::Abs(dx);
      Int_t ady = TMath::Abs(dy);
      if (adx > kAutoScrollFudge) adx = kAutoScrollFudge;
      if (ady > kAutoScrollFudge) ady = kAutoScrollFudge;

      dx *= kAcceleration[adx];
      dy *= kAcceleration[ady];

      Int_t nx = pos.fX-dx;
      Int_t ny = pos.fY-dy;

      fCanvas->SetHsbPosition(nx);
      fCanvas->SetVsbPosition(ny);

      // position inside container
      x += pos.fX;
      y += pos.fY;

      fX0 =  TMath::Min(fXp, x);
      fY0 =  TMath::Min(fYp, y);
      fXf =  TMath::Max(fXp, x);
      fYf =  TMath::Max(fYp ,y);

      total = selected = 0;

      TIter next(fList);

      while ((el = (TGFrameElement *) next())) {
         f = el->fFrame;
         ++total;
         xf0 = f->GetX() + (f->GetWidth() >> 3);
         yf0 = f->GetY() + (f->GetHeight() >> 3);
         xff = xf0 + f->GetWidth() - (f->GetWidth() >> 2);
         yff = yf0 + f->GetHeight() - (f->GetHeight() >> 2);

         if (((xf0 > fX0 && xf0 < fXf) ||
            (xff > fX0 && xff < fXf)) &&
            ((yf0 > fY0 && yf0 < fYf) ||
            (yff > fY0 && yff < fYf))) {
            if (!el->fFrame->IsActive())
               ActivateItem(el);
            ++selected;
         } else {
            if (el->fFrame->IsActive())
               DeActivateItem(el);
         }
      }
      gVirtualX->DrawRectangle(fId, GetLineGC()(), fX0-pos.fX, fY0-pos.fY,
                               fXf-fX0, fYf-fY0);

      if (fTotal != total || fSelected != selected) {
         fTotal = total;
         fSelected = selected;
         SendMessage(fMsgWindow, MK_MSG(kC_CONTAINER, kCT_SELCHANGED),
                     fTotal, fSelected);
      }
      ClearViewPort();
      DoRedraw();
      gVirtualX->DrawRectangle(fId, GetLineGC()(), fX0-pos.fX, fY0-pos.fY,
                               fXf-fX0, fYf-fY0);
   }
}

//______________________________________________________________________________
void TGContainer::SearchPattern()
{
   // Search for entry which name begins with pattern.

   TGFrameElement *fe = 0;
   TIter next(fList);
   TString str;

   while ((fe=( TGFrameElement*)next())) {
      str = fe->fFrame->GetTitle();

      if (str.BeginsWith(fKeyInput,TString::kIgnoreCase)) {
         if (fLastActiveEl && (fLastActiveEl!=fe) ) {
            fLastActiveEl->fFrame->Activate(kFALSE);
         }
         ActivateItem(fe);
         AdjustPosition();
         break;
      }
   }

   fKeyInput = "";   //clear
   fKeyTimerActive = kFALSE;
}

//______________________________________________________________________________
void TGContainer::RepeatSearch()
{
   // Repeats search.

   TGFrameElement *fe = 0;

   if (fLastName == "")
      return Search();

   fe = (TGFrameElement*)FindItem(fLastName, fLastDir, fLastCase, fLastSubstring);

   if (!fe) {
      if (fLastActiveEl) DeActivateItem(fLastActiveEl);
      fLastActiveEl = 0;
      fe = (TGFrameElement*)FindItem(fLastName, fLastDir, fLastCase, fLastSubstring);

      if (!fe) {
         TString msg = "Couldn't find \"" + fLastName + '\"';
         gVirtualX->Bell(50);
         new TGMsgBox(fClient->GetDefaultRoot(), fCanvas, "Container", msg.Data(),
                        kMBIconExclamation, kMBOk, 0);
      } else {
         if (fLastActiveEl) DeActivateItem(fLastActiveEl);
         ActivateItem(fe);
         AdjustPosition();
      }
   } else {
      if (fLastActiveEl) DeActivateItem(fLastActiveEl);
      ActivateItem(fe);
      AdjustPosition();
   }
}

//______________________________________________________________________________
TGFrameElement *TGContainer::FindFrame(Int_t x, Int_t y, Bool_t exclude)
{
   // Find frame located int container at position x,y.

   TIter next(fList);
   TGFrameElement *el;
   TGFrameElement *ret = 0;
   Int_t dx = 0;
   Int_t dy = 0;
   Int_t d = 0;
   Int_t dd;

   el = (TGFrameElement *) next();
   if (!el) return 0;

   dx = TMath::Abs(el->fFrame->GetX()-x);
   dy = TMath::Abs(el->fFrame->GetY()-y);
   d = dx + dy;

   while ((el = (TGFrameElement *) next())) {
      if (exclude && (el==fLastActiveEl) ) continue;
      dx = TMath::Abs(el->fFrame->GetX()-x);
      dy = TMath::Abs(el->fFrame->GetY()-y);
      dd = dx+dy;

      if (dd<d) {
         d = dd;
         ret = el;
      }
   }
   return ret;
}

//______________________________________________________________________________
void *TGContainer::FindItem(const TString& name, Bool_t direction,
                            Bool_t caseSensitive, Bool_t subString)
{

   // Find a frame which assosiated object has a name containing a "name"
   // string.

   if (name.IsNull()) return 0;
   int idx = kNPOS;

   TGFrameElement *el = 0;
   TString str;
   TString::ECaseCompare cmp = caseSensitive ? TString::kExact : TString::kIgnoreCase;

   fLastDir = direction;
   fLastCase = caseSensitive;
   fLastName = name;
   fLastSubstring = subString;

   if (fLastActiveEl) {
      el = fLastActiveEl;

      if (direction) {
         el = (TGFrameElement *)fList->After(el);
      } else {
         el = (TGFrameElement *)fList->Before(el);
      }
   } else {
      if (direction) el = (TGFrameElement *)fList->First();
      else el  = (TGFrameElement *)fList->Last();
   }

   while (el) {
      str = el->fFrame->GetTitle();
      idx = str.Index(name, 0, cmp);

      if (idx != kNPOS) {
         if (subString) {
            return el;
         } else {
            if (str.Length() == name.Length()) return el;
         }
      }

      if (direction) {
         el = (TGFrameElement *)fList->After(el);
      } else {
         el = (TGFrameElement *)fList->Before(el);
      }
   }
   return 0;
}

//______________________________________________________________________________
TGHScrollBar *TGContainer::GetHScrollbar() const
{
   // returns pointer to hor. scroll bar

   return fCanvas ? fCanvas->GetHScrollbar() : 0;
}

//______________________________________________________________________________
TGVScrollBar *TGContainer::GetVScrollbar() const
{
   // returns pointer to vert. scroll bar

   return fCanvas ? fCanvas->GetVScrollbar() : 0;
}

//______________________________________________________________________________
void TGContainer::SetVsbPosition(Int_t newPos)
{
   // Set position of vertical scrollbar.

   if (!fViewPort) return;
   TGVScrollBar *vb = GetVScrollbar();

   if (vb && vb->IsMapped()) {
      vb->SetRange((Int_t)GetHeight(), (Int_t)fViewPort->GetHeight());
      vb->SetPosition(newPos);
   } else {
      fViewPort->SetVPos(0);
   }
}

//______________________________________________________________________________
void TGContainer::SetHsbPosition(Int_t newPos)
{
   // set new hor. position

   if (!fViewPort) return;
   TGHScrollBar *hb = GetHScrollbar();

   if (hb && hb->IsMapped()) {
      hb->SetRange((Int_t)GetWidth(), (Int_t)fViewPort->GetWidth());
      hb->SetPosition(newPos);
   } else {
      fViewPort->SetHPos(0);
   }
}

//______________________________________________________________________________
void TGContainer::AdjustPosition()
{
   // Move content to position of highlighted/activated frame.

   if (!fViewPort) return;
   if (!fLastActiveEl) return;
   TGFrame *f = fLastActiveEl->fFrame;

   Int_t vh = 0;
   Int_t v = 0;

   TGHScrollBar *hb = GetHScrollbar();
   TGVScrollBar *vb = GetVScrollbar();
   Int_t pos = GetPagePosition().fY;
   Int_t pg;


   if (vb && vb->IsMapped()) {
      pg = (vb->GetPageSize()*GetHeight())/fViewPort->GetHeight();
      vh =  pos + (Int_t)fViewPort->GetHeight();

      if (f->GetY() < pos) {
         v = TMath::Max(0, f->GetY() - (Int_t)fViewPort->GetHeight()/2);
         v = (v*pg)/GetHeight();

         SetVsbPosition(v);
      } else if (f->GetY() + (Int_t)f->GetHeight() > vh) {
         v = TMath::Min((Int_t)GetHeight() - (Int_t)fViewPort->GetHeight(),
                        f->GetY() + (Int_t)f->GetHeight() - (Int_t)fViewPort->GetHeight()/2);
         v = (v*pg)/GetHeight();
         SetVsbPosition(v);
      }
   }

   Int_t hw = 0;
   Int_t h = 0;

   if (hb && hb->IsMapped() && (!vb || (vb && !vb->IsMapped()))) {
      pg = (hb->GetPageSize()*GetWidth())/fViewPort->GetWidth();
      pos =GetPagePosition().fX;
      hw = pos + (Int_t)fViewPort->GetWidth();

      if (f->GetX() < pos) {
         h = TMath::Max(0, f->GetX() - (Int_t)fViewPort->GetWidth()/2);
         h = (h*pg)/GetWidth();

         SetHsbPosition(h);
      } else if (f->GetX() + (Int_t)f->GetWidth() > hw) {
         h = TMath::Min((Int_t)GetWidth() - (Int_t)fViewPort->GetWidth(),
                        f->GetX() + (Int_t)f->GetWidth() - (Int_t)fViewPort->GetWidth()/2);
         h = (h*pg)/GetWidth();

         SetHsbPosition(h);
      }
   }
}

//______________________________________________________________________________
void TGContainer::LineLeft(Bool_t select)
{
   // Move current position one column left.

   TGPosition pos = GetPagePosition();
   TGDimension dim = GetPageDimension();

   TGFrameElement *fe = (TGFrameElement*)fList->First();
   if (!fe) return; // empty list

   TGFrameElement *old = fLastActiveEl;

   if (old) DeActivateItem(old);   //
   else fLastActiveEl = fe;

   TGFrameElement *la = fLastActiveEl;
   Int_t dx = la->fLayout->GetPadLeft() + la->fLayout->GetPadRight();
   Int_t dy = la->fLayout->GetPadTop() + la->fLayout->GetPadBottom();
   Int_t y = la->fFrame->GetY();
   Int_t x = la->fFrame->GetX() - dx;

   Int_t hw = pos.fX + dim.fWidth;

   TGHScrollBar *hb = GetHScrollbar();
   if (x<=0 && (hb && !hb->IsMapped())) { // move to previous line
      x = hw;
      y = y - la->fFrame->GetDefaultHeight() - dy;
   }

   fe = FindFrame(x, y);
   if (!fe) fe = (TGFrameElement*)fList->First();

   if (!select) fSelected=1;

   ActivateItem(fe);
   AdjustPosition();
}

//______________________________________________________________________________
void TGContainer::LineRight(Bool_t select)
{
   // Move current position one column right.

   TGPosition pos = GetPagePosition();
   TGDimension dim = GetPageDimension();

   TGFrameElement *fe = (TGFrameElement*)fList->Last();
   if (!fe) return;

   TGFrameElement *old = fLastActiveEl;

   if (old) DeActivateItem(old);
   else fLastActiveEl = (TGFrameElement*)fList->First();

   Int_t dx = fLastActiveEl->fLayout->GetPadLeft() + fLastActiveEl->fLayout->GetPadRight();
   Int_t dy = fLastActiveEl->fLayout->GetPadTop() + fLastActiveEl->fLayout->GetPadBottom();
   Int_t y = fLastActiveEl->fFrame->GetY();
   Int_t x = fLastActiveEl->fFrame->GetX() + fLastActiveEl->fFrame->GetDefaultWidth() + dx;

   Int_t hw = pos.fX + dim.fWidth - dx;

   TGHScrollBar *hb =  GetHScrollbar();
   if (x >= hw && (hb && !hb->IsMapped())) { // move one line down
      x = 0;
      y = y + fLastActiveEl->fFrame->GetDefaultHeight() + dy;
   }

   fe = FindFrame(x, y);
   if (!fe) fe = (TGFrameElement*)fList->Last();
   if (!select) fSelected = 1;

   ActivateItem(fe);
   AdjustPosition();
}

//______________________________________________________________________________
void TGContainer::LineUp(Bool_t select)
{
   // Make current position first line in window by scrolling up.

   TGFrameElement *fe = (TGFrameElement*)fList->First();
   if (!fe) return;

   TGFrameElement *old = fLastActiveEl;

   if (old) {
      DeActivateItem(old);
   } else {
      fLastActiveEl = (TGFrameElement*)fList->First();
   }

   Int_t dy = fLastActiveEl->fLayout->GetPadTop() + fLastActiveEl->fLayout->GetPadBottom();
   Int_t y = fLastActiveEl->fFrame->GetY() - dy;
   Int_t x = fLastActiveEl->fFrame->GetX();

   fe = FindFrame(x, y);
   if (!fe) fe = (TGFrameElement*)fList->First();
   if (fe->fFrame->GetY() > fLastActiveEl->fFrame->GetY()) fe = fLastActiveEl;
   if (!select) fSelected = 1;

   ActivateItem(fe);
   AdjustPosition();
}

//______________________________________________________________________________
void TGContainer::LineDown(Bool_t select)
{
   // Move one line down.

   TGFrameElement *fe = (TGFrameElement*)fList->Last();
   if (!fe) return;

   TGFrameElement* old = fLastActiveEl;

   if (old) DeActivateItem(old);
   else fLastActiveEl = (TGFrameElement*)fList->First();

   Int_t dy = fLastActiveEl->fLayout->GetPadTop() + fLastActiveEl->fLayout->GetPadBottom();
   Int_t y = fLastActiveEl->fFrame->GetY() +
             fLastActiveEl->fFrame->GetHeight() + dy;
   Int_t x = fLastActiveEl->fFrame->GetX();

   fe = FindFrame(x, y);
   if (!fe) fe = (TGFrameElement*)fList->Last();
   if (fe->fFrame->GetY() < fLastActiveEl->fFrame->GetY()) fe = fLastActiveEl;
   if (!select) fSelected = 1;

   ActivateItem(fe);
   AdjustPosition();
}

//______________________________________________________________________________
void TGContainer::PageUp(Bool_t select)
{
   // Move  position one page up.

   TGDimension dim = GetPageDimension();

   TGFrameElement *fe = (TGFrameElement*)fList->First();
   if (!fe) return;

   TGFrameElement* old = fLastActiveEl;

   if (old) DeActivateItem(old);
   else fLastActiveEl = (TGFrameElement*)fList->First();

   Int_t y = fLastActiveEl->fFrame->GetY();
   Int_t x = fLastActiveEl->fFrame->GetX();

   TGVScrollBar *vb =  GetVScrollbar();
   TGHScrollBar *hb =  GetHScrollbar();

   if (vb && vb->IsMapped()) {
      y -= dim.fHeight;
   } else {
      if (hb && hb->IsMapped()) {
         x -= dim.fWidth;
      } else {
         Home();
         return;
      }
   }

   fe = FindFrame(x, y);

   if (!fe || fe->fFrame->GetY()>fLastActiveEl->fFrame->GetY()) {
      fe = (TGFrameElement*)fList->First();
   }

   if (!select) fSelected = 1;

   ActivateItem(fe);
   AdjustPosition();
}

//______________________________________________________________________________
void TGContainer::PageDown(Bool_t select)
{
   // Move position one page down.

   TGDimension dim = GetPageDimension();

   TList *li = GetList();
   TGFrameElement *fe = (TGFrameElement*)fList->Last();
   if (!fe) return;

   TGFrameElement *old = fLastActiveEl;

   if (old) DeActivateItem(old);
   else fLastActiveEl = (TGFrameElement*)fList->First();

   Int_t y = fLastActiveEl->fFrame->GetY();
   Int_t x = fLastActiveEl->fFrame->GetX();

   TGVScrollBar *vb = GetVScrollbar();
   TGHScrollBar *hb = GetHScrollbar();

   if (vb && vb->IsMapped()) {
      y += dim.fHeight;
   } else {
      if (hb && hb->IsMapped()) {
         x += dim.fWidth;
      } else {
         End();
         return;
      }
   }

   fe = FindFrame(x, y);
   if (!fe || fe->fFrame->GetY()<fLastActiveEl->fFrame->GetY() ) {
      fe = (TGFrameElement*)li->Last();
   }

   if (!select) fSelected = 1;

   ActivateItem(fe);
   AdjustPosition();
}

//______________________________________________________________________________
void TGContainer::Home(Bool_t select)
{
   // Move to upper-left corner of container.

   TGFrameElement *fe = (TGFrameElement*)fList->First();
   if (!fe) return;

   TGFrameElement *old = fLastActiveEl;
   if (old) DeActivateItem(old);

   if (!select) fSelected = 1;

   ActivateItem(fe);
   AdjustPosition();
}

//______________________________________________________________________________
void TGContainer::End(Bool_t select)
{
   // Move to the bottom-right corner of container.

   TGFrameElement *fe = (TGFrameElement*)fList->Last();
   if (!fe) return;

   TGFrameElement *old = fLastActiveEl;
   if (old) DeActivateItem(old);

   if (!select) fSelected = 1;

   ActivateItem(fe);
   AdjustPosition();
}

//______________________________________________________________________________
const TGGC &TGContainer::GetLineGC()
{
   // Get graphics context for line drawing.

   if (!fgLineGC) {
      GCValues_t gval;
      gval.fMask = kGCForeground | kGCBackground | kGCFunction | kGCFillStyle |
                   kGCLineWidth  | kGCLineStyle  | kGCSubwindowMode |
                   kGCGraphicsExposures;
      gval.fForeground = fgWhitePixel ^ fgBlackPixel;
      gval.fBackground = fgWhitePixel;
      gval.fFunction   = kGXxor;
      gval.fLineWidth  = 0;
      gval.fLineStyle  = kLineOnOffDash;
      gval.fFillStyle  = kFillSolid;
      gval.fSubwindowMode = kIncludeInferiors;
      gval.fGraphicsExposures = kFALSE;
      fgLineGC = gClient->GetGC(&gval, kTRUE);
      fgLineGC->SetDashOffset(0);
      fgLineGC->SetDashList("\x1\x1", 2);
   }
   return *fgLineGC;
}

//______________________________________________________________________________
TGCanvas::TGCanvas(const TGWindow *p, UInt_t w, UInt_t h,
                   UInt_t options, ULong_t back) :
    TGFrame(p, w, h, options, back)
{
   // Create a canvas object.

   fVport      = new TGViewPort(this, w-4, h-4, kChildFrame | kOwnBackground,
                                fgWhitePixel);
   fHScrollbar = new TGHScrollBar(this, w-4, kDefaultScrollBarWidth);
   fVScrollbar = new TGVScrollBar(this, kDefaultScrollBarWidth, h-4);

   fScrolling  = kCanvasScrollBoth;

   fHScrollbar->Associate(this);
   fVScrollbar->Associate(this);

   fVport->Move(fBorderWidth, fBorderWidth);

   SetWindowName();

   fVScrollbar->SetEditDisabled(kEditDisable | kEditDisableGrab | kEditDisableBtnEnable);
   fHScrollbar->SetEditDisabled(kEditDisable | kEditDisableGrab | kEditDisableBtnEnable);
}

//______________________________________________________________________________
TGCanvas::~TGCanvas()
{
   // Delete canvas.

   delete fHScrollbar;
   delete fVScrollbar;
   delete fVport;
}

//______________________________________________________________________________
void TGCanvas::MapSubwindows()
{
   // Map all canvas sub windows.

   if (fHScrollbar) fHScrollbar->MapSubwindows();
   if (fVScrollbar) fVScrollbar->MapSubwindows();

   if (fVport) {
      TGFrame *container = fVport->GetContainer();
      if (!container) {
         Error("MapSubwindows", "no canvas container set yet");
         return;
      }
      container->MapSubwindows();
      fVport->MapSubwindows();
      fVport->MapWindow();
   }
   Layout();
}

//______________________________________________________________________________
void TGCanvas::AddFrame(TGFrame *f, TGLayoutHints *l)
{
   // Adding a frame to a canvas is actually adding the frame to the
   // viewport container. The viewport container must be at least a
   // TGCompositeFrame for this method to succeed.

   TGFrame *container = fVport->GetContainer();
   if (!container) {
      Error("AddFrame", "no canvas container set yet");
      return;
   }
   if (container->InheritsFrom(TGCompositeFrame::Class()))
      ((TGCompositeFrame*)container)->AddFrame(f, l);
   else
      Error("AddFrame", "canvas container must inherit from TGCompositeFrame");
}

//______________________________________________________________________________
void TGCanvas::DrawBorder()
{
   // Draw canvas border.

   switch (fOptions & (kSunkenFrame | kRaisedFrame | kDoubleBorder)) {
      case kSunkenFrame | kDoubleBorder:
         gVirtualX->DrawLine(fId, GetShadowGC()(), 0, 0, fWidth-2, 0);
         gVirtualX->DrawLine(fId, GetShadowGC()(), 0, 0, 0, fHeight-2);
         gVirtualX->DrawLine(fId, GetBlackGC()(), 1, 1, fWidth-3, 1);
         gVirtualX->DrawLine(fId, GetBlackGC()(), 1, 1, 1, fHeight-3);
         if (gClient->GetStyle() > 1) break;
         gVirtualX->DrawLine(fId, GetHilightGC()(), 0, fHeight-1, fWidth-1, fHeight-1);
         gVirtualX->DrawLine(fId, GetHilightGC()(), fWidth-1, fHeight-1, fWidth-1, 0);
         gVirtualX->DrawLine(fId, GetBckgndGC()(),  1, fHeight-2, fWidth-2, fHeight-2);
         gVirtualX->DrawLine(fId, GetBckgndGC()(),  fWidth-2, 1, fWidth-2, fHeight-2);
         break;

      default:
         TGFrame::DrawBorder();
         break;
   }
}

//______________________________________________________________________________
void TGCanvas::Layout()
{
   // Create layout for canvas. Depending on the size of the container
   // we need to add the scrollbars.

   Bool_t   need_vsb, need_hsb;
   UInt_t   cw, ch, tcw, tch;

   need_vsb = need_hsb = kFALSE;

   TGFrame *container = fVport->GetContainer();
   if (!container) {
      Error("Layout", "no canvas container set yet");
      return;
   }

   Bool_t fixedw = container->IsLayoutBroken() || (container->GetOptions() & kFixedWidth) ?
                   kTRUE : kFALSE;
   Bool_t fixedh = container->IsLayoutBroken() || (container->GetOptions() & kFixedHeight) ?
                   kTRUE : kFALSE;

   // test whether we need scrollbars
   cw = fWidth  - UInt_t(fBorderWidth << 1);
   ch = fHeight - UInt_t(fBorderWidth << 1);

   if (!fixedw) container->SetWidth(cw);
   if (!fixedh) container->SetHeight(ch);

   if (container->GetDefaultWidth() > cw) {
      if ((fScrolling & kCanvasScrollHorizontal) && fHScrollbar) {
         need_hsb = kTRUE;
         ch -= fHScrollbar->GetDefaultHeight();
         if ((Int_t) ch < 0) {
            //Warning("Layout", "height would become too small, setting to 10");
            ch = 10;
         }
         if (!fixedh) container->SetHeight(ch);
      }
   }

   if (container->GetDefaultHeight() > ch) {
      if ((fScrolling & kCanvasScrollVertical) && fVScrollbar) {
         need_vsb = kTRUE;
         cw -= fVScrollbar->GetDefaultWidth();
         if ((Int_t) cw < 0) {
            //Warning("Layout", "width would become too small, setting to 10");
            cw = 10;
         }
         if (!fixedw) container->SetWidth(cw);
      }
   }

   // re-check again (putting the vertical scrollbar could have changed things)

   if (container->GetDefaultWidth() > cw) {
      if (!need_hsb) {
         if ((fScrolling & kCanvasScrollHorizontal) && fHScrollbar) {
            need_hsb = kTRUE;
            ch -= fHScrollbar->GetDefaultHeight();
            if ((Int_t) ch < 0) {
               //Warning("Layout", "height would become too small, setting to 10");
               ch = 10;
            }
            if (!fixedh) container->SetHeight(ch);
         }
      }
   }

   fVport->MoveResize(fBorderWidth, fBorderWidth, cw, ch);

   tcw = TMath::Max(container->GetDefaultWidth(), cw);
   tch = TMath::Max(container->GetDefaultHeight(), ch);
   UInt_t curw = container->GetDefaultWidth();

   container->SetWidth(0); // force a resize in TGFrame::Resize

   if (fixedw && fixedh) {
      container->Resize(curw, container->GetDefaultHeight());
   } else if (fixedw) {
      container->Resize(curw, tch);
   } else if (fixedh) {
      container->Resize(tcw, container->GetDefaultHeight());
   } else {
      container->Resize(tcw, tch);
   }

   if (fHScrollbar) {
      if (need_hsb) {
         fHScrollbar->MoveResize(fBorderWidth, ch+fBorderWidth, cw, fHScrollbar->GetDefaultHeight());
         fHScrollbar->SetRange((Int_t)container->GetWidth(), (Int_t)fVport->GetWidth());
         fHScrollbar->MapWindow();
      } else {
         fHScrollbar->UnmapWindow();
         fHScrollbar->SetPosition(0);
         if (container->IsLayoutBroken()) {
            container->Resize(fVport->GetWidth(), container->GetHeight());
         }
      }
   }

   if (fVScrollbar) {
      if (need_vsb) {
         fVScrollbar->MoveResize(cw+fBorderWidth, fBorderWidth, fVScrollbar->GetDefaultWidth(), ch);
         fVScrollbar->SetRange((Int_t)container->GetHeight(), (Int_t)fVport->GetHeight());
         fVScrollbar->MapWindow();
      } else {
         fVScrollbar->UnmapWindow();
         fVScrollbar->SetPosition(0);
         if (container->IsLayoutBroken()) {
            container->Resize(container->GetWidth(), fVport->GetHeight());
         }
      }
   }
}

//______________________________________________________________________________
Bool_t TGCanvas::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Handle message generated by the canvas scrollbars.

   switch (GET_MSG(msg)) {
      case kC_HSCROLL:
         switch (GET_SUBMSG(msg)) {
            case kSB_SLIDERTRACK:
            case kSB_SLIDERPOS:
               fVport->SetHPos((Int_t)-parm1);
               break;
         }
         break;

      case kC_VSCROLL:
         switch (GET_SUBMSG(msg)) {
            case kSB_SLIDERTRACK:
            case kSB_SLIDERPOS:
               fVport->SetVPos((Int_t)-parm1);
               break;
         }
         break;

      default:
         break;
   }
   return kTRUE;
}

//______________________________________________________________________________
Int_t TGCanvas::GetHsbPosition() const
{
   // Get position of horizontal scrollbar.

   if (fHScrollbar && fHScrollbar->IsMapped())
      return fHScrollbar->GetPosition();
   return 0;
}

//______________________________________________________________________________
Int_t TGCanvas::GetVsbPosition() const
{
   // Get position of vertical scrollbar.

   if (fVScrollbar && fVScrollbar->IsMapped())
      return fVScrollbar->GetPosition();
   return 0;
}

//______________________________________________________________________________
void TGCanvas::SetHsbPosition(Int_t newPos)
{
   // Set position of horizontal scrollbar.

   if (fHScrollbar && fHScrollbar->IsMapped()) {
      TGFrame *container = fVport->GetContainer();
      fHScrollbar->SetRange((Int_t)container->GetWidth(), (Int_t)fVport->GetWidth());
      fHScrollbar->SetPosition(newPos);
   } else {
      fVport->SetHPos(0);
   }
}

//______________________________________________________________________________
void TGCanvas::SetVsbPosition(Int_t newPos)
{
   // Set position of vertical scrollbar.

   if (fVScrollbar && fVScrollbar->IsMapped()) {
      TGFrame *container = fVport->GetContainer();
      fVScrollbar->SetRange((Int_t)container->GetHeight(), (Int_t)fVport->GetHeight());
      fVScrollbar->SetPosition(newPos);
   } else {
      fVport->SetVPos(0);
   }
}

//______________________________________________________________________________
void TGCanvas::SetScrolling(Int_t scrolling)
{
   // Set scrolling policy. Use values defined by the enum: kCanvasNoScroll,
   // kCanvasScrollHorizontal, kCanvasScrollVertical, kCanvasScrollBoth.

   if (scrolling != fScrolling) {
      fScrolling = scrolling;
      Layout();
   }
}

//______________________________________________________________________________
void TGCanvas::ClearViewPort()
{
   // Clear view port and redraw content.

   TGFrame *cont = GetContainer();
   if (!cont) return;

   gVirtualX->ClearArea(cont->GetId(), 0, 0, fVport->GetWidth(), fVport->GetHeight());
   fClient->NeedRedraw(cont);
}

//______________________________________________________________________________
void TGCanvas::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a canvas widget as a C++ statement(s) on output stream out.

   if (fBackground != GetDefaultFrameBackground()) SaveUserColor(out, option);

   out << endl << "   // canvas widget" << endl;

   out << "   TGCanvas *";
   out << GetName() << " = new TGCanvas("<< fParent->GetName()
       << "," << GetWidth() << "," << GetHeight();

   if (fBackground == GetDefaultFrameBackground()) {
      if (GetOptions() == (kSunkenFrame | kDoubleBorder)) {
         out << ");" << endl;
      } else {
         out << "," << GetOptionString() << ");" << endl;
      }
   } else {
      out << "," << GetOptionString() << ",ucolor);" << endl;
   }
   if (option && strstr(option, "keep_names"))
      out << "   " << GetName() << "->SetName(\"" << GetName() << "\");" << endl;

   TGViewPort *vp = GetViewPort();
   out << endl << "   // canvas viewport" << endl;
   out << "   TGViewPort *" << vp->GetName() << " = " << GetName()
       << "->GetViewPort();" << endl;

   TGContainer *cont = (TGContainer*)GetContainer();
   cont->SavePrimitive(out, option);

   out << "   " << vp->GetName() << "->AddFrame(" << cont->GetName()
       << ");" << endl;

   out << "   " << cont->GetName() << "->SetLayoutManager(";
   cont->GetLayoutManager()->SavePrimitive(out, option);
   out << ");"<< endl;

   out << "   " << cont->GetName() << "->MapSubwindows();" << endl;

   out << "   " << GetName() << "->SetContainer(" << cont->GetName()
       << ");" << endl;

   out << "   " << GetName() << "->MapSubwindows();" << endl;

   if (fHScrollbar && fHScrollbar->IsMapped())
      out << "   " << GetName() << "->SetHsbPosition(" << GetHsbPosition()
          << ");" << endl;


   if (fVScrollbar && fVScrollbar->IsMapped())
      out << "   " << GetName() << "->SetVsbPosition(" << GetVsbPosition()
          << ");" << endl;

}

//______________________________________________________________________________
void TGContainer::SavePrimitive(ostream &out, Option_t *option /*= ""*/)
{
   // Save a canvas container as a C++ statement(s) on output stream out.

   if (fBackground != GetDefaultFrameBackground()) SaveUserColor(out, option);

   out << endl << "   // canvas container" << endl;

   if ((fParent->GetParent())->InheritsFrom(TGCanvas::Class())) {
      out << GetName() << " = new TGContainer(" << GetCanvas()->GetName();
   } else {
      out << GetName() << " = new TGContainer(" << fParent->GetName();
      out << "," << GetWidth() << "," << GetHeight();
   }

   if (fBackground == GetDefaultFrameBackground()) {
      if (GetOptions() == (kSunkenFrame | kDoubleBorder)) {
         out <<");" << endl;
      } else {
         out << "," << GetOptionString() <<");" << endl;
      }
   } else {
      out << "," << GetOptionString() << ",ucolor);" << endl;
   }
   if (option && strstr(option, "keep_names"))
      out << "   " << GetName() << "->SetName(\"" << GetName() << "\");" << endl;
}
