#ifndef __REGION_H__
#define __REGION_H__

#include "mesh_element.h"
#include "channel_element.h"
#include "element.h"

// Because I'm passing RegionMessage as a parameter in Charm++ messages I have
// to have a forward declaration of it before including region.decl.h.  I can't
// have a forward declaration of it if it is defined within the scope of the
// Region class.  Therefore, I am just defining it here.
enum RegionMessageTypeEnum
{
  MESH_SURFACEWATER_MESH_NEIGHBOR,       // A message sent to a mesh    element containing surfacewater info from its neighbor that is a mesh    element.
  MESH_SURFACEWATER_CHANNEL_NEIGHBOR,    // A message sent to a mesh    element containing surfacewater info from its neighbor that is a channel element.
  MESH_GROUNDWATER_MESH_NEIGHBOR,        // A message sent to a mesh    element containing groundwater  info from its neighbor that is a mesh    element.
  MESH_GROUNDWATER_CHANNEL_NEIGHBOR,     // A message sent to a mesh    element containing groundwater  info from its neighbor that is a channel element.
  CHANNEL_SURFACEWATER_MESH_NEIGHBOR,    // A message sent to a channel element containing surfacewater info from its neighbor that is a mesh    element.
  CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR, // A message sent to a channel element containing surfacewater info from its neighbor that is a channel element.
  CHANNEL_GROUNDWATER_MESH_NEIGHBOR,     // A message sent to a channel element containing groundwater  info from its neighbor that is a mesh    element.
};

PUPbytes(RegionMessageTypeEnum);

// This struct holds a neighbor element's state being sent to an element for
// calculating the nominal flow rate between them or inflow water being sent
// to an element.
class RegionMessage
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  RegionMessage();
  
  // Constructor.
  //
  // All parameters directly initialize member variables.  For description see
  // member variables.
  RegionMessage(RegionMessageTypeEnum messageTypeInit, int recipientElementNumberInit, int recipientNeighborProxyIndexInit,
                double senderSurfacewaterDepthInit, double senderGroundwaterHeadInit, SimpleNeighborProxy::MaterialTransfer waterInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  RegionMessageTypeEnum                 messageType;                 // The types of the element, neighbor, and interaction.
  int                                   recipientElementNumber;      // The element ID number of the message recipient.
  int                                   recipientNeighborProxyIndex; // The index in the recipient's neighbor proxy vector of the proxy for the sender.
  double                                senderSurfacewaterDepth;     // The surfacewater depth of the sender in meters.  Unused for water messages.
  double                                senderGroundwaterHead;       // The groundwater head of the sender in meters.  Unused for surfacewater state messages
                                                                     // or water messages.
  SimpleNeighborProxy::MaterialTransfer water;                       // The water sent to the recipient.  Unused for state messages.
};

// A simpleNeighborInfo is used to tell an element what neighbors it has.  It
// contains less information than the neighborProxies because elements get that
// extra information from their neighbors.  simpleNeighborInfo just has enough
// information for the element to get in touch with its neighbors to start the
// information exchange.  The one class simpleNeighborInfo is used for all four
// types of neighbor relationships.
class simpleNeighborInfo
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  simpleNeighborInfo();
  
  // Constructor.
  //
  // All parameters directly initialize member variables.  For description see
  // member variables.
  simpleNeighborInfo(double expirationTimeInit, double nominalFlowRateInit, double flowCumulativeShortTermInit,
                     double flowCumulativeLongTermInit, int regionInit, int neighborInit, double edgeLengthInit, double edgeNormalXInit,
                     double edgeNormalYInit, bool downstreamInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  // Cumulative flow variables.
  double expirationTime;          // Simulation time in seconds since Element::referenceDate when nominal flow rate expires.
  double nominalFlowRate;         // Material flow rate in quantity per second.  Positive means flow out of the element into the neighbor.  Negative means flow
                                  // into the element out of the neighbor.  Actual flows may be less than this if the sender does not have enough material to
                                  // satisfy all outflows.
  double flowCumulativeShortTerm; // flowCumulativeShortTerm plus flowCumulativeLongTerm together are the cumulative material flow quantity with the neighbor.
  double flowCumulativeLongTerm;  // Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.  Two
                                  // variables are used because over a long simulation this value could become quite large, and the amount of material moved
                                  // each timestep can be quite small so roundoff error becomes a concern.  New flow is added to flowCumulativeShortTerm each
                                  // timestep, and occasionally flowCumulativeShortTerm is added to flowCumulativeLongTerm and flowCumulativeShortTerm is reset
                                  // to zero.
  
  // Identification parameters.
  int region;   // Region number where the neighbor is.
  int neighbor; // Element ID number of the neighbor or boundary condition code.
  
  // Geometric coordinates and relationships.
  double edgeLength;  // Meters.  2-D distance ignoring Z coordinates.
  double edgeNormalX; // X component of normal unit vector, unitless.
  double edgeNormalY; // Y component of normal unit vector, unitless.
  bool   downstream;  // Whether the neighbor is downstream of the element.  Only used for channel-channel neighbors.
};

