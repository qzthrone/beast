# This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0

# == Compiler & uname ==
# Example: uname_S=Linux uname_M=x86_64 uname_R=2.6.17-44-kvm
uname_S		::= $(shell uname -s 2>/dev/null || echo None)
uname_M		::= $(shell uname -m 2>/dev/null || echo None)
uname_R		::= $(shell uname -r 2>/dev/null || echo None)
CXXKIND		 != case $$($(CXX) --version 2>&1) in *"Free Software Foundation"*) echo gcc;; *clang*) echo clang;; *) echo UNKNOWN;; esac
HAVE_GCC	::= $(if $(findstring $(CXXKIND), gcc),1,)
HAVE_CLANG	::= $(if $(findstring $(CXXKIND), clang),1,)
ifeq ($(HAVE_GCC)$(HAVE_CLANG),)		# do we HAVE_ *any* recognized compiler?
$(error Compiler '$(CXX)' not recognized, version identifier: $(CXXKIND))
endif
CCACHE		 ?= $(if $(CCACHE_DIR), ccache)

# == C/CXX/LD Flags ==
COMMONFLAGS	::= -fno-strict-overflow -fno-strict-aliasing # sane C / C++
COMMONFLAGS	 += -Wall -Wdeprecated -Werror=format-security -Wredundant-decls -Wpointer-arith -Wmissing-declarations
COMMONFLAGS	 += -Werror=incompatible-pointer-types -Werror-implicit-function-declaration
#COMMONFLAGS	 += -Wdate-time -Wconversion -Wshadow
CONLYFLAGS	::= -Wmissing-prototypes -Wnested-externs -Wno-pointer-sign
CXXONLYFLAGS	::= -Woverloaded-virtual -Wsign-promo
#CXXONLYFLAGS	 += -Wnon-virtual-dtor -Wempty-body -Wignored-qualifiers -Wunreachable-code -Wtype-limits
OPTIMIZE	::= -O3 -funroll-loops -ftree-vectorize
LDOPTIMIZE	::= -O1 -Wl,--hash-style=both -Wl,--compress-debug-sections=zlib
ifdef HAVE_CLANG  # clang++
  COMMONFLAGS	 += -Wno-tautological-compare -Wno-constant-logical-operand
  #COMMONFLAGS	 += -Wno-unused-command-line-argument
endif
ifdef HAVE_GCC    # g++
  COMMONFLAGS	 += -fno-delete-null-pointer-checks
  OPTIMIZE	 += -fdevirtualize-speculatively -ftracer -ftree-loop-distribution -ftree-loop-ivcanon -ftree-loop-im
  ifdef CCACHE
    COMMONFLAGS	 += -fdiagnostics-color=auto
  endif
endif
ifeq ($(uname_S),x86_64)
  COMMONFLAGS	 += -mcx16			# for CMPXCHG16B, in AMD64 since 2005
  OPTIMIZE	 += -minline-all-stringops
  OPTIMIZE	 += -mmmx -msse -msse2		# Intel since 2001, AMD since 2003
  OPTIMIZE	 += -msse3			# Intel since 2004, AMD since 2007
  OPTIMIZE	 += -mssse3			# Intel since 2006, AMD since 2011
  #OPTIMIZE	 += -msse4a			# AMD since 2007
  #OPTIMIZE	 += -msse4.1 -msse4.2		# Intel since 2008, AMD since 2011
  #OPTIMIZE	 += -mavx			# Intel since 2011, AMD since 2011
  #OPTIMIZE	 += -mavx2			# Intel since 2013, AMD since 2015
endif
pkgcflags	::= $(strip $(COMMONFLAGS) $(CONLYFLAGS) $(OPTIMIZE)) $(CFLAGS)
pkgcxxflags	::= $(strip $(COMMONFLAGS) $(CXXONLYFLAGS) $(OPTIMIZE)) $(CXXFLAGS)
LDFLAGS		::= $(strip $(LDOPTIMIZE)) -Wl,-export-dynamic -Wl,--as-needed -Wl,--no-undefined -Wl,-Bsymbolic-functions $(LDFLAGS)

