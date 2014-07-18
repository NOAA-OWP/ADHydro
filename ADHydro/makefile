#CHARMDIR       := /project/CI-WATER/rsteinke/charm
CHARMC         := $(CHARMDIR)/bin/charmc
#HDF5DIR        := /apps/HDF5-mpi/intel/14.0.0/1.8.12
HDF5DIR        := /opt
#NETCDFDIR      := /apps/netcdf-mpi/intel/14.0.0/4.3.2
NETCDFDIR      := /opt
NETCDFCPPFLAGS := -I$(NETCDFDIR)/include
#NETCDFLDFLAGS  := -L$(HDF5DIR)/lib -L$(NETCDFDIR)/lib -Wl,--start-group -lnetcdf -lhdf5_hl -lhdf5 -Wl,--end-group -lcurl
NETCDFLDFLAGS  := -L$(HDF5DIR)/lib64 -L$(NETCDFDIR)/lib64 -Wl,--start-group -lnetcdf -lhdf5_hl -lhdf5 -Wl,--end-group -lcurl
#METISDIR       := /project/CI-WATER/rsteinke/metis-5.1.0
#METISLDFLAGS   := -L$(METISDIR)/build/Linux-x86_64/libmetis -lmetis -balancer MetisLB
METISLDFLAGS   := -lmetis -balancer MetisLB
CPPFLAGS       := $(NETCDFCPPFLAGS) -g -Wall -fdiagnostics-show-option
LDFLAGS        := $(NETCDFLDFLAGS) $(METISLDFLAGS) -g -Wall -language charm++

EXES := adhydro                     \
        adhydro_input_preprocessing \
        adhydro_create_xmf_file

ADHYDRO_OBJS := adhydro.o         \
                mesh_element.o    \
                channel_element.o \
                file_manager.o    \
                surfacewater.o    \
                groundwater.o

ADHYDRO_INPUT_PREPROCESSING_OBJS := adhydro_input_preprocessing.o \
                                    file_manager.o

ADHYDRO_CREATE_XMF_FILE_OBJS := adhydro_create_xmf_file.o

all: $(EXES)

.PHONY: all

adhydro: $(ADHYDRO_OBJS)
	$(CHARMC) $^ -o $@ $(LDFLAGS)

adhydro.o: adhydro.cpp         \
           adhydro.h           \
           adhydro.decl.h      \
           adhydro.def.h       \
           mesh_element.h      \
           mesh_element.decl.h \
           file_manager.h      \
           file_manager.decl.h \
           all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

adhydro.decl.h \
adhydro.def.h: adhydro.ci      \
               mesh_element.ci \
               file_manager.ci
	$(CHARMC) $<

test_adhydro: test_adhydro.o    \
              mesh_element.o    \
              channel_element.o \
              file_manager.o    \
              surfacewater.o    \
              groundwater.o
	$(CHARMC) $^ -o $@ $(LDFLAGS)

test_adhydro.o: test_adhydro.cpp       \
                test_adhydro.h         \
                test_adhydro.decl.h    \
                test_adhydro.def.h     \
                mesh_element.h         \
                mesh_element.decl.h    \
                channel_element.h      \
                channel_element.decl.h \
                file_manager.h         \
                file_manager.decl.h    \
                all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

test_adhydro.decl.h \
test_adhydro.def.h: test_adhydro.ci    \
                    mesh_element.ci    \
                    channel_element.ci \
                    file_manager.ci
	$(CHARMC) $<

adhydro_input_preprocessing: $(ADHYDRO_INPUT_PREPROCESSING_OBJS)
	$(CHARMC) $^ -o $@ $(LDFLAGS)

adhydro_input_preprocessing.o: adhydro_input_preprocessing.cpp         \
                               adhydro_input_preprocessing.h           \
                               adhydro_input_preprocessing.decl.h      \
                               adhydro_input_preprocessing.def.h       \
                               file_manager.h                          \
                               file_manager.decl.h                     \
                               all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

adhydro_input_preprocessing.decl.h \
adhydro_input_preprocessing.def.h: adhydro_input_preprocessing.ci \
                                   file_manager.ci
	$(CHARMC) $<

adhydro_create_xmf_file: $(ADHYDRO_CREATE_XMF_FILE_OBJS)
	$(CC) $^ -o $@ -lnetcdf -g -Wall

adhydro_create_xmf_file.o: adhydro_create_xmf_file.c
	$(CC) $< -c -o $@ -g -Wall -fdiagnostics-show-option

mesh_element.o: mesh_element.cpp       \
                mesh_element.h         \
                mesh_element.decl.h    \
                mesh_element.def.h     \
                channel_element.decl.h \
                file_manager.h         \
                file_manager.decl.h    \
                surfacewater.h         \
                groundwater.h          \
                all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

mesh_element.decl.h \
mesh_element.def.h: mesh_element.ci
	$(CHARMC) $<

channel_element.o: channel_element.cpp    \
                   channel_element.h      \
                   channel_element.decl.h \
                   channel_element.def.h  \
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

file_manager.o: file_manager.cpp    \
                file_manager.h      \
                file_manager.decl.h \
                file_manager.def.h  \
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

clean:
	rm -f charmrun $(EXES) $(ADHYDRO_OBJS) $(ADHYDRO_INPUT_PREPROCESSING_OBJS) $(ADHYDRO_CREATE_XMF_FILE_OBJS) *.decl.h *.def.h

.PHONY: clean
