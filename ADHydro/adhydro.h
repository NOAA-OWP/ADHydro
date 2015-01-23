#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

// file_manager.decl.h needs CProxy_MeshElement from mesh_element.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// mesh_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// channel_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_MeshElement from mesh_element.decl.h.
// These forward declarations break the circularity.
class CProxy_MeshElement;
class CProxy_ChannelElement;

#include "evapo_transpiration.h"
#include "all.h"

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
  
  // Global read-only variable specifies whether to run the simulation in drain
  // down mode.  In this mode, there will be no precipitation, and water level
  // in the channels will be capped.
  static bool drainDownMode;
  
  // Global read-only variable specifies whether to run mesh massage to fix
  // digital dams and similar problems.
  static bool doMeshMassage;
  
  // Global read-only variable specifies how much to print to the display.
  // Current verbosity levels are:
  //
  // 0 - Error messages only.
  // 1 - Messages about general simulation progress.
  // 2 - Warning messages about situations that are probably a problem.
  // 3 - Warning messages about situations that may be a problem, but just
  //     create too many messages for level two.
  // 4 - Warning messages about situations that are probably ok.
  static int verbosityLevel;
  
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
  
  // Pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);

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
  std::string inputDirectory;  // The directory from which to read input files.
  std::string outputDirectory; // The directory in which to place output files.
  
  // Time information.
  double referenceDate;      // The Julian date when currentTime is zero.  The current date and time of the simulation is the Julian date equal to
                             // referenceDate + currentTime / (24.0 * 3600.0).
  double currentTime;        // Current simulation time in seconds since referenceDate.
  double simulationDuration; // The duration to simulate in seconds.
  double endTime;            // The time to simulate to in seconds since referenceDate.
  double dt;                 // Next simulation timestep duration in seconds.
  double outputPeriod;       // Simulation time in seconds between outputs to file.  Zero means output every timestep.
  double nextOutputTime;     // Simulation time in seconds since referenceDate of the next output to file.  The next output to file will occur as soon as
                             // currentTime equals or exceeds this value.
  size_t iteration;          // Iteration number to put on all messages this timestep.
  size_t startingIteration;  // Iteration number of the first timestep in this run.  Used to decide when to load balance.
  
  // Flags.
  bool writeGeometry;        // Flag to indicate whether the geometry has changed and needs to be outputted.
  bool writeParameter;       // Flag to indicate whether the parameters have changed and need to be outputted.
  bool needToCheckInvariant; // Flag to indicate whether the invariant needs to be checked.
  bool printMessage;         // Flag to indicate whether you need to print a message after a callback.
};

#endif // __ADHYDRO_H__
