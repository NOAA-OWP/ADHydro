#ifndef __OUTPUT_MANAGER_H__
#define __OUTPUT_MANAGER_H__

#include "file_manager.h"
#include <string>
#include <map>

// TimePointState really should be a nested class of OutputManager, but there's a circular dependency with file_manager.h, and C++ cannot forward declare nested classes.
class TimePointState;

// OutputManager contains the generic code for creating ADHydro output files.
// It outputs a separate file for each time point.
// The purpose of this is to make the early time points available for use as soon as possible.
// What type of files are created is wrapped by the FileManager class.
// OutputManager is generic with respect to what communication system is used.
// To support this it has a pure virtual interface for interaction with the communication system.
// Each communication system must subclass OutputManager with a system specific implementation.
// The most likely two candidates are Charm++ or MPI.
// There should be one OutputManager on each processor, i.e. a group in Charm++, or one per rank in MPI.
class OutputManager
{
public:

  // MeshElementState and ChannelElementState are for individual elements to send their state to an OutputManager.
  // numberOfSoilLayers and/or numberOfNeighbors can be zero in which case applicable array pointers are NULL.
  class MeshElementState
  {
  public:

    size_t  elementNumber;                             // The element that this data is for.
    double  currentTime;                               // The time point that this data is for.
    size_t  numberOfSoilLayers;                        // The size of the soil layer and neighbor arrays can be less than
    size_t  numberOfNeighbors;                         // the maximum number allowed if the element has less.
    double  surfacewaterDepth;
    double  surfacewaterCreated;
    double* groundwaterHead;                           // 1D array of size numberOfSoilLayers.
    double* groundwaterRecharge;                       // 1D array of size numberOfSoilLayers.
    double* groundwaterCreated;                        // 1D array of size numberOfSoilLayers.
    double  precipitationRate;
    double  precipitationCumulative;
    double  evaporationRate;
    double  evaporationCumulative;
    double  transpirationRate;
    double  transpirationCumulative;
    FileManager::EvapoTranspirationStateBlob evapoTranspirationState;
    double  canopyWaterEquivalent;
    double  snowWaterEquivalent;
    FileManager::VadoseZoneStateBlob* vadoseZoneState; // 1D array of size numberOfSoilLayers.
    double* rootZoneWater;                             // 1D array of size numberOfSoilLayers.
    double* totalSoilWater;                            // 1D array of size numberOfSoilLayers.
    double* surfacewaterNeighborsExpirationTime;       // 1D array of size numberOfNeighbors.
    double* surfacewaterNeighborsFlowRate;             // 1D array of size numberOfNeighbors.
    double* surfacewaterNeighborsFlowCumulative;       // 1D array of size numberOfNeighbors.
    double* groundwaterNeighborsExpirationTime;        // 2D array of size numberOfSoilLayers * numberOfNeighbors.
    double* groundwaterNeighborsFlowRate;              // 2D array of size numberOfSoilLayers * numberOfNeighbors.
    double* groundwaterNeighborsFlowCumulative;        // 2D array of size numberOfSoilLayers * numberOfNeighbors.

    // Constructor.  Allocates arrays of the appropriate size.
    //
    // All parameters directly initialize member variables.
    MeshElementState(size_t elementNumberInit, double currentTimeInit, size_t numberOfSoilLayersInit, size_t numberOfNeighborsInit);

    // Destructor.  Deletes arrays.
    ~MeshElementState();

    // Copy constructor.  Deep copy.
    MeshElementState(const MeshElementState& other);

    // Assignment operator.  Deep copy.
    MeshElementState& operator=(const MeshElementState& other);

    // Helper function to allocate arrays.  numberOfSoilLayers and numberOfNeighbors must already be set.  Does not delete anything previously pointed to so arrays must be unallocated or deleted.
    void allocateArrays();

    // Helper function to delete arrays.
    void deleteArrays();

  private:

    // Helper function to copy data.  numberOfSoilLayers and numberOfNeighbors must already be set and arrays must already exist.
    void copyData(const MeshElementState& other);
  };

  // MeshElementState and ChannelElementState are for individual elements to send their state to an OutputManager.
  // numberOfSoilLayers and/or numberOfNeighbors can be zero in which case applicable array pointers are NULL.
  class ChannelElementState
  {
  public:

    size_t  elementNumber;                       // The element that this data is for.
    double  currentTime;                         // The time point that this data is for.
    size_t  numberOfNeighbors;                   // The size of the neighbor arrays can be less than the maximum number allowed if the element has less.
    double  surfacewaterDepth;
    double  surfacewaterCreated;
    double  precipitationRate;
    double  precipitationCumulative;
    double  evaporationRate;
    double  evaporationCumulative;
    FileManager::EvapoTranspirationStateBlob evapoTranspirationState;
    double  snowWaterEquivalent;
    double* surfacewaterNeighborsExpirationTime; // 1D array of size numberOfNeighbors.
    double* surfacewaterNeighborsFlowRate;       // 1D array of size numberOfNeighbors.
    double* surfacewaterNeighborsFlowCumulative; // 1D array of size numberOfNeighbors.
    double* groundwaterNeighborsExpirationTime;  // 1D array of size numberOfNeighbors.
    double* groundwaterNeighborsFlowRate;        // 1D array of size numberOfNeighbors.
    double* groundwaterNeighborsFlowCumulative;  // 1D array of size numberOfNeighbors.

