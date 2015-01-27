#include "adhydro.h"
#include "file_manager.h"
#include "INIReader.h"

// Read-only global variables.
bool ADHydro::appendToInputFiles;
bool ADHydro::drainDownMode;
bool ADHydro::doMeshMassage;
int  ADHydro::verbosityLevel;

void ADHydro::setLoadBalancingToManual()
{
  // Charm++ call to set load balancing to manual.
  TurnManualLBOn();
}

ADHydro::ADHydro(CkArgMsg* msg)
{
  const char* superfileName = (1 < msg->argc) ? (msg->argv[1]) : (""); // The first command line argument protected against non-existance.
  INIReader   superfile(superfileName);                                // superfile reader object.
  long        referenceDateYear;                                       // For converting Gregorian date to Julian date.
  long        referenceDateMonth;                                      // For converting Gregorian date to Julian date.
  long        referenceDateDay;                                        // For converting Gregorian date to Julian date.
  long        referenceDateHour;                                       // For converting Gregorian date to Julian date.
  long        referenceDateMinute;                                     // For converting Gregorian date to Julian date.
  double      referenceDateSecond;                                     // For converting Gregorian date to Julian date.
  std::string asciiFileBasename;                                       // The basename of the ASCII files if initializing from ASCII files.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  // If the superfile cannot be read print usage message and exit.
  if (0 != superfile.ParseError())
    {
      if (0 > superfile.ParseError())
        {
          CkError("ERROR in ADHydro::ADHydro: cannot open superfile %s.\n", superfileName);
        }
      else
        {
          CkError("ERROR in ADHydro::ADHydro: parse error on line number %d of superfile %s.\n", superfile.ParseError(), superfileName);
        }
      
      CkError("\nUsage:\n\nadhydro <superfile>\n\n");
      CkError("E.g.:\n\nadhydro ../input/mesh.1/run_1.superfile\n\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // Get initialization values from the superfile.
  inputDirectory     = superfile.Get("", "inputDirectory", "");
  appendToInputFiles = superfile.GetBoolean("", "appendToInputFiles", false); // If appendToInputFiles is not in the superfile it defaults to false and new
                                                                              // output files will be created in outputDirectory.
  
  if (appendToInputFiles)
    {
      outputDirectory = inputDirectory;
    }
  else
    {
      outputDirectory = superfile.Get("", "outputDirectory", "");
    }
  
  referenceDate = superfile.GetReal("", "referenceDate", NAN); // If referenceDate is not in the superfile it will first attempt to convert the Gregorian date
                                                               // in referenceDateYear, referenceDateMonth, referenceDateDay, referenceDateHour,
                                                               // referenceDateMinute, and referenceDateSecond to a Julian date.  If that is unsuccessful it
                                                               // will be taken from the input NetCDF files.
  
  // If there is no referenceDate read a Gregorian date and convert to julian date.
  if (isnan(referenceDate))
    {
      referenceDateYear   = superfile.GetInteger("", "referenceDateYear", -1);
      referenceDateMonth  = superfile.GetInteger("", "referenceDateMonth", -1);
      referenceDateDay    = superfile.GetInteger("", "referenceDateDay", -1);
      referenceDateHour   = superfile.GetInteger("", "referenceDateHour", -1);
      referenceDateMinute = superfile.GetInteger("", "referenceDateMinute", -1);
      referenceDateSecond = superfile.GetReal("", "referenceDateSecond", -1.0);
      
      if (1 <= referenceDateYear && 1 <= referenceDateMonth && 12 >= referenceDateMonth && 1 <= referenceDateDay && 31 >= referenceDateDay &&
          0 <= referenceDateHour && 23 >= referenceDateHour && 0 <= referenceDateMinute && 59 >= referenceDateMinute && 0.0 <= referenceDateSecond &&
          60.0 > referenceDateSecond)
        {
          referenceDate = gregorianToJulian(referenceDateYear, referenceDateMonth, referenceDateDay, referenceDateHour, referenceDateMinute,
                                            referenceDateSecond);
        }
    }
  
  currentTime        = superfile.GetReal("", "currentTime", NAN);        // If currentTime is not in the superfile it will be taken from the input NetCDF
                                                                         // files.
  simulationDuration = superfile.GetReal("", "simulationDuration", 0.0); // If simulationDuration is not in the superfile it defaults to zero and the
                                                                         // simulation will not run any timesteps.
  dt                 = superfile.GetReal("", "dt", NAN);                 // If dt is not in the superfile it will be taken from the input NetCDF files.
  outputPeriod       = superfile.GetReal("", "outputPeriod", 0.0);       // If outputPeriod is not in the superfile it defaults to zero and the simulation will
                                                                         // output every timestep.
  iteration          = superfile.GetInteger("", "iteration", -1);        // If iteration is not in the superfile it will be taken from the input NetCDF files.
  drainDownMode      = superfile.GetBoolean("", "drainDownMode", false); // If drainDownMode is not in the superfile it defaults to false and the simulation
                                                                         // will run in normal mode.
  doMeshMassage      = superfile.GetBoolean("", "doMeshMassage", false); // If doMeshMassage is not in the superfile it defaults to false and mesh massage will
                                                                         // not be run.
  verbosityLevel     = superfile.GetInteger("", "verbosityLevel", 2);    // If verbosityLevel is not in the superfile it defaults to two.
  
  // Create file manager.
  fileManagerProxy = CProxy_FileManager::ckNew();
  
  // Set the callback to continue initialization when the file manager is ready.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, fileManagerInitialized), thisProxy));
  
  // Initialize the file manager.
  if (superfile.GetBoolean("", "initializeFromASCIIFiles", false))
    {
      asciiFileBasename = superfile.Get("", "asciiFileBasename", "");
      
      fileManagerProxy.initializeFromASCIIFiles(inputDirectory.length() + 1, inputDirectory.c_str(), asciiFileBasename.length() + 1,
                                                asciiFileBasename.c_str());
    }
  else
    {
      // FIXME send a std::string in the message instead of a C string and length?
      // FIXME or make inputDirectory and outputDirectory read only variables?
      fileManagerProxy.initializeFromNetCDFFiles(inputDirectory.length() + 1, inputDirectory.c_str());
    }
  
  if (1 <= verbosityLevel)
    {
      CkPrintf("Reading input files.\n");
    }
}

ADHydro::ADHydro(CkMigrateMessage* msg)
{
  // Do nothing.
}

void ADHydro::pup(PUP::er &p)
{
  CBase_ADHydro::pup(p);
  __sdag_pup(p);
  p | meshProxy;
  p | channelProxy;
  p | fileManagerProxy;
  p | inputDirectory;
  p | outputDirectory;
  p | referenceDate;
  p | currentTime;
  p | simulationDuration;
  p | endTime;
  p | dt;
  p | outputPeriod;
  p | nextOutputTime;
  p | iteration;
  p | startingIteration;
  p | writeGeometry;
  p | writeParameter;
  p | needToCheckInvariant;
  p | printMessage;
}

void ADHydro::fileManagerInitialized()
{
  if (1 <= verbosityLevel)
    {
      CkPrintf("Finished reading input files.\n");
      
      if (appendToInputFiles)
        {
          printMessage = false;
        }
      else
        {
          CkPrintf("Writing output files.\n");
          
          printMessage = true;
        }
    }
  
  // Initialize member variables.
  if (isnan(referenceDate))
    {
      referenceDate = fileManagerProxy.ckLocalBranch()->referenceDate;
    }
  
  if (isnan(currentTime))
    {
      currentTime = fileManagerProxy.ckLocalBranch()->currentTime;
    }
  
  endTime = currentTime + simulationDuration;
  
  if (isnan(dt))
    {
      dt = fileManagerProxy.ckLocalBranch()->dt;
    }
  
  nextOutputTime = currentTime + outputPeriod;
  
  if ((size_t)-1 == iteration)
    {
      iteration = fileManagerProxy.ckLocalBranch()->iteration;
    }
  
  startingIteration    = iteration;
  writeGeometry        = true;
  writeParameter       = true;
  needToCheckInvariant = true;
  
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
  
  if (appendToInputFiles)
    {
      checkForcingData();
    }
  else
    {
      // Set the callback to write output files after they are created.
      fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, writeFiles), thisProxy));

      // Have the file manager create output files.
      fileManagerProxy.createFiles(outputDirectory.length() + 1, outputDirectory.c_str());
    }
}

