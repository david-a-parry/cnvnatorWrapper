/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooLinkedList.h,v 1.15 2007/05/11 09:11:30 verkerke Exp $
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
#ifndef ROO_LINKED_LIST
#define ROO_LINKED_LIST

#include <map>
#include <list>

#include "TNamed.h"
#include "RooLinkedListElem.h"
#include "RooHashTable.h"
class RooLinkedListIter ;
class RooFIter ;
class TIterator ;
class RooAbsArg ;

namespace RooLinkedListImplDetails {
    class Chunk;
    class Pool;
}

class RooLinkedList : public TObject {
public:
  // Constructor
  RooLinkedList(Int_t htsize=0) ;

  // Copy constructor
  RooLinkedList(const RooLinkedList& other) ;

  virtual TObject* Clone(const char* =0) const { 
    return new RooLinkedList(*this) ;
  }

  // Assignment operator
  RooLinkedList& operator=(const RooLinkedList& other) ;

  Int_t getHashTableSize() const {
    // Return size of hash table
    return _htableName ? _htableName->size() : 0 ;
  }

  void setHashTableSize(Int_t size) ;

  // Destructor
  virtual ~RooLinkedList() ;

  Int_t GetSize() const { return _size ; }

  virtual void Add(TObject* arg) { Add(arg,1) ; }
  virtual Bool_t Remove(TObject* arg) ;
  TObject* At(Int_t index) const ;
  Bool_t Replace(const TObject* oldArg, const TObject* newArg) ;
  TIterator* MakeIterator(Bool_t dir=kTRUE) const ;
  RooLinkedListIter iterator(Bool_t dir=kTRUE) const ;
  RooFIter fwdIterator() const ; 

  void Clear(Option_t *o=0) ;
  void Delete(Option_t *o=0) ;
  TObject* find(const char* name) const ;
  RooAbsArg* findArg(const RooAbsArg*) const ;
  TObject* FindObject(const char* name) const ; 
  TObject* FindObject(const TObject* obj) const ;
  Int_t IndexOf(const char* name) const ;
  Int_t IndexOf(const TObject* arg) const ;
  TObject* First() const {
    return _first?_first->_arg:0 ;
  }

  void Print(const char* opt) const ;
  void Sort(Bool_t ascend=kTRUE) ;
  
  const char* GetName() const { return _name.Data() ; }
  void SetName(const char* name) { _name = name ; }
  void useNptr(Bool_t flag) { _useNptr = flag ; }

protected:  

  RooLinkedListElem* createElement(TObject* obj, RooLinkedListElem* elem=0) ;
  void deleteElement(RooLinkedListElem*) ;


  friend class RooLinkedListIter ;
  friend class RooFIter ;

  virtual void Add(TObject* arg, Int_t refCount) ;

  RooLinkedListElem* findLink(const TObject* arg) const ;
    
  Int_t _hashThresh ;          //  Size threshold for hashing
  Int_t _size ;                //  Current size of list
  RooLinkedListElem*  _first ; //! Link to first element of list
  RooLinkedListElem*  _last ;  //! Link to last element of list
  RooHashTable*       _htableName ; //! Hash table by name 
  RooHashTable*       _htableLink ; //! Hash table by link pointer

  TString             _name ; 
  Bool_t              _useNptr ; //!

private:
  template <bool ascending>
  static RooLinkedListElem* mergesort_impl(RooLinkedListElem* l1,
	  const unsigned sz, RooLinkedListElem** tail = 0);
  /// memory pool for quick allocation of RooLinkedListElems
  typedef RooLinkedListImplDetails::Pool Pool;
  /// shared memory pool for allocation of RooLinkedListElems
  static Pool* _pool; //!

  ClassDef(RooLinkedList,3) // Doubly linked list for storage of RooAbsArg objects
};




#endif
