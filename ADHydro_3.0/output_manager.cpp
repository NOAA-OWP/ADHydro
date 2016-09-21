#include "output_manager.h"
#include "all.h"

OutputManager::MeshElementState::MeshElementState(size_t numberOfSoilLayersInit, size_t numberOfNeighborsInit) :
numberOfSoilLayers(numberOfSoilLayersInit),
numberOfNeighbors(numberOfNeighborsInit)
{
  allocateArrays();
}

OutputManager::MeshElementState::~MeshElementState()
{
  deleteArrays();
}

OutputManager::MeshElementState::MeshElementState(const MeshElementState& other) :
numberOfSoilLayers(other.numberOfSoilLayers),
numberOfNeighbors(other.numberOfNeighbors)
{
  allocateArrays();
  copyData(other);
}

OutputManager::MeshElementState& OutputManager::MeshElementState::operator=(const MeshElementState& other)
{
  if (numberOfSoilLayers != other.numberOfSoilLayers || numberOfNeighbors != other.numberOfNeighbors)
    {
      deleteArrays();

      numberOfSoilLayers = other.numberOfSoilLayers;
      numberOfNeighbors  = other.numberOfNeighbors;

      allocateArrays();
    }

  copyData(other);

  return *this;
}

void OutputManager::MeshElementState::allocateArrays()
{
  if (0 < numberOfSoilLayers)
    {
      groundwaterHead     = new double[numberOfSoilLayers];
      groundwaterRecharge = new double[numberOfSoilLayers];
      groundwaterCreated  = new double[numberOfSoilLayers];
      vadoseZoneState     = new FileManager::VadoseZoneStateBlob[numberOfSoilLayers];
      rootZoneWater       = new double[numberOfSoilLayers];
      totalSoilWater      = new double[numberOfSoilLayers];

      if (0 < numberOfNeighbors)
        {
          groundwaterNeighborsExpirationTime = new double[numberOfSoilLayers * numberOfNeighbors];
          groundwaterNeighborsFlowRate       = new double[numberOfSoilLayers * numberOfNeighbors];
          groundwaterNeighborsFlowCumulative = new double[numberOfSoilLayers * numberOfNeighbors];
        }
      else
        {
          groundwaterNeighborsExpirationTime = NULL;
          groundwaterNeighborsFlowRate       = NULL;
          groundwaterNeighborsFlowCumulative = NULL;
        }
    }
  else
    {
      groundwaterHead                    = NULL;
      groundwaterRecharge                = NULL;
      groundwaterCreated                 = NULL;
      vadoseZoneState                    = NULL;
      rootZoneWater                      = NULL;
      totalSoilWater                     = NULL;
      groundwaterNeighborsExpirationTime = NULL;
      groundwaterNeighborsFlowRate       = NULL;
      groundwaterNeighborsFlowCumulative = NULL;
    }

  if (0 < numberOfNeighbors)
    {
      surfacewaterNeighborsExpirationTime = new double[numberOfNeighbors];
      surfacewaterNeighborsFlowRate       = new double[numberOfNeighbors];
      surfacewaterNeighborsFlowCumulative = new double[numberOfNeighbors];
    }
  else
    {
      surfacewaterNeighborsExpirationTime = NULL;
      surfacewaterNeighborsFlowRate       = NULL;
      surfacewaterNeighborsFlowCumulative = NULL;
    }
}

void OutputManager::MeshElementState::deleteArrays()
{
  delete[] groundwaterHead;
  delete[] groundwaterRecharge;
  delete[] groundwaterCreated;
  delete[] vadoseZoneState;
  delete[] rootZoneWater;
  delete[] totalSoilWater;
  delete[] surfacewaterNeighborsExpirationTime;
  delete[] surfacewaterNeighborsFlowRate;
  delete[] surfacewaterNeighborsFlowCumulative;
  delete[] groundwaterNeighborsExpirationTime;
  delete[] groundwaterNeighborsFlowRate;
  delete[] groundwaterNeighborsFlowCumulative;
}

