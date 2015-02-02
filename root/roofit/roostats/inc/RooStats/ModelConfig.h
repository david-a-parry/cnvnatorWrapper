// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke, Sven Kreiss
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_ModelConfig
#define ROOSTATS_ModelConfig


#ifndef ROO_ABS_PDF
#include "RooAbsPdf.h"
#endif

#ifndef ROO_ABS_DATA
#include "RooAbsData.h"
#endif

#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif

#ifndef ROO_WORKSPACE
#include "RooWorkspace.h"
#endif

#ifndef ROOT_TRef
#include "TRef.h"
#endif

#include <string>

//_________________________________________________
/*
BEGIN_HTML
<p>
ModelConfig is a simple class that holds configuration information specifying how a model
should be used in the context of various RooStats tools.  A single model can be used
in different ways, and this class should carry all that is needed to specify how it should be used.
ModelConfig requires a workspace to be set.
</p>
END_HTML
*/
//


namespace RooStats {

class ModelConfig : public TNamed {

public:

   ModelConfig(RooWorkspace * ws = 0) : 
      TNamed()
   {
      if(ws) SetWS(*ws);
   }
    
   ModelConfig(const char* name, RooWorkspace *ws = 0) : 
      TNamed(name, name)
   {
      if(ws) SetWS(*ws);
   }
    
   ModelConfig(const char* name, const char* title, RooWorkspace *ws = 0) : 
      TNamed(name, title)
   {
      if(ws) SetWS(*ws);
   }

    
   // clone
   virtual ModelConfig * Clone(const char * name = "") const {
      ModelConfig * mc =  new ModelConfig(*this);
      if(strcmp(name,"")==0)
	mc->SetName(this->GetName());
      else
	mc->SetName(name); 
      return mc; 
   }

   // set a workspace that owns all the necessary components for the analysis
   virtual void SetWS(RooWorkspace & ws);
   /// alias for SetWS(...)
   virtual void SetWorkspace(RooWorkspace & ws) { SetWS(ws); }

   // Set the proto DataSet, add to the the workspace if not already there
   virtual void SetProtoData(RooAbsData & data) {      
      ImportDataInWS(data); 
      SetProtoData( data.GetName() );
   }
    
   // Set the Pdf, add to the the workspace if not already there
   virtual void SetPdf(const RooAbsPdf& pdf) {
      ImportPdfInWS(pdf);
      SetPdf( pdf.GetName() );      
   }

   // Set the Prior Pdf, add to the the workspace if not already there
   virtual void SetPriorPdf(const RooAbsPdf& pdf) {
      ImportPdfInWS(pdf);
      SetPriorPdf( pdf.GetName() );      
   }
    
   // specify the parameters of interest in the interval
   virtual void SetParameters(const RooArgSet& set) {
     if (!SetHasOnlyParameters(set,"ModelConfig::SetParameters")) return ;
     fPOIName=std::string(GetName()) + "_POI";
     DefineSetInWS(fPOIName.c_str(), set);
   }

   virtual void SetParametersOfInterest(const RooArgSet& set) {
     if (!SetHasOnlyParameters(set,"ModelConfig::SetParametersOfInterest")) return ;
      SetParameters(set); 
   }
   // specify the parameters of interest 
   // through a list of comma-separated arguments already in the workspace
   virtual void SetParameters(const char *argList) {
      if(!GetWS()) return;
      SetParameters(GetWS()->argSet(argList));
   }
   virtual void SetParametersOfInterest(const char *argList) {
      SetParameters(argList);
   }
    
   // specify the nuisance parameters (e.g. the rest of the parameters)
   virtual void SetNuisanceParameters(const RooArgSet& set) {
     if (!SetHasOnlyParameters(set,"ModelConfig::SetNuisanceParameters")) return ;
      fNuisParamsName=std::string(GetName()) + "_NuisParams";
      DefineSetInWS(fNuisParamsName.c_str(), set);
   }
   // specify the nuisance parameters 
   // through a list of comma-separated arguments already in the workspace
   virtual void SetNuisanceParameters(const char *argList) {
      if(!GetWS()) return;
      SetNuisanceParameters(GetWS()->argSet(argList));
   }

