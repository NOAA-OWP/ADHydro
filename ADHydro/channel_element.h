#ifndef __CHANNEL_ELEMENT_H__
#define __CHANNEL_ELEMENT_H__

// file_manager.decl.h needs CProxy_MeshElement from mesh_element.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// mesh_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// channel_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_MeshElement from mesh_element.decl.h.
// These forward declarations break the circularity.
class CProxy_ChannelElement;
class CProxy_MeshElement;

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "file_manager.decl.h"
#pragma GCC diagnostic warning "-Wsign-compare"

#include "all.h"
#include "channel_element.decl.h"
#include "mesh_element.decl.h"

// A ChannelElement object represents one link in the simulation channel
// network.  A chare array of ChannelElement objects represents the entire
// simulation channel network.
class ChannelElement : public CBase_ChannelElement
{
  ChannelElement_SDAG_CODE
  
public:
  
  // Constructor.  All the constructor does is start the runForever function in
  // the SDAG code.  The constructor does not initialize or perform an
  // invariant check on the object.  The calling program should send a
  // checkInvariant message to the entire chare array after initialization.
  ChannelElement();
  
  // Charm++ migration constructor.
  //
  // Parameters:
  //
  // msg - Charm++ migration message.
  ChannelElement(CkMigrateMessage* msg);
  
  // Pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  static const int channelVerticesSize  = 74; // Maximum number of channel vertices.  Unlike the mesh, vertices are not necessarily equal to neighbors.
  static const int channelNeighborsSize = 2;  // Maximum number of channel neighbors.
  static const int meshNeighborsSize    = 4;  // Maximum number of mesh neighbors.
  
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
  // meshProxyInit        - The proxy to the mesh chare array.
  // fileManagerProxyInit - The proxy to the file manager chare group.
  void handleInitialize(CProxy_MeshElement meshProxyInit, CProxy_FileManager fileManagerProxyInit);

  // Initialize member variables from values sent by a channel neighbor.
  //
  // Parameters:
  //
  // neighbor               - Array index of neighbor element in chare array.
  // neighborReciprocalEdge - Array index of me in neighbor's neighbor list.
  // neighborZBank          - Bank Z coordinate of neighbor.
  // neighborZBed           - Bed Z coordinate of neighbor.
  // neighborLength         - Length of neighbor.
  // neighborChannelType    - type of channel of neighbor.
  // neighborBaseWidth      - Base width of neighbor.
  // neighborSideSlope      - Side slope of neighbor.
  // neighborManningsN      - Manning's n of neighbor.
  void handleInitializeChannelNeighbor(int neighbor, int neighborReciprocalEdge, double neighborZBank, double neighborZBed,
                                       double neighborLength, ChannelTypeEnum neighborChannelType, double neighborBaseWidth,
                                       double neighborSideSlope, double neighborManningsN);
  
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
  // neighborSlopeX         - Surface slope of neighbor in X direction.
  // neighborSlopeY         - Surface slope of neighbor in Y direction.
  void handleInitializeMeshNeighbor(int neighbor, int neighborReciprocalEdge, double neighborX, double neighborY, double neighborZSurface,
                                    double neighborZBedrock, double neighborSlopeX, double neighborSlopeY);
  
  // Receive a message with new forcing data.  Store this data in member
  // variables for future use.  When complete, all of the elements will
  // contribute to an empty reduction.
  void handleForcingDataMessage();
  
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
  
  // Check the ready state of all groundwater flow rates.  If all groundwater
  // flow rates are calculated do the flow rate limiting check for groundwater
  // outward flow rates, and if all groundwater flow rate limiting checks are
  // done move groundwater.
  //
  // Parameters:
  //
  // iterationThisMessage - Iteration number to put on all messages this
  //                        timestep.
  void checkGroundwaterFlowRates(CMK_REFNUM_TYPE iterationThisMessage);
  
  // Update the values of water state variables to reflect the result of
  // all groundwater flows then start the surfacewater phase.
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
  
  // Calculate the surfacewater depth corresponding to the given cross
  // sectional wetted area for this channel's base width and side slope.
  // The calculated depth is stored in the object's surfacewaterDepth member
  // variable.
  //
  // Parameters:
  //
  // area - Cross sectional wetted area in square meters used to calculate the
  //        corresponding depth.
  void calculateSurfacewaterDepthFromArea(double area);
  
