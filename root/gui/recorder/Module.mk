# Module.mk for recorder module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Bertrand Bellenot, 29/09/2008

MODNAME   := recorder
MODDIR    := $(ROOT_SRCDIR)/gui/$(MODNAME)
MODDIRS   := $(MODDIR)/src
MODDIRI   := $(MODDIR)/inc

RECDIR    := $(MODDIR)
RECDIRS   := $(RECDIR)/src
RECDIRI   := $(RECDIR)/inc

##### libRecorder #####
RECL      := $(MODDIRI)/LinkDef.h
RECDS     := $(call stripsrc,$(MODDIRS)/G__Recorder.cxx)
RECDO     := $(RECDS:.cxx=.o)
RECDH     := $(RECDS:.cxx=.h)

RECH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
RECS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
RECO      := $(call stripsrc,$(RECS:.cxx=.o))

RECDEP    := $(RECO:.o=.d) $(RECDO:.o=.d)

RECLIB    := $(LPATH)/libRecorder.$(SOEXT)
RECMAP    := $(RECLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(RECH))
ALLLIBS     += $(RECLIB)
ALLMAPS     += $(RECMAP)

# include all dependency files
INCLUDEFILES += $(RECDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h:    $(RECDIRI)/%.h
		cp $< $@

$(RECLIB):      $(RECO) $(RECDO) $(ORDER_) $(MAINLIBS) $(RECLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libRecorder.$(SOEXT) $@ "$(RECO) $(RECDO)" \
		   "$(RECLIBEXTRA)"

$(RECDS):       $(RECH) $(RECL) $(ROOTCINTTMPDEP)
		$(MAKEDIR)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(RECH) $(RECL)

$(RECMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(RECL)
		$(RLIBMAP) -o $@ -l $(RECLIB) \
		   -d $(RECLIBDEPM) -c $(RECL)

all-$(MODNAME): $(RECLIB) $(RECMAP)

clean-$(MODNAME):
		@rm -f $(RECO) $(RECDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(RECDEP) $(RECDS) $(RECDH) $(RECLIB) $(RECMAP)

distclean::     distclean-$(MODNAME)
