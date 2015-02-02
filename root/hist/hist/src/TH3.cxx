// @(#)root/hist:$Id$
// Author: Rene Brun   27/10/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TROOT.h"
#include "TClass.h"
#include "THashList.h"
#include "TH3.h"
#include "TProfile2D.h"
#include "TH2.h"
#include "TF1.h"
#include "TVirtualPad.h"
#include "TVirtualHistPainter.h"
#include "THLimitsFinder.h"
#include "TRandom.h"
#include "TError.h"
#include "TMath.h"
#include "TObjString.h"

ClassImp(TH3)

//______________________________________________________________________________
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*
//*-*  The 3-D histogram classes derived from the 1-D histogram classes.
//*-*  all operations are supported (fill, fit).
//*-*  Drawing is currently restricted to one single option.
//*-*  A cloud of points is drawn. The number of points is proportional to
//*-*  cell content.
//*-*
//
//  TH3C a 3-D histogram with one byte per cell (char)
//  TH3S a 3-D histogram with two bytes per cell (short integer)
//  TH3I a 3-D histogram with four bytes per cell (32 bits integer)
//  TH3F a 3-D histogram with four bytes per cell (float)
//  TH3D a 3-D histogram with eight bytes per cell (double)
//
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

//______________________________________________________________________________
TH3::TH3()
{
   // Default constructor.
   fDimension   = 3;
   fTsumwy      = fTsumwy2 = fTsumwxy = 0;
   fTsumwz      = fTsumwz2 = fTsumwxz = fTsumwyz = 0;
}

//______________________________________________________________________________
TH3::TH3(const char *name,const char *title,Int_t nbinsx,Double_t xlow,Double_t xup
                                     ,Int_t nbinsy,Double_t ylow,Double_t yup
                                     ,Int_t nbinsz,Double_t zlow,Double_t zup)
     :TH1(name,title,nbinsx,xlow,xup),
      TAtt3D()
{
//*-*-*-*-*-*-*-*-*Normal constructor for fix bin size 3-D histograms*-*-*-*-*
//*-*              ==================================================

   fDimension   = 3;
   if (nbinsy <= 0) {Warning("TH3","nbinsy is <=0 - set to nbinsy = 1"); nbinsy = 1; }
   if (nbinsz <= 0) nbinsz = 1;
   fYaxis.Set(nbinsy,ylow,yup);
   fZaxis.Set(nbinsz,zlow,zup);
   fNcells      = (nbinsx+2)*(nbinsy+2)*(nbinsz+2);
   fTsumwy      = fTsumwy2 = fTsumwxy = 0;
   fTsumwz      = fTsumwz2 = fTsumwxz = fTsumwyz = 0;
}

//______________________________________________________________________________
TH3::TH3(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins
                                           ,Int_t nbinsy,const Float_t *ybins
                                           ,Int_t nbinsz,const Float_t *zbins)
     :TH1(name,title,nbinsx,xbins),
      TAtt3D()
{
//*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
//*-*            =======================================================
   fDimension   = 3;
   if (nbinsy <= 0) {Warning("TH3","nbinsy is <=0 - set to nbinsy = 1"); nbinsy = 1; }
   if (nbinsz <= 0) nbinsz = 1;
   if (ybins) fYaxis.Set(nbinsy,ybins);
   else       fYaxis.Set(nbinsy,0,1);
   if (zbins) fZaxis.Set(nbinsz,zbins);
   else       fZaxis.Set(nbinsz,0,1);
   fNcells      = (nbinsx+2)*(nbinsy+2)*(nbinsz+2);
   fTsumwy      = fTsumwy2 = fTsumwxy = 0;
   fTsumwz      = fTsumwz2 = fTsumwxz = fTsumwyz = 0;
}

//______________________________________________________________________________
TH3::TH3(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins
                                           ,Int_t nbinsy,const Double_t *ybins
                                           ,Int_t nbinsz,const Double_t *zbins)
     :TH1(name,title,nbinsx,xbins),
      TAtt3D()
{
//*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
//*-*            =======================================================
   fDimension   = 3;
   if (nbinsy <= 0) {Warning("TH3","nbinsy is <=0 - set to nbinsy = 1"); nbinsy = 1; }
   if (nbinsz <= 0) nbinsz = 1;
   if (ybins) fYaxis.Set(nbinsy,ybins);
   else       fYaxis.Set(nbinsy,0,1);
   if (zbins) fZaxis.Set(nbinsz,zbins);
   else       fZaxis.Set(nbinsz,0,1);
   fNcells      = (nbinsx+2)*(nbinsy+2)*(nbinsz+2);
   fTsumwy      = fTsumwy2 = fTsumwxy = 0;
   fTsumwz      = fTsumwz2 = fTsumwxz = fTsumwyz = 0;
}

//______________________________________________________________________________
TH3::TH3(const TH3 &h) : TH1(), TAtt3D()
{
   // Copy constructor.
   // The list of functions is not copied. (Use Clone if needed)

   ((TH3&)h).Copy(*this);
}

//______________________________________________________________________________
TH3::~TH3()
{
   // Destructor.
}

//______________________________________________________________________________
void TH3::Copy(TObject &obj) const
{
   // Copy.

   TH1::Copy(obj);
   ((TH3&)obj).fTsumwy      = fTsumwy;
   ((TH3&)obj).fTsumwy2     = fTsumwy2;
   ((TH3&)obj).fTsumwxy     = fTsumwxy;
   ((TH3&)obj).fTsumwz      = fTsumwz;
   ((TH3&)obj).fTsumwz2     = fTsumwz2;
   ((TH3&)obj).fTsumwxz     = fTsumwxz;
   ((TH3&)obj).fTsumwyz     = fTsumwyz;
}

//______________________________________________________________________________
Int_t TH3::BufferEmpty(Int_t action)
{
   // Fill histogram with all entries in the buffer.
   // action = -1 histogram is reset and refilled from the buffer (called by THistPainter::Paint)
   // action =  0 histogram is filled from the buffer
   // action =  1 histogram is filled and buffer is deleted
   //             The buffer is automatically deleted when the number of entries
   //             in the buffer is greater than the number of entries in the histogram

   // do we need to compute the bin size?
   if (!fBuffer) return 0;
   Int_t nbentries = (Int_t)fBuffer[0];
   if (!nbentries) return 0;
   Double_t *buffer = fBuffer;
   if (nbentries < 0) {
      if (action == 0) return 0;
      nbentries  = -nbentries;
      fBuffer=0;
      Reset("ICES");  
      fBuffer = buffer;
   }
   if (TestBit(kCanRebin) || fXaxis.GetXmax() <= fXaxis.GetXmin() ||
      fYaxis.GetXmax() <= fYaxis.GetXmin() ||
      fZaxis.GetXmax() <= fZaxis.GetXmin()) {
         //find min, max of entries in buffer
         Double_t xmin = fBuffer[2];
         Double_t xmax = xmin;
         Double_t ymin = fBuffer[3];
         Double_t ymax = ymin;
         Double_t zmin = fBuffer[4];
         Double_t zmax = zmin;
         for (Int_t i=1;i<nbentries;i++) {
            Double_t x = fBuffer[4*i+2];
            if (x < xmin) xmin = x;
            if (x > xmax) xmax = x;
            Double_t y = fBuffer[4*i+3];
            if (y < ymin) ymin = y;
            if (y > ymax) ymax = y;
            Double_t z = fBuffer[4*i+4];
            if (z < zmin) zmin = z;
            if (z > zmax) zmax = z;
         }
         if (fXaxis.GetXmax() <= fXaxis.GetXmin() || fYaxis.GetXmax() <= fYaxis.GetXmin() || fZaxis.GetXmax() <= fZaxis.GetXmin()) {
            THLimitsFinder::GetLimitsFinder()->FindGoodLimits(this,xmin,xmax,ymin,ymax,zmin,zmax);
         } else {
            fBuffer = 0;
            Int_t keep = fBufferSize; fBufferSize = 0;
            if (xmin <  fXaxis.GetXmin()) RebinAxis(xmin,&fXaxis);
            if (xmax >= fXaxis.GetXmax()) RebinAxis(xmax,&fXaxis);
            if (ymin <  fYaxis.GetXmin()) RebinAxis(ymin,&fYaxis);
            if (ymax >= fYaxis.GetXmax()) RebinAxis(ymax,&fYaxis);
            if (zmin <  fZaxis.GetXmin()) RebinAxis(zmin,&fZaxis);
            if (zmax >= fZaxis.GetXmax()) RebinAxis(zmax,&fZaxis);
            fBuffer = buffer;
            fBufferSize = keep;
         }
   }
   fBuffer = 0;

   for (Int_t i=0;i<nbentries;i++) {
      Fill(buffer[4*i+2],buffer[4*i+3],buffer[4*i+4],buffer[4*i+1]);
   }
   fBuffer = buffer;

   if (action > 0) { delete [] fBuffer; fBuffer = 0; fBufferSize = 0;}
   else {
      if (nbentries == (Int_t)fEntries) fBuffer[0] = -nbentries;
      else                              fBuffer[0] = 0;
   }
   return nbentries;
}

//______________________________________________________________________________
Int_t TH3::BufferFill(Double_t x, Double_t y, Double_t z, Double_t w)
{
   // accumulate arguments in buffer. When buffer is full, empty the buffer
   // fBuffer[0] = number of entries in buffer
   // fBuffer[1] = w of first entry
   // fBuffer[2] = x of first entry
   // fBuffer[3] = y of first entry
   // fBuffer[4] = z of first entry

   if (!fBuffer) return -3;
   Int_t nbentries = (Int_t)fBuffer[0];
   if (nbentries < 0) {
      nbentries  = -nbentries;
      fBuffer[0] =  nbentries;
      if (fEntries > 0) {
         Double_t *buffer = fBuffer; fBuffer=0;
         Reset("ICES");  
         fBuffer = buffer;
      }
   }
   if (4*nbentries+4 >= fBufferSize) {
      BufferEmpty(1);
      return Fill(x,y,z,w);
   }
   fBuffer[4*nbentries+1] = w;
   fBuffer[4*nbentries+2] = x;
   fBuffer[4*nbentries+3] = y;
   fBuffer[4*nbentries+4] = z;
   fBuffer[0] += 1;
   return -3;
}

