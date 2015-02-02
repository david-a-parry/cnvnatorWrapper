// @(#)root/vmc:$Id$
// Authors: Alice collaboration 25/06/2002

/*************************************************************************
 * Copyright (C) 2006, Rene Brun and Fons Rademakers.                    *
 * Copyright (C) 2002, ALICE Experiment at CERN.                         *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


//______________________________________________________________________________
//
// Implementation of the TVirtualMCGeometry interface
// for building TGeo geometry.
//______________________________________________________________________________

#include <ctype.h>
#include "TError.h"
#include "TArrayD.h"

#include "TGeoMCGeometry.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TGeoArb8.h"
#include "TGeoTrd1.h"
#include "TGeoTrd2.h"
#include "TGeoTube.h"
#include "TGeoCone.h"
#include "TGeoPgon.h"
#include "TGeoSphere.h"
#include "TGeoPara.h"
#include "TGeoEltu.h"
#include "TGeoHype.h"
#include "TMath.h"

ClassImp(TGeoMCGeometry)

TGeoMCGeometry* TGeoMCGeometry::fgInstance=0;

//_____________________________________________________________________________
TGeoMCGeometry::TGeoMCGeometry(const char *name, const char *title,
                               Bool_t g3CompatibleVolumeNames)
  : TVirtualMCGeometry(name, title),
    fG3CompatibleVolumeNames(g3CompatibleVolumeNames)
{
   //
   // Standard constructor
   //
}

//_____________________________________________________________________________
TGeoMCGeometry::TGeoMCGeometry()
  : TVirtualMCGeometry(),
    fG3CompatibleVolumeNames(kFALSE)
{
   //
   // Default constructor
   //
}

//_____________________________________________________________________________
TGeoMCGeometry::~TGeoMCGeometry()
{
   //
   // Destructor
   //
   fgInstance=0;
}

//
// private methods
//


//_____________________________________________________________________________
TGeoManager* TGeoMCGeometry::GetTGeoManager() const
{
// Return TGeoManager global pointer.
// Create a new TGeoManager object if it does not yet exist.

  if ( ! gGeoManager ) new TGeoManager("TGeo", "Root geometry manager");

  return gGeoManager;
}  

//_____________________________________________________________________________
Double_t* TGeoMCGeometry::CreateDoubleArray(Float_t* array, Int_t size) const
{
// Converts Float_t* array to Double_t*,
// !! The new array has to be deleted by user.
// ---

   Double_t* doubleArray;
   if (size>0) {
      doubleArray = new Double_t[size];
      for (Int_t i=0; i<size; i++) doubleArray[i] = array[i];
   } else {
      //doubleArray = 0;
      doubleArray = new Double_t[1];
   }
   return doubleArray;
}

//______________________________________________________________________________
void TGeoMCGeometry::Vname(const char *name, char *vname) const
{
   //
   //  convert name to upper case. Make vname at least 4 chars
   //
   if (fG3CompatibleVolumeNames) {
      Int_t l = strlen(name);
      Int_t i;
      l = l < 4 ? l : 4;
      for (i=0;i<l;i++) vname[i] = toupper(name[i]);
      for (i=l;i<4;i++) vname[i] = ' ';
      vname[4] = 0;
   } else {
      Int_t l = strlen(name);
      if ( l>=79 ) l = 79;
      for (Int_t i=0;i<l;i++) vname[i] = name[i];
      vname[l] = 0;
   }
}

//
// public methods
//

//_____________________________________________________________________________
void TGeoMCGeometry::Material(Int_t& kmat, const char* name, Double_t a, Double_t z,
                       Double_t dens, Double_t radl, Double_t absl, Float_t* buf,
                       Int_t nwbuf)
{
  //
  // Defines a Material
  //
  //  kmat               number assigned to the material
  //  name               material name
  //  a                  atomic mass in au
  //  z                  atomic number
  //  dens               density in g/cm3
  //  absl               absorbtion length in cm
  //                     if >=0 it is ignored and the program
  //                     calculates it, if <0. -absl is taken
  //  radl               radiation length in cm
  //                     if >=0 it is ignored and the program
  //                     calculates it, if <0. -radl is taken
  //  buf                pointer to an array of user words
  //  nbuf               number of user words
  //

   Double_t* dbuf = CreateDoubleArray(buf, nwbuf);
   Material(kmat, name, a, z, dens, radl, absl, dbuf, nwbuf);
   delete [] dbuf;
}

//_____________________________________________________________________________
void TGeoMCGeometry::Material(Int_t& kmat, const char* name, Double_t a, Double_t z,
                     Double_t dens, Double_t radl, Double_t absl, Double_t* /*buf*/,
                     Int_t /*nwbuf*/)
{
  //
  // Defines a Material
  //
  //  kmat               number assigned to the material
  //  name               material name
  //  a                  atomic mass in au
  //  z                  atomic number
  //  dens               density in g/cm3
  //  absl               absorbtion length in cm
  //                     if >=0 it is ignored and the program
  //                     calculates it, if <0. -absl is taken
  //  radl               radiation length in cm
  //                     if >=0 it is ignored and the program
  //                     calculates it, if <0. -radl is taken
  //  buf                pointer to an array of user words
  //  nbuf               number of user words
  //

   GetTGeoManager()->Material(name, a, z, dens, kmat, radl, absl);
}

