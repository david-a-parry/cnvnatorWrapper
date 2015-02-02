// @(#)root/mathcore:$Id$
// Author: L. Moneta Fri Sep 22 15:06:47 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// implementation file for class DistSampler

#include "Math/DistSampler.h"
#include "Math/DistSamplerOptions.h"
#include "Math/Error.h"

#include "Math/IFunction.h"
#include "Fit/BinData.h"
#include "Fit/UnBinData.h"
#include "Fit/DataRange.h"


namespace ROOT { 
   

namespace Math { 

DistSampler::~DistSampler() { 
   // destructor 
   if (fOwnFunc && fFunc != 0) delete fFunc; 
   if (fRange) delete fRange;
}

bool DistSampler::Init(const DistSamplerOptions & opt ) { 
   // default initialization with algorithm name
   return Init(opt.Algorithm().c_str() );
}

void DistSampler::SetRange(double xmin, double xmax, int icoord) {
   if (!fRange) { 
      MATH_ERROR_MSG("DistSampler::SetRange","Need to set function before setting the range");
      return;
   }
   fRange->SetRange(icoord,xmin,xmax);
}

void DistSampler::SetRange(const double * xmin, const double * xmax) {
   // set range specifying a vector for all coordinates 
   if (!fRange) {
      MATH_ERROR_MSG("DistSampler::SetRange","Need to set function before setting the range");
      return;
   }
   for (unsigned int icoord = 0; icoord < NDim(); ++icoord) 
      fRange->SetRange(icoord,xmin[icoord],xmax[icoord]);
}

void DistSampler::SetRange(const ROOT::Fit::DataRange & range) {
   // copy the given range
   *fRange = range;  
}

void DistSampler::DoSetFunction(const ROOT::Math::IMultiGenFunction & func, bool copy) { 
   // set the internal function
   // if a range exists and it is compatible it will be re-used
   if (fOwnFunc && fFunc != 0) delete fFunc; 
   if (copy) {
      fOwnFunc = true; 
      fFunc = func.Clone(); 
   }
   else {
      fOwnFunc = false; 
      fFunc = &func; 
   }
   fData = std::vector<double>(func.NDim());
   // delete a range if exists and it is not compatible
   if (fRange && fRange->NDim() != fData.size() ) {
      delete fRange; 
      fRange = 0; 
   }
   if (!fRange) fRange = new ROOT::Fit::DataRange(func.NDim() );
}

bool DistSampler::IsInitialized()  { 
   // test if sampler is initialized
   // tryying to generate one event (for this cannot be const) 
   if (NDim() == 0) return false; 
   if (fFunc == 0) return false; 
   if (fFunc->NDim() != NDim() ) return false; 
   // test one event 
   if (!Sample(&fData[0]) ) return false; 
   return true;
}

bool DistSampler::Generate(unsigned int nevt, ROOT::Fit::UnBinData & data) { 
   // generate a un-binned data sets (fill the given data set)
   // if dataset has already data append to it 
   int n0 = data.DataSize(); 
   if (n0 > 0 ) { 
      if (data.PointSize() != NDim() ) {
         MATH_ERROR_MSG("DistSampler::Generate","unbin data not consistent with distribution");
         return false; 
      }
   }
   if (!IsInitialized()) { 
         MATH_WARN_MSG("DistSampler::Generate","sampler has not been initialized correctly");
         return false; 
   }

   data.Initialize( n0 + nevt, NDim() );
   for (unsigned int i = 0; i < nevt; ++i) { 
      const double * x = Sample(); 
      data.Add( x ); 
   }
   return true;
}


   bool DistSampler::Generate(unsigned int nevt, const  int * nbins, ROOT::Fit::BinData & data, bool extend) { 
   // generate a bin data set from given bin center values 
   // bin center values must be present in given data set 
   //if (!IsInitialized()) { 
   if (NDim() == 0 || fFunc == 0 ) {
      MATH_WARN_MSG("DistSampler::Generate","sampler has not been initialized correctly");
      return false; 
   }
   

   int ntotbins = 1;
   for (unsigned int j = 0; j < NDim(); ++j) { 
      ntotbins *= nbins[j];
   }

   data.Initialize(ntotbins, NDim(), ROOT::Fit::BinData::kValueError);    // store always the error
   // use for the moment bin center (should use bin integral) 
   std::vector<double> dx(NDim() );
   std::vector<double> x(NDim() );
   double binVolume = 1;
   for (unsigned int j = 0; j < dx.size(); ++j) { 
      double x1 = 0,x2 = 0; 
      if (!fRange || !fRange->Size(j)) { 
         MATH_WARN_MSG("DistSampler::Generate","sampler has not a range defined for all coordinates");
         return false; 
      }
      fRange->GetRange(j,x1,x2); 
      dx[j] =  (x2-x1)/double(nbins[j]); 
      assert(dx[j] > 0 && 1./dx[j] > 0 ); // avoid dx <= 0 and  not inf
      x[j] = x1 + dx[j]/2;  // use bin centers
      binVolume *= dx[j];
   }
   double nnorm = nevt * binVolume; 

   if (extend) { 

      bool ret = true;
      for (int j = NDim()-1; j >=0; --j) { 
         for (int i = 0; i < nbins[j]; ++i) { 
            //const double * v = Sample(); 
            double val = 0; 
            double eval = 0;
            double yval = (ParentPdf())(&x.front());
            double nexp = yval * nnorm;
            ret &= SampleBin(nexp,val,&eval);
            data.Add(&x.front(), val, eval); 
            x[j] += dx[j]; // increment x bin the bin
         }
         if (!ret) { 
            MATH_WARN_MSG("DistSampler::Generate","error returned from SampleBin");
            return false; 
         } 
      }
   }
   else { 
      MATH_WARN_MSG("DistSampler::Generate","generation with fixed events not yet impelmented");
      return false; 
   }
   return true;
}
   
} // end namespace Math
} // end namespace ROOT
