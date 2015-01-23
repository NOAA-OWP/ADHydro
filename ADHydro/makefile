# Use these values for Mt Moran.

#CHARMDIR   := /project/CI-WATER/tools/build/charm-6.6.0
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
CPPFLAGS       := $(NETCDFCPPFLAGS) $(EXTRAFLAGS)  -I/usr/include/libshp -I../inih/cpp -g -Wall
LDFLAGS        := $(NETCDFLDFLAGS) $(METISLDFLAGS) $(EXTRAFLAGS) -lgfortran -g -Wall -language charm++

VPATH  := ../HRLDAS-v3.6/Noah ../HRLDAS-v3.6/IO_code ../HRLDAS-v3.6/Utility_routines ../inih ../inih/cpp

EXES := adhydro                            \
        adhydro_channel_preprocessing      \
        adhydro_create_xmf_file            \
        adhydro_offline_mass_balance_check \
        adhydro_mesh_check                 \
        test_date_functions

ADHYDRO_OBJS := adhydro.o             \
                mesh_element.o        \
                channel_element.o     \
                file_manager.o        \
                surfacewater.o        \
                groundwater.o         \
                evapo_transpiration.o \
                gar.o                 

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

ADHYDRO_CHANNEL_PREPROCESSING_OBJS := adhydro_channel_preprocessing.o

ADHYDRO_CREATE_XMF_FILE_OBJS := adhydro_create_xmf_file.o

ADHYDRO_OFFLINE_MASS_BALANCE_CHECK_OBJS := adhydro_offline_mass_balance_check.o

ADHYDRO_MESH_CHECK_OBJS := adhydro_mesh_check.o

TEST_DATE_FUNCTIONS_OBJS := test_date_functions.o

all: $(EXES)

.PHONY: all

adhydro: $(ADHYDRO_OBJS) $(NOAHMP_OBJS) $(INIH_OBJS)
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
           evapo_transpiration.h  \
           gar.h                  \
           all.h                  \
           INIReader.h
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
                                 mesh_element.h                    \
                                 mesh_element.decl.h               \
                                 channel_element.h                 \
                                 channel_element.decl.h            \
                                 file_manager.decl.h               \
                                 evapo_transpiration.h             \
                                 gar.h                             \
                                 all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

adhydro_create_xmf_file: $(ADHYDRO_CREATE_XMF_FILE_OBJS)
	$(CC) $^ -o $@ -lnetcdf -g -Wall

adhydro_create_xmf_file.o: adhydro_create_xmf_file.c
	$(CC) $< -c -o $@ -g -Wall -fdiagnostics-show-option

adhydro_offline_mass_balance_check: $(ADHYDRO_OFFLINE_MASS_BALANCE_CHECK_OBJS)
	$(CC) $^ -o $@ -lnetcdf -g -Wall

adhydro_offline_mass_balance_check.o: adhydro_offline_mass_balance_check.c \
                                      adhydro_offline_mass_balance_check.h
	$(CC) $< -c -o $@ -g -Wall -fdiagnostics-show-option

adhydro_mesh_check: $(ADHYDRO_MESH_CHECK_OBJS)
	$(CXX) $^ -o $@ -g -Wall

adhydro_mesh_check.o: adhydro_mesh_check.cpp \
                      all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

test_date_functions: $(TEST_DATE_FUNCTIONS_OBJS)
	$(CXX) $^ -o $@ -g -Wall

test_date_functions.o: test_date_functions.cpp \
                       all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

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
                gar.h                  \
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
                   evapo_transpiration.h  \
                   gar.h                  \
                   all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

channel_element.decl.h \
channel_element.def.h: channel_element.ci
	$(CHARMC) $<

file_manager.o: file_manager.cpp       \
                file_manager.h         \
                file_manager.decl.h    \
                file_manager.def.h     \
                adhydro.h              \
                adhydro.decl.h         \
                mesh_element.h         \
                mesh_element.decl.h    \
                channel_element.h      \
                channel_element.decl.h \
                evapo_transpiration.h  \
                gar.h                  \
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
	
gar.o: gar.cpp \
       gar.h   \
       all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

clean:
	rm -f charmrun $(EXES) $(ADHYDRO_OBJS) $(ADHYDRO_CHANNEL_PREPROCESSING_OBJS) $(ADHYDRO_CREATE_XMF_FILE_OBJS) $(ADHYDRO_OFFLINE_MASS_BALANCE_CHECK_OBJS) $(ADHYDRO_MESH_CHECK_OBJS) $(TEST_DATE_FUNCTIONS_OBJS) *.decl.h *.def.h

.PHONY: clean
