#ifndef __NETCDF_IO_OPS_H__
#define __NETCDF_IO_OPS_H__
#include <netcdf_par.h>
#include <netcdf.h>
#include "../../all_charm.h" //TODO should we macro the CkError to an error function that prints to std::err for non charm++ code?
namespace adhydro
{
    namespace io
    {
	namespace netcdf
	{
	    // Open a NetCDF file in read-only mode.
	    //
	    // Returns: true if there is an error, false otherwise.
	    //
	    // Parameters:
	    //
	    // path   - String containing the file path including filename of the file to
	    //          open.
	    // fileID - Scalar passed by reference will be filled in with an ID number
	    //          generated by the NetCDF library for the file.  Pass this number
	    //          in to other NetCDF calls to use this file.
	    bool NetCDFOpenForRead(const char* path, int* fileID);
	    
	    // If path exists open it as a NetCDF file in read-write mode.  If path does
	    // not exist create it as a new NetCDF file, which will then be open in
	    // read-write mode.
	    //
	    // Returns: true if there is an error, false otherwise.
	    //
	    // Parameters:
	    //
	    // path    - String containing the file path including filename of the file
	    //           to create or open.
	    // fileID  - Scalar passed by reference will be filled in with an ID number
	    //           generated by the NetCDF library for the file.  Pass this number
	    //           in to other NetCDF calls to use this file.
	    // created - Scalar passed by reference will be filled in with true if the
	    //           file was created and false if it was merely opened.  Can be
	    //           passed as NULL in which case it will be ignored.
	    
	    bool NetCDFCreateOrOpenForWrite(const char* path, int* fileID, bool* created);
	    
	    // Create a dimension in a NetCDF file.
	    //
	    // Returns: true if there is an error, false otherwise.
	    //
	    // Parameters:
	    //
	    // fileID        - The file ID of the NetCDF file.
	    // dimensionName - The name of the dimension to create.
	    // dimensionSize - The size of the dimension to create.
	    // dimensionID   - Scalar passed by reference will be filled in with the ID
	    //                 of the created dimension.
	    bool NetCDFCreateDimension(int fileID, const char* dimensionName, size_t dimensionSize, int* dimensionID);
	    
	    // Get the size of a dimension in a NetCDF file.
	    //
	    // Returns: true if there is an error, false otherwise.
	    //
	    // Parameters:
	    //
	    // fileID        - The file ID of the NetCDF file.
	    // dimensionName - The name of the dimension to get the size of.
	    // dimensionSize - Scalar passed by reference will be filled in with the size
	    //                 of the dimension.
	    bool NetCDFReadDimensionSize(int fileID, const char* dimensionName, size_t* dimensionSize);
	    
	    // Create a variable in a NetCDF file.
	    //
	    // Returns: true if there is an error, false otherwise.
	    //
	    // Parameters:
	    //
	    // fileID             - The file ID of the NetCDF file.
	    // variableName       - The name of the variable to create.
	    // dataType           - The type of the variable to create.
	    // numberOfDimensions - The number of dimensions of the variable to create.
	    // dimensionID0       - The ID of the first dimension of the variable.
	    // dimensionID1       - The ID of the second dimension of the variable.
	    //                      Ignored if numberOfDimensions is less than two.
	    // dimensionID2       - The ID of the third dimension of the variable.
	    //                      Ignored if numberOfDimensions is less than three.
	    // units              - A units string that will be added as an attribute of
	    //                      the variable.  Can be passed as NULL in which case no
	    //                      attribute will be added.
	    // comment            - A comment string that will be added as an attribute
	    //                      of the variable.  Can be passed as NULL in which case
	    //                      no attribute will be added.
	    bool NetCDFCreateVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID0, int dimensionID1,
				      int dimensionID2, const char* units, const char* comment);
	    