  // Check invariant conditions on member variables.  Exit if invariant is
  // violated.  When the invariant check is done all of the elements will
  // contribute to an empty reduction.
  void handleCheckInvariant();
  
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
  // neighborZBank                - Bank Z coordinate of neighbor center.
  // neighborZBed                 - Bed Z coordinate of neighbor center.
  // neighborLength               - Length of neighbor.
  // neighborChannelType          - Channel type of neighbor.
  // neighborBaseWidth            - Base width of neighbor.
  // neighborSideSlope            - Side slope of neighbor.
  // neighborManningsN            - Manning's N of neighbor.
  // neighborSurfacewaterFlowRate - What neighbor thinks the surfacewater flow
  //                                rate of our common edge is.
  // neighborSurfacewaterCumulativeFlow - What neighbor thinks the surfacewater
  //                                cumulative flow of our common edge is.
  // neighborDt                   - What neighbor thinks the timestep is.
  void handleCheckChannelNeighborInvariant(int neighbor, int edge, int neighborEdge, InteractionEnum neighborInteraction, double neighborZBank,
                                           double neighborZBed, double neighborLength, double neighborChannelType, double neighborBaseWidth,
                                           double neighborSideSlope, double neighborManningsN, double neighborSurfacewaterFlowRate,
                                           double neighborSurfacewaterCumulativeFlow, double neighborDt);
  
  // Check invariant conditions on member variables using information from a
  // mesh neighbor.  Exit if invariant is violated.  When the invariant
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
  // neighborZSurface             - Surface Z coordinate of neighbor center.
  // neighborZBedrock             - Bedrock Z coordinate of neighbor center.
  // neighborZOffset              - Offset to mesh Z coordinates used by
  //                                neighbor.
  // neighborSlopeX               - X component of surface slope of neighbor.
  // neighborSlopeY               - Y component of surface slope of neighbor.
  // neighborEdgeLength           - What neighbor thinks the length of our
  //                                common edge is.
  // neighborSurfacewaterFlowRate - What neighbor thinks the surfacewater flow
  //                                rate of our common edge is.
  // neighborSurfacewaterCumulativeFlow - What neighbor thinks the surfacewater
  //                                cumulative flow of our common edge is.
  // neighborGroundwaterFlowRate  - What neighbor thinks the groundwater flow
  //                                rate of our common edge is.
  // neighborGroundwaterCumulativeFlow - What neighbor thinks the groundwater
  //                                cumulative flow of our common edge is.
  // neighborDt                   - What neighbor thinks the timestep is.
  void handleCheckMeshNeighborInvariant(int neighbor, int edge, int neighborEdge, InteractionEnum neighborInteraction, double neighborX,
                                        double neighborY, double neighborZSurface, double neighborZBedrock, double neighborZOffset,
                                        double neighborSlopeX, double neighborSlopeY, double neighborEdgeLength,
                                        double neighborSurfacewaterFlowRate, double neighborSurfacewaterCumulativeFlow,
                                        double neighborGroundwaterFlowRate, double neighborGroundwaterCumulativeFlow, double neighborDt);
  
  // Chare proxies.  Array of channel elements is accessible through thisProxy.
  CProxy_MeshElement meshProxy;        // Array of mesh elements.
  CProxy_FileManager fileManagerProxy; // Group of file managers for I/O.
  
  // Geometric coordinates.
  double elementX;      // Meters.
  double elementY;      // Meters.
  double elementZBank;  // Meters.
  double elementZBed;   // Meters.
  double elementLength; // Meters.
  
  // Identification parameters.
  ChannelTypeEnum channelType;   // What type of channel is this.
  int             permanentCode; // Permanent code of link that element came from.
  
  // Hydraulic parameters.
  double baseWidth;       // Width of channel base in meters.
  double sideSlope;       // Widening of each side of the channel for each unit increase in water depth.  It is delta-x over delta-y, the inverse of the
                          // traditional definition of slope, unitless.
  double bedConductivity; // Meters per second.
  double bedThickness;    // Meters.
  double manningsN;       // Roughness parameter in seconds/(meters^(1/3)).
  
  // Water state variables.
  double surfacewaterDepth; // Meters of height.  Must convert to cross sectional area to find quantity of water.
  double surfacewaterError; // Cubic meters of water.  Positive means water was created.  Negative means water was destroyed.
  
