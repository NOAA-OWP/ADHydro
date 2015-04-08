#include "file_manager.h"
#include "adhydro.h"
#include "all.h"
#include <netcdf_par.h>

// Temporary variable to help SDAG parse C++ code.
bool FileManager::needToCreateFiles;

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
  meshTranspiration                                 = NULL;
  meshTranspirationCumulative                       = NULL;
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
  channelReachCode                                  = NULL;
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
  channelChannelNeighborsDownstream                 = NULL;
  channelChannelNeighborsSurfacewaterFlowRate       = NULL;
  channelChannelNeighborsSurfacewaterCumulativeFlow = NULL;
  channelMeshNeighbors                              = NULL;
  channelMeshNeighborsEdgeLength                    = NULL;
  channelMeshNeighborsSurfacewaterFlowRate          = NULL;
  channelMeshNeighborsSurfacewaterCumulativeFlow    = NULL;
  channelMeshNeighborsGroundwaterFlowRate           = NULL;
  channelMeshNeighborsGroundwaterCumulativeFlow     = NULL;
  channelPrunedSize                                 = 0;
  numberOfChannelPruned                             = 0;
  channelPruned                                     = NULL;
  referenceDate                                     = gregorianToJulian(2010, 1, 1, 0, 0, 0.0);
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
  deleteArrayIfNonNull(&meshTranspiration);
  deleteArrayIfNonNull(&meshTranspirationCumulative);
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
  deleteArrayIfNonNull(&channelReachCode);
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
  deleteArrayIfNonNull(&channelChannelNeighborsDownstream);
  deleteArrayIfNonNull(&channelChannelNeighborsSurfacewaterFlowRate);
  deleteArrayIfNonNull(&channelChannelNeighborsSurfacewaterCumulativeFlow);
  deleteArrayIfNonNull(&channelMeshNeighbors);
  deleteArrayIfNonNull(&channelMeshNeighborsEdgeLength);
  deleteArrayIfNonNull(&channelMeshNeighborsSurfacewaterFlowRate);
  deleteArrayIfNonNull(&channelMeshNeighborsSurfacewaterCumulativeFlow);
  deleteArrayIfNonNull(&channelMeshNeighborsGroundwaterFlowRate);
  deleteArrayIfNonNull(&channelMeshNeighborsGroundwaterCumulativeFlow);
  deleteArrayIfNonNull(&channelPruned);
  deleteArrayIfNonNull(&meshVertexUpdated);
  deleteArrayIfNonNull(&channelVertexUpdated);
  deleteArrayIfNonNull(&meshElementUpdated);
  deleteArrayIfNonNull(&channelElementUpdated);
}

void FileManager::getMeshVertexDataMessage(int requester, int node)
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

  thisProxy[requester].meshVertexDataMessage(node, x, y, zSurface);
}

