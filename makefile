#
# This is a top level make file to recursively build the various ADHydro sub components and ADHydro itself
# NOTE:
# 	You still need to modify various environment variables in their respective makefiles before using this file.
# 	TODO Pass these along from this top level makefile ^^^
#
.PHONY: inih HRLDAS management utils ADHydro clean

all: inih management util HRLDAS ADHydro

inih:
	$(MAKE) -C ./inih

HRLDAS:
	$(MAKE) -C ./HRLDAS-v3.6

#TODO Find a way to automatically regenerate code 
management:
	$(MAKE) -C ./management/build

util:
	$(MAKE) -C ./util

ADHydro:
	$(MAKE) -C ./ADHydro

clean:
	$(MAKE) -C ./ADHydro clean
	$(MAKE) -C ./util clean
	$(MAKE) -C ./management/build clean
	$(MAKE) -C ./HRLDAS-v3.6 clean
	$(MAKE) -C ./inih clean
