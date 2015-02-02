// @(#)root/histpainter:$Id: TGraphPainter.cxx,v 1.00
// Author: Olivier Couet

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TROOT.h"
#include "TGraphPainter.h"
#include "TMath.h"
#include "TGraph.h"
#include "TPolyLine.h"
#include "TPolyMarker.h"
#include "TVirtualPad.h"
#include "TView.h"
#include "TStyle.h"
#include "TH1.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TGaxis.h"
#include "TGraphAsymmErrors.h"
#include "TGraphBentErrors.h"
#include "TGraphPolargram.h"
#include "TGraphPolar.h"
#include "TGraphQQ.h"
#include "TLatex.h"
#include "TArrow.h"
#include "TFrame.h"
#include "TVirtualPadEditor.h"

Double_t *gxwork, *gywork, *gxworkl, *gyworkl;

ClassImp(TGraphPainter);


//______________________________________________________________________________
/* Begin_Html
<center><h2>The graph painter class</h2></center>

<ul>
<li><a href="#GP00">Introduction</li></a>
<li><a href="#GP01">Graphs' plotting options</li></a>
<li><a href="#GP02">Exclusion graphs</li></a>
<li><a href="#GP03">Graphs with error bars</li></a>
<ul>
<li><a href="#GP03a">TGraphErrors</li></a>
<li><a href="#GP03b">TGraphAsymmErrors</li></a>
<li><a href="#GP03c">TGraphBentErrors</li></a>
</ul>
<li><a href="#GP04">TGraphPolar options</li></a>
</ul>

<a name="GP00"></a><h3>Introduction</h3>

Graphs are drawn via the painter <tt>TGraphPainter</tt> class. This class
implements techniques needed to display the various kind of
graphs i.e.: <tt>TGraph</tt>, <tt>TGraphErrors</tt>,
<tt>TGraphBentErrors</tt> and <tt>TGraphAsymmErrors</tt>.

<p>
To draw a graph "<tt>graph</tt>" it's enough to do:
<pre>
   graph->Draw("AL");
</pre>


<p>The option <tt>"AL"</tt> in the <tt>Draw()</tt> method means:
<ol>
<li>The axis should be drawn (option <tt>"A"</tt>),</li>
<li>The graph should be drawn as a simple line (option <tt>"L"</tt>).</li>
</ol>
By default a graph is drawn in the current pad in the current coordinate system.
To define a suitable coordinate system and draw the axis the option
<tt>"A"</tt> must be specified.
<p>
<tt>TGraphPainter</tt> offers many options to paint the various kind of graphs.
<p>
It is separated from the graph classes so that one can have graphs without the
graphics overhead, for example in a batch program.
<p>
When a displayed graph is modified, there is no need to call
<tt>Draw()</tt> again; the image will be refreshed the next time the
pad will be updated.
<p>A pad is updated after one of these three actions:
<ol>
<li>  a carriage return on the ROOT command line,
<li>  a click inside the pad,
<li>  a call to <tt>TPad::Update</tt>.
</ol>

<a name="GP01"></a><h3>Graphs' plotting options</h3>
Graphs can be drawn with the following options:
<p>
<table border=0>

<tr><th valign=top>"A"</th><td>
Axis are drawn around the graph
</td></tr>

<tr><th valign=top>"L"</th><td>
A simple polyline is drawn
</td></tr>

<tr><th valign=top>"F"</th><td>
A fill area is drawn ('CF' draw a smoothed fill area)
</td></tr>

<tr><th valign=top>"C"</th><td>
A smooth Curve is drawn
</td></tr>

<tr><th valign=top>"*"</th><td>
A Star is plotted at each point
</td></tr>

<tr><th valign=top>"P"</th><td>
The current marker is plotted at each point
</td></tr>

<tr><th valign=top>"B"</th><td>
A Bar chart is drawn
</td></tr>

<tr><th valign=top>"1"</th><td>
When a graph is drawn as a bar chart, this option makes the bars start from
the bottom of the pad. By default they start at 0.
</td></tr>

<tr><th valign=top>"X+"</th><td>
The X-axis is drawn on the top side of the plot.
</td></tr>

<tr><th valign=top>"Y+"</th><td>
The Y-axis is drawn on the right side of the plot.
</td></tr>

</table>
<p>

Drawing options can be combined. In the following example the graph
is drawn as a smooth curve (option "C") with markers (option "P") and
with axes (option "A").

End_Html
Begin_Macro(source)
{
   TCanvas *c1 = new TCanvas("c1","c1",200,10,600,400);

   c1->SetFillColor(42);
   c1->SetGrid();

   const Int_t n = 20;
   Double_t x[n], y[n];
   for (Int_t i=0;i<n;i++) {
      x[i] = i*0.1;
      y[i] = 10*sin(x[i]+0.2);
   }
   gr = new TGraph(n,x,y);
   gr->SetLineColor(2);
   gr->SetLineWidth(4);
   gr->SetMarkerColor(4);
   gr->SetMarkerSize(1.5);
   gr->SetMarkerStyle(21);
   gr->SetTitle("Option ACP example");
   gr->GetXaxis()->SetTitle("X title");
   gr->GetYaxis()->SetTitle("Y title");
   gr->Draw("ACP");

   // TCanvas::Update() draws the frame, after which one can change it
   c1->Update();
   c1->GetFrame()->SetFillColor(21);
   c1->GetFrame()->SetBorderSize(12);
   c1->Modified();
   return c1;
}
End_Macro
Begin_Html

<p>The following macro shows the option "B" usage. It can be combined with the
option "1".

End_Html
Begin_Macro(source)
{
   TCanvas *c47 = new TCanvas("c47","c47",200,10,600,400);
   c47->Divide(1,2);
   const Int_t n = 20;
   Double_t x[n], y[n];
   for (Int_t i=0;i<n;i++) {
      x[i] = i*0.1;
      y[i] = 10*sin(x[i]+0.2)-6;
   }
   gr = new TGraph(n,x,y);
   gr->SetFillColor(38);
   c47->cd(1); gr->Draw("AB");
   c47->cd(2); gr->Draw("AB1");
 return c47;
}
End_Macro
Begin_Html

<a name="GP02"></a><h3>Exclusion graphs</h3>

When a graph is painted with the option <tt>"C"</tt> or <tt>"L"</tt> it is
possible to draw a filled area on one side of the line. This is useful to show
exclusion zones.

<p>This drawing mode is activated when the absolute value of the graph line
width (set by <tt>SetLineWidth()</tt>) is greater than 99. In that
case the line width number is interpreted as:
<pre>
      100*ff+ll = ffll
</pre>
<ul>
<li> The two digits number <tt>"ll"</tt> represent the normal line width
<li> The two digits number  <tt>"ff"</tt> represent the filled area width.
<li> The sign of "ffll" allows to flip the filled area from one side of the line
     to the other.
</ul>
The current fill area attributes are used to draw the hatched zone.

End_Html
Begin_Macro(source)
../../../tutorials/graphs/exclusiongraph.C
End_Macro
Begin_Html

<a name="GP03"></a><h3>Graphs with error bars</h3>
Three classes are available to handle graphs with error bars:
<tt>TGraphErrors</tt>, <tt>TGraphAsymmErrors</tt> and <tt>TGraphBentErrors</tt>.
The following drawing options are specific to graphs with error bars:
<p>
<table border=0>

<tr><th valign=top>"Z"</th><td>
Do not draw small horizontal and vertical lines the end of the error bars.
Without "Z", the default is to draw these.
</td></tr>

<tr><th valign=top>">"</th><td>
An arrow is drawn at the end of the error bars.
The size of the arrow is set to 2/3 of the marker size.
</td></tr>

<tr><th valign=top>"|>"</th><td>
A filled arrow is drawn at the end of the error bars.
The size of the arrow is set to 2/3 of the marker size.
</td></tr>

<tr><th valign=top>"X"</th><td>
Do not draw error bars.  By default, graph classes that have errors
are drawn with the errors (TGraph itself has no errors, and so this option
has no effect.)
</td></tr>

<tr><th valign=top>"||"</th><td>
Draw only the small vertical/horizontal lines at the ends of the
error bars, without drawing the bars themselves. This option is
interesting to superimpose statistical-only errors on top of a graph
with statistical+systematic errors.
</td></tr>

<tr><th valign=top>"[]"</th><td>
Does the same as option "||" except that it draws additional marks at the
ends of the small vertical/horizontal lines. It makes plots less ambiguous
in case several graphs are drawn on the same picture.
</td></tr>

<tr><th valign=top>"0"</th><td>
By default, when a data point is outside the visible range along the Y
axis, the error bars are not drawn. This option forces error bars' drawing for
the data points outside the visible range along the Y axis (see example below).
</td></tr>

<tr><th valign=top>"2"</th><td>
Error rectangles are drawn.
</td></tr>

<tr><th valign=top>"3"</th><td>
A filled area is drawn through the end points of the vertical error bars.
</td></tr>

<tr><th valign=top>"4"</th><td>
A smoothed filled area is drawn through the end points of the vertical error
bars.
</td></tr>

<tr><th valign=top>"5"</th><td>
Error rectangles are drawn like option "2". In addition the contour line
around the boxes is drawn. This can be useful when boxes' fill colors are very
light or in gray scale mode.
</td></tr>
</table>
<p>
<tt>gStyle->SetErrorX(dx)</tt> controls the size of the error along x.
<tt>dx = 0</tt> removes the error along x.
<p>
<tt>gStyle->SetEndErrorSize(np)</tt> controls the size of the lines
at the end of the error bars (when option 1 is used).
By default <tt>np=1</tt>. (np represents the number of pixels).

<a name="GP03a"></a><h4><u>TGraphErrors</u></h4>
A <tt>TGraphErrors</tt> is a <tt>TGraph</tt> with error bars. The errors are
defined along X and Y and are symmetric: The left and right errors are the same
along X and the bottom and up errors are the same along Y.

End_Html
Begin_Macro(source)
{
   TCanvas *c4 = new TCanvas("c4","c4",200,10,600,400);
   double x[] = {0, 1, 2, 3, 4};
   double y[] = {0, 2, 4, 1, 3};
   double ex[] = {0.1, 0.2, 0.3, 0.4, 0.5};
   double ey[] = {1, 0.5, 1, 0.5, 1};
   TGraphErrors* ge = new TGraphErrors(5, x, y, ex, ey);
   ge->Draw("ap");
   return c4;
}
End_Macro
Begin_Html

<p>The option "0" shows the error bars for data points outside range.

End_Html
Begin_Macro(source)
{
   TCanvas *c48 = new TCanvas("c48","c48",200,10,600,400);
   float x[]     = {1,2,3};
   float err_x[] = {0,0,0};
   float err_y[] = {5,5,5};
   float y[]     = {1,4,9};
   TGraphErrors tg(3,x,y,err_x,err_y);
   c48->Divide(2,1);
   c48->cd(1); gPad->DrawFrame(0,0,4,8); tg.Draw("PC");
   c48->cd(2); gPad->DrawFrame(0,0,4,8); tg.Draw("0PC");
   return c48;
}
End_Macro
Begin_Html

<p>The option "3" shows the errors as a band.

End_Html
Begin_Macro(source)
{
   TCanvas *c41 = new TCanvas("c41","c41",200,10,600,400);
   double x[] = {0, 1, 2, 3, 4};
   double y[] = {0, 2, 4, 1, 3};
   double ex[] = {0.1, 0.2, 0.3, 0.4, 0.5};
   double ey[] = {1, 0.5, 1, 0.5, 1};
   TGraphErrors* ge = new TGraphErrors(5, x, y, ex, ey);
   ge->SetFillColor(4);
   ge->SetFillStyle(3010);
   ge->Draw("a3");
   return c41;
}
End_Macro
Begin_Html

<p>The option "4" is similar to the option "3" except that the band
is smoothed. As the following picture shows, this option should be
used carefully because the smoothing algorithm may show some (huge)
"bouncing" effects. In some cases it looks nicer than option "3"
(because it is smooth) but it can be misleading.

End_Html
Begin_Macro(source)
{
   TCanvas *c42 = new TCanvas("c42","c42",200,10,600,400);
   double x[] = {0, 1, 2, 3, 4};
   double y[] = {0, 2, 4, 1, 3};
   double ex[] = {0.1, 0.2, 0.3, 0.4, 0.5};
   double ey[] = {1, 0.5, 1, 0.5, 1};
   TGraphErrors* ge = new TGraphErrors(5, x, y, ex, ey);
   ge->SetFillColor(6);
   ge->SetFillStyle(3005);
   ge->Draw("a4");
   return c42;
}
End_Macro
Begin_Html

<p>The following example shows how the option "[]" can be used to superimpose
systematic errors on top of a graph with statistical errors.

End_Html
Begin_Macro(source)
{
   TCanvas *c43 = new TCanvas("c43","c43",200,10,600,400);
   c43->DrawFrame(0., -0.5, 6., 2);

   double x[5]    = {1, 2, 3, 4, 5};
   double zero[5] = {0, 0, 0, 0, 0};

   // data set (1) with stat and sys errors
   double py1[5]      = {1.2, 1.15, 1.19, 0.9, 1.4};
   double ey_stat1[5] = {0.2, 0.18, 0.17, 0.2, 0.4};
   double ey_sys1[5]  = {0.5, 0.71, 0.76, 0.5, 0.45};

   // data set (2) with stat and sys errors
   double y2[5]       = {0.25, 0.18, 0.29, 0.2, 0.21};
   double ey_stat2[5] = {0.2, 0.18, 0.17, 0.2, 0.4};
   double ey_sys2[5]  = {0.63, 0.19, 0.7, 0.2, 0.7};

   // Now draw data set (1)

   // We first have to draw it only with the stat errors
   TGraphErrors *graph1 = new TGraphErrors(5, x, py1, zero, ey_stat1);
   graph1->SetMarkerStyle(20);
   graph1->Draw("P");

   // Now we have to somehow depict the sys errors

   TGraphErrors *graph1_sys = new TGraphErrors(5, x, py1, zero, ey_sys1);
   graph1_sys->Draw("[]");

   // Now draw data set (2)

   // We first have to draw it only with the stat errors
   TGraphErrors *graph2 = new TGraphErrors(5, x, y2, zero, ey_stat2);
   graph2->SetMarkerStyle(24);
   graph2->Draw("P");

   // Now we have to somehow depict the sys errors

   TGraphErrors *graph2_sys = new TGraphErrors(5, x, y2, zero, ey_sys2);
   graph2_sys->Draw("[]");
   return c43;
}
End_Macro
Begin_Html

<a name="GP03b"></a><h4><u>TGraphAsymmErrors</u></h4>
A <tt>TGraphAsymmErrors</tt> is like a <tt>TGraphErrors</tt> but the errors
defined along X and Y are not symmetric: The left and right errors are
different along X and the bottom and up errors are different along Y.

End_Html
Begin_Macro(source)
{
   TCanvas *c44 = new TCanvas("c44","c44",200,10,600,400);
   double ax[] = {0, 1, 2, 3, 4};
   double ay[] = {0, 2, 4, 1, 3};
   double aexl[] = {0.1, 0.2, 0.3, 0.4, 0.5};
   double aexh[] = {0.5, 0.4, 0.3, 0.2, 0.1};
   double aeyl[] = {1, 0.5, 1, 0.5, 1};
   double aeyh[] = {0.5, 1, 0.5, 1, 0.5};
   TGraphAsymmErrors* gae = new TGraphAsymmErrors(5, ax, ay, aexl, aexh, aeyl, aeyh);
   gae->SetFillColor(2);
   gae->SetFillStyle(3001);
   gae->Draw("a2");
   gae->Draw("p");
   return c44;
}
End_Macro
Begin_Html


<a name="GP03c"></a><h4><u>TGraphBentErrors</u></h4>
A <tt>TGraphBentErrors</tt> is like a <tt>TGraphAsymmErrors</tt>.
An extra parameter allows to bend the error bars to better see them
when several graphs are drawn on the same plot.

End_Html
Begin_Macro(source)
{
   TCanvas *c45 = new TCanvas("c45","c45",200,10,600,400);
   const Int_t n = 10;
   Double_t x[n]  = {-0.22, 0.05, 0.25, 0.35, 0.5, 0.61,0.7,0.85,0.89,0.95};
   Double_t y[n]  = {1,2.9,5.6,7.4,9,9.6,8.7,6.3,4.5,1};
   Double_t exl[n] = {.05,.1,.07,.07,.04,.05,.06,.07,.08,.05};
   Double_t eyl[n] = {.8,.7,.6,.5,.4,.4,.5,.6,.7,.8};
   Double_t exh[n] = {.02,.08,.05,.05,.03,.03,.04,.05,.06,.03};
   Double_t eyh[n] = {.6,.5,.4,.3,.2,.2,.3,.4,.5,.6};
   Double_t exld[n] = {.0,.0,.0,.0,.0,.0,.0,.0,.0,.0};
   Double_t eyld[n] = {.0,.0,.05,.0,.0,.0,.0,.0,.0,.0};
   Double_t exhd[n] = {.0,.0,.0,.0,.0,.0,.0,.0,.0,.0};
   Double_t eyhd[n] = {.0,.0,.0,.0,.0,.0,.0,.0,.05,.0};
   TGraphBentErrors *gr = new TGraphBentErrors(n,x,y,exl,exh,eyl,eyh,exld,exhd,eyld,eyhd);
   gr->SetTitle("TGraphBentErrors Example");
   gr->SetMarkerColor(4);
   gr->SetMarkerStyle(21);
   gr->Draw("ALP");
   return c45;
}
End_Macro
Begin_Html


<a name="GP04"></a><h3>TGraphPolar options</h3>

The drawing options for the polar graphs are the following:

<table border=0>

<tr><th valign=top>"O"</th><td>
Polar labels are drawn orthogonally to the polargram radius.
</td></tr>

<tr><th valign=top>"P"</th><td>
Polymarker are drawn at each point position.
</td></tr>

<tr><th valign=top>"E"</th><td>
Draw error bars.
</td></tr>

<tr><th valign=top>"F"</th><td>
Draw fill area (closed polygon).
</td></tr>

<tr><th valign=top>"A"</th><td>
Force axis redrawing even if a polargram already exists.
</td></tr>

<tr><th valign=top>"N"</th><td>
Disable the display of the polar labels.
</td></tr>

</table>

End_Html
Begin_Macro(source)
{
   TCanvas *c46 = new TCanvas("c46","c46",500,500);
   TGraphPolar * grP1 = new TGraphPolar();
   grP1->SetTitle("TGraphPolar example");

   grP1->SetPoint(0, (1*TMath::Pi())/4., 0.05);
   grP1->SetPoint(1, (2*TMath::Pi())/4., 0.10);
   grP1->SetPoint(2, (3*TMath::Pi())/4., 0.15);
   grP1->SetPoint(3, (4*TMath::Pi())/4., 0.20);
   grP1->SetPoint(4, (5*TMath::Pi())/4., 0.25);
   grP1->SetPoint(5, (6*TMath::Pi())/4., 0.30);
   grP1->SetPoint(6, (7*TMath::Pi())/4., 0.35);
   grP1->SetPoint(7, (8*TMath::Pi())/4., 0.40);

   grP1->SetMarkerStyle(20);
   grP1->SetMarkerSize(1.);
   grP1->SetMarkerColor(4);
   grP1->SetLineColor(4);
   grP1->Draw("ALP");

   // Update, otherwise GetPolargram returns 0
   c46->Update();
   grP1->GetPolargram()->SetToRadian();

   return c46;
}
End_Macro
Begin_Html

End_Html */


//______________________________________________________________________________
TGraphPainter::TGraphPainter()
{
   /* Begin_Html
   Default constructor
   End_Html */
}


//______________________________________________________________________________
TGraphPainter::~TGraphPainter()
{
   /* Begin_Html
   Destructor.
   End_Html */
}


