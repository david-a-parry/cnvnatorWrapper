// @(#)root/meta:$Id$
// Author: Fons Rademakers   20/06/96

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#ifndef ROOT_TDictionary
#define ROOT_TDictionary

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
// TFunction                              (global functions)            //
// TClass                                 (classes)                     //
//    TBaseClass                          (base classes)                //
//    TDataMember                         (class datamembers)           //
//    TMethod                             (class methods)               //
//       TMethodArg                       (method arguments)            //
//                                                                      //
// All the above classes implement the TDictionary abstract interface   //
// (note: the indentation shows aggregation not inheritance).           //
// The ROOT dictionary system provides a very extensive RTTI            //
// environment that facilitates a.o. object inspectors, object I/O,     //
// ROOT Trees, etc. Most of the type information is provided by the     //
// CINT C++ interpreter.                                                //
//                                                                      //
// TMethodCall                            (method call environment)     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

#include "Property.h"

class TDictAttributeMap;

typedef void CallFunc_t;
typedef void ClassInfo_t;
typedef void BaseClassInfo_t;
typedef void DataMemberInfo_t;
typedef void MethodInfo_t;
typedef void MethodArgInfo_t;
typedef void MethodArgInfo_t;
typedef void TypeInfo_t;
typedef void TypedefInfo_t;

#ifndef ROOT_ESTLType
#include "ESTLType.h"
#endif

enum EProperty {
   kIsClass        = G__BIT_ISCLASS,
   kIsStruct       = G__BIT_ISSTRUCT,
   kIsUnion        = G__BIT_ISUNION,
   kIsEnum         = G__BIT_ISENUM,
   kIsNamespace    = G__BIT_ISNAMESPACE,
   kIsTypedef      = G__BIT_ISTYPEDEF,
   kIsFundamental  = G__BIT_ISFUNDAMENTAL,
   kIsAbstract     = G__BIT_ISABSTRACT,
   kIsVirtual      = G__BIT_ISVIRTUAL,
   kIsPureVirtual  = G__BIT_ISPUREVIRTUAL,
   kIsPublic       = G__BIT_ISPUBLIC,
   kIsProtected    = G__BIT_ISPROTECTED,
   kIsPrivate      = G__BIT_ISPRIVATE,
   kIsPointer      = G__BIT_ISPOINTER,
   kIsArray        = G__BIT_ISARRAY,
   kIsStatic       = G__BIT_ISSTATIC,
   kIsUsingVariable= G__BIT_ISUSINGVARIABLE,
   kIsDefault      = G__BIT_ISDEFAULT,
   kIsReference    = G__BIT_ISREFERENCE,
   kIsConstant     = G__BIT_ISCONSTANT,
   kIsConstPointer = G__BIT_ISPCONSTANT,
   kIsMethConst    = G__BIT_ISMETHCONSTANT
};


class TDictionary : public TNamed {

private:
   TDictAttributeMap *fAttributeMap;    //pointer to a dictionary attribute map

public:
   TDictionary(): fAttributeMap(0) { }
   TDictionary(const char* name): TNamed(name, ""), fAttributeMap(0) { }
   TDictionary(const TDictionary& dict);
   virtual ~TDictionary();

   TDictionary& operator=(const TDictionary& other);
   void               CreateAttributeMap();
   TDictAttributeMap *GetAttributeMap() const
   {
      //Get the TDictAttributeMap pointer to be able to add attribute
      //pairs key-value to the TClass.
      return fAttributeMap;
   }
   virtual Long_t      Property() const = 0;
   static TDictionary* GetDictionary(const char* name);
   static TDictionary* GetDictionary(const type_info &typeinfo);

   // Type of STL container (returned by IsSTLContainer).
   enum ESTLType {
      kNone      = ROOT::kNotSTL,
      kVector    = ROOT::kSTLvector,
      kList      = ROOT::kSTLlist,
      kDeque     = ROOT::kSTLdeque,
      kMap       = ROOT::kSTLmap,
      kMultimap  = ROOT::kSTLmultimap,
      kSet       = ROOT::kSTLset,
      kMultiset  = ROOT::kSTLmultiset,
      kBitset    = ROOT::kSTLbitset
   };

   ClassDef(TDictionary,0)  //ABC defining interface to dictionary
};

#endif
