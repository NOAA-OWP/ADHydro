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
//Abstract Creator classes for various components
//Forward declare the component classes

class Reservoir;
//Reservoir Creator
class Res_Creator
{
    public:
    virtual Reservoir* create(long long reach) = 0;

    virtual ~Res_Creator(){}
};

class Diversion;
//Diversion Creator
class Div_Creator
{
    public:
    virtual Diversion* create(int elementID) = 0;

    virtual ~Div_Creator(){}
};

class Parcel;
//Parcel Creator
class Parcel_Creator
{
    public:
    virtual Parcel* create(int ParcelID) = 0;

    virtual ~Parcel_Creator(){}
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

_cxx_diversion_creator_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef DIVERSION_CREATOR_H
#define DIVERSION_CREATOR_H
#include "Creator.h"
#include "DiversionFactory.h"
#include <iostream>
#include <map>

//Template class for diversion creators
//Each diversion needs to have a DiversionCreator instance
template <class T>
class DiversionCreator : public Div_Creator
{
    public:
	/*
	 * DiversionCreator constructor
	 * This constructor calls the DiversionFactory registerDiversion function
	 * to automatically add itself to the factory's lookup table.
	 */
    DiversionCreator<T>(int elementID, DiversionFactory* factory)
    {
    	factory->registerDiversion(elementID, this);
    }
    /*
     * This function creates an appropriate Diversion object based
     * on the template parameter
     */
    Diversion* create(int elementID)
    {
        return new T;
    }
    /*
     * ReservoirCreator Destructor
     */
    virtual ~DiversionCreator<T>(){}
};
#endif
"""

_cxx_diversion_creator_def_string = \
"""
/*
    This is auto-generated code from TODO/FIXME!!!
*/
#include "DiversionCreator.h"
//Empty cpp to allow make to easily compile the header
"""

_cxx_parcel_creator_header_string = \
"""/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef PARCEL_CREATOR_H
#define PARCEL_CREATOR_H
#include "Creator.h"
#include "ParcelFactory.h"
#include <iostream>
#include <map>

//Template class for parcel creators
//Each parcel needs to have a ParcelCreator instance
template <class T>
class ParcelCreator : public Parcel_Creator
{
    public:
	/*
	 * ParcelCreator constructor
	 * This constructor calls the ParcelFactory registerParcel function
	 * to automatically add itself to the factory's lookup table.
	 */
    ParcelCreator<T>(int parcelID, ParcelFactory* factory)
    {
    	factory->registerParcel(parcelID, this);
    }
    /*
     * This function creates an appropriate Parcel object based
     * on the template parameter
     */
    Parcel* create(int parcelID)
    {
        return new T;
    }
    /*
     * ParcelCreator Destructor
     */
    virtual ~ParcelCreator<T>(){}
};
#endif
"""

_cxx_parcel_creator_def_string = \
"""
/*
    This is auto-generated code from TODO/FIXME!!!
*/
#include "ParcelCreator.h"
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
${PUP_LIST}
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

_cxx_diversion_factory_header_string = \
"""
/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef DIVERSION_FACTORY_H
#define DIVERSION_FACTORY_H
#include "Creator.h"
#include <map>
#include <vector>

/*
    Factory class for Diversion managment components
*/
class DiversionFactory
{
    private:
    std::map<int, Div_Creator*> table;
    std::vector<Div_Creator*> creators;
    public:
    /*
     *	DiversionFactory constructor
     *	This function creates the Diversion factory.
     *	Each component needs to have its creator object
     *	defined, and the implementation will register the creator
     *	with the factory's table.
     */
    DiversionFactory();

    /*
     *	DiversionFactory destructor
     *	Responsible for removing creator instances from the creators vector
     */
    ~DiversionFactory();

    /*
     *	Component creation
     *	Once components and their creators are in the map,
     *	create can look up the appropriate component based on the elementID
     *	and can create an appropriate Diversion object
     */
    Diversion* create(int elementID);
    /*
     *	Diversion Creator Registration
     *	resgisterDiversion adds an entry to the table map for each elementID,
     *  providing a mapping from elementID to the Creator that knows
     *	how to create an object representing that element's Diversion.
     *
     *	Note this registration is done automatically via the DiversionCreator
     *	constructor, which takes a reference to the factory the DiversionCreator
     *	is being added to and calls this function to register itself.
     */
    void registerDiversion(int elementID, Div_Creator* creator);

};

#endif
"""

_cxx_diversion_factory_string = \
"""
/*
    This is auto-generated code from TODO/FIXME!!!
*/
#include "DiversionCreator.h"
//NEED TO INCLUDE EACH COMPONENT DEFINITION AS WELL AS ADD TO CONSTRUCTOR
${INCLUDE_LIST}

//DiversionFactory Constructor
DiversionFactory::DiversionFactory()
{
	//For each component, we need to hold a reference to the component's Creator
	//Until Factory is destroyed.
	//THIS IS WHERE NEW COMPONENTS NEED TO BE ADDED TO THE FACTORY
	${CREATOR_LIST}
}

DiversionFactory::~DiversionFactory()
{
	//Clean up all of the allocated creators when the factory is destroyed
	for(std::vector<Div_Creator*>::iterator it = creators.begin(); it != creators.end(); ++it)
	{
		delete *it;
	}
}


void DiversionFactory::registerDiversion(int elementID, Div_Creator* creator)
{
	//Register each diversion's creator, identified by its elementID
    table[elementID] = creator;
}

Diversion* DiversionFactory::create(int elementID)
{
	//When we "create" a new Diversion instance, we need to
	//search through the factory table to find the creator
	//that knows how to create that Diversion object
	//If it isn't in the table, return NULL
    std::map<int, Div_Creator*>::iterator i;
    i = table.find(elementID);

    if(i != table.end())
    {
        return i->second->create(elementID);
    }    
    else
        return (Diversion*) NULL;
}
"""

_cxx_parcel_factory_header_string = \
"""
/*
    This is auto-generated code from TODO/FIXME!!!
*/
#ifndef PARCEL_FACTORY_H
#define PARCEL_FACTORY_H
#include "Creator.h"
#include <map>
#include <vector>

/*
    Factory class for Parcel managment components
*/
class ParcelFactory
{
    private:
    std::map<int, Parcel_Creator*> table;
    std::vector<Parcel_Creator*> creators;
    public:
    /*
     *	ParcelFactory constructor
     *	This function creates the Parcel factory.
     *	Each component needs to have its creator object
     *	defined, and the implementation will register the creator
     *	with the factory's table.
     */
    ParcelFactory();

    /*
     *	DiversionFactory destructor
     *	Responsible for removing creator instances from the creators vector
     */

    ~ParcelFactory();

    /*
     *	Component creation
     *	Once components and their creators are in the map,
     *	create can look up the appropriate component based on the parcelID
     *	and can create an appropriate Parcel object
     */

    Parcel* create(int parcelID);
    
    /*
     *	Parcel Creator Registration
     *	resgisterParcel adds an entry to the table map for each parcelID,
     *  providing a mapping from parcelID to the Creator that knows
     *	how to create an object representing that Parcel.
     *
     *	Note this registration is done automatically via the ParcelCreator
     *	constructor, which takes a reference to the factory the ParcelCreator
     *	is being added to and calls this function to register itself.
     */

    void registerParcel(int parcelID, Parcel_Creator* creator);

};

#endif
"""

_cxx_parcel_factory_string = \
"""
/*
    This is auto-generated code from TODO/FIXME!!!
*/
#include "ParcelCreator.h"
//NEED TO INCLUDE EACH COMPONENT DEFINITION AS WELL AS ADD TO CONSTRUCTOR
${INCLUDE_LIST}

//ParcelFactory Constructor
ParcelFactory::ParcelFactory()
{
	//For each component, we need to hold a reference to the component's Creator
	//Until Factory is destroyed.
	//THIS IS WHERE NEW COMPONENTS NEED TO BE ADDED TO THE FACTORY
	${CREATOR_LIST}
}

ParcelFactory::~ParcelFactory()
{
	//Clean up all of the allocated creators when the factory is destroyed
	for(std::vector<Parcel_Creator*>::iterator it = creators.begin(); it != creators.end(); ++it)
	{
		delete *it;
	}
}


void ParcelFactory::registerParcel(int parcelID, Parcel_Creator* creator)
{
	//Register each parcel's creator, identified by its parcelID
    table[parcelID] = creator;
}

Parcel* ParcelFactory::create(int parcelID)
{
	//When we "create" a new Parcel instance, we need to
	//search through the factory table to find the creator
	//that knows how to create that Parcel object
	//If it isn't in the table, return NULL
    std::map<int, Parcel_Creator*>::iterator i;
    i = table.find(parcelID);

    if(i != table.end())
    {
        return i->second->create(parcelID);
    }    
    else
        return (Parcel*) NULL;
}
"""

class ResFactory():
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
        include_list = ['#include "{0}.cpp"'.format(n) for n in names]
        include_string = linesep.join(include_list)
        creator_list = ['\tcreators.push_back(new ReservoirCreator<{0}>({0}::reachCode, this));'.format(n) for n in names]
        creator_string = linesep.join(creator_list)
        self.template = Template(self.template.safe_substitute(INCLUDE_LIST=include_string, CREATOR_LIST=creator_string))