//______________________________________________________________________________
void TGraphPainter::ComputeLogs(Int_t npoints, Int_t opt)
{
   /* Begin_Html
   Compute the logarithm of global variables <tt>gxwork</tt> and <tt>gywork</tt>
   according to the value of Options and put the results in the global
   variables <tt>gxworkl</tt> and <tt>gyworkl</tt>.
   <p>
   npoints : Number of points in gxwork and in gywork.
   <ul>
   <li> opt = 1 ComputeLogs is called from PaintGrapHist
   <li> opt = 0 ComputeLogs is called from PaintGraph
   </ul>
   End_Html */

   Int_t i;
   memcpy(gxworkl,gxwork,npoints*8);
   memcpy(gyworkl,gywork,npoints*8);
   if (gPad->GetLogx()) {
      for (i=0;i<npoints;i++) {
         if (gxworkl[i] > 0) gxworkl[i] = TMath::Log10(gxworkl[i]);
         else                gxworkl[i] = gPad->GetX1();
      }
   }
   if (!opt && gPad->GetLogy()) {
      for (i=0;i<npoints;i++) {
         if (gyworkl[i] > 0) gyworkl[i] = TMath::Log10(gyworkl[i]);
         else                gyworkl[i] = gPad->GetY1();
      }
   }
}


//______________________________________________________________________________
Int_t TGraphPainter::DistancetoPrimitiveHelper(TGraph *theGraph, Int_t px, Int_t py)
{
   /* Begin_Html
   Compute distance from point px,py to a graph.
   <p>
   Compute the closest distance of approach from point px,py to this line.
   The distance is computed in pixels units.
   End_Html */

   // Are we on the axis?
   Int_t distance;
   if (theGraph->GetHistogram()) {
      distance = theGraph->GetHistogram()->DistancetoPrimitive(px,py);
      if (distance <= 5) return distance;
   }

   // Somewhere on the graph points?
   const Int_t big = 9999;
   const Int_t kMaxDiff = 10;
   Int_t puxmin = gPad->XtoAbsPixel(gPad->GetUxmin());
   Int_t puymin = gPad->YtoAbsPixel(gPad->GetUymin());
   Int_t puxmax = gPad->XtoAbsPixel(gPad->GetUxmax());
   Int_t puymax = gPad->YtoAbsPixel(gPad->GetUymax());

   // return if point is not in the graph area
   if (px <= puxmin) return big;
   if (py >= puymin) return big;
   if (px >= puxmax) return big;
   if (py <= puymax) return big;

   // check if point is near one of the graph points
   Int_t i, pxp, pyp, d;
   distance = big;

   Int_t theNpoints = theGraph->GetN();
   Double_t *theX, *theY;
   if (theGraph->InheritsFrom(TGraphPolar::Class())) {
      TGraphPolar *theGraphPolar = (TGraphPolar*) theGraph;
      theX   = theGraphPolar->GetXpol();
      theY   = theGraphPolar->GetYpol();
   } else {
      theX   = theGraph->GetX();
      theY   = theGraph->GetY();
   }

   for (i=0;i<theNpoints;i++) {
      pxp = gPad->XtoAbsPixel(gPad->XtoPad(theX[i]));
      pyp = gPad->YtoAbsPixel(gPad->YtoPad(theY[i]));
      d   = TMath::Abs(pxp-px) + TMath::Abs(pyp-py);
      if (d < distance) distance = d;
   }
   if (distance < kMaxDiff) return distance;

   for (i=0;i<theNpoints-1;i++) {
      TAttLine l;
      d = l.DistancetoLine(px, py, gPad->XtoPad(theX[i]), gPad->YtoPad(theY[i]), gPad->XtoPad(theX[i+1]), gPad->YtoPad(theY[i+1]));
      if (d < distance) distance = d;
   }

   // If graph has been drawn with the fill area option, check if we are inside
   TString drawOption = theGraph->GetDrawOption();
   drawOption.ToLower();
   if (drawOption.Contains("f")) {
      Double_t xp = gPad->AbsPixeltoX(px); xp = gPad->PadtoX(xp);
      Double_t yp = gPad->AbsPixeltoY(py); yp = gPad->PadtoY(yp);
      if (TMath::IsInside(xp,yp,theNpoints,theX,theY) != 0) distance = 1;
   }

   // Loop on the list of associated functions and user objects
   TObject *f;
   TList *functions = theGraph->GetListOfFunctions();
   TIter   next(functions);
   while ((f = (TObject*) next())) {
      Int_t dist;
      if (f->InheritsFrom(TF1::Class())) dist = f->DistancetoPrimitive(-px,py);
      else                               dist = f->DistancetoPrimitive(px,py);
      if (dist < kMaxDiff) {
         gPad->SetSelected(f);
         return 0; //must be o and not dist in case of TMultiGraph
      }
   }

   return distance;
}


//______________________________________________________________________________
void TGraphPainter::DrawPanelHelper(TGraph *theGraph)
{
   /* Begin_html
   Display a panel with all histogram drawing options.
   End_html */

   if (!gPad) {
      Error("DrawPanel", "need to draw graph first");
      return;
   }
   TVirtualPadEditor *editor = TVirtualPadEditor::GetPadEditor();
   editor->Show();
   gROOT->ProcessLine(Form("((TCanvas*)0x%lx)->Selected((TVirtualPad*)0x%lx,(TObject*)0x%lx,1)",
                           (ULong_t)gPad->GetCanvas(), (ULong_t)gPad, (ULong_t)theGraph));
}


//______________________________________________________________________________
void TGraphPainter::ExecuteEventHelper(TGraph *theGraph, Int_t event, Int_t px, Int_t py)
{
   /* Begin_Html
   Execute action corresponding to one event.
   <p>
   This member function is called when a graph is clicked with the locator.
   <p>
   If the left mouse button is clicked on one of the line end points, this point
   follows the cursor until button is released.
   <p>
   If the middle mouse button clicked, the line is moved parallel to itself
   until the button is released.
   End_Html */

   Int_t i, d;
   Double_t xmin, xmax, ymin, ymax, dx, dy, dxr, dyr;
   const Int_t kMaxDiff =  10;//3;
   static Bool_t middle, badcase;
   static Int_t ipoint, pxp, pyp;
   static Int_t px1,px2,py1,py2;
   static Int_t pxold, pyold, px1old, py1old, px2old, py2old;
   static Int_t dpx, dpy;
   static Int_t *x=0, *y=0;
   Bool_t opaque  = gPad->OpaqueMoving();

   if (!theGraph->IsEditable() || theGraph->InheritsFrom(TGraphPolar::Class())) {
      gPad->SetCursor(kHand);
      return;
   }
   if (!gPad->IsEditable()) return;
   Int_t theNpoints = theGraph->GetN();
   Double_t *theX  = theGraph->GetX();
   Double_t *theY  = theGraph->GetY();

   switch (event) {

   case kButton1Down:
      badcase = kFALSE;
      gVirtualX->SetLineColor(-1);
      theGraph->TAttLine::Modify();  //Change line attributes only if necessary
      px1 = gPad->XtoAbsPixel(gPad->GetX1());
      py1 = gPad->YtoAbsPixel(gPad->GetY1());
      px2 = gPad->XtoAbsPixel(gPad->GetX2());
      py2 = gPad->YtoAbsPixel(gPad->GetY2());
      ipoint = -1;


      if (x || y) break;
      x = new Int_t[theNpoints+1];
      y = new Int_t[theNpoints+1];
      for (i=0;i<theNpoints;i++) {
         pxp = gPad->XtoAbsPixel(gPad->XtoPad(theX[i]));
         pyp = gPad->YtoAbsPixel(gPad->YtoPad(theY[i]));
         if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
             pyp < -kMaxPixel || pyp >= kMaxPixel) {
            badcase = kTRUE;
            continue;
         }
         if (!opaque) {
            gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
            gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
            gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
            gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
         }
         x[i] = pxp;
         y[i] = pyp;
         d   = TMath::Abs(pxp-px) + TMath::Abs(pyp-py);
         if (d < kMaxDiff) ipoint =i;
      }
      dpx = 0;
      dpy = 0;
      pxold = px;
      pyold = py;
      if (ipoint < 0) return;
      if (ipoint == 0) {
         px1old = 0;
         py1old = 0;
         px2old = gPad->XtoAbsPixel(theX[1]);
         py2old = gPad->YtoAbsPixel(theY[1]);
      } else if (ipoint == theNpoints-1) {
         px1old = gPad->XtoAbsPixel(gPad->XtoPad(theX[theNpoints-2]));
         py1old = gPad->YtoAbsPixel(gPad->YtoPad(theY[theNpoints-2]));
         px2old = 0;
         py2old = 0;
      } else {
         px1old = gPad->XtoAbsPixel(gPad->XtoPad(theX[ipoint-1]));
         py1old = gPad->YtoAbsPixel(gPad->YtoPad(theY[ipoint-1]));
         px2old = gPad->XtoAbsPixel(gPad->XtoPad(theX[ipoint+1]));
         py2old = gPad->YtoAbsPixel(gPad->YtoPad(theY[ipoint+1]));
      }
      pxold = gPad->XtoAbsPixel(gPad->XtoPad(theX[ipoint]));
      pyold = gPad->YtoAbsPixel(gPad->YtoPad(theY[ipoint]));

      break;


   case kMouseMotion:

      middle = kTRUE;
      for (i=0;i<theNpoints;i++) {
         pxp = gPad->XtoAbsPixel(gPad->XtoPad(theX[i]));
         pyp = gPad->YtoAbsPixel(gPad->YtoPad(theY[i]));
         d   = TMath::Abs(pxp-px) + TMath::Abs(pyp-py);
         if (d < kMaxDiff) middle = kFALSE;
      }


   // check if point is close to an axis
      if (middle) gPad->SetCursor(kMove);
      else gPad->SetCursor(kHand);
      break;

   case kButton1Motion:
      if (!opaque) {
         if (middle) {
            for(i=0;i<theNpoints-1;i++) {
               gVirtualX->DrawLine(x[i]+dpx, y[i]+dpy, x[i+1]+dpx, y[i+1]+dpy);
               pxp = x[i]+dpx;
               pyp = y[i]+dpy;
               if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
                   pyp < -kMaxPixel || pyp >= kMaxPixel) continue;
               gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
               gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
               gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
               gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
            }
            pxp = x[theNpoints-1]+dpx;
            pyp = y[theNpoints-1]+dpy;
            gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
            gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
            gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
            gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
            dpx += px - pxold;
            dpy += py - pyold;
            pxold = px;
            pyold = py;
            for(i=0;i<theNpoints-1;i++) {
               gVirtualX->DrawLine(x[i]+dpx, y[i]+dpy, x[i+1]+dpx, y[i+1]+dpy);
               pxp = x[i]+dpx;
               pyp = y[i]+dpy;
               if (pxp < -kMaxPixel || pxp >= kMaxPixel ||
                   pyp < -kMaxPixel || pyp >= kMaxPixel) continue;
               gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
               gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
               gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
               gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
            }
            pxp = x[theNpoints-1]+dpx;
            pyp = y[theNpoints-1]+dpy;
            gVirtualX->DrawLine(pxp-4, pyp-4, pxp+4,  pyp-4);
            gVirtualX->DrawLine(pxp+4, pyp-4, pxp+4,  pyp+4);
            gVirtualX->DrawLine(pxp+4, pyp+4, pxp-4,  pyp+4);
            gVirtualX->DrawLine(pxp-4, pyp+4, pxp-4,  pyp-4);
         } else {
            if (px1old) gVirtualX->DrawLine(px1old, py1old, pxold,  pyold);
            if (px2old) gVirtualX->DrawLine(pxold,  pyold,  px2old, py2old);
            gVirtualX->DrawLine(pxold-4, pyold-4, pxold+4,  pyold-4);
            gVirtualX->DrawLine(pxold+4, pyold-4, pxold+4,  pyold+4);
            gVirtualX->DrawLine(pxold+4, pyold+4, pxold-4,  pyold+4);
            gVirtualX->DrawLine(pxold-4, pyold+4, pxold-4,  pyold-4);
            pxold = px;
            pxold = TMath::Max(pxold, px1);
            pxold = TMath::Min(pxold, px2);
            pyold = py;
            pyold = TMath::Max(pyold, py2);
            pyold = TMath::Min(pyold, py1);
            if (px1old) gVirtualX->DrawLine(px1old, py1old, pxold,  pyold);
            if (px2old) gVirtualX->DrawLine(pxold,  pyold,  px2old, py2old);
            gVirtualX->DrawLine(pxold-4, pyold-4, pxold+4,  pyold-4);
            gVirtualX->DrawLine(pxold+4, pyold-4, pxold+4,  pyold+4);
            gVirtualX->DrawLine(pxold+4, pyold+4, pxold-4,  pyold+4);
            gVirtualX->DrawLine(pxold-4, pyold+4, pxold-4,  pyold-4);
         }
      } else {
         xmin = gPad->GetUxmin();
         xmax = gPad->GetUxmax();
         ymin = gPad->GetUymin();
         ymax = gPad->GetUymax();
         dx   = xmax-xmin;
         dy   = ymax-ymin;
         dxr  = dx/(1 - gPad->GetLeftMargin() - gPad->GetRightMargin());
         dyr  = dy/(1 - gPad->GetBottomMargin() - gPad->GetTopMargin());

         if (theGraph->GetHistogram()) {
            // Range() could change the size of the pad pixmap and therefore should
            // be called before the other paint routines
            gPad->Range(xmin - dxr*gPad->GetLeftMargin(),
                         ymin - dyr*gPad->GetBottomMargin(),
                         xmax + dxr*gPad->GetRightMargin(),
                         ymax + dyr*gPad->GetTopMargin());
            gPad->RangeAxis(xmin, ymin, xmax, ymax);
         }
         if (middle) {
            dpx += px - pxold;
            dpy += py - pyold;
            pxold = px;
            pyold = py;
            for(i=0;i<theNpoints;i++) {
               if (badcase) continue;  //do not update if big zoom and points moved
               if (x) theX[i] = gPad->PadtoX(gPad->AbsPixeltoX(x[i]+dpx));
               if (y) theY[i] = gPad->PadtoY(gPad->AbsPixeltoY(y[i]+dpy));
            }
         } else {
            pxold = px;
            pxold = TMath::Max(pxold, px1);
            pxold = TMath::Min(pxold, px2);
            pyold = py;
            pyold = TMath::Max(pyold, py2);
            pyold = TMath::Min(pyold, py1);
            theX[ipoint] = gPad->PadtoX(gPad->AbsPixeltoX(pxold));
            theY[ipoint] = gPad->PadtoY(gPad->AbsPixeltoY(pyold));
            if (theGraph->InheritsFrom("TCutG")) {
               //make sure first and last point are the same
               if (ipoint == 0) {
                  theX[theNpoints-1] = theX[0];
                  theY[theNpoints-1] = theY[0];
               }
               if (ipoint == theNpoints-1) {
                  theX[0] = theX[theNpoints-1];
                  theY[0] = theY[theNpoints-1];
               }
            }
         }
         badcase = kFALSE;
         gPad->Modified(kTRUE);
         //gPad->Update();
      }
      break;

   case kButton1Up:

      if (gROOT->IsEscaped()) {
         gROOT->SetEscape(kFALSE);
         delete [] x; x = 0;
         delete [] y; y = 0;
         break;
      }

   // Compute x,y range
      xmin = gPad->GetUxmin();
      xmax = gPad->GetUxmax();
      ymin = gPad->GetUymin();
      ymax = gPad->GetUymax();
      dx   = xmax-xmin;
      dy   = ymax-ymin;
      dxr  = dx/(1 - gPad->GetLeftMargin() - gPad->GetRightMargin());
      dyr  = dy/(1 - gPad->GetBottomMargin() - gPad->GetTopMargin());

      if (theGraph->GetHistogram()) {
         // Range() could change the size of the pad pixmap and therefore should
         // be called before the other paint routines
         gPad->Range(xmin - dxr*gPad->GetLeftMargin(),
                      ymin - dyr*gPad->GetBottomMargin(),
                      xmax + dxr*gPad->GetRightMargin(),
                      ymax + dyr*gPad->GetTopMargin());
         gPad->RangeAxis(xmin, ymin, xmax, ymax);
      }
      if (middle) {
         for(i=0;i<theNpoints;i++) {
            if (badcase) continue;  //do not update if big zoom and points moved
            if (x) theX[i] = gPad->PadtoX(gPad->AbsPixeltoX(x[i]+dpx));
            if (y) theY[i] = gPad->PadtoY(gPad->AbsPixeltoY(y[i]+dpy));
         }
      } else {
         theX[ipoint] = gPad->PadtoX(gPad->AbsPixeltoX(pxold));
         theY[ipoint] = gPad->PadtoY(gPad->AbsPixeltoY(pyold));
         if (theGraph->InheritsFrom("TCutG")) {
            //make sure first and last point are the same
            if (ipoint == 0) {
               theX[theNpoints-1] = theX[0];
               theY[theNpoints-1] = theY[0];
            }
            if (ipoint == theNpoints-1) {
               theX[0] = theX[theNpoints-1];
               theY[0] = theY[theNpoints-1];
            }
         }
      }
      badcase = kFALSE;
      delete [] x; x = 0;
      delete [] y; y = 0;
      gPad->Modified(kTRUE);
      gVirtualX->SetLineColor(-1);
   }
}


//______________________________________________________________________________
char *TGraphPainter::GetObjectInfoHelper(TGraph * /*theGraph*/, Int_t /*px*/, Int_t /*py*/) const
{
   return (char*)"";
}


//______________________________________________________________________________
void TGraphPainter::PaintHelper(TGraph *theGraph, Option_t *option)
{
   /* Begin_Html
   Paint a any kind of TGraph
   End_Html */

   if (theGraph) {
      SetBit(TGraph::kClipFrame, theGraph->TestBit(TGraph::kClipFrame));
      if (theGraph->InheritsFrom(TGraphBentErrors::Class())) {
         PaintGraphBentErrors(theGraph,option);
      } else if (theGraph->InheritsFrom(TGraphQQ::Class())) {
         PaintGraphQQ(theGraph,option);
      } else if (theGraph->InheritsFrom(TGraphAsymmErrors::Class())) {
         PaintGraphAsymmErrors(theGraph,option);
      } else if (theGraph->InheritsFrom(TGraphErrors::Class())) {
         if (theGraph->InheritsFrom(TGraphPolar::Class())) {
            PaintGraphPolar(theGraph,option);
         } else {
            PaintGraphErrors(theGraph,option);
         }
      } else {
         PaintGraphSimple(theGraph,option);
      }
   }
}


