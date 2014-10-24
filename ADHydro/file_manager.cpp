#include "file_manager.h"
#include "evapo_transpiration.h"
#include "all.h"
#include <netcdf.h>
#include <netcdf_par.h>

int FileManager::home(int item, int globalNumberOfItems)
{
  int numPes              = CkNumPes();                           // Number of processors.
  int numberOfFatOwners   = globalNumberOfItems % numPes;         // Number of file managers that own one extra item.
  int itemsPerFatOwner    = globalNumberOfItems / numPes + 1;     // Number of items in each file manager that owns one extra item.
  int itemsInAllFatOwners = numberOfFatOwners * itemsPerFatOwner; // Total number of items in all file managers that own one extra item.
  int itemsPerThinOwner   = globalNumberOfItems / numPes;         // Number of items in each file manager that does not own one extra item.
  int itemHome;                                                   // The index of the file manager that owns item.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= item && item < globalNumberOfItems))
    {
      CkError("ERROR in FileManager::home: item must be greater than or equal to zero and less than globalNumberOfItems.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (item < itemsInAllFatOwners)
    {
      // Item is owned by a fat owner.
      itemHome = item / itemsPerFatOwner;
    }
  else
    {
      // Item is owned by a thin owner.
      itemHome = ((item - itemsInAllFatOwners) / itemsPerThinOwner) + numberOfFatOwners;
    }
  
  return itemHome;
}

FileManager::FileManager()
{
  globalNumberOfMeshNodes        = 0;
  localMeshNodeStart             = 0;
  localNumberOfMeshNodes         = 0;
  globalNumberOfMeshElements     = 0;
  localMeshElementStart          = 0;
  localNumberOfMeshElements      = 0;
  globalNumberOfChannelNodes     = 0;
  localChannelNodeStart          = 0;
  localNumberOfChannelNodes      = 0;
  globalNumberOfChannelElements  = 0;
  localChannelElementStart       = 0;
  localNumberOfChannelElements   = 0;
  meshNodeX                      = NULL;
  meshNodeY                      = NULL;
  meshNodeZSurface               = NULL;
  meshNodeZBedrock               = NULL;
  meshElementVertices            = NULL;
  meshVertexX                    = NULL;
  meshVertexY                    = NULL;
  meshVertexZSurface             = NULL;
  meshVertexZBedrock             = NULL;
  meshElementX                   = NULL;
  meshElementY                   = NULL;
  meshElementZSurface            = NULL;
  meshElementZBedrock            = NULL;
  meshElementArea                = NULL;
  meshElementSlopeX              = NULL;
  meshElementSlopeY              = NULL;
  meshCatchment                  = NULL;
  meshConductivity               = NULL;
  meshPorosity                   = NULL;
  meshManningsN                  = NULL;
  meshSurfacewaterDepth          = NULL;
  meshSurfacewaterError          = NULL;
  meshGroundwaterHead            = NULL;
  meshGroundwaterError           = NULL;
  meshMeshNeighbors              = NULL;
  meshMeshNeighborsChannelEdge   = NULL;
  meshMeshNeighborsEdgeLength    = NULL;
  meshMeshNeighborsEdgeNormalX   = NULL;
  meshMeshNeighborsEdgeNormalY   = NULL;
  meshChannelNeighbors           = NULL;
  meshChannelNeighborsEdgeLength = NULL;
  channelNodeX                   = NULL;
  channelNodeY                   = NULL;
  channelNodeZBank               = NULL;
  channelNodeZBed                = NULL;
  channelElementVertices         = NULL;
  channelVertexX                 = NULL;
  channelVertexY                 = NULL;
  channelVertexZBank             = NULL;
  channelVertexZBed              = NULL;
  channelElementX                = NULL;
  channelElementY                = NULL;
  channelElementZBank            = NULL;
  channelElementZBed             = NULL;
  channelElementLength           = NULL;
  channelChannelType             = NULL;
  channelPermanentCode           = NULL;
  channelBaseWidth               = NULL;
  channelSideSlope               = NULL;
  channelBedConductivity         = NULL;
  channelBedThickness            = NULL;
  channelManningsN               = NULL;
  channelSurfacewaterDepth       = NULL;
  channelSurfacewaterError       = NULL;
  channelChannelNeighbors        = NULL;
  channelMeshNeighbors           = NULL;
  channelMeshNeighborsEdgeLength = NULL;
  meshVertexUpdated              = NULL;
  meshElementUpdated             = NULL;
  channelElementUpdated          = NULL;
  currentTime                    = 0.0;
  dt                             = 1.0;
  iteration                      = 1;
  
  // Use sdag code to force receipt of only one initialization call.
  thisProxy[CkMyPe()].initialize();
}

FileManager::~FileManager()
{
  if (NULL != meshNodeX)
    {
      delete[] meshNodeX;
    }
  
  if (NULL != meshNodeY)
    {
      delete[] meshNodeY;
    }
  
  if (NULL != meshNodeZSurface)
    {
      delete[] meshNodeZSurface;
    }
  
  if (NULL != meshNodeZBedrock)
    {
      delete[] meshNodeZBedrock;
    }
  
  if (NULL != meshElementVertices)
    {
      delete[] meshElementVertices;
    }
  
  if (NULL != meshVertexX)
    {
      delete[] meshVertexX;
    }
  
  if (NULL != meshVertexY)
    {
      delete[] meshVertexY;
    }
  
  if (NULL != meshVertexZSurface)
    {
      delete[] meshVertexZSurface;
    }
  
  if (NULL != meshVertexZBedrock)
    {
      delete[] meshVertexZBedrock;
    }
  
  if (NULL != meshElementX)
    {
      delete[] meshElementX;
    }
  
  if (NULL != meshElementY)
    {
      delete[] meshElementY;
    }
  
  if (NULL != meshElementZSurface)
    {
      delete[] meshElementZSurface;
    }
  
  if (NULL != meshElementZBedrock)
    {
      delete[] meshElementZBedrock;
    }
  
  if (NULL != meshElementArea)
    {
      delete[] meshElementArea;
    }
  
  if (NULL != meshElementSlopeX)
    {
      delete[] meshElementSlopeX;
    }
  
  if (NULL != meshElementSlopeY)
    {
      delete[] meshElementSlopeY;
    }
  
  if (NULL != meshCatchment)
    {
      delete[] meshCatchment;
    }
  
  if (NULL != meshConductivity)
    {
      delete[] meshConductivity;
    }
  
  if (NULL != meshPorosity)
    {
      delete[] meshPorosity;
    }
  
  if (NULL != meshManningsN)
    {
      delete[] meshManningsN;
    }
  
  if (NULL != meshSurfacewaterDepth)
    {
      delete[] meshSurfacewaterDepth;
    }
  
  if (NULL != meshSurfacewaterError)
    {
      delete[] meshSurfacewaterError;
    }
  
  if (NULL != meshGroundwaterHead)
    {
      delete[] meshGroundwaterHead;
    }
  
  if (NULL != meshGroundwaterError)
    {
      delete[] meshGroundwaterError;
    }
  
  if (NULL != meshMeshNeighbors)
    {
      delete[] meshMeshNeighbors;
    }
  
  if (NULL != meshMeshNeighborsChannelEdge)
    {
      delete[] meshMeshNeighborsChannelEdge;
    }
  
  if (NULL != meshMeshNeighborsEdgeLength)
    {
      delete[] meshMeshNeighborsEdgeLength;
    }
  
  if (NULL != meshMeshNeighborsEdgeNormalX)
    {
      delete[] meshMeshNeighborsEdgeNormalX;
    }
  
  if (NULL != meshMeshNeighborsEdgeNormalY)
    {
      delete[] meshMeshNeighborsEdgeNormalY;
    }
  
  if (NULL != meshChannelNeighbors)
    {
      delete[] meshChannelNeighbors;
    }
  
  if (NULL != meshChannelNeighborsEdgeLength)
    {
      delete[] meshChannelNeighborsEdgeLength;
    }
  
  if (NULL != channelNodeX)
    {
      delete[] channelNodeX;
    }
  
  if (NULL != channelNodeY)
    {
      delete[] channelNodeY;
    }
  
  if (NULL != channelNodeZBank)
    {
      delete[] channelNodeZBank;
    }
  
  if (NULL != channelNodeZBed)
    {
      delete[] channelNodeZBed;
    }
  
  if (NULL != channelElementVertices)
    {
      delete[] channelElementVertices;
    }
  
  if (NULL != channelVertexX)
    {
      delete[] channelVertexX;
    }
  
  if (NULL != channelVertexY)
    {
      delete[] channelVertexY;
    }
  
  if (NULL != channelVertexZBank)
    {
      delete[] channelVertexZBank;
    }
  
  if (NULL != channelVertexZBed)
    {
      delete[] channelVertexZBed;
    }
  
  if (NULL != channelElementX)
    {
      delete[] channelElementX;
    }
  
  if (NULL != channelElementY)
    {
      delete[] channelElementY;
    }
  
  if (NULL != channelElementZBank)
    {
      delete[] channelElementZBank;
    }
  
  if (NULL != channelElementZBed)
    {
      delete[] channelElementZBed;
    }
  
  if (NULL != channelElementLength)
    {
      delete[] channelElementLength;
    }
  
  if (NULL != channelChannelType)
    {
      delete[] channelChannelType;
    }
  
  if (NULL != channelPermanentCode)
    {
      delete[] channelPermanentCode;
    }
  
  if (NULL != channelBaseWidth)
    {
      delete[] channelBaseWidth;
    }
  
  if (NULL != channelSideSlope)
    {
      delete[] channelSideSlope;
    }
  
  if (NULL != channelBedConductivity)
    {
      delete[] channelBedConductivity;
    }
  
  if (NULL != channelBedThickness)
    {
      delete[] channelBedThickness;
    }
  
  if (NULL != channelManningsN)
    {
      delete[] channelManningsN;
    }
  
  if (NULL != channelSurfacewaterDepth)
    {
      delete[] channelSurfacewaterDepth;
    }
  
  if (NULL != channelSurfacewaterError)
    {
      delete[] channelSurfacewaterError;
    }
  
  if (NULL != channelChannelNeighbors)
    {
      delete[] channelChannelNeighbors;
    }
  
  if (NULL != channelMeshNeighbors)
    {
      delete[] channelMeshNeighbors;
    }
  
  if (NULL != channelMeshNeighborsEdgeLength)
    {
      delete[] channelMeshNeighborsEdgeLength;
    }
  
  if (NULL != meshVertexUpdated)
    {
      delete[] meshVertexUpdated;
    }
  
  if (NULL != meshElementUpdated)
    {
      delete[] meshElementUpdated;
    }
  
  if (NULL != channelElementUpdated)
    {
      delete[] channelElementUpdated;
    }
}

void FileManager::calculateDerivedValues()
{
  int  ii, jj;                      // Loop counters.
  bool needToGetVertexData = false; // Whether we need to get any vertex data.

  // Get vertex data from node data.
  if (NULL != meshElementVertices)
    {
      if (NULL == meshVertexX && NULL != meshNodeX)
        {
          meshVertexX         = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetVertexData = true;
        }

      if (NULL == meshVertexY && NULL != meshNodeY)
        {
          meshVertexY         = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetVertexData = true;
        }

      if (NULL == meshVertexZSurface && NULL != meshNodeZSurface)
        {
          meshVertexZSurface  = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetVertexData = true;
        }

      if (NULL == meshVertexZBedrock && NULL != meshNodeZBedrock)
        {
          meshVertexZBedrock  = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetVertexData = true;
        }
    }

  // We don't try to get channel vertex data because it is unused.

  if (needToGetVertexData)
    {
      meshVertexUpdated = new bool[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for(ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              meshVertexUpdated[ii][jj] = false;

              // FIXME improve efficiency.  Don't send messages to myself.  Don't send duplicate messages for the same node.
              thisProxy[home(meshElementVertices[ii][jj], globalNumberOfMeshNodes)].getMeshVertexDataMessage(CkMyPe(), ii + localMeshElementStart, jj,
                                                                                                             meshElementVertices[ii][jj]);
            }
        }

      thisProxy[CkMyPe()].waitForVertexData();
    }
  else
    {
      finishCalculateDerivedValues();
    }
}

void FileManager::getMeshVertexDataMessage(int requester, int element, int vertex, int node)
{
  double x        = 0.0;
  double y        = 0.0;
  double zSurface = 0.0;
  double zBedrock = 0.0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= requester && requester < CkNumPes()))
    {
      CkError("ERROR in FileManager::getMeshVertexDataMessage: requester must be greater than or equal to zero and less than CkNumPes().\n");
      CkExit();
    }

  if (!(0 <= element && element < globalNumberOfMeshElements))
    {
      CkError("ERROR in FileManager::getMeshVertexDataMessage: element must be greater than or equal to zero and less than "
              "globalNumberOfMeshElements.\n");
      CkExit();
    }

  if (!(0 <= vertex && vertex < MeshElement::meshNeighborsSize))
    {
      CkError("ERROR in FileManager::getMeshVertexDataMessage: vertex must be greater than or equal to zero and less than "
              "MeshElement::meshNeighborsSize.\n");
      CkExit();
    }

  if (!(localMeshNodeStart <= node && node < localMeshNodeStart + localNumberOfMeshNodes))
    {
      CkError("ERROR in FileManager::getMeshVertexDataMessage: node data not owned by this local branch.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (NULL != meshNodeX)
    {
      x = meshNodeX[node - localMeshNodeStart];
    }

  if (NULL != meshNodeY)
    {
      y = meshNodeY[node - localMeshNodeStart];
    }

  if (NULL != meshNodeZSurface)
    {
      zSurface = meshNodeZSurface[node - localMeshNodeStart];
    }

  if (NULL != meshNodeZBedrock)
    {
      zBedrock = meshNodeZBedrock[node - localMeshNodeStart];
    }

  thisProxy[requester].meshVertexDataMessage(element, vertex, x, y, zSurface, zBedrock);
}

void FileManager::readForcingData(CProxy_MeshElement meshProxy, CProxy_ChannelElement channelProxy, double currentTime, size_t directorySize,
                                  const char* directory)
{
  bool   error             = false; // Error flag.
  int ii;                           // Loop counter.
  char*  nameString        = NULL;  // Temporary string for file names.
  size_t nameStringSize;            // Size of buffer allocated for nameString.
  size_t numPrinted;                // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;               // Return value of NetCDF functions.
  int    FileID;                    // ID of NetCDF file.
  bool   forcingFileOpen   = false; // Whether FileID refers to an open file.
  int    dimID;                     // ID of dimension in NetCDF file.
  size_t stateTime;                 // Time index for state file.
  int    varID;                     // ID of variable in NetCDF file.
  size_t start[NC_MAX_VAR_DIMS];    // For specifying subarrays when writing to NetCDF file.
  size_t count[NC_MAX_VAR_DIMS];    // For specifying subarrays when writing to NetCDF file.
  double* forcingT2        = NULL;  // Used to read air temperature at 2m height forcing.
  double* forcingVEGFRA    = NULL;  // Used to read vegetation fraction forcing.
  double* forcingMAXVEGFRA = NULL;  // Used to read maximum vegetation fraction forcing.
  double* forcingPSFC      = NULL;  // Used to read surface pressure forcing.
  double* forcingU         = NULL;  // Used to read wind speed U component forcing.
  double* forcingV         = NULL;  // Used to read wind speed V component forcing.
  double* forcingQVAPOR    = NULL;  // Used to read water vapor mixing ratio forcing.
  double* forcingQCLOUD    = NULL;  // Used to read cloud water mixing ratio forcing.
  double* forcingSWDOWN    = NULL;  // Used to read downward shortwave flux forcing.
  double* forcingGLW       = NULL;  // Used to read downward longwave flux forcing.
  double* forcingTPREC     = NULL;  // Used to read total precipitation forcing.
  double* forcingTSLB      = NULL;  // Used to read soil temperature at the deepest layer forcing.
  double* forcingPBLH      = NULL;  // Used to read planetary boundary layer height forcing.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::readForcingData: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file name strings.
      nameStringSize = strlen(directory) + strlen("/forcing.nc") + 1; // The longest file name is forcing.nc.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/forcing.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/forcing.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::readForcingData: incorrect return value of snprintf when generating forcing file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/forcing.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &FileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to open NetCDF forcing file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          forcingFileOpen = true;
        }
    }
 
  // Get the number of existing Times.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(FileID, "Time", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get dimension Time in NetCDF forcing file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(FileID, dimID, &stateTime);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get length of dimension Time in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      if (0 < stateTime)
        {
          // We're not creating a new Time so use the last Time with index one less than the dimension length.
          stateTime--;
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          // We're not creating a new Time so it's an error if there's not an existing one.
          CkError("ERROR in FileManager::readForcingData: not creating a new Time and no existing Time in NetCDF forcing file %s.\n",
                  nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Read variables.
  // T2
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "T2", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable T2 in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingT2 = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingT2);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable T2 in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // VEGFRA
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "VEGFRA", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable VEGFRA in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingVEGFRA = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingVEGFRA);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable VEGFRA in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // MAXVEGFRA
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "MAXVEGFRA", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable MAXVEGFRA in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingMAXVEGFRA = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingMAXVEGFRA);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable MAXVEGFRA in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // PSFC
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "PSFC", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable PSFC in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingPSFC = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingPSFC);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable PSFC in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // U
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "U", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable U in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingU = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingU);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable U in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // V
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "V", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable V in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingV = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingV);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable V in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // QVAPOR
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "QVAPOR", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable QVAPOR in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingQVAPOR = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingQVAPOR);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable QVAPOR in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // QCLOUD
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "QCLOUD", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable QCLOUD in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingQCLOUD = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingQCLOUD);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable QCLOUD in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // SWDOWN
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "SWDOWN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable SWDOWN in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingSWDOWN = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingSWDOWN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable SWDOWN in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // GLW
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "GLW", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable GLW in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingGLW = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingGLW);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable GLW in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

 // TPREC
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "TPREC", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable TPREC in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingTPREC = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingTPREC);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable TPREC in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }


 // TSLB
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "TSLB", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable TSLB in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingTSLB = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingTSLB);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable TSLB in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }


 // PBLH
  if (!error)
    {
      ncErrorCode = nc_inq_varid(FileID, "PBLH", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to get variable PBLH in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      forcingPBLH = new double[localNumberOfMeshElements];

      start[0]    = stateTime;
      start[1]    = localMeshElementStart;
      count[0]    = 1;
      count[1]    = localNumberOfMeshElements;
      ncErrorCode = nc_get_vara_double(FileID, varID, start, count, forcingPBLH);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to read variable PBLH in NetCDF forcing file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  for (ii = localMeshElementStart; ii < localMeshElementStart + localNumberOfMeshElements; ii++)
    {
	meshProxy[ii].forcingDataMessage(20.0, forcingVEGFRA[ii - localMeshElementStart], forcingMAXVEGFRA[ii - localMeshElementStart],
	                                 forcingT2[ii - localMeshElementStart], forcingPSFC[ii - localMeshElementStart],
	                                 forcingPSFC[ii - localMeshElementStart] - 120.0, forcingU[ii - localMeshElementStart],
	                                 forcingV[ii - localMeshElementStart], forcingQVAPOR[ii - localMeshElementStart],
	                                 forcingQCLOUD[ii - localMeshElementStart],  forcingSWDOWN[ii - localMeshElementStart],
	                                 forcingGLW[ii - localMeshElementStart],  forcingTPREC[ii - localMeshElementStart],
	                                 forcingTSLB[ii - localMeshElementStart],  forcingPBLH[ii - localMeshElementStart]);
    }
  
  for (ii = localChannelElementStart; ii < localChannelElementStart + localNumberOfChannelElements; ii++)
    {
      channelProxy[ii].forcingDataMessage();
    }

  // Close file.
  if (forcingFileOpen)
    {
      ncErrorCode     = nc_close(FileID);
      forcingFileOpen = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to close NetCDF forcing file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (NULL != forcingT2)
    {
      delete[] forcingT2;
    }
  
  if (NULL != forcingVEGFRA)
    {
      delete[] forcingVEGFRA;
    }
  
  if (NULL != forcingMAXVEGFRA)
    {
      delete[] forcingMAXVEGFRA;
    }
  
  if (NULL != forcingPSFC)
    {
      delete[] forcingPSFC;
    }
  
  if (NULL != forcingU)
    {
      delete[] forcingU;
    }
  
  if (NULL != forcingV)
    {
      delete[] forcingV;
    }
  
  if (NULL != forcingQVAPOR)
    {
      delete[] forcingQVAPOR;
    }
  
  if (NULL != forcingQCLOUD)
    {
      delete[] forcingQCLOUD;
    }
  
  if (NULL != forcingSWDOWN)
    {
      delete[] forcingSWDOWN;
    }
  
  if (NULL != forcingGLW)
    {
      delete[] forcingGLW;
    }
  
  if (NULL != forcingTPREC)
    {
      delete[] forcingTPREC;
    }
  
  if (NULL != forcingTSLB)
    {
      delete[] forcingTSLB;
    }
  
  if (NULL != forcingPBLH)
    {
      delete[] forcingPBLH;
    }
}

void FileManager::createFiles(size_t directorySize, const char* directory)
{
  bool   error      = false;                     // Error flag.
  char*  nameString = NULL;                      // Temporary string for file names.
  size_t nameStringSize;                         // Size of buffer allocated for nameString.
  size_t numPrinted;                             // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;                            // Return value of NetCDF functions.
  int    fileID;                                 // ID of NetCDF file.
  bool   fileOpen   = false;                     // Whether fileID refers to an open file.
  int    instancesDimID;                         // ID of dimension in NetCDF file.
  int    meshNodesDimID;                         // ID of dimension in NetCDF file.
  int    meshElementsDimID;                      // ID of dimension in NetCDF file.
  int    meshMeshNeighborsDimID;                 // ID of dimension in NetCDF file.
  int    meshChannelNeighborsDimID;              // ID of dimension in NetCDF file.
  int    channelNodesDimID;                      // ID of dimension in NetCDF file.
  int    channelElementsDimID;                   // ID of dimension in NetCDF file.
  int    sizeOfChannelElementVerticesArrayDimID; // ID of dimension in NetCDF file.
  int    channelVerticesDimID;                   // ID of dimension in NetCDF file.
  int    channelChannelNeighborsDimID;           // ID of dimension in NetCDF file.
  int    channelMeshNeighborsDimID;              // ID of dimension in NetCDF file.
  int    dimIDs[NC_MAX_VAR_DIMS];                // For passing dimension IDs.
  int    varID;                                  // ID of variable in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::createFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file name strings.
      nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // The longest file name is parameter.nc.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // FIXME make directory?

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::createFiles: incorrect return value of snprintf when generating geometry file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file.
  if (!error)
    {
      ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "instances", NC_UNLIMITED, &instancesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension instances in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshNodes", NC_UNLIMITED, &meshNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshNodes in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshElements in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshMeshNeighbors", MeshElement::meshNeighborsSize, &meshMeshNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshMeshNeighbors in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshChannelNeighbors", MeshElement::channelNeighborsSize, &meshChannelNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshChannelNeighbors in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelNodes", NC_UNLIMITED, &channelNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelNodes in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelElements in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "sizeOfChannelElementVerticesArray", ChannelElement::channelVerticesSize + 2, &sizeOfChannelElementVerticesArrayDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension sizeOfChannelElementVerticesArray in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelVertices", ChannelElement::channelVerticesSize, &channelVerticesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelVertices in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelChannelNeighbors", ChannelElement::channelNeighborsSize, &channelChannelNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelChannelNeighbors in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelMeshNeighbors", ChannelElement::meshNeighborsSize, &channelMeshNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelMeshNeighbors in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "numberOfMeshNodes", NC_INT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable numberOfMeshNodes in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "numberOfMeshElements", NC_INT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable numberOfMeshElements in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "numberOfChannelNodes", NC_INT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable numberOfChannelNodes in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "numberOfChannelElements", NC_INT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable numberOfChannelElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshNodesDimID;
      ncErrorCode = nc_def_var(fileID, "meshNodeX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshNodesDimID;
      ncErrorCode = nc_def_var(fileID, "meshNodeY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeZSurface)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshNodesDimID;
      ncErrorCode = nc_def_var(fileID, "meshNodeZSurface", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshNodeZSurface in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeZBedrock)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshNodesDimID;
      ncErrorCode = nc_def_var(fileID, "meshNodeZBedrock", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshNodeZBedrock in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementVertices)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementVertices", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementVertices in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshVertexX", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshVertexX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshVertexY", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshVertexY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexZSurface)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshVertexZSurface", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshVertexZSurface in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexZBedrock)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshVertexZBedrock", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshVertexZBedrock in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementZSurface)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementZSurface", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementZSurface in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementZBedrock)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementZBedrock", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementZBedrock in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementArea)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementArea", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementArea in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementSlopeX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementSlopeX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementSlopeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementSlopeY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementSlopeY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementSlopeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighbors)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighbors", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighbors in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsChannelEdge)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighborsChannelEdge", NC_BYTE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighborsChannelEdge in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeLength)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighborsEdgeLength", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighborsEdgeNormalX", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighborsEdgeNormalY", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshChannelNeighbors)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshChannelNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshChannelNeighbors", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshChannelNeighbors in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshChannelNeighborsEdgeLength)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshChannelNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshChannelNeighborsEdgeLength", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshChannelNeighborsEdgeLength in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelNodesDimID;
      ncErrorCode = nc_def_var(fileID, "channelNodeX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelNodesDimID;
      ncErrorCode = nc_def_var(fileID, "channelNodeY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeZBank)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelNodesDimID;
      ncErrorCode = nc_def_var(fileID, "channelNodeZBank", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelNodeZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeZBed)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelNodesDimID;
      ncErrorCode = nc_def_var(fileID, "channelNodeZBed", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelNodeZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementVertices)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = sizeOfChannelElementVerticesArrayDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementVertices", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementVertices in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelVerticesDimID;
      ncErrorCode = nc_def_var(fileID, "channelVertexX", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelVertexX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelVerticesDimID;
      ncErrorCode = nc_def_var(fileID, "channelVertexY", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelVertexY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexZBank)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelVerticesDimID;
      ncErrorCode = nc_def_var(fileID, "channelVertexZBank", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelVertexZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexZBed)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelVerticesDimID;
      ncErrorCode = nc_def_var(fileID, "channelVertexZBed", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelVertexZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementZBank)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementZBank", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementZBed)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementZBed", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementLength)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementLength", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementLength in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelChannelNeighbors)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelChannelNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "channelChannelNeighbors", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelChannelNeighbors in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelMeshNeighbors)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "channelMeshNeighbors", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelMeshNeighbors in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelMeshNeighborsEdgeLength)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "channelMeshNeighborsEdgeLength", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to close NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::createFiles: incorrect return value of snprintf when generating parameter file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file.
  if (!error)
    {
      ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "instances", NC_UNLIMITED, &instancesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension instances in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshElements in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelElements in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "geometryInstance", NC_UINT64, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable geometryInstance in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshCatchment)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshCatchment", NC_INT, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshCatchment in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshConductivity)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshConductivity", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshConductivity in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshPorosity)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshPorosity", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshPorosity in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshManningsN)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshManningsN", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshManningsN in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelChannelType)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelChannelType", NC_INT, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelChannelType in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelPermanentCode)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelPermanentCode", NC_INT, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelPermanentCode in NetCDF parameter file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBaseWidth)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelBaseWidth", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelBaseWidth in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSideSlope)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelSideSlope", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelSideSlope in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBedConductivity)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelBedConductivity", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelBedConductivity in NetCDF parameter file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBedThickness)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelBedThickness", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelBedThickness in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelManningsN)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelManningsN", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelManningsN in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to close NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::createFiles: incorrect return value of snprintf when generating state file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file.
  if (!error)
    {
      ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "instances", NC_UNLIMITED, &instancesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension instances in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshElements in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelElements in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "iteration", NC_USHORT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable iteration in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "currentTime", NC_DOUBLE, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable currentTime in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "dt", NC_DOUBLE, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "geometryInstance", NC_UINT64, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable geometryInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "parameterInstance", NC_UINT64, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable parameterInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshSurfacewaterDepth)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshSurfacewaterDepth", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshSurfacewaterDepth in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshSurfacewaterError)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshSurfacewaterError", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshSurfacewaterError in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshGroundwaterHead)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshGroundwaterHead", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshGroundwaterHead in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshGroundwaterError)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshGroundwaterError", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshGroundwaterError in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshPrecipitationCumulative)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshPrecipitationCumulative", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshPrecipitationCumulative in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
    
  if (!error && NULL != meshEvaporationCumulative)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshEvaporationCumulative", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshEvaporationCumulative in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshCanopyLiquid)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshCanopyLiquid", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshCanopyLiquid in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error && NULL != meshCanopyIce)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshCanopyIce", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshCanopyIce in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshSnowWaterEquivalent)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshSnowWaterEquivalent", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshSnowWaterEquivalent in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementNeighborsSurfacewaterCumulativeFlow) 
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementNeighborsSurfacewaterCumulativeFlow", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementNeighborsSurfacewaterCumulativeFlow in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
    
  if (!error && NULL != meshElementNeighborsGroundwaterCumulativeFlow) 
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementNeighborsGroundwaterCumulativeFlow", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementNeighborsGroundwaterCumulativeFlow in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
      
  if (!error && NULL != channelSurfacewaterDepth)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelSurfacewaterDepth", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelSurfacewaterDepth in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSurfacewaterError)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelSurfacewaterError", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelSurfacewaterError in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelEvaporationCumulative)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelEvaporationCumulative", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelEvaporationCumulative in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementNeighborsSurfacewaterCumulativeFlow) 
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementNeighborsSurfacewaterCumulativeFlow", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementNeighborsSurfacewaterCumulativeFlow in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    } 
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to close NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Delete nameString.
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

