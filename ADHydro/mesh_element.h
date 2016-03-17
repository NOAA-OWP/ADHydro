#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

#include "neighbor_proxy.h"
#include "evapo_transpiration.h"

// mesh_element.h needs to know about the Region class, but region.h includes mesh_element.h so we just do a forward declaration of the Region class to
// break the circularity.
class Region;

// Proxy for a mesh element's surfacewater neighbor that is a mesh element.
class MeshSurfacewaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  MeshSurfacewaterMeshNeighborProxy();
  
  // Constructor.
  //
  // All parameters directly initialize superclass and member variables.  For
  // description see superclass and member variables.
  //
  // incomingMaterial is initialized to empty.  neighborInitialized and
  // neighborInvariantChecked are initialized to false.
  MeshSurfacewaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, double flowCumulativeShortTermInit,
                                    double flowCumulativeLongTermInit, int regionInit, int neighborInit,
                                    int reciprocalNeighborProxyInit, double neighborXInit, double neighborYInit,
                                    double neighborZSurfaceInit, double neighborAreaInit, double edgeLengthInit,
                                    double edgeNormalXInit, double edgeNormalYInit, double neighborManningsNInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.  Does not check that neighbor values are the
  // same as the values stored at the neighbor.  That is done elsewhere with
  // messages.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  // Identification parameters.
  int region;                  // Region number where the neighbor is.
  int neighbor;                // Mesh element ID number of the neighbor or boundary condition code.
  int reciprocalNeighborProxy; // The index into neighbor's meshNeighbors vector of the proxy for me.
  
  // Geometric coordinates.
  double neighborX;        // Meters.
  double neighborY;        // Meters.
  double neighborZSurface; // Elevation in meters.
  double neighborArea;     // Square meters.
  double edgeLength;       // Meters.  2-D distance ignoring Z coordinates.
  double edgeNormalX;      // X component of normal unit vector, unitless.
  double edgeNormalY;      // Y component of normal unit vector, unitless.
  
  // Hydraulic parameters.
  double neighborManningsN; // Roughness parameter in seconds/(meters^(1/3)).
  
  // Flags
  bool neighborInitialized;      // Whether the information for this neighbor has been initialized.
  bool neighborInvariantChecked; // Whether the information for this neighbor has been invariant checked.
};

// Proxy for a mesh element's surfacewater neighbor that is a channel element.
class MeshSurfacewaterChannelNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  MeshSurfacewaterChannelNeighborProxy();
  
  // Constructor.
  //
  // All parameters directly initialize superclass and member variables.  For
  // description see superclass and member variables.
  //
  // incomingMaterial is initialized to empty.  neighborInitialized and
  // neighborInvariantChecked are initialized to false.
  MeshSurfacewaterChannelNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                       double flowCumulativeShortTermInit, double flowCumulativeLongTermInit,
                                       int regionInit, int neighborInit, int reciprocalNeighborProxyInit,
                                       double neighborZBankInit, double neighborZBedInit, double neighborZOffsetInit,
                                       double edgeLengthInit, double neighborBaseWidthInit, double neighborSideSlopeInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.  Does not check that neighbor values are the
  // same as the values stored at the neighbor.  That is done elsewhere with
  // messages.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  // Identification parameters.
  int region;                  // Region number where the neighbor is.
  int neighbor;                // Channel element ID number of the neighbor.
  int reciprocalNeighborProxy; // The index into neighbor's meshNeighbors vector of the proxy for me.
  
  // Geometric coordinates.
  double neighborZBank;   // Elevation in meters.
  double neighborZBed;    // Elevation in meters.
  double neighborZOffset; // Meters.  The Z coordinate at the center of a mesh element is not the same as the Z coordinate at the edge of the element next to
                          // its channel neighbor.  This Z offset, which can be positive or negative, is added to mesh element center Z surface, Z bottom, and
                          // groundwater head elevations when used for interaction with this channel neighbor.
  double edgeLength;      // Meters.  2-D distance ignoring Z coordinates.
  
  // Hydraulic parameters.
  double neighborBaseWidth; // Width of channel base in meters.
  double neighborSideSlope; // Widening of each side of the channel for each unit increase in water depth.  It is delta-x over delta-y, the inverse of the
                            // traditional definition of slope, unitless.
  
  // Flags
  bool neighborInitialized;      // Whether the information for this neighbor has been initialized.
  bool neighborInvariantChecked; // Whether the information for this neighbor has been invariant checked.
};

