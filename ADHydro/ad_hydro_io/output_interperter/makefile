######
#   Set CHARMDIR to be the install directory of charm
######

######
#EN LAB
######
CHARMDIR    := /opt/build/charm

######
#MOUNT MORAN
######
#CHARMDIR   := /project/CI-WATER/tools/CI-WATER-tools/charm

######
#OTHER
######
#CHARMDIR   := ~/Desktop/charm/

TESTDIR     := ./unit_test/
CXXFLAGS=-g -I../ -I../../management/generated_code/charm_diversions -I../../management/generated_code/charm_factory -I../../management/generated_code/charm_reservoirs -I../../inih/cpp
TESTCXXFLAGS=-g -I./unit_test -I./

 
LDFLAGS=-g -lproj

CHARMC      := $(CHARMDIR)/bin/charmc $(OPTS)
CHARM_INCL  := $(CHARMDIR)/include
CXX = $(CHARMC)

.PHONY: all clean output_manager

all: test output_manager

test:	interperter.o lexer.o parser.o ast.o

interperter.o:	lexer.o ast.o parser.o interperter.h interperter.cpp

lexer.o: lexer.cpp

parser.o: lexer.o ast.o parser.cpp

ast.o: ast.cpp

output_manager: OutputManager.o

OutputManager.o: OutputManager.h OutputManager.cpp
	$(CHARMC) OutputManager.ci
	$(CHARMC) $(TESTCXXFLAGS) -c -o tmp.o OutputManager.cpp
	$(MAKE) -C ../
	$(LD) -r ../adhydro_io.o tmp.o -o OutputManager.o
clean:
	rm -f *.o a.out *.def.h *.decl.h