void ADHydro::writeFiles()
{
  // Set the callback to check forcing data when files are written.
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, checkForcingData), thisProxy));
  
  fileManagerProxy.writeFiles(outputDirectory.length() + 1, outputDirectory.c_str(), writeGeometry, writeParameter, true);
  
  // Now that we have outputted the geometry and parameters we don't need to do it again unless they change.
  writeGeometry  = false;
  writeParameter = false;
}

void ADHydro::checkForcingData()
{
  if (printMessage)
    {
      CkPrintf("Finished writing output files.\n");
      
      printMessage = false;
    }
  
  if (endTime > currentTime && (!fileManagerProxy.ckLocalBranch()->forcingDataInitialized ||
                                fileManagerProxy.ckLocalBranch()->nextForcingDataDate <= referenceDate + currentTime / (24.0 * 3600.0)))
    {
      if (1 <= verbosityLevel)
        {
          CkPrintf("Reading forcing data.\n");
          
          printMessage = true;
        }
      
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
      fileManagerProxy.readForcingData(inputDirectory.length() + 1, inputDirectory.c_str(), meshProxy, channelProxy, referenceDate,
                                       currentTime);
      thisProxy.waitForForcingDataToFinish();
    }
  else
    {
      forcingDataDone();
    }
}

void ADHydro::forcingDataDone()
{
  long   forcingDataYear;   // For calculating Gregorian date from Julian date.
  long   forcingDataMonth;  // For calculating Gregorian date from Julian date.
  long   forcingDataDay;    // For calculating Gregorian date from Julian date.
  long   forcingDataHour;   // For calculating Gregorian date from Julian date.
  long   forcingDataMinute; // For calculating Gregorian date from Julian date.
  double forcingDataSecond; // For calculating Gregorian date from Julian date.
  long   simulationYear;    // For calculating Gregorian date from Julian date.
  long   simulationMonth;   // For calculating Gregorian date from Julian date.
  long   simulationDay;     // For calculating Gregorian date from Julian date.
  long   simulationHour;    // For calculating Gregorian date from Julian date.
  long   simulationMinute;  // For calculating Gregorian date from Julian date.
  double simulationSecond;  // For calculating Gregorian date from Julian date.
  
  if (printMessage)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(fileManagerProxy.ckLocalBranch()->forcingDataInitialized);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
      julianToGregorian(fileManagerProxy.ckLocalBranch()->forcingDataDate, &forcingDataYear, &forcingDataMonth, &forcingDataDay, &forcingDataHour,
                        &forcingDataMinute, &forcingDataSecond);
      julianToGregorian(referenceDate + currentTime / (24.0 * 3600.0), &simulationYear, &simulationMonth, &simulationDay, &simulationHour,
                        &simulationMinute, &simulationSecond);
      
      CkPrintf("Finished reading forcing data for %02d/%02d/%04d %02d:%02d:%05.2lf at simulation time %02d/%02d/%04d %02d:%02d:%05.2lf.\n", forcingDataMonth,
               forcingDataDay, forcingDataYear, forcingDataHour, forcingDataMinute, forcingDataSecond, simulationMonth, simulationDay, simulationYear,
               simulationHour, simulationMinute, simulationSecond);
      
      printMessage = false;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  // If debug level is set to internal invariants check every timestep.
  checkInvariant();
#elif (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  // If debug level is set to user input invariants check once at the beginning.
  if (needToCheckInvariant)
    {
      checkInvariant();
      
      needToCheckInvariant = false;
    }
  else
    {
      doTimestep();
    }
#else // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  // Otherwise do not check the invariant and just start the timestep.
  doTimestep();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
}

