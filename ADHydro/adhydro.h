#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

#include "mesh_element.h"
#include "file_manager.h"
#include "adhydro.decl.h"

class ADHydro : public CBase_ADHydro
{
public:
  
  // FIXME document exit on error.
  
  // Set the load balancing mode to manual.  We need to wait for a few
  // timesteps to generate load statistics, and then we need to load balance
  // once.  After that we need to load balance very rarely if at all.
  static void setLoadBalancingToManual();
  
  // Constructor.  This is the mainchare constructor where the program starts.
  // Exit on error.
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

  // If currentTime is less than endTime do a timestep otherwise exit.  Exit on
  // error.  Also serves as a callback for the dtNew reduction and a barrier at
  // the end of a timestep.
  //
  // Parameters:
  //
  // dtNew - The minimum new timestep requested by any element.
  void doTimestep(double dtNew);
  
  // Entry point to resume from a checkpoint.
  void resumeFromCheckpoint();
  
  // Output the load balancer partitioning as a ShowMe .part file.
  void outputPartitioning(CkReductionMsg *msg);
  
  // Readonly variable for mesh elements to know the size of their array.
  static int meshProxySize;
  
private:
  
  // Initialize the meshProxy array to a mesh read in from a NetCDF file.
  //
  // Parameters:
  //
  // inputFile - The file containing the mesh.
  void initializeMesh(const char* inputFile);
  
  // Check invariant on member variables.  Exit if invariant is violated.
  void checkInvariant();
  
  // Array of mesh elements.
  CProxy_MeshElement meshProxy;
  
  // Time information.
  int    iteration;   // Iteration number to put on all messages this timestep.
  double dt;          // Current timestep duration in seconds.
  double currentTime; // Seconds.
  double endTime;     // Seconds.
};

#endif // __ADHYDRO_H__
