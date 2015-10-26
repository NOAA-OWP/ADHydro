#ifndef DIVISION_6
#define DIVISION_6
#include "../../components/Region.h"

/*
    A regional sublcass implementing the virual release function
*/
class Div6 : Region
{
    private:
    //Put static data as variables here
    double targetVolume; //comment units

    public:
    Div6()
    {
        targetVolume = 5;
    }
    /*
        This is Div6's release function implementation
    */
    double release(/*any run time parameters listed here */)
    {
        int x = 3;
        int y = 3;    
        return x + y;
    }
    /*
        Div6 Destructor
    */
    ~Div6(){}
};

#endif
