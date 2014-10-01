#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

// file_manager.decl.h needs CProxy_MeshElement from mesh_element.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// mesh_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// channel_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_MeshElement from mesh_element.decl.h.
// These forward declarations break the circularity.
class CProxy_MeshElement;
class CProxy_ChannelElement;

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "file_manager.decl.h"
#pragma GCC diagnostic warning "-Wsign-compare"

#include "all.h"
#include "mesh_element.decl.h"
#include "channel_element.decl.h"
#include "evapo_transpiration.h"

// A MeshElement object represents one triangle in the simulation mesh.
// A chare array of MeshElement objects represents the entire simulation mesh.
class MeshElement : public CBase_MeshElement
{
  MeshElement_SDAG_CODE
  
public:
  
  // Constructor.  All the constructor does is start the runForever function in
  // the SDAG code.  The constructor does not initialize or perform an
  // invariant check on the object.  The calling program should send a
  // checkInvariant message to the entire chare array after initialization.
  MeshElement();
  
  // Charm++ migration constructor.
  //
  // Parameters:
  //
  // msg - Charm++ migration message.
  MeshElement(CkMigrateMessage* msg);
  
  // Pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  static const int meshNeighborsSize    = 3;  // Maximum number of mesh neighbors.
  static const int channelNeighborsSize = 16; // Maximum number of channel neighbors.
  
private:
  
  // Returns: true if all neighbor information is initialized, false otherwise.
  bool allInitialized();
  
  // Returns: true if all neighbor invariants are checked, false otherwise.
  bool allInvariantChecked();

  // Initialize member variables by reading public member variables of the
  // local file manager, and send initialization values to neighbors.
  //
  // Parameters:
  //
  // channelProxyInit     - The proxy to the channels chare array.
  // fileManagerProxyInit - The proxy to the file manager chare group.
  void handleInitialize(CProxy_ChannelElement channelProxyInit, CProxy_FileManager fileManagerProxyInit);
  
  // Initialize member variables from values sent by a mesh neighbor.
  //
  // Parameters:
  //
  // neighbor               - Array index of neighbor element in chare array.
  // neighborReciprocalEdge - Array index of me in neighbor's neighbor list.
  // neighborX              - X coordinate of neighbor.
  // neighborY              - Y coordinate of neighbor.
  // neighborZSurface       - Surface Z coordinate of neighbor.
  // neighborZBedrock       - Bedrock Z coordinate of neighbor.
  // neighborArea           - Area of neighbor.
  // neighborConductivity   - Conductivity of neighbor.
  // neighborManningsN      - Manning's n of neighbor.
  void handleInitializeMeshNeighbor(int neighbor, int neighborReciprocalEdge, double neighborX, double neighborY, double neighborZSurface,
                                    double neighborZBedrock, double neighborArea, double neighborConductivity, double neighborManningsN);
  
  // Initialize member variables from values sent by a channel neighbor.
  //
  // Parameters:
  //
  // neighbor                - Array index of neighbor element in chare array.
  // neighborReciprocalEdge  - Array index of me in neighbor's neighbor list.
  // neighborX               - X coordinate of neighbor.
  // neighborY               - Y coordinate of neighbor.
  // neighborZBank           - Bank Z coordinate of neighbor.
  // neighborZBed            - Bed Z coordinate of neighbor.
  // neighborBaseWidth       - Base width of neighbor.
  // neighborSideSlope       - Side slope of neighbor.
  // neighborBedConductivity - Conductivity of neighbor's channel bed.
  // neighborBedThickness    - Thickness of neighbor's channel bed.
  void handleInitializeChannelNeighbor(int neighbor, int neighborReciprocalEdge, double neighborX, double neighborY, double neighborZBank,
                                       double neighborZBed, double neighborBaseWidth, double neighborSideSlope, double neighborBedConductivity,
                                       double neighborBedThickness);

