#ifndef __CHANNEL_ELEMENT_H__
#define __CHANNEL_ELEMENT_H__

#include "neighbor_proxy.h"
#include "evapo_transpiration.h"

// channel_element.h needs to know about the Region class, but region.h includes channel_element.h so we just do a forward declaration of the Region class to
// break the circularity.
class Region;

// Proxy for a channel element's surfacewater neighbor that is a mesh element.
class ChannelSurfacewaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  ChannelSurfacewaterMeshNeighborProxy();
  
  // Constructor.
  //
  // All parameters directly initialize superclass and member variables.  For
  // description see superclass and member variables.
  //
  // incomingMaterial is initialized to empty.  neighborInitialized and
  // neighborInvariantChecked are initialized to false.
  ChannelSurfacewaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                       double flowCumulativeShortTermInit, double flowCumulativeLongTermInit,
                                       int regionInit, int neighborInit, int reciprocalNeighborProxyInit,
                                       double neighborZSurfaceInit, double neighborZOffsetInit, double neighborAreaInit,
                                       double edgeLengthInit);
  
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
  int reciprocalNeighborProxy; // The index into neighbor's channelNeighbors vector of the proxy for me.
  
  // Geometric coordinates.
  double neighborZSurface; // Elevation in meters.
  double neighborZOffset;  // Meters.  The Z coordinate at the center of a mesh element is not the same as the Z coordinate at the edge of the element next to
                           // its channel neighbor.  This Z offset, which can be positive or negative, is added to mesh element center Z surface, Z bottom, and
                           // groundwater head elevations when used for interaction with this channel neighbor.
  double neighborArea;     // Square meters.
  double edgeLength;       // Meters.  2-D distance ignoring Z coordinates.
  
  // Flags
  bool neighborInitialized;      // Whether the information for this neighbor has been initialized.
  bool neighborInvariantChecked; // Whether the information for this neighbor has been invariant checked.
};

// Proxy for a channel element's surfacewater neighbor that is a channel element.
class ChannelSurfacewaterChannelNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  ChannelSurfacewaterChannelNeighborProxy();
  
  // Constructor.
  //
  // All parameters directly initialize superclass and member variables.  For
  // description see superclass and member variables.
  //
  // incomingMaterial is initialized to empty.  neighborInitialized and
  // neighborInvariantChecked are initialized to false.
  ChannelSurfacewaterChannelNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                          double flowCumulativeShortTermInit, double flowCumulativeLongTermInit,
                                          int regionInit, int neighborInit, int reciprocalNeighborProxyInit,
                                          ChannelTypeEnum neighborChannelTypeInit, double neighborZBankInit,
                                          double neighborZBedInit, double neighborLengthInit,
                                          double neighborBaseWidthInit, double neighborSideSlopeInit,
                                          double neighborManningsNInit);
  
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
  int             region;                  // Region number where the neighbor is.
  int             neighbor;                // Channel element ID number of the neighbor.
  int             reciprocalNeighborProxy; // The index into neighbor's channelNeighbors vector of the proxy for me.
  ChannelTypeEnum neighborChannelType;     // What type of channel is this.
  
  // Geometric coordinates.
  double neighborZBank;  // Elevation in meters.
  double neighborZBed;   // Elevation in meters.
  double neighborLength; // Meters.
  
  // Hydraulic parameters.
  double neighborBaseWidth; // Width of channel base in meters.
  double neighborSideSlope; // Widening of each side of the channel for each unit increase in water depth.  It is delta-x over delta-y, the inverse of the
                            // traditional definition of slope, unitless.
  double neighborManningsN; // Roughness parameter in seconds/(meters^(1/3)).
  
  // Flags
  bool neighborInitialized;      // Whether the information for this neighbor has been initialized.
  bool neighborInvariantChecked; // Whether the information for this neighbor has been invariant checked.
};

// Proxy for a channel element's groundwater neighbor that is a mesh element.
class ChannelGroundwaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  ChannelGroundwaterMeshNeighborProxy();
  
  // Constructor.
  //
  // All parameters directly initialize superclass and member variables.  For
  // description see superclass and member variables.
  //
  // incomingMaterial is initialized to empty.  neighborInitialized and
  // neighborInvariantChecked are initialized to false.
  ChannelGroundwaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                      double flowCumulativeShortTermInit, double flowCumulativeLongTermInit, int regionInit,
                                      int neighborInit, int reciprocalNeighborProxyInit, double neighborZSurfaceInit,
                                      double neighborLayerZBottomInit, double neighborZOffsetInit, double edgeLengthInit);
  
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
  int reciprocalNeighborProxy; // The index into neighbor's channelNeighbors vector of the proxy for me.
  
  // Geometric coordinates.
  double neighborZSurface;     // Elevation in meters.
  double neighborLayerZBottom; // Elevation in meters.
  double neighborZOffset;      // Meters.  The Z coordinate at the center of a mesh element is not the same as the Z coordinate at the edge of the element next
                               // to its channel neighbor.  This Z offset, which can be positive or negative, is added to mesh element center Z surface,
                               // Z bottom, and groundwater head elevations when used for interaction with this channel neighbor.
  double edgeLength;           // Meters.  2-D distance ignoring Z coordinates.
  
  // Flags
  bool neighborInitialized;      // Whether the information for this neighbor has been initialized.
  bool neighborInvariantChecked; // Whether the information for this neighbor has been invariant checked.
};

