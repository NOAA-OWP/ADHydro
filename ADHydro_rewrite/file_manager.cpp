#include "file_manager.h"
#include "adhydro.h"

void FileManager::printOutMassBalance(double waterInDomain, double externalFlows, double waterError)
{
  static double massBalanceShouldBe = NAN; // This stores the first value received and uses it as the "should be" value for the rest of the simulation.
  double        massBalance         = waterInDomain + externalFlows - waterError;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= waterInDomain))
    {
      CkError("ERROR in FileManager::printOutMassBalance: waterInDomain must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (isnan(massBalanceShouldBe))
    {
      massBalanceShouldBe = massBalance;
    }
  
  CkPrintf("waterInDomain = %lg, externalFlows = %lg, waterError = %lg, massBalance = %lg, massBalanceError = %lg, all values in cubic meters.\n",
           waterInDomain, externalFlows, waterError, massBalance, massBalance - massBalanceShouldBe);
}

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

FileManager::FileManager() :
  globalNumberOfRegions(0),
  localRegionStart(0),
  localNumberOfRegions(0),
  globalNumberOfMeshNodes(0),
  localMeshNodeStart(0),
  localNumberOfMeshNodes(0),
  globalNumberOfMeshElements(0),
  localMeshElementStart(0),
  localNumberOfMeshElements(0),
  globalNumberOfChannelNodes(0),
  localChannelNodeStart(0),
  localNumberOfChannelNodes(0),
  globalNumberOfChannelElements(0),
  localChannelElementStart(0),
  localNumberOfChannelElements(0),
  regionNumberOfMeshElements(NULL),
  regionNumberOfChannelElements(NULL),
  meshNodeX(NULL),
  meshNodeY(NULL),
  meshNodeZSurface(NULL),
  meshElementVertices(NULL),
  meshVertexX(NULL),
  meshVertexY(NULL),
  meshVertexZSurface(NULL),
  meshRegion(NULL),
  meshCatchment(NULL),
  meshVegetationType(NULL),
  meshSoilType(NULL),
  meshAlluvium(NULL),
  meshElementX(NULL),
  meshElementY(NULL),
  meshElementZSurface(NULL),
  meshElementSoilDepth(NULL),
  meshElementLayerZBottom(NULL),
  meshElementArea(NULL),
  meshElementSlopeX(NULL),
  meshElementSlopeY(NULL),
  meshLatitude(NULL),
  meshLongitude(NULL),
  meshManningsN(NULL),
  meshConductivity(NULL),
  meshPorosity(NULL),
  meshSurfacewaterDepth(NULL),
  meshSurfacewaterError(NULL),
  meshGroundwaterHead(NULL),
  meshGroundwaterRecharge(NULL),
  meshGroundwaterError(NULL),
  meshPrecipitationRate(NULL),
  meshPrecipitationCumulativeShortTerm(NULL),
  meshPrecipitationCumulativeLongTerm(NULL),
  meshEvaporationRate(NULL),
  meshEvaporationCumulativeShortTerm(NULL),
  meshEvaporationCumulativeLongTerm(NULL),
  meshTranspirationRate(NULL),
  meshTranspirationCumulativeShortTerm(NULL),
  meshTranspirationCumulativeLongTerm(NULL),
  meshEvapoTranspirationState(NULL),
  meshInfiltrationMethod(NULL),
  meshGroundwaterMethod(NULL),
  meshMeshNeighbors(NULL),
  meshMeshNeighborsRegion(NULL),
  meshMeshNeighborsChannelEdge(NULL),
  meshMeshNeighborsEdgeLength(NULL),
  meshMeshNeighborsEdgeNormalX(NULL),
  meshMeshNeighborsEdgeNormalY(NULL),
  meshSurfacewaterMeshNeighborsConnection(NULL),
  meshSurfacewaterMeshNeighborsFlowRate(NULL),
  meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  meshGroundwaterMeshNeighborsConnection(NULL),
  meshGroundwaterMeshNeighborsFlowRate(NULL),
  meshGroundwaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  meshGroundwaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  meshChannelNeighbors(NULL),
  meshChannelNeighborsRegion(NULL),
  meshChannelNeighborsEdgeLength(NULL),
  meshSurfacewaterChannelNeighborsConnection(NULL),
  meshSurfacewaterChannelNeighborsFlowRate(NULL),
  meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm(NULL),
  meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm(NULL),
  meshGroundwaterChannelNeighborsConnection(NULL),
  meshGroundwaterChannelNeighborsFlowRate(NULL),
  meshGroundwaterChannelNeighborsFlowCumulativeShortTerm(NULL),
  meshGroundwaterChannelNeighborsFlowCumulativeLongTerm(NULL),
  channelNodeX(NULL),
  channelNodeY(NULL),
  channelNodeZBank(NULL),
  channelElementVertices(NULL),
  channelVertexX(NULL),
  channelVertexY(NULL),
  channelVertexZBank(NULL),
  channelRegion(NULL),
  channelChannelType(NULL),
  channelReachCode(NULL),
  channelElementX(NULL),
  channelElementY(NULL),
  channelElementZBank(NULL),
  channelElementBankFullDepth(NULL),
  channelElementZBed(NULL),
  channelElementLength(NULL),
  channelLatitude(NULL),
  channelLongitude(NULL),
  channelBaseWidth(NULL),
  channelSideSlope(NULL),
  channelBedConductivity(NULL),
  channelBedThickness(NULL),
  channelManningsN(NULL),
  channelSurfacewaterDepth(NULL),
  channelSurfacewaterError(NULL),
  channelPrecipitationRate(NULL),
  channelPrecipitationCumulativeShortTerm(NULL),
  channelPrecipitationCumulativeLongTerm(NULL),
  channelEvaporationRate(NULL),
  channelEvaporationCumulativeShortTerm(NULL),
  channelEvaporationCumulativeLongTerm(NULL),
  channelEvapoTranspirationState(NULL),
  channelMeshNeighbors(NULL),
  channelMeshNeighborsRegion(NULL),
  channelMeshNeighborsEdgeLength(NULL),
  channelSurfacewaterMeshNeighborsConnection(NULL),
  channelSurfacewaterMeshNeighborsFlowRate(NULL),
  channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  channelGroundwaterMeshNeighborsConnection(NULL),
  channelGroundwaterMeshNeighborsFlowRate(NULL),
  channelGroundwaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  channelGroundwaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  channelChannelNeighbors(NULL),
  channelChannelNeighborsRegion(NULL),
  channelChannelNeighborsDownstream(NULL),
  channelSurfacewaterChannelNeighborsConnection(NULL),
  channelSurfacewaterChannelNeighborsFlowRate(NULL),
  channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm(NULL),
  channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm(NULL),
  channelPrunedSize(0),
  numberOfChannelPruned(0),
  channelPruned(NULL),
  meshNodeLocation(),
  channelNodeLocation(),
  meshVertexUpdated(NULL),
  channelVertexUpdated(NULL)
{
  // Initialization will be done in runUntilSimulationEnd.
  thisProxy[CkMyPe()].runUntilSimulationEnd();
}

FileManager::~FileManager()
{
  delete[] regionNumberOfMeshElements;
  delete[] regionNumberOfChannelElements;
  delete[] meshNodeX;
  delete[] meshNodeY;
  delete[] meshNodeZSurface;
  delete[] meshElementVertices;
  delete[] meshVertexX;
  delete[] meshVertexY;
  delete[] meshVertexZSurface;
  delete[] meshRegion;
  delete[] meshCatchment;
  delete[] meshVegetationType;
  delete[] meshSoilType;
  delete[] meshAlluvium;
  delete[] meshElementX;
  delete[] meshElementY;
  delete[] meshElementZSurface;
  delete[] meshElementSoilDepth;
  delete[] meshElementLayerZBottom;
  delete[] meshElementArea;
  delete[] meshElementSlopeX;
  delete[] meshElementSlopeY;
  delete[] meshLatitude;
  delete[] meshLongitude;
  delete[] meshManningsN;
  delete[] meshConductivity;
  delete[] meshPorosity;
  delete[] meshSurfacewaterDepth;
  delete[] meshSurfacewaterError;
  delete[] meshGroundwaterHead;
  delete[] meshGroundwaterRecharge;
  delete[] meshGroundwaterError;
  delete[] meshPrecipitationRate;
  delete[] meshPrecipitationCumulativeShortTerm;
  delete[] meshPrecipitationCumulativeLongTerm;
  delete[] meshEvaporationRate;
  delete[] meshEvaporationCumulativeShortTerm;
  delete[] meshEvaporationCumulativeLongTerm;
  delete[] meshTranspirationRate;
  delete[] meshTranspirationCumulativeShortTerm;
  delete[] meshTranspirationCumulativeLongTerm;
  delete[] meshEvapoTranspirationState;
  delete[] meshInfiltrationMethod;
  delete[] meshGroundwaterMethod;
  delete[] meshMeshNeighbors;
  delete[] meshMeshNeighborsRegion;
  delete[] meshMeshNeighborsChannelEdge;
  delete[] meshMeshNeighborsEdgeLength;
  delete[] meshMeshNeighborsEdgeNormalX;
  delete[] meshMeshNeighborsEdgeNormalY;
  delete[] meshSurfacewaterMeshNeighborsConnection;
  delete[] meshSurfacewaterMeshNeighborsFlowRate;
  delete[] meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] meshGroundwaterMeshNeighborsConnection;
  delete[] meshGroundwaterMeshNeighborsFlowRate;
  delete[] meshGroundwaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] meshGroundwaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] meshChannelNeighbors;
  delete[] meshChannelNeighborsRegion;
  delete[] meshChannelNeighborsEdgeLength;
  delete[] meshSurfacewaterChannelNeighborsConnection;
  delete[] meshSurfacewaterChannelNeighborsFlowRate;
  delete[] meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm;
  delete[] meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm;
  delete[] meshGroundwaterChannelNeighborsConnection;
  delete[] meshGroundwaterChannelNeighborsFlowRate;
  delete[] meshGroundwaterChannelNeighborsFlowCumulativeShortTerm;
  delete[] meshGroundwaterChannelNeighborsFlowCumulativeLongTerm;
  delete[] channelNodeX;
  delete[] channelNodeY;
  delete[] channelNodeZBank;
  delete[] channelElementVertices;
  delete[] channelVertexX;
  delete[] channelVertexY;
  delete[] channelVertexZBank;
  delete[] channelRegion;
  delete[] channelChannelType;
  delete[] channelReachCode;
  delete[] channelElementX;
  delete[] channelElementY;
  delete[] channelElementZBank;
  delete[] channelElementBankFullDepth;
  delete[] channelElementZBed;
  delete[] channelElementLength;
  delete[] channelLatitude;
  delete[] channelLongitude;
  delete[] channelBaseWidth;
  delete[] channelSideSlope;
  delete[] channelBedConductivity;
  delete[] channelBedThickness;
  delete[] channelManningsN;
  delete[] channelSurfacewaterDepth;
  delete[] channelSurfacewaterError;
  delete[] channelPrecipitationRate;
  delete[] channelPrecipitationCumulativeShortTerm;
  delete[] channelPrecipitationCumulativeLongTerm;
  delete[] channelEvaporationRate;
  delete[] channelEvaporationCumulativeShortTerm;
  delete[] channelEvaporationCumulativeLongTerm;
  delete[] channelEvapoTranspirationState;
  delete[] channelMeshNeighbors;
  delete[] channelMeshNeighborsRegion;
  delete[] channelMeshNeighborsEdgeLength;
  delete[] channelSurfacewaterMeshNeighborsConnection;
  delete[] channelSurfacewaterMeshNeighborsFlowRate;
  delete[] channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm;;
  delete[] channelGroundwaterMeshNeighborsConnection;
  delete[] channelGroundwaterMeshNeighborsFlowRate;
  delete[] channelGroundwaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] channelGroundwaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] channelChannelNeighbors;
  delete[] channelChannelNeighborsRegion;
  delete[] channelChannelNeighborsDownstream;
  delete[] channelSurfacewaterChannelNeighborsConnection;
  delete[] channelSurfacewaterChannelNeighborsFlowRate;
  delete[] channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm;
  delete[] channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm;
  delete[] channelPruned;
  delete[] meshVertexUpdated;
  delete[] channelVertexUpdated;
}

