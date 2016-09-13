#ifndef __FILE_MANAGER_NETCDF_H__
#define __FILE_MANAGER_NETCDF_H__

#include "file_manager.h"

// Implementation of FileManager interface for NetCDF files.
class FileManagerNetCDF : public FileManager
{
public:

  // FileManager interface.
  bool createFile(double referenceDate, double outputTime);
};

#endif // __FILE_MANAGER_NETCDF_H__
