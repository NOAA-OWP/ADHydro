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
        double rate;
        long duration;
        reservoir->release(100,70000000,2457492.00347, 55, rate, duration);
        CkPrintf("releasing %f for %d\n",rate, duration);
    }
    else CkPrintf("Error, null reservoir on char %d\n",thisIndex);
        
    if(diversion != NULL)
    {
        double send = 100;
        CkPrintf("Processing diversion %d on processor %d\n",diversion->elementID, CkMyPe());
        CkPrintf("Sending %f\n", send);
        double notUsed = 0;
        diversion->divert(send, notUsed);
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
