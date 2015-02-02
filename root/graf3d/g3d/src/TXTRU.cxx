// @@(#)root/g3d:$Id$
// Author: Robert Hatcher (rhatcher@fnal.gov) 2000.09.06

#include "TXTRU.h"
#include "TVirtualPad.h"

///#include "GLConstants.h"

#include "TBuffer3D.h"
#include "TBuffer3DTypes.h"
#include "TGeometry.h"
#include "TMath.h"

#include "Riostream.h"


ClassImp(TXTRU)


//_____________________________________________________________________________
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/xtru.gif"> </P> End_Html
//
// XTRU is an poly-extrusion with fixed outline shape in x-y,
// a sequence of z extents (segments) and two end faces perpendicular
// to the z axis.  The x-y outline is defined by an ordered list of
// points; the overall scale of the outline scales linearly between
// z points and the center can have an x-y offset specified
// at each segment end.
//
// A TXTRU has the following parameters:
//
//     - name       name of the shape
//     - title      shape's title
//     - material  (see TMaterial)
//     - nxy        number of x-y vertex points constituting the outline --
//                  this number should be at least 3
//     - nz         number of planes perpendicular to the z axis where
//                  the scaling dimension of the section is given --
//                  this number should be at least 2
//     - Xvtx       array [nxy] of X coordinates of vertices
//     - Yvtx       array [nxy] of Y coordinates of vertices
//     - z          array [nz] of z plane positions
//     - scale      array [nz] of scale factors
//     - x0         array [nz] of x offsets
//     - y0         array [nz] of y offsets
//
// Author:  R. Hatcher 2000.04.21
//
// All XTRU shapes are correctly rendered in wire mode but can encounter
// difficulty when rendered as a solid with hidden surfaces.  These
// exceptions occur if the outline shape is not a convex polygon.
// Both the X3D and OpenGL renderers expect polygons to be convex.
// The OpenGL spec specifies that points defining a polygon using the
// GL_POLYGON primitive may be rendered as the convex hull of that set.
//
// Solid rendering under X3D can also give unexpected artifacts if
// the combination of x-y-z offsets and scales for the segments are
// chosen in such a manner that they represent a concave shape when
// sliced along a plane parallel to the z axis.
//
// Choosing sets of point that represent a malformed polygon is
// not supported, but testing for such a condition is not implemented
// and thus it is left to the user to avoid this mistake.
//
// Begin_Html <P ALIGN=CENTER> <IMG SRC="gif/polytype.gif"> </P> End_Html


//______________________________________________________________________________
TXTRU::TXTRU()
   : fNxy(0), fNxyAlloc(0), fNz(0), fNzAlloc(0), fXvtx(0), fYvtx(0),
     fZ(0), fScale(0), fX0(0), fY0(0)
{
   // TXTRU shape - default constructor

   fPolygonShape  = kUncheckedXY;
   fZOrdering     = kUncheckedZ;
   fSplitConcave  = kFALSE;
}


//______________________________________________________________________________
TXTRU::TXTRU(const char *name, const char *title, const char *material,
             Int_t nxy, Int_t nz)
   : TShape (name,title,material)
{
   // TXTRU shape - normal constructor
   //
   // Parameters of Nxy positions must be entered via TXTRU::DefineVertex
   // Parameters of Nz  positions must be entered via TXTRU::DefineSection

   // start in a known state even if "Error" is encountered
   fNxy      = 0;
   fNxyAlloc = 0;
   fNz       = 0;
   fNzAlloc  = 0;
   fXvtx     = 0;
   fYvtx     = 0;
   fZ        = 0;
   fScale    = 0;
   fX0       = 0;
   fY0       = 0;

   fPolygonShape  = kUncheckedXY;
   fZOrdering     = kUncheckedZ;
   fSplitConcave  = kFALSE;

   if ( nxy < 3 ) {
      Error(name,"number of x-y points for %s must be at least three!",name);
      return;
   }
   if ( nz < 2 ) {
      Error(name,"number of z points for %s must be at least two!",name);
      return;
   }

   // allocate space for Nxy vertex points
   fNxy       = nxy;
   fNxyAlloc  = nxy;
   fXvtx      = new Float_t [fNxyAlloc];
   fYvtx      = new Float_t [fNxyAlloc];
   // zero out the vertex points
   Int_t i = 0;
   for (i = 0; i < fNxyAlloc; i++) {
      fXvtx[i] = 0;
      fYvtx[i] = 0;
   }

   // allocate space for Nz sections
   fNz        = nz;
   fNzAlloc   = nz;
   fZ         = new Float_t [fNzAlloc];
   fScale     = new Float_t [fNzAlloc];
   fX0        = new Float_t [fNzAlloc];
   fY0        = new Float_t [fNzAlloc];
   // zero out the z points
   Int_t j = 0;
   for (j = 0; j < fNzAlloc; j++) {
      fZ[j]     = 0;
      fScale[j] = 0;
      fX0[j]    = 0;
      fY0[j]    = 0;
   }

}


