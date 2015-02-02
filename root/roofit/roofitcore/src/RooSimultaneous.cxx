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
// BEGIN_HTML
// RooSimultaneous facilitates simultaneous fitting of multiple PDFs
// to subsets of a given dataset.
// <p>
// The class takes an index category, which is interpreted as
// the data subset indicator, and a list of PDFs, each associated
// with a state of the index category. RooSimultaneous always returns
// the value of the PDF that is associated with the current value
// of the index category
// <p>
// Extended likelihood fitting is supported if all components support
// extended likelihood mode. The expected number of events by a RooSimultaneous
// is that of the component p.d.f. selected by the index category
// END_HTML
//

#include "RooFit.h"
#include "Riostream.h"

#include "TObjString.h"
#include "RooSimultaneous.h"
#include "RooAbsCategoryLValue.h"
#include "RooPlot.h"
#include "RooCurve.h"
#include "RooRealVar.h"
#include "RooAddPdf.h"
#include "RooAbsData.h"
#include "Roo1DTable.h"
#include "RooSimGenContext.h"
#include "RooSimSplitGenContext.h"
#include "RooDataSet.h"
#include "RooCmdConfig.h"
#include "RooNameReg.h"
#include "RooGlobalFunc.h"
#include "RooNameReg.h"
#include "RooMsgService.h"
#include "RooCategory.h"
#include "RooSuperCategory.h"
#include "RooDataHist.h"
#include "RooRandom.h"
#include "RooArgSet.h"

using namespace std ;

ClassImp(RooSimultaneous)
;




//_____________________________________________________________________________
RooSimultaneous::RooSimultaneous(const char *name, const char *title, 
				 RooAbsCategoryLValue& inIndexCat) : 
  RooAbsPdf(name,title), 
  _plotCoefNormSet("!plotCoefNormSet","plotCoefNormSet",this,kFALSE,kFALSE),
  _plotCoefNormRange(0),
  _partIntMgr(this,10),
  _indexCat("indexCat","Index category",this,inIndexCat),
  _numPdf(0)
{
  // Constructor with index category. PDFs associated with indexCat
  // states can be added after construction with the addPdf() function.
  // 
  // RooSimultaneous can function without having a PDF associated
  // with every single state. The normalization in such cases is taken
  // from the number of registered PDFs, but getVal() will assert if
  // when called for an unregistered index state.
}



//_____________________________________________________________________________
RooSimultaneous::RooSimultaneous(const char *name, const char *title, 
				 const RooArgList& inPdfList, RooAbsCategoryLValue& inIndexCat) :
  RooAbsPdf(name,title), 
  _plotCoefNormSet("!plotCoefNormSet","plotCoefNormSet",this,kFALSE,kFALSE),
  _plotCoefNormRange(0),
  _partIntMgr(this,10),
  _indexCat("indexCat","Index category",this,inIndexCat),
  _numPdf(0)
{
  // Constructor from index category and full list of PDFs. 
  // In this constructor form, a PDF must be supplied for each indexCat state
  // to avoid ambiguities. The PDFS are associated in order with the state of the
  // index category as listed by the index categories type iterator.
  //
  // PDFs may not overlap (i.e. share any variables) with the index category (function)

  if (inPdfList.getSize() != inIndexCat.numTypes()) {
    coutE(InputArguments) << "RooSimultaneous::ctor(" << GetName() 
			  << " ERROR: Number PDF list entries must match number of index category states, no PDFs added" << endl ;
    return ;
  }

  map<string,RooAbsPdf*> pdfMap ;
  // Iterator over PDFs and index cat states and add each pair
  TIterator* pIter = inPdfList.createIterator() ;
  TIterator* cIter = inIndexCat.typeIterator() ;
  RooAbsPdf* pdf ;
  RooCatType* type(0) ;
  while ((pdf=(RooAbsPdf*)pIter->Next())) {
    type = (RooCatType*) cIter->Next() ;
    pdfMap[string(type->GetName())] = pdf ;
  }
  delete pIter ;
  delete cIter ;

  initialize(inIndexCat,pdfMap) ;
}


//_____________________________________________________________________________
RooSimultaneous::RooSimultaneous(const char *name, const char *title, 
				 map<string,RooAbsPdf*> pdfMap, RooAbsCategoryLValue& inIndexCat) :
  RooAbsPdf(name,title), 
  _plotCoefNormSet("!plotCoefNormSet","plotCoefNormSet",this,kFALSE,kFALSE),
  _plotCoefNormRange(0),
  _partIntMgr(this,10),
  _indexCat("indexCat","Index category",this,inIndexCat),
  _numPdf(0)
{
  initialize(inIndexCat,pdfMap) ;
}




// This class cannot be locally defined in initialize as it cannot be
// used as a template argument in that case
namespace RooSimultaneousAux {
  struct CompInfo {
    RooAbsPdf* pdf ;
    RooSimultaneous* simPdf ;
    const RooAbsCategoryLValue* subIndex ;
    RooArgSet* subIndexComps ;
  } ;
}

