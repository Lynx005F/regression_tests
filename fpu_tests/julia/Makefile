PULP_APP = test
PULP_APP_SRCS = main.c
PULP_CFLAGS = -O3

ifeq ($(use_redundancy), 1)
PULP_CFLAGS += -DREDUNDANCY
endif

ifdef cores
PULP_CFLAGS += -DNUM_CORES=${cores}
else
PULP_CFLAGS += -DNUM_CORES=1
endif

ifdef check
PULP_CFLAGS += -DCHECK
endif

ifdef stats
PULP_CFLAGS += -DSTATS
endif

include $(PULP_SDK_HOME)/install/rules/pulp_rt.mk