//______________________________________________________________________________
TXTRU::TXTRU(const TXTRU &xtru) : TShape(xtru)
{
   // TXTRU copy constructor

   // patterned after other ROOT objects

   ((TXTRU&)xtru).Copy(*this);
}


//______________________________________________________________________________
TXTRU::~TXTRU()
{
   // TXTRU destructor deallocates arrays

   if (fXvtx) delete [] fXvtx;
   if (fYvtx) delete [] fYvtx;
   fXvtx     = 0;
   fYvtx     = 0;
   fNxy      = 0;
   fNxyAlloc = 0;

   if (fZ)     delete [] fZ;
   if (fScale) delete [] fScale;
   if (fX0)    delete [] fX0;
   if (fY0)    delete [] fY0;
   fZ        = 0;
   fScale    = 0;
   fX0       = 0;
   fY0       = 0;
   fNz       = 0;
   fNzAlloc  = 0;

   fPolygonShape  = kUncheckedXY;
   fZOrdering     = kUncheckedZ;
}


//______________________________________________________________________________
TXTRU& TXTRU::operator=(const TXTRU &rhs)
{
   // Deep assignment operator

   // protect against self-assignment
   if (this == &rhs) return *this;

   if (fNxyAlloc) {
      delete [] fXvtx;
      delete [] fYvtx;
   }
   if (fNzAlloc) {
      delete [] fZ;
      delete [] fScale;
      delete [] fX0;
      delete [] fY0;
   }
   ((TXTRU&)rhs).Copy(*this);

   return *this;
}


//______________________________________________________________________________
void TXTRU::Copy(TObject &obj) const
{
   // TXTRU Copy method

   // patterned after other ROOT objects

   TObject::Copy(obj);
   ((TXTRU&)obj).fNxy       = fNxy;
   ((TXTRU&)obj).fNxyAlloc  = fNxyAlloc;
   ((TXTRU&)obj).fXvtx = new Float_t [fNxyAlloc];
   ((TXTRU&)obj).fYvtx = new Float_t [fNxyAlloc];
   Int_t i = 0;
   for (i = 0; i < fNxyAlloc; i++) {
      ((TXTRU&)obj).fXvtx[i] = fXvtx[i];
      ((TXTRU&)obj).fYvtx[i] = fYvtx[i];
   }

   ((TXTRU&)obj).fNz       = fNz;
   ((TXTRU&)obj).fNzAlloc  = fNzAlloc;
   ((TXTRU&)obj).fZ     = new Float_t [fNzAlloc];
   ((TXTRU&)obj).fScale = new Float_t [fNzAlloc];
   ((TXTRU&)obj).fX0    = new Float_t [fNzAlloc];
   ((TXTRU&)obj).fY0    = new Float_t [fNzAlloc];
   Int_t j = 0;
   for (j = 0; j < fNzAlloc; j++) {
      ((TXTRU&)obj).fZ[j]     = fZ[j];
      ((TXTRU&)obj).fScale[j] = fScale[j];
      ((TXTRU&)obj).fX0[j]    = fX0[j];
      ((TXTRU&)obj).fY0[j]    = fY0[j];
   }

   ((TXTRU&)obj).fPolygonShape = fPolygonShape;
   ((TXTRU&)obj).fZOrdering    = fZOrdering;
}


