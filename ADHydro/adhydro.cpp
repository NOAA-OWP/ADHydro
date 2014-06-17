#include "adhydro.h"
#include "all.h"
#include <netcdf.h>

void ADHydro::setLoadBalancingToManual()
{
  // Charm++ call to set load balancing to manual.
  TurnManualLBOn();
}

ADHydro::ADHydro(CkArgMsg* msg)
{
  // Print usage message and exit if number of arguments is incorrect or either argument is any flag.
  if (!(3 == msg->argc && '-' != msg->argv[1][0] && '-' != msg->argv[2][0]))
    {
      CkPrintf("\nUsage:\n\nadhydro <input directory> <output directory>\n\n");
      CkPrintf("E.g.:\n\nadhydro ../input/mesh.1 ../output/mesh.1/run_1\n\n");
      CkExit();
    }
  
  // Set member variables.  There's no provision for deleting commandLineArguments.  We could do it in a destructor for ADHydro, but that will only be called
  // right before the program exits when the memory for commandLineArguments will be released anyway.
  commandLineArguments = msg;
  // FIXME how to decide which groups to open in input and output files?  Right now default to 0.
  geometryGroup        = 0;
  parameterGroup       = 0;
  stateGroup           = 0;
  currentTime          = 0.0;
  endTime              = 900.0;
  dt                   = 1.0;
  outputPeriod         = 60.0;
  nextOutputTime       = currentTime + outputPeriod;
  iteration            = 1;

  // Create file manager.
  fileManagerProxy = CProxy_FileManager::ckNew();

  // Set callback.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, inputFilesOpened), thisProxy));

  // Open NetCDF input files.
  fileManagerProxy.openFiles(strlen(commandLineArguments->argv[1]) + 1, commandLineArguments->argv[1], 0, 0,
                             FILE_MANAGER_READ, geometryGroup, FILE_MANAGER_READ, parameterGroup, FILE_MANAGER_READ, stateGroup);
  
  waitForFilesToClose = true;
}

ADHydro::ADHydro(CkMigrateMessage* msg)
{
  // Do nothing.
}

void ADHydro::pup(PUP::er &p)
{
  bool commandLineArgumentsNonNULL = (NULL != commandLineArguments);
  
  CBase_ADHydro::pup(p);
  p | meshProxySize;
  p | meshProxy;
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
  
  p | geometryGroup;
  p | parameterGroup;
  p | stateGroup;
  p | waitForFilesToClose;
  p | currentTime;
  p | endTime;
  p | dt;
  p | outputPeriod;
  p | nextOutputTime;
  p | iteration;
}

void ADHydro::inputFilesOpened()
{
  // Get mesh size.
  meshProxySize = fileManagerProxy.ckLocalBranch()->numberOfMeshElements;

  // Create mesh.  MeshElement constructor will read NetCDF input files.
  meshProxy = CProxy_MeshElement::ckNew(fileManagerProxy, meshProxySize);

  // Set callback.
  meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, inputFilesRead), thisProxy));
}

void ADHydro::inputFilesRead()
{
  // Set callback.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, inputFilesClosed), thisProxy));

  // Close NetCDF input files.
  fileManagerProxy.closeFiles();
}

void ADHydro::inputFilesClosed()
{
  waitForFilesToClose = false;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  // Set callback.
  meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, createOutputFiles), thisProxy));

  // Check invariant.
  checkInvariant();
#else
  // checkInvariant will result in a callback to createOutputFiles when finished.  If we are not checking the invariant just call the callback.
  createOutputFiles();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
}

void ADHydro::checkInvariant()
{
  bool error = false; // Error flag.
  
  if (!(0 < meshProxySize))
    {
      CkError("ERROR in ADHydro::checkInvariant: meshProxySize must be greater than zero.\n");
      error = true;
    }
  
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
  
  if (!(0.0 < outputPeriod))
    {
      CkError("ERROR in ADHydro::checkInvariant: outputPeriod must be greater than zero.\n");
      error = true;
    }
  
  if (!error)
    {
      // Check mesh invariant.  Callback must already be set to createOutputFiles or checkOutputTime before calling checkInvariant.
      meshProxy.checkInvariant();
    }
  else
    {
      CkExit();
    }
}

