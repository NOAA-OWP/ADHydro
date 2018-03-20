#ifndef __MAP_PARAMETERS_H__
#define __MAP_PARAMETERS_H__

#include "neighbor_proxy.h"

// A MapParameters contains the parameter data of a map.
// For parallel I/O, each MapParameters might only contain a slice of the total data with each processor getting a different slice.
class MapParameters
{
public:
    
    // Constructor.  All parameters directly initialize member variables.  Sets all pointers to NULL.
    inline MapParameters(size_t globalNumberOfMeshElements, size_t localNumberOfMeshElements, size_t localMeshElementStart, size_t maximumNumberOfMeshNeighbors,
                         size_t globalNumberOfChannelElements, size_t localNumberOfChannelElements, size_t localChannelElementStart, size_t maximumNumberOfChannelNeighbors,
                         size_t globalNumberOfRegions, size_t localNumberOfRegions, size_t localRegionStart) :
                         globalNumberOfMeshElements(globalNumberOfMeshElements), localNumberOfMeshElements(localNumberOfMeshElements), localMeshElementStart(localMeshElementStart),
                         maximumNumberOfMeshNeighbors(maximumNumberOfMeshNeighbors), globalNumberOfChannelElements(globalNumberOfChannelElements), localNumberOfChannelElements(localNumberOfChannelElements),
                         localChannelElementStart(localChannelElementStart), maximumNumberOfChannelNeighbors(maximumNumberOfChannelNeighbors), globalNumberOfRegions(globalNumberOfRegions),
                         localNumberOfRegions(localNumberOfRegions), localRegionStart(localRegionStart), meshRegion(NULL), meshCatchment(NULL), meshVegetationType(NULL), meshGroundType(NULL),
                         meshManningsN(NULL), meshSoilExists(NULL), meshImpedanceConductivity(NULL), meshAquiferExists(NULL), meshDeepConductivity(NULL), meshNeighborLocalEndpoint(NULL),
                         meshNeighborRemoteEndpoint(NULL), meshNeighborRemoteElementNumber(NULL), meshNeighborRegion(NULL), channelRegion(NULL), channelChannelType(NULL), channelReachCode(NULL),
                         channelBaseWidth(NULL), channelSideSlope(NULL), channelManningsN(NULL), channelBedThickness(NULL), channelBedConductivity(NULL), channelNeighborLocalEndpoint(NULL),
                         channelNeighborRemoteEndpoint(NULL), channelNeighborRemoteElementNumber(NULL), channelNeighborRegion(NULL), regionNumberOfMeshElements(NULL), regionNumberOfChannelElements(NULL)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (!(localMeshElementStart + localNumberOfMeshElements <= globalNumberOfMeshElements))
            {
                CkError("ERROR in MapParameters::MapParameters: localMeshElementStart plus localNumberOfMeshElements must be less than or equal to globalNumberOfMeshElements.\n");
                CkExit();
            }
            
            if (!(localChannelElementStart + localNumberOfChannelElements <= globalNumberOfChannelElements))
            {
                CkError("ERROR in MapParameters::MapParameters: localChannelElementStart plus localNumberOfChannelElements must be less than or equal to globalNumberOfChannelElements.\n");
                CkExit();
            }
            
            if (!(localRegionStart + localNumberOfRegions <= globalNumberOfRegions))
            {
                CkError("ERROR in MapParameters::MapParameters: localRegionStart plus localNumberOfRegions must be less than or equal to globalNumberOfRegions.\n");
                CkExit();
            }
        }
    }
    
    // Destructor.  Deletes arrays.
    inline ~MapParameters()
    {
        delete[] meshRegion;
        delete[] meshCatchment;
        delete[] meshVegetationType;
        delete[] meshGroundType;
        delete[] meshManningsN;
        delete[] meshSoilExists;
        delete[] meshImpedanceConductivity;
        delete[] meshAquiferExists;
        delete[] meshDeepConductivity;
        delete[] meshNeighborLocalEndpoint;
        delete[] meshNeighborRemoteEndpoint;
        delete[] meshNeighborRemoteElementNumber;
        delete[] meshNeighborRegion;
        delete[] channelRegion;
        delete[] channelChannelType;
        delete[] channelReachCode;
        delete[] channelBaseWidth;
        delete[] channelSideSlope;
        delete[] channelManningsN;
        delete[] channelBedThickness;
        delete[] channelBedConductivity;
        delete[] channelNeighborLocalEndpoint;
        delete[] channelNeighborRemoteEndpoint;
        delete[] channelNeighborRemoteElementNumber;
        delete[] channelNeighborRegion;
        delete[] regionNumberOfMeshElements;
        delete[] regionNumberOfChannelElements;
    }
    