	    // Write a variable to a NetCDF file.
	    //
	    // Returns: true if there is an error, false otherwise.
	    //
	    // Parameters:
	    //
	    // fileID                - The file ID of the NetCDF file.
	    // variableName          - The name of the variable to write.
	    // instance              - The index of the first dimension to write.
	    //                         The count of the first dimension is always one.
	    //                         This writes one particular instance in time.
	    // nodeElementStart      - The index of the second dimension to write.
	    // numberOfNodesElements - The count of the second dimension to write.
	    //                         nodeElementStart and numberOfNodesElements combine
	    //                         to specify a subset of the nodes or elements
	    //                         stored in the variable.  They are ignored if the
	    //                         variable has less than two dimensions.
	    // memoryDimension       - The count of the third dimension to write.  This
	    //                         is ignored if the variable has less than three
	    //                         dimensions.
	    // variable              - An array of size 1 * numberOfNodesElements *
	    //                         memoryDimension which will be written into the
	    //                         variable in the file.
	    bool NetCDFWriteVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart, size_t numberOfNodesElements,
				    size_t memoryDimension, void* variable);
	    	    
	    // Read a variable from a NetCDF file.
	    //
	    // Returns: true if there is an error, false otherwise.
	    //
	    // Parameters:
	    //
	    // fileID                - The file ID of the NetCDF file.
	    // variableName          - The name of the variable to read.
	    // instance              - The index of the first dimension to read.
	    //                         The count of the first dimension is always one.
	    //                         This reads one particular instance in time.
	    // nodeElementStart      - The index of the second dimension to read.  This
	    //                         is ignored if the variable has less than two
	    //                         dimensions.
	    // numberOfNodesElements - The count of the second dimension to read.  If the
	    //                         variable has less than two dimensions this must be
	    //                         one. nodeElementStart and numberOfNodesElements
	    //                         combine to specify a subset of the nodes or
	    //                         elements stored in the variable.
	    // fileDimension         - The count of the third dimension to read.  The
	    //                         index of the third dimension is always zero.  If
	    //                         the variable has less than three dimensions this
	    //                         must be one.
	    // memoryDimension       - The size of the third dimension in memory.  This
	    //                         function can read an array whose third dimension
	    //                         in the file is smaller than the desired third
	    //                         dimension in memory.  In that case, what gets
	    //                         filled in to the extra cells depends on
	    //                         repeatLastValue and defaultValue.  It is an error
	    //                         if memoryDimension is less than fileDimension.  If
	    //                         the variable has less than three dimensions this
	    //                         must be one.
	    // repeatLastValue       - If there are extra cells to be filled in because
	    //                         memoryDimension is greater than fileDimension then
	    //                         if repeatLastValue is true the last value in each
	    //                         row of the third dimension in the file is repeated
	    //                         in the extra cells.  If repeatLastValue is false,
	    //                         defaultValue is used instead.
	    // defaultValue          - If there are extra cells to be filled in because
	    //                         memoryDimension is greater than fileDimension then
	    //                         if repeatLastValue is false the extra cells are
	    //                         filled in with defaultValue.  If repeatLastValue
	    //                         is true defaultValue is ignored.
	    // mandatory             - Whether the existence of the variable is
	    //                         mandatory.  If true, it is an error if the
	    //                         variable does not exist.  If false, this function
	    //                         does nothing if the variable does not exist.
	    // variable              - A pointer passed by reference.  The pointer (that
	    //                         is, *variable) may point to an array of size 1 *
	    //                         numberOfNodesElements * fileDimension, which is
	    //                         the size of the array that will be read, or it can
	    //                         be NULL.  If it is NULL it will be set to point to
	    //                         a newly allocated array.  This array, whether
	    //                         passed in or newly allocated is filled in with the
	    //                         values read from the NetCDF file.  Then, if
	    //                         memoryDimension is greater than fileDimension it
	    //                         is reallocated to the larger size with new[] and
	    //                         delete[].  NOTE: even if you pass in an array, it
	    //                         will be deleted and *variable will be set to point
	    //                         to a newly allocated array if memoryDimension is
	    //                         greater than fileDimension.  Therefore, the array
	    //                         you pass in must be allocated with new, but this
	    //                         will only happen if memoryDimension is greater
	    //                         than fileDimension.  In any case, *variable will
	    //                         wind up pointing to an array of size 1 *
	    //                         numberOfNodesElements * memoryDimension.
	    template <typename T> bool NetCDFReadVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart,
							  size_t numberOfNodesElements, size_t fileDimension, size_t memoryDimension, bool repeatLastValue,
							  T defaultValue, bool mandatory, T** variable)
	    {/*Defined here in the header so that proper template instansiation occurs*/
	      bool   error = false;          // Error flag.
	      size_t ii, jj;                 // Loop counters.
	      int    ncErrorCode;            // Return value of NetCDF functions.
	      int    variableID;             // ID of variable in NetCDF file.
	      size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
	      size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
	      T*     tempVariable;           // For remapping arrays when fileDimension is smaller than memoryDimension
	      
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      if (!(NULL != variableName))
		{
		  CkError("ERROR in NetCDFReadVariable: variableName must not be null.\n");
		  error = true;
		}
	      
	      if (!(1 <= numberOfNodesElements))
		{
		  CkError("ERROR in NetCDFReadVariable: numberOfNodesElements must be greater than or equal to one.\n");
		  error = true;
		}
	      
	      if (!(1 <= memoryDimension))
		{
		  CkError("ERROR in NetCDFReadVariable: memoryDimension must be greater than or equal to one.\n");
		  error = true;
		}
	      
	      if (!(NULL != variable))
		{
		  CkError("ERROR in NetCDFReadVariable: variable must not be null.\n");
		  error = true;
		}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
	      // fileDimenison must be less than or equal to memoryDimension.  Otherwise there is not enough room to read all of the data and it is an error.
	      if (!(1 <= fileDimension && fileDimension <= memoryDimension))
		{
		  CkError("ERROR in NetCDFReadVariable: fileDimension must be greater than or equal to one and less than or equal to memoryDimension for "
			  "variable %s in NetCDF file.\n", variableName);
		  error = true;
		}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
	      
	      if (!error)
		{
		  // Get the variable ID.
		  ncErrorCode = nc_inq_varid(fileID, variableName, &variableID);

		  if (!(NC_NOERR == ncErrorCode))
		    {
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		      // If the variable does not exist it is only an error if the variable is mandatory.
		      if (mandatory)
			{
			  CkError("ERROR in NetCDFReadVariable: unable to get variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
				  nc_strerror(ncErrorCode));
			  error = true;
			}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		    }
		  else // If the variable does exist get its data.
		    {
		      // Fill in the start and count of the dimensions.
		      start[0] = instance;
		      start[1] = nodeElementStart;
		      start[2] = 0;
		      count[0] = 1;
		      count[1] = numberOfNodesElements;
		      count[2] = fileDimension;

		      // Allocate space if needed.
		      if (NULL == *variable)
			{
			  *variable = new T[numberOfNodesElements * fileDimension];
			}
		      
		      // Get the variable data.
		      ncErrorCode = nc_get_vara(fileID, variableID, start, count, *variable);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		      if (!(NC_NOERR == ncErrorCode))
			{
			  CkError("ERROR in NetCDFReadVariable: unable to read variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
				  nc_strerror(ncErrorCode));
			  error = true;
			}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		      
		      // If fileDimenison is less than memoryDimension we need to remap the array and fill in the extra elements.
		      if (!error && fileDimension < memoryDimension)
			{
			  // Allocate a new array of the right size for memoryDimension.
			  tempVariable = new T[numberOfNodesElements * memoryDimension];
			  
			  for (ii = 0; ii < numberOfNodesElements; ii++)
			    {
			      for (jj = 0; jj < fileDimension; jj++)
				{
				  // Fill in the values up to fileDimension that were read from the file.
				  tempVariable[ii * memoryDimension + jj] = (*variable)[ii * fileDimension + jj];
				}
			      
			      for(jj = fileDimension; jj < memoryDimension; jj++)
				{
				  if (repeatLastValue)
				    {
				      // Fill in the rest of the values by repeating the last value read from the file.
				      tempVariable[ii * memoryDimension + jj] = (*variable)[ii * fileDimension + fileDimension - 1];
				    }
				  else
				    {
				      // Fill in the rest of the values with defaultValue.
				      tempVariable[ii * memoryDimension + jj] = defaultValue;
				    }
				}
			    }
			  
			  // Delete the wrong size array read in from file and set variable to point to the right size array.
			  delete[] *variable;
			  *variable = tempVariable;
			}
		    } // End if the variable does exist get its data.
		} // End if (!error).
	      
	      return error;
	    }
	    
	    
	} //End netcdf namespace
    } //end IO namespace
} //end ADHydro namespace

#endif