#include "region.decl.h"

// A Region is a collection of Elements.  Within a Region, Elements can
// interact directly by accessing each others' member functions and variables.
// Only communication between Regions requires Charm++ messages.
//
// A Region handles stepping its Elements forward in time and uses the same
// timestep for all Elements in the Region.  Therefore, the Region object can
// have one copy of the time variables and individual mesh and channel elements
// don't need to have separate time variables.  Since the only things in the
// Element class are time variables MeshElement and ChannelElement don't
// inherit from Element.  Instead, Region does.
class Region : public CBase_Region, public Element
{
  Region_SDAG_CODE
  
public:
  
  // We were getting large Z coordinate differences between mesh elements and
  // neighboring channel elements because if an element is sloped, the Z
  // coordinate at the center of the mesh element may not be representative of
  // the Z coordinate at the edge of the mesh element where it connects to the
  // neighboring channel element.  Depending on how the mesh element is sloped
  // this can even result in the channel element being higher than the mesh
  // element even though water should be flowing from the mesh element into the
  // channel element.
  //
  // To solve this, we calculate a Z offset by following the slope of the mesh
  // element to the point where it connects to the channel element.  This point
  // can be the center of the channel element or the point on the edge of the
  // mesh element in the direction of the center of the channel element.
  //
  // If the channel element is still higher than the mesh element after
  // applying the offset we raise the offset to make them level.  We do not do
  // this for icemasses because there are often real situations where a glacier
  // on a slope is higher than its neighboring mesh elements.
  //
  // The Z offset should be added to the Z coordinate of the mesh element
  // center for all of its interactions with this channel neighbor.
  //
  // Returns: The Z coordinate offset in meters.
  //
  // Parameters:
  //
  // meshElement         - Element number of mesh element.
  // meshVertexX         - Array of X coordinates in meters of mesh element
  //                       vertices.
  // meshVertexY         - Array of Y coordinates in meters of mesh element
  //                       vertices.
  // meshElementX        - X coordinate in meters of center of mesh element.
  // meshElementY        - Y coordinate in meters of center of mesh element.
  // meshElementZSurface - Surface Z coordinate in meters of center of mesh
  //                       element.
  // meshElementSlopeX   - Slope of mesh element in X direction, unitless.
  // meshElementSlopeY   - Slope of mesh element in Y direction, unitless.
  // channelElement      - Element number of channel element.
  // channelElementX     - X coordinate in meters of center of channel element.
  // channelElementY     - Y coordinate in meters of center of channel element.
  // channelElementZBank - Bank Z coordinate in meters of center of channel
  //                       element.
  // channelType         - Channel type of channel element.
  static double calculateZOffset(int meshElement, double meshVertexX[3], double meshVertexY[3], double meshElementX, double meshElementY,
                                 double meshElementZSurface, double meshElementSlopeX, double meshElementSlopeY, int channelElement, double channelElementX,
                                 double channelElementY, double channelElementZBank, ChannelTypeEnum channelType);
  
