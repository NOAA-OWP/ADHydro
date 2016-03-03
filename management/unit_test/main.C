//#include "Reservoir.h"
#include "main.decl.h"
#include "main.h"
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
    if(doneCount == 2 && doneCount1 == 2) CkExit();
    
}
#include "main.def.h"