//______________________________________________________________________________
void TXTRU::DefineSection(Int_t iz, Float_t z, Float_t scale, Float_t x0, Float_t y0)
{
   // Set z section iz information
   // expand size of array if necessary

   if (iz < 0) return;

   // setting a new section makes things unverified
   fZOrdering  = kUncheckedZ;

   if (iz >= fNzAlloc) {
      // re-allocate the z positions/scales
      Int_t   newNalloc = iz + 1;
      Float_t *newZ = new Float_t [newNalloc];
      Float_t *newS = new Float_t [newNalloc];
      Float_t *newX = new Float_t [newNalloc];
      Float_t *newY = new Float_t [newNalloc];
      Int_t i = 0;
      for (i = 0; i < newNalloc; i++) {
         if (i<fNz) {
            // copy the old points
            newZ[i] = fZ[i];
            newS[i] = fScale[i];
            newX[i] = fX0[i];
            newY[i] = fY0[i];
         } else {
            // zero out the new points
            newZ[i] = 0;
            newS[i] = 0;
            newX[i] = 0;
            newY[i] = 0;
         }
      }
      delete [] fZ;
      delete [] fScale;
      delete [] fX0;
      delete [] fY0;
      fZ     = newZ;
      fScale = newS;
      fX0    = newX;
      fY0    = newY;
      fNzAlloc = newNalloc;
   }

   // filled z "iz" means indices 0...iz have values -> iz+1 entries
   fNz = TMath::Max(iz+1,fNz);

   fZ[iz]     = z;
   fScale[iz] = scale;
   fX0[iz]    = x0;
   fY0[iz]    = y0;
}


//______________________________________________________________________________
void TXTRU::DefineVertex(Int_t ipt, Float_t x, Float_t y) {

   // Set vertex point ipt to (x,y)
   // expand size of array if necessary

   if (ipt < 0) return;

   // setting a new vertex makes things unverified
   fPolygonShape  = kUncheckedXY;

   if (ipt >= fNxyAlloc) {
      // re-allocate the outline points
      Int_t   newNalloc = ipt + 1;
      Float_t *newX = new Float_t [newNalloc];
      Float_t *newY = new Float_t [newNalloc];
      Int_t i = 0;
      for (i = 0; i < newNalloc; i++) {
         if (i<fNxy) {
            // copy the old points
            newX[i] = fXvtx[i];
            newY[i] = fYvtx[i];
         } else {
            // zero out the new points
            newX[i] = 0;
            newY[i] = 0;
         }
      }
      delete [] fXvtx;
      delete [] fYvtx;
      fXvtx = newX;
      fYvtx = newY;
      fNxyAlloc = newNalloc;
   }

   // filled point "ipt" means indices 0...ipt have values -> ipt+1 entries
   fNxy = TMath::Max(ipt+1,fNxy);

   fXvtx[ipt] = x;
   fYvtx[ipt] = y;
}


//______________________________________________________________________________
Int_t TXTRU::DistancetoPrimitive(Int_t px, Int_t py)
{
   // Compute the distance from point px,py to a TXTRU
   // by calculating the closest approach to each corner

   Int_t numPoints = fNz*fNxy;
   return ShapeDistancetoPrimitive(numPoints,px,py);
}


//______________________________________________________________________________
Float_t TXTRU::GetOutlinePointX(Int_t n) const {

   // Return x coordinate of a vertex point

   if ((n < 0) || (n >= fNxy)) {
      Error(fName,"no such point %d [of %d]",n,fNxy);
      return 0.0;
   }
   return fXvtx[n];
}


//______________________________________________________________________________
Float_t TXTRU::GetOutlinePointY(Int_t n) const {

   // Return y coordinate of a vertex point

   if ((n < 0) || (n >= fNxy)) {
      Error(fName,"no such point %d [of %d]",n,fNxy);
      return 0.0;
   }
   return fYvtx[n];
}


//______________________________________________________________________________
Float_t TXTRU::GetSectionX0(Int_t n) const {

   // Return x0 shift of a z section

   if ((n < 0) || (n >= fNz)) {
      Error(fName,"no such section %d [of %d]",n,fNz);
      return 0.0;
   }
   return fX0[n];
}


