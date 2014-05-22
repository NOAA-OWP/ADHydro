#include <mpi.h>
#include "file_manager.h"
#include "all.h"
// FIXME netcdf_par.h needed for netcdf built with parallel I/O, but doesn't exist in netcdf built without parallel I/O
//#include <netcdf_par.h>
#include <netcdf.h>
#include <assert.h>

FileManager::FileManager()
{
  // Initialize file statuses to closed.
  geometryFileStatus  = FILE_MANAGER_STATUS_CLOSED;
  parameterFileStatus = FILE_MANAGER_STATUS_CLOSED;
  stateFileStatus     = FILE_MANAGER_STATUS_CLOSED;

  // Begin structured dagger infinite loop.
  thisProxy[CkMyPe()].runForever();
}

void FileManager::handleOpenFiles(int directoryLength, char* directory, int numberOfMeshElementsInit, FileManagerActionEnum geometryAction,
                                  int geometryIdentifier, FileManagerActionEnum parameterAction, int parameterIdentifier, FileManagerActionEnum stateAction,
                                  int stateIdentifier)
{
  bool    error      = false;                 // Error flag.
  int     ii;                                 // Loop counter.
  char*   nameString = NULL;                  // Temporary string for file and group names.
  int     ncErrorCode;                        // Return value of NetCDF functions.
  int     geometryNumberOfMeshElementsDimID;  // NetCDF Dimension ID in geometry file for number of mesh elements.
  int     geometryNumberOfMeshNodesDimID;     // NetCDF Dimension ID in geometry file for number of mesh nodes.
  int     geometryThreeDimID;                 // NetCDF Dimension ID in geometry file for dimension size of three.
                                              // Used for three vertices for each element and three coordinates (X, Y, Z) for each node.
  int     parameterNumberOfMeshElementsDimID; // NetCDF Dimension ID in parameter file for number of mesh elements.
  int     stateNumberOfMeshElementsDimID;     // NetCDF Dimension ID in state file for number of mesh elements.
  nc_type varType;                            // For passing types of variables.
  int     numDims;                            // For passing number of dimensions of variables.
  int     dimIDs[NC_MAX_VAR_DIMS];            // For passing dimension IDs of variables.
  size_t  dimSizes[NC_MAX_VAR_DIMS];          // For passing size of dimensions of variables.
  char    unusedName[NC_MAX_NAME + 1];        // For passing names of variables.
  int     unusedNumAttributes;                // For passing number of attributes of variables.
  int     numberOfMeshNodes;                  // The number of mesh nodes in the data items.
                                              // The number of mesh nodes in all data items of all files must be the same.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleOpenFiles: directory must not be NULL.\n");
      error = true;
    }
  // Cast to long long to avoid comparison between signed and unsigned without losing range.
  else if (!((long long)strlen(directory) + 1 <= (long long)directoryLength))
    {
      CkError("ERROR in FileManager::handleOpenFiles: "
              "directoryLength must be at least as long as string length of directory including null termination character.\n");
      error = true;
    }

  if ((FILE_MANAGER_WRITE  == geometryAction  || FILE_MANAGER_CREATE == geometryAction || FILE_MANAGER_WRITE  == parameterAction ||
       FILE_MANAGER_CREATE == parameterAction || FILE_MANAGER_WRITE  == stateAction    || FILE_MANAGER_CREATE == stateAction) &&
      !(0 < numberOfMeshElementsInit))
    {
      CkError("ERROR in FileManager::handleOpenFiles: if you are creating or writing to any files numberOfMeshElementsInit must be greater than zero.\n");
      error = true;
    }

  if (!(FILE_MANAGER_NO_ACTION == geometryAction || FILE_MANAGER_READ == geometryAction || FILE_MANAGER_WRITE == geometryAction ||
        FILE_MANAGER_CREATE    == geometryAction))
    {
      CkError("ERROR in FileManager::handleOpenFiles: geometryAction must be a valid enum value.\n");
      error = true;
    }

  if (!(FILE_MANAGER_NO_ACTION == parameterAction || FILE_MANAGER_READ == parameterAction || FILE_MANAGER_WRITE == parameterAction ||
        FILE_MANAGER_CREATE    == parameterAction))
    {
      CkError("ERROR in FileManager::handleOpenFiles: parameterAction must be a valid enum value.\n");
      error = true;
    }

  if (!(FILE_MANAGER_NO_ACTION == stateAction || FILE_MANAGER_READ == stateAction || FILE_MANAGER_WRITE == stateAction ||
        FILE_MANAGER_CREATE    == stateAction))
    {
      CkError("ERROR in FileManager::handleOpenFiles: stateAction must be a valid enum value.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBIC_FUNCTIONS_SIMPLE)

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // File access must follow the pattern open-close-open-close.  This is enforced in the structured dagger code so files must be closed at this point.
  assert(FILE_MANAGER_STATUS_CLOSED == geometryFileStatus && FILE_MANAGER_STATUS_CLOSED == parameterFileStatus &&
         FILE_MANAGER_STATUS_CLOSED == stateFileStatus);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // Minus one here indicates that the number of mesh elements and nodes is currently unknown.  It might be determined by numberOfMeshElementsInit if files are
  // being created or opened for write, or it might be determined by reading from files if files are being opened for read.  If it can be determined from
  // multiple sources it is an error if they are not all the same.
  numberOfMeshElements = -1;
  numberOfMeshNodes    = -1;

  // FIXME make directory?

  // Open or create file.
  if (!error)
    {
      nameString = new char[directoryLength + 11]; // Longest string concatenated to directory is "/parameter" whose length is 10.
                                                   // The maximum length of an int printed as a string is 11.

      // We are not using snprintf or doing error checking on the return value of sprintf.  If the previous call to new fails it will throw an exception, which
      // will be unhandled and cause the program to exit.  If the call to new succeeds as well as the error check comparing the value of directoryLength to the
      // string length of directory then it is guaranteed there is enough room in nameString.
      sprintf(nameString, "%s/geometry.nc", directory);

      switch (geometryAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &geometryFileID);
        break;
      case FILE_MANAGER_WRITE:
        ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &geometryFileID);
        break;
      case FILE_MANAGER_CREATE:
        ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &geometryFileID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  (FILE_MANAGER_CREATE == geometryAction) ? "create" : "open", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          switch (geometryAction)
          {
          // case FILE_MANAGER_NO_ACTION:
            // Do nothing.
            // break;
          case FILE_MANAGER_READ:
            geometryFileStatus = FILE_MANAGER_STATUS_OPEN_FOR_READ;
            break;
          case FILE_MANAGER_WRITE:
            // Fallthrough.
          case FILE_MANAGER_CREATE:
            geometryFileStatus = FILE_MANAGER_STATUS_OPEN_FOR_READ_WRITE;
            break;
          }
        }
    }

  // Get ID of or create group.
  if (!error)
    {
      sprintf(nameString, "%d", geometryIdentifier);

      switch (geometryAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_inq_ncid(geometryFileID, nameString, &geometryGroupID);
        break;
      case FILE_MANAGER_WRITE:
        // Fallthrough.
      case FILE_MANAGER_CREATE:
        ncErrorCode = nc_def_grp(geometryFileID, nameString, &geometryGroupID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s group %s in NetCDF geometry file.  NetCDF error message: %s.\n",
                  (FILE_MANAGER_READ == geometryAction) ? "find" : "create", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create dimensions.
  if (FILE_MANAGER_WRITE == geometryAction || FILE_MANAGER_CREATE == geometryAction)
    {
      if (!error)
        {
          ncErrorCode = nc_def_dim(geometryGroupID, "numberOfMeshElements", numberOfMeshElementsInit, &geometryNumberOfMeshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension numberOfMeshElements in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_def_dim(geometryGroupID, "numberOfMeshNodes", numberOfMeshElementsInit * 3, &geometryNumberOfMeshNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension numberOfMeshNodes in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_def_dim(geometryGroupID, "three", 3, &geometryThreeDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension three in NetCDF geometry file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Get ID of or create variables.
  if (!error)
    {
      switch (geometryAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_inq_varid(geometryGroupID, "meshElementNodeIndices", &meshElementNodeIndicesVarID);
        break;
      case FILE_MANAGER_WRITE:
        // Fallthrough.
      case FILE_MANAGER_CREATE:
        dimIDs[0] = geometryNumberOfMeshElementsDimID;
        dimIDs[1] = geometryThreeDimID;
        dimSizes[0] = numberOfMeshElementsInit;
        dimSizes[1] = 3;
        ncErrorCode = nc_def_var(geometryGroupID, "meshElementNodeIndices", NC_INT, 2, dimIDs, &meshElementNodeIndicesVarID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s variable meshElementNodeIndices in NetCDF geometry file.  NetCDF error message: %s.\n",
                  (FILE_MANAGER_READ == geometryAction) ? "find" : "create", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // If you are reading make sure the variable has the right type and number of dimensions.
  if (FILE_MANAGER_READ == geometryAction)
    {
      if (!error)
        {
          ncErrorCode = nc_inq_var(geometryGroupID, meshElementNodeIndicesVarID, unusedName, &varType, &numDims, dimIDs, &unusedNumAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to get information about variable meshElementNodeIndices in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
          else
            {
              if (!(NC_INT == varType))
                {
                  CkError("ERROR in FileManager::handleOpenFiles: wrong type for variable meshElementNodeIndices in NetCDF geometry file.  %d should be %d.",
                          varType, NC_INT);
                  error = true;
                }

              if (!(2 == numDims))
                {
                  CkError("ERROR in FileManager::handleOpenFiles: wrong number of dimensions for variable meshElementNodeIndices in NetCDF geometry file.  "
                          "%d should be 2.", numDims);
                  error = true;
                }
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      // Read the size of the variable.
      for (ii = 0; !error && ii < 2; ii++)
        {
          ncErrorCode = nc_inq_dimlen(geometryGroupID, dimIDs[ii], &dimSizes[ii]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to get information about dimension %d of variable meshElementNodeIndices "
                      "in NetCDF geometry file.  NetCDF error message: %s.\n", ii, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Check that the size of the variable is consistent with other variables.
  if (!error && FILE_MANAGER_NO_ACTION != geometryAction)
    {
      // If the number of mesh elements is not yet determined get it from the size of this variable.
      if (-1 == numberOfMeshElements)
        {
          numberOfMeshElements = dimSizes[0];
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)numberOfMeshElements == (long long)dimSizes[0] && 3 == dimSizes[1]))
        {
          CkError("ERROR in FileManager::handleOpenFiles: wrong size of variable meshElementNodeIndices in NetCDF geometry file.  %dx%d should be %dx3.\n",
                  dimSizes[0], dimSizes[1], numberOfMeshElements);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Get ID of or create variables.
  if (!error)
    {
      switch (geometryAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_inq_varid(geometryGroupID, "meshNodeXYZSurfaceCoordinates", &meshNodeXYZSurfaceCoordinatesVarID);
        break;
      case FILE_MANAGER_WRITE:
        // Fallthrough.
      case FILE_MANAGER_CREATE:
        dimIDs[0] = geometryNumberOfMeshNodesDimID;
        dimIDs[1] = geometryThreeDimID;
        dimSizes[0] = numberOfMeshElementsInit * 3;
        dimSizes[1] = 3;
        ncErrorCode = nc_def_var(geometryGroupID, "meshNodeXYZSurfaceCoordinates", NC_DOUBLE, 2, dimIDs, &meshNodeXYZSurfaceCoordinatesVarID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", (FILE_MANAGER_READ == geometryAction) ? "find" : "create", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // If you are reading make sure the variable has the right type and number of dimensions.
  if (FILE_MANAGER_READ == geometryAction)
    {
      if (!error)
        {
          ncErrorCode = nc_inq_var(geometryGroupID, meshNodeXYZSurfaceCoordinatesVarID, unusedName, &varType, &numDims, dimIDs, &unusedNumAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to get information about variable meshNodeXYZSurfaceCoordinates "
                      "in NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
          else
            {
              if (!(NC_DOUBLE == varType))
                {
                  CkError("ERROR in FileManager::handleOpenFiles: wrong type for variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                          "%d should be %d.", varType, NC_DOUBLE);
                  error = true;
                }

              if (!(2 == numDims))
                {
                  CkError("ERROR in FileManager::handleOpenFiles: wrong number of dimensions for variable meshNodeXYZSurfaceCoordinates "
                          "in NetCDF geometry file.  %d should be 2.", numDims);
                  error = true;
                }
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      // Read the size of the variable.
      for (ii = 0; !error && ii < 2; ii++)
        {
          ncErrorCode = nc_inq_dimlen(geometryGroupID, dimIDs[ii], &dimSizes[ii]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to get information about dimension %d of variable meshNodeXYZSurfaceCoordinates "
                      "in NetCDF geometry file.  NetCDF error message: %s.\n", ii, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Check that the size of the variable is consistent with other variables.
  if (!error && FILE_MANAGER_NO_ACTION != geometryAction)
    {
      // If the number of mesh nodes is not yet determined get it from the size of this variable.
      if (-1 == numberOfMeshNodes)
        {
          numberOfMeshNodes = dimSizes[0];
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)numberOfMeshNodes == (long long)dimSizes[0] && 3 == dimSizes[1]))
        {
          CkError("ERROR in FileManager::handleOpenFiles: wrong size of variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                  "%dx%d should be %dx3.\n", dimSizes[0], dimSizes[1], numberOfMeshNodes);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Get ID of or create variables.
  if (!error)
    {
      switch (geometryAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_inq_varid(geometryGroupID, "meshNodeZBedrockCoordinates", &meshNodeZBedrockCoordinatesVarID);
        break;
      case FILE_MANAGER_WRITE:
        // Fallthrough.
      case FILE_MANAGER_CREATE:
        dimIDs[0] = geometryNumberOfMeshNodesDimID;
        dimSizes[0] = numberOfMeshElementsInit * 3;
        ncErrorCode = nc_def_var(geometryGroupID, "meshNodeZBedrockCoordinates", NC_DOUBLE, 1, dimIDs, &meshNodeZBedrockCoordinatesVarID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s variable meshNodeZBedrockCoordinates in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", (FILE_MANAGER_READ == geometryAction) ? "find" : "create", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // If you are reading make sure the variable has the right type and number of dimensions.
  if (FILE_MANAGER_READ == geometryAction)
    {
      if (!error)
        {
          ncErrorCode = nc_inq_var(geometryGroupID, meshNodeZBedrockCoordinatesVarID, unusedName, &varType, &numDims, dimIDs, &unusedNumAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to get information about variable meshNodeZBedrockCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
          else
            {
              if (!(NC_DOUBLE == varType))
                {
                  CkError("ERROR in FileManager::handleOpenFiles: wrong type for variable meshNodeZBedrockCoordinates in NetCDF geometry file.  "
                          "%d should be %d.", varType, NC_DOUBLE);
                  error = true;
                }

              if (!(1 == numDims))
                {
                  CkError("ERROR in FileManager::handleOpenFiles: wrong number of dimensions for variable meshNodeZBedrockCoordinates "
                          "in NetCDF geometry file.  %d should be 1.", numDims);
                  error = true;
                }
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      // Read the size of the variable.
      if (!error)
        {
          ncErrorCode = nc_inq_dimlen(geometryGroupID, dimIDs[0], &dimSizes[0]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to get information about dimension 0 of variable meshNodeZBedrockCoordinates "
                      "in NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Check that the size of the variable is consistent with other variables.
  if (!error && FILE_MANAGER_NO_ACTION != geometryAction)
    {
      // If the number of mesh nodes is not yet determined get it from the size of this variable.
      if (-1 == numberOfMeshNodes)
        {
          numberOfMeshNodes = dimSizes[0];
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)numberOfMeshNodes == (long long)dimSizes[0]))
        {
          CkError("ERROR in FileManager::handleOpenFiles: wrong size of variable meshNodeZBedrockCoordinates in NetCDF geometry file.  "
                  "%d should be %d.\n", dimSizes[0], numberOfMeshNodes);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Open or create file.
  if (!error)
    {
      sprintf(nameString, "%s/parameter.nc", directory);

      switch (parameterAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &parameterFileID);
        break;
      case FILE_MANAGER_WRITE:
        ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &parameterFileID);
        break;
      case FILE_MANAGER_CREATE:
        ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &parameterFileID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  (FILE_MANAGER_CREATE == parameterAction) ? "create" : "open", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          switch (parameterAction)
          {
          // case FILE_MANAGER_NO_ACTION:
            // Do nothing.
            // break;
          case FILE_MANAGER_READ:
            parameterFileStatus = FILE_MANAGER_STATUS_OPEN_FOR_READ;
            break;
          case FILE_MANAGER_WRITE:
            // Fallthrough.
          case FILE_MANAGER_CREATE:
            parameterFileStatus = FILE_MANAGER_STATUS_OPEN_FOR_READ_WRITE;
            break;
          }
        }
    }

  // Get ID of or create group.
  if (!error)
    {
      sprintf(nameString, "%d", parameterIdentifier);

      switch (parameterAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_inq_ncid(parameterFileID, nameString, &parameterGroupID);
        break;
      case FILE_MANAGER_WRITE:
        // Fallthrough.
      case FILE_MANAGER_CREATE:
        ncErrorCode = nc_def_grp(parameterFileID, nameString, &parameterGroupID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s group %s in NetCDF parameter file.  NetCDF error message: %s.\n",
                  (FILE_MANAGER_READ == parameterAction) ? "find" : "create", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create dimensions.
  if (!error && (FILE_MANAGER_WRITE == parameterAction || FILE_MANAGER_CREATE == parameterAction))
    {
      ncErrorCode = nc_def_dim(parameterGroupID, "numberOfMeshElements", numberOfMeshElementsInit, &parameterNumberOfMeshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension numberOfMeshElements in NetCDF parameter file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Open or create file.
  if (!error)
    {
      sprintf(nameString, "%s/state.nc", directory);

      switch (stateAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &stateFileID);
        break;
      case FILE_MANAGER_WRITE:
        ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &stateFileID);
        break;
      case FILE_MANAGER_CREATE:
        ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &stateFileID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s NetCDF state file %s.  NetCDF error message: %s.\n",
                  (FILE_MANAGER_CREATE == stateAction) ? "create" : "open", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          switch (stateAction)
          {
          // case FILE_MANAGER_NO_ACTION:
            // Do nothing.
            // break;
          case FILE_MANAGER_READ:
            stateFileStatus = FILE_MANAGER_STATUS_OPEN_FOR_READ;
            break;
          case FILE_MANAGER_WRITE:
            // Fallthrough.
          case FILE_MANAGER_CREATE:
            stateFileStatus = FILE_MANAGER_STATUS_OPEN_FOR_READ_WRITE;
            break;
          }
        }
    }

  // Get ID of or create group.
  if (!error)
    {
      sprintf(nameString, "%d", stateIdentifier);

      switch (stateAction)
      {
      case FILE_MANAGER_NO_ACTION:
        // Do nothing.
        ncErrorCode = NC_NOERR;
        break;
      case FILE_MANAGER_READ:
        ncErrorCode = nc_inq_ncid(stateFileID, nameString, &stateGroupID);
        break;
      case FILE_MANAGER_WRITE:
        // Fallthrough.
      case FILE_MANAGER_CREATE:
        ncErrorCode = nc_def_grp(stateFileID, nameString, &stateGroupID);
        break;
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to %s group %s in NetCDF state file.  NetCDF error message: %s.\n",
                  (FILE_MANAGER_READ == stateAction) ? "find" : "create", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create dimensions.
  if (!error && (FILE_MANAGER_WRITE == stateAction || FILE_MANAGER_CREATE == stateAction))
    {
      ncErrorCode = nc_def_dim(stateGroupID, "numberOfMeshElements", numberOfMeshElementsInit, &stateNumberOfMeshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension numberOfMeshElements in NetCDF state file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Delete dynamically allocated string.
  if (NULL != nameString)
    {
      delete[] nameString;
    }

  if (!error)
    {
      contribute();
    }
  else
    {
      CkExit();
    }
}

void FileManager::handleCloseFiles()
{
  bool error = false; // Error flag.
  int  ncErrorCode;   // Return value of NetCDF functions.

  // Attempt to close all open files even if there is an error.
  if (FILE_MANAGER_STATUS_CLOSED != geometryFileStatus)
    {
      ncErrorCode        = nc_close(geometryFileID);
      geometryFileStatus = FILE_MANAGER_STATUS_CLOSED;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCloseFiles: unable to close NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (FILE_MANAGER_STATUS_CLOSED != parameterFileStatus)
    {
      ncErrorCode         = nc_close(parameterFileID);
      parameterFileStatus = FILE_MANAGER_STATUS_CLOSED;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCloseFiles: unable to close NetCDF parameter file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (FILE_MANAGER_STATUS_CLOSED != stateFileStatus)
    {
      ncErrorCode     = nc_close(stateFileID);
      stateFileStatus = FILE_MANAGER_STATUS_CLOSED;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCloseFiles: unable to close NetCDF state file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      contribute();
    }
  else
    {
      CkExit();
    }
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "file_manager.def.h"
#pragma GCC diagnostic warning "-Wunused-variable"
#pragma GCC diagnostic warning "-Wsign-compare"
