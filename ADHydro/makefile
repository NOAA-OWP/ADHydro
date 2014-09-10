# Use these values for Mt Moran.

#CHARMDIR          := /project/CI-WATER/rsteinke/charm
#HDF5DIR           := /apps/HDF5-mpi/intel/14.0.0/1.8.12
#NETCDFDIR         := /apps/netcdf-mpi/intel/14.0.0/4.3.2
#LIBDIRNAME        := lib
#METISDIR          := /project/CI-WATER/rsteinke/metis-5.1.0
#EXTRAMETISLDFLAGS := -L$(METISDIR)/build/Linux-x86_64/libmetis

# Use these values for EN3038 lab.

HDF5DIR    := /opt
NETCDFDIR  := /opt
LIBDIRNAME := lib64

# Use these values for all.

CHARMC         := $(CHARMDIR)/bin/charmc
NETCDFCPPFLAGS := -I$(NETCDFDIR)/include
NETCDFLDFLAGS  := -L$(HDF5DIR)/$(LIBDIRNAME) -L$(NETCDFDIR)/$(LIBDIRNAME) -lnetcdff -Wl,--start-group -lnetcdf -lhdf5_hl -lhdf5 -Wl,--end-group -lcurl
METISLDFLAGS   := $(EXTRAMETISLDFLAGS) -lmetis -balancer MetisLB
CPPFLAGS       := $(NETCDFCPPFLAGS)  -I/usr/include/libshp -g -Wall -fdiagnostics-show-option
LDFLAGS        := $(NETCDFLDFLAGS) $(METISLDFLAGS) -lgfortran -g -Wall -language charm++

VPATH  := ../HRLDAS-v3.6/Noah ../HRLDAS-v3.6/IO_code ../HRLDAS-v3.6/Utility_routines

EXES := adhydro                       \
        adhydro_channel_preprocessing \
        adhydro_create_xmf_file

ADHYDRO_OBJS := adhydro.o         \
                mesh_element.o    \
                channel_element.o \
                file_manager.o    \
                surfacewater.o    \
                groundwater.o     \
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

ADHYDRO_CHANNEL_PREPROCESSING_OBJS := adhydro_channel_preprocessing.o

ADHYDRO_CREATE_XMF_FILE_OBJS := adhydro_create_xmf_file.o

all: $(EXES)

.PHONY: all

adhydro: $(ADHYDRO_OBJS) $(NOAHMP_OBJS)
	$(CHARMC) $^ -o $@ $(LDFLAGS)

adhydro.o: adhydro.cpp            \
           adhydro.h              \
           adhydro.decl.h         \
           adhydro.def.h          \
           mesh_element.h         \
           mesh_element.decl.h    \
           channel_element.h      \
           channel_element.decl.h \
           file_manager.h         \
           file_manager.decl.h    \
           all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

adhydro.decl.h                    \
adhydro.def.h: adhydro.ci         \
               mesh_element.ci    \
               channel_element.ci \
               file_manager.ci
	$(CHARMC) $<

adhydro_channel_preprocessing: $(ADHYDRO_CHANNEL_PREPROCESSING_OBJS)
	$(CXX) $^ -o $@ -lshp -g -Wall

adhydro_channel_preprocessing.o: adhydro_channel_preprocessing.cpp \
                                 all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

adhydro_create_xmf_file: $(ADHYDRO_CREATE_XMF_FILE_OBJS)
	$(CC) $^ -o $@ -lnetcdf -g -Wall

adhydro_create_xmf_file.o: adhydro_create_xmf_file.c
	$(CC) $< -c -o $@ -g -Wall -fdiagnostics-show-option

mesh_element.o: mesh_element.cpp       \
                mesh_element.h         \
                mesh_element.decl.h    \
                mesh_element.def.h     \
                channel_element.h      \
                channel_element.decl.h \
                file_manager.h         \
                file_manager.decl.h    \
                surfacewater.h         \
                groundwater.h          \
                evapo_transpiration.h  \
                all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

mesh_element.decl.h \
mesh_element.def.h: mesh_element.ci
	$(CHARMC) $<

channel_element.o: channel_element.cpp    \
                   channel_element.h      \
                   channel_element.decl.h \
                   channel_element.def.h  \
                   mesh_element.h         \
                   mesh_element.decl.h    \
                   file_manager.h         \
                   file_manager.decl.h    \
                   surfacewater.h         \
                   groundwater.h          \
                   all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

channel_element.decl.h \
channel_element.def.h: channel_element.ci
	$(CHARMC) $<

file_manager.o: file_manager.cpp       \
                file_manager.h         \
                file_manager.decl.h    \
                file_manager.def.h     \
                mesh_element.h         \
                mesh_element.decl.h    \
                channel_element.h      \
                channel_element.decl.h \
                evapo_transpiration.h  \
                all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

file_manager.decl.h \
file_manager.def.h: file_manager.ci
	$(CHARMC) $<

surfacewater.o: surfacewater.cpp \
                surfacewater.h   \
                all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

groundwater.o: groundwater.cpp \
               groundwater.h   \
               all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

evapo_transpiration.o: evapo_transpiration.cpp \
                       evapo_transpiration.h   \
                       all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

clean:
	rm -f charmrun $(EXES) $(ADHYDRO_OBJS) $(ADHYDRO_CHANNEL_PREPROCESSING_OBJS) $(ADHYDRO_CREATE_XMF_FILE_OBJS) *.decl.h *.def.h

.PHONY: clean