//______________________________________________________________________________
Float_t TXTRU::GetSectionY0(Int_t n) const {

   // Return y0 shift of a z section

   if ((n < 0) || (n >= fNz)) {
      Error(fName,"no such section %d [of %d]",n,fNz);
      return 0.0;
   }
   return fY0[n];
}


//______________________________________________________________________________
Float_t TXTRU::GetSectionScale(Int_t n) const {

   // Return scale factor for a z section

   if ((n < 0) || (n >= fNz)) {
      Error(fName,"no such section %d [of %d]",n,fNz);
      return 0.0;
   }
   return fScale[n];
}


//______________________________________________________________________________
Float_t TXTRU::GetSectionZ(Int_t n) const {

   // Return z of a z section

   if ((n < 0) || (n >= fNz)) {
      Error(fName,"no such section %d [of %d]",n,fNz);
      return 0.0;
   }
   return fZ[n];
}


//______________________________________________________________________________
void TXTRU::Print(Option_t *option) const
{
   // Dump the info of this TXTRU shape
   // Option: "xy" to get x-y information
   //         "z"  to get z information
   //         "alloc" to show full allocated arrays (not just used values)

   TString opt = option;
   opt.ToLower();

   printf("TXTRU %s Nxy=%d [of %d] Nz=%d [of %d] Option=%s\n",
          GetName(),fNxy,fNxyAlloc,fNz,fNzAlloc,option);

   const char *shape = 0;
   const char *zorder = 0;

   switch (fPolygonShape) {
   case kUncheckedXY:   shape = "Unchecked  ";  break;
   case kMalformedXY:   shape = "Malformed  ";  break;
   case kConvexCCW:     shape = "Convex CCW ";  break;
   case kConvexCW:      shape = "Convex CW  ";  break;
   case kConcaveCCW:    shape = "Concave CCW";  break;
   case kConcaveCW:     shape = "Concave CW ";  break;
   }

   switch (fZOrdering) {
   case kUncheckedZ:    zorder = "Unchecked Z";  break;
   case kMalformedZ:    zorder = "Malformed Z";  break;
   case kConvexIncZ:    zorder = "Convex Increasing Z";  break;
   case kConvexDecZ:    zorder = "Convex Decreasing Z";  break;
   case kConcaveIncZ:   zorder = "Concave Increasing Z";  break;
   case kConcaveDecZ:   zorder = "Concave Decreasing Z";  break;
   }

   printf("  XY shape '%s', '%s'\n",shape,zorder);

   Int_t       nxy, nz;

   if (opt.Contains("alloc")) {
      nxy    = fNxy;
      nz     = fNz;
   } else {
      nxy    = fNxyAlloc;
      nz    = fNzAlloc;
   }

   const char *name;
   Float_t *p;
   Int_t   nlimit;
   Bool_t  print_vtx = opt.Contains("xy");
   Bool_t  print_z   = opt.Contains("z");

   Int_t ixyz=0;
   for (ixyz=0; ixyz<6; ixyz++) {
      switch (ixyz) {
      case 0: p = fXvtx;  name = "x";     nlimit = nxy; break;
      case 1: p = fYvtx;  name = "y";     nlimit = nxy; break;
      case 2: p = fZ;     name = "z";     nlimit = nz;  break;
      case 3: p = fScale; name = "scale"; nlimit = nz;  break;
      case 4: p = fX0;    name = "x0";    nlimit = nz;  break;
      case 5: p = fY0;    name = "y0";    nlimit = nz;  break;
      default: continue;
      }
      if (ixyz<=1 && !print_vtx) continue;
      if (ixyz>=2 && !print_z) continue;

      printf(" Float_t %s[] = \n    { %10g",name,*p++);
      Int_t i=1;
      for (i=1;i<nlimit;i++) {
         printf(", %10g",*p++);
         if (i%6==5) printf("\n    ");
      }
      printf(" };\n");
   }

}


