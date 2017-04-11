#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

#include "initialization_manager.h"
#include "checkpoint_manager.h"
#include "adhydro.decl.h"

// An ADHydro object is the main chare of the program.  Execution starts in its constructor.
class ADHydro : public CBase_ADHydro
{
    ADHydro_SDAG_CODE
    
public:
    
    // Constructor.  This is the mainchare constructor where the program starts.
    //
    // Parameters:
    //
    // msg - Charm++ command line arguments message.
    ADHydro(CkArgMsg* msg);
    
    // Global readonly variables.
    static CProxy_InitializationManager initializationManagerProxy; // Charm++ proxy to the chare group for the initialization managers.
    static CProxy_CheckpointManager     checkpointManagerProxy;     // Charm++ proxy to the chare group for the checkpoint managers.
    static CProxy_Region                regionProxy;                // Charm++ proxy to the chare array for the simulation regions.
};

#endif // __ADHYDRO_H__
