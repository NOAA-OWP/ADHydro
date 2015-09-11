#ifndef CREATOR_H
#define CREATOR_H
#include "../components/Reservoir.h"
//Abstract reservoir Creator class 
class Creator
{
    public:
    virtual Reservoir* create(long long reach) = 0;
    virtual ~Creator(){}
};
#endif