//_____________________________________________________________________________
void TGeoMCGeometry::Mixture(Int_t& kmat, const char* name, Float_t* a, Float_t* z,
                    Double_t dens, Int_t nlmat, Float_t* wmat)
{
  //
  // Defines mixture OR COMPOUND IMAT as composed by
  // THE BASIC NLMAT materials defined by arrays A,Z and WMAT
  //
  // If NLMAT > 0 then wmat contains the proportion by
  // weights of each basic material in the mixture.
  //
  // If nlmat < 0 then WMAT contains the number of atoms
  // of a given kind into the molecule of the COMPOUND
  // In this case, WMAT in output is changed to relative
  // weigths.
  //

   Double_t* da = CreateDoubleArray(a, TMath::Abs(nlmat));
   Double_t* dz = CreateDoubleArray(z, TMath::Abs(nlmat));
   Double_t* dwmat = CreateDoubleArray(wmat, TMath::Abs(nlmat));

   Mixture(kmat, name, da, dz, dens, nlmat, dwmat);
   for (Int_t i=0; i<nlmat; i++) {
      a[i] = da[i]; z[i] = dz[i]; wmat[i] = dwmat[i];
   }

   delete [] da;
   delete [] dz;
   delete [] dwmat;
}

//_____________________________________________________________________________
void TGeoMCGeometry::Mixture(Int_t& kmat, const char* name, Double_t* a, Double_t* z,
                    Double_t dens, Int_t nlmat, Double_t* wmat)
{
  //
  // Defines mixture OR COMPOUND IMAT as composed by
  // THE BASIC NLMAT materials defined by arrays A,Z and WMAT
  //
  // If NLMAT > 0 then wmat contains the proportion by
  // weights of each basic material in the mixture.
  //
  // If nlmat < 0 then WMAT contains the number of atoms
  // of a given kind into the molecule of the COMPOUND
  // In this case, WMAT in output is changed to relative
  // weigths.
  //

   if (nlmat < 0) {
      nlmat = - nlmat;
      Double_t amol = 0;
      Int_t i;
      for (i=0;i<nlmat;i++) {
         amol += a[i]*wmat[i];
      }
      for (i=0;i<nlmat;i++) {
         wmat[i] *= a[i]/amol;
      }
   }
   GetTGeoManager()->Mixture(name, a, z, dens, nlmat, wmat, kmat);
}

//_____________________________________________________________________________
void TGeoMCGeometry::Medium(Int_t& kmed, const char* name, Int_t nmat, Int_t isvol,
                   Int_t ifield, Double_t fieldm, Double_t tmaxfd,
                   Double_t stemax, Double_t deemax, Double_t epsil,
                   Double_t stmin, Float_t* ubuf, Int_t nbuf)
{
  //
  //  kmed      tracking medium number assigned
  //  name      tracking medium name
  //  nmat      material number
  //  isvol     sensitive volume flag
  //  ifield    magnetic field
  //  fieldm    max. field value (kilogauss)
  //  tmaxfd    max. angle due to field (deg/step)
  //  stemax    max. step allowed
  //  deemax    max. fraction of energy lost in a step
  //  epsil     tracking precision (cm)
  //  stmin     min. step due to continuous processes (cm)
  //
  //  ifield = 0 if no magnetic field; ifield = -1 if user decision in guswim;
  //  ifield = 1 if tracking performed with g3rkuta; ifield = 2 if tracking
  //  performed with g3helix; ifield = 3 if tracking performed with g3helx3.
  //

  //printf("Creating mediuma: %s, numed=%d, nmat=%d\n",name,kmed,nmat);
   Double_t* dubuf = CreateDoubleArray(ubuf, nbuf);
   Medium(kmed, name, nmat, isvol, ifield, fieldm, tmaxfd, stemax, deemax, epsil,
          stmin, dubuf, nbuf);
   delete [] dubuf;
}

//_____________________________________________________________________________
void TGeoMCGeometry::Medium(Int_t& kmed, const char* name, Int_t nmat, Int_t isvol,
                   Int_t ifield, Double_t fieldm, Double_t tmaxfd,
                   Double_t stemax, Double_t deemax, Double_t epsil,
                   Double_t stmin, Double_t* /*ubuf*/, Int_t /*nbuf*/)
{
  //
  //  kmed      tracking medium number assigned
  //  name      tracking medium name
  //  nmat      material number
  //  isvol     sensitive volume flag
  //  ifield    magnetic field
  //  fieldm    max. field value (kilogauss)
  //  tmaxfd    max. angle due to field (deg/step)
  //  stemax    max. step allowed
  //  deemax    max. fraction of energy lost in a step
  //  epsil     tracking precision (cm)
  //  stmin     min. step due to continuos processes (cm)
  //
  //  ifield = 0 if no magnetic field; ifield = -1 if user decision in guswim;
  //  ifield = 1 if tracking performed with g3rkuta; ifield = 2 if tracking
  //  performed with g3helix; ifield = 3 if tracking performed with g3helx3.
  //

   GetTGeoManager()->Medium(name,kmed,nmat, isvol, ifield, fieldm, tmaxfd, stemax,deemax, epsil, stmin);
}

