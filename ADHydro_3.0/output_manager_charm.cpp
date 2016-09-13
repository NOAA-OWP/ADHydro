#include "output_manager_charm.h"
#include "adhydro.h"

OutputManagerCharm::OutputManagerCharm(FileManagerEnum fileManagerInit) :
OutputManager((FILE_MANAGER_NETCDF == fileManagerInit) ? &fileManagerNetCDFInit :
    NULL), // NULL will cause an error in the OutputManager constructor if fileManagerInit is not a valid enum value.
    fileManagerNetCDFInit(this)
{
  // no-op.
}

int OutputManagerCharm::numberOfOutputManagers()
{
  return CkNumPes();
}

int OutputManagerCharm::myOutputManagerIndex()
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

int OutputManagerCharm::globalNumberOfMeshElements()
{
  return 5; // FIXME
}

int OutputManagerCharm::numberOfMeshSoilLayers()
{
  return 1; // FIXME
}

int OutputManagerCharm::numberOfMeshNeighbors()
{
  return 2; // FIXME
}

int OutputManagerCharm::globalNumberOfChannelElements()
{
  return 3; // FIXME
}

int OutputManagerCharm::numberOfChannelNeighbors()
{
  return 2; // FIXME
}

#include "output_manager_charm.def.h"
