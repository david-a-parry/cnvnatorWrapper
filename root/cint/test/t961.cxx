/* -*- C++ -*- */
/*************************************************************************
 * Copyright(c) 1995~2005  Masaharu Goto (root-cint@cern.ch)
 *
 * For the licensing terms see the file COPYING
 *
 ************************************************************************/

#ifdef __CINT__
#pragma include "test.dll"
#else
#include "t961.h"
#endif

#include <stdio.h>

void test() {
  printf("success\n");
}

int main() {
  test(); 
  return 0;
}