# == implicit rules ==
compiledefs     = $(DEFS) $(EXTRA_DEFS) $($<.DEFS) $($@.DEFS) $(INCLUDES) $(EXTRA_INCLUDES) $($<.INCLUDES) $($@.INCLUDES)
compilecflags   = $(pkgcflags) $(EXTRA_FLAGS) $($<.FLAGS) $($@.FLAGS) -MQ '$@' -MMD -MF '$@'.d
compilecxxflags = $(pkgcxxflags) $(EXTRA_FLAGS) $($<.FLAGS) $($@.FLAGS) -MQ '$@' -MMD -MF '$@'.d
$>/%.o: %.c
	$(QECHO) CC $@
	$(Q) $(CCACHE) $(CC) $(CSTD) -fPIC $(compiledefs) $(compilecflags) -o $@ -c $<
$>/%.o: %.cc
	$(QECHO) CXX $@
	$(Q) $(CCACHE) $(CXX) $(CXXSTD) -fPIC $(compiledefs) $(compilecxxflags) -o $@ -c $<

# == SUBST_O ==
# $(call SUBST_O, sourcefiles...) - generate object file names from sources
SUBST_O = $(sort $(foreach X, .c .C .cc .CC .y .l, $(subst $X,.o,$(filter %$X,$1))))

# == LINKER ==
# $(call LINKER, EXECUTABLE, OBJECTS, DEPS, LIBS, RELPATHS)
define LINKER
$1: $2	$3
	$$(QECHO) LD $$@
	$$Q $$(CXX) $$(CXXSTD) -fPIC -o $$@ $$(LDFLAGS) $$($$@.LDFLAGS) $2 $4 $(foreach P, $5, -Wl$(,)-rpath='$$$$ORIGIN/$P' -Wl$(,)-L'$$(@D)/$P') -Wl$,--print-map >$$@.map
endef

# == BUILD_SHARED_LIB ==
# $(call BUILD_SHARED_LIB_SOLINKS, libfoo.so.1.2.3.4): libfoo.so.1 libfoo.so
BUILD_SHARED_LIB_SOLINKS  = $(shell X="$1" ; \
			      [[ $$X =~ (.*\.so\.[0-9]+)(\.[0-9]+)+$$ ]] && echo "$${BASH_REMATCH[1]}" ; \
			      [[ $$X =~ (.*\.so)(\.[0-9]+)+$$ ]] && echo "$${BASH_REMATCH[1]}" )
# $(call BUILD_SHARED_LIB_SONAME, libfoo.so.1.2.3): libfoo.so.1
BUILD_SHARED_LIB_SONAME = $(shell X="$(notdir $1)" ; while [[ $${X} =~ \.[0-9]+(\.[0-9]+)$$ ]] ; do X="$${X%$${BASH_REMATCH[1]}}"; done ; echo "$$X")
# BUILD_SHARED_LIB implementation
define BUILD_SHARED_LIB.impl
ALL_TARGETS += $(call BUILD_SHARED_LIB_SOLINKS, $1) $1
$1: SONAME_LDFLAGS ::= -shared -Wl,-soname,$(call BUILD_SHARED_LIB_SONAME, $1)
$(call LINKER, $1, $2, $3, $4 $$(SONAME_LDFLAGS), $5)
$(call BUILD_SHARED_LIB_SOLINKS, $1): $1
	$$(QECHO) LN $$@
	$$Q rm -f $$@ && ln -s $$(notdir $$<) $$@
endef
# $(call BUILD_SHARED_LIB, sharedlibrary, objects, deps, libs, rpath)
BUILD_SHARED_LIB = $(eval $(call BUILD_SHARED_LIB.impl, $1, $2, $3, $4, $5))

# == BUILD_STATIC_LIB ==
# BUILD_STATIC_LIB implementation
define BUILD_STATIC_LIB.impl
ALL_TARGETS += $1
$1: $2	$3
	$$(QECHO) AR $$@
	$$Q $$(AR) rcs $$@ $2
endef
# $(call BUILD_STATIC_LIB, staticlibrary, objects, deps)
BUILD_STATIC_LIB = $(eval $(call BUILD_STATIC_LIB.impl, $1, $2, $3))

# == BUILD_PROGRAM ==
# $(call BUILD_PROGRAM, executable, objects, deps, libs, rpath)
BUILD_PROGRAM = $(eval $(call LINKER, $1, $2, $3, $4, $5))	$(eval ALL_TARGETS += $1)

# == BUILD_TEST ==
# $(call BUILD_TEST, executable, objects, deps, libs, rpath)
BUILD_TEST = $(eval $(call LINKER, $1, $2, $3, $4, $5))	$(eval ALL_TESTS += $1)
