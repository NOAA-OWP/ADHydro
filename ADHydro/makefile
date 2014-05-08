CHARMDIR    := /project/CI-WATER/rsteinke/charm
CHARMC      := $(CHARMDIR)/bin/charmc
NETCDFDIR   := /apps/NETCDF/netcdf-4.2.1.1_pgi
METISDIR    := /project/CI-WATER/rsteinke/metis-5.1.0
EXTRA_FLAGS := -I$(NETCDFDIR)/include -L$(NETCDFDIR)/lib -lnetcdf -L$(METISDIR)/build/Linux-x86_64/libmetis -lmetis -g -Wall -balancer MetisLB

OBJS := adhydro.o      \
        mesh_element.o \
        surfacewater.o \
        groundwater.o

adhydro: $(OBJS)
	$(CHARMC) -language charm++ -o adhydro $(OBJS) $(EXTRA_FLAGS)

adhydro.o: adhydro.cpp         \
           adhydro.h           \
           adhydro.decl.h      \
           adhydro.def.h       \
           mesh_element.h      \
           mesh_element.decl.h \
           all.h
	$(CHARMC) -o adhydro.o adhydro.cpp $(EXTRA_FLAGS)

adhydro.decl.h \
adhydro.def.h: adhydro.ci \
               mesh_element.ci
	$(CHARMC) adhydro.ci $(EXTRA_FLAGS)

mesh_element.o: mesh_element.cpp    \
                mesh_element.h      \
                mesh_element.decl.h \
                mesh_element.def.h  \
                adhydro.h           \
                adhydro.decl.h      \
                surfacewater.h      \
                groundwater.h       \
                all.h
	$(CHARMC) -o mesh_element.o mesh_element.cpp $(EXTRA_FLAGS)

mesh_element.decl.h \
mesh_element.def.h: mesh_element.ci
	$(CHARMC) mesh_element.ci $(EXTRA_FLAGS)

surfacewater.o: surfacewater.cpp \
                surfacewater.h   \
                all.h
	$(CHARMC) -o surfacewater.o surfacewater.cpp $(EXTRA_FLAGS)

groundwater.o: groundwater.cpp \
               groundwater.h   \
               all.h
	$(CHARMC) -o groundwater.o groundwater.cpp $(EXTRA_FLAGS)

clean:
	rm -f charmrun adhydro $(OBJS) adhydro.decl.h adhydro.def.h mesh_element.decl.h mesh_element.def.h