//_____________________________________________________________________________
void TGeoMCGeometry::Matrix(Int_t& krot, Double_t thex, Double_t phix, Double_t they,
                   Double_t phiy, Double_t thez, Double_t phiz)
{
  //
  //  krot     rotation matrix number assigned
  //  theta1   polar angle for axis i
  //  phi1     azimuthal angle for axis i
  //  theta2   polar angle for axis ii
  //  phi2     azimuthal angle for axis ii
  //  theta3   polar angle for axis iii
  //  phi3     azimuthal angle for axis iii
  //
  //  it defines the rotation matrix number irot.
  //

   krot = GetTGeoManager()->GetListOfMatrices()->GetEntriesFast();
   GetTGeoManager()->Matrix(krot, thex, phix, they, phiy, thez, phiz);
}

//_____________________________________________________________________________
Int_t TGeoMCGeometry::Gsvolu(const char *name, const char *shape, Int_t nmed,
                    Float_t *upar, Int_t npar)
{
  //
  //  NAME   Volume name
  //  SHAPE  Volume type
  //  NUMED  Tracking medium number
  //  NPAR   Number of shape parameters
  //  UPAR   Vector containing shape parameters
  //
  //  It creates a new volume in the JVOLUM data structure.
  //

   Double_t* dupar = CreateDoubleArray(upar, npar);
   Int_t id = Gsvolu(name, shape, nmed, dupar, npar);
   delete [] dupar;
   return id;
}

//_____________________________________________________________________________
Int_t TGeoMCGeometry::Gsvolu(const char *name, const char *shape, Int_t nmed,
                    Double_t *upar, Int_t npar)
{
  //
  //  NAME   Volume name
  //  SHAPE  Volume type
  //  NUMED  Tracking medium number
  //  NPAR   Number of shape parameters
  //  UPAR   Vector containing shape parameters
  //
  //  It creates a new volume in the JVOLUM data structure.
  //

   char vname[80];
   Vname(name,vname);
   char vshape[5];
   Vname(shape,vshape);

   TGeoVolume* vol = GetTGeoManager()->Volume(vname, vshape, nmed, upar, npar);
   if (!vol) {
      Fatal("Gsvolu", "Could not create volume %s", name);
      return -1;
   }
   return vol->GetNumber();
}

//_____________________________________________________________________________
void  TGeoMCGeometry::Gsdvn(const char *name, const char *mother, Int_t ndiv,
                   Int_t iaxis)
{
  //
  // Create a new volume by dividing an existing one
  //
  //  NAME   Volume name
  //  MOTHER Mother volume name
  //  NDIV   Number of divisions
  //  IAXIS  Axis value
  //
  //  X,Y,Z of CAXIS will be translated to 1,2,3 for IAXIS.
  //  It divides a previously defined volume.
  //
   char vname[80];
   Vname(name,vname);
   char vmother[80];
   Vname(mother,vmother);

   GetTGeoManager()->Division(vname, vmother, iaxis, ndiv, 0, 0, 0, "n");
}

//_____________________________________________________________________________
void  TGeoMCGeometry::Gsdvn2(const char *name, const char *mother, Int_t ndiv,
                    Int_t iaxis, Double_t c0i, Int_t numed)
{
  //
  // Create a new volume by dividing an existing one
  //
  // Divides mother into ndiv divisions called name
  // along axis iaxis starting at coordinate value c0.
  // the new volume created will be medium number numed.
  //
   char vname[80];
   Vname(name,vname);
   char vmother[80];
   Vname(mother,vmother);

   GetTGeoManager()->Division(vname, vmother, iaxis, ndiv, c0i, 0, numed, "nx");
}
//_____________________________________________________________________________
void  TGeoMCGeometry::Gsdvt(const char *name, const char *mother, Double_t step,
                   Int_t iaxis, Int_t numed, Int_t /*ndvmx*/)
{
  //
  // Create a new volume by dividing an existing one
  //
  //       Divides MOTHER into divisions called NAME along
  //       axis IAXIS in steps of STEP. If not exactly divisible
  //       will make as many as possible and will centre them
  //       with respect to the mother. Divisions will have medium
  //       number NUMED. If NUMED is 0, NUMED of MOTHER is taken.
  //       NDVMX is the expected maximum number of divisions
  //          (If 0, no protection tests are performed)
  //
   char vname[80];
   Vname(name,vname);
   char vmother[80];
   Vname(mother,vmother);

   GetTGeoManager()->Division(vname, vmother, iaxis, 0, 0, step, numed, "s");
}

