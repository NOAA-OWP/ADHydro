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
  meshSurfacewaterMeshNeighbors(NULL),
  meshSurfacewaterMeshNeighborsRegion(NULL),
  meshSurfacewaterMeshNeighborsEdgeLength(NULL),
  meshSurfacewaterMeshNeighborsEdgeNormalX(NULL),
  meshSurfacewaterMeshNeighborsEdgeNormalY(NULL),
  meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  meshSurfacewaterChannelNeighbors(NULL),
  meshSurfacewaterChannelNeighborsRegion(NULL),
  meshSurfacewaterChannelNeighborsEdgeLength(NULL),
  meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm(NULL),
  meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm(NULL),
  meshGroundwaterMeshNeighbors(NULL),
  meshGroundwaterMeshNeighborsRegion(NULL),
  meshGroundwaterMeshNeighborsEdgeLength(NULL),
  meshGroundwaterMeshNeighborsEdgeNormalX(NULL),
  meshGroundwaterMeshNeighborsEdgeNormalY(NULL),
  meshGroundwaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  meshGroundwaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  meshGroundwaterChannelNeighbors(NULL),
  meshGroundwaterChannelNeighborsRegion(NULL),
  meshGroundwaterChannelNeighborsEdgeLength(NULL),
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
  channelBaseWidth(NULL),
  channelSideSlope(NULL),
  channelBedConductivity(NULL),
  channelBedThickness(NULL),
  channelManningsN(NULL),
  channelSurfacewaterDepth(NULL),
  channelSurfacewaterError(NULL),
  channelSurfacewaterMeshNeighbors(NULL),
  channelSurfacewaterMeshNeighborsRegion(NULL),
  channelSurfacewaterMeshNeighborsEdgeLength(NULL),
  channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  channelSurfacewaterChannelNeighbors(NULL),
  channelSurfacewaterChannelNeighborsRegion(NULL),
  channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm(NULL),
  channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm(NULL),
  channelGroundwaterMeshNeighbors(NULL),
  channelGroundwaterMeshNeighborsRegion(NULL),
  channelGroundwaterMeshNeighborsEdgeLength(NULL),
  channelGroundwaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  channelGroundwaterMeshNeighborsFlowCumulativeLongTerm(NULL)
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
  delete[] meshSurfacewaterMeshNeighbors;
  delete[] meshSurfacewaterMeshNeighborsRegion;
  delete[] meshSurfacewaterMeshNeighborsEdgeLength;
  delete[] meshSurfacewaterMeshNeighborsEdgeNormalX;
  delete[] meshSurfacewaterMeshNeighborsEdgeNormalY;
  delete[] meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] meshSurfacewaterChannelNeighbors;
  delete[] meshSurfacewaterChannelNeighborsRegion;
  delete[] meshSurfacewaterChannelNeighborsEdgeLength;
  delete[] meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm;
  delete[] meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm;
  delete[] meshGroundwaterMeshNeighbors;
  delete[] meshGroundwaterMeshNeighborsRegion;
  delete[] meshGroundwaterMeshNeighborsEdgeLength;
  delete[] meshGroundwaterMeshNeighborsEdgeNormalX;
  delete[] meshGroundwaterMeshNeighborsEdgeNormalY;
  delete[] meshGroundwaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] meshGroundwaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] meshGroundwaterChannelNeighbors;
  delete[] meshGroundwaterChannelNeighborsRegion;
  delete[] meshGroundwaterChannelNeighborsEdgeLength;
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
  delete[] channelBaseWidth;
  delete[] channelSideSlope;
  delete[] channelBedConductivity;
  delete[] channelBedThickness;
  delete[] channelManningsN;
  delete[] channelSurfacewaterDepth;
  delete[] channelSurfacewaterError;
  delete[] channelSurfacewaterMeshNeighbors;
  delete[] channelSurfacewaterMeshNeighborsRegion;
  delete[] channelSurfacewaterMeshNeighborsEdgeLength;
  delete[] channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] channelSurfacewaterChannelNeighbors;
  delete[] channelSurfacewaterChannelNeighborsRegion;
  delete[] channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm;
  delete[] channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm;
  delete[] channelGroundwaterMeshNeighbors;
  delete[] channelGroundwaterMeshNeighborsRegion;
  delete[] channelGroundwaterMeshNeighborsEdgeLength;
  delete[] channelGroundwaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] channelGroundwaterMeshNeighborsFlowCumulativeLongTerm;
}

void FileManager::initializeFromASCIIFiles()
{
  // FIXME not implemented
  setUpHardcodedMesh();
}

void FileManager::initializeFromNetCDFFiles()
{
  // FIXME not implemented
  setUpHardcodedMesh();
}

