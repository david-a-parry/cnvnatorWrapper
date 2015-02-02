// @(#)root/hist:$Id$
// Authors: Bartolomeu Rabacal    07/2010
/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006 , LCG ROOT MathLib Team                         *
 *                                                                    *
 *                                                                    *
 **********************************************************************/
// Header file for TKDE

#ifndef ROOT_TKDE
#define ROOT_TKDE

#ifndef ROOT_Math_WrappedFunction
   #include "Math/WrappedFunction.h"
#endif

#ifndef ROOT_TNamed
   #include "TNamed.h"
#endif

#ifndef ROOT_Math_Math
#include "Math/Math.h"
#endif

//#include "TF1.h"
class TGraphErrors;
class TF1;

/*
   Kernel Density Estimation class. The three main references are (1) "Scott DW, Multivariate Density Estimation.
Theory, Practice and Visualization. New York: Wiley", (2) "Jann Ben - ETH Zurich, Switzerland -, Univariate kernel density estimation document for KDENS: Stata module for univariate kernel density estimation." and (3) "Hardle W, Muller M, Sperlich S, Werwatz A, Nonparametric and Semiparametric Models. Springer."
   The algorithm is briefly described in (4) "Cranmer KS, Kernel Estimation in High-Energy
Physics. Computer Physics Communications 136:198-207,2001" - e-Print Archive: hep ex/0011057.
   A binned version is also implemented to address the performance issue due to its data size dependence.
*/
class TKDE : public TNamed  {
public:

   enum EKernelType { // Kernel function type option
      kGaussian,
      kEpanechnikov,
      kBiweight,
      kCosineArch,
      kUserDefined, // Internal use only for the class's template constructor
      kTotalKernels // Internal use only for member initialization
   };

   enum EIteration { // KDE fitting option
      kAdaptive,
      kFixed
   };

   enum EMirror { // Data "mirroring" option to address the probability "spill out" boundary effect
      kNoMirror,
      kMirrorLeft,
      kMirrorRight,
      kMirrorBoth,
      kMirrorAsymLeft,
      kMirrorAsymLeftRight,
      kMirrorAsymRight,
      kMirrorLeftAsymRight,
      kMirrorAsymBoth
   };

   enum EBinning{ // Data binning option
      kUnbinned,
      kRelaxedBinning, // The algorithm is allowed to use binning if the data is large enough
      kForcedBinning
   };

   explicit TKDE(UInt_t events = 0, const Double_t* data = 0, Double_t xMin = 0.0, Double_t xMax = 0.0, const Option_t* option = "KernelType:Gaussian;Iteration:Adaptive;Mirror:noMirror;Binning:RelaxedBinning", Double_t rho = 1.0);

   template<class KernelFunction>
   TKDE(const Char_t* /*name*/, const KernelFunction& kernfunc, UInt_t events, const Double_t* data, Double_t xMin = 0.0, Double_t xMax = 0.0, const Option_t* option = "KernelType:UserDefined;Iteration:Adaptive;Mirror:noMirror;Binning:RelaxedBinning", Double_t rho = 1.0)  {
      Instantiate(new ROOT::Math::WrappedFunction<const KernelFunction&>(kernfunc), events, data, xMin, xMax, option, rho);
   }

   virtual ~TKDE();

   void Fill(Double_t data);
   void SetKernelType(EKernelType kern);
   void SetIteration(EIteration iter);
   void SetMirror(EMirror mir);
   void SetBinning(EBinning);
   void SetNBins(UInt_t nbins);
   void SetUseBinsNEvents(UInt_t nEvents);
   void SetTuneFactor(Double_t rho);
   void SetRange(Double_t xMin, Double_t xMax); // By default computed from the data

   virtual void Draw(const Option_t* option = "");

   Double_t operator()(Double_t x) const;
   Double_t operator()(const Double_t* x, const Double_t* p=0) const;  // Needed for creating TF1

   Double_t GetValue(Double_t x) const { return (*this)(x); }
   Double_t GetError(Double_t x) const;

   Double_t GetBias(Double_t x) const;
   Double_t GetMean() const;
   Double_t GetSigma() const;
   Double_t GetRAMISE() const;

   Double_t GetFixedWeight() const;

   TF1* GetFunction(UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);
   TF1* GetUpperFunction(Double_t confidenceLevel = 0.95, UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);
   TF1* GetLowerFunction(Double_t confidenceLevel = 0.95, UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);
   TF1* GetApproximateBias(UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);
   TGraphErrors * GetGraphWithErrors(UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);

   // get the drawn object to chanage settings
   // These objects are managed by TKDE and should not be deleted by the user
   TF1 * GetDrawnFunction() { return fPDF;}  
   TF1 * GetDrawnUpperFunction() { return fUpperPDF;}  
   TF1 * GetDrawnLowerFunction() { return fLowerPDF;}  
   TGraphErrors * GetDrawnGraph() { return fGraph;}  

   const Double_t * GetAdaptiveWeights() const;


private:

   TKDE(TKDE& kde);           // Disallowed copy constructor
   TKDE operator=(TKDE& kde); // Disallowed assign operator

   typedef ROOT::Math::IBaseFunctionOneDim* KernelFunction_Ptr;
   KernelFunction_Ptr fKernelFunction;

   class TKernel;
   friend class TKernel;

   TKernel* fKernel;

   std::vector<Double_t> fData;   // Data events
   std::vector<Double_t> fEvents; // Original data storage

