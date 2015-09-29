#include "../components/IrrigatedLand.h"
#include <iostream>
class il_comp2 : public IrrigatedLand
{
public:

	/* ReachCode identifying the NHD waterbody corresponding to this reservoir */
	/*
	 * To simplify construction of ReservoirCreators, this is a static member that can be accessed
	 * without having to instansiate the object.  The parent also keeps the reach code which is
	 * initialized when the child is constructed so the reach code can be accessed via a generic
	 * Reservoir*
	 */
	static const long long id = 1;
    il_comp2():IrrigatedLand(this->id){};
    ~il_comp2(){};
    double call()
    {
        std::cout<<"Irrigated Land "<<id<<" calling for water"<<std::endl;
        return 21.1;
    }
};
