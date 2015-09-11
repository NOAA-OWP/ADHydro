#include "ReservoirCreator.h"
//NEED TO INCLUDE EACH COMPONENT DEFINITION AS WELL AS ADD TO CONSTRUCTOR
#include "../components/comp1.cpp"
#include "../components/comp2.cpp"

//ReservoirFactory Constructor
ReservoirFactory::ReservoirFactory()
{
	//For each component, we need to hold a reference to the component's Creator
	//Until Factory is destroyed.
	//THIS IS WHERE NEW COMPONENTS NEED TO BE ADDED TO THE FACTORY
    creators.push_back(new ReservoirCreator<comp1>(comp1::reachCode, this));
    creators.push_back(new ReservoirCreator<comp2>(comp2::reachCode, this));
}

ReservoirFactory::~ReservoirFactory()
{
	//Clean up all of the allocated creators when the factory is destroyed
	for(std::vector<Creator*>::iterator it = creators.begin(); it != creators.end(); ++it)
	{
		delete *it;
	}
}


void ReservoirFactory::registerReservoir(long long reach, Creator* creator)
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
    std::map<long long, Creator*>::iterator i;
    i = table.find(reach);

    if(i != table.end())
    {
        return i->second->create(reach);
    }    
    else
        return (Reservoir*) NULL;
}