  // Constructor.  timestepEndTime is initialized to be currentTimeInit
  // indicating that a new value for timestepEndTime needs to be selected in
  // step 2 of the simulation.  nextSyncTime is also initialized to be
  // currentTimeInit so that the element will sync up with other elements to do
  // output, check mass balance, and check invariant at the beginning of the
  // simulation.
  //
  // meshElements, channelElements, and outgoingMessages are initialized to
  // empty.  All iterators are initialized to invalid.
  //
  // FIXME what is regionalDtLimit initialized to?
  //
  // simulationFinished is initialized to false.
  //
  // Parameters:
  //
  // referenceDateInit     - Julian date to initialize referenceDate.
  // currentTimeInit       - Simulation time in seconds since referenceDateInit
  //                         to initialize currentTime and timestepEndTime.
  // simulationEndTimeInit - Simulation time in seconds since referenceDateInit
  //                         to initialize simulationEndTime and nextSyncTime.
  Region(double referenceDateInit, double currentTimeInit, double simulationEndTimeInit);
  Region(){}
  // Charm++ migration constructor.
  //
  // Parameters:
  //
  // msg - Charm++ migration message.
  Region(CkMigrateMessage* msg);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  // Check invariant conditions on a mesh element's proxy for its neighbor
  // that is a mesh element.  Exit if the invariant is violated.
  //
  // Parameters:
  //
  // messageTime    - Simulation time that this message represents.  Because of
  //                  asynchronous operation an element might receive a
  //                  NeighborCheckInvariant message before it reaches that
  //                  simulation time.  If so, it must wait until it reaches
  //                  that simulation time to check if it is consistent with
  //                  that neighbor.
  // neighbor       - Element number of the neighbor.
  // neighborsProxy - The neighbor's neighbor proxy for me.  This contains all
  //                  of the information that must be compared to me to ensure
  //                  that I am consistent with my neighbor.
  void handleMeshSurfacewaterMeshNeighborCheckInvariant(double messageTime, int neighbor, MeshSurfacewaterMeshNeighborProxy& neighborsProxy);
  
  // Check invariant conditions on a mesh element's proxy for its neighbor
  // that is a channel element.  Exit if the invariant is violated.
  //
  // Parameters:
  //
  // messageTime    - Simulation time that this message represents.  Because of
  //                  asynchronous operation an element might receive a
  //                  NeighborCheckInvariant message before it reaches that
  //                  simulation time.  If so, it must wait until it reaches
  //                  that simulation time to check if it is consistent with
  //                  that neighbor.
  // neighbor       - Element number of the neighbor.
  // neighborsProxy - The neighbor's neighbor proxy for me.  This contains all
  //                  of the information that must be compared to me to ensure
  //                  that I am consistent with my neighbor.
  void handleMeshSurfacewaterChannelNeighborCheckInvariant(double messageTime, int neighbor, ChannelSurfacewaterMeshNeighborProxy& neighborsProxy);
  
  // Check invariant conditions on a mesh element's proxy for its neighbor
  // that is a mesh element.  Exit if the invariant is violated.
  //
  // Parameters:
  //
  // messageTime    - Simulation time that this message represents.  Because of
  //                  asynchronous operation an element might receive a
  //                  NeighborCheckInvariant message before it reaches that
  //                  simulation time.  If so, it must wait until it reaches
  //                  that simulation time to check if it is consistent with
  //                  that neighbor.
  // neighbor       - Element number of the neighbor.
  // neighborsProxy - The neighbor's neighbor proxy for me.  This contains all
  //                  of the information that must be compared to me to ensure
  //                  that I am consistent with my neighbor.
  void handleMeshGroundwaterMeshNeighborCheckInvariant(double messageTime, int neighbor, MeshGroundwaterMeshNeighborProxy& neighborsProxy);
  
  // Check invariant conditions on a mesh element's proxy for its neighbor
  // that is a channel element.  Exit if the invariant is violated.
  //
  // Parameters:
  //
  // messageTime    - Simulation time that this message represents.  Because of
  //                  asynchronous operation an element might receive a
  //                  NeighborCheckInvariant message before it reaches that
  //                  simulation time.  If so, it must wait until it reaches
  //                  that simulation time to check if it is consistent with
  //                  that neighbor.
  // neighbor       - Element number of the neighbor.
  // neighborsProxy - The neighbor's neighbor proxy for me.  This contains all
  //                  of the information that must be compared to me to ensure
  //                  that I am consistent with my neighbor.
  void handleMeshGroundwaterChannelNeighborCheckInvariant(double messageTime, int neighbor, ChannelGroundwaterMeshNeighborProxy& neighborsProxy);
  
  // Check invariant conditions on a channel element's proxy for its neighbor
  // that is a mesh element.  Exit if the invariant is violated.
  //
  // Parameters:
  //
  // messageTime    - Simulation time that this message represents.  Because of
  //                  asynchronous operation an element might receive a
  //                  NeighborCheckInvariant message before it reaches that
  //                  simulation time.  If so, it must wait until it reaches
  //                  that simulation time to check if it is consistent with
  //                  that neighbor.
  // neighbor       - Element number of the neighbor.
  // neighborsProxy - The neighbor's neighbor proxy for me.  This contains all
  //                  of the information that must be compared to me to ensure
  //                  that I am consistent with my neighbor.
  void handleChannelSurfacewaterMeshNeighborCheckInvariant(double messageTime, int neighbor, MeshSurfacewaterChannelNeighborProxy& neighborsProxy);
  