void RooSimultaneous::initialize(RooAbsCategoryLValue& inIndexCat, std::map<std::string,RooAbsPdf*> pdfMap) 
{
  // First see if there are any RooSimultaneous input components
  Bool_t simComps(kFALSE) ;
  for (map<string,RooAbsPdf*>::iterator iter=pdfMap.begin() ; iter!=pdfMap.end() ; iter++) {    
    if (dynamic_cast<RooSimultaneous*>(iter->second)) {
      simComps = kTRUE ;
      break ;
    }
  }

  // If there are no simultaneous component p.d.f. do simple processing through addPdf()
  if (!simComps) {
    for (map<string,RooAbsPdf*>::iterator iter=pdfMap.begin() ; iter!=pdfMap.end() ; iter++) {    
      addPdf(*iter->second,iter->first.c_str()) ;
    }
    return ;
  }

  // Issue info message that we are about to do some rearraning
  coutI(InputArguments) << "RooSimultaneous::initialize(" << GetName() << ") INFO: one or more input component of simultaneous p.d.f.s are"
			<< " simultaneous p.d.f.s themselves, rewriting composite expressions as one-level simultaneous p.d.f. in terms of"
			<< " final constituents and extended index category" << endl ;


  RooArgSet allAuxCats ;
  map<string,RooSimultaneousAux::CompInfo> compMap ;
  for (map<string,RooAbsPdf*>::iterator iter=pdfMap.begin() ; iter!=pdfMap.end() ; iter++) {    
    RooSimultaneousAux::CompInfo ci ;
    ci.pdf = iter->second ;
    RooSimultaneous* simComp = dynamic_cast<RooSimultaneous*>(iter->second) ;
    if (simComp) {
      ci.simPdf = simComp ;
      ci.subIndex = &simComp->indexCat() ;      
      ci.subIndexComps = simComp->indexCat().isFundamental() ? new RooArgSet(simComp->indexCat()) : simComp->indexCat().getVariables() ;
      allAuxCats.add(*(ci.subIndexComps),kTRUE) ;
    } else {
      ci.simPdf = 0 ;
      ci.subIndex = 0 ;
      ci.subIndexComps = 0 ;
    }
    compMap[iter->first] = ci ;
  }

  // Construct the 'superIndex' from the nominal index category and all auxiliary components
  RooArgSet allCats(inIndexCat) ;
  allCats.add(allAuxCats) ;
  string siname = Form("%s_index",GetName()) ;
  RooSuperCategory* superIndex = new RooSuperCategory(siname.c_str(),siname.c_str(),allCats) ;
  
  // Now process each of original pdf/state map entries
  for (map<string,RooSimultaneousAux::CompInfo>::iterator citer = compMap.begin() ; citer != compMap.end() ; citer++) {

    RooArgSet repliCats(allAuxCats) ;
    if (citer->second.subIndexComps) {
      repliCats.remove(*citer->second.subIndexComps) ;
      delete citer->second.subIndexComps ;
    }
    inIndexCat.setLabel(citer->first.c_str()) ;
    
       
    if (!citer->second.simPdf) {

      // Entry is a plain p.d.f. assign it to every state permutation of the repliCats set
      RooSuperCategory repliSuperCat("tmp","tmp",repliCats) ;

      // Iterator over all states of repliSuperCat
      TIterator* titer = repliSuperCat.typeIterator() ;
      RooCatType* type ;
      while ((type=(RooCatType*)titer->Next())) {
	// Set value 
	repliSuperCat.setLabel(type->GetName()) ;
	// Retrieve corresponding label of superIndex 
	string superLabel = superIndex->getLabel() ;
	addPdf(*citer->second.pdf,superLabel.c_str()) ;
	cxcoutD(InputArguments) << "RooSimultaneous::initialize(" << GetName() 
				<< ") assigning pdf " << citer->second.pdf->GetName() << " to super label " << superLabel << endl ;
      }
    } else {

      // Entry is a simultaneous p.d.f

      if (repliCats.getSize()==0) {

	// Case 1 -- No replication of components of RooSim component are required

	TIterator* titer = citer->second.subIndex->typeIterator() ;
	RooCatType* type ;
	while ((type=(RooCatType*)titer->Next())) {
	  const_cast<RooAbsCategoryLValue*>(citer->second.subIndex)->setLabel(type->GetName()) ;
	  string superLabel = superIndex->getLabel() ;
	  RooAbsPdf* compPdf = citer->second.simPdf->getPdf(type->GetName()) ;
	  if (compPdf) {
	    addPdf(*compPdf,superLabel.c_str()) ;
	    cxcoutD(InputArguments) << "RooSimultaneous::initialize(" << GetName() 
				    << ") assigning pdf " << compPdf->GetName() << "(member of " << citer->second.pdf->GetName() 
				    << ") to super label " << superLabel << endl ;	  
	  } else {
	    coutW(InputArguments) << "RooSimultaneous::initialize(" << GetName() << ") WARNING: No p.d.f. associated with label " 
				  << type->GetName() << " for component RooSimultaneous p.d.f " << citer->second.pdf->GetName() 
				  << "which is associated with master index label " << citer->first << endl ;	    
	  }		
	}
	delete titer ;

      } else {

	// Case 2 -- Replication of components of RooSim component are required

	// Make replication supercat
	RooSuperCategory repliSuperCat("tmp","tmp",repliCats) ;
	TIterator* triter = repliSuperCat.typeIterator() ;

	TIterator* tsiter = citer->second.subIndex->typeIterator() ;
	RooCatType* stype, *rtype ;
	while ((stype=(RooCatType*)tsiter->Next())) {
	  const_cast<RooAbsCategoryLValue*>(citer->second.subIndex)->setLabel(stype->GetName()) ;
	  triter->Reset() ;
	  while ((rtype=(RooCatType*)triter->Next())) {
	    repliSuperCat.setLabel(rtype->GetName()) ;
	    string superLabel = superIndex->getLabel() ;
	    RooAbsPdf* compPdf = citer->second.simPdf->getPdf(stype->GetName()) ;
	    if (compPdf) {
	      addPdf(*compPdf,superLabel.c_str()) ;
	      cxcoutD(InputArguments) << "RooSimultaneous::initialize(" << GetName() 
				      << ") assigning pdf " << compPdf->GetName() << "(member of " << citer->second.pdf->GetName() 
				      << ") to super label " << superLabel << endl ;	  
	    } else {
	      coutW(InputArguments) << "RooSimultaneous::initialize(" << GetName() << ") WARNING: No p.d.f. associated with label " 
				    << stype->GetName() << " for component RooSimultaneous p.d.f " << citer->second.pdf->GetName() 
				    << "which is associated with master index label " << citer->first << endl ;	    
	    }		
	  }
	}

	delete tsiter ;
	delete triter ;
	
      }
    }
  }

  // Change original master index to super index and take ownership of it
  _indexCat.setArg(*superIndex) ;
  addOwnedComponents(*superIndex) ;

}



