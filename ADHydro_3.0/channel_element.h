#ifndef __CHANNEL_ELEMENT_H__
#define __CHANNEL_ELEMENT_H__

#include "neighbor_proxy.h"
#include "evapo_transpiration.h"

class ChannelState
{
public:
    
    // Charm++ pack/unpack method.
    //
    // Parameters:
    //
    // p - Pack/unpack processing object.
    inline void pup(PUP::er &p)
    {
        p | elementNumber;
        p | evapoTranspirationState;
        p | surfaceWater;
        p | surfaceWaterCreated;
        p | precipitationRate;
        p | precipitationCumulative;
        p | evaporationRate;
        p | evaporationCumulative;
        p | neighbors;
    }
    
    size_t                        elementNumber;
    EvapoTranspirationStateStruct evapoTranspirationState;
    double                        surfaceWater;
    double                        surfaceWaterCreated;
    double                        precipitationRate;
    double                        precipitationCumulative;
    double                        evaporationRate;
    double                        evaporationCumulative;
    std::vector<NeighborState>    neighbors;
};

// A ChannelElement is a length of stream, a waterbody, or a glacier in the channel network.  It is modeled as a linear element.
// It simulates surfacewater state only.  Groundwater underneath the channel is simulated by neighboring mesh elements.
class ChannelElement
{
public:
    
