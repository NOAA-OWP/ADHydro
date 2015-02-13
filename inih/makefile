CPPFLAGS := -Icpp -fdiagnostics-show-option -g -Wall -DINI_ALLOW_MULTILINE=0

VPATH := cpp

OBJS := ini.o \
        INIReader.o

all: $(OBJS)

.PHONY: all

ini.o: ini.c \
       ini.h

INIReader.o: INIReader.cpp \
             INIReader.h

clean:
	rm -f $(OBJS)

.PHONY: clean