void FileManager::getChannelVertexDataMessage(int requester, int node)
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

  thisProxy[requester].channelVertexDataMessage(node, x, y, zBank);
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
  int    index;               // Used to read node numbers.
  double xCoordinate;         // Used to read coordinates from the files.
  double yCoordinate;         // Used to read coordinates from the files.
  double zCoordinate;         // Used to read coordinates from the files.
  
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
          CkError("ERROR in FileManager::readNodeAndZFiles: could not open z file %s.\n", nameString);
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
        }
      else
        {
          globalNumberOfChannelNodes = globalNumberOfNodes;
          
          localStartAndNumber(&localChannelNodeStart, &localNumberOfChannelNodes, globalNumberOfChannelNodes);

          channelNodeX     = new double[localNumberOfChannelNodes];
          channelNodeY     = new double[localNumberOfChannelNodes];
          channelNodeZBank = new double[localNumberOfChannelNodes];
        }
    }
  
  // Read nodes.
  for (ii = 0; !error && ii < globalNumberOfNodes; ii++)
    {
      // Read node file.
      numScanned = fscanf(nodeFile, "%d %lf %lf %*d", &index, &xCoordinate, &yCoordinate);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: unable to read entry %d from node file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: invalid node number in node file.  %d should be %d.\n", index, ii);
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
              CkError("ERROR in FileManager::readNodeAndZFiles: unable to read entry %d from z file.\n", ii);
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
          if (readMesh)
            {
              if (localMeshNodeStart <= index && index < localMeshNodeStart + localNumberOfMeshNodes)
                {
                  meshNodeX[       index - localMeshNodeStart] = xCoordinate;
                  meshNodeY[       index - localMeshNodeStart] = yCoordinate;
                  meshNodeZSurface[index - localMeshNodeStart] = zCoordinate;
                }
            }
          else
            {
              if (localChannelNodeStart <= index && index < localChannelNodeStart + localNumberOfChannelNodes)
                {
                  channelNodeX[    index - localChannelNodeStart] = xCoordinate;
                  channelNodeY[    index - localChannelNodeStart] = yCoordinate;
                  channelNodeZBank[index - localChannelNodeStart] = zCoordinate;
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
  deleteArrayIfNonNull(&nameString);
  
  return error;
}

void FileManager::handleInitializeFromASCIIFiles(const char* directory, const char* fileBasename)
{
  bool               error         = false;    // Error flag.
  int                ii, jj, kk;               // Loop counters.
  char*              nameString    = NULL;     // Temporary string for file names.
  size_t             nameStringSize;           // Size of buffer allocated for nameString.
  size_t             numPrinted;               // Used to check that snprintf printed the correct number of characters.
  size_t             numScanned;               // Used to check that fscanf scanned the correct number of inputs.
  FILE*              eleFile       = NULL;     // The ele file to read from.
  FILE*              neighFile     = NULL;     // The neigh file to read from.
  FILE*              edgeFile      = NULL;     // The edge file to read from.
  FILE*              landCoverFile = NULL;     // The land cover file to read from.
  FILE*              soilFile      = NULL;     // The soil file to read from.
  FILE*              chanEleFile   = NULL;     // The chan.ele file to read from.
  FILE*              chanPruneFile = NULL;     // The chan.prune file to read from.
  int                numberOfEdges;            // The number of edges to read.
  int                dimension;                // Used to check the dimensions in the files.
  int                numberOfAttributes;       // Used to check the number of attributes in the files.
  int                boundary;                 // Used to check the number of boundary markers in the files.
  int                numberCheck;              // Used to check numbers that are error checked but otherwise unused.
  int                index;                    // Used to read element and edge numbers.
  int                vertex0;                  // Used to read vertices from the files.
  int                vertex1;                  // Used to read vertices from the files.
  int                vertex2;                  // Used to read vertices from the files.
  int                catchment;                // Used to read catchments from the files.
  int                vegetationType;           // Used to read vegetation type from the files.
  int                numberOfSoilLayers;       // Used to read the number of soil layers from the files.
  int                soilTypeReader;           // Used to read soil type from the files.
  int                soilType;                 // Used to store soil type of the top layer.
  double             soilDepthReader;          // Used to read each soil layers thickness.
  double             soilDepth;                // Used to store the sum of each soil layers thickness.
  int                neighbor0;                // Used to read neighbors from the files.
  int                neighbor1;                // Used to read neighbors from the files.
  int                neighbor2;                // Used to read neighbors from the files.
  int                type;                     // Used to read channel type from the files.  Read as int because reading as ChannelTypeEnum gives a warning.
  long long          reachCode0;               // Used to read channel reach code from the files.
  long long          reachCode1;               // Used to read channel reach code from the files.
  double             length;                   // Used to read channel length from the files.
  double             topWidth;                 // Used to read top width from the files.
  double             bankFullDepth;            // Used to read bank full depth from the files.
  int                numberOfVertices;         // The number of vertices for a particular channel element.
  int                numberOfChannelNeighbors; // The number of channel neighbors for a particular channel element.
  int                numberOfMeshNeighbors;    // The number of mesh neighbors for a particular channel element.
  int                downstream;               // Used to read whether a channel neighbor is downstream.
  LongLongArrayPair* tempChannelPruned;        // For reallocing channelPruned.
  
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
      error = readNodeAndZFiles(directory, fileBasename, true);
    }

  if (!error)
    {
      // Allocate space for file name strings.
      nameStringSize = strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".chan.prune") + 1; // The longest file extension is .chan.prune.
      nameString     = new char[nameStringSize];                                                           // +1 for null terminating character.

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
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open ele file %s.\n", nameString);
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
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open neigh file %s.\n", nameString);
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

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/%s.landCover", directory, fileBasename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".landCover") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: incorrect return value of snprintf when generating landCover file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted,
                  strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".landCover"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Open file.
  if (!error)
    {
      landCoverFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != landCoverFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open landCover file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Read header.
  if (!error)
    {
      numScanned = fscanf(landCoverFile, "%d", &numberCheck);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(1 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read header from landCover file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid header in landCover file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/%s.soilType", directory, fileBasename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".soilType") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: incorrect return value of snprintf when generating soil file name %s.  %d should be "
                  "equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".soilType"),
                  nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Open file.
  if (!error)
    {
      soilFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != soilFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open soil file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Read header.
  if (!error)
    {
      numScanned = fscanf(soilFile, "%d", &numberCheck);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(1 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read header from soil file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid header in soil file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Calculate local start and number and allocate arrays.
  if (!error)
    {
      localStartAndNumber(&localMeshElementStart, &localNumberOfMeshElements, globalNumberOfMeshElements);

      meshElementVertices            = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      meshElementSoilDepth           = new double[localNumberOfMeshElements];
      meshCatchment                  = new int[localNumberOfMeshElements];
      meshVegetationType             = new int[localNumberOfMeshElements];
      meshSoilType                   = new int[localNumberOfMeshElements];
      meshMeshNeighbors              = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      meshMeshNeighborsChannelEdge   = new bool[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      meshChannelNeighbors           = new int[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
      meshChannelNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
    }
  
  // Read mesh elements.
  for (ii = 0; !error && ii < globalNumberOfMeshElements; ii++)
    {
      // Read ele file.
      numScanned = fscanf(eleFile, "%d %d %d %d %d", &index, &vertex0, &vertex1, &vertex2, &catchment);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(5 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from ele file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in ele file.  %d should be %d.\n", index, ii);
          error = true;
        }

      if (!(0 <= vertex0 && vertex0 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex0);
          error = true;
        }

      if (!(0 <= vertex1 && vertex1 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex1);
          error = true;
        }

      if (!(0 <= vertex2 && vertex2 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex2);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Read neigh file.
      numScanned = fscanf(neighFile, "%d %d %d %d", &numberCheck, &neighbor0, &neighbor1, &neighbor2);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from neigh file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in neigh file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }

      if (!(isBoundary(neighbor0) || (0 <= neighbor0 && neighbor0 < globalNumberOfMeshElements)))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor0);
          error = true;
        }

      if (!(isBoundary(neighbor1) || (0 <= neighbor1 && neighbor1 < globalNumberOfMeshElements)))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor1);
          error = true;
        }

      if (!(isBoundary(neighbor2) || (0 <= neighbor2 && neighbor2 < globalNumberOfMeshElements)))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor2);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Read landCover file.
      numScanned = fscanf(landCoverFile, "%d %d", &numberCheck, &vegetationType);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from landCover file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in landCover file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }

      if (!(1 <= vegetationType && 27 >= vegetationType))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid vegetation type number %d in landCover file.\n", index,
                  vegetationType);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Read soil file.
      numScanned = fscanf(soilFile, "%d %d", &numberCheck, &numberOfSoilLayers);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from soil file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in soil file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }

      if (!(0 <= numberOfSoilLayers))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid number of soil layers %d in soil file.\n", index,
                  numberOfSoilLayers);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      if (!error)
        {
          soilType  = -1;  // If there are no soil layers soilType will be left as -1 as a flag for no soil data, and soilDepth will be left as zero.
          soilDepth = 0.0; // That element will have to get soil type and depth from a neighbor in meshMassage.

          // Loop through the soil layers of this element.
          for (jj = 0; jj < numberOfSoilLayers; jj++)
            {
              numScanned = fscanf(soilFile, "%d,%lf", &soilTypeReader, &soilDepthReader);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(2 == numScanned))
                {
                  CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d soil layer %d from soil file.\n", ii, jj);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
              if (!((1 <= soilTypeReader && 19 >= soilTypeReader) || -1 == soilTypeReader))
                {
                  CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid soil type %s in soil file.\n", index,
                          soilTypeReader);
                  error = true;
                }

              if (!(0.0 <= soilDepthReader))
                {
                  CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: soilDepthReader must be greater than or equal to zero.\n",
                          index);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

              // Only save the first valid soil type.
              if (-1 == soilType)
                {
                  soilType = soilTypeReader;
                }

              // Save the sum of soil thicknesses.
              soilDepth += soilDepthReader;
            }
        }

      // Save values.
      if (!error && localMeshElementStart <= index && index < localMeshElementStart + localNumberOfMeshElements)
        {
          meshElementVertices[ index - localMeshElementStart][0] = vertex0;
          meshElementVertices[ index - localMeshElementStart][1] = vertex1;
          meshElementVertices[ index - localMeshElementStart][2] = vertex2;
          meshElementSoilDepth[index - localMeshElementStart]    = soilDepth;
          meshCatchment[       index - localMeshElementStart]    = catchment - 2;  // The .ele file stores catchment number plus two because zero and one are
                                                                                   // used by triangle.
          meshVegetationType[  index - localMeshElementStart]    = vegetationType;
          meshSoilType[        index - localMeshElementStart]    = soilType;
          meshMeshNeighbors[   index - localMeshElementStart][0] = neighbor0;
          meshMeshNeighbors[   index - localMeshElementStart][1] = neighbor1;
          meshMeshNeighbors[   index - localMeshElementStart][2] = neighbor2;

          // Channel edges and neighbors are filled in later when we read the edge and channel files.  Initialize them here to no channels.
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              meshMeshNeighborsChannelEdge[index - localMeshElementStart][jj] = false;
            }

          for (jj = 0; jj < MeshElement::channelNeighborsSize; jj++)
            {
              meshChannelNeighbors[          index - localMeshElementStart][jj] = NOFLOW;
              meshChannelNeighborsEdgeLength[index - localMeshElementStart][jj] = 0.0;
            }
        }
    } // End read mesh elements.
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/%s.edge", directory, fileBasename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".edge") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: incorrect return value of snprintf when generating edge file name %s.  %d should be "
                  "equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".edge"),
                  nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      edgeFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != edgeFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open edge file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(edgeFile, "%d %d", &numberOfEdges, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read header from edge file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfEdges && 1 == boundary))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid header in edge file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Read mesh edges.
  for (ii = 0; !error && ii < numberOfEdges; ii++)
    {
      numScanned = fscanf(edgeFile, "%d %d %d %d", &index, &vertex0, &vertex1, &boundary);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from edge file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid edge number in edge file.  %d should be %d.\n", index, ii);
          error = true;
        }
      
      if (!(0 <= vertex0 && vertex0 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: edge %d: invalid vertex number %d in edge file.\n", index, vertex0);
          error = true;
        }
      
      if (!(0 <= vertex1 && vertex1 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: edge %d: invalid vertex number %d in edge file.\n", index, vertex1);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // If the edge is a channel edge find it in the mesh and mark it as such.  A boundary code of two or more indicates a channel edge.
      if (!error && 2 <= boundary)
        {
          for (jj = 0; jj < localNumberOfMeshElements; jj++)
            {
              if ((vertex0 == meshElementVertices[jj][1] && vertex1 == meshElementVertices[jj][2]) || (vertex0 == meshElementVertices[jj][2] && vertex1 == meshElementVertices[jj][1]))
                {
                  meshMeshNeighborsChannelEdge[jj][0] = true;
                }
              else if ((vertex0 == meshElementVertices[jj][0] && vertex1 == meshElementVertices[jj][2]) || (vertex0 == meshElementVertices[jj][2] && vertex1 == meshElementVertices[jj][0]))
                {
                  meshMeshNeighborsChannelEdge[jj][1] = true;
                }
              else if ((vertex0 == meshElementVertices[jj][0] && vertex1 == meshElementVertices[jj][1]) || (vertex0 == meshElementVertices[jj][1] && vertex1 == meshElementVertices[jj][0]))
                {
                  meshMeshNeighborsChannelEdge[jj][2] = true;
                }
            }
        }
    } // End read mesh edges.
  
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
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open chan.ele file %s.\n", nameString);
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
      
      channelElementVertices            = new int[localNumberOfChannelElements][XDMF_SIZE];
      channelElementBankFullDepth       = new double[localNumberOfChannelElements];
      channelElementLength              = new double[localNumberOfChannelElements];
      channelChannelType                = new ChannelTypeEnum[localNumberOfChannelElements];
      channelReachCode                  = new long long[localNumberOfChannelElements];
      channelBaseWidth                  = new double[localNumberOfChannelElements];
      channelSideSlope                  = new double[localNumberOfChannelElements];
      channelChannelNeighbors           = new int[localNumberOfChannelElements][ChannelElement::channelNeighborsSize];
      channelChannelNeighborsDownstream = new bool[localNumberOfChannelElements][ChannelElement::channelNeighborsSize];
      channelMeshNeighbors              = new int[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
      channelMeshNeighborsEdgeLength    = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
    }
  
  // Read channel elements.
  for (ii = 0; !error && ii < globalNumberOfChannelElements; ii++)
    {
      numScanned = fscanf(chanEleFile, "%d %d %lld %lf %lf %lf %d %d %d", &index, &type, &reachCode1, &length, &topWidth, &bankFullDepth, &numberOfVertices,
                          &numberOfChannelNeighbors, &numberOfMeshNeighbors);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(9 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in chan.ele file.  %d should be %d.\n", index, ii);
          error = true;
        }
      
      if (!(STREAM == type || WATERBODY == type || ICEMASS == type))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: channel type must be a valid enum value in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0 < reachCode1))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: reach code must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0.0 < length))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: length must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0.0 < topWidth))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: topWidth must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0.0 < bankFullDepth))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: bankFullDepth must be greater than zero in chan.ele file.\n", index);
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
      if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
        {
          // Use polyline (code 2) for STREAM and polygon (code 3) for WATERBODY and ICEMASS
          channelElementVertices[     index - localChannelElementStart][0] = (STREAM == type) ? 2 : 3;
          channelElementVertices[     index - localChannelElementStart][1] = ChannelElement::channelVerticesSize;
          channelElementBankFullDepth[index - localChannelElementStart]    = bankFullDepth;
          channelElementLength[       index - localChannelElementStart]    = length;
          channelChannelType[         index - localChannelElementStart]    = (ChannelTypeEnum)type;
          channelReachCode[           index - localChannelElementStart]    = reachCode1;
          // FIXME This assumes rectangular channels.  Calculate some other way?
          channelBaseWidth[           index - localChannelElementStart]    = topWidth;
          channelSideSlope[           index - localChannelElementStart]    = 0.0;
        }
      
      // Read vertices.
      for (jj = 0; !error && jj < numberOfVertices; jj++)
        {
          numScanned = fscanf(chanEleFile, "%d", &vertex0);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(1 == numScanned))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", index);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 <= vertex0 && vertex0 < globalNumberOfChannelNodes))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: invalid vertex number %d in ele file.\n", index, vertex0);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

          // Save values.
          if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
            {
              // +2 to skip over XDMF metadata.
              channelElementVertices[index - localChannelElementStart][jj + 2] = vertex0;
            }
        }
      
      // Fill in unused vertices by repeating the last used vertex.
      if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
        {
          for (jj = numberOfVertices; jj < ChannelElement::channelVerticesSize; jj++)
            {
              // +2 to skip over XDMF metadata.
              channelElementVertices[index - localChannelElementStart][jj + 2] =
                  channelElementVertices[index - localChannelElementStart][(numberOfVertices - 1) + 2];
            }
        }
      
      // Read channel neighbors.
      for (jj = 0; !error && jj < numberOfChannelNeighbors; jj++)
        {
          numScanned = fscanf(chanEleFile, "%d %d", &neighbor0, &downstream);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(2 == numScanned))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", index);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(isBoundary(neighbor0) || (0 <= neighbor0 && neighbor0 < globalNumberOfChannelElements)))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: invalid channel neighbor number %d in chan.ele file.\n", index,
                      neighbor0);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

          // Save values.
          if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
            {
              channelChannelNeighbors[          index - localChannelElementStart][jj] = neighbor0;
              channelChannelNeighborsDownstream[index - localChannelElementStart][jj] = downstream;
            }
        }
      
      // Fill in unused channel neighbors with NOFLOW.
      if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
        {
          for (jj = numberOfChannelNeighbors; jj < ChannelElement::channelNeighborsSize; jj++)
            {
              channelChannelNeighbors[          index - localChannelElementStart][jj] = NOFLOW;
              channelChannelNeighborsDownstream[index - localChannelElementStart][jj] = false;
            }
        }
      
      // Read mesh neighbors.
      for (jj = 0; !error && jj < numberOfMeshNeighbors; jj++)
        {
          fscanf(chanEleFile, "%d %lf", &neighbor0, &length);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(2 == numScanned))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", index);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(isBoundary(neighbor0) || (0 <= neighbor0 && neighbor0 < globalNumberOfMeshElements)))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: invalid mesh neighbor number %d in chan.ele file.\n", index, neighbor0);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

          // Save channel mesh neighbor.
          if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
            {
              channelMeshNeighbors[          index - localChannelElementStart][jj] = neighbor0;
              channelMeshNeighborsEdgeLength[index - localChannelElementStart][jj] = length;
            }
          
          // Save mesh channel neighbor.
          if (!error && localMeshElementStart <= neighbor0 && neighbor0 < localMeshElementStart + localNumberOfMeshElements)
            {
              kk = 0;

              while (kk < MeshElement::channelNeighborsSize && NOFLOW != meshChannelNeighbors[neighbor0 - localMeshElementStart][kk])
                {
                  kk++;
                }

              if (kk < MeshElement::channelNeighborsSize)
                {
                  meshChannelNeighbors[          neighbor0 - localMeshElementStart][kk] = index;
                  meshChannelNeighborsEdgeLength[neighbor0 - localMeshElementStart][kk] = length;
                }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
              else
                {
                  CkError("ERROR in handleInitializeFromASCIIFiles: mesh element %d: number of channel neighbors exceeds maximum number %d.\n", neighbor0,
                          MeshElement::channelNeighborsSize);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
            }
        }
      
      // Fill in unused mesh neighbors with NOFLOW.
      if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
        {
          for (jj = numberOfMeshNeighbors; jj < ChannelElement::meshNeighborsSize; jj++)
            {
              channelMeshNeighbors[          index - localChannelElementStart][jj] = NOFLOW;
              channelMeshNeighborsEdgeLength[index - localChannelElementStart][jj] = 0.0;
            }
        }
    } // End read channel elements.

  // channelPruned is only used for mesh massage so only read it in if you are doing mesh massage.
  if (ADHydro::doMeshMassage)
    {
      // Create file name.
      if (!error)
        {
          numPrinted = snprintf(nameString, nameStringSize, "%s/%s.chan.prune", directory, fileBasename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".chan.prune") == numPrinted && numPrinted < nameStringSize))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: incorrect return value of snprintf when generating chan.prune file name %s.  %d "
                      "should be equal to %d and less than %d.\n", nameString, numPrinted,
                      strlen(directory) + strlen("/") + strlen(fileBasename) + strlen(".chan.prune"), nameStringSize);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      // Open file.
      if (!error)
        {
          chanPruneFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NULL != chanPruneFile))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open chan.prune file %s.\n", nameString);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      // Allocate array.
      if (!error)
        {
          channelPrunedSize     = 2;
          numberOfChannelPruned = 0;
          channelPruned         = new LongLongArrayPair[channelPrunedSize];
        }

      if (!error)
        {
          do
            {
              numScanned = fscanf(chanPruneFile, "%lld %lld", &reachCode0, &reachCode1);

              if (2 == numScanned)
                {
                  // Realloc channelPruned array if necessary.
                  if (channelPrunedSize == numberOfChannelPruned)
                    {
                      channelPrunedSize *= 2;
                      tempChannelPruned  = new LongLongArrayPair[channelPrunedSize];

                      for (ii = 0; ii < numberOfChannelPruned; ii++)
                        {
                          tempChannelPruned[ii][0] = channelPruned[ii][0];
                          tempChannelPruned[ii][1] = channelPruned[ii][1];
                        }

                      delete[] channelPruned;

                      channelPruned = tempChannelPruned;
                    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                  CkAssert(channelPrunedSize > numberOfChannelPruned);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                  channelPruned[numberOfChannelPruned][0] = reachCode0;
                  channelPruned[numberOfChannelPruned][1] = reachCode1;
                  numberOfChannelPruned++;
                }
            }
          while (2 == numScanned);
        }
    }

  // Close the files.
  if (NULL != eleFile)
    {
      fclose(eleFile);
    }

  if (NULL != neighFile)
    {
      fclose(neighFile);
    }

  if (NULL != landCoverFile)
    {
      fclose(landCoverFile);
    }

  if (NULL != soilFile)
    {
      fclose(soilFile);
    }

  if (NULL != edgeFile)
    {
      fclose(edgeFile);
    }

  if (NULL != chanEleFile)
    {
      fclose(chanEleFile);
    }

  if (NULL != chanPruneFile)
    {
      fclose(chanPruneFile);
    }

  // Delete nameString.
  deleteArrayIfNonNull(&nameString);
  
  // Have to call evapoTranspirationInit once on each Pe. This is a convenient place to do that.
  if (!error)
    {
      error = evapoTranspirationInit(directory);
    }

  if (error)
    {
      CkExit();
    }
  
  // FIXME below are hardcoded values that we want to find real data sources for
  if (NULL == channelBedConductivity)
    {
      channelBedConductivity = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelBedConductivity[ii] = 0.000555;
        }
    }

  if (NULL == channelBedThickness)
    {
      channelBedThickness = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelBedThickness[ii] = 1.0;
        }
    }

  if (NULL == channelManningsN)
    {
      channelManningsN = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelManningsN[ii] = 0.038;
        }
    }
}