    // Constructor.  All parameters directly initialize member variables.
    inline ChannelElement(size_t elementNumber = 0, ChannelTypeEnum channelType = STREAM, long long reachCode = 0, double elementX = 0.0, double elementY = 0.0, double elementZBank = 0.0,
                          double elementZBed = 0.0, double elementLength = 1.0, double latitude = 0.0, double longitude = 0.0, double baseWidth = 1.0, double sideSlope = 1.0,
                          double manningsN = 1.0, double bedThickness = 1.0, double bedConductivity = 0.0, const EvapoTranspirationStateStruct* evapoTranspirationStateInit = NULL,
                          double surfaceWater = 0.0, double surfaceWaterCreated = 0.0, double precipitationCumulative = 0.0, double evaporationCumulative = 0.0,
                          std::map<NeighborConnection, NeighborProxy> neighbors = std::map<NeighborConnection, NeighborProxy>()) :
        elementNumber(elementNumber), channelType(channelType), reachCode(reachCode), elementX(elementX), elementY(elementY), elementZBank(elementZBank),
        elementZBed(elementZBed), elementLength(elementLength), latitude(latitude), longitude(longitude), baseWidth(baseWidth), sideSlope(sideSlope), manningsN(manningsN),
        bedThickness(bedThickness), bedConductivity(bedConductivity), /* evapoTranspirationForcing and evapoTranspirationState initialized below. */ surfaceWater(surfaceWater),
        surfaceWaterCreated(surfaceWaterCreated), precipitationRate(0.0), precipitationCumulativeShortTerm(0.0), precipitationCumulativeLongTerm(precipitationCumulative),
        evaporationRate(0.0), evaporationCumulativeShortTerm(0.0), evaporationCumulativeLongTerm(evaporationCumulative), neighbors(neighbors), neighborsFinished(0)
    {
        // Values for evapoTranspirationForcing are going to be received before we start simulating.  For now, just fill in values that will pass the invariant.
        evapoTranspirationForcing.dz8w   = 20.0f;
        evapoTranspirationForcing.sfcTmp = 300.0f;
        evapoTranspirationForcing.sfcPrs = 101300.0f;
        evapoTranspirationForcing.psfc   = 101180.0f;
        evapoTranspirationForcing.uu     = 0.0f;
        evapoTranspirationForcing.vv     = 0.0f;
        evapoTranspirationForcing.q2     = 0.0f;
        evapoTranspirationForcing.qc     = 0.0f;
        evapoTranspirationForcing.solDn  = 0.0f;
        evapoTranspirationForcing.lwDn   = 0.0f;
        evapoTranspirationForcing.prcp   = 0.0f;
        evapoTranspirationForcing.tBot   = 300.0f;
        evapoTranspirationForcing.pblh   = 0.0f;
        
        // It was necessary to do things this way to combine the regular constructor with the no-argument constructor while initializing evapoTranspirationState to values that pass the invariant in the no-argument case.
        if (NULL != evapoTranspirationStateInit)
        {
            evapoTranspirationState = *evapoTranspirationStateInit;
        }
        else
        {
            evapoTranspirationState.fIceOld[0] = 0;
            evapoTranspirationState.fIceOld[1] = 0;
            evapoTranspirationState.fIceOld[2] = 0;
            evapoTranspirationState.albOld     = 1;
            evapoTranspirationState.snEqvO     = 0;
            evapoTranspirationState.stc[0]     = 0;
            evapoTranspirationState.stc[1]     = 0;
            evapoTranspirationState.stc[2]     = 0;
            evapoTranspirationState.stc[3]     = 300;
            evapoTranspirationState.stc[4]     = 300;
            evapoTranspirationState.stc[5]     = 300;
            evapoTranspirationState.stc[6]     = 300;
            evapoTranspirationState.tah        = 300;
            evapoTranspirationState.eah        = 2000;
            evapoTranspirationState.fWet       = 0;
            evapoTranspirationState.canLiq     = 0;
            evapoTranspirationState.canIce     = 0;
            evapoTranspirationState.tv         = 300;
            evapoTranspirationState.tg         = 300;
            evapoTranspirationState.iSnow      = 0;
            evapoTranspirationState.zSnso[0]   = 0;
            evapoTranspirationState.zSnso[1]   = 0;
            evapoTranspirationState.zSnso[2]   = 0;
            evapoTranspirationState.zSnso[3]   = -.05;
            evapoTranspirationState.zSnso[4]   = -.2;
            evapoTranspirationState.zSnso[5]   = -.5;
            evapoTranspirationState.zSnso[6]   = -1;
            evapoTranspirationState.snowH      = 0;
            evapoTranspirationState.snEqv      = 0;
            evapoTranspirationState.snIce[0]   = 0;
            evapoTranspirationState.snIce[1]   = 0;
            evapoTranspirationState.snIce[2]   = 0;
            evapoTranspirationState.snLiq[0]   = 0;
            evapoTranspirationState.snLiq[1]   = 0;
            evapoTranspirationState.snLiq[2]   = 0;
            evapoTranspirationState.lfMass     = 100000;
            evapoTranspirationState.rtMass     = 100000;
            evapoTranspirationState.stMass     = 100000;
            evapoTranspirationState.wood       = 200000;
            evapoTranspirationState.stblCp     = 200000;
            evapoTranspirationState.fastCp     = 200000;
            evapoTranspirationState.lai        = 4.6;
            evapoTranspirationState.sai        = 0.6;
            evapoTranspirationState.cm         = 0.002;
            evapoTranspirationState.ch         = 0.002;
            evapoTranspirationState.tauss      = 0;
            evapoTranspirationState.deepRech   = 0;
            evapoTranspirationState.rech       = 0;
        }
        
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
        p | elementNumber;
        p | channelType;
        p | reachCode;
        p | elementX;
        p | elementY;
        p | elementZBank;
        p | elementZBed;
        p | elementLength;
        p | latitude;
        p | longitude;
        p | baseWidth;
        p | sideSlope;
        p | manningsN;
        p | bedThickness;
        p | bedConductivity;
        p | evapoTranspirationForcing;
        p | evapoTranspirationState;
        p | surfaceWater;
        p | surfaceWaterCreated;
        p | precipitationRate;
        p | precipitationCumulativeShortTerm;
        p | precipitationCumulativeLongTerm;
        p | evaporationRate;
        p | evaporationCumulativeShortTerm;
        p | evaporationCumulativeLongTerm;
        p | neighbors;
        p | neighborsFinished;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
    // Receive any type of message and pass it on to the appropriate NeighborProxy.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // message          - The received message.
    // elementsFinished - Number of elements in the current Region finished in the current phase.  May be incremented if this call causes this element to be finished.
    // currentTime      - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime  - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    virtual bool receiveMessage(const Message& message, size_t& elementsFinished, double currentTime, double timestepEndTime);
    
    // Call sendNeighborAttributes on all NeighborProxies.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages - Container to aggregate outgoing messages to other Regions.  Key is Region ID number of message destination.
    // elementsFinished - Number of elements in the current Region finished in the initialization phase.  May be incremented if this call causes this element to be finished.
    bool sendNeighborAttributes(std::map<size_t, std::vector<NeighborMessage> >& outgoingMessages, size_t& elementsFinished);
    
    // Call calculateNominalFlowRate on all NeighborProxies.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages - Container to aggregate outgoing messages to other Regions.  Key is Region ID number of message destination.
    // elementsFinished - Number of elements in the current Region finished in the receive state phase.  May be incremented if this call causes this element to be finished.
    // currentTime      - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool calculateNominalFlowRates(std::map<size_t, std::vector<StateMessage> >& outgoingMessages, size_t& elementsFinished, double currentTime);
    
    // Returns: The minimum value of expirationTime for all NeighborProxies.
    double minimumExpirationTime();
    
    // Perform precipitation, snowmelt, evaporation, transpiration, infiltration, and send lateral outflows.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages - Container to aggregate outgoing messages to other Regions.  Key is Region ID number of message destination.
    // elementsFinished - Number of elements in the current Region finished in the receive water phase.  May be incremented if this call causes this element to be finished.
    // currentTime      - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime  - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool doPointProcessesAndSendOutflows(std::map<size_t, std::vector<WaterMessage> >& outgoingMessages, size_t& elementsFinished, double currentTime, double timestepEndTime);
    
    // Recieve lateral inflows, move water through impedance layer, run aquifer capillary fringe solver, update water table heads, and resolve recharge.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // currentTime     - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool receiveInflowsAndUpdateState(double currentTime, double timestepEndTime);
    
    // Fill in this ChannelElement's values into a ChannelState object.
    //
    // Parameters:
    //
    // state - The ChannelState to fill in.
    inline void fillInState(ChannelState& state)
    {
        std::map<NeighborConnection, NeighborProxy>::iterator itProxy; // Loop iterator.
        std::vector<NeighborState>::iterator                  itState; // Loop iterator.
        
        state.elementNumber           = elementNumber;
        state.evapoTranspirationState = evapoTranspirationState;
        state.surfaceWater            = surfaceWater;
        state.surfaceWaterCreated     = surfaceWaterCreated;
        state.precipitationRate       = precipitationRate;
        state.precipitationCumulative = precipitationCumulativeShortTerm + precipitationCumulativeLongTerm;
        state.evaporationRate         = evaporationRate;
        state.evaporationCumulative   = evaporationCumulativeShortTerm + evaporationCumulativeLongTerm;
        
        state.neighbors.resize(neighbors.size());
        
        for (itProxy = neighbors.begin(), itState = state.neighbors.begin(); itProxy != neighbors.end(); ++itProxy, ++itState)
        {
            if (DEBUG_LEVEL && DEBUG_LEVEL_INTERNAL_SIMPLE)
            {
                // Because we resized state.neighbors we shouldn't run out of elements in this loop.
                CkAssert(itState != state.neighbors.end());
            }
            
            itProxy->second.fillInState(*itState, itProxy->first);
        }
    }
    
    // Returns: the value of elementNumber.
    inline double getElementNumber() const
    {
        return elementNumber;
    }
    
private:
    
    // Returns: (m) the surface water depth of a trapeziodal channel from the wetted cross sectional area.
    //
    // Parameters:
    //
    // crossSectionalArea - (m^2) Wetted cross sectional area of the channel.
    double surfaceWaterDepthFromCrossSectionalArea(double crossSectionalArea);
    
    // Returns: (m^2) the wetted cross sectional area of a trapeziodal channel from the surface water depth.
    //
    // Parameters:
    //
    // depth - (m) Surface water depth of the channel.
    inline double crossSectionalAreaFromSurfaceWaterDepth(double depth)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
        {
            CkAssert(0.0 <= depth);
        }
        
        return (baseWidth + sideSlope * depth) * depth;
    }
    