//_____________________________________________________________________________
void  TGeoMCGeometry::Gsdvt2(const char *name, const char *mother, Double_t step,
                    Int_t iaxis, Double_t c0, Int_t numed, Int_t /*ndvmx*/)
{
  //
  // Create a new volume by dividing an existing one
  //
  //           Divides MOTHER into divisions called NAME along
  //            axis IAXIS starting at coordinate value C0 with step
  //            size STEP.
  //           The new volume created will have medium number NUMED.
  //           If NUMED is 0, NUMED of mother is taken.
  //           NDVMX is the expected maximum number of divisions
  //             (If 0, no protection tests are performed)
  //
   char vname[80];
   Vname(name,vname);
   char vmother[80];
   Vname(mother,vmother);

   GetTGeoManager()->Division(vname, vmother, iaxis, 0, c0, step, numed, "sx");
}

//_____________________________________________________________________________
void  TGeoMCGeometry::Gsord(const char * /*name*/, Int_t /*iax*/)
{
  //
  //    Flags volume CHNAME whose contents will have to be ordered
  //    along axis IAX, by setting the search flag to -IAX
  //           IAX = 1    X axis
  //           IAX = 2    Y axis
  //           IAX = 3    Z axis
  //           IAX = 4    Rxy (static ordering only  -> GTMEDI)
  //           IAX = 14   Rxy (also dynamic ordering -> GTNEXT)
  //           IAX = 5    Rxyz (static ordering only -> GTMEDI)
  //           IAX = 15   Rxyz (also dynamic ordering -> GTNEXT)
  //           IAX = 6    PHI   (PHI=0 => X axis)
  //           IAX = 7    THETA (THETA=0 => Z axis)
  //

  // TBC - keep this function
  // nothing to be done for TGeo  //xx
}

//_____________________________________________________________________________
void  TGeoMCGeometry::Gspos(const char *name, Int_t nr, const char *mother, Double_t x,
                   Double_t y, Double_t z, Int_t irot, const char *konly)
{
  //
  // Position a volume into an existing one
  //
  //  NAME   Volume name
  //  NUMBER Copy number of the volume
  //  MOTHER Mother volume name
  //  X      X coord. of the volume in mother ref. sys.
  //  Y      Y coord. of the volume in mother ref. sys.
  //  Z      Z coord. of the volume in mother ref. sys.
  //  IROT   Rotation matrix number w.r.t. mother ref. sys.
  //  ONLY   ONLY/MANY flag
  //
  //  It positions a previously defined volume in the mother.
  //

   TString only = konly;
   only.ToLower();
   Bool_t isOnly = kFALSE;
   if (only.Contains("only")) isOnly = kTRUE;
   char vname[80];
   Vname(name,vname);
   char vmother[80];
   Vname(mother,vmother);

   Double_t *upar=0;
   GetTGeoManager()->Node(vname, nr, vmother, x, y, z, irot, isOnly, upar);
}

//_____________________________________________________________________________
void  TGeoMCGeometry::Gsposp(const char *name, Int_t nr, const char *mother,
                    Double_t x, Double_t y, Double_t z, Int_t irot,
                    const char *konly, Float_t *upar, Int_t np )
{
  //
  //      Place a copy of generic volume NAME with user number
  //      NR inside MOTHER, with its parameters UPAR(1..NP)
  //

   Double_t* dupar = CreateDoubleArray(upar, np);
   Gsposp(name, nr, mother, x, y, z, irot, konly, dupar, np);
   delete [] dupar;
}

//_____________________________________________________________________________
void  TGeoMCGeometry::Gsposp(const char *name, Int_t nr, const char *mother,
                    Double_t x, Double_t y, Double_t z, Int_t irot,
                    const char *konly, Double_t *upar, Int_t np )
{
  //
  //      Place a copy of generic volume NAME with user number
  //      NR inside MOTHER, with its parameters UPAR(1..NP)
  //

   TString only = konly;
   only.ToLower();
   Bool_t isOnly = kFALSE;
   if (only.Contains("only")) isOnly = kTRUE;
   char vname[80];
   Vname(name,vname);
   char vmother[80];
   Vname(mother,vmother);

   GetTGeoManager()->Node(vname,nr,vmother, x,y,z,irot,isOnly,upar,np);
}

//_____________________________________________________________________________
Int_t TGeoMCGeometry::VolId(const char *name) const
{
  //
  // Return the unique numeric identifier for volume name
  //

   Int_t uid = GetTGeoManager()->GetUID(name);
   if (uid<0) {
      printf("VolId: Volume %s not found\n",name);
      return 0;
   }
   return uid;
}

//_____________________________________________________________________________
Int_t TGeoMCGeometry::MediumId(const char *name) const
{
  //
  // Return the unique numeric identifier for medium name
  //

   TGeoMedium* medium = GetTGeoManager()->GetMedium(name);
   if (medium) return medium->GetId();

   printf("MediumId: Medium %s not found\n",name);
   return 0;
}