  // Sequencing and timestep information.
  bool   groundwaterDone;  // Flag indicating when the groundwater phase is done.
  bool   surfacewaterDone; // Flag indicating when the surfacewater phase is done.
  double dt;               // Current or most recent timestep duration in seconds.
  double dtNew;            // Suggested value for next timestep duration in seconds.
  
  // Channel neighbor elements.
  int             channelNeighbors[channelNeighborsSize];                 // Array index into thisProxy or boundary condition code.
  int             channelNeighborsReciprocalEdge[channelNeighborsSize];   // Array index of me in neighbor's neighbor list.
  InteractionEnum channelNeighborsInteraction[channelNeighborsSize];      // Communication pattern to calculate flows.
  bool            channelNeighborsInitialized[channelNeighborsSize];      // Whether the information for this neighbor has been initialized.
  bool            channelNeighborsInvariantChecked[channelNeighborsSize]; // Whether the information for this neighbor has been invariant checked.
  
  // Channel neighbor geometric coordinates.
  double channelNeighborsZBank[channelNeighborsSize];  // Meters.
  double channelNeighborsZBed[channelNeighborsSize];   // Meters.
  double channelNeighborsLength[channelNeighborsSize]; // Meters.
  
  // Channel neighbor parameters.
  ChannelTypeEnum channelNeighborsChannelType[channelNeighborsSize]; // What type of channel is neighbor.
  double          channelNeighborsBaseWidth[channelNeighborsSize];   // Width of channel base in meters.
  double          channelNeighborsSideSlope[channelNeighborsSize];   // Widening of each side of the channel for each unit increase in water depth.  It is
                                                                     // delta-x over delta-y, the inverse of the traditional definition of slope, unitless.
  double          channelNeighborsManningsN[channelNeighborsSize];   // Roughness parameter in seconds/(meters^(1/3)).
  
  // Channel neighbor water flow state variables.
  FlowRateReadyEnum channelNeighborsSurfacewaterFlowRateReady[channelNeighborsSize];  // Flag to indicate updated for the current timestep.
  double            channelNeighborsSurfacewaterFlowRate[channelNeighborsSize];       // Cubic meters of water per second.
  double            channelNeighborsSurfacewaterCumulativeFlow[channelNeighborsSize]; // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                      // Gets set to zero at initialization and each I/O phase.
  
  // Mesh neighbor elements.
  int             meshNeighbors[meshNeighborsSize];                 // Array index into meshProxy or boundary condition code.
  int             meshNeighborsReciprocalEdge[meshNeighborsSize];   // Array index of me in neighbor's neighbor list.
  InteractionEnum meshNeighborsInteraction[meshNeighborsSize];      // Communication pattern to calculate flows.
  bool            meshNeighborsInitialized[meshNeighborsSize];      // Whether the information for this neighbor has been initialized.
  bool            meshNeighborsInvariantChecked[meshNeighborsSize]; // Whether the information for this neighbor has been invariant checked.
  
  // Mesh neighbor geometric coordinates.
  double meshNeighborsZSurface[meshNeighborsSize];   // Meters.
  double meshNeighborsZBedrock[meshNeighborsSize];   // Meters.
  double meshNeighborsZOffset[meshNeighborsSize];    // Meters.  The Z coordinate at the center of a mesh element is not the same as the Z coordinate at
                                                     // the edge of the element next to its channel neighbor.  This Z offset, which can be positive or
                                                     // negative, is added to all mesh element center Z coordinates when used for interaction with this
                                                     // channel neighbor.
  double meshNeighborsEdgeLength[meshNeighborsSize]; // Meters.
  
  // Mesh neighbor water flow state variables.
  FlowRateReadyEnum meshNeighborsSurfacewaterFlowRateReady[meshNeighborsSize];  // Flag to indicate updated for the current timestep.
  double            meshNeighborsSurfacewaterFlowRate[meshNeighborsSize];       // Cubic meters of water per second.
  double            meshNeighborsSurfacewaterCumulativeFlow[meshNeighborsSize]; // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                // Gets set to zero at initialization and each I/O phase.
  FlowRateReadyEnum meshNeighborsGroundwaterFlowRateReady[meshNeighborsSize];   // Flag to indicate updated for the current timestep.
  double            meshNeighborsGroundwaterFlowRate[meshNeighborsSize];        // Cubic meters of water per second.
  double            meshNeighborsGroundwaterCumulativeFlow[meshNeighborsSize];  // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                // Gets set to zero at initialization and each I/O phase.
};

#endif // __CHANNEL_ELEMENT_H__
