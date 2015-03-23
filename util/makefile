CPPFLAGS := -I../ADHydro_rewrite -fdiagnostics-show-option -g -Wall
LDFLAGS  := -fdiagnostics-show-option -g -Wall

CHARMC        := $(CHARMDIR)/bin/charmc
CHARMCPPFLAGS := $(CPPFLAGS)
CHARMLDFLAGS  := $(LDFLAGS) -language charm++

VPATH := ../ADHydro_rewrite

EXES := adhydro_mesh_check  \
        test_date_functions \
        test_random_simulation

TEST_RANDOM_SIMULATION_OBJS := test_random_simulation.o \
                               element.o                \
                               neighbor_proxy.o

all: $(EXES)

.PHONY: all

adhydro_mesh_check: all.h # Uses implicit rule to compile from adhydro_mesh_check.cpp.  This just adds the header file dependency.

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
