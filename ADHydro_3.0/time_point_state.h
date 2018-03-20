#ifndef __TIME_POINT_STATE_H__
#define __TIME_POINT_STATE_H__

#include "checkpoint_manager_data_types.h"

// A TimePointState contains the state to be written to file for a single time point.
// For parallel I/O, each TimePointState might only contain a slice of the total state with each processor getting a different slice.
class TimePointState
{
public:
    
    // Constructor.  All parameters directly initialize member variables.  Allocates arrays, sets elementsReceived to zero, and sets received flags to false.
    // Initializes all neighbors to NO_NEIGHBOR so elements only have to write neighbors that they have.
    TimePointState(size_t globalNumberOfMeshElements, size_t localNumberOfMeshElements, size_t localMeshElementStart, size_t maximumNumberOfMeshNeighbors,
                   size_t globalNumberOfChannelElements, size_t localNumberOfChannelElements, size_t localChannelElementStart, size_t maximumNumberOfChannelNeighbors);
    
    // Destructor.  Deletes arrays.
    ~TimePointState();
    
private:
    
    // Copy constructor unimplemented.  Should never be copy constructed.
    TimePointState(const TimePointState& other);
    
    // Assignment operator unimplemented.  Should never be assigned to.
    TimePointState& operator=(const TimePointState& other);
    
public:
    
    // Put received state into the correct location in TimePointState variables.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // state - The received state.
    bool receiveMeshState(const MeshState& state);
    
    // Put received state into the correct location in TimePointState variables.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // state - The received state.
    bool receiveChannelState(const ChannelState& state);
    
    // Dimension sizes.  These are stored with each TimePointState because mesh adaption may cause them to change over time.
    const size_t globalNumberOfMeshElements;
    const size_t localNumberOfMeshElements;
    const size_t localMeshElementStart;
    const size_t maximumNumberOfMeshNeighbors;
    const size_t globalNumberOfChannelElements;
    const size_t localNumberOfChannelElements;
    const size_t localChannelElementStart;
    const size_t maximumNumberOfChannelNeighbors;
    
    // For checking when all state is received.
    size_t elementsReceived; // Number of both mesh and channel elements received.
    
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    bool* meshStateReceived;    // 1D array of size localNumberOfMeshElements.     True if that element's state has been received.
    bool* channelStateReceived; // 1D array of size localNumberOfChannelElements.  True if that element's state has been received.
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    
    // The following are pointers to dynamically allocated arrays that will contain the state data.
    // Each variable is labeled as priority 1, 2, or 3.
    // Priority 1 variables are necessary for the simulation to run.
    // Priority 2 variables are used to calculate other variables, and/or have informational value.
    // Priority 3 variables are used to calculate other variables, are redundant with other state data, and have no additional informational value.
    // Pointers are all allocated, except if any array dimension is zero applicable array pointers are NULL.
    // Priority 2 and 3 variables can be set to zero if data is unavailable.
    
    // Mesh state.
    EvapoTranspirationStateBlob* meshEvapoTranspirationState;        // priority 1, 1D array of size localNumberOfMeshElements.
    double*                      meshSurfaceWater;                   // priority 1, 1D array of size localNumberOfMeshElements.
    double*                      meshSurfaceWaterCreated;            // priority 2, 1D array of size localNumberOfMeshElements.
    GroundwaterModeEnum*         meshGroundwaterMode;                // priority 1, 1D array of size localNumberOfMeshElements.
    double*                      meshPerchedHead;                    // priority 1, 1D array of size localNumberOfMeshElements.
    VadoseZoneStateBlob*         meshSoilWater;                      // priority 1, 1D array of size localNumberOfMeshElements.
    double*                      meshSoilWaterCreated;               // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshAquiferHead;                    // priority 1, 1D array of size localNumberOfMeshElements.
    VadoseZoneStateBlob*         meshAquiferWater;                   // priority 1, 1D array of size localNumberOfMeshElements.
    double*                      meshAquiferWaterCreated;            // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshDeepGroundwater;                // priority 1, 1D array of size localNumberOfMeshElements.
    
    // Mesh state for outputting point-process flows.
    double*                      meshPrecipitationRate;              // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshPrecipitationCumulative;        // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshEvaporationRate;                // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshEvaporationCumulative;          // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshTranspirationRate;              // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshTranspirationCumulative;        // priority 2, 1D array of size localNumberOfMeshElements.
    
    // Mesh state for outputting values derived from EvapoTranspirationStateBlob and VadoseZoneStateBlob that are nice to have visible outside those opaque blobs.
    double*                      meshCanopyWater;                    // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshSnowWater;                      // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshRootZoneWater;                  // priority 2, 1D array of size localNumberOfMeshElements.
    double*                      meshTotalGroundwater;               // priority 2, 1D array of size localNumberOfMeshElements.
    
    // Mesh neighbor state.
    NeighborEndpointEnum*        meshNeighborLocalEndpoint;          // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    NeighborEndpointEnum*        meshNeighborRemoteEndpoint;         // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    size_t*                      meshNeighborRemoteElementNumber;    // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshNeighborNominalFlowRate;        // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshNeighborExpirationTime;         // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshNeighborInflowCumulative;       // priority 2, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshNeighborOutflowCumulative;      // priority 2, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    
    // Channel state.
    EvapoTranspirationStateBlob* channelEvapoTranspirationState;     // priority 1, 1D array of size localNumberOfChannelElements.
    double*                      channelSurfaceWater;                // priority 1, 1D array of size localNumberOfChannelElements.
    double*                      channelSurfaceWaterCreated;         // priority 2, 1D array of size localNumberOfChannelElements.
    
    // Channel state for outputting point-process flows.
    double*                      channelPrecipitationRate;           // priority 2, 1D array of size localNumberOfChannelElements.
    double*                      channelPrecipitationCumulative;     // priority 2, 1D array of size localNumberOfChannelElements.
    double*                      channelEvaporationRate;             // priority 2, 1D array of size localNumberOfChannelElements.
    double*                      channelEvaporationCumulative;       // priority 2, 1D array of size localNumberOfChannelElements.
    
    // Channel state for outputting values derived from EvapoTranspirationStateBlob that are nice to have visible outside that opaque blob.
    double*                      channelSnowWater;                   // priority 2, 1D array of size localNumberOfChannelElements.
    
    // Channel neighbor state.
    NeighborEndpointEnum*        channelNeighborLocalEndpoint;       // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    NeighborEndpointEnum*        channelNeighborRemoteEndpoint;      // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    size_t*                      channelNeighborRemoteElementNumber; // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelNeighborNominalFlowRate;     // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelNeighborExpirationTime;      // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelNeighborInflowCumulative;    // priority 2, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelNeighborOutflowCumulative;   // priority 2, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
};

#endif // __TIME_POINT_STATE_H__
