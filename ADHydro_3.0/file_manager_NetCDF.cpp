#include "file_manager_NetCDF.h"
#include "all.h"

bool FileManagerNetCDF::createFile(double referenceDate, double outputTime)
{
  bool error = false; // Error flag.

  if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      if (!(1721425.5 <= referenceDate))
        {
          ADHYDRO_ERROR("ERROR in FileManagerNetCDF::createFile: referenceDate must be on or after 1 CE (1721425.5).\n");
          error = true;
        }

      if (!(1721425.5 <= referenceDate + (outputTime / (60.0 * 60.0 * 24.0))))
        {
          ADHYDRO_ERROR("ERROR in FileManagerNetCDF::createFile: the calendar date of outputTime must be on or after 1 CE (1721425.5).\n");
          error = true;
        }
    }

  // FIXME implement
  ADHYDRO_PRINTF("Creating NetCDF file for referenceDate %lf, outputTime %lf.\n", referenceDate, outputTime);

  return error;
}
