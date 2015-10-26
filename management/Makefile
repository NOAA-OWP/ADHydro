CC = g++
CC_FLAGS := -Wall -g
#LD_FLAGS :=

EXEC := test
SOURCES = $(wildcard *.cpp components/*.cpp factory/*.cpp reservoir/*.cpp reservoir/region1/*.cpp reservoir/region1/subregion1/*.cpp)
SWAPFILES = $(wildcard *~ components/*~ factory/*~ reservoir/*~ reservoir/region1/*~ reservoir/region1/subregion1/~*)
OBJECTS = $(SOURCES:.cpp=.o)

$(EXEC): $(OBJECTS)
	$(CC) $(LD_FLAGS) $(OBJECTS) -o $(EXEC)

%.o: %.cpp
	$(CC) $(CC_FLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS) $(SWAPFILES)

clean_o:
	rm -f $(OBJECTS) $(SWAPFILES)