bool FileManager::readNodeAndZFiles(bool readMesh)
{
  bool   error      = false;  // Error flag.
  int    ii;                  // Loop counter.
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

  // Open file.
  nodeFile = fopen(readMesh ? ADHydro::ASCIIInputMeshNodeFilePath.c_str() : ADHydro::ASCIIInputChannelNodeFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != nodeFile))
    {
      CkError("ERROR in FileManager::readNodeAndZFiles: could not open node file %s.\n", readMesh ? ADHydro::ASCIIInputMeshNodeFilePath.c_str() :
                                                                                                    ADHydro::ASCIIInputChannelNodeFilePath.c_str());
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  
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
  
  // Open file.
  if (!error)
    {
      zFile = fopen(readMesh ? ADHydro::ASCIIInputMeshZFilePath.c_str() : ADHydro::ASCIIInputChannelZFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != zFile))
        {
          CkError("ERROR in FileManager::readNodeAndZFiles: could not open z file %s.\n", readMesh ? ADHydro::ASCIIInputMeshZFilePath.c_str() :
                                                                                                     ADHydro::ASCIIInputChannelZFilePath.c_str());
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
  
  return error;
}

void FileManager::initializeFromASCIIFiles()
{
  bool               error         = false;    // Error flag.
  int                ii, jj, kk;               // Loop counters.
  size_t             numScanned;               // Used to check that fscanf scanned the correct number of inputs.
  FILE*              eleFile       = NULL;     // The ele file to read from.
  FILE*              neighFile     = NULL;     // The neigh file to read from.
  FILE*              landCoverFile = NULL;     // The landCover file to read from.
  FILE*              soilTypeFile  = NULL;     // The soilType file to read from.
  FILE*              geolTypeFile  = NULL;     // The geolType file to read from.
  FILE*              edgeFile      = NULL;     // The edge file to read from.
  FILE*              chanEleFile   = NULL;     // The chan.ele file to read from.
  FILE*              chanPruneFile = NULL;     // The chan.prune file to read from.
  int                dimension;                // Used to check the dimensions in the files.
  int                numberOfAttributes;       // Used to check the number of attributes in the files.
  int                numberCheck;              // Used to check numbers that are error checked but otherwise unused.
  int                index;                    // Used to read element and edge numbers.
  int                vertex0;                  // Used to read vertices from the files.
  int                vertex1;                  // Used to read vertices from the files.
  int                vertex2;                  // Used to read vertices from the files.
  int                catchment;                // Used to read catchments from the files.
  int                vegetationType;           // Used to read vegetation type from the files.
  int                numberOfSoilLayers;       // Used to read the number of soil layers from the files.
  int                soilTypeReader;           // Used to read soil type from the files.
  int                soilType      = -1;       // Used to store soil type of the top layer.
  double             soilDepthReader;          // Used to read each soil layers thickness.
  double             soilDepth     = 0.0;      // Used to store the sum of each soil layers thickness.
  int                alluvium;                 // Used to read whether an element has an alluvial aquifer.
  int                neighbor0;                // Used to read neighbors from the files.
  int                neighbor1;                // Used to read neighbors from the files.
  int                neighbor2;                // Used to read neighbors from the files.
  int                numberOfEdges;            // The number of edges to read.
  int                boundary;                 // Used to check the number of boundary markers in the files.
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

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (isnan(ADHydro::centralMeridian))
    {
      CkError("ERROR in FileManager::initializeFromASCIIFiles: centralMeridian must be set in the superfile.  It is not specified in the ASCII input files.\n");
      error = true;
    }
  
  if (isnan(ADHydro::falseEasting))
    {
      CkError("ERROR in FileManager::initializeFromASCIIFiles: falseEasting must be set in the superfile.  It is not specified in the ASCII input files.\n");
      error = true;
    }
  
  if (isnan(ADHydro::falseNorthing))
    {
      CkError("ERROR in FileManager::initializeFromASCIIFiles: falseNorthing must be set in the superfile.  It is not specified in the ASCII input files.\n");
      error = true;
    }
  
  if (isnan(ADHydro::referenceDate))
    {
      CkError("ERROR in FileManager::initializeFromASCIIFiles: referenceDate must be set in the superfile.  It is not specified in the ASCII input files.\n");
      error = true;
    }
  
  if (isnan(ADHydro::currentTime))
    {
      CkError("ERROR in FileManager::initializeFromASCIIFiles: currentTime must be set in the superfile.  It is not specified in the ASCII input files.\n");
      error = true;
    }
  
  if (isnan(ADHydro::dt))
    {
      CkError("ERROR in FileManager::initializeFromASCIIFiles: dt must be set in the superfile.  It is not specified in the ASCII input files.\n");
      error = true;
    }
  
  if (-1 == ADHydro::iteration)
    {
      CkError("ERROR in FileManager::initializeFromASCIIFiles: iteration must be set in the superfile.  It is not specified in the ASCII input files.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

  // Read mesh nodes.
  if (!error)
    {
      error = readNodeAndZFiles(true);
    }

  // Open file.
  if (!error)
    {
      eleFile = fopen(ADHydro::ASCIIInputMeshElementFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != eleFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open ele file %s.\n", ADHydro::ASCIIInputMeshElementFilePath.c_str());
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
  
  // Open file.
  if (!error)
    {
      neighFile = fopen(ADHydro::ASCIIInputMeshNeighborFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != neighFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open neigh file %s.\n", ADHydro::ASCIIInputMeshNeighborFilePath.c_str());
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

  // Open file.
  if (!error)
    {
      landCoverFile = fopen(ADHydro::ASCIIInputMeshLandCoverFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != landCoverFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open landCover file %s.\n", ADHydro::ASCIIInputMeshLandCoverFilePath.c_str());
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

  // Open file.
  if (!error)
    {
      soilTypeFile = fopen(ADHydro::ASCIIInputMeshSoilTypeFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != soilTypeFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open soilType file %s.\n", ADHydro::ASCIIInputMeshSoilTypeFilePath.c_str());
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Read header.
  if (!error)
    {
      numScanned = fscanf(soilTypeFile, "%d", &numberCheck);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(1 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read header from soilType file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid header in soilType file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Open file.
  if (!error)
    {
      geolTypeFile = fopen(ADHydro::ASCIIInputMeshGeolTypeFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != geolTypeFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open geolType file %s.\n", ADHydro::ASCIIInputMeshGeolTypeFilePath.c_str());
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Read header.
  if (!error)
    {
      numScanned = fscanf(geolTypeFile, "%d", &numberCheck);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(1 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read header from geolType file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid header in geolType file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Calculate local start and number and allocate arrays.
  if (!error)
    {
      localStartAndNumber(&localMeshElementStart, &localNumberOfMeshElements, globalNumberOfMeshElements);

      meshElementVertices            = new int[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      meshCatchment                  = new int[localNumberOfMeshElements];
      meshVegetationType             = new int[localNumberOfMeshElements];
      meshSoilType                   = new int[localNumberOfMeshElements];
      meshAlluvium                   = new bool[localNumberOfMeshElements];
      meshElementSoilDepth           = new double[localNumberOfMeshElements];
      meshMeshNeighbors              = new int[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      meshMeshNeighborsChannelEdge   = new bool[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      meshChannelNeighbors           = new int[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      meshChannelNeighborsEdgeLength = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
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
      numScanned = fscanf(soilTypeFile, "%d %d", &numberCheck, &numberOfSoilLayers);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from soilType file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in soilType file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }

      if (!(0 <= numberOfSoilLayers))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid number of soil layers %d in soilType file.\n", index,
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
              numScanned = fscanf(soilTypeFile, "%d,%lf", &soilTypeReader, &soilDepthReader);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(2 == numScanned))
                {
                  CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d soil layer %d from soilType file.\n", ii, jj);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
              if (!((1 <= soilTypeReader && 19 >= soilTypeReader) || -1 == soilTypeReader))
                {
                  CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: mesh element %d: invalid soil type %s in soilType file.\n", index,
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

      // Read geolType file.
      numScanned = fscanf(geolTypeFile, "%d %d", &numberCheck, &alluvium);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: unable to read entry %d from geolType file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: invalid element number in geolType file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Save values.
      if (!error && localMeshElementStart <= index && index < localMeshElementStart + localNumberOfMeshElements)
        {
          meshElementVertices[ index - localMeshElementStart][0] = vertex0;
          meshElementVertices[ index - localMeshElementStart][1] = vertex1;
          meshElementVertices[ index - localMeshElementStart][2] = vertex2;
          meshCatchment[       index - localMeshElementStart]    = catchment - 2;  // The .ele file stores catchment number plus two because zero and one are
                                                                                   // used by triangle.
          meshVegetationType[  index - localMeshElementStart]    = vegetationType;
          meshSoilType[        index - localMeshElementStart]    = soilType;
          meshAlluvium[        index - localMeshElementStart]    = (0 != alluvium);
          meshElementSoilDepth[index - localMeshElementStart]    = soilDepth;
          meshMeshNeighbors[   index - localMeshElementStart][0] = neighbor0;
          meshMeshNeighbors[   index - localMeshElementStart][1] = neighbor1;
          meshMeshNeighbors[   index - localMeshElementStart][2] = neighbor2;

          // Channel edges and neighbors are filled in later when we read the edge and channel files.  Initialize them here to no channels.
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshMeshNeighborsChannelEdge[index - localMeshElementStart][jj] = false;
            }

          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshChannelNeighbors[          index - localMeshElementStart][jj] = NOFLOW;
              meshChannelNeighborsEdgeLength[index - localMeshElementStart][jj] = 0.0;
            }
        }
    } // End read mesh elements.
  
  // Open file.
  if (!error)
    {
      edgeFile = fopen(ADHydro::ASCIIInputMeshEdgeFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != edgeFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open edge file %s.\n", ADHydro::ASCIIInputMeshEdgeFilePath.c_str());
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
      error = readNodeAndZFiles(false);
    }
  
  // Open file.
  if (!error)
    {
      chanEleFile = fopen(ADHydro::ASCIIInputChannelElementFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != chanEleFile))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open chan.ele file %s.\n", ADHydro::ASCIIInputChannelElementFilePath.c_str());
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
      channelChannelType                = new ChannelTypeEnum[localNumberOfChannelElements];
      channelReachCode                  = new long long[localNumberOfChannelElements];
      channelElementBankFullDepth       = new double[localNumberOfChannelElements];
      channelElementLength              = new double[localNumberOfChannelElements];
      channelBaseWidth                  = new double[localNumberOfChannelElements];
      channelSideSlope                  = new double[localNumberOfChannelElements];
      channelMeshNeighbors              = new int[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      channelMeshNeighborsEdgeLength    = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      channelChannelNeighbors           = new int[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      channelChannelNeighborsDownstream = new bool[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
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
      
      if (!(0 < numberOfVertices && numberOfVertices <= CHANNEL_ELEMENT_VERTICES_SIZE))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: numberOfVertices must be greater than zero and less than or "
                  "equal to the maximum number of channel vertices %d in chan.ele file.\n", index, CHANNEL_ELEMENT_VERTICES_SIZE);
          error = true;
        }
      
      if (!(0 <= numberOfChannelNeighbors && numberOfChannelNeighbors <= CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: numberOfChannelNeighbors must be greater than or equal to zero and less "
                  "than or equal to the maximum number of channel channel neighbors %d in chan.ele file.\n", index, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE);
          error = true;
        }
      
      if (!(0 <= numberOfMeshNeighbors && numberOfMeshNeighbors <= CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE))
        {
          CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: channel %d: numberOfMeshNeighbors must be greater than or equal to zero and less "
                  "than or equal to the maximum number of channel mesh neighbors %d in chan.ele file.\n", index, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Save values.
      if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
        {
          // Use polyline (code 2) for STREAM and polygon (code 3) for WATERBODY and ICEMASS
          channelElementVertices[     index - localChannelElementStart][0] = (STREAM == type) ? 2 : 3;
          channelElementVertices[     index - localChannelElementStart][1] = CHANNEL_ELEMENT_VERTICES_SIZE;
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
              channelElementVertices[index - localChannelElementStart][jj + 2] = vertex0; // +2 to skip over XDMF metadata.
            }
        }
      
      // Fill in unused vertices by repeating the last used vertex.
      if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
        {
          for (jj = numberOfVertices; jj < CHANNEL_ELEMENT_VERTICES_SIZE; jj++)
            {
              channelElementVertices[index - localChannelElementStart][jj + 2] =
                  channelElementVertices[index - localChannelElementStart][(numberOfVertices - 1) + 2]; // +2 to skip over XDMF metadata.
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
          for (jj = numberOfChannelNeighbors; jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
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

              while (kk < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE && NOFLOW != meshChannelNeighbors[neighbor0 - localMeshElementStart][kk])
                {
                  kk++;
                }

              if (kk < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE)
                {
                  meshChannelNeighbors[          neighbor0 - localMeshElementStart][kk] = index;
                  meshChannelNeighborsEdgeLength[neighbor0 - localMeshElementStart][kk] = length;
                }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
              else
                {
                  CkError("ERROR in handleInitializeFromASCIIFiles: mesh element %d: number of channel neighbors exceeds maximum number %d.\n", neighbor0,
                      MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
            }
        }
      
      // Fill in unused mesh neighbors with NOFLOW.
      if (!error && localChannelElementStart <= index && index < localChannelElementStart + localNumberOfChannelElements)
        {
          for (jj = numberOfMeshNeighbors; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelMeshNeighbors[          index - localChannelElementStart][jj] = NOFLOW;
              channelMeshNeighborsEdgeLength[index - localChannelElementStart][jj] = 0.0;
            }
        }
    } // End read channel elements.

  // channelPruned is only used for mesh massage so only read it in if you are doing mesh massage.
  if (ADHydro::doMeshMassage)
    {
      // Open file.
      if (!error)
        {
          chanPruneFile = fopen(ADHydro::ASCIIInputChannelPruneFilePath.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NULL != chanPruneFile))
            {
              CkError("ERROR in FileManager::handleInitializeFromASCIIFiles: could not open chan.prune file %s.\n", ADHydro::ASCIIInputChannelPruneFilePath.c_str());
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

  if (NULL != soilTypeFile)
    {
      fclose(soilTypeFile);
    }

  if (NULL != geolTypeFile)
    {
      fclose(geolTypeFile);
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

void FileManager::initializeFromNetCDFFiles()
{
  // FIXME not implemented
  setUpHardcodedMesh();
}

// FIXME remove
void FileManager::setUpHardcodedMesh()
{
  if (0 == CkMyPe())
    {
      globalNumberOfRegions         = 3;
      localRegionStart              = 0;
      localNumberOfRegions          = 3;
      globalNumberOfMeshNodes       = 14;
      localMeshNodeStart            = 0;
      localNumberOfMeshNodes        = 14;
      globalNumberOfMeshElements    = 12;
      localMeshElementStart         = 0;
      localNumberOfMeshElements     = 12;
      globalNumberOfChannelNodes    = 5;
      localChannelNodeStart         = 0;
      localNumberOfChannelNodes     = 5;
      globalNumberOfChannelElements = 4;
      localChannelElementStart      = 0;
      localNumberOfChannelElements  = 4;
      
      meshNodeX        = new double[14];
      meshNodeY        = new double[14];
      meshNodeZSurface = new double[14];
      
      meshNodeX[0]         =    0.0;
      meshNodeY[0]         =  600.0;
      meshNodeZSurface[0]  =    0.0;
      meshNodeX[1]         =  600.0;
      meshNodeY[1]         =  600.0;
      meshNodeZSurface[1]  =    6.0;
      meshNodeX[2]         = 1200.0;
      meshNodeY[2]         =  600.0;
      meshNodeZSurface[2]  =   12.0;
      meshNodeX[3]         = 1800.0;
      meshNodeY[3]         =  600.0;
      meshNodeZSurface[3]  =   18.0;
      meshNodeX[4]         = 2400.0;
      meshNodeY[4]         =  600.0;
      meshNodeZSurface[4]  =   24.0;
      meshNodeX[5]         = 3000.0;
      meshNodeY[5]         =  600.0;
      meshNodeZSurface[5]  =   30.0;
      meshNodeX[6]         = 3600.0;
      meshNodeY[6]         =  600.0;
      meshNodeZSurface[6]  =   36.0;
      meshNodeX[7]         =    0.0;
      meshNodeY[7]         =    0.0;
      meshNodeZSurface[7]  =    0.0;
      meshNodeX[8]         =  600.0;
      meshNodeY[8]         =    0.0;
      meshNodeZSurface[8]  =    6.0;
      meshNodeX[9]         = 1200.0;
      meshNodeY[9]         =    0.0;
      meshNodeZSurface[9]  =   12.0;
      meshNodeX[10]        = 1800.0;
      meshNodeY[10]        =    0.0;
      meshNodeZSurface[10] =   18.0;
      meshNodeX[11]        = 2400.0;
      meshNodeY[11]        =    0.0;
      meshNodeZSurface[11] =   24.0;
      meshNodeX[12]        = 3000.0;
      meshNodeY[12]        =    0.0;
      meshNodeZSurface[12] =   30.0;
      meshNodeX[13]        = 3600.0;
      meshNodeY[13]        =    0.0;
      meshNodeZSurface[13] =   36.0;
      
      meshElementVertices = new IntArrayMMN[12];
      
      meshElementVertices[0][0] = 0;
      meshElementVertices[0][1] = 7;
      meshElementVertices[0][2] = 8;
      meshElementVertices[1][0] = 0;
      meshElementVertices[1][1] = 8;
      meshElementVertices[1][2] = 1;
      meshElementVertices[2][0] = 1;
      meshElementVertices[2][1] = 8;
      meshElementVertices[2][2] = 9;
      meshElementVertices[3][0] = 1;
      meshElementVertices[3][1] = 9;
      meshElementVertices[3][2] = 2;
      meshElementVertices[4][0] = 2;
      meshElementVertices[4][1] = 9;
      meshElementVertices[4][2] = 10;
      meshElementVertices[5][0] = 2;
      meshElementVertices[5][1] = 10;
      meshElementVertices[5][2] = 3;
      meshElementVertices[6][0] = 3;
      meshElementVertices[6][1] = 10;
      meshElementVertices[6][2] = 11;
      meshElementVertices[7][0] = 3;
      meshElementVertices[7][1] = 11;
      meshElementVertices[7][2] = 4;
      meshElementVertices[8][0] = 4;
      meshElementVertices[8][1] = 11;
      meshElementVertices[8][2] = 12;
      meshElementVertices[9][0] = 4;
      meshElementVertices[9][1] = 12;
      meshElementVertices[9][2] = 5;
      meshElementVertices[10][0] = 5;
      meshElementVertices[10][1] = 12;
      meshElementVertices[10][2] = 13;
      meshElementVertices[11][0] = 5;
      meshElementVertices[11][1] = 13;
      meshElementVertices[11][2] = 6;
      
      meshRegion = new int[12];
      
      meshRegion[0]  = 0;
      meshRegion[1]  = 0;
      meshRegion[2]  = 0;
      meshRegion[3]  = 0;
      meshRegion[4]  = 1;
      meshRegion[5]  = 1;
      meshRegion[6]  = 1;
      meshRegion[7]  = 1;
      meshRegion[8]  = 2;
      meshRegion[9]  = 2;
      meshRegion[10] = 2;
      meshRegion[11] = 2;
      
      meshCatchment = new int[12];
      
      meshCatchment[0]  = 0;
      meshCatchment[1]  = 0;
      meshCatchment[2]  = 0;
      meshCatchment[3]  = 0;
      meshCatchment[4]  = 1;
      meshCatchment[5]  = 1;
      meshCatchment[6]  = 1;
      meshCatchment[7]  = 1;
      meshCatchment[8]  = 2;
      meshCatchment[9]  = 2;
      meshCatchment[10] = 2;
      meshCatchment[11] = 2;
      
      meshVegetationType = new int[12];
      
      meshVegetationType[0]  = 8;
      meshVegetationType[1]  = 8;
      meshVegetationType[2]  = 8;
      meshVegetationType[3]  = 8;
      meshVegetationType[4]  = 8;
      meshVegetationType[5]  = 8;
      meshVegetationType[6]  = 8;
      meshVegetationType[7]  = 8;
      meshVegetationType[8]  = 8;
      meshVegetationType[9]  = 8;
      meshVegetationType[10] = 8;
      meshVegetationType[11] = 8;
      
      meshSoilType = new int[12];
      
      meshSoilType[0]  = 1;
      meshSoilType[1]  = 1;
      meshSoilType[2]  = 1;
      meshSoilType[3]  = 1;
      meshSoilType[4]  = 1;
      meshSoilType[5]  = 1;
      meshSoilType[6]  = 1;
      meshSoilType[7]  = 1;
      meshSoilType[8]  = 1;
      meshSoilType[9]  = 1;
      meshSoilType[10] = 1;
      meshSoilType[11] = 1;
      
      meshAlluvium = new bool[12];
      
      meshAlluvium[0]  = true;
      meshAlluvium[1]  = true;
      meshAlluvium[2]  = true;
      meshAlluvium[3]  = true;
      meshAlluvium[4]  = true;
      meshAlluvium[5]  = true;
      meshAlluvium[6]  = true;
      meshAlluvium[7]  = true;
      meshAlluvium[8]  = true;
      meshAlluvium[9]  = true;
      meshAlluvium[10] = true;
      meshAlluvium[11] = true;
      
      meshElementSoilDepth = new double[12];
      
      meshElementSoilDepth[0]  = 1.0;
      meshElementSoilDepth[1]  = 1.0;
      meshElementSoilDepth[2]  = 1.0;
      meshElementSoilDepth[3]  = 1.0;
      meshElementSoilDepth[4]  = 1.0;
      meshElementSoilDepth[5]  = 1.0;
      meshElementSoilDepth[6]  = 1.0;
      meshElementSoilDepth[7]  = 1.0;
      meshElementSoilDepth[8]  = 1.0;
      meshElementSoilDepth[9]  = 1.0;
      meshElementSoilDepth[10] = 1.0;
      meshElementSoilDepth[11] = 1.0;
      
      meshMeshNeighbors = new IntArrayMMN[12];
      
      meshMeshNeighbors[0][0]  = NOFLOW;
      meshMeshNeighbors[0][1]  = 1;
      meshMeshNeighbors[0][2]  = OUTFLOW;
      meshMeshNeighbors[1][0]  = 2;
      meshMeshNeighbors[1][1]  = NOFLOW;
      meshMeshNeighbors[1][2]  = 0;
      meshMeshNeighbors[2][0]  = NOFLOW;
      meshMeshNeighbors[2][1]  = 3;
      meshMeshNeighbors[2][2]  = 1;
      meshMeshNeighbors[3][0]  = 4;
      meshMeshNeighbors[3][1]  = NOFLOW;
      meshMeshNeighbors[3][2]  = 2;
      meshMeshNeighbors[4][0]  = NOFLOW;
      meshMeshNeighbors[4][1]  = 5;
      meshMeshNeighbors[4][2]  = 3;
      meshMeshNeighbors[5][0]  = 6;
      meshMeshNeighbors[5][1]  = NOFLOW;
      meshMeshNeighbors[5][2]  = 4;
      meshMeshNeighbors[6][0]  = NOFLOW;
      meshMeshNeighbors[6][1]  = 7;
      meshMeshNeighbors[6][2]  = 5;
      meshMeshNeighbors[7][0]  = 8;
      meshMeshNeighbors[7][1]  = NOFLOW;
      meshMeshNeighbors[7][2]  = 6;
      meshMeshNeighbors[8][0]  = NOFLOW;
      meshMeshNeighbors[8][1]  = 9;
      meshMeshNeighbors[8][2]  = 7;
      meshMeshNeighbors[9][0]  = 10;
      meshMeshNeighbors[9][1]  = NOFLOW;
      meshMeshNeighbors[9][2]  = 8;
      meshMeshNeighbors[10][0] = NOFLOW;
      meshMeshNeighbors[10][1] = 11;
      meshMeshNeighbors[10][2] = 9;
      meshMeshNeighbors[11][0] = NOFLOW;
      meshMeshNeighbors[11][1] = NOFLOW;
      meshMeshNeighbors[11][2] = 10;
      
      meshMeshNeighborsRegion = new IntArrayMMN[12];
      
      meshMeshNeighborsRegion[0][0]  = 0;
      meshMeshNeighborsRegion[0][1]  = 0;
      meshMeshNeighborsRegion[0][2]  = 0;
      meshMeshNeighborsRegion[1][0]  = 0;
      meshMeshNeighborsRegion[1][1]  = 0;
      meshMeshNeighborsRegion[1][2]  = 0;
      meshMeshNeighborsRegion[2][0]  = 0;
      meshMeshNeighborsRegion[2][1]  = 0;
      meshMeshNeighborsRegion[2][2]  = 0;
      meshMeshNeighborsRegion[3][0]  = 1;
      meshMeshNeighborsRegion[3][1]  = 0;
      meshMeshNeighborsRegion[3][2]  = 0;
      meshMeshNeighborsRegion[4][0]  = 0;
      meshMeshNeighborsRegion[4][1]  = 1;
      meshMeshNeighborsRegion[4][2]  = 0;
      meshMeshNeighborsRegion[5][0]  = 1;
      meshMeshNeighborsRegion[5][1]  = 0;
      meshMeshNeighborsRegion[5][2]  = 1;
      meshMeshNeighborsRegion[6][0]  = 0;
      meshMeshNeighborsRegion[6][1]  = 1;
      meshMeshNeighborsRegion[6][2]  = 1;
      meshMeshNeighborsRegion[7][0]  = 2;
      meshMeshNeighborsRegion[7][1]  = 0;
      meshMeshNeighborsRegion[7][2]  = 1;
      meshMeshNeighborsRegion[8][0]  = 0;
      meshMeshNeighborsRegion[8][1]  = 2;
      meshMeshNeighborsRegion[8][2]  = 1;
      meshMeshNeighborsRegion[9][0]  = 2;
      meshMeshNeighborsRegion[9][1]  = 0;
      meshMeshNeighborsRegion[9][2]  = 2;
      meshMeshNeighborsRegion[10][0] = 0;
      meshMeshNeighborsRegion[10][1] = 2;
      meshMeshNeighborsRegion[10][2] = 2;
      meshMeshNeighborsRegion[11][0] = 0;
      meshMeshNeighborsRegion[11][1] = 0;
      meshMeshNeighborsRegion[11][2] = 2;
      
      meshChannelNeighbors = new IntArrayMCN[12];
      
      meshChannelNeighbors[0][0]  = 0;
      meshChannelNeighbors[0][1]  = NOFLOW;
      meshChannelNeighbors[1][0]  = NOFLOW;
      meshChannelNeighbors[1][1]  = NOFLOW;
      meshChannelNeighbors[2][0]  = 0;
      meshChannelNeighbors[2][1]  = 1;
      meshChannelNeighbors[3][0]  = NOFLOW;
      meshChannelNeighbors[3][1]  = NOFLOW;
      meshChannelNeighbors[4][0]  = 1;
      meshChannelNeighbors[4][1]  = NOFLOW;
      meshChannelNeighbors[5][0]  = NOFLOW;
      meshChannelNeighbors[5][1]  = NOFLOW;
      meshChannelNeighbors[6][0]  = 2;
      meshChannelNeighbors[6][1]  = NOFLOW;
      meshChannelNeighbors[7][0]  = NOFLOW;
      meshChannelNeighbors[7][1]  = NOFLOW;
      meshChannelNeighbors[8][0]  = 2;
      meshChannelNeighbors[8][1]  = 3;
      meshChannelNeighbors[9][0]  = NOFLOW;
      meshChannelNeighbors[9][1]  = NOFLOW;
      meshChannelNeighbors[10][0] = 3;
      meshChannelNeighbors[10][1] = NOFLOW;
      meshChannelNeighbors[11][0] = NOFLOW;
      meshChannelNeighbors[11][1] = NOFLOW;
      
      meshChannelNeighborsRegion = new IntArrayMCN[12];
      
      meshChannelNeighborsRegion[0][0]  = 0;
      meshChannelNeighborsRegion[0][1]  = 0;
      meshChannelNeighborsRegion[1][0]  = 0;
      meshChannelNeighborsRegion[1][1]  = 0;
      meshChannelNeighborsRegion[2][0]  = 0;
      meshChannelNeighborsRegion[2][1]  = 1;
      meshChannelNeighborsRegion[3][0]  = 0;
      meshChannelNeighborsRegion[3][1]  = 0;
      meshChannelNeighborsRegion[4][0]  = 1;
      meshChannelNeighborsRegion[4][1]  = 0;
      meshChannelNeighborsRegion[5][0]  = 0;
      meshChannelNeighborsRegion[5][1]  = 0;
      meshChannelNeighborsRegion[6][0]  = 1;
      meshChannelNeighborsRegion[6][1]  = 0;
      meshChannelNeighborsRegion[7][0]  = 0;
      meshChannelNeighborsRegion[7][1]  = 0;
      meshChannelNeighborsRegion[8][0]  = 1;
      meshChannelNeighborsRegion[8][1]  = 2;
      meshChannelNeighborsRegion[9][0]  = 0;
      meshChannelNeighborsRegion[9][1]  = 0;
      meshChannelNeighborsRegion[10][0] = 2;
      meshChannelNeighborsRegion[10][1] = 0;
      meshChannelNeighborsRegion[11][0] = 0;
      meshChannelNeighborsRegion[11][1] = 0;
      
      meshChannelNeighborsEdgeLength = new DoubleArrayMCN[12];
      
      meshChannelNeighborsEdgeLength[0][0]  = 600.0;
      meshChannelNeighborsEdgeLength[0][1]  = 0.0;
      meshChannelNeighborsEdgeLength[1][0]  = 0.0;
      meshChannelNeighborsEdgeLength[1][1]  = 0.0;
      meshChannelNeighborsEdgeLength[2][0]  = 300.0;
      meshChannelNeighborsEdgeLength[2][1]  = 300.0;
      meshChannelNeighborsEdgeLength[3][0]  = 0.0;
      meshChannelNeighborsEdgeLength[3][1]  = 0.0;
      meshChannelNeighborsEdgeLength[4][0]  = 600.0;
      meshChannelNeighborsEdgeLength[4][1]  = 0.0;
      meshChannelNeighborsEdgeLength[5][0]  = 0.0;
      meshChannelNeighborsEdgeLength[5][1]  = 0.0;
      meshChannelNeighborsEdgeLength[6][0]  = 600.0;
      meshChannelNeighborsEdgeLength[6][1]  = 0.0;
      meshChannelNeighborsEdgeLength[7][0]  = 0.0;
      meshChannelNeighborsEdgeLength[7][1]  = 0.0;
      meshChannelNeighborsEdgeLength[8][0]  = 300.0;
      meshChannelNeighborsEdgeLength[8][1]  = 300.0;
      meshChannelNeighborsEdgeLength[9][0]  = 0.0;
      meshChannelNeighborsEdgeLength[9][1]  = 0.0;
      meshChannelNeighborsEdgeLength[10][0] = 600.0;
      meshChannelNeighborsEdgeLength[10][1] = 0.0;
      meshChannelNeighborsEdgeLength[11][0] = 0.0;
      meshChannelNeighborsEdgeLength[11][1] = 0.0;
      
      channelNodeX     = new double[5];
      channelNodeY     = new double[5];
      channelNodeZBank = new double[5];
      
      channelNodeX[0]     =    0.0;
      channelNodeY[0]     =    0.0;
      channelNodeZBank[0] =    0.0;
      channelNodeX[1]     =  900.0;
      channelNodeY[1]     =    0.0;
      channelNodeZBank[1] =    9.0;
      channelNodeX[2]     = 1800.0;
      channelNodeY[2]     =    0.0;
      channelNodeZBank[2] =   18.0;
      channelNodeX[3]     = 2700.0;
      channelNodeY[3]     =    0.0;
      channelNodeZBank[3] =   27.0;
      channelNodeX[4]     = 3600.0;
      channelNodeY[4]     =    0.0;
      channelNodeZBank[4] =   36.0;
      
      channelElementVertices = new IntArrayXDMF[4];
      
      channelElementVertices[0][0] = 2;
      channelElementVertices[0][1] = 2;
      channelElementVertices[0][2] = 0;
      channelElementVertices[0][3] = 1;
      channelElementVertices[1][0] = 2;
      channelElementVertices[1][1] = 2;
      channelElementVertices[1][2] = 1;
      channelElementVertices[1][3] = 2;
      channelElementVertices[2][0] = 2;
      channelElementVertices[2][1] = 2;
      channelElementVertices[2][2] = 2;
      channelElementVertices[2][3] = 3;
      channelElementVertices[3][0] = 2;
      channelElementVertices[3][1] = 2;
      channelElementVertices[3][2] = 3;
      channelElementVertices[3][3] = 4;
      
      channelRegion = new int[4];
      
      channelRegion[0] = 0;
      channelRegion[1] = 1;
      channelRegion[2] = 1;
      channelRegion[3] = 2;
      
      channelChannelType = new ChannelTypeEnum[4];
      
      channelChannelType[0] = STREAM;
      channelChannelType[1] = STREAM;
      channelChannelType[2] = STREAM;
      channelChannelType[3] = STREAM;
      
      channelReachCode = new long long[4];
      
      channelReachCode[0] = 0;
      channelReachCode[1] = 1;
      channelReachCode[2] = 1;
      channelReachCode[3] = 2;
      
      channelElementBankFullDepth = new double[4];
      
      channelElementBankFullDepth[0] = 10.0;
      channelElementBankFullDepth[1] = 10.0;
      channelElementBankFullDepth[2] = 10.0;
      channelElementBankFullDepth[3] = 10.0;
      
      channelElementLength = new double[4];
      
      channelElementLength[0] = 900.0;
      channelElementLength[1] = 900.0;
      channelElementLength[2] = 900.0;
      channelElementLength[3] = 900.0;
      
      channelBaseWidth = new double[4];
      
      channelBaseWidth[0] = 1.0;
      channelBaseWidth[1] = 1.0;
      channelBaseWidth[2] = 1.0;
      channelBaseWidth[3] = 1.0;
      
      channelSideSlope = new double[4];
      
      channelSideSlope[0] = 1.0;
      channelSideSlope[1] = 1.0;
      channelSideSlope[2] = 1.0;
      channelSideSlope[3] = 1.0;
      
      channelBedConductivity = new double[4];
      
      channelBedConductivity[0] = 4.66E-5;
      channelBedConductivity[1] = 4.66E-5;
      channelBedConductivity[2] = 4.66E-5;
      channelBedConductivity[3] = 4.66E-5;
      
      channelBedThickness = new double[4];
      
      channelBedThickness[0] = 1.0;
      channelBedThickness[1] = 1.0;
      channelBedThickness[2] = 1.0;
      channelBedThickness[3] = 1.0;
      
      channelManningsN = new double[4];
      
      channelManningsN[0] = 0.038;
      channelManningsN[1] = 0.038;
      channelManningsN[2] = 0.038;
      channelManningsN[3] = 0.038;
      
      channelMeshNeighbors = new IntArrayCMN[4];
      
      channelMeshNeighbors[0][0] = 0;
      channelMeshNeighbors[0][1] = 2;
      channelMeshNeighbors[1][0] = 2;
      channelMeshNeighbors[1][1] = 4;
      channelMeshNeighbors[2][0] = 6;
      channelMeshNeighbors[2][1] = 8;
      channelMeshNeighbors[3][0] = 8;
      channelMeshNeighbors[3][1] = 10;
      
      channelMeshNeighborsRegion = new IntArrayCMN[4];
      
      channelMeshNeighborsRegion[0][0] = 0;
      channelMeshNeighborsRegion[0][1] = 0;
      channelMeshNeighborsRegion[1][0] = 0;
      channelMeshNeighborsRegion[1][1] = 1;
      channelMeshNeighborsRegion[2][0] = 1;
      channelMeshNeighborsRegion[2][1] = 2;
      channelMeshNeighborsRegion[3][0] = 2;
      channelMeshNeighborsRegion[3][1] = 2;
      
      channelMeshNeighborsEdgeLength = new DoubleArrayCMN[4];
      
      channelMeshNeighborsEdgeLength[0][0] = 600.0;
      channelMeshNeighborsEdgeLength[0][1] = 300.0;
      channelMeshNeighborsEdgeLength[1][0] = 300.0;
      channelMeshNeighborsEdgeLength[1][1] = 600.0;
      channelMeshNeighborsEdgeLength[2][0] = 600.0;
      channelMeshNeighborsEdgeLength[2][1] = 300.0;
      channelMeshNeighborsEdgeLength[3][0] = 300.0;
      channelMeshNeighborsEdgeLength[3][1] = 600.0;
      
      channelChannelNeighbors = new IntArrayCCN[4];
      
      channelChannelNeighbors[0][0] = 1;
      channelChannelNeighbors[0][1] = OUTFLOW;
      channelChannelNeighbors[1][0] = 0;
      channelChannelNeighbors[1][1] = 2;
      channelChannelNeighbors[2][0] = 1;
      channelChannelNeighbors[2][1] = 3;
      channelChannelNeighbors[3][0] = 2;
      channelChannelNeighbors[3][1] = NOFLOW;
      
      channelChannelNeighborsRegion = new IntArrayCCN[4];
      
      channelChannelNeighborsRegion[0][0] = 1;
      channelChannelNeighborsRegion[0][1] = 0;
      channelChannelNeighborsRegion[1][0] = 0;
      channelChannelNeighborsRegion[1][1] = 1;
      channelChannelNeighborsRegion[2][0] = 1;
      channelChannelNeighborsRegion[2][1] = 2;
      channelChannelNeighborsRegion[3][0] = 1;
      channelChannelNeighborsRegion[3][1] = 0;
      
      channelChannelNeighborsDownstream = new BoolArrayCCN[4];
      
      channelChannelNeighborsDownstream[0][0] = false;
      channelChannelNeighborsDownstream[0][1] = true;
      channelChannelNeighborsDownstream[1][0] = true;
      channelChannelNeighborsDownstream[1][1] = false;
      channelChannelNeighborsDownstream[2][0] = true;
      channelChannelNeighborsDownstream[2][1] = false;
      channelChannelNeighborsDownstream[3][0] = true;
      channelChannelNeighborsDownstream[3][1] = false;
    }
    else
    {
      globalNumberOfRegions         = 3;
      localRegionStart              = 0;
      localNumberOfRegions          = 0;
      globalNumberOfMeshNodes       = 14;
      localMeshNodeStart            = 0;
      localNumberOfMeshNodes        = 0;
      globalNumberOfMeshElements    = 12;
      localMeshElementStart         = 0;
      localNumberOfMeshElements     = 0;
      globalNumberOfChannelNodes    = 5;
      localChannelNodeStart         = 0;
      localNumberOfChannelNodes     = 0;
      globalNumberOfChannelElements = 4;
      localChannelElementStart      = 0;
      localNumberOfChannelElements  = 0;
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
          meshVertexX             = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
          needToGetMeshVertexData = true;
        }

      if (NULL == meshVertexY && NULL != meshNodeY)
        {
          meshVertexY             = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
          needToGetMeshVertexData = true;
        }

      if (NULL == meshVertexZSurface && NULL != meshNodeZSurface)
        {
          meshVertexZSurface      = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
          needToGetMeshVertexData = true;
        }
    }

  if (NULL != channelElementVertices)
    {
      if (NULL == channelVertexX && NULL != channelNodeX)
        {
          channelVertexX             = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_VERTICES_SIZE];
          needToGetChannelVertexData = true;
        }

      if (NULL == channelVertexY && NULL != channelNodeY)
        {
          channelVertexY             = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_VERTICES_SIZE];
          needToGetChannelVertexData = true;
        }

      if (NULL == channelVertexZBank && NULL != channelNodeZBank)
        {
          channelVertexZBank         = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_VERTICES_SIZE];
          needToGetChannelVertexData = true;
        }
    }

  if (needToGetMeshVertexData)
    {
      meshVertexUpdated = new bool[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];

      for(ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
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
      channelVertexUpdated = new bool[localNumberOfChannelElements][CHANNEL_ELEMENT_VERTICES_SIZE];

      for(ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_VERTICES_SIZE; jj++)
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
          for (jj = 0; updated && jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              updated = meshVertexUpdated[ii][jj];
            }
        }
    }
  
  if (NULL != channelVertexUpdated)
    {
      for (ii = 0; updated && ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; updated && jj < CHANNEL_ELEMENT_VERTICES_SIZE; jj++)
            {
              updated = channelVertexUpdated[ii][jj];
            }
        }
    }
  
  return updated;
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
  int    ii, jj;                                                                             // Loop counters.
  bool   done                                                  = false;                      // Loop end condition.
  int    oldNumberOfRemainingElementsWithInvalidVegetationType = globalNumberOfMeshElements; // Number of elements with invalid vegetation type from the last pass.
  int    newNumberOfRemainingElementsWithInvalidVegetationType;                              // Number of elements with invalid vegetation type from the current pass.
  int    oldNumberOfRemainingElementsWithInvalidSoilType       = globalNumberOfMeshElements; // Number of elements with invalid soil type from the last pass.
  int    newNumberOfRemainingElementsWithInvalidSoilType;                                    // Number of elements with invalid soil type from the current pass.
  int    neighbor;                                                                           // A neighbor of an element.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(1 == CkNumPes()))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshMassageVegetationAndSoilType is not implemented for distributed operation.  It can "
              "only be run on one processor.\n");
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
  
  if (!(NULL != meshElementLayerZBottom))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshElementLayerZBottom must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshMeshNeighbors))
    {
      CkError("ERROR in FileManager::meshMassageVegetationAndSoilType: meshSurfacewaterMeshNeighbors must not be NULL.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // For mesh elements that have vegetation type of Water Bodies (16) or Snow or Ice (24) or no soil type (-1) or soil type of WATER (14) or OTHER(land-ice)
  // (16) get values from a neighbor.
  while (!done)
    {
      newNumberOfRemainingElementsWithInvalidVegetationType = 0;
      newNumberOfRemainingElementsWithInvalidSoilType       = 0;

      for (ii = 0; ii < globalNumberOfMeshElements; ii++)
        {
          // If element ii has invalid vegetation or soil type try to get it from a neighbor.  This might not succeed if no neighbor has a valid type.
          // The outer most loop handles this case by repeating this fix until the number of elements with invalid type goes to zero or stops going down.
          jj = 0;

          while (jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE && (16 == meshVegetationType[ii] || 24 == meshVegetationType[ii] ||
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

                      meshSoilType[ii]            = meshSoilType[neighbor];
                      meshElementSoilDepth[ii]    = meshElementSoilDepth[neighbor];
                      meshElementLayerZBottom[ii] = meshElementZSurface[ii] - meshElementSoilDepth[ii];
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

int FileManager::breakMeshDigitalDam(int meshElement, long long reachCode)
{
  int    ii;                       // Loop counter.
  int    neighbor      = NOFLOW;   // A neighbor of meshElement.
  double neighborZBank = INFINITY; // The bank Z coordinate of neighbor.
  double edgeLength;               // The edge length to use for the new connection.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(localMeshElementStart <= meshElement && meshElement < localMeshElementStart + localNumberOfMeshElements))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshElement data not owned by this local branch.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(1 == CkNumPes() && 0 == localMeshElementStart && 0 == localChannelElementStart))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: breakMeshDigitalDam is not implemented for distributed operation.  It can only be run on one "
              "processor.\n");
      CkExit();
    }
  
  if (!(NULL != meshVertexZSurface))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshVertexZSurface must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshRegion))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshRegion must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshGroundwaterMethod))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshGroundwaterMethod must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshMeshNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshMeshNeighborsEdgeLength must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshChannelNeighbors))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshChannelNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshChannelNeighborsRegion))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshChannelNeighborsRegion must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshChannelNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshChannelNeighborsEdgeLength must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshSurfacewaterChannelNeighborsConnection))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshSurfacewaterChannelNeighborsConnection must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshGroundwaterChannelNeighborsConnection))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: meshGroundwaterChannelNeighborsConnection must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelRegion))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelRegion must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelReachCode))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelReachCode must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelElementZBank))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelElementZBank must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelElementLength))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelElementLength must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelMeshNeighbors))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelMeshNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelMeshNeighborsRegion))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelMeshNeighborsRegion must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelMeshNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelMeshNeighborsEdgeLength must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelSurfacewaterMeshNeighborsConnection))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelSurfacewaterMeshNeighborsConnection must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelGroundwaterMeshNeighborsConnection))
    {
      CkError("ERROR in FileManager::breakMeshDigitalDam: channelGroundwaterMeshNeighborsConnection must not be NULL.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
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
          CkError("WARNING in FileManager::breakMeshDigitalDam: breaking digital dam by connecting mesh element %d to channel element %d.\n", meshElement,
                  neighbor);
        }
      
      // Calculate the edge length of the connection as the minimum of the length of the lowest edge of the mesh meshElement or the length of the channel element.
      if (meshVertexZSurface[meshElement][1] + meshVertexZSurface[meshElement][2] < meshVertexZSurface[meshElement][2] + meshVertexZSurface[meshElement][0] &&
          meshVertexZSurface[meshElement][1] + meshVertexZSurface[meshElement][2] < meshVertexZSurface[meshElement][0] + meshVertexZSurface[meshElement][1])
        {
          // Edge zero is the lowest.
          edgeLength = meshMeshNeighborsEdgeLength[meshElement][0];
        }
      else if (meshVertexZSurface[meshElement][2] + meshVertexZSurface[meshElement][0] < meshVertexZSurface[meshElement][0] + meshVertexZSurface[meshElement][1])
        {
          // Edge one is the lowest.
          edgeLength = meshMeshNeighborsEdgeLength[meshElement][1];
        }
      else
        {
          // Edge two is the lowest.
          edgeLength = meshMeshNeighborsEdgeLength[meshElement][2];
        }
      
      if (edgeLength > channelElementLength[neighbor])
        {
          edgeLength = channelElementLength[neighbor];
        }
      
      // Place the elements in each other's neighbor lists.
      ii = 0;
      
      while (ii < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE && NOFLOW != meshChannelNeighbors[meshElement][ii])
        {
          ii++;
        }
      
      if (ii < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE)
        {
          meshChannelNeighbors[meshElement][ii]                       = neighbor;
          meshChannelNeighborsRegion[meshElement][ii]                 = channelRegion[neighbor];
          meshChannelNeighborsEdgeLength[meshElement][ii]             = edgeLength;
          meshSurfacewaterChannelNeighborsConnection[meshElement][ii] = true;
          meshGroundwaterChannelNeighborsConnection[meshElement][ii]  = (InfiltrationAndGroundwater::SHALLOW_AQUIFER == meshGroundwaterMethod[meshElement]);
        }
      else
        {
          CkError("ERROR in FileManager::breakMeshDigitalDam: mesh element %d has more than the maximum %d channel neighbors.\n", meshElement,
                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE);
          CkExit();
        }
      
      ii = 0;
      
      while (ii < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE && NOFLOW != channelMeshNeighbors[neighbor][ii])
        {
          ii++;
        }
      
      if (ii < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE)
        {
          channelMeshNeighbors[neighbor][ii]                       = meshElement;
          channelMeshNeighborsRegion[neighbor][ii]                 = meshRegion[meshElement];
          channelMeshNeighborsEdgeLength[neighbor][ii]             = edgeLength;
          channelSurfacewaterMeshNeighborsConnection[neighbor][ii] = true;
          channelGroundwaterMeshNeighborsConnection[neighbor][ii]  = (InfiltrationAndGroundwater::SHALLOW_AQUIFER == meshGroundwaterMethod[meshElement]);
        }
      else
        {
          CkError("ERROR in FileManager::breakMeshDigitalDam: channel element %d has more than the maximum %d mesh neighbors.\n", neighbor,
                  CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE);
          CkExit();
        }
    }
  
  return neighbor;
}

