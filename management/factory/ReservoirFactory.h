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
    std::map<long long, Creator*> table;
    std::vector<Creator*> creators;
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
    void registerReservoir(long long reach, Creator* creator);

};

#endif
