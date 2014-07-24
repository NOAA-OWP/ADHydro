#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

#include "file_manager.h"
#include "all.h"

// mesh_element.decl.h needs CProxy_ChannelElement from channel_element.decl.h, but channel_element.decl.h needs CProxy_MeshElement from mesh_element.decl.h.
// Declare this here to break the circularity.
class CProxy_ChannelElement;

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "mesh_element.decl.h"
#pragma GCC diagnostic warning "-Wsign-compare"
#include "channel_element.decl.h"

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
  
  static const int meshNeighborsSize    = 3; // Maximum number of mesh neighbors.
  static const int channelNeighborsSize = 2; // Maximum number of channel neighbors.
  
private:

  // Initialize all member variables to the supplied values or default values.
  void handleInitialize(CProxy_ChannelElement channelProxyInit, CProxy_FileManager fileManagerProxyInit, double vertexXInit[3],
                        double vertexYInit[3], double vertexZSurfaceInit[3], double vertexZBedrockInit[3], double edgeLengthInit[3],
                        double edgeNormalXInit[3], double edgeNormalYInit[3], double elementXInit, double elementYInit, double elementZSurfaceInit,
                        double elementZBedrockInit, double elementAreaInit, int meshNeighborsInit[meshNeighborsSize],
                        int meshNeighborsReciprocalEdgeInit[meshNeighborsSize], InteractionEnum meshNeighborsInteractionInit[meshNeighborsSize],
                        bool channelEdgeInit[meshNeighborsSize], int channelNeighborsInit[channelNeighborsSize],
                        int channelNeighborsReciprocalEdgeInit[channelNeighborsSize],
                        InteractionEnum channelNeighborsInteractionInit[channelNeighborsSize], double meshNeighborsXInit[meshNeighborsSize],
                        double meshNeighborsYInit[meshNeighborsSize], double meshNeighborsZSurfaceInit[meshNeighborsSize],
                        double meshNeighborsZBedrockInit[meshNeighborsSize], double meshNeighborsAreaInit[meshNeighborsSize],
                        double channelNeighborsZBankInit[channelNeighborsSize], double channelNeighborsZBedInit[channelNeighborsSize],
                        double channelNeighborsEdgeLengthInit[channelNeighborsSize], double meshNeighborsConductivityInit[meshNeighborsSize],
                        double meshNeighborsManningsNInit[meshNeighborsSize], double channelNeighborsBaseWidthInit[channelNeighborsSize],
                        double channelNeighborsSideSlopeInit[channelNeighborsSize],
                        double channelNeighborsBedConductivityInit[channelNeighborsSize],
                        double channelNeighborsBedThicknessInit[channelNeighborsSize], int catchmentInit, double conductivityInit,
                        double porosityInit, double manningsNInit, double surfacewaterDepthInit, double surfacewaterErrorInit,
                        double groundwaterHeadInit, double groundwaterErrorInit);
  
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
  
  // Chare proxies.  Array of mesh elements is accessible through thisProxy.
  CProxy_ChannelElement channelProxy;     // Array of channel elements.
  CProxy_FileManager    fileManagerProxy; // Group of file managers for I/O.
  
  // Geometric coordinates of vertices.
  double vertexX[3];        // Meters.
  double vertexY[3];        // Meters.
  double vertexZSurface[3]; // Meters.
  double vertexZBedrock[3]; // Meters.
  
  // Geometric coordinates of edges.
  double edgeLength[3];  // Meters.
  double edgeNormalX[3]; // X component of normal unit vector.
  double edgeNormalY[3]; // Y component of normal unit vector.
  
  // Geometric coordinates of element center or entire element.
  double elementX;        // Meters.
  double elementY;        // Meters.
  double elementZSurface; // Meters.
  double elementZBedrock; // Meters.
  double elementArea;     // Square meters.
  
  // Neighbor elements.
  int             meshNeighbors[meshNeighborsSize];                     // Array index into meshProxy or boundary condition code.
  int             meshNeighborsReciprocalEdge[meshNeighborsSize];       // Array index of me in neighbor's neighbor list.
  InteractionEnum meshNeighborsInteraction[meshNeighborsSize];          // Communication pattern to calculate flows.
  bool            channelEdge[meshNeighborsSize];                       // If true the edge to this mesh neighbor has a channel on it.  The edge is treated as
                                                                        // a NOFLOW boundary for mesh neighbor surfacewater interactions.  All surfacewater
                                                                        // flow goes through the channel neighbors.
  int             channelNeighbors[channelNeighborsSize];               // Array index into thisProxy or boundary condition code
  int             channelNeighborsReciprocalEdge[channelNeighborsSize]; // Array index of me in neighbor's neighbor list.
  InteractionEnum channelNeighborsInteraction[channelNeighborsSize];    // Communication pattern to calculate flows.
  
  // Neighbor geometric coordinates.
  double meshNeighborsX[meshNeighborsSize];                // Meters.
  double meshNeighborsY[meshNeighborsSize];                // Meters.
  double meshNeighborsZSurface[meshNeighborsSize];         // Meters.
  double meshNeighborsZBedrock[meshNeighborsSize];         // Meters.
  double meshNeighborsArea[meshNeighborsSize];             // Square meters.
  double channelNeighborsZBank[channelNeighborsSize];      // Meters.
  double channelNeighborsZBed[channelNeighborsSize];       // Meters.
  double channelNeighborsEdgeLength[channelNeighborsSize]; // Meters.
  double channelNeighborsZOffset[channelNeighborsSize];    // Meters.  The Z coordinate at the center of a mesh element is not the same as the Z coordinate at
                                                           // the edge of the element next to its channel neighbor.  This Z offset, which can be positive or
                                                           // negative, is added to all mesh element center Z coordinates when used for interaction with this
                                                           // channel neighbor.
  
  // Neighbor parameters.
  double meshNeighborsConductivity[meshNeighborsSize];          // Meters per second.
  double meshNeighborsManningsN[meshNeighborsSize];             // Roughness parameter in seconds/(meters^(1/3)).
  double channelNeighborsBaseWidth[channelNeighborsSize];       // Width of channel base in meters.
  double channelNeighborsSideSlope[channelNeighborsSize];       // Widening of each side of the channel for each unit increase in water depth.  It is delta-x
                                                                // over delta-y, the inverse of the traditional definition of slope, unitless.
  double channelNeighborsBedConductivity[channelNeighborsSize]; // Meters per second.
  double channelNeighborsBedThickness[channelNeighborsSize];    // Meters.
  
  // Identification parameters of element.
  int catchment; // Catchment ID number that this element belongs to.
  
  // Hydraulic parameters of element.
  double conductivity; // Meters per second.
  double porosity;     // Unitless.
  double manningsN;    // Roughness parameter in seconds/(meters^(1/3)).
  
  // Water state variables.
  double surfacewaterDepth; // Meters of height and meters of water.
  double surfacewaterError; // Cubic meters of water.  Positive means water was created.  Negative means water was destroyed.
  double groundwaterHead;   // Meters of height.
  double groundwaterError;  // Cubic meters of water.  Positive means water was created.  Negative means water was destroyed.
  
  // Water flow state variables.
  FlowRateReadyEnum meshNeighborsSurfacewaterFlowRateReady[meshNeighborsSize];        // Flag to indicate updated for the current timestep.
  double            meshNeighborsSurfacewaterFlowRate[meshNeighborsSize];             // Cubic meters of water per second.
  double            meshNeighborsSurfacewaterCumulativeFlow[meshNeighborsSize];       // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                      // Gets set to zero at initialization and each I/O phase.
  FlowRateReadyEnum meshNeighborsGroundwaterFlowRateReady[meshNeighborsSize];         // Flag to indicate updated for the current timestep.
  double            meshNeighborsGroundwaterFlowRate[meshNeighborsSize];              // Cubic meters of water per second.
  double            meshNeighborsGroundwaterCumulativeFlow[meshNeighborsSize];        // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                      // Gets set to zero at initialization and each I/O phase.
  FlowRateReadyEnum channelNeighborsSurfacewaterFlowRateReady[channelNeighborsSize];  // Flag to indicate updated for the current timestep.
  double            channelNeighborsSurfacewaterFlowRate[channelNeighborsSize];       // Cubic meters of water per second.
  double            channelNeighborsSurfacewaterCumulativeFlow[channelNeighborsSize]; // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                      // Gets set to zero at initialization and each I/O phase.
  FlowRateReadyEnum channelNeighborsGroundwaterFlowRateReady[channelNeighborsSize];   // Flag to indicate updated for the current timestep.
  double            channelNeighborsGroundwaterFlowRate[channelNeighborsSize];        // Cubic meters of water per second.
  double            channelNeighborsGroundwaterCumulativeFlow[channelNeighborsSize];  // Cubic meters of water.  Cumulative flow since last set to zero.
                                                                                      // Gets set to zero at initialization and each I/O phase.
  double            surfacewaterInfiltration;                                         // Meters of water infiltrating from the surface.  Positive means flow
                                                                                      // from the surface to the vadose zone.  Must be non-negative.  If water
                                                                                      // exfiltrates from groundwater to the surface that is handled in a
                                                                                      // different way in moveGroundwater.
                                                                                      // FIXME store cumulative infiltration?
  double            groundwaterRecharge;                                              // Meters of water flowing between the vadose zone and groundwater.
                                                                                      // Positive means flow from the vadose zone to the groundwater.
                                                                                      // Negative means flow from the groundwater to the vadose zone.
                                                                                      // Gets updated during moveGroundwater to the net water added or removed
                                                                                      // from the groundwater including infiltration and all groundwater flows.
                                                                                      // Positive means added.  Negative means removed.
  
  // Sequencing and timestep information.
  bool   groundwaterDone;  // Flag indicating when the groundwater phase is done.
  bool   infiltrationDone; // Flag indicating when the infiltration calculation is done.
  bool   surfacewaterDone; // Flag indicating when the surfacewater phase is done.
  double dt;               // Current or most recent timestep duration in seconds.
  double dtNew;            // Suggested value for next timestep duration in seconds.
};

#endif // __MESH_ELEMENT_H__