   // specify the constraint parameters 
   virtual void SetConstraintParameters(const RooArgSet& set) {
     if (!SetHasOnlyParameters(set,"ModelConfig::SetConstainedParameters")) return ;
      fConstrParamsName=std::string(GetName()) + "_ConstrainedParams";
      DefineSetInWS(fConstrParamsName.c_str(), set);
   }
   // specify the constraint parameters 
   // through a list of comma-separated arguments already in the workspace
   virtual void SetConstraintParameters(const char *argList) {
      if(!GetWS()) return;
      SetConstraintParameters(GetWS()->argSet(argList));
   }

   // specify the observables
   virtual void SetObservables(const RooArgSet& set) {
     if (!SetHasOnlyParameters(set,"ModelConfig::SetObservables")) return ;
      fObservablesName=std::string(GetName()) + "_Observables";
      DefineSetInWS(fObservablesName.c_str(), set);
   }
   // specify the observables 
   // through a list of comma-separated arguments already in the workspace
   virtual void SetObservables(const char *argList) {
      if(!GetWS()) return;
      SetObservables(GetWS()->argSet(argList));
   }

   // specify the conditional observables
   virtual void SetConditionalObservables(const RooArgSet& set) {
     if (!SetHasOnlyParameters(set,"ModelConfig::SetConditionalObservables")) return ;
      fConditionalObsName=std::string(GetName()) + "_ConditionalObservables";
      DefineSetInWS(fConditionalObsName.c_str(), set);
   }
   // specify the conditional observables
   // through a list of comma-separated arguments already in the workspace
   virtual void SetConditionalObservables(const char *argList) {
      if(!GetWS()) return;
      SetConditionalObservables(GetWS()->argSet(argList));
   }

   // specify the global observables
   virtual void SetGlobalObservables(const RooArgSet& set) {

     if (!SetHasOnlyParameters(set,"ModelConfig::SetGlobalObservables")) return ;

      // make global observables constant
      RooFIter iter = set.fwdIterator();
      RooAbsArg *arg = iter.next();
      while(arg != NULL) {
         arg->setAttribute("Constant", kTRUE);
         arg = iter.next();
      }

      fGlobalObsName=std::string(GetName()) + "_GlobalObservables";
      DefineSetInWS(fGlobalObsName.c_str(), set);
   }
   // specify the global observables 
   // through a list of comma-separated arguments already in the workspace
   virtual void SetGlobalObservables(const char *argList) {
      if(!GetWS()) return;
      SetGlobalObservables(GetWS()->argSet(argList));
   }

   // set parameter values for a particular hypothesis if using a common PDF
   // by saving a snapshot in the workspace
   virtual void SetSnapshot(const RooArgSet& set);
    
   // specify the name of the PDF in the workspace to be used
   virtual void SetPdf(const char* name) {
      if (! GetWS() ) return;

      if(GetWS()->pdf(name))
         fPdfName = name;
      else
         coutE(ObjectHandling) << "pdf "<<name<< " does not exist in workspace"<<std::endl;
   }

   // specify the name of the PDF in the workspace to be used
   virtual void SetPriorPdf(const char* name) {
      if (! GetWS() ) return;

      if(GetWS()->pdf(name))
         fPriorPdfName = name;
      else
         coutE(ObjectHandling) << "pdf "<<name<< " does not exist in workspace"<<std::endl;
   }


   // specify the name of the dataset in the workspace to be used
   virtual void SetProtoData(const char* name){
      if (! GetWS() ) return;

      if(GetWS()->data(name))
         fProtoDataName = name;
      else
         coutE(ObjectHandling) << "dataset "<<name<< " does not exist in workspace"<<std::endl;
   }


   /* getter methods */


