#include "../components/Reservoir.h"
#include <iostream>
class comp2 : public Reservoir
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
    comp2():Reservoir(this->reachCode){};
    ~comp2(){};
    double release()
    {
        std::cout<<"Releasing Different Stuff from RC: "<<this->reachCode<<std::endl;
        return 1.1;
    }
};