//______________________________________________________________________________
void TXTRU::SetPoints(Double_t *points) const
{
   // Create TXTRU points in buffer
   // order as expected by other methods (counterclockwise xy, increasing z)

   if (points) {
      Int_t ipt, ixy, iz, ioff;
      Float_t x, y;

      // put xy in counterclockwise order
      Bool_t iscw = (fPolygonShape == kConvexCW ||
                     fPolygonShape == kConcaveCW  );

      // put z
      Bool_t reversez = (fZOrdering == kConvexDecZ ||
                         fZOrdering == kConcaveDecZ  );

      ipt = 0; // point number
      Int_t i=0;
      for (i=0; i<fNz; i++) {        // loop over sections
         iz = (reversez) ? fNz-1 - i : i;
         Int_t j=0;
         for (j=0; j<fNxy; j++) {    // loop over points in section
            ixy = (iscw) ? fNxy-1 - j : j;
            ioff = ipt*3;                  // 3 words per point (x,y,z)
            x = fXvtx[ixy];
            y = fYvtx[ixy];
            points[ioff  ] = x*fScale[iz] + fX0[iz];
            points[ioff+1] = y*fScale[iz] + fY0[iz];
            points[ioff+2] = fZ[iz];
            ipt++;
         }
      }
   }
}


//______________________________________________________________________________
void TXTRU::Sizeof3D() const
{
   // Return total X3D needed by TNode::ls (when called with option "x")

   gSize3D.numPoints += fNz*fNxy;
   gSize3D.numSegs   += (2*fNz-1)*fNxy;
   gSize3D.numPolys  += (fNz-1)*fNxy+2;
}


//______________________________________________________________________________
void TXTRU::SplitConcavePolygon(Bool_t split)
{
   // (Dis)Enable the splitting of concave polygon outlines into
   // multiple convex polygons.  This would make for better rendering
   // in solid mode, but introduces extra, potentially confusing, lines
   // in wireframe mode.
   // *** Not yet implemented ***

   fSplitConcave = split;

   // Not implemented yet
   if (split) {
      fSplitConcave = kFALSE;
      cout << TNamed::GetName()
           << " TXTRU::SplitConcavePolygon is not yet implemented" << endl;
   }

}


//______________________________________________________________________________
void TXTRU::TruncateNxy(Int_t npts) {

   // Truncate the vertex list

   if ((npts < 0) || (npts > fNxy)) {
      Error(fName,"truncate to %d impossible on %d points",npts,fNxy);
      return;
   }
   fNxy = npts;
   return;
}


//______________________________________________________________________________
void TXTRU::TruncateNz(Int_t nz) {

   // Truncate the z section list

   if ((nz < 0) || (nz > fNz)) {
      Error(fName,"truncate to %d impossible on %d points",nz,fNz);
      return;
   }
   fNz = nz;
   return;
}


