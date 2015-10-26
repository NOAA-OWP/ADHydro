#include "../components/Reservoir.h"
#include <iostream>
/*
 * Test component class, extends the Reservoir class
 */
class comp1 : public Reservoir
{
    public:

	/* ReachCode identifying the NHD waterbody corresponding to this reservoir */
	/*
	 * To simplify construction of ReservoirCreators, this is a static member that can be accessed
	 * without having to instansiate the object, this means that the parent Reservoir class can't
	 * hold this variable, since it has to be static per subclass
	 */
	static const long long reachCode = 14050001002411;
    comp1():Reservoir(this->reachCode){};
    ~comp1(){};
    double release()
    {
        std::cout<<"Releasing Stuff from RC: "<<this->reachCode<<std::endl;
        return 0.0;
    }
};