//_____________________________________________________________________________
const char* TGeoMCGeometry::VolName(Int_t id) const
{
  //
  // Return the volume name given the volume identifier
  //

   TGeoVolume *volume = GetTGeoManager()->GetVolume(id);
   if (!volume) {
      Error("VolName","volume with id=%d does not exist",id);
      return "NULL";
   }
   return volume->GetName();
}

//_____________________________________________________________________________
Int_t TGeoMCGeometry::NofVolumes() const
{
  //
  // Return total number of volumes in the geometry
  //

   return GetTGeoManager()->GetListOfUVolumes()->GetEntriesFast()-1;
}

//_____________________________________________________________________________
Int_t TGeoMCGeometry::NofVolDaughters(const char* volName) const
{
// Return number of daughters of the volume specified by volName
// According to A. Morsch' G3toRoot class (by A. Morsch)
// ---

   TGeoVolume* volume = GetTGeoManager()->GetVolume(volName);

   if (!volume) {
      Error("NofVolDaughters", "Volume %s not found.", volName);
      return 0;
   }

   return volume->GetNdaughters();
}

//_____________________________________________________________________________
const char*  TGeoMCGeometry::VolDaughterName(const char* volName, Int_t i) const
{
// Return the name of i-th daughters of the volume specified by volName
// According to A. Morsch' G3toRoot class.
// ---

   // Get volume
   TGeoVolume* volume = GetTGeoManager()->GetVolume(volName);
   if (!volume) {
      Error("VolDaughterName", "Volume %s not found.", volName);
      return "";
   }

   // Check index
   if (i<0 || i>=volume->GetNdaughters()) {
      Error("VolDaughterName", "Volume %s Index out of limits", volName);
      return "";
   }

   // Return node's volume name
   return volume->GetNode(i)->GetVolume()->GetName();
}

//_____________________________________________________________________________
Int_t TGeoMCGeometry::VolDaughterCopyNo(const char* volName, Int_t i) const
{
// Return the copyNo of i-th daughters of the volume specified by volName
// According to A. Morsch' G3toRoot class.
// ---


   // Get volume
   TGeoVolume* volume = GetTGeoManager()->GetVolume(volName);
   if (!volume) {
      Error("VolDaughterName", "Volume %s not found.", volName);
      return 0;
   }

   // Check index
   if (i<0 || i>=volume->GetNdaughters()) {
      Error("VolDaughterName", "Volume %s Index out of limits", volName);
      return 0;
   }

   // Return node's copyNo
   return volume->GetNode(i)->GetNumber();
}

//_____________________________________________________________________________
Int_t TGeoMCGeometry::VolId2Mate(Int_t id) const
{
  //
  // Return material number for a given volume id
  //

   TGeoVolume *volume = GetTGeoManager()->GetVolume(id);
   if (!volume) {
      Error("VolId2Mate","volume with id=%d does not exist",id);
      return 0;
   }
   TGeoMedium *med = volume->GetMedium();
   if (!med) return 0;
   return med->GetId();
}

