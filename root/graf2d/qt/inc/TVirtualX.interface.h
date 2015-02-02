// @(#)root/base:$Id$
// Author: Valeri Fine   28/07/2004

//
//  This is a copy of the all "abstract" TVirtualX methods to make sure 
//  we don't miss it with the implementation
//
/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


public:

   virtual Bool_t    Init(void *display=0);
   virtual void      ClearWindow();
   virtual void      ClosePixmap();
   virtual void      CloseWindow();
   virtual void      CopyPixmap(Int_t wid, Int_t xpos, Int_t ypos);

   //TVirtualX has two versions of CreateOpenGLContext
   using TVirtualX::CreateOpenGLContext;

   //TVirtualX has these two methods with wchar_t
   using TVirtualX::DrawText;
   using TVirtualX::GetTextExtent;

   //TVirtualX now has versions with parameters also,
   //but TGQt defines only versions without parameters.
   using TVirtualX::GetFontAscent;
   using TVirtualX::GetFontDescent;
   
   virtual void      CreateOpenGLContext(Int_t wid=0);
   virtual void      DeleteOpenGLContext(Int_t wid=0);
   virtual void      DrawBox(Int_t x1, Int_t y1, Int_t x2, Int_t y2, EBoxMode mode);
   virtual void      DrawCellArray(Int_t x1, Int_t y1, Int_t x2, Int_t y2,
                                   Int_t nx, Int_t ny, Int_t *ic);
   virtual void      DrawFillArea(Int_t n, TPoint *xy);
   virtual void      DrawLine(Int_t x1, Int_t y1, Int_t x2, Int_t y2);
   virtual void      DrawPolyLine(Int_t n, TPoint *xy);
   virtual void      DrawPolyMarker(Int_t n, TPoint *xy);
   virtual void      DrawText(Int_t x, Int_t y, Float_t angle, Float_t mgn, const char *text,
                              ETextMode mode);
   virtual UInt_t    ExecCommand(TGWin32Command *code);
   virtual void      GetCharacterUp(Float_t &chupx, Float_t &chupy);
   virtual Int_t     GetDoubleBuffer(Int_t wid);
   virtual void      GetGeometry(Int_t wid, Int_t &x, Int_t &y, UInt_t &w, UInt_t &h);
   virtual const char *DisplayName(const char * = 0);
   virtual Handle_t  GetNativeEvent() const;
   virtual ULong_t   GetPixel(Color_t cindex);
   virtual void      GetPlanes(Int_t &nplanes);
   virtual void      GetRGB(Int_t index, Float_t &r, Float_t &g, Float_t &b);
   virtual void      GetTextExtent(UInt_t &w, UInt_t &h, char *mess);
   virtual Int_t     GetFontAscent()  const;
   virtual Int_t     GetFontDescent() const;
   virtual Float_t   GetTextMagnitude();
   virtual Window_t  GetWindowID(Int_t wid);
   virtual Bool_t    HasTTFonts() const;
   virtual Int_t     InitWindow(ULong_t window);
   virtual Int_t     AddWindow(ULong_t qwid, UInt_t w, UInt_t h);
   virtual void      RemoveWindow(ULong_t qwid);
   virtual void      MoveWindow(Int_t wid, Int_t x, Int_t y);
   virtual Int_t     OpenPixmap(UInt_t w, UInt_t h);
   virtual void      QueryPointer(Int_t &ix, Int_t &iy);
   virtual Pixmap_t  ReadGIF(Int_t x0, Int_t y0, const char *file, Window_t id=0);
   virtual Int_t     RequestLocator(Int_t mode, Int_t ctyp, Int_t &x, Int_t &y);
   virtual Int_t     RequestString(Int_t x, Int_t y, char *text);
   virtual void      RescaleWindow(Int_t wid, UInt_t w, UInt_t h);
   virtual Int_t     ResizePixmap(Int_t wid, UInt_t w, UInt_t h);
   virtual void      ResizeWindow(Int_t wid);
   virtual void      SelectWindow(Int_t wid);
   virtual void      SelectPixmap(Int_t qpixid);
   virtual void      SetCharacterUp(Float_t chupx, Float_t chupy);
   virtual void      SetClipOFF(Int_t wid);
   virtual void      SetClipRegion(Int_t wid, Int_t x, Int_t y, UInt_t w, UInt_t h);
   virtual void      SetCursor(Int_t win, ECursor cursor);
   virtual void      SetDoubleBuffer(Int_t wid, Int_t mode);
   virtual void      SetDoubleBufferOFF();
   virtual void      SetDoubleBufferON();
   virtual void      SetDrawMode(EDrawMode mode);
   virtual void      SetFillColor(Color_t cindex);
   virtual void      SetFillStyle(Style_t style);
   virtual void      SetLineColor(Color_t cindex);
   virtual void      SetLineType(Int_t n, Int_t *dash);
   virtual void      SetLineStyle(Style_t linestyle);
   virtual void      SetLineWidth(Width_t width);
   virtual void      SetMarkerColor(Color_t cindex);
   virtual void      SetMarkerSize(Float_t markersize);
   virtual void      SetMarkerStyle(Style_t markerstyle);
   virtual void      SetOpacity(Int_t percent);
   virtual void      SetRGB(Int_t cindex, Float_t r, Float_t g, Float_t b);
   virtual void      SetTextAlign(Short_t talign=11);
   virtual void      SetTextColor(Color_t cindex);
   virtual Int_t     SetTextFont(char *fontname, ETextSetMode mode);
   virtual void      SetTextFont(Font_t fontnumber);
   virtual void      SetTextMagnitude(Float_t mgn);
   virtual void      SetTextSize(Float_t textsize);
   virtual void      UpdateWindow(Int_t mode);
   virtual void      Warp(Int_t ix, Int_t iy){ Warp(ix,iy,0); }
   virtual void      Warp(Int_t ix, Int_t iy, Window_t id);
   virtual Int_t     WriteGIF(char *name);
   virtual void      WritePixmap(Int_t wid, UInt_t w, UInt_t h, char *pxname);

   //---- Methods used for GUI -----
   virtual void         GetWindowAttributes(Window_t id, WindowAttributes_t &attr);
   virtual void         MapWindow(Window_t id);
   virtual void         MapSubwindows(Window_t id);
   virtual void         MapRaised(Window_t id);
   virtual void         UnmapWindow(Window_t id);
   virtual void         DestroyWindow(Window_t id);
   virtual void         RaiseWindow(Window_t id);
   virtual void         LowerWindow(Window_t id);
   virtual void         MoveWindow(Window_t id, Int_t x, Int_t y);
   virtual void         MoveResizeWindow(Window_t id, Int_t x, Int_t y, UInt_t w, UInt_t h);
   virtual void         ResizeWindow(Window_t id, UInt_t w, UInt_t h);
   virtual void         IconifyWindow(Window_t id);
   virtual Bool_t       NeedRedraw(ULong_t tgwindow, Bool_t force);
   virtual void         ReparentWindow(Window_t id, Window_t pid, Int_t x, Int_t y);
   virtual void         SetWindowBackground(Window_t id, ULong_t color);
   virtual void         SetWindowBackgroundPixmap(Window_t id, Pixmap_t pxm);
   virtual Window_t     CreateWindow(Window_t parent, Int_t x, Int_t y,
                                     UInt_t w, UInt_t h, UInt_t border,
                                     Int_t depth, UInt_t clss,
                                     void *visual, SetWindowAttributes_t *attr,
                                     UInt_t wtype);
   virtual Int_t        OpenDisplay(const char *dpyName);
   virtual void         CloseDisplay();
   virtual Display_t    GetDisplay() const;
   virtual Visual_t     GetVisual() const;
   virtual Int_t        GetScreen() const;
   virtual Int_t        GetDepth() const;
   virtual Colormap_t   GetColormap() const;
   virtual Atom_t       InternAtom(const char *atom_name, Bool_t only_if_exist);
   virtual Window_t     GetDefaultRootWindow() const;
   virtual Window_t     GetParent(Window_t id) const;
   virtual FontStruct_t LoadQueryFont(const char *font_name);
   virtual FontH_t      GetFontHandle(FontStruct_t fs);
   virtual void         DeleteFont(FontStruct_t fs);
   virtual GContext_t   CreateGC(Drawable_t id, GCValues_t *gval);
   virtual void         ChangeGC(GContext_t gc, GCValues_t *gval);
   virtual void         CopyGC(GContext_t org, GContext_t dest, Mask_t mask);
   virtual void         DeleteGC(GContext_t gc);
   virtual Cursor_t     CreateCursor(ECursor cursor);
   virtual void         SetCursor(Window_t id, Cursor_t curid);
   virtual Pixmap_t     CreatePixmap(Drawable_t id, UInt_t w, UInt_t h);
   virtual Pixmap_t     CreatePixmap(Drawable_t id, const char *bitmap, UInt_t width,
                                     UInt_t height, ULong_t forecolor, ULong_t backcolor,
                                     Int_t depth);
   virtual Pixmap_t     CreateBitmap(Drawable_t id, const char *bitmap,
                                     UInt_t width, UInt_t height);
   virtual void         DeletePixmap(Pixmap_t pmap);
   virtual Bool_t       CreatePictureFromFile(Drawable_t id, const char *filename,
                                              Pixmap_t &pict, Pixmap_t &pict_mask,
                                              PictureAttributes_t &attr);
   virtual Bool_t       CreatePictureFromData(Drawable_t id, char **data,
                                              Pixmap_t &pict, Pixmap_t &pict_mask,
                                              PictureAttributes_t &attr);
   virtual Bool_t       ReadPictureDataFromFile(const char *filename, char ***ret_data);
   virtual void         DeletePictureData(void *data);
   virtual void         SetDashes(GContext_t gc, Int_t offset, const char *dash_list,
                                  Int_t n);
   virtual Bool_t       ParseColor(Colormap_t cmap, const char *cname, ColorStruct_t &color);
   virtual Bool_t       AllocColor(Colormap_t cmap, ColorStruct_t &color);
   virtual void         QueryColor(Colormap_t cmap, ColorStruct_t &color);
   virtual void         FreeColor(Colormap_t cmap, ULong_t pixel);
   virtual Int_t        EventsPending();
   virtual void         NextEvent(Event_t &event);
   virtual void         Bell(Int_t percent);
   virtual void         CopyArea(Drawable_t src, Drawable_t dest, GContext_t gc,
                                 Int_t src_x, Int_t src_y, UInt_t width,
                                 UInt_t height, Int_t dest_x, Int_t dest_y);
   virtual void         ChangeWindowAttributes(Window_t id, SetWindowAttributes_t *attr);
   virtual void         ChangeProperty(Window_t id, Atom_t property, Atom_t type,
                                       UChar_t *data, Int_t len);
   virtual void         DrawLine(Drawable_t id, GContext_t gc, Int_t x1, Int_t y1, Int_t x2, Int_t y2);
   virtual void         ClearArea(Window_t id, Int_t x, Int_t y, UInt_t w, UInt_t h);
   virtual Bool_t       CheckEvent(Window_t id, EGEventType type, Event_t &ev);
   virtual void         SendEvent(Window_t id, Event_t *ev);
   virtual void         WMDeleteNotify(Window_t id);
   virtual void         SetKeyAutoRepeat(Bool_t on = kTRUE);
   virtual void         GrabKey(Window_t id, Int_t keycode, UInt_t modifier, Bool_t grab = kTRUE);
   virtual void         GrabButton(Window_t id, EMouseButton button, UInt_t modifier,
                                   UInt_t evmask, Window_t confine, Cursor_t cursor,
                                   Bool_t grab = kTRUE);
   virtual void         GrabPointer(Window_t id, UInt_t evmask, Window_t confine,
                                    Cursor_t cursor, Bool_t grab = kTRUE,
                                    Bool_t owner_events = kTRUE);
   virtual void         SetWindowName(Window_t id, char *name);
   virtual void         SetIconName(Window_t id, char *name);
   virtual void         SetIconPixmap(Window_t id, Pixmap_t pix);
   virtual void         SetClassHints(Window_t id, char *className, char *resourceName);
   virtual void         SetMWMHints(Window_t id, UInt_t value, UInt_t funcs, UInt_t input);
   virtual void         SetWMPosition(Window_t id, Int_t x, Int_t y);
   virtual void         SetWMSize(Window_t id, UInt_t w, UInt_t h);
   virtual void         SetWMSizeHints(Window_t id, UInt_t wmin, UInt_t hmin,
                                       UInt_t wmax, UInt_t hmax, UInt_t winc, UInt_t hinc);
   virtual void         SetWMState(Window_t id, EInitialState state);
   virtual void         SetWMTransientHint(Window_t id, Window_t main_id);
   virtual void         DrawString(Drawable_t id, GContext_t gc, Int_t x, Int_t y,
                                   const char *s, Int_t len);
   virtual Int_t        TextWidth(FontStruct_t font, const char *s, Int_t len);
   virtual void         GetFontProperties(FontStruct_t font, Int_t &max_ascent, Int_t &max_descent);
   virtual void         GetGCValues(GContext_t gc, GCValues_t &gval);
   virtual FontStruct_t GetFontStruct(FontH_t fh);
   virtual void         FreeFontStruct(FontStruct_t fs);
   virtual void         ClearWindow(Window_t id);
   virtual Int_t        KeysymToKeycode(UInt_t keysym);
   virtual void         FillRectangle(Drawable_t id, GContext_t gc, Int_t x, Int_t y,
                                      UInt_t w, UInt_t h);
   virtual void         DrawRectangle(Drawable_t id, GContext_t gc, Int_t x, Int_t y,
                                      UInt_t w, UInt_t h);
   virtual void         DrawSegments(Drawable_t id, GContext_t gc, Segment_t *seg, Int_t nseg);
   virtual void         SelectInput(Window_t id, UInt_t evmask);
   virtual Window_t     GetInputFocus();
   virtual void         SetInputFocus(Window_t id);
   virtual Window_t     GetPrimarySelectionOwner();
   virtual void         SetPrimarySelectionOwner(Window_t id);
   virtual void         ConvertPrimarySelection(Window_t id, Atom_t clipboard, Time_t when);
   virtual void         LookupString(Event_t *event, char *buf, Int_t buflen, UInt_t &keysym);
   virtual void         GetPasteBuffer(Window_t id, Atom_t atom, TString &text, Int_t &nchar,
                                       Bool_t del);
   virtual void         TranslateCoordinates(Window_t src, Window_t dest, Int_t src_x,Int_t src_y,
                                             Int_t &dest_x, Int_t &dest_y, Window_t &child);
   virtual void         GetWindowSize(Drawable_t id, Int_t &x, Int_t &y, UInt_t &w, UInt_t &h);
   virtual void         FillPolygon(Window_t id, GContext_t gc, Point_t *points, Int_t npnt);
   virtual void         QueryPointer(Window_t id, Window_t &rootw, Window_t &childw,
                                     Int_t &root_x, Int_t &root_y, Int_t &win_x,
                                     Int_t &win_y, UInt_t &mask);
   virtual void         SetBackground(GContext_t gc, ULong_t background);
   virtual void         SetForeground(GContext_t gc, ULong_t foreground);
   virtual void         SetClipRectangles(GContext_t gc, Int_t x, Int_t y, Rectangle_t *recs, Int_t n);
   virtual void         Update(Int_t mode = 0);
   virtual Region_t     CreateRegion();
   virtual void         DestroyRegion(Region_t reg);
   virtual void         UnionRectWithRegion(Rectangle_t *rect, Region_t src, Region_t dest);
   virtual Region_t     PolygonRegion(Point_t *points, Int_t np, Bool_t winding);
   virtual void         UnionRegion(Region_t rega, Region_t regb, Region_t result);
   virtual void         IntersectRegion(Region_t rega, Region_t regb, Region_t result);
   virtual void         SubtractRegion(Region_t rega, Region_t regb, Region_t result);
   virtual void         XorRegion(Region_t rega, Region_t regb, Region_t result);
   virtual Bool_t       EmptyRegion(Region_t reg);
   virtual Bool_t       PointInRegion(Int_t x, Int_t y, Region_t reg);
   virtual Bool_t       EqualRegion(Region_t rega, Region_t regb);
   virtual void         GetRegionBox(Region_t reg, Rectangle_t *rect);
