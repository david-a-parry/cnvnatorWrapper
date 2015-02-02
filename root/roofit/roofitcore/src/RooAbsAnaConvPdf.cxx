/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 * @(#)root/roofitcore:$Id$
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

//////////////////////////////////////////////////////////////////////////////
// 
//  RooAbsAnaConvPdf is the base class of for PDFs that represents a
//  physics model that can be analytically convolved with a resolution model
//  
//  To achieve factorization between the physics model and the resolution
//  model, each physics model must be able to be written in the form
//           _ _                 _              _ 
//    Phys(x,a,b) = Sum_k coef_k(a) * basis_k(x,b)
//  
//  where basis_k are a limited number of functions in terms of the variable
//  to be convoluted and coef_k are coefficients independent of the convolution
//  variable.
//  
//  Classes derived from RooResolutionModel implement 
//         _ _                        _                  _
//   R_k(x,b,c) = Int(dx') basis_k(x',b) * resModel(x-x',c)
// 
//  which RooAbsAnaConvPdf uses to construct the pdf for [ Phys (x) R ] :
//          _ _ _                 _          _ _
//    PDF(x,a,b,c) = Sum_k coef_k(a) * R_k(x,b,c)
//
//  A minimal implementation of a RooAbsAnaConvPdf physics model consists of
//  
//  - A constructor that declares the required basis functions using the declareBasis() method.
//    The declareBasis() function assigns a unique identifier code to each declare basis
//
//  - An implementation of coefficient(Int_t code) returning the coefficient value for each
//    declared basis function
//
//  Optionally, analytical integrals can be provided for the coefficient functions. The
//  interface for this is quite similar to that for integrals of regular PDFs. Two functions,
//
//   Int_t getCoefAnalyticalIntegral(Int_t coef, RooArgSet& allVars, RooArgSet& analVars, const char* rangeName) const 
//   Double_t coefAnalyticalIntegral(Int_t coef, Int_t code, const char* rangeName) const 
//
//  advertise the coefficient integration capabilities and implement them respectively.
//  Please see RooAbsPdf for additional details. Advertised analytical integrals must be
//  valid for all coefficients.


#include "RooFit.h"
#include "RooMsgService.h"

#include "Riostream.h"
#include "Riostream.h"
#include "RooAbsAnaConvPdf.h"
#include "RooResolutionModel.h"
#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooConvGenContext.h"
#include "RooGenContext.h"
#include "RooTruthModel.h"
#include "RooConvCoefVar.h"
#include "RooNameReg.h"

using namespace std;

ClassImp(RooAbsAnaConvPdf) 
;


//_____________________________________________________________________________
RooAbsAnaConvPdf::RooAbsAnaConvPdf() :
  _isCopy(kFALSE),
  _convNormSet(0),
  _convSetIter(_convSet.createIterator())
{
  // Default constructor, required for persistence
}



//_____________________________________________________________________________
RooAbsAnaConvPdf::RooAbsAnaConvPdf(const char *name, const char *title, 
				   const RooResolutionModel& model, RooRealVar& cVar) :
  RooAbsPdf(name,title), _isCopy(kFALSE),
  _model("!model","Original resolution model",this,(RooResolutionModel&)model,kFALSE,kFALSE),
  _convVar("!convVar","Convolution variable",this,cVar,kFALSE,kFALSE),
  _convSet("!convSet","Set of resModel X basisFunc convolutions",this),
  _convNormSet(0), _convSetIter(_convSet.createIterator()),
  _coefNormMgr(this,10),
  _codeReg(10)
{
  // Constructor. The supplied resolution model must be constructed with the same
  // convoluted variable as this physics model ('convVar')

  _convNormSet = new RooArgSet(cVar,"convNormSet") ;
  _model.absArg()->setAttribute("NOCacheAndTrack") ;
}



//_____________________________________________________________________________
RooAbsAnaConvPdf::RooAbsAnaConvPdf(const RooAbsAnaConvPdf& other, const char* name) : 
  RooAbsPdf(other,name), _isCopy(kTRUE),
  _model("!model",this,other._model),
  _convVar("!convVar",this,other._convVar),
  _convSet("!convSet",this,other._convSet),
  _basisList(other._basisList),
  _convNormSet(other._convNormSet? new RooArgSet(*other._convNormSet) : new RooArgSet() ),
  _convSetIter(_convSet.createIterator()),
  _coefNormMgr(other._coefNormMgr,this),
  _codeReg(other._codeReg)
{
  // Copy constructor
  if (_model.absArg()) {
    _model.absArg()->setAttribute("NOCacheAndTrack") ;
  }
}



//_____________________________________________________________________________
RooAbsAnaConvPdf::~RooAbsAnaConvPdf()
{
  // Destructor

  if (_convNormSet) {
    delete _convNormSet ;
  }
    
  delete _convSetIter ;

  if (!_isCopy) {
    TIterator* iter = _convSet.createIterator() ;
    RooAbsArg* arg ;
    while (((arg = (RooAbsArg*)iter->Next()))) {
      _convSet.remove(*arg) ;
      delete arg ;
    }
    delete iter ;
  }

}


//_____________________________________________________________________________
Int_t RooAbsAnaConvPdf::declareBasis(const char* expression, const RooArgList& params) 
{
  // Declare a basis function for use in this physics model. The string expression 
  // must be a valid RooFormulVar expression representing the basis function, referring
  // to the convolution variable as '@0', and any additional parameters (supplied in
  // 'params' as '@1','@2' etc.
  //
  // The return value is a unique identifier code, that will be passed to coefficient()
  // to identify the basis function for which the coefficient is requested. If the
  // resolution model used does not support the declared basis function, code -1 is
  // returned. 
  //

  // Sanity check
  if (_isCopy) {
    coutE(InputArguments) << "RooAbsAnaConvPdf::declareBasis(" << GetName() << "): ERROR attempt to "
			  << " declare basis functions in a copied RooAbsAnaConvPdf" << endl ;
    return -1 ;
  }

  // Resolution model must support declared basis
  if (!((RooResolutionModel*)_model.absArg())->isBasisSupported(expression)) {
    coutE(InputArguments) << "RooAbsAnaConvPdf::declareBasis(" << GetName() << "): resolution model " 
			  << _model.absArg()->GetName() 
			  << " doesn't support basis function " << expression << endl ;
    return -1 ;
  }

  // Instantiate basis function
  RooArgList basisArgs(_convVar.arg()) ;
  basisArgs.add(params) ;

  TString basisName(expression) ;
  TIterator* iter = basisArgs.createIterator() ;
  RooAbsArg* arg  ;
  while(((arg=(RooAbsArg*)iter->Next()))) {
    basisName.Append("_") ;
    basisName.Append(arg->GetName()) ;
  }
  delete iter ;  

  RooFormulaVar* basisFunc = new RooFormulaVar(basisName,expression,basisArgs) ;
  basisFunc->setAttribute("RooWorkspace::Recycle") ;
  basisFunc->setAttribute("NOCacheAndTrack") ;
  basisFunc->setOperMode(operMode()) ;
  _basisList.addOwned(*basisFunc) ;

  // Instantiate resModel x basisFunc convolution
  RooAbsReal* conv = ((RooResolutionModel*)_model.absArg())->convolution(basisFunc,this) ;
  if (!conv) {
    coutE(InputArguments) << "RooAbsAnaConvPdf::declareBasis(" << GetName() << "): unable to construct convolution with basis function '" 
			  << expression << "'" << endl ;
    return -1 ;
  }
  _convSet.add(*conv) ;

  return _convSet.index(conv) ;
}



//_____________________________________________________________________________
Bool_t RooAbsAnaConvPdf::changeModel(const RooResolutionModel& newModel) 
{
  // Change the current resolution model to newModel

  TIterator* cIter = _convSet.createIterator() ;
  RooResolutionModel* conv ;
  RooArgList newConvSet ;
  Bool_t allOK(kTRUE) ;
  while(((conv=(RooResolutionModel*)cIter->Next()))) {

    // Build new resolution model
    RooResolutionModel* newConv = newModel.convolution((RooFormulaVar*)&conv->basis(),this) ;
    if (!newConvSet.add(*newConv)) {
      allOK = kFALSE ;
      break ;
    }
  }
  delete cIter ;

  // Check if all convolutions were successfully built
  if (!allOK) {
    // Delete new basis functions created sofar
    TIterator* iter = newConvSet.createIterator() ;
    while(((conv=(RooResolutionModel*)iter->Next()))) delete conv ;
    delete iter ;

    return kTRUE ;
  }
  
  // Replace old convolutions with new set
  _convSet.removeAll() ;
  _convSet.addOwned(newConvSet) ;

  _model.setArg((RooResolutionModel&)newModel) ;
  return kFALSE ;
}




//_____________________________________________________________________________
RooAbsGenContext* RooAbsAnaConvPdf::genContext(const RooArgSet &vars, const RooDataSet *prototype, 
					       const RooArgSet* auxProto, Bool_t verbose) const 
{
  // Create a generator context for this p.d.f. If both the p.d.f and the resolution model
  // support internal generation of the convolution observable on an infinite domain,
  // deploy a specialized convolution generator context, which generates the physics distribution
  // and the smearing separately, adding them a posteriori. If this is not possible return
  // a (slower) generic generation context that uses accept/reject sampling

  // Check if the resolution model specifies a special context to be used.
  RooResolutionModel* conv = dynamic_cast<RooResolutionModel*>(_model.absArg());
  assert(conv);

  RooArgSet* modelDep = _model.absArg()->getObservables(&vars) ;
  modelDep->remove(*convVar(),kTRUE,kTRUE) ;
  Int_t numAddDep = modelDep->getSize() ;
  delete modelDep ;

  // Check if physics PDF and resolution model can both directly generate the convolution variable
  RooArgSet dummy ;
  Bool_t pdfCanDir = (getGenerator(*convVar(),dummy) != 0) ;
  Bool_t resCanDir = conv && (conv->getGenerator(*convVar(),dummy)!=0) && conv->isDirectGenSafe(*convVar()) ;

  if (numAddDep>0 || !pdfCanDir || !resCanDir) {
    // Any resolution model with more dependents than the convolution variable
    // or pdf or resmodel do not support direct generation
    string reason ;
    if (numAddDep>0) reason += "Resolution model has more onservables that the convolution variable. " ;
    if (!pdfCanDir) reason += "PDF does not support internal generation of convolution observable. " ;
    if (!resCanDir) reason += "Resolution model does not support internal generation of convolution observable. " ;

    coutI(Generation) << "RooAbsAnaConvPdf::genContext(" << GetName() << ") Using regular accept/reject generator for convolution p.d.f because: " << reason.c_str() << endl ;    
    return new RooGenContext(*this,vars,prototype,auxProto,verbose) ;
  } 

  RooAbsGenContext* context = conv->modelGenContext(*this, vars, prototype, auxProto, verbose);
  if (context) return context;
  
  // Any other resolution model: use specialized generator context
  return new RooConvGenContext(*this,vars,prototype,auxProto,verbose) ;
}



//_____________________________________________________________________________
Bool_t RooAbsAnaConvPdf::isDirectGenSafe(const RooAbsArg& arg) const 
{
  // Return true if it is safe to generate the convolution observable 
  // from the internal generator (this is the case if the chosen resolution
  // model is the truth model)


  // All direct generation of convolution arg if model is truth model
  if (!TString(_convVar.absArg()->GetName()).CompareTo(arg.GetName()) && 
      dynamic_cast<RooTruthModel*>(_model.absArg())) {
    return kTRUE ;
  }

  return RooAbsPdf::isDirectGenSafe(arg) ;
}



//_____________________________________________________________________________
const RooRealVar* RooAbsAnaConvPdf::convVar() const
{
  // Return a pointer to the convolution variable instance used in the resolution model

  RooResolutionModel* conv = (RooResolutionModel*) _convSet.at(0) ;
  if (!conv) return 0 ;  
  return &conv->convVar() ;
}



//_____________________________________________________________________________
Double_t RooAbsAnaConvPdf::evaluate() const
{
  // Calculate the current unnormalized value of the PDF
  //
  // PDF = sum_k coef_k * [ basis_k (x) ResModel ]
  //
  Double_t result(0) ;

  _convSetIter->Reset() ;
  RooAbsPdf* conv ;
  Int_t index(0) ;
  while(((conv=(RooAbsPdf*)_convSetIter->Next()))) {
    Double_t coef = coefficient(index++) ;
    if (coef!=0.) {
      Double_t c = conv->getVal(0) ;
      Double_t r = coef ;
      cxcoutD(Eval) << "RooAbsAnaConvPdf::evaluate(" << GetName() << ") val += coef*conv [" << index-1 << "/" 
		    << _convSet.getSize() << "] coef = " << r << " conv = " << c << endl ;
      result += conv->getVal(0)*coef ;
    } else {
      cxcoutD(Eval) << "RooAbsAnaConvPdf::evaluate(" << GetName() << ") [" << index-1 << "/" << _convSet.getSize() << "] coef = 0" << endl ;
    }
  }
  
  return result ;
}



//_____________________________________________________________________________
Int_t RooAbsAnaConvPdf::getAnalyticalIntegralWN(RooArgSet& allVars, 
	  				        RooArgSet& analVars, const RooArgSet* normSet2, const char* /*rangeName*/) const 
{
  // Advertise capability to perform (analytical) integrals
  // internally. For a given integration request over allVars while
  // normalized over normSet2 and in range 'rangeName', returns
  // largest subset that can be performed internally in analVars
  // Return code is unique integer code identifying integration scenario
  // to be passed to analyticalIntegralWN() to calculate requeste integral
  //
  // Class RooAbsAnaConv defers analytical integration request to
  // resolution model and/or coefficient implementations and
  // aggregates results into composite configuration with a unique
  // code assigned by RooAICRegistry

  // Handle trivial no-integration scenario
  if (allVars.getSize()==0) return 0 ;
  
  if (_forceNumInt) return 0 ;

  // Select subset of allVars that are actual dependents
  RooArgSet* allDeps = getObservables(allVars) ;
  RooArgSet* normSet = normSet2 ? getObservables(normSet2) : 0 ;

  RooAbsArg *arg ;
  RooResolutionModel *conv ;

  RooArgSet* intSetAll = new RooArgSet(*allDeps,"intSetAll") ;

  // Split intSetAll in coef/conv parts
  RooArgSet* intCoefSet = new RooArgSet("intCoefSet") ; 
  RooArgSet* intConvSet = new RooArgSet("intConvSet") ;
  TIterator* varIter  = intSetAll->createIterator() ;
  TIterator* convIter = _convSet.createIterator() ;

  while(((arg=(RooAbsArg*) varIter->Next()))) {
    Bool_t ok(kTRUE) ;
    convIter->Reset() ;
    while(((conv=(RooResolutionModel*) convIter->Next()))) {
      if (conv->dependsOn(*arg)) ok=kFALSE ;
    }
    
    if (ok) {
      intCoefSet->add(*arg) ;
    } else {
      intConvSet->add(*arg) ;
    }
    
  }
  delete varIter ;


  // Split normSetAll in coef/conv parts
  RooArgSet* normCoefSet = new RooArgSet("normCoefSet") ;  
  RooArgSet* normConvSet = new RooArgSet("normConvSet") ;
  RooArgSet* normSetAll = normSet ? (new RooArgSet(*normSet,"normSetAll")) : 0 ;
  if (normSetAll) {
    varIter  =  normSetAll->createIterator() ;
    while(((arg=(RooAbsArg*) varIter->Next()))) {
      Bool_t ok(kTRUE) ;
      convIter->Reset() ;
      while(((conv=(RooResolutionModel*) convIter->Next()))) {
	if (conv->dependsOn(*arg)) ok=kFALSE ;
      }
      
      if (ok) {
	normCoefSet->add(*arg) ;
      } else {
	normConvSet->add(*arg) ;
      }
      
    }
    delete varIter ;
  }
  delete convIter ;

  if (intCoefSet->getSize()==0) {
    delete intCoefSet ; intCoefSet=0 ;
  }
  if (intConvSet->getSize()==0) {
    delete intConvSet ; intConvSet=0 ;
  }
  if (normCoefSet->getSize()==0) {
    delete normCoefSet ; normCoefSet=0 ;
  }
  if (normConvSet->getSize()==0) {
    delete normConvSet ; normConvSet=0 ;
  }



  // Store integration configuration in registry
  Int_t masterCode(0) ;
  std::vector<Int_t> tmp(1, 0) ;

  masterCode = _codeReg.store(tmp, intCoefSet, intConvSet, normCoefSet, normConvSet) + 1 ; // takes ownership of all sets

  analVars.add(*allDeps) ;
  delete allDeps ;
  if (normSet) delete normSet ;
  if (normSetAll) delete normSetAll ;
  delete intSetAll ;

//   cout << this << "---> masterCode = " << masterCode << endl ;
  
  return masterCode  ;
}




//_____________________________________________________________________________
Double_t RooAbsAnaConvPdf::analyticalIntegralWN(Int_t code, const RooArgSet* normSet, const char* rangeName) const 
{
  // Return analytical integral defined by given code, which is returned
  // by getAnalyticalIntegralWN()
  //
  // For unnormalized integrals the returned value is
  //                    _                _     
  //   PDF = sum_k Int(dx) coef_k * Int(dy) [ basis_k (x) ResModel ].
  //       _
  // where x is the set of coefficient dependents to be integrated
  // and y the set of basis function dependents to be integrated. 
  //
  // For normalized integrals this becomes
  //
  //         sum_k Int(dx) coef_k * Int(dy) [ basis_k (x) ResModel ].
  //  PDF =  --------------------------------------------------------
  //         sum_k Int(dv) coef_k * Int(dw) [ basis_k (x) ResModel ].
  //
  // where x is the set of coefficient dependents to be integrated,
  // y the set of basis function dependents to be integrated,
  // v is the set of coefficient dependents over which is normalized and
  // w is the set of basis function dependents over which is normalized.
  //
  // Set x must be contained in v and set y must be contained in w.
  //

  // WVE needs adaptation to handle new rangeName feature

  // Handle trivial passthrough scenario
  if (code==0) return getVal(normSet) ;

  // Unpack master code
  RooArgSet *intCoefSet, *intConvSet, *normCoefSet, *normConvSet ;
  _codeReg.retrieve(code-1,intCoefSet,intConvSet,normCoefSet,normConvSet) ;

  RooResolutionModel* conv ;
  Int_t index(0) ;
  Double_t answer(0) ;
  _convSetIter->Reset() ;

  if (normCoefSet==0&&normConvSet==0) {

    // Integral over unnormalized function
    Double_t integral(0) ;
    const TNamed *_rangeName = RooNameReg::ptr(rangeName);
    while(((conv=(RooResolutionModel*)_convSetIter->Next()))) {
      Double_t coef = getCoefNorm(index++,intCoefSet,_rangeName) ; 
      //cout << "coefInt[" << index << "] = " << coef << " " ; intCoefSet->Print("1") ; 
      if (coef!=0) {
	integral += coef*(_rangeName ? conv->getNormObj(0,intConvSet,_rangeName)->getVal() :  conv->getNorm(intConvSet) ) ;       
	cxcoutD(Eval) << "RooAbsAnaConv::aiWN(" << GetName() << ") [" << index-1 << "] integral += " << conv->getNorm(intConvSet) << endl ;
      }

    }
    answer = integral ;
    
  } else {

    // Integral over normalized function
    Double_t integral(0) ;
    Double_t norm(0) ;
    const TNamed *_rangeName = RooNameReg::ptr(rangeName);
    while(((conv=(RooResolutionModel*)_convSetIter->Next()))) {

      Double_t coefInt = getCoefNorm(index,intCoefSet,_rangeName) ;
      //cout << "coefInt[" << index << "] = " << coefInt << "*" << term << " " << (intCoefSet?*intCoefSet:RooArgSet()) << endl ;
      if (coefInt!=0) {
	Double_t term = (_rangeName ? conv->getNormObj(0,intConvSet,_rangeName)->getVal() : conv->getNorm(intConvSet) ) ;
	integral += coefInt*term ;
      }

      Double_t coefNorm = getCoefNorm(index,normCoefSet) ;
      //cout << "coefNorm[" << index << "] = " << coefNorm << "*" << term << " " << (normCoefSet?*normCoefSet:RooArgSet()) << endl ;
      if (coefNorm!=0) {
	Double_t term = conv->getNorm(normConvSet) ;
	norm += coefNorm*term ;
      }

      index++ ;
    }
    answer = integral/norm ;    
  }

  return answer ;
}



//_____________________________________________________________________________
Int_t RooAbsAnaConvPdf::getCoefAnalyticalIntegral(Int_t /* coef*/, RooArgSet& /*allVars*/, RooArgSet& /*analVars*/, const char* /*rangeName*/) const 
{
  // Default implementation of function advertising integration capabilities. The interface is
  // similar to that of getAnalyticalIntegral except that an integer code is added that 
  // designates the coefficient number for which the integration capabilities are requested
  //
  // This default implementation advertises that no internal integrals are supported.

  return 0 ;
}



//_____________________________________________________________________________
Double_t RooAbsAnaConvPdf::coefAnalyticalIntegral(Int_t coef, Int_t code, const char* /*rangeName*/) const 
{
  // Default implementation of function implementing advertised integrals. Only
  // the pass-through scenario (no integration) is implemented.

  if (code==0) return coefficient(coef) ;
  coutE(InputArguments) << "RooAbsAnaConvPdf::coefAnalyticalIntegral(" << GetName() << ") ERROR: unrecognized integration code: " << code << endl ;
  assert(0) ;
  return 1 ;
}



//_____________________________________________________________________________
Bool_t RooAbsAnaConvPdf::forceAnalyticalInt(const RooAbsArg& /*dep*/) const
{
  // This function forces RooRealIntegral to offer all integration dependents
  // to RooAbsAnaConvPdf::getAnalyticalIntegralWN() for consideration for
  // internal integration, if RooRealIntegral considers this to be unsafe (e.g. due
  // to hidden Jacobian terms).
  //
  // RooAbsAnaConvPdf will not attempt to actually integrate all these dependents
  // but feed them to the resolution models integration interface, which will
  // make the final determination on how to integrate these dependents.

  return kTRUE ;
}                                                                                                                         
               


//_____________________________________________________________________________
Double_t RooAbsAnaConvPdf::getCoefNorm(Int_t coefIdx, const RooArgSet* nset, const TNamed* rangeName) const 
{
  // Returns the normalization integral value of the coefficient with number coefIdx over normalization
  // set nset in range rangeName

  if (nset==0) return coefficient(coefIdx) ;

  CacheElem* cache = (CacheElem*) _coefNormMgr.getObj(nset,0,0,rangeName) ;
  if (!cache) {

    cache = new CacheElem ;

    // Make list of coefficient normalizations
    Int_t i ;
    makeCoefVarList(cache->_coefVarList) ;  

    for (i=0 ; i<cache->_coefVarList.getSize() ; i++) {
      RooAbsReal* coefInt = static_cast<RooAbsReal&>(*cache->_coefVarList.at(i)).createIntegral(*nset,RooNameReg::str(rangeName)) ;
      cache->_normList.addOwned(*coefInt) ;      
    }  

    _coefNormMgr.setObj(nset,0,cache,rangeName) ;
  }

  return ((RooAbsReal*)cache->_normList.at(coefIdx))->getVal() ;
}



//_____________________________________________________________________________
void RooAbsAnaConvPdf::makeCoefVarList(RooArgList& varList) const
{
  // Build complete list of coefficient variables

  // Instantate a coefficient variables
  for (Int_t i=0 ; i<_convSet.getSize() ; i++) {
    RooArgSet* cvars = coefVars(i) ;
    RooAbsReal* coefVar = new RooConvCoefVar(Form("%s_coefVar_%d",GetName(),i),"coefVar",*this,i,cvars) ;
    varList.addOwned(*coefVar) ;
    delete cvars ;
  }
  
}


//_____________________________________________________________________________
RooArgSet* RooAbsAnaConvPdf::coefVars(Int_t /*coefIdx*/) const 
{
  // Return set of parameters with are used exclusively by the coefficient functions

  RooArgSet* cVars = getParameters((RooArgSet*)0) ;
  TIterator* iter = cVars->createIterator() ;
  RooAbsArg* arg ;
  Int_t i ;
  while(((arg=(RooAbsArg*)iter->Next()))) {
    for (i=0 ; i<_convSet.getSize() ; i++) {
      if (_convSet.at(i)->dependsOn(*arg)) {
	cVars->remove(*arg,kTRUE) ;
      }
    }
  }
  delete iter ;  
  return cVars ;
}




//_____________________________________________________________________________
void RooAbsAnaConvPdf::printMultiline(ostream& os, Int_t contents, Bool_t verbose, TString indent) const 
{
  // Print info about this object to the specified stream. In addition to the info
  // from RooAbsPdf::printStream() we add:
  //
  //   Verbose : detailed information on convolution integrals

  RooAbsPdf::printMultiline(os,contents,verbose,indent);

  os << indent << "--- RooAbsAnaConvPdf ---" << endl;
  TIterator* iter = _convSet.createIterator() ;
  RooResolutionModel* conv ;
  while (((conv=(RooResolutionModel*)iter->Next()))) {
    conv->printMultiline(os,contents,verbose,indent) ;
  }
}


