// @(#)root/pyroot:$Id$
// Author: Wim Lavrijsen, Jan 2005

// Bindings
#include "PyROOT.h"
#include "PyStrings.h"
#include "Converters.h"
#include "ObjectProxy.h"
#include "PyBufferFactory.h"
#include "TCustomPyTypes.h"
#include "Utility.h"
#include "RootWrapper.h"

// ROOT
#include "TClass.h"
#include "TClassEdit.h"

// CINT
#include "Api.h"

// Standard
#include <limits.h>
#include <string.h>
#include <utility>
#include <sstream>


//- data ______________________________________________________________________
namespace PyROOT {
   ConvFactories_t gConvFactories;
   R__EXTERN PyObject* gNullPtrObject;
}


//- base converter implementation ---------------------------------------------
PyObject* PyROOT::TConverter::FromMemory( void* )
{
// could happen if no derived class override
   PyErr_SetString( PyExc_TypeError, "unknown type can not be converted from memory" );
   return 0;
}

//_____________________________________________________________________________
Bool_t PyROOT::TConverter::ToMemory( PyObject*, void* )
{
// could happen if no derived class override
   PyErr_SetString( PyExc_TypeError, "unknown type can not be converted to memory" );
   return kFALSE;
}


//- helper macro's ------------------------------------------------------------
#define PYROOT_IMPLEMENT_BASIC_CONVERTER( name, type, stype, F1, F2 )         \
PyObject* PyROOT::T##name##Converter::FromMemory( void* address )             \
{                                                                             \
   return F1( (stype)*((type*)address) );                                     \
}                                                                             \
                                                                              \
Bool_t PyROOT::T##name##Converter::ToMemory( PyObject* value, void* address ) \
{                                                                             \
   type s = (type)F2( value );                                                \
   if ( s == (type)-1 && PyErr_Occurred() )                                   \
      return kFALSE;                                                          \
   *((type*)address) = (type)s;                                               \
   return kTRUE;                                                              \
}

#define PYROOT_IMPLEMENT_BASIC_REF_CONVERTER( name )                          \
PyObject* PyROOT::T##name##Converter::FromMemory( void* )                     \
{                                                                             \
   return 0;                                                                  \
}                                                                             \
                                                                              \
Bool_t PyROOT::T##name##Converter::ToMemory( PyObject*, void* )               \
{                                                                             \
   return kFALSE;                                                             \
}


//_____________________________________________________________________________
#define PYROOT_IMPLEMENT_BASIC_CHAR_CONVERTER( name, type, low, high )        \
Bool_t PyROOT::T##name##Converter::SetArg(                                    \
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )     \
{                                                                             \
/* convert <pyobject> to C++ <<type>>, set arg for call, allow int -> char */ \
   if ( PyROOT_PyUnicode_Check( pyobject ) ) {                                \
      if ( PyROOT_PyUnicode_GET_SIZE( pyobject ) == 1 ) {                     \
         para.fLong = (Long_t)PyROOT_PyUnicode_AsString( pyobject )[0];       \
         if ( func )                                                          \
            func->SetArg( para.fLong );                                       \
      } else {                                                                \
         PyErr_Format( PyExc_TypeError,                                       \
            #type" expected, got string of size " PY_SSIZE_T_FORMAT, PyROOT_PyUnicode_GET_SIZE( pyobject ) );\
         return kFALSE;                                                       \
      }                                                                       \
   } else {                                                                   \
      para.fLong = PyLong_AsLong( pyobject );                                 \
      if ( para.fLong == -1 && PyErr_Occurred() ) {                           \
         return kFALSE;                                                       \
      } else if ( ! ( low <= para.fLong && para.fLong <= high ) ) {           \
         PyErr_Format( PyExc_ValueError,                                      \
            "integer to character: value %ld not in range [%d,%d]", para.fLong, low, high );\
         return kFALSE;                                                       \
      } else if ( func )                                                      \
         func->SetArg( para.fLong );                                          \
   }                                                                          \
   return kTRUE;                                                              \
}                                                                             \
                                                                              \
PyObject* PyROOT::T##name##Converter::FromMemory( void* address )             \
{                                                                             \
   return PyROOT_PyUnicode_FromFormat( "%c", *((type*)address) );             \
}                                                                             \
                                                                              \
Bool_t PyROOT::T##name##Converter::ToMemory( PyObject* value, void* address ) \
{                                                                             \
   if ( PyROOT_PyUnicode_Check( value ) ) {                                   \
      const char* buf = PyROOT_PyUnicode_AsString( value );                   \
      if ( PyErr_Occurred() )                                                 \
         return kFALSE;                                                       \
      int len = PyROOT_PyUnicode_GET_SIZE( value );                           \
      if ( len != 1 ) {                                                       \
         PyErr_Format( PyExc_TypeError, #type" expected, got string of size %d", len );\
         return kFALSE;                                                       \
      }                                                                       \
      *((type*)address) = (type)buf[0];                                       \
   } else {                                                                   \
      Long_t l = PyLong_AsLong( value );                                      \
      if ( l == -1 && PyErr_Occurred() )                                      \
         return kFALSE;                                                       \
      if ( ! ( low <= l && l <= high ) ) {                                    \
         PyErr_Format( PyExc_ValueError, \
            "integer to character: value %ld not in range [%d,%d]", l, low, high );\
         return kFALSE;                                                       \
      }                                                                       \
      *((type*)address) = (type)l;                                            \
   }                                                                          \
   return kTRUE;                                                              \
}


//- converters for built-ins --------------------------------------------------
Bool_t PyROOT::TLongConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ long, set arg for call
#if PY_VERSION_HEX >= 0x02070000
// p2.7 silently converts floats to long ...
   if ( ! (PyLong_Check( pyobject ) || PyInt_Check( pyobject )) )
      return kFALSE;
#endif
   para.fLong = PyLong_AsLong( pyobject );
   if ( para.fLong == -1 && PyErr_Occurred() )
      return kFALSE;
   else if ( func )
      func->SetArg( para.fLong );
   return kTRUE;
}

PYROOT_IMPLEMENT_BASIC_CONVERTER( Long, Long_t, Long_t, PyLong_FromLong, PyLong_AsLong )

//____________________________________________________________________________
Bool_t PyROOT::TLongRefConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ long&, set arg for call
   if ( ! TCustomInt_CheckExact( pyobject ) ) {
      if ( PyInt_Check( pyobject ) )
         PyErr_SetString( PyExc_TypeError, "use ROOT.Long for pass-by-ref of longs" );
      return kFALSE;
   }

#if PY_VERSION_HEX < 0x03000000
   para.fLong = (Long_t)&((PyIntObject*)pyobject)->ob_ival;
   if ( func )
      func->SetArgRef( (Long_t&)((PyIntObject*)pyobject)->ob_ival );
   return kTRUE;
#else
   para.fLong = 0; func = 0;
   return (Bool_t)func; // there no longer is a PyIntObject in p3
#endif
}

