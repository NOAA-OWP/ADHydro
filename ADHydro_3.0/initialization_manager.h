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
    //
    // Parameters:
    //
    // initializeFromASCIIFiles - If true, run in a special mode that just reads in ASCII files and outputs NetCDF files without running the simulation.  Otherwise, run in normal mode.
    inline InitializationManager(bool initializeFromASCIIFiles) : geometryData(NULL), parameterData(NULL), stateData(NULL)
    {
        if (initializeFromASCIIFiles)
        {
            initializeSimulationFromASCIIFiles();
        }
        else if (initializeSimulation())
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
    
    // Read ASCII .node and .z files to get node coordinates.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // nodeFilename        - The name of the .node file to read.
    // zFilename           - the name of the .z file to read.
    // globalNumberOfNodes - Scalar passed by reference will be filled in with the global number of nodes in the file.
    // localNumberOfNodes  - Scalar passed by reference will be filled in with the local number of nodes for this InitializationManager.
    // localNodeStart      - Scalar passed by reference will be filled in with the start node for this InitializationManager.
    // nodeX               - Pointer passed by reference will be set to point to a newly allocated array of size localNumberOfNodes containing the X coordinates of the nodes.
    // nodeY               - Pointer passed by reference will be set to point to a newly allocated array of size localNumberOfNodes containing the Y coordinates of the nodes.
    // nodeZ               - Pointer passed by reference will be set to point to a newly allocated array of size localNumberOfNodes containing the Z coordinates of the nodes.
    bool readNodeAndZFiles(const char* nodeFilename, const char* zFilename, size_t* globalNumberOfNodes, size_t* localNumberOfNodes, size_t* localNodeStart, double** nodeX, double** nodeY, double** nodeZ);
    
    // Read in ASCII files and output NetCDF files without running the simulation.
    //
    // This function never returns.  On error or success it exits the program after doing it's job.
    void initializeSimulationFromASCIIFiles();
    
    // Load initialization data from files and send it to simulation objects.
    //
    // Returns: true if there is an error, false otherwise.
    bool initializeSimulation();
};

#endif // __INITIALIZATION_MANAGER_H__
