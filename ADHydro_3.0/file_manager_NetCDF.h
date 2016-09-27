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
  FileManagerNetCDF(OutputManager& outputManagerInit) : outputManager(outputManagerInit) {}

  // FileManager interface.
  bool createFile(double outputTime);
  bool writeOutput(double outputTime, TimePointState* timePointState);

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
  // dimensionID1       - The ID of the first  dimension of the variable.
  // dimensionID2       - The ID of the second dimension of the variable.  Ignored if numberOfDimensions is less than two.
  // dimensionID3       - The ID of the third  dimension of the variable.  Ignored if numberOfDimensions is less than three.
  // units              - A units   string that will be added as an attribute of the variable.  Can be passed as NULL in which case no attribute will be added.
  // comment            - A comment string that will be added as an attribute of the variable.  Can be passed as NULL in which case no attribute will be added.
  bool createVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID1, int dimensionID2, int dimensionID3, const char* units, const char* comment);
  
  // Helper function to write a variable in a NetCDF file.
  //
  // Can write to a subset of the first dimenison, but always writes all values in higher dimensions of multi-dimensional arrays.
  // Therefore, start values of higher dimensions are always zero and count is set to the dimension size.
  // Dimension size is passed in.  It could be extracted from the file, but it would require several calls and complicate the code.
  // Dimension sizes are always avaialable when this function is called so we took the shortcut of just passing them in even though that is theoretically error prone.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID          - The ID of the NetCDF file.
  // variableName    - The name of the variable to write.
  // startDimension1 - The start index of the first dimenison.
  // countDimension1 - The number of indices to write in the first  dimension.
  // sizeDimension2  - The number of indices to write in the second dimension.  Start is always zero.  Ignored if variable is less than two   dimensions.
  // sizeDimension3  - The number of indices to write in the third  dimension.  Start is always zero.  Ignored if variable is less than three dimensions.
  // data            - The data to write.
  bool writeVariable(int fileID, const char* variableName, size_t startDimension1, size_t countDimension1, size_t sizeDimension2, size_t sizeDimension3, void* data);
  
  OutputManager& outputManager; // Wrapper for some values that FileManagerNetCDF needs to create NetCDF files.
};

#endif // __FILE_MANAGER_NETCDF_H__