int FileManager::findDownstreamChannelElement(int channelElement)
{
  int ii;                         // Loop counter.
  int downstreamElement = NOFLOW; // An element or boundary condition code downstream of channelElement.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(localChannelElementStart <= channelElement && channelElement < localChannelElementStart + localNumberOfChannelElements))
    {
      CkError("ERROR in FileManager::findDownstreamChannelElement: channelElement data not owned by this local branch.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(NULL != channelChannelNeighbors))
    {
      CkError("ERROR in FileManager::findDownstreamChannelElement: channelChannelNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelChannelNeighborsDownstream))
    {
      CkError("ERROR in FileManager::findDownstreamChannelElement: channelChannelNeighborsDownstream must not be NULL.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  for (ii = 0; NOFLOW == downstreamElement && ii < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE &&
               NOFLOW != channelChannelNeighbors[channelElement - localChannelElementStart][ii]; ii++)
    {
      if (!isBoundary(channelChannelNeighbors[channelElement - localChannelElementStart][ii]))
        {
          if (channelChannelNeighborsDownstream[channelElement - localChannelElementStart][ii])
            {
              downstreamElement = channelChannelNeighbors[channelElement - localChannelElementStart][ii];
            }
        }
      else if (OUTFLOW == channelChannelNeighbors[channelElement - localChannelElementStart][ii])
        {
          downstreamElement = channelChannelNeighbors[channelElement - localChannelElementStart][ii];
        }
    }
  
  return downstreamElement;
}

double FileManager::breakChannelDigitalDam(int channelElement, int dammedChannelElement, double length)
{
  double slope;             // The slope downward from dammedChannelElement to the point where the digital dam is broken, unitless.
  int    downstreamElement; // An element or boundary condition code downstream of channelElement.
  double distanceLowered;   // The distance in meters by which an element is lowered to break a digital dam.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(localChannelElementStart <= channelElement && channelElement < localChannelElementStart + localNumberOfChannelElements))
    {
      CkError("ERROR in FileManager::breakChannelDigitalDam: channelElement data not owned by this local branch.\n");
      CkExit();
    }
  
  if (!(localChannelElementStart <= dammedChannelElement && dammedChannelElement < localChannelElementStart + localNumberOfChannelElements))
    {
      CkError("ERROR in FileManager::breakChannelDigitalDam: dammedChannelElement data not owned by this local branch.\n");
      CkExit();
    }
  
  if (!(0.0 < length))
    {
      CkError("ERROR in FileManager::breakChannelDigitalDam: length must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(1 == CkNumPes() && 0 == localMeshElementStart && 0 == localChannelElementStart))
    {
      CkError("ERROR in FileManager::breakChannelDigitalDam: breakChannelDigitalDam is not implemented for distributed operation.  It can only be run on one "
              "processor.\n");
      CkExit();
    }
  
  if (!(NULL != channelChannelType))
    {
      CkError("ERROR in FileManager::breakChannelDigitalDam: channelChannelType must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelElementZBank))
    {
      CkError("ERROR in FileManager::breakChannelDigitalDam: channelElementZBank must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelElementZBed))
    {
      CkError("ERROR in FileManager::breakChannelDigitalDam: channelElementZBed must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelElementLength))
    {
      CkError("ERROR in FileManager::breakChannelDigitalDam: channelElementLength must not be NULL.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  if (channelElementZBed[dammedChannelElement] > channelElementZBed[channelElement])
    {
      // channelElement is lower than dammedChannelElement.  Calculate the slope from dammedChannelElement to channelElement.
      slope = (channelElementZBed[dammedChannelElement] - channelElementZBed[channelElement]) / (length + 0.5 * channelElementLength[channelElement]);
    }
  else if (WATERBODY == channelChannelType[channelElement])
    {
      // channelElement is a waterbody, which is allowed to be dammed, so lower it to the level of dammedChannelElement.
      distanceLowered = channelElementZBed[channelElement] - channelElementZBed[dammedChannelElement];
      
      if ((2 <= ADHydro::verbosityLevel && 10.0 < distanceLowered) || 3 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in FileManager::breakDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", channelElement,
                  distanceLowered, channelElementZBed[channelElement], channelElementZBed[dammedChannelElement]);
        }
      
      channelElementZBank[channelElement] -= distanceLowered;
      channelElementZBed[channelElement]   = channelElementZBed[dammedChannelElement];
      slope                         = 0.0;
    }
  else
    {
      downstreamElement = findDownstreamChannelElement(channelElement);
      
      if (isBoundary(downstreamElement))
        {
          // channelElement has an outflow boundary or no downstream connections at all.  Lower it to the level of dammedChannelElement.
          if (OUTFLOW != downstreamElement)
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::breakDigitalDam: channel element %d has no downstream connections.\n", channelElement);
                }
            }
          
          distanceLowered = channelElementZBed[channelElement] - channelElementZBed[dammedChannelElement];
          
          if ((2 <= ADHydro::verbosityLevel && 10.0 < distanceLowered) || 3 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in FileManager::breakDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", channelElement,
                      distanceLowered, channelElementZBed[channelElement], channelElementZBed[dammedChannelElement]);
            }
          
          channelElementZBank[channelElement] -= distanceLowered;
          channelElementZBed[channelElement]   = channelElementZBed[dammedChannelElement];
          slope                         = 0.0;
        }
      else
        {
          // Continue to search downstream, and when the search returns lower channelElement to a straight line slope down from the dammed element.
          slope = breakChannelDigitalDam(downstreamElement, dammedChannelElement, length + channelElementLength[channelElement]);
          
          distanceLowered = channelElementZBed[channelElement] - (channelElementZBed[dammedChannelElement] - slope * (length + 0.5 * channelElementLength[channelElement]));
          
          if ((2 <= ADHydro::verbosityLevel && 10.0 < distanceLowered) || 3 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in FileManager::breakDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", channelElement,
                      distanceLowered, channelElementZBed[channelElement],
                      channelElementZBed[dammedChannelElement] - slope * (length + 0.5 * channelElementLength[channelElement]));
            }
          
          channelElementZBank[channelElement] -= distanceLowered;
          channelElementZBed[channelElement]   = channelElementZBed[dammedChannelElement] - slope * (length + 0.5 * channelElementLength[channelElement]);
        }
    }
  
  return slope;
}

void FileManager::meshMassage()
{
  int                      ii, jj, kk;                // Loop counters.
  bool                     regionsReassigned = false; // Whether regions were reassigned.
  std::map<long long, int> regionMap;                 // Mapping from catchment and reach code numbers to region indexes.
  bool                     hasLowerNeighbor;          // Whether an element has a lower neighbor.
  bool                     hasChannelNeighbor;        // Whether an element has a channel neighbor.
  int                      downstreamElement;         // An element or boundary condition code downstream of an element.
  int                      neighbor;                  // A neighbor of an element.
  double                   edgeZSurface;              // The surface Z coordinate of the center of a mesh edge.
  bool                     surfacewaterConnection;    // Whether neighboring elements remain connected for surfacewater flow.
  bool                     groundwaterConnection;     // Whether neighboring elements remain connected for groundwater flow.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(1 == CkNumPes() && 0 == localMeshElementStart && 0 == localChannelElementStart))
    {
      CkError("ERROR in FileManager::meshMassage: meshMassage is not implemented for distributed operation.  It can only be run on one processor.\n");
      CkExit();
    }
  
  if (!(NULL != meshVertexZSurface))
    {
      CkError("ERROR in FileManager::meshMassage: meshVertexZSurface must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshCatchment))
    {
      CkError("ERROR in FileManager::meshMassage: meshCatchment must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshElementZSurface))
    {
      CkError("ERROR in FileManager::meshMassage: meshElementZSurface must not be NULL.\n");
      CkExit();
    }

  if (!(NULL != meshGroundwaterMethod))
    {
      CkError("ERROR in FileManager::meshMassage: meshGroundwaterMethod must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshMeshNeighbors))
    {
      CkError("ERROR in FileManager::meshMassage: meshMeshNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshMeshNeighborsChannelEdge))
    {
      CkError("ERROR in FileManager::meshMassage: meshMeshNeighborsChannelEdge must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshSurfacewaterMeshNeighborsConnection))
    {
      CkError("ERROR in FileManager::meshMassage: meshSurfacewaterMeshNeighborsConnection must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshGroundwaterMeshNeighborsConnection))
    {
      CkError("ERROR in FileManager::meshMassage: meshGroundwaterMeshNeighborsConnection must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshChannelNeighbors))
    {
      CkError("ERROR in FileManager::meshMassage: meshChannelNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != meshSurfacewaterChannelNeighborsConnection))
    {
      CkError("ERROR in FileManager::meshMassage: meshSurfacewaterChannelNeighborsConnection must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelChannelType))
    {
      CkError("ERROR in FileManager::meshMassage: channelChannelType must not be NULL.\n");
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
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // Assign elements to regions if they haven't been already.
  if (NULL == meshRegion || NULL == channelRegion)
    {
      // Start by creating a mapping from catchment and reach code numbers to region indexes.
      regionsReassigned     = true;
      globalNumberOfRegions = 0;
      
      for (ii = 0; ii < globalNumberOfMeshElements; ii++)
        {
          if (regionMap.end() == regionMap.find(meshCatchment[ii]))
            {
              regionMap[meshCatchment[ii]] = globalNumberOfRegions;
              globalNumberOfRegions++;
            }
        }
      
      for (ii = 0; ii < globalNumberOfChannelElements; ii++)
        {
          if (regionMap.end() == regionMap.find(channelReachCode[ii]))
            {
              regionMap[channelReachCode[ii]] = globalNumberOfRegions;
              globalNumberOfRegions++;
            }
        }
      
      localStartAndNumber(&localRegionStart, &localNumberOfRegions, globalNumberOfRegions);
      
      // Next fill in the meshRegion and ChannelRegion arrays.
      if (NULL == meshRegion)
        {
          meshRegion = new int[globalNumberOfMeshElements];
        }
      
      if (NULL == channelRegion)
        {
          channelRegion = new int[globalNumberOfChannelElements];
        }
      
      for (ii = 0; ii < globalNumberOfMeshElements; ii++)
        {
          meshRegion[ii] = regionMap[meshCatchment[ii]];
        }
      
      for (ii = 0; ii < globalNumberOfChannelElements; ii++)
        {
          channelRegion[ii] = regionMap[channelReachCode[ii]];
        }
    }
  
  // Calculate meshMeshNeighborsRegion from meshMeshNeighbors and meshRegion.
  if ((NULL == meshMeshNeighborsRegion || regionsReassigned) && NULL != meshMeshNeighbors && NULL != meshRegion)
    {
      if (NULL == meshMeshNeighborsRegion)
        {
          meshMeshNeighborsRegion = new int[globalNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
        }
      
      for (ii = 0; ii < globalNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              if (isBoundary(meshMeshNeighbors[ii][jj]))
                {
                  meshMeshNeighborsRegion[ii][jj] = 0;
                }
              else
                {
                  meshMeshNeighborsRegion[ii][jj] = meshRegion[meshMeshNeighbors[ii][jj]];
                }
            }
        }
    }
  
  // Calculate meshChannelNeighborsRegion from meshChannelNeighbors and channelRegion.
  if ((NULL == meshChannelNeighborsRegion || regionsReassigned) && NULL != meshChannelNeighbors && NULL != channelRegion)
    {
      if (NULL == meshChannelNeighborsRegion)
        {
          meshChannelNeighborsRegion = new int[globalNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
        }
      
      for (ii = 0; ii < globalNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              if (isBoundary(meshChannelNeighbors[ii][jj]))
                {
                  meshChannelNeighborsRegion[ii][jj] = 0;
                }
              else
                {
                  meshChannelNeighborsRegion[ii][jj] = channelRegion[meshChannelNeighbors[ii][jj]];
                }
            }
        }
    }
  
  // Calculate channelMeshNeighborsRegion from channelMeshNeighbors and meshRegion.
  if ((NULL == channelMeshNeighborsRegion || regionsReassigned) && NULL != channelMeshNeighbors && NULL != meshRegion)
    {
      if (NULL == channelMeshNeighborsRegion)
        {
          channelMeshNeighborsRegion = new int[globalNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
        }
      
      for (ii = 0; ii < globalNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              if (isBoundary(channelMeshNeighbors[ii][jj]))
                {
                  channelMeshNeighborsRegion[ii][jj] = 0;
                }
              else
                {
                  channelMeshNeighborsRegion[ii][jj] = meshRegion[channelMeshNeighbors[ii][jj]];
                }
            }
        }
    }
  
  // Calculate channelChannelNeighborsRegion from channelChannelNeighbors and channelRegion.
  if ((NULL == channelChannelNeighborsRegion || regionsReassigned) && NULL != channelChannelNeighbors && NULL != channelRegion)
    {
      if (NULL == channelChannelNeighborsRegion)
        {
          channelChannelNeighborsRegion = new int[globalNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
        }
      
      for (ii = 0; ii < globalNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              if (isBoundary(channelChannelNeighbors[ii][jj]))
                {
                  channelChannelNeighborsRegion[ii][jj] = 0;
                }
              else
                {
                  channelChannelNeighborsRegion[ii][jj] = channelRegion[channelChannelNeighbors[ii][jj]];
                }
            }
        }
    }

  // Calculate regionNumberOfMeshElements from meshRegion.
  if ((NULL == regionNumberOfMeshElements || regionsReassigned) && NULL != meshRegion)
    {
      if (NULL == regionNumberOfMeshElements)
        {
          regionNumberOfMeshElements = new int[globalNumberOfRegions];
        }

      for (ii = 0; ii < globalNumberOfRegions; ii++)
        {
          regionNumberOfMeshElements[ii] = 0;
        }

      for (ii = 0; ii < globalNumberOfMeshElements; ii++)
        {
          if (0 <= meshRegion[ii] && meshRegion[ii] < globalNumberOfRegions)
            {
              regionNumberOfMeshElements[meshRegion[ii]]++;
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else
            {
              CkError("ERROR in FileManager::meshMassage: mesh element %d's region must be greater than or equal to zero and less than "
                      "globalNumberOfRegions.\n", ii);
              CkExit();
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  // Calculate regionNumberOfChannelElements from channelRegion.
  if ((NULL == regionNumberOfChannelElements || regionsReassigned) && NULL != channelRegion)
    {
      if (NULL == regionNumberOfChannelElements)
        {
          regionNumberOfChannelElements = new int[globalNumberOfRegions];
        }

      for (ii = 0; ii < globalNumberOfRegions; ii++)
        {
          regionNumberOfChannelElements[ii] = 0;
        }

      for (ii = 0; ii < globalNumberOfChannelElements; ii++)
        {
          if (0 <= channelRegion[ii] && channelRegion[ii] < globalNumberOfRegions)
            {
              regionNumberOfChannelElements[channelRegion[ii]]++;
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else
            {
              CkError("ERROR in FileManager::meshMassage: channel element %d's region must be greater than or equal to zero and less than "
                      "globalNumberOfRegions.\n", ii);
              CkExit();
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  // Break digital dams in the channel network.
  for (ii = 0; ii < globalNumberOfChannelElements; ii++)
    {
      // Only break digital dams for streams.  The reason waterbodies have standing water in them is because there is some kind of real dam keeping it there.
      if (STREAM == channelChannelType[ii])
        {
          hasLowerNeighbor = false;

          for (jj = 0; !hasLowerNeighbor && jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE && NOFLOW != channelChannelNeighbors[ii][jj]; jj++)
            {
              neighbor = channelChannelNeighbors[ii][jj];
              
              if (!isBoundary(neighbor))
                {
                  if (channelChannelNeighborsDownstream[ii][jj] && channelElementZBed[ii] >= channelElementZBed[neighbor])
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
              downstreamElement = findDownstreamChannelElement(ii);
              
              if (isBoundary(downstreamElement))
                {
                  if (2 <= ADHydro::verbosityLevel)
                    {
                      CkError("WARNING in FileManager::meshMassage: channel element %d has no downstream connections.\n", ii);
                    }
                }
              else
                {
                  breakChannelDigitalDam(downstreamElement, ii, 0.5 * channelElementLength[ii]);
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

          for (jj = 0; !hasLowerNeighbor && jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE && NOFLOW != channelChannelNeighbors[ii][jj]; jj++)
            {
              neighbor = channelChannelNeighbors[ii][jj];
              
              if (!isBoundary(neighbor))
                {
                  if (channelChannelNeighborsDownstream[ii][jj] && channelElementZBed[ii] >= channelElementZBed[neighbor])
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
  
  // Remove certain connections in the mesh.
  for (ii = 0; ii < globalNumberOfMeshElements; ii++)
    {
      for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
        {
          neighbor = meshMeshNeighbors[ii][jj];
          
          // Only the lower ID number of the pair has to check.  This check also excludes boundary condition codes.
          if (ii < neighbor)
            {
              surfacewaterConnection = meshSurfacewaterMeshNeighborsConnection[ii][jj];
              groundwaterConnection  = meshGroundwaterMeshNeighborsConnection[ii][jj];

              // Remove neighbor connection between neighboring mesh elements that are in different catchments and separated by a ridge.
              if (meshCatchment[ii] != meshCatchment[neighbor])
                {
                  // Get the height of the center of the edge separating the neighbors.  If it is higher than both neighbors it is considered a ridge.
                  // FIXME this could be replaced with checking if both elements slope away from the edge.
                  edgeZSurface = 0.5 * (meshVertexZSurface[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] +
                                        meshVertexZSurface[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]);

                  if (edgeZSurface > meshElementZSurface[ii] && edgeZSurface > meshElementZSurface[neighbor])
                    {
                      surfacewaterConnection = false;
                      groundwaterConnection  = false;
                    }
                }
              
              // Remove surfacewater neighbor connection between neighboring mesh elements with a channel edge.
              if (meshMeshNeighborsChannelEdge[ii][jj])
                {
                  surfacewaterConnection = false;
                }
              
              // Remove groundwater neighbor connection between neighboring mesh elements if either is not SHALLOW_AQUIFER.
              // FIXME groundwater INFLOW and OUTFLOW boundaries?
              if (InfiltrationAndGroundwater::SHALLOW_AQUIFER != meshGroundwaterMethod[ii] ||
                  InfiltrationAndGroundwater::SHALLOW_AQUIFER != meshGroundwaterMethod[neighbor])
                {
                  groundwaterConnection  = false;
                }
              
              if (surfacewaterConnection != meshSurfacewaterMeshNeighborsConnection[ii][jj] ||
                  groundwaterConnection  != meshGroundwaterMeshNeighborsConnection[ii][jj])
                {
                  // Find ii in neighbor's neighbor list and remove connection.
                  kk = 0;
                  
                  while (kk < MESH_ELEMENT_MESH_NEIGHBORS_SIZE && ii != meshMeshNeighbors[neighbor][kk])
                    {
                      kk++;
                    }
                  
                  if (kk < MESH_ELEMENT_MESH_NEIGHBORS_SIZE)
                    {
                      meshSurfacewaterMeshNeighborsConnection[neighbor][kk] = surfacewaterConnection;
                      meshGroundwaterMeshNeighborsConnection[neighbor][kk]  = groundwaterConnection;
                    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  else
                    {
                      CkError("ERROR in FileManager::meshMassage: mesh element %d is not in its mesh neighbor %d's neighbor list.\n", ii, neighbor);
                      CkExit();
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  
                  // Remove connection from ii's neighbor list.
                  meshSurfacewaterMeshNeighborsConnection[ii][jj] = surfacewaterConnection;
                  meshGroundwaterMeshNeighborsConnection[ii][jj]  = groundwaterConnection;
                }
            }
        }

      // Remove groundwater neighbor connection between mesh element and neighboring channel element if the mesh element is not SHALLOW_AQUIFER.
      if (InfiltrationAndGroundwater::SHALLOW_AQUIFER != meshGroundwaterMethod[ii])
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              neighbor = meshChannelNeighbors[ii][jj];
              
              if (!isBoundary(neighbor))
                {
                  // Find ii in neighbor's neighbor list and remove connection.
                  kk = 0;
                  
                  while (kk < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE && ii != channelMeshNeighbors[neighbor][kk])
                    {
                      kk++;
                    }
                  
                  if (kk < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE)
                    {
                      channelGroundwaterMeshNeighborsConnection[neighbor][kk]  = false;
                    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  else
                    {
                      CkError("ERROR in FileManager::meshMassage: mesh element %d is not in its channdel neighbor %d's neighbor list.\n", ii, neighbor);
                      CkExit();
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                }
              
              // Remove connection from ii's neighbor list.
              meshGroundwaterChannelNeighborsConnection[ii][jj] = false;
            }
        }
    } // End remove certain connections in the mesh.
  
  // Break digital dams in the mesh.
  for (ii = 0; ii < globalNumberOfMeshElements; ii++)
    {
      hasLowerNeighbor   = false;
      hasChannelNeighbor = false;
      
      for (jj = 0; !hasLowerNeighbor && jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
        {
          neighbor = meshMeshNeighbors[ii][jj];
          
          if (!isBoundary(neighbor))
            {
              if (meshSurfacewaterMeshNeighborsConnection[ii][jj] && meshElementZSurface[ii] > meshElementZSurface[neighbor])
                {
                  hasLowerNeighbor = true;
                }
            }
          else if (OUTFLOW == neighbor)
            {
              hasLowerNeighbor = true;
            }
        }
      
      for (jj = 0; !hasChannelNeighbor && jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE && NOFLOW != meshChannelNeighbors[ii][jj]; jj++)
        {
          neighbor = meshChannelNeighbors[ii][jj];
          
          // Don't count icemasses because an icemass could be higher than the mesh element.
          if (meshSurfacewaterChannelNeighborsConnection[ii][jj] && ICEMASS != channelChannelType[neighbor])
            {
              hasChannelNeighbor = true;
            }
        }
      
      if (!hasLowerNeighbor && !hasChannelNeighbor)
        {
          // Break the digital dam by arbitrarily connecting the mesh element to the lowest stream element in the same catchment.
          neighbor = breakMeshDigitalDam(ii, meshCatchment[ii]);
          
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
                  neighbor = breakMeshDigitalDam(ii, channelPruned[jj][1]);
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
  double lengthSoFar;      // For traversing vertices.  The length traversed so far.
  double nextLength;       // For traversing vertices.  The length to the next vertex.
  double lengthFraction;   // For traversing vertices.  The fraction of the distance of the point of interest from the current vertex to the next vertex.
  double minX;             // For finding the bounding box of vertices.
  double maxX;             // For finding the bounding box of vertices.
  double minY;             // For finding the bounding box of vertices.
  double maxY;             // For finding the bounding box of vertices.
  double minZBank;         // For finding the bounding box of vertices.
  double maxZBank;         // For finding the bounding box of vertices.

  // Delete vertex updated arrays that are no longer needed.
  delete[] meshVertexUpdated;
  delete[] channelVertexUpdated;
  
  // If a mesh element has any channel neighbors it is automatically considered alluvium.
  if (NULL != meshAlluvium && NULL != meshChannelNeighbors)
    {
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          if (NOFLOW != meshChannelNeighbors[ii][0]) // Channel neighbors are compacted to the front so we only need to check the first one.
            {
              meshAlluvium[ii] = true;
            }
        }
    }
  
  // Calculate meshElementX by averaging the X coordinates of its vertices.
  if (NULL == meshElementX && NULL != meshVertexX)
    {
      meshElementX = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              value += meshVertexX[ii][jj];
            }

          meshElementX[ii] = value / MESH_ELEMENT_MESH_NEIGHBORS_SIZE;
        }
    }

  // Calculate meshElementY by averaging the Y coordinates of its vertices.
  if (NULL == meshElementY && NULL != meshVertexY)
    {
      meshElementY = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              value += meshVertexY[ii][jj];
            }

          meshElementY[ii] = value / MESH_ELEMENT_MESH_NEIGHBORS_SIZE;
        }
    }

  // Calculate meshElementZSurface by averaging the surface Z coordinates of its vertices.
  if (NULL == meshElementZSurface && NULL != meshVertexZSurface)
    {
      meshElementZSurface = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              value += meshVertexZSurface[ii][jj];
            }

          meshElementZSurface[ii] = value / MESH_ELEMENT_MESH_NEIGHBORS_SIZE;
        }
    }
  
  // Calculate meshElementZSurface by adding meshElementSoilDepth to meshElementLayerZBottom.
  if (NULL == meshElementZSurface && NULL != meshElementLayerZBottom && NULL != meshElementSoilDepth)
    {
      meshElementZSurface = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshElementZSurface[ii] = meshElementLayerZBottom[ii] + meshElementSoilDepth[ii];
        }
    }
  
  // Calculate meshElementSoilDepth by subtracting meshElementLayerZBottom from meshElementZSurface.
  if (NULL == meshElementSoilDepth && NULL != meshElementZSurface && NULL != meshElementLayerZBottom)
    {
      meshElementSoilDepth = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshElementSoilDepth[ii] = meshElementZSurface[ii] - meshElementLayerZBottom[ii];
        }
    }
  
  // Calculate meshElementLayerZBottom by subtracting meshElementSoilDepth from meshElementZSurface.
  if (NULL == meshElementLayerZBottom && NULL != meshElementZSurface && NULL != meshElementSoilDepth)
    {
      meshElementLayerZBottom = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshElementLayerZBottom[ii] = meshElementZSurface[ii] - meshElementSoilDepth[ii];
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

          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              value += meshVertexX[ii][jj] * (meshVertexY[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] -
                                              meshVertexY[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]);
            }

          meshElementArea[ii] = value * 0.5;
        }
    }

  // Calculate meshElementSlopeX from the Y and Z coordinates of its vertices and its area.
  if (NULL == meshElementSlopeX && NULL != meshVertexY && NULL != meshVertexZSurface && NULL != meshElementArea)
    {
      meshElementSlopeX = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE - 1; jj++)
            {
              value += (meshVertexY[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] - meshVertexY[ii][jj]) *
                       (meshVertexZSurface[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] - meshVertexZSurface[ii][0]);
            }

          meshElementSlopeX[ii] = value / (2.0 * meshElementArea[ii]);
        }
    }

  // Calculate meshElementSlopeY from the X and Z coordinates of its vertices and its area.
  if (NULL == meshElementSlopeY && NULL != meshVertexX && NULL != meshVertexZSurface && NULL != meshElementArea)
    {
      meshElementSlopeY = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE - 1; jj++)
            {
              value += (meshVertexX[ii][jj] - meshVertexX[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]) *
                       (meshVertexZSurface[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] - meshVertexZSurface[ii][0]);
            }

          meshElementSlopeY[ii] = value / (2.0 * meshElementArea[ii]);
        }
    }
  
  // Calculate meshLatitude and meshLongitude from meshElementX and meshElementY.
  if ((NULL == meshLatitude || NULL == meshLongitude) && NULL != meshElementX && NULL != meshElementY)
    {
      if (NULL == meshLatitude)
        {
          meshLatitude = new double[localNumberOfMeshElements];
        }
      
      if (NULL == meshLongitude)
        {
          meshLongitude = new double[localNumberOfMeshElements];
        }
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          ADHydro::getLatLong(meshElementX[ii],  meshElementY[ii],  meshLatitude[ii],  meshLongitude[ii]);
        }
    }
  
  // Fix elements with invalid vegetation or soil type.  Do this here because some values below are derived from vegetation or soil type.
  if (ADHydro::doMeshMassage)
    {
      meshMassageVegetationAndSoilType();
    }
  
  // Calculate meshManningsN by transfering the USGS 27-category vegetation type (in VEGPARM.TBL of Noah-MP) to the 21-category NLCD 1992 Land cover
  // classification and use the Manning's N from Bunya et al., 2009, High-resolution river inflow, tide, wind, wind wave and storm surge model for southern
  // Louisiana and Mississippi Part I.  This needs to correspond to the python script readFromSSURGO_STATSGO_NLCD_GEOL.py. See comments there.
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
                  CkError("WARNING in FileManager::calculateDerivedValues, mesh element %d: unknown vegetation type %d. using default value of Manning's N "
                          "for mixed forest\n", ii, meshVegetationType[ii]);
                }
              meshManningsN[ii] = 0.17;
              break;
          } // End of switch (meshVegetationType[ii]).
        } // End of for (ii = 0; ii < localNumberOfMeshElements; ii++).
    } // End of if (NULL == meshManningsN && NULL != meshVegetationType).
  
  // MeshConductivity and meshPorosity are taken from 19-category SOILPARM.TBL of Noah-MP.
  if ((NULL == meshConductivity || NULL == meshPorosity) && NULL != meshSoilType)
    {
      if (NULL == meshConductivity)
        {
          meshConductivity = new double[localNumberOfMeshElements];
        }

      if (NULL == meshPorosity)
        {
          meshPorosity = new double[localNumberOfMeshElements];
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
                CkError("WARNING in FileManager::calculateDerivedValues, mesh element %d: unknown soil type %d. using default values of conductivity and "
                        "porosity for sand\n", ii, meshSoilType[ii]);
              }
            meshConductivity[ii] = 4.66E-5;
            meshPorosity[ii]     = 0.339;
            break;
          } // End of switch (meshSoilType[ii]).
          
          // If the vegetation type is Urban and Built-Up Land (1) then Noah-MP sets porosity to 0.45 regardless of soil type.
          if (NULL != meshVegetationType && 1 == meshVegetationType[ii])
            {
              meshPorosity[ii] = 0.45;
            }
        } // End of for (ii = 0; ii < localNumberOfMeshElements; ii++).
    } // End of if ((NULL == meshConductivity || NULL == meshPorosity) && NULL != meshSoilType).
    
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
  
  // meshGroundwaterHead is assigned below after the groundwater method is assigned.
  
  // If not already specified meshGroundwaterRecharge defaults to zero.
  if (NULL == meshGroundwaterRecharge)
    {
      meshGroundwaterRecharge = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshGroundwaterRecharge[ii] = 0.0;
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
  
  // If not already specified meshPrecipitationRate defaults to zero.
  if (NULL == meshPrecipitationRate)
    {
      meshPrecipitationRate = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshPrecipitationRate[ii] = 0.0;
        }
    }
  
  // If not already specified meshPrecipitationCumulativeShortTerm defaults to zero.
  if (NULL == meshPrecipitationCumulativeShortTerm)
    {
      meshPrecipitationCumulativeShortTerm = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshPrecipitationCumulativeShortTerm[ii] = 0.0;
        }
    }
  
  // If not already specified meshPrecipitationCumulativeLongTerm defaults to zero.
  if (NULL == meshPrecipitationCumulativeLongTerm)
    {
      meshPrecipitationCumulativeLongTerm = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshPrecipitationCumulativeLongTerm[ii] = 0.0;
        }
    }
  
  // If not already specified meshEvaporationRate defaults to zero.
  if (NULL == meshEvaporationRate)
    {
      meshEvaporationRate = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshEvaporationRate[ii] = 0.0;
        }
    }
  
  // If not already specified meshEvaporationCumulativeShortTerm defaults to zero.
  if (NULL == meshEvaporationCumulativeShortTerm)
    {
      meshEvaporationCumulativeShortTerm = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshEvaporationCumulativeShortTerm[ii] = 0.0;
        }
    }
  
  // If not already specified meshEvaporationCumulativeLongTerm defaults to zero.
  if (NULL == meshEvaporationCumulativeLongTerm)
    {
      meshEvaporationCumulativeLongTerm = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshEvaporationCumulativeLongTerm[ii] = 0.0;
        }
    }
  
  // If not already specified meshTranspirationRate defaults to zero.
  if (NULL == meshTranspirationRate)
    {
      meshTranspirationRate = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTranspirationRate[ii] = 0.0;
        }
    }
  
  // If not already specified meshTranspirationCumulativeShortTerm defaults to zero.
  if (NULL == meshTranspirationCumulativeShortTerm)
    {
      meshTranspirationCumulativeShortTerm = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTranspirationCumulativeShortTerm[ii] = 0.0;
        }
    }
  
  // If not already specified meshTranspirationCumulativeLongTerm defaults to zero.
  if (NULL == meshTranspirationCumulativeLongTerm)
    {
      meshTranspirationCumulativeLongTerm = new double[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshTranspirationCumulativeLongTerm[ii] = 0.0;
        }
    }

  // If not already specified meshEvapoTranspirationState defaults to nominal values.
  if (NULL == meshEvapoTranspirationState)
    {
      meshEvapoTranspirationState = new EvapoTranspirationStateStruct[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshEvapoTranspirationState[ii].fIceOld[0] = 0.0f;
          meshEvapoTranspirationState[ii].fIceOld[1] = 0.0f;
          meshEvapoTranspirationState[ii].fIceOld[2] = 0.0f;
          meshEvapoTranspirationState[ii].albOld     = 1.0f;
          meshEvapoTranspirationState[ii].snEqvO     = 0.0f;
          meshEvapoTranspirationState[ii].stc[0]     = 0.0f;
          meshEvapoTranspirationState[ii].stc[1]     = 0.0f;
          meshEvapoTranspirationState[ii].stc[2]     = 0.0f;
          meshEvapoTranspirationState[ii].stc[3]     = 300.0f;
          meshEvapoTranspirationState[ii].stc[4]     = 300.0f;
          meshEvapoTranspirationState[ii].stc[5]     = 300.0f;
          meshEvapoTranspirationState[ii].stc[6]     = 300.0f;
          meshEvapoTranspirationState[ii].tah        = 300.0f;
          meshEvapoTranspirationState[ii].eah        = 2000.0f;
          meshEvapoTranspirationState[ii].fWet       = 0.0f;
          meshEvapoTranspirationState[ii].canLiq     = 0.0f;
          meshEvapoTranspirationState[ii].canIce     = 0.0f;
          meshEvapoTranspirationState[ii].tv         = 300.0f;
          meshEvapoTranspirationState[ii].tg         = 300.0f;
          meshEvapoTranspirationState[ii].iSnow      = 0;
          meshEvapoTranspirationState[ii].zSnso[0]   = 0.0f;
          meshEvapoTranspirationState[ii].zSnso[1]   = 0.0f;
          meshEvapoTranspirationState[ii].zSnso[2]   = 0.0f;
          
          if (NULL != meshElementSoilDepth && 0.0 < meshElementSoilDepth[ii])
            {
              meshEvapoTranspirationState[ii].zSnso[3]   = -0.05 * meshElementSoilDepth[ii];
              meshEvapoTranspirationState[ii].zSnso[4]   = -0.2  * meshElementSoilDepth[ii];
              meshEvapoTranspirationState[ii].zSnso[5]   = -0.5  * meshElementSoilDepth[ii];
              meshEvapoTranspirationState[ii].zSnso[6]   = -1.0  * meshElementSoilDepth[ii];
            }
          else
            {
              // Noah-MP cannot handle zero soil depth.  Use one meter instead.
              meshEvapoTranspirationState[ii].zSnso[3]   = -0.05f;
              meshEvapoTranspirationState[ii].zSnso[4]   = -0.2f;
              meshEvapoTranspirationState[ii].zSnso[5]   = -0.5f;
              meshEvapoTranspirationState[ii].zSnso[6]   = -1.0f;
            }
          
          meshEvapoTranspirationState[ii].snowH      = 0.0f;
          meshEvapoTranspirationState[ii].snEqv      = 0.0f;
          meshEvapoTranspirationState[ii].snIce[0]   = 0.0f;
          meshEvapoTranspirationState[ii].snIce[1]   = 0.0f;
          meshEvapoTranspirationState[ii].snIce[2]   = 0.0f;
          meshEvapoTranspirationState[ii].snLiq[0]   = 0.0f;
          meshEvapoTranspirationState[ii].snLiq[1]   = 0.0f;
          meshEvapoTranspirationState[ii].snLiq[2]   = 0.0f;
          meshEvapoTranspirationState[ii].lfMass     = 100000.0f;
          meshEvapoTranspirationState[ii].rtMass     = 100000.0f;
          meshEvapoTranspirationState[ii].stMass     = 100000.0f;
          meshEvapoTranspirationState[ii].wood       = 200000.0f;
          meshEvapoTranspirationState[ii].stblCp     = 200000.0f;
          meshEvapoTranspirationState[ii].fastCp     = 200000.0f;
          meshEvapoTranspirationState[ii].lai        = 4.6f;
          meshEvapoTranspirationState[ii].sai        = 0.6f;
          meshEvapoTranspirationState[ii].cm         = 0.002f;
          meshEvapoTranspirationState[ii].ch         = 0.002f;
          meshEvapoTranspirationState[ii].tauss      = 0.0f;
          meshEvapoTranspirationState[ii].deepRech   = 0.0f;
          meshEvapoTranspirationState[ii].rech       = 0.0f;
        }
    }
  
  // If not already specified meshInfiltrationMethod defaults to NO_INFILTRATION if soil type is bedrock or soil depth is zero.  Otherwise it defaults to
  // TRIVIAL_INFILTRATION.  FIXME implement GARTO.
  if (NULL == meshInfiltrationMethod)
    {
      meshInfiltrationMethod = new InfiltrationAndGroundwater::InfiltrationMethodEnum[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          if ((NULL != meshSoilType && 15 == meshSoilType[ii]) || (NULL != meshElementSoilDepth && 0.0 == meshElementSoilDepth[ii]))
            {
              meshInfiltrationMethod[ii] = InfiltrationAndGroundwater::NO_INFILTRATION;
            }
          else
            {
              meshInfiltrationMethod[ii] = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
            }
        }
    }
  
  // If not already specified meshGroundwaterMethod defaults to NO_AQUIFER if meshInfiltrationMethod is NO_INFILTRATION.  Otherwise, it defaults to
  // DEEP_AQUIFER for non-alluvium elements and SHALLOW_AQUIFER for alluvium elements.  If meshAlluvium is not specified it is assumed that all elements are
  // alluvium.
  if (NULL == meshGroundwaterMethod)
    {
      meshGroundwaterMethod = new InfiltrationAndGroundwater::GroundwaterMethodEnum[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          if (NULL != meshInfiltrationMethod && InfiltrationAndGroundwater::NO_INFILTRATION == meshInfiltrationMethod[ii])
            {
              meshGroundwaterMethod[ii] = InfiltrationAndGroundwater::NO_AQUIFER;
            }
          else if (NULL != meshAlluvium && !meshAlluvium[ii])
            {
              meshGroundwaterMethod[ii] = InfiltrationAndGroundwater::DEEP_AQUIFER;
            }
          else
            {
              meshGroundwaterMethod[ii] = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
            }
        }
    }
  
  // If not already specified meshGroundwaterHead defaults to meshElementZSurface for SHALLOW_AQUIFER and 10 meters below meshElementLayerZBottom for
  // DEEP_AQUIFER and NO_AQUIFER.
  if (NULL == meshGroundwaterHead && NULL != meshElementZSurface && NULL != meshElementLayerZBottom && NULL != meshGroundwaterMethod)
    {
      meshGroundwaterHead = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          if (InfiltrationAndGroundwater::SHALLOW_AQUIFER == meshGroundwaterMethod[ii])
            {
              meshGroundwaterHead[ii] = meshElementZSurface[ii];
            }
          else
            {
              meshGroundwaterHead[ii] = meshElementLayerZBottom[ii] - 10.0;
            }
        }
    }

  // Calculate meshMeshNeighborsEdgeLength from the X and Y coordinates of its vertices.
  if (NULL == meshMeshNeighborsEdgeLength && NULL != meshVertexX && NULL != meshVertexY)
    {
      meshMeshNeighborsEdgeLength = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeLength[ii][jj] = sqrt((meshVertexX[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] -
                                                          meshVertexX[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]) *
                                                         (meshVertexX[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] -
                                                          meshVertexX[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]) +
                                                         (meshVertexY[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] -
                                                          meshVertexY[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]) *
                                                         (meshVertexY[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] -
                                                          meshVertexY[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]));
            }
        }
    }

  // Calculate meshMeshNeighborsEdgeNormalX from the Y coordinate of its vertices and edge length.
  if (NULL == meshMeshNeighborsEdgeNormalX && NULL != meshVertexY && NULL != meshMeshNeighborsEdgeLength)
    {
      meshMeshNeighborsEdgeNormalX = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeNormalX[ii][jj] = (meshVertexY[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] -
                                                      meshVertexY[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]) / meshMeshNeighborsEdgeLength[ii][jj];
            }
        }
    }

  // Calculate meshMeshNeighborsEdgeNormalY from the X coordinate of its vertices and edge length.
  if (NULL == meshMeshNeighborsEdgeNormalY && NULL != meshVertexX && NULL != meshMeshNeighborsEdgeLength)
    {
      meshMeshNeighborsEdgeNormalY = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeNormalY[ii][jj] = (meshVertexX[ii][(jj + 1) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE] -
                                                      meshVertexX[ii][(jj + 2) % MESH_ELEMENT_MESH_NEIGHBORS_SIZE]) / meshMeshNeighborsEdgeLength[ii][jj];
            }
        }
    }
  
  // If not already specified meshSurfacewaterMeshNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.
  if (NULL == meshSurfacewaterMeshNeighborsConnection && NULL != meshMeshNeighbors)
    {
      meshSurfacewaterMeshNeighborsConnection = new bool[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterMeshNeighborsConnection[ii][jj] = (NOFLOW != meshMeshNeighbors[ii][jj]);
            }
        }
    }
  
  // If not already specified meshSurfacewaterMeshNeighborsFlowRate defaults to zero.
  if (NULL == meshSurfacewaterMeshNeighborsFlowRate)
    {
      meshSurfacewaterMeshNeighborsFlowRate = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterMeshNeighborsFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm defaults to zero.
  if (NULL == meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm)
    {
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm defaults to zero.
  if (NULL == meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm)
    {
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshGroundwaterMeshNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.
  if (NULL == meshGroundwaterMeshNeighborsConnection && NULL != meshMeshNeighbors)
    {
      meshGroundwaterMeshNeighborsConnection = new bool[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterMeshNeighborsConnection[ii][jj] = (NOFLOW != meshMeshNeighbors[ii][jj]);
            }
        }
    }
  
  // If not already specified meshGroundwaterMeshNeighborsFlowRate defaults to zero.
  if (NULL == meshGroundwaterMeshNeighborsFlowRate)
    {
      meshGroundwaterMeshNeighborsFlowRate = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterMeshNeighborsFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshGroundwaterMeshNeighborsFlowCumulativeShortTerm defaults to zero.
  if (NULL == meshGroundwaterMeshNeighborsFlowCumulativeShortTerm)
    {
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshGroundwaterMeshNeighborsFlowCumulativeLongTerm defaults to zero.
  if (NULL == meshGroundwaterMeshNeighborsFlowCumulativeLongTerm)
    {
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshSurfacewaterChannelNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.
  if (NULL == meshSurfacewaterChannelNeighborsConnection && NULL != meshChannelNeighbors)
    {
      meshSurfacewaterChannelNeighborsConnection = new bool[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterChannelNeighborsConnection[ii][jj] = (NOFLOW != meshChannelNeighbors[ii][jj]);
            }
        }
    }
  
  // If not already specified meshSurfacewaterChannelNeighborsFlowRate defaults to zero.
  if (NULL == meshSurfacewaterChannelNeighborsFlowRate)
    {
      meshSurfacewaterChannelNeighborsFlowRate = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterChannelNeighborsFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm defaults to zero.
  if (NULL == meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm)
    {
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm defaults to zero.
  if (NULL == meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm)
    {
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshGroundwaterChannelNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.
  if (NULL == meshGroundwaterChannelNeighborsConnection && NULL != meshChannelNeighbors)
    {
      meshGroundwaterChannelNeighborsConnection = new bool[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterChannelNeighborsConnection[ii][jj] = (NOFLOW != meshChannelNeighbors[ii][jj]);
            }
        }
    }
  
  // If not already specified meshGroundwaterChannelNeighborsFlowRate defaults to zero.
  if (NULL == meshGroundwaterChannelNeighborsFlowRate)
    {
      meshGroundwaterChannelNeighborsFlowRate = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterChannelNeighborsFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshGroundwaterChannelNeighborsFlowCumulativeShortTerm defaults to zero.
  if (NULL == meshGroundwaterChannelNeighborsFlowCumulativeShortTerm)
    {
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified meshGroundwaterChannelNeighborsFlowCumulativeLongTerm defaults to zero.
  if (NULL == meshGroundwaterChannelNeighborsFlowCumulativeLongTerm)
    {
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // Calculate channelElementX, channelElementY, channelElementZBank from channel type, channel vertices, and channel length.
  if ((NULL == channelElementX || NULL == channelElementY || NULL == channelElementZBank) && NULL != channelChannelType && NULL != channelVertexX &&
      NULL != channelVertexY && NULL != channelVertexZBank && NULL != channelElementLength)
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
              
              for (jj = 1; jj < CHANNEL_ELEMENT_VERTICES_SIZE; jj++)
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
  
  // Calculate channelLatitude and channelLongitude from channelElementX and channelElementY.
  if ((NULL == channelLatitude || NULL == channelLongitude) && NULL != channelElementX && NULL != channelElementY)
    {
      if (NULL == channelLatitude)
        {
          channelLatitude = new double[localNumberOfChannelElements];
        }
      
      if (NULL == channelLongitude)
        {
          channelLongitude = new double[localNumberOfChannelElements];
        }
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          ADHydro::getLatLong(channelElementX[ii],  channelElementY[ii],  channelLatitude[ii],  channelLongitude[ii]);
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
  
  // If not already specified channelPrecipitationRate defaults to zero.
  if (NULL == channelPrecipitationRate)
    {
      channelPrecipitationRate = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelPrecipitationRate[ii] = 0.0;
        }
    }
  
  // If not already specified channelPrecipitationCumulativeShortTerm defaults to zero.
  if (NULL == channelPrecipitationCumulativeShortTerm)
    {
      channelPrecipitationCumulativeShortTerm = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelPrecipitationCumulativeShortTerm[ii] = 0.0;
        }
    }
  
  // If not already specified channelPrecipitationCumulativeLongTerm defaults to zero.
  if (NULL == channelPrecipitationCumulativeLongTerm)
    {
      channelPrecipitationCumulativeLongTerm = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelPrecipitationCumulativeLongTerm[ii] = 0.0;
        }
    }
  
  // If not already specified channelEvaporationRate defaults to zero.
  if (NULL == channelEvaporationRate)
    {
      channelEvaporationRate = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelEvaporationRate[ii] = 0.0;
        }
    }
  
  // If not already specified channelEvaporationCumulativeShortTerm defaults to zero.
  if (NULL == channelEvaporationCumulativeShortTerm)
    {
      channelEvaporationCumulativeShortTerm = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelEvaporationCumulativeShortTerm[ii] = 0.0;
        }
    }
  
  // If not already specified channelEvaporationCumulativeLongTerm defaults to zero.
  if (NULL == channelEvaporationCumulativeLongTerm)
    {
      channelEvaporationCumulativeLongTerm = new double[localNumberOfChannelElements];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelEvaporationCumulativeLongTerm[ii] = 0.0;
        }
    }

  // If not already specified channelEvapoTranspirationState defaults to nominal values.
  if (NULL == channelEvapoTranspirationState)
    {
      channelEvapoTranspirationState = new EvapoTranspirationStateStruct[localNumberOfChannelElements];

      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          channelEvapoTranspirationState[ii].fIceOld[0] = 0.0f;
          channelEvapoTranspirationState[ii].fIceOld[1] = 0.0f;
          channelEvapoTranspirationState[ii].fIceOld[2] = 0.0f;
          channelEvapoTranspirationState[ii].albOld     = 1.0f;
          channelEvapoTranspirationState[ii].snEqvO     = 0.0f;
          channelEvapoTranspirationState[ii].stc[0]     = 0.0f;
          channelEvapoTranspirationState[ii].stc[1]     = 0.0f;
          channelEvapoTranspirationState[ii].stc[2]     = 0.0f;
          channelEvapoTranspirationState[ii].stc[3]     = 300.0f;
          channelEvapoTranspirationState[ii].stc[4]     = 300.0f;
          channelEvapoTranspirationState[ii].stc[5]     = 300.0f;
          channelEvapoTranspirationState[ii].stc[6]     = 300.0f;
          channelEvapoTranspirationState[ii].tah        = 300.0f;
          channelEvapoTranspirationState[ii].eah        = 2000.0f;
          channelEvapoTranspirationState[ii].fWet       = 0.0f;
          channelEvapoTranspirationState[ii].canLiq     = 0.0f;
          channelEvapoTranspirationState[ii].canIce     = 0.0f;
          channelEvapoTranspirationState[ii].tv         = 300.0f;
          channelEvapoTranspirationState[ii].tg         = 300.0f;
          channelEvapoTranspirationState[ii].iSnow      = 0;
          channelEvapoTranspirationState[ii].zSnso[0]   = 0.0f;
          channelEvapoTranspirationState[ii].zSnso[1]   = 0.0f;
          channelEvapoTranspirationState[ii].zSnso[2]   = 0.0f;
          
          if (NULL != channelBedThickness && 0.0 < channelBedThickness[ii])
            {
              channelEvapoTranspirationState[ii].zSnso[3]   = -0.05 * channelBedThickness[ii];
              channelEvapoTranspirationState[ii].zSnso[4]   = -0.2  * channelBedThickness[ii];
              channelEvapoTranspirationState[ii].zSnso[5]   = -0.5  * channelBedThickness[ii];
              channelEvapoTranspirationState[ii].zSnso[6]   = -1.0  * channelBedThickness[ii];
            }
          else
            {
              // Noah-MP cannot handle zero soil depth.  Use one meter instead.
              channelEvapoTranspirationState[ii].zSnso[3]   = -0.05f;
              channelEvapoTranspirationState[ii].zSnso[4]   = -0.2f;
              channelEvapoTranspirationState[ii].zSnso[5]   = -0.5f;
              channelEvapoTranspirationState[ii].zSnso[6]   = -1.0f;
            }
          
          channelEvapoTranspirationState[ii].snowH      = 0.0f;
          channelEvapoTranspirationState[ii].snEqv      = 0.0f;
          channelEvapoTranspirationState[ii].snIce[0]   = 0.0f;
          channelEvapoTranspirationState[ii].snIce[1]   = 0.0f;
          channelEvapoTranspirationState[ii].snIce[2]   = 0.0f;
          channelEvapoTranspirationState[ii].snLiq[0]   = 0.0f;
          channelEvapoTranspirationState[ii].snLiq[1]   = 0.0f;
          channelEvapoTranspirationState[ii].snLiq[2]   = 0.0f;
          channelEvapoTranspirationState[ii].lfMass     = 100000.0f;
          channelEvapoTranspirationState[ii].rtMass     = 100000.0f;
          channelEvapoTranspirationState[ii].stMass     = 100000.0f;
          channelEvapoTranspirationState[ii].wood       = 200000.0f;
          channelEvapoTranspirationState[ii].stblCp     = 200000.0f;
          channelEvapoTranspirationState[ii].fastCp     = 200000.0f;
          channelEvapoTranspirationState[ii].lai        = 4.6f;
          channelEvapoTranspirationState[ii].sai        = 0.6f;
          channelEvapoTranspirationState[ii].cm         = 0.002f;
          channelEvapoTranspirationState[ii].ch         = 0.002f;
          channelEvapoTranspirationState[ii].tauss      = 0.0f;
          channelEvapoTranspirationState[ii].deepRech   = 0.0f;
          channelEvapoTranspirationState[ii].rech       = 0.0f;
        }
    }
  
  // If not already specified channelSurfacewaterMeshNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.
  if (NULL == channelSurfacewaterMeshNeighborsConnection && NULL != channelMeshNeighbors)
    {
      channelSurfacewaterMeshNeighborsConnection = new bool[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterMeshNeighborsConnection[ii][jj] = (NOFLOW != channelMeshNeighbors[ii][jj]);
            }
        }
    }
  
  // If not already specified channelSurfacewaterMeshNeighborsFlowRate defaults to zero.
  if (NULL == channelSurfacewaterMeshNeighborsFlowRate)
    {
      channelSurfacewaterMeshNeighborsFlowRate = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterMeshNeighborsFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm defaults to zero.
  if (NULL == channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm)
    {
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm defaults to zero.
  if (NULL == channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm)
    {
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelGroundwaterMeshNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.
  if (NULL == channelGroundwaterMeshNeighborsConnection && NULL != channelMeshNeighbors)
    {
      channelGroundwaterMeshNeighborsConnection = new bool[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelGroundwaterMeshNeighborsConnection[ii][jj] = (NOFLOW != channelMeshNeighbors[ii][jj]);
            }
        }
    }
  
  // If not already specified channelGroundwaterMeshNeighborsFlowRate defaults to zero.
  if (NULL == channelGroundwaterMeshNeighborsFlowRate)
    {
      channelGroundwaterMeshNeighborsFlowRate = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelGroundwaterMeshNeighborsFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelGroundwaterMeshNeighborsFlowCumulativeShortTerm defaults to zero.
  if (NULL == channelGroundwaterMeshNeighborsFlowCumulativeShortTerm)
    {
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelGroundwaterMeshNeighborsFlowCumulativeLongTerm defaults to zero.
  if (NULL == channelGroundwaterMeshNeighborsFlowCumulativeLongTerm)
    {
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelSurfacewaterChannelNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.
  if (NULL == channelSurfacewaterChannelNeighborsConnection && NULL != channelChannelNeighbors)
    {
      channelSurfacewaterChannelNeighborsConnection = new bool[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterChannelNeighborsConnection[ii][jj] = (NOFLOW != channelChannelNeighbors[ii][jj]);
            }
        }
    }
  
  // If not already specified channelSurfacewaterChannelNeighborsFlowRate defaults to zero.
  if (NULL == channelSurfacewaterChannelNeighborsFlowRate)
    {
      channelSurfacewaterChannelNeighborsFlowRate = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterChannelNeighborsFlowRate[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm defaults to zero.
  if (NULL == channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm)
    {
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[ii][jj] = 0.0;
            }
        }
    }
  
  // If not already specified channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm defaults to zero.
  if (NULL == channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm)
    {
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ii][jj] = 0.0;
            }
        }
    }

  // Allocate arrays to record when state update messages are received.
  // FIXME we will need this eventually.
  //if (0 < localNumberOfMeshElements)
  //  {
  //    meshElementUpdated = new bool[localNumberOfMeshElements];
  //  }
  //
  //if (0 < localNumberOfChannelElements)
  //  {
  //    channelElementUpdated = new bool[localNumberOfChannelElements];
  //  }
  
  // Fix digital dams and similar problems.
  if (ADHydro::doMeshMassage)
    {
      meshMassage();
    }
}

void FileManager::handleSendInitializationMessages(CProxy_Region regionProxy)
{
  bool                            error = false;                // Error flag.
  int                             ii, jj, kk;                   // Loop counters.
  std::vector<simpleNeighborInfo> surfacewaterMeshNeighbors;    // For initializing mesh neighbors.
  std::vector<simpleNeighborInfo> surfacewaterChannelNeighbors; // For initializing mesh neighbors.
  std::vector<simpleNeighborInfo> groundwaterMeshNeighbors;     // For initializing mesh neighbors.
  std::vector<simpleNeighborInfo> groundwaterChannelNeighbors;  // For initializing mesh neighbors.
  
  // FIXME do we want to load forcing data at init time or later?
  EvapoTranspirationForcingStruct evapoTranspirationForcingInit;
  
  evapoTranspirationForcingInit.dz8w   = 20.0;
  evapoTranspirationForcingInit.sfcTmp = 300.0;
  evapoTranspirationForcingInit.sfcPrs = 101300.0;
  evapoTranspirationForcingInit.psfc   = 101200.0;
  evapoTranspirationForcingInit.uu     = 0.0;
  evapoTranspirationForcingInit.vv     = 0.0;
  evapoTranspirationForcingInit.q2     = 0.0;
  evapoTranspirationForcingInit.qc     = 0.0;
  evapoTranspirationForcingInit.solDn  = 1000.0;
  evapoTranspirationForcingInit.lwDn   = 300.0;
  evapoTranspirationForcingInit.prcp   = 0.0;
  evapoTranspirationForcingInit.tBot   = 300.0;
  evapoTranspirationForcingInit.pblh   = 10000.0;
  
  // The region array is not created before the end of the mainchare
  // constructor so its proxy is not copied to all PEs by the Charm++ readonly
  // construct.  This sets the correct value on all PEs.
  ADHydro::regionProxy = regionProxy;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  // Error check for existence of arrays.
  if (0 < localNumberOfMeshElements)
    {
      if (!(NULL != meshVertexX))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshVertexX must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshVertexY))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshVertexY must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshRegion))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshRegion must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshCatchment))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshCatchment must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshVegetationType))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshVegetationType must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSoilType))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSoilType must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshElementX))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshElementX must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshElementY))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshElementY must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshElementZSurface))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshElementZSurface must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshElementLayerZBottom))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshElementLayerZBottom must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshElementArea))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshElementArea must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshElementSlopeX))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshElementSlopeX must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshElementSlopeY))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshElementSlopeY must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshLatitude))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshLatitude must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshLongitude))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshLongitude must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshManningsN))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshManningsN must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshConductivity))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshConductivity must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshPorosity))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshPorosity must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterDepth))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterDepth must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterError))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterError must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterHead))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterHead must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterRecharge))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterRecharge must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterError))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterError must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshPrecipitationRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshPrecipitationRate must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshPrecipitationCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshPrecipitationCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshPrecipitationCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshPrecipitationCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshEvaporationRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshEvaporationRate must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshEvaporationCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshEvaporationCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshEvaporationCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshEvaporationCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshTranspirationRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshTranspirationRate must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshTranspirationCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshTranspirationCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshTranspirationCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshTranspirationCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshEvapoTranspirationState))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshEvapoTranspirationState must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshInfiltrationMethod))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshInfiltrationMethod must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterMethod))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMethod must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshMeshNeighbors))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshMeshNeighbors must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshMeshNeighborsRegion))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshMeshNeighborsRegion must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshMeshNeighborsEdgeLength))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshMeshNeighborsEdgeLength must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshMeshNeighborsEdgeNormalX))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshMeshNeighborsEdgeNormalX must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshMeshNeighborsEdgeNormalY))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshMeshNeighborsEdgeNormalY must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterMeshNeighborsConnection))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsConnection must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterMeshNeighborsConnection))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsConnection must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterMeshNeighborsFlowCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsFlowCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterMeshNeighborsFlowCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsFlowCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshChannelNeighbors))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshChannelNeighbors must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshChannelNeighborsRegion))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshChannelNeighborsRegion must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshChannelNeighborsEdgeLength))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshChannelNeighborsEdgeLength must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterChannelNeighborsConnection))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterChannelNeighborsConnection must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterChannelNeighborsConnection))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterChannelNeighborsConnection must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterChannelNeighborsFlowCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterChannelNeighborsFlowCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterChannelNeighborsFlowCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterChannelNeighborsFlowCumulativeLongTerm must not be NULL.\n");
          error = true;
        }
    }
  
  if (0 < localNumberOfChannelElements)
    {
      if (!(NULL != channelRegion))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelRegion must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelChannelType))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelChannelType must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelReachCode))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelReachCode must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelElementX))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelElementX must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelElementY))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelElementY must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelElementZBank))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelElementZBank must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelElementZBed))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelElementZBed must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelElementLength))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelElementLength must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelBaseWidth))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelBaseWidth must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSideSlope))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSideSlope must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelBedConductivity))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelBedConductivity must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelBedThickness))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelBedThickness must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelManningsN))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelManningsN must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterDepth))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterDepth must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterError))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterError must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelMeshNeighbors))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelMeshNeighbors must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelMeshNeighborsRegion))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelMeshNeighborsRegion must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelMeshNeighborsEdgeLength))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelMeshNeighborsEdgeLength must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterMeshNeighborsConnection))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterMeshNeighborsConnection must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelGroundwaterMeshNeighborsConnection))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelGroundwaterMeshNeighborsConnection must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelGroundwaterMeshNeighborsFlowCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelGroundwaterMeshNeighborsFlowCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelGroundwaterMeshNeighborsFlowCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelGroundwaterMeshNeighborsFlowCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelChannelNeighbors))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelChannelNeighbors must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelChannelNeighborsRegion))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelChannelNeighborsRegion must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterChannelNeighborsConnection))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterChannelNeighborsConnection must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm must not be NULL.\n");
          error = true;
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

  for (ii = 0; !error && ii < localNumberOfMeshElements; ++ii)
    {
      surfacewaterMeshNeighbors.clear();
      groundwaterMeshNeighbors.clear();
      surfacewaterChannelNeighbors.clear();
      groundwaterChannelNeighbors.clear();

      for (jj = 0; !error && jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; ++jj)
        {
          // Invalid neighbors can cause the simulation to hang during initialization before reaching the invariant check so we are checking these here.
          if (0 <= meshMeshNeighbors[ii][jj] && meshMeshNeighbors[ii][jj] < globalNumberOfMeshElements)
            {
              // Self-neighbors and duplicate neighbors are not allowed.
              if (ii == meshMeshNeighbors[ii][jj])
                {
                  CkError("ERROR in FileManager::handleSendInitializationMessages: mesh element %d mesh neighbor %d is a self-neighbor, which is not "
                          "allowed.\n", ii, jj);
                  error = true;
                }
              else
                {
                  for (kk = 0; kk < jj; ++kk)
                    {
                      if (meshMeshNeighbors[ii][kk] == meshMeshNeighbors[ii][jj])
                        {
                          CkError("ERROR in FileManager::handleSendInitializationMessages: mesh element %d mesh neighbor %d is a duplicate neighbor, which is "
                                  "not allowed.\n", ii, jj);
                          error = true;
                        }
                    }
                }
            }
          else if (!(isBoundary(meshMeshNeighbors[ii][jj])))
            {
              CkError("ERROR in FileManager::handleSendInitializationMessages: mesh element %d mesh neighbor %d must be a valid element number or "
                      "boundary condition code.\n", ii, jj);
              error = true;
            }

          if (!error && NOFLOW != meshMeshNeighbors[ii][jj])
            {
              if (meshSurfacewaterMeshNeighborsConnection[ii][jj])
                {
                  surfacewaterMeshNeighbors.push_back(simpleNeighborInfo(meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], meshMeshNeighborsRegion[ii][jj], meshMeshNeighbors[ii][jj],
                      meshMeshNeighborsEdgeLength[ii][jj], meshMeshNeighborsEdgeNormalX[ii][jj], meshMeshNeighborsEdgeNormalY[ii][jj]));
                }

              if (meshGroundwaterMeshNeighborsConnection[ii][jj])
                {
                  groundwaterMeshNeighbors.push_back(simpleNeighborInfo(meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], meshMeshNeighborsRegion[ii][jj], meshMeshNeighbors[ii][jj],
                      meshMeshNeighborsEdgeLength[ii][jj], meshMeshNeighborsEdgeNormalX[ii][jj], meshMeshNeighborsEdgeNormalY[ii][jj]));
                }
            }
        }

      for (jj = 0; !error && jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; ++jj)
        {
          // Invalid neighbors can cause the simulation to hang during initialization before reaching the invariant check so we are checking these here.
          if (0 <= meshChannelNeighbors[ii][jj] && meshChannelNeighbors[ii][jj] < globalNumberOfChannelElements)
            {
              // Duplicate neighbors are not allowed.
              for (kk = 0; kk < jj; ++kk)
                {
                  if (meshChannelNeighbors[ii][kk] == meshChannelNeighbors[ii][jj])
                    {
                      CkError("ERROR in FileManager::handleSendInitializationMessages: mesh element %d channel neighbor %d is a duplicate neighbor, which is "
                              "not allowed.\n", ii, jj);
                      error = true;
                    }
                }
            }
          else if (!(isBoundary(meshChannelNeighbors[ii][jj])))
            {
              CkError("ERROR in FileManager::handleSendInitializationMessages: mesh element %d channel neighbor %d must be a valid element number or "
                      "boundary condition code.\n", ii, jj);
              error = true;
            }
          
          if (!error && NOFLOW != meshChannelNeighbors[ii][jj])
            {
              if (meshSurfacewaterChannelNeighborsConnection[ii][jj])
                {
                  surfacewaterChannelNeighbors.push_back(simpleNeighborInfo(meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ii][jj], meshChannelNeighborsRegion[ii][jj], meshChannelNeighbors[ii][jj],
                      meshChannelNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }

              if (meshGroundwaterChannelNeighborsConnection[ii][jj])
                {
                  groundwaterChannelNeighbors.push_back(simpleNeighborInfo(meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[ii][jj], meshChannelNeighborsRegion[ii][jj], meshChannelNeighbors[ii][jj],
                      meshChannelNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }
            }
        }
      
      if (!error)
        {
          regionProxy[meshRegion[ii]].sendInitializeMeshElement(ii + localMeshElementStart, meshCatchment[ii], meshVegetationType[ii], meshSoilType[ii],
              meshVertexX[ii], meshVertexY[ii], meshElementX[ii], meshElementY[ii], meshElementZSurface[ii], meshElementLayerZBottom[ii], meshElementArea[ii],
              meshElementSlopeX[ii], meshElementSlopeY[ii], meshLatitude[ii], meshLongitude[ii], meshManningsN[ii], meshConductivity[ii], meshPorosity[ii],
              meshSurfacewaterDepth[ii], meshSurfacewaterError[ii], meshGroundwaterHead[ii], meshGroundwaterRecharge[ii], meshGroundwaterError[ii],
              meshPrecipitationRate[ii], meshPrecipitationCumulativeShortTerm[ii], meshPrecipitationCumulativeLongTerm[ii], meshEvaporationRate[ii],
              meshEvaporationCumulativeShortTerm[ii], meshEvaporationCumulativeLongTerm[ii], meshTranspirationRate[ii], meshTranspirationCumulativeShortTerm[ii],
              meshTranspirationCumulativeLongTerm[ii], evapoTranspirationForcingInit, meshEvapoTranspirationState[ii], meshInfiltrationMethod[ii],
              meshGroundwaterMethod[ii], surfacewaterMeshNeighbors, surfacewaterChannelNeighbors, groundwaterMeshNeighbors, groundwaterChannelNeighbors);
        }
    }

  for (ii = 0; !error && ii < localNumberOfChannelElements; ++ii)
    {
      surfacewaterMeshNeighbors.clear();
      groundwaterMeshNeighbors.clear();
      surfacewaterChannelNeighbors.clear();

      for (jj = 0; !error && jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; ++jj)
        {
          // Invalid neighbors can cause the simulation to hang during initialization before reaching the invariant check so we are checking these here.
          if (0 <= channelMeshNeighbors[ii][jj] && channelMeshNeighbors[ii][jj] < globalNumberOfMeshElements)
            {
              // Duplicate neighbors are not allowed.
              for (kk = 0; kk < jj; ++kk)
                {
                  if (channelMeshNeighbors[ii][kk] == channelMeshNeighbors[ii][jj])
                    {
                      CkError("ERROR in FileManager::handleSendInitializationMessages: channel element %d mesh neighbor %d is a duplicate neighbor, which is "
                              "not allowed.\n", ii, jj);
                      error = true;
                    }
                }
            }
          else if (!(isBoundary(channelMeshNeighbors[ii][jj])))
            {
              CkError("ERROR in FileManager::handleSendInitializationMessages: channel element %d mesh neighbor %d must be a valid element number or "
                      "boundary condition code.\n", ii, jj);
              error = true;
            }
          
          if (!error && NOFLOW != channelMeshNeighbors[ii][jj])
            {
              if (channelSurfacewaterMeshNeighborsConnection[ii][jj])
                {
                  surfacewaterMeshNeighbors.push_back(simpleNeighborInfo(channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], channelMeshNeighborsRegion[ii][jj], channelMeshNeighbors[ii][jj],
                      channelMeshNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }

              if (channelGroundwaterMeshNeighborsConnection[ii][jj])
                {
                  groundwaterMeshNeighbors.push_back(simpleNeighborInfo(channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], channelMeshNeighborsRegion[ii][jj], channelMeshNeighbors[ii][jj],
                      channelMeshNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }
            }
        }

      for (jj = 0; !error && jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; ++jj)
        {
          // Invalid neighbors can cause the simulation to hang during initialization before reaching the invariant check so we are checking these here.
          if (0 <= channelChannelNeighbors[ii][jj] && channelChannelNeighbors[ii][jj] < globalNumberOfChannelElements)
            {
              // Self-neighbors and duplicate neighbors are not allowed.
              if (ii == channelChannelNeighbors[ii][jj])
                {
                  CkError("ERROR in FileManager::handleSendInitializationMessages: channel element %d channel neighbor %d is a self-neighbor, which is not "
                          "allowed.\n", ii, jj);
                  error = true;
                }
              else
                {
                  for (kk = 0; kk < jj; ++kk)
                    {
                      if (channelChannelNeighbors[ii][kk] == channelChannelNeighbors[ii][jj])
                        {
                          CkError("ERROR in FileManager::handleSendInitializationMessages: channel element %d channel neighbor %d is a duplicate neighbor, "
                                  "which is not allowed.\n", ii, jj);
                          error = true;
                        }
                    }
                }
            }
          else if (!(isBoundary(channelChannelNeighbors[ii][jj])))
            {
              CkError("ERROR in FileManager::handleSendInitializationMessages: channel element %d channel neighbor %d must be a valid element number or "
                      "boundary condition code.\n", ii, jj);
              error = true;
            }
          
          if (!error && NOFLOW != channelChannelNeighbors[ii][jj] && channelSurfacewaterChannelNeighborsConnection[ii][jj])
            {
              surfacewaterChannelNeighbors.push_back(simpleNeighborInfo(channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
                  channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ii][jj], channelChannelNeighborsRegion[ii][jj],
                  channelChannelNeighbors[ii][jj], 1.0, 1.0, 0.0));
            }
        }

      if (!error)
        {
          regionProxy[channelRegion[ii]].sendInitializeChannelElement(ii + localChannelElementStart, channelChannelType[ii], channelReachCode[ii],
              channelElementX[ii], channelElementY[ii], channelElementZBank[ii], channelElementZBed[ii], channelElementLength[ii], channelBaseWidth[ii],
              channelSideSlope[ii], channelBedConductivity[ii], channelBedThickness[ii], channelManningsN[ii], channelSurfacewaterDepth[ii],
              channelSurfacewaterError[ii], surfacewaterMeshNeighbors, surfacewaterChannelNeighbors, groundwaterMeshNeighbors);
        }
    }

  if (error)
    {
      CkExit();
    }
}

#include "file_manager.def.h"
