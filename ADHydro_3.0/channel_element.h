#ifndef __CHANNEL_ELEMENT_H__
#define __CHANNEL_ELEMENT_H__

#include "neighbor_proxy.h"
#include "evapo_transpiration.h"

// A ChannelElement is a length of stream, a waterbody, or a glacier in the channel network.  It is modeled as a linear element.
// It simulates surfacewater state only.  Groundwater underneath the channel is simulated by neighboring mesh elements.
class ChannelElement
{
public:
    
    // Default constructor.  Only needed for pup_stl.h code.
    inline ChannelElement() : elementNumber(0), channelType(STREAM), reachCode(0), elementX(0.0), elementY(0.0), elementZBank(0.0), elementZBed(0.0), elementLength(1.0), latitude(0.0),
                              longitude(0.0), baseWidth(1.0), sideSlope(1.0), manningsN(1.0), bedThickness(1.0), bedConductivity(0.0), evapoTranspirationForcing(), evapoTranspirationState(),
                              surfaceWater(0.0), surfaceWaterCreated(0.0), precipitationRate(0.0), precipitationCumulativeShortTerm(0.0), precipitationCumulativeLongTerm(0.0),
                              evaporationRate(0.0), evaporationCumulativeShortTerm(0.0), evaporationCumulativeLongTerm(0.0), neighbors() {}
    
    // Constructor.  All parameters directly initialize member variables.  FIXME initialize neighbors somehow.
    inline ChannelElement(size_t elementNumber, ChannelTypeEnum channelType, long long reachCode, double elementX, double elementY, double elementZBank, double elementZBed, double elementLength,
                          double latitude, double longitude, double baseWidth, double sideSlope, double manningsN, double bedThickness, double bedConductivity,
                          const EvapoTranspirationStateStruct& evapoTranspirationState, double surfaceWater, double surfaceWaterCreated, double precipitationCumulative, double evaporationCumulative) :
        elementNumber(elementNumber), channelType(channelType), reachCode(reachCode), elementX(elementX), elementY(elementY), elementZBank(elementZBank), elementZBed(elementZBed),
        elementLength(elementLength), latitude(latitude), longitude(longitude), baseWidth(baseWidth), sideSlope(sideSlope), manningsN(manningsN), bedThickness(bedThickness),
        bedConductivity(bedConductivity), /* evapoTranspirationForcing initialized below. */ evapoTranspirationState(evapoTranspirationState), surfaceWater(surfaceWater),
        surfaceWaterCreated(surfaceWaterCreated), precipitationRate(0.0), precipitationCumulativeShortTerm(0.0), precipitationCumulativeLongTerm(precipitationCumulative),
        evaporationRate(0.0), evaporationCumulativeShortTerm(0.0), evaporationCumulativeLongTerm(evaporationCumulative), neighbors()
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
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
    // Call calculateNominalFlowRate on all NeighborProxies.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages - Container to aggregate outgoing messages to other regions.  Key is region ID number of message destination.
    // currentTime      - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool calculateNominalFlowRates(std::map<size_t, std::vector<StateMessage> >& outgoingMessages, double currentTime);
    
    // Pass the received StateMessage to the correct NeighborProxy so that it can calculate its nominalFlowRate.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // state - The received message.
    bool receiveState(const StateMessage& state);
    
    // Returns: The minimum value of expirationTime for all NeighborProxies.
    double minimumExpirationTime();
    
    // Perform precipitation, snowmelt, evaporation, transpiration, infiltration, and send lateral outflows.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages - Container to aggregate outgoing messages to other regions.  Key is region ID number of message destination.
    // currentTime      - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime  - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool doPointProcessesAndSendOutflows(std::map<size_t, std::vector<WaterMessage> >& outgoingMessages, double currentTime, double timestepEndTime);
    
    // Returns: true if allWaterHasArrived is true for all NeighborProxies that are inflows, false otherwise.  Exit on error.
    //
    // Parameters:
    //
    // currentTime     - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool allInflowsHaveArrived(double currentTime, double timestepEndTime);
    
    // Pass the received WaterMessage to the correct NeighborProxy so that it can add it to its incomingWater.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // water - The received message.
    bool receiveWater(const WaterMessage& water);
    
    // Recieve lateral inflows, move water through impedance layer, run aquifer capillary fringe solver, update water table heads, and resolve recharge.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // currentTime     - (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    // timestepEndTime - (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    bool receiveInflowsAndUpdateState(double currentTime, double timestepEndTime);
    
private:
    
    // Calculate the surface water depth in meters of a trapeziodal channel from the wetted cross sectional area.  Calculated value is stored in member variable surfaceWater.
    //
    // Parameters:
    //
    // crossSectionalArea - (m^2) Wetted cross sectional area of the channel.
    void calculateSurfaceWaterDepthFromCrossSectionalArea(double crossSectionalArea);
    
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
    
    // A map of NeighborProxies allowing the element to find a specific neighor or iterate over all neighbors.  Keys are NeighborConnection objects that uniquely identify each connection.
    std::map<NeighborConnection, NeighborProxy> neighbors;
};

#endif // __CHANNEL_ELEMENT_H__