  // Receive a message with new forcing data.  Store this data in member
  // variables for future use.  When complete, all of the elements will
  // contribute to an empty reduction.
  //
  // Parameters:
  //
  // atmosphereLayerThicknessNew     - New value for thickness in meters of
  //                                   lowest atmosphere layer.
  // shadedFractionNew               - New value for fraction of land area
  //                                   shaded by vegetation, 0.0 to 1.0.
  // shadedFractionMaximumNew        - New value for yearly maximum fraction of
  //                                   land area shaded by vegetation, 0.0 to
  //                                   1.0.
  // surfaceTemperatureNew           - New value for air temperature in Celsius
  //                                   at surface.
  // surfacePressureNew              - New value for air pressure in Pascal at
  //                                   surface.
  // atomsphereLayerPressureNew      - New value for air pressure in Pascal at
  //                                   middle of lowest atmosphere layer.
  // eastWindSpeedNew                - New value for eastward wind speed in
  //                                   meters per second at middle of lowest
  //                                   atmosphere layer.
  // northWindSpeedNew               - New value for northward wind speed in
  //                                   meters per second at middle of lowest
  //                                   atmosphere layer.
  // atmosphereLayerMixingRatioNew   - New value for water vapor mixing ratio
  //                                   at middle of lowest atmosphere layer,
  //                                   unitless.
  // cloudMixingRatioNew             - New value for liquid water mixing ratio
  //                                   in clouds, unitless.
  // shortWaveRadiationDownNew       - New value for downward short wave
  //                                   radiation in Watts per square meter at
  //                                   the top of the canopy.
  // longWaveRadiationDownNew        - New value for downward long wave
  //                                   radiation in Watts per square meter at
  //                                   the top of the canopy.
  // precipitationRateNew            - New value for precipitation rate in
  //                                   meters of water per second at the top of
  //                                   the canopy.
  // soilBottomTemperatureNew        - New value for Boundary condition for
  //                                   soil temperature in Celsius at the
  //                                   bottom of the lowest soil layer.
  // planetaryBoundaryLayerHeightNew - New value for Planetary boundary layer
  //                                   height in meters.
  void handleForcingDataMessage(double atmosphereLayerThicknessNew, double shadedFractionNew, double shadedFractionMaximumNew,
                                double surfaceTemperatureNew, double surfacePressureNew, double atomsphereLayerPressureNew, double eastWindSpeedNew,
                                double northWindSpeedNew, double atmosphereLayerMixingRatioNew, double cloudMixingRatioNew,
                                double shortWaveRadiationDownNew, double longWaveRadiationDownNew, double precipitationRateNew,
                                double soilBottomTemperatureNew, double planetaryBoundaryLayerHeightNew);
  
  // Step forward one timestep.  Performs point processes and starts the
  // groundwater and surfacewater phases.  When the timestep is done all of the
  // elements will contribute to a dtNew reduction.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisTimestep - Iteration number to put on all messages this
  //                         timestep.
  // dtThisTimestep        - Duration for this timestep in seconds.
  void handleDoTimestep(CMK_REFNUM_TYPE iterationThisTimestep, double dtThisTimestep);
  
  // Calculate groundwater flow rate for all boundary condition edges.  Also
  // calculate infiltration.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage - Iteration number to put on all messages this
  //                        timestep.
  void handleCalculateGroundwaterBoundaryConditionsMessage(CMK_REFNUM_TYPE iterationThisMessage);
  
