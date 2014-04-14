#include "adhydro.h"
#include "all.h"
#include <assert.h>

ADHydro::ADHydro(CkArgMsg* msg)
{
  delete msg;
  
  meshProxy = CProxy_MeshElement::ckNew(2);
  
  meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, timestepDone), thisProxy));
  
  currentTime = 0.0;
  endTime     = 10000.0;
  
  timestepDone(1.0);
}

ADHydro::ADHydro(CkMigrateMessage* msg)
{
  // Do nothing.
}

void ADHydro::timestepDone(double dtNew)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME how to return an error?
  assert(0.0 < dtNew);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (endTime > currentTime)
    {
      // FIXME remove
      usleep(100000);
      
      if (endTime - currentTime < dtNew)
        {
          dtNew = endTime - currentTime;
        }
      
      printf("currentTime = %lf, dtNew = %lf\n", currentTime, dtNew);
      
      currentTime += dtNew;
      
      meshProxy.doTimestep(dtNew);
    }
  else
    {
      printf("currentTime = %lf, simulation finished\n", currentTime);
      CkExit();
    }
}

#include "adhydro.def.h"
