#include "output_manager_charm.h"
#include "adhydro.h"

OutputManagerCharm::OutputManagerCharm(FileManagerEnum fileManagerInit) :
OutputManager((FILE_MANAGER_NETCDF == fileManagerInit) ? &fileManagerNetCDFInit :
    NULL) // NULL will cause an error in the OutputManager constructor if fileManagerInit is not a valid enum value.
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

#include "output_manager_charm.def.h"