  // Receive a groundwater state message from a mesh neighbor, calculate
  // groundwater flow rate across the edge shared with the neighbor, and
  // possibly send a groundwater flow rate message back to the neighbor.
  // Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage      - Iteration number to put on all messages this
  //                             timestep.
  // edge                      - My mesh edge number for the edge shared with
  //                             the neighbor.
  // neighborSurfacewaterDepth - Surfacewater depth of the neighbor in meters.
  // neighborGroundwaterHead   - Groundwater head of the neighbor in meters.
  void handleMeshGroundwaterStateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double neighborSurfacewaterDepth, double neighborGroundwaterhead);
  
  // Receive a groundwater state message from a channel neighbor, calculate
  // groundwater flow rate across the edge shared with the neighbor, and
  // possibly send a groundwater flow rate message back to the neighbor.
  // Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage      - Iteration number to put on all messages this
  //                             timestep.
  // edge                      - My channel edge number for the edge shared
  //                             with the neighbor.
  // neighborSurfacewaterDepth - Surfacewater depth of the neighbor in meters.
  void handleChannelGroundwaterStateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double neighborSurfacewaterDepth);
  
  // Receive a groundwater flow rate message from a mesh neighbor.  This
  // message indicates that the neighbor has calculated the groundwater flow
  // rate across the shared edge, but the flow rate might still need to be
  // limited.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage    - Iteration number to put on all messages this
  //                           timestep.
  // edge                    - My edge number for the edge shared with the
  //                           neighbor.
  // edgeGroundwaterFlowRate - Groundwater flow rate in cubic meters across
  //                           the edge shared with the neighbor.
  void handleMeshGroundwaterFlowRateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeGroundwaterFlowRate);
  
  // Receive a groundwater flow rate message from a channel neighbor.  This
  // message indicates that the neighbor has calculated the groundwater flow
  // rate across the shared edge, but the flow rate might still need to be
  // limited.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage    - Iteration number to put on all messages this
  //                           timestep.
  // edge                    - My edge number for the edge shared with the
  //                           neighbor.
  // edgeGroundwaterFlowRate - Groundwater flow rate in cubic meters across
  //                           the edge shared with the neighbor.
  void handleChannelGroundwaterFlowRateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeGroundwaterFlowRate);
  
  // Receive a groundwater flow rate limited message from a mesh neighbor.
  // This message indicates that the neighbor has calculated whether or not the
  // groundwater flow rate needs to be limited and what the final correct value
  // is.  It does not necessarily mean that the flow rate was limited, but that
  // the flow rate limiting check has been done and here is the final value,
  // which may or may not have been limited.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage    - Iteration number to put on all messages this
  //                           timestep.
  // edge                    - My edge number for the edge shared with the
  //                           neighbor.
  // edgeGroundwaterFlowRate - Groundwater flow rate in cubic meters across
  //                           the edge shared with the neighbor.
  void handleMeshGroundwaterFlowRateLimitedMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeGroundwaterFlowRate);
  
  // Receive a groundwater flow rate limited message from a channel neighbor.
  // This message indicates that the neighbor has calculated whether or not the
  // groundwater flow rate needs to be limited and what the final correct value
  // is.  It does not necessarily mean that the flow rate was limited, but that
  // the flow rate limiting check has been done and here is the final value,
  // which may or may not have been limited.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage    - Iteration number to put on all messages this
  //                           timestep.
  // edge                    - My edge number for the edge shared with the
  //                           neighbor.
  // edgeGroundwaterFlowRate - Groundwater flow rate in cubic meters across
  //                           the edge shared with the neighbor.
  void handleChannelGroundwaterFlowRateLimitedMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeGroundwaterFlowRate);
  
  // Check the ready state of all groundwater flow rates.  If all groundwater
  // flow rates are calculated do the flow rate limiting check for groundwater
  // outward flow rates, and if all groundwater flow rate limiting checks are
  // done and infiltration is done move groundwater.
  //
  // Parameters:
  //
  // iterationThisMessage - Iteration number to put on all messages this
  //                        timestep.
  void checkGroundwaterFlowRates(CMK_REFNUM_TYPE iterationThisMessage);
  
  // Update the values of water state variables to reflect the result of
  // infiltration and all groundwater flows then start the surfacewater phase.
  //
  // Parameters:
  //
  // iterationThisMessage - Iteration number to put on all messages this
  //                        timestep.
  void moveGroundwater(CMK_REFNUM_TYPE iterationThisMessage);
  
  // Calculate surfacewater flow rate for all boundary condition edges.
  // Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage - Iteration number to put on all messages this
  //                        timestep.
  void handleCalculateSurfacewaterBoundaryConditionsMessage(CMK_REFNUM_TYPE iterationThisMessage);
  
  // Receive a surfacewater state message from a mesh neighbor, calculate
  // surfacewater flow rate across the edge shared with the neighbor, and
  // possibly send a surfacewater flow rate message back to the neighbor.
  // Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage      - Iteration number to put on all messages this
  //                             timestep.
  // edge                      - My mesh edge number for the edge shared with
  //                             the neighbor.
  // neighborSurfacewaterDepth - Surfacewater depth of the neighbor in meters.
  void handleMeshSurfacewaterStateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double neighborSurfacewaterDepth);
  
  // Receive a surfacewater state message from a channel neighbor, calculate
  // surfacewater flow rate across the edge shared with the neighbor, and
  // possibly send a surfacewater flow rate message back to the neighbor.
  // Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage      - Iteration number to put on all messages this
  //                             timestep.
  // edge                      - My channel edge number for the edge shared
  //                             with the neighbor.
  // neighborSurfacewaterDepth - Surfacewater depth of the neighbor in meters.
  void handleChannelSurfacewaterStateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double neighborSurfacewaterDepth);
  
  // Receive a surfacewater flow rate message from a mesh neighbor.  This
  // message indicates that the neighbor has calculated the surfacewater flow
  // rate across the shared edge, but the flow rate might still need to be
  // limited.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage     - Iteration number to put on all messages this
  //                            timestep.
  // edge                     - My edge number for the edge shared with the
  //                            neighbor.
  // edgeSurfacewaterFlowRate - Surfacewater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  void handleMeshSurfacewaterFlowRateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeSurfacewaterFlowRate);
  
  // Receive a surfacewater flow rate message from a channel neighbor.  This
  // message indicates that the neighbor has calculated the surfacewater flow
  // rate across the shared edge, but the flow rate might still need to be
  // limited.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage     - Iteration number to put on all messages this
  //                            timestep.
  // edge                     - My edge number for the edge shared with the
  //                            neighbor.
  // edgeSurfacewaterFlowRate - Surfacewater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  void handleChannelSurfacewaterFlowRateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeSurfacewaterFlowRate);
  
  // Receive a surfacewater flow rate limited message from a mesh neighbor.
  // This message indicates that the neighbor has calculated whether or not the
  // surfacewater flow rate needs to be limited and what the final correct value
  // is.  It does not necessarily mean that the flow rate was limited, but that
  // the flow rate limiting check has been done and here is the final value,
  // which may or may not have been limited.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage     - Iteration number to put on all messages this
  //                            timestep.
  // edge                     - My edge number for the edge shared with the
  //                            neighbor.
  // edgeSurfacewaterFlowRate - Surfacewater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  void handleMeshSurfacewaterFlowRateLimitedMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeSurfacewaterFlowRate);
  
  // Receive a surfacewater flow rate limited message from a channel neighbor.
  // This message indicates that the neighbor has calculated whether or not the
  // surfacewater flow rate needs to be limited and what the final correct value
  // is.  It does not necessarily mean that the flow rate was limited, but that
  // the flow rate limiting check has been done and here is the final value,
  // which may or may not have been limited.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisMessage     - Iteration number to put on all messages this
  //                            timestep.
  // edge                     - My edge number for the edge shared with the
  //                            neighbor.
  // edgeSurfacewaterFlowRate - Surfacewater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  void handleChannelSurfacewaterFlowRateLimitedMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeSurfacewaterFlowRate);
  
  // Check the ready state of all surfacewater flow rates.  If all surfacewater
  // flow rates are calculated do the flow rate limiting check for surfacewater
  // outward flow rates, and if all surfacewater flow rate limiting checks are
  // done move surfacewater.
  //
  // Parameters:
  //
  // iterationThisMessage - Iteration number to put on all messages this
  //                        timestep.
  void checkSurfacewaterFlowRates(CMK_REFNUM_TYPE iterationThisMessage);
  
  // Update the values of water state variables to reflect the result of
  // all surfacewater flows then contribute to the dtNew reduction.
  void moveSurfacewater();
  
  // Check invariant conditions on member variables.  Exit if invariant is
  // violated.  When the invariant check is done all of the elements will
  // contribute to an empty reduction.
  void handleCheckInvariant();
  
  // Check invariant conditions on member variables using information from a
  // mesh neighbor.  Exit if invariant is violated.  When the invariant
  // check is done all of the elements will contribute to an empty reduction.
  //
  // Parameters:
  //
  // neighbor                     - Array index of neighbor element.
  // edge                         - My edge that neighbor thinks he is on.
  // neighborEdge                 - Neighbor's edge that I am on.
  // neighborChannelEdge          - Whether neighbor thinks our edge is a
  //                                channel edge.
  // neighborInteraction          - What neighbor thinks our interaction is.
  // neighborX                    - X coordinate of neighbor center.
  // neighborY                    - Y coordinate of neighbor center.
  // neighborZSurface             - Surface Z coordinate of neighbor center.
  // neighborZBedrock             - Bedrock Z coordinate of neighbor center.
  // neighborArea                 - Area of neighbor.
  // neighborEdgeLength           - What neighbor thinks the length of our
  //                                common edge is.
  // neighborEdgeNormalX          - What neighbor thinks the X component of the
  //                                normal vector of our common edge is.
  // neighborEdgeNormalY          - What neighbor thinks the Y component of the
  //                                normal vector of our common edge is.
  // neighborConductivity         - Conductivity of neighbor.
  // neighborManningsN            - Manning's N of neighbor.
  // neighborSurfacewaterFlowRate - What neighbor thinks the surfacewater flow
  //                                rate of our common edge is.
  // neighborSurfacewaterCumulativeFlow - What neighbor thinks the surfacewater
  //                                cumulative flow of our common edge is.
  // neighborGroundwaterFlowRate  - What neighbor thinks the groundwater flow
  //                                rate of our common edge is.
  // neighborGroundwaterCumulativeFlow - What neighbor thinks the groundwater
  //                                cumulative flow of our common edge is.
  // neighborDt                   - What neighbor thinks the timestep is.
  void handleCheckMeshNeighborInvariant(int neighbor, int edge, int neighborEdge, bool neighborChannelEdge, InteractionEnum neighborInteraction,
                                        double neighborX, double neighborY, double neighborZSurface, double neighborZBedrock, double neighborArea,
                                        double neighborEdgeLength, double neighborEdgeNormalX, double neighborEdgeNormalY,
                                        double neighborConductivity, double neighborManningsN, double neighborSurfacewaterFlowRate,
                                        double neighborSurfacewaterCumulativeFlow, double neighborGroundwaterFlowRate,
                                        double neighborGroundwaterCumulativeFlow, double neighborDt);
  
  // Check invariant conditions on member variables using information from a
  // channel neighbor.  Exit if invariant is violated.  When the invariant
  // check is done all of the elements will contribute to an empty reduction.
  //
  // Parameters:
  //
  // neighbor                     - Array index of neighbor element.
  // edge                         - My edge that neighbor thinks he is on.
  // neighborEdge                 - Neighbor's edge that I am on.
  // neighborInteraction          - What neighbor thinks our interaction is.
  // neighborX                    - X coordinate of neighbor center.
  // neighborY                    - Y coordinate of neighbor center.
  // neighborZBank                - Bank Z coordinate of neighbor center.
  // neighborZBed                 - Bed Z coordinate of neighbor center.
  // neighborZOffset              - Offset to mesh Z coordinates used by
  //                                neighbor.
  // neighborEdgeLength           - What neighbor thinks the length of our
  //                                common edge is.
  // neighborBaseWidth            - Base width of neighbor.
  // neighborSideSlope            - Side slope of neighbor.
  // neighborBedConductivity      - Bed conductivity of neighbor.
  // neighborBedThickness         - Bed thickness of neighbor.
  // neighborSurfacewaterFlowRate - What neighbor thinks the surfacewater flow
  //                                rate of our common edge is.
  // neighborSurfacewaterCumulativeFlow - What neighbor thinks the surfacewater
  //                                cumulative flow of our common edge is.
  // neighborGroundwaterFlowRate  - What neighbor thinks the groundwater flow
  //                                rate of our common edge is.
  // neighborGroundwaterCumulativeFlow - What neighbor thinks the groundwater
  //                                cumulative flow of our common edge is.
  // neighborDt                   - What neighbor thinks the timestep is.
  void handleCheckChannelNeighborInvariant(int neighbor, int edge, int neighborEdge, InteractionEnum neighborInteraction, double neighborX,
                                           double neighborY, double neighborZBank, double neighborZBed, double neighborZOffset,
                                           double neighborEdgeLength, double neighborBaseWidth, double neighborSideSlope,
                                           double neighborBedConductivity, double neighborBedThickness, double neighborSurfacewaterFlowRate,
                                           double neighborSurfacewaterCumulativeFlow, double neighborGroundwaterFlowRate,
                                           double neighborGroundwaterCumulativeFlow, double neighborDt);
  
  // Chare proxies.  Array of mesh elements is accessible through thisProxy.
  CProxy_ChannelElement channelProxy;     // Array of channel elements.
  CProxy_FileManager    fileManagerProxy; // Group of file managers for I/O.
  
  // Geometric coordinates.
  double elementX;        // Meters.
  double elementY;        // Meters.
  double elementZSurface; // Meters.
  double elementZBedrock; // Meters.
  double elementArea;     // Square meters.
  double elementSlopeX;   // Surface slope in X direction, unitless.
  double elementSlopeY;   // Surface slope in Y direction, unitless.
  
  // Identification parameters.
  int catchment;      // Catchment ID number that this element belongs to.
  int vegetationType; // Vegetation type from Noah-MP VEGPARM.TBL file.
  int soilType;       // Soil type form Noah-MP SOILPARM.TBL file.
  
  // Hydraulic parameters.
  double conductivity; // Meters per second.
  double porosity;     // Unitless.
  double manningsN;    // Roughness parameter in seconds/(meters^(1/3)).
  
  // Water state variables.
  double surfacewaterDepth; // Meters of height and meters of water.
  double surfacewaterError; // Cubic meters of water.  Positive means water was created.  Negative means water was destroyed.
  double groundwaterHead;   // Meters of height.
  double groundwaterError;  // Cubic meters of water.  Positive means water was created.  Negative means water was destroyed.
  
  // Water flow state variables.
  double precipitation;            // Precipitation falling on the mesh element in meters of water.  Positive means water added to the element.  Must be
                                   // non-negative.
  double precipitationCumulative;  // Cumulative precipitation that has fallen on the mesh element in meters of water since last set to zero.  Gets set to zero
                                   // at initialization and each I/O phase.  Positive means water added to the element.  Must be non-negative.
  double evaporation;              // Evaporation or condensation on the mesh element in meters of water.  Positive means water added to the element.  Negative
                                   // means water removed from the element.
  double evaporationCumulative;    // Cumulative evaporation or condensation on the mesh element in meters of water since last set to zero.  Gets set to zero
                                   // at initialization and each I/O phase.  Positive means water added to the element.  Negative means water removed from the
                                   // element.
  double surfacewaterInfiltration; // Meters of water infiltrating from the surface.  Positive means flow from the surface to the vadose zone.  Must be
                                   // non-negative.  If water exfiltrates from groundwater to the surface that is handled in a different way in
                                   // moveGroundwater.  FIXME store cumulative infiltration?
  double groundwaterRecharge;      // Meters of water flowing between the vadose zone and groundwater.  Positive means flow from the vadose zone to the
                                   // groundwater.  Negative means flow from the groundwater to the vadose zone.  During moveGroundwater this is used to store
                                   // the net meters of water added to or removed from the groundwater including infiltration and all groundwater flows.
                                   // Positive means water added to the groundwater.  Negative means water removed from the groundwater.
                                   // FIXME store cumulative recharge?
  
  // Evapo-transpiration state variables.
  EvapoTranspirationStateStruct evapoTranspirationState; // state variables that are simulated by the evapo-transpiration module.
  
  // Forcing data.
  double atmosphereLayerThickness;     // Thickness in meters of lowest atmosphere layer in forcing data.  The following other variables are values from the
                                       // middle of the lowest atmosphere layer, i.e. half of this height: atomsphereLayerPressure, eastWindSpeed,
                                       // northWindSpeed, atmosphereLayerMixingRatio.
  double shadedFraction;               // Fraction of land area shaded by vegetation, 0.0 to 1.0.
  double shadedFractionMaximum;        // Yearly maximum fraction of land area shaded by vegetation, 0.0 to 1.0.
  double surfaceTemperature;           // Air temperature in Celsius at surface.
  double surfacePressure;              // Air pressure in Pascal at surface.
  double atomsphereLayerPressure;      // Air pressure in Pascal at middle of lowest atmosphere layer in forcing data.
  double eastWindSpeed;                // Eastward wind speed in meters per second at middle of lowest atmosphere layer in forcing data.
  double northWindSpeed;               // Northward wind speed in meters per second at middle of lowest atmosphere layer in forcing data.
  double atmosphereLayerMixingRatio;   // Water vapor mixing ratio at middle of lowest atmosphere layer in forcing data, unitless.
  double cloudMixingRatio;             // Liquid water mixing ratio in clouds, unitless.
  double shortWaveRadiationDown;       // Downward short wave radiation in Watts per square meter at the top of the canopy.
  double longWaveRadiationDown;        // Downward long wave radiation in Watts per square meter at the top of the canopy.
  double precipitationRate;            // Precipitation rate in meters of water per second at the top of the canopy.
  double soilBottomTemperature;        // Boundary condition for soil temperature in Celsius at the bottom of the lowest soil layer.
  double planetaryBoundaryLayerHeight; // Planetary boundary layer height in meters.
  
  // Sequencing and timestep information.
  bool   groundwaterDone;  // Flag indicating when the groundwater phase is done.
  bool   infiltrationDone; // Flag indicating when the infiltration calculation is done.
  bool   surfacewaterDone; // Flag indicating when the surfacewater phase is done.
  double dt;               // Current or most recent timestep duration in seconds.
  double dtNew;            // Suggested value for next timestep duration in seconds.
  
  // Mesh neighbor elements.
  int             meshNeighbors[meshNeighborsSize];                 // Array index into thisProxy or boundary condition code.
  bool            meshNeighborsChannelEdge[meshNeighborsSize];      // If true, the edge to this mesh neighbor has a channel on it.  The edge is treated as a
                                                                    // NOFLOW boundary for mesh neighbor surfacewater interactions.  All surfacewater flow goes
                                                                    // through the channel.  Mesh neighbor groundwater interactions still proceed normally with
                                                                    // water flowing under the channel.  FIXME should a channel edge modify groundwater
                                                                    // interactions if the channel bed is below the water table?
  int             meshNeighborsReciprocalEdge[meshNeighborsSize];   // Array index of me in neighbor's neighbor list.
  InteractionEnum meshNeighborsInteraction[meshNeighborsSize];      // Communication pattern to calculate flows.
  bool            meshNeighborsInitialized[meshNeighborsSize];      // Whether the information for this neighbor has been initialized.
  bool            meshNeighborsInvariantChecked[meshNeighborsSize]; // Whether the information for this neighbor has been invariant checked.
  
  // Mesh neighbor geometric coordinates.
  double meshNeighborsX[meshNeighborsSize];           // Meters.
  double meshNeighborsY[meshNeighborsSize];           // Meters.
  double meshNeighborsZSurface[meshNeighborsSize];    // Meters.
  double meshNeighborsZBedrock[meshNeighborsSize];    // Meters.
  double meshNeighborsArea[meshNeighborsSize];        // Square meters.
  double meshNeighborsEdgeLength[meshNeighborsSize];  // Meters.  2-D distance ignoring Z coordinates.
  double meshNeighborsEdgeNormalX[meshNeighborsSize]; // X component of normal unit vector, unitless.
  double meshNeighborsEdgeNormalY[meshNeighborsSize]; // Y component of normal unit vector, unitless.
  
  // Mesh neighbor parameters.
  double meshNeighborsConductivity[meshNeighborsSize]; // Meters per second.
  double meshNeighborsManningsN[meshNeighborsSize];    // Roughness parameter in seconds/(meters^(1/3)).
  
  // Mesh neighbor water flow state variables.
  FlowRateReadyEnum meshNeighborsSurfacewaterFlowRateReady[meshNeighborsSize];  // Flag to indicate updated for the current timestep.
  double            meshNeighborsSurfacewaterFlowRate[meshNeighborsSize];       // Cubic meters of water per second.
  double            meshNeighborsSurfacewaterCumulativeFlow[meshNeighborsSize]; // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                // Gets set to zero at initialization and each I/O phase.
  FlowRateReadyEnum meshNeighborsGroundwaterFlowRateReady[meshNeighborsSize];   // Flag to indicate updated for the current timestep.
  double            meshNeighborsGroundwaterFlowRate[meshNeighborsSize];        // Cubic meters of water per second.
  double            meshNeighborsGroundwaterCumulativeFlow[meshNeighborsSize];  // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                // Gets set to zero at initialization and each I/O phase.
  
  // Channel neighbor elements.
  int             channelNeighbors[channelNeighborsSize];                 // Array index into channelProxy or boundary condition code.
  int             channelNeighborsReciprocalEdge[channelNeighborsSize];   // Array index of me in neighbor's neighbor list.
  InteractionEnum channelNeighborsInteraction[channelNeighborsSize];      // Communication pattern to calculate flows.
  bool            channelNeighborsInitialized[channelNeighborsSize];      // Whether the information for this neighbor has been initialized.
  bool            channelNeighborsInvariantChecked[channelNeighborsSize]; // Whether the information for this neighbor has been invariant checked.
  
  // Channel neighbor geometric coordinates.
  double channelNeighborsZBank[channelNeighborsSize];      // Meters.
  double channelNeighborsZBed[channelNeighborsSize];       // Meters.
  double channelNeighborsZOffset[channelNeighborsSize];    // Meters.  The Z coordinate at the center of a mesh element is not the same as the Z coordinate at
                                                           // the edge of the element next to its channel neighbor.  This Z offset, which can be positive or
                                                           // negative, is added to all mesh element center Z coordinates when used for interaction with this
                                                           // channel neighbor.
  double channelNeighborsEdgeLength[channelNeighborsSize]; // Meters.
  
  // Channel neighbor parameters.
  double channelNeighborsBaseWidth[channelNeighborsSize];       // Width of channel base in meters.
  double channelNeighborsSideSlope[channelNeighborsSize];       // Widening of each side of the channel for each unit increase in water depth.  It is delta-x
                                                                // over delta-y, the inverse of the traditional definition of slope, unitless.
  double channelNeighborsBedConductivity[channelNeighborsSize]; // Meters per second.
  double channelNeighborsBedThickness[channelNeighborsSize];    // Meters.
  
  // Channel neighbor water flow state variables.
  FlowRateReadyEnum channelNeighborsSurfacewaterFlowRateReady[channelNeighborsSize];  // Flag to indicate updated for the current timestep.
  double            channelNeighborsSurfacewaterFlowRate[channelNeighborsSize];       // Cubic meters of water per second.
  double            channelNeighborsSurfacewaterCumulativeFlow[channelNeighborsSize]; // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                      // Gets set to zero at initialization and each I/O phase.
  FlowRateReadyEnum channelNeighborsGroundwaterFlowRateReady[channelNeighborsSize];   // Flag to indicate updated for the current timestep.
  double            channelNeighborsGroundwaterFlowRate[channelNeighborsSize];        // Cubic meters of water per second.
  double            channelNeighborsGroundwaterCumulativeFlow[channelNeighborsSize];  // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                      // Gets set to zero at initialization and each I/O phase.
};

#endif // __MESH_ELEMENT_H__
