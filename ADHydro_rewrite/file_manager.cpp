#include "file_manager.h"
#include "adhydro.h"
#include "garto.h"
#include <sys/stat.h>
#include <libgen.h>
#include <netcdf_par.h>

ElementStateMessage::ElementStateMessage() :
  elementNumber(0), // Dummy values will be overwritten by pup_stl.h code.
  surfacewaterDepth(0.0),
  surfacewaterError(0.0),
  groundwaterHead(0.0),
  groundwaterRecharge(0.0),
  groundwaterError(0.0),
  precipitationRate(0.0),
  precipitationCumulativeShortTerm(0.0),
  precipitationCumulativeLongTerm(0.0),
  evaporationRate(0.0),
  evaporationCumulativeShortTerm(0.0),
  evaporationCumulativeLongTerm(0.0),
  transpirationRate(0.0),
  transpirationCumulativeShortTerm(0.0),
  transpirationCumulativeLongTerm(0.0),
  evapoTranspirationState(),
  surfacewaterMeshNeighbors(),
  groundwaterMeshNeighbors(),
  surfacewaterChannelNeighbors(),
  groundwaterChannelNeighbors()
{
  // Initialization handled by initialization list.
}

ElementStateMessage::ElementStateMessage(int elementNumberInit, double surfacewaterDepthInit, double surfacewaterErrorInit, double groundwaterHeadInit,
                                         double groundwaterRechargeInit, double groundwaterErrorInit, double precipitationRateInit,
                                         double precipitationCumulativeShortTermInit, double precipitationCumulativeLongTermInit, double evaporationRateInit,
                                         double evaporationCumulativeShortTermInit, double evaporationCumulativeLongTermInit, double transpirationRateInit,
                                         double transpirationCumulativeShortTermInit, double transpirationCumulativeLongTermInit,
                                         EvapoTranspirationStateStruct& evapoTranspirationStateInit) :
  elementNumber(elementNumberInit),
  surfacewaterDepth(surfacewaterDepthInit),
  surfacewaterError(surfacewaterErrorInit),
  groundwaterHead(groundwaterHeadInit),
  groundwaterRecharge(groundwaterRechargeInit),
  groundwaterError(groundwaterErrorInit),
  precipitationRate(precipitationRateInit),
  precipitationCumulativeShortTerm(precipitationCumulativeShortTermInit),
  precipitationCumulativeLongTerm(precipitationCumulativeLongTermInit),
  evaporationRate(evaporationRateInit),
  evaporationCumulativeShortTerm(evaporationCumulativeShortTermInit),
  evaporationCumulativeLongTerm(evaporationCumulativeLongTermInit),
  transpirationRate(transpirationRateInit),
  transpirationCumulativeShortTerm(transpirationCumulativeShortTermInit),
  transpirationCumulativeLongTerm(transpirationCumulativeLongTermInit),
  evapoTranspirationState(evapoTranspirationStateInit),
  surfacewaterMeshNeighbors(),
  groundwaterMeshNeighbors(),
  surfacewaterChannelNeighbors(),
  groundwaterChannelNeighbors()
{
}

void ElementStateMessage::pup(PUP::er &p)
{
  p | elementNumber;
  p | surfacewaterDepth;
  p | surfacewaterError;
  p | groundwaterHead;
  p | groundwaterRecharge;
  p | groundwaterError;
  p | precipitationRate;
  p | precipitationCumulativeShortTerm;
  p | precipitationCumulativeLongTerm;
  p | evaporationRate;
  p | evaporationCumulativeShortTerm;
  p | evaporationCumulativeLongTerm;
  p | transpirationRate;
  p | transpirationCumulativeShortTerm;
  p | transpirationCumulativeLongTerm;
  p | evapoTranspirationState;
  p | surfacewaterMeshNeighbors;
  p | groundwaterMeshNeighbors;
  p | surfacewaterChannelNeighbors;
  p | groundwaterChannelNeighbors;
}

bool ElementStateMessage::checkInvariant()
{
  bool error = false; // Error flag.
  
  // FIXME error check
  
  return error;
}

time_t FileManager::wallclockTimeAtStart = time(NULL); // We want to record the wallclock time as early as possible when the program starts.  Doing it when the
                                                       // variable is created seems like a good time.
double FileManager::massBalanceTime      = NAN;        // The simulation time of the last mass balance.  Starts out as NAN until a mass balance completes.
double FileManager::massBalanceShouldBe  = NAN;        // The first mass balance value to use as the "should be" value for the rest of the simulation.
                                                       // Starts out as NAN until a mass balance completes.

