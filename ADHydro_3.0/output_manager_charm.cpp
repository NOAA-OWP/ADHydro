#include "adhydro.h"

PUPbytes(FileManager::EvapoTranspirationStateBlob);
PUPbytes(FileManager::VadoseZoneStateBlob);

void MeshElementState::pup(PUP::er &p)
{
  p | elementNumber;
  p | currentTime;
  p | numberOfSoilLayers;
  p | numberOfNeighbors;

  if (p.isUnpacking())
    {
      deleteArrays();
      allocateArrays();
    }

  p | surfacewaterDepth;
  p | surfacewaterCreated;

  if (0 < numberOfSoilLayers)
    {
      PUParray(p, groundwaterHead,     numberOfSoilLayers);
      PUParray(p, groundwaterRecharge, numberOfSoilLayers);
      PUParray(p, groundwaterCreated,  numberOfSoilLayers);
    }

  p | precipitationRate;
  p | precipitationCumulative;
  p | evaporationRate;
  p | evaporationCumulative;
  p | transpirationRate;
  p | transpirationCumulative;
  p | evapoTranspirationState;
  p | canopyWaterEquivalent;
  p | snowWaterEquivalent;

  if (0 < numberOfSoilLayers)
    {
      PUParray(p, vadoseZoneState, numberOfSoilLayers);
      PUParray(p, rootZoneWater,   numberOfSoilLayers);
      PUParray(p, totalSoilWater,  numberOfSoilLayers);
    }

  if (0 < numberOfNeighbors)
    {
      PUParray(p, surfacewaterNeighborsExpirationTime, numberOfNeighbors);
      PUParray(p, surfacewaterNeighborsFlowRate,       numberOfNeighbors);
      PUParray(p, surfacewaterNeighborsFlowCumulative, numberOfNeighbors);

      if (0 < numberOfSoilLayers)
        {
          PUParray(p, groundwaterNeighborsExpirationTime, numberOfNeighbors * numberOfSoilLayers);
          PUParray(p, groundwaterNeighborsFlowRate,       numberOfNeighbors * numberOfSoilLayers);
          PUParray(p, groundwaterNeighborsFlowCumulative, numberOfNeighbors * numberOfSoilLayers);
        }
    }
}

void ChannelElementState::pup(PUP::er &p)
{
  p | elementNumber;
  p | currentTime;
  p | numberOfNeighbors;

  if (p.isUnpacking())
    {
      deleteArrays();
      allocateArrays();
    }

  p | surfacewaterDepth;
  p | surfacewaterCreated;
  p | precipitationRate;
  p | precipitationCumulative;
  p | evaporationRate;
  p | evaporationCumulative;
  p | evapoTranspirationState;
  p | snowWaterEquivalent;

  if (0 < numberOfNeighbors)
    {
      PUParray(p, surfacewaterNeighborsExpirationTime, numberOfNeighbors);
      PUParray(p, surfacewaterNeighborsFlowRate,       numberOfNeighbors);
      PUParray(p, surfacewaterNeighborsFlowCumulative, numberOfNeighbors);
      PUParray(p, groundwaterNeighborsExpirationTime,  numberOfNeighbors);
      PUParray(p, groundwaterNeighborsFlowRate,        numberOfNeighbors);
      PUParray(p, groundwaterNeighborsFlowCumulative,  numberOfNeighbors);
    }
}

size_t OutputManagerCharm::numberOfOutputManagers()
{
  return CkNumPes();
}

size_t OutputManagerCharm::myOutputManagerIndex()
{
  return CkMyPe();
}

std::string OutputManagerCharm::directory()
{
  return ADHydro::adhydroOutputDirectoryPath;
}

double OutputManagerCharm::referenceDate()
{
  return ADHydro::referenceDate;
}

double OutputManagerCharm::simulationStartTime()
{
  return ADHydro::simulationStartTime;
}

double OutputManagerCharm::simulationDuration()
{
  return ADHydro::simulationDuration;
}

double OutputManagerCharm::outputPeriod()
{
  return ADHydro::outputPeriod;
}

size_t OutputManagerCharm::globalNumberOfMeshElements()
{
  return 5; // FIXME
}

size_t OutputManagerCharm::localNumberOfMeshElements()
{
  return 5; // FIXME
}

size_t OutputManagerCharm::localMeshElementStart()
{
  return 0; // FIXME
}

size_t OutputManagerCharm::maximumNumberOfMeshSoilLayers()
{
  return 1; // FIXME
}

size_t OutputManagerCharm::maximumNumberOfMeshNeighbors()
{
  return 2; // FIXME
}

size_t OutputManagerCharm::globalNumberOfChannelElements()
{
  return 4; // FIXME
}

size_t OutputManagerCharm::localNumberOfChannelElements()
{
  return 4; // FIXME
}

size_t OutputManagerCharm::localChannelElementStart()
{
  return 0; // FIXME
}

size_t OutputManagerCharm::maximumNumberOfChannelNeighbors()
{
  return 3; // FIXME
}

FileManager& OutputManagerCharm::convertFileManagerEnumToFileManagerReference(FileManagerEnum fileManagerInit)
{
  FileManager* fileManager;

  switch (fileManagerInit)
  {
  case FILE_MANAGER_NETCDF:
    fileManager = &fileManagerNetCDFInit;
    break;
  default:
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      {
        CkError("ERROR in OutputManagerCharm::convertFileManagerEnumToFileManagerReference: fileManagerInit must be a valid enum value.\n");
        CkExit();
      }

    fileManager = &fileManagerNetCDFInit;
    break;
  }

  return *fileManager;
}

#include "output_manager_charm.def.h"
