#ifndef __TIME_POINT_STATE_H__
#define __TIME_POINT_STATE_H__

#include "checkpoint_manager_data_types.h"

// TimePointState contains the state to be written to file for a single time point.  If any array dimension is zero applicable array pointers are NULL.
// For parallel output, each TimePointState might only contain a slice of the total state with each processor getting a different slice.
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
    bool*  meshStateReceived;    // 1D array of size localNumberOfMeshElements.     True if that element's state has been received.
    bool*  channelStateReceived; // 1D array of size localNumberOfChannelElements.  True if that element's state has been received.
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    
    // Mesh state.
    EvapoTranspirationStateBlob* meshEvapoTranspirationState;        // 1D array of size localNumberOfMeshElements.
    double*                      meshSurfaceWater;                   // 1D array of size localNumberOfMeshElements.
    double*                      meshSurfaceWaterCreated;            // 1D array of size localNumberOfMeshElements.
    GroundwaterModeEnum*         meshGroundwaterMode;                // 1D array of size localNumberOfMeshElements.
    double*                      meshPerchedHead;                    // 1D array of size localNumberOfMeshElements.
    VadoseZoneStateBlob*         meshSoilWater;                      // 1D array of size localNumberOfMeshElements.
    double*                      meshSoilWaterCreated;               // 1D array of size localNumberOfMeshElements.
    double*                      meshAquiferHead;                    // 1D array of size localNumberOfMeshElements.
    VadoseZoneStateBlob*         meshAquiferWater;                   // 1D array of size localNumberOfMeshElements.
    double*                      meshAquiferWaterCreated;            // 1D array of size localNumberOfMeshElements.
    double*                      meshDeepGroundwater;                // 1D array of size localNumberOfMeshElements.
    
    // Mesh state for outputting point-process flows.
    double*                      meshPrecipitationRate;              // 1D array of size localNumberOfMeshElements.
    double*                      meshPrecipitationCumulative;        // 1D array of size localNumberOfMeshElements.
    double*                      meshEvaporationRate;                // 1D array of size localNumberOfMeshElements.
    double*                      meshEvaporationCumulative;          // 1D array of size localNumberOfMeshElements.
    double*                      meshTranspirationRate;              // 1D array of size localNumberOfMeshElements.
    double*                      meshTranspirationCumulative;        // 1D array of size localNumberOfMeshElements.
    
    // Mesh state for outputting values derived from EvapoTranspirationStateBlob and GroundwaterStateBlob that are nice to have visible outside those opaque blobs.
    double*                      meshCanopyWater;                    // 1D array of size localNumberOfMeshElements.
    double*                      meshSnowWater;                      // 1D array of size localNumberOfMeshElements.
    double*                      meshRootZoneWater;                  // 1D array of size localNumberOfMeshElements.
    double*                      meshTotalGroundwater;               // 1D array of size localNumberOfMeshElements.
    
    // Mesh neighbor state.
    NeighborEndpointEnum*        meshNeighborLocalEndpoint;          // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    NeighborEndpointEnum*        meshNeighborRemoteEndpoint;         // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    size_t*                      meshNeighborRemoteElementNumber;    // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshNeighborNominalFlowRate;        // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshNeighborExpirationTime;         // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshNeighborInflowCumulative;       // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshNeighborOutflowCumulative;      // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    
    // Channel state.
    EvapoTranspirationStateBlob* channelEvapoTranspirationState;     // 1D array of size localNumberOfChannelElements.
    double*                      channelSurfaceWater;                // 1D array of size localNumberOfChannelElements.
    double*                      channelSurfaceWaterCreated;         // 1D array of size localNumberOfChannelElements.
    
    // Channel state for outputting point-process flows.
    double*                      channelPrecipitationRate;           // 1D array of size localNumberOfChannelElements.
    double*                      channelPrecipitationCumulative;     // 1D array of size localNumberOfChannelElements.
    double*                      channelEvaporationRate;             // 1D array of size localNumberOfChannelElements.
    double*                      channelEvaporationCumulative;       // 1D array of size localNumberOfChannelElements.
    
    // Channel state for outputting values derived from EvapoTranspirationStateBlob that are nice to have visible outside that opaque blob.
    double*                      channelSnowWater;                   // 1D array of size localNumberOfChannelElements.
    
    // Channel neighbor state.
    NeighborEndpointEnum*        channelNeighborLocalEndpoint;       // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    NeighborEndpointEnum*        channelNeighborRemoteEndpoint;      // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    size_t*                      channelNeighborRemoteElementNumber; // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelNeighborNominalFlowRate;     // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelNeighborExpirationTime;      // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelNeighborInflowCumulative;    // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelNeighborOutflowCumulative;   // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
};

#endif // __TIME_POINT_STATE_H__
