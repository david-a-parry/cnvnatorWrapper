/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooSetProxy.h,v 1.21 2007/07/13 21:24:36 wouter Exp $
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
#ifndef ROO_SET_PROXY
#define ROO_SET_PROXY

#include "TObject.h"
#include "Riosfwd.h"
#include "RooAbsProxy.h"
#include "RooAbsArg.h"
#include "RooArgSet.h"

class RooSetProxy : public RooArgSet, public RooAbsProxy  {
public:

#ifdef USEMEMPOOL
  void* operator new (size_t bytes);
  void operator delete (void *ptr);
#endif

  // Constructors, assignment etc.
  RooSetProxy() { 
    // coverity[UNINIT_CTOR]
    _iter = createIterator() ; _owner = 0 ; 
  } ;
  RooSetProxy(const char* name, const char* desc, RooAbsArg* owner, 
	      Bool_t defValueServer=kTRUE, Bool_t defShapeServer=kFALSE) ;
  RooSetProxy(const char* name, RooAbsArg* owner, const RooSetProxy& other) ;
  virtual ~RooSetProxy() ;

  virtual const char* name() const { return GetName() ; }

  // List content management (modified for server hooks)
  virtual Bool_t add(const RooAbsArg& var, Bool_t silent=kFALSE) ;
  virtual Bool_t add(const RooAbsCollection& list, Bool_t silent=kFALSE) { return RooAbsCollection::add(list,silent) ; }
  virtual Bool_t add(const RooAbsArg& var, Bool_t valueServer, Bool_t shapeServer, Bool_t silent) ;
  virtual Bool_t addOwned(RooAbsArg& var, Bool_t silent=kFALSE);
  virtual Bool_t addOwned(const RooAbsCollection& list, Bool_t silent=kFALSE) { return RooAbsCollection::addOwned(list,silent) ; }
  virtual RooAbsArg *addClone(const RooAbsArg& var, Bool_t silent=kFALSE) ;
  virtual void   addClone(const RooAbsCollection& list, Bool_t silent=kFALSE) { RooAbsCollection::addClone(list,silent) ; }
  virtual Bool_t replace(const RooAbsArg& var1, const RooAbsArg& var2) ;
  virtual Bool_t remove(const RooAbsArg& var, Bool_t silent=kFALSE, Bool_t matchByNameOnly=kFALSE) ;
  Bool_t remove(const RooAbsCollection& list, Bool_t silent=kFALSE, Bool_t matchByNameOnly=kFALSE) ;
  virtual void removeAll() ;

  virtual void print(std::ostream& os, Bool_t addContents=kFALSE) const ;

  RooSetProxy& operator=(const RooArgSet& other) ;
  
protected:

  RooAbsArg* _owner ;
  Bool_t _defValueServer ;
  Bool_t _defShapeServer ;
  TIterator* _iter ; //! do not persist

  virtual Bool_t changePointer(const RooAbsCollection& newServerSet, Bool_t nameChange=kFALSE, Bool_t factoryInitMode=kFALSE) ;

  ClassDef(RooSetProxy,1) // Proxy class for a RooArgSet
};

#endif