// !!!!!!!!   virtual char       **ListFonts(char *fontname, Int_t max, Int_t &count);
   virtual char       **ListFonts(const char *fontname, Int_t max, Int_t &count);
   virtual void         FreeFontNames(char **fontlist);
   virtual Drawable_t   CreateImage(UInt_t width, UInt_t height);
   virtual void         GetImageSize(Drawable_t id, UInt_t &width, UInt_t &height);
   virtual void         PutPixel(Drawable_t id, Int_t x, Int_t y, ULong_t pixel);
   virtual void         PutImage(Drawable_t id, GContext_t gc, Drawable_t img,
                                 Int_t dx, Int_t dy, Int_t x, Int_t y,
                                 UInt_t w, UInt_t h);
   virtual void         DeleteImage(Drawable_t img);
//  The method to support ASImage (by V.Onuchine
   unsigned char *GetColorBits(Drawable_t wid, Int_t x = 0, Int_t y = 0, UInt_t w = 0, UInt_t h = 0);
   virtual  Pixmap_t    CreatePixmapFromData(unsigned char *bits, UInt_t width, UInt_t height);
   virtual  Window_t    GetCurrentWindow() const;

   virtual Int_t  SupportsExtension(const char *ext) const;

   //---- Drag and Drop -----
   virtual void         DeleteProperty(Window_t, Atom_t&);
   virtual Int_t        GetProperty(Window_t, Atom_t, Long_t, Long_t, Bool_t, Atom_t,
                                    Atom_t*, Int_t*, ULong_t*, ULong_t*, unsigned char**);
   virtual void         ChangeActivePointerGrab(Window_t, UInt_t, Cursor_t);
   virtual void         ConvertSelection(Window_t, Atom_t&, Atom_t&, Atom_t&, Time_t&);
   virtual Bool_t       SetSelectionOwner(Window_t, Atom_t&);
   virtual void         ChangeProperties(Window_t id, Atom_t property, Atom_t type,
                                         Int_t format, UChar_t *data, Int_t len);
   virtual void         SetDNDAware(Window_t, Atom_t *);
   virtual void         SetTypeList(Window_t win, Atom_t prop, Atom_t *typelist);
   virtual Window_t     FindRWindow(Window_t win, Window_t dragwin, Window_t input, int x, int y, int maxd);
   virtual Bool_t       IsDNDAware(Window_t win, Atom_t *typelist);

