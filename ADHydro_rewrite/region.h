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
class Region : public CBase_Region, Element
{
  Region_SDAG_CODE
  
public:
  
  // Constructor.  timestepEndTime is initialized to be currentTimeInit
  // indicating that a new value for timestepEndTime needs to be selected in
  // step 2 of the simulation.  nextSyncTime is initialized to be
  // simulationEndTimeInit indicating that the element will sync up with other
  // elements at the end of the simulation.
  //
  // meshElements, channelElements, and outgoingMessages are initialized to
  // empty.  All iterators are initialized to invalid.
  //
  // FIXME what is regionalDtLimit initialized to?
  //
  // Parameters:
  //
  // referenceDateInit     - Julian date to initialize referenceDate.
  // currentTimeInit       - Simulation time in seconds since referenceDateInit
  //                         to initialize currentTime and timestepEndTime.
  // simulationEndTimeInit - Simulation time in seconds since referenceDateInit
  //                         to initialize simulationEndTime and nextSyncTime.
  Region(double referenceDateInit, double currentTimeInit, double simulationEndTimeInit);
  
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
  
  // FIXME initialization
  void initializeMeshElement(int elementNumberInit, int catchmentInit, int vegetationTypeInit, int soilTypeInit, double elementXInit, double elementYInit,
      double elementZSurfaceInit, double layerZBottomInit, double elementAreaInit, double slopeXInit, double slopeYInit,
      double latitudeInit, double longitudeInit, double manningsNInit, double conductivityInit, double porosityInit,
      double surfacewaterDepthInit, double surfacewaterErrorInit, double groundwaterHeadInit, double groundwaterRechargeInit,
      double groundwaterErrorInit, double precipitationRateInit, double precipitationCumulativeShortTermInit,
      double precipitationCumulativeLongTermInit, double evaporationRateInit, double evaporationCumulativeShortTermInit,
      double evaporationCumulativeLongTermInit, double transpirationRateInit, double transpirationCumulativeShortTermInit,
      double transpirationCumulativeLongTermInit, EvapoTranspirationForcingStruct& evapoTranspirationForcingInit,
      EvapoTranspirationStateStruct& evapoTranspirationStateInit, InfiltrationAndGroundwater::InfiltrationMethodEnum infiltrationMethodInit,
      InfiltrationAndGroundwater::GroundwaterMethodEnum groundwaterMethodInit, void* vadoseZoneStateInit);
  
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
  
  // Elements.  Key is element ID number.
  std::map<int, MeshElement>    meshElements;
  std::map<int, ChannelElement> channelElements;
  
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
  
  // FIXME figure out how to set this limit.
  double regionalDtLimit;
};

#endif // __REGION_H__