//______________________________________________________________________________
void TGraphPainter::PaintGraph(TGraph *theGraph, Int_t npoints, const Double_t *x, const Double_t *y, Option_t *chopt)
{
   /* Begin_Html
   <a href="#GP01">Control function to draw a graph.</a>
   End_Html */
   if (theGraph->InheritsFrom("TGraphPolar"))
      gPad->PushSelectableObject(theGraph);

   Int_t optionLine , optionAxis , optionCurve, optionStar , optionMark;
   Int_t optionBar  , optionR    , optionOne  , optionE;
   Int_t optionFill , optionZ    , optionCurveFill;
   Int_t i, npt, nloop;
   Int_t drawtype=0;
   Double_t xlow, xhigh, ylow, yhigh;
   Double_t barxmin, barxmax, barymin, barymax;
   Double_t uxmin, uxmax;
   Double_t x1, xn, y1, yn;
   Double_t dbar, bdelta;
   Int_t theNpoints = theGraph->GetN();

   if (npoints <= 0) {
      Error("PaintGraph", "illegal number of points (%d)", npoints);
      return;
   }
   TString opt = chopt;
   opt.ToUpper();
   opt.ReplaceAll("SAME","");

   if (opt.Contains("L")) optionLine = 1;  else optionLine = 0;
   if (opt.Contains("A")) optionAxis = 1;  else optionAxis = 0;
   if (opt.Contains("C")) optionCurve= 1;  else optionCurve= 0;
   if (opt.Contains("*")) optionStar = 1;  else optionStar = 0;
   if (opt.Contains("P")) optionMark = 1;  else optionMark = 0;
   if (opt.Contains("B")) optionBar  = 1;  else optionBar  = 0;
   if (opt.Contains("R")) optionR    = 1;  else optionR    = 0;
   if (opt.Contains("1")) optionOne  = 1;  else optionOne  = 0;
   if (opt.Contains("F")) optionFill = 1;  else optionFill = 0;
   if (opt.Contains("2") || opt.Contains("3") ||
      opt.Contains("4") || opt.Contains("5")) optionE = 1;  else optionE = 0;
   optionZ    = 0;

   // If no "drawing" option is selected and if chopt<>' ' nothing is done.
   if (optionLine+optionFill+optionCurve+optionStar+optionMark+optionBar+optionE == 0) {
      if (!chopt[0])  optionLine=1;
      else   return;
   }

   if (optionStar) theGraph->SetMarkerStyle(3);

   optionCurveFill = 0;
   if (optionCurve && optionFill) {
      optionCurveFill = 1;
      optionFill      = 0;
   }

   // Draw the Axis.
   Double_t rwxmin,rwxmax, rwymin, rwymax, maximum, minimum, dx, dy;
   if (optionAxis) {
      if (theGraph->GetHistogram()) {
         rwxmin    = gPad->GetUxmin();
         rwxmax    = gPad->GetUxmax();
         rwymin    = gPad->GetUymin();
         rwymax    = gPad->GetUymax();
         minimum   = theGraph->GetHistogram()->GetMinimumStored();
         maximum   = theGraph->GetHistogram()->GetMaximumStored();
         if (minimum == -1111) { //this can happen after unzooming
            minimum = theGraph->GetHistogram()->GetYaxis()->GetXmin();
            theGraph->GetHistogram()->SetMinimum(minimum);
         }
         if (maximum == -1111) {
            maximum = theGraph->GetHistogram()->GetYaxis()->GetXmax();
            theGraph->GetHistogram()->SetMaximum(maximum);
         }
         uxmin     = gPad->PadtoX(rwxmin);
         uxmax     = gPad->PadtoX(rwxmax);
      } else {

         theGraph->ComputeRange(rwxmin, rwymin, rwxmax, rwymax);  //this is redefined in TGraphErrors

         if (rwxmin == rwxmax) rwxmax += 1.;
         if (rwymin == rwymax) rwymax += 1.;
         dx = 0.1*(rwxmax-rwxmin);
         dy = 0.1*(rwymax-rwymin);
         uxmin    = rwxmin - dx;
         uxmax    = rwxmax + dx;
         minimum  = rwymin - dy;
         maximum  = rwymax + dy;
      }
      if (theGraph->GetMinimum() != -1111) rwymin = minimum = theGraph->GetMinimum();
      if (theGraph->GetMaximum() != -1111) rwymax = maximum = theGraph->GetMaximum();
      if (uxmin < 0 && rwxmin >= 0) uxmin = 0.9*rwxmin;
      if (uxmax > 0 && rwxmax <= 0) {
         if (gPad->GetLogx()) uxmax = 1.1*rwxmax;
         else                 uxmax = 0;
      }
      if (minimum < 0 && rwymin >= 0) minimum = 0.9*rwymin;
      if (maximum > 0 && rwymax <= 0) {
         //if(gPad->GetLogy()) maximum = 1.1*rwymax;
         //else                maximum = 0;
      }
      if (minimum <= 0 && gPad->GetLogy()) minimum = 0.001*maximum;
      if (uxmin <= 0 && gPad->GetLogx()) {
         if (uxmax > 1000) uxmin = 1;
         else              uxmin = 0.001*uxmax;
      }
      rwymin = minimum;
      rwymax = maximum;

      // Create a temporary histogram and fill each bin with the
      // function value.
      char chopth[8] = " ";
      if (strstr(chopt,"x+")) strncat(chopth, "x+",2);
      if (strstr(chopt,"y+")) strncat(chopth, "y+",2);
      if (!theGraph->GetHistogram()) {
         // the graph is created with at least as many bins as there are
         // points to permit zooming on the full range.
         rwxmin = uxmin;
         rwxmax = uxmax;
         npt = 100;
         if (theNpoints > npt) npt = theNpoints;
         TH1F *h = new TH1F(Form("%s_h",GetName()),GetTitle(),npt,rwxmin,rwxmax);
         theGraph->SetHistogram(h);
         if (!theGraph->GetHistogram()) return;
         theGraph->GetHistogram()->SetMinimum(rwymin);
         theGraph->GetHistogram()->SetMaximum(rwymax);
         theGraph->GetHistogram()->GetYaxis()->SetLimits(rwymin,rwymax);
         theGraph->GetHistogram()->SetBit(TH1::kNoStats);
         theGraph->GetHistogram()->SetDirectory(0);
         theGraph->GetHistogram()->Paint(chopth); // Draw histogram axis, title and grid
      } else {
         if (gPad->GetLogy()) {
            theGraph->GetHistogram()->SetMinimum(rwymin);
            theGraph->GetHistogram()->SetMaximum(rwymax);
            theGraph->GetHistogram()->GetYaxis()->SetLimits(rwymin,rwymax);
         }
         theGraph->GetHistogram()->Paint(chopth); // Draw histogram axis, title and grid
      }
   }

   // Set Clipping option
   gPad->SetBit(TGraph::kClipFrame, theGraph->TestBit(TGraph::kClipFrame));

   TF1 *fit = 0;
   TList *functions = theGraph->GetListOfFunctions();
   TObject *f;
   if (functions) {
      f = (TF1*)functions->First();
      if (f) {
         if (f->InheritsFrom(TF1::Class())) fit = (TF1*)f;
      }
      TIter   next(functions);
      while ((f = (TObject*) next())) {
         if (f->InheritsFrom(TF1::Class())) {
            fit = (TF1*)f;
            break;
         }
      }
   }
   if (fit) PaintStats(theGraph, fit);

   rwxmin   = gPad->GetUxmin();
   rwxmax   = gPad->GetUxmax();
   rwymin   = gPad->GetUymin();
   rwymax   = gPad->GetUymax();
   uxmin    = gPad->PadtoX(rwxmin);
   uxmax    = gPad->PadtoX(rwxmax);
   if (theGraph->GetHistogram() && !theGraph->InheritsFrom("TGraphPolar")) {
      maximum = theGraph->GetHistogram()->GetMaximum();
      minimum = theGraph->GetHistogram()->GetMinimum();
   } else {
      maximum = gPad->PadtoY(rwymax);
      minimum = gPad->PadtoY(rwymin);
   }

   // Set attributes
   theGraph->TAttLine::Modify();
   theGraph->TAttFill::Modify();
   theGraph->TAttMarker::Modify();

   // Draw the graph with a polyline or a fill area
   gxwork  = new Double_t[2*npoints+10];
   gywork  = new Double_t[2*npoints+10];
   gxworkl = new Double_t[2*npoints+10];
   gyworkl = new Double_t[2*npoints+10];

   if (optionLine || optionFill) {
      x1    = x[0];
      xn    = x[npoints-1];
      y1    = y[0];
      yn    = y[npoints-1];
      nloop = npoints;
      if (optionFill && (xn != x1 || yn != y1)) nloop++;
      npt = 0;
      for (i=1;i<=nloop;i++) {
         if (i > npoints) {
            gxwork[npt] = gxwork[0];  gywork[npt] = gywork[0];
         } else {
            gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
            npt++;
         }
         if (i == nloop) {
            ComputeLogs(npt, optionZ);
            Int_t bord = gStyle->GetDrawBorder();
            if (optionR) {
               if (optionFill) {
                  gPad->PaintFillArea(npt,gyworkl,gxworkl);
                  if (bord) gPad->PaintPolyLine(npt,gyworkl,gxworkl);
               } else {
                  if (TMath::Abs(theGraph->GetLineWidth())>99) PaintPolyLineHatches(theGraph, npt, gyworkl, gxworkl);
                  gPad->PaintPolyLine(npt,gyworkl,gxworkl);
               }
            } else {
               if (optionFill) {
                  gPad->PaintFillArea(npt,gxworkl,gyworkl);
                  if (bord) gPad->PaintPolyLine(npt,gxworkl,gyworkl);
               } else {
                  if (TMath::Abs(theGraph->GetLineWidth())>99) PaintPolyLineHatches(theGraph, npt, gxworkl, gyworkl);
                  gPad->PaintPolyLine(npt,gxworkl,gyworkl);
               }
            }
            gxwork[0] = gxwork[npt-1];  gywork[0] = gywork[npt-1];
            npt      = 1;
         }
      }
   }

   // Draw the graph with a smooth Curve. Smoothing via Smooth
   if (optionCurve) {
      x1 = x[0];
      xn = x[npoints-1];
      y1 = y[0];
      yn = y[npoints-1];
      drawtype = 1;
      nloop = npoints;
      if (optionCurveFill) {
         drawtype += 1000;
         if (xn != x1 || yn != y1) nloop++;
      }
      if (!optionR) {
         npt = 0;
         for (i=1;i<=nloop;i++) {
            if (i > npoints) {
               gxwork[npt] = gxwork[0];  gywork[npt] = gywork[0];
            } else {
               gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
               npt++;
            }
            ComputeLogs(npt, optionZ);
            if (gyworkl[npt-1] < rwymin || gyworkl[npt-1] > rwymax) {
               if (npt > 2) {
                  ComputeLogs(npt, optionZ);
                  Smooth(theGraph, npt,gxworkl,gyworkl,drawtype);
               }
               gxwork[0] = gxwork[npt-1]; gywork[0] = gywork[npt-1];
               npt=1;
               continue;
            }
         }
         if (npt > 1) {
            ComputeLogs(npt, optionZ);
            Smooth(theGraph, npt,gxworkl,gyworkl,drawtype);
         }
      } else {
         drawtype += 10;
         npt    = 0;
         for (i=1;i<=nloop;i++) {
            if (i > npoints) {
               gxwork[npt] = gxwork[0];  gywork[npt] = gywork[0];
            } else {
               if (y[i-1] < minimum || y[i-1] > maximum) continue;
               if (x[i-1] < uxmin    || x[i-1] > uxmax)  continue;
               gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
               npt++;
            }
            ComputeLogs(npt, optionZ);
            if (gxworkl[npt-1] < rwxmin || gxworkl[npt-1] > rwxmax) {
               if (npt > 2) {
                  ComputeLogs(npt, optionZ);
                  Smooth(theGraph, npt,gxworkl,gyworkl,drawtype);
               }
               gxwork[0] = gxwork[npt-1]; gywork[0] = gywork[npt-1];
               npt=1;
               continue;
            }
         }
         if (npt > 1) {
            ComputeLogs(npt, optionZ);
            Smooth(theGraph, npt,gxworkl,gyworkl,drawtype);
         }
      }
   }

   // Draw the graph with a '*' on every points
   if (optionStar) {
      theGraph->SetMarkerStyle(3);
      npt = 0;
      for (i=1;i<=npoints;i++) {
         gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
         npt++;
         if (i == npoints) {
            ComputeLogs(npt, optionZ);
            if (optionR)  gPad->PaintPolyMarker(npt,gyworkl,gxworkl);
            else          gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
            npt = 0;
         }
      }
   }

   // Draw the graph with the current polymarker on every points
   if (optionMark) {
      npt = 0;
      for (i=1;i<=npoints;i++) {
         gxwork[npt] = x[i-1];      gywork[npt] = y[i-1];
         npt++;
         if (i == npoints) {
            ComputeLogs(npt, optionZ);
            if (optionR) gPad->PaintPolyMarker(npt,gyworkl,gxworkl);
            else         gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
            npt = 0;
         }
      }
   }

   // Draw the graph as a bar chart
   if (optionBar) {
      if (!optionR) {
         barxmin = x[0];
         barxmax = x[0];
         for (i=1;i<npoints;i++) {
            if (x[i] < barxmin) barxmin = x[i];
            if (x[i] > barxmax) barxmax = x[i];
         }
         bdelta = (barxmax-barxmin)/Double_t(npoints);
      } else {
         barymin = y[0];
         barymax = y[0];
         for (i=1;i<npoints;i++) {
            if (y[i] < barymin) barymin = y[i];
            if (y[i] > barymax) barymax = y[i];
         }
         bdelta = (barymax-barymin)/Double_t(npoints);
      }
      dbar  = 0.5*bdelta*gStyle->GetBarWidth();
      if (!optionR) {
         for (i=1;i<=npoints;i++) {
            xlow  = x[i-1] - dbar;
            xhigh = x[i-1] + dbar;
            yhigh = y[i-1];
            if (xlow  < uxmin) xlow = uxmin;
            if (xhigh > uxmax) xhigh = uxmax;
            if (!optionOne) ylow = TMath::Max((Double_t)0,gPad->GetUymin());
            else            ylow = gPad->GetUymin();
            gxwork[0] = xlow;
            gywork[0] = ylow;
            gxwork[1] = xhigh;
            gywork[1] = yhigh;
            ComputeLogs(2, optionZ);
            if (gyworkl[0] < gPad->GetUymin()) gyworkl[0] = gPad->GetUymin();
            if (gyworkl[1] < gPad->GetUymin()) continue;
            if (gyworkl[1] > gPad->GetUymax()) gyworkl[1] = gPad->GetUymax();
            if (gyworkl[0] > gPad->GetUymax()) continue;

            gPad->PaintBox(gxworkl[0],gyworkl[0],gxworkl[1],gyworkl[1]);
         }
      } else {
         for (i=1;i<=npoints;i++) {
            xhigh = x[i-1];
            ylow  = y[i-1] - dbar;
            yhigh = y[i-1] + dbar;
            xlow     = TMath::Max((Double_t)0, gPad->GetUxmin());
            gxwork[0] = xlow;
            gywork[0] = ylow;
            gxwork[1] = xhigh;
            gywork[1] = yhigh;
            ComputeLogs(2, optionZ);
            gPad->PaintBox(gxworkl[0],gyworkl[0],gxworkl[1],gyworkl[1]);
         }
      }
   }
   gPad->ResetBit(TGraph::kClipFrame);

   delete [] gxwork;
   delete [] gywork;
   delete [] gxworkl;
   delete [] gyworkl;
}


