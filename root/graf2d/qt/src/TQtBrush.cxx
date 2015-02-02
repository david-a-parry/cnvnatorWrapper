// @(#)root/qt:$Id$
// Author: Valeri Fine   21/01/2002

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * Copyright (C) 2002 by Valeri Fine.                                    *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


/////////////////////////////////////////////////////////////////////////////////
//
// TQtBrush creates the QBrush Qt object based on the ROOT "TAttFill" attributes 
//
/////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "TQtBrush.h"
#include "TGQt.h"
#include "qbitmap.h"
#include <QDebug>

//
//*-*- data to create fill area interior style
//

static uchar p1_bits[] = {
  (0xaa),(0xaa),(0x55),(0x55),(0xaa),(0xaa),(0x55),(0x55),(0xaa),(0xaa),(0x55),(0x55),
  (0xaa),(0xaa),(0x55),(0x55),(0xaa),(0xaa),(0x55),(0x55),(0xaa),(0xaa),(0x55),(0x55),
  (0xaa),(0xaa),(0x55),(0x55),(0xaa),(0xaa),(0x55),(0x55)};
static uchar p2_bits[] = {
  (0x44),(0x44),(0x11),(0x11),(0x44),(0x44),(0x11),(0x11),(0x44),(0x44),(0x11),(0x11),
  (0x44),(0x44),(0x11),(0x11),(0x44),(0x44),(0x11),(0x11),(0x44),(0x44),(0x11),(0x11),
  (0x44),(0x44),(0x11),(0x11),(0x44),(0x44),(0x11),(0x11)};
static uchar p3_bits[] = {
  (0x00),(0x00),(0x44),(0x44),(0x00),(0x00),(0x11),(0x11),(0x00),(0x00),(0x44),(0x44),
  (0x00),(0x00),(0x11),(0x11),(0x00),(0x00),(0x44),(0x44),(0x00),(0x00),(0x11),(0x11),
  (0x00),(0x00),(0x44),(0x44),(0x00),(0x00),(0x11),(0x11)};
static uchar p4_bits[] = {
  (0x80),(0x80),(0x40),(0x40),(0x20),(0x20),(0x10),(0x10),(0x08),(0x08),(0x04),(0x04),
  (0x02),(0x02),(0x01),(0x01),(0x80),(0x80),(0x40),(0x40),(0x20),(0x20),(0x10),(0x10),
  (0x08),(0x08),(0x04),(0x04),(0x02),(0x02),(0x01),(0x01)};
static uchar p5_bits[] = {
  (0x20),(0x20),(0x40),(0x40),(0x80),(0x80),(0x01),(0x01),(0x02),(0x02),(0x04),(0x04),
  (0x08),(0x08),(0x10),(0x10),(0x20),(0x20),(0x40),(0x40),(0x80),(0x80),(0x01),(0x01),
  (0x02),(0x02),(0x04),(0x04),(0x08),(0x08),(0x10),(0x10)};
static uchar p6_bits[] = {
  (0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),
  (0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),
  (0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44)};
static uchar p7_bits[] = {
  (0x00),(0x00),(0x00),(0x00),(0x00),(0x00),(0xff),(0xff),(0x00),(0x00),(0x00),(0x00),
  (0x00),(0x00),(0xff),(0xff),(0x00),(0x00),(0x00),(0x00),(0x00),(0x00),(0xff),(0xff),
  (0x00),(0x00),(0x00),(0x00),(0x00),(0x00),(0xff),(0xff)};
static uchar p8_bits[] = {
  (0x11),(0x11),(0xb8),(0xb8),(0x7c),(0x7c),(0x3a),(0x3a),(0x11),(0x11),(0xa3),(0xa3),
  (0xc7),(0xc7),(0x8b),(0x8b),(0x11),(0x11),(0xb8),(0xb8),(0x7c),(0x7c),(0x3a),(0x3a),
  (0x11),(0x11),(0xa3),(0xa3),(0xc7),(0xc7),(0x8b),(0x8b)};