void OutputManager::MeshElementState::copyData(const MeshElementState& other)
{
  size_t ii, jj; // Loop counters.

  surfacewaterDepth       = other.surfacewaterDepth;
  surfacewaterCreated     = other.surfacewaterCreated;
  precipitationRate       = other.precipitationRate;
  precipitationCumulative = other.precipitationCumulative;
  evaporationRate         = other.evaporationRate;
  evaporationCumulative   = other.evaporationCumulative;
  transpirationRate       = other.transpirationRate;
  transpirationCumulative = other.transpirationCumulative;
  evapoTranspirationState = other.evapoTranspirationState;
  canopyWaterEquivalent   = other.canopyWaterEquivalent;
  snowWaterEquivalent     = other.snowWaterEquivalent;

  for (ii = 0; ii < numberOfSoilLayers; ++ii)
    {
      groundwaterHead[ii]     = other.groundwaterHead[ii];
      groundwaterRecharge[ii] = other.groundwaterRecharge[ii];
      groundwaterCreated[ii]  = other.groundwaterCreated[ii];
      vadoseZoneState[ii]     = other.vadoseZoneState[ii];
      rootZoneWater[ii]       = other.rootZoneWater[ii];
      totalSoilWater[ii]      = other.totalSoilWater[ii];

      for (jj = 0; jj < numberOfNeighbors; ++jj)
        {
          groundwaterNeighborsExpirationTime[ii * numberOfSoilLayers + jj] = other.groundwaterNeighborsExpirationTime[ii * numberOfSoilLayers + jj];
          groundwaterNeighborsFlowRate[      ii * numberOfSoilLayers + jj] = other.groundwaterNeighborsFlowRate[      ii * numberOfSoilLayers + jj];
          groundwaterNeighborsFlowCumulative[ii * numberOfSoilLayers + jj] = other.groundwaterNeighborsFlowCumulative[ii * numberOfSoilLayers + jj];
        }
    }

  for (ii = 0; ii < numberOfNeighbors; ++ii)
    {
      surfacewaterNeighborsExpirationTime[ii] = other.surfacewaterNeighborsExpirationTime[ii];
      surfacewaterNeighborsFlowRate[ii]       = other.surfacewaterNeighborsFlowRate[ii];
      surfacewaterNeighborsFlowCumulative[ii] = other.surfacewaterNeighborsFlowCumulative[ii];
    }
}

OutputManager::ChannelElementState::ChannelElementState(size_t numberOfNeighborsInit) :
numberOfNeighbors(numberOfNeighborsInit)
{
  allocateArrays();
}

OutputManager::ChannelElementState::~ChannelElementState()
{
  deleteArrays();
}

OutputManager::ChannelElementState::ChannelElementState(const ChannelElementState& other) :
numberOfNeighbors(other.numberOfNeighbors)
{
  allocateArrays();
  copyData(other);
}

OutputManager::ChannelElementState& OutputManager::ChannelElementState::operator=(const ChannelElementState& other)
{
  if (numberOfNeighbors != other.numberOfNeighbors)
    {
      deleteArrays();

      numberOfNeighbors = other.numberOfNeighbors;

      allocateArrays();
    }

  copyData(other);

  return *this;
}

void OutputManager::ChannelElementState::allocateArrays()
{
  if (0 < numberOfNeighbors)
    {
      surfacewaterNeighborsExpirationTime = new double[numberOfNeighbors];
      surfacewaterNeighborsFlowRate       = new double[numberOfNeighbors];
      surfacewaterNeighborsFlowCumulative = new double[numberOfNeighbors];
      groundwaterNeighborsExpirationTime  = new double[numberOfNeighbors];
      groundwaterNeighborsFlowRate        = new double[numberOfNeighbors];
      groundwaterNeighborsFlowCumulative  = new double[numberOfNeighbors];
    }
  else
    {
      surfacewaterNeighborsExpirationTime = NULL;
      surfacewaterNeighborsFlowRate       = NULL;
      surfacewaterNeighborsFlowCumulative = NULL;
      groundwaterNeighborsExpirationTime  = NULL;
      groundwaterNeighborsFlowRate        = NULL;
      groundwaterNeighborsFlowCumulative  = NULL;
    }
}

