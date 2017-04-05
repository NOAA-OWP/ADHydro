#ifndef __FILE_MANAGER_NETCDF_H__
#define __FILE_MANAGER_NETCDF_H__

#include "time_point_state.h"
#include <netcdf.h>

// FileManagerNetCDF wraps some static functions for writing NetCDF files.
class FileManagerNetCDF
{
public:
    
    // Write a TimePointState out to a NetCDF file.  writeState does collective parallel I/O so you must call it from all processors simultaneously,
    // one call per processor, all outputing to the same file.  It will block until all processors attempt to open the same file.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outputTime     - (s) The value of currentTime at which the values in timePointState were saved.
    // timePointState - The state to write out to file.
    static bool writeState(double outputTime, const TimePointState& timePointState);
    
private:
    
    // Helper function to create a variable with units and comment in a NetCDF file.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // fileID             - The ID of the NetCDF file.
    // variableName       - The name of the variable to create.
    // dataType           - The type of the variable to create.
    // numberOfDimensions - The number of dimensions of the variable to create.  Must be one or two.
    // dimensionID1       - The ID of the first  dimension of the variable.
    // dimensionID2       - The ID of the second dimension of the variable.  Ignored if numberOfDimensions is less than two.
    // units              - A units   string that will be added as an attribute of the variable.  Can be passed as NULL in which case no attribute will be added.
    // comment            - A comment string that will be added as an attribute of the variable.  Can be passed as NULL in which case no attribute will be added.
    static bool createVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID1, int dimensionID2, const char* units, const char* comment);
    
    // Helper function to write a variable in a NetCDF file.
    //
    // Can write to a subset of the first dimenison, but always writes all values in higher dimensions of multi-dimensional arrays.
    // Therefore, start values of higher dimensions are always zero and count is set to the dimension size.
    // Dimension size is passed in.  It could be extracted from the file, but it would require several calls and complicate the code.
    // Dimension sizes are always avaialable when this function is called so we took the shortcut of just passing them in.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // fileID          - The ID of the NetCDF file.
    // variableName    - The name of the variable to write.
    // startDimension1 - The start index of the first dimenison.
    // countDimension1 - The number of indices to write in the first  dimension.
    // sizeDimension2  - The number of indices to write in the second dimension.  Start is always zero.  Ignored if variable has less than two dimensions.
    // data            - The data to write.
    static bool writeVariable(int fileID, const char* variableName, size_t startDimension1, size_t countDimension1, size_t sizeDimension2, void* data);
};

#endif // __FILE_MANAGER_NETCDF_H__
