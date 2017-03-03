#ifndef __NEIGHBOR_PROXY_H__
#define __NEIGHBOR_PROXY_H__

#include "all.h"

// A NeighborEndpointEnum describes how a neighbor is connected to an element.
// Elements can have surface flows, subsurface flows, and other types of flows like water management.
// When you receive a message from a neighbor it's not enough to know what neighbor it came from.
// You may have more than one type of connection with that neighbor and you need to know which connection the message refers to.
// NeighborEndpointEnum is used to disambiguate between multiple connections with the same neighbor.
enum NeighborEndpointEnum
{
    // Natural flows.
    MESH_SURFACE         = 0,
    MESH_SOIL            = 1,
    MESH_AQUIFER         = 2,
    CHANNEL_SURFACE      = 3,
    BOUNDARY_INFLOW      = 4, // Only allowed as a remote endpoint.  The local endpoint describes whether the boundary is connected to surface, soil, etc.
    BOUNDARY_OUTFLOW     = 5, // Only allowed as a remote endpoint.  The local endpoint describes whether the boundary is connected to surface, soil, etc.
    
    // Water management.
    TRANSBASIN_INFLOW    = 10, // Only allowed as a remote endpoint.  The local endpoint describes whether the boundary is connected to surface, soil, etc.
    TRANSBASIN_OUTFLOW   = 11, // Only allowed as a remote endpoint.  The local endpoint describes whether the boundary is connected to surface, soil, etc.
    RESERVOIR_RELEASE    = 12, // Releasing element is a channel element.  Water comes from surfaceWater.
    RESERVOIR_RECIPIENT  = 13, // Recipient element is a channel element.  Water goes to    surfaceWater.
    IRRIGATION_DIVERSION = 14, // Diversion element is a channel element.  Water comes from surfaceWater.
    IRRIGATION_RECIPIENT = 15, // Recipient element is a mesh    element.  Water goes to    surfaceWater.
};

PUPbytes(NeighborEndpointEnum);

// A NeighborConnection uniquely identifies a connection between a pair of neighboring elements.
// It consists of two endpoints.  Each endpoint consists of a NeighborEndpointEnum and an element number.
// The element number could be a mesh or channel element number depending on the NeighborEndpointEnum.
// One endpoint is called local and the other remote.
// A NeighborProxy at one element stores its element as the local endpoint and the other element as the remote element.
// The NeighborProxy at the other end of the connection would store the local and remote endpoints reversed.
// Two NeighborConnections that are reverses of each other describe the same connection and are equivalent.
//
// A consequence of the uniqueness of a NeighborConnection is that a given element can only have one connection of each type with a given neighbor.
// This makes sense for natural flows where the connection occupies the entire shared edge between the elements.
// For flows like water mangement, it would be highly unlikely to have multiple diversions from the same element going to the same element,
// but even in that case it would be better to combine them into one conceptual diversion that only requires one message.
class NeighborConnection
{
public:
    
    // Default constructor.  Only needed for pup_stl.h code.
    inline NeighborConnection() : localEndpoint(MESH_SURFACE), localElementNumber(0), remoteEndpoint(CHANNEL_SURFACE), remoteElementNumber(0) {}
    
