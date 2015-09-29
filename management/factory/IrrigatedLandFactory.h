#ifndef IRR_LAND_FACTORY_H
#define IRR_LAND_FACTORY_H
#include "Creator.h"
#include <map>
#include <vector>

/*
    Factory class for Irrigated Land components
*/
class IrrigatedLandFactory
{
    private:
    std::map<long long, Parcel_Creator*> table;
    std::vector<Parcel_Creator*> creators;
    public:
    /*
     *	IrrigatedLandFactory constructor
     *	This function creates the IrrigatedLand factory.
     *	Each component needs to have its creator object
     *	defined, and the implementation will register the creator
     *	with the factory's table.
     */
    IrrigatedLandFactory();

    /*
     *	IrrigatedLandFactory destructor
     *	Responsible for removing creator instances from the creators vector
     */
    ~IrrigatedLandFactory();

    /*
     *	Component creation
     *	Once components are their creators are in the map,
     *	create can look up the appropriate component based on the id
     *	and can create an appropriate IrrigatedLand object
     */
    IrrigatedLand* create(long long id);
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
    void registerParcel(long long id, Parcel_Creator* creator);

};

#endif
