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
  
  // Initialize member variables.
  commandLineArguments = msg;
  currentTime          =  0.0;
  endTime              = 10.0;
  dt                   =  1.0;
  nextOutputTime       = currentTime;
  outputPeriod         =  1.5;
  iteration            =  1;

  // Create file manager.
  fileManagerProxy = CProxy_FileManager::ckNew(strlen(commandLineArguments->argv[1]) + 1, commandLineArguments->argv[1]);
  
  // Set the callback to finish initialization when the file manager is ready.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, fileManagerFirstCallback), thisProxy));
}

ADHydro::ADHydro(CkMigrateMessage* msg)
{
  // Do nothing.
}

ADHydro::~ADHydro()
{
  if (NULL != commandLineArguments)
    {
      delete commandLineArguments;
    }
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

void ADHydro::fileManagerFirstCallback()
{
  // Set the callback to finish initialization when the file manager is ready.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, finishInitialization), thisProxy));
  
  // Have the file manager complete initialization
  fileManagerProxy.finishInitialization(strlen(commandLineArguments->argv[2]) + 1, commandLineArguments->argv[2], 1, 1, iteration, currentTime, dt);
}

void ADHydro::finishInitialization()
{
  // Create mesh and channels.
  meshProxy    = CProxy_MeshElement::ckNew(fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements);
  channelProxy = CProxy_ChannelElement::ckNew(fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements);

  // Initialize mesh and channels.
  meshProxy.initialize(channelProxy, fileManagerProxy);
  channelProxy.initialize(meshProxy, fileManagerProxy);
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  // Check the invariant.  The invariant callback will start the timestep.
  checkInvariant();
#else // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  // Just start the timestep.
  doTimestep();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
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
      CkError("ERROR in ADHydro::timestepDone: dtNew must be greater than zero.\n");
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
  
  // Check if it is time to output to file.
  if (currentTime >= nextOutputTime || currentTime >= endTime)
    {
      if (0.0 < outputPeriod)
        {
          // Force nextOutputTime to be the next multiple of outputPeriod that is greater than currentTime.
          nextOutputTime += outputPeriod * (floor((currentTime - nextOutputTime) / outputPeriod) + 1);
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      // Set the callback to check the invariant when output is done.  The invariant callback will start the timestep.
      fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, checkInvariant), thisProxy));
#else // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      // Set the callback to just start the timestep when output is done.
      fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, doTimestep), thisProxy));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      
      // Start the output phase.
      fileManagerProxy.doOutput(strlen(commandLineArguments->argv[2]) + 1, commandLineArguments->argv[2], false, 1, false, 1, true, iteration, currentTime,
                                dt);
      meshProxy.doOutput();
      channelProxy.doOutput();
    }
  else
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      // Check the invariant.  The invariant callback will start the timestep.
      checkInvariant();
#else // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      // Just start the timestep.
      doTimestep();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    }
}

void ADHydro::checkInvariant()
{
  bool error = false; // Error flag.
  
  if (!(NULL != commandLineArguments))
    {
      CkError("ERROR in ADHydro::checkInvariant: commandLineArguments must not be NULL.\n");
      error = true;
    }
  
  if (!(currentTime <= endTime))
    {
      CkError("ERROR in ADHydro::checkInvariant: currentTime must be less than or equal to endTime.\n");
      error = true;
    }
  
  if (!(0.0 < dt))
    {
      CkError("ERROR in ADHydro::checkInvariant: dt must be greater than zero.\n");
      error = true;
    }
  
  if (!error)
    {
      // Set callbacks.
      meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, meshInvariantDone), thisProxy));
      channelProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, channelInvariantDone), thisProxy));

      // Check invariant.
      meshProxy.checkInvariant();
      channelProxy.checkInvariant();
      thisProxy.waitForInvariantToFinish();
    }
  else
    {
      CkExit();
    }
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "adhydro.def.h"
#pragma GCC diagnostic warning "-Wunused-variable"
