/* -*- C++ -*- */
/*************************************************************************
 * Copyright(c) 1995~2005  Masaharu Goto (root-cint@cern.ch)
 *
 * For the licensing terms see the file COPYING
 *
 ************************************************************************/


class A {
 public:
  double **d;
  double** ev_sc() { return(d); }
};