bool FileManager::openNetCDFFile(const char* directory, const char* filename, bool create, bool write, int* fileID)
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

bool FileManager::createNetCDFDimension(int fileID, const char* dimensionName, size_t dimensionSize, int* dimensionID)
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

bool FileManager::readNetCDFDimensionSize(int fileID, const char* dimensionName, size_t* dimensionSize)
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

bool FileManager::createNetCDFVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID0, int dimensionID1,
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

template <typename T> bool FileManager::readNetCDFVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart,
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

bool FileManager::writeNetCDFVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart, size_t numberOfNodesElements,
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
                                     channelElementVerticesSize, XDMF_SIZE, true, 0, false, (int**)&channelElementVertices);

          // If ChannelElement::channelVerticesSize is not the same value as in the file we need to reset the number of vertices in the XDMF metadata.
          if (channelElementVerticesSize != XDMF_SIZE)
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
          error = readNetCDFVariable(fileID, "channelChannelNeighborsDownstream", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, ChannelElement::channelNeighborsSize, false, false, false,
                                     (bool**)&channelChannelNeighborsDownstream);
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
          error = readNetCDFVariable(fileID, "channelReachCode", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     (long long)0, false, &channelReachCode);
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

      // Do not read precipitation, evaporation, transpiration, and infiltration values.  Instantaneous values will be calculated before they are used.
      // Cumulative values are reset to zero at initialization.
      
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
  int                                                             ii, jj;                             // Loop counters.
  bool                                                            needToGetMeshVertexData    = false; // Whether we need to get any vertex data.
  bool                                                            needToGetChannelVertexData = false; // Whether we need to get any vertex data.
  int                                                             node;                               // The node number of a vertex.
  std::map< int, std::vector< std::pair< int, int > > >::iterator it;                                 // For accessing the elements of meshNodeLocation.
  
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
              node                      = meshElementVertices[ii][jj];

              if (localMeshNodeStart <= node && node < localMeshNodeStart + localNumberOfMeshNodes)
                {
                  // The node belongs to me, no need to send a message.
                  if (NULL != meshNodeX)
                      {
                        meshVertexX[ii][jj] = meshNodeX[node - localMeshNodeStart];
                      }

                    if (NULL != meshNodeY)
                      {
                        meshVertexY[ii][jj] = meshNodeY[node - localMeshNodeStart];
                      }

                    if (NULL != meshNodeZSurface)
                      {
                        meshVertexZSurface[ii][jj] = meshNodeZSurface[node - localMeshNodeStart];
                      }
                    
                    meshVertexUpdated[ii][jj] = true;
                }
              else
                {
                  it = meshNodeLocation.find(node);

                  if (it == meshNodeLocation.end()) // If key does not exist
                    {
                      // Send message.
                      thisProxy[home(node, globalNumberOfMeshNodes)].getMeshVertexDataMessage(CkMyPe(), node);

                      // Save mapping from node number to element and vertex numbers.
                      meshNodeLocation.insert(std::pair< int, std::vector< std::pair< int, int > > >
                                              (node, std::vector< std::pair< int, int > >(1, std::pair< int, int >(ii + localMeshElementStart, jj))));
                    }
                  else // The key does exist
                    {
                      // A message requesting this node has already been sent.  Save mapping from node number to element and vertex numbers.
                      it->second.push_back(std::pair< int, int >(ii + localMeshElementStart, jj));
                    }
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
              node                         = channelElementVertices[ii][jj + 2]; // +2 to skip over XDMF metadata.

              if (localChannelNodeStart <= node && node < localChannelNodeStart + localNumberOfChannelNodes)
                {
                  // The node belongs to me, no need to send a message.
                  if (NULL != channelNodeX)
                    {
                      channelVertexX[ii][jj] = channelNodeX[node - localChannelNodeStart];
                    }

                  if (NULL != channelNodeY)
                    {
                      channelVertexY[ii][jj]  = channelNodeY[node - localChannelNodeStart];
                    }

                  if (NULL != channelNodeZBank)
                    {
                      channelVertexZBank[ii][jj] = channelNodeZBank[node - localChannelNodeStart];
                    }

                  channelVertexUpdated[ii][jj] = true;
                }
              else
                {
                  it = channelNodeLocation.find(node);

                  if (it == channelNodeLocation.end()) // If key does not exist
                    {
                      // Send message.
                      thisProxy[home(node, globalNumberOfChannelNodes)].getChannelVertexDataMessage(CkMyPe(), node);

                      // Save mapping from node number to element and vertex numbers.
                      channelNodeLocation.insert(std::pair< int, std::vector< std::pair< int, int > > >
                                                 (node, std::vector< std::pair< int, int > >(1, std::pair< int, int >(ii + localChannelElementStart, jj))));
                    }
                  else // The key does exist
                    {
                      // A message requesting this node has already been sent.  Save mapping from node number to element and vertex numbers.
                      it->second.push_back(std::pair< int, int >(ii + localChannelElementStart, jj));
                    }
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

void FileManager::handleMeshVertexDataMessage(int node, double x, double y, double zSurface)
{
  unsigned int                                                    ii;      // Loop counter.
  int                                                             element; // Element that needs to be filled in with the node coordinates.
  int                                                             vertex;  // Vertex that needs to be filled in with the node coordinates.
  std::map< int, std::vector< std::pair< int, int > > >::iterator it;      // For accessing the elements of meshNodeLocation.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= node && node < globalNumberOfMeshNodes))
    {
      CkError("ERROR in FileManager::handleMeshVertexDataMessage: node index not valid.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  it = meshNodeLocation.find(node); 

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(it != meshNodeLocation.end());
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // it->second refers to the vector member of the map which stores the elements and vertices where the node coordinates need to be filled in.
  for (ii = 0; ii < it->second.size(); ii++)
    {
      element = it->second[ii].first;
      vertex  = it->second[ii].second;

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
  
  meshNodeLocation.erase(it);
}

void FileManager::handleChannelVertexDataMessage(int node, double x, double y, double zBank)
{
  unsigned int                                                    ii;      // Loop counter.
  int                                                             element; // Element that needs to be filled in with the node coordinates.
  int                                                             vertex;  // Vertex that needs to be filled in with the node coordinates.
  std::map< int, std::vector< std::pair< int, int > > >::iterator it;      // For accessing the elements of channelNodeLocation.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= node && node < globalNumberOfChannelNodes))
    {
      CkError("ERROR in FileManager::handleChannelVertexDataMessage: node index not valid.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  it = channelNodeLocation.find(node); 

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(it != channelNodeLocation.end());
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // it->second refers to the vector member of the map which stores the elements and vertices where the node coordinates need to be filled in.
  for (ii = 0; ii < it->second.size(); ii++)
    {
      element = it->second[ii].first;
      vertex  = it->second[ii].second;

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
  
  channelNodeLocation.erase(it);
}

void FileManager::meshMassageVegetationAndSoilType()
{
  int    ii, jj;                                                // Loop counters.
  bool   done;                                                  // Loop end condition.
  int    oldNumberOfRemainingElementsWithInvalidVegetationType; // Number of elements with invalid vegetation type from the last pass.
  int    newNumberOfRemainingElementsWithInvalidVegetationType; // Number of elements with invalid vegetation type from the current pass.
  int    oldNumberOfRemainingElementsWithInvalidSoilType;       // Number of elements with invalid soil type from the last pass.
  int    newNumberOfRemainingElementsWithInvalidSoilType;       // Number of elements with invalid soil type from the current pass.
  int    neighbor;                                              // A neighbor of an element.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(1 == CkNumPes()))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshMassageVegetationAndSoilType is not implemented for distributed operation.  It can only be run on one "
              "processor.\n");
      CkExit();
    }
  
  if (!(NULL != meshElementZSurface))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshElementZSurface must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshElementSoilDepth))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshElementSoilDepth must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshElementZBedrock))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshElementZBedrock must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshVegetationType))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshVegetationType must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshSoilType))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshSoilType must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshMeshNeighbors))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshMeshNeighbors must not be NULL.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // For mesh elements that have vegetation type of Water Bodies (16) or Snow or Ice (24) or no soil type (-1) or soil type of WATER (14) or OTHER(land-ice)
  // (16) get values from a neighbor.
  done                                                  = false;
  oldNumberOfRemainingElementsWithInvalidVegetationType = globalNumberOfMeshElements;
  oldNumberOfRemainingElementsWithInvalidSoilType       = globalNumberOfMeshElements;
  
  while (!done)
    {
      newNumberOfRemainingElementsWithInvalidVegetationType = 0;
      newNumberOfRemainingElementsWithInvalidSoilType       = 0;

      for (ii = 0; ii < globalNumberOfMeshElements; ii++)
        {
          // If element ii has invalid vegetation or soil type try to get it from a neighbor.  This might not succeed if no neighbor has a valid type.
          // The outer most loop handles this case by repeating this fix until the number of elements with invalid type goes to zero or stops going down.
          jj = 0;

          while (jj < MeshElement::meshNeighborsSize && (16 == meshVegetationType[ii] || 24 == meshVegetationType[ii] ||
                                                         -1 == meshSoilType[ii] || 14 == meshSoilType[ii] || 16 == meshSoilType[ii]))
            {
              neighbor = meshMeshNeighbors[ii][jj];

              if (!isBoundary(neighbor))
                {
                  if ( (16 == meshVegetationType[ii]       || 24 == meshVegetationType[ii]) &&
                      !(16 == meshVegetationType[neighbor] || 24 == meshVegetationType[neighbor]))
                    {
                      if (4 <= ADHydro::verbosityLevel)
                        {
                          CkError("WARNING in FileManager::meshMassageVegetationAndSoilType: getting vegetation type for element %d from neighbor %d.\n", ii,
                                  neighbor);
                        }

                      meshVegetationType[ii] = meshVegetationType[neighbor];
                    }
                  
                  if ( (-1 == meshSoilType[ii]       || 14 == meshSoilType[ii]       || 16 == meshSoilType[ii]) &&
                      !(-1 == meshSoilType[neighbor] || 14 == meshSoilType[neighbor] || 16 == meshSoilType[neighbor]))
                    {
                      if (4 <= ADHydro::verbosityLevel)
                        {
                          CkError("WARNING in FileManager::meshMassageVegetationAndSoilType: getting soil type and depth for element %d from neighbor %d.\n",
                                  ii, neighbor);
                        }

                      meshSoilType[ii]         = meshSoilType[neighbor];
                      meshElementSoilDepth[ii] = meshElementSoilDepth[neighbor];
                      meshElementZBedrock[ii]  = meshElementZSurface[ii] - meshElementSoilDepth[ii];
                    }
                }

              jj++;
            }

          if (16 == meshVegetationType[ii] || 24 == meshVegetationType[ii])
            {
              if (4 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::meshMassageVegetationAndSoilType: mesh element %d has invalid vegetation type and no neighbor with valid "
                          "vegetation type.\n", ii);
                }

              newNumberOfRemainingElementsWithInvalidVegetationType++;
            }

          if (-1 == meshSoilType[ii] || 14 == meshSoilType[ii] || 16 == meshSoilType[ii])
            {
              if (4 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::meshMassageVegetationAndSoilType: mesh element %d has invalid soil type and no neighbor with valid soil "
                          "type.\n", ii);
                }

              newNumberOfRemainingElementsWithInvalidSoilType++;
            }
        }
      
      if (1 <= ADHydro::verbosityLevel)
        {
          CkPrintf("Remaining elements with invalid vegetation type: %d.  Remaining elements with invalid soil type: %d.\n",
                   newNumberOfRemainingElementsWithInvalidVegetationType, newNumberOfRemainingElementsWithInvalidSoilType);
        }
      
      done = ((0 == newNumberOfRemainingElementsWithInvalidSoilType ||
               newNumberOfRemainingElementsWithInvalidSoilType >= oldNumberOfRemainingElementsWithInvalidSoilType) &&
              (0 == newNumberOfRemainingElementsWithInvalidVegetationType ||
               newNumberOfRemainingElementsWithInvalidVegetationType >= oldNumberOfRemainingElementsWithInvalidVegetationType));
      
      oldNumberOfRemainingElementsWithInvalidVegetationType = newNumberOfRemainingElementsWithInvalidVegetationType;
      oldNumberOfRemainingElementsWithInvalidSoilType       = newNumberOfRemainingElementsWithInvalidSoilType;
    }
}