    // Immutable attributes of the element.
    size_t          elementNumber;   // ID number of this element.
    ChannelTypeEnum channelType;     // What type of channel this is.
    long long       reachCode;       // For waterbodies and icemasses, reach code.  For streams, original link number that element came from.
    double          elementX;        // (m) X coordinate of element center.
    double          elementY;        // (m) Y coordinate of element center.
    double          elementZBank;    // (m) Elevation above datum of the channel bank.
    double          elementZBed;     // (m) Elevation above datum of the channel bed.
    double          elementLength;   // (m) 1D length of element ignoring slope.
    double          latitude;        // (radians)
    double          longitude;       // (radians)
    double          baseWidth;       // (m) Width of channel base.
    double          sideSlope;       // (m/m) Widening of each side of the channel for each unit increase in water depth.  It is delta-x over delta-y, the inverse of the traditional definition of slope.
                                     // The width of the channel at height h above elementZBed is baseWidth + 2.0 * sideSlope * h.
                                     // The wetted cross sectional area of the channel is (baseWidth + sideSlope * surfaceWater) * surfaceWater;
    double          manningsN;       // (s/(m^(1/3))) Surface roughness parameter.
    double          bedThickness;    // (m) The thickness of the channel bed.
    double          bedConductivity; // (m/s) The hydraulic conductivity through the channel bed.
    
