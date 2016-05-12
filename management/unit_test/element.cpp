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
    diversion = divFactory->create(123, 2457388.500000);
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
        double expires;
        reservoir->release(100,70000000,2457492.00347, 55, rate, expires);
        CkPrintf("releasing %f until %f\n",rate, expires);
    }
    else CkPrintf("Error, null reservoir on char %d\n",thisIndex);
        
    if(diversion != NULL)
    {
        double referenceDate = 2457388.500000;
        double currentTime = 86400;
        double available = 100;
        std::vector<std::pair<int,double> > toSend;
        CkPrintf("Processing diversion %d on processor %d\n",diversion->elementID, CkMyPe());
        CkPrintf("Sending %f\n", available);
        diversion->divert(&available, referenceDate, currentTime, currentTime+60, toSend);
        CkPrintf("Diversion used %f\n", 100-available);
        for(int i = 0; i < toSend.size(); i++)
        {
            CkPrintf("Sending %f to %d\n", toSend[i].second, toSend[i].first);
        }
    }
    else CkPrintf("Error, null diversion on char %d\n", thisIndex);

    migrateMe((CkMyPe()+1)%CkNumPes());    

    mainProxy.done(thisIndex);
}

void Element::pup(PUP::er &p)
{
    p|reservoir;
    p|diversion;  
}

#include "element.def.h"
