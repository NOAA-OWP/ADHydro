#ifndef __NEIGHBOR_PROXY_H__
#define __NEIGHBOR_PROXY_H__

#include "all.h"

// A NeighborEndpointEnum describes how an element is connected to a neighbor.
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
    IRRIGATION_DIVERSION = 14, // Diverting element is a channel element.  Water comes from surfaceWater.
    IRRIGATION_RECIPIENT = 15, // Recipient element is a mesh    element.  Water goes to    surfaceWater.
};

PUPbytes(NeighborEndpointEnum);

// A NeighborConnection uniquely identifies a connection between a pair of neighboring elements.
// It consists of two endpoints.  Each endpoint consists of a NeighborEndpointEnum and an element number.
// The element number could be a mesh or channel element number depending on the NeighborEndpointEnum.
// One endpoint is called local and the other remote.
// A NeighborProxy at one element stores its element as the local endpoint and the other element as the remote endpoint.
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
    
    // Constructor.  All parameters directly initialize member variables.
    inline NeighborConnection(NeighborEndpointEnum localEndpoint = MESH_SURFACE, size_t localElementNumber = 0, NeighborEndpointEnum remoteEndpoint = CHANNEL_SURFACE, size_t remoteElementNumber = 0) :
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
    bool checkInvariant() const;
    
    // Comparison operator to provide strict ordering so that NeighborConnection can be used as a std::map key.
    //
    // Returns: true if this is less than other, false otherwise.
    //
    // Parameters:
    //
    // other - The other NeighborConnection to compare to.
    bool operator<(const NeighborConnection& other) const;
    
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

// A NeighborAttributes contains the immutable attributes an element needs to know about its remote neighbor in order to calculate nominal flow rates.
class NeighborAttributes
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline NeighborAttributes(int payload = 0) : payload(payload)
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
        p | payload;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
    int payload; // FIXME this is a placeholder.
};

// A StateTransfer contains the state that an element has to exchange with its neighbor so that they can calculate nominal flow rates.
class StateTransfer
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline StateTransfer(int payload = 0) : payload(payload)
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
        p | payload;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
    int payload; // FIXME this is a placeholder.
};

// A WaterTransfer represents some water that has been sent by one element and not yet received by another element.
// If the transfer needs to be split across multiple timesteps at the recipient, it is assumed to be a constant flow rate.
class WaterTransfer
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline WaterTransfer(double water = 0.0, double startTime = 0.0, double endTime = 1.0) : water(water), startTime(startTime), endTime(endTime)
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
    bool checkInvariant() const;
    
    // Comparison operator to provide strict ordering so that NeighborConnection can be used as a std::set key.
    // Objects are ordered by time range, and overlapping time ranges are considered equal.
    // std::set can never store two equal keys at the same time so this guarantees time ranges will be non-overlapping.
    //
    // Returns: true if this is less than other, false otherwise.
    //
    // Parameters:
    //
    // other - The other NeighborConnection to compare to.
    inline bool operator<(const WaterTransfer& other) const
    {
        return (endTime <= other.startTime);
    }
    
    double water;     // (m^3) A quantity of water being transferred from one element to another.
    double startTime; // (s) Simulation time when the transfer starts specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    double endTime;   // (s) Simulation time when the transfer ends   specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
};

class NeighborProxy; // This forward declaration is needed for Message::receive.

// A Message is a superclass for all the different types of messages we send.
class Message
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline Message(const NeighborConnection& destination = NeighborConnection()) : destination(destination)
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
    inline virtual void pup(PUP::er &p)
    {
        p | destination;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    inline virtual bool checkInvariant() const
    {
        return destination.checkInvariant();
    }
    
    // Pass this message on to the right member function of a NeighborProxy.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // proxy             - The NeighborProxy to pass this message to.
    // neighborsFinished - Number of NeighborProxies in the current element finished in the current phase.  May be incremented if this call causes proxy to be finished.
    // currentTime       - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime   - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    virtual bool receive(NeighborProxy& proxy, size_t& neighborsFinished, double currentTime, double timestepEndTime) const = 0;
    
    NeighborConnection destination; // The remote neighbor is the destination of this message.
};