    // Constructor.  All parameters directly initialize member variables.
    inline NeighborConnection(NeighborEndpointEnum localEndpoint, size_t localElementNumber, NeighborEndpointEnum remoteEndpoint, size_t remoteElementNumber) :
        localEndpoint(localEndpoint), localElementNumber(localElementNumber), remoteEndpoint(remoteEndpoint), remoteElementNumber(remoteElementNumber)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (checkInvariant())
            {
                CkExit();
            }
        }
    }
    
    // Charm++ pack/unpack method.
    //
    // Parameters:
    //
    // p - Pack/unpack processing object.
    inline void pup(PUP::er &p)
    {
        p | localEndpoint;
        p | localElementNumber;
        p | remoteEndpoint;
        p | remoteElementNumber;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant();
    
    // Comparison operator to provide strict ordering so that NeighborConnection can be used as a Map key.
    //
    // Returns: true if this is less than other, false otherwise.
    //
    // Parameters:
    //
    // other - The other NeighborConnection to compare to.
    inline bool operator<(const NeighborConnection& other) const
    {
        bool lessThan; // Return value.
        
        if (remoteElementNumber < other.remoteElementNumber)
        {
            lessThan = true;
        }
        else if (remoteElementNumber > other.remoteElementNumber)
        {
            lessThan = false;
        }
        else if (remoteEndpoint < other.remoteEndpoint)
        {
            lessThan = true;
        }
        else if (remoteEndpoint > other.remoteEndpoint)
        {
            lessThan = false;
        }
        else if (localElementNumber < other.localElementNumber)
        {
            lessThan = true;
        }
        else if (localElementNumber > other.localElementNumber)
        {
            lessThan = false;
        }
        else if (localEndpoint < other.localEndpoint)
        {
            lessThan = true;
        }
        else if (localEndpoint > other.localEndpoint)
        {
            lessThan = false;
        }
        else
        {
            lessThan = false;
        }
        
        return lessThan;
    }
    
    // Swap the local and remote ends of the connection.
    inline void reverse()
    {
        std::swap(localEndpoint,      remoteEndpoint);
        std::swap(localElementNumber, remoteElementNumber);
    }
    
    NeighborEndpointEnum localEndpoint;       // How the connection is connected at one end.
    size_t               localElementNumber;  // The number of the mesh or channel element at one end of the connection.
    NeighborEndpointEnum remoteEndpoint;      // How the connection is connected at the other end.
    size_t               remoteElementNumber; // The number of the mesh or channel element at the other end of the connection.
};

// A WaterTransfer represents some water that has been sent by one element and not yet received by another element.
// If the transfer needs to be split across multiple timesteps at the recipient, it is assumed to be a constant flow rate.
class WaterTransfer
{
public:
    
    // Default constructor.  Only needed for pup_stl.h code.
    inline WaterTransfer() : water(0.0), startTime(0.0), endTime(1.0) {}
    
    // Constructor.  All parameters directly initialize member variables.
    inline WaterTransfer(double water, double startTime, double endTime) : water(water), startTime(startTime), endTime(endTime)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (checkInvariant())
            {
                CkExit();
            }
        }
    }
    
    // Charm++ pack/unpack method.
    //
    // Parameters:
    //
    // p - Pack/unpack processing object.
    inline void pup(PUP::er &p)
    {
        p | water;
        p | startTime;
        p | endTime;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant();
    
    double water;     // (m^3) A quantity of water being transferred from one element to another.
    double startTime; // (s) Simulation time when the transfer starts specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    double endTime;   // (s) Simulation time when the transfer ends   specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
};

// A NeighborProxy is how elements store their neighbor connections with other elements.  For each neighbor connection, two NeighborProxies are stored, one at each element.
// The element where the NeighborProxy is stored is its local element.  The other is its remote element.  A NeighborProxy stores the destination information needed to
// communicate with the remote element, immutable attributes of the remote element needed for local calculations, and information about water flows between the elements.
class NeighborProxy
{
public:
    
    // Default constructor.  Only needed for pup_stl.h code.
    inline NeighborProxy() : connection(), nominalFlowRate(0.0), expirationTime(0.0), inflowCumulativeShortTerm(0.0), inflowCumulativeLongTerm(0.0), outflowCumulativeShortTerm(0.0), outflowCumulativeLongTerm(0.0), incomingWater() {}
    