void OutputManager::ChannelElementState::deleteArrays()
{
  delete[] surfacewaterNeighborsExpirationTime;
  delete[] surfacewaterNeighborsFlowRate;
  delete[] surfacewaterNeighborsFlowCumulative;
  delete[] groundwaterNeighborsExpirationTime;
  delete[] groundwaterNeighborsFlowRate;
  delete[] groundwaterNeighborsFlowCumulative;
}

void OutputManager::ChannelElementState::copyData(const ChannelElementState& other)
{
  size_t ii; // Loop counter.

  surfacewaterDepth       = other.surfacewaterDepth;
  surfacewaterCreated     = other.surfacewaterCreated;
  precipitationRate       = other.precipitationRate;
  precipitationCumulative = other.precipitationCumulative;
  evaporationRate         = other.evaporationRate;
  evaporationCumulative   = other.evaporationCumulative;
  evapoTranspirationState = other.evapoTranspirationState;
  snowWaterEquivalent     = other.snowWaterEquivalent;

  for (ii = 0; ii < numberOfNeighbors; ++ii)
    {
      surfacewaterNeighborsExpirationTime[ii] = other.surfacewaterNeighborsExpirationTime[ii];
      surfacewaterNeighborsFlowRate[ii]       = other.surfacewaterNeighborsFlowRate[ii];
      surfacewaterNeighborsFlowCumulative[ii] = other.surfacewaterNeighborsFlowCumulative[ii];
      groundwaterNeighborsExpirationTime[ii]  = other.groundwaterNeighborsExpirationTime[ii];
      groundwaterNeighborsFlowRate[ii]        = other.groundwaterNeighborsFlowRate[ii];
      groundwaterNeighborsFlowCumulative[ii]  = other.groundwaterNeighborsFlowCumulative[ii];
    }
}