  // Check invariant conditions on a channel element's proxy for its neighbor
  // that is a channel element.  Exit if the invariant is violated.
  //
  // Parameters:
  //
  // messageTime    - Simulation time that this message represents.  Because of
  //                  asynchronous operation an element might receive a
  //                  NeighborCheckInvariant message before it reaches that
  //                  simulation time.  If so, it must wait until it reaches
  //                  that simulation time to check if it is consistent with
  //                  that neighbor.
  // neighbor       - Element number of the neighbor.
  // neighborsProxy - The neighbor's neighbor proxy for me.  This contains all
  //                  of the information that must be compared to me to ensure
  //                  that I am consistent with my neighbor.
  void handleChannelSurfacewaterChannelNeighborCheckInvariant(double messageTime, int neighbor, ChannelSurfacewaterChannelNeighborProxy& neighborsProxy);
  
  // Check invariant conditions on a channel element's proxy for its neighbor
  // that is a mesh element.  Exit if the invariant is violated.
  //
  // Parameters:
  //
  // messageTime    - Simulation time that this message represents.  Because of
  //                  asynchronous operation an element might receive a
  //                  NeighborCheckInvariant message before it reaches that
  //                  simulation time.  If so, it must wait until it reaches
  //                  that simulation time to check if it is consistent with
  //                  that neighbor.
  // neighbor       - Element number of the neighbor.
  // neighborsProxy - The neighbor's neighbor proxy for me.  This contains all
  //                  of the information that must be compared to me to ensure
  //                  that I am consistent with my neighbor.
  void handleChannelGroundwaterMeshNeighborCheckInvariant(double messageTime, int neighbor, MeshGroundwaterChannelNeighborProxy& neighborsProxy);
  
  // Returns: true if all neighbor invariants have been checked, false
  //          otherwise.
  bool allNeighborInvariantsChecked();
  
  // Initialize a new mesh element in this region.
  //
  // Parameters:
  //
  // Most parameters directly initialize MeshElement member variables.  For
  // description see member variables.
  //
  // surfacewaterMeshNeighbors    - Surfacewater neighbors of the new element
  //                                that are mesh elements.
  // surfacewaterChannelNeighbors - Surfacewater neighbors of the new element
  //                                that are channel elements.
  // groundwaterMeshNeighbors     - Groundwater neighbors of the new element
  //                                that are mesh elements.
  // groundwaterChannelNeighbors  - Groundwater neighbors of the new element
  //                                that are channel elements.
  void handleInitializeMeshElement(int elementNumberInit, int catchmentInit, int vegetationTypeInit, int soilTypeInit, double vertexXInit[3],
                                   double vertexYInit[3], double elementXInit, double elementYInit, double elementZSurfaceInit, double layerZBottomInit,
                                   double elementAreaInit, double slopeXInit, double slopeYInit, double latitudeInit, double longitudeInit,
                                   double manningsNInit, double conductivityInit, double porosityInit, double surfacewaterDepthInit,
                                   double surfacewaterErrorInit, double groundwaterHeadInit, double groundwaterRechargeInit, double groundwaterErrorInit,
                                   double precipitationRateInit, double precipitationCumulativeShortTermInit, double precipitationCumulativeLongTermInit,
                                   double evaporationRateInit, double evaporationCumulativeShortTermInit, double evaporationCumulativeLongTermInit,
                                   double transpirationRateInit, double transpirationCumulativeShortTermInit, double transpirationCumulativeLongTermInit,
                                   EvapoTranspirationForcingStruct& evapoTranspirationForcingInit, double nextForcingDataTimeInit,
                                   EvapoTranspirationStateStruct& evapoTranspirationStateInit,
                                   InfiltrationAndGroundwater::GroundwaterMethodEnum groundwaterMethodInit,
                                   InfiltrationAndGroundwater::VadoseZone vadoseZoneInit, std::vector<simpleNeighborInfo> surfacewaterMeshNeighbors,
                                   std::vector<simpleNeighborInfo> surfacewaterChannelNeighbors,
                                   std::vector<simpleNeighborInfo> groundwaterMeshNeighbors, std::vector<simpleNeighborInfo> groundwaterChannelNeighbors);
  
