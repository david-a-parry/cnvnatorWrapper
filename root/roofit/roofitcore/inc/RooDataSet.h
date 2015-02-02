/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooDataSet.h,v 1.59 2007/05/11 09:11:30 verkerke Exp $
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
#ifndef ROO_DATA_SET
#define ROO_DATA_SET

class TDirectory ;
class RooAbsRealLValue ;
class RooRealVar ;
class RooDataHist ;
#include "RooAbsData.h"
#include "RooDirItem.h"

#define USEMEMPOOL


class RooDataSet : public RooAbsData, public RooDirItem {
public:

#ifdef USEMEMPOOL
  void* operator new (size_t bytes);
  void operator delete (void *ptr);
#endif
 

  // Constructors, factory methods etc.
  RooDataSet() ; 

  // Empty constructor 
  RooDataSet(const char *name, const char *title, const RooArgSet& vars, const char* wgtVarName=0) ;

  // Universal constructor
  RooDataSet(const char* name, const char* title, const RooArgSet& vars, const RooCmdArg& arg1, const RooCmdArg& arg2=RooCmdArg(), 
	     const RooCmdArg& arg3=RooCmdArg(), const RooCmdArg& arg4=RooCmdArg(),const RooCmdArg& arg5=RooCmdArg(),
	     const RooCmdArg& arg6=RooCmdArg(),const RooCmdArg& arg7=RooCmdArg(),const RooCmdArg& arg8=RooCmdArg()) ; 

    // Constructor for subset of existing dataset
  RooDataSet(const char *name, const char *title, RooDataSet *data, const RooArgSet& vars, 
             const char *cuts=0, const char* wgtVarName=0);
  RooDataSet(const char *name, const char *title, RooDataSet *data, const RooArgSet& vars,  
	     const RooFormulaVar& cutVar, const char* wgtVarName=0) ;  


  // Constructor importing data from external ROOT Tree
  RooDataSet(const char *name, const char *title, TTree *ntuple, const RooArgSet& vars, 
	     const char *cuts=0, const char* wgtVarName=0); 
  RooDataSet(const char *name, const char *title, TTree *t, const RooArgSet& vars, 
	     const RooFormulaVar& cutVar, const char* wgtVarName=0) ;  
  

  RooDataSet(RooDataSet const & other, const char* newname=0) ;  
  virtual TObject* Clone(const char* newname=0) const { return new RooDataSet(*this,newname?newname:GetName()) ; }
  virtual ~RooDataSet() ;

  virtual RooAbsData* emptyClone(const char* newName=0, const char* newTitle=0, const RooArgSet* vars=0, const char* wgtVarName=0) const ;

  RooDataHist* binnedClone(const char* newName=0, const char* newTitle=0) const ;

  virtual Double_t sumEntries() const ;
  virtual Double_t sumEntries(const char* cutSpec, const char* cutRange=0) const ;

  virtual RooPlot* plotOnXY(RooPlot* frame, 
			    const RooCmdArg& arg1=RooCmdArg::none(), const RooCmdArg& arg2=RooCmdArg::none(),
			    const RooCmdArg& arg3=RooCmdArg::none(), const RooCmdArg& arg4=RooCmdArg::none(),
			    const RooCmdArg& arg5=RooCmdArg::none(), const RooCmdArg& arg6=RooCmdArg::none(),
			    const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none()) const ;


  // Read data from a text file and create a dataset from it.
  // The possible options are: (D)ebug, (Q)uiet.
  static RooDataSet *read(const char *filename, const RooArgList &variables,
			  const char *opts= "", const char* commonPath="",
			  const char *indexCatName=0) ;
  Bool_t write(const char* filename) ;

/*   void setWeightVar(const char* name=0) ; */
/*   void setWeightVar(const RooAbsArg& arg) {  */
/*     // Interpret given argument as event weight */
/*     setWeightVar(arg.GetName()) ;  */
/*   } */
  virtual Bool_t isWeighted() const ;
  virtual Bool_t isNonPoissonWeighted() const ;

  virtual Double_t weight() const ; 
  virtual Double_t weightSquared() const ; 
  virtual void weightError(Double_t& lo, Double_t& hi,ErrorType etype=SumW2) const ;
  Double_t weightError(ErrorType etype=SumW2) const ;

  virtual const RooArgSet* get(Int_t index) const;
  virtual const RooArgSet* get() const ; 

  // Add one ore more rows of data
  virtual void add(const RooArgSet& row, Double_t weight=1.0, Double_t weightError=0);
  virtual void add(const RooArgSet& row, Double_t weight, Double_t weightErrorLo, Double_t weightErrorHi);

  virtual void addFast(const RooArgSet& row, Double_t weight=1.0, Double_t weightError=0);

  void append(RooDataSet& data) ;
  Bool_t merge(RooDataSet* data1, RooDataSet* data2=0, RooDataSet* data3=0,  
 	       RooDataSet* data4=0, RooDataSet* data5=0, RooDataSet* data6=0) ; 
  Bool_t merge(std::list<RooDataSet*> dsetList) ;

  virtual RooAbsArg* addColumn(RooAbsArg& var, Bool_t adjustRange=kTRUE) ;
  virtual RooArgSet* addColumns(const RooArgList& varList) ;

  // Plot the distribution of a real valued arg
  using RooAbsData::createHistogram ;
  TH2F* createHistogram(const RooAbsRealLValue& var1, const RooAbsRealLValue& var2, const char* cuts="", 
			const char *name= "hist") const;	 
  TH2F* createHistogram(const RooAbsRealLValue& var1, const RooAbsRealLValue& var2, Int_t nx, Int_t ny,
                        const char* cuts="", const char *name="hist") const;

  void printMultiline(std::ostream& os, Int_t contents, Bool_t verbose=kFALSE, TString indent="") const ;
  virtual void printArgs(std::ostream& os) const ;
  virtual void printValue(std::ostream& os) const ;

  void SetName(const char *name) ;
  void SetNameTitle(const char *name, const char* title) ;

protected:

  virtual RooAbsData* cacheClone(const RooAbsArg* newCacheOwner, const RooArgSet* newCacheVars, const char* newName=0) ;

  friend class RooProdGenContext ;

  void initialize(const char* wgtVarName) ;
  
  // Cache copy feature is not publicly accessible
  RooAbsData* reduceEng(const RooArgSet& varSubset, const RooFormulaVar* cutVar, const char* cutRange=0, 
	                Int_t nStart=0, Int_t nStop=2000000000, Bool_t copyCache=kTRUE) ;
  RooDataSet(const char *name, const char *title, RooDataSet *ntuple, 
	     const RooArgSet& vars, const RooFormulaVar* cutVar, const char* cutRange, int nStart, int nStop, Bool_t copyCache, const char* wgtVarName=0);
  
  RooArgSet addWgtVar(const RooArgSet& origVars, const RooAbsArg* wgtVar) ; 
  
  RooArgSet _varsNoWgt ;   // Vars without weight variable 
  RooRealVar* _wgtVar ;    // Pointer to weight variable (if set) 

  static void cleanup() ;
  static char* _poolBegin ; //! Start of memory pool
  static char* _poolCur ;   //! Next free slot in memory pool
  static char* _poolEnd ;   //! End of memory pool  

  ClassDef(RooDataSet,2) // Unbinned data set
};

#endif
