// @(#)root/mlp:$Id$
// Author: Christophe.Delaere@cern.ch   20/07/03

/*************************************************************************
 * Copyright (C) 1995-2003, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

///////////////////////////////////////////////////////////////////////////
//
// TMultiLayerPerceptron
//
// This class describes a neural network.
// There are facilities to train the network and use the output.
//
// The input layer is made of inactive neurons (returning the
// optionaly normalized input) and output neurons are linear.
// The type of hidden neurons is free, the default being sigmoids.
// (One should still try to pass normalized inputs, e.g. between [0.,1])
//
// The basic input is a TTree and two (training and test) TEventLists.
// Input and output neurons are assigned a value computed for each event
// with the same possibilities as for TTree::Draw().
// Events may be weighted individualy or via TTree::SetWeight().
// 6 learning methods are available: kStochastic, kBatch,
// kSteepestDescent, kRibierePolak, kFletcherReeves and kBFGS.
//
// This implementation, written by C. Delaere, is *inspired* from
// the mlpfit package from J.Schwindling et al. with some extensions:
//   * the algorithms are globally the same
//   * in TMultilayerPerceptron, there is no limitation on the number of
//     layers/neurons, while MLPFIT was limited to 2 hidden layers
//   * TMultilayerPerceptron allows you to save the network in a root file, and
//     provides more export functionalities
//   * TMultilayerPerceptron gives more flexibility regarding the normalization of
//     inputs/outputs
//   * TMultilayerPerceptron provides, thanks to Andrea Bocci, the possibility to
//     use cross-entropy errors, which allows to train a network for pattern
//     classification based on Bayesian posterior probability.
//
///////////////////////////////////////////////////////////////////////////
//BEGIN_HTML <!--
/* -->
<UL>
        <LI><P><A NAME="intro"></A><FONT COLOR="#5c8526">
        <FONT SIZE=4 STYLE="font-size: 15pt">Introduction</FONT></FONT></P>
</UL>
<P>Neural Networks are more and more used in various fields for data
analysis and classification, both for research and commercial
institutions. Some randomly chosen examples are:</P>
<UL>
        <LI><P>image analysis</P>
        <LI><P>financial movements predictions and analysis</P>
        <LI><P>sales forecast and product shipping optimisation</P>
        <LI><P>in particles physics: mainly for classification tasks (signal
        over background discrimination)</P>
</UL>
<P>More than 50% of neural networks are multilayer perceptrons. This
implementation of multilayer perceptrons is inspired from the
<A HREF="http://schwind.home.cern.ch/schwind/MLPfit.html">MLPfit
package</A> originaly written by Jerome Schwindling. MLPfit remains
one of the fastest tool for neural networks studies, and this ROOT
add-on will not try to compete on that. A clear and flexible Object
Oriented implementation has been chosen over a faster but more
difficult to maintain code. Nevertheless, the time penalty does not
exceed a factor 2.</P>
<UL>
        <LI><P><A NAME="mlp"></A><FONT COLOR="#5c8526">
        <FONT SIZE=4 STYLE="font-size: 15pt">The
        MLP</FONT></FONT></P>
</UL>
<P>The multilayer perceptron is a simple feed-forward network with
the following structure:</P>
<P ALIGN=CENTER><IMG SRC="gif/mlp.png" NAME="MLP"
ALIGN=MIDDLE WIDTH=333 HEIGHT=358 BORDER=0>
</P>
<P>It is made of neurons characterized by a bias and weighted links
between them (let's call those links synapses). The input neurons
receive the inputs, normalize them and forward them to the first
hidden layer.
</P>
<P>Each neuron in any subsequent layer first computes a linear
combination of the outputs of the previous layer. The output of the
neuron is then function of that combination with <I>f</I> being
linear for output neurons or a sigmoid for hidden layers. This is
useful because of two theorems:</P>
<OL>
        <LI><P>A linear combination of sigmoids can approximate any
        continuous function.</P>
        <LI><P>Trained with output = 1 for the signal and 0 for the
        background, the approximated function of inputs X is the probability
        of signal, knowing X.</P>
</OL>
<UL>
        <LI><P><A NAME="lmet"></A><FONT COLOR="#5c8526">
        <FONT SIZE=4 STYLE="font-size: 15pt">Learning
        methods</FONT></FONT></P>
</UL>
<P>The aim of all learning methods is to minimize the total error on
a set of weighted examples. The error is defined as the sum in
quadrature, devided by two, of the error on each individual output
neuron.</P>
<P>In all methods implemented, one needs to compute
the first derivative of that error with respect to the weights.
Exploiting the well-known properties of the derivative, especialy the
derivative of compound functions, one can write:</P>
<UL>
        <LI><P>for a neuton: product of the local derivative with the
        weighted sum on the outputs of the derivatives.</P>
        <LI><P>for a synapse: product of the input with the local derivative
        of the output neuron.</P>
</UL>
<P>This computation is called back-propagation of the errors. A
loop over all examples is called an epoch.</P>
<P>Six learning methods are implemented.</P>
<P><FONT COLOR="#006b6b"><I>Stochastic minimization</I>:</FONT> This
is the most trivial learning method. This is the Robbins-Monro
stochastic approximation applied to multilayer perceptrons. The
weights are updated after each example according to the formula:</P>
<P ALIGN=CENTER>$w_{ij}(t+1) = w_{ij}(t) + \Delta w_{ij}(t)$
</P>
<P ALIGN=CENTER>with
</P>
<P ALIGN=CENTER>$\Delta w_{ij}(t) = - \eta(\d e_p / \d w_{ij} +
\delta) + \epsilon \Deltaw_{ij}(t-1)$</P>
<P>The parameters for this method are Eta, EtaDecay, Delta and
Epsilon.</P>
<P><FONT COLOR="#006b6b"><I>Steepest descent with fixed step size
(batch learning)</I>:</FONT> It is the same as the stochastic
minimization, but the weights are updated after considering all the
examples, with the total derivative dEdw. The parameters for this
method are Eta, EtaDecay, Delta and Epsilon.</P>
<P><FONT COLOR="#006b6b"><I>Steepest descent algorithm</I>: </FONT>Weights
are set to the minimum along the line defined by the gradient. The
only parameter for this method is Tau. Lower tau = higher precision =
slower search. A value Tau = 3 seems reasonable.</P>
<P><FONT COLOR="#006b6b"><I>Conjugate gradients with the
Polak-Ribiere updating formula</I>: </FONT>Weights are set to the
minimum along the line defined by the conjugate gradient. Parameters
are Tau and Reset, which defines the epochs where the direction is
reset to the steepes descent.</P>
<P><FONT COLOR="#006b6b"><I>Conjugate gradients with the
Fletcher-Reeves updating formula</I>: </FONT>Weights are set to the
minimum along the line defined by the conjugate gradient. Parameters
are Tau and Reset, which defines the epochs where the direction is
reset to the steepes descent.</P>
<P><FONT COLOR="#006b6b"><I>Broyden, Fletcher, Goldfarb, Shanno
(BFGS) method</I>:</FONT> Implies the computation of a NxN matrix
computation, but seems more powerful at least for less than 300
weights. Parameters are Tau and Reset, which defines the epochs where
the direction is reset to the steepes descent.</P>
<UL>
        <LI><P><A NAME="use"></A><FONT COLOR="#5c8526">
        <FONT SIZE=4 STYLE="font-size: 15pt">How
        to use it...</FONT></FONT></P></LI>
</UL>
<P><FONT SIZE=3>TMLP is build from 3 classes: TNeuron, TSynapse and
TMultiLayerPerceptron. Only TMultiLayerPerceptron should be used
explicitly by the user.</FONT></P>
<P><FONT SIZE=3>TMultiLayerPerceptron will take examples from a TTree
given in the constructor. The network is described by a simple
string: The input/output layers are defined by giving the expression for
each neuron, separated by comas. Hidden layers are just described
by the number of neurons. The layers are separated by colons.
In addition, input/output layer formulas can be preceded by '@' (e.g "@out")
if one wants to also normalize the data from the TTree.
Input and outputs are taken from the TTree given as second argument.
Expressions are evaluated as for TTree::Draw(), arrays are expended in
distinct neurons, one for each index.
This can only be done for fixed-size arrays.
If the formula ends with &quot;!&quot;, softmax functions are used for the output layer.
One defines the training and test datasets by TEventLists.</FONT></P>
<P STYLE="margin-left: 2cm"><FONT SIZE=3><SPAN STYLE="background: #e6e6e6">
<U><FONT COLOR="#ff0000">Example</FONT></U><SPAN STYLE="text-decoration: none">:
</SPAN>TMultiLayerPerceptron(&quot;x,y:10:5:f&quot;,inputTree);</SPAN></FONT></P>
<P><FONT SIZE=3>Both the TTree and the TEventLists can be defined in
the constructor, or later with the suited setter method. The lists
used for training and test can be defined either explicitly, or via
a string containing the formula to be used to define them, exactly as
for a TCut.</FONT></P>
<P><FONT SIZE=3>The learning method is defined using the
TMultiLayerPerceptron::SetLearningMethod() . Learning methods are :</FONT></P>
<P><FONT SIZE=3>TMultiLayerPerceptron::kStochastic, <BR>
TMultiLayerPerceptron::kBatch,<BR>
TMultiLayerPerceptron::kSteepestDescent,<BR>
TMultiLayerPerceptron::kRibierePolak,<BR>
TMultiLayerPerceptron::kFletcherReeves,<BR>
TMultiLayerPerceptron::kBFGS<BR></FONT></P>
<P>A weight can be assigned to events, either in the constructor, either
with TMultiLayerPerceptron::SetEventWeight(). In addition, the TTree weight
is taken into account.</P>
<P><FONT SIZE=3>Finally, one starts the training with
TMultiLayerPerceptron::Train(Int_t nepoch, Option_t* options). The
first argument is the number of epochs while option is a string that
can contain: &quot;text&quot; (simple text output) , &quot;graph&quot;
(evoluting graphical training curves), &quot;update=X&quot; (step for
the text/graph output update) or &quot;+&quot; (will skip the
randomisation and start from the previous values). All combinations
are available. </FONT></P>
<P STYLE="margin-left: 2cm"><FONT SIZE=3><SPAN STYLE="background: #e6e6e6">
<U><FONT COLOR="#ff0000">Example</FONT></U>:
net.Train(100,&quot;text, graph, update=10&quot;).</SPAN></FONT></P>
<P><FONT SIZE=3>When the neural net is trained, it can be used
directly ( TMultiLayerPerceptron::Evaluate() ) or exported to a
standalone C++ code ( TMultiLayerPerceptron::Export() ).</FONT></P>
<P><FONT SIZE=3>Finaly, note that even if this implementation is inspired from the mlpfit code,
the feature lists are not exactly matching:
<UL>
        <LI><P>mlpfit hybrid learning method is not implemented</P></LI>
        <LI><P>output neurons can be normalized, this is not the case for mlpfit</P></LI>
        <LI><P>the neural net is exported in C++, FORTRAN or PYTHON</P></LI>
        <LI><P>the drawResult() method allows a fast check of the learning procedure</P></LI>
</UL>
In addition, the paw version of mlpfit had additional limitations on the number of neurons, hidden layers and inputs/outputs that does not apply to TMultiLayerPerceptron.
<!-- */
// -->END_HTML

#include "TMultiLayerPerceptron.h"
#include "TSynapse.h"
#include "TNeuron.h"
#include "TClass.h"
#include "TTree.h"
#include "TEventList.h"
#include "TRandom3.h"
#include "TTimeStamp.h"
#include "TRegexp.h"
#include "TCanvas.h"
#include "TH2.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TMultiGraph.h"
#include "TDirectory.h"
#include "TSystem.h"
#include "Riostream.h"
#include "TMath.h"
#include "TTreeFormula.h"
#include "TTreeFormulaManager.h"
#include "TMarker.h"
#include "TLine.h"
#include "TText.h"
#include "TObjString.h"
#include <stdlib.h>

ClassImp(TMultiLayerPerceptron)

