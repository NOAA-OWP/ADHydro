#include "IrrigatedLandCreator.h"
//NEED TO INCLUDE EACH COMPONENT DEFINITION AS WELL AS ADD TO CONSTRUCTOR
#include "../irrigated_lands/il_comp1.cpp"
#include "../irrigated_lands/il_comp2.cpp"

//IrrigatedLandFactory Constructor
IrrigatedLandFactory::IrrigatedLandFactory()
{
	//For each component, we need to hold a reference to the component's Creator
	//Until Factory is destroyed.
	//THIS IS WHERE NEW COMPONENTS NEED TO BE ADDED TO THE FACTORY
    creators.push_back(new IrrigatedLandCreator<il_comp1>(il_comp1::id, this));
    creators.push_back(new IrrigatedLandCreator<il_comp2>(il_comp2::id, this));
}

IrrigatedLandFactory::~IrrigatedLandFactory()
{
	//Clean up all of the allocated creators when the factory is destroyed
	for(std::vector<Parcel_Creator*>::iterator it = creators.begin(); it != creators.end(); ++it)
	{
		delete *it;
	}
}


void IrrigatedLandFactory::registerParcel(long long id, Parcel_Creator* creator)
{
	//Register each irrigated land's creator, identified by its id
    table[id] = creator;
}

IrrigatedLand* IrrigatedLandFactory::create(long long id)
{
	//When we "create" a new IrrigatedLand instance, we need to
	//search through the factory table to find the creator
	//that knows how to create that IrrigatedLand object
	//If it isn't in the table, return NULL
    std::map<long long, Parcel_Creator*>::iterator i;
    i = table.find(id);

    if(i != table.end())
    {
        return i->second->create(id);
    }    
    else
        return (IrrigatedLand*) NULL;
}
