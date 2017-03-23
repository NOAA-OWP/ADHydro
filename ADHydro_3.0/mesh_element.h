#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

#include "neighbor_proxy.h"
#include "simple_groundwater.h"
#include "evapo_transpiration.h"

// A GroundwaterModeEnum describes the current mode of the multi-layer groundwater simulation in a single mesh element.
enum GroundwaterModeEnum
{
    SATURATED_AQUIFER,   // Implies no perched water table.
    UNSATURATED_AQUIFER, // Implies no perched water table.
    PERCHED_WATER_TABLE, // Implies aquifer is not saturated.
    NO_MULTILAYER,       // Implies at least one of soil and/or aquifer doesn't exist.
};

PUPbytes(GroundwaterModeEnum);

// A MeshElement is a triangular element in the mesh.  It simulates overland (non-channel) surfacewater state as well as groundwater and vadose zone state.
class MeshElement : public Element
{
public:
    
    // Default constructor.  Only needed for pup_stl.h code.
    inline MeshElement() : elementNumber(0), catchment(0), elementX(0.0), elementY(0.0), elementZ(0.0), elementArea(1.0), latitude(0.0), longitude(0.0), vegetationType(1), groundType(1), manningsN(1.0),
                           soilExists(false), impedanceConductivity(0.0), aquiferExists(false), deepConductivity(0.0), evapoTranspirationForcing(), evapoTranspirationState(), surfaceWater(0.0),
                           surfaceWaterCreated(0.0), groundwaterMode(NO_MULTILAYER), perchedHead(0.0), soilWater(1.0, 1.0, 1.0, 0.0, 0.0), soilWaterCreated(0.0), soilRecharge(0.0), aquiferHead(0.0),
                           aquiferWater(1.0, 1.0, 1.0, 0.0, 0.0), aquiferWaterCreated(0.0), aquiferRecharge(0.0), deepGroundwater(0.0), precipitationRate(0.0), precipitationCumulativeShortTerm(0.0),
                           precipitationCumulativeLongTerm(0.0), evaporationRate(0.0), evaporationCumulativeShortTerm(0.0), evaporationCumulativeLongTerm(0.0), transpirationRate(0.0),
                           transpirationCumulativeShortTerm(0.0), transpirationCumulativeLongTerm(0.0), neighbors() {}
    
    // Constructor.  All parameters directly initialize member variables.
    inline MeshElement(size_t elementNumber, size_t catchment, double elementX, double elementY, double elementZ, double elementArea, double latitude, double longitude, int vegetationType, int groundType,
                       double manningsN, bool soilExists, double impedanceConductivity, bool aquiferExists, double deepConductivity, const EvapoTranspirationStateStruct& evapoTranspirationState,
                       double surfaceWater, double surfaceWaterCreated, GroundwaterModeEnum groundwaterMode, double perchedHead, const SimpleGroundwater& soilWater, double soilWaterCreated, double aquiferHead,
                       const SimpleGroundwater& aquiferWater, double aquiferWaterCreated, double deepGroundwater, double precipitationCumulative, double evaporationCumulative, double transpirationCumulative,
                       const std::map<NeighborConnection, NeighborProxy>& neighbors) :
        elementNumber(elementNumber), catchment(catchment), elementX(elementX), elementY(elementY), elementZ(elementZ), elementArea(elementArea), latitude(latitude), longitude(longitude),
        vegetationType(vegetationType), groundType(groundType), manningsN(manningsN), soilExists(soilExists), impedanceConductivity(impedanceConductivity), aquiferExists(aquiferExists),
        deepConductivity(deepConductivity), /* evapoTranspirationForcing initialized below. */ evapoTranspirationState(evapoTranspirationState), surfaceWater(surfaceWater),
        surfaceWaterCreated(surfaceWaterCreated), groundwaterMode(groundwaterMode), perchedHead(perchedHead), soilWater(soilWater), soilWaterCreated(soilWaterCreated), soilRecharge(0.0),
        aquiferHead(aquiferHead), aquiferWater(aquiferWater), aquiferWaterCreated(aquiferWaterCreated), aquiferRecharge(0.0), deepGroundwater(deepGroundwater), precipitationRate(0.0),
        precipitationCumulativeShortTerm(0.0), precipitationCumulativeLongTerm(precipitationCumulative), evaporationRate(0.0), evaporationCumulativeShortTerm(0.0),
        evaporationCumulativeLongTerm(evaporationCumulative), transpirationRate(0.0), transpirationCumulativeShortTerm(0.0), transpirationCumulativeLongTerm(transpirationCumulative), neighbors(neighbors)
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
        p | catchment;
        p | elementX;
        p | elementY;
        p | elementZ;
        p | elementArea;
        p | latitude;
        p | longitude;
        p | vegetationType;
        p | groundType;
        p | manningsN;
        p | soilExists;
        p | impedanceConductivity;
        p | aquiferExists;
        p | deepConductivity;
        p | evapoTranspirationForcing;
        p | evapoTranspirationState;
        p | surfaceWater;
        p | surfaceWaterCreated;
        p | groundwaterMode;
        p | perchedHead;
        p | soilWater;
        p | soilWaterCreated;
        p | soilRecharge;
        p | aquiferHead;
        p | aquiferWater;
        p | aquiferWaterCreated;
        p | aquiferRecharge;
        p | deepGroundwater;
        p | precipitationRate;
        p | precipitationCumulativeShortTerm;
        p | precipitationCumulativeLongTerm;
        p | evaporationRate;
        p | evaporationCumulativeShortTerm;
        p | evaporationCumulativeLongTerm;
        p | transpirationRate;
        p | transpirationCumulativeShortTerm;
        p | transpirationCumulativeLongTerm;
        p | neighbors;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
    // Call sendNeighborAttributes on all NeighborProxies.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // outgoingMessages - Container to aggregate outgoing messages to other regions.  Key is region ID number of message destination.
    bool sendNeighborAttributes(std::map<size_t, std::vector<NeighborMessage> >& outgoingMessages);
    
