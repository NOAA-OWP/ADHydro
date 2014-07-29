#include "adhydro.h"
#include "file_manager.h"

void ADHydro::setLoadBalancingToManual()
{
  // Charm++ call to set load balancing to manual.
  TurnManualLBOn();
}

ADHydro::ADHydro(CkArgMsg* msg)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  // Print usage message and exit if number of arguments is incorrect or either argument is any flag.
  if (!(3 == msg->argc && '-' != msg->argv[1][0] && '-' != msg->argv[2][0]))
    {
      CkPrintf("\nUsage:\n\nadhydro <input directory> <output directory>\n\n");
      CkPrintf("E.g.:\n\nadhydro ../input/mesh.1 ../output/mesh.1/run_1\n\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

  // Create file manager, mesh, and channels.
  // FIXME we need to make sure file manager variables are finished initializing before reading them.
  fileManagerProxy = CProxy_FileManager::ckNew();
  meshProxy        = CProxy_MeshElement::ckNew(fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements);
  channelProxy     = CProxy_ChannelElement::ckNew(fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements);

  // Initialize mesh and channels.
  meshProxy.initialize(channelProxy, fileManagerProxy);
  channelProxy.initialize(meshProxy, fileManagerProxy);
  
  // Initialize member variables.
  commandLineArguments = msg; // There's no provision for deleting commandLineArguments.  We could do it in a destructor for ADHydro, but that will only be
                              // called right before the program exits when the memory for commandLineArguments will be released anyway.
  currentTime          =  0.0;
  endTime              = 10.0;
  dt                   =  1.0;
  iteration            =  1;
  
  doTimestep();
}

ADHydro::ADHydro(CkMigrateMessage* msg)
{
  // Do nothing.
}

void ADHydro::pup(PUP::er &p)
{
  bool commandLineArgumentsNonNULL = (NULL != commandLineArguments);
  
  CBase_ADHydro::pup(p);
  __sdag_pup(p);
  p | meshProxy;
  p | channelProxy;
  p | fileManagerProxy;
  p | commandLineArgumentsNonNULL;
  
  if (commandLineArgumentsNonNULL)
    {
      if (p.isUnpacking())
        {
          commandLineArguments = new CkArgMsg;
        }
      
      p | *commandLineArguments;
    }
  else
    {
      commandLineArguments = NULL;
    }
  
  p | currentTime;
  p | endTime;
  p | dt;
  p | iteration;
}

void ADHydro::doTimestep()
{
  if (endTime > currentTime)
    {
      // Limit dt to time remaining.
      if (endTime - currentTime < dt)
        {
          dt = endTime - currentTime;
        }
      
      // Print at beginning of timestep.
      CkPrintf("currentTime = %lf, dt = %lf, iteration = %d\n", currentTime, dt, iteration);
      
      // Set callbacks.
      meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, meshTimestepDone), thisProxy));
      channelProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, channelTimestepDone), thisProxy));
      
      // Start timestep.
      meshProxy.doTimestep(iteration, dt);
      channelProxy.doTimestep(iteration, dt);
      thisProxy.waitForTimestepToFinish();
    }
  else
    {
      CkPrintf("currentTime = %lf, simulation finished\n", currentTime);
      CkExit();
    }
}

void ADHydro::timestepDone(double dtNew)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtNew))
    {
      CkError("ERROR in ADHydro::doTimestep: dtNew must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Update time and iteration number.
  currentTime += dt;
  dt           = dtNew;
  iteration++;
  
  // Load balance once after waiting a few iterations to generate load statistics.
  if (5 == iteration && 1 < CkNumPes())
    {
      CkStartLB();
    }
  
  doTimestep();
}

#include "adhydro.def.h"
