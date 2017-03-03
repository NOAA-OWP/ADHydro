# Use these values for Mt Moran.

# FIXME

# Use these values for EN3038 lab.

METISDIR         := /opt/lib64
EXTRAFLAGS       := -fdiagnostics-show-option

# Use these values for all.

CHARMC            := $(CHARMDIR)/bin/charmc -language charm++
NETCDFLDFLAGS     := -lnetcdf -lhdf5_hl -lhdf5 -lcurl
METISLDFLAGS      := -L$(METISDIR) -lmetis -balancer MetisLB
#OPTIMIZATIONFLAGS := -O3 -optimize -production
CPPFLAGS          := $(OPTIMIZATIONFLAGS) $(EXTRAFLAGS) -I../inih/cpp -g -Wall
LDFLAGS           := $(NETCDFLDFLAGS) $(METISLDFLAGS) $(OPTIMIZATIONFLAGS) $(EXTRAFLAGS) -g -Wall

VPATH := ../inih \
         ../inih/cpp

EXES := adhydro

ADHYDRO_OBJS := mesh_element.o        \
                neighbor_proxy.o      \
                evapo_transpiration.o \
                readonly.o

INIH_OBJS := INIReader.o \
             ini.o

all: $(EXES)

.PHONY: all

adhydro: $(ADHYDRO_OBJS) $(INIH_OBJS)
# FIXME uncomment	$(CHARMC) $(LDFLAGS) $^ -o $@

mesh_element.o: mesh_element.cpp        \
                mesh_element.h          \
                neighbor_proxy.h        \
                evapo_transpiration.h   \
                readonly.h              \
                all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

neighbor_proxy.o: neighbor_proxy.cpp \
                  neighbor_proxy.h   \
                  all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

evapo_transpiration.o: evapo_transpiration.cpp \
                       evapo_transpiration.h   \
                       readonly.h              \
                       all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

readonly.o: readonly.cpp \
            readonly.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

clean:
	rm -f charmrun $(EXES) *.o *.decl.h *.def.h

.PHONY: clean