//_____________________________________________________________________________
RooSimultaneous::RooSimultaneous(const RooSimultaneous& other, const char* name) : 
  RooAbsPdf(other,name),
  _plotCoefNormSet("!plotCoefNormSet",this,other._plotCoefNormSet),
  _plotCoefNormRange(other._plotCoefNormRange),
  _partIntMgr(other._partIntMgr,this),
  _indexCat("indexCat",this,other._indexCat), 
  _numPdf(other._numPdf)
{
  // Copy constructor

  // Copy proxy list 
  TIterator* pIter = other._pdfProxyList.MakeIterator() ;
  RooRealProxy* proxy ;
  while ((proxy=(RooRealProxy*)pIter->Next())) {
    _pdfProxyList.Add(new RooRealProxy(proxy->GetName(),this,*proxy)) ;
  }
  delete pIter ;
}



//_____________________________________________________________________________
RooSimultaneous::~RooSimultaneous() 
{
  // Destructor

  _pdfProxyList.Delete() ;
}



//_____________________________________________________________________________
RooAbsPdf* RooSimultaneous::getPdf(const char* catName) const 
{
  // Return the p.d.f associated with the given index category name
  
  RooRealProxy* proxy = (RooRealProxy*) _pdfProxyList.FindObject(catName) ;
  return proxy ? ((RooAbsPdf*)proxy->absArg()) : 0 ;
}



//_____________________________________________________________________________
Bool_t RooSimultaneous::addPdf(const RooAbsPdf& pdf, const char* catLabel)
{
  // Associate given PDF with index category state label 'catLabel'.
  // The names state must be already defined in the index category
  //
  // RooSimultaneous can function without having a PDF associated
  // with every single state. The normalization in such cases is taken
  // from the number of registered PDFs, but getVal() will assert if
  // when called for an unregistered index state.
  //
  // PDFs may not overlap (i.e. share any variables) with the index category (function)

  // PDFs cannot overlap with the index category
  if (pdf.dependsOn(_indexCat.arg())) {
    coutE(InputArguments) << "RooSimultaneous::addPdf(" << GetName() << "): ERROR, PDF " << pdf.GetName() 
			  << " overlaps with index category " << _indexCat.arg().GetName() << endl ;
    return kTRUE ;
  }

  // Each index state can only have one PDF associated with it
  if (_pdfProxyList.FindObject(catLabel)) {
    coutE(InputArguments) << "RooSimultaneous::addPdf(" << GetName() << "): ERROR, index state " 
			  << catLabel << " has already an associated PDF" << endl ;
    return kTRUE ;
  }

  const RooSimultaneous* simPdf = dynamic_cast<const RooSimultaneous*>(&pdf) ;
  if (simPdf) {

    coutE(InputArguments) << "RooSimultaneous::addPdf(" << GetName() 
			  << ") ERROR: you cannot add a RooSimultaneous component to a RooSimultaneous using addPdf()." 
			  << " Use the constructor with RooArgList if input p.d.f.s or the map<string,RooAbsPdf&> instead." << endl ;
    return kTRUE ;

  } else {

    // Create a proxy named after the associated index state
    TObject* proxy = new RooRealProxy(catLabel,catLabel,this,(RooAbsPdf&)pdf) ;
    _pdfProxyList.Add(proxy) ;
    _numPdf += 1 ;
  }

  return kFALSE ;
}





//_____________________________________________________________________________
RooAbsPdf::ExtendMode RooSimultaneous::extendMode() const 
{ 
  // WVE NEEDS FIX
  Bool_t allCanExtend(kTRUE) ;
  Bool_t anyMustExtend(kFALSE) ;

  for (Int_t i=0 ; i<_numPdf ; i++) {
    RooRealProxy* proxy = (RooRealProxy*) _pdfProxyList.FindObject(_indexCat.label()) ;
    if (proxy) {
//       cout << " now processing pdf " << pdf->GetName() << endl ;
      RooAbsPdf* pdf = (RooAbsPdf*) proxy->absArg() ;
      if (!pdf->canBeExtended()) {
// 	cout << "RooSim::extendedMode(" << GetName() << ") component " << pdf->GetName() << " cannot be extended" << endl ;
	allCanExtend=kFALSE ;
      }
      if (pdf->mustBeExtended()) {
	anyMustExtend=kTRUE;
      }
    }
  }
  if (anyMustExtend) {
//     cout << "RooSim::extendedMode(" << GetName() << ") returning MustBeExtended" << endl ;
    return MustBeExtended ;
  }
  if (allCanExtend) {
//     cout << "RooSim::extendedMode(" << GetName() << ") returning CanBeExtended" << endl ;
    return CanBeExtended ;
  }
//   cout << "RooSim::extendedMode(" << GetName() << ") returning CanNotBeExtended" << endl ;
  return CanNotBeExtended ; 
}