   TF1* fPDF;             // Output Kernel Density Estimation PDF function
   TF1* fUpperPDF;        // Output Kernel Density Estimation upper confidence interval PDF function
   TF1* fLowerPDF;        // Output Kernel Density Estimation lower confidence interval PDF function
   TF1* fApproximateBias; // Output Kernel Density Estimation approximate bias
   TGraphErrors* fGraph;  // Graph with the errors

   EKernelType fKernelType;
   EIteration fIteration;
   EMirror fMirror;
   EBinning fBinning;

   Bool_t fUseMirroring, fMirrorLeft, fMirrorRight, fAsymLeft, fAsymRight;
   Bool_t fUseBins;
   Bool_t fNewData;        // flag to control when new data are given
   Bool_t fUseMinMaxFromData; // flag top control if min and max must be used from data

   UInt_t fNBins;          // Number of bins for binned data option
   UInt_t fNEvents;        // Data's number of events
   UInt_t fUseBinsNEvents; // If the algorithm is allowed to use binning this is the minimum number of events to do so

   Double_t fMean;  // Data mean
   Double_t fSigma; // Data std deviation
   Double_t fSigmaRob; // Data std deviation (robust estimation)
   Double_t fXMin;  // Data minimum value
   Double_t fXMax;  // Data maximum value
   Double_t fRho;   // Adjustment factor for sigma
   Double_t fAdaptiveBandwidthFactor; // Geometric mean of the kernel density estimation from the data for adaptive iteration

   Double_t fWeightSize; // Caches the weight size

   std::vector<Double_t> fCanonicalBandwidths;
   std::vector<Double_t> fKernelSigmas2;

   std::vector<UInt_t> fBinCount; // Number of events per bin for binned data option

   std::vector<Bool_t> fSettedOptions; // User input options flag

   struct KernelIntegrand;
   friend struct KernelIntegrand;

   void Instantiate(KernelFunction_Ptr kernfunc, UInt_t events, const Double_t* data,
                    Double_t xMin, Double_t xMax, const Option_t* option, Double_t rho);

   inline Double_t GaussianKernel(Double_t x) const {
      // Returns the kernel evaluation at x
      Double_t k2_PI_ROOT_INV = 0.398942280401432703; // (2 * M_PI)**-0.5
      return (x > -9. && x < 9.) ? k2_PI_ROOT_INV * std::exp(-.5 * x * x) : 0.0;
   }
   inline Double_t EpanechnikovKernel(Double_t x) const {
      return (x > -1. &&  x < 1.) ? 3. / 4. * (1. - x * x) : 0.0;
   }
   inline Double_t BiweightKernel(Double_t x) const {
      // Returns the kernel evaluation at x
      return (x > -1. &&  x < 1.) ? 15. / 16. * (1. - x * x) * (1. - x * x) : 0.0;
   }
   inline Double_t CosineArchKernel(Double_t x) const {
      // Returns the kernel evaluation at x
      return (x > -1. &&  x < 1.) ? M_PI_4 * std::cos(M_PI_2 * x) : 0.0;
   }
   Double_t UpperConfidenceInterval(const Double_t* x, const Double_t* p) const; // Valid if the bandwidth is small compared to nEvents**1/5
   Double_t LowerConfidenceInterval(const Double_t* x, const Double_t* p) const; // Valid if the bandwidth is small compared to nEvents**1/5
   Double_t ApproximateBias(const Double_t* x, const Double_t* ) const { return GetBias(*x); }
   Double_t ComputeKernelL2Norm() const;
   Double_t ComputeKernelSigma2() const;
   Double_t ComputeKernelMu() const;
   Double_t ComputeKernelIntegral() const;
   Double_t ComputeMidspread() ;

   UInt_t Index(Double_t x) const;

   void SetBinCentreData(Double_t xmin, Double_t xmax);
   void SetBinCountData();
   void CheckKernelValidity();
   void SetCanonicalBandwidth();
   void SetKernelSigma2();
   void SetCanonicalBandwidths();
   void SetKernelSigmas2();
   void SetHistogram();
   void SetUseBins();
   void SetMirror();
   void SetMean();
   void SetSigma(Double_t R);
   void SetKernel();
   void SetKernelFunction(KernelFunction_Ptr kernfunc = 0);
   void SetOptions(const Option_t* option, Double_t rho);
   void CheckOptions(Bool_t isUserDefinedKernel = kFALSE);
   void GetOptions(std::string optionType, std::string option);
   void AssureOptions();
   void SetData(const Double_t* data);
   void InitFromNewData();
   void SetMirroredEvents();
   void SetDrawOptions(const Option_t* option, TString& plotOpt, TString& drawOpt);
   void DrawErrors(TString& drawOpt);
   void DrawConfidenceInterval(TString& drawOpt, double cl=0.95);

   TF1* GetKDEFunction(UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);
   TF1* GetKDEApproximateBias(UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);
   // The density to estimate should be at least twice differentiable.
   TF1* GetPDFUpperConfidenceInterval(Double_t confidenceLevel = 0.95, UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);
   TF1* GetPDFLowerConfidenceInterval(Double_t confidenceLevel = 0.95, UInt_t npx = 100, Double_t xMin = 1.0, Double_t xMax = 0.0);

   ClassDef(TKDE, 1) // One dimensional semi-parametric Kernel Density Estimation

};

#endif