PYROOT_IMPLEMENT_BASIC_REF_CONVERTER( LongRef )

//____________________________________________________________________________
Bool_t PyROOT::TConstLongRefConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ const long&, set arg for call using buffer
   para.fLong = fBuffer = PyLong_AsLong( pyobject );
   if ( para.fLong == -1 && PyErr_Occurred() )
      return kFALSE;
   else if ( func )
      func->SetArgRef( fBuffer );
   return kTRUE;
}

//____________________________________________________________________________
Bool_t PyROOT::TIntRefConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ (pseudo)int&, set arg for call
   if ( TCustomInt_CheckExact( pyobject ) ) {
#if PY_VERSION_HEX < 0x03000000
      para.fLong = (Long_t)&((PyIntObject*)pyobject)->ob_ival;
      if ( func ) {
         G__value v;
         G__setnull( &v );
         v.ref = (Long_t)&((PyIntObject*)pyobject)->ob_ival;
         G__letint( &v, 'i', para.fLong );
         func->SetArg( v );
      }

      return kTRUE;
#else
      para.fLong = 0; func = 0;
      PyErr_SetString( PyExc_NotImplementedError, "int pass-by-ref not implemented in p3" );
      return kFALSE; // there no longer is a PyIntObject in p3
#endif
   }

// alternate, pass pointer from buffer
   int buflen = Utility::GetBuffer( pyobject, 'i', sizeof(int), para.fVoidp );
   if ( para.fVoidp && buflen && func ) {
      G__value v;
      G__setnull( &v );
      v.ref = (Long_t)para.fVoidp;
      G__letint( &v, 'i', para.fLong );
      func->SetArg( v );
      return kTRUE;
   }
 
   PyErr_SetString( PyExc_TypeError, "use ROOT.Long for pass-by-ref of ints" );
   return kFALSE;
}

PYROOT_IMPLEMENT_BASIC_REF_CONVERTER( IntRef )

//____________________________________________________________________________
Bool_t PyROOT::TBoolConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ bool, allow int/long -> bool, set arg for call
   para.fLong = PyLong_AsLong( pyobject );
   if ( ! ( para.fLong == 0 || para.fLong == 1 ) ) {
      PyErr_SetString( PyExc_TypeError, "boolean value should be bool, or integer 1 or 0" );
      return kFALSE;
   }

   if ( func )
      func->SetArg( para.fLong );
   return kTRUE;
}

PYROOT_IMPLEMENT_BASIC_CONVERTER( Bool, Bool_t, Long_t, PyInt_FromLong, PyInt_AsLong )

//____________________________________________________________________________
PYROOT_IMPLEMENT_BASIC_CHAR_CONVERTER( Char,  Char_t,  CHAR_MIN, CHAR_MAX  )
PYROOT_IMPLEMENT_BASIC_CHAR_CONVERTER( UChar, UChar_t,        0, UCHAR_MAX )

//____________________________________________________________________________
PYROOT_IMPLEMENT_BASIC_CONVERTER( Short,  Short_t,  Long_t, PyInt_FromLong,  PyInt_AsLong )
PYROOT_IMPLEMENT_BASIC_CONVERTER( UShort, UShort_t, Long_t, PyInt_FromLong,  PyInt_AsLong )
PYROOT_IMPLEMENT_BASIC_CONVERTER( Int,    Int_t,    Long_t, PyInt_FromLong,  PyInt_AsLong )

//____________________________________________________________________________
Bool_t PyROOT::TULongConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ unsigned long, set arg for call
   para.fULong = PyLongOrInt_AsULong( pyobject );
   if ( PyErr_Occurred() )
      return kFALSE;
   else if ( func )
      func->SetArg( para.fULong );
   return kTRUE;
}

PyObject* PyROOT::TULongConverter::FromMemory( void* address )
{
// construct python object from C++ unsigned long read at <address>
   return PyLong_FromUnsignedLong( *((ULong_t*)address) );
}

Bool_t PyROOT::TULongConverter::ToMemory( PyObject* value, void* address )
{
// convert <value> to C++ unsigned long, write it at <address>
   ULong_t u = PyLongOrInt_AsULong( value );
   if ( PyErr_Occurred() )
      return kFALSE;
   *((ULong_t*)address) = u;
   return kTRUE;
}

//____________________________________________________________________________
PyObject* PyROOT::TUIntConverter::FromMemory( void* address )
{
// construct python object from C++ unsigned int read at <address>
   return PyLong_FromUnsignedLong( *((UInt_t*)address) );
}

Bool_t PyROOT::TUIntConverter::ToMemory( PyObject* value, void* address )
{
// convert <value> to C++ unsigned int, write it at <address>
   ULong_t u = PyLongOrInt_AsULong( value );
   if ( PyErr_Occurred() )
      return kFALSE;

   if ( u > (ULong_t)UINT_MAX ) {
      PyErr_SetString( PyExc_OverflowError, "value too large for unsigned int" );
      return kFALSE;
   }

   *((UInt_t*)address) = (UInt_t)u;
   return kTRUE;
}

//____________________________________________________________________________
Bool_t PyROOT::TDoubleConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ double, set arg for call
   para.fDouble = PyFloat_AsDouble( pyobject );
   if ( para.fDouble == -1.0 && PyErr_Occurred() )
      return kFALSE;
   else if ( func )
      func->SetArg( para.fDouble );
   return kTRUE;
}

