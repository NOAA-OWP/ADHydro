#ifndef __INITIALIZATION_MANAGER_H__
#define __INITIALIZATION_MANAGER_H__

#include "map_geometry.h"
#include "map_parameters.h"
#include "time_point_state.h"
#include "initialization_manager.decl.h"

// InitializationManager is a Charm++ group that handles reading in initialization data from files and then sending that data to the appropriate Region objects.
// Reading is done in parallel.
class InitializationManager : public CBase_InitializationManager
{
    InitializationManager_SDAG_CODE
    
public:
    
    // Constructor.
    inline InitializationManager() : geometryData(NULL), parameterData(NULL), stateData(NULL)
    {
        if (initializeSimulation())
        {
            CkExit();
        }
    }
    
    // Destructor.
    inline ~InitializationManager()
    {
        delete geometryData;
        delete parameterData;
        delete stateData;
    }
    
    // For efficient operation, parallel I/O must be done on contiguous arrays.
    // These objects provide an in-memory cache of values read as contiguous arrays indexed by element number.
    // This data will then be reshuffled into per-element objects.
    MapGeometry*    geometryData;
    MapParameters*  parameterData;
    TimePointState* stateData;
    
private:
    
    // Load initialization data from files and send it to simulation objects.
    //
    // Returns: true if there is an error, false otherwise.
    bool initializeSimulation();
};

#endif // __INITIALIZATION_MANAGER_H__