//______________________________________________________________________
Bool_t TGeoMCGeometry::GetTransformation(const TString &volumePath,TGeoHMatrix &mat)
{
    // Returns the Transformation matrix between the volume specified
    // by the path volumePath and the Top or mater volume. The format
    // of the path volumePath is as follows (assuming ALIC is the Top volume)
    // "/ALIC_1/DDIP_1/S05I_2/S05H_1/S05G_3". Here ALIC is the top most
    // or master volume which has only 1 instance of. Of all of the daughter
    // volumes of ALICE, DDIP volume copy #1 is indicated. Similarly for
    // the daughter volume of DDIP is S05I copy #2 and so on.
    // Inputs:
    //   TString& volumePath  The volume path to the specific volume
    //                        for which you want the matrix. Volume name
    //                        hierarchy is separated by "/" while the
    //                        copy number is appended using a "_".
    // Outputs:
    //  TGeoHMatrix &mat      A matrix with its values set to those
    //                        appropriate to the Local to Master transformation
    // Return:
    //   A logical value if kFALSE then an error occurred and no change to
    //   mat was made.

   // We have to preserve the modeler state
   GetTGeoManager()->PushPath();
   if (!GetTGeoManager()->cd(volumePath.Data())) {
      GetTGeoManager()->PopPath();
      return kFALSE;
   }
   mat = *GetTGeoManager()->GetCurrentMatrix();
   GetTGeoManager()->PopPath();
   return kTRUE;
}
//______________________________________________________________________
Bool_t TGeoMCGeometry::GetShape(const TString &volumePath,TString &shapeType,
                         TArrayD &par)
{
    // Returns the shape and its parameters for the volume specified
    // by volumeName.
    // Inputs:
    //   TString& volumeName  The volume name
    // Outputs:
    //   TString &shapeType   Shape type
    //   TArrayD &par         A TArrayD of parameters with all of the
    //                        parameters of the specified shape.
    // Return:
    //   A logical indicating whether there was an error in getting this
    //   information
   Int_t npar;
   GetTGeoManager()->PushPath();
   if (!GetTGeoManager()->cd(volumePath.Data())) {
      GetTGeoManager()->PopPath();
      return kFALSE;
   }
   TGeoVolume * vol = GetTGeoManager()->GetCurrentVolume();
   GetTGeoManager()->PopPath();
   if (!vol) return kFALSE;
   TGeoShape *shape = vol->GetShape();
   TClass *class_type = shape->IsA();
   if (class_type==TGeoBBox::Class()) {
      shapeType = "BOX";
      npar = 3;
      par.Set(npar);
      TGeoBBox *box = (TGeoBBox*)shape;
      par.AddAt(box->GetDX(),0);
      par.AddAt(box->GetDY(),1);
      par.AddAt(box->GetDZ(),2);
      return kTRUE;
   }
   if (class_type==TGeoTrd1::Class()) {
      shapeType = "TRD1";
      npar = 4;
      par.Set(npar);
      TGeoTrd1 *trd1 = (TGeoTrd1*)shape;
      par.AddAt(trd1->GetDx1(),0);
      par.AddAt(trd1->GetDx2(),1);
      par.AddAt(trd1->GetDy(), 2);
      par.AddAt(trd1->GetDz(), 3);
      return kTRUE;
   }
   if (class_type==TGeoTrd2::Class()) {
      shapeType = "TRD2";
      npar = 5;
      par.Set(npar);
      TGeoTrd2 *trd2 = (TGeoTrd2*)shape;
      par.AddAt(trd2->GetDx1(),0);
      par.AddAt(trd2->GetDx2(),1);
      par.AddAt(trd2->GetDy1(),2);
      par.AddAt(trd2->GetDy2(),3);
      par.AddAt(trd2->GetDz(), 4);
      return kTRUE;
   }
   if (class_type==TGeoTrap::Class()) {
      shapeType = "TRAP";
      npar = 11;
      par.Set(npar);
      TGeoTrap *trap = (TGeoTrap*)shape;
      Double_t tth = TMath::Tan(trap->GetTheta()*TMath::DegToRad());
      par.AddAt(trap->GetDz(),0);
      par.AddAt(tth*TMath::Cos(trap->GetPhi()*TMath::DegToRad()),1);
      par.AddAt(tth*TMath::Sin(trap->GetPhi()*TMath::DegToRad()),2);
      par.AddAt(trap->GetH1(),3);
      par.AddAt(trap->GetBl1(),4);
      par.AddAt(trap->GetTl1(),5);
      par.AddAt(TMath::Tan(trap->GetAlpha1()*TMath::DegToRad()),6);
      par.AddAt(trap->GetH2(),7);
      par.AddAt(trap->GetBl2(),8);
      par.AddAt(trap->GetTl2(),9);
      par.AddAt(TMath::Tan(trap->GetAlpha2()*TMath::DegToRad()),10);
      return kTRUE;
   }
   if (class_type==TGeoTube::Class()) {
      shapeType = "TUBE";
      npar = 3;
      par.Set(npar);
      TGeoTube *tube = (TGeoTube*)shape;
      par.AddAt(tube->GetRmin(),0);
      par.AddAt(tube->GetRmax(),1);
      par.AddAt(tube->GetDz(),2);
      return kTRUE;
   }
   if (class_type==TGeoTubeSeg::Class()) {
      shapeType = "TUBS";
      npar = 5;
      par.Set(npar);
      TGeoTubeSeg *tubs = (TGeoTubeSeg*)shape;
      par.AddAt(tubs->GetRmin(),0);
      par.AddAt(tubs->GetRmax(),1);
      par.AddAt(tubs->GetDz(),2);
      par.AddAt(tubs->GetPhi1(),3);
      par.AddAt(tubs->GetPhi2(),4);
      return kTRUE;
   }
   if (class_type==TGeoCone::Class()) {
      shapeType = "CONE";
      npar = 5;
      par.Set(npar);
      TGeoCone *cone = (TGeoCone*)shape;
      par.AddAt(cone->GetDz(),0);
      par.AddAt(cone->GetRmin1(),1);
      par.AddAt(cone->GetRmax1(),2);
      par.AddAt(cone->GetRmin2(),3);
      par.AddAt(cone->GetRmax2(),4);
      return kTRUE;
   }
   if (class_type==TGeoConeSeg::Class()) {
      shapeType = "CONS";
      npar = 7;
      par.Set(npar);
      TGeoConeSeg *cons = (TGeoConeSeg*)shape;
      par.AddAt(cons->GetDz(),0);
      par.AddAt(cons->GetRmin1(),1);
      par.AddAt(cons->GetRmax1(),2);
      par.AddAt(cons->GetRmin2(),3);
      par.AddAt(cons->GetRmax2(),4);
      par.AddAt(cons->GetPhi1(),5);
      par.AddAt(cons->GetPhi2(),6);
      return kTRUE;
   }
   if (class_type==TGeoSphere::Class()) {
      shapeType = "SPHE";
      npar = 6;
      par.Set(npar);
      TGeoSphere *sphe = (TGeoSphere*)shape;
      par.AddAt(sphe->GetRmin(),0);
      par.AddAt(sphe->GetRmax(),1);
      par.AddAt(sphe->GetTheta1(),2);
      par.AddAt(sphe->GetTheta2(),3);
      par.AddAt(sphe->GetPhi1(),4);
      par.AddAt(sphe->GetPhi2(),5);
      return kTRUE;
   }
   if (class_type==TGeoPara::Class()) {
      shapeType = "PARA";
      npar = 6;
      par.Set(npar);
      TGeoPara *para = (TGeoPara*)shape;
      par.AddAt(para->GetX(),0);
      par.AddAt(para->GetY(),1);
      par.AddAt(para->GetZ(),2);
      par.AddAt(para->GetTxy(),3);
      par.AddAt(para->GetTxz(),4);
      par.AddAt(para->GetTyz(),5);
      return kTRUE;
   }
   if (class_type==TGeoPgon::Class()) {
      shapeType = "PGON";
      TGeoPgon *pgon = (TGeoPgon*)shape;
      Int_t nz = pgon->GetNz();
      const Double_t *rmin = pgon->GetRmin();
      const Double_t *rmax = pgon->GetRmax();
      const Double_t *z = pgon->GetZ();
      npar = 4 + 3*nz;
      par.Set(npar);
      par.AddAt(pgon->GetPhi1(),0);
      par.AddAt(pgon->GetDphi(),1);
      par.AddAt(pgon->GetNedges(),2);
      par.AddAt(pgon->GetNz(),3);
      for (Int_t i=0; i<nz; i++) {
         par.AddAt(z[i], 4+3*i);
         par.AddAt(rmin[i], 4+3*i+1);
         par.AddAt(rmax[i], 4+3*i+2);
      }
      return kTRUE;
   }
   if (class_type==TGeoPcon::Class()) {
      shapeType = "PCON";
      TGeoPcon *pcon = (TGeoPcon*)shape;
      Int_t nz = pcon->GetNz();
      const Double_t *rmin = pcon->GetRmin();
      const Double_t *rmax = pcon->GetRmax();
      const Double_t *z = pcon->GetZ();
      npar = 3 + 3*nz;
      par.Set(npar);
      par.AddAt(pcon->GetPhi1(),0);
      par.AddAt(pcon->GetDphi(),1);
      par.AddAt(pcon->GetNz(),2);
      for (Int_t i=0; i<nz; i++) {
         par.AddAt(z[i], 3+3*i);
         par.AddAt(rmin[i], 3+3*i+1);
         par.AddAt(rmax[i], 3+3*i+2);
      }
      return kTRUE;
   }
   if (class_type==TGeoEltu::Class()) {
      shapeType = "ELTU";
      npar = 3;
      par.Set(npar);
      TGeoEltu *eltu = (TGeoEltu*)shape;
      par.AddAt(eltu->GetA(),0);
      par.AddAt(eltu->GetB(),1);
      par.AddAt(eltu->GetDz(),2);
      return kTRUE;
   }
   if (class_type==TGeoHype::Class()) {
      shapeType = "HYPE";
      npar = 5;
      par.Set(npar);
      TGeoHype *hype = (TGeoHype*)shape;
      par.AddAt(TMath::Sqrt(hype->RadiusHypeSq(0.,kTRUE)),0);
      par.AddAt(TMath::Sqrt(hype->RadiusHypeSq(0.,kFALSE)),1);
      par.AddAt(hype->GetDZ(),2);
      par.AddAt(hype->GetStIn(),3);
      par.AddAt(hype->GetStOut(),4);
      return kTRUE;
   }
   if (class_type==TGeoGtra::Class()) {
      shapeType = "GTRA";
      npar = 12;
      par.Set(npar);
      TGeoGtra *trap = (TGeoGtra*)shape;
      Double_t tth = TMath::Tan(trap->GetTheta()*TMath::DegToRad());
      par.AddAt(trap->GetDz(),0);
      par.AddAt(tth*TMath::Cos(trap->GetPhi()*TMath::DegToRad()),1);
      par.AddAt(tth*TMath::Sin(trap->GetPhi()*TMath::DegToRad()),2);
      par.AddAt(trap->GetH1(),3);
      par.AddAt(trap->GetBl1(),4);
      par.AddAt(trap->GetTl1(),5);
      par.AddAt(TMath::Tan(trap->GetAlpha1()*TMath::DegToRad()),6);
      par.AddAt(trap->GetH2(),7);
      par.AddAt(trap->GetBl2(),8);
      par.AddAt(trap->GetTl2(),9);
      par.AddAt(TMath::Tan(trap->GetAlpha2()*TMath::DegToRad()),10);
      par.AddAt(trap->GetTwistAngle(),11);
      return kTRUE;
   }
   if (class_type==TGeoCtub::Class()) {
      shapeType = "CTUB";
      npar = 11;
      par.Set(npar);
      TGeoCtub *ctub = (TGeoCtub*)shape;
      const Double_t *lx = ctub->GetNlow();
      const Double_t *tx = ctub->GetNhigh();
      par.AddAt(ctub->GetRmin(),0);
      par.AddAt(ctub->GetRmax(),1);
      par.AddAt(ctub->GetDz(),2);
      par.AddAt(ctub->GetPhi1(),3);
      par.AddAt(ctub->GetPhi2(),4);
      par.AddAt(lx[0],5);
      par.AddAt(lx[1],6);
      par.AddAt(lx[2],7);
      par.AddAt(tx[0],8);
      par.AddAt(tx[1],9);
      par.AddAt(tx[2],10);
      return kTRUE;

   }
   Error("GetShape","Getting shape parameters for shape %s not implemented", shape->ClassName());
   return kFALSE;
}