    // Returns: true if attributesInitialized is true for all NeighborProxies, false otherwise.
    bool allNeighborAttributesInitialized();
    
    // Pass the received NeighborMessage to the correct NeighborProxy so that it can save the NeighborAttributes.
    //
    // Returns: true if there is an error, false otherwise.
    //
    // Parameters:
    //
    // message - The received message.
    bool receiveNeighborAttributes(const NeighborMessage& message);
    
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
    
    // Returns: the value of elementNumber.
    inline double getElementNumber() const
    {
        return elementNumber;
    }
    
private:
    
    // Fill in the soil moisture profile into a struct needed by Noah-MP.  I believe this is the best place for this function.
    // This function requires knowledge of both the vadose zone data structures and Noah-MP data structures so putting it in either of those modules creates an undesirable linkage.
    //
    // Parameters:
    //
    // evapoTranspirationSoilMoisture - Will be filled in with the soil moisture profile in a format needed by Noah-MP.
    void fillInEvapoTranspirationSoilMoistureStruct(EvapoTranspirationSoilMoistureStruct& evapoTranspirationSoilMoisture);
    
    // Update head based on recharge.  If recharge is positive, head is raised.  If recharge is negative, head is lowered.
    // This creates a stable feedback loop that will find a head that produces near zero net recharge after accounting for all flows.
    // At the end, if head is greater than maxHead it is set to maxHead.  This can be used to prevent head from rising above the top of a given layer.
    //
    // Parameters:
    //
    // head     - (m) The elevation above datum of a water table.  Will be updated to a new value.
    // recharge - (m) A thickness of water that is left over in a recharge bucket after accounting for all flows.  Can be negative for a water deficit.
    // maxHead  - (m) The maximum value allowed for the updated value of head.
    void updateHead(double& head, double recharge, double maxHead = INFINITY);
    
    // A helper function for updating state when there is a saturated aquifer.
    void resolveSoilRechargeSaturatedAquifer();
    
    // A helper function for updating state when there is a perched water table.
    void resolveSoilRechargePerchedWaterTable();
    
    // A helper function for updating state when there is an unsaturated aquifer.
    void resolveAquiferRechargeUnsaturatedAquifer();
    
    // Returns: (m) the thickness of the soil layer, which is the thickness stored in soilWater if soilExists is true, zero otherwise.
    inline double soilThickness() const
    {
        return (soilExists ? soilWater.getThickness() : 0.0);
    }
    
    // Returns: (m) the thickness of the aquifer layer, which is the thickness stored in aquiferWater if aquiferExists is true, zero otherwise.
    inline double aquiferThickness() const
    {
        return (aquiferExists ? aquiferWater.getThickness() : 0.0);
    }
    
