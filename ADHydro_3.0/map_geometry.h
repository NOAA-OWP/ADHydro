#ifndef __MAP_GEOMETRY_H__
#define __MAP_GEOMETRY_H__

#include "neighbor_proxy.h"

// A MapGeometry contains the geometry data of a map.
// For parallel I/O, each MapGeometry might only contain a slice of the total data with each processor getting a different slice.
class MapGeometry
{
public:
    
    // Constructor.  All parameters directly initialize member variables.  Sets all pointers to NULL.
    inline MapGeometry(size_t globalNumberOfMeshElements, size_t localNumberOfMeshElements, size_t localMeshElementStart, size_t maximumNumberOfMeshNeighbors,
                       size_t globalNumberOfChannelElements, size_t localNumberOfChannelElements, size_t localChannelElementStart, size_t maximumNumberOfChannelNeighbors):
                       globalNumberOfMeshElements(globalNumberOfMeshElements), localNumberOfMeshElements(localNumberOfMeshElements), localMeshElementStart(localMeshElementStart),
                       maximumNumberOfMeshNeighbors(maximumNumberOfMeshNeighbors), globalNumberOfChannelElements(globalNumberOfChannelElements), localNumberOfChannelElements(localNumberOfChannelElements),
                       localChannelElementStart(localChannelElementStart), maximumNumberOfChannelNeighbors(maximumNumberOfChannelNeighbors), meshElementX(NULL), meshElementY(NULL), meshElementZ(NULL),
                       meshElementArea(NULL), meshLatitude(NULL), meshLongitude(NULL), meshSlopeX(NULL), meshSlopeY(NULL), meshNeighborLocalEndpoint(NULL), meshNeighborRemoteEndpoint(NULL),
                       meshNeighborRemoteElementNumber(NULL), meshNeighborEdgeLength(NULL), meshNeighborEdgeNormalX(NULL), meshNeighborEdgeNormalY(NULL), meshNeighborZOffset(NULL), channelElementX(NULL),
                       channelElementY(NULL), channelElementZBank(NULL), channelElementZBed(NULL), channelElementLength(NULL), channelLatitude(NULL), channelLongitude(NULL),
                       channelNeighborLocalEndpoint(NULL), channelNeighborRemoteEndpoint(NULL), channelNeighborRemoteElementNumber(NULL), channelNeighborEdgeLength(NULL),
                       channelNeighborEdgeNormalX(NULL), channelNeighborEdgeNormalY(NULL), channelNeighborZOffset(NULL)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (!(localMeshElementStart + localNumberOfMeshElements <= globalNumberOfMeshElements))
            {
                CkError("ERROR in MapGeometry::MapGeometry: localMeshElementStart plus localNumberOfMeshElements must be less than or equal to globalNumberOfMeshElements.\n");
                CkExit();
            }
            
            if (!(localChannelElementStart + localNumberOfChannelElements <= globalNumberOfChannelElements))
            {
                CkError("ERROR in MapGeometry::MapGeometry: localChannelElementStart plus localNumberOfChannelElements must be less than or equal to globalNumberOfChannelElements.\n");
                CkExit();
            }
        }
    }
                
    // Destructor.  Deletes arrays.
    inline ~MapGeometry()
    {
        delete[] meshElementX;
        delete[] meshElementY;
        delete[] meshElementZ;
        delete[] meshElementArea;
        delete[] meshLatitude;
        delete[] meshLongitude;
        delete[] meshSlopeX;
        delete[] meshSlopeY;
        delete[] meshNeighborLocalEndpoint;
        delete[] meshNeighborRemoteEndpoint;
        delete[] meshNeighborRemoteElementNumber;
        delete[] meshNeighborEdgeLength;
        delete[] meshNeighborEdgeNormalX;
        delete[] meshNeighborEdgeNormalY;
        delete[] meshNeighborZOffset;
        delete[] channelElementX;
        delete[] channelElementY;
        delete[] channelElementZBank;
        delete[] channelElementZBed;
        delete[] channelElementLength;
        delete[] channelLatitude;
        delete[] channelLongitude;
        delete[] channelNeighborLocalEndpoint;
        delete[] channelNeighborRemoteEndpoint;
        delete[] channelNeighborRemoteElementNumber;
        delete[] channelNeighborEdgeLength;
        delete[] channelNeighborEdgeNormalX;
        delete[] channelNeighborEdgeNormalY;
        delete[] channelNeighborZOffset;
    }
    
private:
    
    // Copy constructor unimplemented.  Should never be copy constructed.
    MapGeometry(const MapGeometry& other);
    
    // Assignment operator unimplemented.  Should never be assigned to.
    MapGeometry& operator=(const MapGeometry& other);
    
public:
    
    // Dimension sizes.  These are stored with each MapGeometry because mesh adaption may cause them to change over time.
    const size_t globalNumberOfMeshElements;
    const size_t localNumberOfMeshElements;
    const size_t localMeshElementStart;
    const size_t maximumNumberOfMeshNeighbors;
    const size_t globalNumberOfChannelElements;
    const size_t localNumberOfChannelElements;
    const size_t localChannelElementStart;
    const size_t maximumNumberOfChannelNeighbors;
    
    // The following are pointers to dynamically allocated arrays that will contain the geometry data.
    // Each variable is labeled as priority 1, 2, or 3.
    // Priority 1 variables are necessary for the simulation to run.
    // Priority 2 variables are used to calculate other variables, and/or have informational value.
    // Priority 3 variables are used to calculate other variables, are redundant with other state data, and have no additional informational value.
    // Regardless of priority, any pointer can be NULL indicating that data is unavailable.
    // The user of the data must check that the pointer is not NULL before accessing the array and throw an error if necessary data is unavailable.
    
    // Mesh geometry.
    double*               meshElementX;                      // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshElementY;                      // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshElementZ;                      // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshElementArea;                   // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshLatitude;                      // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshLongitude;                     // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshSlopeX;                        // priority 1, 1D array of size localNumberOfMeshElements.
    double*               meshSlopeY;                        // priority 1, 1D array of size localNumberOfMeshElements.
    
    // Mesh neighbor geometry.
    NeighborEndpointEnum* meshNeighborLocalEndpoint;         // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    NeighborEndpointEnum* meshNeighborRemoteEndpoint;        // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    size_t*               meshNeighborRemoteElementNumber;   // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*               meshNeighborEdgeLength;            // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*               meshNeighborEdgeNormalX;           // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*               meshNeighborEdgeNormalY;           // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*               meshNeighborZOffset;               // priority 1, 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    
    // Channel geometry.
    double*               channelElementX;                    // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelElementY;                    // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelElementZBank;                // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelElementZBed;                 // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelElementLength;               // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelLatitude;                    // priority 1, 1D array of size localNumberOfChannelElements.
    double*               channelLongitude;                   // priority 1, 1D array of size localNumberOfChannelElements.
    
    // Channel neighbor geometry.
    NeighborEndpointEnum* channelNeighborLocalEndpoint;       // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    NeighborEndpointEnum* channelNeighborRemoteEndpoint;      // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    size_t*               channelNeighborRemoteElementNumber; // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*               channelNeighborEdgeLength;          // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*               channelNeighborEdgeNormalX;         // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*               channelNeighborEdgeNormalY;         // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*               channelNeighborZOffset;             // priority 1, 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
};

#endif // __MAP_GEOMETRY_H__