static uchar p9_bits[] = {
  (0x10),(0x10),(0x10),(0x10),(0x28),(0x28),(0xc7),(0xc7),(0x01),(0x01),(0x01),(0x01),
  (0x82),(0x82),(0x7c),(0x7c),(0x10),(0x10),(0x10),(0x10),(0x28),(0x28),(0xc7),(0xc7),
  (0x01),(0x01),(0x01),(0x01),(0x82),(0x82),(0x7c),(0x7c)};
static uchar p10_bits[] = {
  (0x10),(0x10),(0x10),(0x10),(0x10),(0x10),(0xff),(0xff),(0x01),(0x01),(0x01),(0x01),
  (0x01),(0x01),(0xff),(0xff),(0x10),(0x10),(0x10),(0x10),(0x10),(0x10),(0xff),(0xff),
  (0x01),(0x01),(0x01),(0x01),(0x01),(0x01),(0xff),(0xff)};
static uchar p11_bits[] = {
  (0x08),(0x08),(0x49),(0x49),(0x2a),(0x2a),(0x1c),(0x1c),(0x2a),(0x2a),(0x49),(0x49),
  (0x08),(0x08),(0x00),(0x00),(0x80),(0x80),(0x94),(0x94),(0xa2),(0xa2),(0xc1),(0xc1),
  (0xa2),(0xa2),(0x94),(0x94),(0x80),(0x80),(0x00),(0x00)};
static uchar p12_bits[] = {
  (0x1c),(0x1c),(0x22),(0x22),(0x41),(0x41),(0x41),(0x41),(0x41),(0x41),(0x22),(0x22),
  (0x1c),(0x1c),(0x00),(0x00),(0xc1),(0xc1),(0x22),(0x22),(0x14),(0x14),(0x14),(0x14),
  (0x14),(0x14),(0x22),(0x22),(0xc1),(0xc1),(0x00),(0x00)};
static uchar p13_bits[] = {
  (0x01),(0x01),(0x82),(0x82),(0x44),(0x44),(0x28),(0x28),(0x10),(0x10),(0x28),(0x28),
  (0x44),(0x44),(0x82),(0x82),(0x01),(0x01),(0x82),(0x82),(0x44),(0x44),(0x28),(0x28),
  (0x10),(0x10),(0x28),(0x28),(0x44),(0x44),(0x82),(0x82)};
static uchar p14_bits[] = {
  (0xff),(0xff),(0x11),(0x10),(0x11),(0x10),(0x11),(0x10),(0xf1),(0x1f),(0x11),(0x11),
  (0x11),(0x11),(0x11),(0x11),(0xff),(0x11),(0x01),(0x11),(0x01),(0x11),(0x01),(0x11),
  (0xff),(0xff),(0x01),(0x10),(0x01),(0x10),(0x01),(0x10)};
static uchar p15_bits[] = {
  (0x22),(0x22),(0x55),(0x55),(0x22),(0x22),(0x00),(0x00),(0x88),(0x88),(0x55),(0x55),
  (0x88),(0x88),(0x00),(0x00),(0x22),(0x22),(0x55),(0x55),(0x22),(0x22),(0x00),(0x00),
  (0x88),(0x88),(0x55),(0x55),(0x88),(0x88),(0x00),(0x00)};
static uchar p16_bits[] = {
  (0x0e),(0x0e),(0x11),(0x11),(0xe0),(0xe0),(0x00),(0x00),(0x0e),(0x0e),(0x11),(0x11),
  (0xe0),(0xe0),(0x00),(0x00),(0x0e),(0x0e),(0x11),(0x11),(0xe0),(0xe0),(0x00),(0x00),
  (0x0e),(0x0e),(0x11),(0x11),(0xe0),(0xe0),(0x00),(0x00)};
static uchar p17_bits[] = {
  (0x44),(0x44),(0x22),(0x22),(0x11),(0x11),(0x00),(0x00),(0x44),(0x44),(0x22),(0x22),
  (0x11),(0x11),(0x00),(0x00),(0x44),(0x44),(0x22),(0x22),(0x11),(0x11),(0x00),(0x00),
  (0x44),(0x44),(0x22),(0x22),(0x11),(0x11),(0x00),(0x00)};