//______________________________________________________________________________
void TXTRU::CheckOrdering()
{
   // Determine ordering over which to process points, segments, surfaces
   // so that they render correctly.  Generally this has to do
   // with getting outward normals in the hidden/solid surface case.

   Float_t plus, minus, zero;

   // Check on polygon's shape
   // Convex vs. Concave and ClockWise vs. Counter-ClockWise
   plus = minus = zero = 0;
   Int_t ixy=0;
   for (ixy=0; ixy<fNxy; ixy++) {
      // calculate the cross product of the two segments that
      // meet at vertex "ixy"
      // concave polygons have a mixture of + and - values
      Int_t ixyprev = (ixy + fNxy - 1)%fNxy;
      Int_t ixynext = (ixy + fNxy + 1)%fNxy;

      Float_t dxprev = fXvtx[ixy]     - fXvtx[ixyprev];
      Float_t dyprev = fYvtx[ixy]     - fYvtx[ixyprev];
      Float_t dxnext = fXvtx[ixynext] - fXvtx[ixy];
      Float_t dynext = fYvtx[ixynext] - fYvtx[ixy];

      Float_t xprod = dxprev*dynext - dxnext*dyprev;

      if (xprod > 0) {
         plus += xprod;
      } else if (xprod < 0) {
         minus -= xprod;
      } else {
         zero++;
      }
   }

   if (fNxy<3) {
      // no check yet written for checking that the segments don't cross
      fPolygonShape = kMalformedXY;
   } else {
      if (plus==0 || minus==0) {
         // convex polygons have all of one sign
         if (plus>minus) {
            fPolygonShape = kConvexCCW;
         } else {
            fPolygonShape = kConvexCW;
         }
      } else {
         // concave
         if (plus>minus) {
            fPolygonShape = kConcaveCCW;
         } else {
            fPolygonShape = kConcaveCW;
         }
      }
   }

   // Check on z ordering
   // Convex vs. Concave and increasing or decreasing in z
   plus = minus = zero = 0;
   Bool_t scaleSignChange = kFALSE;
   Int_t iz=0;
   for (iz=0; iz<fNz; iz++) {
      // calculate the cross product of the two segments that
      // meet at vertex "iz"
      // concave polygons have a mixture of + and - values
      Int_t izprev = (iz + fNz - 1)%fNz;
      Int_t iznext = (iz + fNz + 1)%fNz;

      Float_t dzprev = fZ[iz]         - fZ[izprev];
      Float_t dsprev = fScale[iz]     - fScale[izprev];
      Float_t dznext = fZ[iznext]     - fZ[iz];
      Float_t dsnext = fScale[iznext] - fScale[iz];

      // special cases for end faces
      if (iz==0) {
         dzprev = 0;
         dsprev = fScale[0];
      } else if (iz==fNz-1) {
         dznext = 0;
         dsnext = -fScale[iz];
      }

      Float_t xprod = dznext*dsprev - dzprev*dsnext;

      if (xprod > 0) {
         plus += xprod;
      } else if (xprod < 0) {
         minus -= xprod;
      } else {
         zero++;
      }
      // also check for scale factors that change sign...
      if (fScale[iz]*fScale[iznext] < 0) scaleSignChange = kTRUE;
   }

   if (fNz<1 || scaleSignChange) {
      // no check yet written for checking that the segments don't cross
      fZOrdering = kMalformedZ;
   } else {
      if (plus==0 || minus==0) {
         // convex polygons have all of one sign
         if (plus>minus) {
            fZOrdering = kConvexIncZ;
         } else {
            fZOrdering = kConvexDecZ;
         }
      } else {
         // concave
         if (plus>minus) {
            fZOrdering = kConcaveIncZ;
         } else {
            fZOrdering = kConcaveDecZ;
         }
      }
   }
}


//______________________________________________________________________________
void TXTRU::DumpPoints(int npoints, float *pointbuff) const
{
   // Dump the vertex points for visual inspection

   cout << "TXTRU::DumpPoints - " << npoints << " points" << endl;
   int ioff = 0;
   float x,y,z;
   int ipt=0;
   for (ipt=0; ipt<npoints; ipt++) {
      x = pointbuff[ioff++];
      y = pointbuff[ioff++];
      z = pointbuff[ioff++];
      printf(" [%4d] %6.1f %6.1f %6.1f \n",ipt,x,y,z);
   }
}


//______________________________________________________________________________
void TXTRU::DumpSegments(int nsegments, int *segbuff) const
{
   // Dump the segment info for visual inspection

   cout << "TXTRU::DumpSegments - " << nsegments << " segments" << endl;
   int ioff = 0;
   int icol, p1, p2;
   int iseg=0;
   for (iseg=0; iseg<nsegments; iseg++) {
      icol = segbuff[ioff++];
      p1   = segbuff[ioff++];
      p2   = segbuff[ioff++];
      printf(" [%4d] %3d (%4d,%4d)\n",iseg,icol,p1,p2);
   }
}


