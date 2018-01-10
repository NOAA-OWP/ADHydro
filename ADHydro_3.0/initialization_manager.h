#ifndef __INITIALIZATION_MANAGER_H__
#define __INITIALIZATION_MANAGER_H__

#include "initialization_manager.decl.h"

// InitializationManager is a Charm++ group that handles reading in initialization data from files and then sending that data to the appropriate Region objects.
// Reading is done in parallel.
class InitializationManager : public CBase_InitializationManager
{
    InitializationManager_SDAG_CODE
    
public:
    
    // Constructor.
    inline InitializationManager() :
    meshRegion(NULL),
    channelRegion(NULL)
    {
        initializeSimulation();
    }
    
    // Destructor.
    inline ~InitializationManager()
    {
        delete[] meshRegion;
        delete[] channelRegion;
    }
    
    int* meshRegion;
    int* channelRegion;
    
private:
    
    // Load initialization data from files and send it to simulation objects.
    void initializeSimulation();
};

#endif // __INITIALIZATION_MANAGER_H__
