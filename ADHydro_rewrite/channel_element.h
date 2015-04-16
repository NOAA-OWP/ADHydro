#ifndef __CHANNEL_ELEMENT_H__
#define __CHANNEL_ELEMENT_H__

#include "neighbor_proxy.h"

// Proxy for a channel element's surfacewater neighbor that is a mesh element.
class ChannelSurfacewaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Identification parameters.
  int region;                  // Region number where the neighbor is.
  int neighbor;                // Mesh element ID number of the neighbor.
  int reciprocalNeighborProxy; // The index into neighbor's channelNeighbors vector of the proxy for me.
  
  // Geometric coordinates.
  double neighborZSurface; // Elevation in meters.
  double neighborArea;     // Square meters.
  double edgeLength;       // Meters.  2-D distance ignoring Z coordinates.
};

// Proxy for a channel element's surfacewater neighbor that is a channel element.
class ChannelSurfacewaterChannelNeighborProxy : public SimpleNeighborProxy
{
public:
  
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
};

// Proxy for a channel element's groundwater neighbor that is a mesh element.
class ChannelGroundwaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // Identification parameters.
  int region;                  // Region number where the neighbor is.
  int neighbor;                // Mesh element ID number of the neighbor.
  int reciprocalNeighborProxy; // The index into neighbor's channelNeighbors vector of the proxy for me.
  
  // Geometric coordinates.
  double neighborZSurface;     // Elevation in meters.
  double neighborLayerZBottom; // Elevation in meters.
  double edgeLength;           // Meters.  2-D distance ignoring Z coordinates.
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
  
  // FIXME constructor
  
  // FIXME pup
  
  // FIXME checkInvariant
  
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
  bool doPointProcessesAndSendOutflows(double referenceDate, double currentTime, double timestepEndTime);
  
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
  
  // Identification parameters.
  int             elementNumber; // Channel element ID number of this element.
  ChannelTypeEnum channelType;   // What type of channel is this.
  long long       reachCode;     // For waterbodies, reach code.  For streams, original link number that element came from.
  
  // Geometric coordinates.
  double elementZBank;  // Elevation in meters.
  double elementZBed;   // Elevation in meters.
  double elementLength; // Meters.
  
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