  // Initialize a new channel element in this region.
  //
  // Parameters:
  //
  // Most parameters directly initialize ChannelElement member variables.  For
  // description see member variables.
  //
  // surfacewaterMeshNeighbors    - Surfacewater neighbors of the new element
  //                                that are mesh elements.
  // surfacewaterChannelNeighbors - Surfacewater neighbors of the new element
  //                                that are channel elements.
  // groundwaterMeshNeighbors     - Groundwater neighbors of the new element
  //                                that are mesh elements.
  void handleInitializeChannelElement(int elementNumberInit, ChannelTypeEnum channelTypeInit, long long reachCodeInit, int streamOrderInit, double elementXInit,
                                      double elementYInit, double elementZBankInit, double elementZBedInit, double elementLengthInit,
                                      double latitudeInit, double longitudeInit, double baseWidthInit, double sideSlopeInit, double bedConductivityInit,
                                      double bedThicknessInit, double manningsNInit, double surfacewaterDepthInit, double surfacewaterErrorInit,
                                      double precipitationRateInit, double precipitationCumulativeShortTermInit,
                                      double precipitationCumulativeLongTermInit, double evaporationRateInit, double evaporationCumulativeShortTermInit,
                                      double evaporationCumulativeLongTermInit, EvapoTranspirationForcingStruct& evapoTranspirationForcingInit,
                                      double nextForcingDataTimeInit, EvapoTranspirationStateStruct& evapoTranspirationStateInit,
                                      std::vector<simpleNeighborInfo> surfacewaterMeshNeighbors,
                                      std::vector<simpleNeighborInfo> surfacewaterChannelNeighbors,
                                      std::vector<simpleNeighborInfo> groundwaterMeshNeighbors);
  