// Proxy for a mesh element's groundwater neighbor that is a mesh element.
class MeshGroundwaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  MeshGroundwaterMeshNeighborProxy();
  
  // Constructor.
  //
  // All parameters directly initialize superclass and member variables.  For
  // description see superclass and member variables.
  //
  // incomingMaterial is initialized to empty.  neighborInitialized and
  // neighborInvariantChecked are initialized to false.
  MeshGroundwaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, double flowCumulativeShortTermInit,
                                   double flowCumulativeLongTermInit, int regionInit, int neighborInit,
                                   int reciprocalNeighborProxyInit, double neighborXInit, double neighborYInit,
                                   double neighborZSurfaceInit, double neighborLayerZBottomInit, double neighborAreaInit,
                                   double edgeLengthInit, double edgeNormalXInit, double edgeNormalYInit,
                                   double neighborConductivityInit, double neighborPorosityInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.  Does not check that neighbor values are the
  // same as the values stored at the neighbor.  That is done elsewhere with
  // messages.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  // Identification parameters.
  int region;                  // Region number where the neighbor is.
  int neighbor;                // Mesh element ID number of the neighbor.
  int reciprocalNeighborProxy; // The index into neighbor's meshNeighbors vector of the proxy for me.
  
  // Geometric coordinates.
  double neighborX;            // Meters.
  double neighborY;            // Meters.
  double neighborZSurface;     // Elevation in meters.
  double neighborLayerZBottom; // Elevation in meters.
  double neighborArea;         // Square meters.
  double edgeLength;           // Meters.  2-D distance ignoring Z coordinates.
  double edgeNormalX;          // X component of normal unit vector, unitless.
  double edgeNormalY;          // Y component of normal unit vector, unitless.
  
  // Hydraulic parameters.
  double neighborConductivity; // Meters per second.
  double neighborPorosity;     // Unitless.
  
  // Flags
  bool neighborInitialized;      // Whether the information for this neighbor has been initialized.
  bool neighborInvariantChecked; // Whether the information for this neighbor has been invariant checked.
};

// Proxy for a mesh element's groundwater neighbor that is a channel element.
class MeshGroundwaterChannelNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  MeshGroundwaterChannelNeighborProxy();
  
  // Constructor.
  //
  // All parameters directly initialize superclass and member variables.  For
  // description see superclass and member variables.
  //
  // incomingMaterial is initialized to empty.  neighborInitialized and
  // neighborInvariantChecked are initialized to false.
  MeshGroundwaterChannelNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                      double flowCumulativeShortTermInit, double flowCumulativeLongTermInit, int regionInit,
                                      int neighborInit, int reciprocalNeighborProxyInit, double neighborZBankInit,
                                      double neighborZBedInit, double neighborZOffsetInit, double edgeLengthInit,
                                      double neighborBaseWidthInit, double neighborSideSlopeInit,
                                      double neighborBedConductivityInit, double neighborBedThicknessInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.  Does not check that neighbor values are the
  // same as the values stored at the neighbor.  That is done elsewhere with
  // messages.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  // Identification parameters.
  int region;                  // Region number where the neighbor is.
  int neighbor;                // Channel element ID number of the neighbor.
  int reciprocalNeighborProxy; // The index into neighbor's undergroundMeshNeighbors vector of the proxy for me.
  
  // Geometric coordinates.
  double neighborZBank;   // Meters.
  double neighborZBed;    // Meters.
  double neighborZOffset; // Meters.  The Z coordinate at the center of a mesh element is not the same as the Z coordinate at the edge of the element next to
                          // its channel neighbor.  This Z offset, which can be positive or negative, is added to mesh element center Z surface, Z bottom, and
                          // groundwater head elevations when used for interaction with this channel neighbor.
  double edgeLength;      // Meters.  2-D distance ignoring Z coordinates.
  
  // Hydraulic parameters.
  double neighborBaseWidth;       // Width of channel base in meters.
  double neighborSideSlope;       // Widening of each side of the channel for each unit increase in water depth.  It is delta-x over delta-y, the inverse of
                                  // the traditional definition of slope, unitless.
  double neighborBedConductivity; // Meters per second.
  double neighborBedThickness;    // Meters.
  
  // Flags
  bool neighborInitialized;      // Whether the information for this neighbor has been initialized.
  bool neighborInvariantChecked; // Whether the information for this neighbor has been invariant checked.
};