//______________________________________________________________________________
void TXTRU::DumpPolygons(int npolygons, int *polybuff, int buffsize) const
{
   // Dump the derived polygon info for visual inspection

   cout << "TXTRU::DumpPolygons - " << npolygons << " polygons" << endl;
   int ioff = 0;
   int icol, nseg, iseg;
   int ipoly=0;
   for (ipoly=0; ipoly<npolygons; ipoly++) {
      icol = polybuff[ioff++];
      nseg = polybuff[ioff++];
#ifndef R__MACOSX
      cout << "  [" << setw(4) << ipoly << "] icol " << setw(3) << icol
           << " nseg " << setw(3) << nseg << "  (";
#else
      printf(" [%d4] icol %d3 nseg %d3  (", ipoly, icol, nseg);
#endif
      for (iseg=0; iseg<nseg-1; iseg++) {
         cout << polybuff[ioff++] << ",";
      }
      cout << polybuff[ioff++] << ")" << endl;
   }
   cout << " buffer size " << buffsize << " last used " << --ioff << endl;
}


//______________________________________________________________________________
const TBuffer3D & TXTRU::GetBuffer3D(Int_t reqSections) const
{
   // Get buffer 3d.

   static TBuffer3D buffer(TBuffer3DTypes::kGeneric);

   TShape::FillBuffer3D(buffer, reqSections);

   if (reqSections & TBuffer3D::kRawSizes) {
      // Check that the polygon is well formed
      // convex vs. concave, z ordered monotonically

      if (fPolygonShape == kUncheckedXY ||
          fZOrdering    == kUncheckedZ) {
         const_cast<TXTRU *>(this)->CheckOrdering();
      }
      Int_t nbPnts = fNz*fNxy;
      Int_t nbSegs = fNxy*(2*fNz-1);
      Int_t nbPols = fNxy*(fNz-1)+2;
      if (buffer.SetRawSizes(nbPnts, 3*nbPnts, nbSegs, 3*nbSegs, nbPols, 6*(nbPols-2)+2*(2+fNxy))) {
         buffer.SetSectionsValid(TBuffer3D::kRawSizes);
      }
   }
   if (reqSections & TBuffer3D::kRaw) {
      // Points
      SetPoints(buffer.fPnts);
      if (!buffer.fLocalFrame) {
         TransformPoints(buffer.fPnts, buffer.NbPnts());
      }

      Int_t c = GetBasicColor();

      Int_t i,j, k;
      Int_t indx, indx2;
      indx = indx2 = 0;

      // Segments
      for (i=0; i<fNz; i++) {
         // loop Z planes
         indx2 = i*fNxy;
         // loop polygon segments
         for (j=0; j<fNxy; j++) {
            k = (j+1)%fNxy;
            buffer.fSegs[indx++] = c;
            buffer.fSegs[indx++] = indx2+j;
            buffer.fSegs[indx++] = indx2+k;
         }
      } // total: fNz*fNxy polygon segments
      for (i=0; i<fNz-1; i++) {
         // loop Z planes
         indx2 = i*fNxy;
         // loop polygon segments
         for (j=0; j<fNxy; j++) {
            k = j + fNxy;
            buffer.fSegs[indx++] = c;
            buffer.fSegs[indx++] = indx2+j;
            buffer.fSegs[indx++] = indx2+k;
         }
      } // total (fNz-1)*fNxy lateral segments

            // Polygons
      indx = 0;

      // fill lateral polygons
      for (i=0; i<fNz-1; i++) {
         indx2 = i*fNxy;
         for (j=0; j<fNxy; j++) {
         k = (j+1)%fNxy;
         buffer.fPols[indx++] = c+j%3;
         buffer.fPols[indx++] = 4;
         buffer.fPols[indx++] = indx2+j;
         buffer.fPols[indx++] = fNz*fNxy+indx2+k;
         buffer.fPols[indx++] = indx2+fNxy+j;
         buffer.fPols[indx++] = fNz*fNxy+indx2+j;
         }
      } // total (fNz-1)*fNxy polys
      buffer.fPols[indx++] = c+2;
      buffer.fPols[indx++] = fNxy;
      indx2 = 0;
      for (j = fNxy - 1; j >= 0; --j) {
         buffer.fPols[indx++] = indx2+j;
      }

      buffer.fPols[indx++] = c;
      buffer.fPols[indx++] = fNxy;
      indx2 = (fNz-1)*fNxy;
    
      for (j=0; j<fNxy; j++) {
         buffer.fPols[indx++] = indx2+j;
      }

      buffer.SetSectionsValid(TBuffer3D::kRaw);
   }

   return buffer;
}
