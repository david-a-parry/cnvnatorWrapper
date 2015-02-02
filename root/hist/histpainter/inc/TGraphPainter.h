// @(#)root/histpainter:$Id: TGraphPainter.h,v 1.00
// Author: Olivier Couet

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGraphPainter
#define ROOT_TGraphPainter


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TGraphPainter                                                        //
//                                                                      //
// helper class to draw graphs                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_Object
#include "TVirtualGraphPainter.h"
#endif

class TGraph;
class TF1;

class TGraphPainter : public TVirtualGraphPainter {

public:

   TGraphPainter();

   virtual ~TGraphPainter();

   void           ComputeLogs(Int_t npoints, Int_t opt);
   virtual Int_t  DistancetoPrimitiveHelper(TGraph *theGraph, Int_t px, Int_t py);
   virtual void   DrawPanelHelper(TGraph *theGraph);
   virtual void   ExecuteEventHelper(TGraph *theGraph, Int_t event, Int_t px, Int_t py);
   virtual char  *GetObjectInfoHelper(TGraph *theGraph, Int_t px, Int_t py) const;
   void           PaintHelper(TGraph *theGraph, Option_t *option);
   virtual void   PaintGraph(TGraph *theGraph, Int_t npoints, const Double_t *x, const Double_t *y, Option_t *chopt);
   virtual void   PaintGrapHist(TGraph *theGraph, Int_t npoints, const Double_t *x, const Double_t *y, Option_t *chopt);
   void           PaintGraphAsymmErrors(TGraph *theGraph, Option_t *option);
   void           PaintGraphBentErrors(TGraph *theGraph, Option_t *option);
   void           PaintGraphErrors(TGraph *theGraph, Option_t *option);
   void           PaintGraphPolar(TGraph *theGraph, Option_t *option);
   void           PaintGraphQQ(TGraph *theGraph, Option_t *option);
   void           PaintGraphSimple(TGraph *theGraph, Option_t *option);
   void           PaintPolyLineHatches(TGraph *theGraph, Int_t n, const Double_t *x, const Double_t *y);
   void           PaintStats(TGraph *theGraph, TF1 *fit);
   void           Smooth(TGraph *theGraph, Int_t npoints, Double_t *x, Double_t *y, Int_t drawtype);

   ClassDef(TGraphPainter,0)  // TGraph painter
};

#endif