//_____________________________________________________________________________
Double_t RooSimultaneous::evaluate() const
{  
  // Return the current value: 
  // the value of the PDF associated with the current index category state

  // Retrieve the proxy by index name
  RooRealProxy* proxy = (RooRealProxy*) _pdfProxyList.FindObject(_indexCat.label()) ;
  
  //assert(proxy!=0) ;
  if (proxy==0) return 0 ;

  // Calculate relative weighting factor for sim-pdfs of all extendable components
  Double_t catFrac(1) ;
  if (canBeExtended()) {
    Double_t nEvtCat = ((RooAbsPdf*)(proxy->absArg()))->expectedEvents(_normSet) ; 
    
    Double_t nEvtTot(0) ;
    TIterator* iter = _pdfProxyList.MakeIterator() ;
    RooRealProxy* proxy2 ;
    while((proxy2=(RooRealProxy*)iter->Next())) {      
      nEvtTot += ((RooAbsPdf*)(proxy2->absArg()))->expectedEvents(_normSet) ;
    }
    delete iter ;
    catFrac=nEvtCat/nEvtTot ;
  }

  // Return the selected PDF value, normalized by the number of index states  
  return ((RooAbsPdf*)(proxy->absArg()))->getVal(_normSet)*catFrac ; 
}



//_____________________________________________________________________________
Double_t RooSimultaneous::expectedEvents(const RooArgSet* nset) const 
{
  // Return the number of expected events: If the index is in nset,
  // then return the sum of the expected events of all components,
  // otherwise return the number of expected events of the PDF
  // associated with the current index category state

  if (nset->contains(_indexCat.arg())) {

    Double_t sum(0) ;

    TIterator* iter = _pdfProxyList.MakeIterator() ;
    RooRealProxy* proxy ;
    while((proxy=(RooRealProxy*)iter->Next())) {      
      sum += ((RooAbsPdf*)(proxy->absArg()))->expectedEvents(nset) ;
    }
    delete iter ;

    return sum ;
    
  } else {

    // Retrieve the proxy by index name
    RooRealProxy* proxy = (RooRealProxy*) _pdfProxyList.FindObject(_indexCat.label()) ;
    
    //assert(proxy!=0) ;
    if (proxy==0) return 0 ;

    // Return the selected PDF value, normalized by the number of index states
    return ((RooAbsPdf*)(proxy->absArg()))->expectedEvents(nset); 
  }
}



//_____________________________________________________________________________
Int_t RooSimultaneous::getAnalyticalIntegralWN(RooArgSet& allVars, RooArgSet& analVars, 
					       const RooArgSet* normSet, const char* rangeName) const 
{
  // Forward determination of analytical integration capabilities to component p.d.f.s
  // A unique code is assigned to the combined integration capabilities of all associated
  // p.d.f.s
  
  // Declare that we can analytically integrate all requested observables
  analVars.add(allVars) ;

  // Retrieve (or create) the required partial integral list
  Int_t code ;

  // Check if this configuration was created before
  CacheElem* cache = (CacheElem*) _partIntMgr.getObj(normSet,&analVars,0,RooNameReg::ptr(rangeName)) ;
  if (cache) {
    code = _partIntMgr.lastIndex() ;
    return code+1 ;
  }
  cache = new CacheElem ;

  // Create the partial integral set for this request
  TIterator* iter = _pdfProxyList.MakeIterator() ;
  RooRealProxy* proxy ;
  while((proxy=(RooRealProxy*)iter->Next())) {
    RooAbsReal* pdfInt = proxy->arg().createIntegral(analVars,normSet,0,rangeName) ;
    cache->_partIntList.addOwned(*pdfInt) ;
  }
  delete iter ;

  // Store the partial integral list and return the assigned code ;
  code = _partIntMgr.setObj(normSet,&analVars,cache,RooNameReg::ptr(rangeName)) ;
  
  return code+1 ;
}



//_____________________________________________________________________________
Double_t RooSimultaneous::analyticalIntegralWN(Int_t code, const RooArgSet* normSet, const char* /*rangeName*/) const 
{
  // Return analytical integration defined by given code

  // No integration scenario
  if (code==0) {
    return getVal(normSet) ;
  }

  // Partial integration scenarios, rangeName already encoded in 'code'
  CacheElem* cache = (CacheElem*) _partIntMgr.getObjByIndex(code-1) ;

  RooRealProxy* proxy = (RooRealProxy*) _pdfProxyList.FindObject(_indexCat.label()) ;
  Int_t idx = _pdfProxyList.IndexOf(proxy) ;
  return ((RooAbsReal*)cache->_partIntList.at(idx))->getVal(normSet) ;
}