//______________________________________________________________________________
void TGraphPainter::PaintGrapHist(TGraph *theGraph, Int_t npoints, const Double_t *x,
                                  const Double_t *y, Option_t *chopt)
{
   /* Begin_Html
    This is a service method used by
    <a href="http://root.cern.ch/root/html/THistPainter.html"><tt>THistPainter</tt></a>
    to paint 1D histograms. <b>It is not used to paint TGraph</b>.
    <p>
    Input parameters:
    <ul>
    <li> npoints : Number of points in X or in Y.
    <li> x[npoints] or x[0] : x coordinates or (xmin,xmax).
    <li> y[npoints] or y[0] : y coordinates or (ymin,ymax).
    <li> chopt : Option.
    </ul>
    <p>
    The aspect of the histogram is done according to the value of the chopt.
    <p>
    <table border=0>
    <tr><th valign=top>"R"</th><td>
    Graph is drawn horizontaly, parallel to X axis. (default is vertically,
    parallel to Y axis)
    <br>
    If option R is selected the user must give:
    <ul>
    <li> 2 values for Y (y[0]=YMIN and y[1]=YMAX)
    <li> N values for X, one for each channel.
    </ul>
    Otherwise the user must give:
    <ul>
    <li> N values for Y, one for each channel.
    <li> 2 values for X (x[0]=XMIN and x[1]=XMAX)
    </ul>
    </td></tr>

    <tr><th valign=top>"L"</th><td>
    A simple polyline beetwen every points is drawn.
    </td></tr>

    <tr><th valign=top>"H"</th><td>
    An Histogram with equidistant bins is drawn as a polyline.
    </td></tr>

    <tr><th valign=top>"F"</th><td>
    An histogram with equidistant bins is drawn as a fill area. Contour is not
    drawn unless chopt='H' is also selected..
    </td></tr>

    <tr><th valign=top>"N"</th><td>
    Non equidistant bins (default is equidistant). If N is the number of channels
    array X and Y must be dimensionned as follow:
    <ul>
    <li>>If option R is not selected (default) then the user must give:</li>
      <ul>
      <li>(N+1) values for X (limits of channels).</li>
      <li>N values for Y, one for each channel.</li>
      <ul>
    <li>Otherwise the user must give:</li>
      <ul>
      <li>(N+1) values for Y (limits of channels).</li>
      <li>N values for X, one for each channel.</li>
      </ul>
    </ul>
    </td></tr>

    <tr><th valign=top>"F1"</th><td>
    Idem as 'F' except that fill area base line is the minimum of the pad instead
    of Y=0.
    </td></tr>

    <tr><th valign=top>"F2"</th><td>
    Draw a Fill area polyline connecting the center of bins
    </td></tr>

    <tr><th valign=top>"C"</th><td>
    A smooth Curve is drawn.
    </td></tr>

    <tr><th valign=top>"*"</th><td>
    A Star is plotted at the center of each bin.
    </td></tr>

    <tr><th valign=top>"P"</th><td>
    Idem with the current marker.
    </td></tr>

    <tr><th valign=top>"P0"</th><td>
    Idem with the current marker. Empty bins also drawn.
    </td></tr>

    <tr><th valign=top>"B"</th><td>
    A Bar chart with equidistant bins is drawn as fill areas (Contours are drawn).
    </td></tr>

    <tr><th valign=top>"]["</th><td>
    "Cutoff" style. When this option is selected together with H option, the
    first and last vertical lines of the histogram are not drawn.
    </td></tr>

    </table>
    End_Html */

   const char *where = "PaintGraphHist";

   Int_t optionLine , optionAxis , optionCurve, optionStar, optionMark;
   Int_t optionBar  , optionRot  , optionOne  , optionOff ;
   Int_t optionFill , optionZ;
   Int_t optionHist , optionBins , optionMarker;
   Int_t i, j, npt;
   Int_t drawtype=0, drawborder, drawbordersav;
   Double_t xlow, xhigh, ylow, yhigh;
   Double_t wmin, wmax;
   Double_t dbar, offset, wminstep;
   Double_t delta = 0;
   Double_t ylast = 0;
   Double_t xi, xi1, xj, xj1, yi1, yi, yj, yj1, xwmin, ywmin;
   Int_t first, last, nbins;
   Int_t fillarea;

   char choptaxis[10] = " ";

   if (npoints <= 0) {
      Error(where, "illegal number of points (%d)", npoints);
      return;
   }
   TString opt = chopt;
   opt.ToUpper();
   if (opt.Contains("H"))  optionHist = 1;  else optionHist = 0;
   if (opt.Contains("F"))  optionFill = 1;  else optionFill = 0;
   if (opt.Contains("C"))  optionCurve= 1;  else optionCurve= 0;
   if (opt.Contains("*"))  optionStar = 1;  else optionStar = 0;
   if (opt.Contains("R"))  optionRot  = 1;  else optionRot  = 0;
   if (opt.Contains("1"))  optionOne  = 1;  else optionOne  = 0;
   if (opt.Contains("B"))  optionBar  = 1;  else optionBar  = 0;
   if (opt.Contains("N"))  optionBins = 1;  else optionBins = 0;
   if (opt.Contains("L"))  optionLine = 1;  else optionLine = 0;
   if (opt.Contains("P"))  optionMark = 1;  else optionMark = 0;
   if (opt.Contains("A"))  optionAxis = 1;  else optionAxis = 0;
   if (opt.Contains("][")) optionOff  = 1;  else optionOff  = 0;
   if (opt.Contains("P0")) optionMark = 10;

   Int_t optionFill2 = 0;
   if (opt.Contains("F") && opt.Contains("2")) {
      optionFill = 0; optionFill2 = 1;
   }

   // Set Clipping option
   Option_t *noClip;
   if (theGraph->TestBit(TGraph::kClipFrame)) noClip = "";
   else noClip = "C";
   gPad->SetBit(TGraph::kClipFrame, theGraph->TestBit(TGraph::kClipFrame));

   optionZ = 1;

   if (optionStar) theGraph->SetMarkerStyle(3);

   first = 1;
   last  = npoints;
   nbins = last - first + 1;

   //           Draw the Axis with a fixed number of division: 510

   Double_t baroffset = gStyle->GetBarOffset();
   Double_t barwidth  = gStyle->GetBarWidth();
   Double_t rwxmin    = gPad->GetUxmin();
   Double_t rwxmax    = gPad->GetUxmax();
   Double_t rwymin    = gPad->GetUymin();
   Double_t rwymax    = gPad->GetUymax();
   Double_t uxmin     = gPad->PadtoX(rwxmin);
   Double_t uxmax     = gPad->PadtoX(rwxmax);
   Double_t rounding  = (uxmax-uxmin)*1.e-5;
   drawborder         = gStyle->GetDrawBorder();
   if (optionAxis) {
      Int_t nx1, nx2, ndivx, ndivy, ndiv;
      choptaxis[0]  = 0;
      Double_t rwmin  = rwxmin;
      Double_t rwmax  = rwxmax;
      ndivx = gStyle->GetNdivisions("X");
      ndivy = gStyle->GetNdivisions("Y");
      if (ndivx > 1000) {
         nx2   = ndivx/100;
         nx1   = TMath::Max(1, ndivx%100);
         ndivx = 100*nx2 + Int_t(Double_t(nx1)*gPad->GetAbsWNDC());
      }
      ndiv  =TMath::Abs(ndivx);
      // coverity [Calling risky function]
      if (ndivx < 0) strlcat(choptaxis, "N",10);
      if (gPad->GetGridx()) {
         // coverity [Calling risky function]
         strlcat(choptaxis, "W",10);
      }
      if (gPad->GetLogx()) {
         rwmin = TMath::Power(10,rwxmin);
         rwmax = TMath::Power(10,rwxmax);
         // coverity [Calling risky function]
         strlcat(choptaxis, "G",10);
      }
      TGaxis *axis = new TGaxis();
      axis->SetLineColor(gStyle->GetAxisColor("X"));
      axis->SetTextColor(gStyle->GetLabelColor("X"));
      axis->SetTextFont(gStyle->GetLabelFont("X"));
      axis->SetLabelSize(gStyle->GetLabelSize("X"));
      axis->SetLabelOffset(gStyle->GetLabelOffset("X"));
      axis->SetTickSize(gStyle->GetTickLength("X"));

      axis->PaintAxis(rwxmin,rwymin,rwxmax,rwymin,rwmin,rwmax,ndiv,choptaxis);

      choptaxis[0]  = 0;
      rwmin  = rwymin;
      rwmax  = rwymax;
      if (ndivy < 0) {
         nx2   = ndivy/100;
         nx1   = TMath::Max(1, ndivy%100);
         ndivy = 100*nx2 + Int_t(Double_t(nx1)*gPad->GetAbsHNDC());
         // coverity [Calling risky function]
         strlcat(choptaxis, "N",10);
      }
      ndiv  =TMath::Abs(ndivy);
      if (gPad->GetGridy()) {
         // coverity [Calling risky function]
         strlcat(choptaxis, "W",10);
      }
      if (gPad->GetLogy()) {
         rwmin = TMath::Power(10,rwymin);
         rwmax = TMath::Power(10,rwymax);
         // coverity [Calling risky function]
         strlcat(choptaxis,"G",10);
      }
      axis->SetLineColor(gStyle->GetAxisColor("Y"));
      axis->SetTextColor(gStyle->GetLabelColor("Y"));
      axis->SetTextFont(gStyle->GetLabelFont("Y"));
      axis->SetLabelSize(gStyle->GetLabelSize("Y"));
      axis->SetLabelOffset(gStyle->GetLabelOffset("Y"));
      axis->SetTickSize(gStyle->GetTickLength("Y"));

      axis->PaintAxis(rwxmin,rwymin,rwxmin,rwymax,rwmin,rwmax,ndiv,choptaxis);
      delete axis;
   }


   //           Set attributes
   theGraph->TAttLine::Modify();
   theGraph->TAttFill::Modify();
   theGraph->TAttMarker::Modify();

   //       Min-Max scope

   if (!optionRot) {wmin = x[0];   wmax = x[1];}
   else            {wmin = y[0];   wmax = y[1];}

   if (!optionBins) delta = (wmax - wmin)/ Double_t(nbins);

   Int_t fwidth = gPad->GetFrameLineWidth();
   TFrame *frame = gPad->GetFrame();
   if (frame) fwidth = frame->GetLineWidth();
   if (optionOff) fwidth = 1;
   Double_t dxframe = gPad->AbsPixeltoX(fwidth/2) - gPad->AbsPixeltoX(0);
   Double_t vxmin = gPad->PadtoX(gPad->GetUxmin() + dxframe);
   Double_t vxmax = gPad->PadtoX(gPad->GetUxmax() - dxframe);
   Double_t dyframe = -gPad->AbsPixeltoY(fwidth/2) + gPad->AbsPixeltoY(0);
   Double_t vymin = gPad->GetUymin() + dyframe; //y already in log scale
   vxmin = TMath::Max(vxmin,wmin);
   vxmax = TMath::Min(vxmax,wmax);

   //           Draw the histogram with a fill area

   gxwork  = new Double_t[2*npoints+10];
   gywork  = new Double_t[2*npoints+10];
   gxworkl = new Double_t[2*npoints+10];
   gyworkl = new Double_t[2*npoints+10];

   if (optionFill && !optionCurve) {
      fillarea = kTRUE;
      if (!optionRot) {
         gxwork[0] = vxmin;
         if (!optionOne) gywork[0] = TMath::Min(TMath::Max((Double_t)0,gPad->GetUymin())
                                               ,gPad->GetUymax());
         else            gywork[0] = gPad->GetUymin();
         npt = 2;
         for (j=first; j<=last;j++) {
            if (!optionBins) {
               gxwork[npt-1]   = gxwork[npt-2];
               gxwork[npt]     = wmin+((j-first+1)*delta);
               if (gxwork[npt] < gxwork[0]) gxwork[npt] = gxwork[0];

            } else {
               xj1 = x[j];      xj  = x[j-1];
               if (xj1 < xj) {
                  if (j != last) Error(where, "X must be in increasing order");
                  else           Error(where, "X must have N+1 values with option N");
                  return;
               }
               gxwork[npt-1] = x[j-1];       gxwork[npt] = x[j];
            }
            gywork[npt-1] = y[j-1];
            gywork[npt]   = y[j-1];
            if (gywork[npt] < vymin) {gywork[npt] = vymin; gywork[npt-1] = vymin;}
            if ((gxwork[npt-1] >= uxmin-rounding && gxwork[npt-1] <= uxmax+rounding) ||
                (gxwork[npt]   >= uxmin-rounding && gxwork[npt]   <= uxmax+rounding)) npt += 2;
            if (j == last) {
               gxwork[npt-1] = gxwork[npt-2];
               gywork[npt-1] = gywork[0];
               //make sure that the fill area does not overwrite the frame
               //take into account the frame linewidth
               if (gxwork[0    ] < vxmin) {gxwork[0    ] = vxmin; gxwork[1    ] = vxmin;}
               if (gywork[0] < vymin) {gywork[0] = vymin; gywork[npt-1] = vymin;}

               //transform to log ?
               ComputeLogs(npt, optionZ);
               gPad->PaintFillArea(npt,gxworkl,gyworkl);
               if (drawborder) {
                  if (!fillarea) gyworkl[0] = ylast;
                  gPad->PaintPolyLine(npt-1,gxworkl,gyworkl,noClip);
               }
               continue;
            }
         }  //endfor (j=first; j<=last;j++) {
      } else {
         gywork[0] = wmin;
         if (!optionOne) gxwork[0] = TMath::Max((Double_t)0,gPad->GetUxmin());
         else            gxwork[0] = gPad->GetUxmin();
         npt = 2;
         for (j=first; j<=last;j++) {
            if (!optionBins) {
               gywork[npt-1] = gywork[npt-2];
               gywork[npt]   = wmin+((j-first+1)*delta);
            } else {
               yj1 = y[j];      yj  = y[j-1];
               if (yj1 < yj) {
                  if (j != last) Error(where, "Y must be in increasing order");
                  else           Error(where, "Y must have N+1 values with option N");
                  return;
               }
               gywork[npt-1] = y[j-1];       gywork[npt] = y[j];
            }
            gxwork[npt-1] = x[j-1];      gxwork[npt] = x[j-1];
            if ((gxwork[npt-1] >= uxmin-rounding && gxwork[npt-1] <= uxmax+rounding) ||
                (gxwork[npt]   >= uxmin-rounding && gxwork[npt]   <= uxmax+rounding)) npt += 2;
            if (j == last) {
               gywork[npt-1] = gywork[npt-2];
               gxwork[npt-1] = gxwork[0];
               ComputeLogs(npt, optionZ);
               gPad->PaintFillArea(npt,gxworkl,gyworkl);
               if (drawborder) {
                  if (!fillarea) gyworkl[0] = ylast;
                  gPad->PaintPolyLine(npt-1,gxworkl,gyworkl,noClip);
               }
               continue;
            }
         }  //endfor (j=first; j<=last;j++)
      }
      theGraph->TAttLine::Modify();
      theGraph->TAttFill::Modify();
   }

   //      Draw a standard Histogram (default)

   if ((optionHist) || !chopt[0]) {
      if (!optionRot) {
         gxwork[0] = wmin;
         gywork[0] = gPad->GetUymin();
         ywmin    = gywork[0];
         npt      = 2;
         for (i=first; i<=last;i++) {
            if (!optionBins) {
               gxwork[npt-1] = gxwork[npt-2];
               gxwork[npt]   = wmin+((i-first+1)*delta);
            } else {
               xi1 = x[i];      xi  = x[i-1];
               if (xi1 < xi) {
                  if (i != last) Error(where, "X must be in increasing order");
                  else           Error(where, "X must have N+1 values with option N");
                  return;
               }
               gxwork[npt-1] = x[i-1];      gxwork[npt] = x[i];
            }
            gywork[npt-1] = y[i-1];
            gywork[npt]   = y[i-1];
            if (gywork[npt] < vymin) {gywork[npt] = vymin; gywork[npt-1] = vymin;}
            if ((gxwork[npt-1] >= uxmin-rounding && gxwork[npt-1] <= uxmax+rounding) ||
                (gxwork[npt]   >= uxmin-rounding && gxwork[npt]   <= uxmax+rounding)) npt += 2;
            if (i == last) {
               gxwork[npt-1] = gxwork[npt-2];
               gywork[npt-1] = gywork[0];
               //make sure that the fill area does not overwrite the frame
               //take into account the frame linewidth
               if (gxwork[0] < vxmin) {gxwork[0] = vxmin; gxwork[1    ] = vxmin;}
               if (gywork[0] < vymin) {gywork[0] = vymin; gywork[npt-1] = vymin;}

               ComputeLogs(npt, optionZ);

               // do not draw the two vertical lines on the edges
               Int_t nbpoints = npt-2;
               Int_t point1  = 1;

               if (optionOff) {
                  // remove points before the low cutoff
                  Int_t ip;
                  for (ip=point1; ip<=nbpoints; ip++) {
                     if (gyworkl[ip] != ywmin) {
                        point1 = ip;
                        break;
                     }
                  }
                  // remove points after the high cutoff
                  Int_t point2 = nbpoints;
                  for (ip=point2; ip>=point1; ip--) {
                     if (gyworkl[ip] != ywmin) {
                        point2 = ip;
                        break;
                     }
                  }
                  nbpoints = point2-point1+1;
               } else {
                  // if the 1st or last bin are not on the pad limits the
                  // the two vertical lines on the edges are added.
                  if (gxwork[0] > gPad->GetUxmin()) { nbpoints++; point1 = 0; }
                  if (gxwork[nbpoints] < gPad->GetUxmax()) nbpoints++;
               }

               gPad->PaintPolyLine(nbpoints,&gxworkl[point1],&gyworkl[point1],noClip);
               continue;
            }
         }  //endfor (i=first; i<=last;i++)
      } else {
         gywork[0] = wmin;
         gxwork[0] = TMath::Max((Double_t)0,gPad->GetUxmin());
         xwmin    = gxwork[0];
         npt      = 2;
         for (i=first; i<=last;i++) {
            if (!optionBins) {
               gywork[npt-1]   = gywork[npt-2];
               gywork[npt] = wmin+((i-first+1)*delta);
            } else {
               yi1 = y[i];      yi  = y[i-1];
               if (yi1 < yi) {
                  if (i != last) Error(where, "Y must be in increasing order");
                  else           Error(where, "Y must have N+1 values with option N");
                  return;
               }
               gywork[npt-1] = y[i-1];      gywork[npt] = y[i];
            }
            gxwork[npt-1] = x[i-1];      gxwork[npt] = x[i-1];
            if ((gxwork[npt-1] >= uxmin-rounding && gxwork[npt-1] <= uxmax+rounding) ||
                (gxwork[npt]   >= uxmin-rounding && gxwork[npt]   <= uxmax+rounding)) npt += 2;
            if (i == last) {
               gywork[npt-1] = gywork[npt-2];
               gxwork[npt-1] = xwmin;
               ComputeLogs(npt, optionZ);
               gPad->PaintPolyLine(npt,gxworkl,gyworkl,noClip);
               continue;
            }
         }  //endfor (i=first; i<=last;i++)
      }
   }

   //              Draw the histogram with a smooth Curve.
   //              The smoothing is done by the method Smooth()

   if (optionCurve) {
      if (!optionFill) {
         drawtype = 1;
      } else {
         if (!optionOne) drawtype = 2;
         else            drawtype = 3;
      }
      if (!optionRot) {
         npt = 0;
         for (i=first; i<=last;i++) {
            npt++;
            if (!optionBins) {
               gxwork[npt-1] = wmin+(i-first)*delta+0.5*delta;
            } else {
               xi1 = x[i];      xi  = x[i-1];
               if (xi1 < xi) {
                  if (i != last) Error(where, "X must be in increasing order");
                  else           Error(where, "X must have N+1 values with option N");
                  return;
               }
               gxwork[npt-1] = x[i-1] + 0.5*(x[i]-x[i-1]);
            }
            if (gxwork[npt-1] < uxmin || gxwork[npt-1] > uxmax) {
               npt--;
               continue;
            }
            gywork[npt-1] = y[i-1];
            ComputeLogs(npt, optionZ);
            if ((gyworkl[npt-1] < rwymin) || (gyworkl[npt-1] > rwymax)) {
               if (npt > 2) {
                  ComputeLogs(npt, optionZ);
                  Smooth(theGraph, npt,gxworkl,gyworkl,drawtype);
               }
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
               continue;
            }
            if (npt >= 50) {
               ComputeLogs(50, optionZ);
               Smooth(theGraph, 50,gxworkl,gyworkl,drawtype);
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
            }
         }  //endfor (i=first; i<=last;i++)
         if (npt > 1) {
            ComputeLogs(npt, optionZ);
            Smooth(theGraph, npt,gxworkl,gyworkl,drawtype);
         }
      } else {
         drawtype = drawtype+10;
         npt   = 0;
         for (i=first; i<=last;i++) {
            npt++;
            if (!optionBins) {
               gywork[npt-1] = wmin+(i-first)*delta+0.5*delta;
            } else {
               yi1 = y[i];      yi = y[i-1];
               if (yi1 < yi) {
                  if (i != last) Error(where, "Y must be in increasing order");
                  else           Error(where, "Y must have N+1 values with option N");
                  return;
               }
               gywork[npt-1] = y[i-1] + 0.5*(y[i]-y[i-1]);
            }
            gxwork[npt-1] = x[i-1];
            ComputeLogs(npt, optionZ);
            if ((gxworkl[npt] < uxmin) || (gxworkl[npt] > uxmax)) {
               if (npt > 2) {
                  ComputeLogs(npt, optionZ);
                  Smooth(theGraph, npt,gxworkl,gyworkl,drawtype);
               }
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
               continue;
            }
            if (npt >= 50) {
               ComputeLogs(50, optionZ);
               Smooth(theGraph, 50,gxworkl,gyworkl,drawtype);
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
            }
         }  //endfor (i=first; i<=last;i++)
         if (npt > 1) {
            ComputeLogs(npt, optionZ);
            Smooth(theGraph, npt,gxworkl,gyworkl,drawtype);
         }
      }
   }

   //    Draw the histogram with a simple line or/and a marker

   optionMarker = 0;
   if ((optionStar) || (optionMark))optionMarker=1;
   if ((optionMarker) || (optionLine)) {
      wminstep = wmin + 0.5*delta;
      Axis_t ax1,ax2,ay1,ay2;
      gPad->GetRangeAxis(ax1,ay1,ax2,ay2);

      if (!optionRot) {
         npt = 0;
         for (i=first; i<=last;i++) {
            npt++;
            if (!optionBins) {
               gxwork[npt-1] = wmin+(i-first)*delta+0.5*delta;
            } else {
               xi1 = x[i];      xi = x[i-1];
               if (xi1 < xi) {
                  if (i != last) Error(where, "X must be in increasing order");
                  else           Error(where, "X must have N+1 values with option N");
                  return;
               }
               gxwork[npt-1] = x[i-1] + 0.5*(x[i]-x[i-1]);
            }
            if (gxwork[npt-1] < uxmin || gxwork[npt-1] > uxmax) { npt--; continue;}
            if ((optionMark != 10) && (optionLine == 0)) {
               if (y[i-1] <= rwymin)  {npt--; continue;}
            }
            gywork[npt-1] = y[i-1];
            gywork[npt]   = y[i-1]; //new
            if ((gywork[npt-1] < rwymin) || ((gywork[npt-1] > rwymax) && !optionFill2)) {
               if ((gywork[npt-1] < rwymin)) gywork[npt-1] = rwymin;
               if ((gywork[npt-1] > rwymax)) gywork[npt-1] = rwymax;
               if (npt > 2) {
                  if (optionMarker) {
                     ComputeLogs(npt, optionZ);
                     gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
                  }
                  if (optionLine) {
                     if (!optionMarker) ComputeLogs(npt, optionZ);
                     gPad->PaintPolyLine(npt,gxworkl,gyworkl,noClip);
                  }
               }
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt       = 1;
               continue;
            }

            if (npt >= 50) {
               if (optionMarker) {
                  ComputeLogs(50, optionZ);
                  gPad->PaintPolyMarker(50,gxworkl,gyworkl);
               }
               if (optionLine) {
                  if (!optionMarker) ComputeLogs(50, optionZ);
                  if (optionFill2) {
                     gxworkl[npt]   = gxworkl[npt-1]; gyworkl[npt]   = rwymin;
                     gxworkl[npt+1] = gxworkl[0];     gyworkl[npt+1] = rwymin;
                     gPad->PaintFillArea(52,gxworkl,gyworkl);
                  }
                  gPad->PaintPolyLine(50,gxworkl,gyworkl);
               }
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
            }
         }  //endfor (i=first; i<=last;i++)
         if (optionMarker && npt > 0) {
            ComputeLogs(npt, optionZ);
            gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
         }
         if (optionLine && npt > 1) {
            if (!optionMarker) ComputeLogs(npt, optionZ);
            if (optionFill2) {
               gxworkl[npt]   = gxworkl[npt-1]; gyworkl[npt]   = rwymin;
               gxworkl[npt+1] = gxworkl[0];     gyworkl[npt+1] = rwymin;
               gPad->PaintFillArea(npt+2,gxworkl,gyworkl);
            }
            gPad->PaintPolyLine(npt,gxworkl,gyworkl);
         }
      } else {
         npt = 0;
         for (i=first; i<=last;i++) {
            npt++;
            if (!optionBins) {
               gywork[npt-1] = wminstep+(i-first)*delta+0.5*delta;
            } else {
               yi1 = y[i];      yi = y[i-1];
               if (yi1 < yi) {
                  if (i != last) Error(where, "Y must be in increasing order");
                  else           Error(where, "Y must have N+1 values with option N");
                  return;
               }
               gywork[npt-1] = y[i-1] + 0.5*(y[i]-y[i-1]);
            }
            gxwork[npt-1] = x[i-1];
            if ((gxwork[npt-1] < uxmin) || (gxwork[npt-1] > uxmax)) {
               if (npt > 2) {
                  if (optionMarker) {
                     ComputeLogs(npt, optionZ);
                     gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
                  }
                  if (optionLine) {
                     if (!optionMarker) ComputeLogs(npt, optionZ);
                     gPad->PaintPolyLine(npt,gxworkl,gyworkl,noClip);
                  }
               }
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
               continue;
            }
            if (npt >= 50) {
               if (optionMarker) {
                  ComputeLogs(50, optionZ);
                  gPad->PaintPolyMarker(50,gxworkl,gyworkl);
               }
               if (optionLine) {
                  if (!optionMarker) ComputeLogs(50, optionZ);
                  gPad->PaintPolyLine(50,gxworkl,gyworkl);
               }
               gxwork[0] = gxwork[npt-1];
               gywork[0] = gywork[npt-1];
               npt      = 1;
            }
         }  //endfor (i=first; i<=last;i++)
         if (optionMarker && npt > 0) {
            ComputeLogs(npt, optionZ);
            gPad->PaintPolyMarker(npt,gxworkl,gyworkl);
         }
         if (optionLine != 0 && npt > 1) {
            if (!optionMarker) ComputeLogs(npt, optionZ);
            gPad->PaintPolyLine(npt,gxworkl,gyworkl,noClip);
         }
      }
   }

   //              Draw the histogram as a bar chart

   if (optionBar) {
      if (!optionBins) {
         offset = delta*baroffset; dbar = delta*barwidth;
      } else {
         if (!optionRot) {
            offset = (x[1]-x[0])*baroffset;
            dbar   = (x[1]-x[0])*barwidth;
         } else {
            offset = (y[1]-y[0])*baroffset;
            dbar   = (y[1]-y[0])*barwidth;
         }
      }
      drawbordersav = drawborder;
      gStyle->SetDrawBorder(1);
      if (!optionRot) {
         xlow  = wmin+offset;
         xhigh = wmin+offset+dbar;
         if (!optionOne) ylow = TMath::Min(TMath::Max((Double_t)0,gPad->GetUymin())
                                ,gPad->GetUymax());
         else            ylow = gPad->GetUymin();

         for (i=first; i<=last;i++) {
            yhigh    = y[i-1];
            gxwork[0] = xlow;
            gywork[0] = ylow;
            gxwork[1] = xhigh;
            gywork[1] = yhigh;
            ComputeLogs(2, optionZ);
            gPad->PaintBox(gxworkl[0],gyworkl[0],gxworkl[1],gyworkl[1]);
            if (!optionBins) {
               xlow  = xlow+delta;
               xhigh = xhigh+delta;
            } else {
               if (i < last) {
                  xi1 = x[i];      xi = x[i-1];
                  if (xi1 < xi) {
                     Error(where, "X must be in increasing order");
                     return;
                  }
                  offset  = (x[i+1]-x[i])*baroffset;
                  dbar    = (x[i+1]-x[i])*barwidth;
                  xlow    = x[i] + offset;
                  xhigh   = x[i] + offset + dbar;
               }
            }
         }  //endfor (i=first; i<=last;i++)
      } else {
         ylow  = wmin + offset;
         yhigh = wmin + offset + dbar;
         if (!optionOne) xlow = TMath::Max((Double_t)0,gPad->GetUxmin());
         else            xlow = gPad->GetUxmin();

         for (i=first; i<=last;i++) {
            xhigh    = x[i-1];
            gxwork[0] = xlow;
            gywork[0] = ylow;
            gxwork[1] = xhigh;
            gywork[1] = yhigh;
            ComputeLogs(2, optionZ);
            gPad->PaintBox(gxworkl[0],gyworkl[0],gxworkl[1],gyworkl[1]);
            gPad->PaintBox(xlow,ylow,xhigh,yhigh);
            if (!optionBins) {
               ylow  = ylow  + delta;
               yhigh = yhigh + delta;
            } else {
               if (i < last) {
                  yi1 = y[i];      yi = y[i-1];
                  if (yi1 < yi) {
                     Error(where, "Y must be in increasing order");
                     return;
                  }
                  offset  = (y[i+1]-y[i])*baroffset;
                  dbar    = (y[i+1]-y[i])*barwidth;
                  ylow    = y[i] + offset;
                  yhigh   = y[i] + offset + dbar;
               }
            }
         }  //endfor (i=first; i<=last;i++)
      }
      gStyle->SetDrawBorder(drawbordersav);
   }
   gPad->ResetBit(TGraph::kClipFrame);

   delete [] gxwork;
   delete [] gywork;
   delete [] gxworkl;
   delete [] gyworkl;
}


