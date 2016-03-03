#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template module for automatically generating 
management components for ADHydro.

"""
from string import Template
from os import linesep
_cxx_class_string = \
"""#include "ReservoirCreator.h"
//NEED TO INCLUDE EACH COMPONENT DEFINITION AS WELL AS ADD TO CONSTRUCTOR
${INCLUDE_LIST}

//ReservoirFactory Constructor
ReservoirFactory::ReservoirFactory()
{
	//For each component, we need to hold a reference to the component's Creator
	//Until Factory is destroyed.
	//THIS IS WHERE NEW COMPONENTS NEED TO BE ADDED TO THE FACTORY
${CREATOR_LIST}
}

ReservoirFactory::~ReservoirFactory()
{
	//Clean up all of the allocated creators when the factory is destroyed
	for(std::vector<Res_Creator*>::iterator it = creators.begin(); it != creators.end(); ++it)
	{
		delete *it;
	}
}


void ReservoirFactory::registerReservoir(long long reach, Res_Creator* creator)
{
	//Register each reservoir's creator, identified by its reach code
    table[reach] = creator;
}

Reservoir* ReservoirFactory::create(long long reach)
{
	//When we "create" a new reservoir instance, we need to
	//search through the factory table to find the creator
	//that knows how to create that reservoir object
	//If it isn't in the table, return NULL
    std::map<long long, Res_Creator*>::iterator i;
    i = table.find(reach);

    if(i != table.end())
    {
        return i->second->create(reach);
    }    
    else
        return (Reservoir*) NULL;
}
"""
class Factory():
    """
        Everytime a reservoir object is created, the ReservoirFactory
        Constructor needs to be updated to register the reservoir with
        the factory.  This class is a template of the ReservoirFactory.cpp
        file that can be used to automatically create the .cpp file 
        with the required registration.
    """
    def __init__(self):
        self.template = Template(_cxx_class_string)

    def __str__(self):
        #Use safe_substitute to avoid modifying the template
        return self.template.safe_substitute()

    def creators(self, names):
        include_list = ['#include "../reservoirs/{0}.cpp"'.format(n) for n in names]
        include_string = linesep.join(include_list)
        creator_list = ['\tcreators.push_back(new ReservoirCreator<{0}>({0}::reachCode, this));'.format(n) for n in names]
        creator_string = linesep.join(creator_list)
        self.template = Template(self.template.safe_substitute(INCLUDE_LIST=include_string, CREATOR_LIST=creator_string))
