#ifndef IRR_LAND_CREATOR_H
#define IRR_LAND_CREATOR_H
#include "Creator.h"
#include "IrrigatedLandFactory.h"
#include <iostream>
#include <map>

//Template class for reservoir creators
//Each reservoir needs to have a ReservoirCreator instance
template <class T>
class IrrigatedLandCreator : public Parcel_Creator
{
    public:
	/*
	 * IrrigatedLandCreator constructor
	 * This constructor calls the IrrigatedLandFactory registerParcel function
	 * to automatically add itself to the factory's lookup table.
	 */
    IrrigatedLandCreator<T>(long long id, IrrigatedLandFactory* factory)
    {
    	factory->registerParcel(id, this);
    }
    /*
     * This function creates an appropriate Reservoir object based
     * on the template parameter
     */
    IrrigatedLand* create(long long id)
    {
        return new T;
    }

    /*
     * IrrigatedLandrCreator Destructor
     */
    virtual ~IrrigatedLandCreator<T>(){}
};
#endif