PYROOT_IMPLEMENT_BASIC_CONVERTER( Float,      Float_t,      Double_t,     PyFloat_FromDouble, PyFloat_AsDouble )
PYROOT_IMPLEMENT_BASIC_CONVERTER( Double,     Double_t,     Double_t,     PyFloat_FromDouble, PyFloat_AsDouble )
PYROOT_IMPLEMENT_BASIC_CONVERTER( LongDouble, LongDouble_t, LongDouble_t, PyFloat_FromDouble, PyFloat_AsDouble )

//____________________________________________________________________________
Bool_t PyROOT::TDoubleRefConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ double&, set arg for call
   if ( TCustomFloat_CheckExact( pyobject ) ) {
      para.fLong = (Long_t)&((PyFloatObject*)pyobject)->ob_fval;
      if ( func ) {
         func->SetArgRef( ((PyFloatObject*)pyobject)->ob_fval );
         return kTRUE;
      }
   }

// alternate, pass pointer from buffer
   int buflen = Utility::GetBuffer( pyobject, 'd', sizeof(double), para.fVoidp );
   if ( para.fVoidp && buflen && func ) {
      func->SetArgRef( *(double*)para.fVoidp );
      return kTRUE;
   }

   PyErr_SetString( PyExc_TypeError, "use ROOT.Double for pass-by-ref of doubles" );
   return kFALSE;
}

PYROOT_IMPLEMENT_BASIC_REF_CONVERTER( DoubleRef )

//____________________________________________________________________________
Bool_t PyROOT::TConstDoubleRefConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ const double&, set arg for call using buffer
   para.fDouble = fBuffer = PyFloat_AsDouble( pyobject );
   if ( para.fDouble == -1.0 && PyErr_Occurred() )
      return kFALSE;
   else if ( func )
      func->SetArgRef( fBuffer );
   return kTRUE;
}

//____________________________________________________________________________
Bool_t PyROOT::TVoidConverter::SetArg( PyObject*, TParameter_t&, G__CallFunc*, Long_t )
{
// can't happen (unless a type is mapped wrongly), but implemented for completeness
   PyErr_SetString( PyExc_SystemError, "void/unknown arguments can\'t be set" );
   return kFALSE;
}

//____________________________________________________________________________
Bool_t PyROOT::TMacroConverter::SetArg( PyObject*, TParameter_t&, G__CallFunc*, Long_t )
{
// C++ macro's are not acceptable function args (but their values could be)
   PyErr_SetString( PyExc_SystemError, "macro arguments can\'t be set" );
   return kFALSE;
}

PyObject* PyROOT::TMacroConverter::FromMemory( void* address )
{
// no info available from ROOT/meta; go directly to CINT for the type info
   G__DataMemberInfo dmi;
   while ( dmi.Next() ) {    // using G__ClassInfo().GetDataMember() would cause overwrite

      if ( (Long_t)address == dmi.Offset() ) {
      // for now, only handle int, double, and C-string
         switch ( dmi.Type()->Type() ) {
         case 'p':
            return PyInt_FromLong( (Long_t) *(Int_t*)address );
         case 'P':
            return PyFloat_FromDouble( (double) *(Double_t*)address );
         case 'T':
            return PyROOT_PyUnicode_FromString( *(char**)address );
         default:
         // type unknown/not implemented
            PyErr_SetString( PyExc_NotImplementedError, "macro value could not be converted" );
            return 0;
         }
      }
   }

// type unknown/not implemented
   PyErr_SetString( PyExc_AttributeError, "requested macro not found" );
   return 0;
}

//____________________________________________________________________________
Bool_t PyROOT::TLongLongConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ long long, set arg for call

   if ( PyFloat_Check( pyobject ) ) {
   // special case: float implements nb_int, but allowing rounding conversions
   // interferes with overloading
      PyErr_SetString( PyExc_ValueError, "can not convert float to long long" );
      return kFALSE;
   }

   para.fLongLong = PyLong_AsLongLong( pyobject );
   if ( PyErr_Occurred() )
      return kFALSE;
   else if ( func )
      func->SetArg( para.fLongLong );
   return kTRUE;
}

PyObject* PyROOT::TLongLongConverter::FromMemory( void* address )
{
// construct python object from C++ long long read at <address>
   return PyLong_FromLongLong( *(Long64_t*)address );
}

Bool_t PyROOT::TLongLongConverter::ToMemory( PyObject* value, void* address )
{
// convert <value> to C++ long long, write it at <address>
   Long64_t ll = PyLong_AsLongLong( value );
   if ( ll == -1 && PyErr_Occurred() )
      return kFALSE;
   *((Long64_t*)address) = ll;
   return kTRUE;
}

//____________________________________________________________________________
Bool_t PyROOT::TULongLongConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ unsigned long long, set arg for call
   para.fULongLong = PyLongOrInt_AsULong64( pyobject );
   if ( PyErr_Occurred() )
      return kFALSE;
   else if ( func )
      func->SetArg( para.fULongLong );
   return kTRUE;
}

PyObject* PyROOT::TULongLongConverter::FromMemory( void* address )
{
// construct python object from C++ unsigned long long read at <address>
   return PyLong_FromUnsignedLongLong( *(ULong64_t*)address );
}

Bool_t PyROOT::TULongLongConverter::ToMemory( PyObject* value, void* address )
{
// convert <value> to C++ unsigned long long, write it at <address>
   Long64_t ull = PyLongOrInt_AsULong64( value );
   if ( PyErr_Occurred() )
      return kFALSE;
   *((ULong64_t*)address) = ull;
   return kTRUE;
}

//____________________________________________________________________________
Bool_t PyROOT::TCStringConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// construct a new string and copy it in new memory
   const char* s = PyROOT_PyUnicode_AsStringChecked( pyobject );
   if ( PyErr_Occurred() )
      return kFALSE;

   fBuffer = std::string( s, PyROOT_PyUnicode_GET_SIZE( pyobject ) );
   para.fVoidp = (void*)fBuffer.c_str();

// verify (too long string will cause truncation, no crash)
   if ( fMaxSize < (UInt_t)fBuffer.size() )
      PyErr_Warn( PyExc_RuntimeWarning, (char*)"string too long for char array (truncated)" );
   else if ( fMaxSize != UINT_MAX )
      fBuffer.resize( fMaxSize, '\0' );      // padd remainder of buffer as needed

