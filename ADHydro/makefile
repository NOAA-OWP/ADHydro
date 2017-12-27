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
EXTRAFLAGS       := -fdiagnostics-show-option -fpermissive
EXTRANETCDFFLAGS := -lcurl -lz

# Use these values for Mt Moran and EN3038 lab, but not for Yellowstone.

NETCDFCPPFLAGS := -I$(NETCDFDIR)/include
NETCDFLDFLAGS  := -L$(HDF5DIR)/$(LIBDIRNAME) -L$(NETCDFDIR)/$(LIBDIRNAME) -lnetcdff -Wl,--start-group -lnetcdf -lhdf5_hl -lhdf5 -lz -Wl,--end-group $(EXTRANETCDFFLAGS)

# Use these values for all.

CHARMC         := $(CHARMDIR)/bin/charmc
METISLDFLAGS   := -L$(METISDIR)/$(LIBDIRNAME) -lmetis -balancer MetisLB
CPPFLAGS       := $(NETCDFCPPFLAGS) $(EXTRAFLAGS) -Iad_hydro_io -I../management/generated_code/charm_diversions -I../management/generated_code/charm_factory -I../management/generated_code/charm_reservoirs -I../inih/cpp -g -Wall -O3 -fpermissive
LDFLAGS        := $(NETCDFLDFLAGS) $(METISLDFLAGS) $(EXTRAFLAGS) -lgfortran -g -Wall -language charm++ -O3

export

VPATH  := ad_hydro_io ../management/generated_code/charm_diversions ../management/generated_code/charm_factory ../management/generated_code/charm_reservoirs ../management/build ../HRLDAS-v3.6/Noah ../HRLDAS-v3.6/IO_code ../HRLDAS-v3.6/Utility_routines ../inih ../inih/cpp

EXES := adhydro

ADHYDRO_OBJS := adhydro.o             \
                file_manager.o        \
                region.o              \
                mesh_element.o        \
                channel_element.o     \
                element.o             \
                neighbor_proxy.o      \
                surfacewater.o        \
                groundwater.o         \
                evapo_transpiration.o \
                garto.o

MANAGEMENT_OBJS := ReservoirFactory.o \
                   Reservoir.o        \
                   DiversionFactory.o \
                   Diversion.o        \
                   ParcelFactory.o    \
                   Parcel.o           \
                   common_utilities.o

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


IO_OBJS := adhydro_io.o

all: $(EXES)

.PHONY: all io_modules adhydro #make adhydro a phony target so it gets relinked whenever we change subdirs

adhydro: $(ADHYDRO_OBJS) $(MANAGEMENT_OBJS) $(NOAHMP_OBJS) $(INIH_OBJS) $(IO_OBJS)
	$(CHARMC) $(LDFLAGS) $^ -o $@

ad_hydro_io/adhydro_io.o: io_modules

io_modules:
	$(MAKE) -C ad_hydro_io

adhydro.o: adhydro.cpp           \
           adhydro.h             \
           adhydro.decl.h        \
           adhydro.def.h         \
           file_manager.h        \
           file_manager.decl.h   \
           region.h              \
           region.decl.h         \
           mesh_element.h        \
           channel_element.h     \
           element.h             \
           neighbor_proxy.h      \
           evapo_transpiration.h \
           all_charm.h           \
           all.h                 \
           ReservoirFactory.h    \
           Reservoir.h           \
           DiversionFactory.h    \
           Diversion.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

adhydro.decl.h \
adhydro.def.h: adhydro.ci      \
               file_manager.ci \
               region.ci
	$(CHARMC) $<
	
file_manager.o: file_manager.cpp      \
                file_manager.h        \
                file_manager.decl.h   \
                file_manager.def.h    \
                adhydro.h             \
                adhydro.decl.h        \
                region.h              \
                region.decl.h         \
                mesh_element.h        \
                channel_element.h     \
                element.h             \
                neighbor_proxy.h      \
                evapo_transpiration.h \
                garto.h               \
                all_charm.h           \
                all.h                 \
                ReservoirFactory.h    \
                Reservoir.h           \
                DiversionFactory.h    \
                Diversion.h           
	$(CHARMC) $(CPPFLAGS) $< -o $@

