#include "Reservoir.h"

//Reservoir Constructor, sets reachCode on initialization
Reservoir::Reservoir(long long reachCode_):reachCode(reachCode_)
{
    //Empty
	/*
	 * This is where connections to a database could be established and data
	 * populated for the reservoirs.
	 * In this object oriented model, perhaps a database loader object???
	 *
	 * This could also be pushed to the subclasses to call and load data.
	 * Best to load whatever is standard across all reservoirs here, though.
	 */
}

//Reservoir Destructor
Reservoir::~Reservoir()
{

}


