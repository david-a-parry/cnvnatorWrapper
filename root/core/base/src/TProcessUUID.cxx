// @(#)root/base:$Id$
// Author: Rene Brun    06/07/2002

/*************************************************************************
 * Copyright (C) 1995-2001, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
// 
// TProcessUUID
//
// This class is a specialized TProcessID managing the list of UUIDs.
// In addition to TProcessID, this object has the following members:
//   - fUUIDs  : a THashList of TUUIDs in string format (using a TObjString)
//   - fActive : a TBits table with one bit per TUUID in the table
// When a new TUUID is entered into the list fUUIDs, it is assigned
// the first free slot in the list of bits and the TUUID UUIDNumber
// is set to this slot number.
// When a TUUID is removed from the list, the corresponding bit
// is reset in fActive.
// The object corresponding to a TUUID at slot I can be found
// via fObjects->At(I).
// One can use two mechanisms to find the object corresponding to a TUUID:
//  1- the input is the TUUID.AsString. One can find the corresponding 
//     TObjString object objs in fUUIDs via THashList::FindObject(name).
//     The slot number is then objs->GetUniqueID().
//  2- The input is the UUIDNumber. The slot number is UIUIDNumber
//
// When a TRef points to an object having a TUUID, both the TRef and the
// referenced object have their bit kHasUUID set. In this case, the pointer
// TProcessID *fPID in TRef points to the unique object TProcessUUID.
// The TRef uniqueID is directly the UUIDNumber=slot number.
//
//////////////////////////////////////////////////////////////////////////

#include "TROOT.h"
#include "TProcessUUID.h"
#include "THashList.h"
#include "TBits.h"
#include "TObjString.h"
#include "TUUID.h"

ClassImp(TProcessUUID)

//______________________________________________________________________________
TProcessUUID::TProcessUUID() : TProcessID()
{
   // Default constructor.

   fUUIDs   = new THashList(100,3);
   fActive  = new TBits(100);
   IncrementCount();
}

//______________________________________________________________________________
TProcessUUID::~TProcessUUID()
{
   // Destructor.
   fUUIDs->Delete();
   delete fUUIDs;  fUUIDs  = 0;
   delete fActive; fActive = 0;
}

//______________________________________________________________________________
UInt_t TProcessUUID::AddUUID(TUUID &uuid, TObject *obj)
{
   // Add uuid to the table of UUIDs
   // The TObject *obj has its uniqueID set to the UUID number
   // return entry number in the table

   UInt_t number;
   const char *uuids = uuid.AsString();
   TObjString *objs = (TObjString*)fUUIDs->FindObject(uuids);
   if (objs) {
      number = objs->GetUniqueID();
      uuid.SetUUIDNumber(number);
      objs->SetUniqueID(number);
      obj->SetUniqueID(number);
      obj->SetBit(kHasUUID);
      if (number >= (UInt_t)fObjects->GetSize()) fObjects->AddAtAndExpand(obj,number);
      if (fObjects->UncheckedAt(number) == 0) fObjects->AddAt(obj,number);
      return number;
   }   

   objs = new TObjString(uuids);
   fUUIDs->Add(objs);
   number = fActive->FirstNullBit();
   uuid.SetUUIDNumber(number);
   objs->SetUniqueID(number);
   obj->SetUniqueID(number);
   obj->SetBit(kHasUUID);
   fActive->SetBitNumber(number);
   fObjects->AddAtAndExpand(obj,number);
   return number;
}

//______________________________________________________________________________
UInt_t TProcessUUID::AddUUID(const char *uuids)
{
   // Add uuid with name uuids to the table of UUIDs
   // return entry number in the table

   
   TObjString *objs = (TObjString*)fUUIDs->FindObject(uuids);
   if (objs) return objs->GetUniqueID();
   
   UInt_t number;
   objs = new TObjString(uuids);
   fUUIDs->Add(objs);
   number = fActive->FirstNullBit();
   objs->SetUniqueID(number);
   fActive->SetBitNumber(number);
   return number;
}

//______________________________________________________________________________
TObjString *TProcessUUID::FindUUID(UInt_t number) const
{
   //Find the TObjString by slot number
   
   TObjLink *lnk = fUUIDs->FirstLink();
   while (lnk) {
      TObject *obj = lnk->GetObject();
      if (obj->GetUniqueID() == number) return (TObjString*)obj;
      lnk = lnk->Next();
   }
   return 0;
}

//______________________________________________________________________________
void TProcessUUID::RemoveUUID(UInt_t number)
{
   //Remove entry number in the list of uuids
   
   if (number > (UInt_t)fObjects->GetSize()) return;
   TObjLink *lnk = fUUIDs->FirstLink();
   while (lnk) {
      TObject *obj = lnk->GetObject();
      if (obj->GetUniqueID() == number) {
         fUUIDs->Remove(lnk);
         delete obj;
         fActive->ResetBit(number);
         fObjects->AddAt(0,number);
         return;
      }
      lnk = lnk->Next();
   }
}   