#if 0 
//   Alas CINT does not understand the complex CPP statement below.
//   This forces us to edit the file "by hand" (V.Fine 01/03/2005)
//   #if  ROOT_VERSION_CODE < ROOT_VERSION(4,01,02)
   
   //---- Methods used for OpenGL -----
   virtual Window_t     CreateGLWindow(Window_t wind, Visual_t visual = 0, Int_t depth = 0);
   virtual ULong_t      GetWinDC(Window_t wind);
   virtual ULong_t      wglCreateContext(Window_t wind);
   virtual void         wglDeleteContext(ULong_t ctx);
   virtual void         wglMakeCurrent(Window_t wind, ULong_t ctx);
   virtual void         wglSwapLayerBuffers(Window_t wind, UInt_t mode);
   virtual void         glViewport(Int_t x0, Int_t y0, Int_t x1, Int_t y1);
   virtual void         glClearIndex(Float_t fParam);
   virtual void         glClearColor(Float_t red, Float_t green, Float_t blue, Float_t alpha);
   virtual void         glDrawBuffer(UInt_t mode);
   virtual void         glClear(UInt_t mode);
   virtual void         glDisable(UInt_t mode);
   virtual void         glEnable(UInt_t mode);
   virtual void         glFlush();
   virtual void         glFrontFace(UInt_t mode);
   virtual void         glNewList(UInt_t list, UInt_t mode);
   virtual void         glGetBooleanv(UInt_t mode, UChar_t *bRet);
   virtual void         glGetDoublev(UInt_t mode, Double_t *dRet);
   virtual void         glGetFloatv(UInt_t mode, Float_t *fRet);
   virtual void         glGetIntegerv(UInt_t mode, Int_t *iRet);
   virtual Int_t        glGetError();
   virtual void         glEndList();
   virtual void         glBegin(UInt_t mode);
   virtual void         glEnd();
   virtual void         glPushMatrix();
   virtual void         glPopMatrix();
   virtual void         glRotated(Double_t angle, Double_t x, Double_t y, Double_t z);
   virtual void         glTranslated(Double_t x, Double_t y, Double_t z);
   virtual void         glMultMatrixd(const Double_t *matrix);
   virtual void         glColor3fv(const Float_t *color);
   virtual void         glVertex3f(Float_t x, Float_t y, Float_t z);
   virtual void         glVertex3fv(const Float_t *vert);
   virtual void         glIndexi(Int_t index);
   virtual void         glPointSize(Float_t size);
   virtual void         glLineWidth(Float_t width);
   virtual void         glDeleteLists(UInt_t list, Int_t sizei);
   virtual UInt_t       glGenLists(UInt_t list);
   virtual void         glCallList(UInt_t list);
   virtual void         glMatrixMode(UInt_t mode);
   virtual void         glLoadIdentity();
   virtual void         glFrustum(Double_t min_0, Double_t max_0, Double_t min_1,
                                  Double_t max_1, Double_t dnear, Double_t dfar);
   virtual void         glOrtho(Double_t min_0, Double_t max_0, Double_t min_1,
                                Double_t max_1, Double_t dnear, Double_t dfar);
   virtual void         glCullFace(UInt_t mode);
   virtual void         glPolygonMode(UInt_t face, UInt_t mode);
   virtual void         glLoadMatrixd(const Double_t *matrix);
   virtual void         glShadeModel(UInt_t mode);
   virtual void         glNormal3fv(const Float_t *norm);
#endif
