#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*    Makefile for gasa01 with an additional standalone circlepacking target *
#*    Adapted from your original Makefile.                                   *
#*                                                                           *
#*    Usage: place your new source file circle_packing_scip.c into $(SRCDIR) *
#*           then run `make` (or `make circlepacking`) to build the new bin. *
#*                                                                           *
#*---------------------------------------------------------------------------*

#@file    Makefile

#-----------------------------------------------------------------------------
# paths
#-----------------------------------------------------------------------------

SCIPDIR         =       $(SCIP_PATH)
SCIPREALPATH	=	$(realpath $(SCIPDIR))

#-----------------------------------------------------------------------------
# include default project Makefile from SCIP
#-----------------------------------------------------------------------------

include $(SCIPDIR)/make/make.project

VERSION		=	2.0
TEST		=	basic
DFLAGS		= 	-MM

#-----------------------------------------------------------------------------
# Main Program (existing)
#-----------------------------------------------------------------------------

MAINNAME = circlepacking
MAINOBJ  = circlepacking.o

MAINSRC		=	$(addprefix $(SRCDIR)/,$(MAINOBJ:.o=.c))
MAINDEP		=	$(SRCDIR)/depend.cmain.$(OPT)

MAIN		=	$(MAINNAME).$(BASE).$(LPS)$(EXEEXTENSION)
MAINFILE	=	$(BINDIR)/$(MAIN)
MAINSHORTLINK	=	$(BINDIR)/$(MAINNAME)
MAINOBJFILES	=	$(addprefix $(OBJDIR)/,$(MAINOBJ))

GASSRC		=	$(addprefix $(SRCDIR)/,$(GASOBJ:.o=.c))

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

ifeq ($(VERBOSE),false)
.SILENT:    $(MAINFILE) $(MAINOBJFILES) $(MAINSHORTLINK) $(BINDIR)/test_ode_approx test_ode_approx $(OBJDIR)/test_ode_approx.o $(CPMAINFILE) $(CPOBJFILES) $(CPSHORTLINK)
endif

.PHONY: all
all:            $(SCIPDIR) $(MAINFILE) $(MAINSHORTLINK) $(CPMAINFILE) $(CPSHORTLINK)

.PHONY: scip
scip:
		@$(MAKE) -C $(SCIPDIR) libs $^

.PHONY: doc
doc:
		cd doc; $(DOXY) $(MAINNAME).dxy

$(MAINSHORTLINK):    $(MAINFILE)
		@rm -f $@
		cd $(dir $@) && ln -s $(notdir $(MAINFILE)) $(notdir $@)


$(OBJDIR):
		@-mkdir -p $(OBJDIR)

$(BINDIR):
		@-mkdir -p $(BINDIR)

.PHONY: clean
clean:        $(OBJDIR)
ifneq ($(OBJDIR),)
		@-(rm -f $(OBJDIR)/*.o && rmdir $(OBJDIR));
		@echo "-> remove main objective files"
endif
		@-rm -f $(MAINFILE) $(MAINLINK) $(MAINSHORTLINK) $(CPMAINFILE) $(CPSHORTLINK)
		@echo "-> remove binaries"

.PHONY: depend
depend:        $(SCIPDIR)
		$(SHELL) -ec '$(DCC) $(FLAGS) $(DFLAGS) $(MAINSRC) $(GASSRC) $(CPSRC) \
		| sed '\''s|^\([0-9A-Za-z\_]\{1,\}\)\.o *: *$(SRCDIR)/\([0-9A-Za-z\_]*\).c|$$\(OBJDIR\)/\2.o: $(SRCDIR)/\2.c|g'\'' \
		>$(MAINDEP)'

-include    $(MAINDEP)


# main target (existing)
$(MAINFILE):    $(BINDIR) $(OBJDIR) $(SCIPLIBFILE) $(LPILIBFILE) $(NLPILIBFILE) $(MAINOBJFILES) 
		@echo "-> linking $@"
ifdef LINKCXXSCIPALL
		$(LINKCXX) $(MAINOBJFILES) $(LINKCXXSCIPALL) $(LINKCXX_o)$@
else
		$(LINKCXX) $(MAINOBJFILES) \
		$(LINKCXX_L)$(SCIPDIR)/lib $(LINKCXX_l)$(SCIPLIB)$(LINKLIBSUFFIX) \
                $(LINKCXX_l)$(OBJSCIPLIB)$(LINKLIBSUFFIX) $(LINKCXX_l)$(LPILIB)$(LINKLIBSUFFIX) $(LINKCXX_l)$(NLPILIB)$(LINKLIBSUFFIX) \
                $(OFLAGS) $(LPSLDFLAGS) \
		$(LDFLAGS) $(LINKCXX_o)$@
endif


# circlepacking target (new)
$(CPMAINFILE):    $(BINDIR) $(OBJDIR) $(SCIPLIBFILE) $(LPILIBFILE) $(NLPILIBFILE) $(CPOBJFILES)
		@echo "-> linking $@"
ifdef LINKCXXSCIPALL
		$(LINKCXX) $(CPOBJFILES) $(LINKCXXSCIPALL) $(LINKCXX_o)$@
else
		$(LINKCXX) $(CPOBJFILES) \
		$(LINKCXX_L)$(SCIPDIR)/lib $(LINKCXX_l)$(SCIPLIB)$(LINKLIBSUFFIX) \
                $(LINKCXX_l)$(OBJSCIPLIB)$(LINKLIBSUFFIX) $(LINKCXX_l)$(LPILIB)$(LINKLIBSUFFIX) $(LINKCXX_l)$(NLPILIB)$(LINKLIBSUFFIX) \
                $(OFLAGS) $(LPSLDFLAGS) \
		$(LDFLAGS) $(LINKCXX_o)$@
endif


$(OBJDIR)/%.o:    $(SRCDIR)/%.c
		@echo "-> compiling $@"
		$(CC) $(FLAGS) $(OFLAGS) $(BINOFLAGS) $(CFLAGS) -c $< $(CC_o)$@

$(OBJDIR)/%.o:    $(SRCDIR)/%.cpp
		@echo "-> compiling $@"
		$(CXX) $(FLAGS) $(OFLAGS) $(BINOFLAGS) $(CXXFLAGS) -c $< $(CXX_o)$@

#---- EOF --------------------------------------------------------------------