static uchar p18_bits[] = {
  (0x11),(0x11),(0x22),(0x22),(0x44),(0x44),(0x00),(0x00),(0x11),(0x11),(0x22),(0x22),
  (0x44),(0x44),(0x00),(0x00),(0x11),(0x11),(0x22),(0x22),(0x44),(0x44),(0x00),(0x00),
  (0x11),(0x11),(0x22),(0x22),(0x44),(0x44),(0x00),(0x00)};
static uchar p19_bits[] = {
  (0xe0),(0x03),(0x98),(0x0c),(0x84),(0x10),(0x42),(0x21),(0x42),(0x21),(0x21),(0x42),
  (0x19),(0x4c),(0x07),(0xf0),(0x19),(0x4c),(0x21),(0x42),(0x42),(0x21),(0x42),(0x21),
  (0x84),(0x10),(0x98),(0x0c),(0xe0),(0x03),(0x80),(0x00)};
static uchar p20_bits[] = {
  (0x22),(0x22),(0x11),(0x11),(0x11),(0x11),(0x11),(0x11),(0x22),(0x22),(0x44),(0x44),
  (0x44),(0x44),(0x44),(0x44),(0x22),(0x22),(0x11),(0x11),(0x11),(0x11),(0x11),(0x11),
  (0x22),(0x22),(0x44),(0x44),(0x44),(0x44),(0x44),(0x44)};
static uchar p21_bits[] = {
  (0xf1),(0xf1),(0x10),(0x10),(0x10),(0x10),(0x10),(0x10),(0x1f),(0x1f),(0x01),(0x01),
  (0x01),(0x01),(0x01),(0x01),(0xf1),(0xf1),(0x10),(0x10),(0x10),(0x10),(0x10),(0x10),
  (0x1f),(0x1f),(0x01),(0x01),(0x01),(0x01),(0x01),(0x01)};
static uchar p22_bits[] = {
  (0x8f),(0x8f),(0x08),(0x08),(0x08),(0x08),(0x08),(0x08),(0xf8),(0xf8),(0x80),(0x80),
  (0x80),(0x80),(0x80),(0x80),(0x8f),(0x8f),(0x08),(0x08),(0x08),(0x08),(0x08),(0x08),
  (0xf8),(0xf8),(0x80),(0x80),(0x80),(0x80),(0x80),(0x80)};
static uchar p23_bits[] = {
  (0xAA),(0xAA),(0x55),(0x55),(0x6a),(0x6a),(0x74),(0x74),(0x78),(0x78),(0x74),(0x74),
  (0x6a),(0x6a),(0x55),(0x55),(0xAA),(0xAA),(0x55),(0x55),(0x6a),(0x6a),(0x74),(0x74),
  (0x78),(0x78),(0x74),(0x74),(0x6a),(0x6a),(0x55),(0x55)};
static uchar p24_bits[] = {
  (0x80),(0x00),(0xc0),(0x00),(0xea),(0xa8),(0xd5),(0x54),(0xea),(0xa8),(0xd5),(0x54),
  (0xeb),(0xe8),(0xd5),(0xd4),(0xe8),(0xe8),(0xd4),(0xd4),(0xa8),(0xe8),(0x54),(0xd5),
  (0xa8),(0xea),(0x54),(0xd5),(0xfc),(0xff),(0xfe),(0xff)};
static uchar p25_bits[] = {
  (0x80),(0x00),(0xc0),(0x00),(0xe0),(0x00),(0xf0),(0x00),(0xff),(0xf0),(0xff),(0xf0),
  (0xfb),(0xf0),(0xf9),(0xf0),(0xf8),(0xf0),(0xf8),(0x70),(0xf8),(0x30),(0xff),(0xf0),
  (0xff),(0xf8),(0xff),(0xfc),(0xff),(0xfe),(0xff),(0xff)};


static uchar *patter_bits[]= { p1_bits, p2_bits,   p3_bits,  p4_bits,  p5_bits,
                               p6_bits, p7_bits,   p8_bits,  p9_bits, p10_bits,
                              p11_bits, p12_bits, p13_bits, p14_bits, p15_bits,
                              p16_bits, p17_bits, p18_bits, p19_bits, p20_bits,
                              p21_bits, p22_bits, p23_bits, p24_bits, p25_bits};

