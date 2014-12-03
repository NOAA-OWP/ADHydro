#include "file_manager.h"
#include "all.h"
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
  globalNumberOfMeshNodes                           = 0;
  localMeshNodeStart                                = 0;
  localNumberOfMeshNodes                            = 0;
  globalNumberOfMeshElements                        = 0;
  localMeshElementStart                             = 0;
  localNumberOfMeshElements                         = 0;
  globalNumberOfChannelNodes                        = 0;
  localChannelNodeStart                             = 0;
  localNumberOfChannelNodes                         = 0;
  globalNumberOfChannelElements                     = 0;
  localChannelElementStart                          = 0;
  localNumberOfChannelElements                      = 0;
  meshNodeX                                         = NULL;
  meshNodeY                                         = NULL;
  meshNodeZSurface                                  = NULL;
  meshElementVertices                               = NULL;
  meshVertexX                                       = NULL;
  meshVertexY                                       = NULL;
  meshVertexZSurface                                = NULL;
  meshElementX                                      = NULL;
  meshElementY                                      = NULL;
  meshElementZSurface                               = NULL;
  meshElementSoilDepth                              = NULL;
  meshElementZBedrock                               = NULL;
  meshElementArea                                   = NULL;
  meshElementSlopeX                                 = NULL;
  meshElementSlopeY                                 = NULL;
  meshCatchment                                     = NULL;
  meshVegetationType                                = NULL;
  meshSoilType                                      = NULL;
  meshConductivity                                  = NULL;
  meshPorosity                                      = NULL;
  meshManningsN                                     = NULL;
  meshSurfacewaterDepth                             = NULL;
  meshSurfacewaterError                             = NULL;
  meshGroundwaterHead                               = NULL;
  meshGroundwaterError                              = NULL;
  meshPrecipitation                                 = NULL;
  meshPrecipitationCumulative                       = NULL;
  meshEvaporation                                   = NULL;
  meshEvaporationCumulative                         = NULL;
  meshSurfacewaterInfiltration                      = NULL;
  meshGroundwaterRecharge                           = NULL;
  meshFIceOld                                       = NULL;
  meshAlbOld                                        = NULL;
  meshSnEqvO                                        = NULL;
  meshStc                                           = NULL;
  meshTah                                           = NULL;
  meshEah                                           = NULL;
  meshFWet                                          = NULL;
  meshCanLiq                                        = NULL;
  meshCanIce                                        = NULL;
  meshTv                                            = NULL;
  meshTg                                            = NULL;
  meshISnow                                         = NULL;
  meshZSnso                                         = NULL;
  meshSnowH                                         = NULL;
  meshSnEqv                                         = NULL;
  meshSnIce                                         = NULL;
  meshSnLiq                                         = NULL;
  meshLfMass                                        = NULL;
  meshRtMass                                        = NULL;
  meshStMass                                        = NULL;
  meshWood                                          = NULL;
  meshStblCp                                        = NULL;
  meshFastCp                                        = NULL;
  meshLai                                           = NULL;
  meshSai                                           = NULL;
  meshCm                                            = NULL;
  meshCh                                            = NULL;
  meshTauss                                         = NULL;
  meshDeepRech                                      = NULL;
  meshRech                                          = NULL;
  meshMeshNeighbors                                 = NULL;
  meshMeshNeighborsChannelEdge                      = NULL;
  meshMeshNeighborsEdgeLength                       = NULL;
  meshMeshNeighborsEdgeNormalX                      = NULL;
  meshMeshNeighborsEdgeNormalY                      = NULL;
  meshMeshNeighborsSurfacewaterFlowRate             = NULL;
  meshMeshNeighborsSurfacewaterCumulativeFlow       = NULL;
  meshMeshNeighborsGroundwaterFlowRate              = NULL;
  meshMeshNeighborsGroundwaterCumulativeFlow        = NULL;
  meshChannelNeighbors                              = NULL;
  meshChannelNeighborsEdgeLength                    = NULL;
  meshChannelNeighborsSurfacewaterFlowRate          = NULL;
  meshChannelNeighborsSurfacewaterCumulativeFlow    = NULL;
  meshChannelNeighborsGroundwaterFlowRate           = NULL;
  meshChannelNeighborsGroundwaterCumulativeFlow     = NULL;
  channelNodeX                                      = NULL;
  channelNodeY                                      = NULL;
  channelNodeZBank                                  = NULL;
  channelElementVertices                            = NULL;
  channelVertexX                                    = NULL;
  channelVertexY                                    = NULL;
  channelVertexZBank                                = NULL;
  channelElementX                                   = NULL;
  channelElementY                                   = NULL;
  channelElementZBank                               = NULL;
  channelElementBankFullDepth                       = NULL;
  channelElementZBed                                = NULL;
  channelElementLength                              = NULL;
  channelChannelType                                = NULL;
  channelPermanentCode                              = NULL;
  channelBaseWidth                                  = NULL;
  channelSideSlope                                  = NULL;
  channelBedConductivity                            = NULL;
  channelBedThickness                               = NULL;
  channelManningsN                                  = NULL;
  channelSurfacewaterDepth                          = NULL;
  channelSurfacewaterError                          = NULL;
  channelPrecipitation                              = NULL;
  channelPrecipitationCumulative                    = NULL;
  channelEvaporation                                = NULL;
  channelEvaporationCumulative                      = NULL;
  channelFIceOld                                    = NULL;
  channelAlbOld                                     = NULL;
  channelSnEqvO                                     = NULL;
  channelStc                                        = NULL;
  channelTah                                        = NULL;
  channelEah                                        = NULL;
  channelFWet                                       = NULL;
  channelCanLiq                                     = NULL;
  channelCanIce                                     = NULL;
  channelTv                                         = NULL;
  channelTg                                         = NULL;
  channelISnow                                      = NULL;
  channelZSnso                                      = NULL;
  channelSnowH                                      = NULL;
  channelSnEqv                                      = NULL;
  channelSnIce                                      = NULL;
  channelSnLiq                                      = NULL;
  channelLfMass                                     = NULL;
  channelRtMass                                     = NULL;
  channelStMass                                     = NULL;
  channelWood                                       = NULL;
  channelStblCp                                     = NULL;
  channelFastCp                                     = NULL;
  channelLai                                        = NULL;
  channelSai                                        = NULL;
  channelCm                                         = NULL;
  channelCh                                         = NULL;
  channelTauss                                      = NULL;
  channelDeepRech                                   = NULL;
  channelRech                                       = NULL;
  channelChannelNeighbors                           = NULL;
  channelChannelNeighborsSurfacewaterFlowRate       = NULL;
  channelChannelNeighborsSurfacewaterCumulativeFlow = NULL;
  channelMeshNeighbors                              = NULL;
  channelMeshNeighborsEdgeLength                    = NULL;
  channelMeshNeighborsSurfacewaterFlowRate          = NULL;
  channelMeshNeighborsSurfacewaterCumulativeFlow    = NULL;
  channelMeshNeighborsGroundwaterFlowRate           = NULL;
  channelMeshNeighborsGroundwaterCumulativeFlow     = NULL;
  referenceDate                                     = 0.0;
  currentTime                                       = 0.0;
  dt                                                = 1.0;
  iteration                                         = 1;
  forcingDataInitialized                            = false;
  forcingDataDate                                   = 0.0;
  nextForcingDataDate                               = 0.0;
  meshVertexUpdated                                 = NULL;
  channelVertexUpdated                              = NULL;
  meshElementUpdated                                = NULL;
  channelElementUpdated                             = NULL;
  
  // Initialization will be done in runForever.
  thisProxy[CkMyPe()].runForever();
}

FileManager::~FileManager()
{
  deleteArrayIfNonNull(&meshNodeX);
  deleteArrayIfNonNull(&meshNodeY);
  deleteArrayIfNonNull(&meshNodeZSurface);
  deleteArrayIfNonNull(&meshElementVertices);
  deleteArrayIfNonNull(&meshVertexX);
  deleteArrayIfNonNull(&meshVertexY);
  deleteArrayIfNonNull(&meshVertexZSurface);
  deleteArrayIfNonNull(&meshElementX);
  deleteArrayIfNonNull(&meshElementY);
  deleteArrayIfNonNull(&meshElementZSurface);
  deleteArrayIfNonNull(&meshElementSoilDepth);
  deleteArrayIfNonNull(&meshElementZBedrock);
  deleteArrayIfNonNull(&meshElementArea);
  deleteArrayIfNonNull(&meshElementSlopeX);
  deleteArrayIfNonNull(&meshElementSlopeY);
  deleteArrayIfNonNull(&meshCatchment);
  deleteArrayIfNonNull(&meshVegetationType);
  deleteArrayIfNonNull(&meshSoilType);
  deleteArrayIfNonNull(&meshConductivity);
  deleteArrayIfNonNull(&meshPorosity);
  deleteArrayIfNonNull(&meshManningsN);
  deleteArrayIfNonNull(&meshSurfacewaterDepth);
  deleteArrayIfNonNull(&meshSurfacewaterError);
  deleteArrayIfNonNull(&meshGroundwaterHead);
  deleteArrayIfNonNull(&meshGroundwaterError);
  deleteArrayIfNonNull(&meshPrecipitation);
  deleteArrayIfNonNull(&meshPrecipitationCumulative);
  deleteArrayIfNonNull(&meshEvaporation);
  deleteArrayIfNonNull(&meshEvaporationCumulative);
  deleteArrayIfNonNull(&meshSurfacewaterInfiltration);
  deleteArrayIfNonNull(&meshGroundwaterRecharge);
  deleteArrayIfNonNull(&meshFIceOld);
  deleteArrayIfNonNull(&meshAlbOld);
  deleteArrayIfNonNull(&meshSnEqvO);
  deleteArrayIfNonNull(&meshStc);
  deleteArrayIfNonNull(&meshTah);
  deleteArrayIfNonNull(&meshEah);
  deleteArrayIfNonNull(&meshFWet);
  deleteArrayIfNonNull(&meshCanLiq);
  deleteArrayIfNonNull(&meshCanIce);
  deleteArrayIfNonNull(&meshTv);
  deleteArrayIfNonNull(&meshTg);
  deleteArrayIfNonNull(&meshISnow);
  deleteArrayIfNonNull(&meshZSnso);
  deleteArrayIfNonNull(&meshSnowH);
  deleteArrayIfNonNull(&meshSnEqv);
  deleteArrayIfNonNull(&meshSnIce);
  deleteArrayIfNonNull(&meshSnLiq);
  deleteArrayIfNonNull(&meshLfMass);
  deleteArrayIfNonNull(&meshRtMass);
  deleteArrayIfNonNull(&meshStMass);
  deleteArrayIfNonNull(&meshWood);
  deleteArrayIfNonNull(&meshStblCp);
  deleteArrayIfNonNull(&meshFastCp);
  deleteArrayIfNonNull(&meshLai);
  deleteArrayIfNonNull(&meshSai);
  deleteArrayIfNonNull(&meshCm);
  deleteArrayIfNonNull(&meshCh);
  deleteArrayIfNonNull(&meshTauss);
  deleteArrayIfNonNull(&meshDeepRech);
  deleteArrayIfNonNull(&meshRech);
  deleteArrayIfNonNull(&meshMeshNeighbors);
  deleteArrayIfNonNull(&meshMeshNeighborsChannelEdge);
  deleteArrayIfNonNull(&meshMeshNeighborsEdgeLength);
  deleteArrayIfNonNull(&meshMeshNeighborsEdgeNormalX);
  deleteArrayIfNonNull(&meshMeshNeighborsEdgeNormalY);
  deleteArrayIfNonNull(&meshMeshNeighborsSurfacewaterFlowRate);
  deleteArrayIfNonNull(&meshMeshNeighborsSurfacewaterCumulativeFlow);
  deleteArrayIfNonNull(&meshMeshNeighborsGroundwaterFlowRate);
  deleteArrayIfNonNull(&meshMeshNeighborsGroundwaterCumulativeFlow);
  deleteArrayIfNonNull(&meshChannelNeighbors);
  deleteArrayIfNonNull(&meshChannelNeighborsEdgeLength);
  deleteArrayIfNonNull(&meshChannelNeighborsSurfacewaterFlowRate);
  deleteArrayIfNonNull(&meshChannelNeighborsSurfacewaterCumulativeFlow);
  deleteArrayIfNonNull(&meshChannelNeighborsGroundwaterFlowRate);
  deleteArrayIfNonNull(&meshChannelNeighborsGroundwaterCumulativeFlow);
  deleteArrayIfNonNull(&channelNodeX);
  deleteArrayIfNonNull(&channelNodeY);
  deleteArrayIfNonNull(&channelNodeZBank);
  deleteArrayIfNonNull(&channelElementVertices);
  deleteArrayIfNonNull(&channelVertexX);
  deleteArrayIfNonNull(&channelVertexY);
  deleteArrayIfNonNull(&channelVertexZBank);
  deleteArrayIfNonNull(&channelElementX);
  deleteArrayIfNonNull(&channelElementY);
  deleteArrayIfNonNull(&channelElementZBank);
  deleteArrayIfNonNull(&channelElementBankFullDepth);
  deleteArrayIfNonNull(&channelElementZBed);
  deleteArrayIfNonNull(&channelElementLength);
  deleteArrayIfNonNull(&channelChannelType);
  deleteArrayIfNonNull(&channelPermanentCode);
  deleteArrayIfNonNull(&channelBaseWidth);
  deleteArrayIfNonNull(&channelSideSlope);
  deleteArrayIfNonNull(&channelBedConductivity);
  deleteArrayIfNonNull(&channelBedThickness);
  deleteArrayIfNonNull(&channelManningsN);
  deleteArrayIfNonNull(&channelSurfacewaterDepth);
  deleteArrayIfNonNull(&channelSurfacewaterError);
  deleteArrayIfNonNull(&channelPrecipitation);
  deleteArrayIfNonNull(&channelPrecipitationCumulative);
  deleteArrayIfNonNull(&channelEvaporation);
  deleteArrayIfNonNull(&channelEvaporationCumulative);
  deleteArrayIfNonNull(&channelFIceOld);
  deleteArrayIfNonNull(&channelAlbOld);
  deleteArrayIfNonNull(&channelSnEqvO);
  deleteArrayIfNonNull(&channelStc);
  deleteArrayIfNonNull(&channelTah);
  deleteArrayIfNonNull(&channelEah);
  deleteArrayIfNonNull(&channelFWet);
  deleteArrayIfNonNull(&channelCanLiq);
  deleteArrayIfNonNull(&channelCanIce);
  deleteArrayIfNonNull(&channelTv);
  deleteArrayIfNonNull(&channelTg);
  deleteArrayIfNonNull(&channelISnow);
  deleteArrayIfNonNull(&channelZSnso);
  deleteArrayIfNonNull(&channelSnowH);
  deleteArrayIfNonNull(&channelSnEqv);
  deleteArrayIfNonNull(&channelSnIce);
  deleteArrayIfNonNull(&channelSnLiq);
  deleteArrayIfNonNull(&channelLfMass);
  deleteArrayIfNonNull(&channelRtMass);
  deleteArrayIfNonNull(&channelStMass);
  deleteArrayIfNonNull(&channelWood);
  deleteArrayIfNonNull(&channelStblCp);
  deleteArrayIfNonNull(&channelFastCp);
  deleteArrayIfNonNull(&channelLai);
  deleteArrayIfNonNull(&channelSai);
  deleteArrayIfNonNull(&channelCm);
  deleteArrayIfNonNull(&channelCh);
  deleteArrayIfNonNull(&channelTauss);
  deleteArrayIfNonNull(&channelDeepRech);
  deleteArrayIfNonNull(&channelRech);
  deleteArrayIfNonNull(&channelChannelNeighbors);
  deleteArrayIfNonNull(&channelChannelNeighborsSurfacewaterFlowRate);
  deleteArrayIfNonNull(&channelChannelNeighborsSurfacewaterCumulativeFlow);
  deleteArrayIfNonNull(&channelMeshNeighbors);
  deleteArrayIfNonNull(&channelMeshNeighborsEdgeLength);
  deleteArrayIfNonNull(&channelMeshNeighborsSurfacewaterFlowRate);
  deleteArrayIfNonNull(&channelMeshNeighborsSurfacewaterCumulativeFlow);
  deleteArrayIfNonNull(&channelMeshNeighborsGroundwaterFlowRate);
  deleteArrayIfNonNull(&channelMeshNeighborsGroundwaterCumulativeFlow);
  deleteArrayIfNonNull(&meshVertexUpdated);
  deleteArrayIfNonNull(&channelVertexUpdated);
  deleteArrayIfNonNull(&meshElementUpdated);
  deleteArrayIfNonNull(&channelElementUpdated);
}

void FileManager::getMeshVertexDataMessage(int requester, int element, int vertex, int node)
{
  double x        = 0.0;
  double y        = 0.0;
  double zSurface = 0.0;

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

  if (CkMyPe() == requester)
    {
      // The requester is me, no need to send a message.
      handleMeshVertexDataMessage(element, vertex, x, y, zSurface);
    }
  else
    {
      thisProxy[requester].meshVertexDataMessage(element, vertex, x, y, zSurface);
    }
}

