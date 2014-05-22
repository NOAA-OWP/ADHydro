#CHARMDIR       := /project/CI-WATER/rsteinke/charm
CHARMDIR       := /user2/rsteinke/Desktop/charm
CHARMC         := $(CHARMDIR)/bin/charmc
HDF5DIR        := /apps/HDF5-mpi/intel/14.0.0/1.8.12
NETCDFDIR      := /apps/netcdf-mpi/intel/14.0.0/4.3.2
#NETCDFCPPFLAGS := -I$(NETCDFDIR)/include
NETCDFCPPFLAGS :=
#NETCDFLDFLAGS  := -L$(HDF5DIR)/lib -L$(NETCDFDIR)/lib -Wl,--start-group -lnetcdf -lhdf5_hl -lhdf5 -Wl,--end-group -lcurl
NETCDFLDFLAGS  := -lnetcdf
#METISDIR       := /project/CI-WATER/rsteinke/metis-5.1.0
METISDIR       := /user2/rsteinke/Desktop/metis-5.1.0
METISLDFLAGS   := -L$(METISDIR)/build/Linux-x86_64/libmetis -lmetis -balancer MetisLB
CPPFLAGS       := $(NETCDFCPPFLAGS) -g -Wall
LDFLAGS        := $(NETCDFLDFLAGS) $(METISLDFLAGS) -language charm++ -g -Wall

OBJS := adhydro.o      \
        mesh_element.o \
        file_manager.o \
        surfacewater.o \
        groundwater.o

adhydro: $(OBJS)
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

mesh_element.o: mesh_element.cpp    \
                mesh_element.h      \
                mesh_element.decl.h \
                mesh_element.def.h  \
                adhydro.h           \
                adhydro.decl.h      \
                surfacewater.h      \
                groundwater.h       \
                all.h
	$(CHARMC) $< -o $@ $(CPPFLAGS)

mesh_element.decl.h \
mesh_element.def.h: mesh_element.ci
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
	rm -f charmrun adhydro $(OBJS) adhydro.decl.h adhydro.def.h mesh_element.decl.h mesh_element.def.h file_manager.decl.h file_manager.def.h

.PHONY: clean