ClassImp(TQtBrush)
//______________________________________________________________________________
TQtBrush::TQtBrush(): QBrush(),fStyle(0),fFasi(0),fAlpha(255)
{}
//______________________________________________________________________________
TQtBrush::TQtBrush(const TAttFill &rootFillAttributes)
{
   // TQtBrush ctor from ROOT TAttFill object
   SetFillAttributes(rootFillAttributes);

}
//______________________________________________________________________________
TQtBrush::~TQtBrush() 
{
   // TQtBrush dtor
}

//______________________________________________________________________________
TQtBrush &TQtBrush::operator=(const TAttFill &rootFillAttributes)
{
   SetFillAttributes(rootFillAttributes);
   return *this;
}

//______________________________________________________________________________
void  TQtBrush::SetFillAttributes(const TAttFill &rootFillAttributes)
{
   SetColor(rootFillAttributes.GetFillColor());
   SetStyle(rootFillAttributes.GetFillStyle());
}

//______________________________________________________________________________
 void TQtBrush::SetColor(Color_t cindex)
 {
   // Set color index for to fill shapes
   //  cindex    : color index
    if (cindex >= 0)  SetColor(gQt->ColorIndex(gQt->UpdateColor(cindex)));
    else fAlpha = cindex;
 }
//______________________________________________________________________________
void TQtBrush::SetColor(const QColor &qtcolor)
{
   // remember the user's alpha value and set the  brush color
   fAlpha = qtcolor.alpha();
   fBackground = qtcolor;
   SetColorOwn();
}

//______________________________________________________________________________
void TQtBrush::SetColorOwn()
{
  // Set the brush color and adjust its alpha value from fStyle
  // Take in account the new transperency if needed

   static const int opaqAlpha = QColor(0,0,0).alpha(); // Qt   alpha range is  [0:255]
   static const float opaqFactor = opaqAlpha/100.;     // ROOT alpha range is  [0:100]
   if (fAlpha >=0 ) {
      int alpha = ( fStyle == 4) ? int(opaqFactor*fFasi) : fAlpha;
      if (fBackground.alpha() != alpha) fBackground.setAlpha(alpha);
      setColor(fBackground);
   }
}

//______________________________________________________________________________
void TQtBrush::SetStyle(int sty, int fasi)
{
//*-*-*-*-*-*-*-*-*-*-*Set fill area style index*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
//*-*                  =========================
//*-*  style   : fill area interior style hollow or solid
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
  fStyle =  sty;
  fFasi  =  fasi;

  switch( fStyle ) {

  case 0:
    setStyle(Qt::NoBrush);                          // hollow
    fBackground = Qt::transparent;
    fAlpha = 0;
    break;
  case 1:                                           // solid
    setStyle(Qt::SolidPattern);
    break;
  case 3:                                           // pattern
     {
        int pattern = 1;
        if (fasi > 0 && fasi < 26 ) pattern = fasi-1;
        QBitmap bm =  QBitmap::fromData(QSize(16,16),patter_bits[pattern]);
        setTexture(bm);
     }
    break;
  case 2:                                           // hatch
      switch (fasi)
        {
          case 1: setStyle(Qt::BDiagPattern);
                  break;
          case 2: setStyle(Qt::CrossPattern);
                  break;
          case 3: setStyle(Qt::DiagCrossPattern);
                  break;
          case 4: setStyle(Qt::FDiagPattern);
                  break;
          case 5: setStyle(Qt::HorPattern);
                  break;
          case 6: setStyle(Qt::VerPattern );
                  break;
         default: setStyle(Qt::FDiagPattern);
                  break;
        }
     break;
  case 4:                                      // transparent
     if (!fasi)    setStyle(Qt::NoBrush);      // the window is transparent
     else          setStyle(Qt::SolidPattern);
     break;
  default:                                          // solid  - default
      setStyle(Qt::SolidPattern);
      break;
  }
  SetColorOwn();
}
