#include "main.h"
#include <vector>

CProxy_Main mainProxy;

/*
   Test chare for OutputManager 
*/
CProxy_OutputManager Main::outputManager;
CProxy_Region	     Main::regionProxy;
CProxy_Main	     Main::mainProxy;

// Entry point of Charm++ application
Main::Main(CkArgMsg* msg):doneCount(0)
{    
  // Print a message for the user
  CkPrintf("Main chare starting with %d\n",CkNumPes());
  mainProxy = thisProxy;
/*  
*/
if(CkMyPe() == 0)
{
  Main::outputManager = CProxy_OutputManager::ckNew();
  done(CkMyPe());  
}
else{
 done(CkMyPe()); 
}

  //Main::regionProxy   = CProxy_Region::ckNew(2);
  //regionProxy[0].doSomething();
  //regionProxy[1].doSomething();
  
  CkExit();
}

// Constructor needed for chare object migration (ignore
// for now) NOTE: This constructor does not need to
// appear in the ".ci" file
Main::Main(CkMigrateMessage* msg) { }

void Main::done(int id)
{
    doneCount++;
    CkPrintf("Done Count: %d\n", doneCount);
    if(doneCount == 2)
    {
      CkExit();
    }

}

#include "main.def.h"