    // Constructor.  All parameters directly initialize member variables.
    inline NeighborProxy(NeighborConnection connection, double nominalFlowRate, double expirationTime, double inflowCumulative, double outflowCumulative) :
        connection(connection), nominalFlowRate(nominalFlowRate), expirationTime(expirationTime), inflowCumulativeShortTerm(0.0),
        inflowCumulativeLongTerm(inflowCumulative), outflowCumulativeShortTerm(0.0), outflowCumulativeLongTerm(outflowCumulative), incomingWater()
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (checkInvariant())
            {
                CkExit();
            }
        }
    }
    
    // Charm++ pack/unpack method.
    //
    // Parameters:
    //
    // p - Pack/unpack processing object.
    inline void pup(PUP::er &p)
    {
        p | connection;
        p | nominalFlowRate;
        p | expirationTime;
        p | inflowCumulativeShortTerm;
        p | inflowCumulativeLongTerm;
        p | outflowCumulativeShortTerm;
        p | outflowCumulativeLongTerm;
        p | incomingWater;
    }
    
    // Check invariant conditions on data.  Does not check that neighbor values are the same as the values stored at the neighbor.  That is done elsewhere with messages.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant();
    
    // FIXME helper function to calculate nominalFlowRate
    
    // Send a WaterTransfer to the remote element.  The water has already been removed from the local element.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // water - The water to send.
    bool sendWaterTransfer(WaterTransfer water);
    
    // Receive a WaterTransfer from the remote element.  The WaterTransfer will be placed in incomingWater to wait until the local element is ready to advance time.
    // This keeps the list sorted and checks that the time range of the newly inserted WaterTransfer is non-overlapping.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // water - The water that is being received.
    bool receiveWaterTransfer(WaterTransfer water);
    
    // Returns: true if there are no time gaps in incomingWater between currentTime and timestepEndTime, which implies all WaterTransfers have arrived, false otherwise.  Exit on error.
    //
    // Parameters:
    //
    // currentTime     - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool allWaterHasArrived(double currentTime, double timestepEndTime);
    
    // Remove all water in incomingWater up to timestepEndTime and return that water to the local element to formally receive the water into the state variables of the element.
    //
    // Returns: (m^3) the water that has been received up to timestepEndTime.
    //
    // Parameters:
    //
    // timestepEndTime - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    double receiveWater(double timestepEndTime);
    
    // Returns: the value of nominalFlowRate.
    inline double getNominalFlowRate()
    {
        return nominalFlowRate;
    }
    
private:
    
    // Information identifying the two ends of the neighbor connection.
    NeighborConnection connection;
    
    // Immutable attributes of the remote element.
    // FIXME implement, also constructor, pup, and invariant, and figure out how to initialize
    
    // Mutable state of the current flow between the neighbors.
    double nominalFlowRate; // (m^3/s) Positive means flow out of the local element into the remote element.  Negative means flow into the local element out of the remote element.
                            // Actual flow may be less if the sender does not have enough water to satisfy all outflows.
    double expirationTime;  // (s) Simulation time when the neighbors will renegotiate nominalFlowRate specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    
    // These are variables for recording cumulative flows between the neighbors.  There is some complexity with how they are stored.  Inflows and outflows are stored separately.
    // The purpose of this is to be able to tell the difference between a situation where there has been no flow vs. a situation where flow has gone back and forth with a net flow of zero such as might happen in a tidal basin.
    // In addition, for each direction, flow is stored in two variables, a short term variable and a long term variable.  The purpose of this is to minimize roundoff error.
    // Cumulative flow can get quite large, and the additional flow of a single timestep that needs to be added to it can be quite small.  This can generate significant roundoff error.
    // To combat this, flow is first added to the short term variable, and then when the short term variable gets to a significant size it is moved to the long term variable.
    // There is no semantic difference between the short term and long term variables.  If you ever want to use the quantity of cumulative flow always use the sum of the short term and long term variables.
    double inflowCumulativeShortTerm;  // (m^3) Negative means water flowed into   the local element out of the remote element.  Must be non-positive.
    double inflowCumulativeLongTerm;   // (m^3) Negative means water flowed into   the local element out of the remote element.  Must be non-positive.
    double outflowCumulativeShortTerm; // (m^3) Positive means water flowed out of the local element into   the remote element.  Must be non-negative.
    double outflowCumulativeLongTerm;  // (m^3) Positive means water flowed out of the local element into   the remote element.  Must be non-negative.
    
    // When a message containing a WaterTransfer is received it is initially put in this list.  Later, when the local element is ready to advance time the water is formally received into to state variables of the element.
    // The list can only be non-empty when nominalFlowRate is an inflow (negative).  All transfers in the list must have non-overlapping time ranges.  The list is maintained sorted with the earliest transfers at the front.
    // All transfers must end no later than expirationTime.  When there are no time gaps in the list all inflows have arrived.
    std::list<WaterTransfer> incomingWater;
};

#endif // __NEIGHBOR_PROXY_H__