// set the value and declare success
   if ( func )
      func->SetArg( reinterpret_cast< Long_t >( fBuffer.c_str() ) );
   return kTRUE;
}

PyObject* PyROOT::TCStringConverter::FromMemory( void* address )
{
// construct python object from C++ const char* read at <address>
   if ( address && *(char**)address ) {
      if ( fMaxSize != UINT_MAX ) {          // need to prevent reading beyond boundary
         std::string buf( *(char**)address, fMaxSize );     // cut on fMaxSize
         return PyROOT_PyUnicode_FromString( buf.c_str() ); // cut on \0
      }

      return PyROOT_PyUnicode_FromString( *(char**)address );
   }

// empty string in case there's no address
   Py_INCREF( PyStrings::gEmptyString );
   return PyStrings::gEmptyString;
}

Bool_t PyROOT::TCStringConverter::ToMemory( PyObject* value, void* address )
{
// convert <value> to C++ const char*, write it at <address>
   const char* s = PyROOT_PyUnicode_AsStringChecked( value );
   if ( PyErr_Occurred() )
      return kFALSE;

// verify (too long string will cause truncation, no crash)
   if ( fMaxSize < (UInt_t)PyROOT_PyUnicode_GET_SIZE( value ) )
      PyErr_Warn( PyExc_RuntimeWarning, (char*)"string too long for char array (truncated)" );

   if ( fMaxSize != UINT_MAX )
      strncpy( *(char**)address, s, fMaxSize );   // padds remainder
   else
      // coverity[secure_coding] - can't help it, it's intentional.
      strcpy( *(char**)address, s );

   return kTRUE;
}


//- pointer/array conversions -------------------------------------------------
namespace {

   using namespace PyROOT;

   inline Bool_t CArraySetArg(
      PyObject* pyobject, PyROOT::TParameter_t& para, G__CallFunc* func, char tc, int size )
   {
   // general case of loading a C array pointer (void* + type code) as function argument
      if ( pyobject == gNullPtrObject ) {
         para.fVoidp = NULL;
      } else {
         int buflen = PyROOT::Utility::GetBuffer( pyobject, tc, size, para.fVoidp );
         if ( ! para.fVoidp || buflen == 0 )
            return kFALSE;
      }

      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;
   }

} // unnamed namespace


//____________________________________________________________________________
Bool_t PyROOT::TNonConstCStringConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// attempt base class first (i.e. passing a string), but if that fails, try a buffer
   if ( this->TCStringConverter::SetArg( pyobject, para, func ) )
      return kTRUE;

// apparently failed, try char buffer
   PyErr_Clear();
   return CArraySetArg( pyobject, para, func, 'c', sizeof(char) );
}

//____________________________________________________________________________
PyObject* PyROOT::TNonConstCStringConverter::FromMemory( void* address )
{
// assume this is a buffer access if the size is known; otherwise assume string
   if ( fMaxSize != UINT_MAX )
      return PyROOT_PyUnicode_FromStringAndSize( *(char**)address, fMaxSize );
   return this->TCStringConverter::FromMemory( address );
}

//____________________________________________________________________________
Bool_t PyROOT::TNonConstUCStringConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// attempt base class first (i.e. passing a string), but if that fails, try a buffer
   if ( this->TCStringConverter::SetArg( pyobject, para, func ) )
      return kTRUE;

// apparently failed, try char buffer
   PyErr_Clear();
   return CArraySetArg( pyobject, para, func, 'B', sizeof(unsigned char) );
}

//____________________________________________________________________________
Bool_t PyROOT::TVoidArrayConverter::GetAddressSpecialCase( PyObject* pyobject, void*& address )
{
// (1): "null pointer" or C++11 style nullptr
   if ( pyobject == Py_None || pyobject == gNullPtrObject ) {
      address = (void*)0;
      return kTRUE;
   }

// (2): allow integer zero to act as a null pointer, no deriveds
   if ( PyInt_CheckExact( pyobject ) || PyLong_CheckExact( pyobject ) ) {
      Long_t val = (Long_t)PyLong_AsLong( pyobject );
      if ( val == 0l ) {
         address = (void*)val;
         return kTRUE;
      }

      return kFALSE;
   }

// (3): opaque PyCapsule (CObject in older pythons) from somewhere
   if ( PyROOT_PyCapsule_CheckExact( pyobject ) ) {
      address = (void*)PyROOT_PyCapsule_GetPointer( pyobject, NULL );
      return kTRUE;
   }

   return kFALSE;
}

//____________________________________________________________________________
Bool_t PyROOT::TVoidArrayConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t user )
{
// just convert pointer if it is a ROOT object
   if ( ObjectProxy_Check( pyobject ) ) {
   // depending on memory policy, some objects are no longer owned when passed to C++
      if ( ! fKeepControl && user != Utility::kStrict )
         ((ObjectProxy*)pyobject)->Release();

   // set pointer (may be null) and declare success
      para.fVoidp = ((ObjectProxy*)pyobject)->GetObject();
      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;
   }

// handle special cases
   if ( GetAddressSpecialCase( pyobject, para.fVoidp ) ) {
      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;
   }

// final try: attempt to get buffer
   int buflen = Utility::GetBuffer( pyobject, '*', 1, para.fVoidp, kFALSE );

// ok if buffer exists (can't perform any useful size checks)
   if ( para.fVoidp && buflen != 0 ) {
      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;
   }

// give up
   return kFALSE;
}

//____________________________________________________________________________
PyObject* PyROOT::TVoidArrayConverter::FromMemory( void* address )
{
// nothing sensible can be done, just return <address> as pylong
   return PyLong_FromLong( (Long_t)address );
}

//____________________________________________________________________________
Bool_t PyROOT::TVoidArrayConverter::ToMemory( PyObject* value, void* address )
{
// just convert pointer if it is a ROOT object
   if ( ObjectProxy_Check( value ) ) {
   // depending on memory policy, some objects are no longer owned when passed to C++
      if ( ! fKeepControl && Utility::gMemoryPolicy != Utility::kStrict )
         ((ObjectProxy*)value)->Release();

   // set pointer (may be null) and declare success
      *(void**)address = ((ObjectProxy*)value)->GetObject();
      return kTRUE;
   }

// handle special cases
   void* ptr = 0;
   if ( GetAddressSpecialCase( value, ptr ) ) {
      *(void**)address = ptr;
      return kTRUE;
   }

// final try: attempt to get buffer
   void* buf = 0;
   int buflen = Utility::GetBuffer( value, '*', 1, buf, kFALSE );
   if ( ! buf || buflen == 0 )
      return kFALSE;

   *(void**)address = buf;
   return kTRUE;
}