// A NeighborMessage is a Message containing a NeighborAttributes.
class NeighborMessage : public Message
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline NeighborMessage(const NeighborConnection& destination = NeighborConnection(), const NeighborAttributes& attributes = NeighborAttributes()) : Message(destination), attributes(attributes)
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
    inline virtual void pup(PUP::er &p)
    {
        Message::pup(p);
        
        p | attributes;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    inline virtual bool checkInvariant() const
    {
        return Message::checkInvariant() || attributes.checkInvariant();
    }
    
    // Pass this message on to the right member function of a NeighborProxy.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // proxy             - The NeighborProxy to pass this message to.
    // neighborsFinished - Number of NeighborProxies in the current element finished in the initialization phase.  May be incremented if this call causes proxy to be finished.
    // currentTime       - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime   - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    virtual bool receive(NeighborProxy& proxy, size_t& neighborsFinished, double currentTime, double timestepEndTime) const;
    
    NeighborAttributes attributes; // The attributes that are being sent.
};

// A StateMessage is a Message containing a StateTransfer.
class StateMessage : public Message
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline StateMessage(const NeighborConnection& destination = NeighborConnection(), const StateTransfer& state = StateTransfer()) : Message(destination), state(state)
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
    inline virtual void pup(PUP::er &p)
    {
        Message::pup(p);
        
        p | state;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    inline virtual bool checkInvariant() const
    {
        return Message::checkInvariant() || state.checkInvariant();
    }
    
    // Pass this message on to the right member function of a NeighborProxy.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // proxy             - The NeighborProxy to pass this message to.
    // neighborsFinished - Number of NeighborProxies in the current element finished in the receive state phase.  May be incremented if this call causes proxy to be finished.
    // currentTime       - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime   - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    virtual bool receive(NeighborProxy& proxy, size_t& neighborsFinished, double currentTime, double timestepEndTime) const;
    
    StateTransfer state; // The state that is being sent.
};

// A WaterMessage is a Message containing a WaterTransfer.
class WaterMessage : public Message
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline WaterMessage(const NeighborConnection& destination = NeighborConnection(), const WaterTransfer& water = WaterTransfer()) : Message(destination), water(water)
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
    inline virtual void pup(PUP::er &p)
    {
        Message::pup(p);
        
        p | water;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    inline virtual bool checkInvariant() const
    {
        return Message::checkInvariant() || water.checkInvariant();
    }
    
    // Pass this message on to the right member function of a NeighborProxy.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // proxy             - The NeighborProxy to pass this message to.
    // neighborsFinished - Number of NeighborProxies in the current element finished in the receive water phase.  May be incremented if this call causes proxy to be finished.
    // currentTime       - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime   - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    virtual bool receive(NeighborProxy& proxy, size_t& neighborsFinished, double currentTime, double timestepEndTime) const;
    
    WaterTransfer water; // The water that is being sent.
};

