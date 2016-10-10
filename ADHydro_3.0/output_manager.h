#ifndef __OUTPUT_MANAGER_H__
#define __OUTPUT_MANAGER_H__

#include "file_manager.h"
#include <vector>

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
    size_t  outputIndex;                               // The time point that this data is for.
    size_t  numberOfSoilLayers;                        // The size of the soil layer and neighbor arrays can be less than the maximum number allowed if the element has less.
    size_t  numberOfNeighbors;
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

    // Constructor.  Allocates arrays.
    //
    // All parameters directly initialize member variables.
    MeshElementState(size_t elementNumberInit, size_t outputIndexInit, size_t numberOfSoilLayersInit, size_t numberOfNeighborsInit) :
      elementNumber(elementNumberInit), outputIndex(outputIndexInit), numberOfSoilLayers(numberOfSoilLayersInit), numberOfNeighbors(numberOfNeighborsInit) { allocateArrays(); }

    // Destructor.  Deletes arrays.
    ~MeshElementState() { deleteArrays(); }

    // Copy constructor.  Deep copy.
    MeshElementState(const MeshElementState& other) :
      elementNumber(other.elementNumber), outputIndex(other.outputIndex), numberOfSoilLayers(other.numberOfSoilLayers), numberOfNeighbors(other.numberOfNeighbors) { allocateArrays(); copyData(other); }

    // Assignment operator.  Deep copy.
    MeshElementState& operator=(const MeshElementState& other);

    // Helper function to allocate arrays.  numberOfSoilLayers and numberOfNeighbors must already be set.  Does not delete anything previously pointed to so arrays must be unallocated or deleted.
    void allocateArrays();

    // Helper function to delete arrays.
    void deleteArrays();

    // Helper function to copy data.  numberOfSoilLayers and numberOfNeighbors must already be set and arrays must already exist.
    void copyData(const MeshElementState& other);
  };

  // MeshElementState and ChannelElementState are for individual elements to send their state to an OutputManager.
  // numberOfSoilLayers and/or numberOfNeighbors can be zero in which case applicable array pointers are NULL.
  class ChannelElementState
  {
  public:

    size_t  elementNumber;                       // The element that this data is for.
    size_t  outputIndex;                         // The time point that this data is for.
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

    // Constructor.  Allocates arrays.
    //
    // All parameters directly initialize member variables.
    ChannelElementState(size_t elementNumberInit, size_t outputIndexInit, size_t numberOfNeighborsInit) :
      elementNumber(elementNumberInit), outputIndex(outputIndexInit), numberOfNeighbors(numberOfNeighborsInit) { allocateArrays(); }

    // Destructor.  Deletes arrays.
    ~ChannelElementState() { deleteArrays(); }

    // Copy constructor.  Deep copy.
    ChannelElementState(const ChannelElementState& other) :
      elementNumber(other.elementNumber), outputIndex(other.outputIndex), numberOfNeighbors(other.numberOfNeighbors) { allocateArrays(); copyData(other); }

    // Assignment operator.  Deep copy.
    ChannelElementState& operator=(const ChannelElementState& other);

    // Helper function to allocate arrays.  numberOfNeighbors must already be set.  Does not delete anything previously pointed to so arrays must be unallocated or deleted.
    void allocateArrays();

    // Helper function to delete arrays.
    void deleteArrays();

    // Helper function to copy data.  numberOfNeighbors must already be set and arrays must already exist.
    void copyData(const ChannelElementState& other);
  };

  // Constructor.  Does not completely initialize outputData because that requires values from the pure virtual interface that can't be called in the constructor.
  // You must call handleInitialize after those values are ready.
  //
  // Parameters:
  //
  // fileManagerInit - The subclass of FileManager used determines what type of files are created.  Must exist for the entire lifetime of the OutputManager.
  OutputManager(FileManager& fileManagerInit) : fileManager(fileManagerInit), nextOutputIndex(1) {}

  // outputData is initialized to have a size of the number of output files plus one with all elements set to NULL.  outputData[0] is unused.
  void handleInitialize();

  // Handle a state message received from a mesh element.
  // Puts the received state in the right location in a TimePointState.
  // If this is the first element received for that time point it allocates a new TimePointState.
  //
  // Parameters:
  //
  // state - The state received from the mesh element.
  void handleMeshElementState(const MeshElementState& state);

  // Handle a state message received from a channel element.
  // Puts the received state in the right location in a TimePointState.
  // If this is the first element received for that time point it allocates a new TimePointState.
  //
  // Parameters:
  //
  // state - The state received from the channel element.
  void handleChannelElementState(const ChannelElementState& state);

  // Returns: true if all state that is needed for the next output phase has been received.
  bool readyToOutput();

  FileManager&                              fileManager;     // Wrapper for what type of files are created.
  std::vector<FileManager::TimePointState*> outputData;      // Sets of data for different time points.  The size of outputData is the number of output files plus one and outputData[0] is unused.
  size_t                                    nextOutputIndex; // The next index in outputData to output.  Goes from one to the number of output files.

private:

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
  virtual double      outputPeriod()        = 0; // A file will be outputted for every this many seconds of simulation time.  Must be greater than zero.
  virtual size_t      outputGroupSize()     = 0; // The OutputManager waits until it gets this many files worth of data and outputs them all together in a single output phase.  Must be greater than zero.
  virtual int         verbosityLevel()      = 0; // Flag for how much progress and warning messages to display.

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

  // Returns the outputTime for an outputIndex.
  double calculateOutputTime(size_t outputIndex);
};

#endif // __OUTPUT_MANAGER_H__
