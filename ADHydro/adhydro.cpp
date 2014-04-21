#include "adhydro.h"

ADHydro::ADHydro(CkArgMsg* msg)
{
  delete msg;
  
  meshProxy = CProxy_MeshElement::ckNew(2);
  
  meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, doTimestep), thisProxy));
  
  iteration   = 1;
  currentTime = 0.0;
  endTime     = 10000.0;
  
  doTimestep(1.0);
}

ADHydro::ADHydro(CkMigrateMessage* msg)
{
  // Do nothing.
}

void ADHydro::doTimestep(double dtNew)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtNew))
    {
      CkError("ERROR: dtNew must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (endTime > currentTime)
    {
      if (endTime - currentTime < dtNew)
        {
          dtNew = endTime - currentTime;
        }
      
      CkPrintf("currentTime = %lf, dtNew = %lf\n", currentTime, dtNew);
      
      iteration++;
      currentTime += dtNew;
      
      meshProxy.sendDoTimestep(iteration, dtNew);
    }
  else
    {
      CkPrintf("currentTime = %lf, simulation finished\n", currentTime);
      CkExit();
    }
}

#include "adhydro.def.h"