// An InfiltrationAndGroundwater simulates the groundwater and vadose zone
// state for a MeshElement.
class InfiltrationAndGroundwater
{
public:
  
  enum InfiltrationMethodEnum
  {
    NO_INFILTRATION,      // This is a situation like exposed bedrock where there is no possibility of infiltration and no groundwater storage.  Almost all
                          // member variables are ignored except that porosity must still have a valid value because Noah-MP doesn't allow layers of zero
                          // thickness or zero porosity so we still have to fill in the soil moisture struct with something.
    TRIVIAL_INFILTRATION, // This method infiltrates surfacewater directly to groundwater at a rate equal to conductivity.  No vadose zone state is modeled.
                          // Whenever vadose zone state is needed it is assumed to be in equilibrium with groundwaterHead.  If groundwaterMethod is
                          // SHALLOW_AQUIFER then quantity of water in the domain is groundwaterRecharge + (groundwaterHead - layerZBottom) * porosity, and no
                          // vadose zone water is counted.  If groundwaterMethod is DEEP_AQUIFER then quantity of water in the domain is groundwaterRecharge.
    GARTO_INFILTRATION,   // Vadose zone water is simulated using the Green-Ampt infiltration and redistribution method for the surface wetting front and the
                          // T-O method for the groundwater front.
  };
  
  enum GroundwaterMethodEnum
  {
    NO_AQUIFER,      // This method must be used if and only if infiltrationMethod is NO_INFILTRATION.
    DEEP_AQUIFER,    // The water table is assumed to always be below the modeled domain.  Any water that makes it to groundwaterRecharge gets sent immediately
                     // to an aquifer storage bucket, which may collect water from many elements.  There is no lateral flow to neighboring elements.  The
                     // meshNeighbors and channelNeighbors vectors must be empty.  The member variable groundwaterHead still needs a valid value because it is
                     // used to fill in the soil moisture struct.  The value should be below layerZBottom, and its value is never changed.
    SHALLOW_AQUIFER, // Both vadose zone and water table are modeled.  Water that makes it to groundwaterRecharge stays in the modeled domain.  Water below
                     // groundwaterHead may flow laterally to neighboring elements.
  };
  
  // Because the vadose zone state may contain dynamically allocated data we need to have a copy constructor and assignment operator to make a deep copy of it
  // and a destructor to deallocate it.  This class wraps a pointer to the vadose zone state and provides those methods.
  class VadoseZone
  {
  public:
    
    // Default constructor.  Only needed for pup_stl.h code.
    VadoseZone();
    
    // Constructor.
    //
    // All parameters directly initialize member variables.  For description
    // see member variables.  The VadoseZone object takes ownership of any
    // dynamic data pointed to by vadoseZoneStateInit.  No deep copy is made.
    VadoseZone(InfiltrationMethodEnum infiltrationMethodInit, void* vadoseZoneStateInit);
    
    // Copy constructor.
    VadoseZone(const VadoseZone &other);
    
    // Destructor.
    ~VadoseZone();
    
    // Assignment operator.
    VadoseZone& operator=(const VadoseZone &other);
    
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
    
    // The method to use to calculate infiltration.
    InfiltrationMethodEnum infiltrationMethod;
    
    // The state of the vadose zone.  What this is depends on infiltrationMethod:
    //   NO_INFILTRATION      - NULL.
    //   TRIVIAL_INFILTRATION - NULL.
    //   GARTO_INFILTRATION   - Pointer to garto_domain.
    void* state;
  };
  
  // Default constructor.  Only needed for pup_stl.h code.
  InfiltrationAndGroundwater();
  
  // Constructor.
  //
  // All parameters directly initialize member variables.  For description see
  // member variables.
  //
  // meshNeighbors and channelNeighbors are initialized to empty.
  InfiltrationAndGroundwater(GroundwaterMethodEnum  groundwaterMethodInit, int soilTypeInit, double layerZBottomInit,
                             double slopeXInit, double slopeYInit, double conductivityInit, double porosityInit,
                             double groundwaterHeadInit, double groundwaterRechargeInit, double groundwaterErrorInit,
                             VadoseZone& vadoseZoneInit);
  
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
  