void FileManager::printOutMassBalance(double messageTime, double waterInDomain, double externalFlows, double waterError)
{
  time_t wallclockTime = time(NULL);
  double massBalance   = waterInDomain + externalFlows - waterError;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= waterInDomain))
    {
      CkError("ERROR in FileManager::printOutMassBalance: waterInDomain must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!(massBalanceTime > messageTime))
    {
      massBalanceTime = messageTime;
    }
  
  if (isnan(massBalanceShouldBe))
    {
      massBalanceShouldBe = massBalance;
    }
  
  CkPrintf("simulation time = %.0lf [s], elapsed wallclock time = %d [s], waterInDomain = %lg [m^3], externalFlows = %lg [m^3], waterError = %lg [m^3], "
           "massBalance = %lg [m^3], massBalanceError = %lg [m^3].\n", messageTime, wallclockTime - wallclockTimeAtStart, waterInDomain, externalFlows,
           waterError, massBalance, massBalance - massBalanceShouldBe);
  
  // Signal file manager zero in case it is waiting for the last mass balance to finish.
  ADHydro::fileManagerProxy[0].massBalanceDone();
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
  meshGroundwaterMethod(NULL),
  meshVadoseZone(NULL),
  meshMeshNeighbors(NULL),
  meshMeshNeighborsRegion(NULL),
  meshMeshNeighborsChannelEdge(NULL),
  meshMeshNeighborsEdgeLength(NULL),
  meshMeshNeighborsEdgeNormalX(NULL),
  meshMeshNeighborsEdgeNormalY(NULL),
  meshSurfacewaterMeshNeighborsConnection(NULL),
  meshSurfacewaterMeshNeighborsExpirationTime(NULL),
  meshSurfacewaterMeshNeighborsFlowRate(NULL),
  meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  meshGroundwaterMeshNeighborsConnection(NULL),
  meshGroundwaterMeshNeighborsExpirationTime(NULL),
  meshGroundwaterMeshNeighborsFlowRate(NULL),
  meshGroundwaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  meshGroundwaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  meshChannelNeighbors(NULL),
  meshChannelNeighborsRegion(NULL),
  meshChannelNeighborsEdgeLength(NULL),
  meshSurfacewaterChannelNeighborsConnection(NULL),
  meshSurfacewaterChannelNeighborsExpirationTime(NULL),
  meshSurfacewaterChannelNeighborsFlowRate(NULL),
  meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm(NULL),
  meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm(NULL),
  meshGroundwaterChannelNeighborsConnection(NULL),
  meshGroundwaterChannelNeighborsExpirationTime(NULL),
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
  channelSurfacewaterMeshNeighborsExpirationTime(NULL),
  channelSurfacewaterMeshNeighborsFlowRate(NULL),
  channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  channelGroundwaterMeshNeighborsConnection(NULL),
  channelGroundwaterMeshNeighborsExpirationTime(NULL),
  channelGroundwaterMeshNeighborsFlowRate(NULL),
  channelGroundwaterMeshNeighborsFlowCumulativeShortTerm(NULL),
  channelGroundwaterMeshNeighborsFlowCumulativeLongTerm(NULL),
  channelChannelNeighbors(NULL),
  channelChannelNeighborsRegion(NULL),
  channelChannelNeighborsDownstream(NULL),
  channelSurfacewaterChannelNeighborsConnection(NULL),
  channelSurfacewaterChannelNeighborsExpirationTime(NULL),
  channelSurfacewaterChannelNeighborsFlowRate(NULL),
  channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm(NULL),
  channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm(NULL),
  channelPrunedSize(0),
  numberOfChannelPruned(0),
  channelPruned(NULL),
  meshNodeLocation(),
  channelNodeLocation(),
  meshVertexUpdated(NULL),
  channelVertexUpdated(NULL),
  meshElementUpdated(NULL),
  channelElementUpdated(NULL),
  geometryInstance(-1),
  geometryInstanceChecked(false),
  geometryChanged(false),
  parameterInstance(-1),
  parameterInstanceChecked(false),
  parameterChanged(false),
  stateInstance(-1),
  displayInstance(-1),
  jultimeSize(0),
  jultime(NULL),
  jultimeNextInstance(0),
  t2(NULL),
  psfc(NULL),
  u(NULL),
  v(NULL),
  qVapor(NULL),
  qCloud(NULL),
  swDown(NULL),
  gLw(NULL),
  tPrec(NULL),
  tslb(NULL),
  pblh(NULL),
  t2_c(NULL),
  psfc_c(NULL),
  u_c(NULL),
  v_c(NULL),
  qVapor_c(NULL),
  qCloud_c(NULL),
  swDown_c(NULL),
  gLw_c(NULL),
  tPrec_c(NULL),
  tslb_c(NULL),
  pblh_c(NULL),
  currentTime(0.0),
  nextCheckpointIndex(0),
  nextOutputIndex(0),
  simulationEndTime(0.0),
  simulationFinished(false)
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
  delete[] meshGroundwaterMethod;
  delete[] meshVadoseZone;
  delete[] meshMeshNeighbors;
  delete[] meshMeshNeighborsRegion;
  delete[] meshMeshNeighborsChannelEdge;
  delete[] meshMeshNeighborsEdgeLength;
  delete[] meshMeshNeighborsEdgeNormalX;
  delete[] meshMeshNeighborsEdgeNormalY;
  delete[] meshSurfacewaterMeshNeighborsConnection;
  delete[] meshSurfacewaterMeshNeighborsExpirationTime;
  delete[] meshSurfacewaterMeshNeighborsFlowRate;
  delete[] meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] meshGroundwaterMeshNeighborsConnection;
  delete[] meshGroundwaterMeshNeighborsExpirationTime;
  delete[] meshGroundwaterMeshNeighborsFlowRate;
  delete[] meshGroundwaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] meshGroundwaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] meshChannelNeighbors;
  delete[] meshChannelNeighborsRegion;
  delete[] meshChannelNeighborsEdgeLength;
  delete[] meshSurfacewaterChannelNeighborsConnection;
  delete[] meshSurfacewaterChannelNeighborsExpirationTime;
  delete[] meshSurfacewaterChannelNeighborsFlowRate;
  delete[] meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm;
  delete[] meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm;
  delete[] meshGroundwaterChannelNeighborsConnection;
  delete[] meshGroundwaterChannelNeighborsExpirationTime;
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
  delete[] channelSurfacewaterMeshNeighborsExpirationTime;
  delete[] channelSurfacewaterMeshNeighborsFlowRate;
  delete[] channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm;;
  delete[] channelGroundwaterMeshNeighborsConnection;
  delete[] channelGroundwaterMeshNeighborsExpirationTime;
  delete[] channelGroundwaterMeshNeighborsFlowRate;
  delete[] channelGroundwaterMeshNeighborsFlowCumulativeShortTerm;
  delete[] channelGroundwaterMeshNeighborsFlowCumulativeLongTerm;
  delete[] channelChannelNeighbors;
  delete[] channelChannelNeighborsRegion;
  delete[] channelChannelNeighborsDownstream;
  delete[] channelSurfacewaterChannelNeighborsConnection;
  delete[] channelSurfacewaterChannelNeighborsExpirationTime;
  delete[] channelSurfacewaterChannelNeighborsFlowRate;
  delete[] channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm;
  delete[] channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm;
  delete[] channelPruned;
  delete[] meshVertexUpdated;
  delete[] channelVertexUpdated;
  delete[] meshElementUpdated;
  delete[] channelElementUpdated;
  delete[] jultime;
  delete[] t2;
  delete[] psfc;
  delete[] u;
  delete[] v;
  delete[] qVapor;
  delete[] qCloud;
  delete[] swDown;
  delete[] gLw;
  delete[] tPrec;
  delete[] tslb;
  delete[] pblh;
  delete[] t2_c;
  delete[] psfc_c;
  delete[] u_c;
  delete[] v_c;
  delete[] qVapor_c;
  delete[] qCloud_c;
  delete[] swDown_c;
  delete[] gLw_c;
  delete[] tPrec_c;
  delete[] tslb_c;
  delete[] pblh_c;
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
  int                soilType;                 // Used to store soil type of the top layer.
  double             soilDepthReader;          // Used to read each soil layers thickness.
  double             soilDepth;                // Used to store the sum of each soil layers thickness.
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: could not open ele file %s.\n", ADHydro::ASCIIInputMeshElementFilePath.c_str());
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read header from ele file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < globalNumberOfMeshElements && 3 == dimension && 1 == numberOfAttributes))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid header in ele file.\n");
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: could not open neigh file %s.\n", ADHydro::ASCIIInputMeshNeighborFilePath.c_str());
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read header from neigh file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck && 3 == dimension))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid header in neigh file.\n");
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: could not open landCover file %s.\n", ADHydro::ASCIIInputMeshLandCoverFilePath.c_str());
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read header from landCover file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid header in landCover file.\n");
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: could not open soilType file %s.\n", ADHydro::ASCIIInputMeshSoilTypeFilePath.c_str());
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read header from soilType file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid header in soilType file.\n");
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: could not open geolType file %s.\n", ADHydro::ASCIIInputMeshGeolTypeFilePath.c_str());
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read header from geolType file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(globalNumberOfMeshElements == numberCheck))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid header in geolType file.\n");
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from ele file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid element number in ele file.  %d should be %d.\n", index, ii);
          error = true;
        }

      if (!(0 <= vertex0 && vertex0 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex0);
          error = true;
        }

      if (!(0 <= vertex1 && vertex1 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex1);
          error = true;
        }

      if (!(0 <= vertex2 && vertex2 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex2);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Read neigh file.
      numScanned = fscanf(neighFile, "%d %d %d %d", &numberCheck, &neighbor0, &neighbor1, &neighbor2);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from neigh file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid element number in neigh file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }

      if (!(isBoundary(neighbor0) || (0 <= neighbor0 && neighbor0 < globalNumberOfMeshElements)))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor0);
          error = true;
        }

      if (!(isBoundary(neighbor1) || (0 <= neighbor1 && neighbor1 < globalNumberOfMeshElements)))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor1);
          error = true;
        }

      if (!(isBoundary(neighbor2) || (0 <= neighbor2 && neighbor2 < globalNumberOfMeshElements)))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor2);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Read landCover file.
      numScanned = fscanf(landCoverFile, "%d %d", &numberCheck, &vegetationType);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from landCover file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid element number in landCover file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }

      if (!(1 <= vegetationType && 27 >= vegetationType))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid vegetation type number %d in landCover file.\n", index,
                  vegetationType);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // Read soil file.
      numScanned = fscanf(soilTypeFile, "%d %d", &numberCheck, &numberOfSoilLayers);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from soilType file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid element number in soilType file.  %d should be %d.\n", numberCheck, index);
          error = true;
        }

      if (!(0 <= numberOfSoilLayers))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid number of soil layers %d in soilType file.\n", index,
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
                  CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d soil layer %d from soilType file.\n", ii, jj);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
              if (!((1 <= soilTypeReader && 19 >= soilTypeReader) || -1 == soilTypeReader))
                {
                  CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: invalid soil type %s in soilType file.\n", index,
                          soilTypeReader);
                  error = true;
                }

              if (!(0.0 <= soilDepthReader))
                {
                  CkError("ERROR in FileManager::initializeFromASCIIFiles: mesh element %d: soilDepthReader must be greater than or equal to zero.\n",
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from geolType file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(index == numberCheck))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid element number in geolType file.  %d should be %d.\n", numberCheck, index);
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: could not open edge file %s.\n", ADHydro::ASCIIInputMeshEdgeFilePath.c_str());
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read header from edge file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfEdges && 1 == boundary))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid header in edge file.\n");
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from edge file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid edge number in edge file.  %d should be %d.\n", index, ii);
          error = true;
        }
      
      if (!(0 <= vertex0 && vertex0 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: edge %d: invalid vertex number %d in edge file.\n", index, vertex0);
          error = true;
        }
      
      if (!(0 <= vertex1 && vertex1 < globalNumberOfMeshNodes))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: edge %d: invalid vertex number %d in edge file.\n", index, vertex1);
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: could not open chan.ele file %s.\n", ADHydro::ASCIIInputChannelElementFilePath.c_str());
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read header from chan.ele file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < globalNumberOfChannelElements))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid header in chan.ele file.\n");
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
          CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == index))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: invalid element number in chan.ele file.  %d should be %d.\n", index, ii);
          error = true;
        }
      
      if (!(STREAM == type || WATERBODY == type || ICEMASS == type))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: channel type must be a valid enum value in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0 < reachCode1))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: reach code must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0.0 < length))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: length must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0.0 < topWidth))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: topWidth must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0.0 < bankFullDepth))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: bankFullDepth must be greater than zero in chan.ele file.\n", index);
          error = true;
        }
      
      if (!(0 < numberOfVertices && numberOfVertices <= CHANNEL_ELEMENT_VERTICES_SIZE))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: numberOfVertices must be greater than zero and less than or "
                  "equal to the maximum number of channel vertices %d in chan.ele file.\n", index, CHANNEL_ELEMENT_VERTICES_SIZE);
          error = true;
        }
      
      if (!(0 <= numberOfChannelNeighbors && numberOfChannelNeighbors <= CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: numberOfChannelNeighbors must be greater than or equal to zero and less "
                  "than or equal to the maximum number of channel channel neighbors %d in chan.ele file.\n", index, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE);
          error = true;
        }
      
      if (!(0 <= numberOfMeshNeighbors && numberOfMeshNeighbors <= CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE))
        {
          CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: numberOfMeshNeighbors must be greater than or equal to zero and less "
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
          // FIXLATER This assumes rectangular channels.  Calculate some other way?
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
              CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", index);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 <= vertex0 && vertex0 < globalNumberOfChannelNodes))
            {
              CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: invalid vertex number %d in ele file.\n", index, vertex0);
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
              CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", index);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(isBoundary(neighbor0) || (0 <= neighbor0 && neighbor0 < globalNumberOfChannelElements)))
            {
              CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: invalid channel neighbor number %d in chan.ele file.\n", index,
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
              CkError("ERROR in FileManager::initializeFromASCIIFiles: unable to read entry %d from chan.ele file.\n", index);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(isBoundary(neighbor0) || (0 <= neighbor0 && neighbor0 < globalNumberOfMeshElements)))
            {
              CkError("ERROR in FileManager::initializeFromASCIIFiles: channel %d: invalid mesh neighbor number %d in chan.ele file.\n", index, neighbor0);
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
              CkError("ERROR in FileManager::initializeFromASCIIFiles: could not open chan.prune file %s.\n", ADHydro::ASCIIInputChannelPruneFilePath.c_str());
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

bool FileManager::NetCDFOpenForRead(const char* path, int* fileID)
{
  bool error = false; // Error flag.
  int  ncErrorCode;   // Return value of NetCDF functions.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != path))
    {
      CkError("ERROR in FileManager::NetCDFOpenForRead: path must not be null.\n");
      error = true;
    }
  
  if (!(NULL != fileID))
    {
      CkError("ERROR in FileManager::NetCDFOpenForRead: fileID must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      ncErrorCode = nc_open_par(path, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFOpenForRead: could not open for read NetCDF file %s.  NetCDF error message: %s.\n", path,
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  return error;
}

bool FileManager::NetCDFCreateOrOpenForWrite(const char* path, int* fileID, bool* created)
{
  bool        error     = false; // Error flag.
  int         ncErrorCode;       // Return value of NetCDF functions.
  char*       tmpPath   = NULL;  // Temporary string for string manipulation.
  char*       directory = NULL;  // Directory where file is located.
  struct stat fileStatus;        // Used to detect whether files and directories exist.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != path))
    {
      CkError("ERROR in FileManager::NetCDFCreateOrOpenForWrite: path must not be null.\n");
      error = true;
    }
  
  if (!(NULL != fileID))
    {
      CkError("ERROR in FileManager::NetCDFCreateOrOpenForWrite: fileID must not be null.\n");
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
          CkError("ERROR in FileManager::NetCDFCreateOrOpenForWrite: could not allocate memory for tmpPath.\n");
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
              CkError("ERROR in FileManager::NetCDFCreateOrOpenForWrite: Cannot create directory %s.\n", directory);
              error = true;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      else if (S_IFDIR != (fileStatus.st_mode & S_IFMT))
        {
          // File exists, but is not directory.  Error.
          CkError("ERROR in FileManager::NetCDFCreateOrOpenForWrite: Cannot create file %s because %s already exists, but is not a directory.\n", path,
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
      ncErrorCode = nc_open_par(path, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, fileID);
      
      if (!(NC_NOERR == ncErrorCode))
        {
          ncErrorCode = nc_create_par(path, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, fileID);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::NetCDFCreateOrOpenForWrite: could not create or open for write NetCDF file %s.  NetCDF error message: %s.\n",
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

bool FileManager::NetCDFCreateOrOpenForWriteGeometry(int* fileID)
{
  bool error    = false;                       // Error flag.
  int  ncErrorCode;                            // Return value of NetCDF functions.
  bool created;                                // Whether the file is being created.
  bool fileOpen = false;                       // Whether the file is open.
  int  instancesDimensionID;                   // ID of dimension in NetCDF file.
  int  meshNodesDimensionID;                   // ID of dimension in NetCDF file.
  int  meshElementsDimensionID;                // ID of dimension in NetCDF file.
  int  meshMeshNeighborsSizeDimensionID;       // ID of dimension in NetCDF file.
  int  meshChannelNeighborsSizeDimensionID;    // ID of dimension in NetCDF file.
  int  channelNodesDimensionID;                // ID of dimension in NetCDF file.
  int  channelElementsDimensionID;             // ID of dimension in NetCDF file.
  int  channelVerticesSizeDimensionID;         // ID of dimension in NetCDF file.
  int  channelVerticesXDMFSizeDimensionID;     // ID of dimension in NetCDF file.
  int  channelMeshNeighborsSizeDimensionID;    // ID of dimension in NetCDF file.
  int  channelChannelNeighborsSizeDimensionID; // ID of dimension in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != fileID))
    {
      CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteGeometry: fileID must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      error = NetCDFCreateOrOpenForWrite(ADHydro::adhydroOutputGeometryFilePath.c_str(), fileID, &created);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // If the file was created we also need to create all of the dimensions and variables.
  if (!error && created)
    {
      // Create geometry dimensions.
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "instances", NC_UNLIMITED, &instancesDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshNodes", NC_UNLIMITED, &meshNodesDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshElements", NC_UNLIMITED, &meshElementsDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshMeshNeighborsSize", MESH_ELEMENT_MESH_NEIGHBORS_SIZE, &meshMeshNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshChannelNeighborsSize", MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, &meshChannelNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelNodes", NC_UNLIMITED, &channelNodesDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelElements", NC_UNLIMITED, &channelElementsDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelVerticesSize", CHANNEL_ELEMENT_VERTICES_SIZE, &channelVerticesSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelVerticesXDMFSize", XDMF_SIZE, &channelVerticesXDMFSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelMeshNeighborsSize", CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, &channelMeshNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelChannelNeighborsSize", CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE,
                                        &channelChannelNeighborsSizeDimensionID);
        }
      
      // Create geometry variables.
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "centralMeridian", NC_DOUBLE, 1, instancesDimensionID, 0, 0, "radians",
                                       "The central meridian of the sinusoidal projection that defines the coordinate system.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "falseEasting", NC_DOUBLE, 1, instancesDimensionID, 0, 0, "meters",
                                       "The false easting of the sinusoidal projection that defines the coordinate system.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "falseNorthing", NC_DOUBLE, 1, instancesDimensionID, 0, 0, "meters",
                                       "The false northing of the sinusoidal projection that defines the coordinate system.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "numberOfMeshNodes", NC_INT, 1, instancesDimensionID, 0, 0, NULL,
                                       "The number of mesh nodes that are actually used in each geometry instance.  This can be less than the meshNodes "
                                       "dimension because that dimension has to be the size of the largest instance.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "numberOfMeshElements", NC_INT, 1, instancesDimensionID, 0, 0, NULL,
                                       "The number of mesh elements that are actually used in each geometry instance.  This can be less than the meshElements "
                                       "dimension because that dimension has to be the size of the largest instance.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "numberOfChannelNodes", NC_INT, 1, instancesDimensionID, 0, 0, NULL,
                                       "The number of channel nodes that are actually used in each geometry instance.  This can be less than the channelNodes "
                                       "dimension because that dimension has to be the size of the largest instance.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "numberOfChannelElements", NC_INT, 1, instancesDimensionID, 0, 0, NULL,
                                       "The number of channel elements that are actually used in each geometry instance.  This can be less than the "
                                       "channelElements dimension because that dimension has to be the size of the largest instance.");
        }
      
      if (!error && NULL != meshNodeX)
        {
          error = NetCDFCreateVariable(*fileID, "meshNodeX", NC_DOUBLE, 2, instancesDimensionID, meshNodesDimensionID, 0, "meters",
                                       "X coordinates of mesh nodes indexed by node number.");
        }
      
      if (!error && NULL != meshNodeY)
        {
          error = NetCDFCreateVariable(*fileID, "meshNodeY", NC_DOUBLE, 2, instancesDimensionID, meshNodesDimensionID, 0, "meters",
                                       "Y coordinates of mesh nodes indexed by node number.");
        }
      
      if (!error && NULL != meshNodeZSurface)
        {
          error = NetCDFCreateVariable(*fileID, "meshNodeZSurface", NC_DOUBLE, 2, instancesDimensionID, meshNodesDimensionID, 0, "meters",
                                       "Z coordinates of ground surface of mesh nodes indexed by node number.");
        }
      
      if (!error && NULL != meshElementVertices)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementVertices", NC_INT, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, NULL,
                                       "Node numbers of mesh element vertices indexed by element number and vertex number.");
        }
      
      if (!error && NULL != meshVertexX)
        {
          error = NetCDFCreateVariable(*fileID, "meshVertexX", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID, meshMeshNeighborsSizeDimensionID,
                                       "meters", "X coordinates of mesh element vertices indexed by element number and vertex number.");
        }
      
      if (!error && NULL != meshVertexY)
        {
          error = NetCDFCreateVariable(*fileID, "meshVertexY", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID, meshMeshNeighborsSizeDimensionID,
                                       "meters", "Y coordinates of mesh element vertices indexed by element number and vertex number.");
        }
      
      if (!error && NULL != meshVertexZSurface)
        {
          error = NetCDFCreateVariable(*fileID, "meshVertexZSurface", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, "meters",
                                       "Z coordinates of ground surface of mesh element vertices indexed by element number and vertex number.");
        }
      
      if (!error && NULL != meshElementX)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementX", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "X coordinates of mesh element centers indexed by element number.");
        }
      
      if (!error && NULL != meshElementY)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementY", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Y coordinates of mesh element centers indexed by element number.");
        }
      
      if (!error && NULL != meshElementZSurface)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementZSurface", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Z coordinates of ground surface of mesh element centers indexed by element number.");
        }
      
      if (!error && NULL != meshElementSoilDepth)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementSoilDepth", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Thickness of soil layer of mesh element centers indexed by element number.");
        }
      
      if (!error && NULL != meshElementLayerZBottom)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementLayerZBottom", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Z coordinates of bottom of soil layer of mesh element centers indexed by element number.");
        }
      
      if (!error && NULL != meshElementArea)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementArea", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters^2",
                                       "Horizontal area ignoring Z coordinates of mesh elements indexed by element number.");
        }
      
      if (!error && NULL != meshElementSlopeX)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementSlopeX", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters/meter",
                                       "Slope in the X direction of mesh elements indexed by element number.  Positive means sloping upward in the positive X "
                                       "direction.");
        }
      
      if (!error && NULL != meshElementSlopeY)
        {
          error = NetCDFCreateVariable(*fileID, "meshElementSlopeY", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters/meter",
                                       "Slope in the Y direction of mesh elements indexed by element number.  Positive means sloping upward in the positive Y "
                                       "direction.");
        }
      
      if (!error && NULL != meshLatitude)
        {
          error = NetCDFCreateVariable(*fileID, "meshLatitude", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "radians",
                                       "Latitude of mesh element centers indexed by element number.");
        }
      
      if (!error && NULL != meshLongitude)
        {
          error = NetCDFCreateVariable(*fileID, "meshLongitude", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "radians",
                                       "Longitude of mesh element centers indexed by element number.");
        }
      
      if (!error && NULL != meshMeshNeighbors)
        {
          error = NetCDFCreateVariable(*fileID, "meshMeshNeighbors", NC_INT, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, NULL, "Element numbers of the mesh element neighbors of a mesh element");
        }
      
      if (!error && NULL != meshMeshNeighborsChannelEdge)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "meshMeshNeighborsChannelEdge", NC_BYTE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, NULL, "Whether the edges of a mesh element are covered by a channel");
        }
      
      if (!error && NULL != meshMeshNeighborsEdgeLength)
        {
          error = NetCDFCreateVariable(*fileID, "meshMeshNeighborsEdgeLength", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, "meters", "The lengths of the edges of a mesh element.");
        }
      
      if (!error && NULL != meshMeshNeighborsEdgeNormalX)
        {
          error = NetCDFCreateVariable(*fileID, "meshMeshNeighborsEdgeNormalX", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, NULL,
                                       "X components of unit vectors normal to the edges of a mesh element directed out of the element.");
        }
      
      if (!error && NULL != meshMeshNeighborsEdgeNormalY)
        {
          error = NetCDFCreateVariable(*fileID, "meshMeshNeighborsEdgeNormalY", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, NULL,
                                       "Y components of unit vectors normal to the edges of a mesh element directed out of the element.");
        }
      
      if (!error && NULL != meshSurfacewaterMeshNeighborsConnection)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterMeshNeighborsConnection", NC_BYTE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, NULL,
                                       "Whether the surfacewater of a mesh element is connected to the surfacewater of its mesh neighbor.  They might not be "
                                       "connected if they are in different catchments or have a channel in between them.");
        }
      
      if (!error && NULL != meshGroundwaterMeshNeighborsConnection)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterMeshNeighborsConnection", NC_BYTE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, NULL,
                                       "Whether the groundwater of a mesh element is connected to the groundwater of its mesh neighbor.  They might not be "
                                       "connected if one of them is not simulating groundwater.");
        }
      
      if (!error && NULL != meshChannelNeighbors)
        {
          error = NetCDFCreateVariable(*fileID, "meshChannelNeighbors", NC_INT, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, NULL, "Element numbers of the channel element neighbors of a mesh element");
        }
      
      if (!error && NULL != meshChannelNeighborsEdgeLength)
        {
          error = NetCDFCreateVariable(*fileID, "meshChannelNeighborsEdgeLength", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, "meters",
                                       "The connected lengths between a mesh element and its channel neighbors.");
        }
      
      if (!error && NULL != meshSurfacewaterChannelNeighborsConnection)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterChannelNeighborsConnection", NC_BYTE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, NULL,
                                       "Whether the surfacewater of a mesh element is connected to the surfacewater of its channel neighbor.  There are "
                                       "currently no conditions under which they might not be connected.");
        }
      
      if (!error && NULL != meshGroundwaterChannelNeighborsConnection)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterChannelNeighborsConnection", NC_BYTE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, NULL,
                                       "Whether the groundwater of a mesh element is connected to the surfacewater of its channel neighbor.  They might not "
                                       "be connected if the mesh element is not simulating groundwater.");
        }
      
      if (!error && NULL != channelNodeX)
        {
          error = NetCDFCreateVariable(*fileID, "channelNodeX", NC_DOUBLE, 2, instancesDimensionID, channelNodesDimensionID, 0, "meters",
                                       "X coordinates of channel nodes indexed by node number.");
        }
      
      if (!error && NULL != channelNodeY)
        {
          error = NetCDFCreateVariable(*fileID, "channelNodeY", NC_DOUBLE, 2, instancesDimensionID, channelNodesDimensionID, 0, "meters",
                                       "Y coordinates of channel nodes indexed by node number.");
        }
      
      if (!error && NULL != channelNodeZBank)
        {
          error = NetCDFCreateVariable(*fileID, "channelNodeZBank", NC_DOUBLE, 2, instancesDimensionID, channelNodesDimensionID, 0, "meters",
                                       "Z coordinates of channel bank of channel nodes indexed by node number.");
        }
      
      if (!error && NULL != channelElementVertices)
        {
          error = NetCDFCreateVariable(*fileID, "channelElementVertices", NC_INT, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelVerticesXDMFSizeDimensionID, NULL,
                                       "Node numbers of channel element vertices indexed by element number and vertex number.  The first two entries for each "
                                       "element are metadata used for a mixed topology in the XDMF file format and should be ignored.");
        }
      
      if (!error && NULL != channelVertexX)
        {
          error = NetCDFCreateVariable(*fileID, "channelVertexX", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelVerticesSizeDimensionID, "meters",
                                       "X coordinates of channel element vertices indexed by element number and vertex number.");
        }
      
      if (!error && NULL != channelVertexY)
        {
          error = NetCDFCreateVariable(*fileID, "channelVertexY", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelVerticesSizeDimensionID, "meters",
                                       "Y coordinates of channel element vertices indexed by element number and vertex number.");
        }
      
      if (!error && NULL != channelVertexZBank)
        {
          error = NetCDFCreateVariable(*fileID, "channelVertexZBank", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelVerticesSizeDimensionID, "meters",
                                       "Z coordinates of channel bank of channel element vertices indexed by element number and vertex number.");
        }
      
      if (!error && NULL != channelElementX)
        {
          error = NetCDFCreateVariable(*fileID, "channelElementX", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "X coordinates of channel element centers indexed by element number.");
        }
      
      if (!error && NULL != channelElementY)
        {
          error = NetCDFCreateVariable(*fileID, "channelElementY", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "Y coordinates of channel element centers indexed by element number.");
        }
      
      if (!error && NULL != channelElementZBank)
        {
          error = NetCDFCreateVariable(*fileID, "channelElementZBank", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "Z coordinates of channel bank of channel element centers indexed by element number.");
        }
      
      if (!error && NULL != channelElementBankFullDepth)
        {
          error = NetCDFCreateVariable(*fileID, "channelElementBankFullDepth", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "Depth from bank to bed of channel element centers indexed by element number.");
        }
      
      if (!error && NULL != channelElementZBed)
        {
          error = NetCDFCreateVariable(*fileID, "channelElementZBed", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "Z coordinates of channel bed of channel element centers indexed by element number.");
        }
      
      if (!error && NULL != channelElementLength)
        {
          error = NetCDFCreateVariable(*fileID, "channelElementLength", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "Horizontal length ignoring Z coordinates of channel elements indexed by element number.");
        }
      
      if (!error && NULL != channelLatitude)
        {
          error = NetCDFCreateVariable(*fileID, "channelLatitude", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "radians",
                                       "Latitude of channel element centers indexed by element number.");
        }
      
      if (!error && NULL != channelLongitude)
        {
          error = NetCDFCreateVariable(*fileID, "channelLongitude", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "radians",
                                       "Longitude of channel element centers indexed by element number.");
        }
      
      if (!error && NULL != channelMeshNeighbors)
        {
          error = NetCDFCreateVariable(*fileID, "channelMeshNeighbors", NC_INT, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelMeshNeighborsSizeDimensionID, NULL, "Element numbers of the mesh element neighbors of a channel element");
        }
      
      if (!error && NULL != channelMeshNeighborsEdgeLength)
        {
          error = NetCDFCreateVariable(*fileID, "channelMeshNeighborsEdgeLength", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelMeshNeighborsSizeDimensionID, "meters",
                                       "The connected lengths between a channel element and its mesh neighbors.");
        }
      
      if (!error && NULL != channelSurfacewaterMeshNeighborsConnection)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterMeshNeighborsConnection", NC_BYTE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelMeshNeighborsSizeDimensionID, NULL,
                                       "Whether the surfacewater of a channel element is connected to the surfacewater of its mesh neighbor.  There are "
                                       "currently no conditions under which they might not be connected.");
        }
      
      if (!error && NULL != channelGroundwaterMeshNeighborsConnection)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "channelGroundwaterMeshNeighborsConnection", NC_BYTE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelMeshNeighborsSizeDimensionID, NULL,
                                       "Whether the surfacewater of a channel element is connected to the groundwater of its mesh neighbor.  They might not "
                                       "be connected if the mesh element is not simulating groundwater.");
        }
      
      if (!error && NULL != channelChannelNeighbors)
        {
          error = NetCDFCreateVariable(*fileID, "channelChannelNeighbors", NC_INT, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelChannelNeighborsSizeDimensionID, NULL, "Element numbers of the channel element neighbors of a channel element");
        }
      
      if (!error && NULL != channelChannelNeighborsDownstream)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "channelChannelNeighborsDownstream", NC_BYTE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelChannelNeighborsSizeDimensionID, NULL,
                                       "Whether the channel element neighbors of a channel element are downstream of that channel element.");
        }
      
      if (!error && NULL != channelSurfacewaterChannelNeighborsConnection)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterChannelNeighborsConnection", NC_BYTE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelChannelNeighborsSizeDimensionID, NULL,
                                       "Whether the surfacewater of a channel element is connected to the surfacewater of its channel neighbor.  There are "
                                       "currently no conditions under which they might not be connected.");
        }
    }
  // FIXME else if not error and the file was not created we could check that the dimensions and variables exist and the dimensions are the right size.
  
  if (error && fileOpen)
    {
      ncErrorCode = nc_close(*fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteGeometry: unable to close NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  return error;
}

bool FileManager::NetCDFCreateOrOpenForWriteParameter(int* fileID)
{
  bool error    = false;                       // Error flag.
  int  ncErrorCode;                            // Return value of NetCDF functions.
  bool created;                                // Whether the file is being created.
  bool fileOpen = false;                       // Whether the file is open.
  int  instancesDimensionID;                   // ID of dimension in NetCDF file.
  int  regionsDimensionID;                     // ID of dimension in NetCDF file.
  int  meshElementsDimensionID;                // ID of dimension in NetCDF file.
  int  meshMeshNeighborsSizeDimensionID;       // ID of dimension in NetCDF file.
  int  meshChannelNeighborsSizeDimensionID;    // ID of dimension in NetCDF file.
  int  channelElementsDimensionID;             // ID of dimension in NetCDF file.
  int  channelMeshNeighborsSizeDimensionID;    // ID of dimension in NetCDF file.
  int  channelChannelNeighborsSizeDimensionID; // ID of dimension in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != fileID))
    {
      CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteParameter: fileID must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      error = NetCDFCreateOrOpenForWrite(ADHydro::adhydroOutputParameterFilePath.c_str(), fileID, &created);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // If the file was created we also need to create all of the dimensions and variables.
  if (!error && created)
    {
      // Create parameter dimensions.
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "instances", NC_UNLIMITED, &instancesDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "regions", NC_UNLIMITED, &regionsDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshElements", NC_UNLIMITED, &meshElementsDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshMeshNeighborsSize", MESH_ELEMENT_MESH_NEIGHBORS_SIZE, &meshMeshNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshChannelNeighborsSize", MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, &meshChannelNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelElements", NC_UNLIMITED, &channelElementsDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelMeshNeighborsSize", CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, &channelMeshNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelChannelNeighborsSize", CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE,
                                        &channelChannelNeighborsSizeDimensionID);
        }
      
      // Create parameter variables.
      if (!error)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes size_t is 8 bytes when storing as NC_UINT64.
          CkAssert(8 == sizeof(size_t));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "geometryInstance", NC_UINT64, 1, instancesDimensionID, 0, 0, NULL,
                                       "The instance in the geometry file to use with each instance in the parameter file.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "numberOfRegions", NC_INT, 1, instancesDimensionID, 0, 0, NULL,
                                       "The number of regions that are actually used in each parameter instance.  This can be less than the regions "
                                       "dimension because that dimension has to be the size of the largest instance.");
        }
      
      if (!error && NULL != regionNumberOfMeshElements)
        {
          error = NetCDFCreateVariable(*fileID, "regionNumberOfMeshElements", NC_INT, 2, instancesDimensionID, regionsDimensionID, 0, NULL,
                                       "The number of mesh elements that are in each region.");
        }
      
      if (!error && NULL != regionNumberOfChannelElements)
        {
          error = NetCDFCreateVariable(*fileID, "regionNumberOfChannelElements", NC_INT, 2, instancesDimensionID, regionsDimensionID, 0, NULL,
                                       "The number of channel elements that are in each region.");
        }
      
      if (!error && NULL != meshRegion)
        {
          error = NetCDFCreateVariable(*fileID, "meshRegion", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0, NULL,
                                       "The region number of each mesh element.");
        }
      
      if (!error && NULL != meshCatchment)
        {
          error = NetCDFCreateVariable(*fileID, "meshCatchment", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0, NULL,
                                       "The catchment number of each mesh element.  This is the reach code of the stream reach that drains the catchment.");
        }
      
      if (!error && NULL != meshVegetationType)
        {
          error = NetCDFCreateVariable(*fileID, "meshVegetationType", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0, NULL,
                                       "The vegetation type of each mesh element from the USGS vegetation type list in the Noah-MP VEGPARM.TBL file.");
        }
      
      if (!error && NULL != meshSoilType)
        {
          error = NetCDFCreateVariable(*fileID, "meshSoilType", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0, NULL,
                                       "The soil type of each mesh element from the STAS soil type list in the Noah-MP SOILPARM.TBL file.");
        }
      
      if (!error && NULL != meshAlluvium)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes bool is 1 byte when storing as NC_BYTE.
          CkAssert(1 == sizeof(bool));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
          error = NetCDFCreateVariable(*fileID, "meshAlluvium", NC_BYTE, 2, instancesDimensionID, meshElementsDimensionID, 0, NULL,
                                       "Whether each mesh element is in an alluvial aquifer.  Elements not in an alluvial aquifer do not simulate "
                                       "groundwater.");
        }
      
      if (!error && NULL != meshManningsN)
        {
          error = NetCDFCreateVariable(*fileID, "meshManningsN", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "seconds/meters^(1/3)",
                                       "Surface roughness of each mesh element.");
        }
      
      if (!error && NULL != meshConductivity)
        {
          error = NetCDFCreateVariable(*fileID, "meshConductivity", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters/second",
                                       "Soil conductivity of each mesh element.");
        }
      
      if (!error && NULL != meshPorosity)
        {
          error = NetCDFCreateVariable(*fileID, "meshPorosity", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters^3/meter^3",
                                       "Soil porosity of each mesh element.");
        }
      
      if (!error && NULL != meshGroundwaterMethod)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes InfiltrationAndGroundwater::GroundwaterMethodEnum is 4 bytes when storing as NC_INT.
          CkAssert(4 == sizeof(InfiltrationAndGroundwater::GroundwaterMethodEnum));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterMethod", NC_INT, 2, instancesDimensionID, meshElementsDimensionID, 0, NULL,
                                       "Groundwater simulation method of each mesh element from InfiltrationAndGroundwater::GroundwaterMethodEnum");
        }
      
      if (!error && NULL != meshMeshNeighborsRegion)
        {
          error = NetCDFCreateVariable(*fileID, "meshMeshNeighborsRegion", NC_INT, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, NULL, "Region numbers of the mesh element neighbors of a mesh element");
        }
      
      if (!error && NULL != meshChannelNeighborsRegion)
        {
          error = NetCDFCreateVariable(*fileID, "meshChannelNeighborsRegion", NC_INT, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, NULL, "Region numbers of the channel element neighbors of a mesh element");
        }
      
      if (!error && NULL != channelRegion)
        {
          error = NetCDFCreateVariable(*fileID, "channelRegion", NC_INT, 2, instancesDimensionID, channelElementsDimensionID, 0, NULL,
                                       "The region number of each channel element.");
        }
      
      if (!error && NULL != channelChannelType)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes ChannelTypeEnum is 4 bytes when storing as NC_INT.
          CkAssert(4 == sizeof(ChannelTypeEnum));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "channelChannelType", NC_INT, 2, instancesDimensionID, channelElementsDimensionID, 0, NULL,
                                       "Channel type of each channel element from ChannelTypeEnum");
        }
      
      if (!error && NULL != channelReachCode)
        {
          error = NetCDFCreateVariable(*fileID, "channelReachCode", NC_INT64, 2, instancesDimensionID, channelElementsDimensionID, 0, NULL,
                                       "The reach code of each channel element.  This is a unique ID for stream reaches and waterbodies.");
        }
      
      if (!error && NULL != channelBaseWidth)
        {
          error = NetCDFCreateVariable(*fileID, "channelBaseWidth", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "The width of the base of the trapeziodal channel profile of each channel element.");
        }
      
      if (!error && NULL != channelSideSlope)
        {
          error = NetCDFCreateVariable(*fileID, "channelSideSlope", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters/meter",
                                       "For the trapezoidal channel profile of each channel element, this is the widening of each bank per unit height above "
                                       "the channel bed.  Note: this is run over rise, the inverse of the traditional definition of slope.  Note: this is the "
                                       "widening of each bank so the channel widens by twice this amount.");
        }
      
      if (!error && NULL != channelBedConductivity)
        {
          error = NetCDFCreateVariable(*fileID, "channelBedConductivity", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters/second",
                                       "Soil conductivity of the bed of each channel element.");
        }
      
      if (!error && NULL != channelBedThickness)
        {
          error = NetCDFCreateVariable(*fileID, "channelBedThickness", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "Soil thickness of the bed of each channel element.");
        }
      
      if (!error && NULL != channelManningsN)
        {
          error = NetCDFCreateVariable(*fileID, "channelManningsN", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "seconds/meters^(1/3)",
                                       "Surface roughness of each channel element.");
        }
      
      if (!error && NULL != channelMeshNeighborsRegion)
        {
          error = NetCDFCreateVariable(*fileID, "channelMeshNeighborsRegion", NC_INT, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelMeshNeighborsSizeDimensionID, NULL, "Region numbers of the mesh element neighbors of a channel element");
        }
      
      if (!error && NULL != channelChannelNeighborsRegion)
        {
          error = NetCDFCreateVariable(*fileID, "channelChannelNeighborsRegion", NC_INT, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelChannelNeighborsSizeDimensionID, NULL, "Region numbers of the channel element neighbors of a channel element");
        }
    }
  // FIXME else if not error the file was not created we could check that the dimensions and variables exist and the dimensions are the right size.
  
  if (error && fileOpen)
    {
      ncErrorCode = nc_close(*fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteParameter: unable to close NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFCreateOrOpenForWriteState(int* fileID)
{
  bool    error    = false;                       // Error flag.
  int     ncErrorCode;                            // Return value of NetCDF functions.
  bool    created;                                // Whether the file is being created.
  bool    fileOpen = false;                       // Whether the file is open.
  int     instancesDimensionID;                   // ID of dimension in NetCDF file.
  int     meshElementsDimensionID;                // ID of dimension in NetCDF file.
  int     meshMeshNeighborsSizeDimensionID;       // ID of dimension in NetCDF file.
  int     meshChannelNeighborsSizeDimensionID;    // ID of dimension in NetCDF file.
  int     channelElementsDimensionID;             // ID of dimension in NetCDF file.
  int     channelMeshNeighborsSizeDimensionID;    // ID of dimension in NetCDF file.
  int     channelChannelNeighborsSizeDimensionID; // ID of dimension in NetCDF file.
  nc_type EvapoTranspirationStateStructTypeID;    // ID of type in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != fileID))
    {
      CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteState: fileID must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      error = NetCDFCreateOrOpenForWrite(ADHydro::adhydroOutputStateFilePath.c_str(), fileID, &created);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // If the file was created we also need to create all of the dimensions and variables.
  if (!error && created)
    {
      // Create state dimensions.
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "instances", NC_UNLIMITED, &instancesDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshElements", NC_UNLIMITED, &meshElementsDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshMeshNeighborsSize", MESH_ELEMENT_MESH_NEIGHBORS_SIZE, &meshMeshNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "meshChannelNeighborsSize", MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, &meshChannelNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelElements", NC_UNLIMITED, &channelElementsDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelMeshNeighborsSize", CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, &channelMeshNeighborsSizeDimensionID);
        }
      
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "channelChannelNeighborsSize", CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE,
                                        &channelChannelNeighborsSizeDimensionID);
        }
      
      // Create data type for EvapoTranspirationStateStruct.
      if (!error)
        {
          ncErrorCode = nc_def_opaque(*fileID, sizeof(EvapoTranspirationStateStruct), "EvapoTranspirationStateStruct", &EvapoTranspirationStateStructTypeID);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteState: unable to create EvapoTranspirationStateStruct type.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Create state variables.
      if (!error)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes size_t is 8 bytes when storing as NC_UINT64.
          CkAssert(8 == sizeof(size_t));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "geometryInstance", NC_UINT64, 1, instancesDimensionID, 0, 0, NULL,
                                       "The instance in the geometry file to use with each instance in the state file.");
        }
      
      if (!error)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Assumes size_t is 8 bytes when storing as NC_UINT64.
          CkAssert(8 == sizeof(size_t));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          error = NetCDFCreateVariable(*fileID, "parameterInstance", NC_UINT64, 1, instancesDimensionID, 0, 0, NULL,
                                       "The instance in the parameter file to use with each instance in the state file.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "referenceDate", NC_DOUBLE, 1, instancesDimensionID, 0, 0, "days",
                                       "The Julian date to use as time zero for the simulation.");
        }
      
      if (!error)
        {
          error = NetCDFCreateVariable(*fileID, "currentTime", NC_DOUBLE, 1, instancesDimensionID, 0, 0, "seconds",
                                       "Elapsed time since referenceDate.  Can be negative to indicate times before referenceDate.");
        }
      
      if (!error && NULL != meshSurfacewaterDepth)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterDepth", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Surfacewater depth of each mesh element.");
        }
      
      if (!error && NULL != meshSurfacewaterError)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterError", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Surfacewater created or destroyed as the result of error including roundoff error of each mesh element.  Positive "
                                       "means water was created.  Negative means water was destroyed.");
        }
      
      if (!error && NULL != meshGroundwaterHead)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterHead", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Z coordinate of water table of each mesh element.");
        }
      
      if (!error && NULL != meshGroundwaterRecharge)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterRecharge", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Depth of water in groundwater recharge bucket of each mesh element.");
        }
      
      if (!error && NULL != meshGroundwaterError)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterError", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters",
                                       "Groundwater created or destroyed as the result of error including roundoff error of each mesh element.  Positive "
                                       "means water was created.  Negative means water was destroyed.");
        }
      
      if (!error && NULL != meshPrecipitationRate)
        {
          error = NetCDFCreateVariable(*fileID, "meshPrecipitationRate", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters/second",
                                       "Instantaneous precipitation rate of each mesh element as a negative number.");
        }
      
      if (!error && NULL != meshPrecipitationCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshPrecipitationCumulativeShortTerm", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0,
                                       "meters", "Part of cumulative precipitation that has fallen on each mesh element as a negative number.  Two buckets "
                                       "are used to minimize roundoff error.  To find total cumulative precipitation add short term and long term.");
        }
      
      if (!error && NULL != meshPrecipitationCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshPrecipitationCumulativeLongTerm", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0,
                                       "meters", "Part of cumulative precipitation that has fallen on each mesh element as a negative number.  Two buckets "
                                       "are used to minimize roundoff error.  To find total cumulative precipitation add short term and long term.");
        }
      
      if (!error && NULL != meshEvaporationRate)
        {
          error = NetCDFCreateVariable(*fileID, "meshEvaporationRate", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters/second",
                                       "Instantaneous evaporation or condensation rate of each mesh element.  Positive means water removed from the element.  "
                                       "Negative means water added to the element.");
        }
      
      if (!error && NULL != meshEvaporationCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshEvaporationCumulativeShortTerm", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0,
                                       "meters", "Part of cumulative evaporation or condensation of each mesh element.  Positive means water removed from the "
                                       "element.  Negative means water added to the element.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative evaporation or condensation add short term and long term.");
        }
      
      if (!error && NULL != meshEvaporationCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshEvaporationCumulativeLongTerm", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0,
                                       "meters", "Part of cumulative evaporation or condensation of each mesh element.  Positive means water removed from the "
                                       "element.  Negative means water added to the element.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative evaporation or condensation add short term and long term.");
        }
      
      if (!error && NULL != meshTranspirationRate)
        {
          error = NetCDFCreateVariable(*fileID, "meshTranspirationRate", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0, "meters/second",
                                       "Instantaneous transpiration rate of each mesh element as a positive number.");
        }
      
      if (!error && NULL != meshTranspirationCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshTranspirationCumulativeShortTerm", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0,
                                       "meters", "Part of cumulative transpiration of each mesh element as a positive number.  Two buckets are used to "
                                       "minimize roundoff error.  To find total cumulative transpiration add short term and long term.");
        }
      
      if (!error && NULL != meshTranspirationCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshTranspirationCumulativeLongTerm", NC_DOUBLE, 2, instancesDimensionID, meshElementsDimensionID, 0,
                                       "meters", "Part of cumulative transpiration of each mesh element as a positive number.  Two buckets are used to "
                                       "minimize roundoff error.  To find total cumulative transpiration add short term and long term.");
        }
      
      if (!error && NULL != meshEvapoTranspirationState)
        {
          error = NetCDFCreateVariable(*fileID, "meshEvapoTranspirationState", EvapoTranspirationStateStructTypeID, 2, instancesDimensionID,
                                       meshElementsDimensionID, 0, NULL, "Opaque blob containing EvapoTranspirationStateStruct from evapo_transpiration.h");
          
          // FIXME break out interesting values like snow depth.
        }
      
      // FIXME save vadose zone and break out interesting values like soil moisture.
      
      if (!error && NULL != meshSurfacewaterMeshNeighborsExpirationTime)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterMeshNeighborsExpirationTime", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, "seconds", "Value of currentTime when flow rate negotiated with neighbor expires.");
        }
      
      if (!error && NULL != meshSurfacewaterMeshNeighborsFlowRate)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterMeshNeighborsFlowRate", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, "meters^3/second",
                                       "Instantaneous flow rate of surfacewater between a mesh element and its mesh neighbor.  Positive means flow out of the "
                                       "element into the neighbor.  Negative means flow into the element out of the neighbor.");
        }
      
      if (!error && NULL != meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       meshElementsDimensionID, meshMeshNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of surfacewater "
                                       "between a mesh element and its mesh neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       meshElementsDimensionID, meshMeshNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of surfacewater "
                                       "between a mesh element and its mesh neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != meshGroundwaterMeshNeighborsExpirationTime)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterMeshNeighborsExpirationTime", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, "seconds", "Value of currentTime when flow rate negotiated with neighbor expires.");
        }
      
      if (!error && NULL != meshGroundwaterMeshNeighborsFlowRate)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterMeshNeighborsFlowRate", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshMeshNeighborsSizeDimensionID, "meters^3/second",
                                       "Instantaneous flow rate of groundwater between a mesh element and its mesh neighbor.  Positive means flow out of the "
                                       "element into the neighbor.  Negative means flow into the element out of the neighbor.");
        }
      
      if (!error && NULL != meshGroundwaterMeshNeighborsFlowCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterMeshNeighborsFlowCumulativeShortTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       meshElementsDimensionID, meshMeshNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of groundwater "
                                       "between a mesh element and its mesh neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != meshGroundwaterMeshNeighborsFlowCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterMeshNeighborsFlowCumulativeLongTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       meshElementsDimensionID, meshMeshNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of groundwater "
                                       "between a mesh element and its mesh neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != meshSurfacewaterChannelNeighborsExpirationTime)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterChannelNeighborsExpirationTime", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, "seconds",
                                       "Value of currentTime when flow rate negotiated with neighbor expires.");
        }
      
      if (!error && NULL != meshSurfacewaterChannelNeighborsFlowRate)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterChannelNeighborsFlowRate", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, "meters^3/second",
                                       "Instantaneous flow rate of surfacewater between a mesh element and its channel neighbor.  Positive means flow out of "
                                       "the element into the neighbor.  Negative means flow into the element out of the neighbor.");
        }
      
      if (!error && NULL != meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       meshElementsDimensionID, meshChannelNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of surfacewater "
                                       "between a mesh element and its channel neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       meshElementsDimensionID, meshChannelNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of surfacewater "
                                       "between a mesh element and its channel neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != meshGroundwaterChannelNeighborsExpirationTime)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterChannelNeighborsExpirationTime", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, "seconds",
                                       "Value of currentTime when flow rate negotiated with neighbor expires.");
        }
      
      if (!error && NULL != meshGroundwaterChannelNeighborsFlowRate)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterChannelNeighborsFlowRate", NC_DOUBLE, 3, instancesDimensionID, meshElementsDimensionID,
                                       meshChannelNeighborsSizeDimensionID, "meters^3/second",
                                       "Instantaneous flow rate of groundwater between a mesh element and its channel neighbor.  Positive means flow out of "
                                       "the element into the neighbor.  Negative means flow into the element out of the neighbor.");
        }
      
      if (!error && NULL != meshGroundwaterChannelNeighborsFlowCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterChannelNeighborsFlowCumulativeShortTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       meshElementsDimensionID, meshChannelNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of groundwater "
                                       "between a mesh element and its channel neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != meshGroundwaterChannelNeighborsFlowCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "meshGroundwaterChannelNeighborsFlowCumulativeLongTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       meshElementsDimensionID, meshChannelNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of groundwater "
                                       "between a mesh element and its channel neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != channelSurfacewaterDepth)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterDepth", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters",
                                       "Surfacewater depth of each channel element.");
        }
      
      if (!error && NULL != channelSurfacewaterError)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterError", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters^3",
                                       "Surfacewater created or destroyed as the result of error including roundoff error of each channel element.  Positive "
                                       "means water was created.  Negative means water was destroyed.");
        }
      
      if (!error && NULL != channelPrecipitationRate)
        {
          error = NetCDFCreateVariable(*fileID, "channelPrecipitationRate", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters/second",
                                       "Instantaneous precipitation rate of each channel element as a negative number.");
        }
      
      if (!error && NULL != channelPrecipitationCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelPrecipitationCumulativeShortTerm", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0,
                                       "meters^3", "Part of cumulative precipitation that has fallen on each chanel element as a negative number.  Two "
                                       "buckets are used to minimize roundoff error.  To find total cumulative precipitation add short term and long term.");
        }
      
      if (!error && NULL != channelPrecipitationCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelPrecipitationCumulativeLongTerm", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0,
                                       "meters^3", "Part of cumulative precipitation that has fallen on each chanel element as a negative number.  Two "
                                       "buckets are used to minimize roundoff error.  To find total cumulative precipitation add short term and long term.");
        }
      
      if (!error && NULL != channelEvaporationRate)
        {
          error = NetCDFCreateVariable(*fileID, "channelEvaporationRate", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0, "meters/second",
                                       "Instantaneous evaporation or condensation rate of each channel element.  Positive means water removed from the "
                                       "element.  Negative means water added to the element.");
        }
      
      if (!error && NULL != channelEvaporationCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelEvaporationCumulativeShortTerm", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0,
                                       "meters^3", "Part of cumulative evaporation or condensation of each channel element.  Positive means water removed "
                                       "from the element.  Negative means water added to the element.  Two buckets are used to minimize roundoff error.  To "
                                       "find total cumulative evaporation or condensation add short term and long term.");
        }
      
      if (!error && NULL != channelEvaporationCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelEvaporationCumulativeLongTerm", NC_DOUBLE, 2, instancesDimensionID, channelElementsDimensionID, 0,
                                       "meters^3", "Part of cumulative evaporation or condensation of each channel element.  Positive means water removed "
                                       "from the element.  Negative means water added to the element.  Two buckets are used to minimize roundoff error.  To "
                                       "find total cumulative evaporation or condensation add short term and long term.");
        }
      
      if (!error && NULL != channelEvapoTranspirationState)
        {
          error = NetCDFCreateVariable(*fileID, "channelEvapoTranspirationState", EvapoTranspirationStateStructTypeID, 2, instancesDimensionID,
                                       channelElementsDimensionID, 0, NULL, "Opaque blob containing EvapoTranspirationStateStruct from evapo_transpiration.h");
          
          // FIXME break out interesting values like snow depth.
        }
      
      if (!error && NULL != channelSurfacewaterMeshNeighborsExpirationTime)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterMeshNeighborsExpirationTime", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelMeshNeighborsSizeDimensionID, "seconds",
                                       "Value of currentTime when flow rate negotiated with neighbor expires.");
        }
      
      if (!error && NULL != channelSurfacewaterMeshNeighborsFlowRate)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterMeshNeighborsFlowRate", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelMeshNeighborsSizeDimensionID, "meters^3/second",
                                       "Instantaneous flow rate of surfacewater between a channel element and its mesh neighbor.  Positive means flow out of "
                                       "the element into the neighbor.  Negative means flow into the element out of the neighbor.");
        }
      
      if (!error && NULL != channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelMeshNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of surfacewater "
                                       "between a channel element and its mesh neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelMeshNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of surfacewater "
                                       "between a channel element and its mesh neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != channelGroundwaterMeshNeighborsExpirationTime)
        {
          error = NetCDFCreateVariable(*fileID, "channelGroundwaterMeshNeighborsExpirationTime", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelMeshNeighborsSizeDimensionID, "seconds",
                                       "Value of currentTime when flow rate negotiated with neighbor expires.");
        }
      
      if (!error && NULL != channelGroundwaterMeshNeighborsFlowRate)
        {
          error = NetCDFCreateVariable(*fileID, "channelGroundwaterMeshNeighborsFlowRate", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelMeshNeighborsSizeDimensionID, "meters^3/second",
                                       "Instantaneous flow rate of groundwater between a channel element and its mesh neighbor.  Positive means flow out of "
                                       "the element into the neighbor.  Negative means flow into the element out of the neighbor.");
        }
      
      if (!error && NULL != channelGroundwaterMeshNeighborsFlowCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelGroundwaterMeshNeighborsFlowCumulativeShortTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelMeshNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of groundwater "
                                       "between a channel element and its mesh neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != channelGroundwaterMeshNeighborsFlowCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelGroundwaterMeshNeighborsFlowCumulativeLongTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelMeshNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of groundwater "
                                       "between a channel element and its mesh neighbor.  Positive means flow out of the element into the neighbor.  Negative "
                                       "means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff error.  To find total "
                                       "cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != channelSurfacewaterChannelNeighborsExpirationTime)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterChannelNeighborsExpirationTime", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelChannelNeighborsSizeDimensionID, "seconds",
                                       "Value of currentTime when flow rate negotiated with neighbor expires.");
        }
      
      if (!error && NULL != channelSurfacewaterChannelNeighborsFlowRate)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterChannelNeighborsFlowRate", NC_DOUBLE, 3, instancesDimensionID, channelElementsDimensionID,
                                       channelChannelNeighborsSizeDimensionID, "meters^3/second",
                                       "Instantaneous flow rate of surfacewater between a channel element and its channel neighbor.  Positive means flow out "
                                       "of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
        }
      
      if (!error && NULL != channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelChannelNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of "
                                       "surfacewater between a channel element and its channel neighbor.  Positive means flow out of the element into the "
                                       "neighbor.  Negative means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff "
                                       "error.  To find total cumulative flow add short term and long term.");
        }
      
      if (!error && NULL != channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm)
        {
          error = NetCDFCreateVariable(*fileID, "channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm", NC_DOUBLE, 3, instancesDimensionID,
                                       channelElementsDimensionID, channelChannelNeighborsSizeDimensionID, "meters^3", "Part of cumulative flow of "
                                       "surfacewater between a channel element and its channel neighbor.  Positive means flow out of the element into the "
                                       "neighbor.  Negative means flow into the element out of the neighbor.  Two buckets are used to minimize roundoff "
                                       "error.  To find total cumulative flow add short term and long term.");
        }
    }
  // FIXME else if not error the file was not created we could check that the dimensions and variables exist and the dimensions are the right size.
  
  if (error && fileOpen)
    {
      ncErrorCode = nc_close(*fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteState: unable to close NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFCreateOrOpenForWriteDisplay(int* fileID)
{
  bool error    = false;     // Error flag.
  int  ncErrorCode;          // Return value of NetCDF functions.
  bool created;              // Whether the file is being created.
  bool fileOpen = false;     // Whether the file is open.
  int  instancesDimensionID; // ID of dimension in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != fileID))
    {
      CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteDisplay: fileID must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      error = NetCDFCreateOrOpenForWrite(ADHydro::adhydroOutputDisplayFilePath.c_str(), fileID, &created);
      
      if (!error)
        {
          fileOpen = true;
        }
    }
  
  // If the file was created we also need to create all of the dimensions and variables.
  if (!error && created)
    {
      // Create display dimensions.
      if (!error)
        {
          error = NetCDFCreateDimension(*fileID, "instances", NC_UNLIMITED, &instancesDimensionID);
        }
      
      // FIXME decide what should go into the display file
      
      // Create display variables.
      // FIXME decide what should go into the display file
    }
  // FIXME else if not error the file was not created we could check that the dimensions and variables exist and the dimensions are the right size.
  
  if (error && fileOpen)
    {
      ncErrorCode = nc_close(*fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFCreateOrOpenForWriteDisplay: unable to close NetCDF display file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFCreateDimension(int fileID, const char* dimensionName, size_t dimensionSize, int* dimensionID)
{
  bool error = false; // Error flag.
  int  ncErrorCode;   // Return value of NetCDF functions.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != dimensionName))
    {
      CkError("ERROR in FileManager::NetCDFCreateDimension: dimensionName must not be null.\n");
      error = true;
    }
  
  if (!(NULL != dimensionID))
    {
      CkError("ERROR in FileManager::NetCDFCreateDimension: dimensionID must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, dimensionName, dimensionSize, dimensionID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFCreateDimension: unable to create dimension %s in NetCDF file.  NetCDF error message: %s.\n", dimensionName,
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFReadDimensionSize(int fileID, const char* dimensionName, size_t* dimensionSize)
{
  bool error = false; // Error flag.
  int  ncErrorCode;   // Return value of NetCDF functions.
  int  dimensionID;   // ID of dimension in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != dimensionName))
    {
      CkError("ERROR in FileManager::NetCDFReadDimensionSize: dimensionName must not be null.\n");
      error = true;
    }
  
  if (!(NULL != dimensionSize))
    {
      CkError("ERROR in FileManager::NetCDFReadDimensionSize: dimensionSize must not be null.\n");
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
          CkError("ERROR in FileManager::NetCDFReadDimensionSize: unable to get dimension %s in NetCDF file.  NetCDF error message: %s.\n", dimensionName,
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
          CkError("ERROR in FileManager::NetCDFReadDimensionSize: unable to get length of dimension %s in NetCDF file.  NetCDF error message: %s.\n",
                  dimensionName, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFCreateVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID0, int dimensionID1,
                                       int dimensionID2, const char* units, const char* comment)
{
  bool error = false;                 // Error flag.
  int  ncErrorCode;                   // Return value of NetCDF functions.
  int  dimensionIDs[NC_MAX_VAR_DIMS]; // For passing dimension IDs.
  int  variableID;                    // For getting variable ID.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != variableName))
    {
      CkError("ERROR in FileManager::NetCDFCreateVariable: variableName must not be null.\n");
      error = true;
    }
  
  if (!(1 <= numberOfDimensions && 3 >= numberOfDimensions))
    {
      CkError("ERROR in FileManager::NetCDFCreateVariable: numberOfDimensions must be greater than or equal to one and less than or equal to three.\n");
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
          CkError("ERROR in FileManager::NetCDFCreateVariable: unable to create variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
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
          CkError("ERROR in FileManager::NetCDFCreateVariable: unable to create attribute units of variable %s in NetCDF file.  NetCDF error message: %s.\n",
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
          CkError("ERROR in FileManager::NetCDFCreateVariable: unable to create attribute comment of variable %s in NetCDF file.  NetCDF error message: %s.\n",
                  variableName, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

template <typename T> bool FileManager::NetCDFReadVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart,
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != variableName))
    {
      CkError("ERROR in FileManager::NetCDFReadVariable: variableName must not be null.\n");
      error = true;
    }
  
  if (!(1 <= numberOfNodesElements))
    {
      CkError("ERROR in FileManager::NetCDFReadVariable: numberOfNodesElements must be greater than or equal to one.\n");
      error = true;
    }
  
  if (!(1 <= memoryDimension))
    {
      CkError("ERROR in FileManager::NetCDFReadVariable: memoryDimension must be greater than or equal to one.\n");
      error = true;
    }
  
  if (!(NULL != variable))
    {
      CkError("ERROR in FileManager::NetCDFReadVariable: variable must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  // fileDimenison must be less than or equal to memoryDimension.  Otherwise there is not enough room to read all of the data and it is an error.
  if (!(1 <= fileDimension && fileDimension <= memoryDimension))
    {
      CkError("ERROR in FileManager::NetCDFReadVariable: fileDimension must be greater than or equal to one and less than or equal to memoryDimension for "
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
              CkError("ERROR in FileManager::NetCDFReadVariable: unable to get variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
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
              CkError("ERROR in FileManager::NetCDFReadVariable: unable to read variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
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

bool FileManager::NetCDFReadInstances()
{
  bool    error    = false;   // Error flag.
  int     ncErrorCode;        // Return value of NetCDF functions.
  int     fileID;             // ID of NetCDF file.
  bool    fileOpen = false;   // Whether fileID refers to an open file.
  size_t  stateInstancesSize; // Number of instances in state file.
  size_t* size_tVariable;     // For passing the address of a pointer to a variable.
  
  // Open the state file.
  error = NetCDFOpenForRead(ADHydro::adhydroInputStateFilePath.c_str(), &fileID);

  if (!error)
    {
      fileOpen = true;
    }
  
  // Get the number of instances in the state file.
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "instances", &stateInstancesSize);
    }
  
  // Select the instance to use.
  if (!error)
    {
      if (0 < stateInstancesSize)
        {
          if ((size_t)-1 == stateInstance)
            {
              // If the user specified loading instance -1 then use the last instance in the file.
              stateInstance = stateInstancesSize - 1;
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else if (!(stateInstance < stateInstancesSize))
            {
              CkError("ERROR in FileManager::NetCDFReadInstances: stateInstance must be less than stateInstancesSize.\n");
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          // We're not creating a new instance so it's an error if there's not an existing one.
          CkError("ERROR in FileManager::NetCDFReadInstances: no instances in NetCDF state file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Read geometry and parameter instances.
  if (!error)
    {
      size_tVariable = &geometryInstance;
      error          = NetCDFReadVariable(fileID, "geometryInstance", stateInstance, 0, 1, 1, 1, true, (size_t)0, true, &size_tVariable);
    }
  
  if (!error)
    {
      size_tVariable = &parameterInstance;
      error          = NetCDFReadVariable(fileID, "parameterInstance", stateInstance, 0, 1, 1, 1, true, (size_t)0, true, &size_tVariable);
    }
  
  // Close the state file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFReadInstances: unable to close NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFReadGeometry()
{
  bool    error    = false;            // Error flag.
  int     ncErrorCode;                 // Return value of NetCDF functions.
  int     ii;                          // Loop counter.
  int     fileID;                      // ID of NetCDF file.
  bool    fileOpen = false;            // Whether fileID refers to an open file.
  size_t  meshMeshNeighborsSize;       // size of the fixed dimension in the file.
  size_t  meshChannelNeighborsSize;    // size of the fixed dimension in the file.
  size_t  channelVerticesSize;         // size of the fixed dimension in the file.
  size_t  channelVerticesXDMFSize;     // size of the fixed dimension in the file.
  size_t  channelMeshNeighborsSize;    // size of the fixed dimension in the file.
  size_t  channelChannelNeighborsSize; // size of the fixed dimension in the file.
  int*    intVariable;                 // For passing the address of a pointer to a variable.
  double* doubleVariable;              // For passing the address of a pointer to a variable.
  
  // Open the geometry file.
  error = NetCDFOpenForRead(ADHydro::adhydroInputGeometryFilePath.c_str(), &fileID);

  if (!error)
    {
      fileOpen = true;
    }
  
  // Get the size of the fixed dimensions in the geometry file.
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "meshMeshNeighborsSize", &meshMeshNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "meshChannelNeighborsSize", &meshChannelNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "channelVerticesSize", &channelVerticesSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "channelVerticesXDMFSize", &channelVerticesXDMFSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "channelMeshNeighborsSize", &channelMeshNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "channelChannelNeighborsSize", &channelChannelNeighborsSize);
    }
  
  // Read geometry variables.
  if (!error && isnan(ADHydro::centralMeridian))
    {
      doubleVariable = &ADHydro::centralMeridian;
      error          = NetCDFReadVariable(fileID, "centralMeridian", geometryInstance, 0, 1, 1, 1, true, 0.0, true, &doubleVariable);
      
      if (!error && !(-M_PI * 2.0 <= ADHydro::centralMeridian && M_PI * 2.0 >= ADHydro::centralMeridian))
        {
          CkError("ERROR in FileManager::NetCDFReadGeometry: centralMeridian must be greater than or equal to negative two PI and less than or equal to two "
                  "PI.\n");
          error = true;
        }
    }
  
  if (!error && isnan(ADHydro::falseEasting))
    {
      doubleVariable = &ADHydro::falseEasting;
      error          = NetCDFReadVariable(fileID, "falseEasting", geometryInstance, 0, 1, 1, 1, true, 0.0, true, &doubleVariable);
    }
  
  if (!error && isnan(ADHydro::falseNorthing))
    {
      doubleVariable = &ADHydro::falseNorthing;
      error          = NetCDFReadVariable(fileID, "falseNorthing", geometryInstance, 0, 1, 1, 1, true, 0.0, true, &doubleVariable);
    }
  
  if (!error)
    {
      intVariable = &globalNumberOfMeshNodes;
      error       = NetCDFReadVariable(fileID, "numberOfMeshNodes", geometryInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
    }
  
  if (!error)
    {
      intVariable = &globalNumberOfMeshElements;
      error       = NetCDFReadVariable(fileID, "numberOfMeshElements", geometryInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
    }
  
  if (!error)
    {
      intVariable = &globalNumberOfChannelNodes;
      error       = NetCDFReadVariable(fileID, "numberOfChannelNodes", geometryInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
    }
  
  if (!error)
    {
      intVariable = &globalNumberOfChannelElements;
      error       = NetCDFReadVariable(fileID, "numberOfChannelElements", geometryInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
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
          error = NetCDFReadVariable(fileID, "meshNodeX", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, 1, true, 0.0, false, &meshNodeX);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshNodeY", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, 1, true, 0.0, false, &meshNodeY);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshNodeZSurface", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, 1, true, 0.0, false,
                                     &meshNodeZSurface);
        }
    } // End if (0 < localNumberOfMeshNodes).
  
  if (0 < localNumberOfMeshElements)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementVertices", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MESH_ELEMENT_MESH_NEIGHBORS_SIZE, true, 0, false, (int**)&meshElementVertices);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshVertexX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MESH_ELEMENT_MESH_NEIGHBORS_SIZE, true, 0.0, false, (double**)&meshVertexX);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshVertexY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MESH_ELEMENT_MESH_NEIGHBORS_SIZE, true, 0.0, false, (double**)&meshVertexY);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshVertexZSurface", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MESH_ELEMENT_MESH_NEIGHBORS_SIZE, true, 0.0, false, (double**)&meshVertexZSurface);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementX);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementY);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementZSurface", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementZSurface);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementSoilDepth", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0,
                                     false, &meshElementSoilDepth);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementLayerZBottom", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementLayerZBottom);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementArea", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementArea);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementSlopeX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementSlopeX);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshElementSlopeY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshElementSlopeY);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshLatitude", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshLatitude);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshLongitude", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshLongitude);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshMeshNeighbors", geometryInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, (int)NOFLOW, false, (int**)&meshMeshNeighbors);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshMeshNeighborsChannelEdge", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, false, false, (bool**)&meshMeshNeighborsChannelEdge);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshMeshNeighborsEdgeLength", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 1.0, false, (double**)&meshMeshNeighborsEdgeLength);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshMeshNeighborsEdgeNormalX", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 1.0, false, (double**)&meshMeshNeighborsEdgeNormalX);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshMeshNeighborsEdgeNormalY", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false, (double**)&meshMeshNeighborsEdgeNormalY);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterMeshNeighborsConnection", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, false, false,
                                     (bool**)&meshSurfacewaterMeshNeighborsConnection);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterMeshNeighborsConnection", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, false, false,
                                     (bool**)&meshGroundwaterMeshNeighborsConnection);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshChannelNeighbors", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, (int)NOFLOW, false, (int**)&meshChannelNeighbors);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshChannelNeighborsEdgeLength", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 1.0, false,
                                     (double**)&meshChannelNeighborsEdgeLength);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterChannelNeighborsConnection", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, false, false,
                                     (bool**)&meshSurfacewaterChannelNeighborsConnection);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterChannelNeighborsConnection", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, false, false,
                                     (bool**)&meshGroundwaterChannelNeighborsConnection);
        }
    } // End if (0 < localNumberOfMeshElements).
  
  if (0 < localNumberOfChannelNodes)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelNodeX", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, 1, true, 0.0, false,
                                     &channelNodeX);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelNodeY", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, 1, true, 0.0, false,
                                     &channelNodeY);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelNodeZBank", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, 1, true, 0.0, false,
                                     &channelNodeZBank);
        }
    } // End if (0 < localNumberOfChannelNodes).
  
  if (0 < localNumberOfChannelElements)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelElementVertices", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelVerticesXDMFSize, XDMF_SIZE, true, 0, false, (int**)&channelElementVertices);

          // If CHANNEL_ELEMENT_VERTICES_SIZE is not the same value as in the file we need to reset the number of vertices in the XDMF metadata.
          if (channelVerticesSize != CHANNEL_ELEMENT_VERTICES_SIZE)
            {
              for (ii = 0; ii < localNumberOfChannelElements; ii++)
                {
                  channelElementVertices[ii][1] = CHANNEL_ELEMENT_VERTICES_SIZE;
                }
            }
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelVertexX", geometryInstance, localChannelElementStart, localNumberOfChannelElements, channelVerticesSize,
                                     CHANNEL_ELEMENT_VERTICES_SIZE, true, 0.0, false, (double**)&channelVertexX);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelVertexY", geometryInstance, localChannelElementStart, localNumberOfChannelElements, channelVerticesSize,
                                     CHANNEL_ELEMENT_VERTICES_SIZE, true, 0.0, false, (double**)&channelVertexY);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelVertexZBank", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelVerticesSize, CHANNEL_ELEMENT_VERTICES_SIZE, true, 0.0, false, (double**)&channelVertexZBank);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelElementX", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementX);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelElementY", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementY);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelElementZBank", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementZBank);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelElementBankFullDepth", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1,
                                     true, 0.0, false, &channelElementBankFullDepth);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelElementZBed", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementZBed);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelElementLength", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelElementLength);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelLatitude", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelLatitude);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelLongitude", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelLongitude);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelMeshNeighbors", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, (int)NOFLOW, false, (int**)&channelMeshNeighbors);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelMeshNeighborsEdgeLength", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, 1.0, false,
                                     (double**)&channelMeshNeighborsEdgeLength);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterMeshNeighborsConnection", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, false, false,
                                     (bool**)&channelSurfacewaterMeshNeighborsConnection);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelGroundwaterMeshNeighborsConnection", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, false, false,
                                     (bool**)&channelGroundwaterMeshNeighborsConnection);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelChannelNeighbors", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, (int)NOFLOW, false,
                                     (int**)&channelChannelNeighbors);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelChannelNeighborsDownstream", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, false, false,
                                     (bool**)&channelChannelNeighborsDownstream);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterChannelNeighborsConnection", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, false, false,
                                     (bool**)&channelSurfacewaterChannelNeighborsConnection);
        }
    } // End if (0 < localNumberOfChannelElements).
  
  // Close the geometry file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFReadGeometry: unable to close NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFReadParameter()
{
  bool   error    = false;            // Error flag.
  int    ncErrorCode;                 // Return value of NetCDF functions.
  int    fileID;                      // ID of NetCDF file.
  bool   fileOpen = false;            // Whether fileID refers to an open file.
  size_t meshMeshNeighborsSize;       // size of the fixed dimension in the file.
  size_t meshChannelNeighborsSize;    // size of the fixed dimension in the file.
  size_t channelMeshNeighborsSize;    // size of the fixed dimension in the file.
  size_t channelChannelNeighborsSize; // size of the fixed dimension in the file.
  int*   intVariable;                 // For passing the address of a pointer to a variable.
  
  // Open the parameter file.
  error = NetCDFOpenForRead(ADHydro::adhydroInputParameterFilePath.c_str(), &fileID);

  if (!error)
    {
      fileOpen = true;
    }
  
  // Get the size of the fixed dimensions in the parameter file.
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "meshMeshNeighborsSize", &meshMeshNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "meshChannelNeighborsSize", &meshChannelNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "channelMeshNeighborsSize", &channelMeshNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "channelChannelNeighborsSize", &channelChannelNeighborsSize);
    }
  
  // Read parameter variables.
  if (!error)
    {
      intVariable = &globalNumberOfRegions;
      error       = NetCDFReadVariable(fileID, "numberOfRegions", parameterInstance, 0, 1, 1, 1, true, 0, true, &intVariable);
    }
  
  if (!error)
    {
      localStartAndNumber(&localRegionStart, &localNumberOfRegions, globalNumberOfRegions);
    }
  
  if (0 < localNumberOfRegions)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "regionNumberOfMeshElements", parameterInstance, localRegionStart, localNumberOfRegions, 1, 1, true, 0, false,
                                     &regionNumberOfMeshElements);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "regionNumberOfChannelElements", parameterInstance, localRegionStart, localNumberOfRegions, 1, 1, true, 0, false,
                                     &regionNumberOfChannelElements);
        }
    }
  
  if (0 < localNumberOfMeshElements)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshRegion", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0, false,
                                     &meshRegion);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshCatchment", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0, false,
                                     &meshCatchment);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshVegetationType", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0, false,
                                     &meshVegetationType);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSoilType", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0, false,
                                     &meshSoilType);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshAlluvium", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, false, false, false,
                                     &meshAlluvium);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshManningsN", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshManningsN);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshConductivity", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshConductivity);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshPorosity", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshPorosity);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterMethod", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, InfiltrationAndGroundwater::NO_AQUIFER, false,
                                     &meshGroundwaterMethod);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshMeshNeighborsRegion", parameterInstance, localMeshElementStart, localNumberOfMeshElements, meshMeshNeighborsSize,
                                     MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, (int)NOFLOW, false, (int**)&meshMeshNeighborsRegion);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshChannelNeighborsRegion", parameterInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, (int)NOFLOW, false, (int**)&meshChannelNeighborsRegion);
        }
    } // End if (0 < localNumberOfMeshElements).
  
  if (0 < localNumberOfChannelElements)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelRegion", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0, false,
                                     &channelRegion);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelChannelType", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     NOT_USED, false, &channelChannelType);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelReachCode", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     (long long)0, false, &channelReachCode);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelBaseWidth", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelBaseWidth);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSideSlope", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelSideSlope);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelBedConductivity", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     0.0, false, &channelBedConductivity);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelBedThickness", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelBedThickness);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelManningsN", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelManningsN);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelMeshNeighborsRegion", parameterInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, (int)NOFLOW, false, (int**)&channelMeshNeighborsRegion);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelChannelNeighborsRegion", parameterInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, (int)NOFLOW, false,
                                     (int**)&channelChannelNeighborsRegion);
        }
    } // End if (0 < localNumberOfChannelElements).
  
  // Close the parameter file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFReadParameter: unable to close NetCDF parameter file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFReadState()
{
  bool                          error                  = false;     // Error flag.
  int                           ncErrorCode;                        // Return value of NetCDF functions.
  int                           fileID;                             // ID of NetCDF file.
  bool                          fileOpen               = false;     // Whether fileID refers to an open file.
  size_t                        meshMeshNeighborsSize;              // size of the fixed dimension in the file.
  size_t                        meshChannelNeighborsSize;           // size of the fixed dimension in the file.
  size_t                        channelMeshNeighborsSize;           // size of the fixed dimension in the file.
  size_t                        channelChannelNeighborsSize;        // size of the fixed dimension in the file.
  double*                       doubleVariable;                     // For passing the address of a pointer to a variable.
  bool                          useCurrentTimeFromFile = false;     // Whether currentTime was read from the file.
  EvapoTranspirationStateStruct dummyEvapoTranspirationStateStruct; // Used to satisfy template parameter, but actually unused.
  
  // Open the state file.
  error = NetCDFOpenForRead(ADHydro::adhydroInputStateFilePath.c_str(), &fileID);

  if (!error)
    {
      fileOpen = true;
    }
  
  // Get the size of the fixed dimensions in the state file.
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "meshMeshNeighborsSize", &meshMeshNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "meshChannelNeighborsSize", &meshChannelNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "channelMeshNeighborsSize", &channelMeshNeighborsSize);
    }
  
  if (!error)
    {
      error = NetCDFReadDimensionSize(fileID, "channelChannelNeighborsSize", &channelChannelNeighborsSize);
    }
  
  // Read state variables.
  if (!error && isnan(ADHydro::referenceDate))
    {
      doubleVariable = &ADHydro::referenceDate;
      error          = NetCDFReadVariable(fileID, "referenceDate", stateInstance, 0, 1, 1, 1, true, 0.0, true, &doubleVariable);
    }
  
  if (!error && isnan(ADHydro::currentTime))
    {
      doubleVariable         = &ADHydro::currentTime;
      error                  = NetCDFReadVariable(fileID, "currentTime", stateInstance, 0, 1, 1, 1, true, 0.0, true, &doubleVariable);
      useCurrentTimeFromFile = true;
    }
  
  if (0 < localNumberOfMeshElements)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterDepth", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshSurfacewaterDepth);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterError", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshSurfacewaterError);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterHead", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshGroundwaterHead);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterRecharge", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshGroundwaterRecharge);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterError", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshGroundwaterError);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshPrecipitationRate", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshPrecipitationRate);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshPrecipitationCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true,
                                     0.0, false, &meshPrecipitationCumulativeShortTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshPrecipitationCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true,
                                     0.0, false, &meshPrecipitationCumulativeLongTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshEvaporationRate", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshEvaporationRate);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshEvaporationCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0,
                                     false, &meshEvaporationCumulativeShortTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshEvaporationCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0,
                                     false, &meshEvaporationCumulativeLongTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshTranspirationRate", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0, false,
                                     &meshTranspirationRate);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshTranspirationCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true,
                                     0.0, false, &meshTranspirationCumulativeShortTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshTranspirationCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true,
                                     0.0, false, &meshTranspirationCumulativeLongTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshEvapoTranspirationState", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true,
                                     dummyEvapoTranspirationStateStruct, false, &meshEvapoTranspirationState);
        }
      
      // FIXME read in vadose zone

      if (!error && useCurrentTimeFromFile)
        {
          // If we are using the currentTime value from the file then the expiration times in the file are valid so read them in.  Otherwise, do not read them
          // in and they will default to currentTime in calculateDerivedValues.
          error = NetCDFReadVariable(fileID, "meshSurfacewaterMeshNeighborsExpirationTime", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, ADHydro::currentTime, false,
                                     (double**)&meshSurfacewaterMeshNeighborsExpirationTime);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterMeshNeighborsFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshSurfacewaterMeshNeighborsFlowRate);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm);
        }

      if (!error && useCurrentTimeFromFile)
        {
          // If we are using the currentTime value from the file then the expiration times in the file are valid so read them in.  Otherwise, do not read them
          // in and they will default to currentTime in calculateDerivedValues.
          error = NetCDFReadVariable(fileID, "meshGroundwaterMeshNeighborsExpirationTime", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, ADHydro::currentTime, false,
                                     (double**)&meshGroundwaterMeshNeighborsExpirationTime);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterMeshNeighborsFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshGroundwaterMeshNeighborsFlowRate);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterMeshNeighborsFlowCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshGroundwaterMeshNeighborsFlowCumulativeShortTerm);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterMeshNeighborsFlowCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshMeshNeighborsSize, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshGroundwaterMeshNeighborsFlowCumulativeLongTerm);
        }

      if (!error && useCurrentTimeFromFile)
        {
          // If we are using the currentTime value from the file then the expiration times in the file are valid so read them in.  Otherwise, do not read them
          // in and they will default to currentTime in calculateDerivedValues.
          error = NetCDFReadVariable(fileID, "meshSurfacewaterChannelNeighborsExpirationTime", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, ADHydro::currentTime, false,
                                     (double**)&meshSurfacewaterChannelNeighborsExpirationTime);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterChannelNeighborsFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshSurfacewaterChannelNeighborsFlowRate);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm);
        }

      if (!error && useCurrentTimeFromFile)
        {
          // If we are using the currentTime value from the file then the expiration times in the file are valid so read them in.  Otherwise, do not read them
          // in and they will default to currentTime in calculateDerivedValues.
          error = NetCDFReadVariable(fileID, "meshGroundwaterChannelNeighborsExpirationTime", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, ADHydro::currentTime, false,
                                     (double**)&meshGroundwaterChannelNeighborsExpirationTime);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterChannelNeighborsFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshGroundwaterChannelNeighborsFlowRate);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterChannelNeighborsFlowCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshGroundwaterChannelNeighborsFlowCumulativeShortTerm);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "meshGroundwaterChannelNeighborsFlowCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                     meshChannelNeighborsSize, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&meshGroundwaterChannelNeighborsFlowCumulativeLongTerm);
        }
    } // End if (0 < localNumberOfMeshElements).
  
  if (0 < localNumberOfChannelElements)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterDepth", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1,
                                     true, 0.0, false, &channelSurfacewaterDepth);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterError", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1,
                                     true, 0.0, false, &channelSurfacewaterError);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelPrecipitationRate", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0, false,
                                     &channelPrecipitationRate);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelPrecipitationCumulativeShortTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     0.0, false, &channelPrecipitationCumulativeShortTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelPrecipitationCumulativeLongTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     0.0, false, &channelPrecipitationCumulativeLongTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelEvaporationRate", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0, false,
                                     &channelEvaporationRate);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelEvaporationCumulativeShortTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelEvaporationCumulativeShortTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelEvaporationCumulativeLongTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0,
                                     false, &channelEvaporationCumulativeLongTerm);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelEvapoTranspirationState", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true,
                                     dummyEvapoTranspirationStateStruct, false, &channelEvapoTranspirationState);
        }

      if (!error && useCurrentTimeFromFile)
        {
          // If we are using the currentTime value from the file then the expiration times in the file are valid so read them in.  Otherwise, do not read them
          // in and they will default to currentTime in calculateDerivedValues.
          error = NetCDFReadVariable(fileID, "channelSurfacewaterMeshNeighborsExpirationTime", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, ADHydro::currentTime, false,
                                     (double**)&channelSurfacewaterMeshNeighborsExpirationTime);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterMeshNeighborsFlowRate", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelSurfacewaterMeshNeighborsFlowRate);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm);
        }

      if (!error && useCurrentTimeFromFile)
        {
          // If we are using the currentTime value from the file then the expiration times in the file are valid so read them in.  Otherwise, do not read them
          // in and they will default to currentTime in calculateDerivedValues.
          error = NetCDFReadVariable(fileID, "channelGroundwaterMeshNeighborsExpirationTime", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, ADHydro::currentTime, false,
                                     (double**)&channelGroundwaterMeshNeighborsExpirationTime);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelGroundwaterMeshNeighborsFlowRate", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelGroundwaterMeshNeighborsFlowRate);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelGroundwaterMeshNeighborsFlowCumulativeShortTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelGroundwaterMeshNeighborsFlowCumulativeShortTerm);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelGroundwaterMeshNeighborsFlowCumulativeLongTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelMeshNeighborsSize, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelGroundwaterMeshNeighborsFlowCumulativeLongTerm);
        }

      if (!error && useCurrentTimeFromFile)
        {
          // If we are using the currentTime value from the file then the expiration times in the file are valid so read them in.  Otherwise, do not read them
          // in and they will default to currentTime in calculateDerivedValues.
          error = NetCDFReadVariable(fileID, "channelSurfacewaterChannelNeighborsExpirationTime", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, ADHydro::currentTime, false,
                                     (double**)&channelSurfacewaterChannelNeighborsExpirationTime);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterChannelNeighborsFlowRate", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelSurfacewaterChannelNeighborsFlowRate);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements,
                                     channelChannelNeighborsSize, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, false, 0.0, false,
                                     (double**)&channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm);
        }
    } // End if (0 < localNumberOfChannelElements).
  
  // Close the state file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFReadState: unable to close NetCDF state file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFWriteVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart, size_t numberOfNodesElements,
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
      CkError("ERROR in FileManager::NetCDFWriteVariable: variableName must not be null.\n");
      error = true;
    }
  
  if (!(1 <= numberOfNodesElements))
    {
      CkError("ERROR in FileManager::NetCDFWriteVariable: numberOfNodesElements must be greater than or equal to one.\n");
      error = true;
    }
  
  if (!(1 <= memoryDimension))
    {
      CkError("ERROR in FileManager::NetCDFWriteVariable: memoryDimension must be greater than or equal to one.\n");
      error = true;
    }
  
  if (!(NULL != variable))
    {
      CkError("ERROR in FileManager::NetCDFWriteVariable: variable must not be null.\n");
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
          CkError("ERROR in FileManager::NetCDFWriteVariable: unable to get variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
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
          CkError("ERROR in FileManager::NetCDFWriteVariable: unable to set collective access for variable %s in NetCDF file.  NetCDF error message: %s.\n",
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
          CkError("ERROR in FileManager::NetCDFWriteVariable: unable to write variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName,
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFWriteGeometry()
{
  bool error    = false; // Error flag.
  int  ncErrorCode;      // Return value of NetCDF functions.
  int  fileID;           // ID of NetCDF file.
  bool fileOpen = false; // Whether fileID refers to an open file.
  
  // Open the geometry file.
  error = NetCDFCreateOrOpenForWriteGeometry(&fileID);

  if (!error)
    {
      fileOpen = true;
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "centralMeridian", geometryInstance, 0, 1, 1, &ADHydro::centralMeridian);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "falseEasting", geometryInstance, 0, 1, 1, &ADHydro::falseEasting);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "falseNorthing", geometryInstance, 0, 1, 1, &ADHydro::falseNorthing);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "numberOfMeshNodes", geometryInstance, 0, 1, 1, &globalNumberOfMeshNodes);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "numberOfMeshElements", geometryInstance, 0, 1, 1, &globalNumberOfMeshElements);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "numberOfChannelNodes", geometryInstance, 0, 1, 1, &globalNumberOfChannelNodes);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "numberOfChannelElements", geometryInstance, 0, 1, 1, &globalNumberOfChannelElements);
    }
  
  if (!error && NULL != meshNodeX)
    {
      error = NetCDFWriteVariable(fileID, "meshNodeX", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, meshNodeX);
    }
  
  if (!error && NULL != meshNodeY)
    {
      error = NetCDFWriteVariable(fileID, "meshNodeY", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, meshNodeY);
    }
  
  if (!error && NULL != meshNodeZSurface)
    {
      error = NetCDFWriteVariable(fileID, "meshNodeZSurface", geometryInstance, localMeshNodeStart, localNumberOfMeshNodes, 1, meshNodeZSurface);
    }
  
  if (!error && NULL != meshElementVertices)
    {
      error = NetCDFWriteVariable(fileID, "meshElementVertices", geometryInstance, localMeshElementStart, localNumberOfMeshElements, MESH_ELEMENT_MESH_NEIGHBORS_SIZE,
                                  meshElementVertices);
    }
  
  if (!error && NULL != meshVertexX)
    {
      error = NetCDFWriteVariable(fileID, "meshVertexX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, MESH_ELEMENT_MESH_NEIGHBORS_SIZE,
                                  meshVertexX);
    }
  
  if (!error && NULL != meshVertexY)
    {
      error = NetCDFWriteVariable(fileID, "meshVertexY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, MESH_ELEMENT_MESH_NEIGHBORS_SIZE,
                                  meshVertexY);
    }
  
  if (!error && NULL != meshVertexZSurface)
    {
      error = NetCDFWriteVariable(fileID, "meshVertexZSurface", geometryInstance, localMeshElementStart, localNumberOfMeshElements, MESH_ELEMENT_MESH_NEIGHBORS_SIZE,
                                  meshVertexZSurface);
    }
  
  if (!error && NULL != meshElementX)
    {
      error = NetCDFWriteVariable(fileID, "meshElementX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementX);
    }
  
  if (!error && NULL != meshElementY)
    {
      error = NetCDFWriteVariable(fileID, "meshElementY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementY);
    }
  
  if (!error && NULL != meshElementZSurface)
    {
      error = NetCDFWriteVariable(fileID, "meshElementZSurface", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementZSurface);
    }
  
  if (!error && NULL != meshElementSoilDepth)
    {
      error = NetCDFWriteVariable(fileID, "meshElementSoilDepth", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementSoilDepth);
    }
  
  if (!error && NULL != meshElementLayerZBottom)
    {
      error = NetCDFWriteVariable(fileID, "meshElementLayerZBottom", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementLayerZBottom);
    }
  
  if (!error && NULL != meshElementArea)
    {
      error = NetCDFWriteVariable(fileID, "meshElementArea", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementArea);
    }
  
  if (!error && NULL != meshElementSlopeX)
    {
      error = NetCDFWriteVariable(fileID, "meshElementSlopeX", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementSlopeX);
    }
  
  if (!error && NULL != meshElementSlopeY)
    {
      error = NetCDFWriteVariable(fileID, "meshElementSlopeY", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshElementSlopeY);
    }
  
  if (!error && NULL != meshLatitude)
    {
      error = NetCDFWriteVariable(fileID, "meshLatitude", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshLatitude);
    }
  
  if (!error && NULL != meshLongitude)
    {
      error = NetCDFWriteVariable(fileID, "meshLongitude", geometryInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshLongitude);
    }
  
  if (!error && NULL != meshMeshNeighbors)
    {
      error = NetCDFWriteVariable(fileID, "meshMeshNeighbors", geometryInstance, localMeshElementStart, localNumberOfMeshElements, MESH_ELEMENT_MESH_NEIGHBORS_SIZE,
                                  meshMeshNeighbors);
    }
  
  if (!error && NULL != meshMeshNeighborsChannelEdge)
    {
      error = NetCDFWriteVariable(fileID, "meshMeshNeighborsChannelEdge", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshMeshNeighborsChannelEdge);
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeLength)
    {
      error = NetCDFWriteVariable(fileID, "meshMeshNeighborsEdgeLength", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshMeshNeighborsEdgeLength);
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalX)
    {
      error = NetCDFWriteVariable(fileID, "meshMeshNeighborsEdgeNormalX", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshMeshNeighborsEdgeNormalX);
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalY)
    {
      error = NetCDFWriteVariable(fileID, "meshMeshNeighborsEdgeNormalY", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshMeshNeighborsEdgeNormalY);
    }
  
  if (!error && NULL != meshSurfacewaterMeshNeighborsConnection)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterMeshNeighborsConnection", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshSurfacewaterMeshNeighborsConnection);
    }
  
  if (!error && NULL != meshGroundwaterMeshNeighborsConnection)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterMeshNeighborsConnection", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshGroundwaterMeshNeighborsConnection);
    }
  
  if (!error && NULL != meshChannelNeighbors)
    {
      error = NetCDFWriteVariable(fileID, "meshChannelNeighbors", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshChannelNeighbors);
    }
  
  if (!error && NULL != meshChannelNeighborsEdgeLength)
    {
      error = NetCDFWriteVariable(fileID, "meshChannelNeighborsEdgeLength", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshChannelNeighborsEdgeLength);
    }
  
  if (!error && NULL != meshSurfacewaterChannelNeighborsConnection)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterChannelNeighborsConnection", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshSurfacewaterChannelNeighborsConnection);
    }
  
  if (!error && NULL != meshGroundwaterChannelNeighborsConnection)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterChannelNeighborsConnection", geometryInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshGroundwaterChannelNeighborsConnection);
    }
  
  if (!error && NULL != channelNodeX)
    {
      error = NetCDFWriteVariable(fileID, "channelNodeX", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, channelNodeX);
    }
  
  if (!error && NULL != channelNodeY)
    {
      error = NetCDFWriteVariable(fileID, "channelNodeY", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, channelNodeY);
    }
  
  if (!error && NULL != channelNodeZBank)
    {
      error = NetCDFWriteVariable(fileID, "channelNodeZBank", geometryInstance, localChannelNodeStart, localNumberOfChannelNodes, 1, channelNodeZBank);
    }
  
  if (!error && NULL != channelElementVertices)
    {
      error = NetCDFWriteVariable(fileID, "channelElementVertices", geometryInstance, localChannelElementStart, localNumberOfChannelElements, XDMF_SIZE,
                                  channelElementVertices);
    }
  
  if (!error && NULL != channelVertexX)
    {
      error = NetCDFWriteVariable(fileID, "channelVertexX", geometryInstance, localChannelElementStart, localNumberOfChannelElements, CHANNEL_ELEMENT_VERTICES_SIZE,
                                  channelVertexX);
    }
  
  if (!error && NULL != channelVertexY)
    {
      error = NetCDFWriteVariable(fileID, "channelVertexY", geometryInstance, localChannelElementStart, localNumberOfChannelElements, CHANNEL_ELEMENT_VERTICES_SIZE,
                                  channelVertexY);
    }
  
  if (!error && NULL != channelVertexZBank)
    {
      error = NetCDFWriteVariable(fileID, "channelVertexZBank", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_VERTICES_SIZE, channelVertexZBank);
    }
  
  if (!error && NULL != channelElementX)
    {
      error = NetCDFWriteVariable(fileID, "channelElementX", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelElementX);
    }
  
  if (!error && NULL != channelElementY)
    {
      error = NetCDFWriteVariable(fileID, "channelElementY", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelElementY);
    }
  
  if (!error && NULL != channelElementZBank)
    {
      error = NetCDFWriteVariable(fileID, "channelElementZBank", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelElementZBank);
    }
  
  if (!error && NULL != channelElementBankFullDepth)
    {
      error = NetCDFWriteVariable(fileID, "channelElementBankFullDepth", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelElementBankFullDepth);
    }
  
  if (!error && NULL != channelElementZBed)
    {
      error = NetCDFWriteVariable(fileID, "channelElementZBed", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelElementZBed);
    }
  
  if (!error && NULL != channelElementLength)
    {
      error = NetCDFWriteVariable(fileID, "channelElementLength", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelElementLength);
    }
  
  if (!error && NULL != channelLatitude)
    {
      error = NetCDFWriteVariable(fileID, "channelLatitude", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelLatitude);
    }
  
  if (!error && NULL != channelLongitude)
    {
      error = NetCDFWriteVariable(fileID, "channelLongitude", geometryInstance, localChannelElementStart, localNumberOfChannelElements, 1, channelLongitude);
    }
  
  if (!error && NULL != channelMeshNeighbors)
    {
      error = NetCDFWriteVariable(fileID, "channelMeshNeighbors", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelMeshNeighbors);
    }
  
  if (!error && NULL != channelMeshNeighborsEdgeLength)
    {
      error = NetCDFWriteVariable(fileID, "channelMeshNeighborsEdgeLength", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelMeshNeighborsEdgeLength);
    }
  
  if (!error && NULL != channelSurfacewaterMeshNeighborsConnection)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterMeshNeighborsConnection", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelSurfacewaterMeshNeighborsConnection);
    }
  
  if (!error && NULL != channelGroundwaterMeshNeighborsConnection)
    {
      error = NetCDFWriteVariable(fileID, "channelGroundwaterMeshNeighborsConnection", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelGroundwaterMeshNeighborsConnection);
    }
  
  if (!error && NULL != channelChannelNeighbors)
    {
      error = NetCDFWriteVariable(fileID, "channelChannelNeighbors", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, channelChannelNeighbors);
    }
  
  if (!error && NULL != channelChannelNeighborsDownstream)
    {
      error = NetCDFWriteVariable(fileID, "channelChannelNeighborsDownstream", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, channelChannelNeighborsDownstream);
    }
  
  if (!error && NULL != channelSurfacewaterChannelNeighborsConnection)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterChannelNeighborsConnection", geometryInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, channelSurfacewaterChannelNeighborsConnection);
    }
  
  // Close the geometry file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFWriteGeometry: unable to close NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFWriteParameter()
{
  bool error    = false; // Error flag.
  int  ncErrorCode;      // Return value of NetCDF functions.
  int  fileID;           // ID of NetCDF file.
  bool fileOpen = false; // Whether fileID refers to an open file.
  
  // Open the parameter file.
  error = NetCDFCreateOrOpenForWriteParameter(&fileID);

  if (!error)
    {
      fileOpen = true;
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "geometryInstance", parameterInstance, 0, 1, 1, &geometryInstance);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "numberOfRegions", parameterInstance, 0, 1, 1, &globalNumberOfRegions);
    }
  
  if (!error && NULL != regionNumberOfMeshElements)
    {
      error = NetCDFWriteVariable(fileID, "regionNumberOfMeshElements", parameterInstance, localRegionStart, localNumberOfRegions, 1, regionNumberOfMeshElements);
    }
  
  if (!error && NULL != regionNumberOfChannelElements)
    {
      error = NetCDFWriteVariable(fileID, "regionNumberOfChannelElements", parameterInstance, localRegionStart, localNumberOfRegions, 1, regionNumberOfChannelElements);
    }
  
  if (!error && NULL != meshRegion)
    {
      error = NetCDFWriteVariable(fileID, "meshRegion", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshRegion);
    }
  
  if (!error && NULL != meshCatchment)
    {
      error = NetCDFWriteVariable(fileID, "meshCatchment", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshCatchment);
    }
  
  if (!error && NULL != meshVegetationType)
    {
      error = NetCDFWriteVariable(fileID, "meshVegetationType", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshVegetationType);
    }
  
  if (!error && NULL != meshSoilType)
    {
      error = NetCDFWriteVariable(fileID, "meshSoilType", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSoilType);
    }
  
  if (!error && NULL != meshAlluvium)
    {
      error = NetCDFWriteVariable(fileID, "meshAlluvium", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshAlluvium);
    }
  
  if (!error && NULL != meshManningsN)
    {
      error = NetCDFWriteVariable(fileID, "meshManningsN", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshManningsN);
    }
  
  if (!error && NULL != meshConductivity)
    {
      error = NetCDFWriteVariable(fileID, "meshConductivity", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshConductivity);
    }
  
  if (!error && NULL != meshPorosity)
    {
      error = NetCDFWriteVariable(fileID, "meshPorosity", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshPorosity);
    }
  
  if (!error && NULL != meshGroundwaterMethod)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterMethod", parameterInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshGroundwaterMethod);
    }
  
  if (!error && NULL != meshMeshNeighborsRegion)
    {
      error = NetCDFWriteVariable(fileID, "meshMeshNeighborsRegion", parameterInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshMeshNeighborsRegion);
    }
  
  if (!error && NULL != meshChannelNeighborsRegion)
    {
      error = NetCDFWriteVariable(fileID, "meshChannelNeighborsRegion", parameterInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshChannelNeighborsRegion);
    }
  
  if (!error && NULL != channelRegion)
    {
      error = NetCDFWriteVariable(fileID, "channelRegion", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelRegion);
    }
  
  if (!error && NULL != channelChannelType)
    {
      error = NetCDFWriteVariable(fileID, "channelChannelType", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelChannelType);
    }
  
  if (!error && NULL != channelReachCode)
    {
      error = NetCDFWriteVariable(fileID, "channelReachCode", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelReachCode);
    }
  
  if (!error && NULL != channelBaseWidth)
    {
      error = NetCDFWriteVariable(fileID, "channelBaseWidth", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelBaseWidth);
    }
  
  if (!error && NULL != channelSideSlope)
    {
      error = NetCDFWriteVariable(fileID, "channelSideSlope", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelSideSlope);
    }
  
  if (!error && NULL != channelBedConductivity)
    {
      error = NetCDFWriteVariable(fileID, "channelBedConductivity", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelBedConductivity);
    }
  
  if (!error && NULL != channelBedThickness)
    {
      error = NetCDFWriteVariable(fileID, "channelBedThickness", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelBedThickness);
    }
  
  if (!error && NULL != channelManningsN)
    {
      error = NetCDFWriteVariable(fileID, "channelManningsN", parameterInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelManningsN);
    }
  
  if (!error && NULL != channelMeshNeighborsRegion)
    {
      error = NetCDFWriteVariable(fileID, "channelMeshNeighborsRegion", parameterInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelMeshNeighborsRegion);
    }
  
  if (!error && NULL != channelChannelNeighborsRegion)
    {
      error = NetCDFWriteVariable(fileID, "channelChannelNeighborsRegion", parameterInstance, localChannelElementStart, localNumberOfChannelElements,
                                  CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, channelChannelNeighborsRegion);
    }
  
  // Close the parameter file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFWriteParameter: unable to close NetCDF parameter file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