int FileManager::findDownstreamElement(int element)
{
  int ii;                         // Loop counter.
  int downstreamElement = NOFLOW; // An element or boundary condition code downstream of element.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(0 <= element && element < globalNumberOfChannelElements && NULL != channelChannelNeighbors && NULL != channelChannelNeighborsDownstream);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  for (ii = 0; NOFLOW == downstreamElement && ii < ChannelElement::channelNeighborsSize && NOFLOW != channelChannelNeighbors[element][ii]; ii++)
    {
      if (!isBoundary(channelChannelNeighbors[element][ii]))
        {
          if (channelChannelNeighborsDownstream[element][ii])
            {
              downstreamElement = channelChannelNeighbors[element][ii];
            }
        }
      else if (OUTFLOW == channelChannelNeighbors[element][ii])
        {
          downstreamElement = channelChannelNeighbors[element][ii];
        }
    }
  
  return downstreamElement;
}

double FileManager::breakDigitalDam(int element, int dammedElement, double length)
{
  double slope;             // The slope downward from dammedElement to the point where the digital dam is broken, unitless.
  int    downstreamElement; // An element or boundary condition code downstream of element.
  double distanceLowered;   // The distance in meters by which an element is lowered to break a digital dam.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(0 <= element && element < globalNumberOfChannelElements && 0 <= dammedElement && dammedElement < globalNumberOfChannelElements && 0.0 < length &&
           NULL != channelElementZBank && NULL != channelElementZBed && NULL != channelElementLength && NULL != channelChannelType && NULL != channelChannelNeighbors &&
           NULL != channelChannelNeighborsDownstream);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (channelElementZBed[dammedElement] > channelElementZBed[element])
    {
      // element is lower than dammedElement.  Calculate the slope from dammedElement to element.
      slope = (channelElementZBed[dammedElement] - channelElementZBed[element]) / (length + 0.5 * channelElementLength[element]);
    }
  else if (WATERBODY == channelChannelType[element])
    {
      // element is a waterbody, which is allowed to be dammed, so lower it to the level of dammedElement.
      distanceLowered = channelElementZBed[element] - channelElementZBed[dammedElement];
      
      if ((2 <= ADHydro::verbosityLevel && 10.0 < distanceLowered) || 3 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in FileManager::breakDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", element,
                  distanceLowered, channelElementZBed[element], channelElementZBed[dammedElement]);
        }
      
      channelElementZBank[element] -= distanceLowered;
      channelElementZBed[element]   = channelElementZBed[dammedElement];
      slope                         = 0.0;
    }
  else
    {
      downstreamElement = findDownstreamElement(element);
      
      if (isBoundary(downstreamElement))
        {
          // element has an outflow boundary or no downstream connections at all.  Lower it to the level of dammedElement.
          if (OUTFLOW != downstreamElement)
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::breakDigitalDam: channel element %d has no downstream connections.\n", element);
                }
            }
          
          distanceLowered = channelElementZBed[element] - channelElementZBed[dammedElement];
          
          if ((2 <= ADHydro::verbosityLevel && 10.0 < distanceLowered) || 3 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in FileManager::breakDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", element,
                      distanceLowered, channelElementZBed[element], channelElementZBed[dammedElement]);
            }
          
          channelElementZBank[element] -= distanceLowered;
          channelElementZBed[element]   = channelElementZBed[dammedElement];
          slope                         = 0.0;
        }
      else
        {
          // Continue to search downstream, and when the search returns lower element to a straight line slope down from the dammed element.
          slope = breakDigitalDam(downstreamElement, dammedElement, length + channelElementLength[element]);
          
          distanceLowered = channelElementZBed[element] - (channelElementZBed[dammedElement] - slope * (length + 0.5 * channelElementLength[element]));
          
          if ((2 <= ADHydro::verbosityLevel && 10.0 < distanceLowered) || 3 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in FileManager::breakDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", element,
                      distanceLowered, channelElementZBed[element],
                      channelElementZBed[dammedElement] - slope * (length + 0.5 * channelElementLength[element]));
            }
          
          channelElementZBank[element] -= distanceLowered;
          channelElementZBed[element]   = channelElementZBed[dammedElement] - slope * (length + 0.5 * channelElementLength[element]);
        }
    }
  
  return slope;
}