  // Calculate new values for nominalFlowRate and expirationTime for a proxy in
  // meshNeighbors.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // currentTime               - Current simulation time in seconds since
  //                             Element::referenceDate.
  // regionalDtLimit           - Maximum duration in seconds before the new
  //                             expirationTime.  If the neighbor is in a
  //                             different region you must pass in the minimum
  //                             of the time limits of the two regions.
  // neighborProxyIndex        - Which index of meshNeighbors is the proxy for
  //                             the neighbor.
  // elementX                  - Pass value from MeshElement object.
  // elementY                  - Pass value from MeshElement object.
  // elementZSurface           - Pass value from MeshElement object.
  // elementArea               - Pass value from MeshElement object.
  // elementSurfacewaterDepth  - Pass value from MeshElement object.
  // neighborSurfacewaterDepth - Meters.
  // neighborGroundwaterHead   - Elevation in meters.
  bool calculateNominalFlowRateWithGroundwaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                           std::vector<MeshGroundwaterMeshNeighborProxy>::size_type neighborProxyIndex, double elementX,
                                                           double elementY, double elementZSurface, double elementArea, double elementSurfacewaterDepth,
                                                           double neighborSurfacewaterDepth, double neighborGroundwaterHead);
  
  // Calculate new values for nominalFlowRate and expirationTime for a proxy in
  // channelNeighbors.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // currentTime               - Current simulation time in seconds since
  //                             Element::referenceDate.
  // regionalDtLimit           - Maximum duration in seconds before the new
  //                             expirationTime.  If the neighbor is in a
  //                             different region you must pass in the minimum
  //                             of the time limits of the two regions.
  // neighborProxyIndex        - Which index of meshNeighbors is the proxy for
  //                             the neighbor.
  // elementZSurface           - Pass value from MeshElement object.
  // elementSurfacewaterDepth  - Pass value from MeshElement object.
  // neighborSurfacewaterDepth - Meters.
  bool calculateNominalFlowRateWithGroundwaterChannelNeighbor(double currentTime, double regionalDtLimit,
                                                              std::vector<MeshGroundwaterChannelNeighborProxy>::size_type neighborProxyIndex,
                                                              double elementZSurface, double elementSurfacewaterDepth, double neighborSurfacewaterDepth);
  
  // Fill in the evapo-transpiration soil moisture variables from the current
  // groundwater state.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // elementZSurface - Surface elevation in meters.
  // zSnso           - 1D array of layer bottom depth in meters from snow
  //                   surface of each snow and soil layer.  Specify depths as
  //                   negative numbers.
  // evapoTranspirationSoilMoisture
  //                 - Soil moisture struct passed by reference will be filled
  //                   in based on current groundwater state.
  bool fillInEvapoTranspirationSoilMoistureStruct(double elementZSurface, float zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS],
                                                  EvapoTranspirationSoilMoistureStruct& evapoTranspirationSoilMoisture);
  
  // Remove surface evaporation from groundwater.
  //
  // Returns: The amount of water in meters that was able to be supplied.
  //
  // Parameters:
  //
  // unsatisfiedEvaporation - The requested amount of water in meters.
  // elementZSurface        - Pass value from MeshElement object.
  double evaporate(double unsatisfiedEvaporation, double elementZSurface);
  
  // Remove transpiration from groundwater.
  //
  // Returns: The amount of water in meters that was able to be supplied.
  //
  // Parameters:
  //
  // unsatisfiedTranspiration - The requested amount of water in meters.
  // elementZSurface          - Pass value from MeshElement object.
  double transpire(double unsatisfiedTranspiration, double elementZSurface);
  
  // Perform infiltration, which takes water from surfacewaterDepth and takes
  // or puts water into groundwaterRecharge.  Then limit and send groundwater
  // outflows taking the water from groundwaterRecharge.  Groundwater inflows
  // from last timestep will already be in groundwaterRecharge.  Then update
  // groundwaterHead based on net groundwaterRecharge and take or put
  // groundwaterRecharge water back into the vadose zone, or if the vadose zone
  // is full put it back in surfacewaterDepth.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // currentTime       - Current simulation time in seconds since
  //                     Element::referenceDate.
  // timestepEndTime   - Simulation time at the end of the current timestep in
  //                     seconds since Element::referenceDate.
  // elementZSurface   - Pass value from MeshElement object.
  // elementArea       - Pass value from MeshElement object.
  // surfacewaterDepth - Scalar passed by reference containing the amount of
  //                     surfacewater available for infiltration in meters.
  //                     Will be updated to the new amount of surfacewater
  //                     after infiltration or exfiltration.
  // region            - This element's region for sending water messages.
  bool doInfiltrationAndSendGroundwaterOutflows(double currentTime, double timestepEndTime, double elementZSurface,
                                                double elementArea, double& surfacewaterDepth, Region& region);
  
  // Returns: whether all inflows have arrived that are required to advance
  // time to timestepEndTime.
  //
  // Parameters:
  //
  // currentTime     - Current simulation time in seconds since
  //                   Element::referenceDate.
  // timestepEndTime - Simulation time at the end of the current timestep in
  //                   seconds since referenceDate.
  bool allInflowsArrived(double currentTime, double timestepEndTime);
  
  // Update state to incorporate all incoming water from incomingMaterial lists
  // up to timestepEndTime.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // currentTime     - Current simulation time in seconds since
  //                   Element::referenceDate.
  // timestepEndTime - Simulation time at the end of the current timestep in
  //                   seconds since referenceDate.
  // elementArea     - Pass value from MeshElement object.
  bool receiveInflows(double currentTime, double timestepEndTime, double elementArea);
  
  // Compute values relevant to the mass balance calculation.  To calculate the
  // mass balance take waterInDomain and add externalFlows and subtract
  // waterError.  This will undo any insertion or removal of water from the
  // "black box" of the simulation domain leaving the amount of water that was
  // present when externalFlows and waterError were both zero.  This value
  // summed over all elements should be invariant except for floating point
  // roundoff error.
  //
  // For all three parameters the value for this element is added to whatever
  // value already exists in the passed-in variable.  It is done this way to
  // make it easy to accumulate values from multiple elements.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // waterInDomain - Scalar passed by reference.  The amount of water in cubic
  //                 meters in this element will be added to the existing
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
  // elementArea   - Pass value from MeshElement object.
  bool massBalance(double& waterInDomain, double& externalFlows, double& waterError, double elementArea);
  
  // The method to use to calculate groundwater flow.
  GroundwaterMethodEnum  groundwaterMethod;
  
  // Identification parameters.
  int soilType; // Soil type form Noah-MP SOILPARM.TBL file.
  
  // Geometric coordinates.
  double layerZBottom; // Elevation in meters.
  double slopeX;       // Layer bottom slope in X direction, unitless.
  double slopeY;       // Layer bottom slope in Y direction, unitless.
  
  // Hydraulic parameters.
  double conductivity; // Meters per second.
  double porosity;     // Unitless.
  
  // Water state variables.
  double groundwaterHead;     // Elevation in meters.
  double groundwaterRecharge; // Meters of water.
  double groundwaterError;    // Meters of water.  Positive means water was created.  Negative means water was destroyed.
  
  // Vadose zone variables.
  VadoseZone vadoseZone;
  
  // Neighbors.
  std::vector<MeshGroundwaterMeshNeighborProxy>    meshNeighbors;
  std::vector<MeshGroundwaterChannelNeighborProxy> channelNeighbors;
};

