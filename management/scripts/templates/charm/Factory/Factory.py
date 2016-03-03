#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

This is a template module for automatically generating 
management components for ADHydro.

"""
from string import Template
from os import linesep

_cxx_creator_abstract_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef CREATOR_H
#define CREATOR_H
#include "Reservoir.h"
//Abstract Creator classes for various components

//Reservoir Creator
class Res_Creator
{
    public:
    virtual Reservoir* create(long long reach) = 0;

    virtual ~Res_Creator(){}
};

#endif
"""

_cxx_creator_abstract_def_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#include "Creator.h"
//Abstract class, empty cpp for make
"""

_cxx_res_creator_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef RES_CREATOR_H
#define RES_CREATOR_H
#include "Creator.h"
#include "ReservoirFactory.h"
#include <iostream>
#include <map>

//Template class for reservoir creators
//Each reservoir needs to have a ReservoirCreator instance
template <class T>
class ReservoirCreator : public Res_Creator
{
    public:
	/*
	 * ReservoirCreator constructor
	 * This constructor calls the ReservoirFactory registerReservoir function
	 * to automatically add itself to the factory's lookup table.
	 */
    ReservoirCreator<T>(long long reach, ReservoirFactory* factory)
    {
    	factory->registerReservoir(reach, this);
    }
    /*
     * This function creates an appropriate Reservoir object based
     * on the template parameter
     */
    Reservoir* create(long long reach)
    {
        return new T;
    }
    /*
     * ReservoirCreator Destructor
     */
    virtual ~ReservoirCreator<T>(){}
};
#endif
"""

_cxx_res_creator_def_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#include "ReservoirCreator.h"
//Empty cpp to allow make to easily compile the header
"""

_cxx_res_factory_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef RES_FACTORY_H
#define RES_FACTORY_H
#include "Creator.h"
#include <map>
#include <vector>

/*
    Factory class for reservoir managment components
*/
class ReservoirFactory
{
    private:
    std::map<long long, Res_Creator*> table;
    std::vector<Res_Creator*> creators;
    public:
    /*
     *	ReservoirFactory constructor
     *	This function creates the reservoir factory.
     *	Each component needs to have its creator object
     *	defined, and the implementation will register the creator
     *	with the factory's table.
     */
    ReservoirFactory();

    /*
     *	ReservoirFactory destructor
     *	Responsible for removing creator instances from the creators vector
     */
    ~ReservoirFactory();

    /*
     *	Component creation
     *	Once components are their creators are in the map,
     *	create can look up the appropriate component based on the reach code
     *	and can create an appropriate reservoir object
     */
    Reservoir* create(long long reach);
    /*
     *	Reservoir Creator Registration
     *	resgisterReservoir adds an entry to the table map for each reach
     *	code, providing a mapping from reach code to the Creator that knows
     *	how to create an object representing that reach code.
     *
     *	Note this registration is done automatically via the ReservoirCreator
     *	constructor, which takes a reference to the factory the reservoirCreator
     *	is being added to and calls this function to register itself.
     */
    void registerReservoir(long long reach, Res_Creator* creator);

};

#endif
"""

_cxx_res_factory_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#include "ReservoirCreator.h"
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
        self.template = Template(_cxx_res_factory_string)

    def __str__(self):
        #Use safe_substitute to avoid modifying the template
        return self.template.safe_substitute()

    def creators(self, names):
        include_list = ['#include "../reservoirs/{0}.cpp"'.format(n) for n in names]
        include_string = linesep.join(include_list)
        creator_list = ['\tcreators.push_back(new ReservoirCreator<{0}>({0}::reachCode, this));'.format(n) for n in names]
        creator_string = linesep.join(creator_list)
        self.template = Template(self.template.safe_substitute(INCLUDE_LIST=include_string, CREATOR_LIST=creator_string))

def abstract():
    return {'Creator.h':_cxx_creator_abstract_header_string, 'Creator.cpp':_cxx_creator_abstract_def_string, 'ReservoirCreator.h':_cxx_res_creator_header_string, 'ReservoirCreator.cpp':_cxx_res_creator_def_string, 'ReservoirFactory.h':_cxx_res_factory_header_string}