void FileManager::getChannelVertexDataMessage(int requester, int element, int vertex, int node)
{
  double x     = 0.0;
  double y     = 0.0;
  double zBank = 0.0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= requester && requester < CkNumPes()))
    {
      CkError("ERROR in FileManager::getChannelVertexDataMessage: requester must be greater than or equal to zero and less than CkNumPes().\n");
      CkExit();
    }

  if (!(0 <= element && element < globalNumberOfChannelElements))
    {
      CkError("ERROR in FileManager::getChannelVertexDataMessage: element must be greater than or equal to zero and less than "
              "globalNumberOfChannelElements.\n");
      CkExit();
    }

  if (!(0 <= vertex && vertex < ChannelElement::channelVerticesSize))
    {
      CkError("ERROR in FileManager::getChannelVertexDataMessage: vertex must be greater than or equal to zero and less than "
              "ChannelElement::channelVerticesSize.\n");
      CkExit();
    }

  if (!(localChannelNodeStart <= node && node < localChannelNodeStart + localNumberOfChannelNodes))
    {
      CkError("ERROR in FileManager::getChannelVertexDataMessage: node data not owned by this local branch.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (NULL != channelNodeX)
    {
      x = channelNodeX[node - localChannelNodeStart];
    }

  if (NULL != channelNodeY)
    {
      y = channelNodeY[node - localChannelNodeStart];
    }

  if (NULL != channelNodeZBank)
    {
      zBank = channelNodeZBank[node - localChannelNodeStart];
    }

  if (CkMyPe() == requester)
    {
      // The requester is me, no need to send a message.
      handleChannelVertexDataMessage(element, vertex, x, y, zBank);
    }
  else
    {
      thisProxy[requester].channelVertexDataMessage(element, vertex, x, y, zBank);
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

int FileManager::openNetCDFFile(const char* directory, const char* filename, bool create, bool write, int* fileID)
{
  bool   error      = false; // Error flag.
  char*  nameString = NULL;  // Temporary string for file name.
  size_t nameStringSize;     // Size of buffer allocated for nameString.
  size_t numPrinted;         // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;        // Return value of NetCDF functions.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != directory && NULL != filename && !(create && write) && NULL != fileID);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // FIXME make directory if creating file?
  
  // Allocate space for file name string.
  nameStringSize = strlen(directory) + strlen("/") + strlen(filename) + 1; // +1 for null terminating character.
  nameString     = new char[nameStringSize];

  // Create file name.
  numPrinted = snprintf(nameString, nameStringSize, "%s/%s", directory, filename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(strlen(directory) + strlen("/") + strlen(filename) == numPrinted && numPrinted < nameStringSize))
    {
      CkError("ERROR in FileManager::openNetCDFFile: incorrect return value of snprintf when generating file name %s.  %d should be equal to %d and less than "
              "%d.\n", nameString, numPrinted, strlen(directory) + strlen("/") + strlen(filename), nameStringSize);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  
  // Open file.
  if (!error)
    {
      if (create)
        {
          ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, fileID);
        }
      else if (write)
        {
          ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, fileID);
        }
      else
        {
          ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, fileID);
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::openNetCDFFile: unable to %s NetCDF file %s.  NetCDF error message: %s.\n", create ? "create" : "open", nameString,
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Delete nameString.
  deleteArrayIfNonNull(&nameString);
  
  return error;
}

int FileManager::createNetCDFDimension(int fileID, const char* dimensionName, size_t dimensionSize, int* dimensionID)
{
  bool error = false; // Error flag.
  int  ncErrorCode;   // Return value of NetCDF functions.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != dimensionName && NULL != dimensionID);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  ncErrorCode = nc_def_dim(fileID, dimensionName, dimensionSize, dimensionID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NC_NOERR == ncErrorCode))
    {
      CkError("ERROR in FileManager::createNetCDFdimension: unable to create dimension %s in NetCDF file.  NetCDF error message: %s.\n", dimensionName,
              nc_strerror(ncErrorCode));
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  
  return error;
}

int FileManager::readNetCDFDimensionSize(int fileID, const char* dimensionName, size_t* dimensionSize)
{
  bool error = false; // Error flag.
  int  ncErrorCode;   // Return value of NetCDF functions.
  int  dimensionID;   // ID of dimension in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != dimensionName && NULL != dimensionSize);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Get the dimension ID.
  ncErrorCode = nc_inq_dimid(fileID, dimensionName, &dimensionID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NC_NOERR == ncErrorCode))
    {
      CkError("ERROR in FileManager::readNetCDFDimensionSize: unable to get dimension %s in NetCDF file.  NetCDF error message: %s.\n", dimensionName,
              nc_strerror(ncErrorCode));
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  
  // Get the dimension length.
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(fileID, dimensionID, dimensionSize);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readNetCDFDimensionSize: unable to get length of dimension %s in NetCDF file.  NetCDF error message: %s.\n",
                  dimensionName, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

int FileManager::createNetCDFVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID0, int dimensionID1,
                                      int dimensionID2)
{
  bool error = false;                 // Error flag.
  int  ncErrorCode;                   // Return value of NetCDF functions.
  int  dimensionIDs[NC_MAX_VAR_DIMS]; // For passing dimension IDs.
  int  variableID;                    // For getting unused variable ID.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != variableName && 1 <= numberOfDimensions && 3 >= numberOfDimensions);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  dimensionIDs[0] = dimensionID0;
  dimensionIDs[1] = dimensionID1;
  dimensionIDs[2] = dimensionID2;
  
  ncErrorCode = nc_def_var(fileID, variableName, dataType, numberOfDimensions, dimensionIDs, &variableID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NC_NOERR == ncErrorCode))
    {
      CkError("ERROR in FileManager::createNetCDFVariable: unable to create variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
              nc_strerror(ncErrorCode));
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  
  return error;
}

template <typename T> int FileManager::readNetCDFVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart,
                                                          size_t numberOfNodesElements, size_t fileDimension, size_t memoryDimension, bool repeatLastValue,
                                                          T defaultValue, bool mandatory, T** variable)
{
  bool   error = false;          // Error flag.
  size_t ii, jj;                 // Loop counters.
  int    ncErrorCode;            // Return value of NetCDF functions.
  int    variableID;             // ID of variable in NetCDF file.
  size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
  size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
  T*     tempVariable;           // For remapping arrays when fileDimension is smaller than memoryDimension
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != variableName && 1 <= numberOfNodesElements && 1 <= memoryDimension && NULL != variable);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // fileDimenison must be less than or equal to memoryDimension.  Otherwise there is not enough room to read all of the data and it is an error.
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(1 <= fileDimension && fileDimension <= memoryDimension))
    {
      CkError("ERROR in FileManager::readNetCDFVariable: fileDimension must be greater than or equal to one and less than or equal to memoryDimension for "
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
              CkError("ERROR in FileManager::readNetCDFVariable: unable to get variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
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
              CkError("ERROR in FileManager::readNetCDFVariable: unable to read variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
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

int FileManager::writeNetCDFVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart, size_t numberOfNodesElements,
                                     size_t memoryDimension, void* variable)
{
  bool   error = false;          // Error flag.
  int    ncErrorCode;            // Return value of NetCDF functions.
  int    variableID;             // ID of variable in NetCDF file.
  size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
  size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != variableName && 1 <= numberOfNodesElements && 1 <= memoryDimension && NULL != variable);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Get the variable ID.
  ncErrorCode = nc_inq_varid(fileID, variableName, &variableID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NC_NOERR == ncErrorCode))
    {
      CkError("ERROR in FileManager::writeNetCDFVariable: unable to get variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
              nc_strerror(ncErrorCode));
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  
  // Set collective access.
  if (!error)
    {
      ncErrorCode = nc_var_par_access(fileID, variableID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeNetCDFVariable: unable to set collective access for variable %s in NetCDF file.  NetCDF error message: %s.\n",
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
          CkError("ERROR in FileManager::writeNetCDFVariable: unable to write variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

void FileManager::handleInitializeFromNetCDFFiles(const char* directory)
{
  bool    error         = false;        // Error flag.
  int     ii;                           // Loop counter.
  int     ncErrorCode;                  // Return value of NetCDF functions.
  int     stateFileID;                  // ID of NetCDF file.
  bool    stateFileOpen = false;        // Whether stateFileID refers to an open file.
  int     fileID;                       // ID of NetCDF file.
  bool    fileOpen      = false;        // Whether fileID refers to an open file.
  size_t  geometryInstance;             // Instance index for geometry file.
  size_t  parameterInstance;            // Instance index for parameter file.
  size_t  stateInstance;                // Instance index for state file.
  size_t  meshMeshNeighborsSize;        // size of the fixed dimension in the file.
  size_t  meshChannelNeighborsSize;     // size of the fixed dimension in the file.
  size_t  channelElementVerticesSize;   // size of the fixed dimension in the file.
  size_t  channelVerticesSize;          // size of the fixed dimension in the file.
  size_t  channelChannelNeighborsSize;  // size of the fixed dimension in the file.
  size_t  channelMeshNeighborsSize;     // size of the fixed dimension in the file.
  size_t  evapoTranspirationSnowLayers; // size of the fixed dimension in the file.
  size_t  evapoTranspirationAllLayers;  // size of the fixed dimension in the file.
  int*    intVariable;                  // For passing the address of a pointer to a local variable.
  size_t* sizeTVariable;                // For passing the address of a pointer to a local variable.
  double* doubleVariable;               // For passing the address of a pointer to a local variable.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Open the state file.
  if (!error)
    {
      error = openNetCDFFile(directory, "state.nc", false, false, &stateFileID);
      
      if (!error)
        {
          stateFileOpen = true;
        }
    }
  
  // Get the index of the last existing instance in the state file.
  if (!error)
    {
      error = readNetCDFDimensionSize(stateFileID, "instances", &stateInstance);
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
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: not creating a new instance and no existing instance in NetCDF state file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Get the size of the fixed dimensions in the state file.
  if (!error)
    {
      error = readNetCDFDimensionSize(stateFileID, "evapoTranspirationSnowLayers", &evapoTranspirationSnowLayers);
    }
  
  if (!error)
    {
      error = readNetCDFDimensionSize(stateFileID, "evapoTranspirationAllLayers", &evapoTranspirationAllLayers);
    }
  
  // Read state variables.
  if (!error)
    {
      sizeTVariable = &geometryInstance;
      error         = readNetCDFVariable(stateFileID, "geometryInstance", stateInstance, 0, 1, 1, 1, true, (size_t)0, true, &sizeTVariable);
    }
  
  if (!error)
    {
      sizeTVariable = &parameterInstance;
      error         = readNetCDFVariable(stateFileID, "parameterInstance", stateInstance, 0, 1, 1, 1, true, (size_t)0, true, &sizeTVariable);
    }
  
  if (!error)
    {
      doubleVariable = &referenceDate;
      error          = readNetCDFVariable(stateFileID, "referenceDate", stateInstance, 0, 1, 1, 1, true, 0.0, true, &doubleVariable);
    }
  
  if (!error)
    {
      doubleVariable = &currentTime;
      error          = readNetCDFVariable(stateFileID, "currentTime", stateInstance, 0, 1, 1, 1, true, 0.0, true, &doubleVariable);
    }
  
  if (!error)
    {
      doubleVariable = &dt;
      error          = readNetCDFVariable(stateFileID, "dt", stateInstance, 0, 1, 1, 1, true, 0.0, true, &doubleVariable);
    }
  
  if (!error)
    {
      sizeTVariable = &iteration;
      error         = readNetCDFVariable(stateFileID, "iteration", stateInstance, 0, 1, 1, 1, true, (size_t)0, true, &sizeTVariable);
    }
  
  // Open the geometry file.
  if (!error)
    {
      error = openNetCDFFile(directory, "geometry.nc", false, false, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Get the size of the fixed dimensions in the geometry file.
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "meshMeshNeighborsSize", &meshMeshNeighborsSize);
    }
  
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "meshChannelNeighborsSize", &meshChannelNeighborsSize);
    }
  
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "channelElementVerticesSize", &channelElementVerticesSize);
    }
  
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "channelVerticesSize", &channelVerticesSize);
    }
  
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "channelChannelNeighborsSize", &channelChannelNeighborsSize);
    }
  
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "channelMeshNeighborsSize", &channelMeshNeighborsSize);
    }
  
  // Read geometry variables.
  if (!error)
    {
      intVariable = &globalNumberOfMeshNodes;
      error       = readNetCDFVariable(fileID, "numberOfMeshNodes", geometryInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
    }
  
  if (!error)
    {
      intVariable = &globalNumberOfMeshElements;
      error       = readNetCDFVariable(fileID, "numberOfMeshElements", geometryInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
    }
  
  if (!error)
    {
      intVariable = &globalNumberOfChannelNodes;
      error       = readNetCDFVariable(fileID, "numberOfChannelNodes", geometryInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
    }
  
  if (!error)
    {
      intVariable = &globalNumberOfChannelElements;
      error       = readNetCDFVariable(fileID, "numberOfChannelElements", geometryInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
    }
  
  if (!error)
    {
      localStartAndNumber(&localMeshNodeStart,       &localNumberOfMeshNodes,       globalNumberOfMeshNodes);
      localStartAndNumber(&localMeshElementStart,    &localNumberOfMeshElements,    globalNumberOfMeshElements);
      localStartAndNumber(&localChannelNodeStart,    &localNumberOfChannelNodes,    globalNumberOfChannelNodes);
      localStartAndNumber(&localChannelElementStart, &localNumberOfChannelElements, globalNumberOfChannelElements);
    }
  
  if (0 < localNumberOfMeshNodes)
    {
      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshNodeX", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, 1, true, 0.0, false, &meshNodeX);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshNodeY", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, 1, true, 0.0, false, &meshNodeY);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshNodeZSurface", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, 1, true, 0.0, false,
                                     &meshNodeZSurface);
        }
    } // End if (0 < localNumberOfMeshNodes).
  
  if (0 < localNumberOfMeshElements)
    {
      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementVertices", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MeshElement::meshNeighborsSize, true, 0, false, (int**)&meshElementVertices);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshVertexX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MeshElement::meshNeighborsSize, true, 0.0, false, (double**)&meshVertexX);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshVertexY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MeshElement::meshNeighborsSize, true, 0.0, false, (double**)&meshVertexY);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshVertexZSurface", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MeshElement::meshNeighborsSize, true, 0.0, false, (double**)&meshVertexZSurface);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementX);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementY);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementZSurface", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementZSurface);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementSoilDepth", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0,
                                     false, &meshElementSoilDepth);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementZBedrock", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementZBedrock);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementArea", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementArea);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementSlopeX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementSlopeX);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshElementSlopeY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementSlopeY);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshMeshNeighbors", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MeshElement::meshNeighborsSize, false, (int)NOFLOW, false, (int**)&meshMeshNeighbors);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshMeshNeighborsChannelEdge", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MeshElement::meshNeighborsSize, false, false, false, (bool**)&meshMeshNeighborsChannelEdge);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshMeshNeighborsEdgeLength", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MeshElement::meshNeighborsSize, false, 1.0, false, (double**)&meshMeshNeighborsEdgeLength);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshMeshNeighborsEdgeNormalX", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MeshElement::meshNeighborsSize, false, 1.0, false, (double**)&meshMeshNeighborsEdgeNormalX);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshMeshNeighborsEdgeNormalY", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MeshElement::meshNeighborsSize, false, 0.0, false, (double**)&meshMeshNeighborsEdgeNormalY);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshChannelNeighbors", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MeshElement::channelNeighborsSize, false, (int)NOFLOW, false, (int**)&meshChannelNeighbors);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshChannelNeighborsEdgeLength", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MeshElement::channelNeighborsSize, false, 1.0, false, (double**)&meshChannelNeighborsEdgeLength);
        }
    } // End if (0 < localNumberOfMeshElements).
  
  if (0 < localNumberOfChannelNodes)
    {
      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelNodeX", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, 1, true, 0.0, false,
                                     &channelNodeX);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelNodeY", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, 1, true, 0.0, false,
                                     &channelNodeY);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelNodeZBank", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, 1, true, 0.0, false,
                                     &channelNodeZBank);
        }
    } // End if (0 < localNumberOfChannelNodes).
  
  if (0 < localNumberOfChannelElements)
    {
      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelElementVertices", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelElementVerticesSize, ChannelElement::channelVerticesSize + 2, true, 0, false, (int**)&channelElementVertices);

          // If ChannelElement::channelVerticesSize is not the same value as in the file we need to reset the number of vertices in the XDMF data.
          if (channelElementVerticesSize != ChannelElement::channelVerticesSize + 2)
            {
              for (ii = 0; ii < localNumberOfChannelElements; ii++)
                {
                  channelElementVertices[ii][1] = ChannelElement::channelVerticesSize;
                }
            }
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelVertexX", geometryInstance, localChannelElementStart, localNumberOfChannelElements, channelVerticesSize,
                                     ChannelElement::channelVerticesSize, true, 0.0, false, (double**)&channelVertexX);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelVertexY", geometryInstance, localChannelElementStart, localNumberOfChannelElements, channelVerticesSize,
                                     ChannelElement::channelVerticesSize, true, 0.0, false, (double**)&channelVertexY);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelVertexZBank", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelVerticesSize, ChannelElement::channelVerticesSize, true, 0.0, false, (double**)&channelVertexZBank);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelElementX", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementX);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelElementY", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementY);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelElementZBank", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementZBank);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelElementBankFullDepth", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1,
                                     true, 0.0, false, &channelElementBankFullDepth);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelElementZBed", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementZBed);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelElementLength", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementLength);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelChannelNeighbors", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, ChannelElement::channelNeighborsSize, false, (int)NOFLOW, false,
                                     (int**)&channelChannelNeighbors);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelMeshNeighbors", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, ChannelElement::meshNeighborsSize, false, (int)NOFLOW, false, (int**)&channelMeshNeighbors);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelMeshNeighborsEdgeLength", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, ChannelElement::meshNeighborsSize, false, 1.0, false, (double**)&channelMeshNeighborsEdgeLength);
        }
    } // End if (0 < localNumberOfChannelElements).
  
  // Close the geometry file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to close NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open the parameter file.
  if (!error)
    {
      error = openNetCDFFile(directory, "parameter.nc", false, false, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Read parameter variables.
  if (0 < localNumberOfMeshElements)
    {
      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshCatchment", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0, false,
                                     &meshCatchment);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshVegetationType", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0, false,
                                     &meshVegetationType);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshSoilType", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0, false,
                                     &meshSoilType);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshConductivity", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshConductivity);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshPorosity", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshPorosity);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "meshManningsN", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshManningsN);
        }
    } // End if (0 < localNumberOfMeshElements).
  
  if (0 < localNumberOfChannelElements)
    {
      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelChannelType", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     NOT_USED, false, &channelChannelType);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelPermanentCode", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0,
                                     false, &channelPermanentCode);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelBaseWidth", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelBaseWidth);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelSideSlope", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelSideSlope);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelBedConductivity", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     0.0, false, &channelBedConductivity);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelBedThickness", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelBedThickness);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "channelManningsN", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelManningsN);
        }
    } // End if (0 < localNumberOfChannelElements).
  
  // Close the parameter file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleInitializeFromNetCDFFiles: unable to close NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read state variables.
  if (0 < localNumberOfMeshElements)
    {
      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshSurfacewaterDepth", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0,
                                     false, &meshSurfacewaterDepth);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshSurfacewaterError", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0,
                                     false, &meshSurfacewaterError);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshGroundwaterHead", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0,
                                     false, &meshGroundwaterHead);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshGroundwaterError", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0,
                                     false, &meshGroundwaterError);
        }

      // Do not read precipitation, evaporation, and infiltration values.  Instantaneous values will be calculated before they are used.
      // Cumulative values are reset to zero at every I/O event.
      
      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshFIceOld", stateInstance, localMeshElementStart, localNumberOfMeshElements, evapoTranspirationSnowLayers,
                                     EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, false, 0.0f, false, (float**)&meshFIceOld);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshAlbOld", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshAlbOld);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshSnEqvO", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshSnEqvO);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshStc", stateInstance, localMeshElementStart, localNumberOfMeshElements, evapoTranspirationAllLayers,
                                     EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, true, 0.0f, false, (float**)&meshStc);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshTah", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshTah);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshEah", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshEah);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshFWet", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshFWet);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshCanLiq", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshCanLiq);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshCanIce", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshCanIce);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshTv", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false, &meshTv);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshTg", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false, &meshTg);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshISnow", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0, false,
                                     &meshISnow);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshZSnso", stateInstance, localMeshElementStart, localNumberOfMeshElements, evapoTranspirationAllLayers,
                                     EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, true, 0.0f, false, (float**)&meshZSnso);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshSnowH", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshSnowH);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshSnEqv", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshSnEqv);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshSnIce", stateInstance, localMeshElementStart, localNumberOfMeshElements, evapoTranspirationSnowLayers,
                                     EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, false, 0.0f, false, (float**)&meshSnIce);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshSnLiq", stateInstance, localMeshElementStart, localNumberOfMeshElements, evapoTranspirationSnowLayers,
                                     EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, false, 0.0f, false, (float**)&meshSnLiq);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshLfMass", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshLfMass);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshRtMass", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshRtMass);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshStMass", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshStMass);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshWood", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshWood);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshStblCp", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshStblCp);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshFastCp", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshFastCp);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshLai", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshLai);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshSai", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshSai);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshCm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false, &meshCm);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshCh", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false, &meshCh);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshTauss", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshTauss);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshDeepRech", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshDeepRech);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "meshRech", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, false,
                                     &meshRech);
        }

      // Do not read surfacewater and groundwater flows.  Instantaneous values will be calculated before they are used.
      // Cumulative values are reset to zero at every I/O event.
    } // End if (0 < localNumberOfMeshElements).
  
  if (0 < localNumberOfChannelElements)
    {
      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelSurfacewaterDepth", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1,
                                     true, 0.0, false, &channelSurfacewaterDepth);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelSurfacewaterError", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1,
                                     true, 0.0, false, &channelSurfacewaterError);
        }

      // Do not read precipitation and evaporation values.  Instantaneous values will be calculated before they are used.
      // Cumulative values are reset to zero at every I/O event.

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelFIceOld", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     evapoTranspirationSnowLayers, EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, false, 0.0f, false, (float**)&channelFIceOld);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelAlbOld", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelAlbOld);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelSnEqvO", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelSnEqvO);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelStc", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     evapoTranspirationAllLayers, EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, true, 0.0f, false, (float**)&channelStc);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelTah", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, false,
                                     &channelTah);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelEah", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, false,
                                     &channelEah);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelFWet", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelFWet);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelCanLiq", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelCanLiq);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelCanIce", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelCanIce);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelTv", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, false,
                                     &channelTv);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelTg", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, false,
                                     &channelTg);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelISnow", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0, false,
                                     &channelISnow);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelZSnso", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     evapoTranspirationAllLayers, EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, true, 0.0f, false, (float**)&channelZSnso);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelSnowH", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelSnowH);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelSnEqv", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelSnEqv);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelSnIce", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     evapoTranspirationSnowLayers, EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, false, 0.0f, false, (float**)&channelSnIce);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelSnLiq", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     evapoTranspirationSnowLayers, EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, false, 0.0f, false, (float**)&channelSnLiq);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelLfMass", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelLfMass);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelRtMass", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelRtMass);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelStMass", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelStMass);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelWood", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelWood);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelStblCp", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelStblCp);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelFastCp", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelFastCp);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelLai", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, false,
                                     &channelLai);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelSai", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, false,
                                     &channelSai);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelCm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, false,
                                     &channelCm);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelCh", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, false,
                                     &channelCh);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelTauss", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelTauss);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelDeepRech", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelDeepRech);
        }

      if (!error)
        {
          error = readNetCDFVariable(stateFileID, "channelRech", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f,
                                     false, &channelRech);
        }

      // Do not read surfacewater and groundwater flows.  Instantaneous values will be calculated before they are used.
      // Cumulative values are reset to zero at every I/O event.
    } // End if (0 < localNumberOfChannelElements).
  
  // Close the state file.
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
  
  // Have to call evapoTranspirationInit once on each Pe. This is a convenient place to do that.
  if (!error)
    {
      error = evapoTranspirationInit(directory);
    }

  if (error)
    {
      CkExit();
    }
}

