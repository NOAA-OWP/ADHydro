#ifndef __INITIALIZATION_MANAGER_H__
#define __INITIALIZATION_MANAGER_H__

#include "forcing_manager.h"
#include "region.h"
#include "initialization_manager.decl.h"

// InitializationManager is a Charm++ group that handles reading in initialization data from files and then sending that data to the appropriate Region object.
// Reading is done in parallel.
class InitializationManager : public CBase_InitializationManager
{
    InitializationManager_SDAG_CODE
    
public:
    
    // Constructor.
    inline InitializationManager()
    {
        thisProxy[CkMyPe()].runUntilSimulationEnd();
    }
};

#endif // __INITIALIZATION_MANAGER_H__
