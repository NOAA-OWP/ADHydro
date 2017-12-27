#include "netcdf_io_ops.h"
#include <sys/stat.h>
#include <libgen.h>
#include <cstdlib> //FIXME update code to new/delete/strings
#include <cstring> //FIXME ^^
namespace adhydro
{
    namespace io
    {
	namespace netcdf
	{

	    bool NetCDFOpenForRead(const char* path, int* fileID)
	    {
	      bool error = false; // Error flag.
	      int  ncErrorCode;   // Return value of NetCDF functions.
	      
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      if (!(NULL != path))
		{
		  CkError("ERROR in NetCDFOpenForRead: path must not be null.\n");
		  error = true;
		}
	      
	      if (!(NULL != fileID))
		{
		  CkError("ERROR in NetCDFOpenForRead: fileID must not be null.\n");
		  error = true;
		}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

	      if (!error)
		{
		  ncErrorCode = nc_open_par(path, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD /* FIXME this isn't working NetCDFMPIComm */, MPI_INFO_NULL, fileID);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFOpenForRead: could not open for read NetCDF file %s.  NetCDF error message: %s.\n", path,
			      nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}

	      return error;
	    }
	    
	    
	    bool NetCDFCreateOrOpenForWrite(const char* path, int* fileID, bool* created)
	    {
	      bool        error     = false; // Error flag.
	      int         ncErrorCode;       // Return value of NetCDF functions.
	      char*       tmpPath   = NULL;  // Temporary string for string manipulation.
	      char*       directory = NULL;  // Directory where file is located.
	      struct stat fileStatus;        // Used to detect whether files and directories exist.
	      
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      if (!(NULL != path))
		{
		  CkError("ERROR in NetCDFCreateOrOpenForWrite: path must not be null.\n");
		  error = true;
		}
	      
	      if (!(NULL != fileID))
		{
		  CkError("ERROR in NetCDFCreateOrOpenForWrite: fileID must not be null.\n");
		  error = true;
		}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      
	      if (NULL != created)
		{
		  *created = false;
		}

	      if (!error)
		{
		  // FIXLATER this works for Linux only
		  // Create the directory if it doesn't already exist.  The dirname function may modify the string passed to it so we first need to create a copy of path to
		  // pass to dirname.
		  tmpPath = (char*)malloc(strlen(path) + 1);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NULL != tmpPath))
		    {
		      CkError("ERROR in NetCDFCreateOrOpenForWrite: could not allocate memory for tmpPath.\n");
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      if (!error)
		{
		  strcpy(tmpPath, path);
		  directory = dirname(tmpPath);
		  
		  if (0 != stat(directory, &fileStatus))
		    {
		      // Directory does not exist.  Create it.
		      // FIXME does not create directory hierarchies.
		      if (0 != mkdir(directory, 0777))
			{
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
			  CkError("ERROR in NetCDFCreateOrOpenForWrite: Cannot create directory %s.\n", directory);
			  error = true;
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
			}
		    }
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  else if (S_IFDIR != (fileStatus.st_mode & S_IFMT))
		    {
		      // File exists, but is not directory.  Error.
		      CkError("ERROR in NetCDFCreateOrOpenForWrite: Cannot create file %s because %s already exists, but is not a directory.\n", path,
			      directory);
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      if (NULL != tmpPath)
		{
		  free(tmpPath);
		}
	      
	      if (!error)
		{
		  // At this point the directory exists.  Try to open the file for write, and if that fails try to create the file.
		  ncErrorCode = nc_open_par(path, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD /* FIXME this isn't working NetCDFMPIComm */, MPI_INFO_NULL, fileID);
		  
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      ncErrorCode = nc_create_par(path, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD /* FIXME this isn't working NetCDFMPIComm */, MPI_INFO_NULL, fileID);
		  
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		      if (!(NC_NOERR == ncErrorCode))
			{
			  CkError("ERROR in NetCDFCreateOrOpenForWrite: could not create or open for write NetCDF file %s.  NetCDF error message: %s.\n",
				  path, nc_strerror(ncErrorCode));
			  error = true;
			}
		      else
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
			{
			  if (NULL != created)
			    {
			      *created = true;
			    }
			}
		    }
		}
	      
	      return error;
	    }


	    bool NetCDFCreateDimension(int fileID, const char* dimensionName, size_t dimensionSize, int* dimensionID)
	    {
	      bool error = false; // Error flag.
	      int  ncErrorCode;   // Return value of NetCDF functions.
	      
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      if (!(NULL != dimensionName))
		{
		  CkError("ERROR in NetCDFCreateDimension: dimensionName must not be null.\n");
		  error = true;
		}
	      
	      if (!(NULL != dimensionID))
		{
		  CkError("ERROR in NetCDFCreateDimension: dimensionID must not be null.\n");
		  error = true;
		}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

	      if (!error)
		{
		  ncErrorCode = nc_def_dim(fileID, dimensionName, dimensionSize, dimensionID);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFCreateDimension: unable to create dimension %s in NetCDF file.  NetCDF error message: %s.\n", dimensionName,
			      nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
		
	      return error;
	    }
	    
	    bool NetCDFReadDimensionSize(int fileID, const char* dimensionName, size_t* dimensionSize)
	    {
	      bool error = false; // Error flag.
	      int  ncErrorCode;   // Return value of NetCDF functions.
	      int  dimensionID;   // ID of dimension in NetCDF file.
	      
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      if (!(NULL != dimensionName))
		{
		  CkError("ERROR in NetCDFReadDimensionSize: dimensionName must not be null.\n");
		  error = true;
		}
	      
	      if (!(NULL != dimensionSize))
		{
		  CkError("ERROR in NetCDFReadDimensionSize: dimensionSize must not be null.\n");
		  error = true;
		}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      
	      // Get the dimension ID.
	      if (!error)
		{
		  ncErrorCode = nc_inq_dimid(fileID, dimensionName, &dimensionID);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFReadDimensionSize: unable to get dimension %s in NetCDF file.  NetCDF error message: %s.\n", dimensionName,
			      nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      // Get the dimension length.
	      if (!error)
		{
		  ncErrorCode = nc_inq_dimlen(fileID, dimensionID, dimensionSize);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFReadDimensionSize: unable to get length of dimension %s in NetCDF file.  NetCDF error message: %s.\n",
			      dimensionName, nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      return error;
	    }
	    
	    bool NetCDFCreateVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID0, int dimensionID1,
                                       int dimensionID2, const char* units, const char* comment)
	    {
	      bool error = false;                 // Error flag.
	      int  ncErrorCode;                   // Return value of NetCDF functions.
	      int  dimensionIDs[NC_MAX_VAR_DIMS]; // For passing dimension IDs.
	      int  variableID;                    // For getting variable ID.
	      
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      if (!(NULL != variableName))
		{
		  CkError("ERROR in NetCDFCreateVariable: variableName must not be null.\n");
		  error = true;
		}
	      
	      if (!(1 <= numberOfDimensions && 3 >= numberOfDimensions))
		{
		  CkError("ERROR in NetCDFCreateVariable: numberOfDimensions must be greater than or equal to one and less than or equal to three.\n");
		  error = true;
		}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      
	      if (!error)
		{
		  dimensionIDs[0] = dimensionID0;
		  dimensionIDs[1] = dimensionID1;
		  dimensionIDs[2] = dimensionID2;

		  ncErrorCode = nc_def_var(fileID, variableName, dataType, numberOfDimensions, dimensionIDs, &variableID);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFCreateVariable: unable to create variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
			      nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      if (!error && NULL != units)
		{
		  ncErrorCode = nc_put_att_text(fileID, variableID, "units", strlen(units), units);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFCreateVariable: unable to create attribute units of variable %s in NetCDF file.  NetCDF error message: %s.\n",
			      variableName, nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      if (!error && NULL != comment)
		{
		  ncErrorCode = nc_put_att_text(fileID, variableID, "comment", strlen(comment), comment);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFCreateVariable: unable to create attribute comment of variable %s in NetCDF file.  NetCDF error message: %s.\n",
			      variableName, nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      return error;
	    }
	    
	    bool NetCDFWriteVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart, size_t numberOfNodesElements,
                                      size_t memoryDimension, void* variable)
	    {
	      bool   error = false;          // Error flag.
	      int    ncErrorCode;            // Return value of NetCDF functions.
	      int    variableID;             // ID of variable in NetCDF file.
	      size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
	      size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
	      
	    #if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      if (!(NULL != variableName))
		{
		  CkError("ERROR in NetCDFWriteVariable: variableName must not be null.\n");
		  error = true;
		}
	      
	      if (!(1 <= numberOfNodesElements))
		{
		  CkError("ERROR in NetCDFWriteVariable: numberOfNodesElements must be greater than or equal to one.\n");
		  error = true;
		}
	      
	      if (!(1 <= memoryDimension))
		{
		  CkError("ERROR in NetCDFWriteVariable: memoryDimension must be greater than or equal to one.\n");
		  error = true;
		}
	      
	      if (!(NULL != variable))
		{
		  CkError("ERROR in NetCDFWriteVariable: variable must not be null.\n");
		  error = true;
		}
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
	      
	      // Get the variable ID.
	      if (!error)
		{
		  ncErrorCode = nc_inq_varid(fileID, variableName, &variableID);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFWriteVariable: unable to get variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
			      nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      // Set collective access.
	      if (!error)
		{
		  ncErrorCode = nc_var_par_access(fileID, variableID, NC_COLLECTIVE);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFWriteVariable: unable to set collective access for variable %s in NetCDF file.  NetCDF error message: %s.\n",
			      variableName, nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}

	      if (!error)
		{
		  // Fill in the start and count of the dimensions.
		  start[0] = instance;
		  start[1] = nodeElementStart;
		  start[2] = 0;
		  count[0] = 1;
		  count[1] = numberOfNodesElements;
		  count[2] = memoryDimension;
		  
		  // Write the variable.
		  ncErrorCode = nc_put_vara(fileID, variableID, start, count, variable);

	    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		  if (!(NC_NOERR == ncErrorCode))
		    {
		      CkError("ERROR in NetCDFWriteVariable: unable to write variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
			      nc_strerror(ncErrorCode));
		      error = true;
		    }
	    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
		}
	      
	      return error;
	    }
	  
	} //End netcdf namespace
    } //end IO namespace
} //end ADHydro namespace