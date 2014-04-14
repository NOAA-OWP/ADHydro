#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

#include "adhydro.decl.h"
#include "mesh_element.decl.h"

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
  
  // Callback for the new timestep reduction.  Also serves as a barrier at the
  // end of a timestep.
  //
  // Parameters:
  //
  // dtNew - The minimum new timestep requested by any element.
  void timestepDone(double dtNew);
  
private:
  
  // Array of mesh elements.
  CProxy_MeshElement meshProxy;
  
  // Time information.
  double currentTime; // Seconds.
  double endTime;     // Seconds.
};

#endif // __ADHYDRO_H__
