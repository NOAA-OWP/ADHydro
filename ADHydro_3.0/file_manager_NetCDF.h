#ifndef __FILE_MANAGER_NETCDF_H__
#define __FILE_MANAGER_NETCDF_H__

#include "output_manager.h"
#include <netcdf.h>

// Implementation of FileManager interface for NetCDF files.
class FileManagerNetCDF : public FileManager
{
public:

  // Constructor.
  //
  // Parameters:
  //
  // outputManagerInit - Provides some values that the FileManagerNetCDF needs to create NetCDF files.  Must exist for the entire lifetime of the FileManagerNetCDF,
  FileManagerNetCDF(OutputManager& outputManagerInit);

  // FileManager interface.
  bool createFile(double outputTime);

private:

  // Helper function to create a standard filename string for a given time.
  //
  // Returns: the filename.
  //
  // Parameters:
  //
  // outputTime - Time point to create a filename for.
  std::string createFilename(double outputTime);

  // Helper function to create a variable with units and comment in a NetCDF file.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID             - The ID of the NetCDF file.
  // variableName       - The name of the variable to create.
  // dataType           - The type of the variable to create.
  // numberOfDimensions - The number of dimensions of the variable to create.  Must be one to three.
  // dimensionID0       - The ID of the first  dimension of the variable.
  // dimensionID1       - The ID of the second dimension of the variable.  Ignored if numberOfDimensions is less than two.
  // dimensionID2       - The ID of the third  dimension of the variable.  Ignored if numberOfDimensions is less than three.
  // units              - A units   string that will be added as an attribute of the variable.  Can be passed as NULL in which case no attribute will be added.
  // comment            - A comment string that will be added as an attribute of the variable.  Can be passed as NULL in which case no attribute will be added.
  bool createVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID0, int dimensionID1, int dimensionID2, const char* units, const char* comment);

  OutputManager& outputManager; // Wrapper for some values that FileManagerNetCDF needs to create NetCDF files.
};

#endif // __FILE_MANAGER_NETCDF_H__
