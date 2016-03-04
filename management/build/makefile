CHARMDIR = ~/Desktop/charm/
CHARMC = $(CHARMDIR)/bin/charmc $(OPTS)
CHARM_INCL = $(CHARMDIR)/include

RESDIR  = ../generated_code/charm_reservoirs
FACTDIR = ../generated_code/charm_factory
REGDIR  =  $(RESDIR)/Div6
TESTDIR = ../unit_test

CPPFLAGS := -I. -I$(FACTDIR) -I$(RESDIR) -I$(REGDIR)
VPATH    := $(FACTDIR) $(RESDIR) $(REGDIR) $(TESTDIR)

OBJS := Creator.o          \
        ReservoirCreator.o \
        ReservoirFactory.o \
		Reservoir.o        \
		common_utilities.o

all: $(OBJS)

.PHONY: all

Creator.o: Creator.cpp # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

ReservoirCreator.o: ReservoirCreator.cpp # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

ReservoirFactory.o: ReservoirFactory.cpp \
                    %.decl.h # FIXME add .h file dependencies
	$(CHARMC) $(CPPFLAGS) $< -o $@

Reservoir.o: Reservoir.cpp \
             Reservoir.decl.h
	$(CHARMC) $(CPPFLAGS) $< -o $@

Reservoir.decl.h \
Reservoir.def.h: Reservoir.ci
	$(CHARMC) $<

common_utilities.o: common_utilities.cpp
	$(CHARMC) $(CPPFLAGS) $< -o $@

#Pattern rule for charm generated headers.
%.decl.h \
%.def.h: reservoirs.ci Reservoir.ci
	$(CHARMC) $<

test: $(OBJS)
	$(CHARMC) $(TESTDIR)/element.ci $(TESTDIR)/main.ci 
	$(CHARMC) $(CPPFLAGS) -o element.o $(TESTDIR)/element.cpp
	$(CHARMC) $(CPPFLAGS) -o main.o $(TESTDIR)/main.C
	$(CHARMC) -language charm++ -o test main.o element.o ReservoirFactory.o common_utilities.o Reservoir.o

clean:
	rm -f *.def.h *.decl.h *.o test charmrun
