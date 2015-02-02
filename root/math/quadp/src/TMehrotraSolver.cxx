// @(#)root/quadp:$Id$
// Author: Eddy Offermann   May 2004

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/*************************************************************************
 * Parts of this file are copied from the OOQP distribution and          *
 * are subject to the following license:                                 *
 *                                                                       *
 * COPYRIGHT 2001 UNIVERSITY OF CHICAGO                                  *
 *                                                                       *
 * The copyright holder hereby grants you royalty-free rights to use,    *
 * reproduce, prepare derivative works, and to redistribute this software*
 * to others, provided that any changes are clearly documented. This     *
 * software was authored by:                                             *
 *                                                                       *
 *   E. MICHAEL GERTZ      gertz@mcs.anl.gov                             *
 *   Mathematics and Computer Science Division                           *
 *   Argonne National Laboratory                                         *
 *   9700 S. Cass Avenue                                                 *
 *   Argonne, IL 60439-4844                                              *
 *                                                                       *
 *   STEPHEN J. WRIGHT     swright@cs.wisc.edu                           *
 *   Computer Sciences Department                                        *
 *   University of Wisconsin                                             *
 *   1210 West Dayton Street                                             *
 *   Madison, WI 53706   FAX: (608)262-9777                              *
 *                                                                       *
 * Any questions or comments may be directed to one of the authors.      *
 *                                                                       *
 * ARGONNE NATIONAL LABORATORY (ANL), WITH FACILITIES IN THE STATES OF   *
 * ILLINOIS AND IDAHO, IS OWNED BY THE UNITED STATES GOVERNMENT, AND     *
 * OPERATED BY THE UNIVERSITY OF CHICAGO UNDER PROVISION OF A CONTRACT   *
 * WITH THE DEPARTMENT OF ENERGY.                                        *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMehrotraSolver                                                      //
//                                                                      //
// Derived class of TQpSolverBase implementing the original Mehrotra    //
// predictor-corrector algorithm                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "Riostream.h"
#include "TMath.h"
#include "TMehrotraSolver.h"

ClassImp(TMehrotraSolver)

//______________________________________________________________________________
TMehrotraSolver::TMehrotraSolver()
{
// Default constructor

   fPrintlevel = 0;
   fTsig       = 0.0;
   fStep       = 0;
   fFactory    = 0;
}


//______________________________________________________________________________
TMehrotraSolver::TMehrotraSolver(TQpProbBase *of,TQpDataBase *prob,Int_t verbose)
{
// Constructor

   fFactory = of;
   fStep = fFactory->MakeVariables(prob);

   fPrintlevel = verbose;
   fTsig       = 3.0;            // the usual value for the centering exponent (tau)
}


//______________________________________________________________________________
TMehrotraSolver::TMehrotraSolver(const TMehrotraSolver &another) : TQpSolverBase(another)
{
// Copy constructor

   *this = another;
}


//______________________________________________________________________________
Int_t TMehrotraSolver::Solve(TQpDataBase *prob,TQpVar *iterate,TQpResidual *resid)
{
// Solve the quadratic programming problem as formulated through prob, store
// the final solution in iterate->fX . Monitor the residuals during the iterations
// through resid . The status is returned as defined in TQpSolverBase::ETerminationCode .

   Int_t status_code;
   Double_t alpha = 1;
   Double_t sigma = 1;

   fDnorm = prob->DataNorm();

   // initialization of (x,y,z) and factorization routine.
   fSys = fFactory->MakeLinSys(prob);
   this->Start(fFactory,iterate,prob,resid,fStep);

   fIter = 0;
   Double_t mu = iterate->GetMu();

   Int_t done = 0;
   do {
      fIter++;

      // evaluate residuals and update algorithm status:
      resid->CalcResids(prob,iterate);

      //  termination test:
      status_code = this->DoStatus(prob,iterate,resid,fIter,mu,0);
      if (status_code != kNOT_FINISHED ) break;
      if (fPrintlevel >= 10)
         this->DoMonitor(prob,iterate,resid,alpha,sigma,fIter,mu,status_code,0);

      // *** Predictor step ***

      resid->Set_r3_xz_alpha(iterate,0.0);

      fSys->Factor(prob,iterate);
      fSys->Solve(prob,iterate,resid,fStep);
      fStep->Negate();

      alpha = iterate->StepBound(fStep);

      // calculate centering parameter
      Double_t muaff = iterate->MuStep(fStep,alpha);
      sigma = TMath::Power(muaff/mu,fTsig);

      // *** Corrector step ***

      // form right hand side of linear system:
      resid->Add_r3_xz_alpha(fStep,-sigma*mu );

      fSys->Solve(prob,iterate,resid,fStep);
      fStep->Negate();

      // We've finally decided on a step direction, now calculate the
      // length using Mehrotra's heuristic.
      alpha = this->FinalStepLength(iterate,fStep);

      // alternatively, just use a crude step scaling factor.
      //alpha = 0.995 * iterate->StepBound(fStep);

      // actually take the step and calculate the new mu
      iterate->Saxpy(fStep,alpha);
      mu = iterate->GetMu();
   } while(!done);

   resid->CalcResids(prob,iterate);
   if (fPrintlevel >= 10)
      this->DoMonitor(prob,iterate,resid,alpha,sigma,fIter,mu,status_code,1);

   return status_code;
}


//______________________________________________________________________________
void TMehrotraSolver::DefMonitor(TQpDataBase * /* data */,TQpVar * /* vars */,
                                 TQpResidual *resids,
                                 Double_t alpha,Double_t /* sigma */,Int_t i,Double_t mu,
                                 Int_t status_code,Int_t level)
{
// Print information about the optimization process and monitor the convergence
// status of thye algorithm

   switch (level) {
      case 0 : case 1:
      {
         cout << endl << "Duality Gap: " << resids->GetDualityGap() << endl;
         if (i > 1) {
            cout << " alpha = " << alpha << endl;
         }
         cout << " *** Iteration " << i << " *** " << endl;
         cout << " mu = " << mu << " relative residual norm = "
            << resids->GetResidualNorm()/fDnorm << endl;

         if (level == 1) {
            // Termination has been detected by the status check; print
            // appropriate message
            switch (status_code) {
               case kSUCCESSFUL_TERMINATION:
                  cout << endl << " *** SUCCESSFUL TERMINATION ***" << endl;
                  break;
               case kMAX_ITS_EXCEEDED:
                  cout << endl << " *** MAXIMUM ITERATIONS REACHED *** " << endl;
                  break;
               case kINFEASIBLE:
                  cout << endl << " *** TERMINATION: PROBABLY INFEASIBLE *** " << endl;
                  break;
               case kUNKNOWN:
                  cout << endl << " *** TERMINATION: STATUS UNKNOWN *** " << endl;
                  break;
            }
         }
      } break;                   // end case 0: case 1:
   }                             // end switch(level)
}


//______________________________________________________________________________
TMehrotraSolver::~TMehrotraSolver()
{
// Deconstructor

   delete fStep;
}


//______________________________________________________________________________
TMehrotraSolver &TMehrotraSolver::operator=(const TMehrotraSolver &source)
{
// Assignment operator

   if (this != &source) {
      TQpSolverBase::operator=(source);

      fPrintlevel = source.fPrintlevel;
      fTsig       = source.fTsig;

      if (fStep) delete fStep;

      fStep    = new TQpVar(*source.fStep);
      fFactory = source.fFactory;
   }
   return *this;
}
