//#include "Reservoir.h"
#include "main.decl.h"
#include "main.h"
#include "ReservoirFactory.h"
#include "DiversionFactory.h"
#include "Diversion.h"
#include "Reservoir.h"
#include <vector>
CProxy_Main mainProxy;

/*
    Need to decide exactly how the reservoir factory needs to be created.
    If created as a global from the main chare, then it needs to be PUPable.

    Depending on how mesh elements themselves are initialized, then maybe a new factory
    can get created per region (processing element) without having to distribute the factory.

    This is probably the best idea^^.
*/

// Entry point of Charm++ application
Main::Main(CkArgMsg* msg) 
{    
  // Print a message for the user
  CkPrintf("Main chare starting with %d\n",CkNumPes());
  doneCount = 0;
  doneCount1 = 0;
  mainProxy = thisProxy;
/*  
    //Create the ReservoirFactory used to create the indvidual reservoirs
    ReservoirFactory resFactory =ReservoirFactory();
    //give this element a reservoir object to manage
    Reservoir* reservoir1 = resFactory.create(14050001021460);
    Reservoir* reservoir2 = resFactory.create(14050001002419);
    
    std::vector<Reservoir*> v;
    v.push_back(reservoir1);

  //Test copy and assignment
    Reservoir* test = v[0];
    CkPrintf("Copying %ld ", reservoir1->reachCode);
    CkPrintf("using assignment. Now releasing %f from %ld\n", test->release(1,2,3), test->reachCode);
    delete v[0];
  CkExit();
*/
 /*
    //Create the DiversionFactory used to create the indvidual diversions
    DiversionFactory divFactory = DiversionFactory();
    Diversion* div = divFactory.create(123);
    
    std::vector<Diversion*> v;
    v.push_back(div);

  //Test copy and assignment
    Diversion* test = v[0];
    CkPrintf("Copying %ld ", div->elementID);
    CkPrintf("using assignment. Now releasing %f from %ld\n", test->irrigate(5), test->elementID);
    delete v[0];
  CkExit();
*/
  
  elementArray = CProxy_Element::ckNew(2);
  elementArray[0].process();
  elementArray[1].process();
  
}

// Constructor needed for chare object migration (ignore
// for now) NOTE: This constructor does not need to
// appear in the ".ci" file
Main::Main(CkMigrateMessage* msg) { }

void Main::done(int id)
{
    if(id == 0 && doneCount != 2)
    {
        doneCount++;
        elementArray[0].process();
    }
    else if(id == 1 && doneCount1 != 2)
    {
        doneCount1++;
        elementArray[1].process();
    }
    if(doneCount == 2 && doneCount1 == 2)
    {
        elementArray[0].ckDestroy();
        elementArray[1].ckDestroy();
    
     CkExit();
    }    
}
#include "main.def.h"
