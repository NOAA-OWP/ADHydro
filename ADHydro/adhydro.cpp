#include "adhydro.h"
#include "all.h"

ADHydro::ADHydro(CkArgMsg* msg)
{
  MeshElementInitStruct initialValues;
  
  delete msg;
  
  meshProxySize = 2;
  
  meshProxy = CProxy_MeshElement::ckNew(meshProxySize);
  
  meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, doTimestep), thisProxy));
  
  initialValues.vertexX[0]        =   0.0;
  initialValues.vertexX[1]        = 100.0;
  initialValues.vertexX[2]        =   0.0;
  initialValues.vertexY[0]        =   0.0;
  initialValues.vertexY[1]        = 100.0;
  initialValues.vertexY[2]        = 100.0;
  initialValues.vertexZSurface[0] =  10.0;
  initialValues.vertexZSurface[1] =   0.0;
  initialValues.vertexZSurface[2] =  10.0;
  initialValues.vertexZBedrock[0] =   0.0;
  initialValues.vertexZBedrock[1] = -10.0;
  initialValues.vertexZBedrock[2] =   0.0;
  initialValues.neighbor[0]       = NOFLOW;
  initialValues.neighbor[1]       = INFLOW;
  initialValues.neighbor[2]       = 1;
  initialValues.interaction[2]    = MeshElement::BOTH_CALCULATE_FLOW;
  initialValues.catchment         = 0;
  initialValues.conductivity      = 1.0e-3;
  initialValues.porosity          = 0.5;
  initialValues.manningsN         = 0.04;
  initialValues.surfacewaterDepth = 0.0;
  initialValues.surfacewaterError = 0.0;
  initialValues.groundwaterHead   = 0.0;
  initialValues.groundwaterError  = 0.0;

  meshProxy[0].sendInitialize(initialValues);
  
  initialValues.vertexX[0]        =   0.0;
  initialValues.vertexX[1]        = 100.0;
  initialValues.vertexX[2]        = 100.0;
  initialValues.vertexY[0]        =   0.0;
  initialValues.vertexY[1]        =   0.0;
  initialValues.vertexY[2]        = 100.0;
  initialValues.vertexZSurface[0] =  10.0;
  initialValues.vertexZSurface[1] =   0.0;
  initialValues.vertexZSurface[2] =   0.0;
  initialValues.vertexZBedrock[0] =   0.0;
  initialValues.vertexZBedrock[1] = -10.0;
  initialValues.vertexZBedrock[2] = -10.0;
  initialValues.neighbor[0]       = OUTFLOW;
  initialValues.neighbor[1]       = 0;
  initialValues.neighbor[2]       = NOFLOW;
  initialValues.interaction[1]    = MeshElement::BOTH_CALCULATE_FLOW;
  
  meshProxy[1].sendInitialize(initialValues);
  
  iteration   = 1;
  currentTime = 0.0;
  endTime     = 100.0;
  
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
      
      // FIXME remove
      sleep(1);
    }
  else
    {
      CkPrintf("currentTime = %lf, simulation finished\n", currentTime);
      CkExit();
    }
}

int ADHydro::meshProxySize;

#include "adhydro.def.h"