    def abstract(self):
        return {'Creator.h':_cxx_creator_abstract_header_string, 
                'Creator.cpp':_cxx_creator_abstract_def_string, 
                'ReservoirCreator.h':_cxx_res_creator_header_string, 
                'ReservoirCreator.cpp':_cxx_res_creator_def_string, 
                'ReservoirFactory.h':_cxx_res_factory_header_string}

class DiversionFactory():
    """
        Everytime a Diversion object is created, the DiversionFactory
        Constructor needs to be updated to register the Diversion with
        the factory.  This class is a template of the DiversionFactory.cpp
        file that can be used to automatically create the .cpp file 
        with the required registration.
    """
    def __init__(self):
        self.template = Template(_cxx_diversion_factory_string)

    def __str__(self):
        #Use safe_substitute to avoid modifying the template
        return self.template.safe_substitute()

    def creators(self, names):
        include_list = ['#include "{0}.cpp"'.format(n) for n in names]
        include_string = linesep.join(include_list)
        creator_list = ['\tcreators.push_back(new DiversionCreator<{0}>({0}::elementID, this));'.format(n) for n in names]
        creator_string = linesep.join(creator_list)
        self.template = Template(self.template.safe_substitute(INCLUDE_LIST=include_string, CREATOR_LIST=creator_string))

