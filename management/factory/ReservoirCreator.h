#ifndef RES_CREATOR_H
#define RES_CREATOR_H
#include "Creator.h"
#include "ReservoirFactory.h"
#include <iostream>
#include <map>

//Template class for reservoir creators
//Each reservoir needs to have a ReservoirCreator instance
template <class T>
class ReservoirCreator : public Res_Creator
{
    public:
	/*
	 * ReservoirCreator constructor
	 * This constructor calls the ReservoirFactory registerReservoir function
	 * to automatically add itself to the factory's lookup table.
	 */
    ReservoirCreator<T>(long long reach, ReservoirFactory* factory)
    {
    	factory->registerReservoir(reach, this);
    }
    /*
     * This function creates an appropriate Reservoir object based
     * on the template parameter
     */
    Reservoir* create(long long reach)
    {
        return new T;
    }
    /*
     * ReservoirCreator Destructor
     */
    virtual ~ReservoirCreator<T>(){}
};
#endif