//____________________________________________________________________________
#define PYROOT_IMPLEMENT_ARRAY_CONVERTER( name, type, code )                 \
Bool_t PyROOT::T##name##ArrayConverter::SetArg(                              \
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )    \
{                                                                            \
   return CArraySetArg( pyobject, para, func, code, sizeof(type) );          \
}                                                                            \
                                                                             \
PyObject* PyROOT::T##name##ArrayConverter::FromMemory( void* address )       \
{                                                                            \
   return BufFac_t::Instance()->PyBuffer_FromMemory( *(type**)address, fSize );\
}                                                                            \
                                                                             \
Bool_t PyROOT::T##name##ArrayConverter::ToMemory( PyObject* value, void* address )\
{                                                                            \
   void* buf = 0;                                                            \
   int buflen = Utility::GetBuffer( value, code, sizeof(type), buf );        \
   if ( ! buf || buflen == 0 )                                               \
      return kFALSE;                                                         \
   if ( 0 <= fSize ) {                                                       \
      if ( fSize < buflen/(int)sizeof(type) ) {                              \
         PyErr_SetString( PyExc_ValueError, "buffer too large for value" );  \
         return kFALSE;                                                      \
      }                                                                      \
      memcpy( *(type**)address, buf, 0 < buflen ? ((size_t) buflen) : sizeof(type) );\
   } else                                                                    \
      *(type**)address = (type*)buf;                                         \
   return kTRUE;                                                             \
}

//____________________________________________________________________________
PYROOT_IMPLEMENT_ARRAY_CONVERTER( Bool,   Bool_t,   'b' )   // signed char
PYROOT_IMPLEMENT_ARRAY_CONVERTER( Short,  Short_t,  'h' )
PYROOT_IMPLEMENT_ARRAY_CONVERTER( UShort, UShort_t, 'H' )
PYROOT_IMPLEMENT_ARRAY_CONVERTER( Int,    Int_t,    'i' )
PYROOT_IMPLEMENT_ARRAY_CONVERTER( UInt,   UInt_t,   'I' )
PYROOT_IMPLEMENT_ARRAY_CONVERTER( Long,   Long_t,   'l' )
PYROOT_IMPLEMENT_ARRAY_CONVERTER( ULong,  ULong_t,  'L' )
PYROOT_IMPLEMENT_ARRAY_CONVERTER( Float,  Float_t,  'f' )
PYROOT_IMPLEMENT_ARRAY_CONVERTER( Double, Double_t, 'd' )

//____________________________________________________________________________
Bool_t PyROOT::TLongLongArrayConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t user )
{
// convert <pyobject> to C++ long long*, set arg for call
   PyObject* pytc = PyObject_GetAttr( pyobject, PyStrings::gTypeCode );
   if ( pytc != 0 ) {              // iow, this array has a known type, but there's no
      Py_DECREF( pytc );           // such thing for long long in module array
      return kFALSE;
   }

   return TVoidArrayConverter::SetArg( pyobject, para, func, user );
}


//- converters for special cases ----------------------------------------------
#define PYROOT_IMPLEMENT_STRING_AS_PRIMITIVE_CONVERTER( name, type, F1, F2 )  \
PyROOT::T##name##Converter::T##name##Converter( Bool_t keepControl ) :        \
      TRootObjectConverter( TClass::GetClass( #type ), keepControl ) {}       \
                                                                              \
Bool_t PyROOT::T##name##Converter::SetArg(                                    \
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t user )\
{                                                                             \
   if ( PyROOT_PyUnicode_Check( pyobject ) ) {                                \
      fBuffer = type( PyROOT_PyUnicode_AsString( pyobject ),                  \
                      PyROOT_PyUnicode_GET_SIZE( pyobject ) );                \
      para.fVoidp = &fBuffer;                                                 \
      if ( func ) {                                                           \
         G__value v;                                                          \
         G__setnull( &v );                                                    \
         v.ref = para.fLong;                                                  \
         G__letint( &v, 'u', para.fLong );                                    \
         G__set_tagnum( &v, ((G__ClassInfo*)fClass->GetClassInfo())->Tagnum() ); \
         func->SetArg( v );                                                   \
      }                                                                       \
      return kTRUE;                                                           \
   }                                                                          \
                                                                              \
   if ( ! ( PyInt_Check( pyobject ) || PyLong_Check( pyobject ) ) )           \
      return TRootObjectConverter::SetArg( pyobject, para, func, user );      \
   return kFALSE;                                                             \
}                                                                             \
                                                                              \
PyObject* PyROOT::T##name##Converter::FromMemory( void* address )             \
{                                                                             \
   if ( address )                                                             \
      return PyROOT_PyUnicode_FromStringAndSize( ((type*)address)->F1(), ((type*)address)->F2() );\
   Py_INCREF( PyStrings::gEmptyString );                                      \
   return PyStrings::gEmptyString;                                            \
}                                                                             \
                                                                              \
Bool_t PyROOT::T##name##Converter::ToMemory( PyObject* value, void* address ) \
{                                                                             \
   if ( PyROOT_PyUnicode_Check( value ) ) {                                   \
      *((type*)address) = PyROOT_PyUnicode_AsString( value );                 \
      return kTRUE;                                                           \
   }                                                                          \
                                                                              \
   return TRootObjectConverter::ToMemory( value, address );                   \
}

PYROOT_IMPLEMENT_STRING_AS_PRIMITIVE_CONVERTER( TString,   TString,     Data, Length )
PYROOT_IMPLEMENT_STRING_AS_PRIMITIVE_CONVERTER( STLString, std::string, c_str, size )

//____________________________________________________________________________
Bool_t PyROOT::TRootObjectConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t user )
{
// convert <pyobject> to C++ instance*, set arg for call
   if ( ! ObjectProxy_Check( pyobject ) ) {
      if ( GetAddressSpecialCase( pyobject, para.fVoidp ) ) {
         if ( func )
            func->SetArg( para.fLong );      // allow special cases such as NULL
         return kTRUE;
      }

   // not a PyROOT object (TODO: handle SWIG etc.)
      return kFALSE;
   }

   ObjectProxy* pyobj = (ObjectProxy*)pyobject;
   if ( pyobj->ObjectIsA() && pyobj->ObjectIsA()->GetBaseClass( fClass.GetClass() ) ) {
   // depending on memory policy, some objects need releasing when passed into functions
      if ( ! KeepControl() && user != Utility::kStrict )
         ((ObjectProxy*)pyobject)->Release();

   // calculate offset between formal and actual arguments
      para.fVoidp = pyobj->GetObject();
      G__ClassInfo* clFormalInfo = (G__ClassInfo*)fClass->GetClassInfo();
      G__ClassInfo* clActualInfo = (G__ClassInfo*)pyobj->ObjectIsA()->GetClassInfo();
      Long_t offset = 0;
      if ( clFormalInfo && clActualInfo && clFormalInfo != clActualInfo )
         offset = G__isanybase( clFormalInfo->Tagnum(), clActualInfo->Tagnum(), para.fLong );

   // set pointer (may be null) and declare success
      para.fLong += offset;
      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;

   } else if ( ! fClass.GetClass()->GetClassInfo() ) {
   // assume "user knows best" to allow anonymous pointer passing
      para.fVoidp = pyobj->GetObject();
      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;
   }

   return kFALSE;
}

