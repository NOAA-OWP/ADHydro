#ifndef WATER_REGION
#define WATER_REGION

/*
    Region parent class used to hold data at a regional scope
*/
class Region
{
    private:
    /*
        Any private data that is common to all Regions can be declared here
        If there is private data declared, an appropriate constructor should
        also be implemented.
    */

    public:
    /*
        Virtual functions that subclasses should implement
    */
    virtual double release() = 0;
    virtual ~Region(){}
};

#endif