void FileManager::updateVertices()
{
  int  ii, jj;                             // Loop counters.
  bool needToGetMeshVertexData    = false; // Whether we need to get any vertex data.
  bool needToGetChannelVertexData = false; // Whether we need to get any vertex data.

  // Get vertex data from node data.
  if (NULL != meshElementVertices)
    {
      if (NULL == meshVertexX && NULL != meshNodeX)
        {
          meshVertexX             = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetMeshVertexData = true;
        }

      if (NULL == meshVertexY && NULL != meshNodeY)
        {
          meshVertexY             = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetMeshVertexData = true;
        }

      if (NULL == meshVertexZSurface && NULL != meshNodeZSurface)
        {
          meshVertexZSurface      = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetMeshVertexData = true;
        }
    }

  if (NULL != channelElementVertices)
    {
      if (NULL == channelVertexX && NULL != channelNodeX)
        {
          channelVertexX             = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          needToGetChannelVertexData = true;
        }

      if (NULL == channelVertexY && NULL != channelNodeY)
        {
          channelVertexY             = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          needToGetChannelVertexData = true;
        }

      if (NULL == channelVertexZBank && NULL != channelNodeZBank)
        {
          channelVertexZBank         = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          needToGetChannelVertexData = true;
        }
    }

  if (needToGetMeshVertexData)
    {
      meshVertexUpdated = new bool[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for(ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              meshVertexUpdated[ii][jj] = false;

              if (localMeshNodeStart <= meshElementVertices[ii][jj] && meshElementVertices[ii][jj] < localMeshNodeStart + localNumberOfMeshNodes)
                {
                  // The node belongs to me, no need to send a message.
                  getMeshVertexDataMessage(CkMyPe(), ii + localMeshElementStart, jj, meshElementVertices[ii][jj]);
                }
              else
                {
                  // FIXME improve efficiency.  Don't send duplicate messages for the same node.
                  thisProxy[home(meshElementVertices[ii][jj], globalNumberOfMeshNodes)].getMeshVertexDataMessage(CkMyPe(), ii + localMeshElementStart, jj,
                                                                                                                 meshElementVertices[ii][jj]);
                }
            }
        }
    }

  if (needToGetChannelVertexData)
    {
      channelVertexUpdated = new bool[localNumberOfChannelElements][ChannelElement::channelVerticesSize];

      for(ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < ChannelElement::channelVerticesSize; jj++)
            {
              channelVertexUpdated[ii][jj] = false;

              if (localChannelNodeStart <= channelElementVertices[ii][jj] &&
                  channelElementVertices[ii][jj] < localChannelNodeStart + localNumberOfChannelNodes)
                {
                  // The node belongs to me, no need to send a message.
                  getChannelVertexDataMessage(CkMyPe(), ii + localChannelElementStart, jj, channelElementVertices[ii][jj]);
                }
              else
                {
                  // FIXME improve efficiency.  Don't send duplicate messages for the same node.
                  thisProxy[home(channelElementVertices[ii][jj], globalNumberOfChannelNodes)].getChannelVertexDataMessage(CkMyPe(), ii + localChannelElementStart,
                                                                                                                          jj, channelElementVertices[ii][jj]);
                }
            }
        }
    }
}

bool FileManager::allVerticesUpdated()
{
  int  ii, jj;         // Loop counters.
  bool updated = true; // Flag to record whether we have found an unupdated vertex.
  
  if (NULL != meshVertexUpdated)
    {
      for (ii = 0; updated && ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; updated && jj < MeshElement::meshNeighborsSize; jj++)
            {
              updated = meshVertexUpdated[ii][jj];
            }
        }
    }
  
  if (NULL != channelVertexUpdated)
    {
      for (ii = 0; updated && ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; updated && jj < ChannelElement::channelVerticesSize; jj++)
            {
              updated = channelVertexUpdated[ii][jj];
            }
        }
    }
  
  return updated;
}

void FileManager::handleMeshVertexDataMessage(int element, int vertex, double x, double y, double zSurface)
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
  
  meshVertexUpdated[element - localMeshElementStart][vertex] = true;
}