    // Constructor.  Allocates arrays of the appropriate size.
    //
    // All parameters directly initialize member variables.
    ChannelElementState(size_t elementNumberInit, double currentTimeInit, size_t numberOfNeighborsInit);

    // Destructor.  Deletes arrays.
    ~ChannelElementState();

    // Copy constructor.  Deep copy.
    ChannelElementState(const ChannelElementState& other);

    // Assignment operator.  Deep copy.
    ChannelElementState& operator=(const ChannelElementState& other);

    // Helper function to allocate arrays.  numberOfNeighbors must already be set.  Does not delete anything previously pointed to so arrays must be unallocated or deleted.
    void allocateArrays();

    // Helper function to delete arrays.
    void deleteArrays();

  private:

    // Helper function to copy data.  numberOfNeighbors must already be set and arrays must already exist.
    void copyData(const ChannelElementState& other);
  };

  // Constructor.
  //
  // Parameters:
  //
  // fileManagerInit - The subclass of FileManager used determines what type of files are created.  Must exist for the entire lifetime of the OutputManager.
  OutputManager(FileManager& fileManagerInit) : fileManager(fileManagerInit) {}

  // OutputManager starts by creating all of the files that it will write over the course of a simulation.  We do it this way for performance reasons for NetCDF files.
  // We do not call createFiles in the constructor for two reasons.
  // First, it calls the pure virtual interface, and the virtual dispatch tables won't be set up until the subclass constructor runs.
  // Second, it requires knowing referenceDate and simulationStartTime, which might need to be loaded from input files after construction time.
  // You must call createFiles on all OutputManagers and wait until they all return before proceeding with any other OutputManager operations.
  void createFiles();

  // Handle a state message received from a mesh element.
  // Puts the received state in the right location in a TimePointState.
  // If this is the first element received for that time point it allocates a new TimePointState.
  // If this is the last element needing to be received for that time point it writes the TimePointState to file and deletes it.
  //
  // Parameters:
  //
  // state - The state received from the mesh element.
  void handleMeshElementState(const MeshElementState& state);

  // Handle a state message received from a channel element.
  // Puts the received state in the right location in a TimePointState.
  // If this is the first element received for that time point it allocates a new TimePointState.
  // If this is the last element needing to be received for that time point it writes the TimePointState to file and deletes it.
  //
  // Parameters:
  //
  // state - The state received from the channel element.
  void handleChannelElementState(const ChannelElementState& state);

  // Pure virtual interface to the communication system.

  // Wrappers for communication topology.  Return values must not change over the course of a single run.
  // Indices must be unique and cover all indices from zero to numberOfOutputManagers minus one.
  virtual size_t numberOfOutputManagers() = 0; // Must be greater than zero.
  virtual size_t myOutputManagerIndex()   = 0; // Must be less than numberOfOutputManagers.

  // Wrappers for readonly variables.  Return values must not change over the course of a single run.
  virtual std::string directory()           = 0; // The directory in which to create the output files.  The filenames will be generated from the date and time.
  virtual double      referenceDate()       = 0; // Julian date.  Must be on or after 1 CE (1721425.5).
  virtual double      simulationStartTime() = 0; // Seconds after referenceDate.  Can be positive, negative, or zero, but the calendar date must be on or after 1 CE.
  virtual double      simulationDuration()  = 0; // Seconds.  Must be greater than or equal to zero.
  virtual double      outputPeriod()        = 0; // Seconds.  Must be greater than zero.

  // Wrappers for number of simulation elements.  Return values must not change over the course of a single run.
  // This OutputManager is responsible for the range of elements from localElementStart to (localElementStart + localNumberOfElements - 1).
  // The ranges of all OutputManagers must cover all elements from zero to (globalNumberOfElements - 1) without gaps or overlaps.
  // Any of these values can be zero if a type of entity doesn't exist in a particular simulation.
  virtual size_t globalNumberOfMeshElements()      = 0;
  virtual size_t localNumberOfMeshElements()       = 0; // Must be less than or equal to globalNumberOfMeshElements.
  virtual size_t localMeshElementStart()           = 0; // Must be less than globalNumberOfMeshElements or zero if localNumberOfMeshElements is zero.
  virtual size_t maximumNumberOfMeshSoilLayers()   = 0;
  virtual size_t maximumNumberOfMeshNeighbors()    = 0;
  virtual size_t globalNumberOfChannelElements()   = 0;
  virtual size_t localNumberOfChannelElements()    = 0; // Must be less than or equal to globalNumberOfChannelElements.
  virtual size_t localChannelElementStart()        = 0; // Must be less than globalNumberOfChannelElements or zero if localNumberOfChannelElements is zero.
  virtual size_t maximumNumberOfChannelNeighbors() = 0;

private:

