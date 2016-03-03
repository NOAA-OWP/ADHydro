RESDIR = ../generated_code/charm_reservoirs
FACDIR = ../generaged_code/charm_factory
COMDIR = ../common

RES_INC = -I $(RESDIR) -I $(FACDIR) -I $(COMDIR)

OPTS = -g
CHARMDIR = ~/Desktop/charm/
CHARMC = $(CHARMDIR)/bin/charmc $(OPTS)
CHARM_INCL = $(CHARMDIR)/include


default: all
all: test

test : reservoirs.ci main.o element.o ReservoirFactory.o common.o Reservoir.o
	$(CHARMC) -language charm++ -o test main.o element.o $(FACDIR)/ReservoirFactory.o $(COMDIR)/common.o $(RESDIR)/Reservoir.o

main.o : main.C main.h main.decl.h main.def.h element.decl.h
	$(CHARMC) -o main.o main.C

main.decl.h main.def.h : main.ci
	$(CHARMC) main.ci

element.o: element.cpp element.h element.decl.h element.def.h main.decl.h
	$(CHARMC) $(RES_INC) -o element.o element.cpp

element.decl.h element.def.h : element.ci
	$(CHARMC) element.ci

ReservoirFactory.o: reservoirs.ci $(FACDIR)/ReservoirFactory.h $(FACDIR)/ReservoirFactory.cpp
	$(CHARMC) $(RES_INC) -I $(CHARM_INCL) -o $(FACDIR)/ReservoirFactory.o $(FACDIR)/ReservoirFactory.cpp

reservoirs.ci: $(RESDIR)/reservoirs.ci
	$(CHARMC) $(RESDIR)/reservoirs.ci
	mv *.def.h $(RESDIR)
	mv *.decl.h $(RESDIR)

common.o: $(COMDIR)/common_utilities.h $(COMDIR)/common_utilities.cpp
	$(CHARMC) -o $(COMDIR)/common.o $(COMDIR)/common_utilities.cpp

Reservoir.o: $(RESDIR)Reservoir.h $(RESDIR)/Reservoir.cpp Reservoir.decl.h Reservoir.def.h
	$(CHARMC) $(RES_INC) -I $(CHARM_INCL) -o $(RESDIR)/Reservoir.o $(RESDIR)/Reservoir.cpp

Reservoir.decl.h Reservoir.def.h:
	$(CHARMC) $(RESDIR)/Reservoir.ci
	mv Reservoir.decl.h $(RESDIR)/Reservoir.decl.h
	mv Reservoir.def.h $(RESDIR)/Reservoir.def.h

clean:
	rm -f main.decl.h main.def.h main.o
	rm -f element.decl.h element.def.h element.o
	rm -f $(RESDIR)/Reservoir.o
	rm -f $(FACDIR)/ReservoirFactory.o
	rm -f $(COMDIR)/common.o
	rm -f $(RESDIR)/*.def.h
	rm -f $(RESDIR)/*.decl.h
	rm -f test charmrun
