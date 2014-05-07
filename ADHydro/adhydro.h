#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

#include "mesh_element.h"
#include "adhydro.decl.h"

class ADHydro : public CBase_ADHydro
{
public:
  
  // Constructor.  This is the mainchare constructor where the program starts.
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
  
  // Set the load balancing mode to manual.  We need to wait for a few
  // timesteps to generate load statistics, and then we need to load balance
  // once.  After that we need to load balance very rarely if at all.
  static void setLoadBalancingToManual(void);
  
  // FIXME comment
  void initializeMesh(const char* filePath);
  
  // Do a timestep as long as currentTime is less than endTime otherwise exit.
  // Also serves as a callback for the dtNew reduction and a barrier at the end
  // of a timestep.
  //
  // Parameters:
  //
  // dtNew - The minimum new timestep requested by any element.
  void doTimestep(double dtNew);
  
  // Check invariant on member variables.  Exit if invariant is violated.
  void checkInvariant();
  
  // Output the load balancer partitioning as a ShowMe .part file.
  void outputPartitioning(CkReductionMsg *msg);
  
  // Readonly variable for mesh elements to know the size of their array.
  static int meshProxySize;
  
private:
  
  // Array of mesh elements.
  CProxy_MeshElement meshProxy;
  
  // Time information.
  int    iteration;   // Iteration number to put on all messages this timestep.
  double currentTime; // Seconds.
  double endTime;     // Seconds.
};

#endif // __ADHYDRO_H__
