#include "output_manager.h"
#include "all.h"

OutputManager::MeshElementState& OutputManager::MeshElementState::operator=(const MeshElementState& other)
{
  elementNumber = other.elementNumber;
  outputIndex   = other.outputIndex;

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

OutputManager::ChannelElementState& OutputManager::ChannelElementState::operator=(const ChannelElementState& other)
{
  elementNumber = other.elementNumber;
  outputIndex   = other.outputIndex;

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

void OutputManager::handleInitialize()
{
  size_t ii;                                                                     // Loop counter.
  size_t numberOfOutputFiles = std::ceil(simulationDuration() / outputPeriod()); // Total number of output files for the entire run.

  if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      if (!(myOutputManagerIndex() < numberOfOutputManagers()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: myOutputManagerIndex must be less than numberOfOutputManagers.\n");
          ADHYDRO_EXIT;
        }

      if (!(1721425.5 <= referenceDate()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: referenceDate must be on or after 1 CE (1721425.5).\n");
          ADHYDRO_EXIT;
        }

      if (!(1721425.5 <= referenceDate() + (simulationStartTime() / (60.0 * 60.0 * 24.0))))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: the calendar date of simulationStartTime must be on or after 1 CE (1721425.5).\n");
          ADHYDRO_EXIT;
        }

      if (!(0.0 <= simulationDuration()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: simulationDuration must be greater than or equal to zero.\n");
          ADHYDRO_EXIT;
        }

      if (!(0.0 < outputPeriod()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: outputPeriod must be greater than zero.\n");
          ADHYDRO_EXIT;
        }

      if (!(0 < outputGroupSize()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: outputGroupSize must be greater than zero.\n");
          ADHYDRO_EXIT;
        }

      if (!(localNumberOfMeshElements() <= globalNumberOfMeshElements()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: localNumberOfMeshElements must be less than or equal to globalNumberOfMeshElements.\n");
          ADHYDRO_EXIT;
        }

      if (!((0 != localNumberOfMeshElements() && localMeshElementStart() < globalNumberOfMeshElements()) || (0 == localNumberOfMeshElements() && 0 == localMeshElementStart())))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: localMeshElementStart must be less than globalNumberOfMeshElements or zero if localNumberOfMeshElements is zero.\n");
          ADHYDRO_EXIT;
        }

      if (!(localNumberOfChannelElements() <= globalNumberOfChannelElements()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: localNumberOfChannelElements must be less than or equal to globalNumberOfChannelElements.\n");
          ADHYDRO_EXIT;
        }

      if (!((0 != localNumberOfChannelElements() && localChannelElementStart() < globalNumberOfChannelElements()) || (0 == localNumberOfChannelElements() && 0 == localChannelElementStart())))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleInitialize: localChannelElementStart must be less than globalNumberOfChannelElements or zero if localNumberOfChannelElements is zero.\n");
          ADHYDRO_EXIT;
        }
    }

  // If simulationDuration is zero or outputPeriod is infinity then numberOfOutputFiles could come out as zero.
  // In that case, we want to output once at the end of the simulation.
  if (0 == numberOfOutputFiles)
    {
      numberOfOutputFiles = 1;
    }

  outputData.assign(numberOfOutputFiles + 1, NULL);
  
  // If this outputManager has no elements create the empty TimePointStates now so that the pointers won't be NULL.
  if (0 == localNumberOfMeshElements() && 0 == localNumberOfChannelElements())
    {
      for (ii = 1; ii < outputData.size(); ++ii)
        {
          outputData[ii] = new FileManager::TimePointState(directory(), referenceDate(), calculateOutputTime(ii), globalNumberOfMeshElements(), localNumberOfMeshElements(), localMeshElementStart(),
              maximumNumberOfMeshSoilLayers(),  maximumNumberOfMeshNeighbors(), globalNumberOfChannelElements(), localNumberOfChannelElements(), localChannelElementStart(), maximumNumberOfChannelNeighbors());
        }
    }
}

void OutputManager::handleMeshElementState(const MeshElementState& state)
{
  size_t ii, jj;                                                     // Loop counters.
  size_t localIndex = state.elementNumber - localMeshElementStart(); // The array index in the TimePointState for this data.

  if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      if (!(localMeshElementStart() <= state.elementNumber && state.elementNumber < localMeshElementStart() + localNumberOfMeshElements()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleMeshElementState: OutputManager received state from a mesh element it does not own.\n");
          ADHYDRO_EXIT;
        }

      if (!(nextOutputIndex <= state.outputIndex && state.outputIndex < outputData.size()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleMeshElementState: outputIndex must be greater than or equal to nextOutputIndex and less than outputData.size().\n");
          ADHYDRO_EXIT;
        }

      if (!(state.numberOfSoilLayers <= maximumNumberOfMeshSoilLayers()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleMeshElementState: numberOfSoilLayers must be less than or equal to maximumNumberOfMeshSoilLayers.\n");
          ADHYDRO_EXIT;
        }

      if (!(state.numberOfNeighbors <= maximumNumberOfMeshNeighbors()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleMeshElementState: numberOfNeighbors must be less than or equal to maximumNumberOfMeshNeighbors.\n");
          ADHYDRO_EXIT;
        }
    }

  // If this is the first state received for this time point create a new TimePointState to hold it.
  if (NULL == outputData[state.outputIndex])
    {
      outputData[state.outputIndex] = new FileManager::TimePointState(directory(), referenceDate(), calculateOutputTime(state.outputIndex), globalNumberOfMeshElements(), localNumberOfMeshElements(), localMeshElementStart(),
          maximumNumberOfMeshSoilLayers(),  maximumNumberOfMeshNeighbors(), globalNumberOfChannelElements(), localNumberOfChannelElements(), localChannelElementStart(), maximumNumberOfChannelNeighbors());
    }

  // Copy the data into the right place.
  outputData[state.outputIndex]->meshStateReceived[          localIndex] = true;
  outputData[state.outputIndex]->meshSurfacewaterDepth[      localIndex] = state.surfacewaterDepth;
  outputData[state.outputIndex]->meshSurfacewaterCreated[    localIndex] = state.surfacewaterCreated;
  outputData[state.outputIndex]->meshPrecipitationRate[      localIndex] = state.precipitationRate;
  outputData[state.outputIndex]->meshPrecipitationCumulative[localIndex] = state.precipitationCumulative;
  outputData[state.outputIndex]->meshEvaporationRate[        localIndex] = state.evaporationRate;
  outputData[state.outputIndex]->meshEvaporationCumulative[  localIndex] = state.evaporationCumulative;
  outputData[state.outputIndex]->meshTranspirationRate[      localIndex] = state.transpirationRate;
  outputData[state.outputIndex]->meshTranspirationCumulative[localIndex] = state.transpirationCumulative;
  outputData[state.outputIndex]->meshEvapoTranspirationState[localIndex] = state.evapoTranspirationState;
  outputData[state.outputIndex]->meshCanopyWaterEquivalent[  localIndex] = state.canopyWaterEquivalent;
  outputData[state.outputIndex]->meshSnowWaterEquivalent[    localIndex] = state.snowWaterEquivalent;

  for (ii = 0; ii < state.numberOfSoilLayers; ++ii)
    {
      outputData[state.outputIndex]->meshGroundwaterHead[    localIndex * maximumNumberOfMeshSoilLayers() + ii] = state.groundwaterHead[ii];
      outputData[state.outputIndex]->meshGroundwaterRecharge[localIndex * maximumNumberOfMeshSoilLayers() + ii] = state.groundwaterRecharge[ii];
      outputData[state.outputIndex]->meshGroundwaterCreated[ localIndex * maximumNumberOfMeshSoilLayers() + ii] = state.groundwaterCreated[ii];
      outputData[state.outputIndex]->meshVadoseZoneState[    localIndex * maximumNumberOfMeshSoilLayers() + ii] = state.vadoseZoneState[ii];
      outputData[state.outputIndex]->meshRootZoneWater[      localIndex * maximumNumberOfMeshSoilLayers() + ii] = state.rootZoneWater[ii];
      outputData[state.outputIndex]->meshTotalSoilWater[     localIndex * maximumNumberOfMeshSoilLayers() + ii] = state.totalSoilWater[ii];

      for (jj = 0; jj < state.numberOfNeighbors; ++jj)
        {
          outputData[state.outputIndex]->meshGroundwaterNeighborsExpirationTime[(localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = state.groundwaterNeighborsExpirationTime[ii * state.numberOfNeighbors + jj];
          outputData[state.outputIndex]->meshGroundwaterNeighborsFlowRate[      (localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = state.groundwaterNeighborsFlowRate[      ii * state.numberOfNeighbors + jj];
          outputData[state.outputIndex]->meshGroundwaterNeighborsFlowCumulative[(localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = state.groundwaterNeighborsFlowCumulative[ii * state.numberOfNeighbors + jj];
        }

      // Fill in any empty cells at the end of the arrays.
      while (jj < maximumNumberOfMeshNeighbors())
        {
          outputData[state.outputIndex]->meshGroundwaterNeighborsExpirationTime[(localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = 0.0;
          outputData[state.outputIndex]->meshGroundwaterNeighborsFlowRate[      (localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = 0.0;
          outputData[state.outputIndex]->meshGroundwaterNeighborsFlowCumulative[(localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = 0.0;
          ++jj;
        }
    }

  // Fill in any empty cells at the end of the arrays.
  while (ii < maximumNumberOfMeshSoilLayers())
    {
      outputData[state.outputIndex]->meshGroundwaterHead[    localIndex * maximumNumberOfMeshSoilLayers() + ii] = 0.0;
      outputData[state.outputIndex]->meshGroundwaterRecharge[localIndex * maximumNumberOfMeshSoilLayers() + ii] = 0.0;
      outputData[state.outputIndex]->meshGroundwaterCreated[ localIndex * maximumNumberOfMeshSoilLayers() + ii] = 0.0;
      // outputData[state.outputIndex]->meshVadoseZoneState[    localIndex * maximumNumberOfMeshSoilLayers() + ii] = FIXME how to write "blank" to a data blob?
      outputData[state.outputIndex]->meshRootZoneWater[      localIndex * maximumNumberOfMeshSoilLayers() + ii] = 0.0;
      outputData[state.outputIndex]->meshTotalSoilWater[     localIndex * maximumNumberOfMeshSoilLayers() + ii] = 0.0;

      for (jj = 0; jj < maximumNumberOfMeshNeighbors(); ++jj)
        {
          outputData[state.outputIndex]->meshGroundwaterNeighborsExpirationTime[(localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = 0.0;
          outputData[state.outputIndex]->meshGroundwaterNeighborsFlowRate[      (localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = 0.0;
          outputData[state.outputIndex]->meshGroundwaterNeighborsFlowCumulative[(localIndex * maximumNumberOfMeshSoilLayers() + ii) * maximumNumberOfMeshNeighbors() + jj] = 0.0;
        }

      ++ii;
    }

  for (ii = 0; ii < state.numberOfNeighbors; ++ii)
    {
      outputData[state.outputIndex]->meshSurfacewaterNeighborsExpirationTime[localIndex * maximumNumberOfMeshNeighbors() + ii] = state.surfacewaterNeighborsExpirationTime[ii];
      outputData[state.outputIndex]->meshSurfacewaterNeighborsFlowRate[      localIndex * maximumNumberOfMeshNeighbors() + ii] = state.surfacewaterNeighborsFlowRate[ii];
      outputData[state.outputIndex]->meshSurfacewaterNeighborsFlowCumulative[localIndex * maximumNumberOfMeshNeighbors() + ii] = state.surfacewaterNeighborsFlowCumulative[ii];
    }

  // Fill in any empty cells at the end of the arrays.
  while (ii < maximumNumberOfMeshNeighbors())
    {
      outputData[state.outputIndex]->meshSurfacewaterNeighborsExpirationTime[localIndex * maximumNumberOfMeshNeighbors() + ii] = 0.0;
      outputData[state.outputIndex]->meshSurfacewaterNeighborsFlowRate[      localIndex * maximumNumberOfMeshNeighbors() + ii] = 0.0;
      outputData[state.outputIndex]->meshSurfacewaterNeighborsFlowCumulative[localIndex * maximumNumberOfMeshNeighbors() + ii] = 0.0;
      ++ii;
    }
}

void OutputManager::handleChannelElementState(const ChannelElementState& state)
{
  size_t ii;                                                            // Loop counter.
  size_t localIndex = state.elementNumber - localChannelElementStart(); // The array index in the TimePointState for this data.

  if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      if (!(localChannelElementStart() <= state.elementNumber && state.elementNumber < localChannelElementStart() + localNumberOfChannelElements()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleChannelElementState: OutputManager received state from a channel element it does not own.\n");
          ADHYDRO_EXIT;
        }

      if (!(nextOutputIndex <= state.outputIndex && state.outputIndex < outputData.size()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleChannelElementState: outputIndex must be greater than or equal to nextOutputIndex and less than outputData.size().\n");
          ADHYDRO_EXIT;
        }

      if (!(state.numberOfNeighbors <= maximumNumberOfChannelNeighbors()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::handleChannelElementState: numberOfNeighbors must be less than or equal to maximumNumberOfChannelNeighbors.\n");
          ADHYDRO_EXIT;
        }
    }

  // If this is the first state received for this time point create a new TimePointState to hold it.
  if (NULL == outputData[state.outputIndex])
    {
      outputData[state.outputIndex] = new FileManager::TimePointState(directory(), referenceDate(), calculateOutputTime(state.outputIndex), globalNumberOfMeshElements(), localNumberOfMeshElements(), localMeshElementStart(),
          maximumNumberOfMeshSoilLayers(),  maximumNumberOfMeshNeighbors(), globalNumberOfChannelElements(), localNumberOfChannelElements(), localChannelElementStart(), maximumNumberOfChannelNeighbors());
    }

  // Copy the data into the right place.
  outputData[state.outputIndex]->channelStateReceived[          localIndex] = true;
  outputData[state.outputIndex]->channelSurfacewaterDepth[      localIndex] = state.surfacewaterDepth;
  outputData[state.outputIndex]->channelSurfacewaterCreated[    localIndex] = state.surfacewaterCreated;
  outputData[state.outputIndex]->channelPrecipitationRate[      localIndex] = state.precipitationRate;
  outputData[state.outputIndex]->channelPrecipitationCumulative[localIndex] = state.precipitationCumulative;
  outputData[state.outputIndex]->channelEvaporationRate[        localIndex] = state.evaporationRate;
  outputData[state.outputIndex]->channelEvaporationCumulative[  localIndex] = state.evaporationCumulative;
  outputData[state.outputIndex]->channelEvapoTranspirationState[localIndex] = state.evapoTranspirationState;
  outputData[state.outputIndex]->channelSnowWaterEquivalent[    localIndex] = state.snowWaterEquivalent;

  for (ii = 0; ii < state.numberOfNeighbors; ++ii)
    {
      outputData[state.outputIndex]->channelSurfacewaterNeighborsExpirationTime[localIndex * maximumNumberOfChannelNeighbors() + ii] = state.surfacewaterNeighborsExpirationTime[ii];
      outputData[state.outputIndex]->channelSurfacewaterNeighborsFlowRate[      localIndex * maximumNumberOfChannelNeighbors() + ii] = state.surfacewaterNeighborsFlowRate[ii];
      outputData[state.outputIndex]->channelSurfacewaterNeighborsFlowCumulative[localIndex * maximumNumberOfChannelNeighbors() + ii] = state.surfacewaterNeighborsFlowCumulative[ii];
      outputData[state.outputIndex]->channelGroundwaterNeighborsExpirationTime[ localIndex * maximumNumberOfChannelNeighbors() + ii] = state.groundwaterNeighborsExpirationTime[ii];
      outputData[state.outputIndex]->channelGroundwaterNeighborsFlowRate[       localIndex * maximumNumberOfChannelNeighbors() + ii] = state.groundwaterNeighborsFlowRate[ii];
      outputData[state.outputIndex]->channelGroundwaterNeighborsFlowCumulative[ localIndex * maximumNumberOfChannelNeighbors() + ii] = state.groundwaterNeighborsFlowCumulative[ii];
    }

  // Fill in any empty cells at the end of the arrays.
  while (ii < maximumNumberOfChannelNeighbors())
    {
      outputData[state.outputIndex]->channelSurfacewaterNeighborsExpirationTime[localIndex * maximumNumberOfChannelNeighbors() + ii] = 0.0;
      outputData[state.outputIndex]->channelSurfacewaterNeighborsFlowRate[      localIndex * maximumNumberOfChannelNeighbors() + ii] = 0.0;
      outputData[state.outputIndex]->channelSurfacewaterNeighborsFlowCumulative[localIndex * maximumNumberOfChannelNeighbors() + ii] = 0.0;
      outputData[state.outputIndex]->channelGroundwaterNeighborsExpirationTime[ localIndex * maximumNumberOfChannelNeighbors() + ii] = 0.0;
      outputData[state.outputIndex]->channelGroundwaterNeighborsFlowRate[       localIndex * maximumNumberOfChannelNeighbors() + ii] = 0.0;
      outputData[state.outputIndex]->channelGroundwaterNeighborsFlowCumulative[ localIndex * maximumNumberOfChannelNeighbors() + ii] = 0.0;
      ++ii;
    }
}

bool OutputManager::readyToOutput()
{
  size_t ii, jj;       // Loop counters.
  bool   ready = true; // Return value.  Stays true until we find something that is not ready.

  // We are going to output the next outputGroupSize time points together so we are only ready if they are all ready.
  for (ii = nextOutputIndex; ready && ii < nextOutputIndex + outputGroupSize() && ii < outputData.size(); ++ii)
    {
      // If a TimePointState hasn't been created yet it is not ready.
      ready = (NULL != outputData[ii]);

      // Check that all mesh element data has been received.
      for (jj = 0; ready && jj < localNumberOfMeshElements(); ++jj)
        {
          ready = outputData[ii]->meshStateReceived[jj];
        }

      // Check that all channel element data has been received.
      for (jj = 0; ready && jj < localNumberOfChannelElements(); ++jj)
        {
          ready = outputData[ii]->channelStateReceived[jj];
        }
    }

  return ready;
}

double OutputManager::calculateOutputTime(size_t outputIndex)
{
  double outputTime; // Return value.

  if (outputIndex < outputData.size() - 1)
    {
      outputTime = simulationStartTime() + outputPeriod() * outputIndex;
    }
  else
    {
      // For the last file use simulationDuration rather than a multiple of outputPeriod.
      // simulationDuration might not be an exact multiple of outputPeriod, and even if it is, this avoids roundoff error.
      outputTime = simulationStartTime() + simulationDuration();
    }

  return outputTime;
}
