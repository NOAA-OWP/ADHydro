CHARMDIR   := ~/Desktop/charm/
CHARMC     := $(CHARMDIR)/bin/charmc $(OPTS)
CHARM_INCL := $(CHARMDIR)/include

RESDIR     := ../generated_code/charm_reservoirs
FACTDIR    := ../generated_code/charm_factory
DIVDIR     := ../generated_code/charm_diversions
REGDIR     :=  $(RESDIR)/Div6
TESTDIR    := ../unit_test
ADHYDRODIR := ../../ADHydro

CPPFLAGS := -g -I. -I$(FACTDIR) -I$(RESDIR) -I$(REGDIR) -I$(DIVDIR) -I$(ADHYDRODIR)
VPATH    := $(FACTDIR) $(RESDIR) $(REGDIR) $(TESTDIR) $(DIVDIR) $(ADHYDRODIR)

OBJS := Creator.o          \
        ReservoirCreator.o \
        ReservoirFactory.o \
        Reservoir.o        \
        DiversionCreator.o \
        DiversionFactory.o \
        Diversion.o        \
        ParcelCreator.o    \
        ParcelFactory.o    \
        Parcel.o           \
        common_utilities.o

all: $(OBJS)

.PHONY: all

Creator.o: Creator.cpp # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

#Build reservoirs
ReservoirCreator.o: ReservoirCreator.cpp # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

ReservoirFactory.o: ReservoirFactory.cpp \
                    res%.decl.h # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

Reservoir.o: Reservoir.cpp \
             Reservoir.decl.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

Reservoir.decl.h \
Reservoir.def.h: Reservoir.ci
	$(CHARMC) $<

#Build Diversions
DiversionCreator.o: DiversionCreator.cpp # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

DiversionFactory.o: DiversionFactory.cpp \
                    div%.decl.h # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

Diversion.o: Diversion.cpp \
             Diversion.decl.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

Diversion.decl.h \
Diversion.def.h: Diversion.ci
	$(CHARMC) $<

#Build Parcels
ParcelCreator.o: ParcelCreator.cpp # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

ParcelFactory.o: ParcelFactory.cpp \
                    par%.decl.h # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

Parcel.o: Parcel.cpp \
          Parcel.decl.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

Parcel.decl.h \
Parcel.def.h: Parcel.ci
	$(CHARMC) $<

common_utilities.o: common_utilities.cpp
	$(CHARMC) $(CPPFLAGS) $< -o $@

#Pattern rule for charm generated headers.
res%.decl.h \
res%.def.h: reservoirs.ci Reservoir.ci diversions.ci Diversion.ci parcels.ci Parcel.ci
	$(CHARMC) $<

div%.decl.h \
div%.def.h: diversions.ci Diversion.ci parcels.ci Parcel.ci
	$(CHARMC) $<

par%.decl.h \
par%.def.h: parcels.ci Parcel.ci
	$(CHARMC) $<

test: $(OBJS)
	$(CHARMC) $(TESTDIR)/element.ci $(TESTDIR)/main.ci 
	$(CHARMC) $(CPPFLAGS) -o element.o $(TESTDIR)/element.cpp
	$(CHARMC) $(CPPFLAGS) -o main.o $(TESTDIR)/main.C
	$(CHARMC) -language charm++ -o test main.o element.o ReservoirFactory.o common_utilities.o Reservoir.o DiversionFactory.o Diversion.o ParcelFactory.o Parcel.o

clean:
	rm -f *.def.h *.decl.h *.o test ./charmrun
