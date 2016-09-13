# Use these values for Mt Moran.

# FIXME

# Use these values for EN3038 lab.

METISDIR   := /opt
LIBDIRNAME := lib64
EXTRAFLAGS := -fdiagnostics-show-option

# Use these values for all.

CHARMC            := $(CHARMDIR)/bin/charmc -language charm++
METISLDFLAGS      := -L$(METISDIR)/$(LIBDIRNAME) -lmetis -balancer MetisLB
#OPTIMIZATIONFLAGS := -O3 -optimize -production
CPPFLAGS          := $(OPTIMIZATIONFLAGS) $(EXTRAFLAGS) -I../inih/cpp -g -Wall
LDFLAGS           := $(METISLDFLAGS) $(OPTIMIZATIONFLAGS) $(EXTRAFLAGS) -g -Wall

VPATH := ../inih ../inih/cpp

EXES := adhydro

ADHYDRO_OBJS := adhydro.o              \
                output_manager_charm.o \
                output_manager.o       \
                file_manager_NetCDF.o

INIH_OBJS := INIReader.o \
             ini.o

all: $(EXES)

.PHONY: all

adhydro: $(ADHYDRO_OBJS) $(INIH_OBJS)
	$(CHARMC) $(LDFLAGS) $^ -o $@

adhydro.o: adhydro.cpp                 \
           adhydro.h                   \
           adhydro.decl.h              \
           adhydro.def.h               \
           output_manager_charm.decl.h \
           all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

adhydro.decl.h \
adhydro.def.h: adhydro.ci \
               output_manager_charm.ci
	$(CHARMC) $<

output_manager_charm.o: output_manager_charm.cpp    \
                        output_manager_charm.h      \
                        output_manager_charm.decl.h \
                        output_manager_charm.def.h  \
                        adhydro.h                   \
                        adhydro.decl.h              \
                        output_manager.h            \
                        file_manager_NetCDF.h       \
                        file_manager.h              \
                        all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

output_manager_charm.decl.h \
output_manager_charm.def.h: output_manager_charm.ci
	$(CHARMC) $<

output_manager.o: output_manager.cpp \
                  output_manager.h   \
                  file_manager.h     \
                  all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

file_manager_NetCDF.o: file_manager_NetCDF.cpp \
                       file_manager_NetCDF.h   \
                       file_manager.h          \
                       all.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

clean:
	rm -f charmrun $(EXES) *.o *.decl.h *.def.h

.PHONY: clean
