PULP_APP = test

PULP_APP_FC_SRCS = main.c
PULP_APP_SRCS = support_func.c matmul.c

PULP_CFLAGS += -O3 -g3 
PULP_CFLAGS += -mno-memcpy

PULP_CFLAGS += -DNUM_CORES=4 #-flto -DFABRIC=1

ifdef FABRIC
PULP_CFLAGS += -DFABRIC
endif

PULP_CFLAGS += -DUSE_INTRINSICS

ifdef thr
PULP_CFLAGS += -DTHR=${thr}
endif

PULP_CFLAGS += -fno-tree-vectorize

PULP_CFLAGS += -DMAFP32
PULP_CFLAGS += -DMBFP32
PULP_CFLAGS += -DOUTFP32

PULP_CFLAGS += -DCHECK

ifdef PRINT_RESULTS
PULP_CFLAGS += -DPRINT_RESULTS
endif

ifdef verbose
PULP_CFLAGS += -DVERBOSE
endif

PULP_CFLAGS += -DSTATS

include $(PULP_SDK_HOME)/install/rules/pulp.mk
