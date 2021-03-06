
PBENCH_PATH=../../pbench

include $(PBENCH_PATH)/Makefile_common

PACKAGE_PATH=../../
CMDLINE_HOME=$(PACKAGE_PATH)/cmdline/include
CHUNKEDSEQ_HOME=$(PACKAGE_PATH)/chunkedseq/include
SPTL_HOME=$(PACKAGE_PATH)/sptl/include
PBBS_SPTL_HOME=$(PACKAGE_PATH)/pbbs-sptl/include
PBBS_HOME=$(PACKAGE_PATH)/pbbs-include/

INCLUDE_FILES=$(wildcard *.hpp) $(wildcard $(CHUNKEDSEQ_HOME)/*.hpp) $(wildcard $(CMDLINE_HOME)/*.hpp) $(wildcard $(SPTL_HOME)/*.hpp) $(wildcard $(PBBS_SPTL_HOME)/*.hpp) $(wildcard $(PBBS_HOME)/*.hpp)

INCLUDE_DIRECTIVES=-I $(CHUNKEDSEQ_HOME) -I $(CMDLINE_HOME) -I $(SPTL_HOME) -I $(PBBS_SPTL_HOME) -I $(PBBS_HOME)

####################################################################
# Makefile options

# Create a file called "settings.sh" in this folder if you want to
# configure particular options. See section below for options.

-include settings.sh

####################################
# Example settings.sh 
#
# USE_HWLOC=1
# USE_CILK=1
# USE_32_BIT_WORD_SIZE=1
# CUSTOM_MALLOC_PREFIX=-ltcmalloc 
# CILK_EXTRAS_PREFIX=-L ../../../../cilk-plus-rts/lib -I ../../../../cilk-plus-rts/include -ldl -DCILK_RUNTIME_WITH_STATS

HWLOC_FLAGS=`pkg-config --cflags hwloc`
HWLOC_LIBS=`pkg-config --libs hwloc`
HWLOC_PREFIX_ALL=-DSPTL_HAVE_HWLOC $(HWLOC_FLAGS) $(HWLOC_LIBS)

HWLOC_PREFIX=
ifeq ($(strip $(USE_HWLOC)),1)
   HWLOC_PREFIX=$(HWLOC_PREFIX_ALL)
endif

CILK_PREFIX=
ifeq ($(strip $(USE_CILK)),1)
	CILK_PREFIX=-fcilkplus -lcilkrts -DSPTL_USE_CILK_PLUS_RUNTIME -DUSE_CILK_PLUS_RUNTIME
	CILK_PREFIX+=$(CILK_EXTRAS_PREFIX)
endif

FIBRIL_PREFIX=
ifeq ($(strip $(USE_FIBRIL)),1)
	FIBRIL_PREFIX=-lm -pthread -g -fno-omit-frame-pointer -DSPTL_USE_FIBRIL
	FIBRIL_PREFIX+=$(FIBRIL_EXTRAS_PREFIX)
endif

SMALL_WORD_PREFIX=-DSPTL_SIZE_BITS32
LARGE_WORD_PREFIX=-m64 -DSPTL_SIZE_BITS64
WORD_SIZE_PREFIX=$(LARGE_WORD_PREFIX)
ifeq ($(strip $(USE_32_BIT_WORD_SIZE)),1)
	WORD_SIZE_PREFIX=$(SMALL_WORD_PREFIX)
endif

WARNINGS_PREFIX=-Wno-subobject-linkage -Wno-overflow
COMMON_PREFIX=-std=c++1y -DSPTL_TARGET_LINUX $(WARNINGS_PREFIX) $(HWLOC_PREFIX) $(WORD_SIZE_PREFIX) 
COMMON_OPT_PREFIX=$(COMMON_PREFIX) $(CUSTOM_MALLOC_PREFIX)
RUNTIME_PREFIX=$(CILK_PREFIX) $(FIBRIL_PREFIX) -ldl
DEBUG_PREFIX=$(COMMON_PREFIX) $(RUNTIME_PREFIX) -g3 -Og
O2_PREFIX=-O2 -march=native -DNDEBUG -lm
OPT_PREFIX=$(COMMON_OPT_PREFIX) $(RUNTIME_PREFIX) $(O2_PREFIX)
ELISION_PREFIX=$(COMMON_OPT_PREFIX) $(O2_PREFIX) -DSPTL_USE_SEQUENTIAL_ELISION_RUNTIME
BASELINE_PREFIX=$(ELISION_PREFIX) -DSPTL_USE_SEQUENTIAL_BASELINE
LOG_PREFIX=$(SPTL_PREFIX) $(RUNTIME_PREFIX) -DSPTL_ENABLE_LOGGING

%.baseline: %.cpp $(INCLUDE_FILES)
	g++ $(BASELINE_PREFIX) $(INCLUDE_DIRECTIVES) -o $@ $<

%.elision: %.cpp $(INCLUDE_FILES)
	g++ $(ELISION_PREFIX) $(INCLUDE_DIRECTIVES) -o $@ $<

%.opt: %.cpp $(INCLUDE_FILES)
	g++ $(OPT_PREFIX) $(INCLUDE_DIRECTIVES) -o $@ $<

%.log: %.cpp $(INCLUDE_FILES)
	g++ $(LOG_PREFIX) $(INCLUDE_DIRECTIVES) -o $@ $<

%.dbg: %.cpp $(INCLUDE_FILES)
	g++ $(DEBUG_PREFIX) $(INCLUDE_DIRECTIVES) -o $@ $<

pthread-hello.bin: pthread-hello.cpp $(INCLUDE_FILES)
	g++ $(OPT_PREFIX) $(INCLUDE_DIRECTIVES) -pthread -o pthread-hello.bin pthread-hello.cpp

clean: pbench_clean
	rm -f *.baseline *.elision *.opt *.log *.dbg *.bin