void ADHydro::createOutputFiles()
{
  // Set callback.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, writeOutputFiles), thisProxy));

  // Create NetCDF output files.
  fileManagerProxy.openFiles(strlen(commandLineArguments->argv[2]) + 1, commandLineArguments->argv[2], meshProxySize, meshProxySize * 3,
                             FILE_MANAGER_CREATE, geometryGroup, FILE_MANAGER_CREATE, parameterGroup, FILE_MANAGER_CREATE, stateGroup);
  
  waitForFilesToClose = true;
}

void ADHydro::checkOutputTime()
{
  // Check if output is needed.
  if (currentTime >= nextOutputTime || currentTime >= endTime)
    {
      // Update nextOutputTime and output mesh state.
      while (currentTime >= nextOutputTime)
        {
          nextOutputTime += outputPeriod;
        }
      
      // Increment state file group.
      stateGroup++;
      
      // Set callback.
      fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, writeOutputFiles), thisProxy));

      // Open NetCDF output files.
      fileManagerProxy.openFiles(strlen(commandLineArguments->argv[2]) + 1, commandLineArguments->argv[2], meshProxySize, meshProxySize * 3,
                                 FILE_MANAGER_NO_ACTION, geometryGroup, FILE_MANAGER_NO_ACTION, parameterGroup, FILE_MANAGER_WRITE, stateGroup);
      
      waitForFilesToClose = true;
    }
  else
    {
      // Writing output files will result in a call to doTimestep when finished.  If we are not writing output files just call doTimestep.
      doTimestep();
    }
}

void ADHydro::writeOutputFiles()
{
  bool         error                  = false;                            // Error flag.
  FileManager* fileManagerLocalBranch = fileManagerProxy.ckLocalBranch(); // For accessing public member variables.
  int          ncErrorCode;                                               // Return value of NetCDF functions.
  
  // Write attributes.
  if (FileManager::OPEN_FOR_READ_WRITE == fileManagerLocalBranch->stateFileStatus)
    {
      ncErrorCode = nc_put_att_double(fileManagerLocalBranch->stateGroupID, NC_GLOBAL, "time", NC_DOUBLE, 1, &currentTime);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in ADHydro::writeOutputFiles: unable to write time attribute in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      
      if (!error)
        {
          ncErrorCode = nc_put_att_int(fileManagerLocalBranch->stateGroupID, NC_GLOBAL, "geometryGroup", NC_INT, 1, &geometryGroup);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydro::writeOutputFiles: unable to write geometryGroup attribute in NetCDF state file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          ncErrorCode = nc_put_att_int(fileManagerLocalBranch->stateGroupID, NC_GLOBAL, "parameterGroup", NC_INT, 1, &parameterGroup);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydro::writeOutputFiles: unable to write parameterGroup attribute in NetCDF state file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      // Set callback.
      meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, outputFilesWritten), thisProxy));

      // Write NetCDF output files.
      meshProxy.output();
    }
  else
    {
      CkExit();
    }
}

void ADHydro::outputFilesWritten()
{
  // Set callback.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, outputFilesClosed), thisProxy));

  // Close NetCDF output files.
  fileManagerProxy.closeFiles();
  
  // Don't need to wait for output files to be closed.  Structured dagger code will make sure they are closed before we open them again.
  doTimestep();
}

void ADHydro::outputFilesClosed()
{
  waitForFilesToClose = false;
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
      
      // Set callback.
      meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, timestepDone), thisProxy));
      
      // Process timestep.
      meshProxy.doTimestep(iteration, dt);
    }
  else
    {
      CkPrintf("currentTime = %lf, simulation finished\n", currentTime);
      endProgram();
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

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  // Set callback.
  meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, checkOutputTime), thisProxy));

  // Check invariant.
  checkInvariant();
#else
  // checkInvariant will result in a callback to checkOutputTime when finished.  If we are not checking the invariant just call the callback.
  checkOutputTime();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
}

void ADHydro::endProgram()
{
  if (waitForFilesToClose)
    {
      // Resend message to self.
      thisProxy.endProgram();
    }
  else
    {
      CkExit();
    }
}

int ADHydro::meshProxySize;

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "adhydro.def.h"
#pragma GCC diagnostic warning "-Wunused-variable"