private:
    
    // Copy constructor unimplemented.  Should never be copy constructed.
    MapParameters(const MapParameters& other);
    
    // Assignment operator unimplemented.  Should never be assigned to.
    MapParameters& operator=(const MapParameters& other);
    
public:
    
    // Dimension sizes.  These are stored with each MapParameters because mesh adaption may cause them to change over time.
    const size_t globalNumberOfMeshElements;
    const size_t localNumberOfMeshElements;
    const size_t localMeshElementStart;
    const size_t maximumNumberOfMeshNeighbors;
    const size_t globalNumberOfChannelElements;
    const size_t localNumberOfChannelElements;
    const size_t localChannelElementStart;
    const size_t maximumNumberOfChannelNeighbors;
    const size_t globalNumberOfRegions;
    const size_t localNumberOfRegions;
    const size_t localRegionStart;
    
    // The following are pointers to dynamically allocated arrays that will contain the parameter data.
    // Each variable is labeled as priority 1, 2, or 3.
    // Priority 1 variables are necessary for the simulation to run.
    // Priority 2 variables are used to calculate other variables, and/or have informational value.
    // Priority 3 variables are used to calculate other variables, are redundant with other state data, and have no additional informational value.
    // Regardless of priority, any pointer can be NULL indicating that data is unavailable.
    // The user of the data must check that the pointer is not NULL before accessing the array and throw an error if necessary data is unavailable.
    
    // Mesh parameters.
    size_t*               meshRegion;                         // priority 1, 1D array of size localNumberOfMeshElements.
    size_t*               meshCatchment;                      // priority 2, 1D array of size localNumberOfMeshElements.
    int*                  meshVegetationType;                 // priority 1, 1D array of size localNumberOfMeshElements.
    int*                  meshGroundType;                     // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshManningsN;                      // priority 1, 1D array of size localNumberOfMeshElements.
    bool*                 meshSoilExists;                     // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshImpedanceConductivity;          // priority 1, 1D array of size localNumberOfMeshElements.
    bool*                 meshAquiferExists;                  // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshDeepConductivity;               // priority 1, 1D array of size localNumberOfMeshElements.
    
    // Mesh neighbor parameters.
    NeighborEndpointEnum* meshNeighborLocalEndpoint;          // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    NeighborEndpointEnum* meshNeighborRemoteEndpoint;         // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    size_t*               meshNeighborRemoteElementNumber;    // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    size_t*               meshNeighborRegion;                 // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    
    // Channel parameters.
    size_t*               channelRegion;                      // priority 1, 1D array of size localNumberOfChannelElements.
    ChannelTypeEnum*      channelChannelType;                 // priority 1, 1D array of size localNumberOfChannelElements.
    long long*            channelReachCode;                   // priority 2, 1D array of size localNumberOfChannelElements.
    double*               channelBaseWidth;                   // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelSideSlope;                   // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelManningsN;                   // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelBedThickness;                // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelBedConductivity;             // priority 1, 1D array of size localNumberOfChannelElements.
    
    // Channel neighbor parameters.
    NeighborEndpointEnum* channelNeighborLocalEndpoint;       // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    NeighborEndpointEnum* channelNeighborRemoteEndpoint;      // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    size_t*               channelNeighborRemoteElementNumber; // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    size_t*               channelNeighborRegion;              // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    
    // Region parameters.
    size_t*               regionNumberOfMeshElements;         // priority 1, 1D array of size localNumberOfRegions.
    size_t*               regionNumberOfChannelElements;      // priority 1, 1D array of size localNumberOfRegions.
};

#endif // __MAP_PARAMETERS_H__