//____________________________________________________________________________
PyObject* PyROOT::TRootObjectConverter::FromMemory( void* address )
{
// construct python object from C++ instance read at <address>
   return BindRootObject( address, fClass, kFALSE );
}

//____________________________________________________________________________
Bool_t PyROOT::TRootObjectConverter::ToMemory( PyObject* value, void* address )
{
// convert <value> to C++ instance, write it at <address>
   if ( ! ObjectProxy_Check( value ) ) {
      void* ptr = 0;
      if ( GetAddressSpecialCase( value, ptr ) ) {
         *(void**)address = ptr;             // allow special cases such as NULL
         return kTRUE;
      }

   // not a PyROOT object (TODO: handle SWIG etc.)
      return kFALSE;
   }

   if ( ((ObjectProxy*)value)->ObjectIsA()->GetBaseClass( fClass.GetClass() ) ) {
   // depending on memory policy, some objects need releasing when passed into functions
      if ( ! KeepControl() && Utility::gMemoryPolicy != Utility::kStrict )
         ((ObjectProxy*)value)->Release();

   // call assignment operator through a temporarily wrapped object proxy
      PyObject* pyobj = BindRootObjectNoCast( address, fClass.GetClass() );
      ((ObjectProxy*)pyobj)->Release();     // TODO: might be recycled (?)
      PyObject* result = PyObject_CallMethod( pyobj, (char*)"__assign__", (char*)"O", value );
      Py_DECREF( pyobj );
      if ( result ) {
         Py_DECREF( result );
         return kTRUE;
      }
   }

   return kFALSE;
}

//____________________________________________________________________________
Bool_t PyROOT::TRootObjectPtrConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t user )
{
// convert <pyobject> to C++ instance**, set arg for call
   if ( ! ObjectProxy_Check( pyobject ) )
      return kFALSE;              // not a PyROOT object (TODO: handle SWIG etc.)

   if ( ((ObjectProxy*)pyobject)->ObjectIsA()->GetBaseClass( fClass.GetClass() ) ) {
   // depending on memory policy, some objects need releasing when passed into functions
      if ( ! KeepControl() && user != Utility::kStrict )
         ((ObjectProxy*)pyobject)->Release();

   // set pointer (may be null) and declare success
      para.fVoidp = &((ObjectProxy*)pyobject)->fObject;
      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;
   }

   return kFALSE;
}

//____________________________________________________________________________
PyObject* PyROOT::TRootObjectPtrConverter::FromMemory( void* address )
{
// construct python object from C++ instance* read at <address>
   return BindRootObject( address, fClass, kTRUE );
}

//____________________________________________________________________________
Bool_t PyROOT::TRootObjectPtrConverter::ToMemory( PyObject* value, void* address )
{
// convert <value> to C++ instance*, write it at <address>
   if ( ! ObjectProxy_Check( value ) )
      return kFALSE;              // not a PyROOT object (TODO: handle SWIG etc.)

   if ( ((ObjectProxy*)value)->ObjectIsA()->GetBaseClass( fClass.GetClass() ) ) {
   // depending on memory policy, some objects need releasing when passed into functions
      if ( ! KeepControl() && Utility::gMemoryPolicy != Utility::kStrict )
         ((ObjectProxy*)value)->Release();

   // set pointer (may be null) and declare success
      *(void**)address = ((ObjectProxy*)value)->GetObject();
      return kTRUE;
   }

   return kFALSE;
}

//____________________________________________________________________________
Bool_t PyROOT::TVoidPtrRefConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ void*&, set arg for call
   if ( ObjectProxy_Check( pyobject ) ) {
      para.fVoidp = &((ObjectProxy*)pyobject)->fObject;
      if ( func )
         func->SetArg( para.fLong );    // this assumes that CINT will treat void*& as void**
      return kTRUE;
   }

   return kFALSE;
}

//____________________________________________________________________________
Bool_t PyROOT::TVoidPtrPtrConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// convert <pyobject> to C++ void**, set arg for call
   if ( ObjectProxy_Check( pyobject ) ) {
   // this is a ROOT object, take and set its address
      para.fVoidp = &((ObjectProxy*)pyobject)->fObject;
      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;
   }

// buffer objects are allowed under "user knows best"
   int buflen = Utility::GetBuffer( pyobject, '*', 1, para.fVoidp, kFALSE );

// ok if buffer exists (can't perform any useful size checks)
   if ( para.fVoidp && buflen != 0 ) {
      if ( func )
         func->SetArg( para.fLong );
      return kTRUE;
   }

   return kFALSE;
}