//______________________________________________________________________
Bool_t TGeoMCGeometry::GetMaterial(const TString &volumeName,
                            TString &name,Int_t &imat,
                            Double_t &a,Double_t &z,Double_t &dens,
                            Double_t &radl,Double_t &inter,TArrayD &par)
{
    // Returns the Material and its parameters for the volume specified
    // by volumeName.
    // Note, Geant3 stores and uses mixtures as an element with an effective
    // Z and A. Consequently, if the parameter Z is not integer, then
    // this material represents some sort of mixture.
    // Inputs:
    //   TString& volumeName  The volume name
    // Outputs:
    //   TSrting   &name       Material name
    //   Int_t     &imat       Material index number
    //   Double_t  &a          Average Atomic mass of material
    //   Double_t  &z          Average Atomic number of material
    //   Double_t  &dens       Density of material [g/cm^3]
    //   Double_t  &radl       Average radiation length of material [cm]
    //   Double_t  &inter      Average interaction length of material [cm]
    //   TArrayD   &par        A TArrayD of user defined parameters.
    // Return:
    //   kTRUE if no errors
   TGeoVolume *vol = GetTGeoManager()->GetVolume(volumeName.Data());
   if (!vol) return kFALSE;
   TGeoMedium *med = vol->GetMedium();
   if (!med) return kFALSE;
   TGeoMaterial *mat = med->GetMaterial();
   imat = mat->GetUniqueID();
   name = mat->GetName();
   name = name.Strip(TString::kTrailing, '$');
   a      = mat->GetA();
   z      = mat->GetZ();
   dens   = mat->GetDensity();
   radl   = mat->GetRadLen();
   inter  = mat->GetIntLen(); // WARNING: THIS IS NOT COMPUTED NATIVELY BY TGEO
   par.Set(0); // NO USER PARAMETERS STORED IN TGEO
   return kTRUE;
}