bool FileManager::NetCDFWriteState()
{
  bool error    = false; // Error flag.
  int  ncErrorCode;      // Return value of NetCDF functions.
  int  fileID;           // ID of NetCDF file.
  bool fileOpen = false; // Whether fileID refers to an open file.
  
  // Open the state file.
  error = NetCDFCreateOrOpenForWriteState(&fileID);

  if (!error)
    {
      fileOpen = true;
    }

  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "geometryInstance", stateInstance, 0, 1, 1, &geometryInstance);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "parameterInstance", stateInstance, 0, 1, 1, &parameterInstance);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "referenceDate", stateInstance, 0, 1, 1, &ADHydro::referenceDate);
    }
  
  if (!error)
    {
      error = NetCDFWriteVariable(fileID, "currentTime", stateInstance, 0, 1, 1, &currentTime);
    }
  
  if (!error && NULL != meshSurfacewaterDepth)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterDepth", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSurfacewaterDepth);
    }
  
  if (!error && NULL != meshSurfacewaterError)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterError", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshSurfacewaterError);
    }
  
  if (!error && NULL != meshGroundwaterHead)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterHead", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshGroundwaterHead);
    }
  
  if (!error && NULL != meshGroundwaterRecharge)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterRecharge", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshGroundwaterRecharge);
    }
  
  if (!error && NULL != meshGroundwaterError)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterError", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshGroundwaterError);
    }
  
  if (!error && NULL != meshPrecipitationRate)
    {
      error = NetCDFWriteVariable(fileID, "meshPrecipitationRate", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshPrecipitationRate);
    }
  
  if (!error && NULL != meshPrecipitationCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshPrecipitationCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                  meshPrecipitationCumulativeShortTerm);
    }
  
  if (!error && NULL != meshPrecipitationCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshPrecipitationCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                  meshPrecipitationCumulativeLongTerm);
    }
  
  if (!error && NULL != meshEvaporationRate)
    {
      error = NetCDFWriteVariable(fileID, "meshEvaporationRate", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshEvaporationRate);
    }
  
  if (!error && NULL != meshEvaporationCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshEvaporationCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                  meshEvaporationCumulativeShortTerm);
    }
  
  if (!error && NULL != meshEvaporationCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshEvaporationCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                  meshEvaporationCumulativeLongTerm);
    }
  
  if (!error && NULL != meshTranspirationRate)
    {
      error = NetCDFWriteVariable(fileID, "meshTranspirationRate", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1, meshTranspirationRate);
    }
  
  if (!error && NULL != meshTranspirationCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshTranspirationCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                  meshTranspirationCumulativeShortTerm);
    }
  
  if (!error && NULL != meshTranspirationCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshTranspirationCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                  meshTranspirationCumulativeLongTerm);
    }
  
  if (!error && NULL != meshEvapoTranspirationState)
    {
      error = NetCDFWriteVariable(fileID, "meshEvapoTranspirationState", stateInstance, localMeshElementStart, localNumberOfMeshElements, 1,
                                  meshEvapoTranspirationState);
    }
  
  // FIXME write vadose zone
  
  if (!error && NULL != meshSurfacewaterMeshNeighborsExpirationTime)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterMeshNeighborsExpirationTime", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshSurfacewaterMeshNeighborsExpirationTime);
    }
  
  if (!error && NULL != meshSurfacewaterMeshNeighborsFlowRate)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterMeshNeighborsFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshSurfacewaterMeshNeighborsFlowRate);
    }
  
  if (!error && NULL != meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm", stateInstance, localMeshElementStart,
                                  localNumberOfMeshElements, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm);
    }
  
  if (!error && NULL != meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm", stateInstance, localMeshElementStart,
                                  localNumberOfMeshElements, MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm);
    }
  
  if (!error && NULL != meshGroundwaterMeshNeighborsExpirationTime)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterMeshNeighborsExpirationTime", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshGroundwaterMeshNeighborsExpirationTime);
    }
  
  if (!error && NULL != meshGroundwaterMeshNeighborsFlowRate)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterMeshNeighborsFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshGroundwaterMeshNeighborsFlowRate);
    }
  
  if (!error && NULL != meshGroundwaterMeshNeighborsFlowCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterMeshNeighborsFlowCumulativeShortTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshGroundwaterMeshNeighborsFlowCumulativeShortTerm);
    }
  
  if (!error && NULL != meshGroundwaterMeshNeighborsFlowCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterMeshNeighborsFlowCumulativeLongTerm", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_MESH_NEIGHBORS_SIZE, meshGroundwaterMeshNeighborsFlowCumulativeLongTerm);
    }
  
  if (!error && NULL != meshSurfacewaterChannelNeighborsExpirationTime)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterChannelNeighborsExpirationTime", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshSurfacewaterChannelNeighborsExpirationTime);
    }
  
  if (!error && NULL != meshSurfacewaterChannelNeighborsFlowRate)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterChannelNeighborsFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshSurfacewaterChannelNeighborsFlowRate);
    }
  
  if (!error && NULL != meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm", stateInstance, localMeshElementStart,
                                  localNumberOfMeshElements, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm);
    }
  
  if (!error && NULL != meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm", stateInstance, localMeshElementStart,
                                  localNumberOfMeshElements, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm);
    }
  
  if (!error && NULL != meshGroundwaterChannelNeighborsExpirationTime)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterChannelNeighborsExpirationTime", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshGroundwaterChannelNeighborsExpirationTime);
    }
  
  if (!error && NULL != meshGroundwaterChannelNeighborsFlowRate)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterChannelNeighborsFlowRate", stateInstance, localMeshElementStart, localNumberOfMeshElements,
                                  MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshGroundwaterChannelNeighborsFlowRate);
    }
  
  if (!error && NULL != meshGroundwaterChannelNeighborsFlowCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterChannelNeighborsFlowCumulativeShortTerm", stateInstance, localMeshElementStart,
                                  localNumberOfMeshElements, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshGroundwaterChannelNeighborsFlowCumulativeShortTerm);
    }
  
  if (!error && NULL != meshGroundwaterChannelNeighborsFlowCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "meshGroundwaterChannelNeighborsFlowCumulativeLongTerm", stateInstance, localMeshElementStart,
                                  localNumberOfMeshElements, MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE, meshGroundwaterChannelNeighborsFlowCumulativeLongTerm);
    }
  
  if (!error && NULL != channelSurfacewaterDepth)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterDepth", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelSurfacewaterDepth);
    }
  
  if (!error && NULL != channelSurfacewaterError)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterError", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelSurfacewaterError);
    }
  
  if (!error && NULL != channelPrecipitationRate)
    {
      error = NetCDFWriteVariable(fileID, "channelPrecipitationRate", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelPrecipitationRate);
    }
  
  if (!error && NULL != channelPrecipitationCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelPrecipitationCumulativeShortTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelPrecipitationCumulativeShortTerm);
    }
  
  if (!error && NULL != channelPrecipitationCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelPrecipitationCumulativeLongTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelPrecipitationCumulativeLongTerm);
    }
  
  if (!error && NULL != channelEvaporationRate)
    {
      error = NetCDFWriteVariable(fileID, "channelEvaporationRate", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelEvaporationRate);
    }
  
  if (!error && NULL != channelEvaporationCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelEvaporationCumulativeShortTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelEvaporationCumulativeShortTerm);
    }
  
  if (!error && NULL != channelEvaporationCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelEvaporationCumulativeLongTerm", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelEvaporationCumulativeLongTerm);
    }
  
  if (!error && NULL != channelEvapoTranspirationState)
    {
      error = NetCDFWriteVariable(fileID, "channelEvapoTranspirationState", stateInstance, localChannelElementStart, localNumberOfChannelElements, 1,
                                  channelEvapoTranspirationState);
    }
  
  if (!error && NULL != channelSurfacewaterMeshNeighborsExpirationTime)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterMeshNeighborsExpirationTime", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelSurfacewaterMeshNeighborsExpirationTime);
    }
  
  if (!error && NULL != channelSurfacewaterMeshNeighborsFlowRate)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterMeshNeighborsFlowRate", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelSurfacewaterMeshNeighborsFlowRate);
    }
  
  if (!error && NULL != channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm);
    }
  
  if (!error && NULL != channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm);
    }
  
  if (!error && NULL != channelGroundwaterMeshNeighborsExpirationTime)
    {
      error = NetCDFWriteVariable(fileID, "channelGroundwaterMeshNeighborsExpirationTime", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelGroundwaterMeshNeighborsExpirationTime);
    }
  
  if (!error && NULL != channelGroundwaterMeshNeighborsFlowRate)
    {
      error = NetCDFWriteVariable(fileID, "channelGroundwaterMeshNeighborsFlowRate", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelGroundwaterMeshNeighborsFlowRate);
    }
  
  if (!error && NULL != channelGroundwaterMeshNeighborsFlowCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelGroundwaterMeshNeighborsFlowCumulativeShortTerm", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelGroundwaterMeshNeighborsFlowCumulativeShortTerm);
    }
  
  if (!error && NULL != channelGroundwaterMeshNeighborsFlowCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelGroundwaterMeshNeighborsFlowCumulativeLongTerm", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE, channelGroundwaterMeshNeighborsFlowCumulativeLongTerm);
    }
  
  if (!error && NULL != channelSurfacewaterChannelNeighborsExpirationTime)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterChannelNeighborsExpirationTime", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, channelSurfacewaterChannelNeighborsExpirationTime);
    }
  
  if (!error && NULL != channelSurfacewaterChannelNeighborsFlowRate)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterChannelNeighborsFlowRate", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, channelSurfacewaterChannelNeighborsFlowRate);
    }
  
  if (!error && NULL != channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm);
    }
  
  if (!error && NULL != channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm)
    {
      error = NetCDFWriteVariable(fileID, "channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm", stateInstance, localChannelElementStart,
                                  localNumberOfChannelElements, CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE, channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm);
    }
  
  // Close the state file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFWriteState: unable to close NetCDF state file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  return error;
}