// FIXME remove
void FileManager::setUpHardcodedMesh()
{
  EvapoTranspirationStateStruct evapoTranspirationStateInit;
  
  evapoTranspirationStateInit.fIceOld[0] = 0.0;
  evapoTranspirationStateInit.fIceOld[1] = 0.0;
  evapoTranspirationStateInit.fIceOld[2] = 0.0;
  evapoTranspirationStateInit.albOld     = 1.0;
  evapoTranspirationStateInit.snEqvO     = 0.0;
  evapoTranspirationStateInit.stc[0]     = 0.0;
  evapoTranspirationStateInit.stc[1]     = 0.0;
  evapoTranspirationStateInit.stc[2]     = 0.0;
  evapoTranspirationStateInit.stc[3]     = 300.0;
  evapoTranspirationStateInit.stc[4]     = 300.0;
  evapoTranspirationStateInit.stc[5]     = 300.0;
  evapoTranspirationStateInit.stc[6]     = 300.0;
  evapoTranspirationStateInit.tah        = 300.0;
  evapoTranspirationStateInit.eah        = 0.0;
  evapoTranspirationStateInit.fWet       = 0.0;
  evapoTranspirationStateInit.canLiq     = 0.0;
  evapoTranspirationStateInit.canIce     = 0.0;
  evapoTranspirationStateInit.tv         = 300.0;
  evapoTranspirationStateInit.tg         = 300.0;
  evapoTranspirationStateInit.iSnow      = 0;
  evapoTranspirationStateInit.zSnso[0]   = 0.0;
  evapoTranspirationStateInit.zSnso[1]   = 0.0;
  evapoTranspirationStateInit.zSnso[2]   = 0.0;
  evapoTranspirationStateInit.zSnso[3]   = -0.05;
  evapoTranspirationStateInit.zSnso[4]   = -0.2;
  evapoTranspirationStateInit.zSnso[5]   = -0.5;
  evapoTranspirationStateInit.zSnso[6]   = -1.0;
  evapoTranspirationStateInit.snowH      = 0.0;
  evapoTranspirationStateInit.snEqv      = 0.0;
  evapoTranspirationStateInit.snIce[0]   = 0.0;
  evapoTranspirationStateInit.snIce[1]   = 0.0;
  evapoTranspirationStateInit.snIce[2]   = 0.0;
  evapoTranspirationStateInit.snLiq[0]   = 0.0;
  evapoTranspirationStateInit.snLiq[1]   = 0.0;
  evapoTranspirationStateInit.snLiq[2]   = 0.0;
  evapoTranspirationStateInit.lfMass     = 100000.0;
  evapoTranspirationStateInit.rtMass     = 100000.0;
  evapoTranspirationStateInit.stMass     = 100000.0;
  evapoTranspirationStateInit.wood       = 200000.0;
  evapoTranspirationStateInit.stblCp     = 200000.0;
  evapoTranspirationStateInit.fastCp     = 200000.0;
  evapoTranspirationStateInit.lai        = 4.6;
  evapoTranspirationStateInit.sai        = 0.6;
  evapoTranspirationStateInit.cm         = 0.002;
  evapoTranspirationStateInit.ch         = 0.002;
  evapoTranspirationStateInit.tauss      = 0.0;
  evapoTranspirationStateInit.deepRech   = 0.0;
  evapoTranspirationStateInit.rech       = 0.0;

  if (0 == CkMyPe())
    {
      globalNumberOfRegions         = 3;
      localRegionStart              = 0;
      localNumberOfRegions          = 3;
      globalNumberOfMeshNodes       = 0;
      localMeshNodeStart            = 0;
      localNumberOfMeshNodes        = 0;
      globalNumberOfMeshElements    = 12;
      localMeshElementStart         = 0;
      localNumberOfMeshElements     = 12;
      globalNumberOfChannelNodes    = 0;
      localChannelNodeStart         = 0;
      localNumberOfChannelNodes     = 0;
      globalNumberOfChannelElements = 4;
      localChannelElementStart      = 0;
      localNumberOfChannelElements  = 4;
      
      regionNumberOfMeshElements = new int[3];
      
      regionNumberOfMeshElements[0] = 4;
      regionNumberOfMeshElements[1] = 4;
      regionNumberOfMeshElements[2] = 4;
      
      regionNumberOfChannelElements = new int[3];
      
      regionNumberOfChannelElements[0] = 1;
      regionNumberOfChannelElements[1] = 2;
      regionNumberOfChannelElements[2] = 1;
      
      meshVertexX = new DoubleArrayMMN[12];
      meshVertexY = new DoubleArrayMMN[12];
      
      meshVertexX[0][0]  =    0.0;
      meshVertexY[0][0]  =  600.0;
      meshVertexX[0][1]  =    0.0;
      meshVertexY[0][1]  =    0.0;
      meshVertexX[0][2]  =  600.0;
      meshVertexY[0][2]  =    0.0;
      meshVertexX[1][0]  =    0.0;
      meshVertexY[1][0]  =  600.0;
      meshVertexX[1][1]  =  600.0;
      meshVertexY[1][1]  =    0.0;
      meshVertexX[1][2]  =  600.0;
      meshVertexY[1][2]  =  600.0;
      meshVertexX[2][0]  =  600.0;
      meshVertexY[2][0]  =  600.0;
      meshVertexX[2][1]  =  600.0;
      meshVertexY[2][1]  =    0.0;
      meshVertexX[2][2]  = 1200.0;
      meshVertexY[2][2]  =    0.0;
      meshVertexX[3][0]  =  600.0;
      meshVertexY[3][0]  =  600.0;
      meshVertexX[3][1]  = 1200.0;
      meshVertexY[3][1]  =    0.0;
      meshVertexX[3][2]  = 1200.0;
      meshVertexY[3][2]  =  600.0;
      meshVertexX[4][0]  = 1200.0;
      meshVertexY[4][0]  =  600.0;
      meshVertexX[4][1]  = 1200.0;
      meshVertexY[4][1]  =    0.0;
      meshVertexX[4][2]  = 1800.0;
      meshVertexY[4][2]  =    0.0;
      meshVertexX[5][0]  = 1200.0;
      meshVertexY[5][0]  =  600.0;
      meshVertexX[5][1]  = 1800.0;
      meshVertexY[5][1]  =    0.0;
      meshVertexX[5][2]  = 1800.0;
      meshVertexY[5][2]  =  600.0;
      meshVertexX[6][0]  = 1800.0;
      meshVertexY[6][0]  =  600.0;
      meshVertexX[6][1]  = 1800.0;
      meshVertexY[6][1]  =    0.0;
      meshVertexX[6][2]  = 2400.0;
      meshVertexY[6][2]  =    0.0;
      meshVertexX[7][0]  = 1800.0;
      meshVertexY[7][0]  =  600.0;
      meshVertexX[7][1]  = 2400.0;
      meshVertexY[7][1]  =    0.0;
      meshVertexX[7][2]  = 2400.0;
      meshVertexY[7][2]  =  600.0;
      meshVertexX[8][0]  = 2400.0;
      meshVertexY[8][0]  =  600.0;
      meshVertexX[8][1]  = 2400.0;
      meshVertexY[8][1]  =    0.0;
      meshVertexX[8][2]  = 3000.0;
      meshVertexY[8][2]  =    0.0;
      meshVertexX[9][0]  = 2400.0;
      meshVertexY[9][0]  =  600.0;
      meshVertexX[9][1]  = 3000.0;
      meshVertexY[9][1]  =    0.0;
      meshVertexX[9][2]  = 3000.0;
      meshVertexY[9][2]  =  600.0;
      meshVertexX[10][0] = 3000.0;
      meshVertexY[10][0] =  600.0;
      meshVertexX[10][1] = 3000.0;
      meshVertexY[10][1] =    0.0;
      meshVertexX[10][2] = 3600.0;
      meshVertexY[10][2] =    0.0;
      meshVertexX[11][0] = 3000.0;
      meshVertexY[11][0] =  600.0;
      meshVertexX[11][1] = 3600.0;
      meshVertexY[11][1] =    0.0;
      meshVertexX[11][2] = 3600.0;
      meshVertexY[11][2] =  600.0;
      
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
      
      meshElementX = new double[12];
      
      meshElementX[0]  =  200.0;
      meshElementX[1]  =  400.0;
      meshElementX[2]  =  800.0;
      meshElementX[3]  = 1000.0;
      meshElementX[4]  = 1400.0;
      meshElementX[5]  = 1600.0;
      meshElementX[6]  = 2000.0;
      meshElementX[7]  = 2200.0;
      meshElementX[8]  = 2600.0;
      meshElementX[9]  = 2800.0;
      meshElementX[10] = 3200.0;
      meshElementX[11] = 3400.0;
      
      meshElementY = new double[12];
      
      meshElementY[0]  = 200.0;
      meshElementY[1]  = 400.0;
      meshElementY[2]  = 200.0;
      meshElementY[3]  = 400.0;
      meshElementY[4]  = 200.0;
      meshElementY[5]  = 400.0;
      meshElementY[6]  = 200.0;
      meshElementY[7]  = 400.0;
      meshElementY[8]  = 200.0;
      meshElementY[9]  = 400.0;
      meshElementY[10] = 200.0;
      meshElementY[11] = 400.0;
      
      meshElementZSurface = new double[12];
      
      meshElementZSurface[0]  =  2.0;
      meshElementZSurface[1]  =  4.0;
      meshElementZSurface[2]  =  8.0;
      meshElementZSurface[3]  = 10.0;
      meshElementZSurface[4]  = 14.0;
      meshElementZSurface[5]  = 16.0;
      meshElementZSurface[6]  = 20.0;
      meshElementZSurface[7]  = 22.0;
      meshElementZSurface[8]  = 26.0;
      meshElementZSurface[9]  = 28.0;
      meshElementZSurface[10] = 32.0;
      meshElementZSurface[11] = 34.0;
      
      meshElementLayerZBottom = new double[12];
      
      meshElementLayerZBottom[0]  =  1.0;
      meshElementLayerZBottom[1]  =  3.0;
      meshElementLayerZBottom[2]  =  7.0;
      meshElementLayerZBottom[3]  =  9.0;
      meshElementLayerZBottom[4]  = 13.0;
      meshElementLayerZBottom[5]  = 15.0;
      meshElementLayerZBottom[6]  = 19.0;
      meshElementLayerZBottom[7]  = 21.0;
      meshElementLayerZBottom[8]  = 25.0;
      meshElementLayerZBottom[9]  = 27.0;
      meshElementLayerZBottom[10] = 31.0;
      meshElementLayerZBottom[11] = 33.0;
      
      meshElementArea = new double[12];
      
      meshElementArea[0]  = 180000.0;
      meshElementArea[1]  = 180000.0;
      meshElementArea[2]  = 180000.0;
      meshElementArea[3]  = 180000.0;
      meshElementArea[4]  = 180000.0;
      meshElementArea[5]  = 180000.0;
      meshElementArea[6]  = 180000.0;
      meshElementArea[7]  = 180000.0;
      meshElementArea[8]  = 180000.0;
      meshElementArea[9]  = 180000.0;
      meshElementArea[10] = 180000.0;
      meshElementArea[11] = 180000.0;
      
      meshElementSlopeX = new double[12];
      
      meshElementSlopeX[0]  = 0.01;
      meshElementSlopeX[1]  = 0.01;
      meshElementSlopeX[2]  = 0.01;
      meshElementSlopeX[3]  = 0.01;
      meshElementSlopeX[4]  = 0.01;
      meshElementSlopeX[5]  = 0.01;
      meshElementSlopeX[6]  = 0.01;
      meshElementSlopeX[7]  = 0.01;
      meshElementSlopeX[8]  = 0.01;
      meshElementSlopeX[9]  = 0.01;
      meshElementSlopeX[10] = 0.01;
      meshElementSlopeX[11] = 0.01;
      
      meshElementSlopeY = new double[12];
      
      meshElementSlopeY[0]  = 0.0;
      meshElementSlopeY[1]  = 0.0;
      meshElementSlopeY[2]  = 0.0;
      meshElementSlopeY[3]  = 0.0;
      meshElementSlopeY[4]  = 0.0;
      meshElementSlopeY[5]  = 0.0;
      meshElementSlopeY[6]  = 0.0;
      meshElementSlopeY[7]  = 0.0;
      meshElementSlopeY[8]  = 0.0;
      meshElementSlopeY[9]  = 0.0;
      meshElementSlopeY[10] = 0.0;
      meshElementSlopeY[11] = 0.0;
      
      meshLatitude  = new double[12];
      meshLongitude = new double[12];
      
      ADHydro::getLatLong(meshElementX[0],  meshElementY[0],  meshLatitude[0],  meshLongitude[0]);
      ADHydro::getLatLong(meshElementX[1],  meshElementY[1],  meshLatitude[1],  meshLongitude[1]);
      ADHydro::getLatLong(meshElementX[2],  meshElementY[2],  meshLatitude[2],  meshLongitude[2]);
      ADHydro::getLatLong(meshElementX[3],  meshElementY[3],  meshLatitude[3],  meshLongitude[3]);
      ADHydro::getLatLong(meshElementX[4],  meshElementY[4],  meshLatitude[4],  meshLongitude[4]);
      ADHydro::getLatLong(meshElementX[5],  meshElementY[5],  meshLatitude[5],  meshLongitude[5]);
      ADHydro::getLatLong(meshElementX[6],  meshElementY[6],  meshLatitude[6],  meshLongitude[6]);
      ADHydro::getLatLong(meshElementX[7],  meshElementY[7],  meshLatitude[7],  meshLongitude[7]);
      ADHydro::getLatLong(meshElementX[8],  meshElementY[8],  meshLatitude[8],  meshLongitude[8]);
      ADHydro::getLatLong(meshElementX[9],  meshElementY[9],  meshLatitude[9],  meshLongitude[9]);
      ADHydro::getLatLong(meshElementX[10], meshElementY[10], meshLatitude[10], meshLongitude[10]);
      ADHydro::getLatLong(meshElementX[11], meshElementY[11], meshLatitude[11], meshLongitude[11]);
      
      meshManningsN = new double[12];
      
      meshManningsN[0]  = 0.038;
      meshManningsN[1]  = 0.038;
      meshManningsN[2]  = 0.038;
      meshManningsN[3]  = 0.038;
      meshManningsN[4]  = 0.038;
      meshManningsN[5]  = 0.038;
      meshManningsN[6]  = 0.038;
      meshManningsN[7]  = 0.038;
      meshManningsN[8]  = 0.038;
      meshManningsN[9]  = 0.038;
      meshManningsN[10] = 0.038;
      meshManningsN[11] = 0.038;
      
      meshConductivity = new double[12];
      
      meshConductivity[0]  = 4.66E-5;
      meshConductivity[1]  = 4.66E-5;
      meshConductivity[2]  = 4.66E-5;
      meshConductivity[3]  = 4.66E-5;
      meshConductivity[4]  = 4.66E-5;
      meshConductivity[5]  = 4.66E-5;
      meshConductivity[6]  = 4.66E-5;
      meshConductivity[7]  = 4.66E-5;
      meshConductivity[8]  = 4.66E-5;
      meshConductivity[9]  = 4.66E-5;
      meshConductivity[10] = 4.66E-5;
      meshConductivity[11] = 4.66E-5;
      
      meshPorosity = new double[12];
      
      meshPorosity[0]  = 0.339;
      meshPorosity[1]  = 0.339;
      meshPorosity[2]  = 0.339;
      meshPorosity[3]  = 0.339;
      meshPorosity[4]  = 0.339;
      meshPorosity[5]  = 0.339;
      meshPorosity[6]  = 0.339;
      meshPorosity[7]  = 0.339;
      meshPorosity[8]  = 0.339;
      meshPorosity[9]  = 0.339;
      meshPorosity[10] = 0.339;
      meshPorosity[11] = 0.339;
      
      meshSurfacewaterDepth = new double[12];
      
      meshSurfacewaterDepth[0]  = 0.0;
      meshSurfacewaterDepth[1]  = 0.0;
      meshSurfacewaterDepth[2]  = 0.0;
      meshSurfacewaterDepth[3]  = 0.0;
      meshSurfacewaterDepth[4]  = 0.0;
      meshSurfacewaterDepth[5]  = 0.0;
      meshSurfacewaterDepth[6]  = 0.0;
      meshSurfacewaterDepth[7]  = 0.0;
      meshSurfacewaterDepth[8]  = 0.0;
      meshSurfacewaterDepth[9]  = 0.0;
      meshSurfacewaterDepth[10] = 0.0;
      meshSurfacewaterDepth[11] = 0.0;
      
      meshSurfacewaterError = new double[12];
      
      meshSurfacewaterError[0]  = 0.0;
      meshSurfacewaterError[1]  = 0.0;
      meshSurfacewaterError[2]  = 0.0;
      meshSurfacewaterError[3]  = 0.0;
      meshSurfacewaterError[4]  = 0.0;
      meshSurfacewaterError[5]  = 0.0;
      meshSurfacewaterError[6]  = 0.0;
      meshSurfacewaterError[7]  = 0.0;
      meshSurfacewaterError[8]  = 0.0;
      meshSurfacewaterError[9]  = 0.0;
      meshSurfacewaterError[10] = 0.0;
      meshSurfacewaterError[11] = 0.0;
      
      meshGroundwaterHead = new double[12];
      
      meshGroundwaterHead[0]  = meshElementZSurface[0];
      meshGroundwaterHead[1]  = meshElementZSurface[1];
      meshGroundwaterHead[2]  = meshElementZSurface[2];
      meshGroundwaterHead[3]  = meshElementZSurface[3];
      meshGroundwaterHead[4]  = meshElementZSurface[4];
      meshGroundwaterHead[5]  = meshElementZSurface[5];
      meshGroundwaterHead[6]  = meshElementZSurface[6];
      meshGroundwaterHead[7]  = meshElementZSurface[7];
      meshGroundwaterHead[8]  = meshElementZSurface[8];
      meshGroundwaterHead[9]  = meshElementZSurface[9];
      meshGroundwaterHead[10] = meshElementZSurface[10];
      meshGroundwaterHead[11] = meshElementZSurface[11];
      
      meshGroundwaterRecharge = new double[12];
      
      meshGroundwaterRecharge[0]  = 0.0;
      meshGroundwaterRecharge[1]  = 0.0;
      meshGroundwaterRecharge[2]  = 0.0;
      meshGroundwaterRecharge[3]  = 0.0;
      meshGroundwaterRecharge[4]  = 0.0;
      meshGroundwaterRecharge[5]  = 0.0;
      meshGroundwaterRecharge[6]  = 0.0;
      meshGroundwaterRecharge[7]  = 0.0;
      meshGroundwaterRecharge[8]  = 0.0;
      meshGroundwaterRecharge[9]  = 0.0;
      meshGroundwaterRecharge[10] = 0.0;
      meshGroundwaterRecharge[11] = 0.0;
      
      meshGroundwaterError = new double[12];
      
      meshGroundwaterError[0]  = 0.0;
      meshGroundwaterError[1]  = 0.0;
      meshGroundwaterError[2]  = 0.0;
      meshGroundwaterError[3]  = 0.0;
      meshGroundwaterError[4]  = 0.0;
      meshGroundwaterError[5]  = 0.0;
      meshGroundwaterError[6]  = 0.0;
      meshGroundwaterError[7]  = 0.0;
      meshGroundwaterError[8]  = 0.0;
      meshGroundwaterError[9]  = 0.0;
      meshGroundwaterError[10] = 0.0;
      meshGroundwaterError[11] = 0.0;
      
      meshPrecipitationRate = new double[12];
      
      meshPrecipitationRate[0]  = 0.0;
      meshPrecipitationRate[1]  = 0.0;
      meshPrecipitationRate[2]  = 0.0;
      meshPrecipitationRate[3]  = 0.0;
      meshPrecipitationRate[4]  = 0.0;
      meshPrecipitationRate[5]  = 0.0;
      meshPrecipitationRate[6]  = 0.0;
      meshPrecipitationRate[7]  = 0.0;
      meshPrecipitationRate[8]  = 0.0;
      meshPrecipitationRate[9]  = 0.0;
      meshPrecipitationRate[10] = 0.0;
      meshPrecipitationRate[11] = 0.0;
      
      meshPrecipitationCumulativeShortTerm = new double[12];
      
      meshPrecipitationCumulativeShortTerm[0]  = 0.0;
      meshPrecipitationCumulativeShortTerm[1]  = 0.0;
      meshPrecipitationCumulativeShortTerm[2]  = 0.0;
      meshPrecipitationCumulativeShortTerm[3]  = 0.0;
      meshPrecipitationCumulativeShortTerm[4]  = 0.0;
      meshPrecipitationCumulativeShortTerm[5]  = 0.0;
      meshPrecipitationCumulativeShortTerm[6]  = 0.0;
      meshPrecipitationCumulativeShortTerm[7]  = 0.0;
      meshPrecipitationCumulativeShortTerm[8]  = 0.0;
      meshPrecipitationCumulativeShortTerm[9]  = 0.0;
      meshPrecipitationCumulativeShortTerm[10] = 0.0;
      meshPrecipitationCumulativeShortTerm[11] = 0.0;
      
      meshPrecipitationCumulativeLongTerm = new double[12];
      
      meshPrecipitationCumulativeLongTerm[0]  = 0.0;
      meshPrecipitationCumulativeLongTerm[1]  = 0.0;
      meshPrecipitationCumulativeLongTerm[2]  = 0.0;
      meshPrecipitationCumulativeLongTerm[3]  = 0.0;
      meshPrecipitationCumulativeLongTerm[4]  = 0.0;
      meshPrecipitationCumulativeLongTerm[5]  = 0.0;
      meshPrecipitationCumulativeLongTerm[6]  = 0.0;
      meshPrecipitationCumulativeLongTerm[7]  = 0.0;
      meshPrecipitationCumulativeLongTerm[8]  = 0.0;
      meshPrecipitationCumulativeLongTerm[9]  = 0.0;
      meshPrecipitationCumulativeLongTerm[10] = 0.0;
      meshPrecipitationCumulativeLongTerm[11] = 0.0;
      
      meshEvaporationRate = new double[12];
      
      meshEvaporationRate[0]  = 0.0;
      meshEvaporationRate[1]  = 0.0;
      meshEvaporationRate[2]  = 0.0;
      meshEvaporationRate[3]  = 0.0;
      meshEvaporationRate[4]  = 0.0;
      meshEvaporationRate[5]  = 0.0;
      meshEvaporationRate[6]  = 0.0;
      meshEvaporationRate[7]  = 0.0;
      meshEvaporationRate[8]  = 0.0;
      meshEvaporationRate[9]  = 0.0;
      meshEvaporationRate[10] = 0.0;
      meshEvaporationRate[11] = 0.0;
      
      meshEvaporationCumulativeShortTerm = new double[12];
      
      meshEvaporationCumulativeShortTerm[0]  = 0.0;
      meshEvaporationCumulativeShortTerm[1]  = 0.0;
      meshEvaporationCumulativeShortTerm[2]  = 0.0;
      meshEvaporationCumulativeShortTerm[3]  = 0.0;
      meshEvaporationCumulativeShortTerm[4]  = 0.0;
      meshEvaporationCumulativeShortTerm[5]  = 0.0;
      meshEvaporationCumulativeShortTerm[6]  = 0.0;
      meshEvaporationCumulativeShortTerm[7]  = 0.0;
      meshEvaporationCumulativeShortTerm[8]  = 0.0;
      meshEvaporationCumulativeShortTerm[9]  = 0.0;
      meshEvaporationCumulativeShortTerm[10] = 0.0;
      meshEvaporationCumulativeShortTerm[11] = 0.0;
      
      meshEvaporationCumulativeLongTerm = new double[12];
      
      meshEvaporationCumulativeLongTerm[0]  = 0.0;
      meshEvaporationCumulativeLongTerm[1]  = 0.0;
      meshEvaporationCumulativeLongTerm[2]  = 0.0;
      meshEvaporationCumulativeLongTerm[3]  = 0.0;
      meshEvaporationCumulativeLongTerm[4]  = 0.0;
      meshEvaporationCumulativeLongTerm[5]  = 0.0;
      meshEvaporationCumulativeLongTerm[6]  = 0.0;
      meshEvaporationCumulativeLongTerm[7]  = 0.0;
      meshEvaporationCumulativeLongTerm[8]  = 0.0;
      meshEvaporationCumulativeLongTerm[9]  = 0.0;
      meshEvaporationCumulativeLongTerm[10] = 0.0;
      meshEvaporationCumulativeLongTerm[11] = 0.0;
      
      meshTranspirationRate = new double[12];
      
      meshTranspirationRate[0]  = 0.0;
      meshTranspirationRate[1]  = 0.0;
      meshTranspirationRate[2]  = 0.0;
      meshTranspirationRate[3]  = 0.0;
      meshTranspirationRate[4]  = 0.0;
      meshTranspirationRate[5]  = 0.0;
      meshTranspirationRate[6]  = 0.0;
      meshTranspirationRate[7]  = 0.0;
      meshTranspirationRate[8]  = 0.0;
      meshTranspirationRate[9]  = 0.0;
      meshTranspirationRate[10] = 0.0;
      meshTranspirationRate[11] = 0.0;
      
      meshTranspirationCumulativeShortTerm = new double[12];
      
      meshTranspirationCumulativeShortTerm[0]  = 0.0;
      meshTranspirationCumulativeShortTerm[1]  = 0.0;
      meshTranspirationCumulativeShortTerm[2]  = 0.0;
      meshTranspirationCumulativeShortTerm[3]  = 0.0;
      meshTranspirationCumulativeShortTerm[4]  = 0.0;
      meshTranspirationCumulativeShortTerm[5]  = 0.0;
      meshTranspirationCumulativeShortTerm[6]  = 0.0;
      meshTranspirationCumulativeShortTerm[7]  = 0.0;
      meshTranspirationCumulativeShortTerm[8]  = 0.0;
      meshTranspirationCumulativeShortTerm[9]  = 0.0;
      meshTranspirationCumulativeShortTerm[10] = 0.0;
      meshTranspirationCumulativeShortTerm[11] = 0.0;
      
      meshTranspirationCumulativeLongTerm = new double[12];
      
      meshTranspirationCumulativeLongTerm[0]  = 0.0;
      meshTranspirationCumulativeLongTerm[1]  = 0.0;
      meshTranspirationCumulativeLongTerm[2]  = 0.0;
      meshTranspirationCumulativeLongTerm[3]  = 0.0;
      meshTranspirationCumulativeLongTerm[4]  = 0.0;
      meshTranspirationCumulativeLongTerm[5]  = 0.0;
      meshTranspirationCumulativeLongTerm[6]  = 0.0;
      meshTranspirationCumulativeLongTerm[7]  = 0.0;
      meshTranspirationCumulativeLongTerm[8]  = 0.0;
      meshTranspirationCumulativeLongTerm[9]  = 0.0;
      meshTranspirationCumulativeLongTerm[10] = 0.0;
      meshTranspirationCumulativeLongTerm[11] = 0.0;
      
      meshEvapoTranspirationState = new EvapoTranspirationStateStruct[12];
      
      meshEvapoTranspirationState[0]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[1]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[2]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[3]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[4]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[5]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[6]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[7]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[8]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[9]  = evapoTranspirationStateInit;
      meshEvapoTranspirationState[10] = evapoTranspirationStateInit;
      meshEvapoTranspirationState[11] = evapoTranspirationStateInit;
      
      meshInfiltrationMethod = new InfiltrationAndGroundwater::InfiltrationMethodEnum[12];
      
      meshInfiltrationMethod[0]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[1]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[2]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[3]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[4]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[5]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[6]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[7]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[8]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[9]  = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[10] = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      meshInfiltrationMethod[11] = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
      
      meshGroundwaterMethod = new InfiltrationAndGroundwater::GroundwaterMethodEnum[12];
      
      meshGroundwaterMethod[0]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[1]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[2]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[3]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[4]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[5]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[6]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[7]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[8]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[9]  = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[10] = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      meshGroundwaterMethod[11] = InfiltrationAndGroundwater::SHALLOW_AQUIFER;
      
      meshSurfacewaterMeshNeighbors = new IntArrayMMN[12];
      
      meshSurfacewaterMeshNeighbors[0][0]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[0][1]  = 1;
      meshSurfacewaterMeshNeighbors[0][2]  = OUTFLOW;
      meshSurfacewaterMeshNeighbors[1][0]  = 2;
      meshSurfacewaterMeshNeighbors[1][1]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[1][2]  = 0;
      meshSurfacewaterMeshNeighbors[2][0]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[2][1]  = 3;
      meshSurfacewaterMeshNeighbors[2][2]  = 1;
      meshSurfacewaterMeshNeighbors[3][0]  = 4;
      meshSurfacewaterMeshNeighbors[3][1]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[3][2]  = 2;
      meshSurfacewaterMeshNeighbors[4][0]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[4][1]  = 5;
      meshSurfacewaterMeshNeighbors[4][2]  = 3;
      meshSurfacewaterMeshNeighbors[5][0]  = 6;
      meshSurfacewaterMeshNeighbors[5][1]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[5][2]  = 4;
      meshSurfacewaterMeshNeighbors[6][0]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[6][1]  = 7;
      meshSurfacewaterMeshNeighbors[6][2]  = 5;
      meshSurfacewaterMeshNeighbors[7][0]  = 8;
      meshSurfacewaterMeshNeighbors[7][1]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[7][2]  = 6;
      meshSurfacewaterMeshNeighbors[8][0]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[8][1]  = 9;
      meshSurfacewaterMeshNeighbors[8][2]  = 7;
      meshSurfacewaterMeshNeighbors[9][0]  = 10;
      meshSurfacewaterMeshNeighbors[9][1]  = NOFLOW;
      meshSurfacewaterMeshNeighbors[9][2]  = 8;
      meshSurfacewaterMeshNeighbors[10][0] = NOFLOW;
      meshSurfacewaterMeshNeighbors[10][1] = 11;
      meshSurfacewaterMeshNeighbors[10][2] = 9;
      meshSurfacewaterMeshNeighbors[11][0] = NOFLOW;
      meshSurfacewaterMeshNeighbors[11][1] = NOFLOW;
      meshSurfacewaterMeshNeighbors[11][2] = 10;
      
      meshSurfacewaterMeshNeighborsRegion = new IntArrayMMN[12];
      
      meshSurfacewaterMeshNeighborsRegion[0][0]  = 0;
      meshSurfacewaterMeshNeighborsRegion[0][1]  = 0;
      meshSurfacewaterMeshNeighborsRegion[0][2]  = 0;
      meshSurfacewaterMeshNeighborsRegion[1][0]  = 0;
      meshSurfacewaterMeshNeighborsRegion[1][1]  = 0;
      meshSurfacewaterMeshNeighborsRegion[1][2]  = 0;
      meshSurfacewaterMeshNeighborsRegion[2][0]  = 0;
      meshSurfacewaterMeshNeighborsRegion[2][1]  = 0;
      meshSurfacewaterMeshNeighborsRegion[2][2]  = 0;
      meshSurfacewaterMeshNeighborsRegion[3][0]  = 1;
      meshSurfacewaterMeshNeighborsRegion[3][1]  = 0;
      meshSurfacewaterMeshNeighborsRegion[3][2]  = 0;
      meshSurfacewaterMeshNeighborsRegion[4][0]  = 0;
      meshSurfacewaterMeshNeighborsRegion[4][1]  = 1;
      meshSurfacewaterMeshNeighborsRegion[4][2]  = 0;
      meshSurfacewaterMeshNeighborsRegion[5][0]  = 1;
      meshSurfacewaterMeshNeighborsRegion[5][1]  = 0;
      meshSurfacewaterMeshNeighborsRegion[5][2]  = 1;
      meshSurfacewaterMeshNeighborsRegion[6][0]  = 0;
      meshSurfacewaterMeshNeighborsRegion[6][1]  = 1;
      meshSurfacewaterMeshNeighborsRegion[6][2]  = 1;
      meshSurfacewaterMeshNeighborsRegion[7][0]  = 2;
      meshSurfacewaterMeshNeighborsRegion[7][1]  = 0;
      meshSurfacewaterMeshNeighborsRegion[7][2]  = 1;
      meshSurfacewaterMeshNeighborsRegion[8][0]  = 0;
      meshSurfacewaterMeshNeighborsRegion[8][1]  = 2;
      meshSurfacewaterMeshNeighborsRegion[8][2]  = 1;
      meshSurfacewaterMeshNeighborsRegion[9][0]  = 2;
      meshSurfacewaterMeshNeighborsRegion[9][1]  = 0;
      meshSurfacewaterMeshNeighborsRegion[9][2]  = 2;
      meshSurfacewaterMeshNeighborsRegion[10][0] = 0;
      meshSurfacewaterMeshNeighborsRegion[10][1] = 2;
      meshSurfacewaterMeshNeighborsRegion[10][2] = 2;
      meshSurfacewaterMeshNeighborsRegion[11][0] = 0;
      meshSurfacewaterMeshNeighborsRegion[11][1] = 0;
      meshSurfacewaterMeshNeighborsRegion[11][2] = 2;
      
      meshSurfacewaterMeshNeighborsEdgeLength = new DoubleArrayMMN[12];
      
      meshSurfacewaterMeshNeighborsEdgeLength[0][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[0][1]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[0][2]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[1][0]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[1][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[1][2]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[2][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[2][1]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[2][2]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[3][0]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[3][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[3][2]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[4][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[4][1]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[4][2]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[5][0]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[5][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[5][2]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[6][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[6][1]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[6][2]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[7][0]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[7][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[7][2]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[8][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[8][1]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[8][2]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[9][0]  = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[9][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[9][2]  = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[10][0] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[10][1] = 600.0 * sqrt(2.0);
      meshSurfacewaterMeshNeighborsEdgeLength[10][2] = 600.0;
      meshSurfacewaterMeshNeighborsEdgeLength[11][0] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[11][1] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeLength[11][2] = 600.0 * sqrt(2.0);
      
      meshSurfacewaterMeshNeighborsEdgeNormalX = new DoubleArrayMMN[12];
      
      meshSurfacewaterMeshNeighborsEdgeNormalX[0][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[0][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[0][2]  = -1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[1][0]  = 1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[1][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[1][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[2][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[2][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[2][2]  = -1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[3][0]  = 1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[3][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[3][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[4][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[4][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[4][2]  = -1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[5][0]  = 1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[5][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[5][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[6][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[6][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[6][2]  = -1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[7][0]  = 1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[7][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[7][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[8][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[8][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[8][2]  = -1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[9][0]  = 1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[9][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[9][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[10][0] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[10][1] = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[10][2] = -1.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[11][0] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[11][1] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalX[11][2] = -sqrt(2.0) / 2.0;
      
      meshSurfacewaterMeshNeighborsEdgeNormalY = new DoubleArrayMMN[12];
      
      meshSurfacewaterMeshNeighborsEdgeNormalY[0][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[0][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[0][2]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[1][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[1][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[1][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[2][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[2][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[2][2]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[3][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[3][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[3][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[4][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[4][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[4][2]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[5][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[5][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[5][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[6][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[6][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[6][2]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[7][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[7][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[7][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[8][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[8][1]  = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[8][2]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[9][0]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[9][1]  = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[9][2]  = -sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[10][0] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[10][1] = sqrt(2.0) / 2.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[10][2] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[11][0] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[11][1] = 0.0;
      meshSurfacewaterMeshNeighborsEdgeNormalY[11][2] = -sqrt(2.0) / 2.0;
      
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm = new DoubleArrayMMN[12];
      
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[0][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[0][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[0][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[1][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[1][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[1][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[2][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[2][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[2][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[3][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[3][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[3][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[4][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[4][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[4][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[5][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[5][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[5][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[6][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[6][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[6][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[7][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[7][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[7][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[8][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[8][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[8][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[9][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[9][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[9][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[10][0] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[10][1] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[10][2] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[11][0] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[11][1] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[11][2] = 0.0;
      
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm = new DoubleArrayMMN[12];
      
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[0][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[0][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[0][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[1][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[1][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[1][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[2][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[2][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[2][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[3][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[3][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[3][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[4][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[4][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[4][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[5][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[5][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[5][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[6][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[6][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[6][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[7][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[7][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[7][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[8][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[8][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[8][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[9][0]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[9][1]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[9][2]  = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[10][0] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[10][1] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[10][2] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[11][0] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[11][1] = 0.0;
      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[11][2] = 0.0;
      
      meshSurfacewaterChannelNeighbors = new IntArrayMCN[12];
      
      meshSurfacewaterChannelNeighbors[0][0]  = 0;
      meshSurfacewaterChannelNeighbors[0][1]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[1][0]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[1][1]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[2][0]  = 0;
      meshSurfacewaterChannelNeighbors[2][1]  = 1;
      meshSurfacewaterChannelNeighbors[3][0]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[3][1]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[4][0]  = 1;
      meshSurfacewaterChannelNeighbors[4][1]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[5][0]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[5][1]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[6][0]  = 2;
      meshSurfacewaterChannelNeighbors[6][1]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[7][0]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[7][1]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[8][0]  = 2;
      meshSurfacewaterChannelNeighbors[8][1]  = 3;
      meshSurfacewaterChannelNeighbors[9][0]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[9][1]  = NOFLOW;
      meshSurfacewaterChannelNeighbors[10][0] = 3;
      meshSurfacewaterChannelNeighbors[10][1] = NOFLOW;
      meshSurfacewaterChannelNeighbors[11][0] = NOFLOW;
      meshSurfacewaterChannelNeighbors[11][1] = NOFLOW;
      
      meshSurfacewaterChannelNeighborsRegion = new IntArrayMCN[12];
      
      meshSurfacewaterChannelNeighborsRegion[0][0]  = 0;
      meshSurfacewaterChannelNeighborsRegion[0][1]  = 0;
      meshSurfacewaterChannelNeighborsRegion[1][0]  = 0;
      meshSurfacewaterChannelNeighborsRegion[1][1]  = 0;
      meshSurfacewaterChannelNeighborsRegion[2][0]  = 0;
      meshSurfacewaterChannelNeighborsRegion[2][1]  = 1;
      meshSurfacewaterChannelNeighborsRegion[3][0]  = 0;
      meshSurfacewaterChannelNeighborsRegion[3][1]  = 0;
      meshSurfacewaterChannelNeighborsRegion[4][0]  = 1;
      meshSurfacewaterChannelNeighborsRegion[4][1]  = 0;
      meshSurfacewaterChannelNeighborsRegion[5][0]  = 0;
      meshSurfacewaterChannelNeighborsRegion[5][1]  = 0;
      meshSurfacewaterChannelNeighborsRegion[6][0]  = 1;
      meshSurfacewaterChannelNeighborsRegion[6][1]  = 0;
      meshSurfacewaterChannelNeighborsRegion[7][0]  = 0;
      meshSurfacewaterChannelNeighborsRegion[7][1]  = 0;
      meshSurfacewaterChannelNeighborsRegion[8][0]  = 1;
      meshSurfacewaterChannelNeighborsRegion[8][1]  = 2;
      meshSurfacewaterChannelNeighborsRegion[9][0]  = 0;
      meshSurfacewaterChannelNeighborsRegion[9][1]  = 0;
      meshSurfacewaterChannelNeighborsRegion[10][0] = 2;
      meshSurfacewaterChannelNeighborsRegion[10][1] = 0;
      meshSurfacewaterChannelNeighborsRegion[11][0] = 0;
      meshSurfacewaterChannelNeighborsRegion[11][1] = 0;
      
      meshSurfacewaterChannelNeighborsEdgeLength = new DoubleArrayMCN[12];
      
      meshSurfacewaterChannelNeighborsEdgeLength[0][0]  = 600.0;
      meshSurfacewaterChannelNeighborsEdgeLength[0][1]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[1][0]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[1][1]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[2][0]  = 300.0;
      meshSurfacewaterChannelNeighborsEdgeLength[2][1]  = 300.0;
      meshSurfacewaterChannelNeighborsEdgeLength[3][0]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[3][1]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[4][0]  = 600.0;
      meshSurfacewaterChannelNeighborsEdgeLength[4][1]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[5][0]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[5][1]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[6][0]  = 600.0;
      meshSurfacewaterChannelNeighborsEdgeLength[6][1]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[7][0]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[7][1]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[8][0]  = 300.0;
      meshSurfacewaterChannelNeighborsEdgeLength[8][1]  = 300.0;
      meshSurfacewaterChannelNeighborsEdgeLength[9][0]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[9][1]  = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[10][0] = 600.0;
      meshSurfacewaterChannelNeighborsEdgeLength[10][1] = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[11][0] = 0.0;
      meshSurfacewaterChannelNeighborsEdgeLength[11][1] = 0.0;
      
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm = new DoubleArrayMCN[12];
      
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[0][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[0][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[1][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[1][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[2][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[2][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[3][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[3][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[4][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[4][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[5][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[5][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[6][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[6][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[7][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[7][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[8][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[8][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[9][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[9][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[10][0] = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[10][1] = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[11][0] = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[11][1] = 0.0;
      
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm = new DoubleArrayMCN[12];
      
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[0][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[0][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[1][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[1][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[2][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[2][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[3][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[3][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[4][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[4][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[5][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[5][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[6][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[6][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[7][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[7][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[8][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[8][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[9][0]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[9][1]  = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[10][0] = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[10][1] = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[11][0] = 0.0;
      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[11][1] = 0.0;
      
      meshGroundwaterMeshNeighbors = new IntArrayMMN[12];
      
      meshGroundwaterMeshNeighbors[0][0]  = NOFLOW;
      meshGroundwaterMeshNeighbors[0][1]  = 1;
      meshGroundwaterMeshNeighbors[0][2]  = OUTFLOW;
      meshGroundwaterMeshNeighbors[1][0]  = 2;
      meshGroundwaterMeshNeighbors[1][1]  = NOFLOW;
      meshGroundwaterMeshNeighbors[1][2]  = 0;
      meshGroundwaterMeshNeighbors[2][0]  = NOFLOW;
      meshGroundwaterMeshNeighbors[2][1]  = 3;
      meshGroundwaterMeshNeighbors[2][2]  = 1;
      meshGroundwaterMeshNeighbors[3][0]  = 4;
      meshGroundwaterMeshNeighbors[3][1]  = NOFLOW;
      meshGroundwaterMeshNeighbors[3][2]  = 2;
      meshGroundwaterMeshNeighbors[4][0]  = NOFLOW;
      meshGroundwaterMeshNeighbors[4][1]  = 5;
      meshGroundwaterMeshNeighbors[4][2]  = 3;
      meshGroundwaterMeshNeighbors[5][0]  = 6;
      meshGroundwaterMeshNeighbors[5][1]  = NOFLOW;
      meshGroundwaterMeshNeighbors[5][2]  = 4;
      meshGroundwaterMeshNeighbors[6][0]  = NOFLOW;
      meshGroundwaterMeshNeighbors[6][1]  = 7;
      meshGroundwaterMeshNeighbors[6][2]  = 5;
      meshGroundwaterMeshNeighbors[7][0]  = 8;
      meshGroundwaterMeshNeighbors[7][1]  = NOFLOW;
      meshGroundwaterMeshNeighbors[7][2]  = 6;
      meshGroundwaterMeshNeighbors[8][0]  = NOFLOW;
      meshGroundwaterMeshNeighbors[8][1]  = 9;
      meshGroundwaterMeshNeighbors[8][2]  = 7;
      meshGroundwaterMeshNeighbors[9][0]  = 10;
      meshGroundwaterMeshNeighbors[9][1]  = NOFLOW;
      meshGroundwaterMeshNeighbors[9][2]  = 8;
      meshGroundwaterMeshNeighbors[10][0] = NOFLOW;
      meshGroundwaterMeshNeighbors[10][1] = 11;
      meshGroundwaterMeshNeighbors[10][2] = 9;
      meshGroundwaterMeshNeighbors[11][0] = NOFLOW;
      meshGroundwaterMeshNeighbors[11][1] = NOFLOW;
      meshGroundwaterMeshNeighbors[11][2] = 10;
      
      meshGroundwaterMeshNeighborsRegion = new IntArrayMMN[12];
      
      meshGroundwaterMeshNeighborsRegion[0][0]  = 0;
      meshGroundwaterMeshNeighborsRegion[0][1]  = 0;
      meshGroundwaterMeshNeighborsRegion[0][2]  = 0;
      meshGroundwaterMeshNeighborsRegion[1][0]  = 0;
      meshGroundwaterMeshNeighborsRegion[1][1]  = 0;
      meshGroundwaterMeshNeighborsRegion[1][2]  = 0;
      meshGroundwaterMeshNeighborsRegion[2][0]  = 0;
      meshGroundwaterMeshNeighborsRegion[2][1]  = 0;
      meshGroundwaterMeshNeighborsRegion[2][2]  = 0;
      meshGroundwaterMeshNeighborsRegion[3][0]  = 1;
      meshGroundwaterMeshNeighborsRegion[3][1]  = 0;
      meshGroundwaterMeshNeighborsRegion[3][2]  = 0;
      meshGroundwaterMeshNeighborsRegion[4][0]  = 0;
      meshGroundwaterMeshNeighborsRegion[4][1]  = 1;
      meshGroundwaterMeshNeighborsRegion[4][2]  = 0;
      meshGroundwaterMeshNeighborsRegion[5][0]  = 1;
      meshGroundwaterMeshNeighborsRegion[5][1]  = 0;
      meshGroundwaterMeshNeighborsRegion[5][2]  = 1;
      meshGroundwaterMeshNeighborsRegion[6][0]  = 0;
      meshGroundwaterMeshNeighborsRegion[6][1]  = 1;
      meshGroundwaterMeshNeighborsRegion[6][2]  = 1;
      meshGroundwaterMeshNeighborsRegion[7][0]  = 2;
      meshGroundwaterMeshNeighborsRegion[7][1]  = 0;
      meshGroundwaterMeshNeighborsRegion[7][2]  = 1;
      meshGroundwaterMeshNeighborsRegion[8][0]  = 0;
      meshGroundwaterMeshNeighborsRegion[8][1]  = 2;
      meshGroundwaterMeshNeighborsRegion[8][2]  = 1;
      meshGroundwaterMeshNeighborsRegion[9][0]  = 2;
      meshGroundwaterMeshNeighborsRegion[9][1]  = 0;
      meshGroundwaterMeshNeighborsRegion[9][2]  = 2;
      meshGroundwaterMeshNeighborsRegion[10][0] = 0;
      meshGroundwaterMeshNeighborsRegion[10][1] = 2;
      meshGroundwaterMeshNeighborsRegion[10][2] = 2;
      meshGroundwaterMeshNeighborsRegion[11][0] = 0;
      meshGroundwaterMeshNeighborsRegion[11][1] = 0;
      meshGroundwaterMeshNeighborsRegion[11][2] = 2;
      
      meshGroundwaterMeshNeighborsEdgeLength = new DoubleArrayMMN[12];
      
      meshGroundwaterMeshNeighborsEdgeLength[0][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[0][1]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[0][2]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[1][0]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[1][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[1][2]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[2][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[2][1]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[2][2]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[3][0]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[3][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[3][2]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[4][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[4][1]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[4][2]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[5][0]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[5][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[5][2]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[6][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[6][1]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[6][2]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[7][0]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[7][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[7][2]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[8][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[8][1]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[8][2]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[9][0]  = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[9][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[9][2]  = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[10][0] = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[10][1] = 600.0 * sqrt(2.0);
      meshGroundwaterMeshNeighborsEdgeLength[10][2] = 600.0;
      meshGroundwaterMeshNeighborsEdgeLength[11][0] = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[11][1] = 0.0;
      meshGroundwaterMeshNeighborsEdgeLength[11][2] = 600.0 * sqrt(2.0);
      
      meshGroundwaterMeshNeighborsEdgeNormalX = new DoubleArrayMMN[12];
      
      meshGroundwaterMeshNeighborsEdgeNormalX[0][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[0][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[0][2]  = -1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[1][0]  = 1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[1][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[1][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[2][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[2][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[2][2]  = -1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[3][0]  = 1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[3][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[3][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[4][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[4][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[4][2]  = -1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[5][0]  = 1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[5][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[5][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[6][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[6][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[6][2]  = -1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[7][0]  = 1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[7][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[7][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[8][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[8][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[8][2]  = -1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[9][0]  = 1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[9][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[9][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[10][0] = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[10][1] = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[10][2] = -1.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[11][0] = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[11][1] = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalX[11][2] = -sqrt(2.0) / 2.0;
      
      meshGroundwaterMeshNeighborsEdgeNormalY = new DoubleArrayMMN[12];
      
      meshGroundwaterMeshNeighborsEdgeNormalY[0][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[0][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[0][2]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[1][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[1][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[1][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[2][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[2][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[2][2]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[3][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[3][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[3][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[4][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[4][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[4][2]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[5][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[5][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[5][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[6][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[6][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[6][2]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[7][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[7][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[7][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[8][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[8][1]  = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[8][2]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[9][0]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[9][1]  = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[9][2]  = -sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[10][0] = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[10][1] = sqrt(2.0) / 2.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[10][2] = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[11][0] = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[11][1] = 0.0;
      meshGroundwaterMeshNeighborsEdgeNormalY[11][2] = -sqrt(2.0) / 2.0;
      
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm = new DoubleArrayMMN[12];
      
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[0][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[0][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[0][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[1][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[1][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[1][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[2][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[2][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[2][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[3][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[3][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[3][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[4][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[4][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[4][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[5][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[5][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[5][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[6][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[6][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[6][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[7][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[7][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[7][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[8][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[8][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[8][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[9][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[9][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[9][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[10][0] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[10][1] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[10][2] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[11][0] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[11][1] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[11][2] = 0.0;
      
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm = new DoubleArrayMMN[12];
      
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[0][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[0][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[0][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[1][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[1][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[1][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[2][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[2][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[2][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[3][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[3][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[3][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[4][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[4][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[4][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[5][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[5][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[5][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[6][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[6][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[6][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[7][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[7][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[7][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[8][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[8][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[8][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[9][0]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[9][1]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[9][2]  = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[10][0] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[10][1] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[10][2] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[11][0] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[11][1] = 0.0;
      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[11][2] = 0.0;
      
      meshGroundwaterChannelNeighbors = new IntArrayMCN[12];
      
      meshGroundwaterChannelNeighbors[0][0]  = 0;
      meshGroundwaterChannelNeighbors[0][1]  = NOFLOW;
      meshGroundwaterChannelNeighbors[1][0]  = NOFLOW;
      meshGroundwaterChannelNeighbors[1][1]  = NOFLOW;
      meshGroundwaterChannelNeighbors[2][0]  = 0;
      meshGroundwaterChannelNeighbors[2][1]  = 1;
      meshGroundwaterChannelNeighbors[3][0]  = NOFLOW;
      meshGroundwaterChannelNeighbors[3][1]  = NOFLOW;
      meshGroundwaterChannelNeighbors[4][0]  = 1;
      meshGroundwaterChannelNeighbors[4][1]  = NOFLOW;
      meshGroundwaterChannelNeighbors[5][0]  = NOFLOW;
      meshGroundwaterChannelNeighbors[5][1]  = NOFLOW;
      meshGroundwaterChannelNeighbors[6][0]  = 2;
      meshGroundwaterChannelNeighbors[6][1]  = NOFLOW;
      meshGroundwaterChannelNeighbors[7][0]  = NOFLOW;
      meshGroundwaterChannelNeighbors[7][1]  = NOFLOW;
      meshGroundwaterChannelNeighbors[8][0]  = 2;
      meshGroundwaterChannelNeighbors[8][1]  = 3;
      meshGroundwaterChannelNeighbors[9][0]  = NOFLOW;
      meshGroundwaterChannelNeighbors[9][1]  = NOFLOW;
      meshGroundwaterChannelNeighbors[10][0] = 3;
      meshGroundwaterChannelNeighbors[10][1] = NOFLOW;
      meshGroundwaterChannelNeighbors[11][0] = NOFLOW;
      meshGroundwaterChannelNeighbors[11][1] = NOFLOW;
      
      meshGroundwaterChannelNeighborsRegion = new IntArrayMCN[12];
      
      meshGroundwaterChannelNeighborsRegion[0][0]  = 0;
      meshGroundwaterChannelNeighborsRegion[0][1]  = 0;
      meshGroundwaterChannelNeighborsRegion[1][0]  = 0;
      meshGroundwaterChannelNeighborsRegion[1][1]  = 0;
      meshGroundwaterChannelNeighborsRegion[2][0]  = 0;
      meshGroundwaterChannelNeighborsRegion[2][1]  = 1;
      meshGroundwaterChannelNeighborsRegion[3][0]  = 0;
      meshGroundwaterChannelNeighborsRegion[3][1]  = 0;
      meshGroundwaterChannelNeighborsRegion[4][0]  = 1;
      meshGroundwaterChannelNeighborsRegion[4][1]  = 0;
      meshGroundwaterChannelNeighborsRegion[5][0]  = 0;
      meshGroundwaterChannelNeighborsRegion[5][1]  = 0;
      meshGroundwaterChannelNeighborsRegion[6][0]  = 1;
      meshGroundwaterChannelNeighborsRegion[6][1]  = 0;
      meshGroundwaterChannelNeighborsRegion[7][0]  = 0;
      meshGroundwaterChannelNeighborsRegion[7][1]  = 0;
      meshGroundwaterChannelNeighborsRegion[8][0]  = 1;
      meshGroundwaterChannelNeighborsRegion[8][1]  = 2;
      meshGroundwaterChannelNeighborsRegion[9][0]  = 0;
      meshGroundwaterChannelNeighborsRegion[9][1]  = 0;
      meshGroundwaterChannelNeighborsRegion[10][0] = 2;
      meshGroundwaterChannelNeighborsRegion[10][1] = 0;
      meshGroundwaterChannelNeighborsRegion[11][0] = 0;
      meshGroundwaterChannelNeighborsRegion[11][1] = 0;
      
      meshGroundwaterChannelNeighborsEdgeLength = new DoubleArrayMCN[12];
      
      meshGroundwaterChannelNeighborsEdgeLength[0][0]  = 600.0;
      meshGroundwaterChannelNeighborsEdgeLength[0][1]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[1][0]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[1][1]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[2][0]  = 300.0;
      meshGroundwaterChannelNeighborsEdgeLength[2][1]  = 300.0;
      meshGroundwaterChannelNeighborsEdgeLength[3][0]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[3][1]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[4][0]  = 600.0;
      meshGroundwaterChannelNeighborsEdgeLength[4][1]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[5][0]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[5][1]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[6][0]  = 600.0;
      meshGroundwaterChannelNeighborsEdgeLength[6][1]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[7][0]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[7][1]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[8][0]  = 300.0;
      meshGroundwaterChannelNeighborsEdgeLength[8][1]  = 300.0;
      meshGroundwaterChannelNeighborsEdgeLength[9][0]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[9][1]  = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[10][0] = 600.0;
      meshGroundwaterChannelNeighborsEdgeLength[10][1] = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[11][0] = 0.0;
      meshGroundwaterChannelNeighborsEdgeLength[11][1] = 0.0;
      
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm = new DoubleArrayMCN[12];
      
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[0][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[0][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[1][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[1][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[2][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[2][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[3][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[3][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[4][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[4][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[5][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[5][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[6][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[6][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[7][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[7][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[8][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[8][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[9][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[9][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[10][0] = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[10][1] = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[11][0] = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[11][1] = 0.0;
      
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm = new DoubleArrayMCN[12];
      
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[0][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[0][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[1][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[1][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[2][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[2][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[3][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[3][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[4][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[4][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[5][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[5][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[6][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[6][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[7][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[7][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[8][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[8][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[9][0]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[9][1]  = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[10][0] = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[10][1] = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[11][0] = 0.0;
      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[11][1] = 0.0;
      
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
      
      channelElementX = new double[4];
      
      channelElementX[0] = 450.0;
      channelElementX[1] = 1350.0;
      channelElementX[2] = 2250.0;
      channelElementX[3] = 3150.0;
      
      channelElementY = new double[4];
      
      channelElementY[0] = 0.0;
      channelElementY[1] = 0.0;
      channelElementY[2] = 0.0;
      channelElementY[3] = 0.0;
      
      channelElementZBank = new double[4];
      
      channelElementZBank[0] = 4.5;
      channelElementZBank[1] = 13.5;
      channelElementZBank[2] = 22.5;
      channelElementZBank[3] = 31.5;
      
      channelElementZBed = new double[4];
      
      channelElementZBed[0] = -5.5;
      channelElementZBed[1] = 3.5;
      channelElementZBed[2] = 12.5;
      channelElementZBed[3] = 21.5;
      
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
      
      channelSurfacewaterDepth = new double[4];
      
      channelSurfacewaterDepth[0] = 0.0;
      channelSurfacewaterDepth[1] = 0.0;
      channelSurfacewaterDepth[2] = 0.0;
      channelSurfacewaterDepth[3] = 0.0;
      
      channelSurfacewaterError = new double[4];
      
      channelSurfacewaterError[0] = 0.0;
      channelSurfacewaterError[1] = 0.0;
      channelSurfacewaterError[2] = 0.0;
      channelSurfacewaterError[3] = 0.0;
      
      channelSurfacewaterMeshNeighbors = new IntArrayCMN[4];
      
      channelSurfacewaterMeshNeighbors[0][0] = 0;
      channelSurfacewaterMeshNeighbors[0][1] = 2;
      channelSurfacewaterMeshNeighbors[1][0] = 2;
      channelSurfacewaterMeshNeighbors[1][1] = 4;
      channelSurfacewaterMeshNeighbors[2][0] = 6;
      channelSurfacewaterMeshNeighbors[2][1] = 8;
      channelSurfacewaterMeshNeighbors[3][0] = 8;
      channelSurfacewaterMeshNeighbors[3][1] = 10;
      
      channelSurfacewaterMeshNeighborsRegion = new IntArrayCMN[4];
      
      channelSurfacewaterMeshNeighborsRegion[0][0] = 0;
      channelSurfacewaterMeshNeighborsRegion[0][1] = 0;
      channelSurfacewaterMeshNeighborsRegion[1][0] = 0;
      channelSurfacewaterMeshNeighborsRegion[1][1] = 1;
      channelSurfacewaterMeshNeighborsRegion[2][0] = 1;
      channelSurfacewaterMeshNeighborsRegion[2][1] = 2;
      channelSurfacewaterMeshNeighborsRegion[3][0] = 2;
      channelSurfacewaterMeshNeighborsRegion[3][1] = 2;
      
      channelSurfacewaterMeshNeighborsEdgeLength = new DoubleArrayCMN[4];
      
      channelSurfacewaterMeshNeighborsEdgeLength[0][0] = 600.0;
      channelSurfacewaterMeshNeighborsEdgeLength[0][1] = 300.0;
      channelSurfacewaterMeshNeighborsEdgeLength[1][0] = 300.0;
      channelSurfacewaterMeshNeighborsEdgeLength[1][1] = 600.0;
      channelSurfacewaterMeshNeighborsEdgeLength[2][0] = 600.0;
      channelSurfacewaterMeshNeighborsEdgeLength[2][1] = 300.0;
      channelSurfacewaterMeshNeighborsEdgeLength[3][0] = 300.0;
      channelSurfacewaterMeshNeighborsEdgeLength[3][1] = 600.0;
      
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm = new DoubleArrayCMN[4];
      
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[0][0] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[0][1] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[1][0] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[1][1] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[2][0] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[2][1] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[3][0] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[3][1] = 0.0;
      
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm = new DoubleArrayCMN[4];
      
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[0][0] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[0][1] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[1][0] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[1][1] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[2][0] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[2][1] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[3][0] = 0.0;
      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[3][1] = 0.0;
      
      channelSurfacewaterChannelNeighbors = new IntArrayCCN[4];
      
      channelSurfacewaterChannelNeighbors[0][0] = 1;
      channelSurfacewaterChannelNeighbors[0][1] = OUTFLOW;
      channelSurfacewaterChannelNeighbors[1][0] = 0;
      channelSurfacewaterChannelNeighbors[1][1] = 2;
      channelSurfacewaterChannelNeighbors[2][0] = 1;
      channelSurfacewaterChannelNeighbors[2][1] = 3;
      channelSurfacewaterChannelNeighbors[3][0] = 2;
      channelSurfacewaterChannelNeighbors[3][1] = NOFLOW;
      
      channelSurfacewaterChannelNeighborsRegion = new IntArrayCCN[4];
      
      channelSurfacewaterChannelNeighborsRegion[0][0] = 1;
      channelSurfacewaterChannelNeighborsRegion[0][1] = 0;
      channelSurfacewaterChannelNeighborsRegion[1][0] = 0;
      channelSurfacewaterChannelNeighborsRegion[1][1] = 1;
      channelSurfacewaterChannelNeighborsRegion[2][0] = 1;
      channelSurfacewaterChannelNeighborsRegion[2][1] = 2;
      channelSurfacewaterChannelNeighborsRegion[3][0] = 1;
      channelSurfacewaterChannelNeighborsRegion[3][1] = 0;
      
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm = new DoubleArrayCCN[4];
      
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[0][0] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[0][1] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[1][0] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[1][1] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[2][0] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[2][1] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[3][0] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[3][1] = 0.0;
      
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm = new DoubleArrayCCN[4];
      
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[0][0] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[0][1] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[1][0] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[1][1] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[2][0] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[2][1] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[3][0] = 0.0;
      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[3][1] = 0.0;
      
      channelGroundwaterMeshNeighbors = new IntArrayCMN[4];
      
      channelGroundwaterMeshNeighbors[0][0] = 0;
      channelGroundwaterMeshNeighbors[0][1] = 2;
      channelGroundwaterMeshNeighbors[1][0] = 2;
      channelGroundwaterMeshNeighbors[1][1] = 4;
      channelGroundwaterMeshNeighbors[2][0] = 6;
      channelGroundwaterMeshNeighbors[2][1] = 8;
      channelGroundwaterMeshNeighbors[3][0] = 8;
      channelGroundwaterMeshNeighbors[3][1] = 10;
      
      channelGroundwaterMeshNeighborsRegion = new IntArrayCMN[4];
      
      channelGroundwaterMeshNeighborsRegion[0][0] = 0;
      channelGroundwaterMeshNeighborsRegion[0][1] = 0;
      channelGroundwaterMeshNeighborsRegion[1][0] = 0;
      channelGroundwaterMeshNeighborsRegion[1][1] = 1;
      channelGroundwaterMeshNeighborsRegion[2][0] = 1;
      channelGroundwaterMeshNeighborsRegion[2][1] = 2;
      channelGroundwaterMeshNeighborsRegion[3][0] = 2;
      channelGroundwaterMeshNeighborsRegion[3][1] = 2;
      
      channelGroundwaterMeshNeighborsEdgeLength = new DoubleArrayCMN[4];
      
      channelGroundwaterMeshNeighborsEdgeLength[0][0] = 600.0;
      channelGroundwaterMeshNeighborsEdgeLength[0][1] = 300.0;
      channelGroundwaterMeshNeighborsEdgeLength[1][0] = 300.0;
      channelGroundwaterMeshNeighborsEdgeLength[1][1] = 600.0;
      channelGroundwaterMeshNeighborsEdgeLength[2][0] = 600.0;
      channelGroundwaterMeshNeighborsEdgeLength[2][1] = 300.0;
      channelGroundwaterMeshNeighborsEdgeLength[3][0] = 300.0;
      channelGroundwaterMeshNeighborsEdgeLength[3][1] = 600.0;
      
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm = new DoubleArrayCMN[4];
      
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[0][0] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[0][1] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[1][0] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[1][1] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[2][0] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[2][1] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[3][0] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[3][1] = 0.0;
      
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm = new DoubleArrayCMN[4];
      
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[0][0] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[0][1] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[1][0] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[1][1] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[2][0] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[2][1] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[3][0] = 0.0;
      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[3][1] = 0.0;
    }
    else
    {
      globalNumberOfRegions         = 3;
      localRegionStart              = 0;
      localNumberOfRegions          = 0;
      globalNumberOfMeshNodes       = 0;
      localMeshNodeStart            = 0;
      localNumberOfMeshNodes        = 0;
      globalNumberOfMeshElements    = 12;
      localMeshElementStart         = 0;
      localNumberOfMeshElements     = 0;
      globalNumberOfChannelNodes    = 0;
      localChannelNodeStart         = 0;
      localNumberOfChannelNodes     = 0;
      globalNumberOfChannelElements = 4;
      localChannelElementStart      = 0;
      localNumberOfChannelElements  = 0;
    }
}

void FileManager::handleSendInitializationMessages(CProxy_Region regionProxy)
{
  bool                            error = false;                // Error flag.
  int                             ii, jj;                       // Loop counters.
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
  
  if (!(NULL != meshSurfacewaterMeshNeighbors))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighbors must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshSurfacewaterMeshNeighborsRegion))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsRegion must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshSurfacewaterMeshNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsEdgeLength must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshSurfacewaterMeshNeighborsEdgeNormalX))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsEdgeNormalX must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshSurfacewaterMeshNeighborsEdgeNormalY))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsEdgeNormalY must not be NULL.\n");
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
  
  if (!(NULL != meshSurfacewaterChannelNeighbors))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterChannelNeighbors must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshSurfacewaterChannelNeighborsRegion))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterChannelNeighborsRegion must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshSurfacewaterChannelNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterChannelNeighborsEdgeLength must not be NULL.\n");
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
  
  if (!(NULL != meshGroundwaterMeshNeighbors))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighbors must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshGroundwaterMeshNeighborsRegion))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsRegion must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshGroundwaterMeshNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsEdgeLength must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshGroundwaterMeshNeighborsEdgeNormalX))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsEdgeNormalX must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshGroundwaterMeshNeighborsEdgeNormalY))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsEdgeNormalY must not be NULL.\n");
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
  
  if (!(NULL != meshGroundwaterChannelNeighbors))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterChannelNeighbors must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshGroundwaterChannelNeighborsRegion))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterChannelNeighborsRegion must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != meshGroundwaterChannelNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterChannelNeighborsEdgeLength must not be NULL.\n");
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
  
  if (!(NULL != channelSurfacewaterMeshNeighbors))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterMeshNeighbors must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != channelSurfacewaterMeshNeighborsRegion))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterMeshNeighborsRegion must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != channelSurfacewaterMeshNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterMeshNeighborsEdgeLength must not be NULL.\n");
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
  
  if (!(NULL != channelSurfacewaterChannelNeighbors))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterChannelNeighbors must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != channelSurfacewaterChannelNeighborsRegion))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterChannelNeighborsRegion must not be NULL.\n");
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
  
  if (!(NULL != channelGroundwaterMeshNeighbors))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: channelGroundwaterMeshNeighbors must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != channelGroundwaterMeshNeighborsRegion))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: channelGroundwaterMeshNeighborsRegion must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != channelGroundwaterMeshNeighborsEdgeLength))
    {
      CkError("ERROR in FileManager::handleSendInitializationMessages: channelGroundwaterMeshNeighborsEdgeLength must not be NULL.\n");
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
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  if (!error)
    {
      for (ii = 0; ii < localNumberOfMeshElements; ++ii)
        {
          surfacewaterMeshNeighbors.clear();
          surfacewaterChannelNeighbors.clear();
          groundwaterMeshNeighbors.clear();
          groundwaterChannelNeighbors.clear();

          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; ++jj)
            {
              if (NOFLOW != meshSurfacewaterMeshNeighbors[ii][jj])
                {
                  surfacewaterMeshNeighbors.push_back(simpleNeighborInfo(meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], meshSurfacewaterMeshNeighborsRegion[ii][jj],
                      meshSurfacewaterMeshNeighbors[ii][jj], meshSurfacewaterMeshNeighborsEdgeLength[ii][jj], meshSurfacewaterMeshNeighborsEdgeNormalX[ii][jj],
                      meshSurfacewaterMeshNeighborsEdgeNormalY[ii][jj]));
                }
            }

          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; ++jj)
            {
              if (NOFLOW != meshSurfacewaterChannelNeighbors[ii][jj])
                {
                  surfacewaterChannelNeighbors.push_back(simpleNeighborInfo(meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ii][jj], meshSurfacewaterChannelNeighborsRegion[ii][jj],
                      meshSurfacewaterChannelNeighbors[ii][jj], meshSurfacewaterChannelNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }
            }

          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; ++jj)
            {
              if (NOFLOW != meshGroundwaterMeshNeighbors[ii][jj])
                {
                  groundwaterMeshNeighbors.push_back(simpleNeighborInfo(meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], meshGroundwaterMeshNeighborsRegion[ii][jj],
                      meshGroundwaterMeshNeighbors[ii][jj], meshGroundwaterMeshNeighborsEdgeLength[ii][jj], meshGroundwaterMeshNeighborsEdgeNormalX[ii][jj],
                      meshGroundwaterMeshNeighborsEdgeNormalY[ii][jj]));
                }
            }

          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; ++jj)
            {
              if (NOFLOW != meshGroundwaterChannelNeighbors[ii][jj])
                {
                  groundwaterChannelNeighbors.push_back(simpleNeighborInfo(meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[ii][jj], meshGroundwaterChannelNeighborsRegion[ii][jj],
                      meshGroundwaterChannelNeighbors[ii][jj], meshGroundwaterChannelNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }
            }

          regionProxy[meshRegion[ii]].sendInitializeMeshElement(ii + localMeshElementStart, meshCatchment[ii], meshVegetationType[ii], meshSoilType[ii],
              meshVertexX[ii], meshVertexY[ii], meshElementX[ii], meshElementY[ii], meshElementZSurface[ii], meshElementLayerZBottom[ii], meshElementArea[ii],
              meshElementSlopeX[ii], meshElementSlopeY[ii], meshLatitude[ii], meshLongitude[ii], meshManningsN[ii], meshConductivity[ii], meshPorosity[ii],
              meshSurfacewaterDepth[ii], meshSurfacewaterError[ii], meshGroundwaterHead[ii], meshGroundwaterRecharge[ii], meshGroundwaterError[ii],
              meshPrecipitationRate[ii], meshPrecipitationCumulativeShortTerm[ii], meshPrecipitationCumulativeLongTerm[ii], meshEvaporationRate[ii],
              meshEvaporationCumulativeShortTerm[ii], meshEvaporationCumulativeLongTerm[ii], meshTranspirationRate[ii], meshTranspirationCumulativeShortTerm[ii],
              meshTranspirationCumulativeLongTerm[ii], evapoTranspirationForcingInit, meshEvapoTranspirationState[ii], meshInfiltrationMethod[ii],
              meshGroundwaterMethod[ii], surfacewaterMeshNeighbors, surfacewaterChannelNeighbors, groundwaterMeshNeighbors, groundwaterChannelNeighbors);
        }

      for (ii = 0; ii < localNumberOfChannelElements; ++ii)
        {
          surfacewaterMeshNeighbors.clear();
          surfacewaterChannelNeighbors.clear();
          groundwaterMeshNeighbors.clear();

          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; ++jj)
            {
              if (NOFLOW != channelSurfacewaterMeshNeighbors[ii][jj])
                {
                  surfacewaterMeshNeighbors.push_back(simpleNeighborInfo(channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], channelSurfacewaterMeshNeighborsRegion[ii][jj],
                      channelSurfacewaterMeshNeighbors[ii][jj], channelSurfacewaterMeshNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }
            }

          for (jj = 0; jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; ++jj)
            {
              if (NOFLOW != channelSurfacewaterChannelNeighbors[ii][jj])
                {
                  surfacewaterChannelNeighbors.push_back(simpleNeighborInfo(channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
                      channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ii][jj], channelSurfacewaterChannelNeighborsRegion[ii][jj],
                      channelSurfacewaterChannelNeighbors[ii][jj], 1.0, 1.0, 0.0));
                }
            }

          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; ++jj)
            {
              if (NOFLOW != channelGroundwaterMeshNeighbors[ii][jj])
                {
                  groundwaterMeshNeighbors.push_back(simpleNeighborInfo(channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], channelGroundwaterMeshNeighborsRegion[ii][jj],
                      channelGroundwaterMeshNeighbors[ii][jj], channelGroundwaterMeshNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }
            }

          regionProxy[channelRegion[ii]].sendInitializeChannelElement(ii + localChannelElementStart, channelChannelType[ii], channelReachCode[ii],
              channelElementX[ii], channelElementY[ii], channelElementZBank[ii], channelElementZBed[ii], channelElementLength[ii], channelBaseWidth[ii],
              channelSideSlope[ii], channelBedConductivity[ii], channelBedThickness[ii], channelManningsN[ii], channelSurfacewaterDepth[ii],
              channelSurfacewaterError[ii], surfacewaterMeshNeighbors, surfacewaterChannelNeighbors, groundwaterMeshNeighbors);
        }
    }
  else
    {
      CkExit();
    }
}

#include "file_manager.def.h"