void FileManager::handleChannelVertexDataMessage(int element, int vertex, double x, double y, double zBank)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(localChannelElementStart <= element && element < localChannelElementStart + localNumberOfChannelElements))
    {
      CkError("ERROR in FileManager::channelVertexDataMessage: element data not owned by this local branch.\n");
      CkExit();
    }

  if (!(0 <= vertex && vertex < ChannelElement::channelVerticesSize))
    {
      CkError("ERROR in FileManager::channelVertexDataMessage: vertex must be greater than or equal to zero and less than "
              "ChannelElement::channelVerticesSize.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (NULL != channelNodeX)
    {
      channelVertexX[element - localChannelElementStart][vertex] = x;
    }

  if (NULL != channelNodeY)
    {
      channelVertexY[element - localChannelElementStart][vertex] = y;
    }

  if (NULL != channelNodeZBank)
    {
      channelVertexZBank[element - localChannelElementStart][vertex] = zBank;
    }
  
  channelVertexUpdated[element - localChannelElementStart][vertex] = true;
}

void FileManager::calculateDerivedValues()
{
  int    ii, jj; // Loop counters.
  double value;  // For calculating derived values.

  // Delete vertex updated arrays that are no longer needed.
  deleteArrayIfNonNull(&meshVertexUpdated);
  deleteArrayIfNonNull(&channelVertexUpdated);
  
  // Calculate meshElementX by averaging the X coordinates of its vertices.
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

  // Calculate meshElementY by averaging the Y coordinates of its vertices.
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

  // Calculate meshElementZSurface by averaging the surface Z coordinates of its vertices.
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
  
  // Calculate meshElementSoilDepth by subtracting meshElementZBedrock from meshElementZSurface.
  if (NULL == meshElementSoilDepth && NULL != meshElementZSurface && NULL != meshElementZBedrock)
    {
      meshElementSoilDepth = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshElementSoilDepth[ii] = meshElementZSurface[ii] - meshElementZBedrock[ii];
        }
    }
  
  // Calculate meshElementZBedrock by subtracting meshElementSoilDepth from meshElementZSurface.
  if (NULL == meshElementZBedrock && NULL != meshElementZSurface && NULL != meshElementSoilDepth)
    {
      meshElementZBedrock = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshElementZBedrock[ii] = meshElementZSurface[ii] - meshElementSoilDepth[ii];
        }
    }

  // Calculate meshElementArea from the X and Y coordinates of its vertices.
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

  // Calculate meshElementSlopeX from the X, Y, and Z coordinates of its vertices.
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

  // Calculate meshElementSlopeY from the X, Y, and Z coordinates of its vertices.
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
  
  // MeshConductivity and meshPorosity are taken from 19-category SOILPARM.TBL of Noah-MP.
  if ((NULL == meshConductivity || NULL == meshPorosity) && NULL != meshSoilType)
    {
      if (NULL == meshConductivity)
        {
          meshConductivity = new double[localNumberOfMeshElements];
        }

      if (NULL == meshPorosity)
        {
          meshPorosity     = new double[localNumberOfMeshElements];
        }

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          switch (meshSoilType[ii])
          {
          case  1: // SAND.
            meshConductivity[ii] = 4.66E-5;
            meshPorosity[ii]     = 0.339;
            break;
          case  2: // LOAMY SAND.
            meshConductivity[ii] = 1.41E-5;
            meshPorosity[ii]     = 0.421;
            break;
          case  3: // SANDY LOAM.
            meshConductivity[ii] = 5.23E-6;
            meshPorosity[ii]     = 0.434;
            break;
          case  4: // SILT LOAM.
            meshConductivity[ii] = 2.81E-6;
            meshPorosity[ii]     = 0.476;
            break;
          case  5: // SILT.
            meshConductivity[ii] = 2.81E-6;
            meshPorosity[ii]     = 0.476;
            break;
          case  6: // LOAM.
            meshConductivity[ii] = 3.38E-6;
            meshPorosity[ii]     = 0.439;
            break;
          case  7: // SANDY CLAY LOAM.
            meshConductivity[ii] = 4.45E-6;
            meshPorosity[ii]     = 0.404;
            break;
          case  8: // SILTY CLAY LOAM.
            meshConductivity[ii] = 2.03E-6;
            meshPorosity[ii]     = 0.464;
            break;
          case  9: // CLAY LOAM.
            meshConductivity[ii] = 2.45E-6;
            meshPorosity[ii]     = 0.465;
            break;
          case 10: // SANDY CLAY.
            meshConductivity[ii] = 7.22E-6;
            meshPorosity[ii]     = 0.406;
            break;
          case 11: // SILTY CLAY.
            meshConductivity[ii] = 1.34E-6;
            meshPorosity[ii]     = 0.468;
            break;
          case 12: // CLAY.
            meshConductivity[ii] = 9.74E-7;
            meshPorosity[ii]     = 0.468;
            break;
          case 13: // ORGANIC MATERIAL.
            meshConductivity[ii] = 3.38E-6;
            meshPorosity[ii]     = 0.439;
            break;
          case 14: // WATER.
            meshConductivity[ii] = 0.0;
            meshPorosity[ii]     = 1.0;
            break;
          case 15: // BEDROCK.
            meshConductivity[ii] = 1.41E-4;
            meshPorosity[ii]     = 0.2;
            break;
          case 16: // OTHER(land-ice).
            meshConductivity[ii] = 1.41E-5;
            meshPorosity[ii]     = 0.421;
            break;
          case 17: // PLAYA.
            meshConductivity[ii] = 9.74E-7;
            meshPorosity[ii]     = 0.468;
            break;
          case 18: // LAVA.
            meshConductivity[ii] = 1.41E-4;
            meshPorosity[ii]     = 0.2;
            break;
          case 19: // WHITE SAND.
            meshConductivity[ii] = 4.66E-5;
            meshPorosity[ii]     = 0.339;
            break;
          } // End of switch.
        } // End of element loop.
    } // End of assigning meshConductivity and meshPorosity.
  
  // Calculate meshManningsN by transfering the USGS 27-category vegetation type (in VEGPARM.TBL of Noah-MP) to the 21-category NLCD 1992 Land cover
  // classification and use the Manning's N from Bunya et al., 2009.  (High-resolution river inflow, tide, wind, wind wave and storm surge model for southern
  // Louisiana and Mississippi Part I.)
  // Need to correspond to the python scrip in ADHydro/scripts/readFromSSURGO_STASGO.py. See comments there.
  if (NULL == meshManningsN && NULL != meshVegetationType)
    {
      meshManningsN = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          switch (meshVegetationType[ii])
          {
            case 16: // open water
                   meshManningsN[ii] = 0.02;
                   break;
            case 24: //  ice/snow
                   meshManningsN[ii] = 0.022;
                   break; 
            case 1: //  Urban and Built-Up Land
                   meshManningsN[ii] = 0.12;
                   break;
            case 19: //  barren land
                   meshManningsN[ii] = 0.04;
                   break;
            case 11: //  deciduous forest
                   meshManningsN[ii] = 0.16;
                   break;
            case 13: //  evergreen forest
                   meshManningsN[ii] = 0.18;
                   break;
            case 15: //  mixed forest
                   meshManningsN[ii] = 0.17;
                   break;
            case 22: //  'Mixed Tundra' Alaska only, use n = 0.05.
                   meshManningsN[ii] = 0.05;
                   break;
            case 8: //   'Shrubland'
                   meshManningsN[ii] = 0.07;
                   break;
            case 7: //   'Grassland'
                   meshManningsN[ii] = 0.035;
                   break;
            case 2: //  'Dryland Cropland and Pasture'
                   meshManningsN[ii] = 0.033;
                   break;
            case 3: //  'Irrigated Cropland and Pasture' 
                   meshManningsN[ii] = 0.04;
                   break;
            case 18: //   'Wooded Wetland' 
                   meshManningsN[ii] = 0.14;
                   break;
            case 17: //  'Herbaceous Wetland' 
                   meshManningsN[ii] = 0.035;
                   break;            
          } // End of switch meshVegetationType[ii].
        } // End of element loop.
    } // End of assigning meshManningsN.
    
  // If not already specified meshSurfacewaterDepth defaults to zero.
  if (NULL == meshSurfacewaterDepth)
    {
      meshSurfacewaterDepth = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshSurfacewaterDepth[ii] = 0.0;
        }
    }
  
  // If not already specified meshSurfacewaterError defaults to zero.
  if (NULL == meshSurfacewaterError)
    {
      meshSurfacewaterError = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshSurfacewaterError[ii] = 0.0;
        }
    }
  
  // If not already specified meshGroundwaterHead defaults to meshElementZSurface.
  if (NULL == meshGroundwaterHead && NULL != meshElementZSurface)
    {
      meshGroundwaterHead = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshGroundwaterHead[ii] = meshElementZSurface[ii];
        }
    }
  
  // If not already specified meshGroundwaterError defaults to zero.
  if (NULL == meshGroundwaterError)
    {
      meshGroundwaterError = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshGroundwaterError[ii] = 0.0;
        }
    }
  
  // If not already specified meshPrecipitation defaults to zero.
  if (NULL == meshPrecipitation)
    {
      meshPrecipitation = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshPrecipitation[ii] = 0.0;
        }
    }
  
  // If not already specified meshPrecipitationCumulative defaults to zero.
  if (NULL == meshPrecipitationCumulative)
    {
      meshPrecipitationCumulative = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshPrecipitationCumulative[ii] = 0.0;
        }
    }
  
  // If not already specified meshEvaporation defaults to zero.
  if (NULL == meshEvaporation)
    {
      meshEvaporation = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshEvaporation[ii] = 0.0;
        }
    }
  
  // If not already specified meshEvaporationCumulative defaults to zero.
  if (NULL == meshEvaporationCumulative)
    {
      meshEvaporationCumulative = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshEvaporationCumulative[ii] = 0.0;
        }
    }
  
  // If not already specified meshSurfacewaterInfiltration defaults to zero.
  if (NULL == meshSurfacewaterInfiltration)
    {
      meshSurfacewaterInfiltration = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshSurfacewaterInfiltration[ii] = 0.0;
        }
    }
  
  // If not already specified meshGroundwaterRecharge defaults to zero.
  if (NULL == meshGroundwaterRecharge)
    {
      meshGroundwaterRecharge = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshGroundwaterRecharge[ii] = 0.0;
        }
    }
  
  // If not already specified meshFIceOld defaults to zero.
  if (NULL == meshFIceOld)
    {
      meshFIceOld = new float[localNumberOfMeshElements][EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; jj++)
            {
              meshFIceOld[ii][jj] = 0.0f;
            }
        }
    }
  
  // If not already specified meshAlbOld defaults to one.
  if (NULL == meshAlbOld)
    {
      meshAlbOld = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshAlbOld[ii] = 1.0f;
        }
    }
  
  // If not already specified meshSnEqvO defaults to zero.
  if (NULL == meshSnEqvO)
    {
      meshSnEqvO = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshSnEqvO[ii] = 0.0f;
        }
    }
  
  // Default for meshStc requires meshISnow so it is done after the defualt for meshISnow
  
  // If not already specified meshTah defaults to 300K.
  if (NULL == meshTah)
    {
      meshTah = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTah[ii] = 300.0f;
        }
    }
  
  // If not already specified meshEah defaults to 2000Pa.
  if (NULL == meshEah)
    {
      meshEah = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshEah[ii] = 2000.0f;
        }
    }
  
  // If not already specified meshFWet defaults to zero.
  if (NULL == meshFWet)
    {
      meshFWet = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshFWet[ii] = 0.0f;
        }
    }
  
  // If not already specified meshCanLiq defaults to zero.
  if (NULL == meshCanLiq)
    {
      meshCanLiq = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshCanLiq[ii] = 0.0f;
        }
    }
  
  // If not already specified meshCanIce defaults to zero.
  if (NULL == meshCanIce)
    {
      meshCanIce = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshCanIce[ii] = 0.0f;
        }
    }
  
  // If not already specified meshTv defaults to 300K.
  if (NULL == meshTv)
    {
      meshTv = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTv[ii] = 300.0f;
        }
    }
  
  // If not already specified meshTg defaults to 300K.
  if (NULL == meshTg)
    {
      meshTg = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTg[ii] = 300.0f;
        }
    }
  
  // If not already specified meshISnow defaults to zero.
  if (NULL == meshISnow)
    {
      meshISnow = new int[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshISnow[ii] = 0;
        }
    }
  
  // If not already specified meshStc defaults to zero in non-existant snow layers and 300K in existant snow layers and soil layers.
  if (NULL == meshStc && NULL != meshISnow)
    {
      meshStc = new float[localNumberOfMeshElements][EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - meshISnow[ii]; jj++)
            {
              meshStc[ii][jj] = 0.0f;
            }
          
          for (jj = EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - meshISnow[ii]; jj < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; jj++)
            {
              meshStc[ii][jj] = 300.0f;
            }
        }
    }
  
  // If not already specified meshZSnso defaults to zero in snow layers and 5%, 20%, 50%, and 100% of meshElementSoilDepth in soil layers.
  if (NULL == meshZSnso && NULL != meshElementSoilDepth)
    {
      meshZSnso = new float[localNumberOfMeshElements][EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshZSnso[ii][0] = 0.0f;
          meshZSnso[ii][1] = 0.0f;
          meshZSnso[ii][2] = 0.0f;
          meshZSnso[ii][3] = (float)(-0.05 * meshElementSoilDepth[ii]);
          meshZSnso[ii][4] = (float)(-0.2  * meshElementSoilDepth[ii]);
          meshZSnso[ii][5] = (float)(-0.5  * meshElementSoilDepth[ii]);
          meshZSnso[ii][6] = (float)(-1.0  * meshElementSoilDepth[ii]);
        }
    }
  
  // If not already specified meshSnowH defaults to zero.
  if (NULL == meshSnowH)
    {
      meshSnowH = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshSnowH[ii] = 0.0f;
        }
    }
  
  // If not already specified meshSnEqv defaults to zero.
  if (NULL == meshSnEqv)
    {
      meshSnEqv = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshSnEqv[ii] = 0.0f;
        }
    }
  
  // If not already specified meshSnIce defaults to zero.
  if (NULL == meshSnIce)
    {
      meshSnIce = new float[localNumberOfMeshElements][EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; jj++)
            {
              meshSnIce[ii][jj] = 0.0f;
            }
        }
    }
  
  // If not already specified meshSnLiq defaults to zero.
  if (NULL == meshSnLiq)
    {
      meshSnLiq = new float[localNumberOfMeshElements][EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; jj++)
            {
              meshSnLiq[ii][jj] = 0.0f;
            }
        }
    }
  
  // If not already specified meshLfMass defaults to 100000.
  if (NULL == meshLfMass)
    {
      meshLfMass = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // FIXME set according to veg type and area?
          meshLfMass[ii] = 100000.0f;
        }
    }
  
  // If not already specified meshRtMass defaults to 100000.
  if (NULL == meshRtMass)
    {
      meshRtMass = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // FIXME set according to veg type and area?
          meshRtMass[ii] = 100000.0f;
        }
    }
  
  // If not already specified meshStMass defaults to 100000.
  if (NULL == meshStMass)
    {
      meshStMass = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // FIXME set according to veg type and area?
          meshStMass[ii] = 100000.0f;
        }
    }
  
  // If not already specified meshWood defaults to 200000.
  if (NULL == meshWood)
    {
      meshWood = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // FIXME set according to veg type and area?
          meshWood[ii] = 200000.0f;
        }
    }
  
  // If not already specified meshStblCp defaults to 200000.
  if (NULL == meshStblCp)
    {
      meshStblCp = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // FIXME set according to veg type and area?
          meshStblCp[ii] = 200000.0f;
        }
    }
  
  // If not already specified meshFastCp defaults to 200000.
  if (NULL == meshFastCp)
    {
      meshFastCp = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // FIXME set according to veg type and area?
          meshFastCp[ii] = 200000.0f;
        }
    }
  
  // If not already specified meshLai defaults to 4.6.
  if (NULL == meshLai)
    {
      meshLai = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // FIXME set according to veg type and area?
          meshLai[ii] = 4.6f;
        }
    }
  
  // If not already specified meshSai defaults to 0.6.
  if (NULL == meshSai)
    {
      meshSai = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // FIXME set according to veg type and area?
          meshSai[ii] = 0.6f;
        }
    }
  
  // If not already specified meshCm defaults to 0.002.
  if (NULL == meshCm)
    {
      meshCm = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshCm[ii] = 0.002f;
        }
    }
  
  // If not already specified meshCh defaults to 0.002.
  if (NULL == meshCh)
    {
      meshCh = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshCh[ii] = 0.002f;
        }
    }
  
  // If not already specified meshTauss defaults to zero.
  if (NULL == meshTauss)
    {
      meshTauss = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTauss[ii] = 0.0f;
        }
    }
  
  // If not already specified meshDeepRech defaults to zero.
  if (NULL == meshDeepRech)
    {
      meshDeepRech = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshDeepRech[ii] = 0.0f;
        }
    }
  
  // If not already specified meshRech defaults to zero.
  if (NULL == meshRech)
    {
      meshRech = new float[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshRech[ii] = 0.0f;
        }
    }

  // Calculate meshMeshNeighborsEdgeLength from the X and Y coordinates of its vertices.
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

  // Calculate meshMeshNeighborsEdgeNormalX from the X and Y coordinates of its vertices.
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

  // Calculate meshMeshNeighborsEdgeNormalY from the X and Y coordinates of its vertices.
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
  
  // If not already specified meshMeshNeighborsSurfacewaterFlowRate defaults to zero.
  if (NULL == meshMeshNeighborsSurfacewaterFlowRate)
    {
      meshMeshNeighborsSurfacewaterFlowRate = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              meshMeshNeighborsSurfacewaterFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshMeshNeighborsSurfacewaterCumulativeFlow defaults to zero.
  if (NULL == meshMeshNeighborsSurfacewaterCumulativeFlow)
    {
      meshMeshNeighborsSurfacewaterCumulativeFlow = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              meshMeshNeighborsSurfacewaterCumulativeFlow[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshMeshNeighborsGroundwaterFlowRate defaults to zero.
  if (NULL == meshMeshNeighborsGroundwaterFlowRate)
    {
      meshMeshNeighborsGroundwaterFlowRate = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              meshMeshNeighborsGroundwaterFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshMeshNeighborsGroundwaterCumulativeFlow defaults to zero.
  if (NULL == meshMeshNeighborsGroundwaterCumulativeFlow)
    {
      meshMeshNeighborsGroundwaterCumulativeFlow = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              meshMeshNeighborsGroundwaterCumulativeFlow[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshChannelNeighborsSurfacewaterFlowRate defaults to zero.
  if (NULL == meshChannelNeighborsSurfacewaterFlowRate)
    {
      meshChannelNeighborsSurfacewaterFlowRate = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::channelNeighborsSize; jj++)
            {
              meshChannelNeighborsSurfacewaterFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshChannelNeighborsSurfacewaterCumulativeFlow defaults to zero.
  if (NULL == meshChannelNeighborsSurfacewaterCumulativeFlow)
    {
      meshChannelNeighborsSurfacewaterCumulativeFlow = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::channelNeighborsSize; jj++)
            {
              meshChannelNeighborsSurfacewaterCumulativeFlow[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshChannelNeighborsGroundwaterFlowRate defaults to zero.
  if (NULL == meshChannelNeighborsGroundwaterFlowRate)
    {
      meshChannelNeighborsGroundwaterFlowRate = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::channelNeighborsSize; jj++)
            {
              meshChannelNeighborsGroundwaterFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshChannelNeighborsGroundwaterCumulativeFlow defaults to zero.
  if (NULL == meshChannelNeighborsGroundwaterCumulativeFlow)
    {
      meshChannelNeighborsGroundwaterCumulativeFlow = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::channelNeighborsSize; jj++)
            {
              meshChannelNeighborsGroundwaterCumulativeFlow[ii][jj] = 0.0;
            }
        }
    }
  
  // FIXME calculate channelElementX, channelElementY, channelElementZBank from channel vertices.
  
  // Calculate channelElementBankFullDepth by subtracting channelElementZBed from channelElementZBank.
  if (NULL == channelElementBankFullDepth && NULL != channelElementZBank && NULL != channelElementZBed)
    {
      channelElementBankFullDepth = new double[localNumberOfChannelElements];

      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelElementBankFullDepth[ii] = channelElementZBank[ii] - channelElementZBed[ii];
        }
    }
  
  // Calculate channelElementZBed by subtracting channelElementBankFullDepth from channelElementZBank.
  if (NULL == channelElementZBed && NULL != channelElementZBank && NULL != channelElementBankFullDepth)
    {
      channelElementZBed = new double[localNumberOfChannelElements];

      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelElementZBed[ii] = channelElementZBank[ii] - channelElementBankFullDepth[ii];
        }
    }
  
  // FIXME how do we calculate channelBaseWidth, channelSideSlope, channelBedConductivity, channelBedThickness, channelManningsN
  
  // If not already specified channelSurfacewaterDepth defaults to zero.
  if (NULL == channelSurfacewaterDepth)
    {
      channelSurfacewaterDepth = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelSurfacewaterDepth[ii] = 0.0;
        }
    }
  
  // If not already specified channelSurfacewaterError defaults to zero.
  if (NULL == channelSurfacewaterError)
    {
      channelSurfacewaterError = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelSurfacewaterError[ii] = 0.0;
        }
    }
  
  // If not already specified channelPrecipitation defaults to zero.
  if (NULL == channelPrecipitation)
    {
      channelPrecipitation = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelPrecipitation[ii] = 0.0;
        }
    }
  
  // If not already specified channelPrecipitationCumulative defaults to zero.
  if (NULL == channelPrecipitationCumulative)
    {
      channelPrecipitationCumulative = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelPrecipitationCumulative[ii] = 0.0;
        }
    }
  
  // If not already specified channelEvaporation defaults to zero.
  if (NULL == channelEvaporation)
    {
      channelEvaporation = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelEvaporation[ii] = 0.0;
        }
    }
  
  // If not already specified channelEvaporationCumulative defaults to zero.
  if (NULL == channelEvaporationCumulative)
    {
      channelEvaporationCumulative = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelEvaporationCumulative[ii] = 0.0;
        }
    }
  
  // If not already specified channelFIceOld defaults to zero.
  if (NULL == channelFIceOld)
    {
      channelFIceOld = new float[localNumberOfChannelElements][EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; jj++)
            {
              channelFIceOld[ii][jj] = 0.0f;
            }
        }
    }
  
  // If not already specified channelAlbOld defaults to one.
  if (NULL == channelAlbOld)
    {
      channelAlbOld = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelAlbOld[ii] = 1.0f;
        }
    }
  
  // If not already specified channelSnEqvO defaults to zero.
  if (NULL == channelSnEqvO)
    {
      channelSnEqvO = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelSnEqvO[ii] = 0.0f;
        }
    }
  
  // Default for channelStc requires channelISnow so it is done after the defualt for channelISnow
  
  // If not already specified channelTah defaults to 300K.
  if (NULL == channelTah)
    {
      channelTah = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelTah[ii] = 300.0f;
        }
    }
  
  // If not already specified channelEah defaults to 2000Pa.
  if (NULL == channelEah)
    {
      channelEah = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelEah[ii] = 2000.0f;
        }
    }
  
  // If not already specified channelFWet defaults to zero.
  if (NULL == channelFWet)
    {
      channelFWet = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelFWet[ii] = 0.0f;
        }
    }
  
  // If not already specified channelCanLiq defaults to zero.
  if (NULL == channelCanLiq)
    {
      channelCanLiq = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelCanLiq[ii] = 0.0f;
        }
    }
  
  // If not already specified channelCanIce defaults to zero.
  if (NULL == channelCanIce)
    {
      channelCanIce = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelCanIce[ii] = 0.0f;
        }
    }
  
  // If not already specified channelTv defaults to 300K.
  if (NULL == channelTv)
    {
      channelTv = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelTv[ii] = 300.0f;
        }
    }
  
  // If not already specified channelTg defaults to 300K.
  if (NULL == channelTg)
    {
      channelTg = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelTg[ii] = 300.0f;
        }
    }
  
  // If not already specified channelISnow defaults to zero.
  if (NULL == channelISnow)
    {
      channelISnow = new int[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelISnow[ii] = 0;
        }
    }
  
  // If not already specified channelStc defaults to zero in non-existant snow layers and 300K in existant snow layers and soil layers.
  if (NULL == channelStc && NULL != channelISnow)
    {
      channelStc = new float[localNumberOfChannelElements][EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - channelISnow[ii]; jj++)
            {
              channelStc[ii][jj] = 0.0f;
            }
          
          for (jj = EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - channelISnow[ii]; jj < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; jj++)
            {
              channelStc[ii][jj] = 300.0f;
            }
        }
    }
  
  // FIXME channelElementBankFullDepth isn't really an indicator of soil depth.
  // If not already specified channelZSnso defaults to zero in snow layers and 5%, 20%, 50%, and 100% of channelElementBankFullDepth in soil layers.
  if (NULL == channelZSnso && NULL != channelElementBankFullDepth)
    {
      channelZSnso = new float[localNumberOfChannelElements][EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelZSnso[ii][0] = 0.0f;
          channelZSnso[ii][1] = 0.0f;
          channelZSnso[ii][2] = 0.0f;
          channelZSnso[ii][3] = (float)(-0.05 * channelElementBankFullDepth[ii]);
          channelZSnso[ii][4] = (float)(-0.2  * channelElementBankFullDepth[ii]);
          channelZSnso[ii][5] = (float)(-0.5  * channelElementBankFullDepth[ii]);
          channelZSnso[ii][6] = (float)(-1.0  * channelElementBankFullDepth[ii]);
        }
    }
  
  // If not already specified channelSnowH defaults to zero.
  if (NULL == channelSnowH)
    {
      channelSnowH = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelSnowH[ii] = 0.0f;
        }
    }
  
  // If not already specified channelSnEqv defaults to zero.
  if (NULL == channelSnEqv)
    {
      channelSnEqv = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelSnEqv[ii] = 0.0f;
        }
    }
  
  // If not already specified channelSnIce defaults to zero.
  if (NULL == channelSnIce)
    {
      channelSnIce = new float[localNumberOfChannelElements][EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; jj++)
            {
              channelSnIce[ii][jj] = 0.0f;
            }
        }
    }
  
  // If not already specified channelSnLiq defaults to zero.
  if (NULL == channelSnLiq)
    {
      channelSnLiq = new float[localNumberOfChannelElements][EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; jj++)
            {
              channelSnLiq[ii][jj] = 0.0f;
            }
        }
    }
  
  // If not already specified channelLfMass defaults to zero.
  if (NULL == channelLfMass)
    {
      channelLfMass = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelLfMass[ii] = 0.0f;
        }
    }
  
  // If not already specified channelRtMass defaults to zero.
  if (NULL == channelRtMass)
    {
      channelRtMass = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelRtMass[ii] = 0.0f;
        }
    }
  
  // If not already specified channelStMass defaults to zero.
  if (NULL == channelStMass)
    {
      channelStMass = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelStMass[ii] = 0.0f;
        }
    }
  
  // If not already specified channelWood defaults to zero.
  if (NULL == channelWood)
    {
      channelWood = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelWood[ii] = 0.0f;
        }
    }
  
  // If not already specified channelStblCp defaults to zero.
  if (NULL == channelStblCp)
    {
      channelStblCp = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelStblCp[ii] = 0.0f;
        }
    }
  
  // If not already specified channelFastCp defaults to zero.
  if (NULL == channelFastCp)
    {
      channelFastCp = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelFastCp[ii] = 0.0f;
        }
    }
  
  // If not already specified channelLai defaults to zero.
  if (NULL == channelLai)
    {
      channelLai = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelLai[ii] = 0.0f;
        }
    }
  
  // If not already specified channelSai defaults to zero.
  if (NULL == channelSai)
    {
      channelSai = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelSai[ii] = 0.0f;
        }
    }
  
  // If not already specified channelCm defaults to 0.002.
  if (NULL == channelCm)
    {
      channelCm = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelCm[ii] = 0.002f;
        }
    }
  
  // If not already specified channelCh defaults to 0.002.
  if (NULL == channelCh)
    {
      channelCh = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelCh[ii] = 0.002f;
        }
    }
  
  // If not already specified channelTauss defaults to zero.
  if (NULL == channelTauss)
    {
      channelTauss = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelTauss[ii] = 0.0f;
        }
    }
  
  // If not already specified channelDeepRech defaults to zero.
  if (NULL == channelDeepRech)
    {
      channelDeepRech = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelDeepRech[ii] = 0.0f;
        }
    }
  
  // If not already specified channelRech defaults to zero.
  if (NULL == channelRech)
    {
      channelRech = new float[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelRech[ii] = 0.0f;
        }
    }
  
  // If not already specified channelChannelNeighborsSurfacewaterFlowRate defaults to zero.
  if (NULL == channelChannelNeighborsSurfacewaterFlowRate)
    {
      channelChannelNeighborsSurfacewaterFlowRate = new double[localNumberOfChannelElements][ChannelElement::channelNeighborsSize];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < ChannelElement::channelNeighborsSize; jj++)
            {
              channelChannelNeighborsSurfacewaterFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelChannelNeighborsSurfacewaterCumulativeFlow defaults to zero.
  if (NULL == channelChannelNeighborsSurfacewaterCumulativeFlow)
    {
      channelChannelNeighborsSurfacewaterCumulativeFlow = new double[localNumberOfChannelElements][ChannelElement::channelNeighborsSize];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < ChannelElement::channelNeighborsSize; jj++)
            {
              channelChannelNeighborsSurfacewaterCumulativeFlow[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelMeshNeighborsSurfacewaterFlowRate defaults to zero.
  if (NULL == channelMeshNeighborsSurfacewaterFlowRate)
    {
      channelMeshNeighborsSurfacewaterFlowRate = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < ChannelElement::meshNeighborsSize; jj++)
            {
              channelMeshNeighborsSurfacewaterFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelMeshNeighborsSurfacewaterCumulativeFlow defaults to zero.
  if (NULL == channelMeshNeighborsSurfacewaterCumulativeFlow)
    {
      channelMeshNeighborsSurfacewaterCumulativeFlow = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < ChannelElement::meshNeighborsSize; jj++)
            {
              channelMeshNeighborsSurfacewaterCumulativeFlow[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelMeshNeighborsGroundwaterFlowRate defaults to zero.
  if (NULL == channelMeshNeighborsGroundwaterFlowRate)
    {
      channelMeshNeighborsGroundwaterFlowRate = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < ChannelElement::meshNeighborsSize; jj++)
            {
              channelMeshNeighborsGroundwaterFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelMeshNeighborsGroundwaterCumulativeFlow defaults to zero.
  if (NULL == channelMeshNeighborsGroundwaterCumulativeFlow)
    {
      channelMeshNeighborsGroundwaterCumulativeFlow = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < ChannelElement::meshNeighborsSize; jj++)
            {
              channelMeshNeighborsGroundwaterCumulativeFlow[ii][jj] = 0.0;
            }
        }
    }

  // Allocate arrays to record when state update messages are received.
  if (0 < localNumberOfMeshElements)
    {
      meshElementUpdated = new bool[localNumberOfMeshElements];
    }

  if (0 < localNumberOfChannelElements)
    {
      channelElementUpdated = new bool[localNumberOfChannelElements];
    }

  contribute();
}

void FileManager::handleCreateFiles(const char* directory)
{
  bool error    = false;                        // Error flag.
  int  ncErrorCode;                             // Return value of NetCDF functions.
  int  fileID;                                  // ID of NetCDF file.
  bool fileOpen = false;                        // Whether fileID refers to an open file.
  int  instancesDimensionID;                    // ID of dimension in NetCDF file.
  int  meshNodesDimensionID;                    // ID of dimension in NetCDF file.
  int  meshElementsDimensionID;                 // ID of dimension in NetCDF file.
  int  meshMeshNeighborsSizeDimensionID;        // ID of dimension in NetCDF file.
  int  meshChannelNeighborsSizeDimensionID;     // ID of dimension in NetCDF file.
  int  channelNodesDimensionID;                 // ID of dimension in NetCDF file.
  int  channelElementsDimensionID;              // ID of dimension in NetCDF file.
  int  channelElementVerticesSizeDimensionID;   // ID of dimension in NetCDF file.
  int  channelVerticesSizeDimensionID;          // ID of dimension in NetCDF file.
  int  channelChannelNeighborsSizeDimensionID;  // ID of dimension in NetCDF file.
  int  channelMeshNeighborsSizeDimensionID;     // ID of dimension in NetCDF file.
  int  evapoTranspirationSnowLayersDimensionID; // ID of dimension in NetCDF file.
  int  evapoTranspirationAllLayersDimensionID;  // ID of dimension in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleCreateFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // Create the geometry file.
  if (!error)
    {
      error = openNetCDFFile(directory, "geometry.nc", true, false, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Create geometry dimensions.
  if (!error)
    {
      error = createNetCDFDimension(fileID, "instances", NC_UNLIMITED, &instancesDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "meshNodes", NC_UNLIMITED, &meshNodesDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "meshMeshNeighborsSize", MeshElement::meshNeighborsSize, &meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "meshChannelNeighborsSize", MeshElement::channelNeighborsSize, &meshChannelNeighborsSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelNodes", NC_UNLIMITED, &channelNodesDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelElementVerticesSize", ChannelElement::channelVerticesSize + 2, &channelElementVerticesSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelVerticesSize", ChannelElement::channelVerticesSize, &channelVerticesSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelChannelNeighborsSize", ChannelElement::channelNeighborsSize, &channelChannelNeighborsSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelMeshNeighborsSize", ChannelElement::meshNeighborsSize, &channelMeshNeighborsSizeDimensionID);
    }
  
  // Create geometry variables.
  if (!error)
    {
      error = createNetCDFVariable(fileID, "numberOfMeshNodes", NC_INT, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error)
    {
      error = createNetCDFVariable(fileID, "numberOfMeshElements", NC_INT, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error)
    {
      error = createNetCDFVariable(fileID, "numberOfChannelNodes", NC_INT, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error)
    {
      error = createNetCDFVariable(fileID, "numberOfChannelElements", NC_INT, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error && NULL != meshNodeX)
    {
      error = createNetCDFVariable(fileID, "meshNodeX", NC_DOUBLE, 2, instancesDimensionID, meshNodesDimensionID, 0);
    }
  
  if (!error && NULL != meshNodeY)
    {
      error = createNetCDFVariable(fileID, "meshNodeY", NC_DOUBLE, 2, instancesDimensionID, meshNodesDimensionID, 0);
    }
  
  if (!error && NULL != meshNodeZSurface)
    {
      error = createNetCDFVariable(fileID, "meshNodeZSurface", NC_DOUBLE, 2, instancesDimensionID, meshNodesDimensionID, 0);
    }
  
  if (!error && NULL != meshElementVertices)
    {
      error = createNetCDFVariable(fileID, "meshElementVertices", NC_INT, 3, instancesDimensionID, meshElementsDimensionID, meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshVertexX)
    {
      error = createNetCDFVariable(fileID, "meshVertexX", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID, meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshVertexY)
    {
      error = createNetCDFVariable(fileID, "meshVertexY", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID, meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshVertexZSurface)
    {
      error = createNetCDFVariable(fileID, "meshVertexZSurface", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshElementX)
    {
      error = createNetCDFVariable(fileID, "meshElementX", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshElementY)
    {
      error = createNetCDFVariable(fileID, "meshElementY", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshElementZSurface)
    {
      error = createNetCDFVariable(fileID, "meshElementZSurface", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshElementSoilDepth)
    {
      error = createNetCDFVariable(fileID, "meshElementSoilDepth", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshElementZBedrock)
    {
      error = createNetCDFVariable(fileID, "meshElementZBedrock", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshElementArea)
    {
      error = createNetCDFVariable(fileID, "meshElementArea", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshElementSlopeX)
    {
      error = createNetCDFVariable(fileID, "meshElementSlopeX", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshElementSlopeY)
    {
      error = createNetCDFVariable(fileID, "meshElementSlopeY", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshMeshNeighbors)
    {
      error = createNetCDFVariable(fileID, "meshMeshNeighbors", NC_INT, 3, instancesDimensionID, meshElementsDimensionID, meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshMeshNeighborsChannelEdge)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Assumes bool is 1 byte when storing as NC_BYTE.
      CkAssert(1 == sizeof(bool));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      error = createNetCDFVariable(fileID, "meshMeshNeighborsChannelEdge", NC_BYTE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeLength)
    {
      error = createNetCDFVariable(fileID, "meshMeshNeighborsEdgeLength", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalX)
    {
      error = createNetCDFVariable(fileID, "meshMeshNeighborsEdgeNormalX", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalY)
    {
      error = createNetCDFVariable(fileID, "meshMeshNeighborsEdgeNormalY", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshChannelNeighbors)
    {
      error = createNetCDFVariable(fileID, "meshChannelNeighbors", NC_INT, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshChannelNeighborsEdgeLength)
    {
      error = createNetCDFVariable(fileID, "meshChannelNeighborsEdgeLength", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelNodeX)
    {
      error = createNetCDFVariable(fileID, "channelNodeX", NC_DOUBLE, 2, instancesDimensionID, channelNodesDimensionID, 0);
    }
  
  if (!error && NULL != channelNodeY)
    {
      error = createNetCDFVariable(fileID, "channelNodeY", NC_DOUBLE, 2, instancesDimensionID, channelNodesDimensionID, 0);
    }
  
  if (!error && NULL != channelNodeZBank)
    {
      error = createNetCDFVariable(fileID, "channelNodeZBank", NC_DOUBLE, 2, instancesDimensionID, channelNodesDimensionID, 0);
    }
  
  if (!error && NULL != channelElementVertices)
    {
      error = createNetCDFVariable(fileID, "channelElementVertices", NC_INT, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelElementVerticesSizeDimensionID);
    }
  
  if (!error && NULL != channelVertexX)
    {
      error = createNetCDFVariable(fileID, "channelVertexX", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID, channelVerticesSizeDimensionID);
    }
  
  if (!error && NULL != channelVertexY)
    {
      error = createNetCDFVariable(fileID, "channelVertexY", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID, channelVerticesSizeDimensionID);
    }
  
  if (!error && NULL != channelVertexZBank)
    {
      error = createNetCDFVariable(fileID, "channelVertexZBank", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelVerticesSizeDimensionID);
    }
  
  if (!error && NULL != channelElementX)
    {
      error = createNetCDFVariable(fileID, "channelElementX", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelElementY)
    {
      error = createNetCDFVariable(fileID, "channelElementY", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelElementZBank)
    {
      error = createNetCDFVariable(fileID, "channelElementZBank", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelElementBankFullDepth)
    {
      error = createNetCDFVariable(fileID, "channelElementBankFullDepth", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelElementZBed)
    {
      error = createNetCDFVariable(fileID, "channelElementZBed", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelElementLength)
    {
      error = createNetCDFVariable(fileID, "channelElementLength", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelChannelNeighbors)
    {
      error = createNetCDFVariable(fileID, "channelChannelNeighbors", NC_INT, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelMeshNeighbors)
    {
      error = createNetCDFVariable(fileID, "channelMeshNeighbors", NC_INT, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelMeshNeighborsEdgeLength)
    {
      error = createNetCDFVariable(fileID, "channelMeshNeighborsEdgeLength", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelMeshNeighborsSizeDimensionID);
    }
  
  // Close the geometry file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCreateFiles: unable to close NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create the parameter file.
  if (!error)
    {
      error = openNetCDFFile(directory, "parameter.nc", true, false, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Create parameter dimensions.
  if (!error)
    {
      error = createNetCDFDimension(fileID, "instances", NC_UNLIMITED, &instancesDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimensionID);
    }
  
  // Create parameter variables.
  if (!error)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Assumes size_t is 8 bytes when storing as NC_UINT64.
      CkAssert(8 == sizeof(size_t));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      error = createNetCDFVariable(fileID, "geometryInstance", NC_UINT64, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error && NULL != meshCatchment)
    {
      error = createNetCDFVariable(fileID, "meshCatchment", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshVegetationType)
    {
      error = createNetCDFVariable(fileID, "meshVegetationType", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshSoilType)
    {
      error = createNetCDFVariable(fileID, "meshSoilType", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshConductivity)
    {
      error = createNetCDFVariable(fileID, "meshConductivity", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshPorosity)
    {
      error = createNetCDFVariable(fileID, "meshPorosity", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshManningsN)
    {
      error = createNetCDFVariable(fileID, "meshManningsN", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelChannelType)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Assumes ChannelTypeEnum is 4 bytes when storing as NC_INT.
      CkAssert(4 == sizeof(ChannelTypeEnum));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      error = createNetCDFVariable(fileID, "channelChannelType", NC_INT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelPermanentCode)
    {
      error = createNetCDFVariable(fileID, "channelPermanentCode", NC_INT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelBaseWidth)
    {
      error = createNetCDFVariable(fileID, "channelBaseWidth", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelSideSlope)
    {
      error = createNetCDFVariable(fileID, "channelSideSlope", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelBedConductivity)
    {
      error = createNetCDFVariable(fileID, "channelBedConductivity", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelBedThickness)
    {
      error = createNetCDFVariable(fileID, "channelBedThickness", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelManningsN)
    {
      error = createNetCDFVariable(fileID, "channelManningsN", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  // Close the parameter file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCreateFiles: unable to close NetCDF parameter file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create the state file.
  if (!error)
    {
      error = openNetCDFFile(directory, "state.nc", true, false, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Create state dimensions.
  if (!error)
    {
      error = createNetCDFDimension(fileID, "instances", NC_UNLIMITED, &instancesDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "meshMeshNeighborsSize", MeshElement::meshNeighborsSize, &meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "meshChannelNeighborsSize", MeshElement::channelNeighborsSize, &meshChannelNeighborsSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelChannelNeighborsSize", ChannelElement::channelNeighborsSize, &channelChannelNeighborsSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelMeshNeighborsSize", ChannelElement::meshNeighborsSize, &channelMeshNeighborsSizeDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "evapoTranspirationSnowLayers", EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS,
                                    &evapoTranspirationSnowLayersDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "evapoTranspirationAllLayers", EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, &evapoTranspirationAllLayersDimensionID);
    }
  
  // Create state variables.
  if (!error)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Assumes size_t is 8 bytes when storing as NC_UINT64.
      CkAssert(8 == sizeof(size_t));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      error = createNetCDFVariable(fileID, "geometryInstance", NC_UINT64, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Assumes size_t is 8 bytes when storing as NC_UINT64.
      CkAssert(8 == sizeof(size_t));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      error = createNetCDFVariable(fileID, "parameterInstance", NC_UINT64, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error)
    {
      error = createNetCDFVariable(fileID, "referenceDate", NC_DOUBLE, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error)
    {
      error = createNetCDFVariable(fileID, "currentTime", NC_DOUBLE, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error)
    {
      error = createNetCDFVariable(fileID, "dt", NC_DOUBLE, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Assumes size_t is 8 bytes when storing as NC_UINT64.
      CkAssert(8 == sizeof(size_t));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      error = createNetCDFVariable(fileID, "iteration", NC_UINT64, 1, instancesDimensionID, 0, 0);
    }
  
  if (!error && NULL != meshSurfacewaterDepth)
    {
      error = createNetCDFVariable(fileID, "meshSurfacewaterDepth", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshSurfacewaterError)
    {
      error = createNetCDFVariable(fileID, "meshSurfacewaterError", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshGroundwaterHead)
    {
      error = createNetCDFVariable(fileID, "meshGroundwaterHead", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshGroundwaterError)
    {
      error = createNetCDFVariable(fileID, "meshGroundwaterError", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshPrecipitation)
    {
      error = createNetCDFVariable(fileID, "meshPrecipitation", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshPrecipitationCumulative)
    {
      error = createNetCDFVariable(fileID, "meshPrecipitationCumulative", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshEvaporation)
    {
      error = createNetCDFVariable(fileID, "meshEvaporation", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshEvaporationCumulative)
    {
      error = createNetCDFVariable(fileID, "meshEvaporationCumulative", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshSurfacewaterInfiltration)
    {
      error = createNetCDFVariable(fileID, "meshSurfacewaterInfiltration", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshGroundwaterRecharge)
    {
      error = createNetCDFVariable(fileID, "meshGroundwaterRecharge", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshFIceOld)
    {
      error = createNetCDFVariable(fileID, "meshFIceOld", NC_FLOAT, 3, instancesDimensionID, meshElementsDimensionID, evapoTranspirationSnowLayersDimensionID);
    }
  
  if (!error && NULL != meshAlbOld)
    {
      error = createNetCDFVariable(fileID, "meshAlbOld", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshSnEqvO)
    {
      error = createNetCDFVariable(fileID, "meshSnEqvO", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshStc)
    {
      error = createNetCDFVariable(fileID, "meshStc", NC_FLOAT, 3, instancesDimensionID, meshElementsDimensionID, evapoTranspirationAllLayersDimensionID);
    }
  
  if (!error && NULL != meshTah)
    {
      error = createNetCDFVariable(fileID, "meshTah", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshEah)
    {
      error = createNetCDFVariable(fileID, "meshEah", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshFWet)
    {
      error = createNetCDFVariable(fileID, "meshFWet", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshCanLiq)
    {
      error = createNetCDFVariable(fileID, "meshCanLiq", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshCanIce)
    {
      error = createNetCDFVariable(fileID, "meshCanIce", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshTv)
    {
      error = createNetCDFVariable(fileID, "meshTv", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshTg)
    {
      error = createNetCDFVariable(fileID, "meshTg", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshISnow)
    {
      error = createNetCDFVariable(fileID, "meshISnow", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshZSnso)
    {
      error = createNetCDFVariable(fileID, "meshZSnso", NC_FLOAT, 3, instancesDimensionID, meshElementsDimensionID, evapoTranspirationAllLayersDimensionID);
    }
  
  if (!error && NULL != meshSnowH)
    {
      error = createNetCDFVariable(fileID, "meshSnowH", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshSnEqv)
    {
      error = createNetCDFVariable(fileID, "meshSnEqv", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshSnIce)
    {
      error = createNetCDFVariable(fileID, "meshSnIce", NC_FLOAT, 3, instancesDimensionID, meshElementsDimensionID, evapoTranspirationSnowLayersDimensionID);
    }
  
  if (!error && NULL != meshSnLiq)
    {
      error = createNetCDFVariable(fileID, "meshSnLiq", NC_FLOAT, 3, instancesDimensionID, meshElementsDimensionID, evapoTranspirationSnowLayersDimensionID);
    }
  
  if (!error && NULL != meshLfMass)
    {
      error = createNetCDFVariable(fileID, "meshLfMass", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshRtMass)
    {
      error = createNetCDFVariable(fileID, "meshRtMass", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshStMass)
    {
      error = createNetCDFVariable(fileID, "meshStMass", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshWood)
    {
      error = createNetCDFVariable(fileID, "meshWood", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshStblCp)
    {
      error = createNetCDFVariable(fileID, "meshStblCp", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshFastCp)
    {
      error = createNetCDFVariable(fileID, "meshFastCp", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshLai)
    {
      error = createNetCDFVariable(fileID, "meshLai", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshSai)
    {
      error = createNetCDFVariable(fileID, "meshSai", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshCm)
    {
      error = createNetCDFVariable(fileID, "meshCm", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshCh)
    {
      error = createNetCDFVariable(fileID, "meshCh", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshTauss)
    {
      error = createNetCDFVariable(fileID, "meshTauss", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshDeepRech)
    {
      error = createNetCDFVariable(fileID, "meshDeepRech", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshRech)
    {
      error = createNetCDFVariable(fileID, "meshRech", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshMeshNeighborsSurfacewaterFlowRate)
    {
      error = createNetCDFVariable(fileID, "meshMeshNeighborsSurfacewaterFlowRate", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshMeshNeighborsSurfacewaterCumulativeFlow)
    {
      error = createNetCDFVariable(fileID, "meshMeshNeighborsSurfacewaterCumulativeFlow", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshMeshNeighborsGroundwaterFlowRate)
    {
      error = createNetCDFVariable(fileID, "meshMeshNeighborsGroundwaterFlowRate", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshMeshNeighborsGroundwaterCumulativeFlow)
    {
      error = createNetCDFVariable(fileID, "meshMeshNeighborsGroundwaterCumulativeFlow", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshChannelNeighborsSurfacewaterFlowRate)
    {
      error = createNetCDFVariable(fileID, "meshChannelNeighborsSurfacewaterFlowRate", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshChannelNeighborsSurfacewaterCumulativeFlow)
    {
      error = createNetCDFVariable(fileID, "meshChannelNeighborsSurfacewaterCumulativeFlow", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshChannelNeighborsGroundwaterFlowRate)
    {
      error = createNetCDFVariable(fileID, "meshChannelNeighborsGroundwaterFlowRate", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != meshChannelNeighborsGroundwaterCumulativeFlow)
    {
      error = createNetCDFVariable(fileID, "meshChannelNeighborsGroundwaterCumulativeFlow", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                   meshChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelSurfacewaterDepth)
    {
      error = createNetCDFVariable(fileID, "channelSurfacewaterDepth", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelSurfacewaterError)
    {
      error = createNetCDFVariable(fileID, "channelSurfacewaterError", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelPrecipitation)
    {
      error = createNetCDFVariable(fileID, "channelPrecipitation", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelPrecipitationCumulative)
    {
      error = createNetCDFVariable(fileID, "channelPrecipitationCumulative", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelEvaporation)
    {
      error = createNetCDFVariable(fileID, "channelEvaporation", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelEvaporationCumulative)
    {
      error = createNetCDFVariable(fileID, "channelEvaporationCumulative", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelFIceOld)
    {
      error = createNetCDFVariable(fileID, "channelFIceOld", NC_FLOAT, 3, instancesDimensionID, channelElementsDimensionID,
                                   evapoTranspirationSnowLayersDimensionID);
    }
  
  if (!error && NULL != channelAlbOld)
    {
      error = createNetCDFVariable(fileID, "channelAlbOld", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelSnEqvO)
    {
      error = createNetCDFVariable(fileID, "channelSnEqvO", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelStc)
    {
      error = createNetCDFVariable(fileID, "channelStc", NC_FLOAT, 3, instancesDimensionID, channelElementsDimensionID,
                                   evapoTranspirationAllLayersDimensionID);
    }
  
  if (!error && NULL != channelTah)
    {
      error = createNetCDFVariable(fileID, "channelTah", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelEah)
    {
      error = createNetCDFVariable(fileID, "channelEah", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelFWet)
    {
      error = createNetCDFVariable(fileID, "channelFWet", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelCanLiq)
    {
      error = createNetCDFVariable(fileID, "channelCanLiq", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelCanIce)
    {
      error = createNetCDFVariable(fileID, "channelCanIce", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelTv)
    {
      error = createNetCDFVariable(fileID, "channelTv", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelTg)
    {
      error = createNetCDFVariable(fileID, "channelTg", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelISnow)
    {
      error = createNetCDFVariable(fileID, "channelISnow", NC_INT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelZSnso)
    {
      error = createNetCDFVariable(fileID, "channelZSnso", NC_FLOAT, 3, instancesDimensionID, channelElementsDimensionID,
                                   evapoTranspirationAllLayersDimensionID);
    }
  
  if (!error && NULL != channelSnowH)
    {
      error = createNetCDFVariable(fileID, "channelSnowH", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelSnEqv)
    {
      error = createNetCDFVariable(fileID, "channelSnEqv", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelSnIce)
    {
      error = createNetCDFVariable(fileID, "channelSnIce", NC_FLOAT, 3, instancesDimensionID, channelElementsDimensionID,
                                   evapoTranspirationSnowLayersDimensionID);
    }
  
  if (!error && NULL != channelSnLiq)
    {
      error = createNetCDFVariable(fileID, "channelSnLiq", NC_FLOAT, 3, instancesDimensionID, channelElementsDimensionID,
                                   evapoTranspirationSnowLayersDimensionID);
    }
  
  if (!error && NULL != channelLfMass)
    {
      error = createNetCDFVariable(fileID, "channelLfMass", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelRtMass)
    {
      error = createNetCDFVariable(fileID, "channelRtMass", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelStMass)
    {
      error = createNetCDFVariable(fileID, "channelStMass", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelWood)
    {
      error = createNetCDFVariable(fileID, "channelWood", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelStblCp)
    {
      error = createNetCDFVariable(fileID, "channelStblCp", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelFastCp)
    {
      error = createNetCDFVariable(fileID, "channelFastCp", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelLai)
    {
      error = createNetCDFVariable(fileID, "channelLai", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelSai)
    {
      error = createNetCDFVariable(fileID, "channelSai", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelCm)
    {
      error = createNetCDFVariable(fileID, "channelCm", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelCh)
    {
      error = createNetCDFVariable(fileID, "channelCh", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelTauss)
    {
      error = createNetCDFVariable(fileID, "channelTauss", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelDeepRech)
    {
      error = createNetCDFVariable(fileID, "channelDeepRech", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelRech)
    {
      error = createNetCDFVariable(fileID, "channelRech", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
    }
  
  if (!error && NULL != channelChannelNeighborsSurfacewaterFlowRate)
    {
      error = createNetCDFVariable(fileID, "channelChannelNeighborsSurfacewaterFlowRate", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelChannelNeighborsSurfacewaterCumulativeFlow)
    {
      error = createNetCDFVariable(fileID, "channelChannelNeighborsSurfacewaterCumulativeFlow", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelChannelNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelMeshNeighborsSurfacewaterFlowRate)
    {
      error = createNetCDFVariable(fileID, "channelMeshNeighborsSurfacewaterFlowRate", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelMeshNeighborsSurfacewaterCumulativeFlow)
    {
      error = createNetCDFVariable(fileID, "channelMeshNeighborsSurfacewaterCumulativeFlow", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelMeshNeighborsGroundwaterFlowRate)
    {
      error = createNetCDFVariable(fileID, "channelMeshNeighborsGroundwaterFlowRate", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelMeshNeighborsSizeDimensionID);
    }
  
  if (!error && NULL != channelMeshNeighborsGroundwaterCumulativeFlow)
    {
      error = createNetCDFVariable(fileID, "channelMeshNeighborsGroundwaterCumulativeFlow", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                   channelMeshNeighborsSizeDimensionID);
    }
  
  // Close the state file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCreateFiles: unable to close NetCDF state file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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

void FileManager::handleWriteFiles(const char* directory, bool writeGeometry, bool writeParameter, bool writeState)
{
  bool   error    = false;  // Error flag.
  int    ncErrorCode;       // Return value of NetCDF functions.
  int    fileID;            // ID of NetCDF file.
  bool   fileOpen = false;  // Whether fileID refers to an open file.
  size_t geometryInstance;  // Instance index for geometry file.
  size_t parameterInstance; // Instance index for parameter file.
  size_t stateInstance;     // Instance index for state file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleWriteFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // Open the geometry file.
  if (!error)
    {
      error = openNetCDFFile(directory, "geometry.nc", false, true, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances in the geometry file.
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "instances", &geometryInstance);
    }
  
  // Write geometry variables.
  if (writeGeometry)
    {
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "numberOfMeshNodes", geometryInstance, 0, 1, 1, &globalNumberOfMeshNodes);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "numberOfMeshElements", geometryInstance, 0, 1, 1, &globalNumberOfMeshElements);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "numberOfChannelNodes", geometryInstance, 0, 1, 1, &globalNumberOfChannelNodes);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "numberOfChannelElements", geometryInstance, 0, 1, 1, &globalNumberOfChannelElements);
        }
      
      if (!error && NULL != meshNodeX)
        {
          error = writeNetCDFVariable(fileID, "meshNodeX", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, meshNodeX);
        }
      
      if (!error && NULL != meshNodeY)
        {
          error = writeNetCDFVariable(fileID, "meshNodeY", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, meshNodeY);
        }
      
      if (!error && NULL != meshNodeZSurface)
        {
          error = writeNetCDFVariable(fileID, "meshNodeZSurface", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, meshNodeZSurface);
        }
      
      if (!error && NULL != meshElementVertices)
        {
          error = writeNetCDFVariable(fileID, "meshElementVertices", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshElementVertices);
        }
      
      if (!error && NULL != meshVertexX)
        {
          error = writeNetCDFVariable(fileID, "meshVertexX", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshVertexX);
        }
      
      if (!error && NULL != meshVertexY)
        {
          error = writeNetCDFVariable(fileID, "meshVertexY", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshVertexY);
        }
      
      if (!error && NULL != meshVertexZSurface)
        {
          error = writeNetCDFVariable(fileID, "meshVertexZSurface", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshVertexZSurface);
        }
      
      if (!error && NULL != meshElementX)
        {
          error = writeNetCDFVariable(fileID, "meshElementX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementX);
        }
      
      if (!error && NULL != meshElementY)
        {
          error = writeNetCDFVariable(fileID, "meshElementY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementY);
        }
      
      if (!error && NULL != meshElementZSurface)
        {
          error = writeNetCDFVariable(fileID, "meshElementZSurface", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshElementZSurface);
        }
      
      if (!error && NULL != meshElementSoilDepth)
        {
          error = writeNetCDFVariable(fileID, "meshElementSoilDepth", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshElementSoilDepth);
        }
      
      if (!error && NULL != meshElementZBedrock)
        {
          error = writeNetCDFVariable(fileID, "meshElementZBedrock", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshElementZBedrock);
        }
      
      if (!error && NULL != meshElementArea)
        {
          error = writeNetCDFVariable(fileID, "meshElementArea", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementArea);
        }
      
      if (!error && NULL != meshElementSlopeX)
        {
          error = writeNetCDFVariable(fileID, "meshElementSlopeX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementSlopeX);
        }
      
      if (!error && NULL != meshElementSlopeY)
        {
          error = writeNetCDFVariable(fileID, "meshElementSlopeY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementSlopeY);
        }
      
      if (!error && NULL != meshMeshNeighbors)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighbors", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighbors);
        }
      
      if (!error && NULL != meshMeshNeighborsChannelEdge)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsChannelEdge", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsChannelEdge);
        }
      
      if (!error && NULL != meshMeshNeighborsEdgeLength)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsEdgeLength", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsEdgeLength);
        }
      
      if (!error && NULL != meshMeshNeighborsEdgeNormalX)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsEdgeNormalX", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsEdgeNormalX);
        }
      
      if (!error && NULL != meshMeshNeighborsEdgeNormalY)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsEdgeNormalY", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsEdgeNormalY);
        }
      
      if (!error && NULL != meshChannelNeighbors)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighbors", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::channelNeighborsSize, meshChannelNeighbors);
        }
      
      if (!error && NULL != meshChannelNeighborsEdgeLength)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsEdgeLength", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::channelNeighborsSize, meshChannelNeighborsEdgeLength);
        }
      
      if (!error && NULL != channelNodeX)
        {
          error = writeNetCDFVariable(fileID, "channelNodeX", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, channelNodeX);
        }
      
      if (!error && NULL != channelNodeY)
        {
          error = writeNetCDFVariable(fileID, "channelNodeY", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, channelNodeY);
        }
      
      if (!error && NULL != channelNodeZBank)
        {
          error = writeNetCDFVariable(fileID, "channelNodeZBank", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, channelNodeZBank);
        }
      
      if (!error && NULL != channelElementVertices)
        {
          error = writeNetCDFVariable(fileID, "channelElementVertices", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                      ChannelElement::channelVerticesSize + 2, channelElementVertices);
        }
      
      if (!error && NULL != channelVertexX)
        {
          error = writeNetCDFVariable(fileID, "channelVertexX", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                      ChannelElement::channelVerticesSize, channelVertexX);
        }
      
      if (!error && NULL != channelVertexY)
        {
          error = writeNetCDFVariable(fileID, "channelVertexY", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                      ChannelElement::channelVerticesSize, channelVertexY);
        }
      
      if (!error && NULL != channelVertexZBank)
        {
          error = writeNetCDFVariable(fileID, "channelVertexZBank", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                      ChannelElement::channelVerticesSize, channelVertexZBank);
        }
      
      if (!error && NULL != channelElementX)
        {
          error = writeNetCDFVariable(fileID, "channelElementX", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelElementX);
        }
      
      if (!error && NULL != channelElementY)
        {
          error = writeNetCDFVariable(fileID, "channelElementY", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelElementY);
        }
      
      if (!error && NULL != channelElementZBank)
        {
          error = writeNetCDFVariable(fileID, "channelElementZBank", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelElementZBank);
        }
      
      if (!error && NULL != channelElementBankFullDepth)
        {
          error = writeNetCDFVariable(fileID, "channelElementBankFullDepth", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelElementBankFullDepth);
        }
      
      if (!error && NULL != channelElementZBed)
        {
          error = writeNetCDFVariable(fileID, "channelElementZBed", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelElementZBed);
        }
      
      if (!error && NULL != channelElementLength)
        {
          error = writeNetCDFVariable(fileID, "channelElementLength", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelElementLength);
        }
      
      if (!error && NULL != channelChannelNeighbors)
        {
          error = writeNetCDFVariable(fileID, "channelChannelNeighbors", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                      ChannelElement::channelNeighborsSize, channelChannelNeighbors);
        }
      
      if (!error && NULL != channelMeshNeighbors)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighbors", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                      ChannelElement::meshNeighborsSize, channelMeshNeighbors);
        }
      
      if (!error && NULL != channelMeshNeighborsEdgeLength)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsEdgeLength", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                      ChannelElement::meshNeighborsSize, channelMeshNeighborsEdgeLength);
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
              CkError("ERROR in FileManager::handleWriteFiles: not creating a new instance and no existing instance in NetCDF geometry file.\n");
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  // Close the geometry file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleWriteFiles: unable to close NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Open the parameter file.
  if (!error)
    {
      error = openNetCDFFile(directory, "parameter.nc", false, true, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances in the parameter file.
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "instances", &parameterInstance);
    }
  
  // Write parameter variables.
  if (writeParameter)
    {
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "geometryInstance", parameterInstance, 0, 1, 1, &geometryInstance);
        }
      
      if (!error && NULL != meshCatchment)
        {
          error = writeNetCDFVariable(fileID, "meshCatchment", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshCatchment);
        }
      
      if (!error && NULL != meshVegetationType)
        {
          error = writeNetCDFVariable(fileID, "meshVegetationType", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshVegetationType);
        }
      
      if (!error && NULL != meshSoilType)
        {
          error = writeNetCDFVariable(fileID, "meshSoilType", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSoilType);
        }
      
      if (!error && NULL != meshConductivity)
        {
          error = writeNetCDFVariable(fileID, "meshConductivity", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshConductivity);
        }
      
      if (!error && NULL != meshPorosity)
        {
          error = writeNetCDFVariable(fileID, "meshPorosity", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshPorosity);
        }
      
      if (!error && NULL != meshManningsN)
        {
          error = writeNetCDFVariable(fileID, "meshManningsN", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshManningsN);
        }
      
      if (!error && NULL != channelChannelType)
        {
          error = writeNetCDFVariable(fileID, "channelChannelType", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelChannelType);
        }
      
      if (!error && NULL != channelPermanentCode)
        {
          error = writeNetCDFVariable(fileID, "channelPermanentCode", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelPermanentCode);
        }
      
      if (!error && NULL != channelBaseWidth)
        {
          error = writeNetCDFVariable(fileID, "channelBaseWidth", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelBaseWidth);
        }
      
      if (!error && NULL != channelSideSlope)
        {
          error = writeNetCDFVariable(fileID, "channelSideSlope", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelSideSlope);
        }
      
      if (!error && NULL != channelBedConductivity)
        {
          error = writeNetCDFVariable(fileID, "channelBedConductivity", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelBedConductivity);
        }
      
      if (!error && NULL != channelBedThickness)
        {
          error = writeNetCDFVariable(fileID, "channelBedThickness", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelBedThickness);
        }
      
      if (!error && NULL != channelManningsN)
        {
          error = writeNetCDFVariable(fileID, "channelManningsN", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelManningsN);
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
              CkError("ERROR in FileManager::handleWriteFiles: not creating a new instance and no existing instance in NetCDF parameter file.\n");
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  // Close the parameter file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleWriteFiles: unable to close NetCDF parameter file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Open the state file.
  if (!error)
    {
      error = openNetCDFFile(directory, "state.nc", false, true, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances in the state file.
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "instances", &stateInstance);
    }
  
  // Write state variables.
  if (writeState)
    {
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "geometryInstance", stateInstance, 0, 1, 1, &geometryInstance);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "parameterInstance", stateInstance, 0, 1, 1, &parameterInstance);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "referenceDate", stateInstance, 0, 1, 1, &referenceDate);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "currentTime", stateInstance, 0, 1, 1, &currentTime);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "dt", stateInstance, 0, 1, 1, &dt);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "iteration", stateInstance, 0, 1, 1, &iteration);
        }
      
      if (!error && NULL != meshSurfacewaterDepth)
        {
          error = writeNetCDFVariable(fileID, "meshSurfacewaterDepth", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshSurfacewaterDepth);
        }
      
      if (!error && NULL != meshSurfacewaterError)
        {
          error = writeNetCDFVariable(fileID, "meshSurfacewaterError", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshSurfacewaterError);
        }
      
      if (!error && NULL != meshGroundwaterHead)
        {
          error = writeNetCDFVariable(fileID, "meshGroundwaterHead", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshGroundwaterHead);
        }
      
      if (!error && NULL != meshGroundwaterError)
        {
          error = writeNetCDFVariable(fileID, "meshGroundwaterError", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshGroundwaterError);
        }
      
      if (!error && NULL != meshPrecipitation)
        {
          error = writeNetCDFVariable(fileID, "meshPrecipitation", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshPrecipitation);
        }
      
      if (!error && NULL != meshPrecipitationCumulative)
        {
          error = writeNetCDFVariable(fileID, "meshPrecipitationCumulative", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshPrecipitationCumulative);
        }
      
      if (!error && NULL != meshEvaporation)
        {
          error = writeNetCDFVariable(fileID, "meshEvaporation", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshEvaporation);
        }
      
      if (!error && NULL != meshEvaporationCumulative)
        {
          error = writeNetCDFVariable(fileID, "meshEvaporationCumulative", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshEvaporationCumulative);
        }
      
      if (!error && NULL != meshSurfacewaterInfiltration)
        {
          error = writeNetCDFVariable(fileID, "meshSurfacewaterInfiltration", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshSurfacewaterInfiltration);
        }
      
      if (!error && NULL != meshGroundwaterRecharge)
        {
          error = writeNetCDFVariable(fileID, "meshGroundwaterRecharge", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshGroundwaterRecharge);
        }
      
      if (!error && NULL != meshFIceOld)
        {
          error = writeNetCDFVariable(fileID, "meshFIceOld", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, meshFIceOld);
        }
      
      if (!error && NULL != meshAlbOld)
        {
          error = writeNetCDFVariable(fileID, "meshAlbOld", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshAlbOld);
        }
      
      if (!error && NULL != meshSnEqvO)
        {
          error = writeNetCDFVariable(fileID, "meshSnEqvO", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSnEqvO);
        }
      
      if (!error && NULL != meshStc)
        {
          error = writeNetCDFVariable(fileID, "meshStc", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, meshStc);
        }
      
      if (!error && NULL != meshTah)
        {
          error = writeNetCDFVariable(fileID, "meshTah", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshTah);
        }
      
      if (!error && NULL != meshEah)
        {
          error = writeNetCDFVariable(fileID, "meshEah", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshEah);
        }
      
      if (!error && NULL != meshFWet)
        {
          error = writeNetCDFVariable(fileID, "meshFWet", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshFWet);
        }
      
      if (!error && NULL != meshCanLiq)
        {
          error = writeNetCDFVariable(fileID, "meshCanLiq", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshCanLiq);
        }
      
      if (!error && NULL != meshCanIce)
        {
          error = writeNetCDFVariable(fileID, "meshCanIce", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshCanIce);
        }
      
      if (!error && NULL != meshTv)
        {
          error = writeNetCDFVariable(fileID, "meshTv", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshTv);
        }
      
      if (!error && NULL != meshTg)
        {
          error = writeNetCDFVariable(fileID, "meshTg", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshTg);
        }
      
      if (!error && NULL != meshISnow)
        {
          error = writeNetCDFVariable(fileID, "meshISnow", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshISnow);
        }
      
      if (!error && NULL != meshZSnso)
        {
          error = writeNetCDFVariable(fileID, "meshZSnso", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, meshZSnso);
        }
      
      if (!error && NULL != meshSnowH)
        {
          error = writeNetCDFVariable(fileID, "meshSnowH", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSnowH);
        }
      
      if (!error && NULL != meshSnEqv)
        {
          error = writeNetCDFVariable(fileID, "meshSnEqv", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSnEqv);
        }
      
      if (!error && NULL != meshSnIce)
        {
          error = writeNetCDFVariable(fileID, "meshSnIce", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, meshSnIce);
        }
      
      if (!error && NULL != meshSnLiq)
        {
          error = writeNetCDFVariable(fileID, "meshSnLiq", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, meshSnLiq);
        }
      
      if (!error && NULL != meshLfMass)
        {
          error = writeNetCDFVariable(fileID, "meshLfMass", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshLfMass);
        }
      
      if (!error && NULL != meshRtMass)
        {
          error = writeNetCDFVariable(fileID, "meshRtMass", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshRtMass);
        }
      
      if (!error && NULL != meshStMass)
        {
          error = writeNetCDFVariable(fileID, "meshStMass", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshStMass);
        }
      
      if (!error && NULL != meshWood)
        {
          error = writeNetCDFVariable(fileID, "meshWood", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshWood);
        }
      
      if (!error && NULL != meshStblCp)
        {
          error = writeNetCDFVariable(fileID, "meshStblCp", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshStblCp);
        }
      
      if (!error && NULL != meshFastCp)
        {
          error = writeNetCDFVariable(fileID, "meshFastCp", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshFastCp);
        }
      
      if (!error && NULL != meshLai)
        {
          error = writeNetCDFVariable(fileID, "meshLai", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshLai);
        }
      
      if (!error && NULL != meshSai)
        {
          error = writeNetCDFVariable(fileID, "meshSai", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSai);
        }
      
      if (!error && NULL != meshCm)
        {
          error = writeNetCDFVariable(fileID, "meshCm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshCm);
        }
      
      if (!error && NULL != meshCh)
        {
          error = writeNetCDFVariable(fileID, "meshCh", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshCh);
        }
      
      if (!error && NULL != meshTauss)
        {
          error = writeNetCDFVariable(fileID, "meshTauss", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshTauss);
        }
      
      if (!error && NULL != meshDeepRech)
        {
          error = writeNetCDFVariable(fileID, "meshDeepRech", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshDeepRech);
        }
      
      if (!error && NULL != meshRech)
        {
          error = writeNetCDFVariable(fileID, "meshRech", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshRech);
        }
      
      if (!error && NULL != meshMeshNeighborsSurfacewaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsSurfacewaterFlowRate", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshMeshNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsSurfacewaterCumulativeFlow", geometryInstance, localMeshElementStart,
                                      localNumberOfMeshElements, MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != meshMeshNeighborsGroundwaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsGroundwaterFlowRate", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshMeshNeighborsGroundwaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsGroundwaterCumulativeFlow", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshChannelNeighborsSurfacewaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsSurfacewaterFlowRate", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::channelNeighborsSize, meshChannelNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshChannelNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsSurfacewaterCumulativeFlow", geometryInstance, localMeshElementStart,
                                      localNumberOfMeshElements, MeshElement::channelNeighborsSize, meshChannelNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != meshChannelNeighborsGroundwaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsGroundwaterFlowRate", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::channelNeighborsSize, meshChannelNeighborsGroundwaterFlowRate);
        }
      
      if (!error && NULL != meshChannelNeighborsGroundwaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsGroundwaterCumulativeFlow", geometryInstance, localMeshElementStart,
                                      localNumberOfMeshElements, MeshElement::channelNeighborsSize, meshChannelNeighborsGroundwaterCumulativeFlow);
        }
      
      if (!error && NULL != channelSurfacewaterDepth)
        {
          error = writeNetCDFVariable(fileID, "channelSurfacewaterDepth", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelSurfacewaterDepth);
        }
      
      if (!error && NULL != channelSurfacewaterError)
        {
          error = writeNetCDFVariable(fileID, "channelSurfacewaterError", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelSurfacewaterError);
        }
      
      if (!error && NULL != channelPrecipitation)
        {
          error = writeNetCDFVariable(fileID, "channelPrecipitation", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelPrecipitation);
        }
      
      if (!error && NULL != channelPrecipitationCumulative)
        {
          error = writeNetCDFVariable(fileID, "channelPrecipitationCumulative", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelPrecipitationCumulative);
        }
      
      if (!error && NULL != channelEvaporation)
        {
          error = writeNetCDFVariable(fileID, "channelEvaporation", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelEvaporation);
        }
      
      if (!error && NULL != channelEvaporationCumulative)
        {
          error = writeNetCDFVariable(fileID, "channelEvaporationCumulative", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelEvaporationCumulative);
        }
      
      if (!error && NULL != channelFIceOld)
        {
          error = writeNetCDFVariable(fileID, "channelFIceOld", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, channelFIceOld);
        }
      
      if (!error && NULL != channelAlbOld)
        {
          error = writeNetCDFVariable(fileID, "channelAlbOld", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelAlbOld);
        }
      
      if (!error && NULL != channelSnEqvO)
        {
          error = writeNetCDFVariable(fileID, "channelSnEqvO", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelSnEqvO);
        }
      
      if (!error && NULL != channelStc)
        {
          error = writeNetCDFVariable(fileID, "channelStc", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, channelStc);
        }
      
      if (!error && NULL != channelTah)
        {
          error = writeNetCDFVariable(fileID, "channelTah", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelTah);
        }
      
      if (!error && NULL != channelEah)
        {
          error = writeNetCDFVariable(fileID, "channelEah", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelEah);
        }
      
      if (!error && NULL != channelFWet)
        {
          error = writeNetCDFVariable(fileID, "channelFWet", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelFWet);
        }
      
      if (!error && NULL != channelCanLiq)
        {
          error = writeNetCDFVariable(fileID, "channelCanLiq", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelCanLiq);
        }
      
      if (!error && NULL != channelCanIce)
        {
          error = writeNetCDFVariable(fileID, "channelCanIce", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelCanIce);
        }
      
      if (!error && NULL != channelTv)
        {
          error = writeNetCDFVariable(fileID, "channelTv", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelTv);
        }
      
      if (!error && NULL != channelTg)
        {
          error = writeNetCDFVariable(fileID, "channelTg", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelTg);
        }
      
      if (!error && NULL != channelISnow)
        {
          error = writeNetCDFVariable(fileID, "channelISnow", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelISnow);
        }
      
      if (!error && NULL != channelZSnso)
        {
          error = writeNetCDFVariable(fileID, "channelZSnso", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS, channelZSnso);
        }
      
      if (!error && NULL != channelSnowH)
        {
          error = writeNetCDFVariable(fileID, "channelSnowH", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelSnowH);
        }
      
      if (!error && NULL != channelSnEqv)
        {
          error = writeNetCDFVariable(fileID, "channelSnEqv", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelSnEqv);
        }
      
      if (!error && NULL != channelSnIce)
        {
          error = writeNetCDFVariable(fileID, "channelSnIce", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, channelSnIce);
        }
      
      if (!error && NULL != channelSnLiq)
        {
          error = writeNetCDFVariable(fileID, "channelSnLiq", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                      EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS, channelSnLiq);
        }
      
      if (!error && NULL != channelLfMass)
        {
          error = writeNetCDFVariable(fileID, "channelLfMass", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelLfMass);
        }
      
      if (!error && NULL != channelRtMass)
        {
          error = writeNetCDFVariable(fileID, "channelRtMass", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelRtMass);
        }
      
      if (!error && NULL != channelStMass)
        {
          error = writeNetCDFVariable(fileID, "channelStMass", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelStMass);
        }
      
      if (!error && NULL != channelWood)
        {
          error = writeNetCDFVariable(fileID, "channelWood", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelWood);
        }
      
      if (!error && NULL != channelStblCp)
        {
          error = writeNetCDFVariable(fileID, "channelStblCp", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelStblCp);
        }
      
      if (!error && NULL != channelFastCp)
        {
          error = writeNetCDFVariable(fileID, "channelFastCp", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelFastCp);
        }
      
      if (!error && NULL != channelLai)
        {
          error = writeNetCDFVariable(fileID, "channelLai", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelLai);
        }
      
      if (!error && NULL != channelSai)
        {
          error = writeNetCDFVariable(fileID, "channelSai", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelSai);
        }
      
      if (!error && NULL != channelCm)
        {
          error = writeNetCDFVariable(fileID, "channelCm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelCm);
        }
      
      if (!error && NULL != channelCh)
        {
          error = writeNetCDFVariable(fileID, "channelCh", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelCh);
        }
      
      if (!error && NULL != channelTauss)
        {
          error = writeNetCDFVariable(fileID, "channelTauss", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelTauss);
        }
      
      if (!error && NULL != channelDeepRech)
        {
          error = writeNetCDFVariable(fileID, "channelDeepRech", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelDeepRech);
        }
      
      if (!error && NULL != channelRech)
        {
          error = writeNetCDFVariable(fileID, "channelRech", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelRech);
        }
      
      if (!error && NULL != channelChannelNeighborsSurfacewaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "channelChannelNeighborsSurfacewaterFlowRate", geometryInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::channelNeighborsSize, channelChannelNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != channelChannelNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "channelChannelNeighborsSurfacewaterCumulativeFlow", geometryInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::channelNeighborsSize, channelChannelNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != channelMeshNeighborsSurfacewaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsSurfacewaterFlowRate", geometryInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::meshNeighborsSize, channelMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != channelMeshNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsSurfacewaterCumulativeFlow", geometryInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::meshNeighborsSize, channelMeshNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != channelMeshNeighborsGroundwaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsGroundwaterFlowRate", geometryInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::meshNeighborsSize, channelMeshNeighborsGroundwaterFlowRate);
        }
      
      if (!error && NULL != channelMeshNeighborsGroundwaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsGroundwaterCumulativeFlow", geometryInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::meshNeighborsSize, channelMeshNeighborsGroundwaterCumulativeFlow);
        }
    } // End if (writeState).
  
  // Close the state file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleWriteFiles: unable to close NetCDF state file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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

void FileManager::handleReadForcingData(const char* directory, CProxy_MeshElement meshProxy, CProxy_ChannelElement channelProxy, double referenceDateNew,
                                        double currentTimeNew)
{
  bool   error    = false;  // Error flag.
  int    ii;                // Loop counter.
  int    ncErrorCode;       // Return value of NetCDF functions.
  int    fileID;            // ID of NetCDF file.
  bool   fileOpen = false;  // Whether fileID refers to an open file.
  int    variableID;        // ID of variable in NetCDF file.
  size_t numberOfInstances; // Size of instance dimension.
  size_t instance;          // Instance index for file.
  double currentDate;       // The date and time represented by referenceDateNew and currentTimeNew as a Julian date.
  float* jultime   = NULL;  // Used to read Julian date.
  float* t2        = NULL;  // Used to read air temperature at 2m height forcing.
  float* vegFra    = NULL;  // Used to read vegetation fraction forcing.
  float* maxVegFra = NULL;  // Used to read maximum vegetation fraction forcing.
  float* psfc      = NULL;  // Used to read surface pressure forcing.
  float* u         = NULL;  // Used to read wind speed U component forcing.
  float* v         = NULL;  // Used to read wind speed V component forcing.
  float* qVapor    = NULL;  // Used to read water vapor mixing ratio forcing.
  float* qCloud    = NULL;  // Used to read cloud water mixing ratio forcing.
  float* swDown    = NULL;  // Used to read downward shortwave flux forcing.
  float* gLw       = NULL;  // Used to read downward longwave flux forcing.
  float* tPrec     = NULL;  // Used to read total precipitation forcing.
  float* tslb      = NULL;  // Used to read soil temperature at the deepest layer forcing.
  float* pblh      = NULL;  // Used to read planetary boundary layer height forcing.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleReadForcingData: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // Open the forcing file.
  if (!error)
    {
      error = openNetCDFFile(directory, "forcing.nc", false, false, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of instances.
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "Time", &numberOfInstances);
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!error && !(0 < numberOfInstances))
    {
      CkError("ERROR in FileManager::handleReadForcingData: No forcing data in NetCDF forcing file.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // Get the Julian dates for all instances.
  if (!error)
    {
      // Get the variable ID.
      ncErrorCode = nc_inq_varid(fileID, "JULTIME", &variableID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleReadForcingData: unable to get variable JULTIME in NetCDF forcing file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      jultime = new float[numberOfInstances];
      
      // Get the variable data.
      ncErrorCode = nc_get_var(fileID, variableID, jultime);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleReadForcingData: unable to read variable JULTIME in NetCDF forcing file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // It is an error if the first forcing data instance is after the current date and time.
  if (!error)
    {
      currentDate = referenceDateNew + currentTimeNew / (24.0 * 3600.0);
      
      if (!(jultime[0] <= currentDate))
        {
          CkError("ERROR in FileManager::handleReadForcingData: Current time is before first forcing data instance in NetCDF forcing file.\n");
          error = true;
        }
    }
  
  // Search for the last instance that is before or equal to the current date and time.
  // FIXME to improve efficiency make this a binary search.
  if (!error)
    {
      instance = 0;
      
      while (instance + 1 < numberOfInstances && jultime[instance + 1] <= currentDate)
        {
          instance++;
        }
      
      // instance is now the index of the forcing data instance we will use.
      // Record the dates of the current forcing data and next forcing data and that the forcing data is initialized.
      forcingDataDate = jultime[instance];
      
      if (instance + 1 < numberOfInstances)
        {
          nextForcingDataDate = jultime[instance + 1];
        }
      else
        {
          CkError("WARNING in FileManager::handleReadForcingData: Using the last forcing data instance in NetCDF forcing file.  No new forcing data will be "
                  "loaded after this no matter how long the simulation runs.\n");
          
          nextForcingDataDate = INFINITY;
        }
      
      forcingDataInitialized = true;
    }
  
  deleteArrayIfNonNull(&jultime);
  
  // Read forcing variables.
  if (0 < localNumberOfMeshElements)
    {
      if (!error)
        {
          error = readNetCDFVariable(fileID, "VEGFRA", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &vegFra);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "MAXVEGFRA", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &maxVegFra);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "T2", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &t2);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "PSFC", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &psfc);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "U", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &u);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "V", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &v);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "QVAPOR", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &qVapor);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "QCLOUD", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &qCloud);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "SWDOWN", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &swDown);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "GLW", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &gLw);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "TPREC", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &tPrec);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "TSLB", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &tslb);
        }
      
      if (!error)
        {
          error = readNetCDFVariable(fileID, "PBLH", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &pblh);
        }
      
      for (ii = localMeshElementStart; ii < localMeshElementStart + localNumberOfMeshElements; ii++)
        {
          meshProxy[ii].forcingDataMessage(20.0f, vegFra[ii - localMeshElementStart], maxVegFra[ii - localMeshElementStart], t2[ii - localMeshElementStart],
                                           psfc[ii - localMeshElementStart], psfc[ii - localMeshElementStart] - 120.0f, u[ii - localMeshElementStart],
                                           v[ii - localMeshElementStart], qVapor[ii - localMeshElementStart], qCloud[ii - localMeshElementStart],
                                           swDown[ii - localMeshElementStart], gLw[ii - localMeshElementStart], tPrec[ii - localMeshElementStart],
                                           tslb[ii - localMeshElementStart],  pblh[ii - localMeshElementStart]);
        }
      
      deleteArrayIfNonNull(&vegFra);
      deleteArrayIfNonNull(&maxVegFra);
      deleteArrayIfNonNull(&t2);
      deleteArrayIfNonNull(&psfc);
      deleteArrayIfNonNull(&u);
      deleteArrayIfNonNull(&v);
      deleteArrayIfNonNull(&qVapor);
      deleteArrayIfNonNull(&qCloud);
      deleteArrayIfNonNull(&swDown);
      deleteArrayIfNonNull(&gLw);
      deleteArrayIfNonNull(&tPrec);
      deleteArrayIfNonNull(&tslb);
      deleteArrayIfNonNull(&pblh);
    }
  
  if (0 < localNumberOfChannelElements)
    {
      // FIXME read forcing data for channels.

      for (ii = localChannelElementStart; ii < localChannelElementStart + localNumberOfChannelElements; ii++)
        {
          channelProxy[ii].forcingDataMessage();
        }
      
      deleteArrayIfNonNull(&vegFra);
      deleteArrayIfNonNull(&maxVegFra);
      deleteArrayIfNonNull(&t2);
      deleteArrayIfNonNull(&psfc);
      deleteArrayIfNonNull(&u);
      deleteArrayIfNonNull(&v);
      deleteArrayIfNonNull(&qVapor);
      deleteArrayIfNonNull(&qCloud);
      deleteArrayIfNonNull(&swDown);
      deleteArrayIfNonNull(&gLw);
      deleteArrayIfNonNull(&tPrec);
      deleteArrayIfNonNull(&tslb);
      deleteArrayIfNonNull(&pblh);
    }
  
  // Close the forcing file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleReadForcingData: unable to close NetCDF forcing file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (error)
    {
      CkExit();
    }
}

void FileManager::handleUpdateState(double referenceDateNew, double currentTimeNew, double dtNew, size_t iterationNew)
{
  int ii; // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtNew))
    {
      CkError("ERROR in FileManager::updateState: dtNew must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  referenceDate = referenceDateNew;
  currentTime   = currentTimeNew;
  dt            = dtNew;
  iteration     = iterationNew;
  
  if (NULL != meshElementUpdated)
    {
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshElementUpdated[ii] = false;
        }
    }

  if (NULL != channelElementUpdated)
    {
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelElementUpdated[ii] = false;
        }
    }
}

bool FileManager::allElementsUpdated()
{
  int  ii;             // Loop counter.
  bool updated = true; // Flag to record whether we have found an unupdated element.
  
  if (NULL != meshElementUpdated)
    {
      for (ii = 0; updated && ii < localNumberOfMeshElements; ii++)
        {
          updated = meshElementUpdated[ii];
        }
    }
  
  if (NULL != channelElementUpdated)
    {
      for (ii = 0; updated && ii < localNumberOfChannelElements; ii++)
        {
          updated = channelElementUpdated[ii];
        }
    }
  
  return updated;
}

void FileManager::handleMeshStateMessage(int element, double surfacewaterDepth, double surfacewaterError, double groundwaterHead, double groundwaterError,
                                         double precipitation, double precipitationCumulative, double evaporation, double evaporationCumulative,
                                         double surfacewaterInfiltration, double groundwaterRecharge, EvapoTranspirationStateStruct evapoTranspirationState,
                                         double* meshNeighborsSurfacewaterFlowRate, double* meshNeighborsSurfacewaterCumulativeFlow,
                                         double* meshNeighborsGroundwaterFlowRate, double* meshNeighborsGroundwaterCumulativeFlow,
                                         double* channelNeighborsSurfacewaterFlowRate, double* channelNeighborsSurfacewaterCumulativeFlow,
                                         double* channelNeighborsGroundwaterFlowRate, double* channelNeighborsGroundwaterCumulativeFlow)
{
  int ii; // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(localMeshElementStart <= element && element < localMeshElementStart + localNumberOfMeshElements))
    {
      CkError("ERROR in FileManager::handleMeshStateMessage, element %d: element data not owned by this local branch.\n", element);
      CkExit();
    }

  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in FileManager::handleMeshStateMessage, element %d: surfacewaterDepth must be greater than or equal to zero.\n", element);
      CkExit();
    }
  // FIXME error check other inputs
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  meshSurfacewaterDepth[element - localMeshElementStart]        = surfacewaterDepth;
  meshSurfacewaterError[element - localMeshElementStart]        = surfacewaterError;
  meshGroundwaterHead[element - localMeshElementStart]          = groundwaterHead;
  meshGroundwaterError[element - localMeshElementStart]         = groundwaterError;
  meshPrecipitation[element - localMeshElementStart]            = precipitation;
  meshPrecipitationCumulative[element - localMeshElementStart]  = precipitationCumulative;
  meshEvaporation[element - localMeshElementStart]              = evaporation;
  meshEvaporationCumulative[element - localMeshElementStart]    = evaporationCumulative;
  meshSurfacewaterInfiltration[element - localMeshElementStart] = surfacewaterInfiltration;
  meshGroundwaterRecharge[element - localMeshElementStart]      = groundwaterRecharge;
  
  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
    {
      meshFIceOld[element - localMeshElementStart][ii] = evapoTranspirationState.fIceOld[ii];
    }

  meshAlbOld[element - localMeshElementStart] = evapoTranspirationState.albOld;
  meshSnEqvO[element - localMeshElementStart] = evapoTranspirationState.snEqvO;
  
  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
    {
      meshStc[element - localMeshElementStart][ii] = evapoTranspirationState.stc[ii];
    }

  meshTah[element - localMeshElementStart]    = evapoTranspirationState.tah;
  meshEah[element - localMeshElementStart]    = evapoTranspirationState.eah;
  meshFWet[element - localMeshElementStart]   = evapoTranspirationState.fWet;
  meshCanLiq[element - localMeshElementStart] = evapoTranspirationState.canLiq;
  meshCanIce[element - localMeshElementStart] = evapoTranspirationState.canIce;
  meshTv[element - localMeshElementStart]     = evapoTranspirationState.tv;
  meshTg[element - localMeshElementStart]     = evapoTranspirationState.tg;
  meshISnow[element - localMeshElementStart]  = evapoTranspirationState.iSnow;
  
  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
    {
      meshZSnso[element - localMeshElementStart][ii] = evapoTranspirationState.zSnso[ii];
    }
  
  meshSnowH[element - localMeshElementStart] = evapoTranspirationState.snowH;
  meshSnEqv[element - localMeshElementStart] = evapoTranspirationState.snEqv;

  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
    {
      meshSnIce[element - localMeshElementStart][ii] = evapoTranspirationState.snIce[ii];
      meshSnLiq[element - localMeshElementStart][ii] = evapoTranspirationState.snLiq[ii];
    }

  meshLfMass[element - localMeshElementStart]   = evapoTranspirationState.lfMass;
  meshRtMass[element - localMeshElementStart]   = evapoTranspirationState.rtMass;
  meshStMass[element - localMeshElementStart]   = evapoTranspirationState.stMass;
  meshWood[element - localMeshElementStart]     = evapoTranspirationState.wood;
  meshStblCp[element - localMeshElementStart]   = evapoTranspirationState.stblCp;
  meshFastCp[element - localMeshElementStart]   = evapoTranspirationState.fastCp;
  meshLai[element - localMeshElementStart]      = evapoTranspirationState.lai;
  meshSai[element - localMeshElementStart]      = evapoTranspirationState.sai;
  meshCm[element - localMeshElementStart]       = evapoTranspirationState.cm;
  meshCh[element - localMeshElementStart]       = evapoTranspirationState.ch;
  meshTauss[element - localMeshElementStart]    = evapoTranspirationState.tauss;
  meshDeepRech[element - localMeshElementStart] = evapoTranspirationState.deepRech;
  meshRech[element - localMeshElementStart]     = evapoTranspirationState.rech;
  
  for (ii = 0; ii < MeshElement::meshNeighborsSize; ii++)
    {
      meshMeshNeighborsSurfacewaterFlowRate[element - localMeshElementStart][ii]       = meshNeighborsSurfacewaterFlowRate[ii];
      meshMeshNeighborsSurfacewaterCumulativeFlow[element - localMeshElementStart][ii] = meshNeighborsSurfacewaterCumulativeFlow[ii];
      meshMeshNeighborsGroundwaterFlowRate[element - localMeshElementStart][ii]        = meshNeighborsGroundwaterFlowRate[ii];
      meshMeshNeighborsGroundwaterCumulativeFlow[element - localMeshElementStart][ii]  = meshNeighborsGroundwaterCumulativeFlow[ii];
    }
  
  for (ii = 0; ii < MeshElement::channelNeighborsSize; ii++)
    {
      meshChannelNeighborsSurfacewaterFlowRate[element - localMeshElementStart][ii]       = channelNeighborsSurfacewaterFlowRate[ii];
      meshChannelNeighborsSurfacewaterCumulativeFlow[element - localMeshElementStart][ii] = channelNeighborsSurfacewaterCumulativeFlow[ii];
      meshChannelNeighborsGroundwaterFlowRate[element - localMeshElementStart][ii]        = channelNeighborsGroundwaterFlowRate[ii];
      meshChannelNeighborsGroundwaterCumulativeFlow[element - localMeshElementStart][ii]  = channelNeighborsGroundwaterCumulativeFlow[ii];
    }
  
  meshElementUpdated[element - localMeshElementStart] = true;
}

void FileManager::handleChannelStateMessage(int element, double surfacewaterDepth, double surfacewaterError, double precipitation,
                                            double precipitationCumulative, double evaporation, double evaporationCumulative,
                                            EvapoTranspirationStateStruct evapoTranspirationState,
                                            double* channelNeighborsSurfacewaterFlowRate, double* channelNeighborsSurfacewaterCumulativeFlow,
                                            double* meshNeighborsSurfacewaterFlowRate, double* meshNeighborsSurfacewaterCumulativeFlow,
                                            double* meshNeighborsGroundwaterFlowRate, double* meshNeighborsGroundwaterCumulativeFlow)
{
  int ii; // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(localChannelElementStart <= element && element < localChannelElementStart + localNumberOfChannelElements))
    {
      CkError("ERROR in FileManager::handleChannelStateMessage, element %d: element data not owned by this local branch.\n", element);
      CkExit();
    }

  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in FileManager::handleChannelStateMessage, element %d: surfacewaterDepth must be greater than or equal to zero.\n", element);
      CkExit();
    }
  // FIXME error check other inputs
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  channelSurfacewaterDepth[element - localChannelElementStart]       = surfacewaterDepth;
  channelSurfacewaterError[element - localChannelElementStart]       = surfacewaterError;
  channelPrecipitation[element - localChannelElementStart]           = precipitation;
  channelPrecipitationCumulative[element - localChannelElementStart] = precipitationCumulative;
  channelEvaporation[element - localChannelElementStart]             = evaporation;
  channelEvaporationCumulative[element - localChannelElementStart]   = evaporationCumulative;
  
  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
    {
      channelFIceOld[element - localChannelElementStart][ii] = evapoTranspirationState.fIceOld[ii];
    }

  channelAlbOld[element - localChannelElementStart] = evapoTranspirationState.albOld;
  channelSnEqvO[element - localChannelElementStart] = evapoTranspirationState.snEqvO;
  
  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
    {
      channelStc[element - localChannelElementStart][ii] = evapoTranspirationState.stc[ii];
    }

  channelTah[element - localChannelElementStart]    = evapoTranspirationState.tah;
  channelEah[element - localChannelElementStart]    = evapoTranspirationState.eah;
  channelFWet[element - localChannelElementStart]   = evapoTranspirationState.fWet;
  channelCanLiq[element - localChannelElementStart] = evapoTranspirationState.canLiq;
  channelCanIce[element - localChannelElementStart] = evapoTranspirationState.canIce;
  channelTv[element - localChannelElementStart]     = evapoTranspirationState.tv;
  channelTg[element - localChannelElementStart]     = evapoTranspirationState.tg;
  channelISnow[element - localChannelElementStart]  = evapoTranspirationState.iSnow;
  
  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
    {
      channelZSnso[element - localChannelElementStart][ii] = evapoTranspirationState.zSnso[ii];
    }
  
  channelSnowH[element - localChannelElementStart] = evapoTranspirationState.snowH;
  channelSnEqv[element - localChannelElementStart] = evapoTranspirationState.snEqv;

  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
    {
      channelSnIce[element - localChannelElementStart][ii] = evapoTranspirationState.snIce[ii];
      channelSnLiq[element - localChannelElementStart][ii] = evapoTranspirationState.snLiq[ii];
    }

  channelLfMass[element - localChannelElementStart]   = evapoTranspirationState.lfMass;
  channelRtMass[element - localChannelElementStart]   = evapoTranspirationState.rtMass;
  channelStMass[element - localChannelElementStart]   = evapoTranspirationState.stMass;
  channelWood[element - localChannelElementStart]     = evapoTranspirationState.wood;
  channelStblCp[element - localChannelElementStart]   = evapoTranspirationState.stblCp;
  channelFastCp[element - localChannelElementStart]   = evapoTranspirationState.fastCp;
  channelLai[element - localChannelElementStart]      = evapoTranspirationState.lai;
  channelSai[element - localChannelElementStart]      = evapoTranspirationState.sai;
  channelCm[element - localChannelElementStart]       = evapoTranspirationState.cm;
  channelCh[element - localChannelElementStart]       = evapoTranspirationState.ch;
  channelTauss[element - localChannelElementStart]    = evapoTranspirationState.tauss;
  channelDeepRech[element - localChannelElementStart] = evapoTranspirationState.deepRech;
  channelRech[element - localChannelElementStart]     = evapoTranspirationState.rech;
  
  for (ii = 0; ii < ChannelElement::channelNeighborsSize; ii++)
    {
      channelChannelNeighborsSurfacewaterFlowRate[element - localChannelElementStart][ii]       = channelNeighborsSurfacewaterFlowRate[ii];
      channelChannelNeighborsSurfacewaterCumulativeFlow[element - localChannelElementStart][ii] = channelNeighborsSurfacewaterCumulativeFlow[ii];
    }
  
  for (ii = 0; ii < ChannelElement::meshNeighborsSize; ii++)
    {
      channelMeshNeighborsSurfacewaterFlowRate[element - localChannelElementStart][ii]       = meshNeighborsSurfacewaterFlowRate[ii];
      channelMeshNeighborsSurfacewaterCumulativeFlow[element - localChannelElementStart][ii] = meshNeighborsSurfacewaterCumulativeFlow[ii];
      channelMeshNeighborsGroundwaterFlowRate[element - localChannelElementStart][ii]        = meshNeighborsGroundwaterFlowRate[ii];
      channelMeshNeighborsGroundwaterCumulativeFlow[element - localChannelElementStart][ii]  = meshNeighborsGroundwaterCumulativeFlow[ii];
    }
  
  channelElementUpdated[element - localChannelElementStart] = true;
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "file_manager.def.h"
#pragma GCC diagnostic warning "-Wsign-compare"
#pragma GCC diagnostic warning "-Wunused-variable"