   /// get model PDF (return NULL if pdf has not been specified or does not exist)
   RooAbsPdf * GetPdf() const { return (GetWS()) ? GetWS()->pdf(fPdfName.c_str()) : 0;   }

   /// get RooArgSet containing the parameter of interest (return NULL if not existing) 
   const RooArgSet * GetParametersOfInterest() const { return (GetWS()) ? GetWS()->set(fPOIName.c_str()) : 0; }

   /// get RooArgSet containing the nuisance parameters (return NULL if not existing) 
   const RooArgSet * GetNuisanceParameters() const { return (GetWS()) ? GetWS()->set(fNuisParamsName.c_str()) : 0; }

   /// get RooArgSet containing the constraint parameters (return NULL if not existing) 
   const RooArgSet * GetConstraintParameters() const { return (GetWS()) ? GetWS()->set(fConstrParamsName.c_str()) : 0; }

   /// get parameters prior pdf  (return NULL if not existing) 
   RooAbsPdf * GetPriorPdf() const { return (GetWS()) ? GetWS()->pdf(fPriorPdfName.c_str()) : 0; }

   /// get RooArgSet for observables  (return NULL if not existing)
   const RooArgSet * GetObservables() const { return (GetWS()) ? GetWS()->set(fObservablesName.c_str()) : 0; }

   /// get RooArgSet for conditional observables  (return NULL if not existing)
   const RooArgSet * GetConditionalObservables() const { return (GetWS()) ? GetWS()->set(fConditionalObsName.c_str()) : 0; }

   /// get RooArgSet for global observables  (return NULL if not existing)
   const RooArgSet * GetGlobalObservables() const { return (GetWS()) ? GetWS()->set(fGlobalObsName.c_str()) : 0; }

   /// get Proto data set (return NULL if not existing) 
   RooAbsData * GetProtoData()  const {  return (GetWS()) ? GetWS()->data(fProtoDataName.c_str()) : 0; }

   /// get RooArgSet for parameters for a particular hypothesis  (return NULL if not existing) 
   const RooArgSet * GetSnapshot() const;

   void LoadSnapshot() const;
 
   RooWorkspace * GetWS() const;
   /// alias for GetWS()
   RooWorkspace * GetWorkspace() const { return GetWS(); }

   /// guesses Observables and ParametersOfInterest if not already set
   void GuessObsAndNuisance(const RooAbsData& data);

   // overload the print method
   virtual void Print(Option_t* option = "") const;

protected:

   // helper function to check that content of a given set is exclusively parameters
   Bool_t SetHasOnlyParameters(const RooArgSet& set, const char* errorMsgPrefix=0) ;

   // helper functions to define a set in the WS
   void DefineSetInWS(const char* name, const RooArgSet& set);
    
   // internal function to import Pdf in WS
   void ImportPdfInWS(const RooAbsPdf & pdf);
      
   // internal function to import data in WS
   void ImportDataInWS(RooAbsData & data); 
    
   TRef fRefWS;  // WS reference used in the file

   std::string fWSName;  // name of the WS

   std::string fPdfName; // name of  PDF in workspace
   std::string fDataName; // name of data set in workspace
   std::string fPOIName; // name for RooArgSet specifying parameters of interest
    
   std::string fNuisParamsName; // name for RooArgSet specifying nuisance parameters
   std::string fConstrParamsName; // name for RooArgSet specifying constrained parameters
   std::string fPriorPdfName; // name for RooAbsPdf specifying a prior on the parameters
    
   std::string fConditionalObsName; // name for RooArgSet specifying conditional observables
   std::string fGlobalObsName; // name for RooArgSet specifying global observables
   std::string fProtoDataName; // name for RooArgSet specifying dataset that should be used as protodata
    
   std::string fSnapshotName; // name for RooArgSet that specifies a particular hypothesis
    
   std::string fObservablesName; // name for RooArgSet specifying observable parameters. 
    
   ClassDef(ModelConfig,4) // A class that holds configuration information for a model using a workspace as a store
      
};

}   // end namespace RooStats


#endif
