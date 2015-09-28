#ifndef CREATOR_H
#define CREATOR_H
#include "../components/Reservoir.h"
#include "../components/IrrigatedLand.h"
//Abstract Creator classes for various components

//Reservoir Creator
class Res_Creator
{
    public:
    virtual Reservoir* create(long long reach) = 0;

    virtual ~Res_Creator(){}
};

//IrrigatedLand Creator
class Parcel_Creator
{
    public:
	virtual IrrigatedLand* create(long long id) = 0;
    virtual ~Parcel_Creator(){}
};


#endif
