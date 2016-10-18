#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "all.h"
#include <string>

// FileManager is a wrapper for file operations used by OutputManager.
// FileManager must be subclassed for each type of file that can be created.
// Currently, only NetCDF files are implemented, but this could be extended.
class FileManager
{
public:

  // The state of the evapotranspiration and vadose zone infiltration simulations are generally complex data structures.
  // We want ADHydro to support using third party modules for those simulations so we don't necessarily have control over the data structure,
  // and the data structure can change if you change to a different module, but we need to save that state in the input/output files as a fixed size blob.
  // These declarations provide fixed size blobs for that data.  Any new simulation module must shoehorn their state into a fixed size blob and add it as an additional union option.
  union EvapoTranspirationStateBlob
  {
    int dummy;
    // FIXME EvapoTranspirationStateStruct NoahMPStateBlob;
  };

  union VadoseZoneStateBlob
  {
    int dummy;
    // FIXME GARTOStateBlob
  };

  // TimePointState contains the state to be written to file for a single time point.  If any array dimension is zero applicable array pointers are NULL.
  // For parallel output, each TimePointState might only contain a slice of the total state with each processor getting a different TimePointState.
  class TimePointState
  {
  public:

    // Constructor.  Allocates arrays, sets elementsReceived to zero, and sets received flags to false.
    //
    // All parameters directly initialize member variables.
    TimePointState(const std::string& directoryInit, double referenceDateInit, double outputTimeInit, size_t globalNumberOfMeshElementsInit, size_t localNumberOfMeshElementsInit,
        size_t localMeshElementStartInit, size_t maximumNumberOfMeshSoilLayersInit,  size_t maximumNumberOfMeshNeighborsInit, size_t globalNumberOfChannelElementsInit,
        size_t localNumberOfChannelElementsInit, size_t localChannelElementStartInit, size_t maximumNumberOfChannelNeighborsInit);

    // Destructor.  Deletes arrays.
    ~TimePointState();

  private:

    // Copy constructor.  Should never be copy constructed.
    TimePointState(const TimePointState& other);

    // Assignment operator.  Should never be assigned to.
    TimePointState& operator=(const TimePointState& other);

  public:

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    // Sets received flags to false.  Received flags are only used in debug mode for checking that the same element isn't received twice.
    void clearReceivedFlags();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

    // Returns: true if the state from all elements have been received.
    bool allStateReceived() { return (localNumberOfMeshElements + localNumberOfChannelElements == elementsReceived); }

    // Helper function to create a standard filename string for the TimePointState.
    //
    // Returns: the filename.
    std::string createFilename() const;

    // Values that are used to create filenames and/or stored in the files.
    const std::string directory;     // The directory in which to create output files.  Filenames will be generated from the date and time.
    const double      referenceDate; // Julian date.  Must be on or after 1 CE (1721425.5).
    double            outputTime;    // Time point to write output for in seconds after referenceDate.  Can be positive, negative, or zero, but the corresponding calendar date must be on or after 1 CE.

    // Dimensions of the entire state and this slice.
    const size_t globalNumberOfMeshElements;
    const size_t localNumberOfMeshElements;    // Must be less than or equal to globalNumberOfMeshElements.
    const size_t localMeshElementStart;        // Must be less than globalNumberOfMeshElements or zero if localNumberOfMeshElements is zero.
    const size_t maximumNumberOfMeshSoilLayers;
    const size_t maximumNumberOfMeshNeighbors;
    const size_t globalNumberOfChannelElements;
    const size_t localNumberOfChannelElements; // Must be less than or equal to globalNumberOfChannelElements.
    const size_t localChannelElementStart;     // Must be less than globalNumberOfChannelElements or zero if localNumberOfChannelElements is zero.
    const size_t maximumNumberOfChannelNeighbors;

    // For checking when all state is received.
    size_t elementsReceived;     // Number of both mesh and channel elements received.

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    bool*  meshStateReceived;    // 1D array of size localNumberOfMeshElements.     True if that element's state has been received.
    bool*  channelStateReceived; // 1D array of size localNumberOfChannelElements.  True if that element's state has been received.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

    // The state.
    double*                      meshSurfacewaterDepth;                      // 1D array of size localNumberOfMeshElements.
    double*                      meshSurfacewaterCreated;                    // 1D array of size localNumberOfMeshElements.
    double*                      meshGroundwaterHead;                        // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
    double*                      meshGroundwaterRecharge;                    // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
    double*                      meshGroundwaterCreated;                     // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
    double*                      meshPrecipitationRate;                      // 1D array of size localNumberOfMeshElements.
    double*                      meshPrecipitationCumulative;                // 1D array of size localNumberOfMeshElements.
    double*                      meshEvaporationRate;                        // 1D array of size localNumberOfMeshElements.
    double*                      meshEvaporationCumulative;                  // 1D array of size localNumberOfMeshElements.
    double*                      meshTranspirationRate;                      // 1D array of size localNumberOfMeshElements.
    double*                      meshTranspirationCumulative;                // 1D array of size localNumberOfMeshElements.
    EvapoTranspirationStateBlob* meshEvapoTranspirationState;                // 1D array of size localNumberOfMeshElements.
    double*                      meshCanopyWaterEquivalent;                  // 1D array of size localNumberOfMeshElements.
    double*                      meshSnowWaterEquivalent;                    // 1D array of size localNumberOfMeshElements.
    VadoseZoneStateBlob*         meshVadoseZoneState;                        // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
    double*                      meshRootZoneWater;                          // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
    double*                      meshTotalSoilWater;                         // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
    double*                      meshSurfacewaterNeighborsExpirationTime;    // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshSurfacewaterNeighborsFlowRate;          // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshSurfacewaterNeighborsFlowCumulative;    // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
    double*                      meshGroundwaterNeighborsExpirationTime;     // 3D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors.
    double*                      meshGroundwaterNeighborsFlowRate;           // 3D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors.
    double*                      meshGroundwaterNeighborsFlowCumulative;     // 3D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors.
    double*                      channelSurfacewaterDepth;                   // 1D array of size localNumberOfChannelElements.
    double*                      channelSurfacewaterCreated;                 // 1D array of size localNumberOfChannelElements.
    double*                      channelPrecipitationRate;                   // 1D array of size localNumberOfChannelElements.
    double*                      channelPrecipitationCumulative;             // 1D array of size localNumberOfChannelElements.
    double*                      channelEvaporationRate;                     // 1D array of size localNumberOfChannelElements.
    double*                      channelEvaporationCumulative;               // 1D array of size localNumberOfChannelElements.
    EvapoTranspirationStateBlob* channelEvapoTranspirationState;             // 1D array of size localNumberOfChannelElements.
    double*                      channelSnowWaterEquivalent;                 // 1D array of size localNumberOfChannelElements.
    double*                      channelSurfacewaterNeighborsExpirationTime; // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelSurfacewaterNeighborsFlowRate;       // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelSurfacewaterNeighborsFlowCumulative; // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelGroundwaterNeighborsExpirationTime;  // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelGroundwaterNeighborsFlowRate;        // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
    double*                      channelGroundwaterNeighborsFlowCumulative;  // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
  };

  // Pure virtual interface to file operations.

  // Write data out to a file.  Each FileManager may only write a slice of data, may write in parallel or serially, etc.  The subclass must figure all this out.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // timePointState - Output data to write.
  virtual bool writeOutput(const TimePointState& timePointState) = 0;
};

#endif // __FILE_MANAGER_H__
