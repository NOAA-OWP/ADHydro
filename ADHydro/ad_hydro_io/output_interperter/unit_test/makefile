# Use these values for Yellowstone.

#METISDIR   := /glade/p/work/nfrazie1/CI-WATER-tools
#LIBDIRNAME := lib
#EXTRAFLAGS := -DINTEL_COMPILER -ld++ mpif90 -ld++-option -nofor_main

# Use these values for Mt Moran.

#CHARMDIR         := /project/CI-WATER/tools/CI-WATER-tools/charm
#HDF5DIR          := /apps/HDF5-mpi/intel/15.0.0/1.8.13
#NETCDFDIR        := /apps/NetCDF-mpi/intel/15.0.0/4.3.2
#METISDIR         := /project/CI-WATER/tools/CI-WATER-tools
#LIBDIRNAME       := lib
#EXTRAFLAGS       := -DINTEL_COMPILER
#EXTRANETCDFFLAGS :=

# Use these values for EN3038 lab.

HDF5DIR          := /opt
NETCDFDIR        := /opt
METISDIR         := /opt
LIBDIRNAME       := lib64
EXTRAFLAGS       := -fdiagnostics-show-option
EXTRANETCDFFLAGS := -lcurl

######
#   Set CHARMDIR to be the install directory of charm
######

######
#EN LAB
######
CHARMDIR    := /opt/build/charm

######
#MOUNT MORAN
######
#CHARMDIR   := /project/CI-WATER/tools/CI-WATER-tools/charm

######
#OTHER
######
#CHARMDIR   := ~/Desktop/charm/
CXXFLAGS := -I../
NETCDFCPPFLAGS := -I$(NETCDFDIR)/include
NETCDFLDFLAGS  := -L$(HDF5DIR)/$(LIBDIRNAME) -L$(NETCDFDIR)/$(LIBDIRNAME) -lnetcdff -Wl,--start-group -lnetcdf -lhdf5_hl -lhdf5 -lz -Wl,--end-group $(EXTRANETCDFFLAGS)
LDFLAGS        := $(NETCDFLDFLAGS) $(EXTRAFLAGS)  -g -Wall -language charm++ -O3 -lproj

CHARMC      := $(CHARMDIR)/bin/charmc $(OPTS)
CHARM_INCL  := $(CHARMDIR)/include

.PHONY: all clean 

all: charm_test
export 
charm_test: main.h main.C main.ci region.h region.cpp region.ci ../OutputManager.h ../OutputManager.cpp ../interperter.h ../interperter.cpp
	$(CHARMC) main.ci region.ci
	make -C ../
	$(CHARMC) $(CXXFLAGS) -c -o element.o element.cpp
	$(CHARMC) $(CXXFLAGS) -c -o region.o region.cpp
	$(CHARMC) $(CXXFLAGS) -c -o main.o main.C
	$(CHARMC) $(LDFLAGS) main.o region.o element.o ../interperter.o ../OutputManager.o ../lexer.o ../parser.o ../ast.o -o charm_test $(NETCDFLDFLAGS)

clean:
	rm -f *.o a.out *.def.h *.decl.h charmrun charm_test ../*.o
