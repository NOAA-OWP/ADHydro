#include "element.decl.h"

#include "element.h"
#include "main.decl.h"
#include "ReservoirFactory.h"
#include "DiversionFactory.h"
extern CProxy_Main mainProxy;
Element::Element()
{
//Define some reachCodes for testing
	int numRes = 2;
	long long codes[numRes];
	codes[0] = 14050001021460;
	codes[1] = 14050001002419;
	
	//Create the ReservoirFactory used to create the indvidual reservoirs
    ReservoirFactory* resFactory = new ReservoirFactory();
    //give this element a reservoir object to manage
    reservoir = resFactory->create(codes[thisIndex]);

    
    //We don't need the factory anymore since the reservoirs are all created
    delete resFactory;

    //Create a Diversion to test
    DiversionFactory* divFactory = new DiversionFactory();
    diversion = divFactory->create(123);
    delete divFactory;
    
}

Element::Element(CkMigrateMessage* msg)
{

}

Element::~Element()
{
    delete reservoir;
    delete diversion;
}
void Element::process()
{
    CkPrintf("char %d is processing on processor %d\n",thisIndex, CkMyPe());
    if(reservoir != NULL)
    {
        CkPrintf("Processing reservoir %ld on processor %d\n",reservoir->reachCode, CkMyPe());
        CkPrintf("releasing %f\n",reservoir->release(1,2,3));
    }
    else CkPrintf("Error, null reservoir on char %d\n",thisIndex);
        
    if(diversion != NULL)
    {
        double send = 20;
        CkPrintf("Processing diversion %d on processor %d\n",diversion->elementID, CkMyPe());
        double request = diversion->getRequestedDiversionAmount();
        CkPrintf("Diversion wants %f, giving %f\n", request, send);
        double notUsed = diversion->irrigate(request);
        CkPrintf("Diversion used %f\n", send-notUsed);
    }
    else CkPrintf("Error, null diversion on char %d\n", thisIndex);

    migrateMe((CkMyPe()+1)%CkNumPes());    

    mainProxy.done(thisIndex);
}

void Element::pup(PUP::er &p)
{
    if(p.isUnpacking()) CkPrintf("Unpacking ELEMENT %d\n", thisIndex);
    else CkPrintf("Packing ELEMENT %d\n",thisIndex);
    p|reservoir;
    p|diversion;  
}

#include "element.def.h"