//______________________________________________________________________________
Int_t TH3::Fill(Double_t )
{
   // Invalid Fill method
   Error("Fill", "Invalid signature - do nothing"); 
   return -1;
}
//______________________________________________________________________________
Int_t TH3::Fill(Double_t x, Double_t y, Double_t z)
{
   //*-*-*-*-*-*-*-*-*-*-*Increment cell defined by x,y,z by 1 *-*-*-*-*
   //*-*                  ====================================
   //*-*
   //*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   if (fBuffer) return BufferFill(x,y,z,1);

   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(x);
   biny = fYaxis.FindBin(y);
   binz = fZaxis.FindBin(z);
   if (binx <0 || biny <0 || binz<0) return -1;
   bin  =  binx + (fXaxis.GetNbins()+2)*(biny + (fYaxis.GetNbins()+2)*binz);
   AddBinContent(bin);
   if (fSumw2.fN) ++fSumw2.fArray[bin];
   if (binx == 0 || binx > fXaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }

   if (biny == 0 || biny > fYaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   if (binz == 0 || binz > fZaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   ++fTsumw;
   ++fTsumw2;
   fTsumwx  += x;
   fTsumwx2 += x*x;
   fTsumwy  += y;
   fTsumwy2 += y*y;
   fTsumwxy += x*y;
   fTsumwz  += z;
   fTsumwz2 += z*z;
   fTsumwxz += x*z;
   fTsumwyz += y*z;
   return bin;
}

//______________________________________________________________________________
Int_t TH3::Fill(Double_t x, Double_t y, Double_t z, Double_t w)
{
   //*-*-*-*-*-*-*-*-*-*-*Increment cell defined by x,y,z by a weight w*-*-*-*-*
   //*-*                  =============================================
   //*-*
   //*-* If the storage of the sum of squares of weights has been triggered,
   //*-* via the function Sumw2, then the sum of the squares of weights is incremented
   //*-* by w^2 in the cell corresponding to x,y,z.
   //*-*
   //*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

   if (fBuffer) return BufferFill(x,y,z,w);

   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(x);
   biny = fYaxis.FindBin(y);
   binz = fZaxis.FindBin(z);
   if (binx <0 || biny <0 || binz<0) return -1;
   bin  =  binx + (fXaxis.GetNbins()+2)*(biny + (fYaxis.GetNbins()+2)*binz);
   AddBinContent(bin,w);
   if (fSumw2.fN) fSumw2.fArray[bin] += w*w;
   if (binx == 0 || binx > fXaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   if (biny == 0 || biny > fYaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   if (binz == 0 || binz > fZaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   fTsumw   += w;
   fTsumw2  += w*w;
   fTsumwx  += w*x;
   fTsumwx2 += w*x*x;
   fTsumwy  += w*y;
   fTsumwy2 += w*y*y;
   fTsumwxy += w*x*y;
   fTsumwz  += w*z;
   fTsumwz2 += w*z*z;
   fTsumwxz += w*x*z;
   fTsumwyz += w*y*z;
   return bin;
}

//______________________________________________________________________________
Int_t TH3::Fill(const char *namex, const char *namey, const char *namez, Double_t w)
{
   // Increment cell defined by namex,namey,namez by a weight w
   //
   // If the storage of the sum of squares of weights has been triggered,
   // via the function Sumw2, then the sum of the squares of weights is incremented
   // by w^2 in the cell corresponding to x,y,z.
   //
   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(namex);
   biny = fYaxis.FindBin(namey);
   binz = fZaxis.FindBin(namez);
   if (binx <0 || biny <0 || binz<0) return -1;
   bin  =  binx + (fXaxis.GetNbins()+2)*(biny + (fYaxis.GetNbins()+2)*binz);
   AddBinContent(bin,w);
   if (fSumw2.fN) fSumw2.fArray[bin] += w*w;
   if (binx == 0 || binx > fXaxis.GetNbins()) return -1;
   if (biny == 0 || biny > fYaxis.GetNbins()) return -1;
   if (binz == 0 || binz > fZaxis.GetNbins()) return -1;
   Double_t x = fXaxis.GetBinCenter(binx);
   Double_t y = fYaxis.GetBinCenter(biny);
   Double_t z = fZaxis.GetBinCenter(binz);
   Double_t v = w; //(w > 0 ? w : -w);
   fTsumw   += v;
   fTsumw2  += v*v;
   fTsumwx  += v*x;
   fTsumwx2 += v*x*x;
   fTsumwy  += v*y;
   fTsumwy2 += v*y*y;
   fTsumwxy += v*x*y;
   fTsumwz  += v*z;
   fTsumwz2 += v*z*z;
   fTsumwxz += v*x*z;
   fTsumwyz += v*y*z;
   return bin;
}

//______________________________________________________________________________
Int_t TH3::Fill(const char *namex, Double_t y, const char *namez, Double_t w)
{
   // Increment cell defined by namex,y,namez by a weight w
   //
   // If the storage of the sum of squares of weights has been triggered,
   // via the function Sumw2, then the sum of the squares of weights is incremented
   // by w^2 in the cell corresponding to x,y,z.
   //
   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(namex);
   biny = fYaxis.FindBin(y);
   binz = fZaxis.FindBin(namez);
   if (binx <0 || biny <0 || binz<0) return -1;
   bin  =  binx + (fXaxis.GetNbins()+2)*(biny + (fYaxis.GetNbins()+2)*binz);
   AddBinContent(bin,w);
   if (fSumw2.fN) fSumw2.fArray[bin] += w*w;
   if (binx == 0 || binx > fXaxis.GetNbins()) return -1;
   if (biny == 0 || biny > fYaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   if (binz == 0 || binz > fZaxis.GetNbins()) return -1;
   Double_t x = fXaxis.GetBinCenter(binx);
   Double_t z = fZaxis.GetBinCenter(binz);
   Double_t v = w; //(w > 0 ? w : -w);
   fTsumw   += v;
   fTsumw2  += v*v;
   fTsumwx  += v*x;
   fTsumwx2 += v*x*x;
   fTsumwy  += v*y;
   fTsumwy2 += v*y*y;
   fTsumwxy += v*x*y;
   fTsumwz  += v*z;
   fTsumwz2 += v*z*z;
   fTsumwxz += v*x*z;
   fTsumwyz += v*y*z;
   return bin;
}

//______________________________________________________________________________
Int_t TH3::Fill(const char *namex, const char *namey, Double_t z, Double_t w)
{
   // Increment cell defined by namex,namey,z by a weight w
   //
   // If the storage of the sum of squares of weights has been triggered,
   // via the function Sumw2, then the sum of the squares of weights is incremented
   // by w^2 in the cell corresponding to x,y,z.
   //
   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(namex);
   biny = fYaxis.FindBin(namey);
   binz = fZaxis.FindBin(z);
   if (binx <0 || biny <0 || binz<0) return -1;
   bin  =  binx + (fXaxis.GetNbins()+2)*(biny + (fYaxis.GetNbins()+2)*binz);
   AddBinContent(bin,w);
   if (fSumw2.fN) fSumw2.fArray[bin] += w*w;
   if (binx == 0 || binx > fXaxis.GetNbins()) return -1;
   if (biny == 0 || biny > fYaxis.GetNbins()) return -1;
   if (binz == 0 || binz > fZaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   Double_t x = fXaxis.GetBinCenter(binx);
   Double_t y = fYaxis.GetBinCenter(biny);
   Double_t v = w; // (w > 0 ? w : -w);
   fTsumw   += v;
   fTsumw2  += v*v;
   fTsumwx  += v*x;
   fTsumwx2 += v*x*x;
   fTsumwy  += v*y;
   fTsumwy2 += v*y*y;
   fTsumwxy += v*x*y;
   fTsumwz  += v*z;
   fTsumwz2 += v*z*z;
   fTsumwxz += v*x*z;
   fTsumwyz += v*y*z;
   return bin;
}

//______________________________________________________________________________
Int_t TH3::Fill(Double_t x, const char *namey, const char *namez, Double_t w)
{
   // Increment cell defined by x,namey,namezz by a weight w
   //
   // If the storage of the sum of squares of weights has been triggered,
   // via the function Sumw2, then the sum of the squares of weights is incremented
   // by w^2 in the cell corresponding to x,y,z.
   //
   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(x);
   biny = fYaxis.FindBin(namey);
   binz = fZaxis.FindBin(namez);
   if (binx <0 || biny <0 || binz<0) return -1;
   bin  =  binx + (fXaxis.GetNbins()+2)*(biny + (fYaxis.GetNbins()+2)*binz);
   AddBinContent(bin,w);
   if (fSumw2.fN) fSumw2.fArray[bin] += w*w;
   if (binx == 0 || binx > fXaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   if (biny == 0 || biny > fYaxis.GetNbins()) return -1;
   if (binz == 0 || binz > fZaxis.GetNbins()) return -1;
   Double_t y = fYaxis.GetBinCenter(biny);
   Double_t z = fZaxis.GetBinCenter(binz);
   Double_t v = w; //(w > 0 ? w : -w);
   fTsumw   += v;
   fTsumw2  += v*v;
   fTsumwx  += v*x;
   fTsumwx2 += v*x*x;
   fTsumwy  += v*y;
   fTsumwy2 += v*y*y;
   fTsumwxy += v*x*y;
   fTsumwz  += v*z;
   fTsumwz2 += v*z*z;
   fTsumwxz += v*x*z;
   fTsumwyz += v*y*z;
   return bin;
}

//______________________________________________________________________________
Int_t TH3::Fill(Double_t x, const char *namey, Double_t z, Double_t w)
{
   // Increment cell defined by x,namey,z by a weight w
   //
   // If the storage of the sum of squares of weights has been triggered,
   // via the function Sumw2, then the sum of the squares of weights is incremented
   // by w^2 in the cell corresponding to x,y,z.
   //
   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(x);
   biny = fYaxis.FindBin(namey);
   binz = fZaxis.FindBin(z);
   if (binx <0 || biny <0 || binz<0) return -1;
   bin  =  binx + (fXaxis.GetNbins()+2)*(biny + (fYaxis.GetNbins()+2)*binz);
   AddBinContent(bin,w);
   if (fSumw2.fN) fSumw2.fArray[bin] += w*w;
   if (binx == 0 || binx > fXaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   if (biny == 0 || biny > fYaxis.GetNbins()) return -1;
   if (binz == 0 || binz > fZaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   Double_t y = fYaxis.GetBinCenter(biny);
   Double_t v = w; // (w > 0 ? w : -w);
   fTsumw   += v;
   fTsumw2  += v*v;
   fTsumwx  += v*x;
   fTsumwx2 += v*x*x;
   fTsumwy  += v*y;
   fTsumwy2 += v*y*y;
   fTsumwxy += v*x*y;
   fTsumwz  += v*z;
   fTsumwz2 += v*z*z;
   fTsumwxz += v*x*z;
   fTsumwyz += v*y*z;
   return bin;
}

//______________________________________________________________________________
Int_t TH3::Fill(Double_t x, Double_t y, const char *namez, Double_t w)
{
   // Increment cell defined by x,y,namez by a weight w
   //
   // If the storage of the sum of squares of weights has been triggered,
   // via the function Sumw2, then the sum of the squares of weights is incremented
   // by w^2 in the cell corresponding to x,y,z.
   //
   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(x);
   biny = fYaxis.FindBin(y);
   binz = fZaxis.FindBin(namez);
   if (binx <0 || biny <0 || binz<0) return -1;
   bin  =  binx + (fXaxis.GetNbins()+2)*(biny + (fYaxis.GetNbins()+2)*binz);
   AddBinContent(bin,w);
   if (fSumw2.fN) fSumw2.fArray[bin] += w*w;
   if (binx == 0 || binx > fXaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   if (biny == 0 || biny > fYaxis.GetNbins()) {
      if (!fgStatOverflows) return -1;
   }
   if (binz == 0 || binz > fZaxis.GetNbins()) return -1;
   Double_t z = fZaxis.GetBinCenter(binz);
   Double_t v = w; //(w > 0 ? w : -w);
   fTsumw   += v;
   fTsumw2  += v*v;
   fTsumwx  += v*x;
   fTsumwx2 += v*x*x;
   fTsumwy  += v*y;
   fTsumwy2 += v*y*y;
   fTsumwxy += v*x*y;
   fTsumwz  += v*z;
   fTsumwz2 += v*z*z;
   fTsumwxz += v*x*z;
   fTsumwyz += v*y*z;
   return bin;
}

//______________________________________________________________________________
void TH3::FillRandom(const char *fname, Int_t ntimes)
{
   //*-*-*-*-*-*-*Fill histogram following distribution in function fname*-*-*-*
   //*-*          =======================================================
   //*-*
   //*-*   The distribution contained in the function fname (TF1) is integrated
   //*-*   over the channel contents.
   //*-*   It is normalized to 1.
   //*-*   Getting one random number implies:
   //*-*     - Generating a random number between 0 and 1 (say r1)
   //*-*     - Look in which bin in the normalized integral r1 corresponds to
   //*-*     - Fill histogram channel
   //*-*   ntimes random numbers are generated
   //*-*
   //*-*  One can also call TF1::GetRandom to get a random variate from a function.
   //*-*
   //*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*

   Int_t bin, binx, biny, binz, ibin, loop;
   Double_t r1, x, y,z, xv[3];
   //*-*- Search for fname in the list of ROOT defined functions
   TF1 *f1 = (TF1*)gROOT->GetFunction(fname);
   if (!f1) { Error("FillRandom", "Unknown function: %s",fname); return; }

   //*-*- Allocate temporary space to store the integral and compute integral
   Int_t nbinsx = GetNbinsX();
   Int_t nbinsy = GetNbinsY();
   Int_t nbinsz = GetNbinsZ();
   Int_t nxy    = nbinsx*nbinsy;
   Int_t nbins  = nxy*nbinsz;

   Double_t *integral = new Double_t[nbins+1];
   ibin = 0;
   integral[ibin] = 0;
   for (binz=1;binz<=nbinsz;binz++) {
      xv[2] = fZaxis.GetBinCenter(binz);
      for (biny=1;biny<=nbinsy;biny++) {
         xv[1] = fYaxis.GetBinCenter(biny);
         for (binx=1;binx<=nbinsx;binx++) {
            xv[0] = fXaxis.GetBinCenter(binx);
            ibin++;
            integral[ibin] = integral[ibin-1] + f1->Eval(xv[0],xv[1],xv[2]);
         }
      }
   }

   //*-*- Normalize integral to 1
   if (integral[nbins] == 0 ) {
      delete [] integral;
      Error("FillRandom", "Integral = zero"); return;
   }
   for (bin=1;bin<=nbins;bin++)  integral[bin] /= integral[nbins];

   //*-*--------------Start main loop ntimes
   if (fDimension < 2) nbinsy = -1;
   if (fDimension < 3) nbinsz = -1;
   for (loop=0;loop<ntimes;loop++) {
      r1 = gRandom->Rndm(loop);
      ibin = TMath::BinarySearch(nbins,&integral[0],r1);
      binz = ibin/nxy;
      biny = (ibin - nxy*binz)/nbinsx;
      binx = 1 + ibin - nbinsx*(biny + nbinsy*binz);
      if (nbinsz) binz++;
      if (nbinsy) biny++;
      x    = fXaxis.GetBinCenter(binx);
      y    = fYaxis.GetBinCenter(biny);
      z    = fZaxis.GetBinCenter(binz);
      Fill(x,y,z, 1.);
   }
   delete [] integral;
}

//______________________________________________________________________________
void TH3::FillRandom(TH1 *h, Int_t ntimes)
{
   //*-*-*-*-*-*-*Fill histogram following distribution in histogram h*-*-*-*
   //*-*          ====================================================
   //*-*
   //*-*   The distribution contained in the histogram h (TH3) is integrated
   //*-*   over the channel contents.
   //*-*   It is normalized to 1.
   //*-*   Getting one random number implies:
   //*-*     - Generating a random number between 0 and 1 (say r1)
   //*-*     - Look in which bin in the normalized integral r1 corresponds to
   //*-*     - Fill histogram channel
   //*-*   ntimes random numbers are generated
   //*-*
   //*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*

   if (!h) { Error("FillRandom", "Null histogram"); return; }
   if (fDimension != h->GetDimension()) {
      Error("FillRandom", "Histograms with different dimensions"); return;
   }

   if (h->ComputeIntegral() == 0) return;

   TH3 *h3 = (TH3*)h;
   Int_t loop;
   Double_t x,y,z;
   for (loop=0;loop<ntimes;loop++) {
      h3->GetRandom3(x,y,z);
      Fill(x,y,z,1.);
   }
}


//______________________________________________________________________________
Int_t TH3::FindFirstBinAbove(Double_t threshold, Int_t axis) const
{
   //find first bin with content > threshold for axis (1=x, 2=y, 3=z)
   //if no bins with content > threshold is found the function returns -1.
   
   if (axis < 1 || axis > 3) {
      Warning("FindFirstBinAbove","Invalid axis number : %d, axis x assumed\n",axis);
      axis = 1;
   }
   Int_t nbinsx = fXaxis.GetNbins();
   Int_t nbinsy = fYaxis.GetNbins();
   Int_t nbinsz = fZaxis.GetNbins();
   Int_t binx, biny, binz;
   if (axis == 1) {
      for (binx=1;binx<=nbinsx;binx++) {
         for (biny=1;biny<=nbinsy;biny++) {
            for (binz=1;binz<=nbinsz;binz++) {
               if (GetBinContent(binx,biny,binz) > threshold) return binx;
            }
         }
      }
   } else if (axis == 2) {
      for (biny=1;biny<=nbinsy;biny++) {
         for (binx=1;binx<=nbinsx;binx++) {
            for (binz=1;binz<=nbinsz;binz++) {
               if (GetBinContent(binx,biny,binz) > threshold) return biny;
            }
         }
      }
   } else {
      for (binz=1;binz<=nbinsz;binz++) {
         for (binx=1;binx<=nbinsx;binx++) {
            for (biny=1;biny<=nbinsy;biny++) {
               if (GetBinContent(binx,biny,binz) > threshold) return binz;
            }
         }
      }
   }
   return -1;
}


//______________________________________________________________________________
Int_t TH3::FindLastBinAbove(Double_t threshold, Int_t axis) const
{
   //find last bin with content > threshold for axis (1=x, 2=y, 3=z)
   //if no bins with content > threshold is found the function returns -1.
   
   if (axis < 1 || axis > 3) {
      Warning("FindLastBinAbove","Invalid axis number : %d, axis x assumed\n",axis);
      axis = 1;
   }
   Int_t nbinsx = fXaxis.GetNbins();
   Int_t nbinsy = fYaxis.GetNbins();
   Int_t nbinsz = fZaxis.GetNbins();
   Int_t binx, biny, binz;
   if (axis == 1) {
      for (binx=nbinsx;binx>=1;binx--) {
         for (biny=1;biny<=nbinsy;biny++) {
            for (binz=1;binz<=nbinsz;binz++) {
               if (GetBinContent(binx,biny,binz) > threshold) return binx;
            }
         }
      }
   } else if (axis == 2) {
      for (biny=nbinsy;biny>=1;biny--) {
         for (binx=1;binx<=nbinsx;binx++) {
            for (binz=1;binz<=nbinsz;binz++) {
               if (GetBinContent(binx,biny,binz) > threshold) return biny;
            }
         }
      }
   } else {
      for (binz=nbinsz;binz>=1;binz--) {
         for (binx=1;binx<=nbinsx;binx++) {
            for (biny=1;biny<=nbinsy;biny++) {
               if (GetBinContent(binx,biny,binz) > threshold) return binz;
            }
         }
      }
   }
   return -1;
}


//______________________________________________________________________________
void TH3::FitSlicesZ(TF1 *f1, Int_t binminx, Int_t binmaxx, Int_t binminy, Int_t binmaxy, Int_t cut, Option_t *option)
{
   // Project slices along Z in case of a 3-D histogram, then fit each slice
   // with function f1 and make a 2-d histogram for each fit parameter
   // Only cells in the bin range [binminx,binmaxx] and [binminy,binmaxy] are considered.
   // if f1=0, a gaussian is assumed
   // Before invoking this function, one can set a subrange to be fitted along Z
   // via f1->SetRange(zmin,zmax)
   // The argument option (default="QNR") can be used to change the fit options.
   //     "Q" means Quiet mode
   //     "N" means do not show the result of the fit
   //     "R" means fit the function in the specified function range
   //
   // Note that the generated histograms are added to the list of objects
   // in the current directory. It is the user's responsability to delete
   // these histograms.
   //
   //  Example: Assume a 3-d histogram h3
   //   Root > h3->FitSlicesZ(); produces 4 TH2D histograms
   //          with h3_0 containing parameter 0(Constant) for a Gaus fit
   //                    of each cell in X,Y projected along Z
   //          with h3_1 containing parameter 1(Mean) for a gaus fit
   //          with h3_2 containing parameter 2(RMS)  for a gaus fit
   //          with h3_chi2 containing the chisquare/number of degrees of freedom for a gaus fit
   //
   //   Root > h3->Fit(0,15,22,0,0,10);
   //          same as above, but only for bins 15 to 22 along X
   //          and only for cells in X,Y for which the corresponding projection
   //          along Z has more than cut bins filled.
   //
   //  NOTE: To access the generated histograms in the current directory, do eg:
   //     TH2D *h3_1 = (TH2D*)gDirectory->Get("h3_1");

   Int_t nbinsx  = fXaxis.GetNbins();
   Int_t nbinsy  = fYaxis.GetNbins();
   Int_t nbinsz  = fZaxis.GetNbins();
   if (binminx < 1) binminx = 1;
   if (binmaxx > nbinsx) binmaxx = nbinsx;
   if (binmaxx < binminx) {binminx = 1; binmaxx = nbinsx;}
   if (binminy < 1) binminy = 1;
   if (binmaxy > nbinsy) binmaxy = nbinsy;
   if (binmaxy < binminy) {binminy = 1; binmaxy = nbinsy;}

   //default is to fit with a gaussian
   if (f1 == 0) {
      f1 = (TF1*)gROOT->GetFunction("gaus");
      if (f1 == 0) f1 = new TF1("gaus","gaus",fZaxis.GetXmin(),fZaxis.GetXmax());
      else         f1->SetRange(fZaxis.GetXmin(),fZaxis.GetXmax());
   }
   const char *fname = f1->GetName();
   Int_t npar = f1->GetNpar();
   Double_t *parsave = new Double_t[npar];
   f1->GetParameters(parsave);

   //Create one 2-d histogram for each function parameter
   Int_t ipar;
   char name[80], title[80];
   TH2D *hlist[25];
   const TArrayD *xbins = fXaxis.GetXbins();
   const TArrayD *ybins = fYaxis.GetXbins();
   for (ipar=0;ipar<npar;ipar++) {
      snprintf(name,80,"%s_%d",GetName(),ipar);
      snprintf(title,80,"Fitted value of par[%d]=%s",ipar,f1->GetParName(ipar));
      if (xbins->fN == 0) {
         hlist[ipar] = new TH2D(name, title,
                                nbinsx, fXaxis.GetXmin(), fXaxis.GetXmax(),
                                nbinsy, fYaxis.GetXmin(), fYaxis.GetXmax());
      } else {
         hlist[ipar] = new TH2D(name, title,
                                nbinsx, xbins->fArray,
                                nbinsy, ybins->fArray);
      }
      hlist[ipar]->GetXaxis()->SetTitle(fXaxis.GetTitle());
      hlist[ipar]->GetYaxis()->SetTitle(fYaxis.GetTitle());
   }
   snprintf(name,80,"%s_chi2",GetName());
   TH2D *hchi2 = new TH2D(name,"chisquare", nbinsx, fXaxis.GetXmin(), fXaxis.GetXmax()
      , nbinsy, fYaxis.GetXmin(), fYaxis.GetXmax());

   //Loop on all cells in X,Y generate a projection along Z
   TH1D *hpz = new TH1D("R_temp","_temp",nbinsz, fZaxis.GetXmin(), fZaxis.GetXmax());
   Int_t bin,binx,biny,binz;
   for (biny=binminy;biny<=binmaxy;biny++) {
      Float_t y = fYaxis.GetBinCenter(biny);
      for (binx=binminx;binx<=binmaxx;binx++) {
         Float_t x = fXaxis.GetBinCenter(binx);
         hpz->Reset();
         Int_t nfill = 0;
         for (binz=1;binz<=nbinsz;binz++) {
            bin = GetBin(binx,biny,binz);
            Float_t w = GetBinContent(bin);
            if (w == 0) continue;
            hpz->Fill(fZaxis.GetBinCenter(binz),w);
            hpz->SetBinError(binz,GetBinError(bin));
            nfill++;
         }
         if (nfill < cut) continue;
         f1->SetParameters(parsave);
         hpz->Fit(fname,option);
         Int_t npfits = f1->GetNumberFitPoints();
         if (npfits > npar && npfits >= cut) {
            for (ipar=0;ipar<npar;ipar++) {
               hlist[ipar]->Fill(x,y,f1->GetParameter(ipar));
               hlist[ipar]->SetCellError(binx,biny,f1->GetParError(ipar));
            }
            hchi2->Fill(x,y,f1->GetChisquare()/(npfits-npar));
         }
      }
   }
   delete [] parsave;
   delete hpz;
}

//______________________________________________________________________________
Double_t TH3::GetBinWithContent3(Double_t c, Int_t &binx, Int_t &biny, Int_t &binz, Int_t firstx, Int_t lastx, Int_t firsty, Int_t lasty, Int_t firstz, Int_t lastz, Double_t maxdiff) const
{
   // compute first cell (binx,biny,binz) in the range [firstx,lastx](firsty,lasty][firstz,lastz] for which
   // diff = abs(cell_content-c) <= maxdiff
   // In case several cells in the specified range with diff=0 are found
   // the first cell found is returned in binx,biny,binz.
   // In case several cells in the specified range satisfy diff <=maxdiff
   // the cell with the smallest difference is returned in binx,biny,binz.
   // In all cases the function returns the smallest difference.
   //
   // NOTE1: if firstx <= 0, firstx is set to bin 1
   //        if (lastx < firstx then firstx is set to the number of bins in X
   //        ie if firstx=0 and lastx=0 (default) the search is on all bins in X.
   //        if firsty <= 0, firsty is set to bin 1
   //        if (lasty < firsty then firsty is set to the number of bins in Y
   //        ie if firsty=0 and lasty=0 (default) the search is on all bins in Y.
   //        if firstz <= 0, firstz is set to bin 1
   //        if (lastz < firstz then firstz is set to the number of bins in Z
   //        ie if firstz=0 and lastz=0 (default) the search is on all bins in Z.
   // NOTE2: if maxdiff=0 (default), the first cell with content=c is returned.

   if (fDimension != 3) {
      binx = 0;
      biny = 0;
      binz = 0;
      Error("GetBinWithContent3","function is only valid for 3-D histograms");
      return 0;
   }
   if (firstx <= 0) firstx = 1;
   if (lastx < firstx) lastx = fXaxis.GetNbins();
   if (firsty <= 0) firsty = 1;
   if (lasty < firsty) lasty = fYaxis.GetNbins();
   if (firstz <= 0) firstz = 1;
   if (lastz < firstz) lastz = fZaxis.GetNbins();
   Int_t binminx = 0, binminy=0, binminz=0;
   Double_t diff, curmax = 1.e240;
   for (Int_t k=firstz;k<=lastz;k++) {
      for (Int_t j=firsty;j<=lasty;j++) {
         for (Int_t i=firstx;i<=lastx;i++) {
            diff = TMath::Abs(GetBinContent(i,j,k)-c);
            if (diff <= 0) {binx = i; biny=j; binz=k; return diff;}
            if (diff < curmax && diff <= maxdiff) {curmax = diff, binminx=i; binminy=j;binminz=k;}
         }
      }
   }
   binx = binminx;
   biny = binminy;
   binz = binminz;
   return curmax;
}

//______________________________________________________________________________
Double_t TH3::GetCorrelationFactor(Int_t axis1, Int_t axis2) const
{
   //*-*-*-*-*-*-*-*Return correlation factor between axis1 and axis2*-*-*-*-*
   //*-*            ====================================================
   if (axis1 < 1 || axis2 < 1 || axis1 > 3 || axis2 > 3) {
      Error("GetCorrelationFactor","Wrong parameters");
      return 0;
   }
   if (axis1 == axis2) return 1;
   Double_t rms1 = GetRMS(axis1);
   if (rms1 == 0) return 0;
   Double_t rms2 = GetRMS(axis2);
   if (rms2 == 0) return 0;
   return GetCovariance(axis1,axis2)/rms1/rms2;
}

//______________________________________________________________________________
Double_t TH3::GetCovariance(Int_t axis1, Int_t axis2) const
{
   //*-*-*-*-*-*-*-*Return covariance between axis1 and axis2*-*-*-*-*
   //*-*            ====================================================

   if (axis1 < 1 || axis2 < 1 || axis1 > 3 || axis2 > 3) {
      Error("GetCovariance","Wrong parameters");
      return 0;
   }
   Double_t stats[kNstat];
   GetStats(stats);
   Double_t sumw   = stats[0];
   Double_t sumw2  = stats[1];
   Double_t sumwx  = stats[2];
   Double_t sumwx2 = stats[3];
   Double_t sumwy  = stats[4];
   Double_t sumwy2 = stats[5];
   Double_t sumwxy = stats[6];
   Double_t sumwz  = stats[7];
   Double_t sumwz2 = stats[8];
   Double_t sumwxz = stats[9];
   Double_t sumwyz = stats[10];

   if (sumw == 0) return 0;
   if (axis1 == 1 && axis2 == 1) {
      return TMath::Abs(sumwx2/sumw - sumwx*sumwx/sumw2);
   }
   if (axis1 == 2 && axis2 == 2) {
      return TMath::Abs(sumwy2/sumw - sumwy*sumwy/sumw2);
   }
   if (axis1 == 3 && axis2 == 3) {
      return TMath::Abs(sumwz2/sumw - sumwz*sumwz/sumw2);
   }
   if ((axis1 == 1 && axis2 == 2) || (axis1 == 2 && axis2 == 1)) {
      return sumwxy/sumw - sumwx/sumw*sumwy/sumw;
   }
   if ((axis1 == 1 && axis2 == 3) || (axis1 == 3 && axis2 == 1)) {
      return sumwxz/sumw - sumwx/sumw*sumwz/sumw;
   }
   if ((axis1 == 2 && axis2 == 3) || (axis1 == 3 && axis2 == 2)) {
      return sumwyz/sumw - sumwy/sumw*sumwz/sumw;
   }
   return 0;
}


//______________________________________________________________________________
void TH3::GetRandom3(Double_t &x, Double_t &y, Double_t &z)
{
   // return 3 random numbers along axis x , y and z distributed according
   // the cellcontents of a 3-dim histogram

   Int_t nbinsx = GetNbinsX();
   Int_t nbinsy = GetNbinsY();
   Int_t nbinsz = GetNbinsZ();
   Int_t nxy    = nbinsx*nbinsy;
   Int_t nbins  = nxy*nbinsz;
   Double_t integral;
   // compute integral checking that all bins have positive content (see ROOT-5894)
   if (fIntegral) {
      if (fIntegral[nbins+1] != fEntries) integral = ComputeIntegral(true);
      else integral = fIntegral[nbins];
   } else {
      integral = ComputeIntegral(true);
   }
   if (integral == 0 ) { x = 0; y = 0; z = 0; return;}
   // case histogram has negative bins
   if (integral == TMath::QuietNaN() ) { x = TMath::QuietNaN(); y = TMath::QuietNaN(); z = TMath::QuietNaN(); return;}

   Double_t r1 = gRandom->Rndm();
   Int_t ibin = TMath::BinarySearch(nbins,fIntegral,(Double_t) r1);
   Int_t binz = ibin/nxy;
   Int_t biny = (ibin - nxy*binz)/nbinsx;
   Int_t binx = ibin - nbinsx*(biny + nbinsy*binz);
   x = fXaxis.GetBinLowEdge(binx+1);
   if (r1 > fIntegral[ibin]) x +=
      fXaxis.GetBinWidth(binx+1)*(r1-fIntegral[ibin])/(fIntegral[ibin+1] - fIntegral[ibin]);
   y = fYaxis.GetBinLowEdge(biny+1) + fYaxis.GetBinWidth(biny+1)*gRandom->Rndm();
   z = fZaxis.GetBinLowEdge(binz+1) + fZaxis.GetBinWidth(binz+1)*gRandom->Rndm();
}

//______________________________________________________________________________
void TH3::GetStats(Double_t *stats) const
{
   // fill the array stats from the contents of this histogram
   // The array stats must be correctly dimensionned in the calling program.
   // stats[0] = sumw
   // stats[1] = sumw2
   // stats[2] = sumwx
   // stats[3] = sumwx2
   // stats[4] = sumwy
   // stats[5] = sumwy2
   // stats[6] = sumwxy
   // stats[7] = sumwz
   // stats[8] = sumwz2
   // stats[9] = sumwxz
   // stats[10]= sumwyz

   if (fBuffer) ((TH3*)this)->BufferEmpty();

   Int_t bin, binx, biny, binz;
   Double_t w,err;
   Double_t x,y,z;
   if ((fTsumw == 0 && fEntries > 0) || fXaxis.TestBit(TAxis::kAxisRange) || fYaxis.TestBit(TAxis::kAxisRange) || fZaxis.TestBit(TAxis::kAxisRange)) {
      for (bin=0;bin<9;bin++) stats[bin] = 0;

      Int_t firstBinX = fXaxis.GetFirst();
      Int_t lastBinX  = fXaxis.GetLast();
      Int_t firstBinY = fYaxis.GetFirst();
      Int_t lastBinY  = fYaxis.GetLast();
      Int_t firstBinZ = fZaxis.GetFirst();
      Int_t lastBinZ  = fZaxis.GetLast();
      // include underflow/overflow if TH1::StatOverflows(kTRUE) in case no range is set on the axis
      if (fgStatOverflows) {
         if ( !fXaxis.TestBit(TAxis::kAxisRange) ) {
            if (firstBinX == 1) firstBinX = 0;
            if (lastBinX ==  fXaxis.GetNbins() ) lastBinX += 1;
         }
         if ( !fYaxis.TestBit(TAxis::kAxisRange) ) {
            if (firstBinY == 1) firstBinY = 0;
            if (lastBinY ==  fYaxis.GetNbins() ) lastBinY += 1;
         }
         if ( !fZaxis.TestBit(TAxis::kAxisRange) ) {
            if (firstBinZ == 1) firstBinZ = 0;
            if (lastBinZ ==  fZaxis.GetNbins() ) lastBinZ += 1;
         }
      }
      for (binz = firstBinZ; binz <= lastBinZ; binz++) {
         z = fZaxis.GetBinCenter(binz);
         for (biny = firstBinY; biny <= lastBinY; biny++) {
            y = fYaxis.GetBinCenter(biny);
            for (binx = firstBinX; binx <= lastBinX; binx++) {
               bin = GetBin(binx,biny,binz);
               x   = fXaxis.GetBinCenter(binx);
               //w   = TMath::Abs(GetBinContent(bin));
               w   = GetBinContent(bin);
               err = TMath::Abs(GetBinError(bin));
               stats[0] += w;
               stats[1] += err*err;
               stats[2] += w*x;
               stats[3] += w*x*x;
               stats[4] += w*y;
               stats[5] += w*y*y;
               stats[6] += w*x*y;
               stats[7] += w*z;
               stats[8] += w*z*z;
               stats[9] += w*x*z;
               stats[10]+= w*y*z;
            }
         }
      }
   } else {
      stats[0] = fTsumw;
      stats[1] = fTsumw2;
      stats[2] = fTsumwx;
      stats[3] = fTsumwx2;
      stats[4] = fTsumwy;
      stats[5] = fTsumwy2;
      stats[6] = fTsumwxy;
      stats[7] = fTsumwz;
      stats[8] = fTsumwz2;
      stats[9] = fTsumwxz;
      stats[10]= fTsumwyz;
   }
}

//______________________________________________________________________________
Double_t TH3::Integral(Option_t *option) const
{
   //Return integral of bin contents. Only bins in the bins range are considered.
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x, y and in z.

   return Integral(fXaxis.GetFirst(),fXaxis.GetLast(),
      fYaxis.GetFirst(),fYaxis.GetLast(),
      fZaxis.GetFirst(),fZaxis.GetLast(),option);
}

//______________________________________________________________________________
Double_t TH3::Integral(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2, Option_t *option) const
{
   //Return integral of bin contents in range [binx1,binx2],[biny1,biny2],[binz1,binz2]
   // for a 3-D histogram
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x, y and in z.

   Double_t err = 0; 
   return DoIntegral(binx1,binx2,biny1,biny2,binz1,binz2,err,option);
}
//______________________________________________________________________________
Double_t TH3::IntegralAndError(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2, Double_t & error, Option_t *option) const
{
   //Return integral of bin contents in range [binx1,binx2],[biny1,biny2],[binz1,binz2]
   // for a 3-D histogram. Calculates also the integral error using error propagation 
   // from the bin errors assumming that all the bins are uncorrelated. 
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x, y and in z.
   return DoIntegral(binx1,binx2,biny1,biny2,binz1,binz2,error,option,kTRUE);
}

//______________________________________________________________________________
Double_t TH3::Interpolate(Double_t)
{
   
   //Not yet implemented
   Error("Interpolate","This function must be called with 3 arguments for a TH3");
   return 0;
}

//______________________________________________________________________________
Double_t TH3::Interpolate(Double_t, Double_t)
{
   
   //Not yet implemented
   Error("Interpolate","This function must be called with 3 arguments for a TH3");
   return 0;
}

//______________________________________________________________________________
Double_t TH3::Interpolate(Double_t x, Double_t y, Double_t z)
{
   // Given a point P(x,y,z), Interpolate approximates the value via trilinear interpolation
   // based on the 8 nearest bin center points ( corner of the cube surronding the points) 
   // The Algorithm is described in http://en.wikipedia.org/wiki/Trilinear_interpolation
   // The given values (x,y,z) must be between first bin center and  last bin center for each coordinate: 
   //
   //   fXAxis.GetBinCenter(1) < x  < fXaxis.GetBinCenter(nbinX)     AND
   //   fYAxis.GetBinCenter(1) < y  < fYaxis.GetBinCenter(nbinY)     AND
   //   fZAxis.GetBinCenter(1) < z  < fZaxis.GetBinCenter(nbinZ) 

   Int_t ubx = fXaxis.FindBin(x);
   if ( x < fXaxis.GetBinCenter(ubx) ) ubx -= 1;
   Int_t obx = ubx + 1;

   Int_t uby = fYaxis.FindBin(y);
   if ( y < fYaxis.GetBinCenter(uby) ) uby -= 1;
   Int_t oby = uby + 1;

   Int_t ubz = fZaxis.FindBin(z);
   if ( z < fZaxis.GetBinCenter(ubz) ) ubz -= 1;
   Int_t obz = ubz + 1;


//    if ( IsBinUnderflow(GetBin(ubx, uby, ubz)) ||
//         IsBinOverflow (GetBin(obx, oby, obz)) ) {
   if (ubx <=0 || uby <=0 || ubz <= 0 ||
       obx > fXaxis.GetNbins() || oby > fYaxis.GetNbins() || obz > fZaxis.GetNbins() ) {
      Error("Interpolate","Cannot interpolate outside histogram domain.");
      return 0;
   }

   Double_t xw = fXaxis.GetBinCenter(obx) - fXaxis.GetBinCenter(ubx);
   Double_t yw = fYaxis.GetBinCenter(oby) - fYaxis.GetBinCenter(uby);
   Double_t zw = fZaxis.GetBinCenter(obz) - fZaxis.GetBinCenter(ubz);

   Double_t xd = (x - fXaxis.GetBinCenter(ubx)) / xw;
   Double_t yd = (y - fYaxis.GetBinCenter(uby)) / yw;
   Double_t zd = (z - fZaxis.GetBinCenter(ubz)) / zw;


   Double_t v[] = { GetBinContent( ubx, uby, ubz ), GetBinContent( ubx, uby, obz ),
                    GetBinContent( ubx, oby, ubz ), GetBinContent( ubx, oby, obz ),
                    GetBinContent( obx, uby, ubz ), GetBinContent( obx, uby, obz ),
                    GetBinContent( obx, oby, ubz ), GetBinContent( obx, oby, obz ) };


   Double_t i1 = v[0] * (1 - zd) + v[1] * zd;
   Double_t i2 = v[2] * (1 - zd) + v[3] * zd;
   Double_t j1 = v[4] * (1 - zd) + v[5] * zd;
   Double_t j2 = v[6] * (1 - zd) + v[7] * zd;


   Double_t w1 = i1 * (1 - yd) + i2 * yd;
   Double_t w2 = j1 * (1 - yd) + j2 * yd;


   Double_t result = w1 * (1 - xd) + w2 * xd;

   return result;
}

//______________________________________________________________________________
Double_t TH3::KolmogorovTest(const TH1 *h2, Option_t *option) const
{
   //  Statistical test of compatibility in shape between
   //  THIS histogram and h2, using Kolmogorov test.
   //     Default: Ignore under- and overflow bins in comparison
   //
   //     option is a character string to specify options
   //         "U" include Underflows in test
   //         "O" include Overflows
   //         "N" include comparison of normalizations
   //         "D" Put out a line of "Debug" printout
   //         "M" Return the Maximum Kolmogorov distance instead of prob
   //
   //   The returned function value is the probability of test
   //       (much less than one means NOT compatible)
   //
   //   The KS test uses the distance between the pseudo-CDF's obtained 
   //   from the histogram. Since in more than 1D the order for generating the pseudo-CDF is 
   //   arbitrary, we use the pseudo-CDF's obtained from all the possible 6 combinatons of the 3 axis. 
   //   The average of all the maximum  distances obtained is used in the tests.  

   TString opt = option;
   opt.ToUpper();

   Double_t prb = 0;
   TH1 *h1 = (TH1*)this;
   if (h2 == 0) return 0;
   TAxis *xaxis1 = h1->GetXaxis();
   TAxis *xaxis2 = h2->GetXaxis();
   TAxis *yaxis1 = h1->GetYaxis();
   TAxis *yaxis2 = h2->GetYaxis();
   TAxis *zaxis1 = h1->GetZaxis();
   TAxis *zaxis2 = h2->GetZaxis();
   Int_t ncx1   = xaxis1->GetNbins();
   Int_t ncx2   = xaxis2->GetNbins();
   Int_t ncy1   = yaxis1->GetNbins();
   Int_t ncy2   = yaxis2->GetNbins();
   Int_t ncz1   = zaxis1->GetNbins();
   Int_t ncz2   = zaxis2->GetNbins();

   // Check consistency of dimensions
   if (h1->GetDimension() != 3 || h2->GetDimension() != 3) {
      Error("KolmogorovTest","Histograms must be 3-D\n");
      return 0;
   }

   // Check consistency in number of channels
   if (ncx1 != ncx2) {
      Error("KolmogorovTest","Number of channels in X is different, %d and %d\n",ncx1,ncx2);
      return 0;
   }
   if (ncy1 != ncy2) {
      Error("KolmogorovTest","Number of channels in Y is different, %d and %d\n",ncy1,ncy2);
      return 0;
   }
   if (ncz1 != ncz2) {
      Error("KolmogorovTest","Number of channels in Z is different, %d and %d\n",ncz1,ncz2);
      return 0;
   }

   // Check consistency in channel edges
   Bool_t afunc1 = kFALSE;
   Bool_t afunc2 = kFALSE;
   Double_t difprec = 1e-5;
   Double_t diff1 = TMath::Abs(xaxis1->GetXmin() - xaxis2->GetXmin());
   Double_t diff2 = TMath::Abs(xaxis1->GetXmax() - xaxis2->GetXmax());
   if (diff1 > difprec || diff2 > difprec) {
      Error("KolmogorovTest","histograms with different binning along X");
      return 0;
   }
   diff1 = TMath::Abs(yaxis1->GetXmin() - yaxis2->GetXmin());
   diff2 = TMath::Abs(yaxis1->GetXmax() - yaxis2->GetXmax());
   if (diff1 > difprec || diff2 > difprec) {
      Error("KolmogorovTest","histograms with different binning along Y");
      return 0;
   }
   diff1 = TMath::Abs(zaxis1->GetXmin() - zaxis2->GetXmin());
   diff2 = TMath::Abs(zaxis1->GetXmax() - zaxis2->GetXmax());
   if (diff1 > difprec || diff2 > difprec) {
      Error("KolmogorovTest","histograms with different binning along Z");
      return 0;
   }

   //   Should we include Uflows, Oflows?
   Int_t ibeg = 1, jbeg = 1, kbeg = 1;
   Int_t iend = ncx1, jend = ncy1, kend = ncz1;
   if (opt.Contains("U")) {ibeg = 0; jbeg = 0; kbeg = 0;}
   if (opt.Contains("O")) {iend = ncx1+1; jend = ncy1+1; kend = ncz1+1;}

   Int_t i,j,k,bin;
   Double_t sum1  = 0;
   Double_t sum2  = 0;
   Double_t w1    = 0;
   Double_t w2    = 0;
   for (i = ibeg; i <= iend; i++) {
      for (j = jbeg; j <= jend; j++) {
         for (k = kbeg; k <= kend; k++) {
            bin = h1->GetBin(i,j,k);
            sum1 += h1->GetBinContent(bin);
            sum2 += h2->GetBinContent(bin);
            Double_t ew1   = h1->GetBinError(bin);
            Double_t ew2   = h2->GetBinError(bin);
            w1   += ew1*ew1;
            w2   += ew2*ew2;
         }
      }
   }


   //    Check that both scatterplots contain events
   if (sum1 == 0) {
      Error("KolmogorovTest","Integral is zero for h1=%s\n",h1->GetName());
      return 0;
   }
   if (sum2 == 0) {
      Error("KolmogorovTest","Integral is zero for h2=%s\n",h2->GetName());
      return 0;
   }
   // calculate the effective entries.  
   // the case when errors are zero (w1 == 0 or w2 ==0) are equivalent to 
   // compare to a function. In that case the rescaling is done only on sqrt(esum2) or sqrt(esum1) 
   Double_t esum1 = 0, esum2 = 0; 
   if (w1 > 0) 
      esum1 = sum1 * sum1 / w1; 
   else 
      afunc1 = kTRUE;    // use later for calculating z
   
   if (w2 > 0) 
      esum2 = sum2 * sum2 / w2; 
   else 
      afunc2 = kTRUE;    // use later for calculating z
   
   if (afunc2 && afunc1) { 
      Error("KolmogorovTest","Errors are zero for both histograms\n");
      return 0;
   }

   //   Find Kolmogorov distance
   //   order is arbitrary take average of all possible 6 starting orders x,y,z 
   int order[3] = {0,1,2};
   int binbeg[3]; 
   int binend[3]; 
   int ibin[3];
   binbeg[0] = ibeg; binbeg[1] = jbeg; binbeg[2] = kbeg; 
   binend[0] = iend; binend[1] = jend; binend[2] = kend; 
   Double_t vdfmax[6]; // there are in total 6 combinations 
   int icomb = 0; 
   Double_t s1 = 1./(6.*sum1);
   Double_t s2 = 1./(6.*sum2);
   Double_t rsum1=0, rsum2=0;
   do { 
      // loop on bins
      Double_t dmax = 0;
      for (i = binbeg[order[0] ]; i <= binend[order[0] ]; i++) {
         for ( j = binbeg[order[1] ]; j <= binend[order[1] ]; j++) {
            for ( k = binbeg[order[2] ]; k <= binend[order[2] ]; k++) {
                  ibin[ order[0] ] = i;
                  ibin[ order[1] ] = j;
                  ibin[ order[2] ] = k;
                  bin = h1->GetBin(ibin[0],ibin[1],ibin[2]);
                  rsum1 += s1*h1->GetBinContent(bin);
                  rsum2 += s2*h2->GetBinContent(bin);
                  dmax   = TMath::Max(dmax, TMath::Abs(rsum1-rsum2));
            }
         }
      }
      vdfmax[icomb] = dmax; 
      icomb++;
   } while (TMath::Permute(3,order)  );


   // get average of distances 
   Double_t dfmax = TMath::Mean(6,vdfmax);
   
   //    Get Kolmogorov probability
   Double_t factnm;
   if (afunc1)      factnm = TMath::Sqrt(sum2);
   else if (afunc2) factnm = TMath::Sqrt(sum1);
   else             factnm = TMath::Sqrt(sum1*sum2/(sum1+sum2));
   Double_t z  = dfmax*factnm;

   prb = TMath::KolmogorovProb(z); 

   Double_t prb1 = 0, prb2 = 0; 
   // option N to combine normalization makes sense if both afunc1 and afunc2 are false
   if (opt.Contains("N")  && !(afunc1 || afunc2 ) ) { 
      // Combine probabilities for shape and normalization
      prb1   = prb;
      Double_t d12    = esum1-esum2;
      Double_t chi2   = d12*d12/(esum1+esum2);
      prb2   = TMath::Prob(chi2,1);
      //     see Eadie et al., section 11.6.2
      if (prb > 0 && prb2 > 0) prb = prb*prb2*(1-TMath::Log(prb*prb2));
      else                     prb = 0;
   }

   //    debug printout
   if (opt.Contains("D")) {
      printf(" Kolmo Prob  h1 = %s, sum1=%g\n",h1->GetName(),sum1);
      printf(" Kolmo Prob  h2 = %s, sum2=%g\n",h2->GetName(),sum2);
      printf(" Kolmo Probabil = %f, Max Dist = %g\n",prb,dfmax);
      if (opt.Contains("N"))
         printf(" Kolmo Probabil = %f for shape alone, =%f for normalisation alone\n",prb1,prb2);
   }
   // This numerical error condition should never occur:
   if (TMath::Abs(rsum1-1) > 0.002) Warning("KolmogorovTest","Numerical problems with h1=%s\n",h1->GetName());
   if (TMath::Abs(rsum2-1) > 0.002) Warning("KolmogorovTest","Numerical problems with h2=%s\n",h2->GetName());

   if(opt.Contains("M"))      return dfmax;  // return avergae of max distance

   return prb;
}


//______________________________________________________________________________
Long64_t TH3::Merge(TCollection *list)
{
   //Add all histograms in the collection to this histogram.
   //This function computes the min/max for the axes,
   //compute a new number of bins, if necessary,
   //add bin contents, errors and statistics.
   //If overflows are present and limits are different the function will fail.
   //The function returns the total number of entries in the result histogram
   //if the merge is successfull, -1 otherwise.
   //
   //IMPORTANT remark. The 2 axis x and y may have different number
   //of bins and different limits, BUT the largest bin width must be
   //a multiple of the smallest bin width and the upper limit must also
   //be a multiple of the bin width.

   if (!list) return 0;
   if (list->IsEmpty()) return (Long64_t) GetEntries();

   TList inlist;
   inlist.AddAll(list);

   TAxis newXAxis;
   TAxis newYAxis;
   TAxis newZAxis;
   Bool_t initialLimitsFound = kFALSE;
   Bool_t allSameLimits = kTRUE;
   Bool_t sameLimitsX = kTRUE;
   Bool_t sameLimitsY = kTRUE;
   Bool_t sameLimitsZ = kTRUE;
   Bool_t allHaveLimits = kTRUE;
   Bool_t firstHistWithLimits = kTRUE;

   TIter next(&inlist);
   TH3* h = this;
   do { 
      Bool_t hasLimits = h->GetXaxis()->GetXmin() < h->GetXaxis()->GetXmax();
      allHaveLimits = allHaveLimits && hasLimits;

      if (hasLimits) {
         h->BufferEmpty();

         // this is done in case the first histograms are empty and 
         // the histogram have different limits
         if (firstHistWithLimits ) { 
            // set axis limits in the case the first histogram did not have limits
            if (h != this ) { 
              if (!SameLimitsAndNBins(fXaxis, *(h->GetXaxis())) ) {
                if (h->GetXaxis()->GetXbins()->GetSize() != 0) fXaxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXbins()->GetArray());
                else                                           fXaxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
              }
              if (!SameLimitsAndNBins(fYaxis, *(h->GetYaxis())) ) {
                if (h->GetYaxis()->GetXbins()->GetSize() != 0) fYaxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXbins()->GetArray());
                else                                           fYaxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax());
              }
              if (!SameLimitsAndNBins(fZaxis, *(h->GetZaxis())) ) {
                if (h->GetZaxis()->GetXbins()->GetSize() != 0) fZaxis.Set(h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXbins()->GetArray());
                else                                           fZaxis.Set(h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXmin(), h->GetZaxis()->GetXmax());
              }
            }
            firstHistWithLimits = kFALSE;
         }

         if (!initialLimitsFound) {
            // this is executed the first time an histogram with limits is found
            // to set some initial values on the new axes
            initialLimitsFound = kTRUE;
            if (h->GetXaxis()->GetXbins()->GetSize() != 0) newXAxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXbins()->GetArray());
            else                                           newXAxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
            if (h->GetYaxis()->GetXbins()->GetSize() != 0) newYAxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXbins()->GetArray());
            else                                           newYAxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax());
            if (h->GetZaxis()->GetXbins()->GetSize() != 0) newZAxis.Set(h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXbins()->GetArray());
            else                                           newZAxis.Set(h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXmin(), h->GetZaxis()->GetXmax());
         }
         else {
           if(!SameLimitsAndNBins(newXAxis, *(h->GetXaxis()))) {
             sameLimitsX = kFALSE;
             if (!RecomputeAxisLimits(newXAxis, *(h->GetXaxis()))) {
               Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                     "first: (%d, %f, %f), second: (%d, %f, %f)",
                     newXAxis.GetNbins(), newXAxis.GetXmin(), newXAxis.GetXmax(),
                     h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(),
                     h->GetXaxis()->GetXmax());
               return -1;
             }
           }
           if(!SameLimitsAndNBins(newYAxis, *(h->GetYaxis()))) {
             sameLimitsY = kFALSE;
             if (!RecomputeAxisLimits(newYAxis, *(h->GetYaxis()))) {
               Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                     "first: (%d, %f, %f), second: (%d, %f, %f)",
                     newYAxis.GetNbins(), newYAxis.GetXmin(), newYAxis.GetXmax(),
                     h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(),
                     h->GetYaxis()->GetXmax());
               return -1;
             }
           }
           if(!SameLimitsAndNBins(newZAxis, *(h->GetZaxis()))) {
             sameLimitsZ = kFALSE;
             if (!RecomputeAxisLimits(newZAxis, *(h->GetZaxis()))) {
               Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                     "first: (%d, %f, %f), second: (%d, %f, %f)",
                     newZAxis.GetNbins(), newZAxis.GetXmin(), newZAxis.GetXmax(),
                     h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXmin(),
                     h->GetZaxis()->GetXmax());
               return -1;
             }
           }
           allSameLimits = sameLimitsX && sameLimitsY && sameLimitsZ;
         }
      }
   } while ( ( h = dynamic_cast<TH3*> ( next() ) ) != NULL );
   if (!h && (*next) ) {
      Error("Merge","Attempt to merge object of class: %s to a %s",
            (*next)->ClassName(),this->ClassName());
      return -1;
   }
   next.Reset();

   // In the case of histogram with different limits
   // newX(Y)Axis will now have the new found limits
   // but one needs first to clone this histogram to perform the merge
   // The clone is not needed when all histograms have the same limits
   TH3 * hclone = 0;
   if (!allSameLimits) { 
      // We don't want to add the clone to gDirectory,
      // so remove our kMustCleanup bit temporarily
      Bool_t mustCleanup = TestBit(kMustCleanup);
      if (mustCleanup) ResetBit(kMustCleanup);
      hclone = (TH3*)IsA()->New();
      hclone->SetDirectory(0);
      Copy(*hclone);
      if (mustCleanup) SetBit(kMustCleanup);
      BufferEmpty(1);         // To remove buffer.
      Reset();                // BufferEmpty sets limits so we can't use it later.
      SetEntries(0);
      inlist.AddFirst(hclone);
   }

   if (!allSameLimits && initialLimitsFound) {
     if (!sameLimitsX) {
       fXaxis.SetRange(0,0);
       if (newXAxis.GetXbins()->GetSize() != 0) fXaxis.Set(newXAxis.GetNbins(),newXAxis.GetXbins()->GetArray());
       else                                     fXaxis.Set(newXAxis.GetNbins(),newXAxis.GetXmin(), newXAxis.GetXmax());
     }
     if (!sameLimitsY) {
       fYaxis.SetRange(0,0);
       if (newYAxis.GetXbins()->GetSize() != 0) fYaxis.Set(newYAxis.GetNbins(),newYAxis.GetXbins()->GetArray());
       else                                     fYaxis.Set(newYAxis.GetNbins(),newYAxis.GetXmin(), newYAxis.GetXmax());
     }
     if (!sameLimitsZ) {
       fZaxis.SetRange(0,0);
       if (newZAxis.GetXbins()->GetSize() != 0) fZaxis.Set(newZAxis.GetNbins(),newZAxis.GetXbins()->GetArray());
       else                                     fZaxis.Set(newZAxis.GetNbins(),newZAxis.GetXmin(), newZAxis.GetXmax());
     }
     fNcells = (fXaxis.GetNbins()+2)*(fYaxis.GetNbins()+2)*(fZaxis.GetNbins()+2);
     SetBinsLength(fNcells);
     if (fSumw2.fN) {
       fSumw2.Set(fNcells);
     }
   }

   if (!allHaveLimits) {
      // fill this histogram with all the data from buffers of histograms without limits
      while ( (h = dynamic_cast<TH3*> (next())) ) {
         if (h->GetXaxis()->GetXmin() >= h->GetXaxis()->GetXmax() && h->fBuffer) {
            // no limits
            Int_t nbentries = (Int_t)h->fBuffer[0];
            for (Int_t i = 0; i < nbentries; i++)
               Fill(h->fBuffer[4*i + 2], h->fBuffer[4*i + 3],
               h->fBuffer[4*i + 4], h->fBuffer[4*i + 1]);
            // Entries from buffers have to be filled one by one
            // because FillN doesn't resize histograms.
         }
      }
      if (!initialLimitsFound) {
         if (hclone) { 
            inlist.Remove(hclone);
            delete hclone; 
         }
         return (Long64_t) GetEntries();  // all histograms have been processed
      }
      next.Reset();
   }

   //merge bin contents and errors
   Double_t stats[kNstat], totstats[kNstat];
   for (Int_t i=0;i<kNstat;i++) {totstats[i] = stats[i] = 0;}
   GetStats(totstats);
   Double_t nentries = GetEntries();
   Int_t binx, biny, binz, ix, iy, iz, nx, ny, nz, bin, ibin;
   Double_t cu;
   Int_t nbix = fXaxis.GetNbins();
   Int_t nbiy = fYaxis.GetNbins();
   Bool_t canRebin=TestBit(kCanRebin);
   ResetBit(kCanRebin); // reset, otherwise setting the under/overflow will rebin

   while ( (h=(TH3*)next()) ) {

      // skip empty histograms
      Double_t histEntries = h->GetEntries();
      if (h->fTsumw == 0 && h->GetEntries() == 0) continue;

      // process only if the histogram has limits; otherwise it was processed before
      if (h->GetXaxis()->GetXmin() < h->GetXaxis()->GetXmax()) {
         // import statistics
         h->GetStats(stats);
         for (Int_t i = 0; i < kNstat; i++)
            totstats[i] += stats[i];
         nentries += histEntries;

         nx = h->GetXaxis()->GetNbins();
         ny = h->GetYaxis()->GetNbins();
         nz = h->GetZaxis()->GetNbins();
         
         // mantain loop in separate binz, biny and binz to avoid 
         // callinig FindBin(x,y,z) for every bin
         for (binz = 0; binz <= nz + 1; binz++) {
            if (!allSameLimits)
               iz = fZaxis.FindBin(h->GetZaxis()->GetBinCenter(binz));
            else
               iz = binz;
            for (biny = 0; biny <= ny + 1; biny++) {
               if (!allSameLimits)
                  iy = fYaxis.FindBin(h->GetYaxis()->GetBinCenter(biny));
               else 
                  iy = biny; 

               for (binx = 0; binx <= nx + 1; binx++) {
                  bin = binx +(nx+2)*(biny + (ny+2)*binz);
                  cu  = h->GetBinContent(bin);
                  if (!allSameLimits) { 
                     // look at non-empty unerflow/overflows
                     if (cu != 0 && ( (!sameLimitsX && (binx == 0 || binx == nx+1)) || (!sameLimitsY && (biny == 0 || biny == ny+1)) || (!sameLimitsZ && (binz == 0 || binz == nz+1)))) {
                        Error("Merge", "Cannot merge histograms - the histograms have"
                              " different limits and undeflows/overflows are present."
                              " The initial histogram is now broken!");
                        return -1;
                     }
                     ix = fXaxis.FindBin(h->GetXaxis()->GetBinCenter(binx));
                  }
                  else { 
                     // case histograms have same limits 
                     ix = binx; 
                  }
                  
                  ibin = ix +(nbix+2)*(iy + (nbiy+2)*iz);
                  if (ibin <0) continue;
                  AddBinContent(ibin,cu);
                  if (fSumw2.fN) {
                     Double_t error1 = h->GetBinError(bin);
                     fSumw2.fArray[ibin] += error1*error1;
                  }
               }
            }
         }
      }
   }
   if (canRebin) SetBit(kCanRebin);

   //copy merged stats
   PutStats(totstats);
   SetEntries(nentries);
   if (hclone) { 
      inlist.Remove(hclone);
      delete hclone;
   }
   return (Long64_t)nentries;
}

//______________________________________________________________________________
TH1D *TH3::ProjectionX(const char *name, Int_t iymin, Int_t iymax, Int_t izmin, Int_t izmax, Option_t *option) const
{
   //*-*-*-*-*Project a 3-D histogram into a 1-D histogram along X*-*-*-*-*-*-*
   //*-*      ====================================================
   //
   //   The projection is always of the type TH1D.
   //   The projection is made from the cells along the X axis
   //   ranging from iymin to iymax and izmin to izmax included.
   //   By default, underflow and overflows are included
   //   By Setting iymin=1 and iymax=NbinsY the underflow and/or overflow will be excluded
   //
   //   if option "e" is specified, the errors are computed.
   //   if option "d" is specified, the projection is drawn in the current pad.
   //   if option "o" original axis range of the target axes will be
   //   kept, but only bins inside the selected range will be filled.
   //
   //   NOTE that if a TH1D named "name" exists in the current directory or pad 
   //   the histogram is reset and filled again with the projected contents of the TH3.
   //
   //  implemented using Project3D


   TString opt = option;
   opt.ToLower();

   Int_t iyminOld = GetYaxis()->GetFirst();
   Int_t iymaxOld = GetYaxis()->GetLast();
   Int_t izminOld = GetZaxis()->GetFirst();
   Int_t izmaxOld = GetZaxis()->GetLast();   

   GetYaxis()->SetRange(iymin,iymax);
   GetZaxis()->SetRange(izmin,izmax);

   Bool_t computeErrors = GetSumw2N();
   if (opt.Contains("e") ) { 
      computeErrors = kTRUE;
      opt.Remove(opt.First("e"),1);
   }
   Bool_t originalRange = kFALSE;
   if (opt.Contains('o') ) { 
      originalRange = kTRUE; 
      opt.Remove(opt.First("o"),1);
   }
  
   TH1D * h1 = DoProject1D(name, GetTitle(), this->GetXaxis(), computeErrors, originalRange,true,true);

   // restore original range
   if (GetYaxis()->TestBit(TAxis::kAxisRange)) GetYaxis()->SetRange(iyminOld,iymaxOld);
   if (GetZaxis()->TestBit(TAxis::kAxisRange)) GetZaxis()->SetRange(izminOld,izmaxOld);

   // draw in current pad 
   if (h1 && opt.Contains("d")) {
      opt.Remove(opt.First("d"),1);
      TVirtualPad *padsav = gPad;
      TVirtualPad *pad = gROOT->GetSelectedPad();
      if (pad) pad->cd();
      if (!gPad || !gPad->FindObject(h1)) {
         h1->Draw(opt);
      } else {
         h1->Paint(opt);
      }
      if (padsav) padsav->cd();
   }

   return h1;
}

//______________________________________________________________________________
TH1D *TH3::ProjectionY(const char *name, Int_t ixmin, Int_t ixmax, Int_t izmin, Int_t izmax, Option_t *option) const
{
   //*-*-*-*-*Project a 3-D histogram into a 1-D histogram along Y*-*-*-*-*-*-*
   //*-*      ====================================================
   //
   //   The projection is always of the type TH1D.
   //   The projection is made from the cells along the Y axis
   //   ranging from ixmin to ixmax and izmin to izmax included.
   //   By default, underflow and overflow are included. 
   //   By Setting ixmin=1 and ixmax=NbinsX the underflow and/or overflow will be excluded
   //
   //   if option "e" is specified, the errors are computed.
   //   if option "d" is specified, the projection is drawn in the current pad.
   //   if option "o" original axis range of the target axes will be
   //   kept, but only bins inside the selected range will be filled.
   //
   //   NOTE that if a TH1D named "name" exists in the current directory or pad,    
   //   the histogram is reset and filled again with the projected contents of the TH3.
   //
   //  implemented using Project3D


   TString opt = option;
   opt.ToLower();

   Int_t ixminOld = GetXaxis()->GetFirst();
   Int_t ixmaxOld = GetXaxis()->GetLast();
   Int_t izminOld = GetZaxis()->GetFirst();
   Int_t izmaxOld = GetZaxis()->GetLast();   

   GetXaxis()->SetRange(ixmin,ixmax);
   GetZaxis()->SetRange(izmin,izmax);

   Bool_t computeErrors = GetSumw2N();
   if (opt.Contains("e") ) { 
      computeErrors = kTRUE;
      opt.Remove(opt.First("e"),1);
   }
   Bool_t originalRange = kFALSE;
   if (opt.Contains('o') ) { 
      originalRange = kTRUE; 
      opt.Remove(opt.First("o"),1);
   }
  
   TH1D * h1 = DoProject1D(name, GetTitle(), this->GetYaxis(), computeErrors, originalRange, true, true);

   // restore axis range
   if (GetXaxis()->TestBit(TAxis::kAxisRange)) GetXaxis()->SetRange(ixminOld,ixmaxOld);
   if (GetZaxis()->TestBit(TAxis::kAxisRange)) GetZaxis()->SetRange(izminOld,izmaxOld);

   // draw in current pad 
   if (h1 && opt.Contains("d")) {
      opt.Remove(opt.First("d"),1);
      TVirtualPad *padsav = gPad;
      TVirtualPad *pad = gROOT->GetSelectedPad();
      if (pad) pad->cd();
      if (!gPad || !gPad->FindObject(h1)) {
         h1->Draw(opt);
      } else {
         h1->Paint(opt);
      }
      if (padsav) padsav->cd();
   }
   
   return h1;
}

//______________________________________________________________________________
TH1D *TH3::ProjectionZ(const char *name, Int_t ixmin, Int_t ixmax, Int_t iymin, Int_t iymax, Option_t *option) const
{
   //*-*-*-*-*Project a 3-D histogram into a 1-D histogram along Z*-*-*-*-*-*-*
   //*-*      ====================================================
   //
   //   The projection is always of the type TH1D.
   //   The projection is made from the cells along the Z axis
   //   ranging from ixmin to ixmax and iymin to iymax included.
   //   By default, bins 1 to nx and 1 to ny  are included
   //   By setting ixmin=1 and/or ixmax=NbinsX the underflow and/or overflow in X will be excluded
   //   By setting iymin=1 and/or iymax=NbinsY the underflow and/or overflow in Y will be excluded
   //
   //   if option "e" is specified, the errors are computed.
   //   if option "d" is specified, the projection is drawn in the current pad.
   //   if option "o" original axis range of the target axes will be
   //   kept, but only bins inside the selected range will be filled.
   //
   //   NOTE that if a TH1D named "name" exists in the current directory or pad,   
   //   the histogram is reset and filled again with the projected contents of the TH3.
   //
   //  implemented using Project3D


   TString opt = option;
   opt.ToLower();

   Int_t ixminOld = GetXaxis()->GetFirst();
   Int_t ixmaxOld = GetXaxis()->GetLast();
   Int_t iyminOld = GetYaxis()->GetFirst();
   Int_t iymaxOld = GetYaxis()->GetLast();  

   GetXaxis()->SetRange(ixmin,ixmax);
   GetYaxis()->SetRange(iymin,iymax);

   Bool_t computeErrors = GetSumw2N();
   if (opt.Contains("e") ) { 
      computeErrors = kTRUE;
      opt.Remove(opt.First("e"),1);
   }
   Bool_t originalRange = kFALSE;
   if (opt.Contains('o') ) { 
      originalRange = kTRUE; 
      opt.Remove(opt.First("o"),1);
   }

   TH1D * h1 =  DoProject1D(name, GetTitle(), this->GetZaxis(), computeErrors, originalRange, true, true);

   // restore the range
   if (GetXaxis()->TestBit(TAxis::kAxisRange)) GetXaxis()->SetRange(ixminOld,ixmaxOld);
   if (GetYaxis()->TestBit(TAxis::kAxisRange)) GetYaxis()->SetRange(iyminOld,iymaxOld);

   // draw in current pad 
   if (h1 && opt.Contains("d")) {
      opt.Remove(opt.First("d"),1);
      TVirtualPad *padsav = gPad;
      TVirtualPad *pad = gROOT->GetSelectedPad();
      if (pad) pad->cd();
      if (!gPad || !gPad->FindObject(h1)) {
         h1->Draw(opt);
      } else {
         h1->Paint(opt);
      }
      if (padsav) padsav->cd();
   }

   return h1;
}

//______________________________________________________________________________
TH1D *TH3::DoProject1D(const char* name, const char* title, TAxis* projX, 
                       bool computeErrors, bool originalRange, 
                       bool useUF, bool useOF) const
{
   // internal methdod performing the projection to 1D histogram
   // called from TH3::Project3D

   // Create the projection histogram
   TH1D *h1 = 0;

   // Get range to use as well as bin limits
   Int_t ixmin = projX->GetFirst();
   Int_t ixmax = projX->GetLast();
//   if (ixmin == 0 && ixmax == 0) { ixmin = 1; ixmax = projX->GetNbins(); }
   Int_t nx = ixmax-ixmin+1;

   // Create the histogram, either reseting a preexisting one 
   TObject *h1obj = gROOT->FindObject(name);
   if (h1obj && h1obj->InheritsFrom(TH1::Class())) {
      if (h1obj->IsA() != TH1D::Class() ) { 
         Error("DoProject1D","Histogram with name %s must be a TH1D and is a %s",name,h1obj->ClassName());
         return 0; 
      }
      h1 = (TH1D*)h1obj;
      // reset histogram and re-set the axis in any case
      h1->Reset();
      const TArrayD *bins = projX->GetXbins();
      if ( originalRange )
      {
         if (bins->fN == 0) {
            h1->SetBins(projX->GetNbins(),projX->GetXmin(),projX->GetXmax());
         } else {
            h1->SetBins(projX->GetNbins(),bins->fArray);
         }
      } else {
         if (bins->fN == 0) {
            h1->SetBins(nx,projX->GetBinLowEdge(ixmin),projX->GetBinUpEdge(ixmax));
         } else {
            h1->SetBins(nx,&bins->fArray[ixmin-1]);
         }
      }
   }

   if (!h1) { 
      const TArrayD *bins = projX->GetXbins();
      if ( originalRange )
      {
         if (bins->fN == 0) {
            h1 = new TH1D(name,title,projX->GetNbins(),projX->GetXmin(),projX->GetXmax());
         } else {
            h1 = new TH1D(name,title,projX->GetNbins(),bins->fArray);
         }
      } else {
         if (bins->fN == 0) {
            h1 = new TH1D(name,title,nx,projX->GetBinLowEdge(ixmin),projX->GetBinUpEdge(ixmax));
         } else {
            h1 = new TH1D(name,title,nx,&bins->fArray[ixmin-1]);
         }
      }
   }

   // Copy the axis attributes and the axis labels if needed.
   h1->GetXaxis()->ImportAttributes(projX);
   THashList* labels = projX->GetLabels();
   if (labels) {
      TIter iL(labels);
      TObjString* lb;
      Int_t i = 1;
      while ((lb=(TObjString*)iL())) {
         h1->GetXaxis()->SetBinLabel(i,lb->String().Data());
         i++;
      }
   }
   h1->SetLineColor(this->GetLineColor());
   h1->SetFillColor(this->GetFillColor());
   h1->SetMarkerColor(this->GetMarkerColor());
   h1->SetMarkerStyle(this->GetMarkerStyle());

   // Activate errors
   if ( computeErrors ) h1->Sumw2();

   // Set references to the axis, so that the bucle has no branches.
   TAxis* out1 = 0;
   TAxis* out2 = 0;
   if ( projX == GetXaxis() ) {
      out1 = GetYaxis();
      out2 = GetZaxis();
   } else if ( projX == GetYaxis() ) {
      out1 = GetZaxis();
      out2 = GetXaxis();
   } else {
      out1 = GetYaxis();
      out2 = GetXaxis();
   }

   Int_t *refX = 0, *refY = 0, *refZ = 0;
   Int_t ixbin, out1bin, out2bin;
   if ( projX == GetXaxis() ) { refX = &ixbin;   refY = &out1bin; refZ = &out2bin; }
   if ( projX == GetYaxis() ) { refX = &out2bin; refY = &ixbin;   refZ = &out1bin; }
   if ( projX == GetZaxis() ) { refX = &out2bin; refY = &out1bin; refZ = &ixbin;   }
   R__ASSERT (refX != 0 && refY != 0 && refZ != 0); 

   // Fill the projected histogram excluding underflow/overflows if considered in the option
   // if specified in the option (by default they considered)
   Double_t totcont  = 0;

   Int_t out1min = out1->GetFirst(); 
   Int_t out1max = out1->GetLast(); 
   // GetFirst(), GetLast() can return (0,0) when the range bit is set artifically (see TAxis::SetRange)
 //if (out1min == 0 && out1max == 0) { out1min = 1; out1max = out1->GetNbins(); }
   // correct for underflow/overflows
   if (useUF && !out1->TestBit(TAxis::kAxisRange) )  out1min -= 1; 
   if (useOF && !out1->TestBit(TAxis::kAxisRange) )  out1max += 1; 
   Int_t out2min = out2->GetFirst(); 
   Int_t out2max = out2->GetLast(); 
//   if (out2min == 0 && out2max == 0) { out2min = 1; out2max = out2->GetNbins(); }
   if (useUF && !out2->TestBit(TAxis::kAxisRange) )  out2min -= 1; 
   if (useOF && !out2->TestBit(TAxis::kAxisRange) )  out2max += 1; 

   for (ixbin=0;ixbin<=1+projX->GetNbins();ixbin++){
      if ( projX->TestBit(TAxis::kAxisRange) && ( ixbin < ixmin || ixbin > ixmax )) continue;

      Double_t cont = 0; 
      Double_t err2 = 0; 

      // loop on the bins to be integrated (outbin should be called inbin)
      for (out1bin = out1min; out1bin <= out1max; out1bin++){
         for (out2bin = out2min; out2bin <= out2max; out2bin++){

            Int_t bin = GetBin(*refX, *refY, *refZ);

            // sum the bin contents and errors if needed
            cont += GetBinContent(bin);
            if (computeErrors) { 
               Double_t exyz = GetBinError(bin);
               err2 += exyz*exyz;
            }
         }
      }
      Int_t ix    = h1->FindBin( projX->GetBinCenter(ixbin) );
      h1->SetBinContent(ix ,cont);
      if (computeErrors) h1->SetBinError(ix, TMath::Sqrt(err2) ); 
      // sum all content
      totcont += cont;
      
   }

   // since we use a combination of fill and SetBinError we need to reset and recalculate the statistics
   // for weighted histograms otherwise sumw2 will be wrong. 
   // We  can keep the original statistics from the TH3 if the projected sumw is consistent with original one
   // i.e. when no events are thrown away  
   bool resetStats = true; 
   double eps = 1.E-12;
   if (IsA() == TH3F::Class() ) eps = 1.E-6;
   if (fTsumw != 0 && TMath::Abs( fTsumw - totcont) <  TMath::Abs(fTsumw) * eps) resetStats = false; 

   bool resetEntries = resetStats; 
   // entries are calculated using underflow/overflow. If excluded entries must be reset
   resetEntries |= !useUF || !useOF; 
   

   if (!resetStats) {
      Double_t stats[kNstat];
      GetStats(stats); 
      if ( projX == GetYaxis() ) {
         stats[2] = stats[4];
         stats[3] = stats[5]; 
      }
      else if  ( projX == GetZaxis() ) {
         stats[2] = stats[7];
         stats[3] = stats[8]; 
      }
      h1->PutStats(stats);
   }
   else {
      // reset statistics 
      h1->ResetStats();
   }
   if (resetEntries) { 
      // in case of error calculation (i.e. when Sumw2() is set) 
      // use the effective entries for the entries
      // since this  is the only way to estimate them
      Double_t entries =  TMath::Floor( totcont + 0.5); // to avoid numerical rounding         
      if (computeErrors) entries = h1->GetEffectiveEntries();
      h1->SetEntries( entries );  
   }
   else { 
      h1->SetEntries( fEntries );  
   }

   return h1;
}

//______________________________________________________________________________
TH2D *TH3::DoProject2D(const char* name, const char * title, TAxis* projX, TAxis* projY,  
                    bool computeErrors, bool originalRange,
                    bool useUF, bool useOF) const
{
   // internal method performing the projection to a 2D histogram
   // called from TH3::Project3D 

   TH2D *h2 = 0;

   // Get range to use as well as bin limits
   Int_t ixmin = projX->GetFirst();
   Int_t ixmax = projX->GetLast();
   Int_t iymin = projY->GetFirst();
   Int_t iymax = projY->GetLast();
   if (ixmin == 0 && ixmax == 0) { ixmin = 1; ixmax = projX->GetNbins(); }
   if (iymin == 0 && iymax == 0) { iymin = 1; iymax = projY->GetNbins(); }
   Int_t nx = ixmax-ixmin+1;
   Int_t ny = iymax-iymin+1;

   // Create the histogram, either reseting a preexisting one 
   //  or creating one from scratch.
   // Does an object with the same name exists?
   TObject *h2obj = gROOT->FindObject(name);
   if (h2obj && h2obj->InheritsFrom(TH1::Class())) {
      if ( h2obj->IsA() != TH2D::Class() ) { 
         Error("DoProject2D","Histogram with name %s must be a TH2D and is a %s",name,h2obj->ClassName());
         return 0; 
      }
      h2 = (TH2D*)h2obj;
      // reset histogram and its axes
      h2->Reset();
      const TArrayD *xbins = projX->GetXbins();
      const TArrayD *ybins = projY->GetXbins();
      if ( originalRange ) {
         h2->SetBins(projY->GetNbins(),projY->GetXmin(),projY->GetXmax()
                     ,projX->GetNbins(),projX->GetXmin(),projX->GetXmax());
         // set bins for mixed axis do not exists - need to set afterwards the variable bins
         if (ybins->fN != 0) 
            h2->GetXaxis()->Set(projY->GetNbins(),&ybins->fArray[iymin-1]);
         if (xbins->fN != 0)
            h2->GetYaxis()->Set(projX->GetNbins(),&xbins->fArray[ixmin-1]);
      } else {
         h2->SetBins(ny,projY->GetBinLowEdge(iymin),projY->GetBinUpEdge(iymax)
                     ,nx,projX->GetBinLowEdge(ixmin),projX->GetBinUpEdge(ixmax));
         if (ybins->fN != 0) 
            h2->GetXaxis()->Set(ny,&ybins->fArray[iymin-1]);
         if (xbins->fN != 0) 
            h2->GetYaxis()->Set(nx,&xbins->fArray[ixmin-1]);
      }
   }

      
   if (!h2) { 
      const TArrayD *xbins = projX->GetXbins();
      const TArrayD *ybins = projY->GetXbins();
      if ( originalRange )
      {
         if (xbins->fN == 0 && ybins->fN == 0) {
            h2 = new TH2D(name,title,projY->GetNbins(),projY->GetXmin(),projY->GetXmax()
                          ,projX->GetNbins(),projX->GetXmin(),projX->GetXmax());
         } else if (ybins->fN == 0) {
            h2 = new TH2D(name,title,projY->GetNbins(),projY->GetXmin(),projY->GetXmax()
                          ,projX->GetNbins(),&xbins->fArray[ixmin-1]);
         } else if (xbins->fN == 0) {
            h2 = new TH2D(name,title,projY->GetNbins(),&ybins->fArray[iymin-1]
                          ,projX->GetNbins(),projX->GetXmin(),projX->GetXmax());
         } else {
            h2 = new TH2D(name,title,projY->GetNbins(),&ybins->fArray[iymin-1],projX->GetNbins(),&xbins->fArray[ixmin-1]);
         }
      } else {
         if (xbins->fN == 0 && ybins->fN == 0) {
            h2 = new TH2D(name,title,ny,projY->GetBinLowEdge(iymin),projY->GetBinUpEdge(iymax)
                          ,nx,projX->GetBinLowEdge(ixmin),projX->GetBinUpEdge(ixmax));
         } else if (ybins->fN == 0) {
            h2 = new TH2D(name,title,ny,projY->GetBinLowEdge(iymin),projY->GetBinUpEdge(iymax)
                          ,nx,&xbins->fArray[ixmin-1]);
         } else if (xbins->fN == 0) {
            h2 = new TH2D(name,title,ny,&ybins->fArray[iymin-1]
                          ,nx,projX->GetBinLowEdge(ixmin),projX->GetBinUpEdge(ixmax));
         } else {
            h2 = new TH2D(name,title,ny,&ybins->fArray[iymin-1],nx,&xbins->fArray[ixmin-1]);
         }
      }
   }

   // Copy the axis attributes and the axis labels if needed.
   THashList* labels1 = 0;
   THashList* labels2 = 0;
   // "xy"
   h2->GetXaxis()->ImportAttributes(projY);
   h2->GetYaxis()->ImportAttributes(projX);
   labels1 = projY->GetLabels();
   labels2 = projX->GetLabels();
   if (labels1) {
      TIter iL(labels1);
      TObjString* lb;
      Int_t i = 1;
      while ((lb=(TObjString*)iL())) {
         h2->GetXaxis()->SetBinLabel(i,lb->String().Data());
         i++;
      }
   }
   if (labels2) {
      TIter iL(labels2);
      TObjString* lb;
      Int_t i = 1;
      while ((lb=(TObjString*)iL())) {
         h2->GetYaxis()->SetBinLabel(i,lb->String().Data());
         i++;
      }
   }
   h2->SetLineColor(this->GetLineColor());
   h2->SetFillColor(this->GetFillColor());
   h2->SetMarkerColor(this->GetMarkerColor());
   h2->SetMarkerStyle(this->GetMarkerStyle());

   // Activate errors
   if ( computeErrors) h2->Sumw2();

   // Set references to the axis, so that the bucle has no branches.
   TAxis* out = 0;
   if ( projX != GetXaxis() && projY != GetXaxis() ) {
      out = GetXaxis();
   } else if ( projX != GetYaxis() && projY != GetYaxis() ) {
      out = GetYaxis();
   } else {
      out = GetZaxis();
   }

   Int_t *refX = 0, *refY = 0, *refZ = 0;
   Int_t ixbin, iybin, outbin;
   if ( projX == GetXaxis() && projY == GetYaxis() ) { refX = &ixbin;  refY = &iybin;  refZ = &outbin; }
   if ( projX == GetYaxis() && projY == GetXaxis() ) { refX = &iybin;  refY = &ixbin;  refZ = &outbin; }
   if ( projX == GetXaxis() && projY == GetZaxis() ) { refX = &ixbin;  refY = &outbin; refZ = &iybin;  }
   if ( projX == GetZaxis() && projY == GetXaxis() ) { refX = &iybin;  refY = &outbin; refZ = &ixbin;  }
   if ( projX == GetYaxis() && projY == GetZaxis() ) { refX = &outbin; refY = &ixbin;  refZ = &iybin;  }
   if ( projX == GetZaxis() && projY == GetYaxis() ) { refX = &outbin; refY = &iybin;  refZ = &ixbin;  }
   R__ASSERT (refX != 0 && refY != 0 && refZ != 0); 

   // Fill the projected histogram excluding underflow/overflows if considered in the option
   // if specified in the option (by default they considered)
   Double_t totcont  = 0;

   Int_t outmin = out->GetFirst(); 
   Int_t outmax = out->GetLast(); 
   // GetFirst(), GetLast() can return (0,0) when the range bit is set artifically (see TAxis::SetRange)
   if (outmin == 0 && outmax == 0) { outmin = 1; outmax = out->GetNbins(); }
   // correct for underflow/overflows
   if (useUF && !out->TestBit(TAxis::kAxisRange) )  outmin -= 1; 
   if (useOF && !out->TestBit(TAxis::kAxisRange) )  outmax += 1; 

   for (ixbin=0;ixbin<=1+projX->GetNbins();ixbin++){
      if ( projX->TestBit(TAxis::kAxisRange) && ( ixbin < ixmin || ixbin > ixmax )) continue;
      Int_t ix = h2->GetYaxis()->FindBin( projX->GetBinCenter(ixbin) );

      for (iybin=0;iybin<=1+projY->GetNbins();iybin++){
         if ( projY->TestBit(TAxis::kAxisRange) && ( iybin < iymin || iybin > iymax )) continue;
         Int_t iy = h2->GetXaxis()->FindBin( projY->GetBinCenter(iybin) );

         Double_t cont = 0; 
         Double_t err2 = 0;

         // loop on the bins to be integrated (outbin should be called inbin)
         for (outbin = outmin; outbin <= outmax; outbin++){

            Int_t bin = GetBin(*refX,*refY,*refZ);

            // sum the bin contents and errors if needed
            cont += GetBinContent(bin);
            if (computeErrors) { 
               Double_t exyz = GetBinError(bin);
               err2 += exyz*exyz;
            }

         }

         // remember axis are inverted 
         h2->SetBinContent(iy , ix, cont);
         if (computeErrors) h2->SetBinError(iy, ix, TMath::Sqrt(err2) ); 
         // sum all content
         totcont += cont;

      }
   }

   // since we use fill we need to reset and recalculate the statistics (see comment in DoProject1D )
   // or keep original statistics if consistent sumw2  
   bool resetStats = true; 
   double eps = 1.E-12;
   if (IsA() == TH3F::Class() ) eps = 1.E-6;
   if (fTsumw != 0 && TMath::Abs( fTsumw - totcont) <  TMath::Abs(fTsumw) * eps) resetStats = false; 

   bool resetEntries = resetStats; 
   // entries are calculated using underflow/overflow. If excluded entries must be reset
   resetEntries |= !useUF || !useOF; 

   if (!resetStats) {
      Double_t stats[kNstat];
      Double_t oldst[kNstat]; // old statistics
      for (Int_t i = 0; i < kNstat; ++i) { oldst[i] = 0; }
      GetStats(oldst); 
      std::copy(oldst,oldst+kNstat,stats);
      // not that projX refer to Y axis and projX refer to the X axis of projected histogram
      // nothing to do for projection in Y vs X
      if ( projY == GetXaxis() && projX == GetZaxis() ) {  // case XZ
         stats[4] = oldst[7];
         stats[5] = oldst[8];
         stats[6] = oldst[9];
      }
      if ( projY == GetYaxis() ) {
         stats[2] = oldst[4];
         stats[3] = oldst[5]; 
         if ( projX == GetXaxis() )  { // case YX
            stats[4] = oldst[2]; 
            stats[5] = oldst[3];
         }
         if ( projX == GetZaxis() )  { // case YZ
            stats[4] = oldst[7]; 
            stats[5] = oldst[8];
            stats[6] = oldst[10];
         }
      }
      else if  ( projY == GetZaxis() ) {
         stats[2] = oldst[7];
         stats[3] = oldst[8]; 
         if ( projX == GetXaxis() )  { // case ZX
            stats[4] = oldst[2]; 
            stats[5] = oldst[3];
            stats[6] = oldst[9];
         }
         if ( projX == GetYaxis() )  { // case ZY
            stats[4] = oldst[4]; 
            stats[5] = oldst[5];
            stats[6] = oldst[10];
         }
      }
      // set the new statistics 
      h2->PutStats(stats);
   }
   else { 
      // recalculate the statistics
      h2->ResetStats(); 
   }

   if (resetEntries) { 
      // use the effective entries for the entries
      // since this  is the only way to estimate them
      Double_t entries =  h2->GetEffectiveEntries();
      if (!computeErrors) entries = TMath::Floor( entries + 0.5); // to avoid numerical rounding
      h2->SetEntries( entries );  
   }
   else { 
      h2->SetEntries( fEntries );  
   }


   return h2;
}


//______________________________________________________________________________
TH1 *TH3::Project3D(Option_t *option) const
{
   // Project a 3-d histogram into 1 or 2-d histograms depending on the
   // option parameter
   // option may contain a combination of the characters x,y,z,e
   // option = "x" return the x projection into a TH1D histogram
   // option = "y" return the y projection into a TH1D histogram
   // option = "z" return the z projection into a TH1D histogram
   // option = "xy" return the x versus y projection into a TH2D histogram
   // option = "yx" return the y versus x projection into a TH2D histogram
   // option = "xz" return the x versus z projection into a TH2D histogram
   // option = "zx" return the z versus x projection into a TH2D histogram
   // option = "yz" return the y versus z projection into a TH2D histogram
   // option = "zy" return the z versus y projection into a TH2D histogram
   // NB: the notation "a vs b" means "a" vertical and "b" horizontal
   //
   // option = "o" original axis range of the target axes will be
   //   kept, but only bins inside the selected range will be filled.
   //
   // If option contains the string "e", errors are computed
   //
   // The projection is made for the selected bins only.
   // To select a bin range along an axis, use TAxis::SetRange, eg
   //    h3.GetYaxis()->SetRange(23,56);
   //
   // NOTE 1: The generated histogram is named th3name + option
   // eg if the TH3* h histogram is named "myhist", then
   // h->Project3D("xy"); produces a TH2D histogram named "myhist_xy"
   // if a histogram of the same type already exists, it is overwritten.
   // The following sequence
   //    h->Project3D("xy");
   //    h->Project3D("xy2");
   //  will generate two TH2D histograms named "myhist_xy" and "myhist_xy2"
   //  A different name can be generated by attaching a string to the option
   //  For example h->Project3D("name_xy") will generate an histogram with the name:  h3dname_name_xy. 
   //
   //  NOTE 2: If an histogram of the same type already exists, 
   //  the histogram is reset and filled again with the projected contents of the TH3.
   //
   //  NOTE 3: The number of entries in the projected histogram is estimated from the number of 
   //  effective entries for all the cells included in the projection. 
   //
   //  NOTE 4: underflow/overflow are included by default in the projection 
   //  To exclude underflow and/or overflow (for both axis in case of a projection to a 1D histogram) use option "NUF" and/or "NOF"
   //  With SetRange() you can have all bins except underflow/overflow only if you set the axis bit range as 
   //  following after having called SetRange:  axis->SetRange(1, axis->GetNbins());
   //          

   TString opt = option; opt.ToLower();
   Int_t pcase = 0;
   TString ptype; 
   if (opt.Contains("x"))  { pcase = 1; ptype = "x"; }
   if (opt.Contains("y"))  { pcase = 2; ptype = "y"; }
   if (opt.Contains("z"))  { pcase = 3; ptype = "z"; }
   if (opt.Contains("xy")) { pcase = 4; ptype = "xy"; }
   if (opt.Contains("yx")) { pcase = 5; ptype = "yx"; }
   if (opt.Contains("xz")) { pcase = 6; ptype = "xz"; }
   if (opt.Contains("zx")) { pcase = 7; ptype = "zx"; }
   if (opt.Contains("yz")) { pcase = 8; ptype = "yz"; }
   if (opt.Contains("zy")) { pcase = 9; ptype = "zy"; }
   
   if (pcase == 0) { 
      Error("Project3D","No projection axis specified - return a NULL pointer"); 
      return 0; 
   }
   // do not remove ptype from opt to use later in the projected histo name

   Bool_t computeErrors = GetSumw2N();
   if (opt.Contains("e") ) { 
      computeErrors = kTRUE;
      opt.Remove(opt.First("e"),1);
   }

   Bool_t useUF = kTRUE;
   Bool_t useOF = kTRUE;
   if (opt.Contains("nuf") ) { 
      useUF = kFALSE;
      opt.Remove(opt.Index("nuf"),3);
   }
   if (opt.Contains("nof") ) { 
      useOF = kFALSE;
      opt.Remove(opt.Index("nof"),3);
   }

   Bool_t originalRange = kFALSE;
   if (opt.Contains('o') ) { 
      originalRange = kTRUE; 
      opt.Remove(opt.First("o"),1);
   }


   // Create the projection histogram
   TH1 *h = 0;

   TString name = GetName();
   TString title = GetTitle();
   name  += "_";   name  += opt;  // opt may include a user defined name
   title += " ";   title += ptype; title += " projection";   

   switch (pcase) {
      case 1: 
         // "x"
         h = DoProject1D(name, title, this->GetXaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;

      case 2:
         // "y"
         h = DoProject1D(name, title, this->GetYaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;

      case 3:
         // "z"
         h = DoProject1D(name, title, this->GetZaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;

      case 4:
         // "xy"
         h = DoProject2D(name, title, this->GetXaxis(),this->GetYaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;

      case 5:
         // "yx"
         h = DoProject2D(name, title, this->GetYaxis(),this->GetXaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;

      case 6:
         // "xz"
         h = DoProject2D(name, title, this->GetXaxis(),this->GetZaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;
         
      case 7:
         // "zx"
         h = DoProject2D(name, title, this->GetZaxis(),this->GetXaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;

      case 8:
         // "yz"
         h = DoProject2D(name, title, this->GetYaxis(),this->GetZaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;

      case 9:
         // "zy"
         h = DoProject2D(name, title, this->GetZaxis(),this->GetYaxis(), 
                       computeErrors, originalRange, useUF, useOF);
         break;

   }

   // draw in current pad 
   if (h && opt.Contains("d")) {
      opt.Remove(opt.First("d"),1);
      TVirtualPad *padsav = gPad;
      TVirtualPad *pad = gROOT->GetSelectedPad();
      if (pad) pad->cd();
      if (!gPad || !gPad->FindObject(h)) {
         h->Draw(opt);
      } else {
         h->Paint(opt);
      }
      if (padsav) padsav->cd();
   }

   return h;
}

void TH3::DoFillProfileProjection(TProfile2D * p2, const TAxis & a1, const TAxis & a2,  const TAxis & a3, Int_t bin1, Int_t bin2, Int_t bin3, Int_t inBin, Bool_t useWeights ) const { 
   // internal function to fill the bins of the projected profile 2D histogram 
   // called from DoProjectProfile2D

   Double_t cont = GetBinContent(inBin); 
   if (!cont) return; 
   TArrayD & binSumw2 = *(p2->GetBinSumw2()); 
   if (useWeights && binSumw2.fN <= 0) useWeights = false;  
   // the following fill update wrongly the fBinSumw2- need to save it before
   Double_t u = a1.GetBinCenter(bin1);
   Double_t v = a2.GetBinCenter(bin2);
   Double_t w = a3.GetBinCenter(bin3);
   Int_t outBin = p2->FindBin(u, v);
   if (outBin <0) return;
   Double_t tmp = 0;
   if ( useWeights ) tmp = binSumw2.fArray[outBin];            
   p2->Fill( u , v, w, cont);
//    std::cout << "use "  << useWeights; 
//    std::cout << " size " << binSumw2.fN << " inBin " << inBin << " o " << outBin << "  "  << bin1 << "  " << bin2 << " " << fSumw2.fN 
//              << " u " << u << " v " << v << " w " << w << " tmp " << tmp << " sumw2 " << fSumw2.fArray[outBin] 
//              << " fbinsumw2 " << binSumw2.fArray[outBin]
//              << std::endl; 
   if (useWeights ) binSumw2.fArray[outBin] = tmp + fSumw2.fArray[inBin];
} 

//______________________________________________________________________________
TProfile2D *TH3::DoProjectProfile2D(const char* name, const char * title, TAxis* projX, TAxis* projY, 
                                          bool originalRange, bool useUF, bool useOF) const
{
   // internal method to project to a 2D Profile
   // called from TH3::Project3DProfile

   // Get the ranges where we will work.
   Int_t ixmin = projX->GetFirst();
   Int_t ixmax = projX->GetLast();
   Int_t iymin = projY->GetFirst();
   Int_t iymax = projY->GetLast();
   if (ixmin == 0 && ixmax == 0) { ixmin = 1; ixmax = projX->GetNbins(); }
   if (iymin == 0 && iymax == 0) { iymin = 1; iymax = projY->GetNbins(); }
   Int_t nx = ixmax-ixmin+1;
   Int_t ny = iymax-iymin+1;

   // Create the projected profiles
   TProfile2D *p2 = 0;

   // Create the histogram, either reseting a preexisting one 
   // Does an object with the same name exists?
   TObject *p2obj = gROOT->FindObject(name);
   if (p2obj && p2obj->InheritsFrom(TH1::Class())) {
      if (p2obj->IsA() != TProfile2D::Class() ) { 
         Error("DoProjectProfile2D","Histogram with name %s must be a TProfile2D and is a %s",name,p2obj->ClassName());
         return 0; 
      }
      p2 = (TProfile2D*)p2obj;
      // reset existing profile and re-set bins
      p2->Reset();
      const TArrayD *xbins = projX->GetXbins();
      const TArrayD *ybins = projY->GetXbins();
      if ( originalRange ) {
         p2->SetBins(projY->GetNbins(),projY->GetXmin(),projY->GetXmax()
                     ,projX->GetNbins(),projX->GetXmin(),projX->GetXmax());
         // set bins for mixed axis do not exists - need to set afterwards the variable bins
         if (ybins->fN != 0) 
            p2->GetXaxis()->Set(projY->GetNbins(),&ybins->fArray[iymin-1]);
         if (xbins->fN != 0)
            p2->GetYaxis()->Set(projX->GetNbins(),&xbins->fArray[ixmin-1]);
      } else {
         p2->SetBins(ny,projY->GetBinLowEdge(iymin),projY->GetBinUpEdge(iymax)
                     ,nx,projX->GetBinLowEdge(ixmin),projX->GetBinUpEdge(ixmax));
         if (ybins->fN != 0) 
            p2->GetXaxis()->Set(ny,&ybins->fArray[iymin-1]);
         if (xbins->fN != 0) 
            p2->GetYaxis()->Set(nx,&xbins->fArray[ixmin-1]);
      }
   }

   if (!p2) { 
      const TArrayD *xbins = projX->GetXbins();
      const TArrayD *ybins = projY->GetXbins();
      if ( originalRange ) {
         if (xbins->fN == 0 && ybins->fN == 0) {
            p2 = new TProfile2D(name,title,projY->GetNbins(),projY->GetXmin(),projY->GetXmax()
                                ,projX->GetNbins(),projX->GetXmin(),projX->GetXmax());
         } else if (ybins->fN == 0) {
            p2 = new TProfile2D(name,title,projY->GetNbins(),projY->GetXmin(),projY->GetXmax()
                                ,projX->GetNbins(),&xbins->fArray[ixmin-1]);
         } else if (xbins->fN == 0) {
            p2 = new TProfile2D(name,title,projY->GetNbins(),&ybins->fArray[iymin-1]
                                ,projX->GetNbins(),projX->GetXmin(),projX->GetXmax());
         } else {
            p2 = new TProfile2D(name,title,projY->GetNbins(),&ybins->fArray[iymin-1],projX->GetNbins(),&xbins->fArray[ixmin-1]);
         }
      } else {
         if (xbins->fN == 0 && ybins->fN == 0) {
            p2 = new TProfile2D(name,title,ny,projY->GetBinLowEdge(iymin),projY->GetBinUpEdge(iymax)
                                ,nx,projX->GetBinLowEdge(ixmin),projX->GetBinUpEdge(ixmax));
         } else if (ybins->fN == 0) {
            p2 = new TProfile2D(name,title,ny,projY->GetBinLowEdge(iymin),projY->GetBinUpEdge(iymax)
                                ,nx,&xbins->fArray[ixmin-1]);
         } else if (xbins->fN == 0) {
            p2 = new TProfile2D(name,title,ny,&ybins->fArray[iymin-1]
                                ,nx,projX->GetBinLowEdge(ixmin),projX->GetBinUpEdge(ixmax));
         } else {
            p2 = new TProfile2D(name,title,ny,&ybins->fArray[iymin-1],nx,&xbins->fArray[ixmin-1]);
         }
      }
   }

   // Set references to the axis, so that the loop has no branches.
   TAxis* outAxis = 0;
   if ( projX != GetXaxis() && projY != GetXaxis() ) {
      outAxis = GetXaxis();
   } else if ( projX != GetYaxis() && projY != GetYaxis() ) {
      outAxis = GetYaxis();
   } else {
      outAxis = GetZaxis();
   }

   // Weights management
   bool useWeights = (GetSumw2N() > 0); 
   if (useWeights ) p2->Sumw2(); // store sum of w2 in profile if histo is weighted
   
   // Set references to the bins, so that the loop has no branches.
   Int_t *refX = 0, *refY = 0, *refZ = 0;
   Int_t ixbin, iybin, outbin;
   if ( projX == GetXaxis() && projY == GetYaxis() ) { refX = &ixbin;  refY = &iybin;  refZ = &outbin; }
   if ( projX == GetYaxis() && projY == GetXaxis() ) { refX = &iybin;  refY = &ixbin;  refZ = &outbin; }
   if ( projX == GetXaxis() && projY == GetZaxis() ) { refX = &ixbin;  refY = &outbin; refZ = &iybin;  }
   if ( projX == GetZaxis() && projY == GetXaxis() ) { refX = &iybin;  refY = &outbin; refZ = &ixbin;  }
   if ( projX == GetYaxis() && projY == GetZaxis() ) { refX = &outbin; refY = &ixbin;  refZ = &iybin;  }
   if ( projX == GetZaxis() && projY == GetYaxis() ) { refX = &outbin; refY = &iybin;  refZ = &ixbin;  }
   R__ASSERT (refX != 0 && refY != 0 && refZ != 0); 

   Int_t outmin = outAxis->GetFirst(); 
   Int_t outmax = outAxis->GetLast(); 
   // GetFirst(), GetLast() can return (0,0) when the range bit is set artifically (see TAxis::SetRange)
   if (outmin == 0 && outmax == 0) { outmin = 1; outmax = outAxis->GetNbins(); }
   // correct for underflow/overflows
   if (useUF && !outAxis->TestBit(TAxis::kAxisRange) )  outmin -= 1; 
   if (useOF && !outAxis->TestBit(TAxis::kAxisRange) )  outmax += 1; 

   TArrayD & binSumw2 = *(p2->GetBinSumw2()); 
   if (useWeights && binSumw2.fN <= 0) useWeights = false;  

   // Call specific method for the projection
   for (ixbin=0;ixbin<=1+projX->GetNbins();ixbin++){
      if ( (ixbin < ixmin || ixbin > ixmax) && projX->TestBit(TAxis::kAxisRange)) continue;
      for ( iybin=0;iybin<=1+projY->GetNbins();iybin++){
         if ( (iybin < iymin || iybin > iymax) && projX->TestBit(TAxis::kAxisRange)) continue;

         // profile output bin
         Int_t poutBin = p2->FindBin(projY->GetBinCenter(iybin), projX->GetBinCenter(ixbin));
         if (poutBin <0) continue;
         // loop on the bins to be integrated (outbin should be called inbin)
         for (outbin = outmin; outbin <= outmax; outbin++){

            Int_t bin = GetBin(*refX,*refY,*refZ);

            //DoFillProfileProjection(p2, *projY, *projX, *outAxis, iybin, ixbin, outbin, bin, useWeights); 

            Double_t cont = GetBinContent(bin); 
            if (!cont) continue; 

            Double_t tmp = 0;
            // the following fill update wrongly the fBinSumw2- need to save it before
            if ( useWeights ) tmp = binSumw2.fArray[poutBin];            
            p2->Fill( projY->GetBinCenter(iybin) , projX->GetBinCenter(ixbin), outAxis->GetBinCenter(outbin), cont);
            if (useWeights ) binSumw2.fArray[poutBin] = tmp + fSumw2.fArray[bin];
  
         }
      }
   }

   // recompute statistics for the projected profiles 
   // forget about preserving old statistics
   bool resetStats = true; 
   Double_t stats[kNstat];
   // reset statistics 
   if (resetStats) 
      for (Int_t i=0;i<kNstat;i++) stats[i] = 0;

   p2->PutStats(stats);
   Double_t entries = fEntries; 
   // recalculate the statistics
   if (resetStats) { 
      entries =  p2->GetEffectiveEntries();
      if (!useWeights) entries = TMath::Floor( entries + 0.5); // to avoid numerical rounding
      p2->SetEntries( entries );  
   }

   p2->SetEntries(entries);

   return p2;
}

//______________________________________________________________________________
TProfile2D *TH3::Project3DProfile(Option_t *option) const
{
   // Project a 3-d histogram into a 2-d profile histograms depending
   // on the option parameter
   // option may contain a combination of the characters x,y,z
   // option = "xy" return the x versus y projection into a TProfile2D histogram
   // option = "yx" return the y versus x projection into a TProfile2D histogram
   // option = "xz" return the x versus z projection into a TProfile2D histogram
   // option = "zx" return the z versus x projection into a TProfile2D histogram
   // option = "yz" return the y versus z projection into a TProfile2D histogram
   // option = "zy" return the z versus y projection into a TProfile2D histogram
   // NB: the notation "a vs b" means "a" vertical and "b" horizontal
   //
   // option = "o" original axis range of the target axes will be
   //   kept, but only bins inside the selected range will be filled.
   //
   // The projection is made for the selected bins only.
   // To select a bin range along an axis, use TAxis::SetRange, eg
   //    h3.GetYaxis()->SetRange(23,56);
   //
   // NOTE 1: The generated histogram is named th3name + "_p" + option
   // eg if the TH3* h histogram is named "myhist", then
   // h->Project3D("xy"); produces a TProfile2D histogram named "myhist_pxy".
   // The following sequence
   //    h->Project3DProfile("xy");
   //    h->Project3DProfile("xy2");
   //  will generate two TProfile2D histograms named "myhist_pxy" and "myhist_pxy2"
   //  So, passing additional characters in the option string one can customize the name.
   //
   //  NOTE 2: If a profile of the same type already exists with compatible axes, 
   //  the profile is reset and filled again with the projected contents of the TH3.
   //  In the case of axes incompatibility, an error is reported and a NULL pointer is returned.
   //
   //  NOTE 3: The number of entries in the projected profile is estimated from the number of 
   //  effective entries for all the cells included in the projection. 
   //
   //  NOTE 4: underflow/overflow are by default excluded from the projection 
   //  (Note that this is a different default behavior compared to the projection to an histogram) 
   //  To include the underflow and/or overflow use option "UF" and/or "OF"

   TString opt = option; opt.ToLower();
   Int_t pcase = 0;
   TString ptype; 
   if (opt.Contains("xy")) { pcase = 4; ptype = "xy"; }
   if (opt.Contains("yx")) { pcase = 5; ptype = "yx"; }
   if (opt.Contains("xz")) { pcase = 6; ptype = "xz"; }
   if (opt.Contains("zx")) { pcase = 7; ptype = "zx"; }
   if (opt.Contains("yz")) { pcase = 8; ptype = "yz"; }
   if (opt.Contains("zy")) { pcase = 9; ptype = "zy"; }

   if (pcase == 0) { 
      Error("Project3D","No projection axis specified - return a NULL pointer"); 
      return 0; 
   }
   // do not remove ptype from opt to use later in the projected histo name

   Bool_t useUF = kFALSE;
   if (opt.Contains("uf") ) { 
      useUF = kTRUE;
      opt.Remove(opt.Index("uf"),2);
   }
   Bool_t useOF = kFALSE;
   if (opt.Contains("of") ) { 
      useOF = kTRUE;
      opt.Remove(opt.Index("of"),2);
   }

   Bool_t originalRange = kFALSE;
   if (opt.Contains('o') ) { 
      originalRange = kTRUE; 
      opt.Remove(opt.First("o"),1);
   }

   // Create the projected profile
   TProfile2D *p2 = 0;
   TString name = GetName();
   TString title = GetTitle();
   name  += "_p";   name  += opt;  // opt may include a user defined name
   title += " profile ";   title += ptype; title += " projection";   

   // Call the method with the specific projected axes.
   switch (pcase) {
      case 4:
         // "xy"
         p2 = DoProjectProfile2D(name, title, GetXaxis(), GetYaxis(), originalRange, useUF, useOF);
         break;

      case 5:
         // "yx"
         p2 = DoProjectProfile2D(name, title, GetYaxis(), GetXaxis(), originalRange, useUF, useOF);
         break;

      case 6:
         // "xz"
         p2 = DoProjectProfile2D(name, title, GetXaxis(), GetZaxis(), originalRange, useUF, useOF);
         break;

      case 7:
         // "zx"
         p2 = DoProjectProfile2D(name, title, GetZaxis(), GetXaxis(), originalRange, useUF, useOF);
         break;

      case 8:
         // "yz"
         p2 = DoProjectProfile2D(name, title, GetYaxis(), GetZaxis(), originalRange, useUF, useOF);
         break;

      case 9:
         // "zy"
         p2 = DoProjectProfile2D(name, title, GetZaxis(), GetYaxis(), originalRange, useUF, useOF);
         break;

   }

   return p2;
}


//______________________________________________________________________________
void TH3::PutStats(Double_t *stats)
{
   // Replace current statistics with the values in array stats

   TH1::PutStats(stats);
   fTsumwy  = stats[4];
   fTsumwy2 = stats[5];
   fTsumwxy = stats[6];
   fTsumwz  = stats[7];
   fTsumwz2 = stats[8];
   fTsumwxz = stats[9];
   fTsumwyz = stats[10];
}

//______________________________________________________________________________
TH3 *TH3::RebinX(Int_t ngroup, const char *newname)
{
  // Rebin only the X axis
  // see Rebin3D
  return Rebin3D(ngroup, 1, 1, newname);
}

//______________________________________________________________________________
TH3 *TH3::RebinY(Int_t ngroup, const char *newname)
{
  // Rebin only the Y axis
  // see Rebin3D
  return Rebin3D(1, ngroup, 1, newname);
}

//______________________________________________________________________________
TH3 *TH3::RebinZ(Int_t ngroup, const char *newname)
{
  // Rebin only the Z axis
  // see Rebin3D
  return Rebin3D(1, 1, ngroup, newname);
  
}

//______________________________________________________________________________
TH3 *TH3::Rebin3D(Int_t nxgroup, Int_t nygroup, Int_t nzgroup, const char *newname)
{
   //   -*-*-*Rebin this histogram grouping nxgroup/nygroup/nzgroup bins along the xaxis/yaxis/zaxis together*-*-*-*-
   //         =================================================================================
   //   if newname is not blank a new temporary histogram hnew is created.
   //   else the current histogram is modified (default)
   //   The parameter nxgroup/nygroup indicate how many bins along the xaxis/yaxis of this
   //   have to me merged into one bin of hnew
   //   If the original histogram has errors stored (via Sumw2), the resulting
   //   histograms has new errors correctly calculated.
   //
   //   examples: if hpxpy is an existing TH3 histogram with 40 x 40 x 40 bins
   //     hpxpypz->Rebin3D();  // merges two bins along the xaxis and yaxis in one in hpxpypz
   //                          // Carefull: previous contents of hpxpy are lost
   //     hpxpypz->RebinX(5);  //merges five bins along the xaxis in one in hpxpypz
   //     TH3 *hnew = hpxpypz->RebinY(5,"hnew"); // creates a new histogram hnew
   //                                          // merging 5 bins of h1 along the yaxis in one bin
   //
   //   NOTE : If nxgroup/nygroup is not an exact divider of the number of bins,
   //          along the xaxis/yaxis the top limit(s) of the rebinned histogram
   //          is changed to the upper edge of the xbin=newxbins*nxgroup resp.
   //          ybin=newybins*nygroup and the corresponding bins are added to
   //          the overflow bin.
   //          Statistics will be recomputed from the new bin contents.

   Int_t i,j,k,xbin,ybin,zbin;
   Int_t nxbins  = fXaxis.GetNbins();
   Int_t nybins  = fYaxis.GetNbins();
   Int_t nzbins  = fZaxis.GetNbins();
   Double_t xmin  = fXaxis.GetXmin();
   Double_t xmax  = fXaxis.GetXmax();
   Double_t ymin  = fYaxis.GetXmin();
   Double_t ymax  = fYaxis.GetXmax();
   Double_t zmin  = fZaxis.GetXmin();
   Double_t zmax  = fZaxis.GetXmax();
   if ((nxgroup <= 0) || (nxgroup > nxbins)) {
      Error("Rebin", "Illegal value of nxgroup=%d",nxgroup);
      return 0;
   }
   if ((nygroup <= 0) || (nygroup > nybins)) {
      Error("Rebin", "Illegal value of nygroup=%d",nygroup);
      return 0;
   }
   if ((nzgroup <= 0) || (nzgroup > nzbins)) {
      Error("Rebin", "Illegal value of nzgroup=%d",nzgroup);
      return 0;
   }

   Int_t newxbins = nxbins/nxgroup;
   Int_t newybins = nybins/nygroup;
   Int_t newzbins = nzbins/nzgroup;

   // Save old bin contents into a new array
   Double_t entries = fEntries;
   Double_t *oldBins = new Double_t[fNcells];
   for (Int_t ibin = 0; ibin < fNcells; ibin++) {
      oldBins[ibin] = GetBinContent(ibin);
   }
   Double_t *oldSumw2 = 0;
   if (fSumw2.fN != 0) { 
      oldSumw2 = new Double_t[fNcells];
      for (Int_t ibin = 0; ibin < fNcells; ibin++) {
         oldSumw2[ibin] = fSumw2.fArray[ibin];
      }   
   }

   // create a clone of the old histogram if newname is specified
   TH3 *hnew = this;
   if (newname && strlen(newname)) {
      hnew = (TH3*)Clone();
      hnew->SetName(newname);
   }

   // save original statistics
   Double_t stat[kNstat];
   GetStats(stat);
   bool resetStat = false;


   // change axis specs and rebuild bin contents array
   if(newxbins*nxgroup != nxbins) {
      xmax = fXaxis.GetBinUpEdge(newxbins*nxgroup);
      resetStat = true; //stats must be reset because top bins will be moved to overflow bin
   }
   if(newybins*nygroup != nybins) {
      ymax = fYaxis.GetBinUpEdge(newybins*nygroup);
      resetStat = true; //stats must be reset because top bins will be moved to overflow bin
   }
   if(newzbins*nzgroup != nzbins) {
      zmax = fZaxis.GetBinUpEdge(newzbins*nzgroup);
      resetStat = true; //stats must be reset because top bins will be moved to overflow bin
   }
   // save the TAttAxis members (reset by SetBins) for x axis
   Int_t    nXdivisions  = fXaxis.GetNdivisions();
   Color_t  xAxisColor   = fXaxis.GetAxisColor();
   Color_t  xLabelColor  = fXaxis.GetLabelColor();
   Style_t  xLabelFont   = fXaxis.GetLabelFont();
   Float_t  xLabelOffset = fXaxis.GetLabelOffset();
   Float_t  xLabelSize   = fXaxis.GetLabelSize();
   Float_t  xTickLength  = fXaxis.GetTickLength();
   Float_t  xTitleOffset = fXaxis.GetTitleOffset();
   Float_t  xTitleSize   = fXaxis.GetTitleSize();
   Color_t  xTitleColor  = fXaxis.GetTitleColor();
   Style_t  xTitleFont   = fXaxis.GetTitleFont();
   // save the TAttAxis members (reset by SetBins) for y axis
   Int_t    nYdivisions  = fYaxis.GetNdivisions();
   Color_t  yAxisColor   = fYaxis.GetAxisColor();
   Color_t  yLabelColor  = fYaxis.GetLabelColor();
   Style_t  yLabelFont   = fYaxis.GetLabelFont();
   Float_t  yLabelOffset = fYaxis.GetLabelOffset();
   Float_t  yLabelSize   = fYaxis.GetLabelSize();
   Float_t  yTickLength  = fYaxis.GetTickLength();
   Float_t  yTitleOffset = fYaxis.GetTitleOffset();
   Float_t  yTitleSize   = fYaxis.GetTitleSize();
   Color_t  yTitleColor  = fYaxis.GetTitleColor();
   Style_t  yTitleFont   = fYaxis.GetTitleFont();
   // save the TAttAxis members (reset by SetBins) for z axis
   Int_t    nZdivisions  = fZaxis.GetNdivisions();
   Color_t  zAxisColor   = fZaxis.GetAxisColor();
   Color_t  zLabelColor  = fZaxis.GetLabelColor();
   Style_t  zLabelFont   = fZaxis.GetLabelFont();
   Float_t  zLabelOffset = fZaxis.GetLabelOffset();
   Float_t  zLabelSize   = fZaxis.GetLabelSize();
   Float_t  zTickLength  = fZaxis.GetTickLength();
   Float_t  zTitleOffset = fZaxis.GetTitleOffset();
   Float_t  zTitleSize   = fZaxis.GetTitleSize();
   Color_t  zTitleColor  = fZaxis.GetTitleColor();
   Style_t  zTitleFont   = fZaxis.GetTitleFont();

   // copy merged bin contents (ignore under/overflows)
   if (nxgroup != 1 || nygroup != 1 || nzgroup != 1) {
      if(fXaxis.GetXbins()->GetSize() > 0 || fYaxis.GetXbins()->GetSize() > 0 || fZaxis.GetXbins()->GetSize() > 0){
         // variable bin sizes in x or y, don't treat both cases separately
         Double_t *xbins = new Double_t[newxbins+1];
         for(i = 0; i <= newxbins; ++i) xbins[i] = fXaxis.GetBinLowEdge(1+i*nxgroup);
         Double_t *ybins = new Double_t[newybins+1];
         for(i = 0; i <= newybins; ++i) ybins[i] = fYaxis.GetBinLowEdge(1+i*nygroup);
         Double_t *zbins = new Double_t[newzbins+1];
         for(i = 0; i <= newzbins; ++i) zbins[i] = fZaxis.GetBinLowEdge(1+i*nzgroup);
         hnew->SetBins(newxbins,xbins, newybins, ybins, newzbins, zbins);//changes also errors array (if any)
         delete [] xbins;
         delete [] ybins;
         delete [] zbins;
      } else {
         hnew->SetBins(newxbins, xmin, xmax, newybins, ymin, ymax, newzbins, zmin, zmax);//changes also errors array
      }

      Double_t binContent, binSumw2;
      Int_t oldxbin = 1;
      Int_t oldybin = 1;
      Int_t oldzbin = 1;
      Int_t bin;
      for (xbin = 1; xbin <= newxbins; xbin++) {
         oldybin=1;
         oldzbin=1;
         for (ybin = 1; ybin <= newybins; ybin++) {
            oldzbin=1;
            for (zbin = 1; zbin <= newzbins; zbin++) {
               binContent = 0;
               binSumw2   = 0;
               for (i = 0; i < nxgroup; i++) {
                  if (oldxbin+i > nxbins) break;
                  for (j =0; j < nygroup; j++) {
                     if (oldybin+j > nybins) break;
                     for (k =0; k < nzgroup; k++) {
                        if (oldzbin+k > nzbins) break;
                        //get global bin (same conventions as in TH1::GetBin(xbin,ybin)
                        bin = oldxbin + i + (oldybin + j)*(nxbins + 2) + (oldzbin + k)*(nxbins + 2)*(nybins + 2);
                        binContent += oldBins[bin];
                        if (oldSumw2) binSumw2 += oldSumw2[bin];
                     }
                  }
               }
               Int_t ibin = hnew->GetBin(xbin,ybin,zbin);  // new bin number
               hnew->SetBinContent(ibin, binContent);
               if (oldSumw2) hnew->fSumw2.fArray[ibin] = binSumw2;
               oldzbin += nzgroup;
            }
            oldybin += nygroup;
         }
         oldxbin += nxgroup;
      }

      // compute new underflow/overflows for the 8 vertices
      for (Int_t xover = 0; xover <= 1; xover++) {
         for (Int_t yover = 0; yover <= 1; yover++) {
            for (Int_t zover = 0; zover <= 1; zover++) {
               binContent = 0;
               binSumw2 = 0;
               // make loop in case of only underflow/overflow
               for(xbin = xover*oldxbin; xbin <= xover*(nxbins+1); xbin++) {
                  for(ybin = yover*oldybin; ybin <= yover*(nybins+1); ybin++){
                     for(zbin = zover*oldzbin; zbin <= zover*(nzbins+1); zbin++){
                        bin = GetBin(xbin,ybin,zbin);
                        binContent += oldBins[bin];
                        if(oldSumw2) binSumw2 += oldSumw2[bin];
                     }
                  }
               }
               Int_t binNew = hnew->GetBin( xover *(newxbins+1),
                                           yover*(newybins+1), zover*(newzbins+1) );
               hnew->SetBinContent(binNew,binContent);                     
               if (oldSumw2) hnew->fSumw2.fArray[binNew] = binSumw2;
            }
         }
      }         
      
      Double_t binContent0, binContent2, binContent3, binContent4;
      Double_t binError0, binError2, binError3, binError4;
      Int_t oldxbin2, oldybin2, oldzbin2;
      Int_t ufbin, ofbin, ofbin2, ofbin3, ofbin4;

      //  recompute under/overflow contents in y for the new  x and z bins
      oldxbin2 = 1;
      oldybin2 = 1;
      oldzbin2 = 1;
      for (xbin = 1; xbin<=newxbins; xbin++) {
         oldzbin2 = 1;
         for (zbin = 1; zbin<=newzbins; zbin++) {
            binContent0 = binContent2 = 0;
            binError0 = binError2 = 0;
            for (i=0; i<nxgroup; i++) {
               if (oldxbin2+i > nxbins) break;
               for (k=0; k<nzgroup; k++) {
                  if (oldzbin2+k > nzbins) break;

                  //old underflow bin (in y)
                  ufbin = oldxbin2 + i + (nxbins+2)*(nybins+2)*(oldzbin2+k);
                  binContent0 += oldBins[ufbin];
                  if(oldSumw2) binError0 += oldSumw2[ufbin];
                  for(ybin = oldybin; ybin <= nybins + 1; ybin++){
                     //old overflow bin (in y)
                     ofbin = ufbin + ybin*(nxbins+2);
                     binContent2 += oldBins[ofbin];
                     if(oldSumw2) binError2 += oldSumw2[ofbin];
                  }
               }
            }
            hnew->SetBinContent(xbin,0,zbin,binContent0);
            hnew->SetBinContent(xbin,newybins+1,zbin,binContent2);
            if (oldSumw2) {
               hnew->SetBinError(xbin,0,zbin,TMath::Sqrt(binError0));
               hnew->SetBinError(xbin,newybins+1,zbin,TMath::Sqrt(binError2) );
            }
            oldzbin2 += nzgroup;
         }
         oldxbin2 += nxgroup;
      }

      //  recompute under/overflow contents in x for the new  y and z bins
      oldxbin2 = 1;
      oldybin2 = 1;
      oldzbin2 = 1;
      for (ybin = 1; ybin<=newybins; ybin++) {
         oldzbin2 = 1;
         for (zbin = 1; zbin<=newzbins; zbin++) {
            binContent0 = binContent2 = 0;
            binError0 = binError2 = 0;
            for (j=0; j<nygroup; j++) {
               if (oldybin2+j > nybins) break;
               for (k=0; k<nzgroup; k++) {
                  if (oldzbin2+k > nzbins) break;

                  //old underflow bin (in y)
                  ufbin = (oldybin2 + j)*(nxbins+2) + (nxbins+2)*(nybins+2)*(oldzbin2+k);
                  binContent0 += oldBins[ufbin];
                  if(oldSumw2) binError0 += oldSumw2[ufbin];
                  for(xbin = oldxbin; xbin <= nxbins + 1; xbin++){
                     //old overflow bin (in x)
                     ofbin = ufbin + xbin;
                     binContent2 += oldBins[ofbin];
                     if(oldSumw2) binError2 += oldSumw2[ofbin];
                  }
               }
            }
            hnew->SetBinContent(0,ybin,zbin,binContent0);
            hnew->SetBinContent(newxbins+1,ybin,zbin,binContent2);
            if (oldSumw2) {
               hnew->SetBinError(0,ybin,zbin,TMath::Sqrt(binError0));
               hnew->SetBinError(newxbins+1,ybin,zbin,TMath::Sqrt(binError2) );
            }
            oldzbin2 += nzgroup;
         }
         oldybin2 += nygroup;
      }

      //  recompute under/overflow contents in z for the new  x and y bins
      oldxbin2 = 1;
      oldybin2 = 1;
      oldzbin2 = 1;
      for (xbin = 1; xbin<=newxbins; xbin++) {
         oldybin2 = 1;
         for (ybin = 1; ybin<=newybins; ybin++) {
            binContent0 = binContent2 = 0;
            binError0 = binError2 = 0;
            for (i=0; i<nxgroup; i++) {
               if (oldxbin2+i > nxbins) break;
               for (j=0; j<nygroup; j++) {
                  if (oldybin2+j > nybins) break;
                  
                  //old underflow bin (in z)
                  ufbin = oldxbin2 + i + (nxbins+2)*(oldybin2+j);
                  binContent0 += oldBins[ufbin];
                  if(oldSumw2) binError0 += oldSumw2[ufbin];
                  for(zbin = oldzbin; zbin <= nzbins + 1; zbin++){
                     //old overflow bin (in z)
                     ofbin = ufbin + (nxbins+2)*(nybins+2)*zbin;
                     binContent2 += oldBins[ofbin];
                     if(oldSumw2) binError2 += oldSumw2[ofbin];
                  }
               }
            }
            hnew->SetBinContent(xbin,ybin,0,binContent0);
            hnew->SetBinContent(xbin,ybin,newzbins+1,binContent2);
            if (oldSumw2) {
               hnew->SetBinError(xbin,ybin,0,TMath::Sqrt(binError0));
               hnew->SetBinError(xbin,ybin,newzbins+1,TMath::Sqrt(binError2) );
            }
            oldybin2 += nygroup;
         }
         oldxbin2 += nxgroup;
      }

      //  recompute under/overflow contents in y, z for the new  x
      oldxbin2 = 1;
      oldybin2 = 1;
      oldzbin2 = 1;
      for (xbin = 1; xbin<=newxbins; xbin++) {
         binContent0 = 0;
         binContent2 = 0;
         binContent3 = 0;
         binContent4 = 0;
         binError0 = 0;
         binError2 = 0;
         binError3 = 0;
         binError4 = 0;
         for (i=0; i<nxgroup; i++) {
            if (oldxbin2+i > nxbins) break;
            ufbin = oldxbin2 + i; //
            binContent0 += oldBins[ufbin];
            if(oldSumw2) binError0 += oldSumw2[ufbin];

            for(ybin = oldybin; ybin <= nybins + 1; ybin++){
               ofbin3 =  ufbin+ybin*(nxbins+2);
               binContent3 += oldBins[ ofbin3 ];
               if (oldSumw2)  binError3 += oldSumw2[ofbin3];
               for(zbin = oldzbin; zbin <= nzbins + 1; zbin++){
                  //old overflow bin (in z)
                  ofbin4 =   oldxbin2 + i + ybin*(nxbins+2) + (nxbins+2)*(nybins+2)*zbin;
                  binContent4 += oldBins[ofbin4];
                  if(oldSumw2) binError4 += oldSumw2[ofbin4];
               }
            }
            for(zbin = oldzbin; zbin <= nzbins + 1; zbin++){
               ofbin2 =  ufbin+zbin*(nxbins+2)*(nybins+2);
               binContent2 += oldBins[ ofbin2 ];
               if (oldSumw2)  binError2 += oldSumw2[ofbin2];
            }
         }
         hnew->SetBinContent(xbin,0,0,binContent0);
         hnew->SetBinContent(xbin,0,newzbins+1,binContent2);
         hnew->SetBinContent(xbin,newybins+1,0,binContent3);
         hnew->SetBinContent(xbin,newybins+1,newzbins+1,binContent4);
         if (oldSumw2) {
            hnew->SetBinError(xbin,0,0,TMath::Sqrt(binError0));
            hnew->SetBinError(xbin,0,newzbins+1,TMath::Sqrt(binError2) );
            hnew->SetBinError(xbin,newybins+1,0,TMath::Sqrt(binError3) );
            hnew->SetBinError(xbin,newybins+1,newzbins+1,TMath::Sqrt(binError4) );
         }
         oldxbin2 += nxgroup;
      }


      //  recompute under/overflow contents in x, y for the new z
      oldxbin2 = 1;
      oldybin2 = 1;
      oldzbin2 = 1;
      for (zbin = 1; zbin<=newzbins; zbin++) {
         binContent0 = 0;
         binContent2 = 0;
         binContent3 = 0;
         binContent4 = 0;
         binError0 = 0;
         binError2 = 0;
         binError3 = 0;
         binError4 = 0;
         for (i=0; i<nzgroup; i++) {
            if (oldzbin2+i > nzbins) break;
            ufbin = (oldzbin2 + i)*(nxbins+2)*(nybins+2); //
            binContent0 += oldBins[ufbin];
            if(oldSumw2) binError0 += oldSumw2[ufbin];
            for(ybin = oldybin; ybin <= nybins + 1; ybin++){
               ofbin3 =  ufbin+ybin*(nxbins+2);
               binContent3 += oldBins[ ofbin3 ];
               if (oldSumw2)  binError3 += oldSumw2[ofbin3];
               for(xbin = oldxbin; xbin <= nxbins + 1; xbin++){
                  //old overflow bin (in z)
                  ofbin4 = ufbin + xbin + ybin*(nxbins+2);
                  binContent4 += oldBins[ofbin4];
                  if(oldSumw2) binError4 += oldSumw2[ofbin4];
               }
            }
            for(xbin = oldxbin; xbin <= nxbins + 1; xbin++){
               ofbin2 =  xbin +(oldzbin2+i)*(nxbins+2)*(nybins+2);
               binContent2 += oldBins[ ofbin2 ];
               if (oldSumw2)  binError2 += oldSumw2[ofbin2];
            }
         }
         hnew->SetBinContent(0,0,zbin,binContent0);
         hnew->SetBinContent(0,newybins+1,zbin,binContent3);
         hnew->SetBinContent(newxbins+1,0,zbin,binContent2);
         hnew->SetBinContent(newxbins+1,newybins+1,zbin,binContent4);
         if (oldSumw2) {
            hnew->SetBinError(0,0,zbin,TMath::Sqrt(binError0));
            hnew->SetBinError(0,newybins+1,zbin,TMath::Sqrt(binError3) );
            hnew->SetBinError(newxbins+1,0,zbin,TMath::Sqrt(binError2) );
            hnew->SetBinError(newxbins+1,newybins+1,zbin,TMath::Sqrt(binError4) );
         }
         oldzbin2 += nzgroup;
      }


      //  recompute under/overflow contents in x, z for the new  y
      oldxbin2 = 1;
      oldybin2 = 1;
      oldzbin2 = 1;
      for (ybin = 1; ybin<=newybins; ybin++) {
         binContent0 = 0;
         binContent2 = 0;
         binContent3 = 0;
         binContent4 = 0;
         binError0 = 0;
         binError2 = 0;
         binError3 = 0;
         binError4 = 0;
         for (i=0; i<nygroup; i++) {
            if (oldybin2+i > nybins) break;
            ufbin = (oldybin2 + i)*(nxbins+2); //
            binContent0 += oldBins[ufbin];
            if(oldSumw2) binError0 += oldSumw2[ufbin];
            for(xbin = oldxbin; xbin <= nxbins + 1; xbin++){
               ofbin3 =  ufbin+xbin;
               binContent3 += oldBins[ ofbin3 ];
               if (oldSumw2)  binError3 += oldSumw2[ofbin3];
               for(zbin = oldzbin; zbin <= nzbins + 1; zbin++){
                  //old overflow bin (in z)
                  ofbin4 = xbin + (nxbins+2)*(nybins+2)*zbin+(oldybin2+i)*(nxbins+2);
                  binContent4 += oldBins[ofbin4];
                  if(oldSumw2) binError4 += oldSumw2[ofbin4];
               }
            }
            for(zbin = oldzbin; zbin <= nzbins + 1; zbin++){
               ofbin2 =  (oldybin2+i)*(nxbins+2)+zbin*(nxbins+2)*(nybins+2);
               binContent2 += oldBins[ ofbin2 ];
               if (oldSumw2)  binError2 += oldSumw2[ofbin2];
            }
         }
         hnew->SetBinContent(0,ybin,0,binContent0);
         hnew->SetBinContent(0,ybin,newzbins+1,binContent2);
         hnew->SetBinContent(newxbins+1,ybin,0,binContent3);
         hnew->SetBinContent(newxbins+1,ybin,newzbins+1,binContent4);
         if (oldSumw2) {
            hnew->SetBinError(0,ybin,0,TMath::Sqrt(binError0));
            hnew->SetBinError(0,ybin,newzbins+1,TMath::Sqrt(binError2) );
            hnew->SetBinError(newxbins+1,ybin,0,TMath::Sqrt(binError3) );
            hnew->SetBinError(newxbins+1,ybin,newzbins+1,TMath::Sqrt(binError4) );
         }
         oldybin2 += nygroup;
      }
   }

   // Restore x axis attributes
   fXaxis.SetNdivisions(nXdivisions);
   fXaxis.SetAxisColor(xAxisColor);
   fXaxis.SetLabelColor(xLabelColor);
   fXaxis.SetLabelFont(xLabelFont);
   fXaxis.SetLabelOffset(xLabelOffset);
   fXaxis.SetLabelSize(xLabelSize);
   fXaxis.SetTickLength(xTickLength);
   fXaxis.SetTitleOffset(xTitleOffset);
   fXaxis.SetTitleSize(xTitleSize);
   fXaxis.SetTitleColor(xTitleColor);
   fXaxis.SetTitleFont(xTitleFont);
   // Restore y axis attributes
   fYaxis.SetNdivisions(nYdivisions);
   fYaxis.SetAxisColor(yAxisColor);
   fYaxis.SetLabelColor(yLabelColor);
   fYaxis.SetLabelFont(yLabelFont);
   fYaxis.SetLabelOffset(yLabelOffset);
   fYaxis.SetLabelSize(yLabelSize);
   fYaxis.SetTickLength(yTickLength);
   fYaxis.SetTitleOffset(yTitleOffset);
   fYaxis.SetTitleSize(yTitleSize);
   fYaxis.SetTitleColor(yTitleColor);
   fYaxis.SetTitleFont(yTitleFont);
   // Restore z axis attributes
   fZaxis.SetNdivisions(nZdivisions);
   fZaxis.SetAxisColor(zAxisColor);
   fZaxis.SetLabelColor(zLabelColor);
   fZaxis.SetLabelFont(zLabelFont);
   fZaxis.SetLabelOffset(zLabelOffset);
   fZaxis.SetLabelSize(zLabelSize);
   fZaxis.SetTickLength(zTickLength);
   fZaxis.SetTitleOffset(zTitleOffset);
   fZaxis.SetTitleSize(zTitleSize);
   fZaxis.SetTitleColor(zTitleColor);
   fZaxis.SetTitleFont(zTitleFont);
   
   //restore statistics and entries  modified by SetBinContent
   hnew->SetEntries(entries);
   if (!resetStat) hnew->PutStats(stat);
   
   delete [] oldBins;
   if (oldSumw2) delete [] oldSumw2;
   return hnew;
}





//______________________________________________________________________________
void TH3::Reset(Option_t *option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   TH1::Reset(option);
   TString opt = option;
   opt.ToUpper();
   if (opt.Contains("ICE") && !opt.Contains("S")) return;
   fTsumwy  = 0;
   fTsumwy2 = 0;
   fTsumwxy = 0;
   fTsumwz  = 0;
   fTsumwz2 = 0;
   fTsumwxz = 0;
   fTsumwyz = 0;
}

//______________________________________________________________________________
void TH3::Streamer(TBuffer &R__b)
{
   // Stream an object of class TH3.

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v > 2) {
         R__b.ReadClassBuffer(TH3::Class(), this, R__v, R__s, R__c);
         return;
      }
      //====process old versions before automatic schema evolution
      TH1::Streamer(R__b);
      TAtt3D::Streamer(R__b);
      R__b.CheckByteCount(R__s, R__c, TH3::IsA());
      //====end of old versions

   } else {
      R__b.WriteClassBuffer(TH3::Class(),this);
   }
}



//______________________________________________________________________________
//                     TH3C methods
//  TH3C a 3-D histogram with one byte per cell (char)
//______________________________________________________________________________

ClassImp(TH3C)

//______________________________________________________________________________
TH3C::TH3C(): TH3(), TArrayC()
{
   // Constructor.
   SetBinsLength(27);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3C::~TH3C()
{
   // Destructor.
}

//______________________________________________________________________________
TH3C::TH3C(const char *name,const char *title,Int_t nbinsx,Double_t xlow,Double_t xup
           ,Int_t nbinsy,Double_t ylow,Double_t yup
           ,Int_t nbinsz,Double_t zlow,Double_t zup)
           :TH3(name,title,nbinsx,xlow,xup,nbinsy,ylow,yup,nbinsz,zlow,zup)
{
   //*-*-*-*-*-*-*-*-*Normal constructor for fix bin size 3-D histograms*-*-*-*-*
   //*-*              ==================================================

   TArrayC::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();

   if (xlow >= xup || ylow >= yup || zlow >= zup) SetBuffer(fgBufferSize);
}

//______________________________________________________________________________
TH3C::TH3C(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins
           ,Int_t nbinsy,const Float_t *ybins
           ,Int_t nbinsz,const Float_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TArrayC::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3C::TH3C(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins
           ,Int_t nbinsy,const Double_t *ybins
           ,Int_t nbinsz,const Double_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TArrayC::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3C::TH3C(const TH3C &h3c) : TH3(), TArrayC()
{
   // Copy constructor.

   ((TH3C&)h3c).Copy(*this);
}

//______________________________________________________________________________
void TH3C::AddBinContent(Int_t bin)
{
   //*-*-*-*-*-*-*-*-*-*Increment bin content by 1*-*-*-*-*-*-*-*-*-*-*-*-*-*
   //*-*                ==========================

   if (fArray[bin] < 127) fArray[bin]++;
}

//______________________________________________________________________________
void TH3C::AddBinContent(Int_t bin, Double_t w)
{
   //*-*-*-*-*-*-*-*-*-*Increment bin content by w*-*-*-*-*-*-*-*-*-*-*-*-*-*
   //*-*                ==========================

   Int_t newval = fArray[bin] + Int_t(w);
   if (newval > -128 && newval < 128) {fArray[bin] = Char_t(newval); return;}
   if (newval < -127) fArray[bin] = -127;
   if (newval >  127) fArray[bin] =  127;
}

//______________________________________________________________________________
void TH3C::Copy(TObject &newth3) const
{
   //*-*-*-*-*-*-*Copy this 3-D histogram structure to newth3*-*-*-*-*-*-*-*-*-*
   //*-*          ===========================================

   TH3::Copy((TH3C&)newth3);
}

//______________________________________________________________________________
TH1 *TH3C::DrawCopy(Option_t *option) const
{
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if (gPad && !opt.Contains("same")) gPad->Clear();
   TH3C *newth3 = (TH3C*)Clone();
   newth3->SetDirectory(0);
   newth3->SetBit(kCanDelete);
   newth3->AppendPad(option);
   return newth3;
}

//______________________________________________________________________________
Double_t TH3C::GetBinContent(Int_t bin) const
{
   // Get bin content.

   if (fBuffer) ((TH3C*)this)->BufferEmpty();
   if (bin < 0) bin = 0;
   if (bin >= fNcells) bin = fNcells-1;
   if (!fArray) return 0;
   return Double_t (fArray[bin]);
}

//______________________________________________________________________________
void TH3C::Reset(Option_t *option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   TH3::Reset(option);
   TArrayC::Reset();
   // should also reset statistics once statistics are implemented for TH3
}

//______________________________________________________________________________
void TH3C::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if (n < 0) n = (fXaxis.GetNbins()+2)*(fYaxis.GetNbins()+2)*(fZaxis.GetNbins()+2);
   fNcells = n;
   TArrayC::Set(n);
}

//______________________________________________________________________________
void TH3C::SetBinContent(Int_t bin, Double_t content)
{
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if (bin < 0) return;
   if (bin >= fNcells) return;
   fArray[bin] = Char_t (content);
}


//______________________________________________________________________________
void TH3::SetShowProjection(const char *option,Int_t nbins)
{
   // When the mouse is moved in a pad containing a 3-d view of this histogram
   // a second canvas shows a projection type given as option.
   // To stop the generation of the projections, delete the canvas
   // containing the projection.
   // option may contain a combination of the characters x,y,z,e
   // option = "x" return the x projection into a TH1D histogram
   // option = "y" return the y projection into a TH1D histogram
   // option = "z" return the z projection into a TH1D histogram
   // option = "xy" return the x versus y projection into a TH2D histogram
   // option = "yx" return the y versus x projection into a TH2D histogram
   // option = "xz" return the x versus z projection into a TH2D histogram
   // option = "zx" return the z versus x projection into a TH2D histogram
   // option = "yz" return the y versus z projection into a TH2D histogram
   // option = "zy" return the z versus y projection into a TH2D histogram
   // option can also include the drawing option for the projection, eg to draw
   // the xy projection using the draw option "box" do
   //   myhist.SetShowProjection("xy box");
   // This function is typically called from the context menu.
   // NB: the notation "a vs b" means "a" vertical and "b" horizontal

   GetPainter();

   if (fPainter) fPainter->SetShowProjection(option,nbins);
}

//______________________________________________________________________________
void TH3C::Streamer(TBuffer &R__b)
{
   // Stream an object of class TH3C.

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      if (R__b.GetParent() && R__b.GetVersionOwner() < 22300) return;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v > 2) {
         R__b.ReadClassBuffer(TH3C::Class(), this, R__v, R__s, R__c);
         return;
      }
      //====process old versions before automatic schema evolution
      if (R__v < 2) {
         R__b.ReadVersion();
         TH1::Streamer(R__b);
         TArrayC::Streamer(R__b);
         R__b.ReadVersion(&R__s, &R__c);
         TAtt3D::Streamer(R__b);
      } else {
         TH3::Streamer(R__b);
         TArrayC::Streamer(R__b);
         R__b.CheckByteCount(R__s, R__c, TH3C::IsA());
      }
      //====end of old versions

   } else {
      R__b.WriteClassBuffer(TH3C::Class(),this);
   }
}

//______________________________________________________________________________
TH3C& TH3C::operator=(const TH3C &h1)
{
   // Operator =

   if (this != &h1)  ((TH3C&)h1).Copy(*this);
   return *this;
}

//______________________________________________________________________________
TH3C operator*(Float_t c1, TH3C &h1)
{
   // Operator *

   TH3C hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3C operator+(TH3C &h1, TH3C &h2)
{
   // Operator +

   TH3C hnew = h1;
   hnew.Add(&h2,1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3C operator-(TH3C &h1, TH3C &h2)
{
   // Operator -

   TH3C hnew = h1;
   hnew.Add(&h2,-1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3C operator*(TH3C &h1, TH3C &h2)
{
   // Operator *

   TH3C hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3C operator/(TH3C &h1, TH3C &h2)
{
   // Operator /

   TH3C hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(0);
   return hnew;
}


//______________________________________________________________________________
//                     TH3S methods
//  TH3S a 3-D histogram with two bytes per cell (short integer)
//______________________________________________________________________________

ClassImp(TH3S)

//______________________________________________________________________________
TH3S::TH3S(): TH3(), TArrayS()
{
   // Constructor.
   SetBinsLength(27);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3S::~TH3S()
{
   // Destructor.
}

//______________________________________________________________________________
TH3S::TH3S(const char *name,const char *title,Int_t nbinsx,Double_t xlow,Double_t xup
           ,Int_t nbinsy,Double_t ylow,Double_t yup
           ,Int_t nbinsz,Double_t zlow,Double_t zup)
           :TH3(name,title,nbinsx,xlow,xup,nbinsy,ylow,yup,nbinsz,zlow,zup)
{
   //*-*-*-*-*-*-*-*-*Normal constructor for fix bin size 3-D histograms*-*-*-*-*
   //*-*              ==================================================
   TH3S::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();

   if (xlow >= xup || ylow >= yup || zlow >= zup) SetBuffer(fgBufferSize);
}

//______________________________________________________________________________
TH3S::TH3S(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins
           ,Int_t nbinsy,const Float_t *ybins
           ,Int_t nbinsz,const Float_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TH3S::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3S::TH3S(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins
           ,Int_t nbinsy,const Double_t *ybins
           ,Int_t nbinsz,const Double_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TH3S::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3S::TH3S(const TH3S &h3s) : TH3(), TArrayS()
{
   // Copy Constructor.

   ((TH3S&)h3s).Copy(*this);
}

//______________________________________________________________________________
void TH3S::AddBinContent(Int_t bin)
{
   //*-*-*-*-*-*-*-*-*-*Increment bin content by 1*-*-*-*-*-*-*-*-*-*-*-*-*-*
   //*-*                ==========================

   if (fArray[bin] < 32767) fArray[bin]++;
}

//______________________________________________________________________________
void TH3S::AddBinContent(Int_t bin, Double_t w)
{
   //*-*-*-*-*-*-*-*-*-*Increment bin content by w*-*-*-*-*-*-*-*-*-*-*-*-*-*
   //*-*                ==========================

   Int_t newval = fArray[bin] + Int_t(w);
   if (newval > -32768 && newval < 32768) {fArray[bin] = Short_t(newval); return;}
   if (newval < -32767) fArray[bin] = -32767;
   if (newval >  32767) fArray[bin] =  32767;
}

//______________________________________________________________________________
void TH3S::Copy(TObject &newth3) const
{
   //*-*-*-*-*-*-*Copy this 3-D histogram structure to newth3*-*-*-*-*-*-*-*-*-*
   //*-*          ===========================================

   TH3::Copy((TH3S&)newth3);
}

//______________________________________________________________________________
TH1 *TH3S::DrawCopy(Option_t *option) const
{
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if (gPad && !opt.Contains("same")) gPad->Clear();
   TH3S *newth3 = (TH3S*)Clone();
   newth3->SetDirectory(0);
   newth3->SetBit(kCanDelete);
   newth3->AppendPad(option);
   return newth3;
}

//______________________________________________________________________________
Double_t TH3S::GetBinContent(Int_t bin) const
{
   // Get bin content.

   if (fBuffer) ((TH3S*)this)->BufferEmpty();
   if (bin < 0) bin = 0;
   if (bin >= fNcells) bin = fNcells-1;
   if (!fArray) return 0;
   return Double_t (fArray[bin]);
}

//______________________________________________________________________________
void TH3S::Reset(Option_t *option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   TH3::Reset(option);
   TArrayS::Reset();
   // should also reset statistics once statistics are implemented for TH3
}

//______________________________________________________________________________
void TH3S::SetBinContent(Int_t bin, Double_t content)
{
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if (bin < 0) return;
   if (bin >= fNcells) return;
   fArray[bin] = Short_t (content);
}

//______________________________________________________________________________
void TH3S::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if (n < 0) n = (fXaxis.GetNbins()+2)*(fYaxis.GetNbins()+2)*(fZaxis.GetNbins()+2);
   fNcells = n;
   TArrayS::Set(n);
}

//______________________________________________________________________________
void TH3S::Streamer(TBuffer &R__b)
{
   // Stream an object of class TH3S.

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      if (R__b.GetParent() && R__b.GetVersionOwner() < 22300) return;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v > 2) {
         R__b.ReadClassBuffer(TH3S::Class(), this, R__v, R__s, R__c);
         return;
      }
      //====process old versions before automatic schema evolution
      if (R__v < 2) {
         R__b.ReadVersion();
         TH1::Streamer(R__b);
         TArrayS::Streamer(R__b);
         R__b.ReadVersion(&R__s, &R__c);
         TAtt3D::Streamer(R__b);
      } else {
         TH3::Streamer(R__b);
         TArrayS::Streamer(R__b);
         R__b.CheckByteCount(R__s, R__c, TH3S::IsA());
      }
      //====end of old versions

   } else {
      R__b.WriteClassBuffer(TH3S::Class(),this);
   }
}

//______________________________________________________________________________
TH3S& TH3S::operator=(const TH3S &h1)
{
   // Operator =

   if (this != &h1)  ((TH3S&)h1).Copy(*this);
   return *this;
}

//______________________________________________________________________________
TH3S operator*(Float_t c1, TH3S &h1)
{
   // Operator *

   TH3S hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3S operator+(TH3S &h1, TH3S &h2)
{
   // Operator +

   TH3S hnew = h1;
   hnew.Add(&h2,1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3S operator-(TH3S &h1, TH3S &h2)
{
   // Operator -

   TH3S hnew = h1;
   hnew.Add(&h2,-1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3S operator*(TH3S &h1, TH3S &h2)
{
   // Operator *

   TH3S hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3S operator/(TH3S &h1, TH3S &h2)
{
   // Operator /

   TH3S hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(0);
   return hnew;
}


//______________________________________________________________________________
//                     TH3I methods
//  TH3I a 3-D histogram with four bytes per cell (32 bits integer)
//______________________________________________________________________________

ClassImp(TH3I)

//______________________________________________________________________________
TH3I::TH3I(): TH3(), TArrayI()
{
   // Constructor.
   SetBinsLength(27);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3I::~TH3I()
{
   // Destructor.
}

//______________________________________________________________________________
TH3I::TH3I(const char *name,const char *title,Int_t nbinsx,Double_t xlow,Double_t xup
           ,Int_t nbinsy,Double_t ylow,Double_t yup
           ,Int_t nbinsz,Double_t zlow,Double_t zup)
           :TH3(name,title,nbinsx,xlow,xup,nbinsy,ylow,yup,nbinsz,zlow,zup)
{
   //*-*-*-*-*-*-*-*-*Normal constructor for fix bin size 3-D histograms*-*-*-*-*
   //*-*              ==================================================
   TH3I::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();

   if (xlow >= xup || ylow >= yup || zlow >= zup) SetBuffer(fgBufferSize);
}

//______________________________________________________________________________
TH3I::TH3I(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins
           ,Int_t nbinsy,const Float_t *ybins
           ,Int_t nbinsz,const Float_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TArrayI::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3I::TH3I(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins
           ,Int_t nbinsy,const Double_t *ybins
           ,Int_t nbinsz,const Double_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TArrayI::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3I::TH3I(const TH3I &h3i) : TH3(), TArrayI()
{
   // Copy constructor.

   ((TH3I&)h3i).Copy(*this);
}

//______________________________________________________________________________
void TH3I::AddBinContent(Int_t bin)
{
   //*-*-*-*-*-*-*-*-*-*Increment bin content by 1*-*-*-*-*-*-*-*-*-*-*-*-*-*
   //*-*                ==========================

   if (fArray[bin] < 2147483647) fArray[bin]++;
}

//______________________________________________________________________________
void TH3I::AddBinContent(Int_t bin, Double_t w)
{
   //*-*-*-*-*-*-*-*-*-*Increment bin content by w*-*-*-*-*-*-*-*-*-*-*-*-*-*
   //*-*                ==========================

   Int_t newval = fArray[bin] + Int_t(w);
   if (newval > -2147483647 && newval < 2147483647) {fArray[bin] = Int_t(newval); return;}
   if (newval < -2147483647) fArray[bin] = -2147483647;
   if (newval >  2147483647) fArray[bin] =  2147483647;
}

//______________________________________________________________________________
void TH3I::Copy(TObject &newth3) const
{
   //*-*-*-*-*-*-*Copy this 3-D histogram structure to newth3*-*-*-*-*-*-*-*-*-*
   //*-*          ===========================================

   TH3::Copy((TH3I&)newth3);
}

//______________________________________________________________________________
TH1 *TH3I::DrawCopy(Option_t *option) const
{
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if (gPad && !opt.Contains("same")) gPad->Clear();
   TH3I *newth3 = (TH3I*)Clone();
   newth3->SetDirectory(0);
   newth3->SetBit(kCanDelete);
   newth3->AppendPad(option);
   return newth3;
}

//______________________________________________________________________________
Double_t TH3I::GetBinContent(Int_t bin) const
{
   // Get bin content.

   if (fBuffer) ((TH3I*)this)->BufferEmpty();
   if (bin < 0) bin = 0;
   if (bin >= fNcells) bin = fNcells-1;
   if (!fArray) return 0;
   return Double_t (fArray[bin]);
}

//______________________________________________________________________________
void TH3I::Reset(Option_t *option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   TH3::Reset(option);
   TArrayI::Reset();
   // should also reset statistics once statistics are implemented for TH3
}

//______________________________________________________________________________
void TH3I::SetBinContent(Int_t bin, Double_t content)
{
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if (bin < 0) return;
   if (bin >= fNcells) return;
   fArray[bin] = Int_t (content);
}

//______________________________________________________________________________
void TH3I::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if (n < 0) n = (fXaxis.GetNbins()+2)*(fYaxis.GetNbins()+2)*(fZaxis.GetNbins()+2);
   fNcells = n;
   TArrayI::Set(n);
}

//______________________________________________________________________________
TH3I& TH3I::operator=(const TH3I &h1)
{
   // Operator =

   if (this != &h1)  ((TH3I&)h1).Copy(*this);
   return *this;
}

//______________________________________________________________________________
TH3I operator*(Float_t c1, TH3I &h1)
{
   // Operator *

   TH3I hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3I operator+(TH3I &h1, TH3I &h2)
{
   // Operator +

   TH3I hnew = h1;
   hnew.Add(&h2,1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3I operator-(TH3I &h1, TH3I &h2)
{
   // Operator _

   TH3I hnew = h1;
   hnew.Add(&h2,-1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3I operator*(TH3I &h1, TH3I &h2)
{
   // Operator *

   TH3I hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3I operator/(TH3I &h1, TH3I &h2)
{
   // Operator /

   TH3I hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(0);
   return hnew;
}


//______________________________________________________________________________
//                     TH3F methods
//  TH3F a 3-D histogram with four bytes per cell (float)
//______________________________________________________________________________

ClassImp(TH3F)

//______________________________________________________________________________
TH3F::TH3F(): TH3(), TArrayF()
{
   // Constructor.
   SetBinsLength(27);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3F::~TH3F()
{
   // Destructor.
}

//______________________________________________________________________________
TH3F::TH3F(const char *name,const char *title,Int_t nbinsx,Double_t xlow,Double_t xup
           ,Int_t nbinsy,Double_t ylow,Double_t yup
           ,Int_t nbinsz,Double_t zlow,Double_t zup)
           :TH3(name,title,nbinsx,xlow,xup,nbinsy,ylow,yup,nbinsz,zlow,zup)
{
   //*-*-*-*-*-*-*-*-*Normal constructor for fix bin size 3-D histograms*-*-*-*-*
   //*-*              ==================================================
   TArrayF::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();

   if (xlow >= xup || ylow >= yup || zlow >= zup) SetBuffer(fgBufferSize);
}

//______________________________________________________________________________
TH3F::TH3F(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins
           ,Int_t nbinsy,const Float_t *ybins
           ,Int_t nbinsz,const Float_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TArrayF::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3F::TH3F(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins
           ,Int_t nbinsy,const Double_t *ybins
           ,Int_t nbinsz,const Double_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TArrayF::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3F::TH3F(const TH3F &h3f) : TH3(), TArrayF()
{
   // Copy constructor.

   ((TH3F&)h3f).Copy(*this);
}

//______________________________________________________________________________
void TH3F::Copy(TObject &newth3) const
{
   //*-*-*-*-*-*-*Copy this 3-D histogram structure to newth3*-*-*-*-*-*-*-*-*-*
   //*-*          ===========================================

   TH3::Copy((TH3F&)newth3);
}

//______________________________________________________________________________
TH1 *TH3F::DrawCopy(Option_t *option) const
{
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if (gPad && !opt.Contains("same")) gPad->Clear();
   TH3F *newth3 = (TH3F*)Clone();
   newth3->SetDirectory(0);
   newth3->SetBit(kCanDelete);
   newth3->AppendPad(option);
   return newth3;
}

//______________________________________________________________________________
Double_t TH3F::GetBinContent(Int_t bin) const
{
   // Get bin content.

   if (fBuffer) ((TH3F*)this)->BufferEmpty();
   if (bin < 0) bin = 0;
   if (bin >= fNcells) bin = fNcells-1;
   if (!fArray) return 0;
   return Double_t (fArray[bin]);
}

//______________________________________________________________________________
void TH3F::Reset(Option_t *option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   TH3::Reset(option);
   TArrayF::Reset();
   // should also reset statistics once statistics are implemented for TH3
}

//______________________________________________________________________________
void TH3F::SetBinContent(Int_t bin, Double_t content)
{
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if (bin < 0) return;
   if (bin >= fNcells) return;
   fArray[bin] = Float_t (content);
}

//______________________________________________________________________________
void TH3F::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if (n < 0) n = (fXaxis.GetNbins()+2)*(fYaxis.GetNbins()+2)*(fZaxis.GetNbins()+2);
   fNcells = n;
   TArrayF::Set(n);
}

//______________________________________________________________________________
void TH3F::Streamer(TBuffer &R__b)
{
   // Stream an object of class TH3F.

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      if (R__b.GetParent() && R__b.GetVersionOwner() < 22300) return;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v > 2) {
         R__b.ReadClassBuffer(TH3F::Class(), this, R__v, R__s, R__c);
         return;
      }
      //====process old versions before automatic schema evolution
      if (R__v < 2) {
         R__b.ReadVersion();
         TH1::Streamer(R__b);
         TArrayF::Streamer(R__b);
         R__b.ReadVersion(&R__s, &R__c);
         TAtt3D::Streamer(R__b);
      } else {
         TH3::Streamer(R__b);
         TArrayF::Streamer(R__b);
         R__b.CheckByteCount(R__s, R__c, TH3F::IsA());
      }
      //====end of old versions

   } else {
      R__b.WriteClassBuffer(TH3F::Class(),this);
   }
}

//______________________________________________________________________________
TH3F& TH3F::operator=(const TH3F &h1)
{
   // Operator =

   if (this != &h1)  ((TH3F&)h1).Copy(*this);
   return *this;
}

//______________________________________________________________________________
TH3F operator*(Float_t c1, TH3F &h1)
{
   // Operator *

   TH3F hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3F operator+(TH3F &h1, TH3F &h2)
{
   // Operator +

   TH3F hnew = h1;
   hnew.Add(&h2,1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3F operator-(TH3F &h1, TH3F &h2)
{
   // Operator -

   TH3F hnew = h1;
   hnew.Add(&h2,-1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3F operator*(TH3F &h1, TH3F &h2)
{
   // Operator *

   TH3F hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3F operator/(TH3F &h1, TH3F &h2)
{
   // Operator /

   TH3F hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(0);
   return hnew;
}


//______________________________________________________________________________
//                     TH3D methods
//  TH3D a 3-D histogram with eight bytes per cell (double)
//______________________________________________________________________________

ClassImp(TH3D)

//______________________________________________________________________________
TH3D::TH3D(): TH3(), TArrayD()
{
   // Constructor.
   SetBinsLength(27);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3D::~TH3D()
{
   // Destructor.
}

//______________________________________________________________________________
TH3D::TH3D(const char *name,const char *title,Int_t nbinsx,Double_t xlow,Double_t xup
           ,Int_t nbinsy,Double_t ylow,Double_t yup
           ,Int_t nbinsz,Double_t zlow,Double_t zup)
           :TH3(name,title,nbinsx,xlow,xup,nbinsy,ylow,yup,nbinsz,zlow,zup)
{
   //*-*-*-*-*-*-*-*-*Normal constructor for fix bin size 3-D histograms*-*-*-*-*
   //*-*              ==================================================
   TArrayD::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();

   if (xlow >= xup || ylow >= yup || zlow >= zup) SetBuffer(fgBufferSize);
}

//______________________________________________________________________________
TH3D::TH3D(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins
           ,Int_t nbinsy,const Float_t *ybins
           ,Int_t nbinsz,const Float_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TArrayD::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3D::TH3D(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins
           ,Int_t nbinsy,const Double_t *ybins
           ,Int_t nbinsz,const Double_t *zbins)
           :TH3(name,title,nbinsx,xbins,nbinsy,ybins,nbinsz,zbins)
{
   //*-*-*-*-*-*-*-*Normal constructor for variable bin size 3-D histograms*-*-*-*
   //*-*            =======================================================
   TArrayD::Set(fNcells);
   if (fgDefaultSumw2) Sumw2();
}

//______________________________________________________________________________
TH3D::TH3D(const TH3D &h3d) : TH3(), TArrayD()
{
   // Copy constructor.

   ((TH3D&)h3d).Copy(*this);
}

//______________________________________________________________________________
void TH3D::Copy(TObject &newth3) const
{
   //*-*-*-*-*-*-*Copy this 3-D histogram structure to newth3*-*-*-*-*-*-*-*-*-*
   //*-*          ===========================================

   TH3::Copy((TH3D&)newth3);
}

//______________________________________________________________________________
TH1 *TH3D::DrawCopy(Option_t *option) const
{
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if (gPad && !opt.Contains("same")) gPad->Clear();
   TH3D *newth3 = (TH3D*)Clone();
   newth3->SetDirectory(0);
   newth3->SetBit(kCanDelete);
   newth3->AppendPad(option);
   return newth3;
}

//______________________________________________________________________________
Double_t TH3D::GetBinContent(Int_t bin) const
{
   // Get bin content.

   if (fBuffer) ((TH3D*)this)->BufferEmpty();
   if (bin < 0) bin = 0;
   if (bin >= fNcells) bin = fNcells-1;
   if (!fArray) return 0;
   return Double_t (fArray[bin]);
}

//______________________________________________________________________________
void TH3D::Reset(Option_t *option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   TH3::Reset(option);
   TArrayD::Reset();
   // should also reset statistics once statistics are implemented for TH3
}

//______________________________________________________________________________
void TH3D::SetBinContent(Int_t bin, Double_t content)
{
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if (bin < 0) return;
   if (bin >= fNcells) return;
   fArray[bin] = Double_t (content);
}


//______________________________________________________________________________
void TH3D::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if (n < 0) n = (fXaxis.GetNbins()+2)*(fYaxis.GetNbins()+2)*(fZaxis.GetNbins()+2);
   fNcells = n;
   TArrayD::Set(n);
}

//______________________________________________________________________________
void TH3D::Streamer(TBuffer &R__b)
{
   // Stream an object of class TH3D.

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      if (R__b.GetParent() && R__b.GetVersionOwner() < 22300) return;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v > 2) {
         R__b.ReadClassBuffer(TH3D::Class(), this, R__v, R__s, R__c);
         return;
      }
      //====process old versions before automatic schema evolution
      if (R__v < 2) {
         R__b.ReadVersion();
         TH1::Streamer(R__b);
         TArrayD::Streamer(R__b);
         R__b.ReadVersion(&R__s, &R__c);
         TAtt3D::Streamer(R__b);
      } else {
         TH3::Streamer(R__b);
         TArrayD::Streamer(R__b);
         R__b.CheckByteCount(R__s, R__c, TH3D::IsA());
      }
      //====end of old versions

   } else {
      R__b.WriteClassBuffer(TH3D::Class(),this);
   }
}

//______________________________________________________________________________
TH3D& TH3D::operator=(const TH3D &h1)
{
   // Operator =

   if (this != &h1)  ((TH3D&)h1).Copy(*this);
   return *this;
}

//______________________________________________________________________________
TH3D operator*(Float_t c1, TH3D &h1)
{
   // Operator *

   TH3D hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3D operator+(TH3D &h1, TH3D &h2)
{
   // Operator +

   TH3D hnew = h1;
   hnew.Add(&h2,1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3D operator-(TH3D &h1, TH3D &h2)
{
   // Operator -

   TH3D hnew = h1;
   hnew.Add(&h2,-1);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3D operator*(TH3D &h1, TH3D &h2)
{
   // Operator *

   TH3D hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

//______________________________________________________________________________
TH3D operator/(TH3D &h1, TH3D &h2)
{
   // Operator /

   TH3D hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(0);
   return hnew;
}
