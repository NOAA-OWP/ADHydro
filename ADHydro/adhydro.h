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
  
  // Do a timestep as long as currentTime is less than endTime otherwise exit.
  // Also serves as a callback for the dtNew reduction and a barrier at the end
  // of a timestep.
  //
  // Parameters:
  //
  // dtNew - The minimum new timestep requested by any element.
  void doTimestep(double dtNew);
  
private:
  
  // Array of mesh elements.
  CProxy_MeshElement meshProxy;
  
  // Time information.
  int    iteration;   // Iteration number to put on all messages this timestep.
  double currentTime; // Seconds.
  double endTime;     // Seconds.
};

#endif // __ADHYDRO_H__
