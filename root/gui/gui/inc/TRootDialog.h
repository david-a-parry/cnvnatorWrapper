// @(#)root/gui:$Id$
// Author: Fons Rademakers   20/02/98

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TRootDialog
#define ROOT_TRootDialog


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TRootDialog                                                          //
//                                                                      //
// A TRootDialog is used to prompt for the arguments of an object's     //
// member function. A TRootDialog is created via the context menu's     //
// when selecting a member function taking arguments.                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif

class TRootContextMenu;


class TRootDialog : public TGTransientFrame {

private:
   TRootContextMenu *fMenu;    // associated context menu
   TGLayoutHints    *fL1;      // label layout
   TGLayoutHints    *fL2;      // text entry layout
   TList            *fWidgets; // label and text field widgets created in dialog
   Bool_t            fOk;      // if true show OK button
   Bool_t            fCancel;  // if true show Cancel button
   Bool_t            fApply;   // if true show Apply button
   Bool_t            fHelp;    // if true show Online Help button

public:
   TRootDialog(TRootContextMenu *cmenu = 0, const TGWindow *main = 0, 
               const char *title = "ROOT Dialog", Bool_t okB = kTRUE, 
               Bool_t cancelB = kTRUE, Bool_t applyB = kFALSE, 
               Bool_t helpB = kTRUE);
   virtual ~TRootDialog();

   virtual void Add(const char *argname, const char *value, const char *type);
   //virtual void Add(TGComboBox *optionSel);

   virtual const char *GetParameters();

   virtual void   CloseWindow();
   virtual void   Popup();
   virtual Bool_t HandleKey(Event_t *event);

   void TabPressed();

   ClassDef(TRootDialog,0)  //Native GUI method argument prompt dialog box
};

#endif