PUPbytes(InfiltrationAndGroundwater::InfiltrationMethodEnum);
PUPbytes(InfiltrationAndGroundwater::GroundwaterMethodEnum);

// A MeshElement is a triangular element in the mesh.  It simulates the
// overland (non-channel) surfacewater state, and through a contained
// InfiltrationAndGroundwater also the groundwater and vadose zone state.
class MeshElement
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  MeshElement();
  
  // Constructor.
  //
  // All parameters directly initialize member variables.  For description see
  // member variables.
  //
  // meshNeighbors and channelNeighbors are initialized to empty.
  MeshElement(int elementNumberInit, int catchmentInit, int vegetationTypeInit, int soilTypeInit, double vertexXInit[3], double vertexYInit[3],
              double elementXInit, double elementYInit, double elementZSurfaceInit, double layerZBottomInit, double elementAreaInit,
              double slopeXInit, double slopeYInit, double latitudeInit, double longitudeInit, double manningsNInit, double conductivityInit,
              double porosityInit, double surfacewaterDepthInit, double surfacewaterErrorInit, double groundwaterHeadInit,
              double groundwaterRechargeInit, double groundwaterErrorInit, double precipitationRateInit,
              double precipitationCumulativeShortTermInit, double precipitationCumulativeLongTermInit, double evaporationRateInit,
              double evaporationCumulativeShortTermInit, double evaporationCumulativeLongTermInit, double transpirationRateInit,
              double transpirationCumulativeShortTermInit, double transpirationCumulativeLongTermInit,
              EvapoTranspirationForcingStruct& evapoTranspirationForcingInit, EvapoTranspirationStateStruct& evapoTranspirationStateInit,
              InfiltrationAndGroundwater::GroundwaterMethodEnum  groundwaterMethodInit, InfiltrationAndGroundwater::VadoseZone vadoseZoneInit);
  
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
  
  // Calculate new values for nominalFlowRate and expirationTime for a proxy in
  // meshNeighbors.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // currentTime               - Current simulation time in seconds since
  //                             Element::referenceDate.
  // regionalDtLimit           - Maximum duration in seconds before the new
  //                             expirationTime.  If the neighbor is in a
  //                             different region you must pass in the minimum
  //                             of the time limits of the two regions.
  // neighborProxyIndex        - Which index of meshNeighbors is the proxy for
  //                             the neighbor.
  // neighborSurfacewaterDepth - Meters.
  bool calculateNominalFlowRateWithSurfacewaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                            std::vector<MeshSurfacewaterMeshNeighborProxy>::size_type neighborProxyIndex,
                                                            double neighborSurfacewaterDepth);
  
  // Calculate new values for nominalFlowRate and expirationTime for a proxy in
  // channelNeighbors.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // currentTime               - Current simulation time in seconds since
  //                             Element::referenceDate.
  // regionalDtLimit           - Maximum duration in seconds before the new
  //                             expirationTime.  If the neighbor is in a
  //                             different region you must pass in the minimum
  //                             of the time limits of the two regions.
  // neighborProxyIndex        - Which index of channelNeighbors is the proxy
  //                             for the neighbor.
  // neighborSurfacewaterDepth - Meters.
  bool calculateNominalFlowRateWithSurfacewaterChannelNeighbor(double currentTime, double regionalDtLimit,
                                                               std::vector<MeshSurfacewaterChannelNeighborProxy>::size_type neighborProxyIndex,
                                                               double neighborSurfacewaterDepth);
  
  // Update state for point processes that require no communication with other
  // elements.  Then send outflow water to neighbors.  Handles surfacewater and
  // calls underground to handle groundwater.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // referenceDate   - Julian date when currentTime is zero.  The current date
  //                   and time of the simulation is the Julian date equal to
  //                   referenceDate + currentTime / (24.0 * 60.0 * 60.0).
  //                   Time zone is UTC.
  // currentTime     - Current simulation time in seconds since referenceDate.
  // timestepEndTime - Simulation time at the end of the current timestep in
  //                   seconds since referenceDate.
  // region          - This element's region for sending water messages.
  bool doPointProcessesAndSendOutflows(double referenceDate, double currentTime, double timestepEndTime, Region& region);
  
  // Returns: whether all inflows have arrived that are required to advance
  //          time to timestepEndTime.  Handles surfacewater and calls
  //          underground to handle groundwater.
  //
  // Parameters:
  //
  // currentTime     - Current simulation time in seconds since
  //                   Element::referenceDate.
  // timestepEndTime - Simulation time at the end of the current timestep in
  //                   seconds since referenceDate.
  bool allInflowsArrived(double currentTime, double timestepEndTime);
  
  // Update state to incorporate all incoming water from incomingMaterial lists
  // up to timestepEndTime.  Handles surfacewater and calls underground to
  // handle groundwater.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // currentTime     - Current simulation time in seconds since
  //                   Element::referenceDate.
  // timestepEndTime - Simulation time at the end of the current timestep in
  //                   seconds since referenceDate.
  bool receiveInflows(double currentTime, double timestepEndTime);
  
  // Compute values relevant to the mass balance calculation.  To calculate the
  // mass balance take waterInDomain and add externalFlows and subtract
  // waterError.  This will undo any insertion or removal of water from the
  // "black box" of the simulation domain leaving the amount of water that was
  // present when externalFlows and waterError were both zero.  This value
  // summed over all elements should be invariant except for floating point
  // roundoff error.
  //
  // For all three parameters the value for this element is added to whatever
  // value already exists in the passed-in variable.  It is done this way to
  // make it easy to accumulate values from multiple elements.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // waterInDomain - Scalar passed by reference.  The amount of water in cubic
  //                 meters in this element will be added to the existing
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
  
  // Identification parameters.
  int elementNumber;  // Mesh element ID number of this element.
  int catchment;      // Catchment ID number that this element belongs to.
  int vegetationType; // Vegetation type from Noah-MP VEGPARM.TBL file.
  
  // Geometric coordinates.
  double vertexX[3];      // Meters
  double vertexY[3];      // Meters
  double elementX;        // Meters.
  double elementY;        // Meters.
  double elementZSurface; // Elevation in meters.
  double elementArea;     // Square meters.
  double latitude;        // Radians.
  double longitude;       // Radians.
  
  // Hydraulic parameters.
  double manningsN; // Roughness parameter in seconds/(meters^(1/3)).
  
  // Water state variables.
  double surfacewaterDepth; // Meters.
  double surfacewaterError; // Meters.  Positive means water was created.  Negative means water was destroyed.
  
  // Water flow variables.
  double precipitationRate;                // Precipitation falling on the mesh element in meters of water per second.  Negative means water added to the
                                           // element.  Must be non-positive.
  double precipitationCumulativeShortTerm; // precipitationCumulativeShortTerm plus precipitationCumulativeLongTerm together are the cumulative precipitation
  double precipitationCumulativeLongTerm;  // that has fallen on the mesh element in meters of water.  Negative means water added to the element.  Must be
                                           // non-positive.  Two variables are used because over a long simulation this value could become quite large, and the
                                           // amount of water added each timestep can be quite small so roundoff error becomes a concern.  New precipitation is
                                           // added to precipitationCumulativeShortTerm each timestep, and occasionally precipitationCumulativeShortTerm is
                                           // added to precipitationCumulativeLongTerm and precipitationCumulativeShortTerm is reset to zero.
  double evaporationRate;                  // Evaporation or condensation on the mesh element in meters of water per second.  Positive means water removed from
                                           // the element.  Negative means water added to the element.
  double evaporationCumulativeShortTerm;   // evaporationCumulativeShortTerm plus evaporationCumulativeLongTerm together are the cumulative evaporation or
  double evaporationCumulativeLongTerm;    // condensation on the mesh element in meters of water.  Positive means water removed from the element.  Negative
                                           // means water added to the element.  See the comment of precipitationCumulativeShortTerm for why two variables are
                                           // used.
  double transpirationRate;                // Transpiration from the mesh element in meters of water per second.  Positive means water removed from the
                                           // element.  Must be non-negative.
  double transpirationCumulativeShortTerm; // transpirationCumulativeShortTerm plus transpirationCumulativeLongTerm together are the cumulative transpiration
  double transpirationCumulativeLongTerm;  // from the mesh element in meters of water.  Positive means water removed from the element.  Must be non-negative.
                                           // See the comment of precipitationCumulativeShortTerm for why two variables are used.
  
  // Evapo-transpiration variables.
  EvapoTranspirationForcingStruct evapoTranspirationForcing; // Input variables that come from forcing data.
  bool                            forcingUpdated;            // Whether the forcing data has been updated.
  EvapoTranspirationStateStruct   evapoTranspirationState;   // State variables whose values are updated and passed to the next timestep.
  
  // This is the first element of a linked list of objects that represent the
  // infiltration and groundwater layers below ground.  There is a linked list
  // because there can be multiple layers.  This one is an object and not a
  // pointer because the linked list has to have at least one object and if you
  // are doing a simple simulation with just one layer everywhere then you
  // don't have to mess with pointers at all.
  // FIXLATER currently the code is implemented for only a single layer.
  InfiltrationAndGroundwater underground;
  
  // Neighbors.
  std::vector<MeshSurfacewaterMeshNeighborProxy>    meshNeighbors;
  std::vector<MeshSurfacewaterChannelNeighborProxy> channelNeighbors;
};

#endif // __MESH_ELEMENT_H__