int FileManager::connectMeshElementToChannelElementByReachCode(int element, long long reachCode)
{
  int    ii;                // Loop counter.
  int    neighbor = NOFLOW; // A neighbor of element.
  double neighborZBank;     // The bank Z coordinate of neighbor.
  double edgeLength;        // The edge length to use for the new connection.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(0 <= element && element < globalNumberOfMeshElements && NULL != meshVertexZSurface && NULL != meshMeshNeighborsEdgeLength &&
           NULL != meshChannelNeighbors && NULL != channelElementZBank && NULL != channelElementLength && NULL != channelChannelType &&
           NULL != channelReachCode && NULL != channelMeshNeighbors);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  for (ii = 0; ii < globalNumberOfChannelElements; ii++)
    {
      if (reachCode == channelReachCode[ii] && (NOFLOW == neighbor || neighborZBank > channelElementZBank[ii]))
        {
          neighbor      = ii;
          neighborZBank = channelElementZBank[ii];
        }
    }
  
  if (NOFLOW != neighbor)
    {
      if (3 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in FileManager::connectMeshElementToChannelElementByReachCode: breaking digital dam by connecting mesh element %d to channel "
                  "element %d.\n", element, neighbor);
        }
      
      // Calculate the edge length of the connection as the minimum of the length of the lowest edge of the mesh element or the length of the channel element.
      if (meshVertexZSurface[element][1] + meshVertexZSurface[element][2] < meshVertexZSurface[element][2] + meshVertexZSurface[element][0] &&
          meshVertexZSurface[element][1] + meshVertexZSurface[element][2] < meshVertexZSurface[element][0] + meshVertexZSurface[element][1])
        {
          // Edge zero is the lowest.
          edgeLength = meshMeshNeighborsEdgeLength[element][0];
        }
      else if (meshVertexZSurface[element][2] + meshVertexZSurface[element][0] < meshVertexZSurface[element][0] + meshVertexZSurface[element][1])
        {
          // Edge one is the lowest.
          edgeLength = meshMeshNeighborsEdgeLength[element][1];
        }
      else
        {
          // Edge two is the lowest.
          edgeLength = meshMeshNeighborsEdgeLength[element][2];
        }
      
      if (edgeLength > channelElementLength[neighbor])
        {
          edgeLength = channelElementLength[neighbor];
        }
      
      // Place the elements in each other's neighbor lists.
      ii = 0;
      
      while (ii < MeshElement::channelNeighborsSize && NOFLOW != meshChannelNeighbors[element][ii])
        {
          ii++;
        }
      
      if (ii < MeshElement::channelNeighborsSize)
        {
          meshChannelNeighbors[          element][ii] = neighbor;
          meshChannelNeighborsEdgeLength[element][ii] = edgeLength;
        }
      else
        {
          CkError("ERROR in FileManager::connectMeshElementToChannelElementByReachCode: mesh element %d has more than the maximum %d channel neighbors.\n",
                  element, MeshElement::channelNeighborsSize);
          CkExit();
        }
      
      ii = 0;
      
      while (ii < ChannelElement::meshNeighborsSize && NOFLOW != channelMeshNeighbors[neighbor][ii])
        {
          ii++;
        }
      
      if (ii < ChannelElement::meshNeighborsSize)
        {
          channelMeshNeighbors[          neighbor][ii] = element;
          channelMeshNeighborsEdgeLength[neighbor][ii] = edgeLength;
        }
      else
        {
          CkError("ERROR in FileManager::connectMeshElementToChannelElementByReachCode: channel element %d has more than the maximum %d mesh neighbors.\n",
                  neighbor, ChannelElement::meshNeighborsSize);
          CkExit();
        }
    }
  
  return neighbor;
}