bool FileManager::NetCDFWriteDisplay()
{
  bool error    = false; // Error flag.
  int  ncErrorCode;      // Return value of NetCDF functions.
  int  fileID;           // ID of NetCDF file.
  bool fileOpen = false; // Whether fileID refers to an open file.
  
  // Open the display file.
  error = NetCDFCreateOrOpenForWriteDisplay(&fileID);

  if (!error)
    {
      fileOpen = true;
    }

  // FIXME decide what should go into the display file.
  
  // Close the state file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::NetCDFWriteDisplay: unable to close NetCDF display file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  return error;
}

void FileManager::initializeFromNetCDFFiles()
{
  bool error = false; // Error flag.
  
  // Copy state instance value requested by user.
  stateInstance = ADHydro::adhydroInputStateInstance;
  
  error = NetCDFReadInstances();
  
  if (!error)
    {
      error = NetCDFReadGeometry();
    }
  
  if (!error)
    {
      error = NetCDFReadParameter();
    }
  
  if (!error)
    {
      error = NetCDFReadState();
    }

  if (error)
    {
      CkExit();
    }
}

void FileManager::writeNetCDFFiles()
{
  bool   error          = false; // Error flag.
  int    ncErrorCode;            // Return value of NetCDF functions.
  int    fileID;                 // ID of NetCDF file.
  bool   fileOpen       = false; // Whether fileID refers to an open file.
  bool   writeGeometry  = false; // Whether to write a new geometry instance.
  bool   writeParameter = false; // Whether to write a new parameter instance.
  bool   writeState     = false; // Whether to write a new state instance.
  bool   writeDisplay   = false; // Whether to write a new display instance.
  size_t geometryInstancesSize;  // Number of instances in geometry file.
  size_t parameterInstancesSize; // Number of instances in parameter file.
  size_t stateInstancesSize;     // Number of instances in state file.
  size_t displayInstancesSize;   // Number of instances in display file.
  
  // First, decide whether we need to write each file.
  
  // For the geometry file, there are three cases:
  // 1) The geometry has been changed.  In this case the geometry must always be written.
  // 2) The geometry was loaded, has not been changed, and this is the first time we are checking whether the geometry needs to be written.  In this case the
  //    geometry needs to be written unless the user is appending output to the existing input file that already has the geometry in it.  It's difficult to
  //    know whether the specified output file already has the correct geometry in it.  The user could accidentally point at a NetCDF file that has instances
  //    that are not the geometry that was loaded.  However, we are not going to worry about this right now.  We will look in the file, and if it has the
  //    instance number that was loaded we will assume that's the right geometry and it does not need to be written.  If we initialized from ASCII files the
  //    instance number will be -1 and so will never match.
  // 3) The geometry has not been changed, and we have checked that the geometry is already in the output file.  In this case the geometry does not need to be
  //    written.
  if (geometryChanged)
    {
      writeGeometry = true;
    }
  else if (!geometryInstanceChecked && (size_t)-1 == geometryInstance)
    {
      writeGeometry = true;
    }
  else if (!geometryInstanceChecked)
    {
      // Open the geometry file.
      error = NetCDFCreateOrOpenForWriteGeometry(&fileID);

      if (!error)
        {
          fileOpen = true;
        }
      
      if (!error)
        {
          error = NetCDFReadDimensionSize(fileID, "instances", &geometryInstancesSize);
        }
      
      if (!error && !(geometryInstance < geometryInstancesSize))
        {
          writeGeometry = true;
        }
    }
  
  // If we do need to write the geometry we will write a new instance one after the last instance currently in the file.
  if (!error && writeGeometry)
    {
      // Get the number of instances in the geometry file if we haven't already.
      if (!fileOpen)
        {
          // Open the geometry file.
          error = NetCDFCreateOrOpenForWriteGeometry(&fileID);

          if (!error)
            {
              fileOpen = true;
            }
          
          if (!error)
            {
              error = NetCDFReadDimensionSize(fileID, "instances", &geometryInstancesSize);
            }
        }
      
      // Use the instance one after the last instance currently in the file, which is equal to the number of instances in the file.
      geometryInstance = geometryInstancesSize;
      
      // Once the geometry is written it is no longer changed and it has been checked that the geometry exists in the output file.
      geometryChanged         = false;
      geometryInstanceChecked = true;
    }
  
  // Close the geometry file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeNetCDFFiles: unable to close NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // The parameter file has the same three cases as the geometry file plus if the geometry is being written the parameter file must be written so that it links
  // back to the correct geometry.
  if (writeGeometry)
    {
      writeParameter = true;
    }
  if (parameterChanged)
    {
      writeParameter = true;
    }
  else if (!parameterInstanceChecked && (size_t)-1 == parameterInstance)
    {
      writeParameter = true;
    }
  else if (!parameterInstanceChecked)
    {
      // Open the parameter file.
      error = NetCDFCreateOrOpenForWriteParameter(&fileID);

      if (!error)
        {
          fileOpen = true;
        }
      
      if (!error)
        {
          error = NetCDFReadDimensionSize(fileID, "instances", &parameterInstancesSize);
        }
      
      if (!error && !(parameterInstance < parameterInstancesSize))
        {
          writeParameter = true;
        }
    }
  
  // If we do need to write the parameter file we will write a new instance one after the last instance currently in the file.
  if (!error && writeParameter)
    {
      // Get the number of instances in the parameter file if we haven't already.
      if (!fileOpen)
        {
          // Open the parameter file.
          error = NetCDFCreateOrOpenForWriteParameter(&fileID);

          if (!error)
            {
              fileOpen = true;
            }
          
          if (!error)
            {
              error = NetCDFReadDimensionSize(fileID, "instances", &parameterInstancesSize);
            }
        }
      
      // Use the instance one after the last instance currently in the file, which is equal to the number of instances in the file.
      parameterInstance = parameterInstancesSize;
      
      // Once the parameter file is written it is no longer changed and it has been checked that the parameter instance exists in the output file.
      parameterChanged         = false;
      parameterInstanceChecked = true;
    }
  
  // Close the parameter file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeNetCDFFiles: unable to close NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // FIXME Implement write criteria for the state and display files.  For now, always write a new instance.
  writeState = true;
  
  // If we do need to write the state we will write a new instance one after the last instance currently in the file.
  if (!error && writeState)
    {
      // Get the number of instances in the state file if we haven't already.
      if (!fileOpen)
        {
          // Open the state file.
          error = NetCDFCreateOrOpenForWriteState(&fileID);

          if (!error)
            {
              fileOpen = true;
            }
          
          if (!error)
            {
              error = NetCDFReadDimensionSize(fileID, "instances", &stateInstancesSize);
            }
        }
      
      // Use the instance one after the last instance currently in the file, which is equal to the number of instances in the file.
      stateInstance = stateInstancesSize;
    }
  
  // Close the state file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeNetCDFFiles: unable to close NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // FIXME Implement write criteria for the state and display files.  For now, always write a new instance.
  writeDisplay = true;
  
  // If we do need to write the display we will write a new instance one after the last instance currently in the file.
  if (!error && writeDisplay)
    {
      // Get the number of instances in the display file if we haven't already.
      if (!fileOpen)
        {
          // Open the display file.
          error = NetCDFCreateOrOpenForWriteDisplay(&fileID);

          if (!error)
            {
              fileOpen = true;
            }
          
          if (!error)
            {
              error = NetCDFReadDimensionSize(fileID, "instances", &displayInstancesSize);
            }
        }
      
      // Use the instance one after the last instance currently in the file, which is equal to the number of instances in the file.
      displayInstance = displayInstancesSize;
    }
  
  // Close the display file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeNetCDFFiles: unable to close NetCDF display file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write instances as necessary.
  // FIXME if we had to open the files above figure out how to use the already open files instead of reopening them.
  if (!error && writeGeometry)
    {
      error = NetCDFWriteGeometry();
    }
  
  if (!error && writeParameter)
    {
      error = NetCDFWriteParameter();
    }
  
  if (!error && writeState)
    {
      error = NetCDFWriteState();
    }
  
  if (!error && writeDisplay)
    {
      error = NetCDFWriteDisplay();
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
  if (!(1 == CkNumPes() && 0 == localMeshElementStart && globalNumberOfMeshElements == localNumberOfMeshElements &&
      0 == localChannelElementStart && globalNumberOfChannelElements == localNumberOfChannelElements))
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
  if (!(1 == CkNumPes() && 0 == localMeshElementStart && globalNumberOfMeshElements == localNumberOfMeshElements &&
        0 == localChannelElementStart && globalNumberOfChannelElements == localNumberOfChannelElements))
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
  
  // Find the lowest channel element that has the proper reach code.
  for (ii = 0; ii < globalNumberOfChannelElements; ii++)
    {
      if (reachCode == channelReachCode[ii] && (NOFLOW == neighbor || neighborZBank > channelElementZBank[ii]))
        {
          neighbor      = ii;
          neighborZBank = channelElementZBank[ii];
        }
    }
  
  // Connect the mesh element to that channel element
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
  if (!(1 == CkNumPes() && 0 == localMeshElementStart && globalNumberOfMeshElements == localNumberOfMeshElements &&
        0 == localChannelElementStart && globalNumberOfChannelElements == localNumberOfChannelElements))
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
          CkError("WARNING in FileManager::breakChannelDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", channelElement,
                  distanceLowered, channelElementZBed[channelElement], channelElementZBed[dammedChannelElement]);
        }
      
      channelElementZBank[channelElement] -= distanceLowered;
      channelElementZBed[channelElement]   = channelElementZBed[dammedChannelElement];
      slope                                = 0.0;
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
                  CkError("WARNING in FileManager::breakChannelDigitalDam: channel element %d has no downstream connections.\n", channelElement);
                }
            }
          
          distanceLowered = channelElementZBed[channelElement] - channelElementZBed[dammedChannelElement];
          
          if ((2 <= ADHydro::verbosityLevel && 10.0 < distanceLowered) || 3 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in FileManager::breakChannelDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", channelElement,
                      distanceLowered, channelElementZBed[channelElement], channelElementZBed[dammedChannelElement]);
            }
          
          channelElementZBank[channelElement] -= distanceLowered;
          channelElementZBed[channelElement]   = channelElementZBed[dammedChannelElement];
          slope                                = 0.0;
        }
      else
        {
          // Continue to search downstream, and when the search returns lower channelElement to a straight line slope down from the dammed element.
          slope = breakChannelDigitalDam(downstreamElement, dammedChannelElement, length + channelElementLength[channelElement]);
          
          distanceLowered = channelElementZBed[channelElement] - (channelElementZBed[dammedChannelElement] - slope * (length + 0.5 * channelElementLength[channelElement]));
          
          if ((2 <= ADHydro::verbosityLevel && 10.0 < distanceLowered) || 3 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in FileManager::breakChannelDigitalDam: breaking digital dam by lowering channel element %d by %lf meters from %lf to %lf.\n", channelElement,
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
  if (!(1 == CkNumPes() && 0 == localMeshElementStart && globalNumberOfMeshElements == localNumberOfMeshElements &&
        0 == localChannelElementStart && globalNumberOfChannelElements == localNumberOfChannelElements))
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
  
  if (!(NULL != meshGroundwaterChannelNeighborsConnection))
    {
      CkError("ERROR in FileManager::meshMassage: meshGroundwaterChannelNeighborsConnection must not be NULL.\n");
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
  
  if (!(NULL != channelMeshNeighbors))
    {
      CkError("ERROR in FileManager::meshMassage: channelMeshNeighbors must not be NULL.\n");
      CkExit();
    }
  
  if (!(NULL != channelGroundwaterMeshNeighborsConnection))
    {
      CkError("ERROR in FileManager::meshMassage: channelGroundwaterMeshNeighborsConnection must not be NULL.\n");
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
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0 == localRegionStart && globalNumberOfRegions == localNumberOfRegions);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
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
          
          // Remove groundwater INFLOW and OUTFLOW boundaries from elements that are not SHALLOW_AQUIFER.
          if ((INFLOW == neighbor || OUTFLOW == neighbor) && InfiltrationAndGroundwater::SHALLOW_AQUIFER != meshGroundwaterMethod[ii])
            {
              meshGroundwaterMeshNeighborsConnection[ii][jj] = false;
            }
          
          // Only the lower ID number of the pair has to check.  This check also excludes boundary condition codes.
          if (ii < neighbor)
            {
              surfacewaterConnection = meshSurfacewaterMeshNeighborsConnection[ii][jj];
              groundwaterConnection  = meshGroundwaterMeshNeighborsConnection[ii][jj];

              // Remove neighbor connection between neighboring mesh elements that are in different catchments and separated by a ridge.
              if (meshCatchment[ii] != meshCatchment[neighbor])
                {
                  // Get the height of the center of the edge separating the neighbors.  If it is higher than both neighbors it is considered a ridge.
                  // FIXLATER this could be replaced with checking if both elements slope away from the edge.
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
          else if (!meshAlluvium[ii])
            {
              // FIXME this is a really tangled knot.  If a mesh element has any channel neighbors we set it to alluvium so that it will have a shallow
              // aquifer, groundwater neighbor connections, etc.  We do this early in calculateDerivedValues because we want to have alluvium set before we set
              // meshGroundwaterMethod, which is used to set a bunch of other things that have to come before here for various reasons.  So there's a cyclic
              // dependency and we can't just set alluvium true here.  Alternatively, we could try to redo everything here to make it what it would have been
              // if the element had been alluvium, but that would create a maintainence nightmare to keep the behavior here exactly the same as the behavior in
              // other parts of the code.  For now, what we are doing is just not making those elements alluvium.  This doesn't seem like too bad behavior
              // since the only purpose of making this connection is to allow the channel to drain surfacewater from the element so it's not as important that
              // the element have groundwater connections.  One final thing you can do to fix this is to edit the .geolType file to set the element to be
              // alluvium from the start and re-load from ASCII files.
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::meshMassage: mesh element %d was a digital dam that got connected to a stream, but it is not alluvial.\n",
                          ii);
                }
            }
        }
    } // End break digital dams in the mesh.
}

