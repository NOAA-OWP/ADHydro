#include "../components/IrrigatedLand.h"
#include <iostream>
/*
 * Test component class, extends the Reservoir class
 */
class il_comp1 : public IrrigatedLand
{
    public:

	/* ReachCode identifying the NHD waterbody corresponding to this reservoir */
	/*
	 * To simplify construction of ReservoirCreators, this is a static member that can be accessed
	 * without having to instansiate the object, this means that the parent Reservoir class can't
	 * hold this variable, since it has to be static per subclass
	 */
	static const long long id = 0;
    il_comp1():IrrigatedLand(this->id){};
    ~il_comp1(){};
    double call()
    {
        std::cout<<"Irrigated Land "<<id<<" calling for water."<<std::endl;
        return 10.0;
    }
};