void FileManager::meshMassage()
{
  int    ii, jj, kk;         // Loop counters.
  bool   hasLowerNeighbor;   // Whether an element has a lower neighbor.
  bool   hasChannelNeighbor; // Whether an element has a channel neighbor.
  int    downstreamElement;  // An element or boundary condition code downstream of an element.
  int    neighbor;           // A neighbor of an element.
  double edgeZSurface;       // The surface Z coordinate of the center of a mesh edge.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(1 == CkNumPes()))
    {
      CkError("ERROR in FileManager::meshMassage: meshMassage is not implemented for distributed operation.  It can only be run on one processor.\n");
      CkExit();
    }
  
  if (!(NULL != meshVertexZSurface))
    {
      CkError("ERROR in FileManager::meshMassage: meshVertexZSurface must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshElementZSurface))
    {
      CkError("ERROR in FileManager::meshMassage: meshElementZSurface must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshCatchment))
    {
      CkError("ERROR in FileManager::meshMassage: meshCatchment must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshMeshNeighbors))
    {
      CkError("ERROR in FileManager::meshMassage: meshMeshNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshMeshNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::meshMassage: meshMeshNeighborsEdgeLength must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshChannelNeighbors))
    {
      CkError("ERROR in FileManager::meshMassage: meshChannelNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelElementZBank))
    {
      CkError("ERROR in FileManager::meshMassage: channelElementZBank must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelElementZBed))
    {
      CkError("ERROR in FileManager::meshMassage: channelElementZBed must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelElementLength))
    {
      CkError("ERROR in FileManager::meshMassage: channelElementLength must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelChannelType))
    {
      CkError("ERROR in FileManager::meshMassage: channelChannelType must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelReachCode))
    {
      CkError("ERROR in FileManager::meshMassage: channelReachCode must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelChannelNeighbors))
    {
      CkError("ERROR in FileManager::meshMassage: channelChannelNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelChannelNeighborsDownstream))
    {
      CkError("ERROR in FileManager::meshMassage: channelChannelNeighborsDownstream must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelMeshNeighbors))
    {
      CkError("ERROR in FileManager::meshMassage: channelMeshNeighbors must not be NULL.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // Break digital dams in the channel network.
  for (ii = 0; ii < globalNumberOfChannelElements; ii++)
    {
      // Only break digital dams for streams.  The reason waterbodies have standing water in them is because there is some kind of real dam keeping it there.
      if (STREAM == channelChannelType[ii])
        {
          hasLowerNeighbor = false;

          for (jj = 0; !hasLowerNeighbor && jj < ChannelElement::channelNeighborsSize && NOFLOW != channelChannelNeighbors[ii][jj]; jj++)
            {
              neighbor = channelChannelNeighbors[ii][jj];
              
              if (!isBoundary(neighbor))
                {
                  if (channelChannelNeighborsDownstream[ii][jj] && channelElementZBed[ii] > channelElementZBed[neighbor])
                    {
                      hasLowerNeighbor = true;
                    }
                }
              else if (OUTFLOW == neighbor)
                {
                  hasLowerNeighbor = true;
                }
            }
          
          if (!hasLowerNeighbor)
            {
              downstreamElement = findDownstreamElement(ii);
              
              if (isBoundary(downstreamElement))
                {
                  if (2 <= ADHydro::verbosityLevel)
                    {
                      CkError("WARNING in FileManager::meshMassage: channel element %d has no downstream connections.\n", ii);
                    }
                }
              else
                {
                  breakDigitalDam(downstreamElement, ii, 0.5 * channelElementLength[ii]);
                }
            }
        }
    } // End break digital dams in the channel network.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  // Check that all digital dams in the channel network are broken.
  for (ii = 0; ii < globalNumberOfChannelElements; ii++)
    {
      // Only check digital dams for streams.  The reason waterbodies have standing water in them is because there is some kind of real dam keeping it there.
      if (STREAM == channelChannelType[ii])
        {
          hasLowerNeighbor = false;

          for (jj = 0; !hasLowerNeighbor && jj < ChannelElement::channelNeighborsSize && NOFLOW != channelChannelNeighbors[ii][jj]; jj++)
            {
              neighbor = channelChannelNeighbors[ii][jj];
              
              if (!isBoundary(neighbor))
                {
                  if (channelChannelNeighborsDownstream[ii][jj] && channelElementZBed[ii] > channelElementZBed[neighbor])
                    {
                      hasLowerNeighbor = true;
                    }
                }
              else if (OUTFLOW == neighbor)
                {
                  hasLowerNeighbor = true;
                }
            }
          
          if (!hasLowerNeighbor)
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::meshMassage: channel element %d is still a digital dam after all digital dams were broken.\n", ii);
                }
            }
        }
    } // End check that all digital dams in the channel network are broken.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  
  // Place a NOFLOW boundary between neighboring mesh elements that are in different catchments and separated by a ridge.
  for (ii = 0; ii < globalNumberOfMeshElements; ii++)
    {
      for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
        {
          neighbor = meshMeshNeighbors[ii][jj];
          
          // Only the lower ID number of the pair has to check.  This check also excludes boundary condition codes.
          if (ii < neighbor && meshCatchment[ii] != meshCatchment[neighbor])
            {
              // Get the height of the center of the edge separating the neighbors.  If it is higher than both neighbors it is considered a ridge.
              // FIXME this could be replaced with checking if both elements slope away from the edge.
              edgeZSurface = 0.5 * (meshVertexZSurface[ii][(jj + 1) % MeshElement::meshNeighborsSize] +
                                    meshVertexZSurface[ii][(jj + 2) % MeshElement::meshNeighborsSize]);
              
              if (edgeZSurface > meshElementZSurface[ii] && edgeZSurface > meshElementZSurface[neighbor])
                {
                  // Find and remove ii from neighbor's neighbor list
                  kk = 0;
                  
                  while (kk < MeshElement::meshNeighborsSize && ii != meshMeshNeighbors[neighbor][kk])
                    {
                      kk++;
                    }
                  
                  if (kk < MeshElement::meshNeighborsSize)
                    {
                      meshMeshNeighbors[neighbor][kk] = NOFLOW;
                    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  else
                    {
                      CkError("ERROR in FileManager::meshMassage: mesh element %d is not in his mesh neighbor %d's neighbor list.\n", ii, neighbor);
                      CkExit();
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  
                  // Remove neighbor from ii's neighbor list.
                  meshMeshNeighbors[ii][jj] = NOFLOW;
                }
            }
        }
    } // End place a NOFLOW boundary between neighboring mesh elements that are in different catchments and separated by a ridge.
  
  // Break digital dams in the mesh.
  for (ii = 0; ii < globalNumberOfMeshElements; ii++)
    {
      hasLowerNeighbor   = false;
      hasChannelNeighbor = false;
      
      for (jj = 0; !hasLowerNeighbor && jj < MeshElement::meshNeighborsSize; jj++)
        {
          neighbor = meshMeshNeighbors[ii][jj];
          
          if (!isBoundary(neighbor))
            {
              if (meshElementZSurface[ii] > meshElementZSurface[neighbor])
                {
                  hasLowerNeighbor = true;
                }
            }
          else if (OUTFLOW == neighbor)
            {
              hasLowerNeighbor = true;
            }
        }
      
      for (jj = 0; !hasChannelNeighbor && jj < MeshElement::channelNeighborsSize && NOFLOW != meshChannelNeighbors[ii][jj]; jj++)
        {
          neighbor = meshChannelNeighbors[ii][jj];
          
          // Don't count icemasses because an icemass could be higher than the mesh element.
          if (ICEMASS != channelChannelType[neighbor])
            {
              hasChannelNeighbor = true;
            }
        }
      
      if (!hasLowerNeighbor && !hasChannelNeighbor)
        {
          // Break the digital dam by arbitrarily connecting the mesh element to the lowest stream element in the same catchment.
          neighbor = connectMeshElementToChannelElementByReachCode(ii, meshCatchment[ii]);
          
          if (NOFLOW == neighbor && NULL != channelPruned)
            {
              // The stream for that catchment was pruned.  Get the reach code of a link downstream of it.
              jj = 0;
              
              while (jj < numberOfChannelPruned && channelPruned[jj][0] != meshCatchment[ii])
                {
                  jj++;
                }
              
              if (jj < numberOfChannelPruned)
                {
                  neighbor = connectMeshElementToChannelElementByReachCode(ii, channelPruned[jj][1]);
                }
            }

          if (NOFLOW == neighbor)
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::meshMassage: mesh element %d is a digital dam, but there is no channel element in catchment %d to connect "
                          "it to.\n", ii, meshCatchment[ii]);
                }
            }
        }
    } // End break digital dams in the mesh.
}

void FileManager::calculateDerivedValues()
{
  int    ii, jj;           // Loop counters.
  double value;            // For calculating derived values.
  double nominalSoilDepth; // Noah-MP cannot handle zero soil depth.  For setting depth of one meter instead.
  double lengthSoFar;      // For traversing vertices.  The length traversed so far.
  double nextLength;       // For traversing vertices.  The length to the next vertex.
  double lengthFraction;   // For traversing vertices.  The fraction of the distance from the current vertex to the next of the point of interest.
  double minX;             // For finding the bounding box of vertices.
  double maxX;             // For finding the bounding box of vertices.
  double minY;             // For finding the bounding box of vertices.
  double maxY;             // For finding the bounding box of vertices.
  double minZBank;         // For finding the bounding box of vertices.
  double maxZBank;         // For finding the bounding box of vertices.

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
  
  // Calculate meshElementZSurface by adding meshElementSoilDepth to meshElementZBedrock.
  if (NULL == meshElementZSurface && NULL != meshElementZBedrock && NULL != meshElementSoilDepth)
    {
      meshElementZSurface = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshElementZSurface[ii] = meshElementZBedrock[ii] + meshElementSoilDepth[ii];
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
  
  // Fix elements with invalid vegetation or soil type.
  if (ADHydro::doMeshMassage)
    {
      meshMassageVegetationAndSoilType();
    }
  
  // MeshConductivity and meshPorosity are taken from 19-category SOILPARM.TBL of Noah-MP.
  if ((NULL == meshConductivity || NULL == meshPorosity) && NULL != meshSoilType && NULL != meshVegetationType)
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
          default:
            if (2 <= ADHydro::verbosityLevel)
              {
                CkError("WARNING in FileManager::calculateDerivedValues: unknown soil type %d. using default values of conductivity and porosity for sand\n",
                        meshSoilType[ii]);
              }
            meshConductivity[ii] = 4.66E-5;
            meshPorosity[ii]     = 0.339;
            break;
          } // End of switch.
          
          // If the vegetation type is Urban and Built-Up Land (1) then Noah-MP sets porosity to 0.45 regardless of soil type.
          if (1 == meshVegetationType[ii])
            {
              meshPorosity[ii] = 0.45;
            }
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
            case 16: // USGS 16 Water Bodies,                   NLCD 11 open water
                   meshManningsN[ii] = 0.02;
                   break;
            case 24: // USGS 24 Snow or Ice,                    NLCD 12 ice/snow
                   meshManningsN[ii] = 0.022;
                   break;
            case 1:  // USGS 1  Urban and Built-Up Land,        NLCD 21 developed open space
                     //                                         NLCD 22 developed low intensity
                     //                                         NLCD 23 developed medium intensity
                     //                                         NLCD 24 developed high intensity
                   meshManningsN[ii] = 0.12;
                   break;
            case 19: // USGS 19 Barren or Sparsely Vegetated,   NLCD 31 barren land
                   meshManningsN[ii] = 0.04;
                   break;
            case 11: // USGS 11 Deciduous Broadleaf Forest,     NLCD 41 deciduous forest
                   meshManningsN[ii] = 0.16;
                   break;
            case 13: // USGS 13 Evergreen Broadleaf Forest,     NLCD 42 evergreen forest
                   meshManningsN[ii] = 0.18;
                   break;
            case 15: // USGS 15 Mixed Forest,                   NLCD 43 mixed forest
                   meshManningsN[ii] = 0.17;
                   break;
            case 22: // USGS 22 Mixed Tundra,                   NLCD 51 dwarf scrub-Alaska only
                     //                                         NLCD 72 sedge/herbaceous-Alaska only
                     //                                         NLCD 73 Lichens-Alaska only
                     //                                         NLCD 74 Moss-Alaska only
                   meshManningsN[ii] = 0.05;
                   break;
            case 8:  // USGS 8  Shrubland,                      NLCD 52 shrub
                   meshManningsN[ii] = 0.07;
                   break;
            case 7:  // USGS 7  Grassland,                      NLCD 71 grassland
                   meshManningsN[ii] = 0.035;
                   break;
            case 2:  // USGS 2  Dryland Cropland and Pasture,   NLCD 81 Pasture
                   meshManningsN[ii] = 0.033;
                   break;
            case 3:  // USGS 3  Irrigated Cropland and Pasture, NLCD 82 cultivated crops
                   meshManningsN[ii] = 0.04;
                   break;
            case 18: // USGS 18  Wooded Wetland,                NLCD 90 woody wetland
                   meshManningsN[ii] = 0.14;
                   break;
            case 17: // USGS 17 Herbaceous Wetland,             NLCD 95 herbaceous wetland
                   meshManningsN[ii] = 0.035;
                   break;
            default:
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::calculateDerivedValues: unknown vegetation type %d. using default value of Manning's N for mixed forest\n",
                          meshVegetationType[ii]);
                }
              meshManningsN[ii] = 0.17;
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
  
  // If not already specified meshTranspiration defaults to zero.
  if (NULL == meshTranspiration)
    {
      meshTranspiration = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTranspiration[ii] = 0.0;
        }
    }
  
  // If not already specified meshTranspirationCumulative defaults to zero.
  if (NULL == meshTranspirationCumulative)
    {
      meshTranspirationCumulative = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTranspirationCumulative[ii] = 0.0;
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
          // Noah-MP cannot handle zero soil depth.  Use one meter instead.
          if (0.0 == meshElementSoilDepth[ii])
            {
              nominalSoilDepth = 1.0;
            }
          else
            {
              nominalSoilDepth = meshElementSoilDepth[ii];
            }
          
          meshZSnso[ii][0] = 0.0f;
          meshZSnso[ii][1] = 0.0f;
          meshZSnso[ii][2] = 0.0f;
          meshZSnso[ii][3] = (float)(-0.05 * nominalSoilDepth);
          meshZSnso[ii][4] = (float)(-0.2  * nominalSoilDepth);
          meshZSnso[ii][5] = (float)(-0.5  * nominalSoilDepth);
          meshZSnso[ii][6] = (float)(-1.0  * nominalSoilDepth);
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
  
  // Calculate channelElementX, channelElementY, channelElementZBank from channel vertices.
  if ((NULL == channelElementX || NULL == channelElementY || NULL == channelElementZBank) && NULL != channelVertexX && NULL != channelVertexY &&
      NULL != channelVertexZBank && NULL != channelElementLength)
    {
      if (NULL == channelElementX)
        {
          channelElementX = new double[localNumberOfChannelElements];
        }
      
      if (NULL == channelElementY)
        {
          channelElementY = new double[localNumberOfChannelElements];
        }
      
      if (NULL == channelElementZBank)
        {
          channelElementZBank = new double[localNumberOfChannelElements];
        }
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          if (STREAM == channelChannelType[ii])
            {
              // For streams traverse the vertices to find the midpoint along the length of the stream.
              jj          = 0;   // The vertex being processed.
              lengthSoFar = 0.0; // The length up to jj - 1.
              nextLength  = 0.0; // The length from jj - 1 to jj.
              
              do
                {
                  jj++;
                  lengthSoFar += nextLength;
                  nextLength   = sqrt((channelVertexX[ii][jj] - channelVertexX[ii][jj - 1]) * (channelVertexX[ii][jj] - channelVertexX[ii][jj - 1]) +
                                      (channelVertexY[ii][jj] - channelVertexY[ii][jj - 1]) * (channelVertexY[ii][jj] - channelVertexY[ii][jj - 1]));
                }
              while (jj + 1 < localNumberOfChannelElements && channelElementLength[ii] * 0.5 > lengthSoFar + nextLength);
              
              if (0.0 == nextLength)
                {
                  lengthFraction = 0.0;
                }
              else
                {
                  lengthFraction = (channelElementLength[ii] * 0.5 - lengthSoFar) / nextLength;
                }
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(0.0 <= lengthFraction && 1.0 >= lengthFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              channelElementX[ii]     = channelVertexX[ii][jj - 1]     + (channelVertexX[ii][jj]     - channelVertexX[ii][jj - 1])     * lengthFraction;
              channelElementY[ii]     = channelVertexY[ii][jj - 1]     + (channelVertexY[ii][jj]     - channelVertexY[ii][jj - 1])     * lengthFraction;
              channelElementZBank[ii] = channelVertexZBank[ii][jj - 1] + (channelVertexZBank[ii][jj] - channelVertexZBank[ii][jj - 1]) * lengthFraction;
            }
          else
            {
              // For waterbodies find the center of the bounding box of the vertices.
              minX     = channelVertexX[ii][0];
              maxX     = channelVertexX[ii][0];
              minY     = channelVertexY[ii][0];
              maxY     = channelVertexY[ii][0];
              minZBank = channelVertexZBank[ii][0];
              maxZBank = channelVertexZBank[ii][0];
              
              for (jj = 1; jj < ChannelElement::channelVerticesSize; jj++)
                {
                  if (minX > channelVertexX[ii][jj])
                    {
                      minX = channelVertexX[ii][jj];
                    }
                  
                  if (maxX < channelVertexX[ii][jj])
                    {
                      maxX = channelVertexX[ii][jj];
                    }
                  
                  if (minY > channelVertexY[ii][jj])
                    {
                      minY = channelVertexY[ii][jj];
                    }
                  
                  if (maxY < channelVertexY[ii][jj])
                    {
                      maxY = channelVertexY[ii][jj];
                    }
                  
                  if (minZBank > channelVertexZBank[ii][jj])
                    {
                      minZBank = channelVertexZBank[ii][jj];
                    }
                  
                  if (maxZBank < channelVertexZBank[ii][jj])
                    {
                      maxZBank = channelVertexZBank[ii][jj];
                    }
                  
                }
              
              channelElementX[ii]     = (minX     + maxX)     * 0.5;
              channelElementY[ii]     = (minY     + maxY)     * 0.5;
              channelElementZBank[ii] = (minZBank + maxZBank) * 0.5;
            }
        }
    }
  
  // Calculate channelElementZBank by adding channelElementBankFullDepth to channelElementZBed.
  if (NULL == channelElementZBank && NULL != channelElementBankFullDepth && NULL != channelElementZBed)
    {
      channelElementZBank = new double[localNumberOfChannelElements];

      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelElementZBank[ii] = channelElementZBed[ii] + channelElementBankFullDepth[ii];
        }
    }
  
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
  
  // If not already specified channelSurfacewaterDepth defaults to channelElementBankFullDepth.
  if (NULL == channelSurfacewaterDepth && NULL != channelElementBankFullDepth)
    {
      channelSurfacewaterDepth = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelSurfacewaterDepth[ii] = channelElementBankFullDepth[ii];
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
  
  // Fix digital dams and similar problems.
  if (ADHydro::doMeshMassage)
    {
      meshMassage();
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
      error = createNetCDFDimension(fileID, "channelElementVerticesSize", XDMF_SIZE, &channelElementVerticesSizeDimensionID);
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
  
  if (!error && NULL != channelChannelNeighborsDownstream)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Assumes bool is 1 byte when storing as NC_BYTE.
      CkAssert(1 == sizeof(bool));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      error = createNetCDFVariable(fileID, "channelChannelNeighborsDownstream", NC_BYTE, 3, instancesDimensionID, channelElementsDimensionID,
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
  
  if (!error && NULL != channelReachCode)
    {
      error = createNetCDFVariable(fileID, "channelReachCode", NC_INT64, 2, instancesDimensionID, channelElementsDimensionID, 0);
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
  
  if (!error && NULL != meshTranspiration)
    {
      error = createNetCDFVariable(fileID, "meshTranspiration", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshTranspirationCumulative)
    {
      error = createNetCDFVariable(fileID, "meshTranspirationCumulative", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
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
  
  // Create the display file.
  if (!error)
    {
      error = openNetCDFFile(directory, "display.nc", true, false, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Create display dimensions.
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
      error = createNetCDFDimension(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimensionID);
    }
  
  if (!error)
    {
      error = createNetCDFDimension(fileID, "channelChannelNeighborsSize", ChannelElement::channelNeighborsSize, &channelChannelNeighborsSizeDimensionID);
    }
  
  // Create display variables.
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
  
  if (!error && NULL != meshTranspiration)
    {
      error = createNetCDFVariable(fileID, "meshTranspiration", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshTranspirationCumulative)
    {
      error = createNetCDFVariable(fileID, "meshTranspirationCumulative", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0);
    }
  
  if (!error && NULL != meshSnEqv)
    {
      error = createNetCDFVariable(fileID, "meshSnEqv", NC_FLOAT, 2, instancesDimensionID, meshElementsDimensionID, 0);
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
  
  if (!error && NULL != channelSnEqv)
    {
      error = createNetCDFVariable(fileID, "channelSnEqv", NC_FLOAT, 2, instancesDimensionID, channelElementsDimensionID, 0);
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
  
  // Close the display file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleCreateFiles: unable to close NetCDF display file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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

void FileManager::handleWriteFiles(const char* directory, bool writeGeometry, bool writeParameter, bool writeState, bool writeDisplay, double referenceDateNew,
                                   double currentTimeNew, double dtNew, size_t iterationNew)
{
  bool   error    = false;  // Error flag.
  int    ncErrorCode;       // Return value of NetCDF functions.
  int    fileID;            // ID of NetCDF file.
  bool   fileOpen = false;  // Whether fileID refers to an open file.
  size_t geometryInstance;  // Instance index for geometry file.
  size_t parameterInstance; // Instance index for parameter file.
  size_t stateInstance;     // Instance index for state file.
  size_t displayInstance;   // Instance index for display file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleWriteFiles: directory must not be null.\n");
      error = true;
    }

  if (!(0.0 < dtNew))
    {
      CkError("ERROR in FileManager::handleUpdateState: dtNew must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Save reference date, current time, dt, and iteration.
      referenceDate = referenceDateNew;
      currentTime   = currentTimeNew;
      dt            = dtNew;
      iteration     = iterationNew;
      
      // Open the geometry file.
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
          error = writeNetCDFVariable(fileID, "channelElementVertices", geometryInstance, localChannelElementStart, localNumberOfChannelElements, XDMF_SIZE,
                                      channelElementVertices);
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
      
      if (!error && NULL != channelChannelNeighborsDownstream)
        {
          error = writeNetCDFVariable(fileID, "channelChannelNeighborsDownstream", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                      ChannelElement::channelNeighborsSize, channelChannelNeighborsDownstream);
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
      
      if (!error && NULL != channelReachCode)
        {
          error = writeNetCDFVariable(fileID, "channelReachCode", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelReachCode);
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
      
      if (!error && NULL != meshTranspiration)
        {
          error = writeNetCDFVariable(fileID, "meshTranspiration", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshTranspiration);
        }
      
      if (!error && NULL != meshTranspirationCumulative)
        {
          error = writeNetCDFVariable(fileID, "meshTranspirationCumulative", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshTranspirationCumulative);
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
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsSurfacewaterFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshMeshNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsSurfacewaterCumulativeFlow", stateInstance, localMeshElementStart,
                                      localNumberOfMeshElements, MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != meshMeshNeighborsGroundwaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsGroundwaterFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshMeshNeighborsGroundwaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsGroundwaterCumulativeFlow", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshChannelNeighborsSurfacewaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsSurfacewaterFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::channelNeighborsSize, meshChannelNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshChannelNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsSurfacewaterCumulativeFlow", stateInstance, localMeshElementStart,
                                      localNumberOfMeshElements, MeshElement::channelNeighborsSize, meshChannelNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != meshChannelNeighborsGroundwaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsGroundwaterFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::channelNeighborsSize, meshChannelNeighborsGroundwaterFlowRate);
        }
      
      if (!error && NULL != meshChannelNeighborsGroundwaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshChannelNeighborsGroundwaterCumulativeFlow", stateInstance, localMeshElementStart,
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
          error = writeNetCDFVariable(fileID, "channelChannelNeighborsSurfacewaterFlowRate", stateInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::channelNeighborsSize, channelChannelNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != channelChannelNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "channelChannelNeighborsSurfacewaterCumulativeFlow", stateInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::channelNeighborsSize, channelChannelNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != channelMeshNeighborsSurfacewaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsSurfacewaterFlowRate", stateInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::meshNeighborsSize, channelMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != channelMeshNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsSurfacewaterCumulativeFlow", stateInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::meshNeighborsSize, channelMeshNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != channelMeshNeighborsGroundwaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsGroundwaterFlowRate", stateInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::meshNeighborsSize, channelMeshNeighborsGroundwaterFlowRate);
        }
      
      if (!error && NULL != channelMeshNeighborsGroundwaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "channelMeshNeighborsGroundwaterCumulativeFlow", stateInstance, localChannelElementStart,
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
  
  // Open the display file.
  if (!error)
    {
      error = openNetCDFFile(directory, "display.nc", false, true, &fileID);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances in the display file.
  if (!error)
    {
      error = readNetCDFDimensionSize(fileID, "instances", &displayInstance);
    }
  
  // Write display variables.
  if (writeDisplay)
    {
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "geometryInstance", displayInstance, 0, 1, 1, &geometryInstance);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "parameterInstance", displayInstance, 0, 1, 1, &parameterInstance);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "referenceDate", displayInstance, 0, 1, 1, &referenceDate);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "currentTime", displayInstance, 0, 1, 1, &currentTime);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "dt", displayInstance, 0, 1, 1, &dt);
        }
      
      if (!error)
        {
          error = writeNetCDFVariable(fileID, "iteration", displayInstance, 0, 1, 1, &iteration);
        }
      
      if (!error && NULL != meshSurfacewaterDepth)
        {
          error = writeNetCDFVariable(fileID, "meshSurfacewaterDepth", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshSurfacewaterDepth);
        }
      
      if (!error && NULL != meshSurfacewaterError)
        {
          error = writeNetCDFVariable(fileID, "meshSurfacewaterError", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshSurfacewaterError);
        }
      
      if (!error && NULL != meshGroundwaterHead)
        {
          error = writeNetCDFVariable(fileID, "meshGroundwaterHead", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshGroundwaterHead);
        }
      
      if (!error && NULL != meshGroundwaterError)
        {
          error = writeNetCDFVariable(fileID, "meshGroundwaterError", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshGroundwaterError);
        }
      
      if (!error && NULL != meshPrecipitation)
        {
          error = writeNetCDFVariable(fileID, "meshPrecipitation", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshPrecipitation);
        }
      
      if (!error && NULL != meshPrecipitationCumulative)
        {
          error = writeNetCDFVariable(fileID, "meshPrecipitationCumulative", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshPrecipitationCumulative);
        }
      
      if (!error && NULL != meshEvaporation)
        {
          error = writeNetCDFVariable(fileID, "meshEvaporation", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshEvaporation);
        }
      
      if (!error && NULL != meshEvaporationCumulative)
        {
          error = writeNetCDFVariable(fileID, "meshEvaporationCumulative", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshEvaporationCumulative);
        }
      
      if (!error && NULL != meshTranspiration)
        {
          error = writeNetCDFVariable(fileID, "meshTranspiration", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshTranspiration);
        }
      
      if (!error && NULL != meshTranspirationCumulative)
        {
          error = writeNetCDFVariable(fileID, "meshTranspirationCumulative", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                      meshTranspirationCumulative);
        }
      
      if (!error && NULL != meshSnEqv)
        {
          error = writeNetCDFVariable(fileID, "meshSnEqv", displayInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSnEqv);
        }
      
      if (!error && NULL != meshMeshNeighborsSurfacewaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsSurfacewaterFlowRate", displayInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshMeshNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsSurfacewaterCumulativeFlow", displayInstance, localMeshElementStart,
                                      localNumberOfMeshElements, MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterCumulativeFlow);
        }
      
      if (!error && NULL != meshMeshNeighborsGroundwaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsGroundwaterFlowRate", displayInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != meshMeshNeighborsGroundwaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "meshMeshNeighborsGroundwaterCumulativeFlow", displayInstance, localMeshElementStart, localNumberOfMeshElements,
                                      MeshElement::meshNeighborsSize, meshMeshNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != channelSurfacewaterDepth)
        {
          error = writeNetCDFVariable(fileID, "channelSurfacewaterDepth", displayInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelSurfacewaterDepth);
        }
      
      if (!error && NULL != channelSurfacewaterError)
        {
          error = writeNetCDFVariable(fileID, "channelSurfacewaterError", displayInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelSurfacewaterError);
        }
      
      if (!error && NULL != channelPrecipitation)
        {
          error = writeNetCDFVariable(fileID, "channelPrecipitation", displayInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelPrecipitation);
        }
      
      if (!error && NULL != channelPrecipitationCumulative)
        {
          error = writeNetCDFVariable(fileID, "channelPrecipitationCumulative", displayInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelPrecipitationCumulative);
        }
      
      if (!error && NULL != channelEvaporation)
        {
          error = writeNetCDFVariable(fileID, "channelEvaporation", displayInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelEvaporation);
        }
      
      if (!error && NULL != channelEvaporationCumulative)
        {
          error = writeNetCDFVariable(fileID, "channelEvaporationCumulative", displayInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                      channelEvaporationCumulative);
        }
      
      if (!error && NULL != channelSnEqv)
        {
          error = writeNetCDFVariable(fileID, "channelSnEqv", displayInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelSnEqv);
        }
      
      if (!error && NULL != channelChannelNeighborsSurfacewaterFlowRate)
        {
          error = writeNetCDFVariable(fileID, "channelChannelNeighborsSurfacewaterFlowRate", displayInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::channelNeighborsSize, channelChannelNeighborsSurfacewaterFlowRate);
        }
      
      if (!error && NULL != channelChannelNeighborsSurfacewaterCumulativeFlow)
        {
          error = writeNetCDFVariable(fileID, "channelChannelNeighborsSurfacewaterCumulativeFlow", displayInstance, localChannelElementStart,
                                      localNumberOfChannelElements, ChannelElement::channelNeighborsSize, channelChannelNeighborsSurfacewaterCumulativeFlow);
        }
    } // End if (writeDisplay).
  
  // Close the display file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::handleWriteFiles: unable to close NetCDF display file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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
  bool    error    = false;  // Error flag.
  int     ii;                // Loop counter.
  int     ncErrorCode;       // Return value of NetCDF functions.
  int     fileID;            // ID of NetCDF file.
  bool    fileOpen = false;  // Whether fileID refers to an open file.
  int     variableID;        // ID of variable in NetCDF file.
  size_t  numberOfInstances; // Size of instance dimension.
  size_t  instance;          // Instance index for file.
  double  currentDate;       // The date and time represented by referenceDateNew and currentTimeNew as a Julian date.
  double* jultime   = NULL;  // Used to read Julian date.
  float*  t2        = NULL;  // Used to read air temperature at 2m height forcing.
  float*  vegFra    = NULL;  // Used to read vegetation fraction forcing.
  float*  maxVegFra = NULL;  // Used to read maximum vegetation fraction forcing.
  float*  psfc      = NULL;  // Used to read surface pressure forcing.
  float*  u         = NULL;  // Used to read wind speed U component forcing.
  float*  v         = NULL;  // Used to read wind speed V component forcing.
  float*  qVapor    = NULL;  // Used to read water vapor mixing ratio forcing.
  float*  qCloud    = NULL;  // Used to read cloud water mixing ratio forcing.
  float*  swDown    = NULL;  // Used to read downward shortwave flux forcing.
  float*  gLw       = NULL;  // Used to read downward longwave flux forcing.
  float*  tPrec     = NULL;  // Used to read total precipitation forcing.
  float*  tslb      = NULL;  // Used to read soil temperature at the deepest layer forcing.
  float*  pblh      = NULL;  // Used to read planetary boundary layer height forcing.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::handleReadForcingData: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Save reference date and current time.
      referenceDate = referenceDateNew;
      currentTime   = currentTimeNew;
      
      // Open the forcing file.
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
      jultime = new double[numberOfInstances];
      
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
  
  // Search for the last instance that is before or equal to the current date and time.
  // FIXME to improve efficiency make this a binary search.
  if (!error)
    {
      instance    = 0;
      
      if (ADHydro::drainDownMode)
        {
          // In drain down mode use a single time forever.
          currentDate = referenceDate + ADHydro::drainDownTime / (24.0 * 3600.0);
        }
      else
        {
          currentDate = referenceDate + currentTime / (24.0 * 3600.0);
        }
      
      if (!(jultime[0] <= currentDate))
        {
          if (2 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in FileManager::handleReadForcingData: All forcing data in the NetCDF forcing file is in the future.  Using the first forcing "
                      "data instance.\n");
            }
        }
      
      while (instance + 1 < numberOfInstances && jultime[instance + 1] <= currentDate)
        {
          instance++;
        }
      
      // instance is now the index of the forcing data instance we will use.
      // Record the dates of the current forcing data and next forcing data and that the forcing data is initialized.
      forcingDataDate = jultime[instance];
      
      if (ADHydro::drainDownMode)
        {
          // In drain down mode use the initial forcing data forever.
          nextForcingDataDate = INFINITY;
        }
      else if (instance + 1 < numberOfInstances)
        {
          nextForcingDataDate = jultime[instance + 1];
        }
      else
        {
          if (2 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in FileManager::handleReadForcingData: Using the last forcing data instance in NetCDF forcing file.  No new forcing data will "
                      "be loaded after this no matter how long the simulation runs.\n");
            }
          
          nextForcingDataDate = INFINITY;
        }
      
      forcingDataInitialized = true;
    }
  
  deleteArrayIfNonNull(&jultime);
  
  // Read forcing variables.
  if (0 < localNumberOfMeshElements)
    {
      // We don't use vegFra anymore.
      //if (!error)
      //  {
      //    error = readNetCDFVariable(fileID, "VEGFRA", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &vegFra);
      //  }
      
      // We don't use maxVegFra antmore.
      //if (!error)
      //  {
      //    error = readNetCDFVariable(fileID, "MAXVEGFRA", instance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &maxVegFra);
      //  }
      
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
      
      if (!error && !ADHydro::drainDownMode)
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
          meshProxy[ii].forcingDataMessage(20.0f, 0.0f /*vegFra[ii - localMeshElementStart]*/, 0.0f /*maxVegFra[ii - localMeshElementStart]*/, t2[ii - localMeshElementStart],
                                           psfc[ii - localMeshElementStart], psfc[ii - localMeshElementStart] - 120.0f, u[ii - localMeshElementStart],
                                           v[ii - localMeshElementStart], qVapor[ii - localMeshElementStart], qCloud[ii - localMeshElementStart],
                                           swDown[ii - localMeshElementStart], gLw[ii - localMeshElementStart],
                                           ADHydro::drainDownMode ? 0.0f : tPrec[ii - localMeshElementStart], tslb[ii - localMeshElementStart],
                                           pblh[ii - localMeshElementStart]);
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
      // We don't use vegFra anymore.
      //if (!error)
      //  {
      //    error = readNetCDFVariable(fileID, "VEGFRA_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &vegFra);
      //  }

      // We don't use maxVegFra anymore.
      //if (!error)
      //  {
      //    error = readNetCDFVariable(fileID, "MAXVEGFRA_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &maxVegFra);
      //  }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "T2_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &t2);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "PSFC_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &psfc);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "U_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &u);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "V_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &v);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "QVAPOR_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &qVapor);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "QCLOUD_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &qCloud);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "SWDOWN_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &swDown);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "GLW_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &gLw);
        }

      if (!error && !ADHydro::drainDownMode)
        {
          error = readNetCDFVariable(fileID, "TPREC_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &tPrec);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "TSLB_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &tslb);
        }

      if (!error)
        {
          error = readNetCDFVariable(fileID, "PBLH_C", instance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &pblh);
        }

      for (ii = localChannelElementStart; ii < localChannelElementStart + localNumberOfChannelElements; ii++)
        {
          channelProxy[ii].forcingDataMessage(20.0f, 0.0f /*vegFra[ii - localChannelElementStart]*/, 0.0f /*maxVegFra[ii - localChannelElementStart]*/,
                                              t2[ii - localChannelElementStart], psfc[ii - localChannelElementStart],
                                              psfc[ii - localChannelElementStart] - 120.0f, u[ii - localChannelElementStart], v[ii - localChannelElementStart],
                                              qVapor[ii - localChannelElementStart], qCloud[ii - localChannelElementStart],
                                              swDown[ii - localChannelElementStart], gLw[ii - localChannelElementStart],
                                              ADHydro::drainDownMode ? 0.0f : tPrec[ii - localChannelElementStart], tslb[ii - localChannelElementStart],
                                              pblh[ii - localChannelElementStart]);
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

void FileManager::handleUpdateState()
{
  int ii; // Loop counter.
  
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
                                         double transpiration, double transpirationCumulative, double surfacewaterInfiltration, double groundwaterRecharge,
                                         EvapoTranspirationStateStruct evapoTranspirationState, double* meshNeighborsSurfacewaterFlowRate,
                                         double* meshNeighborsSurfacewaterCumulativeFlow, double* meshNeighborsGroundwaterFlowRate,
                                         double* meshNeighborsGroundwaterCumulativeFlow, double* channelNeighborsSurfacewaterFlowRate,
                                         double* channelNeighborsSurfacewaterCumulativeFlow, double* channelNeighborsGroundwaterFlowRate,
                                         double* channelNeighborsGroundwaterCumulativeFlow)
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
  
  if (!(0.0 <= precipitation))
    {
      CkError("ERROR in FileManager::handleMeshStateMessage, element %d: precipitation must be greater than or equal to zero.\n", element);
      CkExit();
    }
  
  if (!(0.0 <= precipitationCumulative))
    {
      CkError("ERROR in FileManager::handleMeshStateMessage, element %d: precipitationCumulative must be greater than or equal to zero.\n", element);
      CkExit();
    }
  
  if (!(0.0 >= transpiration))
    {
      CkError("ERROR in FileManager::handleMeshStateMessage, element %d: transpiration must be less than or equal to zero.\n", element);
      CkExit();
    }
  
  if (!(0.0 >= transpirationCumulative))
    {
      CkError("ERROR in FileManager::handleMeshStateMessage, element %d: transpirationCumulative must be less than or equal to zero.\n", element);
      CkExit();
    }
  
  if (!(0.0 <= surfacewaterInfiltration))
    {
      CkError("ERROR in FileManager::handleMeshStateMessage, element %d: surfacewaterInfiltration must be greater than or equal to zero.\n", element);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (checkEvapoTranspirationStateStructInvariant(&evapoTranspirationState))
    {
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  meshSurfacewaterDepth[element - localMeshElementStart]        = surfacewaterDepth;
  meshSurfacewaterError[element - localMeshElementStart]        = surfacewaterError;
  meshGroundwaterHead[element - localMeshElementStart]          = groundwaterHead;
  meshGroundwaterError[element - localMeshElementStart]         = groundwaterError;
  meshPrecipitation[element - localMeshElementStart]            = precipitation;
  meshPrecipitationCumulative[element - localMeshElementStart]  = precipitationCumulative;
  meshEvaporation[element - localMeshElementStart]              = evaporation;
  meshEvaporationCumulative[element - localMeshElementStart]    = evaporationCumulative;
  meshTranspiration[element - localMeshElementStart]            = transpiration;
  meshTranspirationCumulative[element - localMeshElementStart]  = transpirationCumulative;
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
  
  if (!(0.0 <= precipitation))
    {
      CkError("ERROR in FileManager::handleChannelStateMessage, element %d: precipitation must be greater than or equal to zero.\n", element);
      CkExit();
    }
  
  if (!(0.0 <= precipitationCumulative))
    {
      CkError("ERROR in FileManager::handleChannelStateMessage, element %d: precipitationCumulative must be greater than or equal to zero.\n", element);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (checkEvapoTranspirationStateStructInvariant(&evapoTranspirationState))
    {
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

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
