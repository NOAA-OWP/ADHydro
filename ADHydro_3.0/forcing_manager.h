#ifndef __FORCING_MANAGER_H__
#define __FORCING_MANAGER_H__

#include "readonly.h"
#include "all.h"
#include "forcing_manager.decl.h"

// ForcingManager is a Charm++ group that reads forcing data from file and sends it to the appropriate regions.
class ForcingManager : public CBase_ForcingManager
{
    ForcingManager_SDAG_CODE
    
public:
    
    // Constructor.
    inline ForcingManager() : nextForcingTime(Readonly::simulationStartTime)
    {
        // Tell myself I'm ready for the first forcing data.  After the first one, Regions will tell me when they are ready for the next one.
        thisProxy[CkMyPe()].readyForForcing(nextForcingTime);
        thisProxy[CkMyPe()].runUntilSimulationEnd();
    }
    
    // FIXME checkInvariant?
    
private:
    
    // Simulation time.
    double       nextForcingTime; // (s) The next simulation time when new forcing data needs to be read.
    const double simulationEndTime = Readonly::simulationStartTime + Readonly::simulationDuration;
                                  // This is partly for efficiency so we don't do the addition over and over and partly because Charm++ is having trouble parsing Readonly:: in the .ci file.
};

#endif // __FORCING_MANAGER_H__