//______________________________________________________________________
Bool_t TGeoMCGeometry::GetMedium(const TString &volumeName,TString &name,
                          Int_t &imed,Int_t &nmat,Int_t &isvol,Int_t &ifield,
                          Double_t &fieldm,Double_t &tmaxfd,Double_t &stemax,
                          Double_t &deemax,Double_t &epsil, Double_t &stmin,
                          TArrayD &par)
{
    // Returns the Medium and its parameters for the volume specified
    // by volumeName.
    // Inputs:
    //   TString& volumeName  The volume name.
    // Outputs:
    //   TString  &name       Medium name
    //   Int_t    &nmat       Material number defined for this medium
    //   Int_t    &imed       The medium index number
    //   Int_t    &isvol      volume number defined for this medium
    //   Int_t    &iflield    Magnetic field flag
    //   Double_t &fieldm     Magnetic field strength
    //   Double_t &tmaxfd     Maximum angle of deflection per step
    //   Double_t &stemax     Maximum step size
    //   Double_t &deemax     Maximum fraction of energy allowed to be lost
    //                        to continuous process.
    //   Double_t &epsil      Boundary crossing precision
    //   Double_t &stmin      Minimum step size allowed
    //   TArrayD  &par        A TArrayD of user parameters with all of the
    //                        parameters of the specified medium.
    // Return:
    //   kTRUE if there where no errors
   TGeoVolume *vol = GetTGeoManager()->GetVolume(volumeName.Data());
   if (!vol) return kFALSE;
   TGeoMedium *med = vol->GetMedium();
   if (!med) return kFALSE;
   TGeoMaterial *mat = med->GetMaterial();
   nmat = mat->GetUniqueID();
   imed = med->GetId();
   name = med->GetName();
   name = name.Strip(TString::kTrailing, '$');
   par.Set(0); // NO USER PARAMETERS IN TGEO
   isvol  = (Int_t)med->GetParam(0);
   ifield = (Int_t)med->GetParam(1);
   fieldm = med->GetParam(2);
   tmaxfd = med->GetParam(3);
   stemax = med->GetParam(4);
   deemax = med->GetParam(5);
   epsil  = med->GetParam(6);
   stmin  = med->GetParam(7);
   return kTRUE;
}