void ADHydro::checkInvariant()
{
  bool error = false; // Error flag.
  
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
      if (1 <= verbosityLevel)
        {
          CkPrintf("currentTime = %lf, dt = %lf, iteration = %lu.\n", currentTime, dt, iteration);
        }

      // Set callbacks and start timestep.
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
        {
          meshProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, meshTimestepDone), thisProxy));
          meshProxy.doTimestep(iteration, referenceDate + currentTime / (24.0 * 3600.0), dt);
        }
      else
        {
          thisProxy.meshTimestepDone(2.0 * dt);
        }
      
      if (0 < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
        {
          channelProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, channelTimestepDone), thisProxy));
          channelProxy.doTimestep(iteration, referenceDate + currentTime / (24.0 * 3600.0), dt);
        }
      else
        {
          thisProxy.channelTimestepDone(2.0 * dt);
        }

      thisProxy.waitForTimestepToFinish();
    }
  else
    {
      if (1 <= verbosityLevel)
        {
          CkPrintf("currentTime = %lf, simulation finished.\n", currentTime);
        }
      
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
  if (startingIteration + 5 == iteration && 1 < CkNumPes())
    {
      if (1 <= verbosityLevel)
        {
          CkPrintf("Starting load balancing.\n");
        }
      
      CkStartLB();
    }

  // Check if it is time to output to file.
  if (currentTime >= nextOutputTime || currentTime >= endTime)
    {
      if (1 <= verbosityLevel)
        {
          CkPrintf("Writing output files.\n");
          
          printMessage = true;
        }
      
      if (0.0 < outputPeriod)
        {
          // Advance nextOutputTime by multiples of outputPeriod until it is greater than currentTime.
          nextOutputTime += outputPeriod * (floor((currentTime - nextOutputTime) / outputPeriod) + 1);
        }

      // Set the callback to write output files after state information is updated.
      fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydro, writeFiles), thisProxy));

      // Start the output phase.
      fileManagerProxy.updateState(referenceDate, currentTime, dt, iteration);
      
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
