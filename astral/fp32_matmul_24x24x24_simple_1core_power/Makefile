PULP_APP = test

PULP_APP_FC_SRCS = main.c
PULP_APP_SRCS = support_func.c matmul.c

PULP_CFLAGS += -O3 -g3 
PULP_CFLAGS += -mno-memcpy

PULP_CFLAGS += -DNUM_CORES=1 #-flto -DFABRIC=1

ifdef FABRIC
PULP_CFLAGS += -DFABRIC
endif

ifdef thr
PULP_CFLAGS += -DTHR=${thr}
endif

PULP_CFLAGS += -fno-tree-vectorize

PULP_CFLAGS += -DMAFP32
PULP_CFLAGS += -DMBFP32
PULP_CFLAGS += -DOUTFP32

include $(PULP_SDK_HOME)/install/rules/pulp.mk