//_____________________________________________________________________________
RooPlot* RooSimultaneous::plotOn(RooPlot *frame, RooLinkedList& cmdList) const
{
  // Back-end for plotOn() implementation on RooSimultaneous which
  // needs special handling because a RooSimultaneous PDF cannot
  // project out its index category via integration, plotOn() will
  // abort if this is requested without providing a projection dataset

  // Sanity checks
  if (plotSanityChecks(frame)) return frame ;
  
  // Extract projection configuration from command list
  RooCmdConfig pc(Form("RooSimultaneous::plotOn(%s)",GetName())) ;
  pc.defineString("sliceCatState","SliceCat",0,"",kTRUE) ;
  pc.defineDouble("scaleFactor","Normalization",0,1.0) ;
  pc.defineInt("scaleType","Normalization",0,RooAbsPdf::Relative) ;
  pc.defineObject("sliceCatList","SliceCat",0,0,kTRUE) ;
  pc.defineObject("projSet","Project",0) ;
  pc.defineObject("sliceSet","SliceVars",0) ;
  pc.defineObject("projDataSet","ProjData",0) ;
  pc.defineObject("projData","ProjData",1) ;
  pc.defineMutex("Project","SliceVars") ;
  pc.allowUndefined() ; // there may be commands we don't handle here

  // Process and check varargs 
  pc.process(cmdList) ;
  if (!pc.ok(kTRUE)) {
    return frame ;
  }

  const RooAbsData* projData = (const RooAbsData*) pc.getObject("projData") ;
  const RooArgSet* projDataSet = (const RooArgSet*) pc.getObject("projDataSet") ;
  const RooArgSet* sliceSetTmp = (const RooArgSet*) pc.getObject("sliceSet") ;
  RooArgSet* sliceSet = sliceSetTmp ? ((RooArgSet*) sliceSetTmp->Clone()) : 0 ;
  const RooArgSet* projSet = (const RooArgSet*) pc.getObject("projSet") ;  
  Double_t scaleFactor = pc.getDouble("scaleFactor") ;
  ScaleType stype = (ScaleType) pc.getInt("scaleType") ;


  // Look for category slice arguments and add them to the master slice list if found
  const char* sliceCatState = pc.getString("sliceCatState",0,kTRUE) ;
  const RooLinkedList& sliceCatList = pc.getObjectList("sliceCatList") ;
  if (sliceCatState) {

    // Make the master slice set if it doesnt exist
    if (!sliceSet) {
      sliceSet = new RooArgSet ;
    }

    // Prepare comma separated label list for parsing
    char buf[1024] ;
    strlcpy(buf,sliceCatState,1024) ;
    const char* slabel = strtok(buf,",") ;

    // Loop over all categories provided by (multiple) Slice() arguments
    TIterator* iter = sliceCatList.MakeIterator() ;
    RooCategory* scat ;
    while((scat=(RooCategory*)iter->Next())) {
      if (slabel) {
	// Set the slice position to the value indicate by slabel
	scat->setLabel(slabel) ;
	// Add the slice category to the master slice set
	sliceSet->add(*scat,kFALSE) ;
      }
      slabel = strtok(0,",") ;
    }
    delete iter ;
  }

  // Check if we have a projection dataset
  if (!projData) {
    coutE(InputArguments) << "RooSimultaneous::plotOn(" << GetName() << ") ERROR: must have a projection dataset for index category" << endl ;
    return frame ;
  }

  // Make list of variables to be projected
  RooArgSet projectedVars ;
  if (sliceSet) {
    //cout << "frame->getNormVars() = " ; frame->getNormVars()->Print("1") ;

    makeProjectionSet(frame->getPlotVar(),frame->getNormVars(),projectedVars,kTRUE) ;
    
    // Take out the sliced variables
    TIterator* iter = sliceSet->createIterator() ;
    RooAbsArg* sliceArg ;
    while((sliceArg=(RooAbsArg*)iter->Next())) {
      RooAbsArg* arg = projectedVars.find(sliceArg->GetName()) ;
      if (arg) {
	projectedVars.remove(*arg) ;
      } else {
	coutI(Plotting) << "RooAbsReal::plotOn(" << GetName() << ") slice variable " 
			<< sliceArg->GetName() << " was not projected anyway" << endl ;
      }
    }
    delete iter ;
  } else if (projSet) {
    makeProjectionSet(frame->getPlotVar(),projSet,projectedVars,kFALSE) ;
  } else {
    makeProjectionSet(frame->getPlotVar(),frame->getNormVars(),projectedVars,kTRUE) ;
  }

  Bool_t projIndex(kFALSE) ;

  if (!_indexCat.arg().isDerived()) {
    // *** Error checking for a fundamental index category ***
    //cout << "RooSim::plotOn: index is fundamental" << endl ;
      
    // Check that the provided projection dataset contains our index variable
    if (!projData->get()->find(_indexCat.arg().GetName())) {
      coutE(Plotting) << "RooSimultaneous::plotOn(" << GetName() << ") ERROR: Projection over index category "
		      << "requested, but projection data set doesn't contain index category" << endl ;
      return frame ;
    }

    if (projectedVars.find(_indexCat.arg().GetName())) {
      projIndex=kTRUE ;
    }

  } else {
    // *** Error checking for a composite index category ***

    // Determine if any servers of the index category are in the projectedVars
    TIterator* sIter = _indexCat.arg().serverIterator() ;
    RooAbsArg* server ;
    RooArgSet projIdxServers ;
    Bool_t anyServers(kFALSE) ;
    while((server=(RooAbsArg*)sIter->Next())) {
      if (projectedVars.find(server->GetName())) {
	anyServers=kTRUE ;
	projIdxServers.add(*server) ;
      }
    }
    delete sIter ;

    // Check that the projection dataset contains all the 
    // index category components we're projecting over

    // Determine if all projected servers of the index category are in the projection dataset
    sIter = projIdxServers.createIterator() ;
    Bool_t allServers(kTRUE) ;
    while((server=(RooAbsArg*)sIter->Next())) {
      if (!projData->get()->find(server->GetName())) {
	allServers=kFALSE ;
      }
    }
    delete sIter ;
    
    if (!allServers) {      
      coutE(Plotting) << "RooSimultaneous::plotOn(" << GetName() 
	   << ") ERROR: Projection dataset doesn't contain complete set of index category dependents" << endl ;
      return frame ;
    }

    if (anyServers) {
      projIndex = kTRUE ;
    }
  } 

  // Calculate relative weight fractions of components
  Roo1DTable* wTable = projData->table(_indexCat.arg()) ;

  // If we don't project over the index, just do the regular plotOn
  if (!projIndex) {

    coutI(Plotting) << "RooSimultaneous::plotOn(" << GetName() << ") plot on " << frame->getPlotVar()->GetName() 
		    << " represents a slice in the index category ("  << _indexCat.arg().GetName() << ")" << endl ;

    // Reduce projData: take out fitCat (component) columns and entries that don't match selected slice
    // Construct cut string to only select projection data event that match the current slice

    const RooAbsData* projDataTmp(projData) ;
    if (projData) {
      // Make list of categories columns to exclude from projection data      
      RooArgSet* indexCatComps = _indexCat.arg().getObservables(frame->getNormVars());

      // Make cut string to exclude rows from projection data
      TString cutString ;
      TIterator* compIter =  indexCatComps->createIterator() ;    
      RooAbsCategory* idxComp ;
      Bool_t first(kTRUE) ;
      while((idxComp=(RooAbsCategory*)compIter->Next())) {
	if (!first) {
	  cutString.Append("&&") ;
	} else {
	  first=kFALSE ;
	}
	cutString.Append(Form("%s==%d",idxComp->GetName(),idxComp->getIndex())) ;
      }
      delete compIter ;

      // Make temporary projData without RooSim index category components
      RooArgSet projDataVars(*projData->get()) ;
      projDataVars.remove(*indexCatComps,kTRUE,kTRUE) ;
      
      projDataTmp = ((RooAbsData*)projData)->reduce(projDataVars,cutString) ;
      delete indexCatComps ;
    }

    // Multiply scale factor with fraction of events in current state of index

//     RooPlot* retFrame =  getPdf(_indexCat.arg().getLabel())->plotOn(frame,drawOptions,
// 					   scaleFactor*wTable->getFrac(_indexCat.arg().getLabel()),
// 					   stype,projDataTmp,projSet) ;

    // Override normalization and projection dataset
    RooCmdArg tmp1 = RooFit::Normalization(scaleFactor*wTable->getFrac(_indexCat.arg().getLabel()),stype) ;
    RooCmdArg tmp2 = RooFit::ProjWData(*projDataSet,*projDataTmp) ;

    // WVE -- do not adjust normalization for asymmetry plots
    RooLinkedList cmdList2(cmdList) ;
    if (!cmdList.find("Asymmetry")) {
      cmdList2.Add(&tmp1) ;
    }
    cmdList2.Add(&tmp2) ;

    // Plot single component
    RooPlot* retFrame =  getPdf(_indexCat.arg().getLabel())->plotOn(frame,cmdList2) ;

    // Delete temporary dataset
    if (projDataTmp) {
      delete projDataTmp ;
    }

    delete wTable ;
    delete sliceSet ;
    return retFrame ;
  }

  // If we project over the index, plot using a temporary RooAddPdf
  // using the weights from the data as coefficients

  // Make a deep clone of our index category
  RooArgSet* idxCloneSet = (RooArgSet*) RooArgSet(_indexCat.arg()).snapshot(kTRUE) ;
  RooAbsCategoryLValue* idxCatClone = (RooAbsCategoryLValue*) idxCloneSet->find(_indexCat.arg().GetName()) ;

  // Build the list of indexCat components that are sliced
  RooArgSet* idxCompSliceSet = _indexCat.arg().getObservables(frame->getNormVars()) ;
  idxCompSliceSet->remove(projectedVars,kTRUE,kTRUE) ;
  TIterator* idxCompSliceIter = idxCompSliceSet->createIterator() ;

  // Make a new expression that is the weighted sum of requested components
  RooArgList pdfCompList ;
  RooArgList wgtCompList ;
//RooAbsPdf* pdf ;
  RooRealProxy* proxy ;
  TIterator* pIter = _pdfProxyList.MakeIterator() ;
  Double_t sumWeight(0) ;
  while((proxy=(RooRealProxy*)pIter->Next())) {

    idxCatClone->setLabel(proxy->name()) ;

    // Determine if this component is the current slice (if we slice)
    Bool_t skip(kFALSE) ;
    idxCompSliceIter->Reset() ;
    RooAbsCategory* idxSliceComp ;
    while((idxSliceComp=(RooAbsCategory*)idxCompSliceIter->Next())) {
      RooAbsCategory* idxComp = (RooAbsCategory*) idxCloneSet->find(idxSliceComp->GetName()) ;
      if (idxComp->getIndex()!=idxSliceComp->getIndex()) {
	skip=kTRUE ;
	break ;
      }
    }
    if (skip) continue ;
 
    // Instantiate a RRV holding this pdfs weight fraction
    RooRealVar *wgtVar = new RooRealVar(proxy->name(),"coef",wTable->getFrac(proxy->name())) ;
    wgtCompList.addOwned(*wgtVar) ;
    sumWeight += wTable->getFrac(proxy->name()) ;

    // Add the PDF to list list
    pdfCompList.add(proxy->arg()) ;
  }

  TString plotVarName(GetName()) ;
  RooAddPdf *plotVar = new RooAddPdf(plotVarName,"weighted sum of RS components",pdfCompList,wgtCompList) ;

  // Fix appropriate coefficient normalization in plot function
  if (_plotCoefNormSet.getSize()>0) {
    plotVar->fixAddCoefNormalization(_plotCoefNormSet) ;
  }

  RooAbsData* projDataTmp(0) ;
  RooArgSet projSetTmp ;
  if (projData) {
    
    // Construct cut string to only select projection data event that match the current slice
    TString cutString ;
    if (idxCompSliceSet->getSize()>0) {
      idxCompSliceIter->Reset() ;
      RooAbsCategory* idxSliceComp ;
      Bool_t first(kTRUE) ;
      while((idxSliceComp=(RooAbsCategory*)idxCompSliceIter->Next())) {
	if (!first) {
	  cutString.Append("&&") ;
	} else {
	  first=kFALSE ;
	}
	cutString.Append(Form("%s==%d",idxSliceComp->GetName(),idxSliceComp->getIndex())) ;
      }
    }

    // Make temporary projData without RooSim index category components
    RooArgSet projDataVars(*projData->get()) ;
    RooArgSet* idxCatServers = _indexCat.arg().getObservables(frame->getNormVars()) ;

    projDataVars.remove(*idxCatServers,kTRUE,kTRUE) ;

    if (idxCompSliceSet->getSize()>0) {
      projDataTmp = ((RooAbsData*)projData)->reduce(projDataVars,cutString) ;
    } else {
      projDataTmp = ((RooAbsData*)projData)->reduce(projDataVars) ;      
    }

    

    if (projSet) {
      projSetTmp.add(*projSet) ;
      projSetTmp.remove(*idxCatServers,kTRUE,kTRUE);
    }

    
    delete idxCatServers ;
  }


  if (_indexCat.arg().isDerived() && idxCompSliceSet->getSize()>0) {
    coutI(Plotting) << "RooSimultaneous::plotOn(" << GetName() << ") plot on " << frame->getPlotVar()->GetName() 
		    << " represents a slice in index category components " << *idxCompSliceSet << endl ;

    RooArgSet* idxCompProjSet = _indexCat.arg().getObservables(frame->getNormVars()) ;
    idxCompProjSet->remove(*idxCompSliceSet,kTRUE,kTRUE) ;
    if (idxCompProjSet->getSize()>0) {
      coutI(Plotting) << "RooSimultaneous::plotOn(" << GetName() << ") plot on " << frame->getPlotVar()->GetName() 
		      << " averages with data index category components " << *idxCompProjSet << endl ;
    }
    delete idxCompProjSet ;
  } else {
    coutI(Plotting) << "RooSimultaneous::plotOn(" << GetName() << ") plot on " << frame->getPlotVar()->GetName() 
		    << " averages with data index category (" << _indexCat.arg().GetName() << ")" << endl ;
  }


  // Override normalization and projection dataset
  RooLinkedList cmdList2(cmdList) ;

  RooCmdArg tmp1 = RooFit::Normalization(scaleFactor*sumWeight,stype) ;
  RooCmdArg tmp2 = RooFit::ProjWData(*projDataSet,*projDataTmp) ;
  // WVE -- do not adjust normalization for asymmetry plots
  if (!cmdList.find("Asymmetry")) {
    cmdList2.Add(&tmp1) ;
  }
  cmdList2.Add(&tmp2) ;

  RooPlot* frame2 ;
  if (projSetTmp.getSize()>0) {
    // Plot temporary function  
    RooCmdArg tmp3 = RooFit::Project(projSetTmp) ;
    cmdList2.Add(&tmp3) ;
    frame2 = plotVar->plotOn(frame,cmdList2) ;
  } else {
    // Plot temporary function  
    frame2 = plotVar->plotOn(frame,cmdList2) ;
  }

  // Cleanup
  delete sliceSet ;
  delete pIter ;
  delete wTable ;
  delete idxCloneSet ;
  delete idxCompSliceIter ;
  delete idxCompSliceSet ;
  delete plotVar ;

  if (projDataTmp) delete projDataTmp ;

  return frame2 ;
}



