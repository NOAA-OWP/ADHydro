# Use these values for Mt Moran.

#CHARMDIR   := /project/CI-WATER/tools/build/charm
#HDF5DIR    := /apps/HDF5-mpi/intel/15.0.0/1.8.13
#NETCDFDIR  := /apps/NetCDF-mpi/intel/15.0.0/4.3.2
#METISDIR   := /project/CI-WATER/tools
#LIBDIRNAME := lib
#EXTRAFLAGS := -DINTEL_COMPILER

# Use these values for EN3038 lab.

HDF5DIR    := /opt
NETCDFDIR  := /opt
METISDIR   := /opt
LIBDIRNAME := lib64
EXTRAFLAGS := -fdiagnostics-show-option

# Use these values for all.

CHARMC   := $(CHARMDIR)/bin/charmc
CPPFLAGS := $(EXTRAFLAGS) -g -Wall
LDFLAGS  := $(EXTRAFLAGS) -g -Wall -language charm++

EXES := adhydro

ADHYDRO_OBJS := element.o \
                neighbor_proxy.o

all: $(EXES)

.PHONY: all

adhydro: $(ADHYDRO_OBJS)
	$(CHARMC) $(LDFLAGS) $^ -o $@

element.o: element.cpp      \
           element.h        \
           neighbor_proxy.h \
           all_charm.h      \
           all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@
           
neighbor_proxy.o: neighbor_proxy.cpp \
                  neighbor_proxy.h   \
                  all_charm.h        \
                  all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

clean:
	rm -f charmrun $(EXES) *.o *.decl.h *.def.h

.PHONY: clean