//______________________________________________________________________________
void TGraphPainter::PaintGraphAsymmErrors(TGraph *theGraph, Option_t *option)
{
   /* Begin_Html
   <a href="#GP03">Paint this TGraphAsymmErrors with its current attributes.</a>
   End_Html */

   Double_t *xline = 0;
   Double_t *yline = 0;
   Int_t if1 = 0;
   Int_t if2 = 0;
   Double_t xb[4], yb[4];

   const Int_t kBASEMARKER=8;
   Double_t s2x, s2y, symbolsize, sbase;
   Double_t x, y, xl1, xl2, xr1, xr2, yup1, yup2, ylow1, ylow2, tx, ty;
   static Float_t cxx[15] = {1,1,0.6,0.6,1,1,0.6,0.5,1,0.6,0.6,1,0.6,1,1};
   static Float_t cyy[15] = {1,1,1,1,1,1,1,1,1,0.5,0.6,1,1,1,1};
   Int_t theNpoints = theGraph->GetN();
   Double_t *theX  = theGraph->GetX();
   Double_t *theY  = theGraph->GetY();
   Double_t *theEXlow  = theGraph->GetEXlow();  if (!theEXlow) return;
   Double_t *theEYlow  = theGraph->GetEYlow();  if (!theEYlow) return;
   Double_t *theEXhigh = theGraph->GetEXhigh(); if (!theEXhigh) return;
   Double_t *theEYhigh = theGraph->GetEYhigh(); if (!theEYhigh) return;

   if (strchr(option,'X') || strchr(option,'x')) {PaintGraphSimple(theGraph, option); return;}
   Bool_t brackets = kFALSE;
   Bool_t braticks = kFALSE;
   if (strstr(option,"||") || strstr(option,"[]")) {
      brackets = kTRUE;
      if (strstr(option,"[]")) braticks = kTRUE;
   }
   Bool_t endLines = kTRUE;
   if (strchr(option,'z')) endLines = kFALSE;
   if (strchr(option,'Z')) endLines = kFALSE;
   const char *arrowOpt = 0;
   if (strchr(option,'>'))  arrowOpt = ">";
   if (strstr(option,"|>")) arrowOpt = "|>";

   Bool_t axis = kFALSE;
   if (strchr(option,'a')) axis = kTRUE;
   if (strchr(option,'A')) axis = kTRUE;
   if (axis) PaintGraphSimple(theGraph, option);

   Bool_t option0 = kFALSE;
   Bool_t option2 = kFALSE;
   Bool_t option3 = kFALSE;
   Bool_t option4 = kFALSE;
   Bool_t option5 = kFALSE;
   if (strchr(option,'0')) option0 = kTRUE;
   if (strchr(option,'2')) option2 = kTRUE;
   if (strchr(option,'3')) option3 = kTRUE;
   if (strchr(option,'4')) {option3 = kTRUE; option4 = kTRUE;}
   if (strchr(option,'5')) {option2 = kTRUE; option5 = kTRUE;}

   if (option3) {
      xline = new Double_t[2*theNpoints];
      yline = new Double_t[2*theNpoints];
      if (!xline || !yline) {
         Error("Paint", "too many points, out of memory");
         return;
      }
      if1 = 1;
      if2 = 2*theNpoints;
   }

   theGraph->TAttLine::Modify();

   TArrow arrow;
   arrow.SetLineWidth(theGraph->GetLineWidth());
   arrow.SetLineColor(theGraph->GetLineColor());
   arrow.SetFillColor(theGraph->GetFillColor());

   TBox box;
   Double_t x1b,y1b,x2b,y2b;
   box.SetLineWidth(theGraph->GetLineWidth());
   box.SetLineColor(theGraph->GetLineColor());
   box.SetFillColor(theGraph->GetFillColor());
   box.SetFillStyle(theGraph->GetFillStyle());

   symbolsize  = theGraph->GetMarkerSize();
   sbase       = symbolsize*kBASEMARKER;
   Int_t mark  = theGraph->GetMarkerStyle();
   Double_t cx  = 0;
   Double_t cy  = 0;
   if (mark >= 20 && mark <= 34) {
      cx = cxx[mark-20];
      cy = cyy[mark-20];
   }

   // Define the offset of the error bars due to the symbol size
   s2x  = gPad->PixeltoX(Int_t(0.5*sbase)) - gPad->PixeltoX(0);
   s2y  =-gPad->PixeltoY(Int_t(0.5*sbase)) + gPad->PixeltoY(0);
   Int_t dxend = Int_t(gStyle->GetEndErrorSize());
   tx    = gPad->PixeltoX(dxend) - gPad->PixeltoX(0);
   ty    =-gPad->PixeltoY(dxend) + gPad->PixeltoY(0);
   Float_t asize = 0.6*symbolsize*kBASEMARKER/gPad->GetWh();

   gPad->SetBit(TGraph::kClipFrame, theGraph->TestBit(TGraph::kClipFrame));
   for (Int_t i=0;i<theNpoints;i++) {
      x  = gPad->XtoPad(theX[i]);
      y  = gPad->YtoPad(theY[i]);
      if (!option0) {
         if (option3) {
            if (x < gPad->GetUxmin()) x = gPad->GetUxmin();
            if (x > gPad->GetUxmax()) x = gPad->GetUxmax();
            if (y < gPad->GetUymin()) y = gPad->GetUymin();
            if (y > gPad->GetUymax()) y = gPad->GetUymax();
         } else {
            if (x < gPad->GetUxmin()) continue;
            if (x > gPad->GetUxmax()) continue;
            if (y < gPad->GetUymin()) continue;
            if (y > gPad->GetUymax()) continue;
         }
      }
      xl1 = x - s2x*cx;
      xl2 = gPad->XtoPad(theX[i] - theEXlow[i]);

      //  draw the error rectangles
      if (option2) {
         x1b = gPad->XtoPad(theX[i] - theEXlow[i]);
         y1b = gPad->YtoPad(theY[i] - theEYlow[i]);
         x2b = gPad->XtoPad(theX[i] + theEXhigh[i]);
         y2b = gPad->YtoPad(theY[i] + theEYhigh[i]);
         if (x1b < gPad->GetUxmin()) x1b = gPad->GetUxmin();
         if (x1b > gPad->GetUxmax()) x1b = gPad->GetUxmax();
         if (y1b < gPad->GetUymin()) y1b = gPad->GetUymin();
         if (y1b > gPad->GetUymax()) y1b = gPad->GetUymax();
         if (x2b < gPad->GetUxmin()) x2b = gPad->GetUxmin();
         if (x2b > gPad->GetUxmax()) x2b = gPad->GetUxmax();
         if (y2b < gPad->GetUymin()) y2b = gPad->GetUymin();
         if (y2b > gPad->GetUymax()) y2b = gPad->GetUymax();
         if (option5) box.PaintBox(x1b, y1b, x2b, y2b, "l");
         else         box.PaintBox(x1b, y1b, x2b, y2b);
         continue;
      }

      //  keep points for fill area drawing
      if (option3) {
         xline[if1-1] = x;
         xline[if2-1] = x;
         yline[if1-1] = gPad->YtoPad(theY[i] + theEYhigh[i]);
         yline[if2-1] = gPad->YtoPad(theY[i] - theEYlow[i]);
         if1++;
         if2--;
         continue;
      }

      if (xl1 > xl2) {
         if (arrowOpt) {
            arrow.PaintArrow(xl1,y,xl2,y,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(xl1,y,xl2,y);
            if (endLines) {
               if (braticks) {
                  xb[0] = xl2+tx; yb[0] = y-ty;
                  xb[1] = xl2;    yb[1] = y-ty;
                  xb[2] = xl2;    yb[2] = y+ty;
                  xb[3] = xl2+tx; yb[3] = y+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(xl2,y-ty,xl2,y+ty);
               }
            }
         }
      }
      xr1 = x + s2x*cx;
      xr2 = gPad->XtoPad(theX[i] + theEXhigh[i]);
      if (xr1 < xr2) {
         if (arrowOpt) {
            arrow.PaintArrow(xr1,y,xr2,y,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(xr1,y,xr2,y);
            if (endLines) {
               if (braticks) {
                  xb[0] = xr2-tx; yb[0] = y-ty;
                  xb[1] = xr2;    yb[1] = y-ty;
                  xb[2] = xr2;    yb[2] = y+ty;
                  xb[3] = xr2-tx; yb[3] = y+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(xr2,y-ty,xr2,y+ty);
               }
            }
         }
      }
      yup1 = y + s2y*cy;
      yup2 = gPad->YtoPad(theY[i] + theEYhigh[i]);
      if (yup2 > gPad->GetUymax()) yup2 =  gPad->GetUymax();
      if (yup2 > yup1) {
         if (arrowOpt) {
            arrow.PaintArrow(x,yup1,x,yup2,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(x,yup1,x,yup2);
            if (endLines) {
               if (braticks) {
                  xb[0] = x-tx; yb[0] = yup2-ty;
                  xb[1] = x-tx; yb[1] = yup2;
                  xb[2] = x+tx; yb[2] = yup2;
                  xb[3] = x+tx; yb[3] = yup2-ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(x-tx,yup2,x+tx,yup2);
               }
            }
         }
      }
      ylow1 = y - s2y*cy;
      ylow2 = gPad->YtoPad(theY[i] - theEYlow[i]);
      if (ylow2 < gPad->GetUymin()) ylow2 =  gPad->GetUymin();
      if (ylow2 < ylow1) {
         if (arrowOpt) {
            arrow.PaintArrow(x,ylow1,x,ylow2,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(x,ylow1,x,ylow2);
            if (endLines) {
               if (braticks) {
                  xb[0] = x-tx; yb[0] = ylow2+ty;
                  xb[1] = x-tx; yb[1] = ylow2;
                  xb[2] = x+tx; yb[2] = ylow2;
                  xb[3] = x+tx; yb[3] = ylow2+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(x-tx,ylow2,x+tx,ylow2);
               }
            }
         }
      }
   }
   if (!brackets && !axis) PaintGraphSimple(theGraph, option);
   gPad->ResetBit(TGraph::kClipFrame);

   if (option3) {
      Int_t logx = gPad->GetLogx();
      Int_t logy = gPad->GetLogy();
      gPad->SetLogx(0);
      gPad->SetLogy(0);
      if (option4) PaintGraph(theGraph, 2*theNpoints, xline, yline,"FC");
      else         PaintGraph(theGraph, 2*theNpoints, xline, yline,"F");
      gPad->SetLogx(logx);
      gPad->SetLogy(logy);
      delete [] xline;
      delete [] yline;
   }
}


//_____________________________________________________________________________
void TGraphPainter::PaintGraphBentErrors(TGraph *theGraph, Option_t *option)
{
   /* Begin_Html
   <a href="#GP03">Paint this TGraphBentErrors with its current attributes.</a>
   End_Html */

   Double_t *xline = 0;
   Double_t *yline = 0;
   Int_t if1 = 0;
   Int_t if2 = 0;
   Double_t xb[4], yb[4];

   const Int_t kBASEMARKER=8;
   Double_t s2x, s2y, symbolsize, sbase;
   Double_t x, y, xl1, xl2, xr1, xr2, yup1, yup2, ylow1, ylow2, tx, ty;
   Double_t bxl, bxh, byl, byh;
   static Float_t cxx[15] = {1,1,0.6,0.6,1,1,0.6,0.5,1,0.6,0.6,1,0.6,1,1};
   static Float_t cyy[15] = {1,1,1,1,1,1,1,1,1,0.5,0.6,1,1,1,1};
   Int_t theNpoints = theGraph->GetN();
   Double_t *theX  = theGraph->GetX();
   Double_t *theY  = theGraph->GetY();
   Double_t *theEXlow   = theGraph->GetEXlow();   if (!theEXlow) return;
   Double_t *theEYlow   = theGraph->GetEYlow();   if (!theEYlow) return;
   Double_t *theEXhigh  = theGraph->GetEXhigh();  if (!theEXhigh) return;
   Double_t *theEYhigh  = theGraph->GetEYhigh();  if (!theEYhigh) return;
   Double_t *theEXlowd  = theGraph->GetEXlowd();  if (!theEXlowd) return;
   Double_t *theEXhighd = theGraph->GetEXhighd(); if (!theEXhighd) return;
   Double_t *theEYlowd  = theGraph->GetEYlowd();  if (!theEYlowd) return;
   Double_t *theEYhighd = theGraph->GetEYhighd(); if (!theEYhighd) return;

   if (strchr(option,'X') || strchr(option,'x')) {PaintGraphSimple(theGraph, option); return;}
   Bool_t brackets = kFALSE;
   Bool_t braticks = kFALSE;
   if (strstr(option,"||") || strstr(option,"[]")) {
      brackets = kTRUE;
      if (strstr(option,"[]")) braticks = kTRUE;
   }
   Bool_t endLines = kTRUE;
   if (strchr(option,'z')) endLines = kFALSE;
   if (strchr(option,'Z')) endLines = kFALSE;
   const char *arrowOpt = 0;
   if (strchr(option,'>'))  arrowOpt = ">";
   if (strstr(option,"|>")) arrowOpt = "|>";

   Bool_t axis = kFALSE;
   if (strchr(option,'a')) axis = kTRUE;
   if (strchr(option,'A')) axis = kTRUE;
   if (axis) PaintGraphSimple(theGraph,option);

   Bool_t option0 = kFALSE;
   Bool_t option2 = kFALSE;
   Bool_t option3 = kFALSE;
   Bool_t option4 = kFALSE;
   Bool_t option5 = kFALSE;
   if (strchr(option,'0')) option0 = kTRUE;
   if (strchr(option,'2')) option2 = kTRUE;
   if (strchr(option,'3')) option3 = kTRUE;
   if (strchr(option,'4')) {option3 = kTRUE; option4 = kTRUE;}
   if (strchr(option,'5')) {option2 = kTRUE; option5 = kTRUE;}

   if (option3) {
      xline = new Double_t[2*theNpoints];
      yline = new Double_t[2*theNpoints];
      if (!xline || !yline) {
         Error("Paint", "too many points, out of memory");
         return;
      }
      if1 = 1;
      if2 = 2*theNpoints;
   }

   theGraph->TAttLine::Modify();

   TArrow arrow;
   arrow.SetLineWidth(theGraph->GetLineWidth());
   arrow.SetLineColor(theGraph->GetLineColor());
   arrow.SetFillColor(theGraph->GetFillColor());

   TBox box;
   Double_t x1b,y1b,x2b,y2b;
   box.SetLineWidth(theGraph->GetLineWidth());
   box.SetLineColor(theGraph->GetLineColor());
   box.SetFillColor(theGraph->GetFillColor());
   box.SetFillStyle(theGraph->GetFillStyle());

   symbolsize  = theGraph->GetMarkerSize();
   sbase       = symbolsize*kBASEMARKER;
   Int_t mark  = theGraph->GetMarkerStyle();
   Double_t cx  = 0;
   Double_t cy  = 0;
   if (mark >= 20 && mark <= 34) {
      cx = cxx[mark-20];
      cy = cyy[mark-20];
   }

   // define the offset of the error bars due to the symbol size
   s2x  = gPad->PixeltoX(Int_t(0.5*sbase)) - gPad->PixeltoX(0);
   s2y  =-gPad->PixeltoY(Int_t(0.5*sbase)) + gPad->PixeltoY(0);
   Int_t dxend = Int_t(gStyle->GetEndErrorSize());
   tx   = gPad->PixeltoX(dxend) - gPad->PixeltoX(0);
   ty   =-gPad->PixeltoY(dxend) + gPad->PixeltoY(0);
   Float_t asize = 0.6*symbolsize*kBASEMARKER/gPad->GetWh();

   gPad->SetBit(TGraph::kClipFrame, theGraph->TestBit(TGraph::kClipFrame));
   for (Int_t i=0;i<theNpoints;i++) {
      x  = gPad->XtoPad(theX[i]);
      y  = gPad->YtoPad(theY[i]);
      bxl = gPad->YtoPad(theY[i]+theEXlowd[i]);
      bxh = gPad->YtoPad(theY[i]+theEXhighd[i]);
      byl = gPad->XtoPad(theX[i]+theEYlowd[i]);
      byh = gPad->XtoPad(theX[i]+theEYhighd[i]);
      if (!option0) {
         if (option3) {
            if (x < gPad->GetUxmin()) x = gPad->GetUxmin();
            if (x > gPad->GetUxmax()) x = gPad->GetUxmax();
            if (y < gPad->GetUymin()) y = gPad->GetUymin();
            if (y > gPad->GetUymax()) y = gPad->GetUymax();
         } else {
            if (x < gPad->GetUxmin()) continue;
            if (x > gPad->GetUxmax()) continue;
            if (y < gPad->GetUymin()) continue;
            if (y > gPad->GetUymax()) continue;
         }
      }

      //  draw the error rectangles
      if (option2) {
         x1b = gPad->XtoPad(theX[i] - theEXlow[i]);
         y1b = gPad->YtoPad(theY[i] - theEYlow[i]);
         x2b = gPad->XtoPad(theX[i] + theEXhigh[i]);
         y2b = gPad->YtoPad(theY[i] + theEYhigh[i]);
         if (x1b < gPad->GetUxmin()) x1b = gPad->GetUxmin();
         if (x1b > gPad->GetUxmax()) x1b = gPad->GetUxmax();
         if (y1b < gPad->GetUymin()) y1b = gPad->GetUymin();
         if (y1b > gPad->GetUymax()) y1b = gPad->GetUymax();
         if (x2b < gPad->GetUxmin()) x2b = gPad->GetUxmin();
         if (x2b > gPad->GetUxmax()) x2b = gPad->GetUxmax();
         if (y2b < gPad->GetUymin()) y2b = gPad->GetUymin();
         if (y2b > gPad->GetUymax()) y2b = gPad->GetUymax();
         if (option5) box.PaintBox(x1b, y1b, x2b, y2b, "l");
         else         box.PaintBox(x1b, y1b, x2b, y2b);
         continue;
      }

      //  keep points for fill area drawing
      if (option3) {
         xline[if1-1] = byh;
         xline[if2-1] = byl;
         yline[if1-1] = gPad->YtoPad(theY[i] + theEYhigh[i]);
         yline[if2-1] = gPad->YtoPad(theY[i] - theEYlow[i]);
         if1++;
         if2--;
         continue;
      }

      xl1 = x - s2x*cx;
      xl2 = gPad->XtoPad(theX[i] - theEXlow[i]);
      if (xl1 > xl2) {
         if (arrowOpt) {
            arrow.PaintArrow(xl1,y,xl2,bxl,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(xl1,y,xl2,bxl);
            if (endLines) {
               if (braticks) {
                  xb[0] = xl2+tx; yb[0] = bxl-ty;
                  xb[1] = xl2;    yb[1] = bxl-ty;
                  xb[2] = xl2;    yb[2] = bxl+ty;
                  xb[3] = xl2+tx; yb[3] = bxl+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(xl2,bxl-ty,xl2,bxl+ty);
               }
            }
         }
      }
      xr1 = x + s2x*cx;
      xr2 = gPad->XtoPad(theX[i] + theEXhigh[i]);
      if (xr1 < xr2) {
         if (arrowOpt) {
            arrow.PaintArrow(xr1,y,xr2,bxh,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(xr1,y,xr2,bxh);
            if (endLines) {
               if (braticks) {
                  xb[0] = xr2-tx; yb[0] = bxh-ty;
                  xb[1] = xr2;    yb[1] = bxh-ty;
                  xb[2] = xr2;    yb[2] = bxh+ty;
                  xb[3] = xr2-tx; yb[3] = bxh+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(xr2,bxh-ty,xr2,bxh+ty);
               }
            }
         }
      }
      yup1 = y + s2y*cy;
      yup2 = gPad->YtoPad(theY[i] + theEYhigh[i]);
      if (yup2 > gPad->GetUymax()) yup2 =  gPad->GetUymax();
      if (yup2 > yup1) {
         if (arrowOpt) {
            arrow.PaintArrow(x,yup1,byh,yup2,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(x,yup1,byh,yup2);
            if (endLines) {
               if (braticks) {
                  xb[0] = byh-tx; yb[0] = yup2-ty;
                  xb[1] = byh-tx; yb[1] = yup2;
                  xb[2] = byh+tx; yb[2] = yup2;
                  xb[3] = byh+tx; yb[3] = yup2-ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(byh-tx,yup2,byh+tx,yup2);
               }
            }
         }
      }
      ylow1 = y - s2y*cy;
      ylow2 = gPad->YtoPad(theY[i] - theEYlow[i]);
      if (ylow2 < gPad->GetUymin()) ylow2 =  gPad->GetUymin();
      if (ylow2 < ylow1) {
         if (arrowOpt) {
            arrow.PaintArrow(x,ylow1,byl,ylow2,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(x,ylow1,byl,ylow2);
            if (endLines) {
               if (braticks) {
                  xb[0] = byl-tx; yb[0] = ylow2+ty;
                  xb[1] = byl-tx; yb[1] = ylow2;
                  xb[2] = byl+tx; yb[2] = ylow2;
                  xb[3] = byl+tx; yb[3] = ylow2+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(byl-tx,ylow2,byl+tx,ylow2);
               }
            }
         }
      }
   }
   if (!brackets && !axis) PaintGraphSimple(theGraph, option);
   gPad->ResetBit(TGraph::kClipFrame);

   if (option3) {
      Int_t logx = gPad->GetLogx();
      Int_t logy = gPad->GetLogy();
      gPad->SetLogx(0);
      gPad->SetLogy(0);
      if (option4) PaintGraph(theGraph, 2*theNpoints, xline, yline,"FC");
      else         PaintGraph(theGraph, 2*theNpoints, xline, yline,"F");
      gPad->SetLogx(logx);
      gPad->SetLogy(logy);
      delete [] xline;
      delete [] yline;
   }
}


//______________________________________________________________________________
void TGraphPainter::PaintGraphErrors(TGraph *theGraph, Option_t *option)
{
   /* Begin_Html
   <a href="#GP03">Paint this TGraphErrors with its current attributes.</a>
   End_Html */

   Double_t *xline = 0;
   Double_t *yline = 0;
   Int_t if1 = 0;
   Int_t if2 = 0;
   Double_t xb[4], yb[4];

   const Int_t kBASEMARKER=8;
   Double_t s2x, s2y, symbolsize, sbase;
   Double_t x, y, ex, ey, xl1, xl2, xr1, xr2, yup1, yup2, ylow1, ylow2, tx, ty;
   static Float_t cxx[15] = {1,1,0.6,0.6,1,1,0.6,0.5,1,0.6,0.6,1,0.6,1,1};
   static Float_t cyy[15] = {1,1,1,1,1,1,1,1,1,0.5,0.6,1,1,1,1};
   Int_t theNpoints = theGraph->GetN();
   Double_t *theX  = theGraph->GetX();
   Double_t *theY  = theGraph->GetY();
   Double_t *theEX = theGraph->GetEX(); if (!theEX) return;
   Double_t *theEY = theGraph->GetEY(); if (!theEY) return;

   if (strchr(option,'X') || strchr(option,'x')) {PaintGraphSimple(theGraph, option); return;}
   Bool_t brackets = kFALSE;
   Bool_t braticks = kFALSE;
   if (strstr(option,"||") || strstr(option,"[]")) {
      brackets = kTRUE;
      if (strstr(option,"[]")) braticks = kTRUE;
   }
   Bool_t endLines = kTRUE;
   if (strchr(option,'z')) endLines = kFALSE;
   if (strchr(option,'Z')) endLines = kFALSE;
   const char *arrowOpt = 0;
   if (strchr(option,'>'))  arrowOpt = ">";
   if (strstr(option,"|>")) arrowOpt = "|>";

   Bool_t axis = kFALSE;
   if (strchr(option,'a')) axis = kTRUE;
   if (strchr(option,'A')) axis = kTRUE;
   if (axis) PaintGraphSimple(theGraph, option);

   Bool_t option0 = kFALSE;
   Bool_t option2 = kFALSE;
   Bool_t option3 = kFALSE;
   Bool_t option4 = kFALSE;
   Bool_t option5 = kFALSE;
   if (strchr(option,'0')) option0 = kTRUE;
   if (strchr(option,'2')) option2 = kTRUE;
   if (strchr(option,'3')) option3 = kTRUE;
   if (strchr(option,'4')) {option3 = kTRUE; option4 = kTRUE;}
   if (strchr(option,'5')) {option2 = kTRUE; option5 = kTRUE;}

   if (option3) {
      xline = new Double_t[2*theNpoints];
      yline = new Double_t[2*theNpoints];
      if (!xline || !yline) {
         Error("Paint", "too many points, out of memory");
         return;
      }
      if1 = 1;
      if2 = 2*theNpoints;
   }

   theGraph->TAttLine::Modify();

   TArrow arrow;
   arrow.SetLineWidth(theGraph->GetLineWidth());
   arrow.SetLineColor(theGraph->GetLineColor());
   arrow.SetFillColor(theGraph->GetFillColor());

   TBox box;
   Double_t x1b,y1b,x2b,y2b;
   box.SetLineWidth(theGraph->GetLineWidth());
   box.SetLineColor(theGraph->GetLineColor());
   box.SetFillColor(theGraph->GetFillColor());
   box.SetFillStyle(theGraph->GetFillStyle());

   symbolsize  = theGraph->GetMarkerSize();
   sbase       = symbolsize*kBASEMARKER;
   Int_t mark  = theGraph->GetMarkerStyle();
   Double_t cx  = 0;
   Double_t cy  = 0;
   if (mark >= 20 && mark <= 34) {
      cx = cxx[mark-20];
      cy = cyy[mark-20];
   }

   //      define the offset of the error bars due to the symbol size
   s2x  = gPad->PixeltoX(Int_t(0.5*sbase)) - gPad->PixeltoX(0);
   s2y  =-gPad->PixeltoY(Int_t(0.5*sbase)) + gPad->PixeltoY(0);
   Int_t dxend = Int_t(gStyle->GetEndErrorSize());
   tx    = gPad->PixeltoX(dxend) - gPad->PixeltoX(0);
   ty    =-gPad->PixeltoY(dxend) + gPad->PixeltoY(0);
   Float_t asize = 0.6*symbolsize*kBASEMARKER/gPad->GetWh();

   gPad->SetBit(TGraph::kClipFrame, theGraph->TestBit(TGraph::kClipFrame));
   for (Int_t i=0;i<theNpoints;i++) {
      x  = gPad->XtoPad(theX[i]);
      y  = gPad->YtoPad(theY[i]);
      if (!option0) {
         if (option3) {
            if (x < gPad->GetUxmin()) x = gPad->GetUxmin();
            if (x > gPad->GetUxmax()) x = gPad->GetUxmax();
            if (y < gPad->GetUymin()) y = gPad->GetUymin();
            if (y > gPad->GetUymax()) y = gPad->GetUymax();
         } else {
            if (x < gPad->GetUxmin()) continue;
            if (x > gPad->GetUxmax()) continue;
            if (y < gPad->GetUymin()) continue;
            if (y > gPad->GetUymax()) continue;
         }
      }
      ex = theEX[i];
      ey = theEY[i];

      //  draw the error rectangles
      if (option2) {
         x1b = gPad->XtoPad(theX[i] - ex);
         y1b = gPad->YtoPad(theY[i] - ey);
         x2b = gPad->XtoPad(theX[i] + ex);
         y2b = gPad->YtoPad(theY[i] + ey);
         if (x1b < gPad->GetUxmin()) x1b = gPad->GetUxmin();
         if (x1b > gPad->GetUxmax()) x1b = gPad->GetUxmax();
         if (y1b < gPad->GetUymin()) y1b = gPad->GetUymin();
         if (y1b > gPad->GetUymax()) y1b = gPad->GetUymax();
         if (x2b < gPad->GetUxmin()) x2b = gPad->GetUxmin();
         if (x2b > gPad->GetUxmax()) x2b = gPad->GetUxmax();
         if (y2b < gPad->GetUymin()) y2b = gPad->GetUymin();
         if (y2b > gPad->GetUymax()) y2b = gPad->GetUymax();
         if (option5) box.PaintBox(x1b, y1b, x2b, y2b, "l");
         else         box.PaintBox(x1b, y1b, x2b, y2b);
         continue;
      }

      //  keep points for fill area drawing
      if (option3) {
         xline[if1-1] = x;
         xline[if2-1] = x;
         yline[if1-1] = gPad->YtoPad(theY[i] + ey);
         yline[if2-1] = gPad->YtoPad(theY[i] - ey);
         if1++;
         if2--;
         continue;
      }

      xl1 = x - s2x*cx;
      xl2 = gPad->XtoPad(theX[i] - ex);
      if (xl1 > xl2) {
         if (arrowOpt) {
            arrow.PaintArrow(xl1,y,xl2,y,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(xl1,y,xl2,y);
            if (endLines) {
               if (braticks) {
                  xb[0] = xl2+tx; yb[0] = y-ty;
                  xb[1] = xl2;    yb[1] = y-ty;
                  xb[2] = xl2;    yb[2] = y+ty;
                  xb[3] = xl2+tx; yb[3] = y+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(xl2,y-ty,xl2,y+ty);
               }
            }
         }
      }
      xr1 = x + s2x*cx;
      xr2 = gPad->XtoPad(theX[i] + ex);
      if (xr1 < xr2) {
         if (arrowOpt) {
            arrow.PaintArrow(xr1,y,xr2,y,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(xr1,y,xr2,y);
            if (endLines) {
               if (braticks) {
                  xb[0] = xr2-tx; yb[0] = y-ty;
                  xb[1] = xr2;    yb[1] = y-ty;
                  xb[2] = xr2;    yb[2] = y+ty;
                  xb[3] = xr2-tx; yb[3] = y+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(xr2,y-ty,xr2,y+ty);
               }
            }
         }
      }
      yup1 = y + s2y*cy;
      yup2 = gPad->YtoPad(theY[i] + ey);
      if (yup2 > gPad->GetUymax()) yup2 =  gPad->GetUymax();
      if (yup2 > yup1) {
         if (arrowOpt) {
            arrow.PaintArrow(x,yup1,x,yup2,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(x,yup1,x,yup2);
            if (endLines) {
               if (braticks) {
                  xb[0] = x-tx; yb[0] = yup2-ty;
                  xb[1] = x-tx; yb[1] = yup2;
                  xb[2] = x+tx; yb[2] = yup2;
                  xb[3] = x+tx; yb[3] = yup2-ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(x-tx,yup2,x+tx,yup2);
               }
            }
         }
      }
      ylow1 = y - s2y*cy;
      ylow2 = gPad->YtoPad(theY[i] - ey);
      if (ylow2 < gPad->GetUymin()) ylow2 =  gPad->GetUymin();
      if (ylow2 < ylow1) {
         if (arrowOpt) {
            arrow.PaintArrow(x,ylow1,x,ylow2,asize,arrowOpt);
         } else {
            if (!brackets) gPad->PaintLine(x,ylow1,x,ylow2);
            if (endLines) {
               if (braticks) {
                  xb[0] = x-tx; yb[0] = ylow2+ty;
                  xb[1] = x-tx; yb[1] = ylow2;
                  xb[2] = x+tx; yb[2] = ylow2;
                  xb[3] = x+tx; yb[3] = ylow2+ty;
                  gPad->PaintPolyLine(4, xb, yb);
               } else {
                  gPad->PaintLine(x-tx,ylow2,x+tx,ylow2);
               }
            }
         }
      }
   }
   if (!brackets && !axis) PaintGraphSimple(theGraph, option);
   gPad->ResetBit(TGraph::kClipFrame);

   if (option3) {
      Int_t logx = gPad->GetLogx();
      Int_t logy = gPad->GetLogy();
      gPad->SetLogx(0);
      gPad->SetLogy(0);
      if (option4) PaintGraph(theGraph, 2*theNpoints, xline, yline,"FC");
      else         PaintGraph(theGraph, 2*theNpoints, xline, yline,"F");
      gPad->SetLogx(logx);
      gPad->SetLogy(logy);
      delete [] xline;
      delete [] yline;
   }
}


//______________________________________________________________________________
void TGraphPainter::PaintGraphPolar(TGraph *theGraph, Option_t* options)
{
   /* Begin_Html
   <a href="#GP04">Paint this TGraphPolar with its current attributes.</a>
   End_Html */

   Int_t ipt, i;
   Double_t rwrmin, rwrmax, rwtmin, rwtmax;

   TGraphPolar *theGraphPolar = (TGraphPolar*) theGraph;

   Int_t theNpoints  = theGraphPolar->GetN();
   Double_t *theX    = theGraphPolar->GetX();
   Double_t *theY    = theGraphPolar->GetY();
   Double_t *theEX   = theGraphPolar->GetEX();
   Double_t *theEY   = theGraphPolar->GetEY();

   if (theNpoints<1) return;
   TString opt = options;
   opt.ToUpper();

   Bool_t nolabel = kFALSE;
   if (opt.Contains("N")){
      nolabel = kTRUE;
      opt.ReplaceAll("N","");
   }

   TGraphPolargram *thePolargram = theGraphPolar->GetPolargram();

   // Check for existing TGraphPolargram in the Pad
   if (gPad) {
      // Existing polargram
      if (thePolargram) if (!gPad->FindObject(thePolargram->GetName())) thePolargram=0;
      if (!thePolargram) {
         // Find any other Polargram in the Pad
         TListIter padObjIter(gPad->GetListOfPrimitives());
         while (TObject* AnyObj = padObjIter.Next()) {
            if (TString(AnyObj->ClassName()).CompareTo("TGraphPolargram",
                                                      TString::kExact)==0)
            thePolargram = (TGraphPolargram*)AnyObj;
            theGraphPolar->SetPolargram(thePolargram);
         }
      }
   }

   // Get new polargram range if necessary.
   if (!thePolargram) {
      // Get range, initialize with first/last value
      rwrmin = theY[0]; rwrmax = theY[theNpoints-1];
      rwtmin = theX[0]; rwtmax = theX[theNpoints-1];

      for (ipt = 0; ipt < theNpoints; ipt++) {
         // Check for errors if available
         if (theEX) {
            if (theX[ipt] -theEX[ipt] < rwtmin) rwtmin = theX[ipt]-theEX[ipt];
            if (theX[ipt] +theEX[ipt] > rwtmax) rwtmax = theX[ipt]+theEX[ipt];
         } else {
            if (theX[ipt] < rwtmin) rwtmin=theX[ipt];
            if (theX[ipt] > rwtmax) rwtmax=theX[ipt];
         }
         if (theEY) {
            if (theY[ipt] -theEY[ipt] < rwrmin) rwrmin = theY[ipt]-theEY[ipt];
            if (theY[ipt] +theEY[ipt] > rwrmax) rwrmax = theY[ipt]+theEY[ipt];
         } else {
            if (theY[ipt] < rwrmin) rwrmin=theY[ipt];
            if (theY[ipt] > rwrmax) rwrmax=theY[ipt];
         }
      }
      // Add radial and Polar margins.
      if (rwrmin == rwrmax) rwrmax += 1.;
      if (rwtmin == rwtmax) rwtmax += 1.;
      Double_t dr = (rwrmax-rwrmin);
      Double_t dt = (rwtmax-rwtmin);
      rwrmax += 0.1*dr;
      rwrmin -= 0.1*dr;

      // Assume equaly spaced points for full 2*Pi.
      rwtmax += dt/theNpoints;
   } else {
      rwrmin = thePolargram->GetRMin();
      rwrmax = thePolargram->GetRMax();
      rwtmin = thePolargram->GetTMin();
      rwtmax = thePolargram->GetTMax();
   }

   if ((!thePolargram) || theGraphPolar->GetOptionAxis()) {
      // Draw Polar coord system
      thePolargram = new TGraphPolargram("Polargram",rwrmin,rwrmax,rwtmin,rwtmax);
      theGraphPolar->SetPolargram(thePolargram);
      if (opt.Contains("O")) thePolargram->SetBit(TGraphPolargram::kLabelOrtho);
      else thePolargram->ResetBit(TGraphPolargram::kLabelOrtho);
      if (nolabel) thePolargram->Draw("N");
      else         thePolargram->Draw("");
      theGraphPolar->SetOptionAxis(kFALSE);   //Prevent redrawing
   }

   // Convert points to polar.
   Double_t *theXpol = theGraphPolar->GetXpol();
   Double_t *theYpol = theGraphPolar->GetYpol();

   // Project theta in [0,2*Pi] and radius in [0,1].
   Double_t radiusNDC = rwrmax-rwrmin;
   Double_t thetaNDC  = (rwtmax-rwtmin)/(2*TMath::Pi());

   // Draw the error bars.
   // Y errors are lines, but X errors are pieces of circles.
   if (opt.Contains("E")) {
      if (theEY) {
         for (i=0; i<theNpoints; i++) {
            Double_t eymin, eymax, exmin,exmax;
            exmin = (theY[i]-theEY[i]-rwrmin)/radiusNDC*
                     TMath::Cos((theX[i]-rwtmin)/thetaNDC);
            eymin = (theY[i]-theEY[i]-rwrmin)/radiusNDC*
                     TMath::Sin((theX[i]-rwtmin)/thetaNDC);
            exmax = (theY[i]+theEY[i]-rwrmin)/radiusNDC*
                     TMath::Cos((theX[i]-rwtmin)/thetaNDC);
            eymax = (theY[i]+theEY[i]-rwrmin)/radiusNDC*
                     TMath::Sin((theX[i]-rwtmin)/thetaNDC);
            theGraphPolar->TAttLine::Modify();
            if (exmin != exmax || eymin != eymax) gPad->PaintLine(exmin,eymin,exmax,eymax);
         }
      }
      if (theEX) {
         for (i=0; i<theNpoints; i++) {
            Double_t rad = (theY[i]-rwrmin)/radiusNDC;
            Double_t phimin = (theX[i]-theEX[i]-rwtmin)/thetaNDC*180/TMath::Pi();
            Double_t phimax = (theX[i]+theEX[i]-rwtmin)/thetaNDC*180/TMath::Pi();
            theGraphPolar->TAttLine::Modify();
            if (phimin != phimax) thePolargram->PaintCircle(0,0,rad,phimin,phimax,0);
         }
      }
   }

   // Draw the graph itself.
   if (!(gPad->GetLogx()) && !(gPad->GetLogy())) {
      Double_t a, b, c=1, x1, x2, y1, y2, discr, norm1, norm2, xts, yts;
      Bool_t previouspointin = kFALSE;
      Double_t norm = 0;
      Double_t xt   = 0;
      Double_t yt   = 0 ;
      Int_t j       = -1;
      for (i=0; i<theNpoints; i++) {
         if (thePolargram->IsRadian()) {c=1;}
         if (thePolargram->IsDegree()) {c=180/TMath::Pi();}
         if (thePolargram->IsGrad())   {c=100/TMath::Pi();}
         xts  = xt;
         yts  = yt;
         xt   = (theY[i]-rwrmin)/radiusNDC*TMath::Cos(c*(theX[i]-rwtmin)/thetaNDC);
         yt   = (theY[i]-rwrmin)/radiusNDC*TMath::Sin(c*(theX[i]-rwtmin)/thetaNDC);
         norm = sqrt(xt*xt+yt*yt);
         // Check if points are in the main circle.
         if ( norm <= 1) {
            // We check that the previous point was in the circle too.
            // We record new point position.
            if (!previouspointin) {
               j++;
               theXpol[j] = xt;
               theYpol[j] = yt;
            } else {
               a = (yt-yts)/(xt-xts);
               b = yts-a*xts;
               discr = 4*(a*a-b*b+1);
               x1 = (-2*a*b+sqrt(discr))/(2*(a*a+1));
               x2 = (-2*a*b-sqrt(discr))/(2*(a*a+1));
               y1 = a*x1+b;
               y2 = a*x2+b;
               norm1 = sqrt((x1-xt)*(x1-xt)+(y1-yt)*(y1-yt));
               norm2 = sqrt((x2-xt)*(x2-xt)+(y2-yt)*(y2-yt));
               previouspointin = kFALSE;
               j = 0;
               if (norm1 < norm2) {
                  theXpol[j] = x1;
                  theYpol[j] = y1;
               } else {
                  theXpol[j] = x2;
                  theYpol[j] = y2;
               }
               j++;
               theXpol[j] = xt;
               theYpol[j] = yt;
               PaintGraph(theGraphPolar, j+1, theXpol, theYpol, opt);
            }
         } else {
            // We check that the previous point was in the circle.
            // We record new point position
            if (j>=1 && !previouspointin) {
               a = (yt-theYpol[j])/(xt-theXpol[j]);
               b = theYpol[j]-a*theXpol[j];
               previouspointin = kTRUE;
               discr = 4*(a*a-b*b+1);
               x1 = (-2*a*b+sqrt(discr))/(2*(a*a+1));
               x2 = (-2*a*b-sqrt(discr))/(2*(a*a+1));
               y1 = a*x1+b;
               y2 = a*x2+b;
               norm1 = sqrt((x1-xt)*(x1-xt)+(y1-yt)*(y1-yt));
               norm2 = sqrt((x2-xt)*(x2-xt)+(y2-yt)*(y2-yt));
               j++;
               if (norm1 < norm2) {
                  theXpol[j] = x1;
                  theYpol[j] = y1;
               } else {
                  theXpol[j] = x2;
                  theYpol[j] = y2;
               }
               PaintGraph(theGraphPolar, j+1, theXpol, theYpol, opt);
            }
            j=-1;
         }
      }
      if (j>=1) {
         // If the last point is in the circle, we draw the last serie of point.
         PaintGraph(theGraphPolar, j+1, theXpol, theYpol, opt);
      }
   } else {
      for (i=0; i<theNpoints; i++) {
         theXpol[i] = TMath::Abs((theY[i]-rwrmin)/radiusNDC*TMath::Cos((theX[i]-rwtmin)/thetaNDC)+1);
         theYpol[i] = TMath::Abs((theY[i]-rwrmin)/radiusNDC*TMath::Sin((theX[i]-rwtmin)/thetaNDC)+1);
      }
      PaintGraph(theGraphPolar, theNpoints, theXpol, theYpol,opt);
   }

   // Paint the title.

   if (TestBit(TH1::kNoTitle)) return;
   Int_t nt = strlen(theGraph->GetTitle());
   TPaveText *title = 0;
   TObject *obj;
   TIter next(gPad->GetListOfPrimitives());
   while ((obj = next())) {
      if (!obj->InheritsFrom(TPaveText::Class())) continue;
      title = (TPaveText*)obj;
      if (title->GetName())
         if (strcmp(title->GetName(),"title")) {title = 0; continue;}
      break;
   }
   if (nt == 0 || gStyle->GetOptTitle() <= 0) {
      if (title) delete title;
      return;
   }
   Double_t ht = gStyle->GetTitleH();
   Double_t wt = gStyle->GetTitleW();
   if (ht <= 0) ht = 1.1*gStyle->GetTitleFontSize();
   if (ht <= 0) ht = 0.05;
   if (wt <= 0) {
      TLatex l;
      l.SetTextSize(ht);
      l.SetTitle(theGraph->GetTitle());
      // Adjustment in case the title has several lines (#splitline)
      ht = TMath::Max(ht, 1.2*l.GetYsize()/(gPad->GetY2() - gPad->GetY1()));
      Double_t wndc = l.GetXsize()/(gPad->GetX2() - gPad->GetX1());
      wt = TMath::Min(0.7, 0.02+wndc);
   }
   if (title) {
      TText *t0 = (TText*)title->GetLine(0);
      if (t0) {
         if (!strcmp(t0->GetTitle(),theGraph->GetTitle())) return;
         t0->SetTitle(theGraph->GetTitle());
         if (wt > 0) title->SetX2NDC(title->GetX1NDC()+wt);
      }
      return;
   }

   Int_t talh = gStyle->GetTitleAlign()/10;
   if (talh < 1) talh = 1; if (talh > 3) talh = 3;
   Int_t talv = gStyle->GetTitleAlign()%10;
   if (talv < 1) talv = 1; if (talv > 3) talv = 3;

   Double_t xpos, ypos;
   xpos = gStyle->GetTitleX();
   ypos = gStyle->GetTitleY();

   if (talh == 2) xpos = xpos-wt/2.;
   if (talh == 3) xpos = xpos-wt;
   if (talv == 2) ypos = ypos+ht/2.;
   if (talv == 1) ypos = ypos+ht;

   TPaveText *ptitle = new TPaveText(xpos, ypos-ht, xpos+wt, ypos,"blNDC");

   // Box with the histogram title.
   ptitle->SetFillColor(gStyle->GetTitleFillColor());
   ptitle->SetFillStyle(gStyle->GetTitleStyle());
   ptitle->SetName("title");
   ptitle->SetBorderSize(gStyle->GetTitleBorderSize());
   ptitle->SetTextColor(gStyle->GetTitleTextColor());
   ptitle->SetTextFont(gStyle->GetTitleFont(""));
   if (gStyle->GetTitleFont("")%10 > 2)
   ptitle->SetTextSize(gStyle->GetTitleFontSize());
   ptitle->AddText(theGraph->GetTitle());
   ptitle->SetBit(kCanDelete);
   ptitle->Draw();
   ptitle->Paint();
}


//______________________________________________________________________________
void TGraphPainter::PaintGraphQQ(TGraph *theGraph, Option_t *option)
{
   /* Begin_Html
   Paint this graphQQ. No options for the time being.
   End_Html */

   TGraphQQ *theGraphQQ = (TGraphQQ*) theGraph;

   Double_t *theX    = theGraphQQ->GetX();
   Double_t  theXq1  = theGraphQQ->GetXq1();
   Double_t  theXq2  = theGraphQQ->GetXq2();
   Double_t  theYq1  = theGraphQQ->GetYq1();
   Double_t  theYq2  = theGraphQQ->GetYq2();
   TF1      *theF    = theGraphQQ->GetF();

   if (!theX){
      Error("TGraphQQ::Paint", "2nd dataset or theoretical function not specified");
      return;
   }

   if (theF){
      theGraphQQ->GetXaxis()->SetTitle("theoretical quantiles");
      theGraphQQ->GetYaxis()->SetTitle("data quantiles");
   }

   PaintGraphSimple(theGraph,option);

   Double_t xmin = gPad->GetUxmin();
   Double_t xmax = gPad->GetUxmax();
   Double_t ymin = gPad->GetUymin();
   Double_t ymax = gPad->GetUymax();
   Double_t yxmin, xymin, yxmax, xymax;
   Double_t xqmin = TMath::Max(xmin, theXq1);
   Double_t xqmax = TMath::Min(xmax, theXq2);
   Double_t yqmin = TMath::Max(ymin, theYq1);
   Double_t yqmax = TMath::Min(ymax, theYq2);

   TLine line1, line2, line3;
   line1.SetLineStyle(2);
   line3.SetLineStyle(2);
   yxmin = (theYq2-theYq1)*(xmin-theXq1)/(theXq2-theXq1) + theYq1;
   if (yxmin < ymin){
      xymin = (theXq2-theXq1)*(ymin-theYq1)/(theYq2-theYq1) + theXq1;
      line1.PaintLine(xymin, ymin, xqmin, yqmin);
   }
   else
      line1.PaintLine(xmin, yxmin, xqmin, yqmin);

   line2.PaintLine(xqmin, yqmin, xqmax, yqmax);

   yxmax = (theYq2-theYq1)*(xmax-theXq1)/(theXq2-theXq1) + theYq1;
   if (yxmax > ymax){
      xymax = (theXq2-theXq1)*(ymax-theYq1)/(theYq2-theYq1) + theXq1;
      line3.PaintLine(xqmax, yqmax, xymax, ymax);
   }
   else
      line3.PaintLine(xqmax, yqmax, xmax, yxmax);
}


//______________________________________________________________________________
void TGraphPainter::PaintGraphSimple(TGraph *theGraph, Option_t *option)
{
   /* Begin_Html
   Paint a simple graph, without errors bars.
   End_Html */

   if (strstr(option,"H") || strstr(option,"h")) {
      PaintGrapHist(theGraph, theGraph->GetN(), theGraph->GetX(), theGraph->GetY(), option);
   } else {
      PaintGraph(theGraph, theGraph->GetN(), theGraph->GetX(), theGraph->GetY(), option);
   }

   // Paint associated objects in the list of functions (for instance
   // the fit function).
   TList *functions = theGraph->GetListOfFunctions();
   if (!functions) return;
   TObjOptLink *lnk = (TObjOptLink*)functions->FirstLink();
   TObject *obj;

   while (lnk) {
      obj = lnk->GetObject();
      TVirtualPad *padsave = gPad;
      if (obj->InheritsFrom(TF1::Class())) {
         if (obj->TestBit(TF1::kNotDraw) == 0) obj->Paint("lsame");
      } else  {
         obj->Paint(lnk->GetOption());
      }
      lnk = (TObjOptLink*)lnk->Next();
      padsave->cd();
   }
   return;
}


//______________________________________________________________________________
void TGraphPainter::PaintPolyLineHatches(TGraph *theGraph, Int_t n, const Double_t *x, const Double_t *y)
{
   /* Begin_Html
   Paint a polyline with hatches on one side showing an exclusion zone. x and y
   are the the vectors holding the polyline and n the number of points in the
   polyline and <tt>w</tt> the width of the hatches. <tt>w</tt> can be negative.
   This method is not meant to be used directly. It is called automatically
   according to the line style convention.
   End_Html */

   Int_t i,j,nf;
   Double_t w = (theGraph->GetLineWidth()/100)*0.005;

   Double_t *xf = new Double_t[2*n];
   Double_t *yf = new Double_t[2*n];
   Double_t *xt = new Double_t[n];
   Double_t *yt = new Double_t[n];
   Double_t x1, x2, y1, y2, x3, y3, xm, ym, a, a1, a2, a3;

   // Compute the gPad coordinates in TRUE normalized space (NDC)
   Int_t ix1,iy1,ix2,iy2;
   Int_t iw = gPad->GetWw();
   Int_t ih = gPad->GetWh();
   Double_t x1p,y1p,x2p,y2p;
   gPad->GetPadPar(x1p,y1p,x2p,y2p);
   ix1 = (Int_t)(iw*x1p);
   iy1 = (Int_t)(ih*y1p);
   ix2 = (Int_t)(iw*x2p);
   iy2 = (Int_t)(ih*y2p);
   Double_t wndc  = TMath::Min(1.,(Double_t)iw/(Double_t)ih);
   Double_t hndc  = TMath::Min(1.,(Double_t)ih/(Double_t)iw);
   Double_t rh    = hndc/(Double_t)ih;
   Double_t rw    = wndc/(Double_t)iw;
   Double_t x1ndc = (Double_t)ix1*rw;
   Double_t y1ndc = (Double_t)iy1*rh;
   Double_t x2ndc = (Double_t)ix2*rw;
   Double_t y2ndc = (Double_t)iy2*rh;

   // Ratios to convert user space in TRUE normalized space (NDC)
   Double_t rx1,ry1,rx2,ry2;
   gPad->GetRange(rx1,ry1,rx2,ry2);
   Double_t rx = (x2ndc-x1ndc)/(rx2-rx1);
   Double_t ry = (y2ndc-y1ndc)/(ry2-ry1);

   // The first part of the filled area is made of the graph points.
   // Make sure that two adjacent points are different.
   xf[0] = rx*(x[0]-rx1)+x1ndc;
   yf[0] = ry*(y[0]-ry1)+y1ndc;
   nf = 0;
   for (i=1; i<n; i++) {
      if (x[i]==x[i-1] && y[i]==y[i-1]) continue;
      nf++;
      xf[nf] = rx*(x[i]-rx1)+x1ndc;
      if (xf[i]==xf[i-1]) xf[i] += 0.000001; // add an epsilon to avoid exact vertical lines.
      yf[nf] = ry*(y[i]-ry1)+y1ndc;
   }

   // For each graph points a shifted points is computed to build up
   // the second part of the filled area. First and last points are
   // treated as special cases, outside of the loop.
   if (xf[1]==xf[0]) {
      a = TMath::PiOver2();
   } else {
      a = TMath::ATan((yf[1]-yf[0])/(xf[1]-xf[0]));
   }
   if (xf[0]<=xf[1]) {
      xt[0] = xf[0]-w*TMath::Sin(a);
      yt[0] = yf[0]+w*TMath::Cos(a);
   } else {
      xt[0] = xf[0]+w*TMath::Sin(a);
      yt[0] = yf[0]-w*TMath::Cos(a);
   }

   if (xf[nf]==xf[nf-1]) {
      a = TMath::PiOver2();
   } else {
      a = TMath::ATan((yf[nf]-yf[nf-1])/(xf[nf]-xf[nf-1]));
   }
   if (xf[nf]>=xf[nf-1]) {
      xt[nf] = xf[nf]-w*TMath::Sin(a);
      yt[nf] = yf[nf]+w*TMath::Cos(a);
   } else {
      xt[nf] = xf[nf]+w*TMath::Sin(a);
      yt[nf] = yf[nf]-w*TMath::Cos(a);
   }

   Double_t xi0,yi0,xi1,yi1,xi2,yi2;
   for (i=1; i<nf; i++) {
      xi0 = xf[i];
      yi0 = yf[i];
      xi1 = xf[i+1];
      yi1 = yf[i+1];
      xi2 = xf[i-1];
      yi2 = yf[i-1];
      if (xi1==xi0) {
         a1 = TMath::PiOver2();
      } else {
         a1  = TMath::ATan((yi1-yi0)/(xi1-xi0));
      }
      if (xi1<xi0) a1 = a1+3.14159;
      if (xi2==xi0) {
         a2 = TMath::PiOver2();
      } else {
         a2  = TMath::ATan((yi0-yi2)/(xi0-xi2));
      }
      if (xi0<xi2) a2 = a2+3.14159;
      x1 = xi0-w*TMath::Sin(a1);
      y1 = yi0+w*TMath::Cos(a1);
      x2 = xi0-w*TMath::Sin(a2);
      y2 = yi0+w*TMath::Cos(a2);
      xm = (x1+x2)*0.5;
      ym = (y1+y2)*0.5;
      if (xm==xi0) {
         a3 = TMath::PiOver2();
      } else {
         a3 = TMath::ATan((ym-yi0)/(xm-xi0));
      }
      x3 = xi0-w*TMath::Sin(a3+1.57079);
      y3 = yi0+w*TMath::Cos(a3+1.57079);
      // Rotate (x3,y3) by PI around (xi0,yi0) if it is not on the (xm,ym) side.
      if ((xm-xi0)*(x3-xi0)<0 && (ym-yi0)*(y3-yi0)<0) {
         x3 = 2*xi0-x3;
         y3 = 2*yi0-y3;
      }
      if ((xm==x1) && (ym==y1)) {
         x3 = xm;
         y3 = ym;
      }
      xt[i] = x3;
      yt[i] = y3;
   }

   // Close the polygon if the first and last points are the same
   if (xf[nf]==xf[0] && yf[nf]==yf[0]) {
      xm = (xt[nf]+xt[0])*0.5;
      ym = (yt[nf]+yt[0])*0.5;
      if (xm==xf[0]) {
         a3 = TMath::PiOver2();
      } else {
         a3 = TMath::ATan((ym-yf[0])/(xm-xf[0]));
      }
      x3 = xf[0]+w*TMath::Sin(a3+1.57079);
      y3 = yf[0]-w*TMath::Cos(a3+1.57079);
      if ((xm-xf[0])*(x3-xf[0])<0 && (ym-yf[0])*(y3-yf[0])<0) {
         x3 = 2*xf[0]-x3;
         y3 = 2*yf[0]-y3;
      }
      xt[nf] = x3;
      xt[0]  = x3;
      yt[nf] = y3;
      yt[0]  = y3;
   }

   // Find the crossing segments and remove the useless ones
   Double_t xc, yc, c1, b1, c2, b2;
   Bool_t cross = kFALSE;
   Int_t nf2 = nf;
   for (i=nf2; i>0; i--) {
      for (j=i-1; j>0; j--) {
         if (xt[i-1]==xt[i] || xt[j-1]==xt[j]) continue;
         c1  = (yt[i-1]-yt[i])/(xt[i-1]-xt[i]);
         b1  = yt[i]-c1*xt[i];
         c2  = (yt[j-1]-yt[j])/(xt[j-1]-xt[j]);
         b2  = yt[j]-c2*xt[j];
         if (c1 != c2) {
            xc = (b2-b1)/(c1-c2);
            yc = c1*xc+b1;
            if (xc>TMath::Min(xt[i],xt[i-1]) && xc<TMath::Max(xt[i],xt[i-1]) &&
                xc>TMath::Min(xt[j],xt[j-1]) && xc<TMath::Max(xt[j],xt[j-1]) &&
                yc>TMath::Min(yt[i],yt[i-1]) && yc<TMath::Max(yt[i],yt[i-1]) &&
                yc>TMath::Min(yt[j],yt[j-1]) && yc<TMath::Max(yt[j],yt[j-1])) {
               nf++; xf[nf] = xt[i]; yf[nf] = yt[i];
               nf++; xf[nf] = xc   ; yf[nf] = yc;
               i = j;
               cross = kTRUE;
               break;
            } else {
               continue;
            }
         } else {
            continue;
         }
      }
      if (!cross) {
         nf++;
         xf[nf] = xt[i];
         yf[nf] = yt[i];
      }
      cross = kFALSE;
   }
   nf++; xf[nf] = xt[0]; yf[nf] = yt[0];

   // NDC to user coordinates
   for (i=0; i<nf+1; i++) {
      xf[i] = (1/rx)*(xf[i]-x1ndc)+rx1;
      yf[i] = (1/ry)*(yf[i]-y1ndc)+ry1;
   }

   // Draw filled area
   gPad->PaintFillArea(nf+1,xf,yf);
   theGraph->TAttLine::Modify(); // In case of PaintFillAreaHatches

   delete [] xf;
   delete [] yf;
   delete [] xt;
   delete [] yt;
}


//______________________________________________________________________________
void TGraphPainter::PaintStats(TGraph *theGraph, TF1 *fit)
{
   /* Begin_Html
   Paint the statistics box with the fit info.
   End_Html */

   Int_t dofit;
   TPaveStats *stats  = 0;
   TList *functions = theGraph->GetListOfFunctions();
   TIter next(functions);
   TObject *obj;
   while ((obj = next())) {
      if (obj->InheritsFrom(TPaveStats::Class())) {
         stats = (TPaveStats*)obj;
         break;
      }
   }

   if (stats) dofit  = stats->GetOptFit();
   else       dofit  = gStyle->GetOptFit();

   if (!dofit) fit = 0;
   if (!fit) return;
   if (dofit  == 1) dofit  =  111;
   Int_t nlines = 0;
   Int_t print_fval    = dofit%10;
   Int_t print_ferrors = (dofit/10)%10;
   Int_t print_fchi2   = (dofit/100)%10;
   Int_t print_fprob   = (dofit/1000)%10;
   Int_t nlinesf = print_fval + print_fchi2 + print_fprob;
   if (fit) nlinesf += fit->GetNpar();
   Bool_t done = kFALSE;
   Double_t  statw  = 1.8*gStyle->GetStatW();
   Double_t  stath  = 0.25*(nlines+nlinesf)*gStyle->GetStatH();
   if (stats) {
      stats->Clear();
      done = kTRUE;
   } else {
      stats  = new TPaveStats(
               gStyle->GetStatX()-statw,
               gStyle->GetStatY()-stath,
               gStyle->GetStatX(),
               gStyle->GetStatY(),"brNDC");

      stats->SetParent(functions);
      stats->SetOptFit(dofit);
      stats->SetOptStat(0);
      stats->SetFillColor(gStyle->GetStatColor());
      stats->SetFillStyle(gStyle->GetStatStyle());
      stats->SetBorderSize(gStyle->GetStatBorderSize());
      stats->SetTextFont(gStyle->GetStatFont());
      if (gStyle->GetStatFont()%10 > 2)
         stats->SetTextSize(gStyle->GetStatFontSize());
      stats->SetFitFormat(gStyle->GetFitFormat());
      stats->SetStatFormat(gStyle->GetStatFormat());
      stats->SetName("stats");

      stats->SetTextColor(gStyle->GetStatTextColor());
      stats->SetTextAlign(12);
      stats->SetBit(kCanDelete);
      stats->SetBit(kMustCleanup);
   }

   char t[64];
   char textstats[50];
   Int_t ndf = fit->GetNDF();
   snprintf(textstats,50,"#chi^{2} / ndf = %s%s / %d","%",stats->GetFitFormat(),ndf);
   snprintf(t,64,textstats,(Float_t)fit->GetChisquare());
   if (print_fchi2) stats->AddText(t);
   if (print_fprob) {
      snprintf(textstats,50,"Prob  = %s%s","%",stats->GetFitFormat());
      snprintf(t,64,textstats,(Float_t)TMath::Prob(fit->GetChisquare(),ndf));
      stats->AddText(t);
   }
   if (print_fval || print_ferrors) {
      for (Int_t ipar=0;ipar<fit->GetNpar();ipar++) {
         if (print_ferrors) {
            snprintf(textstats,50,"%-8s = %s%s #pm %s%s ",fit->GetParName(ipar),"%",stats->GetFitFormat(),"%",stats->GetFitFormat());
            snprintf(t,64,textstats,(Float_t)fit->GetParameter(ipar)
                            ,(Float_t)fit->GetParError(ipar));
         } else {
            snprintf(textstats,50,"%-8s = %s%s ",fit->GetParName(ipar),"%",stats->GetFitFormat());
            snprintf(t,64,textstats,(Float_t)fit->GetParameter(ipar));
         }
         t[63] = 0;
         stats->AddText(t);
      }
   }

   if (!done) functions->Add(stats);
   stats->Paint();
}


//______________________________________________________________________________
void TGraphPainter::Smooth(TGraph *theGraph, Int_t npoints, Double_t *x, Double_t *y, Int_t drawtype)
{
   /* Begin_Html
   Smooth a curve given by N points.
   <p>
   The original code is from an underlaying routine for Draw based on the
   CERN GD3 routine TVIPTE:
   <pre>
        Author - Marlow etc.   Modified by - P. Ward     Date -  3.10.1973
   </pre>
   This method draws a smooth tangentially continuous curve through
   the sequence of data points P(I) I=1,N where P(I)=(X(I),Y(I)).
   The curve is approximated by a polygonal arc of short vectors.
   The data points can represent open curves, P(1) != P(N) or closed
   curves P(2) == P(N). If a tangential discontinuity at P(I) is
   required, then set P(I)=P(I+1). Loops are also allowed.
   <p>
   Reference Marlow and Powell, Harwell report No.R.7092.1972
   MCCONALOGUE, Computer Journal VOL.13, NO4, NOV1970P p392 6
   <p>
   <ul>
   <li>  npoints   : Number of data points.
   <li>  x         : Abscissa
   <li>  y         : Ordinate
   </ul>
   End_Html */

   Int_t i, k, kp, km, npointsMax, banksize, n2, npt;
   Int_t maxiterations, finished;
   Int_t jtype, ktype, closed;
   Double_t sxmin, sxmax, symin, symax;
   Double_t delta;
   Double_t xorg, yorg;
   Double_t ratio_signs, xratio, yratio;
   Int_t flgic, flgis;
   Int_t iw, loptx;
   Double_t p1, p2, p3, p4, p5, p6;
   Double_t w1, w2, w3;
   Double_t a, b, c, r, s, t, z;
   Double_t co, so, ct, st, ctu, stu, xnt;
   Double_t dx1, dy1, dx2, dy2, dk1, dk2;
   Double_t xo, yo, dx, dy, xt, yt;
   Double_t xa, xb, ya, yb;
   Double_t u1, u2, u3, tj;
   Double_t cc, err;
   Double_t sb, sth;
   Double_t wsign, tsquare, tcube;
   c = t = co = so = ct = st = ctu = stu = dx1 = dy1 = dx2 = dy2 = 0;
   xt = yt = xa = xb = ya = yb = u1 = u2 = u3 = tj = sb = 0;

   npointsMax  = npoints*10;
   n2          = npointsMax-2;
   banksize    = n2;

   Double_t *qlx = new Double_t[npointsMax];
   Double_t *qly = new Double_t[npointsMax];
   if (!qlx || !qly) {
      Error("Smooth", "not enough space in memory");
      return;
   }

   //  Decode the type of curve (drawtype).

   loptx = kFALSE;
   jtype  = (drawtype%1000)-10;
   if (jtype > 0) { ktype = jtype; loptx = kTRUE; }
   else             ktype = drawtype%1000;

   Double_t ruxmin = gPad->GetUxmin();
   Double_t ruymin = gPad->GetUymin();
   if (ktype == 3) {
      xorg = ruxmin;
      yorg = ruymin;
   } else {
      xorg = TMath::Max((Double_t)0,ruxmin);
      yorg = TMath::Min(TMath::Max((Double_t)0,ruymin),gPad->GetUymax());
   }

   // delta is the accuracy required in constructing the curve.
   // If it is zero then the routine calculates a value otherwise
   // it uses this value. (default is 0.0)

   delta         = 0.00055;
   maxiterations = 20;

   //       Scale data to the range 0-ratio_signs in X, 0-1 in Y
   //       where ratio_signs is the ratio between the number of changes
   //       of sign in Y divided by the number of changes of sign in X

   sxmin = x[0];
   sxmax = x[0];
   symin = y[0];
   symax = y[0];
   Double_t six   = 1;
   Double_t siy   = 1;
   for (i=1;i<npoints;i++) {
      if (i > 1) {
         if ((x[i]-x[i-1])*(x[i-1]-x[i-2]) < 0) six++;
         if ((y[i]-y[i-1])*(y[i-1]-y[i-2]) < 0) siy++;
      }
      if (x[i] < sxmin) sxmin = x[i];
      if (x[i] > sxmax) sxmax = x[i];
      if (y[i] < symin) symin = y[i];
      if (y[i] > symax) symax = y[i];
   }
   closed = 0;
   Double_t dx1n   = TMath::Abs(x[npoints-1]-x[0]);
   Double_t dy1n   = TMath::Abs(y[npoints-1]-y[0]);
   if (dx1n < 0.01*(sxmax-sxmin) && dy1n < 0.01*(symax-symin))  closed = 1;
   if (sxmin == sxmax) {
      xratio = 1;
   } else {
      if (six > 1) ratio_signs = siy/six;
      else         ratio_signs = 20;
      xratio = ratio_signs/(sxmax-sxmin);
   }
   if (symin == symax) yratio = 1;
   else                yratio = 1/(symax-symin);

   qlx[0] = x[0];
   qly[0] = y[0];
   for (i=0;i<npoints;i++) {
      x[i] = (x[i]-sxmin)*xratio;
      y[i] = (y[i]-symin)*yratio;
   }

   //          "finished" is minus one if we must draw a straight line from P(k-1)
   //          to P(k). "finished" is one if the last call to PaintPolyLine has < n2
   //          points. "finished" is zero otherwise. npt counts the X and Y
   //          coordinates in work . When npt=n2 a call to IPL is made.

   finished = 0;
   npt      = 1;
   k        = 1;

   //           Convert coordinates back to original system

   //           Separate the set of data points into arcs P(k-1),P(k).
   //           Calculate the direction cosines. first consider whether
   //           there is a continuous tangent at the endpoints.

   if (!closed) {
      if (x[0] != x[npoints-1] || y[0] != y[npoints-1]) goto L40;
      if (x[npoints-2] == x[npoints-1] && y[npoints-2] == y[npoints-1]) goto L40;
      if (x[0] == x[1] && y[0] == y[1]) goto L40;
   }
   flgic = kFALSE;
   flgis = kTRUE;

   //           flgic is true if the curve is open and false if it is closed.
   //           flgis is true in the main loop, but is false if there is
   //           a deviation from the main loop.

   km = npoints - 1;

   //           Calculate direction cosines at P(1) using P(N-1),P(1),P(2).

   goto L100;
L40:
   flgic = kTRUE;
   flgis = kFALSE;

   //           Skip excessive consecutive equal points.

L50:
   if (k >= npoints) {
      finished = 1;  // Prepare to clear out remaining short vectors before returning
      if (npt > 1) goto L310;
      goto L390;
   }
   k++;
   if (x[k-1] == x[k-2] && y[k-1] == y[k-2])  goto L50;
L60:
   km = k-1;
   if (k > npoints) {
      finished = 1;  // Prepare to clear out remaining short vectors before returning
      if (npt > 1) goto L310;
      goto L390;
   }
   if (k < npoints) goto L90;
   if (!flgic) { kp = 2; goto L130;}

L80:
   if (flgis) goto L150;

   //           Draw a straight line from P(k-1) to P(k).

   finished = -1;
   goto L170;

   //           Test whether P(k) is a cusp.

L90:
   if (x[k-1] == x[k] && y[k-1] == y[k]) goto L80;
L100:
   kp = k+1;
   goto L130;

   //           Branch if the next section of the curve begins at a cusp.

L110:
   if (!flgis) goto L50;

   //           Carry forward the direction cosines from the previous arc.

L120:
   co = ct;
   so = st;
   k++;
   goto L60;

   //           Calculate the direction cosines at P(k).  If k=1 then
   //           N-1 is used for k-1. If k=N then 2 is used for k+1.
   //           direction cosines at P(k) obtained from P(k-1),P(k),P(k+1).

L130:
   dx1 = x[k-1]  - x[km-1];
   dy1 = y[k-1]  - y[km-1];
   dk1 = dx1*dx1 + dy1*dy1;
   dx2 = x[kp-1] - x[k-1];
   dy2 = y[kp-1] - y[k-1];
   dk2 = dx2*dx2 + dy2*dy2;
   ctu = dx1*dk2 + dx2*dk1;
   stu = dy1*dk2 + dy2*dk1;
   xnt = ctu*ctu + stu*stu;

   //           If both ctu and stu are zero,then default.This can
   //           occur when P(k)=P(k+1). I.E. A loop.

   if (xnt < 1.E-25) {
      ctu = dy1;
      stu =-dx1;
      xnt = dk1;
   }
   //           Normalise direction cosines.

   ct = ctu/TMath::Sqrt(xnt);
   st = stu/TMath::Sqrt(xnt);
   if (flgis) goto L160;

   //           Direction cosines at P(k-1) obtained from P(k-1),P(k),P(k+1).

   w3    = 2*(dx1*dy2-dx2*dy1);
   co    = ctu+w3*dy1;
   so    = stu-w3*dx1;
   xnt   = 1/TMath::Sqrt(co*co+so*so);
   co    = co*xnt;
   so    = so*xnt;
   flgis = kTRUE;
   goto L170;

   //           Direction cosines at P(k) obtained from P(k-2),P(k-1),P(k).

L150:
   w3    = 2*(dx1*dy2-dx2*dy1);
   ct    = ctu-w3*dy2;
   st    = stu+w3*dx2;
   xnt   = 1/TMath::Sqrt(ct*ct+st*st);
   ct    = ct*xnt;
   st    = st*xnt;
   flgis = kFALSE;
   goto L170;
L160:
   if (k <= 1) goto L120;

   //           For the arc between P(k-1) and P(k) with direction cosines co,
   //           so and ct,st respectively, calculate the coefficients of the
   //           parametric cubic represented by X(t) and Y(t) where
   //           X(t)=xa*t**3 + xb*t**2 + co*t + xo
   //           Y(t)=ya*t**3 + yb*t**2 + so*t + yo

L170:
   xo = x[k-2];
   yo = y[k-2];
   dx = x[k-1] - xo;
   dy = y[k-1] - yo;

   //           Initialise the values of X(TI),Y(TI) in xt and yt respectively.

   xt = xo;
   yt = yo;
   if (finished < 0) {  // Draw a straight line between (xo,yo) and (xt,yt)
      xt += dx;
      yt += dy;
      goto L300;
   }
   c  = dx*dx+dy*dy;
   a  = co+ct;
   b  = so+st;
   r  = dx*a+dy*b;
   t  = c*6/(TMath::Sqrt(r*r+2*(7-co*ct-so*st)*c)+r);
   tsquare = t*t;
   tcube   = t*tsquare;
   xa = (a*t-2*dx)/tcube;
   xb = (3*dx-(co+a)*t)/tsquare;
   ya = (b*t-2*dy)/tcube;
   yb = (3*dy-(so+b)*t)/tsquare;

   //           If the curve is close to a straight line then use a straight
   //           line between (xo,yo) and (xt,yt).

   if (.75*TMath::Max(TMath::Abs(dx*so-dy*co),TMath::Abs(dx*st-dy*ct)) <= delta) {
      finished = -1;
      xt += dx;
      yt += dy;
      goto L300;
   }

   //           Calculate a set of values 0 == t(0).LTCT(1) <  ...  < t(M)=TC
   //           such that polygonal arc joining X(t(J)),Y(t(J)) (J=0,1,..M)
   //           is within the required accuracy of the curve

   tj = 0;
   u1 = ya*xb-yb*xa;
   u2 = yb*co-xb*so;
   u3 = so*xa-ya*co;

   //           Given t(J), calculate t(J+1). The values of X(t(J)),
   //           Y(t(J)) t(J) are contained in xt,yt and tj respectively.

L180:
   s  = t - tj;
   iw = -2;

   //           Define iw here later.

   p1 = (2*u1)*tj-u3;
   p2 = (u1*tj-u3)*3*tj+u2;
   p3 = 3*tj*ya+yb;
   p4 = (p3+yb)*tj+so;
   p5 = 3*tj*xa+xb;
   p6 = (p5+xb)*tj+co;

   //           Test D(tj,THETA). A is set to (Y(tj+s)-Y(tj))/s.b is
   //           set to (X(tj+s)-X(tj))/s.

   cc  = 0.8209285;
   err = 0.1209835;
L190:
   iw -= 2;
L200:
   a   = (s*ya+p3)*s+p4;
   b   = (s*xa+p5)*s+p6;

   //           Set z to PSI(D/delta)-cc.

   w1 = -s*(s*u1+p1);
   w2 = s*s*u1-p2;
   w3 = 1.5*w1+w2;

   //           Set the estimate of (THETA-tj)/s.Then set the numerator
   //           of the expression (EQUATION 4.4)/s. Then set the square
   //           of D(tj,tj+s)/delta. Then replace z by PSI(D/delta)-cc.

   if (w3 > 0) wsign = TMath::Abs(w1);
   else        wsign = -TMath::Abs(w1);
   sth = 0.5+wsign/(3.4*TMath::Abs(w1)+5.2*TMath::Abs(w3));
   z   = s*sth*(s-s*sth)*(w1*sth+w1+w2);
   z   = z*z/((a*a+b*b)*(delta*delta));
   z   = (z+2.642937)*z/((.3715652*z+3.063444)*z+.2441889)-cc;

   //           Branch if z has been calculated

   if (iw > 0) goto L250;
   if (z > err) goto L240;
   goto L220;
L210:
   iw -= 2;
L220:
   if (iw+2 == 0) goto L190;
   if (iw+2 >  0) goto L290;

   //           Last part of arc.

L230:
   xt = x[k-1];
   yt = y[k-1];
   s  = 0;
   goto L300;

   //           z(s). find a value of s where 0 <= s <= sb such that
   //           TMath::Abs(z(s)) < err

L240:
   kp = 0;
   c  = z;
   sb = s;
L250:
   theGraph->Zero(kp,0,sb,err,s,z,maxiterations);
   if (kp == 2) goto L210;
   if (kp > 2) {
      Error("Smooth", "Attempt to plot outside plot limits");
      goto L230;
   }
   if (iw > 0) goto L200;

   //           Set z=z(s) for s=0.

   if (iw < 0) {
      z  = -cc;
      iw = 0;
      goto L250;
   }

   //           Set z=z(s) for s=sb.

   z  = c;
   iw = 1;
   goto L250;

   //           Update tj,xt and yt.

L290:
   xt = xt + s*b;
   yt = yt + s*a;
   tj = s  + tj;

   //           Convert coordinates to original system

L300:
   qlx[npt] = sxmin + xt/xratio;
   qly[npt] = symin + yt/yratio;
   npt++;

   //           If a fill area must be drawn and if the banks LX and
   //           LY are too small they are enlarged in order to draw
   //           the filled area in one go.

   if (npt < banksize)  goto L320;
   if (drawtype >= 1000 || ktype > 1) {
      Int_t newsize = banksize + n2;
      Double_t *qtemp = new Double_t[banksize];
      for (i=0;i<banksize;i++) qtemp[i] = qlx[i];
      delete [] qlx;
      qlx = new Double_t[newsize];
      for (i=0;i<banksize;i++) qlx[i]   = qtemp[i];
      for (i=0;i<banksize;i++) qtemp[i] = qly[i];
      delete [] qly;
      qly = new Double_t[newsize];
      for (i=0;i<banksize;i++) qly[i] = qtemp[i];
      delete [] qtemp;
      banksize = newsize;
      goto L320;
   }

   //           Draw the graph

L310:
   if (drawtype >= 1000) {
      gPad->PaintFillArea(npt,qlx,qly, "B");
   } else {
      if (ktype > 1) {
         if (!loptx) {
            qlx[npt]   = qlx[npt-1];
            qlx[npt+1] = qlx[0];
            qly[npt]   = yorg;
            qly[npt+1] = yorg;
         } else {
            qlx[npt]   = xorg;
            qlx[npt+1] = xorg;
            qly[npt]   = qly[npt-1];
            qly[npt+1] = qly[0];
         }
         gPad->PaintFillArea(npt+2,qlx,qly);
      }
      if (TMath::Abs(theGraph->GetLineWidth())>99) PaintPolyLineHatches(theGraph, npt, qlx, qly);
      gPad->PaintPolyLine(npt,qlx,qly);
   }
   npt = 1;
   qlx[0] = sxmin + xt/xratio;
   qly[0] = symin + yt/yratio;
L320:
   if (finished > 0) goto L390;
   if (finished < 0) { finished = 0; goto L110;}
   if (s > 0) goto L180;
   goto L110;

   //           Convert coordinates back to original system

L390:
   for (i=0;i<npoints;i++) {
      x[i] = sxmin + x[i]/xratio;
      y[i] = symin + y[i]/yratio;
   }

   delete [] qlx;
   delete [] qly;
}
