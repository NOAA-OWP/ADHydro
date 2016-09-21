#include "output_manager_charm.h"
#include "adhydro.h"

OutputManagerCharm::OutputManagerCharm(FileManagerEnum fileManagerInit) :
OutputManager(convertFileManagerEnumToFileManagerReference(fileManagerInit)),
fileManagerNetCDFInit(*this)
{
  // No-op.
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
  return 3; // FIXME
}

size_t OutputManagerCharm::localNumberOfChannelElements()
{
  return 3; // FIXME
}

size_t OutputManagerCharm::localChannelElementStart()
{
  return 0; // FIXME
}

size_t OutputManagerCharm::maximumNumberOfChannelNeighbors()
{
  return 2; // FIXME
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
