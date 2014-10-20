#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

#include "all.h"

// file_manager.decl.h needs CProxy_MeshElement from mesh_element.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// mesh_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// channel_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_MeshElement from mesh_element.decl.h.
// These forward declarations break the circularity.
class CProxy_MeshElement;
class CProxy_ChannelElement;

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "adhydro.decl.h"
#pragma GCC diagnostic warning "-Wsign-compare"

// An ADHydro object is the main chare of the program.  Execution starts in its
// constructor.  Here is a flowchart of how the execution proceeds.
//
// create file managers
// initialize file managers
// initialize adhydro member variables
// create mesh and channel elements
// initialize mesh and channel elements
// create files
//   |
//   V
// write files <----------------|
//   |                          |
//   V                          |
// check forcing data <------\  |
//   |      |                |  |
//   |      V                |  |
//   |  update forcing data  |  |
//   |      |                |  |
//   V      V                |  |
// forcing data done         |  |
//   |      |                |  |
//   |      V                |  |
//   |  check invariant      |  |
//   |      |                |  |
//   V      V                |  |
// do timestep               |  |
//   |      |                |  |
//   |      V                |  |
//   |    exit               |  |
//   |                       |  |
//   V                       |  |
// timestep done ------------/  |
//   |                          |
//   V                          |
// update state ----------------/
class ADHydro : public CBase_ADHydro
{
  ADHydro_SDAG_CODE
  
public:
  
  // Set the load balancing mode to manual.  We need to wait for a few
  // timesteps to generate load statistics, and then we need to load balance
  // once.  After that we need to load balance very rarely if at all.
  static void setLoadBalancingToManual();
  
  // Constructor.  This is the mainchare constructor where the program starts.
  // Will cause a callback to fileManagerBarrier.
  //
  // Parameters:
  //
  // msg - Charm++ command line arguments message.
  ADHydro(CkArgMsg* msg);
  
  // Charm++ migration constructor.
  //
  // Parameters:
  //
  // msg - Charm++ migration message.
  ADHydro(CkMigrateMessage* msg);
  
  // Destructor.  commandLineArguments needs to be deleted.
  ~ADHydro();
  
  // Pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);

  // Reduction callback used as a barrier during file manager initialization.
  // Will cause a callback to fileManagerInitialized.
  void fileManagerBarrier();
  
  // Finish initialization after the reduction callback indicating the file
  // manager is ready.  Will cause a callback to writeFiles.
  void fileManagerInitialized();
  
  // Tell the file manager to write files.  Will cause a callback to
  // checkForcingData.
  void writeFiles();
  
  // Check if the forcing data needs to be updated and if so update it.
  // Will cause a callback or direct call to forcingDataDone.
  void checkForcingData();
  
private:
  
  // After the forcing data is checked and possibly updated this function
  // serves as a branch point to decide whether to check the invariant.
  // Will cause a direct call to checkInvariant or doTimestep.
  void forcingDataDone();
  
  // Check invariant conditions on member variables.  Exit if invariant is
  // violated.  Will cause a callback to doTimestep.
  void checkInvariant();

  // If currentTime is less than endTime do a timestep, otherwise exit.  If it
  // does a timestep it will cause a callback to timestepDone.
  void doTimestep();
  
  // Callback for the dtNew reduction at the end of a timestep.  Will cause a
  // callback to writeFiles or a direct call to checkForcingData.
  //
  // Parameters:
  //
  // dtNew - The minimum new timestep requested by any element.
  void timestepDone(double dtNew);
  
  // Chare proxies.
  CProxy_MeshElement    meshProxy;        // Array of mesh elements.
  CProxy_ChannelElement channelProxy;     // Array of channel elements.
  CProxy_FileManager    fileManagerProxy; // Group of file managers.
  
  // I/O information
  CkArgMsg* commandLineArguments; // Contains the input and output directory paths.
  
  // Time information.
  double currentTime;       // Seconds.
  double endTime;           // Seconds.
  double dt;                // Next timestep duration in seconds.
  double outputPeriod;      // Simulation time in seconds between outputting to file.  Zero for output every timestep.
  double nextOutputTime;    // Next time in seconds to output to file.
  size_t iteration;         // Iteration number to put on all messages this timestep.
  size_t startingIteration; // Iteration number of the first timestep in this run.  Used to decide when to load balance.
  
  // Flags.
  bool writeGeometry;           // Flag to indicate whether the geometry has changed and needs to be outputted.
  bool writeParameter;          // Flag to indicate whether the parameters have changed and need to be outputted.
  bool needToUpdateForcingData; // Flag to indicate whether forcing data needs to be updated.  FIXME Change this to be based on absolute time.
  bool needToCheckInvariant;    // Flag to indicate whether the invariant needs to be checked.
};

#endif // __ADHYDRO_H__