    // Forcing data changes over the course of the simulation, but it is sent by someone else and is never changed by the MeshElement so it's not immutable, but it's not state.
    EvapoTranspirationForcingStruct evapoTranspirationForcing; // Data structure containing the forcing used by Noah-MP.
    
    // Mutable state of the element.
    EvapoTranspirationStateStruct evapoTranspirationState; // Data structure containing the state used by Noah-MP.
    double                        surfaceWater;            // (m) Depth of ponded surface water.
    double                        surfaceWaterCreated;     // (m^3) Surface water created or destroyed for exceptional and possibly erroneous circumstances.  Positive means water was created.  Negative means water was destroyed.
    
    // These are variables for recording flows for point processes.  The rate variables are simple instantaneous rates.  The cumulative variables have some complexity with how they are stored.
    // For each cumulative quantity, flow is stored in two variables, a short term variable and a long term variable.  The purpose of this is to minimize roundoff error.
    // Cumulative flow can get quite large, and the additional flow of a single timestep that needs to be added to it can be quite small.  This can generate significant roundoff error.
    // To combat this, flow is first added to the short term variable, and then when the short term variable gets to a significant size it is moved to the long term variable.
    // There is no semantic difference between the short term and long term variables.  If you ever want to use the quantity of cumulative flow always use the sum of the short term and long term variables.
    double precipitationRate;                // (m/s) Negative means water added to the element.  Must be non-positive.
    double precipitationCumulativeShortTerm; // (m^3) Negative means water added to the element.  Must be non-positive.
    double precipitationCumulativeLongTerm;  // (m^3) Negative means water added to the element.  Must be non-positive.
    double evaporationRate;                  // (m/s) Positive means water evaporated from the element.  Negative means water condensed on to the element.
    double evaporationCumulativeShortTerm;   // (m^3) Positive means water evaporated from the element.  Negative means water condensed on to the element.
    double evaporationCumulativeLongTerm;    // (m^3) Positive means water evaporated from the element.  Negative means water condensed on to the element.
    
    // Neighbors of the element.
    std::map<NeighborConnection, NeighborProxy> neighbors;         // A map of NeighborProxies allowing the element to find a specific neighor or iterate over all neighbors.
                                                                   // Keys are NeighborConnection objects that uniquely identify each connection.
    size_t                                      neighborsFinished; // Number of neighbors finished in the current phase such as initialization, invariant check, receive state, or receive water.
                                                                   // This element is finished when neighborsFinished equals neighbors.size().
};

#endif // __CHANNEL_ELEMENT_H__