OutputManager::TimePointState::TimePointState(OutputManager& outputManager)
{
  size_t ii; // Loop counter.

  if (0 < outputManager.localNumberOfMeshElements())
    {
      meshStateReceived = new bool[outputManager.localNumberOfMeshElements()];

      for (ii = 0; ii < outputManager.localNumberOfMeshElements(); ++ii)
        {
          meshStateReceived[ii] = false;
        }

      meshSurfacewaterDepth   = new double[outputManager.localNumberOfMeshElements()];
      meshSurfacewaterCreated = new double[outputManager.localNumberOfMeshElements()];

      if (0 < outputManager.maximumNumberOfMeshSoilLayers())
        {
          meshGroundwaterHead     = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers()];
          meshGroundwaterRecharge = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers()];
          meshGroundwaterCreated  = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers()];
          meshVadoseZoneState     = new FileManager::VadoseZoneStateBlob[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers()];
          meshRootZoneWater       = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers()];
          meshTotalSoilWater      = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers()];
        }
      else
        {
          meshGroundwaterHead     = NULL;
          meshGroundwaterRecharge = NULL;
          meshGroundwaterCreated  = NULL;
          meshVadoseZoneState     = NULL;
          meshRootZoneWater       = NULL;
          meshTotalSoilWater      = NULL;
        }

      meshPrecipitationRate       = new double[outputManager.localNumberOfMeshElements()];
      meshPrecipitationCumulative = new double[outputManager.localNumberOfMeshElements()];
      meshEvaporationRate         = new double[outputManager.localNumberOfMeshElements()];
      meshEvaporationCumulative   = new double[outputManager.localNumberOfMeshElements()];
      meshTranspirationRate       = new double[outputManager.localNumberOfMeshElements()];
      meshTranspirationCumulative = new double[outputManager.localNumberOfMeshElements()];
      meshEvapoTranspirationState = new FileManager::EvapoTranspirationStateBlob[outputManager.localNumberOfMeshElements()];
      meshCanopyWaterEquivalent   = new double[outputManager.localNumberOfMeshElements()];
      meshSnowWaterEquivalent     = new double[outputManager.localNumberOfMeshElements()];

      if (0 < outputManager.maximumNumberOfMeshNeighbors())
        {
          meshSurfacewaterNeighborsExpirationTime = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshNeighbors()];
          meshSurfacewaterNeighborsFlowRate       = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshNeighbors()];
          meshSurfacewaterNeighborsFlowCumulative = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshNeighbors()];

          if (0 < outputManager.maximumNumberOfMeshSoilLayers())
            {
              meshGroundwaterNeighborsExpirationTime = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers() * outputManager.maximumNumberOfMeshNeighbors()];
              meshGroundwaterNeighborsFlowRate       = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers() * outputManager.maximumNumberOfMeshNeighbors()];
              meshGroundwaterNeighborsFlowCumulative = new double[outputManager.localNumberOfMeshElements() * outputManager.maximumNumberOfMeshSoilLayers() * outputManager.maximumNumberOfMeshNeighbors()];
            }
          else
            {
              meshGroundwaterNeighborsExpirationTime = NULL;
              meshGroundwaterNeighborsFlowRate       = NULL;
              meshGroundwaterNeighborsFlowCumulative = NULL;
            }
        }
      else
        {
          meshSurfacewaterNeighborsExpirationTime = NULL;
          meshSurfacewaterNeighborsFlowRate       = NULL;
          meshSurfacewaterNeighborsFlowCumulative = NULL;
          meshGroundwaterNeighborsExpirationTime  = NULL;
          meshGroundwaterNeighborsFlowRate        = NULL;
          meshGroundwaterNeighborsFlowCumulative  = NULL;
        }
    }
  else
    {
      meshStateReceived                       = NULL;
      meshSurfacewaterDepth                   = NULL;
      meshSurfacewaterCreated                 = NULL;
      meshGroundwaterHead                     = NULL;
      meshGroundwaterRecharge                 = NULL;
      meshGroundwaterCreated                  = NULL;
      meshPrecipitationRate                   = NULL;
      meshPrecipitationCumulative             = NULL;
      meshEvaporationRate                     = NULL;
      meshEvaporationCumulative               = NULL;
      meshTranspirationRate                   = NULL;
      meshTranspirationCumulative             = NULL;
      meshEvapoTranspirationState             = NULL;
      meshCanopyWaterEquivalent               = NULL;
      meshSnowWaterEquivalent                 = NULL;
      meshVadoseZoneState                     = NULL;
      meshRootZoneWater                       = NULL;
      meshTotalSoilWater                      = NULL;
      meshSurfacewaterNeighborsExpirationTime = NULL;
      meshSurfacewaterNeighborsFlowRate       = NULL;
      meshSurfacewaterNeighborsFlowCumulative = NULL;
      meshGroundwaterNeighborsExpirationTime  = NULL;
      meshGroundwaterNeighborsFlowRate        = NULL;
      meshGroundwaterNeighborsFlowCumulative  = NULL;
    }

  if (0 < outputManager.localNumberOfChannelElements())
    {
      channelStateReceived           = new bool[outputManager.localNumberOfChannelElements()];

      for (ii = 0; ii < outputManager.localNumberOfChannelElements(); ++ii)
        {
          channelStateReceived[ii] = false;
        }

      channelSurfacewaterDepth       = new double[outputManager.localNumberOfChannelElements()];
      channelSurfacewaterCreated     = new double[outputManager.localNumberOfChannelElements()];
      channelPrecipitationRate       = new double[outputManager.localNumberOfChannelElements()];
      channelPrecipitationCumulative = new double[outputManager.localNumberOfChannelElements()];
      channelEvaporationRate         = new double[outputManager.localNumberOfChannelElements()];
      channelEvaporationCumulative   = new double[outputManager.localNumberOfChannelElements()];
      channelEvapoTranspirationState = new FileManager::EvapoTranspirationStateBlob[outputManager.localNumberOfChannelElements()];
      channelSnowWaterEquivalent     = new double[outputManager.localNumberOfChannelElements()];

      if (0 < outputManager.maximumNumberOfChannelNeighbors())
        {
          channelSurfacewaterNeighborsExpirationTime = new double[outputManager.localNumberOfChannelElements() * outputManager.maximumNumberOfChannelNeighbors()];
          channelSurfacewaterNeighborsFlowRate       = new double[outputManager.localNumberOfChannelElements() * outputManager.maximumNumberOfChannelNeighbors()];
          channelSurfacewaterNeighborsFlowCumulative = new double[outputManager.localNumberOfChannelElements() * outputManager.maximumNumberOfChannelNeighbors()];
          channelGroundwaterNeighborsExpirationTime  = new double[outputManager.localNumberOfChannelElements() * outputManager.maximumNumberOfChannelNeighbors()];
          channelGroundwaterNeighborsFlowRate        = new double[outputManager.localNumberOfChannelElements() * outputManager.maximumNumberOfChannelNeighbors()];
          channelGroundwaterNeighborsFlowCumulative  = new double[outputManager.localNumberOfChannelElements() * outputManager.maximumNumberOfChannelNeighbors()];
        }
      else
        {
          channelSurfacewaterNeighborsExpirationTime = NULL;
          channelSurfacewaterNeighborsFlowRate       = NULL;
          channelSurfacewaterNeighborsFlowCumulative = NULL;
          channelGroundwaterNeighborsExpirationTime  = NULL;
          channelGroundwaterNeighborsFlowRate        = NULL;
          channelGroundwaterNeighborsFlowCumulative  = NULL;
        }
    }
  else
    {
      channelStateReceived                       = NULL;
      channelSurfacewaterDepth                   = NULL;
      channelSurfacewaterCreated                 = NULL;
      channelPrecipitationRate                   = NULL;
      channelPrecipitationCumulative             = NULL;
      channelEvaporationRate                     = NULL;
      channelEvaporationCumulative               = NULL;
      channelEvapoTranspirationState             = NULL;
      channelSnowWaterEquivalent                 = NULL;
      channelSurfacewaterNeighborsExpirationTime = NULL;
      channelSurfacewaterNeighborsFlowRate       = NULL;
      channelSurfacewaterNeighborsFlowCumulative = NULL;
      channelGroundwaterNeighborsExpirationTime  = NULL;
      channelGroundwaterNeighborsFlowRate        = NULL;
      channelGroundwaterNeighborsFlowCumulative  = NULL;
    }
}