//_____________________________________________________________________________
RooPlot* RooSimultaneous::plotOn(RooPlot *frame, Option_t* drawOptions, Double_t scaleFactor, 
				 ScaleType stype, const RooAbsData* projData, const RooArgSet* projSet,
				 Double_t /*precision*/, Bool_t /*shiftToZero*/, const RooArgSet* /*projDataSet*/,
				 Double_t /*rangeLo*/, Double_t /*rangeHi*/, RooCurve::WingMode /*wmode*/) const
{
  // OBSOLETE -- Retained for backward compatibility

  // Make command list
  RooLinkedList cmdList ;
  cmdList.Add(new RooCmdArg(RooFit::DrawOption(drawOptions))) ;
  cmdList.Add(new RooCmdArg(RooFit::Normalization(scaleFactor,stype))) ;
  if (projData) cmdList.Add(new RooCmdArg(RooFit::ProjWData(*projData))) ;
  if (projSet) cmdList.Add(new RooCmdArg(RooFit::Project(*projSet))) ;

  // Call new method
  RooPlot* ret = plotOn(frame,cmdList) ;

  // Cleanup
  cmdList.Delete() ;
  return ret ;  
}



//_____________________________________________________________________________
void RooSimultaneous::selectNormalization(const RooArgSet* normSet, Bool_t /*force*/) 
{
  // Interface function used by test statistics to freeze choice of observables
  // for interpretation of fraction coefficients. Needed here because a RooSimultaneous
  // works like a RooAddPdf when plotted
  
  _plotCoefNormSet.removeAll() ;
  if (normSet) _plotCoefNormSet.add(*normSet) ;
}