void FileManager::calculateDerivedValues()
{
  int               ii, jj;          // Loop counters.
  double            value;           // For calculating derived values.
  double            bcLambda;        // For allocating GARTO domains.
  double            bcPsib;          // For allocating GARTO domains.
  garto_parameters* gartoParameters; // For allocating GARTO domains.
  garto_domain*     gartoDomain;     // For allocating GARTO domains.
  double            lengthSoFar;     // For traversing vertices.  The length traversed so far.
  double            nextLength;      // For traversing vertices.  The length to the next vertex.
  double            lengthFraction;  // For traversing vertices.  The fraction of the distance of the point of interest from the current vertex to the next vertex.
  double            minX;            // For finding the bounding box of vertices.
  double            maxX;            // For finding the bounding box of vertices.
  double            minY;            // For finding the bounding box of vertices.
  double            maxY;            // For finding the bounding box of vertices.
  double            minZBank;        // For finding the bounding box of vertices.
  double            maxZBank;        // For finding the bounding box of vertices.

  // Delete vertex updated arrays that are no longer needed.
  delete[] meshVertexUpdated;
  delete[] channelVertexUpdated;
  
  // If a mesh element has any channel neighbors it is automatically considered alluvium.
  if (NULL != meshAlluvium && NULL != meshChannelNeighbors)
    {
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          if (!meshAlluvium[ii] && NOFLOW != meshChannelNeighbors[ii][0]) // Channel neighbors are compacted to the front so we only need to check the first one.
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
                        "for mixed forest\n", ii + localMeshElementStart, meshVegetationType[ii]);
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
                        "porosity for sand\n", ii + localMeshElementStart, meshSoilType[ii]);
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
  
  // Creating a GARTO domain depends on meshGroundwaterHead, which depends on meshGroundwaterMethod, which depends on meshVadoseZone.infiltrationMethod.
  // So here we are merely choosing between NO_INFILTRATION and TRIVIAL_INFILTRATION to indicate whether infiltration is performed or not.  The method used
  // if infiltration is performed is chosen after calculating meshGroundwaterHead.  If not already specified meshVadoseZone.infiltrationMethod defaults to
  // NO_INFILTRATION if soil type is bedrock or soil depth is zero.  Otherwise it defaults to TRIVIAL_INFILTRATION.
  if (NULL == meshVadoseZone)
    {
      meshVadoseZone = new InfiltrationAndGroundwater::VadoseZone[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          if ((NULL != meshSoilType && 15 == meshSoilType[ii]) || (NULL != meshElementSoilDepth && 0.0 == meshElementSoilDepth[ii]))
            {
              meshVadoseZone[ii].infiltrationMethod = InfiltrationAndGroundwater::NO_INFILTRATION;
            }
          else
            {
              meshVadoseZone[ii].infiltrationMethod = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
            }
        }
    }
  
  // If not already specified meshGroundwaterMethod defaults to NO_AQUIFER if meshVadoseZone.infiltrationMethod is NO_INFILTRATION.  Otherwise, it defaults to
  // DEEP_AQUIFER for non-alluvium elements and SHALLOW_AQUIFER for alluvium elements.  If meshAlluvium is not specified it is assumed that all elements are
  // alluvium.
  if (NULL == meshGroundwaterMethod)
    {
      meshGroundwaterMethod = new InfiltrationAndGroundwater::GroundwaterMethodEnum[localNumberOfMeshElements];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          if (NULL != meshVadoseZone && InfiltrationAndGroundwater::NO_INFILTRATION == meshVadoseZone[ii].infiltrationMethod)
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
  
  // Select which infiltration method to use if infiltration is performed.  If the user specifies switching infiltration methods any previous vadose zone state
  // is erased and replaced with hydrostatic equilibrium with the water table.
  if (NULL != meshVadoseZone)
    {
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          if (InfiltrationAndGroundwater::NO_INFILTRATION != meshVadoseZone[ii].infiltrationMethod &&
              InfiltrationAndGroundwater::NO_INFILTRATION != ADHydro::infiltrationMethod &&
              ADHydro::infiltrationMethod != meshVadoseZone[ii].infiltrationMethod)
            {
              switch (ADHydro::infiltrationMethod)
              {
              case InfiltrationAndGroundwater::NO_INFILTRATION:
                // Unreachable code.
                break;
              case InfiltrationAndGroundwater::TRIVIAL_INFILTRATION:
                // Assignment operator handles deallocating any previous state.
                meshVadoseZone[ii] = InfiltrationAndGroundwater::VadoseZone(InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, NULL);
                break;
              case InfiltrationAndGroundwater::GARTO_INFILTRATION:
                if (NULL != meshSoilType && NULL != meshElementZSurface && NULL != meshElementSoilDepth && NULL != meshConductivity && NULL != meshPorosity &&
                    NULL != meshGroundwaterHead && NULL != meshGroundwaterMethod)
                  {
                    // Set Brooks-Corey parameters from table 1 of Talbot, C. A., and F. L. Ogden (2008), A method for computing infiltration and
                    // redistribution in a discretized moisture content domain, Water Resour. Res., 44, W08453, doi:10.1029/2008WR006815.
                    switch (meshSoilType[ii])
                    {
                    case  1: // SAND.
                      bcLambda = 0.694;
                      bcPsib   = 0.0726;
                      break;
                    case  2: // LOAMY SAND.
                      bcLambda = 0.553;
                      bcPsib   = 0.0869;
                      break;
                    case  3: // SANDY LOAM.
                      bcLambda = 0.378;
                      bcPsib   = 0.1466;
                      break;
                    case  4: // SILT LOAM.
                      bcLambda = 0.234;
                      bcPsib   = 0.2079;
                      break;
                    case  5: // SILT.
                      bcLambda = 0.192; // Taken from averaging SILT LOAM and SILTY CLAY.
                      bcPsib   = 0.2749;
                      break;
                    case  6: // LOAM.
                      bcLambda = 0.252;
                      bcPsib   = 0.1115;
                      break;
                    case  7: // SANDY CLAY LOAM.
                      bcLambda = 0.319;
                      bcPsib   = 0.2808;
                      break;
                    case  8: // SILTY CLAY LOAM.
                      bcLambda = 0.177;
                      bcPsib   = 0.3256;
                      break;
                    case  9: // CLAY LOAM.
                      bcLambda = 0.242;
                      bcPsib   = 0.2589;
                      break;
                    case 10: // SANDY CLAY.
                      bcLambda = 0.223;
                      bcPsib   = 0.2917;
                      break;
                    case 11: // SILTY CLAY.
                      bcLambda = 0.150;
                      bcPsib   = 0.3419;
                      break;
                    case 12: // CLAY.
                      bcLambda = 0.165;
                      bcPsib   = 0.3730;
                      break;
                    case 13: // ORGANIC MATERIAL.
                      bcLambda = 0.252; // Taken from LOAM.
                      bcPsib   = 0.1115;
                      break;
                    case 14: // WATER.
                      bcLambda = 0.165; // Taken from CLAY.
                      bcPsib   = 0.3730;
                      break;
                    case 15: // BEDROCK.
                      bcLambda = 0.553; // Taken from LOAMY SAND.
                      bcPsib   = 0.0869;
                      break;
                    case 16: // OTHER(land-ice).
                      bcLambda = 0.553; // Taken from LOAMY SAND.
                      bcPsib   = 0.0869;
                      break;
                    case 17: // PLAYA.
                      bcLambda = 0.165; // Taken from CLAY.
                      bcPsib   = 0.3730;
                      break;
                    case 18: // LAVA.
                      bcLambda = 0.553; // Taken from LOAMY SAND.
                      bcPsib   = 0.0869;
                      break;
                    case 19: // WHITE SAND.
                      bcLambda = 0.694;
                      bcPsib   = 0.0726;
                      break;
                    default:
                      if (2 <= ADHydro::verbosityLevel)
                        {
                          CkError("WARNING in FileManager::calculateDerivedValues, mesh element %d: unknown soil type %d. using default values of bcLambda "
                                  "and bcPsib for sand.\n", ii + localMeshElementStart, meshSoilType[ii]);
                        }

                      bcLambda = 0.694;
                      bcPsib   = 0.0726;
                      break;
                    } // End of switch (meshSoilType[ii]).

                    // FIXLATER How many GARTO bins to use?  Currently set to 10.  What value for residual saturation?  Currently set to 1% of porosity.
                    garto_parameters_alloc(&gartoParameters, 10, meshConductivity[ii], meshPorosity[ii], meshPorosity[ii] * 0.01, false, 0.0, 0.0, bcLambda,
                                           bcPsib);

                    // FIXLATER What value for initial water content?  Currently set to 10% of porosity
                    garto_domain_alloc(&gartoDomain, gartoParameters, 0.0, meshElementSoilDepth[ii],
                                       InfiltrationAndGroundwater::SHALLOW_AQUIFER == meshGroundwaterMethod[ii], meshPorosity[ii] * 0.1, true,
                                       meshElementZSurface[ii] - meshGroundwaterHead[ii]);

                    // Assignment operator handles deallocating any previous state.
                    meshVadoseZone[ii] = InfiltrationAndGroundwater::VadoseZone(InfiltrationAndGroundwater::GARTO_INFILTRATION, gartoDomain);
                  }
                else
                  {
                    if (2 <= ADHydro::verbosityLevel)
                      {
                        CkError("WARNING in FileManager::calculateDerivedValues, mesh element %d: insufficient information to create GARTO domain.  "
                                "Defaulting to trivial infiltration.", ii + localMeshElementStart);
                      }

                    // Assignment operator handles deallocating any previous state.
                    meshVadoseZone[ii] = InfiltrationAndGroundwater::VadoseZone(InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, NULL);
                  }
                break;
              }

#if !(DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
              if (meshVadoseZone[ii].checkInvariant())
                {
                  CkExit();
                }
#endif // !(DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
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
  
  // If not already specified meshSurfacewaterMeshNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.  Removing connections
  // for channel edges is done in mesh massage.
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
  
  // If not already specified meshSurfacewaterMeshNeighborsExpirationTime defaults to currentTime.
  if (NULL == meshSurfacewaterMeshNeighborsExpirationTime)
    {
      meshSurfacewaterMeshNeighborsExpirationTime = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterMeshNeighborsExpirationTime[ii][jj] = ADHydro::currentTime;
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
  
  // If not already specified meshGroundwaterMeshNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.  Removing connections
  // for no shallow aquifer is done in mesh massage.
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
  
  // If not already specified meshGroundwaterMeshNeighborsExpirationTime defaults to currentTime.
  if (NULL == meshGroundwaterMeshNeighborsExpirationTime)
    {
      meshGroundwaterMeshNeighborsExpirationTime = new double[localNumberOfMeshElements][MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterMeshNeighborsExpirationTime[ii][jj] = ADHydro::currentTime;
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
  
  // If not already specified meshSurfacewaterChannelNeighborsExpirationTime defaults to currentTime.
  if (NULL == meshSurfacewaterChannelNeighborsExpirationTime)
    {
      meshSurfacewaterChannelNeighborsExpirationTime = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshSurfacewaterChannelNeighborsExpirationTime[ii][jj] = ADHydro::currentTime;
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
  
  // If not already specified meshGroundwaterChannelNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.  Removing connections
  // for no aquifer is done in mesh massage.
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
  
  // If not already specified meshGroundwaterChannelNeighborsExpirationTime defaults to currentTime.
  if (NULL == meshGroundwaterChannelNeighborsExpirationTime)
    {
      meshGroundwaterChannelNeighborsExpirationTime = new double[localNumberOfMeshElements][MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              meshGroundwaterChannelNeighborsExpirationTime[ii][jj] = ADHydro::currentTime;
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
  
  // If not already specified channelSurfacewaterMeshNeighborsExpirationTime defaults to currentTime.
  if (NULL == channelSurfacewaterMeshNeighborsExpirationTime)
    {
      channelSurfacewaterMeshNeighborsExpirationTime = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterMeshNeighborsExpirationTime[ii][jj] = ADHydro::currentTime;
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
  
  // If not already specified channelGroundwaterMeshNeighborsConnection defaults to true if the neighbor is not NOFLOW, otherwise false.  Removing connections
  // for no aquifer is done in mesh massage.
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
  
  // If not already specified channelGroundwaterMeshNeighborsExpirationTime defaults to currentTime.
  if (NULL == channelGroundwaterMeshNeighborsExpirationTime)
    {
      channelGroundwaterMeshNeighborsExpirationTime = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; jj++)
            {
              channelGroundwaterMeshNeighborsExpirationTime[ii][jj] = ADHydro::currentTime;
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
  
  // If not already specified channelSurfacewaterChannelNeighborsExpirationTime defaults to currentTime.
  if (NULL == channelSurfacewaterChannelNeighborsExpirationTime)
    {
      channelSurfacewaterChannelNeighborsExpirationTime = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
      
      for (ii = 0; ii < localNumberOfChannelElements; ii++)
        {
          for (jj = 0; jj < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; jj++)
            {
              channelSurfacewaterChannelNeighborsExpirationTime[ii][jj] = ADHydro::currentTime;
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
  if (0 < localNumberOfMeshElements)
    {
      meshElementUpdated = new bool[localNumberOfMeshElements];
    }

  if (0 < localNumberOfChannelElements)
    {
      channelElementUpdated = new bool[localNumberOfChannelElements];
    }
  
  currentTime         = ADHydro::currentTime;
  nextCheckpointIndex = 1 + (int)floor(currentTime / ADHydro::checkpointPeriod);
  nextOutputIndex     = 1 + (int)floor(currentTime / ADHydro::outputPeriod);
  simulationEndTime   = currentTime + ADHydro::simulationDuration;

  // Fix digital dams and similar problems.
  if (ADHydro::doMeshMassage)
    {
      meshMassage();
    }
}

bool FileManager::readForcingData()
{
  bool   error       = false; // Error flag.
  int    ncErrorCode;         // Return value of NetCDF functions.
  int    fileID;              // ID of NetCDF file.
  bool   fileOpen    = false; // Whether fileID refers to an open file.
  int    variableID;          // ID of variable in NetCDF file.
  double forcingDate = ADHydro::referenceDate + (ADHydro::drainDownMode ? ADHydro::drainDownTime : currentTime) / (24.0 * 3600.0);
                              // The date and time to read forcing data for as a Julian date.
  long   forcingDataYear;     // For calculating Gregorian date from Julian date.
  long   forcingDataMonth;    // For calculating Gregorian date from Julian date.
  long   forcingDataDay;      // For calculating Gregorian date from Julian date.
  long   forcingDataHour;     // For calculating Gregorian date from Julian date.
  long   forcingDataMinute;   // For calculating Gregorian date from Julian date.
  double forcingDataSecond;   // For calculating Gregorian date from Julian date.
  long   simulationYear;      // For calculating Gregorian date from Julian date.
  long   simulationMonth;     // For calculating Gregorian date from Julian date.
  long   simulationDay;       // For calculating Gregorian date from Julian date.
  long   simulationHour;      // For calculating Gregorian date from Julian date.
  long   simulationMinute;    // For calculating Gregorian date from Julian date.
  double simulationSecond;    // For calculating Gregorian date from Julian date.
  
  // Open the forcing file.
  error = NetCDFOpenForRead(ADHydro::adhydroInputForcingFilePath.c_str(), &fileID);
  
  if (!error)
    {
      fileOpen = true;
    }
  
  // Get the list of Julian dates in the forcing file.
  if (NULL == jultime)
    {
      if (!error)
        {
          error = NetCDFReadDimensionSize(fileID, "Time", &jultimeSize);

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!error && !(0 < jultimeSize))
            {
              CkError("ERROR in FileManager::readForcingData: No forcing data in NetCDF forcing file.\n");
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }

      if (!error)
        {
          // Get the variable ID.
          ncErrorCode = nc_inq_varid(fileID, "JULTIME", &variableID);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::readForcingData: unable to get variable JULTIME in NetCDF forcing file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          jultime = new double[jultimeSize];
          
          // Get the variable data.
          ncErrorCode = nc_get_var(fileID, variableID, jultime);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::readForcingData: unable to read variable JULTIME in NetCDF forcing file.  NetCDF error message: %s.\n",
                      nc_strerror(ncErrorCode));
              error = true;
            }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Search for the last instance that is before or equal to the current date and time.
      // FIXME to improve efficiency make this a binary search.
      if (!error)
        {
          jultimeNextInstance = 0;

          if (!(jultime[0] <= forcingDate))
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in FileManager::readForcingData: All forcing data in the NetCDF forcing file is in the future.  Using the first forcing "
                          "data instance.\n");
                }
            }
          else
            {
              while (jultimeNextInstance + 1 < jultimeSize && jultime[jultimeNextInstance + 1] <= forcingDate)
                {
                  jultimeNextInstance++;
                }
            }
        }
    }
  
  if (!error)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // jultimeNextInstance is now the index of the forcing data we will use.
      CkAssert(jultimeSize > jultimeNextInstance);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      if (0 == CkMyPe() && 1 <= ADHydro::verbosityLevel)
        {
          julianToGregorian(jultime[jultimeNextInstance], &forcingDataYear, &forcingDataMonth, &forcingDataDay, &forcingDataHour, &forcingDataMinute,
                            &forcingDataSecond);
          julianToGregorian(ADHydro::referenceDate + currentTime / (24.0 * 3600.0), &simulationYear, &simulationMonth, &simulationDay, &simulationHour,
                            &simulationMinute, &simulationSecond);

          CkPrintf("Reading forcing data for %02d/%02d/%04d %02d:%02d:%02.0lf UTC at simulation time %.0lf, date %02d/%02d/%04d %02d:%02d:%02.0lf UTC.\n",
                   forcingDataMonth, forcingDataDay, forcingDataYear, forcingDataHour, forcingDataMinute, forcingDataSecond, currentTime, simulationMonth,
                   simulationDay, simulationYear, simulationHour, simulationMinute, simulationSecond);
        }
    }
  
  // Read forcing data
  if (0 < localNumberOfMeshElements)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "T2", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &t2);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "PSFC", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &psfc);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "U", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &u);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "V", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &v);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "QVAPOR", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &qVapor);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "QCLOUD", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &qCloud);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "SWDOWN", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &swDown);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "GLW", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &gLw);
        }
      
      if (!error && !ADHydro::drainDownMode)
        {
          error = NetCDFReadVariable(fileID, "TPREC", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &tPrec);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "TSLB", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &tslb);
        }
      
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "PBLH", jultimeNextInstance, localMeshElementStart, localNumberOfMeshElements, 1, 1, true, 0.0f, true, &pblh);
        }
    }
  
  if (0 < localNumberOfChannelElements)
    {
      if (!error)
        {
          error = NetCDFReadVariable(fileID, "T2_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &t2_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "PSFC_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &psfc_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "U_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &u_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "V_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &v_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "QVAPOR_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &qVapor_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "QCLOUD_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &qCloud_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "SWDOWN_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &swDown_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "GLW_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &gLw_c);
        }

      if (!error && !ADHydro::drainDownMode)
        {
          error = NetCDFReadVariable(fileID, "TPREC_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &tPrec_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "TSLB_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &tslb_c);
        }

      if (!error)
        {
          error = NetCDFReadVariable(fileID, "PBLH_C", jultimeNextInstance, localChannelElementStart, localNumberOfChannelElements, 1, 1, true, 0.0f, true, &pblh_c);
        }
    }
  
  if (!error)
    {
      if (0 == CkMyPe() && 1 <= ADHydro::verbosityLevel)
        {
          CkPrintf("Finished reading forcing data.\n");
        }
      
      // Increment to the next instance
      jultimeNextInstance++;
      
      // FIXME deal with the next instance possibly being in the past.
      
      if (0 == CkMyPe() && 2 <= ADHydro::verbosityLevel && !(jultimeSize > jultimeNextInstance))
        {
          CkError("WARNING in FileManager::readForcingData: Using the last forcing data instance in NetCDF forcing file.  No new forcing data will "
                  "be loaded after this no matter how long the simulation runs.\n");
        }
    }
  
  // Close the forcing file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::readForcingData: unable to close NetCDF forcing file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