    def abstract(self):
        return {'Creator.h':_cxx_creator_abstract_header_string, 
                'Creator.cpp':_cxx_creator_abstract_def_string, 
                'DiversionCreator.h':_cxx_diversion_creator_header_string, 
                'DiversionCreator.cpp':_cxx_diversion_creator_def_string, 
                'DiversionFactory.h':_cxx_diversion_factory_header_string}

class ParcelFactory():
    """
        Everytime a Parcel object is created, the ParcelFactory
        Constructor needs to be updated to register the Parcel with
        the factory.  This class is a template of the ParcelFactory.cpp
        file that can be used to automatically create the .cpp file 
        with the required registration.
    """
    def __init__(self):
        self.template = Template(_cxx_parcel_factory_string)

    def __str__(self):
        #Use safe_substitute to avoid modifying the template
        return self.template.safe_substitute()

    def creators(self, names):
        include_list = ['#include "{0}.cpp"'.format(n) for n in names]
        pup_list = ['register{0}();' for n in names]
        include_string = linesep.join(include_list)
        creator_list = ['\tcreators.push_back(new ParcelCreator<{0}>({0}::parcelID, this));'.format(n) for n in names]
        creator_string = linesep.join(creator_list)
        self.template = Template(self.template.safe_substitute(INCLUDE_LIST=include_string, CREATOR_LIST=creator_string, PUP_LIST=pup_list))

    def abstract(self):
        return {'Creator.h':_cxx_creator_abstract_header_string, 
                'Creator.cpp':_cxx_creator_abstract_def_string, 
                'ParcelCreator.h':_cxx_parcel_creator_header_string, 
                'ParcelCreator.cpp':_cxx_parcel_creator_def_string, 
                'ParcelFactory.h':_cxx_parcel_factory_header_string}