//______________________________________________________________________________
TMultiLayerPerceptron::TMultiLayerPerceptron()
{
   // Default constructor
   if(!TClass::GetClass("TTreePlayer")) gSystem->Load("libTreePlayer");
   fNetwork.SetOwner(true);
   fFirstLayer.SetOwner(false);
   fLastLayer.SetOwner(false);
   fSynapses.SetOwner(true);
   fData = 0;
   fCurrentTree = -1;
   fCurrentTreeWeight = 1;
   fStructure = "";
   fWeight = "1";
   fTraining = 0;
   fTrainingOwner = false;
   fTest = 0;
   fTestOwner = false;
   fEventWeight = 0;
   fManager = 0;
   fLearningMethod = TMultiLayerPerceptron::kBFGS;
   fEta = .1;
   fEtaDecay = 1;
   fDelta = 0;
   fEpsilon = 0;
   fTau = 3;
   fLastAlpha = 0;
   fReset = 50;
   fType = TNeuron::kSigmoid;
   fOutType =  TNeuron::kLinear;
   fextF = "";
   fextD = "";
}

//______________________________________________________________________________
TMultiLayerPerceptron::TMultiLayerPerceptron(const char * layout, TTree * data,
                                             TEventList * training,
                                             TEventList * test,
                                             TNeuron::ENeuronType type,
                                             const char* extF, const char* extD)
{
   // The network is described by a simple string:
   // The input/output layers are defined by giving
   // the branch names separated by comas.
   // Hidden layers are just described by the number of neurons.
   // The layers are separated by colons.
   // Ex: "x,y:10:5:f"
   // The output can be prepended by '@' if the variable has to be
   // normalized.
   // The output can be followed by '!' to use Softmax neurons for the
   // output layer only.
   // Ex: "x,y:10:5:c1,c2,c3!"
   // Input and outputs are taken from the TTree given as second argument.
   // training and test are the two TEventLists defining events
   // to be used during the neural net training.
   // Both the TTree and the TEventLists  can be defined in the constructor,
   // or later with the suited setter method.

   if(!TClass::GetClass("TTreePlayer")) gSystem->Load("libTreePlayer");
   fNetwork.SetOwner(true);
   fFirstLayer.SetOwner(false);
   fLastLayer.SetOwner(false);
   fSynapses.SetOwner(true);
   fStructure = layout;
   fData = data;
   fCurrentTree = -1;
   fCurrentTreeWeight = 1;
   fTraining = training;
   fTrainingOwner = false;
   fTest = test;
   fTestOwner = false;
   fWeight = "1";
   fType = type;
   fOutType =  TNeuron::kLinear;
   fextF = extF;
   fextD = extD;
   fEventWeight = 0;
   fManager = 0;
   if (data) {
      BuildNetwork();
      AttachData();
   }
   fLearningMethod = TMultiLayerPerceptron::kBFGS;
   fEta = .1;
   fEpsilon = 0;
   fDelta = 0;
   fEtaDecay = 1;
   fTau = 3;
   fLastAlpha = 0;
   fReset = 50;
}

//______________________________________________________________________________
TMultiLayerPerceptron::TMultiLayerPerceptron(const char * layout,
                                             const char * weight, TTree * data,
                                             TEventList * training,
                                             TEventList * test,
                                             TNeuron::ENeuronType type,
                                             const char* extF, const char* extD)
{
   // The network is described by a simple string:
   // The input/output layers are defined by giving
   // the branch names separated by comas.
   // Hidden layers are just described by the number of neurons.
   // The layers are separated by colons.
   // Ex: "x,y:10:5:f"
   // The output can be prepended by '@' if the variable has to be
   // normalized.
   // The output can be followed by '!' to use Softmax neurons for the
   // output layer only.
   // Ex: "x,y:10:5:c1,c2,c3!"
   // Input and outputs are taken from the TTree given as second argument.
   // training and test are the two TEventLists defining events
   // to be used during the neural net training.
   // Both the TTree and the TEventLists  can be defined in the constructor,
   // or later with the suited setter method.

   if(!TClass::GetClass("TTreePlayer")) gSystem->Load("libTreePlayer");
   fNetwork.SetOwner(true);
   fFirstLayer.SetOwner(false);
   fLastLayer.SetOwner(false);
   fSynapses.SetOwner(true);
   fStructure = layout;
   fData = data;
   fCurrentTree = -1;
   fCurrentTreeWeight = 1;
   fTraining = training;
   fTrainingOwner = false;
   fTest = test;
   fTestOwner = false;
   fWeight = weight;
   fType = type;
   fOutType =  TNeuron::kLinear;
   fextF = extF;
   fextD = extD;
   fEventWeight = 0;
   fManager = 0;
   if (data) {
      BuildNetwork();
      AttachData();
   }
   fLearningMethod = TMultiLayerPerceptron::kBFGS;
   fEta = .1;
   fEtaDecay = 1;
   fDelta = 0;
   fEpsilon = 0;
   fTau = 3;
   fLastAlpha = 0;
   fReset = 50;
}

//______________________________________________________________________________
TMultiLayerPerceptron::TMultiLayerPerceptron(const char * layout, TTree * data,
                                             const char * training,
                                             const char * test,
                                             TNeuron::ENeuronType type,
                                             const char* extF, const char* extD)
{
   // The network is described by a simple string:
   // The input/output layers are defined by giving
   // the branch names separated by comas.
   // Hidden layers are just described by the number of neurons.
   // The layers are separated by colons.
   // Ex: "x,y:10:5:f"
   // The output can be prepended by '@' if the variable has to be
   // normalized.
   // The output can be followed by '!' to use Softmax neurons for the
   // output layer only.
   // Ex: "x,y:10:5:c1,c2,c3!"
   // Input and outputs are taken from the TTree given as second argument.
   // training and test are two cuts (see TTreeFormula) defining events
   // to be used during the neural net training and testing.
   // Example: "Entry$%2", "(Entry$+1)%2".
   // Both the TTree and the cut can be defined in the constructor,
   // or later with the suited setter method.

   if(!TClass::GetClass("TTreePlayer")) gSystem->Load("libTreePlayer");
   fNetwork.SetOwner(true);
   fFirstLayer.SetOwner(false);
   fLastLayer.SetOwner(false);
   fSynapses.SetOwner(true);
   fStructure = layout;
   fData = data;
   fCurrentTree = -1;
   fCurrentTreeWeight = 1;
   fTraining = new TEventList(Form("fTrainingList_%lu",(ULong_t)this));
   fTrainingOwner = true;
   fTest = new TEventList(Form("fTestList_%lu",(ULong_t)this));
   fTestOwner = true;
   fWeight = "1";
   TString testcut = test;
   if(testcut=="") testcut = Form("!(%s)",training);
   fType = type;
   fOutType =  TNeuron::kLinear;
   fextF = extF;
   fextD = extD;
   fEventWeight = 0;
   fManager = 0;
   if (data) {
      BuildNetwork();
      data->Draw(Form(">>fTrainingList_%lu",(ULong_t)this),training,"goff");
      data->Draw(Form(">>fTestList_%lu",(ULong_t)this),(const char *)testcut,"goff");
      AttachData();
   }
   else {
      Warning("TMultiLayerPerceptron::TMultiLayerPerceptron","Data not set. Cannot define datasets");
   }
   fLearningMethod = TMultiLayerPerceptron::kBFGS;
   fEta = .1;
   fEtaDecay = 1;
   fDelta = 0;
   fEpsilon = 0;
   fTau = 3;
   fLastAlpha = 0;
   fReset = 50;
}

//______________________________________________________________________________
TMultiLayerPerceptron::TMultiLayerPerceptron(const char * layout,
                                             const char * weight, TTree * data,
                                             const char * training,
                                             const char * test,
                                             TNeuron::ENeuronType type,
                                             const char* extF, const char* extD)
{
   // The network is described by a simple string:
   // The input/output layers are defined by giving
   // the branch names separated by comas.
   // Hidden layers are just described by the number of neurons.
   // The layers are separated by colons.
   // Ex: "x,y:10:5:f"
   // The output can be prepended by '@' if the variable has to be
   // normalized.
   // The output can be followed by '!' to use Softmax neurons for the
   // output layer only.
   // Ex: "x,y:10:5:c1,c2,c3!"
   // Input and outputs are taken from the TTree given as second argument.
   // training and test are two cuts (see TTreeFormula) defining events
   // to be used during the neural net training and testing.
   // Example: "Entry$%2", "(Entry$+1)%2".
   // Both the TTree and the cut can be defined in the constructor,
   // or later with the suited setter method.

   if(!TClass::GetClass("TTreePlayer")) gSystem->Load("libTreePlayer");
   fNetwork.SetOwner(true);
   fFirstLayer.SetOwner(false);
   fLastLayer.SetOwner(false);
   fSynapses.SetOwner(true);
   fStructure = layout;
   fData = data;
   fCurrentTree = -1;
   fCurrentTreeWeight = 1;
   fTraining = new TEventList(Form("fTrainingList_%lu",(ULong_t)this));
   fTrainingOwner = true;
   fTest = new TEventList(Form("fTestList_%lu",(ULong_t)this));
   fTestOwner = true;
   fWeight = weight;
   TString testcut = test;
   if(testcut=="") testcut = Form("!(%s)",training);
   fType = type;
   fOutType =  TNeuron::kLinear;
   fextF = extF;
   fextD = extD;
   fEventWeight = 0;
   fManager = 0;
   if (data) {
      BuildNetwork();
      data->Draw(Form(">>fTrainingList_%lu",(ULong_t)this),training,"goff");
      data->Draw(Form(">>fTestList_%lu",(ULong_t)this),(const char *)testcut,"goff");
      AttachData();
   }
   else {
      Warning("TMultiLayerPerceptron::TMultiLayerPerceptron","Data not set. Cannot define datasets");
   }
   fLearningMethod = TMultiLayerPerceptron::kBFGS;
   fEta = .1;
   fEtaDecay = 1;
   fDelta = 0;
   fEpsilon = 0;
   fTau = 3;
   fLastAlpha = 0;
   fReset = 50;
}

