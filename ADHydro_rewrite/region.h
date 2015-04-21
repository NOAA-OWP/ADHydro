#ifndef __REGION_H__
#define __REGION_H__

#include "mesh_element.h"
#include "channel_element.h"
#include "element.h"

// Because I'm passing RegionMessageStruct as a parameter in Charm++ messages I
// have to have a forward declaration of it before including region.decl.h.  I
// can't have a forward declaration of it if it is defined within the scope of
// the Region class.  Therefore, I am just defining it here.
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
typedef struct RegionMessageStruct
{
  // Default constructor.  Only needed for pup_stl.h code.
  RegionMessageStruct()
  {
    // Do nothing.
  }
  
  // Constructor.
  RegionMessageStruct(RegionMessageTypeEnum messageTypeInit, int recipientElementNumberInit, int recipientNeighborProxyIndexInit, double surfacewaterDepthInit,
                      double groundwaterHeadInit, SimpleNeighborProxy::MaterialTransfer waterInit) :
    messageType(messageTypeInit),
    recipientElementNumber(recipientElementNumberInit),
    recipientNeighborProxyIndex(recipientNeighborProxyIndexInit),
    surfacewaterDepth(surfacewaterDepthInit),
    groundwaterHead(groundwaterHeadInit),
    water(waterInit)
  {
    // Initialization handled by initialization list.
  }
  
  RegionMessageTypeEnum                 messageType;                 // The types of the element, neighbor, and interaction.
  int                                   recipientElementNumber;      // The element ID number of the message recipient.
  int                                   recipientNeighborProxyIndex; // The index in the recipient's neighbor proxy vector of the proxy for the sender.
  double                                surfacewaterDepth;           // The surfacewater depth of the sender in meters.  Unused for water messages.
  double                                groundwaterHead;             // The groundwater head of the sender in meters.  Unused for surfacewater state messages
                                                                     // or water messages.
  SimpleNeighborProxy::MaterialTransfer water;                       // The water sent to the recipient.  Unused for state messages.
} RegionMessageStruct;

PUPbytes(RegionMessageStruct);

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
  
  // FIXME constructor, pup, and initialization
  
  // Charm++ migration constructor.
  //
  // Parameters:
  //
  // msg - Charm++ migration message.
  Region(CkMigrateMessage* msg);
  
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
  // senderCurrentTime - Simulation time when state occured at the sender in
  //                     seconds since referenceDate.
  // stateMessages     - Vector of state messages from neighbors of elements in
  //                     this region.
  void processStateMessages(double senderCurrentTime, std::vector<RegionMessageStruct>& stateMessages);
  
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
  
  // Receive a vector of water messages and for each message place the water in
  // the recipient neighbor proxy's incoming material list.
  //
  // Parameters:
  //
  // waterMessages - Vector of water messages from neighbors of elements in
  //                 this region.
  void processWaterMessages(std::vector<RegionMessageStruct>& waterMessages);
  
  // Loop over all elements calling their functions to move water from incoming
  // material lists to element state variables.  Advance current time to the
  // end of the timestep.
  void receiveInflowsAndAdvanceTime();
  
  // Elements.  Key is element ID number.
  std::map<int, MeshElement>    meshElements;
  std::map<int, ChannelElement> channelElements;
  
  // Aggregator for messages going out to other regions.  Key is region ID number.
  std::map<int, std::vector<RegionMessageStruct> > outgoingMessages;
  
  // When we are waiting for state or water messages to arrive we need to scan
  // to see if they have all arrived.  Rather than re-scan all neighbor proxies
  // every time, we save these iterators pointing to the first neighbor proxy
  // that is waiting.  That way we can start scanning from the end of the last
  // scan.  When we are not in the waiting loops these iterators can be used
  // locally within functions.
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
