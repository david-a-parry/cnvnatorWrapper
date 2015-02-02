/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 *    File: $Id$
 * Authors:                                                                  *
 *   GR, Gerhard Raven,   Nikhef & VU, Gerhard.Raven@nikhef.nl
 *                                                                           *
 * Copyright (c) 2010, Nikhef & VU. All rights reserved.
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
// BEGIN_HTML
//   Implementation of the so-called real spherical harmonics, using the orthonormal normalization,
// which are related to spherical harmonics as:
//
//  Y_{l0} = Y_l^0   (m=0)
//  Y_{lm} = \frac{1}{\sqrt{2}}  \left( Y_l^m     + (-1)^m     Y_l^{-m}   \right) (m>0)
//  Y_{lm} = \frac{1}{i\sqrt{2}} \left( Y_l^{|m|} - (-1)^{|m|} Y_l^{-|m|} \right) (m<0)
//
// which implies:
//
// Y_{l0}(\cos\theta,\phi) =          N_{l0}   P_l^0    (\cos\theta)              (m=0)
// Y_{lm}(\cos\theta,\phi) = \sqrt{2} N_{lm}   P_l^m    (\cos\theta) cos(|m|\phi) (m>0)
// Y_{lm}(\cos\theta,\phi) = \sqrt{2} N_{l|m|} P_l^{|m|}(\cos\theta) sin(|m|\phi) (m<0)
//
// where
//  N_{lm} = \sqrt{ \frac{2l+1}{4\pi} \frac{ (l-m)! }{ (l+m)! } } 
//
// Note that the normalization corresponds to the orthonormal case,
// and thus we have Int d\cos\theta d\phi Y_{lm} Y_{l'm'} = \delta_{ll'} \delta{mm'}
//
// Note that in addition, this code can also represent the product of two
// (real) spherical harmonics -- it actually uses the fact that Y_{00} = \sqrt{\frac{1}{4\pi}}
// in order to represent a single spherical harmonics by multiplying it
// by \sqrt{4\pi} Y_00, as this makes it trivial to compute the analytical
// integrals, using the orthogonality properties of Y_l^m...
//
// END_HTML
//

#include "RooFit.h"
#include "Riostream.h"
#include <math.h>

#include "RooSpHarmonic.h"
#include "Math/SpecFunc.h"
#include "TMath.h"

using namespace std;

ClassImp(RooSpHarmonic)
;


//_____________________________________________________________________________
namespace {
    inline double N(int l, int m=0) { 
        double n = sqrt( double(2*l+1)/(4*TMath::Pi())*TMath::Factorial(l-m)/TMath::Factorial(l+m) );
        return m==0 ? n : TMath::Sqrt2() * n;
    }
}

//_____________________________________________________________________________
RooSpHarmonic::RooSpHarmonic() :
  _n(0),
  _sgn1(0),
  _sgn2(0)
{
}

//_____________________________________________________________________________
RooSpHarmonic::RooSpHarmonic(const char* name, const char* title, RooAbsReal& ctheta, RooAbsReal& phi, int l, int m) 
 : RooLegendre(name, title,ctheta,l,m<0?-m:m)
 , _phi("phi", "phi", this, phi)
 , _n( 2*sqrt(TMath::Pi())) 
 , _sgn1( m==0 ? 0 : m<0 ? -1 : +1 )
 , _sgn2( 0 )
{
}

//_____________________________________________________________________________
RooSpHarmonic::RooSpHarmonic(const char* name, const char* title, RooAbsReal& ctheta, RooAbsReal& phi, int l1, int m1, int l2, int m2) 
 : RooLegendre(name, title,ctheta,l1, m1<0?-m1:m1,l2,m2<0?-m2:m2)
 , _phi("phi", "phi", this, phi)
 , _n(1)
 , _sgn1( m1==0 ? 0 : m1<0 ? -1 : +1 )
 , _sgn2( m2==0 ? 0 : m2<0 ? -1 : +1 )
{
}

//_____________________________________________________________________________
RooSpHarmonic::RooSpHarmonic(const RooSpHarmonic& other, const char* name) 
 : RooLegendre(other, name)
 , _phi("phi", this,other._phi)
 , _n(other._n)
 , _sgn1( other._sgn1 )
 , _sgn2( other._sgn2 )
{
}

//_____________________________________________________________________________
Double_t RooSpHarmonic::evaluate() const 
{
    double n = _n*N(_l1,_m1)*N(_l2,_m2)*RooLegendre::evaluate();
    if (_sgn1!=0) n *= (_sgn1<0 ? sin(_m1*_phi) : cos(_m1*_phi) );
    if (_sgn2!=0) n *= (_sgn2<0 ? sin(_m2*_phi) : cos(_m2*_phi) );
    return n;
}

namespace {
    bool fullRange(const RooRealProxy& x ,const char* range)  {
      return ( x.min(range) == x.min() && x.max(range) == x.max() ) ; 
    }
}

//_____________________________________________________________________________
Int_t RooSpHarmonic::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* rangeName) const 
{
  // TODO: check that phi.max - phi.min = 2 pi... ctheta.max = +1, and ctheta.min = -1
  // we don't support indefinite integrals... maybe one day, when there is a use for it.....
  bool noRange  = ( rangeName == 0 || strlen(rangeName)==0 );
  bool phiOK    = ( noRange || fullRange(_phi,rangeName) );
  bool cthetaOK = ( noRange || fullRange(_ctheta,rangeName) );
  if (cthetaOK && phiOK && matchArgs(allVars, analVars, _ctheta,_phi)) return 3;
  if (            phiOK && matchArgs(allVars, analVars,         _phi)) return 2;
  return RooLegendre::getAnalyticalIntegral(allVars,analVars,rangeName);
}

//_____________________________________________________________________________
Double_t RooSpHarmonic::analyticalIntegral(Int_t code, const char* range) const 
{
  if (code==3) {
    return (_l1==_l2 && _sgn1*_m1==_sgn2*_m2 ) ? _n : 0 ;  
  } else if (code == 2) {
    if ( _sgn1*_m1 != _sgn2*_m2) return 0;
    return ( _m1==0 ? 2 : 1 ) * TMath::Pi()*_n*N(_l1,_m1)*N(_l2,_m2)*RooLegendre::evaluate();
  } else {
    double n = _n*N(_l1,_m1)*N(_l2,_m2)*RooLegendre::analyticalIntegral(code,range);
    if (_sgn1!=0) n *= (_sgn1<0 ? sin(_m1*_phi) : cos(_m1*_phi) );
    if (_sgn2!=0) n *= (_sgn2<0 ? sin(_m2*_phi) : cos(_m2*_phi) );
    return n;
  } 
}

Int_t RooSpHarmonic::getMaxVal( const RooArgSet& vars) const {
    return RooLegendre::getMaxVal(vars);
}

Double_t RooSpHarmonic::maxVal( Int_t code) const {
    double n = _n*N(_l1,_m1)*N(_l2,_m2);
    return n*RooLegendre::maxVal(code);
}