//_____________________________________________________________________________
void RooSimultaneous::selectNormalizationRange(const char* normRange2, Bool_t /*force*/) 
{
  // Interface function used by test statistics to freeze choice of range
  // for interpretation of fraction coefficients. Needed here because a RooSimultaneous
  // works like a RooAddPdf when plotted

  _plotCoefNormRange = RooNameReg::ptr(normRange2) ;
}




//_____________________________________________________________________________
RooAbsGenContext* RooSimultaneous::autoGenContext(const RooArgSet &vars, const RooDataSet* prototype, 
						  const RooArgSet* auxProto, Bool_t verbose, Bool_t autoBinned, const char* binnedTag) const 
{
  const char* idxCatName = _indexCat.arg().GetName() ;
  
  if (vars.find(idxCatName) && prototype==0 && (auxProto==0 || auxProto->getSize()==0) && (autoBinned || (binnedTag && strlen(binnedTag)))) {    

    // Return special generator config that can also do binned generation for selected states
    return new RooSimSplitGenContext(*this,vars,verbose,autoBinned,binnedTag) ;

  } else {
    
    // Return regular generator config ;
    return genContext(vars,prototype,auxProto,verbose) ;
  }     
}



//_____________________________________________________________________________
RooAbsGenContext* RooSimultaneous::genContext(const RooArgSet &vars, const RooDataSet *prototype, 
					      const RooArgSet* auxProto, Bool_t verbose) const 
{
  // Return specialized generator contenxt for simultaneous p.d.f.s

  const char* idxCatName = _indexCat.arg().GetName() ;
  const RooArgSet* protoVars = prototype ? prototype->get() : 0 ;

  if (vars.find(idxCatName) || (protoVars && protoVars->find(idxCatName))) {

    // Generating index category: return special sim-context
    return new RooSimGenContext(*this,vars,prototype,auxProto,verbose) ;

  } else if (_indexCat.arg().isDerived()) {
    // Generating dependents of a derived index category

    // Determine if we none,any or all servers
    Bool_t anyServer(kFALSE), allServers(kTRUE) ;
    if (prototype) {
      TIterator* sIter = _indexCat.arg().serverIterator() ;
      RooAbsArg* server ;
      while((server=(RooAbsArg*)sIter->Next())) {
	if (prototype->get()->find(server->GetName())) {
	  anyServer=kTRUE ;
	} else {
	  allServers=kFALSE ;
	}
      }
      delete sIter ;
    } else {
      allServers=kTRUE ;
    }

    if (allServers) {
      // Use simcontext if we have all servers

      return new RooSimGenContext(*this,vars,prototype,auxProto,verbose) ;
    } else if (!allServers && anyServer) {
      // Abort if we have only part of the servers
      coutE(Plotting) << "RooSimultaneous::genContext: ERROR: prototype must include either all "
		      << " components of the RooSimultaneous index category or none " << endl ;
      return 0 ; 
    } 
    // Otherwise make single gencontext for current state
  } 

  // Not generating index cat: return context for pdf associated with present index state
  RooRealProxy* proxy = (RooRealProxy*) _pdfProxyList.FindObject(_indexCat.arg().getLabel()) ;
  if (!proxy) {
    coutE(InputArguments) << "RooSimultaneous::genContext(" << GetName() 
			  << ") ERROR: no PDF associated with current state (" 
			  << _indexCat.arg().GetName() << "=" << _indexCat.arg().getLabel() << ")" << endl ; 
    return 0 ;
  }
  return ((RooAbsPdf*)proxy->absArg())->genContext(vars,prototype,auxProto,verbose) ;
}




