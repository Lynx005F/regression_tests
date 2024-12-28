PULP_APP = test
PULP_APP_SRCS = redmule.c
PULP_CFLAGS = -O3

PULP_CFLAGS += -DUSE_REDUNDANCY

ifeq ($(use_dma),1)
 PULP_CFLAGS += -DUSE_DMA
endif

PULP_CFLAGS += -DCHECK
PULP_CFLAGS += -DSTATS

include $(PULP_SDK_HOME)/install/rules/pulp_rt.mk
