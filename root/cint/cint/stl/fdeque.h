/* -*- C++ -*- */

/************************************************************************
 *
 * Copyright(c) 1995~2006  Masaharu Goto (root-cint@cern.ch)
 *
 * For the licensing terms see the file COPYING
 *
 ************************************************************************/

/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef FDEQUE_H
#define FDEQUE_H

#ifdef DEQUE_H
#undef DEQUE_H
#define __DEQUE_WAS_DEFINED
#endif

#define Allocator far_allocator
#define deque far_deque
#include <faralloc.h>
#include <deque.h>

#undef DEQUE_H

#ifdef __DEQUE_WAS_DEFINED
#define DEQUE_H
#undef  __DEQUE_WAS_DEFINED
#endif

#undef Allocator
#undef deque

#endif