OutputManager::TimePointState::~TimePointState()
{
  delete[] meshStateReceived;
  delete[] meshSurfacewaterDepth;
  delete[] meshSurfacewaterCreated;
  delete[] meshGroundwaterHead;
  delete[] meshGroundwaterRecharge;
  delete[] meshGroundwaterCreated;
  delete[] meshPrecipitationRate;
  delete[] meshPrecipitationCumulative;
  delete[] meshEvaporationRate;
  delete[] meshEvaporationCumulative;
  delete[] meshTranspirationRate;
  delete[] meshTranspirationCumulative;
  delete[] meshEvapoTranspirationState;
  delete[] meshCanopyWaterEquivalent;
  delete[] meshSnowWaterEquivalent;
  delete[] meshVadoseZoneState;
  delete[] meshRootZoneWater;
  delete[] meshTotalSoilWater;
  delete[] meshSurfacewaterNeighborsExpirationTime;
  delete[] meshSurfacewaterNeighborsFlowRate;
  delete[] meshSurfacewaterNeighborsFlowCumulative;
  delete[] meshGroundwaterNeighborsExpirationTime;
  delete[] meshGroundwaterNeighborsFlowRate;
  delete[] meshGroundwaterNeighborsFlowCumulative;
  delete[] channelStateReceived;
  delete[] channelSurfacewaterDepth;
  delete[] channelSurfacewaterCreated;
  delete[] channelPrecipitationRate;
  delete[] channelPrecipitationCumulative;
  delete[] channelEvaporationRate;
  delete[] channelEvaporationCumulative;
  delete[] channelEvapoTranspirationState;
  delete[] channelSnowWaterEquivalent;
  delete[] channelSurfacewaterNeighborsExpirationTime;
  delete[] channelSurfacewaterNeighborsFlowRate;
  delete[] channelSurfacewaterNeighborsFlowCumulative;
  delete[] channelGroundwaterNeighborsExpirationTime;
  delete[] channelGroundwaterNeighborsFlowRate;
  delete[] channelGroundwaterNeighborsFlowCumulative;
}