// A NeighborProxy is how elements store their neighbor connections with other elements.  For each neighbor connection, two NeighborProxies are stored, one at each element.
// The element where the NeighborProxy is stored is its local neighbor.  The other is its remote neighbor.  A NeighborProxy stores the destination information needed to
// communicate with the remote neighbor, immutable attributes of the remote neighbor needed for local calculations, and information about water flows between the neighbors.
class NeighborProxy
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline NeighborProxy(size_t neighborRegion = 0, double nominalFlowRate = 0.0, double expirationTime = 0.0, double inflowCumulative = 0.0, double outflowCumulative = 0.0) :
        neighborRegion(neighborRegion), attributes(0), attributesInitialized(false), nominalFlowRate(nominalFlowRate), expirationTime(expirationTime), inflowCumulativeShortTerm(0.0),
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
        p | neighborRegion;
        p | attributes;
        p | attributesInitialized;
        p | nominalFlowRate;
        p | expirationTime;
        p | inflowCumulativeShortTerm;
        p | inflowCumulativeLongTerm;
        p | outflowCumulativeShortTerm;
        p | outflowCumulativeLongTerm;
        p | incomingWater;
    }
    
    // Check invariant conditions on data.  Does not check that values are the same as corresponding values stored at the neighbor.  That is done elsewhere with messages.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
    // Send a message to the remote neighbor with the immutable attributes of the local element that the remote neighbor needs to calculate nominal flow rates.
    // If this is a boundary condition with no remote neighbor no message is sent and attributesInitialized is marked true.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages  - A container in which to put any message that needs to be sent.  Key is Region ID number of message destination.
    // neighborsFinished - Number of NeighborProxies in the current element finished in the initialization phase.  May be incremented if this call causes this NeighborProxy to be finished.
    // message           - The message to send.
    bool sendNeighborMessage(std::map<size_t, std::vector<NeighborMessage> >& outgoingMessages, size_t& neighborsFinished, const NeighborMessage& message);
    
    // Store the received immutable attributes of the remote neighbor and mark attributesInitialized true.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // neighborsFinished - Number of NeighborProxies in the current element finished in the initialization phase.  May be incremented if this call causes this NeighborProxy to be finished.
    // remoteAttributes  - The attributes that are being received.
    bool receiveNeighborAttributes(size_t& neighborsFinished, const NeighborAttributes& remoteAttributes);
    
    // If nominalFlowRate has expired, begin the process of recalculating it.  This may require sending a message to the remote neighbor and waiting for a message in return.
    // In some situations nominalFlowRate can be calculated before leaving this method such as a neighbor in the same Region, or a boundary condition where there is no neighbor.
    // If expirationTime is already past currentTime then nominalFlowRate hasn't expired so do nothing.
    //
    // FIXME this will actually require the local state to send in the StateMessage.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages  - A container in which to put any message that needs to be sent.  Key is Region ID number of message destination.
    // neighborsFinished - Number of NeighborProxies in the current element finished in the receive state phase.  May be incremented if this call causes this NeighborProxy to be finished.
    // connection        - How the local and remote neighbors are connected.
    // currentTime       - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool calculateNominalFlowRate(std::map<size_t, std::vector<StateMessage> >& outgoingMessages, size_t& neighborsFinished, const NeighborConnection& connection, double currentTime);
    
    // Receive a StateMessage from the remote neighbor and finish recalculating nominalFlowRate.
    //
    // FIXME this will require the local state as well to do the calculation.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // neighborsFinished - Number of NeighborProxies in the current element finished in the receive state phase.  May be incremented if this call causes this NeighborProxy to be finished.
    // state             - The state that is being received.  It is the entire StateMessage, not just the StateTransfer because the function needs the NeighborConnection as well.
    // currentTime       - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool receiveStateMessage(size_t& neighborsFinished, const StateMessage& state, double currentTime);
    
    // Send a WaterTransfer to the remote neighbor.  The water has already been removed from the local element.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages - A container in which to put any message that needs to be sent.  Key is Region ID number of message destination.
    // water            - The water to send.
    bool sendWater(std::map<size_t, std::vector<WaterMessage> >& outgoingMessages, const WaterMessage& water);
    
    // Receive a WaterMessage from the remote neighbor.  The WaterTransfer will be placed in incomingWater to wait until the local element is ready to advance time.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // neighborsFinished - Number of NeighborProxies in the current element finished in the receive water phase.  May be incremented if this call causes this NeighborProxy to be finished.
    // water             - The water that is being received.  It is the entire WaterMessage, not just the WaterTransfer because the function needs the NeighborConnection as well.
    // currentTime     - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool receiveWaterMessage(size_t& neighborsFinished, const WaterMessage& water, double currentTime, double timestepEndTime);
    
    // Returns: true if there are no time gaps in incomingWater between currentTime and timestepEndTime, which implies all WaterTransfers have arrived, false otherwise.  Exit on error.
    //
    // Parameters:
    //
    // connection      - How the local and remote neighbors are connected.
    // currentTime     - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool allWaterHasArrived(const NeighborConnection& connection, double currentTime, double timestepEndTime);
    
    // Remove all water in incomingWater up to timestepEndTime and return that water to the local element to formally receive the water into the state variables of the element.
    //
    // Returns: (m^3) the water that has been received up to timestepEndTime.
    //
    // Parameters:
    //
    // connection      - How the local and remote neighbors are connected.
    // currentTime     - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    double receiveWater(const NeighborConnection& connection, double currentTime, double timestepEndTime);
    
    // Returns: the value of nominalFlowRate.
    inline double getNominalFlowRate() const
    {
        return nominalFlowRate;
    }
    
    // Returns: the value of expirationTime.
    inline double getExpirationTime() const
    {
        return expirationTime;
    }
    
private:
    
    // Destination information needed to communicate with the remote neighbor.
    size_t neighborRegion;
    
    // Immutable attributes of the remote neighbor.
    NeighborAttributes attributes;
    bool               attributesInitialized; // If true, attributes have been received from the neighbor.
    
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
    
    // incomingWater implements a sorted list of WaterTransfers with non-overlapping time ranges.  WaterTransfers are sorted by time with overlapping time ranges considered equal so no two overlapping ranges can be put in the set.
    // When a message containing a WaterTransfer is received it is initially put in incomingWater.  Later, when the local element is ready to advance time the water is formally received into the state variables of the element.
    // incomingWater can only be non-empty when nominalFlowRate is an inflow (negative).  All transfers must end no later than expirationTime.  When there are no time gaps all inflows have arrived.
    std::set<WaterTransfer> incomingWater;
};

#endif // __NEIGHBOR_PROXY_H__