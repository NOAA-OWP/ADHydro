#include "../components/Reservoir.h"
#include "region1/subregion1/district54.h"
#include <iostream>
//Test component class that subclasses both Reservoir and a District
//This allows a component to be a reservoir as well as a subclass of a district
//In this fashion, a reservoir can defer its release to a regional release function
class comp2 : public Reservoir, District54
{
public:

	/* ReachCode identifying the NHD waterbody corresponding to this reservoir */
	/*
	 * To simplify construction of ReservoirCreators, this is a static member that can be accessed
	 * without having to instansiate the object.  The parent also keeps the reach code which is
	 * initialized when the child is constructed so the reach code can be accessed via a generic
	 * Reservoir*
	 */
	static const long long reachCode = 14050001003012;
    comp2():Reservoir(this->reachCode){/*call database, load target volume*/};
    ~comp2(){};
    double targetVolume;
    double release()
    {
        std::cout<<"Releasing Different Stuff from RC: "<<this->reachCode<<std::endl;
        
        return District54::release();
    }
};
