#
# This is a top level make file to recursively build the various ADHydro sub components and ADHydro itself
# NOTE:
# 	You still need to modify various environment variables in their respective makefiles before using this file.
# 	TODO Pass these along from this top level makefile ^^^
#
.PHONY: util scripts inih HRLDAS management ADHydro clean

all:    util scripts inih HRLDAS management ADHydro

util:
	$(MAKE) -C ./util

scripts:
	$(MAKE) -C ./scripts

inih:
	$(MAKE) -C ./inih

HRLDAS:
	$(MAKE) -C ./HRLDAS-v3.6

#TODO Find a way to automatically regenerate code 
management:
	$(MAKE) -C ./management/build

ADHydro:
	$(MAKE) -C ./ADHydro

clean:
	$(MAKE) -C ./util             clean
	$(MAKE) -C ./scripts          clean
	$(MAKE) -C ./inih             clean
	$(MAKE) -C ./HRLDAS-v3.6      clean
	$(MAKE) -C ./management/build clean
	$(MAKE) -C ./ADHydro          clean

