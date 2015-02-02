// @(#)root/meta:$Id$
// Author: Fons Rademakers   20/06/96

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TDictionary                                                          //
//                                                                      //
// This class defines an abstract interface that must be implemented    //
// by all classes that contain dictionary information.                  //
//                                                                      //
// The dictionary is defined by the followling classes:                 //
// TDataType                              (typedef definitions)         //
// TGlobal                                (global variables)            //
// TGlobalFunc                            (global functions)            //
// TClass                                 (classes)                     //
//    TBaseClass                          (base classes)                //
//    TDataMember                         (class datamembers)           //
//    TMethod                             (class methods)               //
//       TMethodArg                       (method arguments)            //
//                                                                      //
// All the above classes implement the TDictionary abstract interface.  //
// Note: the indentation shows aggregation not inheritance.             //
//                                                                      //
// TMethodCall                            (method call environment)     //
//                                                                      //
//Begin_Html
/*
<img src="gif/tdictionary_classtree.gif">
*/
//End_Html
//////////////////////////////////////////////////////////////////////////

#include "TDictionary.h"
#include "TClass.h"
#include "TClassEdit.h"
#include "TDataType.h"
#include "TDictAttributeMap.h"
#include "TROOT.h"

ClassImp(TDictionary)

TDictionary::TDictionary(const TDictionary& dict):
   TNamed(dict),
   fAttributeMap(dict.fAttributeMap ?
                 ((TDictAttributeMap*)dict.fAttributeMap->Clone()) : 0 )
{ }

TDictionary::~TDictionary() {
   // Destruct a TDictionary, delete the attribute map.
   if(fAttributeMap) {
      delete fAttributeMap;
   }
}

TDictionary &TDictionary::operator=(const TDictionary& dict)
{
  // Assignment op, cloning fAttributeMap.
  TNamed::operator=(dict);

  delete fAttributeMap;
  fAttributeMap = 0;
  if (dict.fAttributeMap)
    fAttributeMap = ((TDictAttributeMap*)dict.fAttributeMap->Clone());

  return *this;
}

void TDictionary::CreateAttributeMap()
{
   //Create a TDictAttributeMap for a TClass to be able to add attribute pairs
   //key-value to the TClass.

   if (!fAttributeMap)
      fAttributeMap = new TDictAttributeMap;
}

TDictionary* TDictionary::GetDictionary(const char* name)
{
   // Retrieve the type (class, fundamental type, typedef etc)
   // named "name". Returned object is either a TClass or TDataType.
   // Returns 0 if the type is unknown.

   TClassEdit::TSplitType stname(name, TClassEdit::kDropStd);
   std::string shorttype;
   stname.ShortType(shorttype, TClassEdit::kDropAllDefault);

   TDictionary* ret = (TDictionary*)gROOT->GetListOfTypes()
      ->FindObject(shorttype.c_str());
   if (ret) return ret;

   return TClass::GetClass(shorttype.c_str(), true);
}

TDictionary* TDictionary::GetDictionary(const type_info &typeinfo)
{
   // Retrieve the type (class, fundamental type, typedef etc)
   // with typeid typeinfo. Returned object is either a TClass or TDataType.
   // Returns 0 if the type is unknown.

   EDataType datatype = TDataType::GetType(typeinfo);
   TDictionary* ret = TDataType::GetDataType(datatype);
   if (ret) return ret;

   return TClass::GetClass(typeinfo, true);
}