void FileManager::resizeUnlimitedDimensions(size_t directorySize, const char* directory, bool writeGeometry, bool writeParameter, bool writeState)
{
  bool               error      = false;       // Error flag.
  char*              nameString = NULL;        // Temporary string for file names.
  size_t             nameStringSize;           // Size of buffer allocated for nameString.
  size_t             numPrinted;               // Used to check that snprintf printed the correct number of characters.
  int                ncErrorCode;              // Return value of NetCDF functions.
  int                fileID;                   // ID of NetCDF file.
  bool               fileOpen   = false;       // Whether fileID refers to an open file.
  int                dimID;                    // ID of dimension in NetCDF file.
  int                varID;                    // ID of variable in NetCDF file.
  size_t             geometryInstance;         // Instance index for geometry file.
  size_t             parameterInstance;        // Instance index for parameter file.
  size_t             stateInstance;            // Instance index for state file.
  size_t             start[NC_MAX_VAR_DIMS];   // For specifying subarrays when writing to NetCDF file.
  int                intZero = 0;              // For writing dummy value to resize dimension.
  double             doubleZero = 0.0;         // For writing dummy value to resize dimension.
  signed char        signedCharZero = 0;       // For writing dummy value to resize dimension.
  unsigned short     unsignedShortZero = 0;    // For writing dummy value to resize dimension.
  unsigned long long unsignedLongLongZero = 0; // For writing dummy value to resize dimension.

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
  // This function should only be called if using the NetCDF collective I/O workaround.
  CkAssert(false);
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  if (0 == CkMyPe())
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      if (!(NULL != directory))
        {
          CkError("ERROR in FileManager::resizeUnlimitedDimensions: directory must not be null.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

      // Allocate space for file name strings.
      if (!error)
        {
          nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // The longest file name is parameter.nc.  +1 for null terminating character.
          nameString     = new char[nameStringSize];
        }

      if (writeGeometry)
        {
          // Create file name.
          if (!error)
            {
              numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: incorrect return value of snprintf when generating geometry file name %s.  "
                          "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Open file.
          if (!error)
            {
              ncErrorCode = nc_open(nameString, NC_NETCDF4 | NC_WRITE, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to open NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
              else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                {
                  fileOpen = true;
                }
            }

          // Get the number of existing instances.
          if (!error)
            {
              ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get dimension instances in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_dimlen(fileID, dimID, &geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get length of dimension instances in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Write variables.
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfMeshNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable numberOfMeshNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = geometryInstance;
              ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable numberOfMeshNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfMeshElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable numberOfMeshElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = geometryInstance;
              ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable numberOfMeshElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfChannelNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable numberOfChannelNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = geometryInstance;
              ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable numberOfChannelNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfChannelElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable numberOfChannelElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = geometryInstance;
              ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable numberOfChannelElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (NULL != meshNodeX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshNodeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshNodeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshNodeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshNodeY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshNodeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshNodeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshNodeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshNodeZSurface)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshNodeZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshNodeZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshNodeZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshNodeZBedrock)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshNodeZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshNodeZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshNodeZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementVertices)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementVertices", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementVertices in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementVertices in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshVertexX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshVertexX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshVertexX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshVertexY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshVertexY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshVertexY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshVertexY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshVertexZSurface)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshVertexZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshVertexZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshVertexZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshVertexZBedrock)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshVertexZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshVertexZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshVertexZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementZSurface)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementZBedrock)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementArea)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementArea", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementArea in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementArea in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementSlopeX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementSlopeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementSlopeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementSlopeY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementSlopeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementSlopeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighbors)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighborsChannelEdge)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsChannelEdge", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighborsChannelEdge in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_schar(fileID, varID, start, &signedCharZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighborsChannelEdge in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighborsEdgeLength)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighborsEdgeLength in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighborsEdgeLength in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighborsEdgeNormalX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighborsEdgeNormalY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshChannelNeighbors)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshChannelNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::channelNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshChannelNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshChannelNeighborsEdgeLength)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshChannelNeighborsEdgeLength in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::channelNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshChannelNeighborsEdgeLength in NetCDF geometry "
                              "file %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelNodeX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelNodeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelNodeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelNodeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelNodeY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelNodeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelNodeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelNodeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelNodeZBank)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelNodeZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelNodeZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelNodeZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelNodeZBed)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelNodeZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelNodeZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelNodeZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementVertices)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementVertices", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementVertices in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize + 2 - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementVertices in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelVertexX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelVertexX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelVertexX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelVertexX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelVertexY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelVertexY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelVertexY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelVertexY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelVertexZBank)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelVertexZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelVertexZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelVertexZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelVertexZBed)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelVertexZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelVertexZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelVertexZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementZBank)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementZBed)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementLength)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementLength in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementLength in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelChannelNeighbors)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelChannelNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelChannelNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelChannelNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelMeshNeighbors)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelMeshNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelMeshNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelMeshNeighborsEdgeLength)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelMeshNeighborsEdgeLength in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelMeshNeighborsEdgeLength in NetCDF geometry "
                              "file %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          // Close file.
          if (fileOpen)
            {
              ncErrorCode = nc_close(fileID);
              fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to close NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        } // End if (writeGeometry).

      if (writeParameter)
        {
          // Create file name.
          if (!error)
            {
              numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: incorrect return value of snprintf when generating parameter file name %s.  "
                          "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Open file.
          if (!error)
            {
              ncErrorCode = nc_open(nameString, NC_NETCDF4 | NC_WRITE, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to open NetCDF parameter file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
              else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                {
                  fileOpen = true;
                }
            }

          // Get the number of existing instances.
          if (!error)
            {
              ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get dimension instances in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_dimlen(fileID, dimID, &parameterInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get length of dimension instances in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Write variables.
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "geometryInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable geometryInstance in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = parameterInstance;
              ncErrorCode = nc_put_var1_ulonglong(fileID, varID, start, &unsignedLongLongZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable geometryInstance in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (NULL != meshCatchment)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshCatchment", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshCatchment in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshCatchment in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshConductivity)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshConductivity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshConductivity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshConductivity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshPorosity)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshPorosity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshPorosity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshPorosity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshManningsN)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshManningsN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshManningsN in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshManningsN in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelChannelType)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelChannelType", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelChannelType in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelChannelType in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelPermanentCode)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelPermanentCode", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelPermenentCode in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelPermanentCode in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelBaseWidth)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelBaseWidth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelBaseWidth in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelBaseWidth in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelSideSlope)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelSideSlope", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelSideSlope in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelSideSlope in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelBedConductivity)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelBedConductivity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelBedConductivity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelBedConductivity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelBedThickness)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelBedThickness", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelBedThickness in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelBedThickness in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelManningsN)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelManningsN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelManningsN in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelManningsN in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          // Close file.
          if (fileOpen)
            {
              ncErrorCode = nc_close(fileID);
              fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to close NetCDF parameter file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        } // End if (writeParameter).

      if (writeState)
        {
          // Create file name.
          if (!error)
            {
              numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: incorrect return value of snprintf when generating state file name %s.  "
                          "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Open file.
          if (!error)
            {
              ncErrorCode = nc_open(nameString, NC_NETCDF4 | NC_WRITE, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to open NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
              else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                {
                  fileOpen = true;
                }
            }

          // Get the number of existing instances.
          if (!error)
            {
              ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get dimension instances in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_dimlen(fileID, dimID, &stateInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get length of dimension instances in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Write variables.
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "iteration", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable iteration in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_ushort(fileID, varID, start, &unsignedShortZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable iteration in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "currentTime", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable currentTime in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable currentTime in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "dt", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "geometryInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable geometryInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_ulonglong(fileID, varID, start, &unsignedLongLongZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable geometryInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "parameterInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable parameterInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_ulonglong(fileID, varID, start, &unsignedLongLongZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable parameterInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (NULL != meshSurfacewaterDepth)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshSurfacewaterDepth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshSurfacewaterDepth in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshSurfacewaterDepth in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshSurfacewaterError)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshSurfacewaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshSurfacewaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshSurfacewaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshGroundwaterHead)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshGroundwaterHead", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshGroundwaterHead in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshGroundwaterHead in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshGroundwaterError)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshGroundwaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshGroundwaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshGroundwaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelSurfacewaterDepth)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelSurfacewaterDepth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelSurfacewaterDepth in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelSurfacewaterDepth in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelSurfacewaterError)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelSurfacewaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelSurfacewaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelSurfacewaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          // Close file.
          if (fileOpen)
            {
              ncErrorCode = nc_close(fileID);
              fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to close NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        } // End if (writeState).

      // Delete nameString.
      if (NULL != nameString)
        {
          delete[] nameString;
        }

      if (!error)
        {
          thisProxy.writeFiles(directorySize, directory, writeGeometry, writeParameter, writeState);
        }
      else
        {
          CkExit();
        }
    } // End if (0 == CkMyPe()).
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      // This function should only be called on file manager zero.
      CkAssert(false);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
}

void FileManager::writeFiles(size_t directorySize, const char* directory, bool writeGeometry, bool writeParameter, bool writeState)
{
  bool   error      = false;     // Error flag.
  char*  nameString = NULL;      // Temporary string for file names.
  size_t nameStringSize;         // Size of buffer allocated for nameString.
  size_t numPrinted;             // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;            // Return value of NetCDF functions.
  int    fileID;                 // ID of NetCDF file.
  bool   fileOpen   = false;     // Whether fileID refers to an open file.
  int    dimID;                  // ID of dimension in NetCDF file.
  int    varID;                  // ID of variable in NetCDF file.
  size_t geometryInstance;       // Instance index for geometry file.
  size_t parameterInstance;      // Instance index for parameter file.
  size_t stateInstance;          // Instance index for state file.
  size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when writing to NetCDF file.
  size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when writing to NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::writeFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file name strings.
      nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // The longest file name is parameter.nc.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeFiles: incorrect return value of snprintf when generating geometry file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to open NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get dimension instances in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(fileID, dimID, &geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get length of dimension instances in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (writeGeometry)
    {
#ifdef NETCDF_COLLECTIVE_IO_WORKAROUND
      // The new instance was already created in resizeUnlimitedDimensions so use the last existing instance, not a new one.
      geometryInstance--;
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
      
      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "numberOfMeshNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable numberOfMeshNodes in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable numberOfMeshNodes in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          start[0]    = geometryInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_int(fileID, varID, start, count, &globalNumberOfMeshNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable numberOfMeshNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "numberOfMeshElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable numberOfMeshElements in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable numberOfMeshElements in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          start[0]    = geometryInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_int(fileID, varID, start, count, &globalNumberOfMeshElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable numberOfMeshElements in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "numberOfChannelNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable numberOfChannelNodes in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable numberOfChannelNodes in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          start[0]    = geometryInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_int(fileID, varID, start, count, &globalNumberOfChannelNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable numberOfChannelNodes in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "numberOfChannelElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable numberOfChannelElements in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable numberOfChannelElements in NetCDF geometry file "
                      "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          start[0]    = geometryInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_int(fileID, varID, start, count, &globalNumberOfChannelElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable numberOfChannelElements in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (NULL != meshNodeX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshNodeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshNodeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshNodeY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshNodeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshNodeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshNodeZSurface)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshNodeZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshNodeZSurface in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshNodeZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshNodeZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshNodeZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshNodeZBedrock)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshNodeZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshNodeZBedrock in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshNodeZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshNodeZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshNodeZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementVertices)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementVertices", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)meshElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshVertexX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshVertexX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshVertexY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshVertexY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshVertexZSurface)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshVertexZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshVertexZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshVertexZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshVertexZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshVertexZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshVertexZBedrock)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshVertexZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshVertexZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshVertexZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshVertexZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshVertexZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementZSurface)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementZBedrock)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementArea)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementArea", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementArea in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementArea in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementArea);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementArea in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementSlopeX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementSlopeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementSlopeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementSlopeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementSlopeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementSlopeY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementSlopeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementSlopeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementSlopeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementSlopeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighbors)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)meshMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighborsChannelEdge)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsChannelEdge", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighborsChannelEdge in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighborsChannelEdge in NetCDF geometry file"
                          " %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              // Assumes bool is one byte when casting to signed char.
              CkAssert(sizeof(bool) == sizeof(signed char));
              
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_schar(fileID, varID, start, count, (signed char*)meshMeshNeighborsChannelEdge);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighborsChannelEdge in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighborsEdgeLength)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighborsEdgeLength in NetCDF geometry file "
                          "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighborsEdgeNormalX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file"
                          " %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeNormalX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighborsEdgeNormalY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file"
                          " %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeNormalY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshChannelNeighbors)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::channelNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)meshChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshChannelNeighborsEdgeLength)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshChannelNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshChannelNeighborsEdgeLength in NetCDF geometry "
                          "file %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::channelNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshChannelNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshChannelNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelNodeX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelNodeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelNodeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelNodeY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelNodeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelNodeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelNodeZBank)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelNodeZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelNodeZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelNodeZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelNodeZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelNodeZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelNodeZBed)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelNodeZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelNodeZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelNodeZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelNodeZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelNodeZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementVertices)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementVertices", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize + 2;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)channelElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelVertexX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelVertexX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelVertexY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelVertexY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelVertexZBank)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelVertexZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelVertexZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelVertexZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelVertexZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelVertexZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelVertexZBed)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelVertexZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelVertexZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelVertexZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelVertexZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelVertexZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementZBank)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementZBed)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementLength)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelChannelNeighbors)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelChannelNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)channelChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelMeshNeighbors)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)channelMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelMeshNeighborsEdgeLength)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelMeshNeighborsEdgeLength in NetCDF geometry "
                          "file %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
    }
  else // if (!writeGeometry)
    {
      if (!error)
        {
          if (0 < geometryInstance)
            {
              // We're not creating a new instance so use the last instance with index one less than the dimension length.
              geometryInstance--;
              
              // FIXME check if the number of nodes and elements is consisitent?
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else
            {
              // We're not creating a new instance so it's an error if there's not an existing one.
              CkError("ERROR in FileManager::writeFiles: not creating a new instance and no existing instance in NetCDF geometry file %s.\n", nameString);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to close NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeFiles: incorrect return value of snprintf when generating parameter file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to open NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get dimension instances in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(fileID, dimID, &parameterInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get length of dimension instances in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (writeParameter)
    {
#ifdef NETCDF_COLLECTIVE_IO_WORKAROUND
      // The new instance was already created in resizeUnlimitedDimensions so use the last existing instance, not a new one.
      parameterInstance--;
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
      
      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "geometryInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable geometryInstance in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable geometryInstance in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          // Assumes size_t is eight bytes when casting to unsigned long long.
          CkAssert(sizeof(size_t) == sizeof(unsigned long long));

          start[0]    = parameterInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_ulonglong(fileID, varID, start, count, (unsigned long long *)&geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable geometryInstance in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (NULL != meshCatchment)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshCatchment", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshCatchment in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshCatchment in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, meshCatchment);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshCatchment in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshConductivity)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshConductivity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshPorosity)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshPorosity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshPorosity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshPorosity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshPorosity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshPorosity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshManningsN)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshManningsN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelChannelType)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelChannelType", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelChannelType in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelChannelType in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              // Assumes ChannelTypeEnum is four bytes when casting to int.
              CkAssert(sizeof(ChannelTypeEnum) == sizeof(int));
              
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)channelChannelType);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelChannelType in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelPermanentCode)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelPermanentCode", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelPermenentCode in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelPermenentCode in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, channelPermanentCode);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelPermanentCode in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelBaseWidth)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelBaseWidth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelBaseWidth in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelBaseWidth in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelBaseWidth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelBaseWidth in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelSideSlope)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelSideSlope", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelSideSlope in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelSideSlope in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelSideSlope);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelSideSlope in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelBedConductivity)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelBedConductivity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelBedConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelBedConductivity in NetCDF parameter file "
                          "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelBedConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelBedConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelBedThickness)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelBedThickness", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelBedThickness in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelBedThickness in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelBedThickness);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelBedThickness in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelManningsN)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelManningsN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
    }
  else // if (!writeParameter)
    {
      if (!error)
        {
          if (0 < parameterInstance)
            {
              // We're not creating a new instance so use the last instance with index one less than the dimension length.
              parameterInstance--;
              
              // FIXME check if the number of nodes and elements is consisitent?
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else
            {
              // We're not creating a new instance so it's an error if there's not an existing one.
              CkError("ERROR in FileManager::writeFiles: not creating a new instance and no existing instance in NetCDF parameter file %s.\n", nameString);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to close NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeFiles: incorrect return value of snprintf when generating state file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to open NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get dimension instances in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(fileID, dimID, &stateInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get length of dimension instances in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (writeState)
    {
#ifdef NETCDF_COLLECTIVE_IO_WORKAROUND
      // The new instance was already created in resizeUnlimitedDimensions so use the last existing instance, not a new one.
      stateInstance--;
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
      
      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "iteration", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable iteration in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable iteration in NetCDF state file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          start[0]    = stateInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_ushort(fileID, varID, start, count, &iteration);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable iteration in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "currentTime", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable currentTime in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable currentTime in NetCDF state file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          start[0]    = stateInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_double(fileID, varID, start, count, &currentTime);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable currentTime in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "dt", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable dt in NetCDF state file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          start[0]    = stateInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_double(fileID, varID, start, count, &dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "geometryInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable geometryInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable geometryInstance in NetCDF state file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          // Assumes size_t is eight bytes when casting to unsigned long long.
          CkAssert(sizeof(size_t) == sizeof(unsigned long long));

          start[0]    = stateInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_ulonglong(fileID, varID, start, count, (unsigned long long *)&geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable geometryInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_inq_varid(fileID, "parameterInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to get variable parameterInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
      if (!error)
        {
          ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable parameterInstance in NetCDF state file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

      if (!error)
        {
          // Assumes size_t is eight bytes when casting to unsigned long long.
          CkAssert(sizeof(size_t) == sizeof(unsigned long long));

          start[0]    = stateInstance;
          count[0]    = 1;
          ncErrorCode = nc_put_vara_ulonglong(fileID, varID, start, count, (unsigned long long *)&parameterInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::writeFiles: unable to write variable parameterInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                      nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (NULL != meshSurfacewaterDepth)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshSurfacewaterDepth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshSurfacewaterError)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshSurfacewaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshGroundwaterHead)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshGroundwaterHead", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshGroundwaterHead in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshGroundwaterHead in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshGroundwaterHead);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshGroundwaterHead in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshGroundwaterError)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshGroundwaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshGroundwaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshGroundwaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshGroundwaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshGroundwaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
     if (NULL != meshPrecipitationCumulative)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshPrecipitationCumulative", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshPrecipitationCumulative in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshPrecipitationCumulative in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshPrecipitationCumulative);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable precipitation in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
        
     if (NULL != meshEvaporationCumulative) 
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshEvaporationCumulative", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshEvaporationCumulative in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshEvaporationCumulative in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshEvaporationCumulative);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshEvaporationCumulative in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
        
      if (NULL != meshCanopyLiquid)  
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshCanopyLiquid", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshCanopyLiquid in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshCanopyLiquid in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshCanopyLiquid);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshCanopyLiquid in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
     
     if (NULL != meshCanopyIce) 
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshCanopyIce", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshCanopyIce in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshCanopyIce in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshCanopyIce);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable canopyIce in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
     
     if (NULL != meshSnowWaterEquivalent)  
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshSnowWaterEquivalent", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshSnowWaterEquivalent in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshSnowWaterEquivalent in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshSnowWaterEquivalent);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshSnowWaterEquivalent in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
        
     if (NULL != meshElementNeighborsSurfacewaterCumulativeFlow) 
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementNeighborsSurfacewaterCumulativeFlow", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementNeighborsSurfacewaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementNeighborsSurfacewaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshElementNeighborsSurfacewaterCumulativeFlow);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementNeighborsSurfacewaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
         
     if (NULL != meshElementNeighborsGroundwaterCumulativeFlow)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementNeighborsGroundwaterCumulativeFlow", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementNeighborsGroundwaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementNeighborsGroundwaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshElementNeighborsGroundwaterCumulativeFlow);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementNeighborsGroundwaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
         
      if (NULL != channelSurfacewaterDepth)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelSurfacewaterDepth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelSurfacewaterError)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelSurfacewaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
     
     if (NULL != channelEvaporationCumulative) 
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelEvaporationCumulative", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelEvaporationCumulative in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelEvaporationCumulative in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelEvaporationCumulative);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelEvaporationCumulative in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
        
     if (NULL != channelElementNeighborsSurfacewaterCumulativeFlow)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementNeighborsSurfacewaterCumulativeFlow", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementNeighborsSurfacewaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementNeighborsSurfacewaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelElementNeighborsSurfacewaterCumulativeFlow);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementNeighborsSurfacewaterCumulativeFlow in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
       
    } // End if (writeState).
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to close NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Delete nameString.
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

void FileManager::localStartAndNumber(int* localItemStart, int* localNumberOfItems, int globalNumberOfItems)
{
  int numPes              = CkNumPes();                           // Number of processors.
  int myPe                = CkMyPe();                             // The processor of this file manager.
  int numberOfFatOwners   = globalNumberOfItems % numPes;         // Number of file managers that own one extra item.
  int itemsPerFatOwner    = globalNumberOfItems / numPes + 1;     // Number of items in each file manager that owns one extra item.
  int itemsInAllFatOwners = numberOfFatOwners * itemsPerFatOwner; // Total number of items in all file managers that own one extra item.
  int itemsPerThinOwner   = globalNumberOfItems / numPes;         // Number of items in each file manager that does not own one extra item.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != localItemStart && NULL != localNumberOfItems && 0 <= globalNumberOfItems);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (myPe < numberOfFatOwners)
    {
      // I am a fat owner.
      *localItemStart     = myPe * itemsPerFatOwner;
      *localNumberOfItems = itemsPerFatOwner;
    }
  else
    {
      // I am a thin owner.
      *localItemStart     = (myPe - numberOfFatOwners) * itemsPerThinOwner + itemsInAllFatOwners;
      *localNumberOfItems = itemsPerThinOwner;
    }
}

bool FileManager::readNodeAndZFiles(const char* directory, const char* fileBasename, bool readMesh)
{
  bool   error      = false;  // Error flag.
  int    ii;                  // Loop counter.
  char*  nameString = NULL;   // Temporary string for file names.
  size_t nameStringSize;      // Size of buffer allocated for nameString.
  size_t numPrinted;          // Used to check that snprintf printed the correct number of characters.
  size_t numScanned;          // Used to check that fscanf scanned the correct number of inputs.
  FILE*  nodeFile   = NULL;   // The node file to read from.
  FILE*  zFile      = NULL;   // The z file to read from.
  int    globalNumberOfNodes; // The number of nodes to read.
  int    dimension;           // Used to check the dimensions in the files.
  int    numberOfAttributes;  // Used to check the number of attributes in the files.
  int    boundary;            // Used to check the number of boundary markers in the files.
  int    numberCheck;         // Used to check numbers that are error checked but otherwise unused.
  int    index;               // Used to read node and element numbers.
  int    firstIndex;          // Used to store if node numbers are zero based or one based.
  double xCoordinate;         // Used to read coordinates from file.
  double yCoordinate;         // Used to read coordinates from file.
  double zCoordinate;         // Used to read coordinates from file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != directory && NULL != fileBasename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  // Allocate space for file name strings.
  nameStringSize = strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".chan.node") + 1; // The longest file extension is .chan.node.
  nameString     = new char[nameStringSize];                                                          // +1 for null terminating character.

  // Create file name.
  numPrinted = snprintf(nameString, nameStringSize, "%s/%s%s", directory, fileBasename, (readMesh ? ".node" : ".chan.node"));

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(readMesh ? ".node" : ".chan.node") == numPrinted && numPrinted < nameStringSize))
    {
      CkError("ERROR in FileManager::readNodeAndZFiles: incorrect return value of snprintf when generating node file name %s.  %d should be equal to %d and "
              "less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(readMesh ? ".node" : ".chan.node"),
              nameStringSize);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  
  // Open file.
  if (!error)
    {
      nodeFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != nodeFile))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: could not open node file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(nodeFile, "%d %d %d %d", &globalNumberOfNodes, &dimension, &numberOfAttributes, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: unable to read header from node file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < globalNumberOfNodes && 2 == dimension && 0 == numberOfAttributes && 1 == boundary))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: invalid header in node file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/%s%s", directory, fileBasename, (readMesh ? ".z" : ".chan.z"));

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(readMesh ? ".z" : ".chan.z") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: incorrect return value of snprintf when generating z file name %s.  %d should be equal to %d and "
                  "less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(readMesh ? ".z" : ".chan.z"),
                  nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      zFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != zFile))
        {
          fprintf(stderr, "ERROR in FileManager::readNodeAndZFiles: could not open z file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(zFile, "%d %d", &numberCheck, &dimension);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: unable to read header from z file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfNodes == numberCheck && 1 == dimension))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: invalid header in z file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Calculate local start and number and allocate arrays.
  if (!error)
    {
      if (readMesh)
        {
          globalNumberOfMeshNodes = globalNumberOfNodes;
          
          localStartAndNumber(&localMeshNodeStart, &localNumberOfMeshNodes, globalNumberOfMeshNodes);

          meshNodeX        = new double[localNumberOfMeshNodes];
          meshNodeY        = new double[localNumberOfMeshNodes];
          meshNodeZSurface = new double[localNumberOfMeshNodes];
          meshNodeZBedrock = new double[localNumberOfMeshNodes];
        }
      else
        {
          globalNumberOfChannelNodes = globalNumberOfNodes;
          
          localStartAndNumber(&localChannelNodeStart, &localNumberOfChannelNodes, globalNumberOfChannelNodes);

          channelNodeX     = new double[localNumberOfChannelNodes];
          channelNodeY     = new double[localNumberOfChannelNodes];
          channelNodeZBank = new double[localNumberOfChannelNodes];
          channelNodeZBed  = new double[localNumberOfChannelNodes];
        }
    }
  
  // Read nodes.
  for (ii = 0; !error && ii < globalNumberOfNodes; ii++)
    {
      // Read node file.
      numScanned = fscanf(nodeFile, "%d %lf %lf %*d", &index, &xCoordinate, &yCoordinate);
      
      // Set zero based or one based indices.
      if (0 == ii)
        {
          if (0 == index)
            {
              firstIndex = 0;
            }
          else
            {
              firstIndex = 1;
            }
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: unable to read entry %d from node file.\n", ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii + firstIndex == index))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: invalid node number in node file.  %d should be %d.\n", index, ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Read z file.
      if (!error)
        {
          numScanned = fscanf(zFile, "%d %lf", &numberCheck, &zCoordinate);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(2 == numScanned))
            {
              CkError("ERROR in FileManager::readNodeAndZFiles: unable to read entry %d from z file.\n", ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(index == numberCheck))
            {
              CkError("ERROR in FileManager::readNodeAndZFiles: invalid node number in z file.  %d should be %d.\n", numberCheck, index);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      // Save values.
      if (!error)
        {
          index %= globalNumberOfNodes;
          
          if (readMesh)
            {
              if (localMeshNodeStart <= index && index < localMeshNodeStart + localNumberOfMeshNodes)
                {
                  meshNodeX[       index - localMeshNodeStart] = xCoordinate;
                  meshNodeY[       index - localMeshNodeStart] = yCoordinate;
                  meshNodeZSurface[index - localMeshNodeStart] = zCoordinate;
                  meshNodeZBedrock[index - localMeshNodeStart] = zCoordinate - 5.0; // FIXME figure out real soil depth.
                }
            }
          else
            {
              if (localChannelNodeStart <= index && index < localChannelNodeStart + localNumberOfChannelNodes)
                {
                  channelNodeX[    index - localChannelNodeStart] = xCoordinate;
                  channelNodeY[    index - localChannelNodeStart] = yCoordinate;
                  channelNodeZBank[index - localChannelNodeStart] = zCoordinate;
                  channelNodeZBed[ index - localChannelNodeStart] = zCoordinate - 5.0; // FIXME figure out real bank full depth.
                }
            }
        }
    } // End read nodes.
  
  // Close the files.
  if (NULL != nodeFile)
    {
      fclose(nodeFile);
    }

  if (NULL != zFile)
    {
      fclose(zFile);
    }

  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
    }
  
  return error;
}

void FileManager::handleInitializeFromASCIIFiles(size_t directorySize, const char* directory, size_t fileBasenameSize, const char* fileBasename)
{
  bool   error       = false;      // Error flag.
  int    ii, jj, kk, ll, mm, nn;   // Loop counters.
  char*  nameString  = NULL;       // Temporary string for file names.
  size_t nameStringSize;           // Size of buffer allocated for nameString.
  size_t numPrinted;               // Used to check that snprintf printed the correct number of characters.
  size_t numScanned;               // Used to check that fscanf scanned the correct number of inputs.
  FILE*  eleFile     = NULL;       // The ele file to read from.
  FILE*  neighFile   = NULL;       // The neigh file to read from.
  FILE*  chanEleFile = NULL;       // The chan.ele file to read from.
  int    dimension;                // Used to check the dimensions in the files.
  int    numberOfAttributes;       // Used to check the number of attributes in the files.
  int    numberCheck;              // Used to check numbers that are error checked but otherwise unused.
  int    index;                    // Used to read node and element numbers.
  int    firstIndex;               // Used to store if element numbers are zero based or one based.
  int    type;                     // Used to read channel type.  Read as an int because reading as a ChannelTypeEnum gives a warning.
  int    permanent;                // Used to read channel permanent code.
  double length;                   // Used to read channel length
  int    numberOfVertices;         // The number of vertices for a particular channel element.
  int    vertex0;                  // A vertex of an element.
  int    vertex1;                  // A vertex of an element.
  int    vertex2;                  // A vertex of an element.
  int    catchment;                // The catchment of an element.
  int    numberOfChannelNeighbors; // The number of channel neighbors for a particular channel element.
  int    numberOfMeshNeighbors;    // The number of mesh neighbors for a particular channel element.
  int    neighbor0;                // A neighbor of an element.
  int    neighbor1;                // A neighbor of an element.
  int    neighbor2;                // A neighbor of an element.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: directory must not be null.\n");
      error = true;
    }
  
  if (!(NULL != fileBasename))
    {
      CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: fileBasename must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // Read mesh nodes.
  if (!error)
    {
      // FIXME output firstIndex from node and z files and make sure element files are consistent?
      error = readNodeAndZFiles(directory, fileBasename, true);
    }

  if (!error)
    {
      // Allocate space for file name strings.
      //nameStringSize = strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".chan.ele") + 1; // The longest file extension is .chan.ele.
      nameStringSize = strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".chan.node") + 1; // FIXME A kludge below uses .chan.node.
      nameString     = new char[nameStringSize];                                                         // +1 for null terminating character.

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/%s.ele", directory, fileBasename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".ele") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: incorrect return value of snprintf when generating ele file name %s.  %d should be "
                  "equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".ele"),
                  nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      eleFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != eleFile))
        {
          fprintf(stderr, "ERROR in FileManager::handleInitializeFromASCIIFiles: could not open ele file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(eleFile, "%d %d %d", &globalNumberOfMeshElements, &dimension, &numberOfAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read header from ele file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < globalNumberOfMeshElements && 3 == dimension && 1 == numberOfAttributes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid header in ele file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/%s.neigh", directory, fileBasename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".neigh") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: incorrect return value of snprintf when generating neigh file name %s.  %d should be "
                  "equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".neigh"),
                  nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      neighFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != neighFile))
        {
          fprintf(stderr, "ERROR in FileManager::handleInitializeFromASCIIFiles: could not open neigh file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(neighFile, "%d %d", &numberCheck, &dimension);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read header from neigh file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck && 3 == dimension))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid header in neigh file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Calculate local start and number and allocate arrays.
  if (!error)
    {
      localStartAndNumber(&localMeshElementStart, &localNumberOfMeshElements, globalNumberOfMeshElements);
      
      meshElementVertices            = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      meshCatchment                  = new int[localNumberOfMeshElements];
      meshConductivity               = new double[localNumberOfMeshElements];
      meshPorosity                   = new double[localNumberOfMeshElements];
      meshManningsN                  = new double[localNumberOfMeshElements];
      meshSurfacewaterDepth          = new double[localNumberOfMeshElements];
      meshSurfacewaterError          = new double[localNumberOfMeshElements];
      // meshGroundwaterHead will be allocated and set in finishCalculateDerivedValues where it will default to saturated.
      meshGroundwaterError           = new double[localNumberOfMeshElements];
      meshMeshNeighbors              = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      meshMeshNeighborsChannelEdge   = new bool[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      meshChannelNeighbors           = new int[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
      meshChannelNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
    }
  
  // Read mesh elements.
  for (ii = 0; !error && ii < globalNumberOfMeshElements; ii++)
    {
      numScanned = fscanf(eleFile, "%d %d %d %d %d", &index, &vertex0, &vertex1, &vertex2, &catchment);
      
      // Set zero based or one based indices.
      if (0 == ii)
        {
          if (0 == index)
            {
              firstIndex = 0;
            }
          else
            {
              firstIndex = 1;
            }
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(5 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from ele file.\n", ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii + firstIndex == index))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in ele file.  %d should be %d.\n", index, ii + firstIndex);
          error = true;
        }
      
      // FIXME I can do tighter bounds checking if I know that firstIndex is consistent between node files and element files.
      //if (!(firstIndex <= vertex0 && vertex0 < globalNumberOfMeshNodes + firstIndex))
      if (!(0 <= vertex0 && vertex0 <= globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid vertex number %d in ele file.\n", vertex0);
          error = true;
        }
      
      //if (!(firstIndex <= vertex1 && vertex1 <= globalNumberOfMeshNodes + firstIndex))
      if (!(0 <= vertex1 && vertex1 <= globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid vertex number %d in ele file.\n", vertex1);
          error = true;
        }
      
      //if (!(firstIndex <= vertex2 && vertex2 <= globalNumberOfMeshNodes + firstIndex))
      if (!(0 <= vertex2 && vertex2 <= globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid vertex number %d in ele file.\n", vertex2);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Read neigh file.
      numScanned = fscanf(neighFile, "%d %d %d %d", &numberCheck, &neighbor0, &neighbor1, &neighbor2);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from neigh file.\n", ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in neigh file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }
      
      if (!(isBoundary(neighbor0) || (firstIndex <= neighbor0 && neighbor0 <= globalNumberOfMeshElements + firstIndex)))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid neighbor number %d in neigh file.\n", neighbor0);
          error = true;
        }
      
      if (!(isBoundary(neighbor1) || (firstIndex <= neighbor1 && neighbor1 <= globalNumberOfMeshElements + firstIndex)))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid neighbor number %d in neigh file.\n", neighbor1);
          error = true;
        }
      
      if (!(isBoundary(neighbor2) || (firstIndex <= neighbor2 && neighbor2 <= globalNumberOfMeshElements + firstIndex)))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid neighbor number %d in neigh file.\n", neighbor2);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Save values.
      if (!error)
        {
          index %= globalNumberOfMeshElements;
          
          if (localMeshElementStart <= index && index < localMeshElementStart + localNumberOfMeshElements)
            {
              meshElementVertices[         index - localMeshElementStart][0] = vertex0 % globalNumberOfMeshNodes;
              meshElementVertices[         index - localMeshElementStart][1] = vertex1 % globalNumberOfMeshNodes;
              meshElementVertices[         index - localMeshElementStart][2] = vertex2 % globalNumberOfMeshNodes;
              meshCatchment[               index - localMeshElementStart]    = catchment;
              
              // FIXME fill these in with real data.
              meshConductivity[            index - localMeshElementStart]    = 5.55e-4;
              meshPorosity[                index - localMeshElementStart]    = 0.5;
              meshManningsN[               index - localMeshElementStart]    = 0.038;
              
              meshSurfacewaterDepth[       index - localMeshElementStart]    = 0.0;
              meshSurfacewaterError[       index - localMeshElementStart]    = 0.0;
              meshGroundwaterError[        index - localMeshElementStart]    = 0.0;
              meshMeshNeighbors[           index - localMeshElementStart][0] = isBoundary(neighbor0) ? neighbor0 : neighbor0 % globalNumberOfMeshElements;
              meshMeshNeighbors[           index - localMeshElementStart][1] = isBoundary(neighbor1) ? neighbor1 : neighbor1 % globalNumberOfMeshElements;
              meshMeshNeighbors[           index - localMeshElementStart][2] = isBoundary(neighbor2) ? neighbor2 : neighbor2 % globalNumberOfMeshElements;
              
              // Channel edges and neighbors are filled in later when we read the edge file.  Initialize them here to no channels.
              meshMeshNeighborsChannelEdge[index - localMeshElementStart][0] = false;
              meshMeshNeighborsChannelEdge[index - localMeshElementStart][1] = false;
              meshMeshNeighborsChannelEdge[index - localMeshElementStart][2] = false;
              
              for (jj = 0; jj < MeshElement::channelNeighborsSize; jj++)
                {
                  meshChannelNeighbors[          index - localMeshElementStart][jj] = NOFLOW;
                  meshChannelNeighborsEdgeLength[index - localMeshElementStart][jj] = 1.0;
                }
            }
        }
    } // End read mesh elements.
  
  // Read channel nodes.
  if (!error)
    {
      error = readNodeAndZFiles(directory, fileBasename, false);
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/%s.chan.ele", directory, fileBasename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".chan.ele") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: incorrect return value of snprintf when generating chan.ele file name %s.  %d should "
                  "be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".chan.ele"),
                  nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      chanEleFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != chanEleFile))
        {
          fprintf(stderr, "ERROR in FileManager::handleInitializeFromASCIIFiles: could not open chan.ele file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(chanEleFile, "%d", &globalNumberOfChannelElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(1 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read header from chan.ele file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < globalNumberOfChannelElements))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid header in chan.ele file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Calculate local start and number and allocate arrays.
  if (!error)
    {
      localStartAndNumber(&localChannelElementStart, &localNumberOfChannelElements, globalNumberOfChannelElements);
      
      channelElementVertices         = new int[localNumberOfChannelElements][ChannelElement::channelVerticesSize + 2];
      channelElementX                = new double[localNumberOfChannelElements];
      channelElementY                = new double[localNumberOfChannelElements];
      channelElementZBank            = new double[localNumberOfChannelElements];
      channelElementZBed             = new double[localNumberOfChannelElements];
      channelElementLength           = new double[localNumberOfChannelElements];
      channelChannelType             = new ChannelTypeEnum[localNumberOfChannelElements];
      channelPermanentCode           = new int[localNumberOfChannelElements];
      channelBaseWidth               = new double[localNumberOfChannelElements];
      channelSideSlope               = new double[localNumberOfChannelElements];
      channelBedConductivity         = new double[localNumberOfChannelElements];
      channelBedThickness            = new double[localNumberOfChannelElements];
      channelManningsN               = new double[localNumberOfChannelElements];
      channelSurfacewaterDepth       = new double[localNumberOfChannelElements];
      channelSurfacewaterError       = new double[localNumberOfChannelElements];
      channelChannelNeighbors        = new int[localNumberOfChannelElements][ChannelElement::channelNeighborsSize];
      channelMeshNeighbors           = new int[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
      channelMeshNeighborsEdgeLength = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
    }
  
  // Read channel elements.
  for (ii = 0; !error && ii < globalNumberOfChannelElements; ii++)
    {
      numScanned = fscanf(chanEleFile, "%d %d %d %lf %d %d %d %d", &index, &type, &permanent, &length, &vertex0, &numberOfVertices,
                          &numberOfChannelNeighbors, &numberOfMeshNeighbors);
      
      // Set zero based or one based indices.
      if (0 == ii)
        {
          if (0 == index)
            {
              firstIndex = 0;
            }
          else
            {
              firstIndex = 1;
            }
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(8 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii + firstIndex == index))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in chan.ele file.  %d should be %d.\n", index, ii + firstIndex);
          error = true;
        }
      
      if (!(STREAM == type || WATERBODY == type || ICEMASS == type))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: channel type must be a valid enum value in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0 < permanent))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: permenent code must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0.0 < length))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: length must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      // FIXME I can do tighter bounds checking if I know that firstIndex is consistent between node files and element files.
      //if (!(firstIndex <= vertex0 && vertex0 < globalNumberOfMeshNodes + firstIndex))
      if (!(0 <= vertex0 && vertex0 <= globalNumberOfChannelNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element center node number %d in ele file.\n", vertex0);
          error = true;
        }
      
      if (!(0 < numberOfVertices && numberOfVertices <= ChannelElement::channelVerticesSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: numberOfVertices must be greater than zero and less than or "
                  "equal to the maximum number of channel vertices %d in chan.ele file.\n", index, ChannelElement::channelVerticesSize);
          error = true;
        }
      
      if (!(0 <= numberOfChannelNeighbors && numberOfChannelNeighbors <= ChannelElement::channelNeighborsSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: numberOfChannelNeighbors must be greater than or equal to zero and less "
                  "than or equal to the maximum number of channel channel neighbors %d in chan.ele file.\n", index, ChannelElement::channelNeighborsSize);
          error = true;
        }
      
      if (!(0 <= numberOfMeshNeighbors && numberOfMeshNeighbors <= ChannelElement::meshNeighborsSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: numberOfMeshNeighbors must be greater than or equal to zero and less "
                  "than or equal to the maximum number of channel mesh neighbors %d in chan.ele file.\n", index, ChannelElement::meshNeighborsSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Save values.
      if (!error)
        {
          index %= globalNumberOfChannelElements;
          
          if (localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
            {
              // Use polyline (code 2) for STREAM and polygon (code 3) for WATERBODY and ICEMASS
              channelElementVertices[index - localChannelElementStart][0] = (STREAM == type) ? 2 : 3;
              channelElementVertices[index - localChannelElementStart][1] = ChannelElement::channelVerticesSize;

              // FIXME this is a total kludge to get the element center node if it is not owned by me by re-reading the entire node file.
              if (localChannelNodeStart <= vertex0 && vertex0 < localChannelNodeStart + localNumberOfChannelNodes)
                {
                  channelElementX[    index - localChannelElementStart] = channelNodeX[    vertex0 - localChannelNodeStart];
                  channelElementY[    index - localChannelElementStart] = channelNodeY[    vertex0 - localChannelNodeStart];
                  channelElementZBank[index - localChannelElementStart] = channelNodeZBank[vertex0 - localChannelNodeStart];
                  channelElementZBed[ index - localChannelElementStart] = channelNodeZBed[ vertex0 - localChannelNodeStart];
                }
              else
                {
                  snprintf(nameString, nameStringSize, "%s/%s.chan.node", directory, fileBasename);
                  FILE* nodeFile = fopen(nameString, "r");
                  fscanf(nodeFile, "%*d %*d %*d %*d");
                  snprintf(nameString, nameStringSize, "%s/%s.chan.z", directory, fileBasename);
                  FILE* zFile = fopen(nameString, "r");
                  fscanf(zFile, "%*d %*d");
                  for (jj = 0; jj < vertex0; jj++)
                    {
                      fscanf(nodeFile, "%*d %*f %*f %*d");
                      fscanf(zFile, "%*d %*f");
                    }
                  fscanf(nodeFile, "%*d %lf %lf %*d", &channelElementX[index - localChannelElementStart], &channelElementY[index - localChannelElementStart]);
                  fscanf(zFile, "%*d %lf", &channelElementZBank[index - localChannelElementStart]);
                  channelElementZBed[index - localChannelElementStart] = channelElementZBank[index - localChannelElementStart] - 5.0; // FIXME figure out real bank full depth.
                  fclose(nodeFile);
                  fclose(zFile);
                }
              
              channelElementLength[    index - localChannelElementStart] = length;
              channelChannelType[      index - localChannelElementStart] = (ChannelTypeEnum)type;
              channelPermanentCode[    index - localChannelElementStart] = permanent;
              
              // FIXME fill in real values.
              channelBaseWidth[        index - localChannelElementStart] = 1.0;
              channelSideSlope[        index - localChannelElementStart] = 1.0;
              channelBedConductivity[  index - localChannelElementStart] = 5.55e-4;
              channelBedThickness[     index - localChannelElementStart] = 1.0;
              channelManningsN[        index - localChannelElementStart] = 0.038;
              
              channelSurfacewaterDepth[index - localChannelElementStart] = 0.0;
              channelSurfacewaterError[index - localChannelElementStart] = 0.0;
              
              // Fill in unused neighbors with NOFLOW.
              for (jj = 0; jj < ChannelElement::channelNeighborsSize; jj++)
                {
                  channelChannelNeighbors[index - localChannelElementStart][jj] = NOFLOW;
                }
              
              for (jj = 0; jj < ChannelElement::meshNeighborsSize; jj++)
                {
                  channelMeshNeighbors[          index - localChannelElementStart][jj] = NOFLOW;
                  channelMeshNeighborsEdgeLength[index - localChannelElementStart][jj] = 1.0;
                }
            }
        }
      
      // Read vertices.
      for (jj = 0; !error && jj < numberOfVertices; jj++)
        {
          numScanned = fscanf(chanEleFile, "%d", &vertex0);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(1 == numScanned))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          // FIXME I can do tighter bounds checking if I know that firstIndex is consistent between node files and element files.
          //if (!(firstIndex <= vertex0 && vertex0 < globalNumberOfMeshNodes + firstIndex))
          if (!(0 <= vertex0 && vertex0 <= globalNumberOfChannelNodes))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid vertex number %d in ele file.\n", vertex0);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

          if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
            {
              channelElementVertices[index - localChannelElementStart][jj + 2] = vertex0 % globalNumberOfChannelNodes;
            }
        }
      
      // Fill in unused vertices by repeating the last used vertex.
      if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
        {
          for (jj = numberOfVertices; jj < ChannelElement::channelVerticesSize; jj++)
            {
              channelElementVertices[index - localChannelElementStart][jj + 2] =
                  channelElementVertices[index - localChannelElementStart][(numberOfVertices - 1) + 2];
            }
        }
      
      // Read channel neighbors.
      for (jj = 0; !error && jj < numberOfChannelNeighbors; jj++)
        {
          numScanned = fscanf(chanEleFile, "%d", &neighbor0);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(1 == numScanned))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(isBoundary(neighbor0) || (firstIndex <= neighbor0 && neighbor0 <= globalNumberOfChannelElements + firstIndex)))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: invalid neighbor number %d in chan.ele file.\n",
                      ii + firstIndex, neighbor0);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

          if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
            {
              channelChannelNeighbors[index - localChannelElementStart][jj] = neighbor0 % globalNumberOfChannelElements;
            }
        }
      
      // Read mesh neighbors.
      for (jj = 0; !error && jj < numberOfMeshNeighbors; jj++)
        {
          fscanf(chanEleFile, "%d %lf", &neighbor0, &length);
          
          if (-1 != neighbor0)
            {
              // FIXME this is a total kludge to get the vertices of the mesh edge.
              snprintf(nameString, nameStringSize, "%s/%s.edge", directory, fileBasename);
              FILE* edgeFile = fopen(nameString, "r");
              fscanf(edgeFile, "%*d %*d");
              for (kk = 0; kk < neighbor0 - 1; kk++) // FIXME Assumes one based file.
                {
                  fscanf(edgeFile, "%*d %*d %*d %*d");
                }
              fscanf(edgeFile, "%*d %d %d %*d", &vertex0, &vertex1);
              fclose(edgeFile);
              
              vertex0 %= globalNumberOfMeshNodes;
              vertex1 %= globalNumberOfMeshNodes;
              
              // Search for those vertices in the mesh.
              // FIXME this is a kludge that won't work on more than one processor.
              CkAssert(localNumberOfMeshNodes == globalNumberOfMeshNodes && localNumberOfMeshElements == globalNumberOfMeshElements);
              
              // Convert length from the fraction of the mesh edge to meters.
              length *= sqrt((meshNodeX[vertex0] - meshNodeX[vertex1]) * (meshNodeX[vertex0] - meshNodeX[vertex1]) +
                             (meshNodeY[vertex0] - meshNodeY[vertex1]) * (meshNodeY[vertex0] - meshNodeY[vertex1]));
              
              for (kk = 0; kk < globalNumberOfMeshElements; kk++)
                {
                  for (ll = 0; ll < MeshElement::meshNeighborsSize; ll++)
                    {
                      if ((vertex0 == meshElementVertices[kk][(ll + 1) % MeshElement::meshNeighborsSize] &&
                           vertex1 == meshElementVertices[kk][(ll + 2) % MeshElement::meshNeighborsSize]) ||
                          (vertex0 == meshElementVertices[kk][(ll + 2) % MeshElement::meshNeighborsSize] &&
                           vertex1 == meshElementVertices[kk][(ll + 1) % MeshElement::meshNeighborsSize]))
                        {
                          meshMeshNeighborsChannelEdge[kk][ll] = true;
                          
                          mm = 0;
                          
                          while (mm < ChannelElement::meshNeighborsSize && kk != channelMeshNeighbors[index - localChannelElementStart][mm] &&
                                 NOFLOW != channelMeshNeighbors[index - localChannelElementStart][mm])
                            {
                              mm++;
                            }
                          
                          if (mm < ChannelElement::meshNeighborsSize)
                            {
                              if (kk == channelMeshNeighbors[index - localChannelElementStart][mm])
                                {
                                  // A channel element can touch a mesh element on more than one edge.  Do not create duplicate neighbor entries.  Just add the
                                  // length.
                                  channelMeshNeighborsEdgeLength[index - localChannelElementStart][mm] += length;
                                }
                              else
                                {
                                  channelMeshNeighbors[          index - localChannelElementStart][mm] = kk;
                                  channelMeshNeighborsEdgeLength[index - localChannelElementStart][mm] = length;
                                }
                              
                              nn = 0;
                              
                              while (nn < MeshElement::channelNeighborsSize && index != meshChannelNeighbors[kk][nn] && NOFLOW != meshChannelNeighbors[kk][nn])
                                {
                                  nn++;
                                }
                              
                              if (nn < MeshElement::channelNeighborsSize)
                                {
                                  if (index == meshChannelNeighbors[kk][nn])
                                    {
                                      // A channel element can touch a mesh element on more than one edge.  Do not create duplicate neighbor entries.  Just add the
                                      // length.
                                      meshChannelNeighborsEdgeLength[kk][nn] += length;
                                    }
                                  else
                                    {
                                      meshChannelNeighbors[kk][nn]           = index;
                                      meshChannelNeighborsEdgeLength[kk][nn] = length;
                                    }
                                }
                              else
                                {
                                  CkError("ERROR in FileManager::handleInitializeFromASCIIFiles mesh element %d: element has more than maximum %d channel "
                                          "neighbors.\n", kk, MeshElement::channelNeighborsSize);
                                  CkExit();
                                }
                            }
                          else
                            {
                              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles channel element %d: element has more than maximum %d mesh "
                                      "neighbors.\n", index, ChannelElement::meshNeighborsSize);
                              CkExit();
                            }
                        }
                    }
                }
            }
        }
    } // End read channel elements.
  
  // This is a kludge to link the waterbody channel neighbors that aren't in the .chan.ele file yet.
  // FIXME this is a kludge that won't work on more than one processor.
  CkAssert(localNumberOfMeshNodes == globalNumberOfMeshNodes && localNumberOfMeshElements == globalNumberOfMeshElements);
  
  snprintf(nameString, nameStringSize, "%s/%s.edge", directory, fileBasename);
  FILE* edgeFile = fopen(nameString, "r");
  int numberOfEdges;
  int boundary;
  fscanf(edgeFile, "%d %*d", &numberOfEdges);
  for (jj = 0; jj < numberOfEdges; jj++)
    {
      fscanf(edgeFile, "%*d %d %d %d", &vertex0, &vertex1, &boundary);
      
      if (0 > boundary)
        {
          // This is a channel edge.  Find its channel from its permanent code.
          snprintf(nameString, nameStringSize, "%s/%s.link", directory, fileBasename);
          FILE* linkFile = fopen(nameString, "r");
          fscanf(linkFile, "%*d %*d");
          for (kk = 0; kk < -boundary - 1; kk++) // FIXME Assumes one based file.
            {
              fscanf(linkFile, "%*d %*d");
            }
          fscanf(linkFile, "%*d %d", &boundary);
          fclose(linkFile);
          
          ii = 0;
          
          while (ii < globalNumberOfChannelElements && boundary != channelPermanentCode[ii])
            {
              ii++;
            }
          
          if (ii < globalNumberOfChannelElements)
            {
              if (WATERBODY == channelChannelType[ii] || ICEMASS == channelChannelType[ii])
                {
                  // Search for those vertices in the mesh.
                  // FIXME this is a kludge that won't work on more than one processor.
                  CkAssert(localNumberOfMeshNodes == globalNumberOfMeshNodes && localNumberOfMeshElements == globalNumberOfMeshElements);
                  
                  // Convert length from the fraction of the mesh edge to meters.
                  length = sqrt((meshNodeX[vertex0] - meshNodeX[vertex1]) * (meshNodeX[vertex0] - meshNodeX[vertex1]) +
                                (meshNodeY[vertex0] - meshNodeY[vertex1]) * (meshNodeY[vertex0] - meshNodeY[vertex1]));
                  
                  for (kk = 0; kk < globalNumberOfMeshElements; kk++)
                    {
                      for (ll = 0; ll < MeshElement::meshNeighborsSize; ll++)
                        {
                          if ((vertex0 == meshElementVertices[kk][(ll + 1) % MeshElement::meshNeighborsSize] &&
                               vertex1 == meshElementVertices[kk][(ll + 2) % MeshElement::meshNeighborsSize]) ||
                              (vertex0 == meshElementVertices[kk][(ll + 2) % MeshElement::meshNeighborsSize] &&
                               vertex1 == meshElementVertices[kk][(ll + 1) % MeshElement::meshNeighborsSize]))
                            {
                              meshMeshNeighborsChannelEdge[kk][ll] = true;
                              
                              mm = 0;
                              
                              while (mm < ChannelElement::meshNeighborsSize && kk != channelMeshNeighbors[ii][mm] && NOFLOW != channelMeshNeighbors[ii][mm])
                                {
                                  mm++;
                                }
                              
                              if (mm < ChannelElement::meshNeighborsSize)
                                {
                                  if (kk == channelMeshNeighbors[ii][mm])
                                    {
                                      // A channel element can touch a mesh element on more than one edge.  Do not create duplicate neighbor entries.  Just add
                                      // the length.
                                      channelMeshNeighborsEdgeLength[ii][mm] += length;
                                    }
                                  else
                                    {
                                      channelMeshNeighbors[          ii][mm] = kk;
                                      channelMeshNeighborsEdgeLength[ii][mm] = length;
                                    }
                                  
                                  nn = 0;
                                  
                                  while (nn < MeshElement::channelNeighborsSize && ii != meshChannelNeighbors[kk][nn] && NOFLOW != meshChannelNeighbors[kk][nn])
                                    {
                                      nn++;
                                    }
                                  
                                  if (nn < MeshElement::channelNeighborsSize)
                                    {
                                      if (ii == meshChannelNeighbors[kk][nn])
                                        {
                                          // A channel element can touch a mesh element on more than one edge.  Do not create duplicate neighbor entries.  Just add the
                                          // length.
                                          meshChannelNeighborsEdgeLength[kk][nn] += length;
                                        }
                                      else
                                        {
                                          meshChannelNeighbors[kk][nn]           = ii;
                                          meshChannelNeighborsEdgeLength[kk][nn] = length;
                                        }
                                    }
                                  else
                                    {
                                      CkError("ERROR in FileManager::handleInitializeFromASCIIFiles mesh element %d: element has more than maximum %d channel "
                                              "neighbors.\n", kk, MeshElement::channelNeighborsSize);
                                      CkExit();
                                    }
                                }
                              else
                                {
                                  CkError("ERROR in FileManager::handleInitializeFromASCIIFiles channel element %d: element has more than maximum %d mesh "
                                          "neighbors.\n", ii, ChannelElement::meshNeighborsSize);
                                  CkExit();
                                }
                            }
                        }
                    }
                }
            }
          else
            {
              CkError("WARNING in FileManager::handleInitializeFromASCIIFiles channel edge %d: permanent code %d does not exist in channel network.\n", jj, boundary);
            }
        }
    }
  fclose(edgeFile);
  
  // Close the files.
  if (NULL != eleFile)
    {
      fclose(eleFile);
    }

  if (NULL != neighFile)
    {
      fclose(neighFile);
    }

  if (NULL != chanEleFile)
    {
      fclose(chanEleFile);
    }

  if (!error)
    {
      meshElementUpdated    = new bool[localNumberOfMeshElements];
      channelElementUpdated = new bool[localNumberOfChannelElements];
    }
  
  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
    }
  
  // Have to call evapoTranspirationInit once on each Pe. This is a convenient place to do that.
  if (!error)
    {
      error = evapoTranspirationInit(directory);
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

void FileManager::handleInitializeFromNetCDFFiles(size_t directorySize, const char* directory)
{
  bool   error         = false;  // Error flag.
  char*  nameString    = NULL;   // Temporary string for file names.
  size_t nameStringSize;         // Size of buffer allocated for nameString.
  size_t numPrinted;             // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;            // Return value of NetCDF functions.
  int    stateFileID;            // ID of NetCDF file.
  bool   stateFileOpen = false;  // Whether stateFileID refers to an open file.
  int    fileID;                 // ID of NetCDF file.
  bool   fileOpen      = false;  // Whether fileID refers to an open file.
  int    dimID;                  // ID of dimension in NetCDF file.
  int    varID;                  // ID of variable in NetCDF file.
  size_t geometryInstance;       // Instance index for geometry file.
  size_t parameterInstance;      // Instance index for parameter file.
  size_t stateInstance;          // Instance index for state file.
  size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when writing to NetCDF file.
  size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when writing to NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file name strings.
      nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // The longest file name is parameter.nc.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: incorrect return value of snprintf when generating state file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &stateFileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to open NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          stateFileOpen = true;
        }
    }
  
  // Get the number of existing instances.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(stateFileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get dimension instances in NetCDF state file %s.  NetCDF error message: "
                  "%s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(stateFileID, dimID, &stateInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get length of dimension instances in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      if (0 < stateInstance)
        {
          // We're not creating a new instance so use the last instance with index one less than the dimension length.
          stateInstance--;
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          // We're not creating a new instance so it's an error if there's not an existing one.
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: not creating a new instance and no existing instance in NetCDF state file %s.\n",
                  nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Read variables.
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "iteration", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable iteration in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_ushort(stateFileID, varID, start, count, &iteration);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable iteration in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "currentTime", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable currentTime in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_double(stateFileID, varID, start, count, &currentTime);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable currentTime in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "dt", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable dt in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_double(stateFileID, varID, start, count, &dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable dt in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "geometryInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable geometryInstance in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      // Assumes size_t is eight bytes when casting to unsigned long long.
      CkAssert(sizeof(size_t) == sizeof(unsigned long long));
      
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_ulonglong(stateFileID, varID, start, count, (unsigned long long *)&geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable geometryInstance in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "parameterInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable parameterInstance in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      // Assumes size_t is eight bytes when casting to unsigned long long.
      CkAssert(sizeof(size_t) == sizeof(unsigned long long));
      
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_ulonglong(stateFileID, varID, start, count, (unsigned long long *)&parameterInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable parameterInstance in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: incorrect return value of snprintf when generating geometry file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to open NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Read variables.
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "numberOfMeshNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable numberOfMeshNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = geometryInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_int(fileID, varID, start, count, &globalNumberOfMeshNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable numberOfMeshNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "numberOfMeshElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable numberOfMeshElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = geometryInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_int(fileID, varID, start, count, &globalNumberOfMeshElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable numberOfMeshElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "numberOfChannelNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable numberOfChannelNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = geometryInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_int(fileID, varID, start, count, &globalNumberOfChannelNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable numberOfChannelNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "numberOfChannelElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to get variable numberOfChannelElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = geometryInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_int(fileID, varID, start, count, &globalNumberOfChannelElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable numberOfChannelElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      localStartAndNumber(&localMeshNodeStart,       &localNumberOfMeshNodes,       globalNumberOfMeshNodes);
      localStartAndNumber(&localMeshElementStart,    &localNumberOfMeshElements,    globalNumberOfMeshElements);
      localStartAndNumber(&localChannelNodeStart,    &localNumberOfChannelNodes,    globalNumberOfChannelNodes);
      localStartAndNumber(&localChannelElementStart, &localNumberOfChannelElements, globalNumberOfChannelElements);

      ncErrorCode = nc_inq_varid(fileID, "meshNodeX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshNodeX   = new double[localNumberOfMeshNodes];
          start[0]    = geometryInstance;
          start[1]    = localMeshNodeStart;
          count[0]    = 1;
          count[1]    = localNumberOfMeshNodes;
          ncErrorCode = nc_get_vara_double(fileID, varID, start, count, meshNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshNodeX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshNodeY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshNodeY   = new double[localNumberOfMeshNodes];
          start[0]    = geometryInstance;
          start[1]    = localMeshNodeStart;
          count[0]    = 1;
          count[1]    = localNumberOfMeshNodes;
          ncErrorCode = nc_get_vara_double(fileID, varID, start, count, meshNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshNodeY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshNodeZSurface", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshNodeZSurface = new double[localNumberOfMeshNodes];
          start[0]         = geometryInstance;
          start[1]         = localMeshNodeStart;
          count[0]         = 1;
          count[1]         = localNumberOfMeshNodes;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, meshNodeZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshNodeZSurface in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshNodeZBedrock", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshNodeZBedrock = new double[localNumberOfMeshNodes];
          start[0]         = geometryInstance;
          start[1]         = localMeshNodeStart;
          count[0]         = 1;
          count[1]         = localNumberOfMeshNodes;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, meshNodeZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshNodeZBedrock in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementVertices", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementVertices = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]            = geometryInstance;
          start[1]            = localMeshElementStart;
          start[2]            = 0;
          count[0]            = 1;
          count[1]            = localNumberOfMeshElements;
          count[2]            = MeshElement::meshNeighborsSize;
          ncErrorCode         = nc_get_vara_int(fileID, varID, start, count, (int*)meshElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshElementVertices in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshVertexX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshVertexX = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]    = geometryInstance;
          start[1]    = localMeshElementStart;
          start[2]    = 0;
          count[0]    = 1;
          count[1]    = localNumberOfMeshElements;
          count[2]    = MeshElement::meshNeighborsSize;
          ncErrorCode = nc_get_vara_double(fileID, varID, start, count, (double*)meshVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshVertexX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshVertexY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshVertexY = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]    = geometryInstance;
          start[1]    = localMeshElementStart;
          start[2]    = 0;
          count[0]    = 1;
          count[1]    = localNumberOfMeshElements;
          count[2]    = MeshElement::meshNeighborsSize;
          ncErrorCode = nc_get_vara_double(fileID, varID, start, count, (double*)meshVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshVertexY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshVertexZSurface", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshVertexZSurface = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]           = geometryInstance;
          start[1]           = localMeshElementStart;
          start[2]           = 0;
          count[0]           = 1;
          count[1]           = localNumberOfMeshElements;
          count[2]           = MeshElement::meshNeighborsSize;
          ncErrorCode        = nc_get_vara_double(fileID, varID, start, count, (double*)meshVertexZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshVertexZSurface in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshVertexZBedrock", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshVertexZBedrock = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]           = geometryInstance;
          start[1]           = localMeshElementStart;
          start[2]           = 0;
          count[0]           = 1;
          count[1]           = localNumberOfMeshElements;
          count[2]           = MeshElement::meshNeighborsSize;
          ncErrorCode        = nc_get_vara_double(fileID, varID, start, count, (double*)meshVertexZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshVertexZBedrock in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementX = new double[localNumberOfMeshElements];
          start[0]     = geometryInstance;
          start[1]     = localMeshElementStart;
          count[0]     = 1;
          count[1]     = localNumberOfMeshElements;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, meshElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshElementX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementY = new double[localNumberOfMeshElements];
          start[0]     = geometryInstance;
          start[1]     = localMeshElementStart;
          count[0]     = 1;
          count[1]     = localNumberOfMeshElements;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, meshElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshElementY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementZSurface", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementZSurface = new double[localNumberOfMeshElements];
          start[0]            = geometryInstance;
          start[1]            = localMeshElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfMeshElements;
          ncErrorCode         = nc_get_vara_double(fileID, varID, start, count, meshElementZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshElementZSurface in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementZBedrock", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementZBedrock = new double[localNumberOfMeshElements];
          start[0]            = geometryInstance;
          start[1]            = localMeshElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfMeshElements;
          ncErrorCode         = nc_get_vara_double(fileID, varID, start, count, meshElementZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshElementZBedrock in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementArea", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementArea = new double[localNumberOfMeshElements];
          start[0]        = geometryInstance;
          start[1]        = localMeshElementStart;
          count[0]        = 1;
          count[1]        = localNumberOfMeshElements;
          ncErrorCode     = nc_get_vara_double(fileID, varID, start, count, meshElementArea);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshElementArea in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementSlopeX = new double[localNumberOfMeshElements];
          start[0]          = geometryInstance;
          start[1]          = localMeshElementStart;
          count[0]          = 1;
          count[1]          = localNumberOfMeshElements;
          ncErrorCode       = nc_get_vara_double(fileID, varID, start, count, meshElementSlopeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshElementSlopeX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementSlopeY = new double[localNumberOfMeshElements];
          start[0]          = geometryInstance;
          start[1]          = localMeshElementStart;
          count[0]          = 1;
          count[1]          = localNumberOfMeshElements;
          ncErrorCode       = nc_get_vara_double(fileID, varID, start, count, meshElementSlopeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshElementSlopeY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighbors", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshMeshNeighbors = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]          = geometryInstance;
          start[1]          = localMeshElementStart;
          start[2]          = 0;
          count[0]          = 1;
          count[1]          = localNumberOfMeshElements;
          count[2]          = MeshElement::meshNeighborsSize;
          ncErrorCode       = nc_get_vara_int(fileID, varID, start, count, (int*)meshMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshMeshNeighbors in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsChannelEdge", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          // Assumes bool is one byte when casting to signed char.
          CkAssert(sizeof(bool) == sizeof(signed char));
          
          meshMeshNeighborsChannelEdge = new bool[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]                     = geometryInstance;
          start[1]                     = localMeshElementStart;
          start[2]                     = 0;
          count[0]                     = 1;
          count[1]                     = localNumberOfMeshElements;
          count[2]                     = MeshElement::meshNeighborsSize;
          ncErrorCode                  = nc_get_vara_schar(fileID, varID, start, count, (signed char*)meshMeshNeighborsChannelEdge);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshMeshNeighborsChannelEdge in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeLength", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshMeshNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]                    = geometryInstance;
          start[1]                    = localMeshElementStart;
          start[2]                    = 0;
          count[0]                    = 1;
          count[1]                    = localNumberOfMeshElements;
          count[2]                    = MeshElement::meshNeighborsSize;
          ncErrorCode                 = nc_get_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshMeshNeighborsEdgeLength in NetCDF geometry file %s."
                      "  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshMeshNeighborsEdgeNormalX = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]                     = geometryInstance;
          start[1]                     = localMeshElementStart;
          start[2]                     = 0;
          count[0]                     = 1;
          count[1]                     = localNumberOfMeshElements;
          count[2]                     = MeshElement::meshNeighborsSize;
          ncErrorCode                  = nc_get_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeNormalX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file %s."
                      "  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshMeshNeighborsEdgeNormalY = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]                     = geometryInstance;
          start[1]                     = localMeshElementStart;
          start[2]                     = 0;
          count[0]                     = 1;
          count[1]                     = localNumberOfMeshElements;
          count[2]                     = MeshElement::meshNeighborsSize;
          ncErrorCode                  = nc_get_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeNormalY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file %s."
                      "  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighbors", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshChannelNeighbors = new int[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
          start[0]             = geometryInstance;
          start[1]             = localMeshElementStart;
          start[2]             = 0;
          count[0]             = 1;
          count[1]             = localNumberOfMeshElements;
          count[2]             = MeshElement::channelNeighborsSize;
          ncErrorCode          = nc_get_vara_int(fileID, varID, start, count, (int*)meshChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshChannelNeighbors in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighborsEdgeLength", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshChannelNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
          start[0]                       = geometryInstance;
          start[1]                       = localMeshElementStart;
          start[2]                       = 0;
          count[0]                       = 1;
          count[1]                       = localNumberOfMeshElements;
          count[2]                       = MeshElement::channelNeighborsSize;
          ncErrorCode                    = nc_get_vara_double(fileID, varID, start, count, (double*)meshChannelNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshChannelNeighborsEdgeLength in NetCDF geometry file "
                      "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelNodeX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelNodeX = new double[localNumberOfChannelNodes];
          start[0]     = geometryInstance;
          start[1]     = localChannelNodeStart;
          count[0]     = 1;
          count[1]     = localNumberOfChannelNodes;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, channelNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelNodeX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelNodeY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelNodeY = new double[localNumberOfChannelNodes];
          start[0]     = geometryInstance;
          start[1]     = localChannelNodeStart;
          count[0]     = 1;
          count[1]     = localNumberOfChannelNodes;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, channelNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelNodeY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelNodeZBank", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelNodeZBank = new double[localNumberOfChannelNodes];
          start[0]         = geometryInstance;
          start[1]         = localChannelNodeStart;
          count[0]         = 1;
          count[1]         = localNumberOfChannelNodes;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, channelNodeZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelNodeZBank in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelNodeZBed", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelNodeZBed = new double[localNumberOfChannelNodes];
          start[0]        = geometryInstance;
          start[1]        = localChannelNodeStart;
          count[0]        = 1;
          count[1]        = localNumberOfChannelNodes;
          ncErrorCode     = nc_get_vara_double(fileID, varID, start, count, channelNodeZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelNodeZBed in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementVertices", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementVertices = new int[localNumberOfChannelElements][ChannelElement::channelVerticesSize + 2];
          start[0]               = geometryInstance;
          start[1]               = localChannelElementStart;
          start[2]               = 0;
          count[0]               = 1;
          count[1]               = localNumberOfChannelElements;
          count[2]               = ChannelElement::channelVerticesSize + 2;
          ncErrorCode            = nc_get_vara_int(fileID, varID, start, count, (int*)channelElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelElementVertices in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelVertexX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelVertexX = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          start[0]       = geometryInstance;
          start[1]       = localChannelElementStart;
          start[2]       = 0;
          count[0]       = 1;
          count[1]       = localNumberOfChannelElements;
          count[2]       = ChannelElement::channelVerticesSize;
          ncErrorCode    = nc_get_vara_double(fileID, varID, start, count, (double*)channelVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelVertexX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelVertexY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelVertexY = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          start[0]       = geometryInstance;
          start[1]       = localChannelElementStart;
          start[2]       = 0;
          count[0]       = 1;
          count[1]       = localNumberOfChannelElements;
          count[2]       = ChannelElement::channelVerticesSize;
          ncErrorCode    = nc_get_vara_double(fileID, varID, start, count, (double*)channelVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelVertexY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelVertexZBank", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelVertexZBank = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          start[0]           = geometryInstance;
          start[1]           = localChannelElementStart;
          start[2]           = 0;
          count[0]           = 1;
          count[1]           = localNumberOfChannelElements;
          count[2]           = ChannelElement::channelVerticesSize;
          ncErrorCode        = nc_get_vara_double(fileID, varID, start, count, (double*)channelVertexZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelVertexZBank in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelVertexZBed", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelVertexZBed = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          start[0]          = geometryInstance;
          start[1]          = localChannelElementStart;
          start[2]          = 0;
          count[0]          = 1;
          count[1]          = localNumberOfChannelElements;
          count[2]          = ChannelElement::channelVerticesSize;
          ncErrorCode       = nc_get_vara_double(fileID, varID, start, count, (double*)channelVertexZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelVertexZBed in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementX = new double[localNumberOfChannelElements];
          start[0]        = geometryInstance;
          start[1]        = localChannelElementStart;
          count[0]        = 1;
          count[1]        = localNumberOfChannelElements;
          ncErrorCode     = nc_get_vara_double(fileID, varID, start, count, channelElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelElementX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementY = new double[localNumberOfChannelElements];
          start[0]        = geometryInstance;
          start[1]        = localChannelElementStart;
          count[0]        = 1;
          count[1]        = localNumberOfChannelElements;
          ncErrorCode     = nc_get_vara_double(fileID, varID, start, count, channelElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelElementY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementZBank", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementZBank = new double[localNumberOfChannelElements];
          start[0]            = geometryInstance;
          start[1]            = localChannelElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfChannelElements;
          ncErrorCode         = nc_get_vara_double(fileID, varID, start, count, channelElementZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelElementZBank in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementZBed", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementZBed = new double[localNumberOfChannelElements];
          start[0]           = geometryInstance;
          start[1]           = localChannelElementStart;
          count[0]           = 1;
          count[1]           = localNumberOfChannelElements;
          ncErrorCode        = nc_get_vara_double(fileID, varID, start, count, channelElementZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelElementZBed in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementLength", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementLength = new double[localNumberOfChannelElements];
          start[0]             = geometryInstance;
          start[1]             = localChannelElementStart;
          count[0]             = 1;
          count[1]             = localNumberOfChannelElements;
          ncErrorCode          = nc_get_vara_double(fileID, varID, start, count, channelElementLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelElementLength in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelChannelNeighbors", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelChannelNeighbors = new int[localNumberOfChannelElements][ChannelElement::channelNeighborsSize];
          start[0]                = geometryInstance;
          start[1]                = localChannelElementStart;
          start[2]                = 0;
          count[0]                = 1;
          count[1]                = localNumberOfChannelElements;
          count[2]                = ChannelElement::channelNeighborsSize;
          ncErrorCode             = nc_get_vara_int(fileID, varID, start, count, (int*)channelChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelChannelNeighbors in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighbors", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelMeshNeighbors = new int[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
          start[0]             = geometryInstance;
          start[1]             = localChannelElementStart;
          start[2]             = 0;
          count[0]             = 1;
          count[1]             = localNumberOfChannelElements;
          count[2]             = ChannelElement::meshNeighborsSize;
          ncErrorCode          = nc_get_vara_int(fileID, varID, start, count, (int*)channelMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelMeshNeighbors in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighborsEdgeLength", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelMeshNeighborsEdgeLength = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
          start[0]                       = geometryInstance;
          start[1]                       = localChannelElementStart;
          start[2]                       = 0;
          count[0]                       = 1;
          count[1]                       = localNumberOfChannelElements;
          count[2]                       = ChannelElement::meshNeighborsSize;
          ncErrorCode                    = nc_get_vara_double(fileID, varID, start, count, (double*)channelMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to close NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: incorrect return value of snprintf when generating parameter file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to open NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Read variables.
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshCatchment", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshCatchment = new int[localNumberOfMeshElements];
          start[0]      = parameterInstance;
          start[1]      = localMeshElementStart;
          count[0]      = 1;
          count[1]      = localNumberOfMeshElements;
          ncErrorCode   = nc_get_vara_int(fileID, varID, start, count, meshCatchment);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshCatchment in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshConductivity", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshConductivity = new double[localNumberOfMeshElements];
          start[0]         = parameterInstance;
          start[1]         = localMeshElementStart;
          count[0]         = 1;
          count[1]         = localNumberOfMeshElements;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, meshConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshConductivity in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshPorosity", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshPorosity = new double[localNumberOfMeshElements];
          start[0]     = parameterInstance;
          start[1]     = localMeshElementStart;
          count[0]     = 1;
          count[1]     = localNumberOfMeshElements;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, meshPorosity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshPorosity in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshManningsN", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshManningsN = new double[localNumberOfMeshElements];
          start[0]      = parameterInstance;
          start[1]      = localMeshElementStart;
          count[0]      = 1;
          count[1]      = localNumberOfMeshElements;
          ncErrorCode   = nc_get_vara_double(fileID, varID, start, count, meshManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshManningsN in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelChannelType", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          // Assumes ChannelTypeEnum is four bytes when casting to int.
          CkAssert(sizeof(ChannelTypeEnum) == sizeof(int));
          
          channelChannelType = new ChannelTypeEnum[localNumberOfChannelElements];
          start[0]           = parameterInstance;
          start[1]           = localChannelElementStart;
          count[0]           = 1;
          count[1]           = localNumberOfChannelElements;
          ncErrorCode        = nc_get_vara_int(fileID, varID, start, count, (int*)channelChannelType);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelChannelType in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelPermanentCode", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelPermanentCode = new int[localNumberOfChannelElements];
          start[0]             = parameterInstance;
          start[1]             = localChannelElementStart;
          count[0]             = 1;
          count[1]             = localNumberOfChannelElements;
          ncErrorCode          = nc_get_vara_int(fileID, varID, start, count, channelPermanentCode);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelPermanentCode in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelBaseWidth", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelBaseWidth = new double[localNumberOfChannelElements];
          start[0]         = parameterInstance;
          start[1]         = localChannelElementStart;
          count[0]         = 1;
          count[1]         = localNumberOfChannelElements;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, channelBaseWidth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelBaseWidth in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelSideSlope", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelSideSlope = new double[localNumberOfChannelElements];
          start[0]         = parameterInstance;
          start[1]         = localChannelElementStart;
          count[0]         = 1;
          count[1]         = localNumberOfChannelElements;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, channelSideSlope);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelSideSlope in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelBedConductivity", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelBedConductivity = new double[localNumberOfChannelElements];
          start[0]               = parameterInstance;
          start[1]               = localChannelElementStart;
          count[0]               = 1;
          count[1]               = localNumberOfChannelElements;
          ncErrorCode            = nc_get_vara_double(fileID, varID, start, count, channelBedConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelBedConductivity in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelBedThickness", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelBedThickness = new double[localNumberOfChannelElements];
          start[0]            = parameterInstance;
          start[1]            = localChannelElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfChannelElements;
          ncErrorCode         = nc_get_vara_double(fileID, varID, start, count, channelBedThickness);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelBedThickness in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelManningsN", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelManningsN = new double[localNumberOfChannelElements];
          start[0]         = parameterInstance;
          start[1]         = localChannelElementStart;
          count[0]         = 1;
          count[1]         = localNumberOfChannelElements;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, channelManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelManningsN in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to close NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read variables.
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "meshSurfacewaterDepth", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshSurfacewaterDepth = new double[localNumberOfMeshElements];
          start[0]              = stateInstance;
          start[1]              = localMeshElementStart;
          count[0]              = 1;
          count[1]              = localNumberOfMeshElements;
          ncErrorCode           = nc_get_vara_double(stateFileID, varID, start, count, meshSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshSurfacewaterDepth in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "meshSurfacewaterError", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshSurfacewaterError = new double[localNumberOfMeshElements];
          start[0]              = stateInstance;
          start[1]              = localMeshElementStart;
          count[0]              = 1;
          count[1]              = localNumberOfMeshElements;
          ncErrorCode           = nc_get_vara_double(stateFileID, varID, start, count, meshSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshSurfacewaterError in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "meshGroundwaterHead", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshGroundwaterHead = new double[localNumberOfMeshElements];
          start[0]            = stateInstance;
          start[1]            = localMeshElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfMeshElements;
          ncErrorCode         = nc_get_vara_double(stateFileID, varID, start, count, meshGroundwaterHead);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshGroundwaterHead in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "meshGroundwaterError", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshGroundwaterError = new double[localNumberOfMeshElements];
          start[0]             = stateInstance;
          start[1]             = localMeshElementStart;
          count[0]             = 1;
          count[1]             = localNumberOfMeshElements;
          ncErrorCode          = nc_get_vara_double(stateFileID, varID, start, count, meshGroundwaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable meshGroundwaterError in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

   if (!error)
    {
       meshPrecipitationCumulative             = new double[localNumberOfMeshElements]; 
       meshEvaporationCumulative               = new double[localNumberOfMeshElements];
       meshCanopyLiquid                        = new double[localNumberOfMeshElements];
       meshCanopyIce                           = new double[localNumberOfMeshElements];
       meshSnowWaterEquivalent                 = new double[localNumberOfMeshElements];
       meshElementNeighborsSurfacewaterCumulativeFlow = new doublearraymmn[localNumberOfMeshElements];
       meshElementNeighborsGroundwaterCumulativeFlow  = new doublearraymmn[localNumberOfMeshElements];
    } 
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "channelSurfacewaterDepth", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelSurfacewaterDepth = new double[localNumberOfChannelElements];
          start[0]                 = stateInstance;
          start[1]                 = localChannelElementStart;
          count[0]                 = 1;
          count[1]                 = localNumberOfChannelElements;
          ncErrorCode              = nc_get_vara_double(stateFileID, varID, start, count, channelSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelSurfacewaterDepth in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "channelSurfacewaterError", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelSurfacewaterError = new double[localNumberOfChannelElements];
          start[0]                 = stateInstance;
          start[1]                 = localChannelElementStart;
          count[0]                 = 1;
          count[1]                 = localNumberOfChannelElements;
          ncErrorCode              = nc_get_vara_double(stateFileID, varID, start, count, channelSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to read variable channelSurfacewaterError in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
       channelEvaporationCumulative  = new double[localNumberOfMeshElements];
       channelElementNeighborsSurfacewaterCumulativeFlow = new doublearrayccn[localNumberOfMeshElements];
    }
        
  // Close file.
  if (stateFileOpen)
    {
      ncErrorCode   = nc_close(stateFileID);
      stateFileOpen = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to close NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      meshElementUpdated    = new bool[localNumberOfMeshElements];
      channelElementUpdated = new bool[localNumberOfChannelElements];
    }
  
  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
    }
  
  // Have to call evapoTranspirationInit once on each Pe. This is a convenient place to do that.
  if (!error)
    {
      error = evapoTranspirationInit(directory);
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

bool FileManager::allVerticesUpdated()
{
  int  ii, jj;         // Loop counters.
  bool updated = true; // Flag to record whether we have found an unupdated vertex.
  
  for (ii = 0; updated && ii < localNumberOfMeshElements; ii++)
  {
      for (jj = 0; updated && jj < MeshElement::meshNeighborsSize; jj++)
        {
          updated = meshVertexUpdated[ii][jj];
        }
  }
  
  return updated;
}

void FileManager::handleMeshVertexDataMessage(int element, int vertex, double x, double y, double zSurface, double zBedrock)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(localMeshElementStart <= element && element < localMeshElementStart + localNumberOfMeshElements))
    {
      CkError("ERROR in FileManager::meshVertexDataMessage: element data not owned by this local branch.\n");
      CkExit();
    }

  if (!(0 <= vertex && vertex < MeshElement::meshNeighborsSize))
    {
      CkError("ERROR in FileManager::meshVertexDataMessage: vertex must be greater than or equal to zero and less than "
          "MeshElement::meshNeighborsSize.\n");
      CkExit();
    }

  if (!(zSurface >= zBedrock))
    {
      CkError("ERROR in FileManager::meshVertexDataMessage: zSurface must be greater than or equal to zBedrock.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (NULL != meshNodeX)
    {
      meshVertexX[element - localMeshElementStart][vertex] = x;
    }

  if (NULL != meshNodeY)
    {
      meshVertexY[element - localMeshElementStart][vertex] = y;
    }

  if (NULL != meshNodeZSurface)
    {
      meshVertexZSurface[element - localMeshElementStart][vertex] = zSurface;
    }

  if (NULL != meshNodeZBedrock)
    {
      meshVertexZBedrock[element - localMeshElementStart][vertex] = zBedrock;
    }

  meshVertexUpdated[element - localMeshElementStart][vertex] = true;
}

void FileManager::finishCalculateDerivedValues()
{
  int    ii, jj; // Loop counters.
  double value;  // For calculating derived values.

  if (NULL == meshElementX && NULL != meshVertexX)
    {
      meshElementX = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexX[ii][jj];
            }

          meshElementX[ii] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementY && NULL != meshVertexY)
    {
      meshElementY = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexY[ii][jj];
            }

          meshElementY[ii] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementZSurface && NULL != meshVertexZSurface)
    {
      meshElementZSurface = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexZSurface[ii][jj];
            }

          meshElementZSurface[ii] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementZBedrock && NULL != meshVertexZBedrock)
    {
      meshElementZBedrock = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexZBedrock[ii][jj];
            }

          meshElementZBedrock[ii] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementArea && NULL != meshVertexX && NULL != meshVertexY)
    {
      meshElementArea = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexX[ii][jj] * (meshVertexY[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                              meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize]);
            }

          meshElementArea[ii] = value * 0.5;
        }
    }

  if (NULL == meshElementSlopeX && NULL != meshVertexX && NULL != meshVertexY && NULL != meshVertexZSurface)
    {
      meshElementSlopeX = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize - 1; jj++)
            {
              value += (meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize] - meshVertexY[ii][jj]) *
                       (meshVertexZSurface[ii][(jj + 1) % MeshElement::meshNeighborsSize] - meshVertexZSurface[ii][0]);
            }

          meshElementSlopeX[ii] = value / (2.0 * meshElementArea[ii]);
        }
    }

  if (NULL == meshElementSlopeY && NULL != meshVertexX && NULL != meshVertexY && NULL != meshVertexZSurface)
    {
      meshElementSlopeY = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize - 1; jj++)
            {
              value += (meshVertexX[ii][jj] - meshVertexX[ii][(jj + 2) % MeshElement::meshNeighborsSize]) *
                       (meshVertexZSurface[ii][(jj + 1) % MeshElement::meshNeighborsSize] - meshVertexZSurface[ii][0]);
            }

          meshElementSlopeY[ii] = value / (2.0 * meshElementArea[ii]);
        }
    }

  if (NULL == meshMeshNeighborsEdgeLength && NULL != meshVertexX && NULL != meshVertexY)
    {
      meshMeshNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeLength[ii][jj] = sqrt((meshVertexX[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                          meshVertexX[ii][(jj + 2) % MeshElement::meshNeighborsSize]) *
                                                         (meshVertexX[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                          meshVertexX[ii][(jj + 2) % MeshElement::meshNeighborsSize]) +
                                                         (meshVertexY[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                          meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize]) *
                                                         (meshVertexY[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                          meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize]));
            }
        }
    }

  if (NULL == meshMeshNeighborsEdgeNormalX && NULL != meshVertexY && NULL != meshMeshNeighborsEdgeLength)
    {
      meshMeshNeighborsEdgeNormalX = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeNormalX[ii][jj] = (meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize] -
                                                      meshVertexY[ii][(jj + 1) % MeshElement::meshNeighborsSize]) / meshMeshNeighborsEdgeLength[ii][jj];
            }
        }
    }

  if (NULL == meshMeshNeighborsEdgeNormalY && NULL != meshVertexX && NULL != meshMeshNeighborsEdgeLength)
    {
      meshMeshNeighborsEdgeNormalY = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeNormalY[ii][jj] = (meshVertexX[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                      meshVertexX[ii][(jj + 2) % MeshElement::meshNeighborsSize]) / meshMeshNeighborsEdgeLength[ii][jj];
            }
        }
    }

  // If meshGroundwaterHead is uninitialized it defaults to saturated (water table at the surface).
  if (NULL == meshGroundwaterHead && NULL != meshElementZSurface)
    {
      meshGroundwaterHead = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshGroundwaterHead[ii] = meshElementZSurface[ii];
        }
    }
  
  contribute();
}

bool FileManager::allElementsUpdated()
{
  int  ii;             // Loop counter.
  bool updated = true; // Flag to record whether we have found an unupdated element.
  
  for (ii = 0; updated && ii < localNumberOfMeshElements; ii++)
  {
    updated = meshElementUpdated[ii];
  }
  
  for (ii = 0; updated && ii < localNumberOfChannelElements; ii++)
  {
    updated = channelElementUpdated[ii];
  }
  
  return updated;
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "file_manager.def.h"
#pragma GCC diagnostic warning "-Wsign-compare"
#pragma GCC diagnostic warning "-Wunused-variable"