//____________________________________________________________________________
PyObject* PyROOT::TVoidPtrPtrConverter::FromMemory( void* address )
{
// read a void** from address; since this is unknown, long is used (user can cast)
   return PyLong_FromLong( (Long_t)*((Long_t**)address) );
}

//____________________________________________________________________________
Bool_t PyROOT::TPyObjectConverter::SetArg(
      PyObject* pyobject, TParameter_t& para, G__CallFunc* func, Long_t )
{
// by definition: set and declare success
   para.fVoidp = pyobject;
   if ( func )
      func->SetArg( para.fLong );
   return kTRUE;
}

PyObject* PyROOT::TPyObjectConverter::FromMemory( void* address )
{
// construct python object from C++ PyObject* read at <address>
   PyObject* pyobject = *((PyObject**)address);

   if ( ! pyobject ) {
      Py_INCREF( Py_None );
      return Py_None;
   }

   Py_INCREF( pyobject );
   return pyobject;
}

Bool_t PyROOT::TPyObjectConverter::ToMemory( PyObject* value, void* address )
{
// no conversion needed, write <value> at <address>
   Py_INCREF( value );
   *((PyObject**)address) = value;
   return kTRUE;
}


//- factories -----------------------------------------------------------------
PyROOT::TConverter* PyROOT::CreateConverter( const std::string& fullType, Long_t user )
{
// The matching of the fulltype to a converter factory goes through up to five levels:
//   1) full, exact match
//   2) match of decorated, unqualified type
//   3) accept const ref as by value
//   4) accept ref as pointer
//   5) generalized cases (covers basically all ROOT classes)
//
// If all fails, void is used, which will generate a run-time warning when used.

// resolve typedefs etc.
   G__TypeInfo ti( fullType.c_str() );
   std::string resolvedType = ti.TrueName();
   if ( ! ti.IsValid() )
      resolvedType = fullType;     // otherwise, resolvedType will be "(unknown)"

// an exactly matching converter is preferred
   ConvFactories_t::iterator h = gConvFactories.find( resolvedType );
   if ( h != gConvFactories.end() )
      return (h->second)( user );

//-- nothing? ok, collect information about the type and possible qualifiers/decorators
   const std::string& cpd = Utility::Compound( resolvedType );
   std::string realType   = TClassEdit::ShortType( resolvedType.c_str(), 1 );

// accept unqualified type (as python does not know about qualifiers)
   h = gConvFactories.find( realType + cpd );
   if ( h != gConvFactories.end() )
      return (h->second)( user );

//-- nothing? collect qualifier information
   Bool_t isConst = ti.Property() & G__BIT_ISCONSTANT;

// accept const <type>& as converter by value (as python copies most types)
   if ( isConst && cpd == "&" ) {
      h = gConvFactories.find( realType );
      if ( h != gConvFactories.end() )
         return (h->second)( user );
   }

//-- still nothing? try pointer instead of ref, if ref
   if ( cpd == "&" ) {
      h = gConvFactories.find( realType + "*" );
      if ( h != gConvFactories.end() )
         return (h->second)( user );
   }

//-- still nothing? use a generalized converter
   Bool_t control = cpd == "&" || isConst;

// converters for known/ROOT classes and default (void*)
   TConverter* result = 0;
   if ( TClass* klass = TClass::GetClass( realType.c_str() ) ) {
      if ( cpd == "**" || cpd == "*&" || cpd == "&*" )
         result = new TRootObjectPtrConverter( klass, control );
      else if ( cpd == "*" )
         result = new TRootObjectConverter( klass, control );
      else if ( cpd == "&" )
         result = new TStrictRootObjectConverter( klass, control );
      else if ( cpd == "" )               // by value
         result = new TStrictRootObjectConverter( klass, kTRUE );

   } else if ( ti.Property() & G__BIT_ISENUM ) {
   // special case (CINT): represent enums as unsigned integers
      if ( cpd == "&" ) {
         h = isConst ? gConvFactories.find( "const long&" ) : gConvFactories.find( "long&" );
      } else
         h = gConvFactories.find( "UInt_t" );
   }

   if ( ! result && h != gConvFactories.end() )
   // converter factory available, use it to create converter
      result = (h->second)( user );
   else if ( ! result ) {
      if ( cpd != "" ) {
         std::stringstream s;
         s << "creating converter for unknown type \"" << fullType << "\"" << std::ends;
         PyErr_Warn( PyExc_RuntimeWarning, (char*)s.str().c_str() );
         result = new TVoidArrayConverter();       // "user knows best"
      } else
         result = new TVoidConverter();            // fails on use
   }

   return result;
}

//____________________________________________________________________________
#define PYROOT_BASIC_CONVERTER_FACTORY( name )                               \
TConverter* Create##name##Converter( Long_t )                                \
{                                                                            \
   return new T##name##Converter();                                          \
}

#define PYROOT_ARRAY_CONVERTER_FACTORY( name )                               \
TConverter* Create##name##Converter( Long_t user )                           \
{                                                                            \
   return new T##name##Converter( (Int_t)user );                             \
}

//____________________________________________________________________________
namespace {

