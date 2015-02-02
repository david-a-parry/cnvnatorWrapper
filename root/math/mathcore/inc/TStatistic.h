// @(#)root/mathcore:$Id$
// Author: G. Ganis 2012

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TStatistic
#define ROOT_TStatistic


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TStatistic                                                           //
//                                                                      //
// Statistical variable, defined by its mean, RMS and related errors.   //
// Named, streamable, storable and mergeable.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TCollection
#include "TCollection.h"
#endif

#ifndef ROOT_TMath
#include "TMath.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TROOT
#include "TROOT.h"
#endif

class TStatistic : public TObject {

private:
   TString     fName;
   Long64_t    fN;       // Number of fills
   Double_t    fW;       // Sum of weights
   Double_t    fW2;      // Sum of weights**2
   Double_t    fM;       // Sum of elements ( i.e. mean * sum_of_weights)
   Double_t    fM2;      // Second order momentum

public:

   TStatistic(const char *name = "") : fName(name), fN(0), fW(0.), fW2(0.), fM(0.), fM2(0.) { }
   TStatistic(const char *name, Int_t n, const Double_t *val, const Double_t *w = 0);
   ~TStatistic() { }

   // Getters
   const char    *GetName() const { return fName; }
   ULong_t        Hash() const { return fName.Hash(); }

   inline       Long64_t GetN() const { return fN; }
   inline       Long64_t GetNeff() const { return fW*fW/fW2; }
   inline       Double_t GetM2() const { return fM2; }
   inline       Double_t GetMean() const { return (fW > 0) ? fM/fW : 0; }
   inline       Double_t GetMeanErr() const { return  (fW > 0.) ?  TMath::Sqrt( GetVar()/ GetNeff() ) : 0; }
   inline       Double_t GetRMS() const { double var = GetVar(); return (var>0) ? TMath::Sqrt(var) : -1; }  
   inline       Double_t GetVar() const { return (fW>0) ? ( (fN>1) ? (fM2 / fW)*(fN / (fN-1.)) : 0 ) : -1; }
   inline       Double_t GetW() const { return fW; }
   inline       Double_t GetW2() const { return fW2; }

   // Merging
   Int_t Merge(TCollection *in);

   // Fill
   void Fill(Double_t val, Double_t w = 1.);

   // Print
   void Print(Option_t * = "") const;
   void ls(Option_t *opt = "") const { Print(opt); }

   ClassDef(TStatistic,2)  //Named statistical variable
};

#endif
