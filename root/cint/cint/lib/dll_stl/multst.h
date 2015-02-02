/* -*- C++ -*- */
/*************************************************************************
 * Copyright(c) 1995~2005  Masaharu Goto (root-cint@cern.ch)
 *
 * For the licensing terms see the file COPYING
 *
 ************************************************************************/
// lib/dll_stl/multst.h

#ifdef __CINT__
#include <multiset>
#else
#include <set>
#endif
#include <algorithm>
#include <string>
#ifndef __hpux
using namespace std;
#endif

#ifdef __MAKECINT__
#ifndef G__MULTISET_DLL
#define G__MULTISET_DLL
#endif
#pragma link C++ global G__MULTISET_DLL;
#pragma link C++ nestedtypedef;
#pragma link C++ nestedclass;

#pragma link C++ class multiset<int>;
#pragma link C++ class multiset<long>;
#pragma link C++ class multiset<float>;
#pragma link C++ class multiset<double>;
#pragma link C++ class multiset<void*>;
#pragma link C++ class multiset<char*>;
#if defined(G__STRING_DLL) || defined(G__ROOT)
#pragma link C++ class multiset<string>;
#pragma link C++ operators multiset<string>::iterator;
#endif

#pragma link C++ operators multiset<int>::iterator;
#pragma link C++ operators multiset<long>::iterator;
#pragma link C++ operators multiset<float>::iterator;
#pragma link C++ operators multiset<double>::iterator;
#pragma link C++ operators multiset<void*>::iterator;
#pragma link C++ operators multiset<char*>::iterator;

#pragma link C++ operators multiset<int>::const_iterator;
#pragma link C++ operators multiset<long>::const_iterator;
#pragma link C++ operators multiset<float>::const_iterator;
#pragma link C++ operators multiset<double>::const_iterator;
#pragma link C++ operators multiset<void*>::const_iterator;
#pragma link C++ operators multiset<char*>::const_iterator;

#pragma link C++ operators multiset<int>::reverse_iterator;
#pragma link C++ operators multiset<long>::reverse_iterator;
#pragma link C++ operators multiset<float>::reverse_iterator;
#pragma link C++ operators multiset<double>::reverse_iterator;
#pragma link C++ operators multiset<void*>::reverse_iterator;
#pragma link C++ operators multiset<char*>::reverse_iterator;

#endif