   using namespace PyROOT;

// use macro rather than template for portability ...
   PYROOT_BASIC_CONVERTER_FACTORY( Bool )
   PYROOT_BASIC_CONVERTER_FACTORY( Char )
   PYROOT_BASIC_CONVERTER_FACTORY( UChar )
   PYROOT_BASIC_CONVERTER_FACTORY( Short )
   PYROOT_BASIC_CONVERTER_FACTORY( UShort )
   PYROOT_BASIC_CONVERTER_FACTORY( Int )
   PYROOT_BASIC_CONVERTER_FACTORY( IntRef )
   PYROOT_BASIC_CONVERTER_FACTORY( UInt )
   PYROOT_BASIC_CONVERTER_FACTORY( Long )
   PYROOT_BASIC_CONVERTER_FACTORY( LongRef )
   PYROOT_BASIC_CONVERTER_FACTORY( ConstLongRef )
   PYROOT_BASIC_CONVERTER_FACTORY( ULong )
   PYROOT_BASIC_CONVERTER_FACTORY( Float )
   PYROOT_BASIC_CONVERTER_FACTORY( Double )
   PYROOT_BASIC_CONVERTER_FACTORY( DoubleRef )
   PYROOT_BASIC_CONVERTER_FACTORY( ConstDoubleRef )
   PYROOT_BASIC_CONVERTER_FACTORY( LongDouble )
   PYROOT_BASIC_CONVERTER_FACTORY( Void )
   PYROOT_BASIC_CONVERTER_FACTORY( Macro )
   PYROOT_BASIC_CONVERTER_FACTORY( LongLong )
   PYROOT_BASIC_CONVERTER_FACTORY( ULongLong )
   PYROOT_ARRAY_CONVERTER_FACTORY( CString )
   PYROOT_ARRAY_CONVERTER_FACTORY( NonConstCString )
   PYROOT_ARRAY_CONVERTER_FACTORY( NonConstUCString )
   PYROOT_ARRAY_CONVERTER_FACTORY( BoolArray )
   PYROOT_ARRAY_CONVERTER_FACTORY( ShortArray )
   PYROOT_ARRAY_CONVERTER_FACTORY( UShortArray )
   PYROOT_ARRAY_CONVERTER_FACTORY( IntArray )
   PYROOT_ARRAY_CONVERTER_FACTORY( UIntArray )
   PYROOT_ARRAY_CONVERTER_FACTORY( LongArray )
   PYROOT_ARRAY_CONVERTER_FACTORY( ULongArray )
   PYROOT_ARRAY_CONVERTER_FACTORY( FloatArray )
   PYROOT_ARRAY_CONVERTER_FACTORY( DoubleArray )
   PYROOT_BASIC_CONVERTER_FACTORY( VoidArray )
   PYROOT_BASIC_CONVERTER_FACTORY( LongLongArray )
   PYROOT_BASIC_CONVERTER_FACTORY( TString )
   PYROOT_BASIC_CONVERTER_FACTORY( STLString )
   PYROOT_BASIC_CONVERTER_FACTORY( VoidPtrRef )
   PYROOT_BASIC_CONVERTER_FACTORY( VoidPtrPtr )
   PYROOT_BASIC_CONVERTER_FACTORY( PyObject )

// converter factories for ROOT types
   typedef std::pair< const char*, ConverterFactory_t > NFp_t;

   NFp_t factories_[] = {
   // factories for built-ins
      NFp_t( "bool",               &CreateBoolConverter               ),
      NFp_t( "char",               &CreateCharConverter               ),
      NFp_t( "unsigned char",      &CreateUCharConverter              ),
      NFp_t( "short",              &CreateShortConverter              ),
      NFp_t( "unsigned short",     &CreateUShortConverter             ),
      NFp_t( "int",                &CreateIntConverter                ),
      NFp_t( "int&",               &CreateIntRefConverter             ),
      NFp_t( "const int&",         &CreateIntConverter                ),
      NFp_t( "unsigned int",       &CreateUIntConverter               ),
      NFp_t( "UInt_t", /* enum */  &CreateUIntConverter               ),
      NFp_t( "long",               &CreateLongConverter               ),
      NFp_t( "long&",              &CreateLongRefConverter            ),
      NFp_t( "const long&",        &CreateConstLongRefConverter       ),
      NFp_t( "unsigned long",      &CreateULongConverter              ),
      NFp_t( "long long",          &CreateLongLongConverter           ),
      NFp_t( "unsigned long long", &CreateULongLongConverter          ),
      NFp_t( "float",              &CreateFloatConverter              ),
      NFp_t( "double",             &CreateDoubleConverter             ),
      NFp_t( "double&",            &CreateDoubleRefConverter          ),
      NFp_t( "const double&",      &CreateConstDoubleRefConverter     ),
      NFp_t( "long double",        &CreateLongDoubleConverter         ),
      NFp_t( "void",               &CreateVoidConverter               ),
      NFp_t( "#define",            &CreateMacroConverter              ),

   // pointer/array factories
      NFp_t( "bool*",              &CreateBoolArrayConverter          ),
      NFp_t( "const unsigned char*", &CreateCStringConverter          ),
      NFp_t( "unsigned char*",     &CreateNonConstUCStringConverter   ),
      NFp_t( "short*",             &CreateShortArrayConverter         ),
      NFp_t( "unsigned short*",    &CreateUShortArrayConverter        ),
      NFp_t( "int*",               &CreateIntArrayConverter           ),
      NFp_t( "unsigned int*",      &CreateUIntArrayConverter          ),
      NFp_t( "long*",              &CreateLongArrayConverter          ),
      NFp_t( "unsigned long*",     &CreateULongArrayConverter         ),
      NFp_t( "float*",             &CreateFloatArrayConverter         ),
      NFp_t( "double*",            &CreateDoubleArrayConverter        ),
      NFp_t( "long long*",         &CreateLongLongArrayConverter      ),
      NFp_t( "unsigned long long*", &CreateLongLongArrayConverter     ),
      NFp_t( "void*",              &CreateVoidArrayConverter          ),

   // factories for special cases
      NFp_t( "const char*",        &CreateCStringConverter            ),
      NFp_t( "char*",              &CreateNonConstCStringConverter    ),
      NFp_t( "TString",            &CreateTStringConverter            ),
      NFp_t( "const TString&",     &CreateTStringConverter            ),
      NFp_t( "std::string",        &CreateSTLStringConverter          ),
      NFp_t( "string",             &CreateSTLStringConverter          ),
      NFp_t( "const std::string&", &CreateSTLStringConverter          ),
      NFp_t( "const string&",      &CreateSTLStringConverter          ),
      NFp_t( "void*&",             &CreateVoidPtrRefConverter         ),
      NFp_t( "void**",             &CreateVoidPtrPtrConverter         ),
      NFp_t( "PyObject*",          &CreatePyObjectConverter           ),
      NFp_t( "_object*",           &CreatePyObjectConverter           ),
      NFp_t( "FILE*",              &CreateVoidArrayConverter          )
   };

   struct InitConvFactories_t {
   public:
      InitConvFactories_t()
      {
      // load all converter factories in the global map 'gConvFactories'
         int nf = sizeof( factories_ ) / sizeof( factories_[ 0 ] );
         for ( int i = 0; i < nf; ++i ) {
            gConvFactories[ factories_[ i ].first ] = factories_[ i ].second;
         }
      }
   } initConvFactories_;

} // unnamed namespace
