#ifndef __ELEMENT_H__
#define __ELEMENT_H__
#include "Reservoir.h"
#include "Diversion.h"
class Element : public CBase_Element
{
private:
    Reservoir* reservoir;
    Diversion* diversion;
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

