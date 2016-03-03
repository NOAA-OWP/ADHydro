#ifndef __ELEMENT_H__
#define __ELEMENT_H__
#include "../generated_code/charm_reservoirs/Reservoir.h"
class Element : public CBase_Element
{
private:
    Reservoir* reservoir;
public:
    Element();
    Element(CkArgMsg* msg);
    Element(CkMigrateMessage* msg);
    ~Element();
    virtual void pup(PUP::er &p);
    //Entry Method
    void process();
};

#endif //__Element_H__