void FileManager::handleSendInitializationMessages(CProxy_Region regionProxy)
{
  bool                            error = false;                 // Error flag.
  int                             ii, jj, kk;                    // Loop counters.
  EvapoTranspirationForcingStruct evapoTranspirationForcingInit; // For initializing forcing data.
  std::vector<simpleNeighborInfo> surfacewaterMeshNeighbors;     // For initializing mesh neighbors.
  std::vector<simpleNeighborInfo> surfacewaterChannelNeighbors;  // For initializing mesh neighbors.
  std::vector<simpleNeighborInfo> groundwaterMeshNeighbors;      // For initializing mesh neighbors.
  std::vector<simpleNeighborInfo> groundwaterChannelNeighbors;   // For initializing mesh neighbors.
  
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
      
      if (!(NULL != meshGroundwaterMethod))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMethod must not be NULL.\n");
          error = true;
        }
      
      if (!(NULL != meshVadoseZone))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshVadoseZone must not be NULL.\n");
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

      if (!(NULL != meshSurfacewaterMeshNeighborsExpirationTime))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsExpirationTime must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterMeshNeighborsFlowRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterMeshNeighborsFlowRate must not be NULL.\n");
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

      if (!(NULL != meshGroundwaterMeshNeighborsExpirationTime))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsExpirationTime must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterMeshNeighborsFlowRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterMeshNeighborsFlowRate must not be NULL.\n");
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

      if (!(NULL != meshSurfacewaterChannelNeighborsExpirationTime))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterChannelNeighborsExpirationTime must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshSurfacewaterChannelNeighborsFlowRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshSurfacewaterChannelNeighborsFlowRate must not be NULL.\n");
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

      if (!(NULL != meshGroundwaterChannelNeighborsExpirationTime))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterChannelNeighborsExpirationTime must not be NULL.\n");
          error = true;
        }

      if (!(NULL != meshGroundwaterChannelNeighborsFlowRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: meshGroundwaterChannelNeighborsFlowRate must not be NULL.\n");
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

      if (!(NULL != channelLatitude))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelLatitude must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelLongitude))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelLongitude must not be NULL.\n");
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

      if (!(NULL != channelPrecipitationRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelPrecipitationRate must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelPrecipitationCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelPrecipitationCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelPrecipitationCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelPrecipitationCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelEvaporationRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelEvaporationRate must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelEvaporationCumulativeShortTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelEvaporationCumulativeShortTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelEvaporationCumulativeLongTerm))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelEvaporationCumulativeLongTerm must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelEvapoTranspirationState))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelEvapoTranspirationState must not be NULL.\n");
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

      if (!(NULL != channelSurfacewaterMeshNeighborsExpirationTime))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterMeshNeighborsExpirationTime must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterMeshNeighborsFlowRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterMeshNeighborsFlowRate must not be NULL.\n");
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

      if (!(NULL != channelGroundwaterMeshNeighborsExpirationTime))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelGroundwaterMeshNeighborsExpirationTime must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelGroundwaterMeshNeighborsFlowRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelGroundwaterMeshNeighborsFlowRate must not be NULL.\n");
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

      if (!(NULL != channelSurfacewaterChannelNeighborsExpirationTime))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterChannelNeighborsExpirationTime must not be NULL.\n");
          error = true;
        }

      if (!(NULL != channelSurfacewaterChannelNeighborsFlowRate))
        {
          CkError("ERROR in FileManager::handleSendInitializationMessages: channelSurfacewaterChannelNeighborsFlowRate must not be NULL.\n");
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
  
  if (!error)
    {
      // The region array is not created before the end of the mainchare
      // constructor so its proxy is not copied to all PEs by the Charm++ readonly
      // construct.  This sets the correct value on all PEs.
      ADHydro::regionProxy = regionProxy;

      // Only load forcing data if the simulation duration is greater than zero.
      if (currentTime < simulationEndTime)
        {
          error = readForcingData();
        }
      else
        {
          evapoTranspirationForcingInit.dz8w   = 20.0f;
          evapoTranspirationForcingInit.sfcTmp = 300.0f;
          evapoTranspirationForcingInit.sfcPrs = 101300.0f;
          evapoTranspirationForcingInit.psfc   = 101180.0f;
          evapoTranspirationForcingInit.uu     = 0.0f;
          evapoTranspirationForcingInit.vv     = 0.0f;
          evapoTranspirationForcingInit.q2     = 0.0f;
          evapoTranspirationForcingInit.qc     = 0.0f;
          evapoTranspirationForcingInit.solDn  = 800.0f;
          evapoTranspirationForcingInit.lwDn   = 300.0f;
          evapoTranspirationForcingInit.prcp   = 0.0f;
          evapoTranspirationForcingInit.tBot   = 300.0f;
          evapoTranspirationForcingInit.pblh   = 0.0f;
        }
    }
  
  for (ii = 0; !error && ii < localNumberOfMeshElements; ++ii)
    {
      if (currentTime < simulationEndTime)
        {
          evapoTranspirationForcingInit.dz8w   = 20.0f;
          evapoTranspirationForcingInit.sfcTmp = t2[ii] + ZERO_C_IN_KELVIN;
          evapoTranspirationForcingInit.sfcPrs = psfc[ii];
          evapoTranspirationForcingInit.psfc   = psfc[ii] - 120.0f;
          evapoTranspirationForcingInit.uu     = u[ii];
          evapoTranspirationForcingInit.vv     = v[ii];
          evapoTranspirationForcingInit.q2     = qVapor[ii];
          evapoTranspirationForcingInit.qc     = qCloud[ii];
          evapoTranspirationForcingInit.solDn  = swDown[ii];
          evapoTranspirationForcingInit.lwDn   = gLw[ii];
          evapoTranspirationForcingInit.prcp   = ADHydro::drainDownMode ? 0.0f : tPrec[ii];
          evapoTranspirationForcingInit.tBot   = tslb[ii] + ZERO_C_IN_KELVIN;
          evapoTranspirationForcingInit.pblh   = pblh[ii];
        }
      
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
              if (ii + localMeshElementStart == meshMeshNeighbors[ii][jj])
                {
                  CkError("ERROR in FileManager::handleSendInitializationMessages: mesh element %d mesh neighbor %d is a self-neighbor, which is not "
                          "allowed.\n", ii + localMeshElementStart, jj);
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
                  surfacewaterMeshNeighbors.push_back(simpleNeighborInfo(meshSurfacewaterMeshNeighborsExpirationTime[ii][jj],
                      meshSurfacewaterMeshNeighborsFlowRate[ii][jj], meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], meshMeshNeighborsRegion[ii][jj], meshMeshNeighbors[ii][jj],
                      meshMeshNeighborsEdgeLength[ii][jj], meshMeshNeighborsEdgeNormalX[ii][jj], meshMeshNeighborsEdgeNormalY[ii][jj]));
                }

              if (meshGroundwaterMeshNeighborsConnection[ii][jj])
                {
                  groundwaterMeshNeighbors.push_back(simpleNeighborInfo(meshGroundwaterMeshNeighborsExpirationTime[ii][jj],
                      meshGroundwaterMeshNeighborsFlowRate[ii][jj], meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
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
          else if (!(NOFLOW == meshChannelNeighbors[ii][jj]))
            {
              CkError("ERROR in FileManager::handleSendInitializationMessages: mesh element %d channel neighbor %d must be a valid element number.\n", ii, jj);
              error = true;
            }
          
          if (!error && NOFLOW != meshChannelNeighbors[ii][jj])
            {
              if (meshSurfacewaterChannelNeighborsConnection[ii][jj])
                {
                  surfacewaterChannelNeighbors.push_back(simpleNeighborInfo(meshSurfacewaterChannelNeighborsExpirationTime[ii][jj],
                      meshSurfacewaterChannelNeighborsFlowRate[ii][jj], meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
                      meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ii][jj], meshChannelNeighborsRegion[ii][jj], meshChannelNeighbors[ii][jj],
                      meshChannelNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }

              if (meshGroundwaterChannelNeighborsConnection[ii][jj])
                {
                  groundwaterChannelNeighbors.push_back(simpleNeighborInfo(meshGroundwaterChannelNeighborsExpirationTime[ii][jj],
                      meshGroundwaterChannelNeighborsFlowRate[ii][jj], meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
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
              meshTranspirationCumulativeLongTerm[ii], evapoTranspirationForcingInit, meshEvapoTranspirationState[ii], meshGroundwaterMethod[ii],
              meshVadoseZone[ii], surfacewaterMeshNeighbors, surfacewaterChannelNeighbors, groundwaterMeshNeighbors, groundwaterChannelNeighbors);
        }
    }

  for (ii = 0; !error && ii < localNumberOfChannelElements; ++ii)
    {
      if (currentTime < simulationEndTime)
        {
          evapoTranspirationForcingInit.dz8w   = 20.0f;
          evapoTranspirationForcingInit.sfcTmp = t2_c[ii] + ZERO_C_IN_KELVIN;
          evapoTranspirationForcingInit.sfcPrs = psfc_c[ii];
          evapoTranspirationForcingInit.psfc   = psfc_c[ii] - 120.0f;
          evapoTranspirationForcingInit.uu     = u_c[ii];
          evapoTranspirationForcingInit.vv     = v_c[ii];
          evapoTranspirationForcingInit.q2     = qVapor_c[ii];
          evapoTranspirationForcingInit.qc     = qCloud_c[ii];
          evapoTranspirationForcingInit.solDn  = swDown_c[ii];
          evapoTranspirationForcingInit.lwDn   = gLw_c[ii];
          evapoTranspirationForcingInit.prcp   = ADHydro::drainDownMode ? 0.0f : tPrec_c[ii];
          evapoTranspirationForcingInit.tBot   = tslb_c[ii] + ZERO_C_IN_KELVIN;
          evapoTranspirationForcingInit.pblh   = pblh_c[ii];
        }
      
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
          else if (!(NOFLOW == channelMeshNeighbors[ii][jj]))
            {
              CkError("ERROR in FileManager::handleSendInitializationMessages: channel element %d mesh neighbor %d must be a valid element number.\n", ii, jj);
              error = true;
            }
          
          if (!error && NOFLOW != channelMeshNeighbors[ii][jj])
            {
              if (channelSurfacewaterMeshNeighborsConnection[ii][jj])
                {
                  surfacewaterMeshNeighbors.push_back(simpleNeighborInfo(channelSurfacewaterMeshNeighborsExpirationTime[ii][jj],
                      channelSurfacewaterMeshNeighborsFlowRate[ii][jj], channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
                      channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ii][jj], channelMeshNeighborsRegion[ii][jj], channelMeshNeighbors[ii][jj],
                      channelMeshNeighborsEdgeLength[ii][jj], 1.0, 0.0));
                }

              if (channelGroundwaterMeshNeighborsConnection[ii][jj])
                {
                  groundwaterMeshNeighbors.push_back(simpleNeighborInfo(channelGroundwaterMeshNeighborsExpirationTime[ii][jj],
                      channelGroundwaterMeshNeighborsFlowRate[ii][jj], channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[ii][jj],
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
              if (ii + localChannelElementStart == channelChannelNeighbors[ii][jj])
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
              surfacewaterChannelNeighbors.push_back(simpleNeighborInfo(channelSurfacewaterChannelNeighborsExpirationTime[ii][jj],
                  channelSurfacewaterChannelNeighborsFlowRate[ii][jj], channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[ii][jj],
                  channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ii][jj], channelChannelNeighborsRegion[ii][jj],
                  channelChannelNeighbors[ii][jj], 1.0, 1.0, 0.0));
            }
        }

      if (!error)
        {
          regionProxy[channelRegion[ii]].sendInitializeChannelElement(ii + localChannelElementStart, channelChannelType[ii], channelReachCode[ii],
              channelElementX[ii], channelElementY[ii], channelElementZBank[ii], channelElementZBed[ii], channelElementLength[ii], channelLatitude[ii],
              channelLongitude[ii], channelBaseWidth[ii], channelSideSlope[ii], channelBedConductivity[ii], channelBedThickness[ii], channelManningsN[ii],
              channelSurfacewaterDepth[ii], channelSurfacewaterError[ii], channelPrecipitationRate[ii], channelPrecipitationCumulativeShortTerm[ii],
              channelPrecipitationCumulativeLongTerm[ii], channelEvaporationRate[ii], channelEvaporationCumulativeShortTerm[ii],
              channelEvaporationCumulativeLongTerm[ii], evapoTranspirationForcingInit, channelEvapoTranspirationState[ii], surfacewaterMeshNeighbors,
              surfacewaterChannelNeighbors, groundwaterMeshNeighbors);
        }
    }

  if (error)
    {
      CkExit();
    }
}

bool FileManager::allStateUpdated()
{
  int  ii;             // Loop counter.
  bool updated = true; // Flag to record whether we have found an unupdated vertex.
  
  for (ii = 0; updated && ii < localNumberOfMeshElements; ++ii)
    {
      updated = meshElementUpdated[ii];
    }
  
  for (ii = 0; updated && ii < localNumberOfChannelElements; ++ii)
    {
      updated = channelElementUpdated[ii];
    }
  
  return updated;
}

void FileManager::handleMeshElementStateMessage(ElementStateMessage& message)
{
  int                                       ii; // Loop counter.
  std::vector<simpleNeighborInfo>::iterator it; // Loop iterator.
  
  // FIXME error check inputs
  
  meshSurfacewaterDepth[               message.elementNumber - localMeshElementStart] = message.surfacewaterDepth;
  meshSurfacewaterError[               message.elementNumber - localMeshElementStart] = message.surfacewaterError;
  meshGroundwaterHead[                 message.elementNumber - localMeshElementStart] = message.groundwaterHead;
  meshGroundwaterRecharge[             message.elementNumber - localMeshElementStart] = message.groundwaterRecharge;
  meshGroundwaterError[                message.elementNumber - localMeshElementStart] = message.groundwaterError;
  meshPrecipitationRate[               message.elementNumber - localMeshElementStart] = message.precipitationRate;
  meshPrecipitationCumulativeShortTerm[message.elementNumber - localMeshElementStart] = message.precipitationCumulativeShortTerm;
  meshPrecipitationCumulativeLongTerm[ message.elementNumber - localMeshElementStart] = message.precipitationCumulativeLongTerm;
  meshEvaporationRate[                 message.elementNumber - localMeshElementStart] = message.evaporationRate;
  meshEvaporationCumulativeShortTerm[  message.elementNumber - localMeshElementStart] = message.evaporationCumulativeShortTerm;
  meshEvaporationCumulativeLongTerm[   message.elementNumber - localMeshElementStart] = message.evaporationCumulativeLongTerm;
  meshTranspirationRate[               message.elementNumber - localMeshElementStart] = message.transpirationRate;
  meshTranspirationCumulativeShortTerm[message.elementNumber - localMeshElementStart] = message.transpirationCumulativeShortTerm;
  meshTranspirationCumulativeLongTerm[ message.elementNumber - localMeshElementStart] = message.transpirationCumulativeLongTerm;
  meshEvapoTranspirationState[         message.elementNumber - localMeshElementStart] = message.evapoTranspirationState;
  
  for (it = message.surfacewaterMeshNeighbors.begin(); it != message.surfacewaterMeshNeighbors.end(); ++it)
    {
      for (ii = 0; ii < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; ++ii)
        {
          if (meshMeshNeighbors[message.elementNumber - localMeshElementStart][ii] == it->neighbor)
            {
              meshSurfacewaterMeshNeighborsExpirationTime[         message.elementNumber - localMeshElementStart][ii] = it->expirationTime;
              meshSurfacewaterMeshNeighborsFlowRate[               message.elementNumber - localMeshElementStart][ii] = it->nominalFlowRate;
              meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm[message.elementNumber - localMeshElementStart][ii] = it->flowCumulativeShortTerm;
              meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ message.elementNumber - localMeshElementStart][ii] = it->flowCumulativeLongTerm;
              break;
            }
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      if (!(MESH_ELEMENT_MESH_NEIGHBORS_SIZE > ii))
        {
          CkError("ERROR in FileManager::handleMeshElementStateMessage: Mesh element %d does not have %d as a mesh neighbor.\n", message.elementNumber, it->neighbor);
          CkExit();
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  for (it = message.groundwaterMeshNeighbors.begin(); it != message.groundwaterMeshNeighbors.end(); ++it)
    {
      for (ii = 0; ii < MESH_ELEMENT_MESH_NEIGHBORS_SIZE; ++ii)
        {
          if (meshMeshNeighbors[message.elementNumber - localMeshElementStart][ii] == it->neighbor)
            {
              meshGroundwaterMeshNeighborsExpirationTime[         message.elementNumber - localMeshElementStart][ii] = it->expirationTime;
              meshGroundwaterMeshNeighborsFlowRate[               message.elementNumber - localMeshElementStart][ii] = it->nominalFlowRate;
              meshGroundwaterMeshNeighborsFlowCumulativeShortTerm[message.elementNumber - localMeshElementStart][ii] = it->flowCumulativeShortTerm;
              meshGroundwaterMeshNeighborsFlowCumulativeLongTerm[ message.elementNumber - localMeshElementStart][ii] = it->flowCumulativeLongTerm;
              break;
            }
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      if (!(MESH_ELEMENT_MESH_NEIGHBORS_SIZE > ii))
        {
          CkError("ERROR in FileManager::handleMeshElementStateMessage: Mesh element %d does not have %d as a mesh neighbor.\n", message.elementNumber, it->neighbor);
          CkExit();
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  for (it = message.surfacewaterChannelNeighbors.begin(); it != message.surfacewaterChannelNeighbors.end(); ++it)
    {
      for (ii = 0; ii < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; ++ii)
        {
          if (meshChannelNeighbors[message.elementNumber - localMeshElementStart][ii] == it->neighbor)
            {
              meshSurfacewaterChannelNeighborsExpirationTime[         message.elementNumber - localMeshElementStart][ii] = it->expirationTime;
              meshSurfacewaterChannelNeighborsFlowRate[               message.elementNumber - localMeshElementStart][ii] = it->nominalFlowRate;
              meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm[message.elementNumber - localMeshElementStart][ii] = it->flowCumulativeShortTerm;
              meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ message.elementNumber - localMeshElementStart][ii] = it->flowCumulativeLongTerm;
              break;
            }
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      if (!(MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE > ii))
        {
          CkError("ERROR in FileManager::handleMeshElementStateMessage: Mesh element %d does not have %d as a channel neighbor.\n", message.elementNumber, it->neighbor);
          CkExit();
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  for (it = message.groundwaterChannelNeighbors.begin(); it != message.groundwaterChannelNeighbors.end(); ++it)
    {
      for (ii = 0; ii < MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE; ++ii)
        {
          if (meshChannelNeighbors[message.elementNumber - localMeshElementStart][ii] == it->neighbor)
            {
              meshGroundwaterChannelNeighborsExpirationTime[         message.elementNumber - localMeshElementStart][ii] = it->expirationTime;
              meshGroundwaterChannelNeighborsFlowRate[               message.elementNumber - localMeshElementStart][ii] = it->nominalFlowRate;
              meshGroundwaterChannelNeighborsFlowCumulativeShortTerm[message.elementNumber - localMeshElementStart][ii] = it->flowCumulativeShortTerm;
              meshGroundwaterChannelNeighborsFlowCumulativeLongTerm[ message.elementNumber - localMeshElementStart][ii] = it->flowCumulativeLongTerm;
              break;
            }
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      if (!(MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE > ii))
        {
          CkError("ERROR in FileManager::handleMeshElementStateMessage: Mesh element %d does not have %d as a channel neighbor.\n", message.elementNumber, it->neighbor);
          CkExit();
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  meshElementUpdated[message.elementNumber - localMeshElementStart] = true;
}

void FileManager::handleChannelElementStateMessage(ElementStateMessage& message)
{
  int                                       ii; // Loop counter.
  std::vector<simpleNeighborInfo>::iterator it; // Loop iterator.
  
  // FIXME error check inputs
  
  channelSurfacewaterDepth[               message.elementNumber - localChannelElementStart] = message.surfacewaterDepth;
  channelSurfacewaterError[               message.elementNumber - localChannelElementStart] = message.surfacewaterError;
  channelPrecipitationRate[               message.elementNumber - localChannelElementStart] = message.precipitationRate;
  channelPrecipitationCumulativeShortTerm[message.elementNumber - localChannelElementStart] = message.precipitationCumulativeShortTerm;
  channelPrecipitationCumulativeLongTerm[ message.elementNumber - localChannelElementStart] = message.precipitationCumulativeLongTerm;
  channelEvaporationRate[                 message.elementNumber - localChannelElementStart] = message.evaporationRate;
  channelEvaporationCumulativeShortTerm[  message.elementNumber - localChannelElementStart] = message.evaporationCumulativeShortTerm;
  channelEvaporationCumulativeLongTerm[   message.elementNumber - localChannelElementStart] = message.evaporationCumulativeLongTerm;
  channelEvapoTranspirationState[         message.elementNumber - localChannelElementStart] = message.evapoTranspirationState;
  
  for (it = message.surfacewaterMeshNeighbors.begin(); it != message.surfacewaterMeshNeighbors.end(); ++it)
    {
      for (ii = 0; ii < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; ++ii)
        {
          if (channelMeshNeighbors[message.elementNumber - localChannelElementStart][ii] == it->neighbor)
            {
              channelSurfacewaterMeshNeighborsExpirationTime[         message.elementNumber - localChannelElementStart][ii] = it->expirationTime;
              channelSurfacewaterMeshNeighborsFlowRate[               message.elementNumber - localChannelElementStart][ii] = it->nominalFlowRate;
              channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm[message.elementNumber - localChannelElementStart][ii] = it->flowCumulativeShortTerm;
              channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm[ message.elementNumber - localChannelElementStart][ii] = it->flowCumulativeLongTerm;
              break;
            }
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      if (!(CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE > ii))
        {
          CkError("ERROR in FileManager::handleMeshElementStateMessage: Channel element %d does not have %d as a mesh neighbor.\n", message.elementNumber, it->neighbor);
          CkExit();
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  for (it = message.groundwaterMeshNeighbors.begin(); it != message.groundwaterMeshNeighbors.end(); ++it)
    {
      for (ii = 0; ii < CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE; ++ii)
        {
          if (channelMeshNeighbors[message.elementNumber - localChannelElementStart][ii] == it->neighbor)
            {
              channelGroundwaterMeshNeighborsExpirationTime[         message.elementNumber - localChannelElementStart][ii] = it->expirationTime;
              channelGroundwaterMeshNeighborsFlowRate[               message.elementNumber - localChannelElementStart][ii] = it->nominalFlowRate;
              channelGroundwaterMeshNeighborsFlowCumulativeShortTerm[message.elementNumber - localChannelElementStart][ii] = it->flowCumulativeShortTerm;
              channelGroundwaterMeshNeighborsFlowCumulativeLongTerm[ message.elementNumber - localChannelElementStart][ii] = it->flowCumulativeLongTerm;
              break;
            }
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      if (!(CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE > ii))
        {
          CkError("ERROR in FileManager::handleMeshElementStateMessage: Channel element %d does not have %d as a mesh neighbor.\n", message.elementNumber, it->neighbor);
          CkExit();
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  for (it = message.surfacewaterChannelNeighbors.begin(); it != message.surfacewaterChannelNeighbors.end(); ++it)
    {
      for (ii = 0; ii < CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE; ++ii)
        {
          if (channelChannelNeighbors[message.elementNumber - localChannelElementStart][ii] == it->neighbor)
            {
              channelSurfacewaterChannelNeighborsExpirationTime[         message.elementNumber - localChannelElementStart][ii] = it->expirationTime;
              channelSurfacewaterChannelNeighborsFlowRate[               message.elementNumber - localChannelElementStart][ii] = it->nominalFlowRate;
              channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm[message.elementNumber - localChannelElementStart][ii] = it->flowCumulativeShortTerm;
              channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm[ message.elementNumber - localChannelElementStart][ii] = it->flowCumulativeLongTerm;
              break;
            }
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      if (!(CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE > ii))
        {
          CkError("ERROR in FileManager::handleMeshElementStateMessage: channel element %d does not have %d as a channel neighbor.\n", message.elementNumber, it->neighbor);
          CkExit();
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  channelElementUpdated[message.elementNumber - localChannelElementStart] = true;
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "file_manager.def.h"
#pragma GCC diagnostic warning "-Wunused-variable"
