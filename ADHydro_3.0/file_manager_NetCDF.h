#ifndef __FILE_MANAGER_NETCDF_H__
#define __FILE_MANAGER_NETCDF_H__

#include "time_point_state.h"
#include <netcdf.h>

// FileManagerNetCDF wraps some static functions for writing NetCDF files.
class FileManagerNetCDF
{
public:
    
    // Read a variable from a NetCDF file.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // fileID                - The file ID of the NetCDF file.
    // variableName          - The name of the variable to read.
    // instance              - The index of the first  dimension to read.  The count of the first dimension is always one.  This reads one particular instance in time.
    // nodeElementStart      - The index of the second dimension to read.  This is ignored if the variable has less than two dimensions.
    // numberOfNodesElements - The count of the second dimension to read.  If the variable has less than two dimensions this must be one.
    //                         nodeElementStart and numberOfNodesElements combine to specify a subset of the nodes or elements stored in the variable.
    // fileDimension         - The count of the third  dimension to read.  The index of the third dimension is always zero.
    //                         If the variable has less than three dimensions this must be one.
    // memoryDimension       - The size of the third dimension in memory.  This function can read an array whose third dimension in the file is smaller than the
    //                         desired third dimension in memory.  In that case, what gets filled in to the extra cells depends on repeatLastValue and defaultValue.
    //                         It is an error if memoryDimension is less than fileDimension.  If the variable has less than three dimensions this must be one.
    // repeatLastValue       - If there are extra cells to be filled in because memoryDimension is greater than fileDimension then if repeatLastValue is true, the last value in
    //                         each row of the third dimension in the file is repeated in the extra cells.  If repeatLastValue is false, the extra cells are filled in with defaultValue.
    // defaultValue          - If there are extra cells to be filled in because memoryDimension is greater than fileDimension then if repeatLastValue is false, the extra cells are
    //                         filled in with defaultValue.  If repeatLastValue is true, defaultValue is ignored.
    // mandatory             - Whether the existence of the variable is mandatory.  If true, it is an error if the variable does not exist.
    //                         If false, this function does nothing if the variable does not exist.
    // variable              - A pointer passed by reference.  The pointer (that is, *variable) may point to an array of size numberOfNodesElements * fileDimension,
    //                         which is the size of the array that will be read, or it can be NULL.  If it is NULL it will be set to point to a newly allocated array.
    //                         This array, whether passed in or newly allocated, is filled in with the values read from the NetCDF file.  Then, if memoryDimension is
    //                         greater than fileDimension it is reallocated to the larger size with new[] and delete[].  NOTE: even if you pass in an array, it will be
    //                         deleted and *variable will be set to point to a newly allocated array if memoryDimension is greater than fileDimension.  Therefore, the
    //                         array you pass in must be allocated with new[].  Reallocation will only happen if memoryDimension is greater than fileDimension.
    //                         In any case, *variable will wind up pointing to an array of size numberOfNodesElements * memoryDimension.
    template <typename T> static bool readVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart, size_t numberOfNodesElements,
                                                   size_t fileDimension, size_t memoryDimension, bool repeatLastValue, T defaultValue, bool mandatory, T** variable);
    
    // Write a TimePointState out to a NetCDF file.  writeState does collective parallel I/O so you must call it from all processors simultaneously,
    // one call per processor, all outputing to the same file.  It will block until all processors attempt to open the same file.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // checkpointTime - (s) The value of currentTime at which the values in timePointState were saved.
    // timePointState - The state to write out to file.
    static bool writeState(double checkpointTime, const TimePointState& timePointState);
    
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