//______________________________________________________________________________
TMultiLayerPerceptron::~TMultiLayerPerceptron()
{
   // Destructor
   if(fTraining && fTrainingOwner) delete fTraining;
   if(fTest && fTestOwner) delete fTest;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetData(TTree * data)
{
   // Set the data source
   if (fData) {
      cerr << "Error: data already defined." << endl;
      return;
   }
   fData = data;
   if (data) {
      BuildNetwork();
      AttachData();
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetEventWeight(const char * branch)
{
   // Set the event weight
   fWeight=branch;
   if (fData) {
      if (fEventWeight) {
         fManager->Remove(fEventWeight);
         delete fEventWeight;
      }
      fManager->Add((fEventWeight = new TTreeFormula("NNweight",fWeight.Data(),fData)));
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetTrainingDataSet(TEventList* train)
{
   // Sets the Training dataset.
   // Those events will be used for the minimization
   if(fTraining && fTrainingOwner) delete fTraining;
   fTraining = train;
   fTrainingOwner = false;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetTestDataSet(TEventList* test)
{
   // Sets the Test dataset.
   // Those events will not be used for the minimization but for control
   if(fTest && fTestOwner) delete fTest;
   fTest = test;
   fTestOwner = false;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetTrainingDataSet(const char * train)
{
   // Sets the Training dataset.
   // Those events will be used for the minimization.
   // Note that the tree must be already defined.
   if(fTraining && fTrainingOwner) delete fTraining;
   fTraining = new TEventList(Form("fTrainingList_%lu",(ULong_t)this));
   fTrainingOwner = true;
   if (fData) {
      fData->Draw(Form(">>fTrainingList_%lu",(ULong_t)this),train,"goff");
   }
   else {
      Warning("TMultiLayerPerceptron::TMultiLayerPerceptron","Data not set. Cannot define datasets");
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetTestDataSet(const char * test)
{
   // Sets the Test dataset.
   // Those events will not be used for the minimization but for control.
   // Note that the tree must be already defined.
   if(fTest && fTestOwner) {delete fTest; fTest=0;}
   if(fTest) if(strncmp(fTest->GetName(),Form("fTestList_%lu",(ULong_t)this),10)) delete fTest;
   fTest = new TEventList(Form("fTestList_%lu",(ULong_t)this));
   fTestOwner = true;
   if (fData) {
      fData->Draw(Form(">>fTestList_%lu",(ULong_t)this),test,"goff");
   }
   else {
      Warning("TMultiLayerPerceptron::TMultiLayerPerceptron","Data not set. Cannot define datasets");
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetLearningMethod(TMultiLayerPerceptron::ELearningMethod method)
{
   // Sets the learning method.
   // Available methods are: kStochastic, kBatch,
   // kSteepestDescent, kRibierePolak, kFletcherReeves and kBFGS.
   // (look at the constructor for the complete description
   // of learning methods and parameters)
   fLearningMethod = method;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetEta(Double_t eta)
{
   // Sets Eta - used in stochastic minimisation
   // (look at the constructor for the complete description
   // of learning methods and parameters)
   fEta = eta;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetEpsilon(Double_t eps)
{
   // Sets Epsilon - used in stochastic minimisation
   // (look at the constructor for the complete description
   // of learning methods and parameters)
   fEpsilon = eps;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetDelta(Double_t delta)
{
   // Sets Delta - used in stochastic minimisation
   // (look at the constructor for the complete description
   // of learning methods and parameters)
   fDelta = delta;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetEtaDecay(Double_t ed)
{
   // Sets EtaDecay - Eta *= EtaDecay at each epoch
   // (look at the constructor for the complete description
   // of learning methods and parameters)
   fEtaDecay = ed;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetTau(Double_t tau)
{
   // Sets Tau - used in line search
   // (look at the constructor for the complete description
   // of learning methods and parameters)
   fTau = tau;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetReset(Int_t reset)
{
   // Sets number of epochs between two resets of the
   // search direction to the steepest descent.
   // (look at the constructor for the complete description
   // of learning methods and parameters)
   fReset = reset;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::GetEntry(Int_t entry) const
{
   // Load an entry into the network
   if (!fData) return;
   fData->GetEntry(entry);
   if (fData->GetTreeNumber() != fCurrentTree) {
      ((TMultiLayerPerceptron*)this)->fCurrentTree = fData->GetTreeNumber();
      fManager->Notify();
      ((TMultiLayerPerceptron*)this)->fCurrentTreeWeight = fData->GetWeight();
   }
   Int_t nentries = fNetwork.GetEntriesFast();
   for (Int_t i=0;i<nentries;i++) {
      TNeuron *neuron = (TNeuron *)fNetwork.UncheckedAt(i);
      neuron->SetNewEvent();
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::Train(Int_t nEpoch, Option_t * option, Double_t minE)
{
   // Train the network.
   // nEpoch is the number of iterations.
   // option can contain:
   // - "text" (simple text output)
   // - "graph" (evoluting graphical training curves)
   // - "update=X" (step for the text/graph output update)
   // - "+" will skip the randomisation and start from the previous values.
   // - "current" (draw in the current canvas)
   // - "minErrorTrain" (stop when NN error on the training sample gets below minE
   // - "minErrorTest" (stop when NN error on the test sample gets below minE
   // All combinations are available.

   Int_t i;
   TString opt = option;
   opt.ToLower();
   // Decode options and prepare training.
   Int_t verbosity = 0;
   Bool_t newCanvas = true;
   Bool_t minE_Train = false;
   Bool_t minE_Test  = false;
   if (opt.Contains("text"))
      verbosity += 1;
   if (opt.Contains("graph"))
      verbosity += 2;
   Int_t displayStepping = 1;
   if (opt.Contains("update=")) {
      TRegexp reg("update=[0-9]*");
      TString out = opt(reg);
      displayStepping = atoi(out.Data() + 7);
   }
   if (opt.Contains("current"))
      newCanvas = false;
   if (opt.Contains("minerrortrain"))
      minE_Train = true;
   if (opt.Contains("minerrortest"))
      minE_Test = true;
   TVirtualPad *canvas = 0;
   TMultiGraph *residual_plot = 0;
   TGraph *train_residual_plot = 0;
   TGraph *test_residual_plot = 0;
   if ((!fData) || (!fTraining) || (!fTest)) {
      Error("Train","Training/Test samples still not defined. Cannot train the neural network");
      return;
   }
   Info("Train","Using %d train and %d test entries.",
        fTraining->GetN(), fTest->GetN());
   // Text and Graph outputs
   if (verbosity % 2)
      cout << "Training the Neural Network" << endl;
   if (verbosity / 2) {
      residual_plot = new TMultiGraph;
      if(newCanvas)
         canvas = new TCanvas("NNtraining", "Neural Net training");
      else {
         canvas = gPad;
         if(!canvas) canvas = new TCanvas("NNtraining", "Neural Net training");
      }
      train_residual_plot = new TGraph(nEpoch);
      test_residual_plot  = new TGraph(nEpoch);
      canvas->SetLeftMargin(0.14);
      train_residual_plot->SetLineColor(4);
      test_residual_plot->SetLineColor(2);
      residual_plot->Add(train_residual_plot);
      residual_plot->Add(test_residual_plot);
      residual_plot->Draw("LA");
      if (residual_plot->GetXaxis())  residual_plot->GetXaxis()->SetTitle("Epoch");
      if (residual_plot->GetYaxis())  residual_plot->GetYaxis()->SetTitle("Error");
   }
   // If the option "+" is not set, one has to randomize the weights first
   if (!opt.Contains("+"))
      Randomize();
   // Initialisation
   fLastAlpha = 0;
   Int_t els = fNetwork.GetEntriesFast() + fSynapses.GetEntriesFast();
   Double_t *buffer = new Double_t[els];
   Double_t *dir = new Double_t[els];
   for (i = 0; i < els; i++)
      buffer[i] = 0;
   Int_t matrix_size = fLearningMethod==TMultiLayerPerceptron::kBFGS ? els : 1;
   TMatrixD bfgsh(matrix_size, matrix_size);
   TMatrixD gamma(matrix_size, 1);
   TMatrixD delta(matrix_size, 1);
   // Epoch loop. Here is the training itself.
   Double_t training_E = 1e10;
   Double_t test_E = 1e10;
   for (Int_t iepoch = 0; (iepoch < nEpoch) && (!minE_Train || training_E>minE) && (!minE_Test || test_E>minE) ; iepoch++) {
      switch (fLearningMethod) {
      case TMultiLayerPerceptron::kStochastic:
         {
            MLP_Stochastic(buffer);
            break;
         }
      case TMultiLayerPerceptron::kBatch:
         {
            ComputeDEDw();
            MLP_Batch(buffer);
            break;
         }
      case TMultiLayerPerceptron::kSteepestDescent:
         {
            ComputeDEDw();
            SteepestDir(dir);
            if (LineSearch(dir, buffer))
               MLP_Batch(buffer);
            break;
         }
      case TMultiLayerPerceptron::kRibierePolak:
         {
            ComputeDEDw();
            if (!(iepoch % fReset)) {
               SteepestDir(dir);
            } else {
               Double_t norm = 0;
               Double_t onorm = 0;
               for (i = 0; i < els; i++)
                  onorm += dir[i] * dir[i];
               Double_t prod = 0;
               Int_t idx = 0;
               TNeuron *neuron = 0;
               TSynapse *synapse = 0;
               Int_t nentries = fNetwork.GetEntriesFast();
               for (i=0;i<nentries;i++) {
                  neuron = (TNeuron *) fNetwork.UncheckedAt(i);
                  prod -= dir[idx++] * neuron->GetDEDw();
                  norm += neuron->GetDEDw() * neuron->GetDEDw();
               }
               nentries = fSynapses.GetEntriesFast();
               for (i=0;i<nentries;i++) {
                  synapse = (TSynapse *) fSynapses.UncheckedAt(i);
                  prod -= dir[idx++] * synapse->GetDEDw();
                  norm += synapse->GetDEDw() * synapse->GetDEDw();
               }
               ConjugateGradientsDir(dir, (norm - prod) / onorm);
            }
            if (LineSearch(dir, buffer))
               MLP_Batch(buffer);
            break;
         }
      case TMultiLayerPerceptron::kFletcherReeves:
         {
            ComputeDEDw();
            if (!(iepoch % fReset)) {
               SteepestDir(dir);
            } else {
               Double_t norm = 0;
               Double_t onorm = 0;
               for (i = 0; i < els; i++)
                  onorm += dir[i] * dir[i];
               TNeuron *neuron = 0;
               TSynapse *synapse = 0;
               Int_t nentries = fNetwork.GetEntriesFast();
               for (i=0;i<nentries;i++) {
                  neuron = (TNeuron *) fNetwork.UncheckedAt(i);
                  norm += neuron->GetDEDw() * neuron->GetDEDw();
               }
               nentries = fSynapses.GetEntriesFast();
               for (i=0;i<nentries;i++) {
                  synapse = (TSynapse *) fSynapses.UncheckedAt(i);
                  norm += synapse->GetDEDw() * synapse->GetDEDw();
               }
               ConjugateGradientsDir(dir, norm / onorm);
            }
            if (LineSearch(dir, buffer))
               MLP_Batch(buffer);
            break;
         }
      case TMultiLayerPerceptron::kBFGS:
         {
            SetGammaDelta(gamma, delta, buffer);
            if (!(iepoch % fReset)) {
               SteepestDir(dir);
               bfgsh.UnitMatrix();
            } else {
               if (GetBFGSH(bfgsh, gamma, delta)) {
                  SteepestDir(dir);
                  bfgsh.UnitMatrix();
               } else {
                  BFGSDir(bfgsh, dir);
               }
            }
            if (DerivDir(dir) > 0) {
               SteepestDir(dir);
               bfgsh.UnitMatrix();
            }
            if (LineSearch(dir, buffer)) {
               bfgsh.UnitMatrix();
               SteepestDir(dir);
               if (LineSearch(dir, buffer)) {
                  Error("TMultiLayerPerceptron::Train()","Line search fail");
                  iepoch = nEpoch;
               }
            }
            break;
         }
      }
      // Security: would the learning lead to non real numbers,
      // the learning should stop now.
      if (TMath::IsNaN(GetError(TMultiLayerPerceptron::kTraining))) {
         Error("TMultiLayerPerceptron::Train()","Stop.");
         iepoch = nEpoch;
      }
      // Process other ROOT events.  Time penalty is less than
      // 1/1000 sec/evt on a mobile AMD Athlon(tm) XP 1500+
      gSystem->ProcessEvents();
      training_E = TMath::Sqrt(GetError(TMultiLayerPerceptron::kTraining) / fTraining->GetN());
      test_E = TMath::Sqrt(GetError(TMultiLayerPerceptron::kTest) / fTest->GetN());
      // Intermediate graph and text output
      if ((verbosity % 2) && ((!(iepoch % displayStepping)) || (iepoch == nEpoch - 1))) {
         cout << "Epoch: " << iepoch
              << " learn=" << training_E
              << " test=" << test_E
              << endl;
      }
      if (verbosity / 2) {
         train_residual_plot->SetPoint(iepoch, iepoch,training_E);
         test_residual_plot->SetPoint(iepoch, iepoch,test_E);
         if (!iepoch) {
            Double_t trp = train_residual_plot->GetY()[iepoch];
            Double_t tep = test_residual_plot->GetY()[iepoch];
            for (i = 1; i < nEpoch; i++) {
               train_residual_plot->SetPoint(i, i, trp);
               test_residual_plot->SetPoint(i, i, tep);
            }
         }
         if ((!(iepoch % displayStepping)) || (iepoch == nEpoch - 1)) {
            if (residual_plot->GetYaxis()) {
               residual_plot->GetYaxis()->UnZoom();
               residual_plot->GetYaxis()->SetTitleOffset(1.4);
               residual_plot->GetYaxis()->SetDecimals();
            }
            canvas->Modified();
            canvas->Update();
         }
      }
   }
   // Cleaning
   delete [] buffer;
   delete [] dir;
   // Final Text and Graph outputs
   if (verbosity % 2)
      cout << "Training done." << endl;
   if (verbosity / 2) {
      TLegend *legend = new TLegend(.75, .80, .95, .95);
      legend->AddEntry(residual_plot->GetListOfGraphs()->At(0),
                       "Training sample", "L");
      legend->AddEntry(residual_plot->GetListOfGraphs()->At(1),
                       "Test sample", "L");
      legend->Draw();
   }
}

//______________________________________________________________________________
Double_t TMultiLayerPerceptron::Result(Int_t event, Int_t index) const
{
   // Computes the output for a given event.
   // Look at the output neuron designed by index.
   GetEntry(event);
   TNeuron *out = (TNeuron *) (fLastLayer.At(index));
   if (out)
      return out->GetValue();
   else
      return 0;
}

//______________________________________________________________________________
Double_t TMultiLayerPerceptron::GetError(Int_t event) const
{
   // Error on the output for a given event
   GetEntry(event);
   Double_t error = 0;
   // look at 1st output neruon to determine type and error function
   Int_t nEntries = fLastLayer.GetEntriesFast();
   if (nEntries == 0) return 0.0;
   switch (fOutType) {
   case (TNeuron::kSigmoid):
         error = GetCrossEntropyBinary();
         break;
   case (TNeuron::kSoftmax):
         error = GetCrossEntropy();
         break;
   case (TNeuron::kLinear):
         error = GetSumSquareError();
         break;
   default:
         // default to sum-of-squares error
         error = GetSumSquareError();
   }
   error *= fEventWeight->EvalInstance();
   error *= fCurrentTreeWeight;
   return error;
}

//______________________________________________________________________________
Double_t TMultiLayerPerceptron::GetError(TMultiLayerPerceptron::EDataSet set) const
{
   // Error on the whole dataset
   TEventList *list =
       ((set == TMultiLayerPerceptron::kTraining) ? fTraining : fTest);
   Double_t error = 0;
   Int_t i;
   if (list) {
      Int_t nEvents = list->GetN();
      for (i = 0; i < nEvents; i++) {
         error += GetError(list->GetEntry(i));
      }
   } else if (fData) {
      Int_t nEvents = (Int_t) fData->GetEntries();
      for (i = 0; i < nEvents; i++) {
         error += GetError(i);
      }
   }
   return error;
}

//______________________________________________________________________________
Double_t TMultiLayerPerceptron::GetSumSquareError() const
{
   // Error on the output for a given event
   Double_t error = 0;
   for (Int_t i = 0; i < fLastLayer.GetEntriesFast(); i++) {
      TNeuron *neuron = (TNeuron *) fLastLayer[i];
      error += neuron->GetError() * neuron->GetError();
   }
   return (error / 2.);
}

//______________________________________________________________________________
Double_t TMultiLayerPerceptron::GetCrossEntropyBinary() const
{
   // Cross entropy error for sigmoid output neurons, for a given event
   Double_t error = 0;
   for (Int_t i = 0; i < fLastLayer.GetEntriesFast(); i++) {
      TNeuron *neuron = (TNeuron *) fLastLayer[i];
      Double_t output = neuron->GetValue();     // sigmoid output and target
      Double_t target = neuron->GetTarget();    // values lie in [0,1]
      if (target < DBL_EPSILON) {
         if (output == 1.0)
            error = DBL_MAX;
         else
            error -= TMath::Log(1 - output);
      } else
      if ((1 - target) < DBL_EPSILON) {
         if (output == 0.0)
            error = DBL_MAX;
         else
            error -= TMath::Log(output);
      } else {
         if (output == 0.0 || output == 1.0)
            error = DBL_MAX;
         else
            error -= target * TMath::Log(output / target) + (1-target) * TMath::Log((1 - output)/(1 - target));
      }
   }
   return error;
}

//______________________________________________________________________________
Double_t TMultiLayerPerceptron::GetCrossEntropy() const
{
   // Cross entropy error for a softmax output neuron, for a given event
   Double_t error = 0;
   for (Int_t i = 0; i < fLastLayer.GetEntriesFast(); i++) {
      TNeuron *neuron = (TNeuron *) fLastLayer[i];
      Double_t output = neuron->GetValue();     // softmax output and target
      Double_t target = neuron->GetTarget();    // values lie in [0,1]
      if (target > DBL_EPSILON) {               // (target == 0) => dE = 0
         if (output == 0.0)
            error = DBL_MAX;
         else
            error -= target * TMath::Log(output / target);
      }
   }
   return error;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::ComputeDEDw() const
{
   // Compute the DEDw = sum on all training events of dedw for each weight
   // normalized by the number of events.
   Int_t i,j;
   Int_t nentries = fSynapses.GetEntriesFast();
   TSynapse *synapse;
   for (i=0;i<nentries;i++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(i);
      synapse->SetDEDw(0.);
   }
   TNeuron *neuron;
   nentries = fNetwork.GetEntriesFast();
   for (i=0;i<nentries;i++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(i);
      neuron->SetDEDw(0.);
   }
   Double_t eventWeight = 1.;
   if (fTraining) {
      Int_t nEvents = fTraining->GetN();
      for (i = 0; i < nEvents; i++) {
         GetEntry(fTraining->GetEntry(i));
         eventWeight = fEventWeight->EvalInstance();
         eventWeight *= fCurrentTreeWeight;
         nentries = fSynapses.GetEntriesFast();
         for (j=0;j<nentries;j++) {
            synapse = (TSynapse *) fSynapses.UncheckedAt(j);
            synapse->SetDEDw(synapse->GetDEDw() + (synapse->GetDeDw()*eventWeight));
         }
         nentries = fNetwork.GetEntriesFast();
         for (j=0;j<nentries;j++) {
            neuron = (TNeuron *) fNetwork.UncheckedAt(j);
            neuron->SetDEDw(neuron->GetDEDw() + (neuron->GetDeDw()*eventWeight));
         }
      }
      nentries = fSynapses.GetEntriesFast();
      for (j=0;j<nentries;j++) {
         synapse = (TSynapse *) fSynapses.UncheckedAt(j);
         synapse->SetDEDw(synapse->GetDEDw() / (Double_t) nEvents);
      }
      nentries = fNetwork.GetEntriesFast();
      for (j=0;j<nentries;j++) {
         neuron = (TNeuron *) fNetwork.UncheckedAt(j);
         neuron->SetDEDw(neuron->GetDEDw() / (Double_t) nEvents);
      }
   } else if (fData) {
      Int_t nEvents = (Int_t) fData->GetEntries();
      for (i = 0; i < nEvents; i++) {
         GetEntry(i);
         eventWeight = fEventWeight->EvalInstance();
         eventWeight *= fCurrentTreeWeight;
         nentries = fSynapses.GetEntriesFast();
         for (j=0;j<nentries;j++) {
            synapse = (TSynapse *) fSynapses.UncheckedAt(j);
            synapse->SetDEDw(synapse->GetDEDw() + (synapse->GetDeDw()*eventWeight));
         }
         nentries = fNetwork.GetEntriesFast();
         for (j=0;j<nentries;j++) {
            neuron = (TNeuron *) fNetwork.UncheckedAt(j);
            neuron->SetDEDw(neuron->GetDEDw() + (neuron->GetDeDw()*eventWeight));
         }
      }
      nentries = fSynapses.GetEntriesFast();
      for (j=0;j<nentries;j++) {
         synapse = (TSynapse *) fSynapses.UncheckedAt(j);
         synapse->SetDEDw(synapse->GetDEDw() / (Double_t) nEvents);
      }
      nentries = fNetwork.GetEntriesFast();
      for (j=0;j<nentries;j++) {
         neuron = (TNeuron *) fNetwork.UncheckedAt(j);
         neuron->SetDEDw(neuron->GetDEDw() / (Double_t) nEvents);
      }
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::Randomize() const
{
   // Randomize the weights
   Int_t nentries = fSynapses.GetEntriesFast();
   Int_t j;
   TSynapse *synapse;
   TNeuron *neuron;
   TTimeStamp ts;
   TRandom3 gen(ts.GetSec());
   for (j=0;j<nentries;j++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(j);
      synapse->SetWeight(gen.Rndm() - 0.5);
   }
   nentries = fNetwork.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(j);
      neuron->SetWeight(gen.Rndm() - 0.5);
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::AttachData()
{
   // Connects the TTree to Neurons in input and output
   // layers. The formulas associated to each neuron are created
   // and reported to the network formula manager.
   // By default, the branch is not normalised since this would degrade
   // performance for classification jobs.
   // Normalisation can be requested by putting '@' in front of the formula.
   Int_t j = 0;
   TNeuron *neuron = 0;
   Bool_t normalize = false;
   fManager = new TTreeFormulaManager;
   //first layer
   const TString input = TString(fStructure(0, fStructure.First(':')));
   const TObjArray *inpL = input.Tokenize(", ");
   Int_t nentries = fFirstLayer.GetEntriesFast();
   // make sure nentries == entries in inpL
   R__ASSERT(nentries == inpL->GetLast()+1);
   for (j=0;j<nentries;j++) {
      normalize = false;
      const TString brName = ((TObjString *)inpL->At(j))->GetString();
      neuron = (TNeuron *) fFirstLayer.UncheckedAt(j);
      if (brName[0]=='@')
         normalize = true;
      fManager->Add(neuron->UseBranch(fData,brName.Data() + (normalize?1:0)));
      if(!normalize) neuron->SetNormalisation(0., 1.);
   }
   delete inpL;

   // last layer
   TString output = TString(
           fStructure(fStructure.Last(':') + 1,
                      fStructure.Length() - fStructure.Last(':')));
   const TObjArray *outL = output.Tokenize(", ");
   nentries = fLastLayer.GetEntriesFast();
   // make sure nentries == entries in outL
   R__ASSERT(nentries == outL->GetLast()+1);
   for (j=0;j<nentries;j++) {
      normalize = false;
      const TString brName = ((TObjString *)outL->At(j))->GetString();
      neuron = (TNeuron *) fLastLayer.UncheckedAt(j);
      if (brName[0]=='@')
         normalize = true;
      fManager->Add(neuron->UseBranch(fData,brName.Data() + (normalize?1:0)));
      if(!normalize) neuron->SetNormalisation(0., 1.);
   }
   delete outL;

   fManager->Add((fEventWeight = new TTreeFormula("NNweight",fWeight.Data(),fData)));
   //fManager->Sync();
}

//______________________________________________________________________________
void TMultiLayerPerceptron::ExpandStructure()
{
   // Expand the structure of the first layer
   TString input  = TString(fStructure(0, fStructure.First(':')));
   const TObjArray *inpL = input.Tokenize(", ");
   Int_t nneurons = inpL->GetLast()+1;

   TString hiddenAndOutput = TString(
         fStructure(fStructure.First(':') + 1,
                    fStructure.Length() - fStructure.First(':')));
   TString newInput;
   Int_t i = 0;
   TTreeFormula* f = 0;
   // loop on input neurons
   for (i = 0; i<nneurons; i++) {
      const TString name = ((TObjString *)inpL->At(i))->GetString();
      f = new TTreeFormula("sizeTestFormula",name,fData);
      // Variable size arrays are unrelialable
      if(f->GetMultiplicity()==1 && f->GetNdata()>1) {
         Warning("TMultiLayerPerceptron::ExpandStructure()","Variable size arrays cannot be used to build implicitely an input layer. The index 0 will be assumed.");
      }
      // Check if we are coping with an array... then expand
      // The array operator used is {}. It is detected in TNeuron, and
      // passed directly as instance index of the TTreeFormula,
      // so that complex compounds made of arrays can be used without
      // parsing the details.
      else if(f->GetNdata()>1) {
         for(Int_t j=0; j<f->GetNdata(); j++) {
            if(i||j) newInput += ",";
            newInput += name;
            newInput += "{";
            newInput += j;
            newInput += "}";
         }
         continue;
      }
      if(i) newInput += ",";
      newInput += name;
   }
   delete inpL;

   // Save the result
   fStructure = newInput + ":" + hiddenAndOutput;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::BuildNetwork()
{
   // Instanciates the network from the description
   ExpandStructure();
   TString input  = TString(fStructure(0, fStructure.First(':')));
   TString hidden = TString(
           fStructure(fStructure.First(':') + 1,
                      fStructure.Last(':') - fStructure.First(':') - 1));
   TString output = TString(
           fStructure(fStructure.Last(':') + 1,
                      fStructure.Length() - fStructure.Last(':')));
   Int_t bll = atoi(TString(
           hidden(hidden.Last(':') + 1,
                  hidden.Length() - (hidden.Last(':') + 1))).Data());
   if (input.Length() == 0) {
      Error("BuildNetwork()","malformed structure. No input layer.");
      return;
   }
   if (output.Length() == 0) {
      Error("BuildNetwork()","malformed structure. No output layer.");
      return;
   }
   BuildFirstLayer(input);
   BuildHiddenLayers(hidden);
   BuildLastLayer(output, bll);
}

//______________________________________________________________________________
void TMultiLayerPerceptron::BuildFirstLayer(TString & input)
{
   // Instanciates the neurons in input
   // Inputs are normalised and the type is set to kOff
   // (simple forward of the formula value)

   const TObjArray *inpL = input.Tokenize(", ");
   const Int_t nneurons =inpL->GetLast()+1;
   TNeuron *neuron = 0;
   Int_t i = 0;
   for (i = 0; i<nneurons; i++) {
      const TString name = ((TObjString *)inpL->At(i))->GetString();
      neuron = new TNeuron(TNeuron::kOff, name);
      fFirstLayer.AddLast(neuron);
      fNetwork.AddLast(neuron);
   }
   delete inpL;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::BuildHiddenLayers(TString & hidden)
{
   // Builds hidden layers.
   Int_t beg = 0;
   Int_t end = hidden.Index(":", beg + 1);
   Int_t prevStart = 0;
   Int_t prevStop = fNetwork.GetEntriesFast();
   Int_t layer = 1;
   while (end != -1) {
      BuildOneHiddenLayer(hidden(beg, end - beg), layer, prevStart, prevStop, false);
      beg = end + 1;
      end = hidden.Index(":", beg + 1);
   }

   BuildOneHiddenLayer(hidden(beg, hidden.Length() - beg), layer, prevStart, prevStop, true);
}

//______________________________________________________________________________
void TMultiLayerPerceptron::BuildOneHiddenLayer(const TString& sNumNodes, Int_t& layer,
                                                  Int_t& prevStart, Int_t& prevStop,
                                                  Bool_t lastLayer)
{
   // Builds a hidden layer, updates the number of layers.
   TNeuron *neuron = 0;
   TSynapse *synapse = 0;
   TString name;
   if (!sNumNodes.IsAlnum() || sNumNodes.IsAlpha()) {
      Error("BuildOneHiddenLayer",
            "The specification '%s' for hidden layer %d must contain only numbers!",
            sNumNodes.Data(), layer - 1);
   } else {
      Int_t num = atoi(sNumNodes.Data());
      for (Int_t i = 0; i < num; i++) {
         name.Form("HiddenL%d:N%d",layer,i);
         neuron = new TNeuron(fType, name, "", (const char*)fextF, (const char*)fextD);
         fNetwork.AddLast(neuron);
         for (Int_t j = prevStart; j < prevStop; j++) {
            synapse = new TSynapse((TNeuron *) fNetwork[j], neuron);
            fSynapses.AddLast(synapse);
         }
      }

      if (!lastLayer) {
         // tell each neuron which ones are in its own layer (for Softmax)
         Int_t nEntries = fNetwork.GetEntriesFast();
         for (Int_t i = prevStop; i < nEntries; i++) {
            neuron = (TNeuron *) fNetwork[i];
            for (Int_t j = prevStop; j < nEntries; j++)
               neuron->AddInLayer((TNeuron *) fNetwork[j]);
         }
      }

      prevStart = prevStop;
      prevStop = fNetwork.GetEntriesFast();
      layer++;
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::BuildLastLayer(TString & output, Int_t prev)
{
   // Builds the output layer
   // Neurons are linear combinations of input, by defaul.
   // If the structure ends with "!", neurons are set up for classification,
   // ie. with a sigmoid (1 neuron) or softmax (more neurons) activation function.

   Int_t nneurons = output.CountChar(',')+1;
   if (fStructure.EndsWith("!")) {
      fStructure = TString(fStructure(0, fStructure.Length() - 1));  // remove "!"
      if (nneurons == 1)
         fOutType = TNeuron::kSigmoid;
      else
         fOutType = TNeuron::kSoftmax;
   }
   Int_t prevStop = fNetwork.GetEntriesFast();
   Int_t prevStart = prevStop - prev;
   Ssiz_t pos = 0;
   TNeuron *neuron;
   TSynapse *synapse;
   TString name;
   Int_t i,j;
   for (i = 0; i<nneurons; i++) {
      Ssiz_t nextpos=output.Index(",",pos);
      if (nextpos!=kNPOS)
         name=output(pos,nextpos-pos);
      else name=output(pos,output.Length());
      pos+=nextpos+1;
      neuron = new TNeuron(fOutType, name);
      for (j = prevStart; j < prevStop; j++) {
         synapse = new TSynapse((TNeuron *) fNetwork[j], neuron);
         fSynapses.AddLast(synapse);
      }
      fLastLayer.AddLast(neuron);
      fNetwork.AddLast(neuron);
   }
   // tell each neuron which ones are in its own layer (for Softmax)
   Int_t nEntries = fNetwork.GetEntriesFast();
   for (i = prevStop; i < nEntries; i++) {
      neuron = (TNeuron *) fNetwork[i];
      for (j = prevStop; j < nEntries; j++)
         neuron->AddInLayer((TNeuron *) fNetwork[j]);
   }

}

//______________________________________________________________________________
void TMultiLayerPerceptron::DrawResult(Int_t index, Option_t * option) const
{
   // Draws the neural net output
   // It produces an histogram with the output for the two datasets.
   // Index is the number of the desired output neuron.
   // "option" can contain:
   // - test or train to select a dataset
   // - comp to produce a X-Y comparison plot
   // - nocanv to not create a new TCanvas for the plot

   TString opt = option;
   opt.ToLower();
   TNeuron *out = (TNeuron *) (fLastLayer.At(index));
   if (!out) {
      Error("DrawResult()","no such output.");
      return;
   }
   //TCanvas *canvas = new TCanvas("NNresult", "Neural Net output");
   if (!opt.Contains("nocanv"))
      new TCanvas("NNresult", "Neural Net output");
   const Double_t *norm = out->GetNormalisation();
   TEventList *events = 0;
   TString setname;
   Int_t i;
   if (opt.Contains("train")) {
      events = fTraining;
      setname = Form("train%d",index);
   } else if (opt.Contains("test")) {
      events = fTest;
      setname = Form("test%d",index);
   }
   if ((!fData) || (!events)) {
      Error("DrawResult()","no dataset.");
      return;
   }
   if (opt.Contains("comp")) {
      //comparison plot
      TString title = "Neural Net Output control. ";
      title += setname;
      setname = "MLP_" + setname + "_comp";
      TH2D *hist = ((TH2D *) gDirectory->Get(setname.Data()));
      if (!hist)
         hist = new TH2D(setname.Data(), title.Data(), 50, -1, 1, 50, -1, 1);
      hist->Reset();
      Int_t nEvents = events->GetN();
      for (i = 0; i < nEvents; i++) {
         GetEntry(events->GetEntry(i));
         hist->Fill(out->GetValue(), (out->GetBranch() - norm[1]) / norm[0]);
      }
      hist->Draw();
   } else {
      //output plot
      TString title = "Neural Net Output. ";
      title += setname;
      setname = "MLP_" + setname;
      TH1D *hist = ((TH1D *) gDirectory->Get(setname.Data()));
      if (!hist)
         hist = new TH1D(setname, title, 50, 1, -1);
      hist->Reset();
      Int_t nEvents = events->GetN();
      for (i = 0; i < nEvents; i++)
         hist->Fill(Result(events->GetEntry(i), index));
      hist->Draw();
      if (opt.Contains("train") && opt.Contains("test")) {
         events = fTraining;
         setname = "train";
         hist = ((TH1D *) gDirectory->Get("MLP_test"));
         if (!hist)
            hist = new TH1D(setname, title, 50, 1, -1);
         hist->Reset();
         nEvents = events->GetN();
         for (i = 0; i < nEvents; i++)
            hist->Fill(Result(events->GetEntry(i), index));
         hist->Draw("same");
      }
   }
}

//______________________________________________________________________________
Bool_t TMultiLayerPerceptron::DumpWeights(Option_t * filename) const
{
   // Dumps the weights to a text file.
   // Set filename to "-" (default) to dump to the standard output
   TString filen = filename;
   ostream * output;
   if (filen == "") {   
      Error("TMultiLayerPerceptron::DumpWeights()","Invalid file name");
      return kFALSE;
   }
   if (filen == "-")
      output = &cout;
   else
      output = new ofstream(filen.Data());
   TNeuron *neuron = 0;
   *output << "#input normalization" << endl;
   Int_t nentries = fFirstLayer.GetEntriesFast();
   Int_t j=0;
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fFirstLayer.UncheckedAt(j);
      *output << neuron->GetNormalisation()[0] << " "
              << neuron->GetNormalisation()[1] << endl;
   }
   *output << "#output normalization" << endl;
   nentries = fLastLayer.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fLastLayer.UncheckedAt(j);
      *output << neuron->GetNormalisation()[0] << " "
              << neuron->GetNormalisation()[1] << endl;
   }
   *output << "#neurons weights" << endl;
   TObjArrayIter *it = (TObjArrayIter *) fNetwork.MakeIterator();
   while ((neuron = (TNeuron *) it->Next()))
      *output << neuron->GetWeight() << endl;
   delete it;
   it = (TObjArrayIter *) fSynapses.MakeIterator();
   TSynapse *synapse = 0;
   *output << "#synapses weights" << endl;
   while ((synapse = (TSynapse *) it->Next()))
      *output << synapse->GetWeight() << endl;
   delete it;
   if (filen != "-") {
      ((ofstream *) output)->close();
      delete output;
   }
   return kTRUE;
}

//______________________________________________________________________________
Bool_t TMultiLayerPerceptron::LoadWeights(Option_t * filename)
{
   // Loads the weights from a text file conforming to the format
   // defined by DumpWeights.
   TString filen = filename;
   Double_t w;
   if (filen == "") {
      Error("TMultiLayerPerceptron::LoadWeights()","Invalid file name");
      return kFALSE;
   }
   char *buff = new char[100];
   ifstream input(filen.Data());
   // input normalzation
   input.getline(buff, 100);
   TObjArrayIter *it = (TObjArrayIter *) fFirstLayer.MakeIterator();
   Float_t n1,n2;
   TNeuron *neuron = 0;
   while ((neuron = (TNeuron *) it->Next())) {
      input >> n1 >> n2;
      neuron->SetNormalisation(n2,n1);
   }
   input.getline(buff, 100);
   // output normalization
   input.getline(buff, 100);
   delete it;
   it = (TObjArrayIter *) fLastLayer.MakeIterator();
   while ((neuron = (TNeuron *) it->Next())) {
      input >> n1 >> n2;
      neuron->SetNormalisation(n2,n1);
   }
   input.getline(buff, 100);
   // neuron weights
   input.getline(buff, 100);
   delete it;
   it = (TObjArrayIter *) fNetwork.MakeIterator();
   while ((neuron = (TNeuron *) it->Next())) {
      input >> w;
      neuron->SetWeight(w);
   }
   delete it;
   input.getline(buff, 100);
   // synapse weights
   input.getline(buff, 100);
   it = (TObjArrayIter *) fSynapses.MakeIterator();
   TSynapse *synapse = 0;
   while ((synapse = (TSynapse *) it->Next())) {
      input >> w;
      synapse->SetWeight(w);
   }
   delete it;
   delete[] buff;
   return kTRUE;
}

//______________________________________________________________________________
Double_t TMultiLayerPerceptron::Evaluate(Int_t index, Double_t *params) const
{
   // Returns the Neural Net for a given set of input parameters
   // #parameters must equal #input neurons

   TObjArrayIter *it = (TObjArrayIter *) fNetwork.MakeIterator();
   TNeuron *neuron;
   while ((neuron = (TNeuron *) it->Next()))
      neuron->SetNewEvent();
   delete it;
   it = (TObjArrayIter *) fFirstLayer.MakeIterator();
   Int_t i=0;
   while ((neuron = (TNeuron *) it->Next()))
      neuron->ForceExternalValue(params[i++]);
   delete it;
   TNeuron *out = (TNeuron *) (fLastLayer.At(index));
   if (out)
      return out->GetValue();
   else
      return 0;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::Export(Option_t * filename, Option_t * language) const
{
   // Exports the NN as a function for any non-ROOT-dependant code
   // Supported languages are: only C++ , FORTRAN and Python (yet)
   // This feature is also usefull if you want to plot the NN as
   // a function (TF1 or TF2).

   TString lg = language;
   lg.ToUpper();
   Int_t i;
   if(GetType()==TNeuron::kExternal) {
      Warning("TMultiLayerPerceptron::Export","Request to export a network using an external function");
   }
   if (lg == "C++") {
      TString basefilename = filename;
      Int_t slash = basefilename.Last('/')+1;
      if (slash) basefilename = TString(basefilename(slash, basefilename.Length()-slash));

      TString classname = basefilename;
      TString header = filename;
      header += ".h";
      TString source = filename;
      source += ".cxx";
      ofstream headerfile(header);
      ofstream sourcefile(source);
      headerfile << "#ifndef " << basefilename << "_h" << endl;
      headerfile << "#define " << basefilename << "_h" << endl << endl;
      headerfile << "class " << classname << " { " << endl;
      headerfile << "public:" << endl;
      headerfile << "   " << classname << "() {}" << endl;
      headerfile << "   ~" << classname << "() {}" << endl;
      sourcefile << "#include \"" << header << "\"" << endl;
      sourcefile << "#include <cmath>" << endl << endl;
      headerfile << "   double Value(int index";
      sourcefile << "double " << classname << "::Value(int index";
      for (i = 0; i < fFirstLayer.GetEntriesFast(); i++) {
         headerfile << ",double in" << i;
         sourcefile << ",double in" << i;
      }
      headerfile << ");" << endl;
      sourcefile << ") {" << endl;
      for (i = 0; i < fFirstLayer.GetEntriesFast(); i++)
         sourcefile << "   input" << i << " = (in" << i << " - "
             << ((TNeuron *) fFirstLayer[i])->GetNormalisation()[1] << ")/"
             << ((TNeuron *) fFirstLayer[i])->GetNormalisation()[0] << ";"
             << endl;
      sourcefile << "   switch(index) {" << endl;
      TNeuron *neuron;
      TObjArrayIter *it = (TObjArrayIter *) fLastLayer.MakeIterator();
      Int_t idx = 0;
      while ((neuron = (TNeuron *) it->Next()))
         sourcefile << "     case " << idx++ << ":" << endl
                    << "         return neuron" << neuron << "();" << endl;
      sourcefile << "     default:" << endl
                 << "         return 0.;" << endl << "   }"
                 << endl;
      sourcefile << "}" << endl << endl;
      headerfile << "   double Value(int index, double* input);" << endl;
      sourcefile << "double " << classname << "::Value(int index, double* input) {" << endl;
      for (i = 0; i < fFirstLayer.GetEntriesFast(); i++)
         sourcefile << "   input" << i << " = (input[" << i << "] - "
             << ((TNeuron *) fFirstLayer[i])->GetNormalisation()[1] << ")/"
             << ((TNeuron *) fFirstLayer[i])->GetNormalisation()[0] << ";"
             << endl;
      sourcefile << "   switch(index) {" << endl;
      delete it;
      it = (TObjArrayIter *) fLastLayer.MakeIterator();
      idx = 0;
      while ((neuron = (TNeuron *) it->Next()))
         sourcefile << "     case " << idx++ << ":" << endl
                    << "         return neuron" << neuron << "();" << endl;
      sourcefile << "     default:" << endl
                 << "         return 0.;" << endl << "   }"
                 << endl;
      sourcefile << "}" << endl << endl;
      headerfile << "private:" << endl;
      for (i = 0; i < fFirstLayer.GetEntriesFast(); i++)
         headerfile << "   double input" << i << ";" << endl;
      delete it;
      it = (TObjArrayIter *) fNetwork.MakeIterator();
      idx = 0;
      while ((neuron = (TNeuron *) it->Next())) {
         if (!neuron->GetPre(0)) {
            headerfile << "   double neuron" << neuron << "();" << endl;
            sourcefile << "double " << classname << "::neuron" << neuron
                       << "() {" << endl;
            sourcefile << "   return input" << idx++ << ";" << endl;
            sourcefile << "}" << endl << endl;
         } else {
            headerfile << "   double input" << neuron << "();" << endl;
            sourcefile << "double " << classname << "::input" << neuron
                       << "() {" << endl;
            sourcefile << "   double input = " << neuron->GetWeight()
                       << ";" << endl;
            TSynapse *syn = 0;
            Int_t n = 0;
            while ((syn = neuron->GetPre(n++))) {
               sourcefile << "   input += synapse" << syn << "();" << endl;
            }
            sourcefile << "   return input;" << endl;
            sourcefile << "}" << endl << endl;

            headerfile << "   double neuron" << neuron << "();" << endl;
            sourcefile << "double " << classname << "::neuron" << neuron << "() {" << endl;
            sourcefile << "   double input = input" << neuron << "();" << endl;
            switch(neuron->GetType()) {
               case (TNeuron::kSigmoid):
                  {
                     sourcefile << "   return ((input < -709. ? 0. : (1/(1+exp(-input)))) * ";
                     break;
                  }
               case (TNeuron::kLinear):
                  {
                     sourcefile << "   return (input * ";
                     break;
                  }
               case (TNeuron::kTanh):
                  {
                     sourcefile << "   return (tanh(input) * ";
                     break;
                  }
               case (TNeuron::kGauss):
                  {
                     sourcefile << "   return (exp(-input*input) * ";
                     break;
                  }
               case (TNeuron::kSoftmax):
                  {
                     sourcefile << "   return (exp(input) / (";
                     Int_t nn = 0;
                     TNeuron* side = neuron->GetInLayer(nn++);
                     sourcefile << "exp(input" << side << "())";
                     while ((side = neuron->GetInLayer(nn++)))
                        sourcefile << " + exp(input" << side << "())";
                     sourcefile << ") * ";
                     break;
                  }
               default:
                  {
                     sourcefile << "   return (0.0 * ";
                  }
            }
            sourcefile << neuron->GetNormalisation()[0] << ")+" ;
            sourcefile << neuron->GetNormalisation()[1] << ";" << endl;
            sourcefile << "}" << endl << endl;
         }
      }
      delete it;
      TSynapse *synapse = 0;
      it = (TObjArrayIter *) fSynapses.MakeIterator();
      while ((synapse = (TSynapse *) it->Next())) {
         headerfile << "   double synapse" << synapse << "();" << endl;
         sourcefile << "double " << classname << "::synapse"
                    << synapse << "() {" << endl;
         sourcefile << "   return (neuron" << synapse->GetPre()
                    << "()*" << synapse->GetWeight() << ");" << endl;
         sourcefile << "}" << endl << endl;
      }
      delete it;
      headerfile << "};" << endl << endl;
      headerfile << "#endif // " << basefilename << "_h" << endl << endl;
      headerfile.close();
      sourcefile.close();
      cout << header << " and " << source << " created." << endl;
   }
   else if(lg == "FORTRAN") {
      TString implicit = "      implicit double precision (a-h,n-z)\n";
      ofstream sigmoid("sigmoid.f");
      sigmoid         << "      double precision FUNCTION SIGMOID(X)"        << endl
                    << implicit
                << "      IF(X.GT.37.) THEN"                        << endl
                    << "         SIGMOID = 1."                        << endl
                << "      ELSE IF(X.LT.-709.) THEN"                << endl
                    << "         SIGMOID = 0."                        << endl
                    << "      ELSE"                                        << endl
                    << "         SIGMOID = 1./(1.+EXP(-X))"                << endl
                    << "      ENDIF"                                << endl
                    << "      END"                                        << endl;
      sigmoid.close();
      TString source = filename;
      source += ".f";
      ofstream sourcefile(source);

      // Header
      sourcefile << "      double precision function " << filename
                 << "(x, index)" << endl;
      sourcefile << implicit;
      sourcefile << "      double precision x(" <<
      fFirstLayer.GetEntriesFast() << ")" << endl << endl;

      // Last layer
      sourcefile << "C --- Last Layer" << endl;
      TNeuron *neuron;
      TObjArrayIter *it = (TObjArrayIter *) fLastLayer.MakeIterator();
      Int_t idx = 0;
      TString ifelseif = "      if (index.eq.";
      while ((neuron = (TNeuron *) it->Next())) {
         sourcefile << ifelseif.Data() << idx++ << ") then" << endl
                    << "          " << filename
                    << "=neuron" << neuron << "(x);" << endl;
         ifelseif = "      else if (index.eq.";
      }
      sourcefile << "      else" << endl
                 << "          " << filename << "=0.d0" << endl
                 << "      endif" << endl;
      sourcefile << "      end" << endl;

      // Network
      sourcefile << "C --- First and Hidden layers" << endl;
      delete it;
      it = (TObjArrayIter *) fNetwork.MakeIterator();
      idx = 0;
      while ((neuron = (TNeuron *) it->Next())) {
         sourcefile << "      double precision function neuron"
                    << neuron << "(x)" << endl
                    << implicit;
         sourcefile << "      double precision x("
                    << fFirstLayer.GetEntriesFast() << ")" << endl << endl;
         if (!neuron->GetPre(0)) {
            sourcefile << "      neuron" << neuron
             << " = (x(" << idx+1 << ") - "
             << ((TNeuron *) fFirstLayer[idx])->GetNormalisation()[1]
             << "d0)/"
             << ((TNeuron *) fFirstLayer[idx])->GetNormalisation()[0]
             << "d0" << endl;
            idx++;
         } else {
            sourcefile << "      neuron" << neuron
                       << " = " << neuron->GetWeight() << "d0" << endl;
            TSynapse *syn;
            Int_t n = 0;
            while ((syn = neuron->GetPre(n++)))
               sourcefile << "      neuron" << neuron
                              << " = neuron" << neuron
                          << " + synapse" << syn << "(x)" << endl;
            switch(neuron->GetType()) {
               case (TNeuron::kSigmoid):
                  {
                     sourcefile << "      neuron" << neuron
                                << "= (sigmoid(neuron" << neuron << ")*";
                     break;
                  }
               case (TNeuron::kLinear):
                  {
                     break;
                  }
               case (TNeuron::kTanh):
                  {
                     sourcefile << "      neuron" << neuron
                                << "= (tanh(neuron" << neuron << ")*";
                     break;
                  }
               case (TNeuron::kGauss):
                  {
                     sourcefile << "      neuron" << neuron
                                << "= (exp(-neuron" << neuron << "*neuron"
                                << neuron << "))*";
                     break;
                  }
               case (TNeuron::kSoftmax):
                  {
                     Int_t nn = 0;
                     TNeuron* side = neuron->GetInLayer(nn++);
                     sourcefile << "      div = exp(neuron" << side << "())" << endl;
                     while ((side = neuron->GetInLayer(nn++)))
                        sourcefile << "      div = div + exp(neuron" << side << "())" << endl;
                     sourcefile << "      neuron"  << neuron ;
                     sourcefile << "= (exp(neuron" << neuron << ") / div * ";
                     break;
                  }
               default:
                  {
                     sourcefile << "   neuron " << neuron << "= 0.";
                  }
            }
            sourcefile << neuron->GetNormalisation()[0] << "d0)+" ;
            sourcefile << neuron->GetNormalisation()[1] << "d0" << endl;
         }
         sourcefile << "      end" << endl;
      }
      delete it;

      // Synapses
      sourcefile << "C --- Synapses" << endl;
      TSynapse *synapse = 0;
      it = (TObjArrayIter *) fSynapses.MakeIterator();
      while ((synapse = (TSynapse *) it->Next())) {
         sourcefile << "      double precision function " << "synapse"
                    << synapse << "(x)\n" << implicit;
         sourcefile << "      double precision x("
                    << fFirstLayer.GetEntriesFast() << ")" << endl << endl;
         sourcefile << "      synapse" << synapse
                    << "=neuron" << synapse->GetPre()
                    << "(x)*" << synapse->GetWeight() << "d0" << endl;
         sourcefile << "      end" << endl << endl;
      }
      delete it;
      sourcefile.close();
      cout << source << " created." << endl;
   }
   else if(lg == "PYTHON") {
      TString classname = filename;
      TString pyfile = filename;
      pyfile += ".py";
      ofstream pythonfile(pyfile);
      pythonfile << "from math import exp" << endl << endl;
      pythonfile << "from math import tanh" << endl << endl;
      pythonfile << "class " << classname << ":" << endl;
      pythonfile << "\tdef value(self,index";
      for (i = 0; i < fFirstLayer.GetEntriesFast(); i++) {
         pythonfile << ",in" << i;
      }
      pythonfile << "):" << endl;
      for (i = 0; i < fFirstLayer.GetEntriesFast(); i++)
         pythonfile << "\t\tself.input" << i << " = (in" << i << " - "
             << ((TNeuron *) fFirstLayer[i])->GetNormalisation()[1] << ")/"
             << ((TNeuron *) fFirstLayer[i])->GetNormalisation()[0] << endl;
      TNeuron *neuron;
      TObjArrayIter *it = (TObjArrayIter *) fLastLayer.MakeIterator();
      Int_t idx = 0;
      while ((neuron = (TNeuron *) it->Next()))
         pythonfile << "\t\tif index==" << idx++
                    << ": return self.neuron" << neuron << "();" << endl;
      pythonfile << "\t\treturn 0." << endl;
      delete it;
      it = (TObjArrayIter *) fNetwork.MakeIterator();
      idx = 0;
      while ((neuron = (TNeuron *) it->Next())) {
         pythonfile << "\tdef neuron" << neuron << "(self):" << endl;
         if (!neuron->GetPre(0))
            pythonfile << "\t\treturn self.input" << idx++ << endl;
         else {
            pythonfile << "\t\tinput = " << neuron->GetWeight() << endl;
            TSynapse *syn;
            Int_t n = 0;
            while ((syn = neuron->GetPre(n++)))
               pythonfile << "\t\tinput = input + self.synapse"
                          << syn << "()" << endl;
            switch(neuron->GetType()) {
               case (TNeuron::kSigmoid):
                  {
                     pythonfile << "\t\tif input<-709. : return " << neuron->GetNormalisation()[1] << endl;
                     pythonfile << "\t\treturn ((1/(1+exp(-input)))*";
                     break;
                  }
               case (TNeuron::kLinear):
                  {
                     pythonfile << "\t\treturn (input*";
                     break;
                  }
               case (TNeuron::kTanh):
                  {
                     pythonfile << "\t\treturn (tanh(input)*";
                     break;
                  }
               case (TNeuron::kGauss):
                  {
                     pythonfile << "\t\treturn (exp(-input*input)*";
                     break;
                  }
               case (TNeuron::kSoftmax):
                  {
                     pythonfile << "\t\treturn (exp(input) / (";
                     Int_t nn = 0;
                     TNeuron* side = neuron->GetInLayer(nn++);
                     pythonfile << "exp(self.neuron" << side << "())";
                     while ((side = neuron->GetInLayer(nn++)))
                        pythonfile << " + exp(self.neuron" << side << "())";
                     pythonfile << ") * ";
                     break;
                  }
               default:
                  {
                     pythonfile << "\t\treturn 0.";
                  }
            }
            pythonfile << neuron->GetNormalisation()[0] << ")+" ;
            pythonfile << neuron->GetNormalisation()[1] << endl;
         }
      }
      delete it;
      TSynapse *synapse = 0;
      it = (TObjArrayIter *) fSynapses.MakeIterator();
      while ((synapse = (TSynapse *) it->Next())) {
         pythonfile << "\tdef synapse" << synapse << "(self):" << endl;
         pythonfile << "\t\treturn (self.neuron" << synapse->GetPre()
                    << "()*" << synapse->GetWeight() << ")" << endl;
      }
      delete it;
      pythonfile.close();
      cout << pyfile << " created." << endl;
   }
}

//______________________________________________________________________________
void TMultiLayerPerceptron::Shuffle(Int_t * index, Int_t n) const
{
   // Shuffle the Int_t index[n] in input.
   // Input:
   //   index: the array to shuffle
   //   n: the size of the array
   // Output:
   //   index: the shuffled indexes
   // This method is used for stochastic training

   TTimeStamp ts;
   TRandom3 rnd(ts.GetSec());
   Int_t j, k;
   Int_t a = n - 1;
   for (Int_t i = 0; i < n; i++) {
      j = (Int_t) (rnd.Rndm() * a);
      k = index[j];
      index[j] = index[i];
      index[i] = k;
   }
   return;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::MLP_Stochastic(Double_t * buffer)
{
   // One step for the stochastic method
   // buffer should contain the previous dw vector and will be updated

   Int_t nEvents = fTraining->GetN();
   Int_t *index = new Int_t[nEvents];
   Int_t i,j,nentries;
   for (i = 0; i < nEvents; i++)
      index[i] = i;
   fEta *= fEtaDecay;
   Shuffle(index, nEvents);
   TNeuron *neuron;
   TSynapse *synapse;
   for (i = 0; i < nEvents; i++) {
      GetEntry(fTraining->GetEntry(index[i]));
      // First compute DeDw for all neurons: force calculation before
      // modifying the weights.
      nentries = fFirstLayer.GetEntriesFast();
      for (j=0;j<nentries;j++) {
         neuron = (TNeuron *) fFirstLayer.UncheckedAt(j);
         neuron->GetDeDw();
      }
      Int_t cnt = 0;
      // Step for all neurons
      nentries = fNetwork.GetEntriesFast();
      for (j=0;j<nentries;j++) {
         neuron = (TNeuron *) fNetwork.UncheckedAt(j);
         buffer[cnt] = (-fEta) * (neuron->GetDeDw() + fDelta)
                       + fEpsilon * buffer[cnt];
         neuron->SetWeight(neuron->GetWeight() + buffer[cnt++]);
      }
      // Step for all synapses
      nentries = fSynapses.GetEntriesFast();
      for (j=0;j<nentries;j++) {
         synapse = (TSynapse *) fSynapses.UncheckedAt(j);
         buffer[cnt] = (-fEta) * (synapse->GetDeDw() + fDelta)
                       + fEpsilon * buffer[cnt];
         synapse->SetWeight(synapse->GetWeight() + buffer[cnt++]);
      }
   }
   delete[]index;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::MLP_Batch(Double_t * buffer)
{
   // One step for the batch (stochastic) method.
   // DEDw should have been updated before calling this.

   fEta *= fEtaDecay;
   Int_t cnt = 0;
   TObjArrayIter *it = (TObjArrayIter *) fNetwork.MakeIterator();
   TNeuron *neuron = 0;
   // Step for all neurons
   while ((neuron = (TNeuron *) it->Next())) {
      buffer[cnt] = (-fEta) * (neuron->GetDEDw() + fDelta)
                    + fEpsilon * buffer[cnt];
      neuron->SetWeight(neuron->GetWeight() + buffer[cnt++]);
   }
   delete it;
   it = (TObjArrayIter *) fSynapses.MakeIterator();
   TSynapse *synapse = 0;
   // Step for all synapses
   while ((synapse = (TSynapse *) it->Next())) {
      buffer[cnt] = (-fEta) * (synapse->GetDEDw() + fDelta)
                    + fEpsilon * buffer[cnt];
      synapse->SetWeight(synapse->GetWeight() + buffer[cnt++]);
   }
   delete it;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::MLP_Line(Double_t * origin, Double_t * dir, Double_t dist)
{
   // Sets the weights to a point along a line
   // Weights are set to [origin + (dist * dir)].

   Int_t idx = 0;
   TNeuron *neuron = 0;
   TSynapse *synapse = 0;
   TObjArrayIter *it = (TObjArrayIter *) fNetwork.MakeIterator();
   while ((neuron = (TNeuron *) it->Next())) {
      neuron->SetWeight(origin[idx] + (dir[idx] * dist));
      idx++;
   }
   delete it;
   it = (TObjArrayIter *) fSynapses.MakeIterator();
   while ((synapse = (TSynapse *) it->Next())) {
      synapse->SetWeight(origin[idx] + (dir[idx] * dist));
      idx++;
   }
   delete it;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SteepestDir(Double_t * dir)
{
   // Sets the search direction to steepest descent.
   Int_t idx = 0;
   TNeuron *neuron = 0;
   TSynapse *synapse = 0;
   TObjArrayIter *it = (TObjArrayIter *) fNetwork.MakeIterator();
   while ((neuron = (TNeuron *) it->Next()))
      dir[idx++] = -neuron->GetDEDw();
   delete it;
   it = (TObjArrayIter *) fSynapses.MakeIterator();
   while ((synapse = (TSynapse *) it->Next()))
      dir[idx++] = -synapse->GetDEDw();
   delete it;
}

//______________________________________________________________________________
bool TMultiLayerPerceptron::LineSearch(Double_t * direction, Double_t * buffer)
{
   // Search along the line defined by direction.
   // buffer is not used but is updated with the new dw
   // so that it can be used by a later stochastic step.
   // It returns true if the line search fails.

   Int_t idx = 0;
   Int_t j,nentries;
   TNeuron *neuron = 0;
   TSynapse *synapse = 0;
   // store weights before line search
   Double_t *origin = new Double_t[fNetwork.GetEntriesFast() +
                                   fSynapses.GetEntriesFast()];
   nentries = fNetwork.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(j);
      origin[idx++] = neuron->GetWeight();
   }
   nentries = fSynapses.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(j);
      origin[idx++] = synapse->GetWeight();
   }
   // try to find a triplet (alpha1, alpha2, alpha3) such that
   // Error(alpha1)>Error(alpha2)<Error(alpha3)
   Double_t err1 = GetError(kTraining);
   Double_t alpha1 = 0.;
   Double_t alpha2 = fLastAlpha;
   if (alpha2 < 0.01)
      alpha2 = 0.01;
   if (alpha2 > 2.0)
      alpha2 = 2.0;
   Double_t alpha3 = alpha2;
   MLP_Line(origin, direction, alpha2);
   Double_t err2 = GetError(kTraining);
   Double_t err3 = err2;
   Bool_t bingo = false;
   Int_t icount;
   if (err1 > err2) {
      for (icount = 0; icount < 100; icount++) {
         alpha3 *= fTau;
         MLP_Line(origin, direction, alpha3);
         err3 = GetError(kTraining);
         if (err3 > err2) {
            bingo = true;
            break;
         }
         alpha1 = alpha2;
         err1 = err2;
         alpha2 = alpha3;
         err2 = err3;
      }
      if (!bingo) {
         MLP_Line(origin, direction, 0.);
         delete[]origin;
         return true;
      }
   } else {
      for (icount = 0; icount < 100; icount++) {
         alpha2 /= fTau;
         MLP_Line(origin, direction, alpha2);
         err2 = GetError(kTraining);
         if (err1 > err2) {
            bingo = true;
            break;
         }
         alpha3 = alpha2;
         err3 = err2;
      }
      if (!bingo) {
         MLP_Line(origin, direction, 0.);
         delete[]origin;
         fLastAlpha = 0.05;
         return true;
      }
   }
   // Sets the weights to the bottom of parabola
   fLastAlpha = 0.5 * (alpha1 + alpha3 -
                (err3 - err1) / ((err3 - err2) / (alpha3 - alpha2)
                - (err2 - err1) / (alpha2 - alpha1)));
   fLastAlpha = fLastAlpha < 10000 ? fLastAlpha : 10000;
   MLP_Line(origin, direction, fLastAlpha);
   GetError(kTraining);
   // Stores weight changes (can be used by a later stochastic step)
   idx = 0;
   nentries = fNetwork.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(j);
      buffer[idx] = neuron->GetWeight() - origin[idx];
      idx++;
   }
   nentries = fSynapses.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(j);
      buffer[idx] = synapse->GetWeight() - origin[idx];
      idx++;
   }
   delete[]origin;
   return false;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::ConjugateGradientsDir(Double_t * dir, Double_t beta)
{
   // Sets the search direction to conjugate gradient direction
   // beta should be:
   //  ||g_{(t+1)}||^2 / ||g_{(t)}||^2                   (Fletcher-Reeves)
   //  g_{(t+1)} (g_{(t+1)}-g_{(t)}) / ||g_{(t)}||^2     (Ribiere-Polak)

   Int_t idx = 0;
   Int_t j,nentries;
   TNeuron *neuron = 0;
   TSynapse *synapse = 0;
   nentries = fNetwork.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(j);
      dir[idx] = -neuron->GetDEDw() + beta * dir[idx];
      idx++;
   }
   nentries = fSynapses.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(j);
      dir[idx] = -synapse->GetDEDw() + beta * dir[idx];
      idx++;
   }
}

//______________________________________________________________________________
bool TMultiLayerPerceptron::GetBFGSH(TMatrixD & bfgsh, TMatrixD & gamma, TMatrixD & delta)
{
   // Computes the hessian matrix using the BFGS update algorithm.
   // from gamma (g_{(t+1)}-g_{(t)}) and delta (w_{(t+1)}-w_{(t)}).
   // It returns true if such a direction could not be found
   // (if gamma and delta are orthogonal).

   TMatrixD gd(gamma, TMatrixD::kTransposeMult, delta);
   if ((Double_t) gd[0][0] == 0.)
      return true;
   TMatrixD aHg(bfgsh, TMatrixD::kMult, gamma);
   TMatrixD tmp(gamma, TMatrixD::kTransposeMult, bfgsh);
   TMatrixD gHg(gamma, TMatrixD::kTransposeMult, aHg);
   Double_t a = 1 / (Double_t) gd[0][0];
   Double_t f = 1 + ((Double_t) gHg[0][0] * a);
   TMatrixD res( TMatrixD(delta, TMatrixD::kMult,
                TMatrixD(TMatrixD::kTransposed, delta)));
   res *= f;
   res -= (TMatrixD(delta, TMatrixD::kMult, tmp) +
           TMatrixD(aHg, TMatrixD::kMult,
                   TMatrixD(TMatrixD::kTransposed, delta)));
   res *= a;
   bfgsh += res;
   return false;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::SetGammaDelta(TMatrixD & gamma, TMatrixD & delta,
                                          Double_t * buffer)
{
   // Sets the gamma (g_{(t+1)}-g_{(t)}) and delta (w_{(t+1)}-w_{(t)}) vectors
   // Gamma is computed here, so ComputeDEDw cannot have been called before,
   // and delta is a direct translation of buffer into a TMatrixD.

   Int_t els = fNetwork.GetEntriesFast() + fSynapses.GetEntriesFast();
   Int_t idx = 0;
   Int_t j,nentries;
   TNeuron *neuron = 0;
   TSynapse *synapse = 0;
   nentries = fNetwork.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(j);
      gamma[idx++][0] = -neuron->GetDEDw();
   }
   nentries = fSynapses.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(j);
      gamma[idx++][0] = -synapse->GetDEDw();
   }
   for (Int_t i = 0; i < els; i++)
      delta[i] = buffer[i];
   //delta.SetElements(buffer,"F");
   ComputeDEDw();
   idx = 0;
   nentries = fNetwork.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(j);
      gamma[idx++][0] += neuron->GetDEDw();
   }
   nentries = fSynapses.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(j);
      gamma[idx++][0] += synapse->GetDEDw();
   }
}

//______________________________________________________________________________
Double_t TMultiLayerPerceptron::DerivDir(Double_t * dir)
{
   // scalar product between gradient and direction
   // = derivative along direction

   Int_t idx = 0;
   Int_t j,nentries;
   Double_t output = 0;
   TNeuron *neuron = 0;
   TSynapse *synapse = 0;
   nentries = fNetwork.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(j);
      output += neuron->GetDEDw() * dir[idx++];
   }
   nentries = fSynapses.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(j);
      output += synapse->GetDEDw() * dir[idx++];
   }
   return output;
}

//______________________________________________________________________________
void TMultiLayerPerceptron::BFGSDir(TMatrixD & bfgsh, Double_t * dir)
{
   // Computes the direction for the BFGS algorithm as the product
   // between the Hessian estimate (bfgsh) and the dir.

   Int_t els = fNetwork.GetEntriesFast() + fSynapses.GetEntriesFast();
   TMatrixD dedw(els, 1);
   Int_t idx = 0;
   Int_t j,nentries;
   TNeuron *neuron = 0;
   TSynapse *synapse = 0;
   nentries = fNetwork.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      neuron = (TNeuron *) fNetwork.UncheckedAt(j);
      dedw[idx++][0] = neuron->GetDEDw();
   }
   nentries = fSynapses.GetEntriesFast();
   for (j=0;j<nentries;j++) {
      synapse = (TSynapse *) fSynapses.UncheckedAt(j);
      dedw[idx++][0] = synapse->GetDEDw();
   }
   TMatrixD direction(bfgsh, TMatrixD::kMult, dedw);
   for (Int_t i = 0; i < els; i++)
      dir[i] = -direction[i][0];
   //direction.GetElements(dir,"F");
}

//______________________________________________________________________________
void TMultiLayerPerceptron::Draw(Option_t * /*option*/)
{
  // Draws the network structure.
  // Neurons are depicted by a blue disk, and synapses by
  // lines connecting neurons.
  // The line width is proportionnal to the weight.

#define NeuronSize 2.5

   Int_t nLayers = fStructure.CountChar(':')+1;
   Float_t xStep = 1./(nLayers+1.);
   Int_t layer;
   for(layer=0; layer< nLayers-1; layer++) {
      Float_t nNeurons_this = 0;
      if(layer==0) {
         TString input      = TString(fStructure(0, fStructure.First(':')));
         nNeurons_this = input.CountChar(',')+1;
      }
      else {
         Int_t cnt=0;
         TString hidden = TString(fStructure(fStructure.First(':') + 1,fStructure.Last(':') - fStructure.First(':') - 1));
         Int_t beg = 0;
         Int_t end = hidden.Index(":", beg + 1);
         while (end != -1) {
            Int_t num = atoi(TString(hidden(beg, end - beg)).Data());
            cnt++;
            beg = end + 1;
            end = hidden.Index(":", beg + 1);
            if(layer==cnt) nNeurons_this = num;
         }
         Int_t num = atoi(TString(hidden(beg, hidden.Length() - beg)).Data());
         cnt++;
         if(layer==cnt) nNeurons_this = num;
      }
      Float_t nNeurons_next = 0;
      if(layer==nLayers-2) {
         TString output = TString(fStructure(fStructure.Last(':') + 1,fStructure.Length() - fStructure.Last(':')));
         nNeurons_next = output.CountChar(',')+1;
      }
      else {
         Int_t cnt=0;
         TString hidden = TString(fStructure(fStructure.First(':') + 1,fStructure.Last(':') - fStructure.First(':') - 1));
         Int_t beg = 0;
         Int_t end = hidden.Index(":", beg + 1);
         while (end != -1) {
            Int_t num = atoi(TString(hidden(beg, end - beg)).Data());
            cnt++;
            beg = end + 1;
            end = hidden.Index(":", beg + 1);
            if(layer+1==cnt) nNeurons_next = num;
         }
         Int_t num = atoi(TString(hidden(beg, hidden.Length() - beg)).Data());
         cnt++;
         if(layer+1==cnt) nNeurons_next = num;
      }
      Float_t yStep_this = 1./(nNeurons_this+1.);
      Float_t yStep_next = 1./(nNeurons_next+1.);
      TObjArrayIter* it = (TObjArrayIter *) fSynapses.MakeIterator();
      TSynapse *theSynapse = 0;
      Float_t maxWeight = 0;
      while ((theSynapse = (TSynapse *) it->Next()))
         maxWeight = maxWeight < theSynapse->GetWeight() ? theSynapse->GetWeight() : maxWeight;
      delete it;
      it = (TObjArrayIter *) fSynapses.MakeIterator();
      for(Int_t neuron1=0; neuron1<nNeurons_this; neuron1++) {
         for(Int_t neuron2=0; neuron2<nNeurons_next; neuron2++) {
            TLine* synapse = new TLine(xStep*(layer+1),yStep_this*(neuron1+1),xStep*(layer+2),yStep_next*(neuron2+1));
            synapse->Draw();
            theSynapse = (TSynapse *) it->Next();
            if (!theSynapse) continue;
            synapse->SetLineWidth(Int_t((theSynapse->GetWeight()/maxWeight)*10.));
            synapse->SetLineStyle(1);
            if(((TMath::Abs(theSynapse->GetWeight())/maxWeight)*10.)<0.5) synapse->SetLineStyle(2);
            if(((TMath::Abs(theSynapse->GetWeight())/maxWeight)*10.)<0.25) synapse->SetLineStyle(3);
         }
      }
      delete it;
   }
   for(layer=0; layer< nLayers; layer++) {
      Float_t nNeurons = 0;
      if(layer==0) {
         TString input      = TString(fStructure(0, fStructure.First(':')));
         nNeurons = input.CountChar(',')+1;
      }
      else if(layer==nLayers-1) {
         TString output = TString(fStructure(fStructure.Last(':') + 1,fStructure.Length() - fStructure.Last(':')));
         nNeurons = output.CountChar(',')+1;
      }
      else {
         Int_t cnt=0;
         TString hidden = TString(fStructure(fStructure.First(':') + 1,fStructure.Last(':') - fStructure.First(':') - 1));
         Int_t beg = 0;
         Int_t end = hidden.Index(":", beg + 1);
         while (end != -1) {
            Int_t num = atoi(TString(hidden(beg, end - beg)).Data());
            cnt++;
            beg = end + 1;
            end = hidden.Index(":", beg + 1);
            if(layer==cnt) nNeurons = num;
         }
         Int_t num = atoi(TString(hidden(beg, hidden.Length() - beg)).Data());
         cnt++;
         if(layer==cnt) nNeurons = num;
      }
      Float_t yStep = 1./(nNeurons+1.);
      for(Int_t neuron=0; neuron<nNeurons; neuron++) {
         TMarker* m = new TMarker(xStep*(layer+1),yStep*(neuron+1),20);
         m->SetMarkerColor(4);
         m->SetMarkerSize(NeuronSize);
         m->Draw();
      }
   }
   const TString input = TString(fStructure(0, fStructure.First(':')));
   const TObjArray *inpL = input.Tokenize(" ,");
   const Int_t nrItems = inpL->GetLast()+1;
   Float_t yStep = 1./(nrItems+1);
   for (Int_t item = 0; item < nrItems; item++) {
      const TString brName = ((TObjString *)inpL->At(item))->GetString();
      TText* label = new TText(0.5*xStep,yStep*(item+1),brName.Data());
      label->Draw();
   }
   delete inpL;

   Int_t numOutNodes=fLastLayer.GetEntriesFast();
   yStep=1./(numOutNodes+1);
   for (Int_t outnode=0; outnode<numOutNodes; outnode++) {
      TNeuron* neuron=(TNeuron*)fLastLayer[outnode];
      if (neuron && neuron->GetName()) {
         TText* label = new TText(xStep*nLayers,
                                  yStep*(outnode+1),
                                  neuron->GetName());
         label->Draw();
      }
   }
}