    // Immutable attributes of the element.
    size_t elementNumber;         // ID number of this element.
    size_t catchment;             // ID number of the catchment this element belongs to.
    double elementX;              // (m) X coordinate of element center.
    double elementY;              // (m) Y coordinate of element center.
    double elementZ;              // (m) Elevation above datum of the land surface.
    double elementArea;           // (m^2) 2D surface area of element ignoring slope.
    double latitude;              // (radians)
    double longitude;             // (radians)
    int    vegetationType;        // Type of land cover or land use.
    int    groundType;            // Type of ground material exposed at the surface.  This could be the type of dirt in the soil layer, or the type of material in the aquifer layer if there is no soil layer,
                                  // or something like pavement if there is no soil layer but there is an impedance layer above the aquifer, or it could be bedrock if there is no soil or aquifer layers.
    double manningsN;             // (s/(m^(1/3))) Surface roughness parameter.
    bool   soilExists;            // True if a soil layer exists for this element.
    double impedanceConductivity; // (m/s) Hydraulic conductivity through impedance layer.
    bool   aquiferExists;         // True if an aquifer layer exists for this element.
    double deepConductivity;      // (m/s) Hydraulic conductivity of leakage to deep groundwater.
    
    // Forcing data changes over the course of the simulation, but it is sent by someone else and is never changed by the MeshElement so it's not immutable, but it's not state.
    EvapoTranspirationForcingStruct evapoTranspirationForcing; // Data structure containing the forcing used by Noah-MP.
    
    // Mutable state of the element.
    EvapoTranspirationStateStruct evapoTranspirationState; // Data structure containing the state used by Noah-MP.
    double                        surfaceWater;            // (m) Depth of ponded surface water.
    double                        surfaceWaterCreated;     // (m) Surface water created or destroyed for exceptional and possibly erroneous circumstances.  Positive means water was created.  Negative means water was destroyed.
    GroundwaterModeEnum           groundwaterMode;         // The mode corresponding to the current groundwater state.
    double                        perchedHead;             // (m) Elevation above datum of the perched water table if it exists.  Only valid when groundwaterMode is PERCHED_WATER_TABLE or NO_MULTILAYER and soil exists.
    SimpleGroundwater             soilWater;               // Data structure containing the moisture content profile of the soil layer.
    double                        soilWaterCreated;        // (m) Soil water created or destroyed for exceptional and possibly erroneous circumstances.  Positive means water was created.  Negative means water was destroyed.
    double                        soilRecharge;            // (m) Temporary holding bucket for water that is moving to/from the soil expressed as a thickness of water.  Can be negative for a water deficit.
    double                        aquiferHead;             // (m) Elevation above datum of the aquifer water table.
    SimpleGroundwater             aquiferWater;            // Data structure containing the moisture content profile of the aquifer layer.
    double                        aquiferWaterCreated;     // (m) Aquifer water created or destroyed for exceptional and possibly erroneous circumstances.  Positive means water was created.  Negative means water was destroyed.
    double                        aquiferRecharge;         // (m) Temporary holding bucket for water that is moving to/from the aquifer expressed as a thickness of water.  Can be negative for a water deficit.
    double                        deepGroundwater;         // (m) Water that has reached a deep aquifer.  FIXME this might not be accounted for within individual elements separately.
    
    // These are variables for recording flows for point processes.  The rate variables are simple instantaneous rates.  The cumulative variables have some complexity with how they are stored.
    // For each cumulative quantity, flow is stored in two variables, a short term variable and a long term variable.  The purpose of this is to minimize roundoff error.
    // Cumulative flow can get quite large, and the additional flow of a single timestep that needs to be added to it can be quite small.  This can generate significant roundoff error.
    // To combat this, flow is first added to the short term variable, and then when the short term variable gets to a significant size it is moved to the long term variable.
    // There is no semantic difference between the short term and long term variables.  If you ever want to use the quantity of cumulative flow always use the sum of the short term and long term variables.
    double precipitationRate;                // (m/s) Negative means water added to the element.  Must be non-positive.
    double precipitationCumulativeShortTerm; // (m)   Negative means water added to the element.  Must be non-positive.
    double precipitationCumulativeLongTerm;  // (m)   Negative means water added to the element.  Must be non-positive.
    double evaporationRate;                  // (m/s) Positive means water evaporated from the element.  Negative means water condensed on to the element.
    double evaporationCumulativeShortTerm;   // (m)   Positive means water evaporated from the element.  Negative means water condensed on to the element.
    double evaporationCumulativeLongTerm;    // (m)   Positive means water evaporated from the element.  Negative means water condensed on to the element.
    double transpirationRate;                // (m/s) Positive means water transpired from the element.  Must be non-negative.
    double transpirationCumulativeShortTerm; // (m)   Positive means water transpired from the element.  Must be non-negative.
    double transpirationCumulativeLongTerm;  // (m)   Positive means water transpired from the element.  Must be non-negative.
    
    // A map of NeighborProxies allowing the element to find a specific neighor or iterate over all neighbors.  Keys are NeighborConnection objects that uniquely identify each connection.
    std::map<NeighborConnection, NeighborProxy> neighbors;
};

#endif // __MESH_ELEMENT_H__