//_____________________________________________________________________________
RooDataHist* RooSimultaneous::fillDataHist(RooDataHist *hist,
                                           const RooArgSet* nset,
                                           Double_t scaleFactor,
                                           Bool_t correctForBinVolume,
                                           Bool_t showProgress) const
{
  if (RooAbsReal::fillDataHist (hist, nset, scaleFactor,
                                correctForBinVolume, showProgress) == 0)
    return 0;

  Double_t sum = 0;
  for (int i=0 ; i<hist->numEntries() ; i++) {
    hist->get(i) ;
    sum += hist->weight();
  }
  if (sum != 0) {
    for (int i=0 ; i<hist->numEntries() ; i++) {
      hist->get(i) ;
      hist->set (hist->weight() / sum);
    }
  }

  return hist;
}




//_____________________________________________________________________________
RooDataSet* RooSimultaneous::generateSimGlobal(const RooArgSet& whatVars, Int_t nEvents) 
{
  // Special generator interface for generation of 'global observables' -- for RooStats tools

  // Make set with clone of variables (placeholder for output)
  RooArgSet* globClone = (RooArgSet*) whatVars.snapshot() ;

  RooDataSet* data = new RooDataSet("gensimglobal","gensimglobal",whatVars) ;
  
  // Construct iterator over index types
  TIterator* iter = indexCat().typeIterator() ;

  for (Int_t i=0 ; i<nEvents ; i++) {
    iter->Reset() ;
    RooCatType* tt ; 
    while((tt=(RooCatType*) iter->Next())) {
      
      // Get pdf associated with state from simpdf
      RooAbsPdf* pdftmp = getPdf(tt->GetName()) ;
      
      // Generate only global variables defined by the pdf associated with this state
      RooArgSet* globtmp = pdftmp->getObservables(whatVars) ;
      RooDataSet* tmp = pdftmp->generate(*globtmp,1) ;
      
      // Transfer values to output placeholder
      *globClone = *tmp->get(0) ;
      
      // Cleanup 
      delete globtmp ;
      delete tmp ;
    }
    data->add(*globClone) ;
  }


  delete iter ;
  delete globClone ;
  return data ;
}









