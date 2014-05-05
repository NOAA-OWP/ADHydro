#include "adhydro.h"
#include "all.h"

ADHydro::ADHydro(CkArgMsg* msg)
{
  double          vertexXInit[3];
  double          vertexYInit[3];
  double          vertexZSurfaceInit[3];
  double          vertexZBedrockInit[3];
  int             neighborInit[3];
  InteractionEnum interactionInit[3];
  
  delete msg;
  
  meshProxySize = 2;
  
  meshProxy = CProxy_MeshElement::ckNew(meshProxySize);
  
  meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, doTimestep), thisProxy));
  
  vertexXInit[0]        =   0.0;
  vertexXInit[1]        = 100.0;
  vertexXInit[2]        =   0.0;
  vertexYInit[0]        =   0.0;
  vertexYInit[1]        = 100.0;
  vertexYInit[2]        = 100.0;
  vertexZSurfaceInit[0] =  10.0;
  vertexZSurfaceInit[1] =   0.0;
  vertexZSurfaceInit[2] =  10.0;
  vertexZBedrockInit[0] =   0.0;
  vertexZBedrockInit[1] = -10.0;
  vertexZBedrockInit[2] =   0.0;
  neighborInit[0]       = NOFLOW;
  neighborInit[1]       = INFLOW;
  neighborInit[2]       = 1;
  interactionInit[2]    = BOTH_CALCULATE_FLOW_RATE;
  
  meshProxy[0].sendInitialize(vertexXInit, vertexYInit, vertexZSurfaceInit, vertexZBedrockInit, neighborInit, interactionInit, 0, 1.0e-3, 0.5, 0.04, 0.0, 0.0, 0.0, 0.0, 0.0);
  
  vertexXInit[0]        =   0.0;
  vertexXInit[1]        = 100.0;
  vertexXInit[2]        = 100.0;
  vertexYInit[0]        =   0.0;
  vertexYInit[1]        =   0.0;
  vertexYInit[2]        = 100.0;
  vertexZSurfaceInit[0] =  10.0;
  vertexZSurfaceInit[1] =   0.0;
  vertexZSurfaceInit[2] =   0.0;
  vertexZBedrockInit[0] =   0.0;
  vertexZBedrockInit[1] = -10.0;
  vertexZBedrockInit[2] = -10.0;
  neighborInit[0]       = OUTFLOW;
  neighborInit[1]       = 0;
  neighborInit[2]       = NOFLOW;
  interactionInit[1]    = BOTH_CALCULATE_FLOW_RATE;
  
  meshProxy[1].sendInitialize(vertexXInit, vertexYInit, vertexZSurfaceInit, vertexZBedrockInit, neighborInit, interactionInit, 0, 1.0e-3, 0.5, 0.04, 0.0, 0.0, 0.0, 0.0, 0.0);
  
  iteration   = 1;
  currentTime = 0.0;
  endTime     = 10000000.0;
  
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
      meshProxy.sendCheckInvariant();
      
      // FIXME remove
      usleep(10000);
    }
  else
    {
      CkPrintf("currentTime = %lf, simulation finished\n", currentTime);
      CkExit();
    }
}

int ADHydro::meshProxySize;

#include "adhydro.def.h"
