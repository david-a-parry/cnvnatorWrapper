/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, NIKHEF, verkerke@nikhef.nl                         *
 *                                                                           *
 * Copyright (c) 2000-2011, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
#ifndef ROO_UNIT_TEST
#define ROO_UNIT_TEST

#include "Rtypes.h"
#include "TNamed.h" 
#include "RooTable.h"
#include "RooWorkspace.h"
#include "RooFitResult.h"
#include "RooPlot.h"
#include "TFile.h"
#include "TH1.h"
#include <list>
#include <string>
#include <map>


class RooUnitTest : public TNamed {
public:
  RooUnitTest(const char* name, TFile* refFile, Bool_t writeRef, Int_t verbose) ;
  ~RooUnitTest() ;
  
  void setDebug(Bool_t flag) { _debug = flag ; }
  void setSilentMode() ;
  void clearSilentMode() ;
  void regPlot(RooPlot* frame, const char* refName) ;  
  void regResult(RooFitResult* r, const char* refName) ;
  void regValue(Double_t value, const char* refName) ;
  void regTable(RooTable* t, const char* refName) ;
  void regWS(RooWorkspace* ws, const char* refName) ;
  void regTH(TH1* h, const char* refName) ;
  RooWorkspace* getWS(const char* refName) ;
  Bool_t runTest() ;
  Bool_t runCompTests() ;
  Bool_t areTHidentical(TH1* htest, TH1* href) ;

  virtual Bool_t isTestAvailable() { return kTRUE ; }
  virtual Bool_t testCode() = 0 ;  

  virtual Double_t htol() { return 5e-4 ; } // histogram test tolerance (KS dist != prob)
  virtual Double_t ctol() { return 2e-3 ; } // curve test tolerance
  virtual Double_t fptol() { return 1e-3 ; } // fit parameter test tolerance
  virtual Double_t fctol() { return 1e-3 ; } // fit correlation test tolerance
  virtual Double_t vtol() { return 1e-3 ; } // value test tolerance

  static void setMemDir(TDirectory* memDir);

protected:

  static TDirectory* gMemDir ;

  TFile* _refFile ;
  Bool_t _debug ;
  Bool_t _write ;
  Int_t _verb ;
   std::list<std::pair<RooPlot*, std::string> > _regPlots ;
   std::list<std::pair<RooFitResult*, std::string> > _regResults ;
   std::list<std::pair<Double_t, std::string> > _regValues ;
   std::list<std::pair<RooTable*,std::string> > _regTables ;
   std::list<std::pair<RooWorkspace*,std::string> > _regWS ;
   std::list<std::pair<TH1*,std::string> > _regTH ;

  ClassDef(RooUnitTest,0) ; // Abstract base class for RooFit/RooStats unit regression tests
} ;
#endif