OutputManager::OutputManager(FileManager& fileManagerInit) :
fileManager(fileManagerInit)
{
  // No-op.
}

void OutputManager::createFiles()
{
  bool   error = false;       // Error flag.
  size_t ii;                  // Loop counter.
  size_t numberOfOutputFiles; // Total number of output files for the entire run.
  double outputTime;          // The time point of the next output file to create.

  if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      if (!(myOutputManagerIndex() < numberOfOutputManagers()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: myOutputManagerIndex must be less than numberOfOutputManagers.\n");
          error = true;
        }

      if (!(1721425.5 <= referenceDate()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: referenceDate must be on or after 1 CE (1721425.5).\n");
          error = true;
        }

      if (!(1721425.5 <= referenceDate() + (simulationStartTime() / (60.0 * 60.0 * 24.0))))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: the calendar date of simulationStartTime must be on or after 1 CE (1721425.5).\n");
          error = true;
        }

      if (!(0.0 <= simulationDuration()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: simulationDuration must be greater than or equal to zero.\n");
          error = true;
        }

      if (!(0.0 < outputPeriod()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: outputPeriod must be greater than zero.\n");
          error = true;
        }

      if (!(localNumberOfMeshElements() <= globalNumberOfMeshElements()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: localNumberOfMeshElements must be less than or equal to globalNumberOfMeshElements.\n");
          error = true;
        }

      if (!((0 != localNumberOfMeshElements() && localMeshElementStart() < globalNumberOfMeshElements()) || (0 == localNumberOfMeshElements() && 0 == localMeshElementStart())))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: localMeshElementStart must be less than globalNumberOfMeshElements or zero if localNumberOfMeshElements is zero.\n");
          error = true;
        }

      if (!(localNumberOfChannelElements() <= globalNumberOfChannelElements()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: localNumberOfChannelElements must be less than or equal to globalNumberOfChannelElements.\n");
          error = true;
        }

      if (!((0 != localNumberOfChannelElements() && localChannelElementStart() < globalNumberOfChannelElements()) || (0 == localNumberOfChannelElements() && 0 == localChannelElementStart())))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: localChannelElementStart must be less than globalNumberOfChannelElements or zero if localNumberOfChannelElements is zero.\n");
          error = true;
        }
    }

  if (!error)
    {
      // Calculate the number of output files to create.
      numberOfOutputFiles = std::ceil(simulationDuration() / outputPeriod());

      // If simulationDuration is zero or outputPeriod is infinity then numberOfOutputFiles could come out as zero.
      // In that case, we want to output once at the end of the simulation.
      if (0 == numberOfOutputFiles)
        {
          numberOfOutputFiles = 1;
        }
    }

  // Create all of the output files.  Each file will be created serially by one OutputManager with multiple files handled by multiple OutputManagers.
  for (ii = myOutputManagerIndex() + 1; !error && ii <= numberOfOutputFiles; ii += numberOfOutputManagers())
    {
      // Get the time point for the next output file to create.
      if (ii < numberOfOutputFiles)
        {
          outputTime = simulationStartTime() + outputPeriod() * ii;
        }
      else
        {
          // For the last file use simulationDuration rather than a multiple of outputPeriod.
          // simulationDuration might not be an exact multiple of outputPeriod, and even if it is, this avoids roundoff error.
          outputTime = simulationStartTime() + simulationDuration();
        }

      error = fileManager.createFile(outputTime);
    }

  if (error)
    {
      ADHYDRO_EXIT;
    }
}
