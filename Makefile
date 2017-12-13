# Adapted from:
# Makefile for GENIE test programs
# Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
#

SHELL = /bin/sh
NAME = all
MAKEFILE = Makefile

# Include machine specific flags and locations (inc. files & libs)
#
include $(GENIE)/src/make/Make.include

GENIE_LIBS  = $(shell $(GENIE)/src/scripts/setup/genie-config --libs)
LIBRARIES  := $(GENIE_LIBS) $(LIBRARIES) $(CERN_LIBRARIES)

INCLUDES := $(INCLUDES) $(shell `pwd`)

TGT =	overlay_genie

all: $(TGT)


###

overlay_genie: FORCE
	$(CXX) $(CXXFLAGS) -c overlay_genie.cxx $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c event_source.cxx $(INCLUDES)
	$(LD) $(LDFLAGS) overlay_genie.o event_source.o $(LIBRARIES) -o ./overlay_genie


#################### CLEANING

purge: FORCE
	$(RM) *.o *~ core 

clean: FORCE
	$(RM) *.o *~ core 
	$(RM) ./gtestEventLoop

FORCE:

# DO NOT DELETE