file_manager.decl.h \
file_manager.def.h: file_manager.ci
	$(CHARMC) $<

region.o: region.cpp            \
          region.h              \
          region.decl.h         \
          region.def.h          \
          adhydro.h             \
          adhydro.decl.h        \
          file_manager.h        \
          file_manager.decl.h   \
          mesh_element.h        \
          channel_element.h     \
          element.h             \
          neighbor_proxy.h      \
          evapo_transpiration.h \
          garto.h               \
          all_charm.h           \
          all.h                 \
          ReservoirFactory.h    \
          Reservoir.h           \
          DiversionFactory.h    \
          Diversion.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

region.decl.h \
region.def.h: region.ci
	$(CHARMC) $<

mesh_element.o : mesh_element.cpp      \
                 mesh_element.h        \
                 adhydro.h             \
                 adhydro.decl.h        \
                 file_manager.h        \
                 file_manager.decl.h   \
                 region.h              \
                 region.decl.h         \
                 channel_element.h     \
                 element.h             \
                 neighbor_proxy.h      \
                 surfacewater.h        \
                 groundwater.h         \
                 evapo_transpiration.h \
                 garto.h               \
                 all_charm.h           \
                 all.h                 \
                 ReservoirFactory.h    \
                 Reservoir.h           \
                 DiversionFactory.h    \
                 Diversion.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

channel_element.o : channel_element.cpp   \
                    channel_element.h     \
                    adhydro.h             \
                    adhydro.decl.h        \
                    file_manager.h        \
                    file_manager.decl.h   \
                    region.h              \
                    region.decl.h         \
                    mesh_element.h        \
                    element.h             \
                    neighbor_proxy.h      \
                    surfacewater.h        \
                    groundwater.h         \
                    evapo_transpiration.h \
                    all_charm.h           \
                    all.h                 \
                    ReservoirFactory.h    \
                    Reservoir.h           \
                    DiversionFactory.h    \
                    Diversion.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

element.o: element.cpp      \
           element.h        \
           all_charm.h      \
           all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

neighbor_proxy.o: neighbor_proxy.cpp \
                  neighbor_proxy.h   \
                  all_charm.h        \
                  all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

surfacewater.o: surfacewater.cpp      \
                surfacewater.h        \
                adhydro.h             \
                adhydro.decl.h        \
                file_manager.h        \
                file_manager.decl.h   \
                region.h              \
                region.decl.h         \
                mesh_element.h        \
                channel_element.h     \
                element.h             \
                neighbor_proxy.h      \
                evapo_transpiration.h \
                all_charm.h           \
                all.h                 \
                ReservoirFactory.h    \
                Reservoir.h           \
                DiversionFactory.h    \
                Diversion.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

groundwater.o: groundwater.cpp       \
               groundwater.h         \
               adhydro.h             \
               adhydro.decl.h        \
               file_manager.h        \
               file_manager.decl.h   \
               region.h              \
               region.decl.h         \
               mesh_element.h        \
               channel_element.h     \
               element.h             \
               neighbor_proxy.h      \
               evapo_transpiration.h \
               all_charm.h           \
               all.h                 \
               ReservoirFactory.h    \
               Reservoir.h           \
               DiversionFactory.h    \
               Diversion.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

evapo_transpiration.o: evapo_transpiration.cpp \
                       evapo_transpiration.h   \
                       adhydro.h               \
                       adhydro.decl.h          \
                       file_manager.h          \
                       file_manager.decl.h     \
                       region.h                \
                       region.decl.h           \
                       mesh_element.h          \
                       channel_element.h       \
                       element.h               \
                       neighbor_proxy.h        \
                       all_charm.h             \
                       all.h                   \
                       ReservoirFactory.h    \
                       Reservoir.h             \
                       DiversionFactory.h    \
                       Diversion.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

garto.o: garto.cpp   \
         garto.h     \
         all_charm.h \
         all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

clean: clean_modules
	rm -f charmrun $(EXES) *.o *.decl.h *.def.h

clean_modules:
	$(MAKE) -C ad_hydro_io clean

.PHONY: clean clean_modules
