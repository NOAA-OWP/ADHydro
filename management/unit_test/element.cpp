#include "element.decl.h"

#include "element.h"
#include "main.decl.h"
#include "../generated_code/charm_factory/ReservoirFactory.h"
extern CProxy_Main mainProxy;
Element::Element()
{
//Define some reachCodes for testing
	int numRes = 2;
	long long codes[numRes];
	codes[0] = -1;
	codes[1] = 14050001002504;
	
	//Create the ReservoirFactory used to create the indvidual reservoirs
    ReservoirFactory* resFactory = new ReservoirFactory();
    //give this element a reservoir object to manage
    reservoir = resFactory->create(codes[thisIndex]);

    //We don't need the factory anymore since the reservoirs are all created
    delete resFactory;
}

Element::Element(CkMigrateMessage* msg)
{

}

Element::~Element()
{
    delete reservoir;
}
void Element::process()
{
 
    if(reservoir != NULL)
    {
        CkPrintf("char %d is processing %ld on processor %d\n",thisIndex,reservoir->reachCode, CkMyPe());
        CkPrintf("releasing %f\n",reservoir->release(1,2,3));
    }
    else CkPrintf("Error, null reservoir on char %d\n",thisIndex);
    migrateMe((CkMyPe()+1)%CkNumPes());    

    mainProxy.done(thisIndex);
}

void Element::pup(PUP::er &p)
{
    if(p.isUnpacking()) CkPrintf("Unpacking ELEMENT %d\n", thisIndex);
    else CkPrintf("Packing ELEMENT %d\n",thisIndex);
    p|reservoir;    
}

#include "element.def.h"
