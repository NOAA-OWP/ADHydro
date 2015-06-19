# Use these values for Mt Moran.

#CHARMDIR   := /project/CI-WATER/tools/build/charm

# Use these values for EN3038 lab.

EXTRACPPFLAGS := -I/usr/include/libshp -fdiagnostics-show-option
EXTRALDFLAGS  := -lnetcdf -lshp -fdiagnostics-show-option

# Use these values for all.

CPPFLAGS := $(EXTRACPPFLAGS) -I../ADHydro_rewrite -g -Wall
LDFLAGS  := $(EXTRALDFLAGS) -g -Wall

CHARMC        := $(CHARMDIR)/bin/charmc
CHARMCPPFLAGS := $(CPPFLAGS)
CHARMLDFLAGS  := $(LDFLAGS) -language charm++

VPATH := ../ADHydro_rewrite

EXES := adhydro_channel_preprocessing    \
        adhydro_mesh_check               \
        adhydro_create_channel_shapefile \
        test_date_functions              \
        test_random_simulation

TEST_RANDOM_SIMULATION_OBJS := test_random_simulation.o \
                               element.o                \
                               neighbor_proxy.o

all: $(EXES)

.PHONY: all

adhydro_channel_preprocessing: all.h # Uses implicit rule to compile from adhydro_channel_preprocessing.cpp.  This just adds the header file dependency.

adhydro_mesh_check: all.h # Uses implicit rule to compile from adhydro_mesh_check.cpp.  This just adds the header file dependency.

adhydro_create_channel_shapefile: all.h # Uses implicit rule to compile from adhydro_create_channel_shapefile.cpp.  This just adds the header file dependency.

test_date_functions: all.h # Uses implicit rule to compile from test_date_functions.cpp.  This just adds the header file dependency.

test_random_simulation: $(TEST_RANDOM_SIMULATION_OBJS)
	$(CHARMC) $(CHARMLDFLAGS) $^ -o $@

test_random_simulation.o: test_random_simulation.cpp    \
                          test_random_simulation.decl.h \
                          test_random_simulation.def.h  \
                          element.h                     \
                          neighbor_proxy.h              \
                          all_charm.h                   \
                          all.h
	$(CHARMC) $(CHARMCPPFLAGS) $< -o $@

test_random_simulation.decl.h \
test_random_simulation.def.h: test_random_simulation.ci
	$(CHARMC) $<

element.o: element.cpp        \
           element.h          \
           neighbor_proxy.h   \
           all_charm.h        \
           all.h
	$(CHARMC) $(CHARMCPPFLAGS) $< -o $@
           
neighbor_proxy.o: neighbor_proxy.cpp \
                  neighbor_proxy.h   \
                  all_charm.h        \
                  all.h
	$(CHARMC) $(CHARMCPPFLAGS) $< -o $@

clean:
	rm -f charmrun $(EXES) *.o *.decl.h *.def.h

.PHONY: clean
