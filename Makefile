#
# Copyright (C) 2015 CompuLab ltd.
# Author: Andrey Gelman <andrey.gelman@compulab.co.il>
# License: GNU GPLv2 or later, at your option
#

SOURCES = main.c panel.c sensors.c queue.c thread-pool.c domain-logic.c \
	i2c-tools.c stats.c cpu-freq.c vga-tools.c nvml-tools.c \
	dlist.c watchdog.c options.c hdd-info.c

SUBDIRS = gpu-temp

OBJDIR  = obj
BINDIR  = bin
OUTFILE = $(BINDIR)/airtop-fpsvc

NVMLDIR = nvml

OBJS    = $(SOURCES:%.c=$(OBJDIR)/%.o)
CFLAGS  = -Wall -I$(NVMLDIR)
LFLAGS  = -Wall -rdynamic
LLIBS   = -lsensors -lpthread -lm -ldl -latasmart
COMPILE_CMD = gcc $(CFLAGS)
STRIP_CMD   = strip
LINK_CMD    = gcc $(LFLAGS)
MKDIR_CMD   = mkdir -p
RM_CMD      = rm -rf

ifdef debug
	CFLAGS += -g
	STRIP_CMD = @\#
else
	CFLAGS += -O2
endif

all: fpsvc subdirs

fpsvc: $(SOURCES) $(OBJS) $(BINDIR)
	$(LINK_CMD) $(OBJS) $(LLIBS) -o $(OUTFILE)
	$(STRIP_CMD) $(OUTFILE)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(COMPILE_CMD) -c $+ -o $@

$(OBJDIR):
	$(MKDIR_CMD) $(OBJDIR)

$(BINDIR):
	$(MKDIR_CMD) $(BINDIR)

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(RM_CMD) $(OBJDIR)/*
	$(RM_CMD) $(BINDIR)/*

