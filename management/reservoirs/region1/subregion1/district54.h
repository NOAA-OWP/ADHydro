#ifndef DISTRICT_54
#define DISTRICT_54
#include "../div6.h"
/*
    This is a sub region class example
    District 54 is a subclass of Div6
*/
class District54 : public Div6
{
    private:
    /*
        Any private data relative to this district
        can be declared here and initialized via the
        constructor below.
    */

    public:
    /*
        District level constructor
    */
    District54(){}
    /*
        District level destructor
    */
    ~District54(){}

    /*
        District54's release function implementation
    */
    double release()
    { 
        /*
            A district can define its own release function
            or it can defer to the regional level it subclasses
        */

        //Defer to Div6 Regional release function
        return Div6::release(); 
        /*or return districtRelease*/
        double districtRelease = 54;
        return 54;    
    }
};

#endif
