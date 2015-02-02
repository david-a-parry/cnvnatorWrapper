# Module.mk for proof module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODNAME      := proof
MODDIR       := $(ROOT_SRCDIR)/proof/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

PROOFDIR     := $(MODDIR)
PROOFDIRS    := $(PROOFDIR)/src
PROOFDIRI    := $(PROOFDIR)/inc

##### libProof #####
PROOFL       := $(MODDIRI)/LinkDef.h
PROOFDS      := $(call stripsrc,$(MODDIRS)/G__Proof.cxx)
PROOFDO      := $(PROOFDS:.cxx=.o)
PROOFDH      := $(PROOFDS:.cxx=.h)

PROOFH_ALL   := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
PROOFS_ALL   := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))

# Build AliEn dataset manager only when AliEn is enabled
ifeq ($(BUILDALIEN),yes)
  ALIENDSMGR := -DALIENDSMGR
  PROOFH := $(PROOFH_ALL)
  PROOFS := $(PROOFS_ALL)
else
  PROOFS := $(filter-out $(MODDIRS)/TDataSetManagerAliEn%,$(PROOFS_ALL))
  PROOFH := $(filter-out $(MODDIRI)TDataSetManagerAliEn%,$(PROOFH_ALL))
endif

PROOFO   := $(call stripsrc,$(PROOFS:.cxx=.o))
PROOFDEP     := $(PROOFO:.o=.d) $(PROOFDO:.o=.d)

PROOFLIB     := $(LPATH)/libProof.$(SOEXT)
PROOFMAP     := $(PROOFLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(PROOFH))
ALLLIBS     += $(PROOFLIB)
ALLMAPS     += $(PROOFMAP)

# include all dependency files
INCLUDEFILES += $(PROOFDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h:    $(PROOFDIRI)/%.h
		cp $< $@

$(PROOFLIB):    $(PROOFO) $(PROOFDO) $(ORDER_) $(MAINLIBS) $(PROOFLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libProof.$(SOEXT) $@ "$(PROOFO) $(PROOFDO)" \
		   "$(PROOFLIBEXTRA)"

$(PROOFDS):     $(PROOFH) $(PROOFL) $(ROOTCINTTMPDEP)
		$(MAKEDIR)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(ALIENDSMGR) $(PROOFH) $(PROOFL)

$(PROOFMAP):    $(RLIBMAP) $(MAKEFILEDEP) $(PROOFL)
		$(RLIBMAP) -o $@ -l $(PROOFLIB) \
		   -d $(PROOFLIBDEPM) -c $(PROOFL)

all-$(MODNAME): $(PROOFLIB) $(PROOFMAP)

clean-$(MODNAME):
		@rm -f $(PROOFO) $(PROOFDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(PROOFDEP) $(PROOFDS) $(PROOFDH) $(PROOFLIB) $(PROOFMAP)

distclean::     distclean-$(MODNAME)

##### extra rules ######
ifeq ($(PLATFORM),linux)
ifeq ($(GCC_VERS_FULL),gcc-4.5.2)
ifneq ($(filter -O%,$(OPT)),)
   $(call stripsrc,$(PROOFDIRS)/TDataSetManager.o): OPT = -O
   $(call stripsrc,$(PROOFDIRS)/TDSet.o): OPT = -O
endif
endif
endif

# Optimize dictionary with stl containers.
$(PROOFDO): NOOPT = $(OPT)
