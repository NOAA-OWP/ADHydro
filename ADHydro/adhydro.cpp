#include "adhydro.h"
#include "file_manager.h"

void ADHydro::setLoadBalancingToManual()
{
  // Charm++ call to set load balancing to manual.
  TurnManualLBOn();
}

ADHydro::ADHydro(CkArgMsg* msg)
{
  // Save command line arguments.
  commandLineArguments = msg;

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  // Print usage message and exit if number of arguments is incorrect or either argument is any flag.
  if (!(3 == commandLineArguments->argc && '-' != commandLineArguments->argv[1][0] && '-' != commandLineArguments->argv[2][0]))
    {
      CkPrintf("\nUsage:\n\nadhydro <input directory> <output directory>\n\n");
      CkPrintf("E.g.:\n\nadhydro ../input/mesh.1 ../output/mesh.1/run_1\n\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // Create file manager.
  fileManagerProxy = CProxy_FileManager::ckNew();
  
  // Set the callback to continue initialization when the file manager is ready.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, fileManagerBarrier), thisProxy));
  
  // Initialize the file manager.
  //fileManagerProxy.initializeFromASCIIFiles(strlen(commandLineArguments->argv[1]) + 1, commandLineArguments->argv[1], strlen("mesh.1") + 1, "mesh.1");
  fileManagerProxy.initializeFromNetCDFFiles(strlen(commandLineArguments->argv[1]) + 1, commandLineArguments->argv[1]);
  // FIXME make ascii fileBasename a command line parameter.
  // FIXME remove hardcoded mesh form source code? fileManagerProxy.initializeHardcodedMesh();
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
  p | outputPeriod;
  p | nextOutputTime;
  p | iteration;
  p | writeGeometry;
  p | writeParameter;
  p | needToUpdateForcingData;
  p | needToCheckInvariant;
}

void ADHydro::fileManagerBarrier()
{
  // Set the callback to continue initialization when the file manager is ready.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, fileManagerInitialized), thisProxy));
  
  // Have the file manager complete initialization.
  fileManagerProxy.calculateDerivedValues();
}

void ADHydro::fileManagerInitialized()
{
  // Initialize member variables.
  currentTime             = fileManagerProxy.ckLocalBranch()->currentTime;
  endTime                 = currentTime + 12000; // FIXME
  dt                      = fileManagerProxy.ckLocalBranch()->dt;
  outputPeriod            = 3600; // FIXME
  nextOutputTime          = currentTime + outputPeriod;
  iteration               = fileManagerProxy.ckLocalBranch()->iteration;
  writeGeometry           = true;
  writeParameter          = true;
  needToUpdateForcingData = true;
  needToCheckInvariant    = true;
  
  // Create mesh and channels.
  if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
    {
      meshProxy = CProxy_MeshElement::ckNew(fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements);
    }
  
  if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
    {
      channelProxy = CProxy_ChannelElement::ckNew(fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements);
    }

  // Initialize mesh and channels.
  if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
    {
      meshProxy.initialize(channelProxy, fileManagerProxy);
    }
  
  if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
    {
      channelProxy.initialize(meshProxy, fileManagerProxy);
    }
  
  // Set the callback to write output files after they are created.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, writeFiles), thisProxy));
  
  // Have the file manager create output files.
  fileManagerProxy.createFiles(strlen(commandLineArguments->argv[2]) + 1, commandLineArguments->argv[2]);
}

void ADHydro::writeFiles()
{
  // Set the callback to check forcing data when files are written.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, checkForcingData), thisProxy));
  
#ifdef NETCDF_COLLECTIVE_IO_WORKAROUND
  fileManagerProxy[0].resizeUnlimitedDimensions(strlen(commandLineArguments->argv[2]) + 1, commandLineArguments->argv[2], writeGeometry, writeParameter, true);
#else // NETCDF_COLLECTIVE_IO_WORKAROUND
  fileManagerProxy.writeFiles(strlen(commandLineArguments->argv[2]) + 1, commandLineArguments->argv[2], writeGeometry, writeParameter, true);
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
  
  // Now that we have outputted the geometry and parameters we don't need to do it again unless they change.
  writeGeometry  = false;
  writeParameter = false;
}

void ADHydro::checkForcingData()
{
  if (needToUpdateForcingData)
    {
      // Set callback.
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
        {
          meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, meshForcingDataDone), thisProxy));
        }
      else
        {
          thisProxy.meshForcingDataDone();
        }
      
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
        {
          channelProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, channelForcingDataDone), thisProxy));
        }
      else
        {
          thisProxy.channelForcingDataDone();
        }

      // Update forcing data.
      fileManagerProxy.readForcingData(meshProxy, channelProxy, currentTime, strlen(commandLineArguments->argv[1]) + 1, commandLineArguments->argv[1]);
      thisProxy.waitForForcingDataToFinish();
      
      // Now that we have updated the forcing data we don't need to do it again unless they change.
      needToUpdateForcingData = false;
    }
  else
    {
      forcingDataDone();
    }
}

void ADHydro::forcingDataDone()
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  // If debug level is set to internal invariants check every timestep.
  checkInvariant();
#elif (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (needToCheckInvariant)
    {
      checkInvariant();
      
      // If debug level is set to user input invariants check once at the beginning.
      needToCheckInvariant = false;
    }
#else // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  // Otherwise do not check the invariant and just start the timestep.
  doTimestep();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
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
  
  if (!(0.0 <= outputPeriod))
    {
      CkError("ERROR in ADHydro::checkInvariant: outputPeriod must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!error)
    {
      // Set callbacks and check invariant.
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
        {
          meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, meshInvariantDone), thisProxy));
          meshProxy.checkInvariant();
        }
      else
        {
          thisProxy.meshInvariantDone();
        }
      
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
        {
          channelProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, channelInvariantDone), thisProxy));
          channelProxy.checkInvariant();
        }
      else
        {
          thisProxy.channelInvariantDone();
        }

      thisProxy.waitForInvariantToFinish();
    }
  else
    {
      CkExit();
    }
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
      CkPrintf("currentTime = %lf, dt = %lf, iteration = %u\n", currentTime, dt, iteration);

      // Set callbacks and start timestep.
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
        {
          meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, meshTimestepDone), thisProxy));
          meshProxy.doTimestep(iteration, dt);
        }
      else
        {
          thisProxy.meshTimestepDone(2.0 * dt);
        }
      
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
        {
          channelProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, channelTimestepDone), thisProxy));
          channelProxy.doTimestep(iteration, dt);
        }
      else
        {
          thisProxy.channelTimestepDone(2.0 * dt);
        }

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

  // FIXME replace this with absolute time
  if ((int)(currentTime / 3600.0) < (int)(currentTime + dt / 3600.0))
  {
    needToUpdateForcingData = true;
  }

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
          // Advance nextOutputTime by multiples of outputPeriod until it is greater than currentTime.
          nextOutputTime += outputPeriod * (floor((currentTime - nextOutputTime) / outputPeriod) + 1);
        }

      // Set the callback to write output files after state information is updated.
      fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, writeFiles), thisProxy));

      // Start the output phase.
      fileManagerProxy.updateState(currentTime, dt, iteration);
      
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
        {
          meshProxy.updateState();
        }
      
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
        {
          channelProxy.updateState();
        }
    }
  else
    {
      checkForcingData();
    }
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "adhydro.def.h"
#pragma GCC diagnostic warning "-Wunused-variable"