// A ChannelElement is a length of stream, a waterbody, or a glacier in the
// simulation channel network.
//
// Channel elements only have surfacewater.  They store no groundwater state.
// Infiltration of channel surfacewater is handled by flows with mesh
// groundwater neighbors.
class ChannelElement
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  ChannelElement();
  
  // Constructor.
  //
  // All parameters directly initialize member variables.  For description see
  // member variables.
  //
  // meshNeighbors, channelNeighbors, and undergroundMeshNeighbors are
  // initialized to empty.
  ChannelElement(int elementNumberInit, ChannelTypeEnum channelTypeInit, long long reachCodeInit, double elementXInit, double elementYInit,
                 double elementZBankInit, double elementZBedInit, double elementLengthInit, double latitudeInit, double longitudeInit,
                 double baseWidthInit, double sideSlopeInit, double bedConductivityInit, double bedThicknessInit, double manningsNInit,
                 double surfacewaterDepthInit, double surfacewaterErrorInit, double precipitationRateInit,
                 double precipitationCumulativeShortTermInit, double precipitationCumulativeLongTermInit, double evaporationRateInit,
                 double evaporationCumulativeShortTermInit, double evaporationCumulativeLongTermInit,
                 EvapoTranspirationForcingStruct& evapoTranspirationForcingInit, EvapoTranspirationStateStruct& evapoTranspirationStateInit);
  
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
                                                            std::vector<ChannelSurfacewaterMeshNeighborProxy>::size_type neighborProxyIndex,
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
                                                               std::vector<ChannelSurfacewaterChannelNeighborProxy>::size_type neighborProxyIndex,
                                                               double neighborSurfacewaterDepth);
  
  
  // Calculate new values for nominalFlowRate and expirationTime for a proxy in
  // undergroundMeshNeighbors.
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
  // neighborProxyIndex        - Which index of undergroundMeshNeighbors is the
  //                             proxy for the neighbor.
  // neighborSurfacewaterDepth - Meters.
  // neighborGroundwaterHead   - Elevation in meters.
  bool calculateNominalFlowRateWithGroundwaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                           std::vector<ChannelGroundwaterMeshNeighborProxy>::size_type neighborProxyIndex,
                                                           double neighborSurfacewaterDepth, double neighborGroundwaterHead);
  
  // Update state for point processes that require no communication with other
  // elements.  Then send outflow water to neighbors.
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
  int             elementNumber; // Channel element ID number of this element.
  ChannelTypeEnum channelType;   // What type of channel is this.
  long long       reachCode;     // For waterbodies, reach code.  For streams, original link number that element came from.
  
  // Geometric coordinates.
  double elementX;      // Meters.
  double elementY;      // Meters.
  double elementZBank;  // Elevation in meters.
  double elementZBed;   // Elevation in meters.
  double elementLength; // Meters.
  double latitude;      // Radians.
  double longitude;     // Radians.
  
  // Hydraulic parameters.
  double baseWidth;       // Width of channel base in meters.
  double sideSlope;       // Widening of each side of the channel for each unit increase in water depth.  It is delta-x over delta-y, the inverse of the
                          // traditional definition of slope, unitless.
  double bedConductivity; // Meters per second.
  double bedThickness;    // Meters.
  double manningsN;       // Roughness parameter in seconds/(meters^(1/3)).
  
  // Water state variables.
  double surfacewaterDepth; // Meters.
  double surfacewaterError; // Cubic meters.  Positive means water was created.  Negative means water was destroyed.
  
  // Water flow variables.
  double precipitationRate;                // Precipitation falling on the channel element in meters of water per second.  Negative means water added to the
                                           // element.  Must be non-positive.
  double precipitationCumulativeShortTerm; // precipitationCumulativeShortTerm plus precipitationCumulativeLongTerm together are the cumulative precipitation
  double precipitationCumulativeLongTerm;  // that has fallen on the channel element in cubic meters of water.  Negative means water added to the element.  Must be
                                           // non-positive.  Two variables are used because over a long simulation this value could become quite large, and the
                                           // amount of water added each timestep can be quite small so roundoff error becomes a concern.  New precipitation is
                                           // added to precipitationCumulativeShortTerm each timestep, and occasionally precipitationCumulativeShortTerm is
                                           // added to precipitationCumulativeLongTerm and precipitationCumulativeShortTerm is reset to zero.
  double evaporationRate;                  // Evaporation or condensation on the channel element in meters of water per second.  Positive means water removed from
                                           // the element.  Negative means water added to the element.
  double evaporationCumulativeShortTerm;   // evaporationCumulativeShortTerm plus evaporationCumulativeLongTerm together are the cumulative evaporation or
  double evaporationCumulativeLongTerm;    // condensation on the channel element in cubic meters of water.  Positive means water removed from the element.  Negative
                                           // means water added to the element.  See the comment of precipitationCumulativeShortTerm for why two variables are
                                           // used.
  
  // Evapo-transpiration variables.
  EvapoTranspirationForcingStruct evapoTranspirationForcing; // Input variables that come from forcing data.
  EvapoTranspirationStateStruct   evapoTranspirationState;   // State variables whose values are updated and passed to the next timestep.
  
  // Neighbors.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>    meshNeighbors;
  std::vector<ChannelSurfacewaterChannelNeighborProxy> channelNeighbors;
  std::vector<ChannelGroundwaterMeshNeighborProxy>     undergroundMeshNeighbors;
  
private:
  
  // Calculate the surfacewater depth of a trapeziodal channel from the wetted
  // cross sectional area.  Calculated value is stored in member variable
  // surfacewaterDepth.
  //
  // Parameters:
  //
  // area - Wetted cross sectional area of channel in square meters.
  void calculateSurfacewaterDepthFromArea(double area);
};

#endif // __CHANNEL_ELEMENT_H__
