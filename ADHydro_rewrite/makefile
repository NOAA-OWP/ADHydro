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

CHARMC         := $(CHARMDIR)/bin/charmc
NETCDFCPPFLAGS := -I$(NETCDFDIR)/include
NETCDFLDFLAGS  := -L$(HDF5DIR)/$(LIBDIRNAME) -L$(NETCDFDIR)/$(LIBDIRNAME) -lnetcdff -Wl,--start-group -lnetcdf -lhdf5_hl -lhdf5 -Wl,--end-group -lcurl
METISLDFLAGS   := -L$(METISDIR)/$(LIBDIRNAME) -lmetis -balancer MetisLB
CPPFLAGS       := $(NETCDFCPPFLAGS) $(EXTRAFLAGS) -I../inih/cpp -g -Wall
LDFLAGS        := $(NETCDFLDFLAGS) $(METISLDFLAGS) $(EXTRAFLAGS) -lgfortran -g -Wall -language charm++

VPATH  := ../HRLDAS-v3.6/Noah ../HRLDAS-v3.6/IO_code ../HRLDAS-v3.6/Utility_routines ../inih ../inih/cpp

EXES := adhydro

ADHYDRO_OBJS := adhydro.o        \
                mesh_element.o   \
                element.o        \
                neighbor_proxy.o \
                evapo_transpiration.o

NOAHMP_OBJS := module_sf_noahmpdrv.o          \
               module_sf_noahmplsm.o          \
               module_sf_noahmp_glacier.o     \
               module_sf_noahmp_groundwater.o \
               module_sf_myjsfc.o             \
               module_sf_sfclay.o             \
               module_NoahMP_hrldas_driver.o  \
               module_hrldas_netcdf_io.o      \
               kwm_string_utilities.o         \
               module_date_utilities.o        \
               module_wrf_utilities.o

INIH_OBJS := INIReader.o \
             ini.o

all: $(EXES)

.PHONY: all

adhydro: $(ADHYDRO_OBJS) $(NOAHMP_OBJS) $(INIH_OBJS)
	$(CHARMC) $(LDFLAGS) $^ -o $@

adhydro.o: adhydro.cpp    \
           adhydro.h      \
           adhydro.decl.h \
           adhydro.def.h  \
           all_charm.h    \
           all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

adhydro.decl.h \
adhydro.def.h: adhydro.ci
	$(CHARMC) $<

mesh_element.o : mesh_element.cpp      \
                 mesh_element.h        \
                 adhydro.h             \
                 adhydro.decl.h        \
                 element.h             \
                 neighbor_proxy.h      \
                 evapo_transpiration.h \
                 all_charm.h           \
                 all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

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

evapo_transpiration.o: evapo_transpiration.cpp \
                       evapo_transpiration.h   \
                       adhydro.h               \
                       adhydro.decl.h          \
                       all_charm.h             \
                       all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

clean:
	rm -f charmrun $(EXES) *.o *.decl.h *.def.h

.PHONY: clean
