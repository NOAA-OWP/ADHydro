#include <mpi.h>
#include "file_manager.h"
#include "all.h"
#include <netcdf_par.h>

FileManager::FileManager()
{
  // Initialize file statuses to closed.
  geometryFileStatus  = CLOSED;
  parameterFileStatus = CLOSED;
  stateFileStatus     = CLOSED;

  // Begin structured dagger infinite loop.
  thisProxy[CkMyPe()].runForever();
}

// Suppress warning enum value not handled in switch.
#pragma GCC diagnostic ignored "-Wswitch"
void FileManager::handleOpenFiles(int directoryLength, char* directory, int numberOfMeshElementsToCreate, int numberOfMeshNodesToCreate,
                                  FileManagerActionEnum geometryAction, int geometryGroup, FileManagerActionEnum parameterAction, int parameterGroup,
                                  FileManagerActionEnum stateAction, int stateGroup)
{
  bool    error      = false;                 // Error flag.
  char*   nameString = NULL;                  // Temporary string for file and group names.
  int     nameStringSize;                     // Size of buffer allocated for nameString.
  int     numPrinted;                         // Used to check that snprintf printed the correct nubmer of characters.
  int     ncErrorCode;                        // Return value of NetCDF functions.
  int     geometryNumberOfMeshElementsDimID;  // NetCDF Dimension ID in geometry file for number of mesh elements.
  int     geometryNumberOfMeshNodesDimID;     // NetCDF Dimension ID in geometry file for number of mesh nodes.
  int     geometryThreeDimID;                 // NetCDF Dimension ID in geometry file for dimension size of three.
                                              // Used for three vertices and edges for each element and three coordinates (X, Y, Z) for each node.
  int     parameterNumberOfMeshElementsDimID; // NetCDF Dimension ID in parameter file for number of mesh elements.
  int     stateNumberOfMeshElementsDimID;     // NetCDF Dimension ID in state file for number of mesh elements.
  int     stateNumberOfMeshNodesDimID;        // NetCDF Dimension ID in state file for number of mesh nodes.
  int     stateThreeDimID;                    // NetCDF Dimension ID in state file for dimension size of three.  Used for three edges for each element.
  double  doubleZero = 0.0;                   // Default value for attributes.
  int     intZero    = 0;                     // Default value for attributes.

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
      !(0 < numberOfMeshElementsToCreate))
    {
      CkError("ERROR in FileManager::handleOpenFiles: if you are creating or writing to any files numberOfMeshElementsToCreate must be greater than zero.\n");
      error = true;
    }

  if ((FILE_MANAGER_WRITE == geometryAction  || FILE_MANAGER_CREATE == geometryAction) && !(0 < numberOfMeshNodesToCreate))
    {
      CkError("ERROR in FileManager::handleOpenFiles: if you are creating or writing to the geometry file numberOfMeshNodesToCreate must be greater than "
              "zero.\n");
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
  CkAssert(CLOSED == geometryFileStatus && CLOSED == parameterFileStatus && CLOSED == stateFileStatus);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  if (!error)
    {
      // Minus one here indicates that the number of mesh elements and nodes is currently unknown.  They might be determined by numberOfMeshElementsToCreate or
      // numberOfMeshNodesToCreate if files are being created or opened for write, or they might be determined by reading from files if files are being opened
      // for read.  If they can be determined from multiple sources it is an error if they are not all the same.
      numberOfMeshElements = -1;
      numberOfMeshNodes    = -1;

      // Allocate space for file and group name string
      nameStringSize = directoryLength + strlen("/parameter.nc"); // Longest string concatenated to directory is "/parameter.nc" whose length is 13.
      nameString     = new char[nameStringSize];                  // The maximum length of an int printed as a string is 11.

      // FIXME make directory?
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)(strlen(directory) + strlen("/geometry.nc")) == (long long)numPrinted))
        {
          CkError("ERROR in FileManager::handleOpenFiles: incorrect return value of snprintf when generating geometry file name.  %d should be %d.\n",
                  numPrinted, strlen(directory) + strlen("/geometry.nc"));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open or create file.
  if (!error)
    {
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
            geometryFileStatus = OPEN_FOR_READ;
            break;
          case FILE_MANAGER_WRITE:
            // Fallthrough.
          case FILE_MANAGER_CREATE:
            geometryFileStatus = OPEN_FOR_READ_WRITE;
            break;
          }
        }
    }

  // Create group name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%d", geometryGroup);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleOpenFiles: incorrect return value %d from snprintf when generating geometry group name.\n", numPrinted);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Get ID of or create group.
  if (!error)
    {
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
          ncErrorCode = nc_def_dim(geometryGroupID, "numberOfMeshElements", numberOfMeshElementsToCreate, &geometryNumberOfMeshElementsDimID);

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
          ncErrorCode = nc_def_dim(geometryGroupID, "numberOfMeshNodes", numberOfMeshNodesToCreate, &geometryNumberOfMeshNodesDimID);

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
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshElementNodeIndices", &meshElementNodeIndicesVarID, NC_INT, 2,
                                      geometryNumberOfMeshElementsDimID, geometryThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshNodeXYZSurfaceCoordinates", &meshNodeXYZSurfaceCoordinatesVarID,
                                      NC_DOUBLE, 2, geometryNumberOfMeshNodesDimID, geometryThreeDimID, numberOfMeshNodesToCreate, &numberOfMeshNodes);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshNodeZBedrockCoordinates", &meshNodeZBedrockCoordinatesVarID,
                                      NC_DOUBLE, 1, geometryNumberOfMeshNodesDimID, 0, numberOfMeshNodesToCreate, &numberOfMeshNodes);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshEdgeLength", &meshEdgeLengthVarID, NC_DOUBLE, 2,
                                      geometryNumberOfMeshElementsDimID, geometryThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshEdgeNormalX", &meshEdgeNormalXVarID, NC_DOUBLE, 2,
                                      geometryNumberOfMeshElementsDimID, geometryThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshEdgeNormalY", &meshEdgeNormalYVarID, NC_DOUBLE, 2,
                                      geometryNumberOfMeshElementsDimID, geometryThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshElementX", &meshElementXVarID, NC_DOUBLE, 1,
                                      geometryNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshElementY", &meshElementYVarID, NC_DOUBLE, 1,
                                      geometryNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshElementZSurface", &meshElementZSurfaceVarID, NC_DOUBLE, 1,
                                      geometryNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshElementZBedrock", &meshElementZBedrockVarID, NC_DOUBLE, 1,
                                      geometryNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshElementArea", &meshElementAreaVarID, NC_DOUBLE, 1,
                                      geometryNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshNeighborIndices", &meshNeighborIndicesVarID, NC_INT, 2,
                                      geometryNumberOfMeshElementsDimID, geometryThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(geometryAction, "geometry", geometryGroupID, "meshNeighborReciprocalEdge", &meshNeighborReciprocalEdgeVarID, NC_INT, 2,
                                      geometryNumberOfMeshElementsDimID, geometryThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)(strlen(directory) + strlen("/parameter.nc")) == (long long)numPrinted))
        {
          CkError("ERROR in FileManager::handleOpenFiles: incorrect return value of snprintf when generating parameter file name.  %d should be %d.\n",
                  numPrinted, strlen(directory) + strlen("/parameter.nc"));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open or create file.
  if (!error)
    {
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
            parameterFileStatus = OPEN_FOR_READ;
            break;
          case FILE_MANAGER_WRITE:
            // Fallthrough.
          case FILE_MANAGER_CREATE:
            parameterFileStatus = OPEN_FOR_READ_WRITE;
            break;
          }
        }
    }

  // Create group name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%d", parameterGroup);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleOpenFiles: incorrect return value %d from snprintf when generating parameter group name.\n", numPrinted);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Get ID of or create group.
  if (!error)
    {
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
      ncErrorCode = nc_def_dim(parameterGroupID, "numberOfMeshElements", numberOfMeshElementsToCreate, &parameterNumberOfMeshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension numberOfMeshElements in NetCDF parameter file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Get ID of or create variables.
  if (!error)
    {
      error = getIDOfOrCreateVariable(parameterAction, "parameter", parameterGroupID, "meshElementCatchment", &meshElementCatchmentVarID, NC_INT, 1,
                                      parameterNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(parameterAction, "parameter", parameterGroupID, "meshElementConductivity", &meshElementConductivityVarID, NC_DOUBLE, 1,
                                      parameterNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(parameterAction, "parameter", parameterGroupID, "meshElementPorosity", &meshElementPorosityVarID, NC_DOUBLE, 1,
                                      parameterNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(parameterAction, "parameter", parameterGroupID, "meshElementManningsN", &meshElementManningsNVarID, NC_DOUBLE, 1,
                                      parameterNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)(strlen(directory) + strlen("/state.nc")) == (long long)numPrinted))
        {
          CkError("ERROR in FileManager::handleOpenFiles: incorrect return value of snprintf when generating state file name.  %d should be %d.\n",
                  numPrinted, strlen(directory) + strlen("/state.nc"));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open or create file.
  if (!error)
    {
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
            stateFileStatus = OPEN_FOR_READ;
            break;
          case FILE_MANAGER_WRITE:
            // Fallthrough.
          case FILE_MANAGER_CREATE:
            stateFileStatus = OPEN_FOR_READ_WRITE;
            break;
          }
        }
    }

  // Create group name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%d", stateGroup);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleOpenFiles: incorrect return value %d from snprintf when generating state group name.\n", numPrinted);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Get ID of or create group.
  if (!error)
    {
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

  if (FILE_MANAGER_WRITE == stateAction || FILE_MANAGER_CREATE == stateAction)
    {
      // Create dimensions.
      if (!error)
        {
          ncErrorCode = nc_def_dim(stateGroupID, "numberOfMeshElements", numberOfMeshElementsToCreate, &stateNumberOfMeshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension numberOfMeshElements in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      // numberOfMeshNodes is not used for any variables in the state file, but it makes creating a .xmf file easier to have it here.
      if (!error)
        {
          ncErrorCode = nc_def_dim(stateGroupID, "numberOfMeshNodes", numberOfMeshNodesToCreate, &stateNumberOfMeshNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension numberOfMeshNodes in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_def_dim(stateGroupID, "three", 3, &stateThreeDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create dimension three in NetCDF state file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Create attributes.
      if (!error)
        {
          ncErrorCode = nc_put_att_double(stateGroupID, NC_GLOBAL, "time", NC_DOUBLE, 1, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create time attribute in NetCDF state file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          ncErrorCode = nc_put_att_int(stateGroupID, NC_GLOBAL, "geometryGroup", NC_INT, 1, &intZero);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create geometryGroup attribute in NetCDF state file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          ncErrorCode = nc_put_att_int(stateGroupID, NC_GLOBAL, "parameterGroup", NC_INT, 1, &intZero);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleOpenFiles: unable to create parameterGroup attribute in NetCDF state file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Get ID of or create variables.
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshElementSurfacewaterDepth", &meshElementSurfacewaterDepthVarID, NC_DOUBLE, 1,
                                      stateNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshElementSurfacewaterError", &meshElementSurfacewaterErrorVarID, NC_DOUBLE, 1,
                                      stateNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshElementGroundwaterHead", &meshElementGroundwaterHeadVarID, NC_DOUBLE, 1,
                                      stateNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshElementGroundwaterError", &meshElementGroundwaterErrorVarID, NC_DOUBLE, 1,
                                      stateNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshElementGroundwaterRecharge", &meshElementGroundwaterRechargeVarID, NC_DOUBLE, 1,
                                      stateNumberOfMeshElementsDimID, 0, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshEdgeSurfacewaterFlowRate", &meshEdgeSurfacewaterFlowRateVarID,
                                      NC_DOUBLE, 2, stateNumberOfMeshElementsDimID, stateThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshEdgeSurfacewaterCumulativeFlow", &meshEdgeSurfacewaterCumulativeFlowVarID,
                                      NC_DOUBLE, 2, stateNumberOfMeshElementsDimID, stateThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshEdgeGroundwaterFlowRate", &meshEdgeGroundwaterFlowRateVarID,
                                      NC_DOUBLE, 2, stateNumberOfMeshElementsDimID, stateThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
    }
  
  if (!error)
    {
      error = getIDOfOrCreateVariable(stateAction, "state", stateGroupID, "meshEdgeGroundwaterCumulativeFlow", &meshEdgeGroundwaterCumulativeFlowVarID,
                                      NC_DOUBLE, 2, stateNumberOfMeshElementsDimID, stateThreeDimID, numberOfMeshElementsToCreate, &numberOfMeshElements);
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
#pragma GCC diagnostic warning "-Wswitch"

bool FileManager::getIDOfOrCreateVariable(FileManagerActionEnum action, const char* fileName, int groupID, const char* variableName, int* variableID,
                                          nc_type variableType, int numDims, int dimID0, int dimID1, int sizeToCreate, int* sizeShouldBe)
{
  bool    error = false;               // Error flag.
  int     ii;                          // Loop counter.
  int     ncErrorCode;                 // Return value of NetCDF functions.
  int     dimIDs[NC_MAX_VAR_DIMS];     // For passing dimension IDs of variable.
  size_t  dimSizes[NC_MAX_VAR_DIMS];   // Dimension sizes of variable.
  nc_type readVariableType;            // For reading type of variable.
  int     readNumDims;                 // For reading number of dimensions of variable.
  char    unusedName[NC_MAX_NAME + 1]; // For reading name of variable.
  int     unusedNumAttributes;         // For reading number of attributes of variable.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(FILE_MANAGER_NO_ACTION == action || FILE_MANAGER_READ == action || FILE_MANAGER_WRITE == action || FILE_MANAGER_CREATE == action);
  CkAssert(NULL != fileName);
  CkAssert(NULL != variableName);
  CkAssert(NULL != variableID);
  CkAssert(NC_BYTE   == variableType || NC_CHAR == variableType || NC_SHORT == variableType || NC_LONG == variableType || NC_FLOAT == variableType ||
           NC_DOUBLE == variableType);
  CkAssert(1 == numDims || 2 == numDims);
  
  if (FILE_MANAGER_WRITE == action || FILE_MANAGER_CREATE == action)
    {
      ncErrorCode = nc_inq_dimlen(groupID, dimID0, &dimSizes[0]);
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      CkAssert(NC_NOERR == ncErrorCode && (long long)dimSizes[0] == (long long)sizeToCreate);
    }
  
  CkAssert(&numberOfMeshElements == sizeShouldBe || &numberOfMeshNodes == sizeShouldBe);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  switch (action)
  {
  case FILE_MANAGER_NO_ACTION:
    // Do nothing.
    ncErrorCode = NC_NOERR;
    break;
  case FILE_MANAGER_READ:
    // Get ID of variable.
    ncErrorCode = nc_inq_varid(groupID, variableName, variableID);
    break;
  case FILE_MANAGER_WRITE:
    // Fallthrough.
  case FILE_MANAGER_CREATE:
    // Create variable.  If numDims is 1, dimIDs[1] will be ignored.
    dimIDs[0]   = dimID0;
    dimIDs[1]   = dimID1;
    dimSizes[0] = sizeToCreate;
    dimSizes[1] = 3;
    ncErrorCode = nc_def_var(groupID, variableName, variableType, numDims, dimIDs, variableID);
    break;
  }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NC_NOERR == ncErrorCode))
    {
      CkError("ERROR in FileManager::getIDOfOrCreateVariable: unable to %s variable %s in NetCDF %s file.  NetCDF error message: %s.\n",
              (FILE_MANAGER_READ == action) ? "find" : "create", variableName, fileName, nc_strerror(ncErrorCode));
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // If you are reading make sure the variable has the right type and number of dimensions.
  if (FILE_MANAGER_READ == action)
    {
      if (!error)
        {
          ncErrorCode = nc_inq_var(groupID, *variableID, unusedName, &readVariableType, &readNumDims, dimIDs, &unusedNumAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::getIDOfOrCreateVariable: unable to get information about variable %s in NetCDF %s file.  "
                      "NetCDF error message: %s.\n", variableName, fileName, nc_strerror(ncErrorCode));
              error = true;
            }
          else
            {
              if (!(readVariableType == variableType))
                {
                  CkError("ERROR in FileManager::getIDOfOrCreateVariable: wrong type for variable %s in NetCDF %s file.  %d should be %d.",
                          variableName, fileName, readVariableType, variableType);
                  error = true;
                }

              if (!(readNumDims == numDims))
                {
                  CkError("ERROR in FileManager::getIDOfOrCreateVariable: wrong number of dimensions for variable %s in NetCDF %s file.  %d should be %d.",
                          variableName, fileName, readNumDims, numDims);
                  error = true;
                }
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      // Read the size of the variable.
      for (ii = 0; !error && ii < numDims; ii++)
        {
          ncErrorCode = nc_inq_dimlen(groupID, dimIDs[ii], &dimSizes[ii]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::getIDOfOrCreateVariable: unable to get information about dimension %d of variable %s in NetCDF %s file.  "
                      "NetCDF error message: %s.\n", ii, variableName, fileName, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Check that the size of the variable is consistent with other variables.
  if (!error && FILE_MANAGER_NO_ACTION != action)
    {
      // If the size is not yet determined get it from the size of this variable.
      if (-1 == *sizeShouldBe)
        {
          *sizeShouldBe = dimSizes[0];
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)*sizeShouldBe == (long long)dimSizes[0] && (1 == numDims || 3 == dimSizes[1])))
        {
          CkError("ERROR in FileManager::getIDOfOrCreateVariable: wrong size of variable %s in NetCDF %s file.  %dx%d should be %dx%d.\n",
                  variableName, fileName, dimSizes[0], (1 == numDims) ? 1 : dimSizes[1], *sizeShouldBe, (1 == numDims) ? 1 : 3);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  return error;
}

void FileManager::handleCloseFiles()
{
  bool error = false; // Error flag.
  int  ncErrorCode;   // Return value of NetCDF functions.

  // Attempt to close all open files even if there is an error.
  if (CLOSED != geometryFileStatus)
    {
      ncErrorCode        = nc_close(geometryFileID);
      geometryFileStatus = CLOSED;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCloseFiles: unable to close NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (CLOSED != parameterFileStatus)
    {
      ncErrorCode         = nc_close(parameterFileID);
      parameterFileStatus = CLOSED;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCloseFiles: unable to close NetCDF parameter file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (CLOSED != stateFileStatus)
    {
      ncErrorCode     = nc_close(stateFileID);
      stateFileStatus = CLOSED;

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
