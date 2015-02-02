# Module.mk for guibuilder module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Valeriy Onuchin, 19/8/2004

MODNAME      := guibuilder
MODDIR       := $(ROOT_SRCDIR)/gui/$(MODNAME)
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GUIBLDDIR    := $(MODDIR)
GUIBLDDIRS   := $(GUIBLDDIR)/src
GUIBLDDIRI   := $(GUIBLDDIR)/inc

##### libGuiBld #####
GUIBLDL      := $(MODDIRI)/LinkDef.h
GUIBLDDS     := $(call stripsrc,$(MODDIRS)/G__GuiBld.cxx)
GUIBLDDO     := $(GUIBLDDS:.cxx=.o)
GUIBLDDH     := $(GUIBLDDS:.cxx=.h)

GUIBLDH      := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
GUIBLDS      := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GUIBLDO      := $(call stripsrc,$(GUIBLDS:.cxx=.o))

GUIBLDDEP    := $(GUIBLDO:.o=.d) $(GUIBLDDO:.o=.d)

GUIBLDLIB    := $(LPATH)/libGuiBld.$(SOEXT)
GUIBLDMAP    := $(GUIBLDLIB:.$(SOEXT)=.rootmap)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GUIBLDH))
ALLLIBS     += $(GUIBLDLIB)
ALLMAPS     += $(GUIBLDMAP)

# include all dependency files
INCLUDEFILES += $(GUIBLDDEP)

##### local rules #####
.PHONY:         all-$(MODNAME) clean-$(MODNAME) distclean-$(MODNAME)

include/%.h:    $(GUIBLDDIRI)/%.h
		cp $< $@

$(GUIBLDLIB):   $(GUIBLDO) $(GUIBLDDO) $(ORDER_) $(MAINLIBS) $(GUIBLDLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libGuiBld.$(SOEXT) $@ "$(GUIBLDO) $(GUIBLDDO)" \
		   "$(GUIBLDLIBEXTRA)"

$(GUIBLDDS):    $(GUIBLDH) $(GUIBLDL) $(ROOTCINTTMPDEP)
		$(MAKEDIR)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(GUIBLDH) $(GUIBLDL)

$(GUIBLDMAP):   $(RLIBMAP) $(MAKEFILEDEP) $(GUIBLDL)
		$(RLIBMAP) -o $@ -l $(GUIBLDLIB) \
		   -d $(GUIBLDLIBDEPM) -c $(GUIBLDL)

all-$(MODNAME): $(GUIBLDLIB) $(GUIBLDMAP)

clean-$(MODNAME):
		@rm -f $(GUIBLDO) $(GUIBLDDO)

clean::         clean-$(MODNAME)

distclean-$(MODNAME): clean-$(MODNAME)
		@rm -f $(GUIBLDDEP) $(GUIBLDDS) $(GUIBLDDH) $(GUIBLDLIB) $(GUIBLDMAP)

distclean::     distclean-$(MODNAME)