  // Returns: true if all state has been received for a time point, false otherwise.
  //
  // Parameters:
  //
  // timePointState - The data container for the time point to check if all state has been received.
  bool allStateReceived(TimePointState* timePointState);

  FileManager&                      fileManager; // Wrapper for what type of files are created.
  std::map<double, TimePointState*> outputData;  // Sets of data for different time points.  Key is time in seconds.
};

// TimePointState contains all of this OutputManager's state for a single time point.  It is the data that will be written out to a single file.
// If any array dimension is zero applicable array pointers are NULL.
class TimePointState
{
public:

  bool*   meshStateReceived;                                                // 1D array of size localNumberOfMeshElements.  True if that element's state has been received.
  double* meshSurfacewaterDepth;                                            // 1D array of size localNumberOfMeshElements.
  double* meshSurfacewaterCreated;                                          // 1D array of size localNumberOfMeshElements.
  double* meshGroundwaterHead;                                              // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
  double* meshGroundwaterRecharge;                                          // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
  double* meshGroundwaterCreated;                                           // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
  double* meshPrecipitationRate;                                            // 1D array of size localNumberOfMeshElements.
  double* meshPrecipitationCumulative;                                      // 1D array of size localNumberOfMeshElements.
  double* meshEvaporationRate;                                              // 1D array of size localNumberOfMeshElements.
  double* meshEvaporationCumulative;                                        // 1D array of size localNumberOfMeshElements.
  double* meshTranspirationRate;                                            // 1D array of size localNumberOfMeshElements.
  double* meshTranspirationCumulative;                                      // 1D array of size localNumberOfMeshElements.
  FileManager::EvapoTranspirationStateBlob* meshEvapoTranspirationState;    // 1D array of size localNumberOfMeshElements.
  double* meshCanopyWaterEquivalent;                                        // 1D array of size localNumberOfMeshElements.
  double* meshSnowWaterEquivalent;                                          // 1D array of size localNumberOfMeshElements.
  FileManager::VadoseZoneStateBlob* meshVadoseZoneState;                    // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
  double* meshRootZoneWater;                                                // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
  double* meshTotalSoilWater;                                               // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers.
  double* meshSurfacewaterNeighborsExpirationTime;                          // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
  double* meshSurfacewaterNeighborsFlowRate;                                // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
  double* meshSurfacewaterNeighborsFlowCumulative;                          // 2D array of size localNumberOfMeshElements * maximumNumberOfMeshNeighbors.
  double* meshGroundwaterNeighborsExpirationTime;                           // 3D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors.
  double* meshGroundwaterNeighborsFlowRate;                                 // 3D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors.
  double* meshGroundwaterNeighborsFlowCumulative;                           // 3D array of size localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors.
  bool*   channelStateReceived;                                             // 1D array of size localNumberOfChannelElements.  True if that element's state has been received.
  double* channelSurfacewaterDepth;                                         // 1D array of size localNumberOfChannelElements.
  double* channelSurfacewaterCreated;                                       // 1D array of size localNumberOfChannelElements.
  double* channelPrecipitationRate;                                         // 1D array of size localNumberOfChannelElements.
  double* channelPrecipitationCumulative;                                   // 1D array of size localNumberOfChannelElements.
  double* channelEvaporationRate;                                           // 1D array of size localNumberOfChannelElements.
  double* channelEvaporationCumulative;                                     // 1D array of size localNumberOfChannelElements.
  FileManager::EvapoTranspirationStateBlob* channelEvapoTranspirationState; // 1D array of size localNumberOfChannelElements.
  double* channelSnowWaterEquivalent;                                       // 1D array of size localNumberOfChannelElements.
  double* channelSurfacewaterNeighborsExpirationTime;                       // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
  double* channelSurfacewaterNeighborsFlowRate;                             // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
  double* channelSurfacewaterNeighborsFlowCumulative;                       // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
  double* channelGroundwaterNeighborsExpirationTime;                        // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
  double* channelGroundwaterNeighborsFlowRate;                              // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.
  double* channelGroundwaterNeighborsFlowCumulative;                        // 2D array of size localNumberOfChannelElements * maximumNumberOfChannelNeighbors.

  // Constructor.  Allocates arrays of the appropriate size and sets all received flags to false.
  //
  // Parameters:
  //
  // outputManager - The OutputManager creating this TimePointState is used to get array sizes.
  TimePointState(OutputManager& outputManager);

  // Destructor.  Deletes arrays.
  ~TimePointState();

private:

  // Copy constructor.  Should never be copy constructed.
  TimePointState(const TimePointState& other);

  // Assignment operator.  Should never be assigned to.
  TimePointState& operator=(const TimePointState& other);
};

#endif // __OUTPUT_MANAGER_H__