  // Initialize neighbor information for a mesh element's surfacewater
  // neighbor that is a mesh element.
  //
  // Parameters:
  //
  // element                 - The element to add neighbor information to.
  // neighbor                - The element that the neighbor information is
  //                           from.
  // reciprocalNeighborProxy - The index of element in neighbor's neighbor
  //                           proxy array.
  //
  // Other parameters come from MeshElement member variables of neighbor.
  // For description see member variables.
  void handleMeshSurfacewaterMeshNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, double neighborX, double neighborY,
                                                     double neighborZSurface, double neighborArea, double neighborManningsN);
  
  // Initialize neighbor information for a mesh element's surfacewater
  // neighbor that is a channel element.
  //
  // Parameters:
  //
  // element                 - The element to add neighbor information to.
  // neighbor                - The element that the neighbor information is
  //                           from.
  // reciprocalNeighborProxy - The index of element in neighbor's neighbor
  //                           proxy array.
  //
  // Other parameters come from ChannelElement member variables of neighbor.
  // For description see member variables.
  void handleMeshSurfacewaterChannelNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, ChannelTypeEnum neighborChannelType,
                                                        double neighborX, double neighborY, double neighborZBank, double neighborZBed,
                                                        double neighborBaseWidth, double neighborSideSlope);
  
  // Initialize neighbor information for a mesh element's groundwater
  // neighbor that is a mesh element.
  //
  // Parameters:
  //
  // element                 - The element to add neighbor information to.
  // neighbor                - The element that the neighbor information is
  //                           from.
  // reciprocalNeighborProxy - The index of element in neighbor's neighbor
  //                           proxy array.
  //
  // Other parameters come from MeshElement member variables of neighbor.
  // For description see member variables.
  void handleMeshGroundwaterMeshNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, double neighborX, double neighborY,
                                                    double neighborZSurface, double neighborLayerZBottom, double neighborArea,
                                                    double neighborConductivity, double neighborPorosity);
  
  // Initialize neighbor information for a mesh element's groundwater
  // neighbor that is a channel element.
  //
  // Parameters:
  //
  // element                 - The element to add neighbor information to.
  // neighbor                - The element that the neighbor information is
  //                           from.
  // reciprocalNeighborProxy - The index of element in neighbor's neighbor
  //                           proxy array.
  //
  // Other parameters come from ChannelElement member variables of neighbor.
  // For description see member variables.
  void handleMeshGroundwaterChannelNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, ChannelTypeEnum neighborChannelType,
                                                       double neighborX, double neighborY, double neighborZBank, double neighborZBed,
                                                       double neighborBaseWidth, double neighborSideSlope, double neighborBedConductivity,
                                                       double neighborBedThickness);
  
  // Initialize neighbor information for a channel element's surfacewater
  // neighbor that is a mesh element.
  //
  // Parameters:
  //
  // element                 - The element to add neighbor information to.
  // neighbor                - The element that the neighbor information is
  //                           from.
  // reciprocalNeighborProxy - The index of element in neighbor's neighbor
  //                           proxy array.
  //
  // Other parameters come from MeshElement member variables of neighbor.
  // For description see member variables.
  void handleChannelSurfacewaterMeshNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, double neighborVertexX[3],
                                                        double neighborVertexY[3], double neighborX, double neighborY, double neighborZSurface,
                                                        double neighborArea, double neighborSlopeX, double neighborSlopeY);
  
  // Initialize neighbor information for a channel element's surfacewater
  // neighbor that is a channel element.
  //
  // Parameters:
  //
  // element                 - The element to add neighbor information to.
  // neighbor                - The element that the neighbor information is
  //                           from.
  // reciprocalNeighborProxy - The index of element in neighbor's neighbor
  //                           proxy array.
  //
  // Other parameters come from ChannelElement member variables of neighbor.
  // For description see member variables.
  void handleChannelSurfacewaterChannelNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, ChannelTypeEnum neighborChannelType,
                                                           double neighborZBank, double neighborZBed, double neighborLength, double neighborBaseWidth,
                                                           double neighborSideSlope, double neighborManningsN, bool inflowOnly);
  
  // Initialize neighbor information for a channel element's groundwater
  // neighbor that is a mesh element.
  //
  // Parameters:
  //
  // element                 - The element to add neighbor information to.
  // neighbor                - The element that the neighbor information is
  //                           from.
  // reciprocalNeighborProxy - The index of element in neighbor's neighbor
  //                           proxy array.
  //
  // Other parameters come from MeshElement member variables of neighbor.
  // For description see member variables.
  void handleChannelGroundwaterMeshNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, double neighborVertexX[3],
                                                       double neighborVertexY[3], double neighborX, double neighborY, double neighborZSurface,
                                                       double neighborLayerZBottom, double neighborSlopeX, double neighborSlopeY);
  
  // FIXME comment
  void handleDiversionReleaseRecipientInitMessage(int element, int neighbor, int neighborRegion, ChannelTypeEnum neighborChannelType, double neighborX,
                                                  double neighborY, double neighborZBank, double neighborZBed, double neighborBaseWidth,
                                                  double neighborSideSlope);
  
  // FIXME comment
  void handleDiversionReleaseRecipientInitializedMessage(int element, int neighbor, int neighborRegion, int reciprocalNeighborProxy);
  
  // Returns: true if all neighbor proxies have been initialized, false
  //          otherwise.
  bool allNeighborsInitialized();
  
  // FIXME comment
  void checkIfMyDiversionsInitialized();
  
  // For external nominal flow rates that have expired send the element's state
  // to the neighbor.
  void sendStateToExternalNeighbors();
  
  // Calculate internal nominal flow rates.  Initialize iterators and other
  // variables for the incremental scan in allNominalFlowRatesCalculated.
  void handleCalculateNominalFlowRatesForInternalNeighbors();
  
  // Scan neighbor proxies to see if all nominal flow rates are calculated.
  // This function uses the itMesh and itChannel member variable iterators to
  // incrementally scan from where the last scan ended.  This scan also finds
  // the minimum of nextSyncTime and all nominal flow rate expiration times,
  // which is selected as the next timestep end time.
  //
  // Returns: true if all nominal flow rates have been calculated, false
  //          otherwise.
  bool allNominalFlowRatesCalculated();
  
  // Receive a vector of state messages and for each message calculate the
  // nominal flow rate for the recipient neighbor proxy.
  //
  // Parameters:
  //
  // senderCurrentTime     - Simulation time when state occured at the sender
  //                         in seconds since referenceDate.
  // senderRegionalDtLimit - Maximum duration in seconds for expiration times
  //                         in the neighboring region.
  // stateMessages         - Vector of state messages from neighbors of
  //                         elements in this region.
  void processStateMessages(double senderCurrentTime, double senderRegionalDtLimit, std::vector<RegionMessage>& stateMessages);
  
  // Loop over all elements calling their functions to do point processes and
  // send outflows.  Initialize iterators and other variables for the
  // incremental scan in allInflowsArrived.
  void doPointProcessesAndSendOutflows();
  
  // Scan neighbor proxies to see if all inflows have arrived.  This function
  // uses the itMesh and itChannel iterators to incrementally scan from where
  // the last scan ended.
  //
  // Returns: true if all inflows have arrived, false otherwise.
  bool allInflowsArrived();
  
  // Place waterMessage in the incoming material list of a neighborProxy.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // waterMessage - The water to receive and the identity of the neighbor proxy
  //                receiving it.
  bool receiveWater(RegionMessage waterMessage);
  
  // Send water to a recipient neighborProxy.  If recipientRegion is me just
  // call receiveWater, otherwise send a Charm++ message.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // recipientRegion - The region where the recipient is.
  // waterMessage    - The water to send and the identity of the neighbor proxy
  //                   to send it to.
  bool sendWater(int recipientRegion, RegionMessage waterMessage);
  
  // Receive a vector of water messages and for each message place the water in
  // the recipient neighbor proxy's incoming material list.
  //
  // Parameters:
  //
  // waterMessages - Vector of water messages from neighbors of elements in
  //                 this region.
  void processWaterMessages(std::vector<RegionMessage>& waterMessages);
  
  // Loop over all elements calling their functions to move water from incoming
  // material lists to element state variables.  Advance current time to the
  // end of the timestep.
  void receiveInflowsAndAdvanceTime();
  
  // FIXME comment
  void sendStateToFileManagers();
  
  // Scan elements to see if all forcing data has been updated.
  //
  // Returns: true if all forcing data has been updated, false otherwise.
  bool allForcingUpdated();
  
  // Compute values relevant to the mass balance calculation.  To calculate the
  // mass balance take waterInDomain and add externalFlows and subtract
  // waterError.  This will undo any insertion or removal of water from the
  // "black box" of the simulation domain leaving the amount of water that was
  // present when externalFlows and waterError were both zero.  This value
  // summed over all regions should be invariant except for floating point
  // roundoff error.
  //
  // For all three parameters the value for this element is added to whatever
  // value already exists in the passed-in variable.  It is done this way to
  // make it easy to accumulate values from multiple regions.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // waterInDomain - Scalar passed by reference.  The amount of water in cubic
  //                 meters in this region will be added to the existing
  //                 value in this variable.  Positive means the existance of
  //                 water.  Must be non-negative.
  // externalFlows - Scalar passed by reference.  The amount of water in cubic
  //                 meters that has flowed to or from external sources and
  //                 sinks (boundary conditions, precipitation, E-T, etc.) will
  //                 be added to the existing value in this variable.  Positive
  //                 means flow out of the element.  Negative means flow into
  //                 the element.
  // waterError    - Scalar passed by reference.  The amount of water in cubic
  //                 meters that was created or destroyed be error will be
  //                 added to the existing value in this variable.  Positive
  //                 means water was created.  Negative means water was
  //                 destroyed.
  bool massBalance(double& waterInDomain, double& externalFlows, double& waterError);

  // Turn off needToCheckInvariant if DEBUG_LEVEL_INTERNAL_INVARIANTS is false.
  //
  // ADHydro can either check the invariant once on input data, or at every
  // sync point.  If DEBUG_LEVEL_INTERNAL_INVARIANTS is false we will not be
  // checking the invariant at every sync point so we need to turn off
  // needToCheckInvariant after the first invariant check.
  //
  // This code would normally be in the .ci file, but #if isn't working in
  // the .ci file so I'm wrapping this code in a member function.
  void conditionallyTurnOffNeedToCheckInvariant()
  {
    // FIXME #if not working right in .ci file
#if !(DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    needToCheckInvariant = false;
#endif // !(DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  }

  // FIXME decide if we want this permanently
  // It is necessary to do this in the .cpp file because the scoping construct
  // causes problems in the .ci file, and adhydro.h isn't included in the .h file.
  void sendMaxDepthForTurnedOffElements();

  // FIXME decide if we want this permanently
  inline static bool regionIsTurnedOn(int regionNumber)
  {
      // To run the simulation normally on the entire map just always return true.
      //return true;
      return (  57 == regionNumber ||
                65 == regionNumber ||   66 == regionNumber ||   77 == regionNumber ||
                78 == regionNumber ||   96 == regionNumber ||   99 == regionNumber ||
               100 == regionNumber ||  101 == regionNumber ||  105 == regionNumber ||
               139 == regionNumber ||  141 == regionNumber ||  142 == regionNumber ||
               146 == regionNumber ||  168 == regionNumber ||  235 == regionNumber ||
               241 == regionNumber ||  242 == regionNumber ||  244 == regionNumber ||
               283 == regionNumber ||  284 == regionNumber ||  286 == regionNumber ||
               320 == regionNumber ||  321 == regionNumber ||
               364 == regionNumber ||  367 == regionNumber ||  411 == regionNumber ||
               424 == regionNumber ||  472 == regionNumber ||  479 == regionNumber ||
               782 == regionNumber ||  928 == regionNumber ||  929 == regionNumber ||
               930 == regionNumber ||  931 == regionNumber ||
               932 == regionNumber ||  933 == regionNumber ||  934 == regionNumber ||
               935 == regionNumber ||  937 == regionNumber ||  938 == regionNumber ||
               964 == regionNumber ||  997 == regionNumber ||  998 == regionNumber ||
               999 == regionNumber ||
              1000 == regionNumber || 1001 == regionNumber || 1003 == regionNumber ||
              1004 == regionNumber || 1005 == regionNumber || 1006 == regionNumber ||
              1007 == regionNumber || 1008 == regionNumber || 1009 == regionNumber ||
              1011 == regionNumber || 1013 == regionNumber || 1014 == regionNumber ||
              1015 == regionNumber ||
              1016 == regionNumber || 1018 == regionNumber || 1019 == regionNumber ||
              1026 == regionNumber || 1028 == regionNumber || 1029 == regionNumber ||
              1030 == regionNumber || 1031 == regionNumber || 1033 == regionNumber ||
              1440 == regionNumber || 1441 == regionNumber || 1443 == regionNumber ||
              1445 == regionNumber || 1447 == regionNumber || 1453 == regionNumber ||
              1454 == regionNumber);
  }
  
  // Elements.
  std::map<int, MeshElement>::size_type    numberOfMeshElements;    // During initialization the region waits until it has received this many mesh elements.
  std::map<int, MeshElement>               meshElements;            // A container for the mesh elements.  Map key is element ID number.
  std::map<int, ChannelElement>::size_type numberOfChannelElements; // During initialization the region waits until it has received this many channel elements.
  std::map<int, ChannelElement>            channelElements;         // A container for the channel elements.  Map key is element ID number.
  
  // Aggregator for messages going out to other regions.  Key is region ID number.
  std::map<int, std::vector<RegionMessage> > outgoingMessages;
  
  // When we are waiting for state or water messages to arrive we need to scan
  // to see if they have all arrived.  Rather than re-scan all neighbor proxies
  // every time, we save these iterators pointing to the first neighbor proxy
  // that is waiting.  That way we can start scanning from the end of the last
  // scan.  When we are not in the waiting loops these iterators can be used
  // locally within functions.  We can't pup iterators directly so we are
  // pupping index values, but we can't even get index values safely if the
  // iterators are not set so we have flags to say whether the iterators are
  // in use and thus are properly set and need to be pupped.
  bool                                                             pupItMeshAndItChannel;
  bool                                                             pupItNeighbor;
  std::map<int, MeshElement>::iterator                             itMesh;
  std::map<int, ChannelElement>::iterator                          itChannel;
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator         itMeshSurfacewaterMeshNeighbor;
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator      itMeshSurfacewaterChannelNeighbor;
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator          itMeshGroundwaterMeshNeighbor;
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator       itMeshGroundwaterChannelNeighbor;
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator      itChannelSurfacewaterMeshNeighbor;
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator   itChannelSurfacewaterChannelNeighbor;
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator       itChannelGroundwaterMeshNeighbor;
  
  // Time and simulation control variables.
  double regionalDtLimit;      // The largest expiration time dt that will be allowed for elements of this region.
                               // FIXME figure out how to set this limit.  Currently, it is hard coded to 60 seconds.
  bool   needToCheckInvariant; // Flag to indicate if the invariant needs tobe checked.
  int    nextCheckpointIndex;  // This multiplied by ADHydro::checkpointPeriod is the next time that a checkpoint will be done.
  int    nextOutputIndex;      // This multiplied by ADHydro::outputPeriod is the next time that an output will be done.
  bool   simulationFinished;   // Flag to indicate the simulation is finished.
  double nextForcingDataTime;  // Simulation time to receive the next forcing data in seconds since ADHydro::referenceDate.
  
  // These flags are used for initialization of diversions.
  bool sentMyDiversionsInitialized; // FIXME comment
  bool allDiversionsInitialized;    // FIXME comment
};

#endif // __REGION_H__
