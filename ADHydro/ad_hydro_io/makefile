######
#   Set CHARMDIR to be the install directory of charm
######

######
#EN LAB
######
#CHARMDIR    := /opt/build/charm

######
#MOUNT MORAN
######
#CHARMDIR   := /project/CI-WATER/tools/CI-WATER-tools/charm

######
#OTHER
######
#CHARMDIR   := ~/Desktop/charm/



CHARMC      := $(CHARMDIR)/bin/charmc $(OPTS)
CHARM_INCL  := $(CHARMDIR)/include

all: test

test:	interperter.o lexer.o parser.o ast.o

interperter.o:	lexer.o       \
		ast.o         \
		parser.o      

lexer.o: lexer.cpp

parser.o: lexer.o ast.o parser.cpp

ast.o: ast.cpp

.PHONY: all clean

clean:
	rm -f *.o a.out
