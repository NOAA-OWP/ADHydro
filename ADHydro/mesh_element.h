#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

#include "mesh_element.decl.h"

class MeshElement : public CBase_MeshElement
{
  MeshElement_SDAG_CODE
  
public:
  
  // Constructor.
  MeshElement();
  
  // Charm++ migration constructor.
  //
  // Parameters:
  //
  // msg - Charm++ migration message.
  MeshElement(CkMigrateMessage* msg);
  
  // Pack/unpack method.
  void pup(PUP::er &p);
  
  // Flags to indicate how to interact with neighbors.
  enum InteractionEnum
  {
    I_CALCULATE_FLOW,
    NEIGHBOR_CALCULATES_FLOW,
    BOTH_CALCULATE_FLOW,
  };
  
  // Flags to indicate whether temporary flow variables have been updated for
  // the current timestep.
  enum FlowReadyEnum
  {
    FLOW_NOT_READY,
    FLOW_CALCULATED,
    FLOW_LIMITING_CHECK_DONE
  };
  
private:
  
  // Step forward one timestep.  Performs point processes and starts the
  // surfacewater, groundwater, and channels algorithm.
  //
  // Parameters:
  //
  // iterationThisTimestep - Iteration number to put on all messages this
  //                         timestep.
  // dtThisTimestep        - Duration for this timestep in seconds.
  void receiveDoTimestep(int iterationThisTimestep, double dtThisTimestep);
  
  // Perform the snow melt point process.
  void doSnowMelt();

  // Perform the rainfall point process.
  void doRainfall();

  // Perform the evapo-transpiration point process.
  void doEvapoTranspiration();

  // Perform the infiltration point process.
  void doInfiltration();
  
  // Send the state messages that start the surfacewater, groundwater, and
  // channels algorithm.
  void sendState();
  
  // Calculate flow for all boundary condition edges.
  void receiveCalculateBoundaryConditionFlow();
  
  // Receive a state message from a neighbor, calculate flow across the edge
  // shared with the neighbor, and possibly send a flow message back to the
  // neighbor.
  //
  // Parameters:
  //
  // edge                      - My edge number for the edge shared with the
  //                             neighbor.
  // neighborSurfacewaterDepth - Surfacewater depth of the neighbor in meters.
  // neighborGroundwaterHead   - Groundwater head of the neighbor in meters.
  void receiveState(int edge, double neighborSurfacewaterDepth, double neighborGroundwaterHead);
  
  // Receive a flow message from a neighbor.  This message indicates that the
  // neighbor has calculated the flow across the shared edge, but the flow
  // might still need to be limited.
  //
  // Parameters:
  //
  // edge                 - My edge number for the edge shared with the
  //                        neighbor.
  // edgeSurfacewaterFlow - Surfacewater flow in cubic meters across the edge
  //                        shared with the neighbor.
  // edgeGroundwaterFlow  - Groundwater flow in cubic meters across the edge
  //                        shared with the neighbor.
  void receiveFlow(int edge, double edgeSurfacewaterFlow, double edgeGroundwaterFlow);
  
  // Receive a flow limited message from a neighbor.  This message indicates
  // that the neighbor has calculated whether or not the flow needs to be
  // limited and what the final correct flow value is.  It does not necessarily
  // mean that the flow was limited, but that the flow limiting check has been
  // done and here is the final value, which may or may not have been limited.
  //
  // Parameters:
  //
  // edge                 - My edge number for the edge shared with the
  //                        neighbor.
  // edgeSurfacewaterFlow - Surfacewater flow in cubic meters across the edge
  //                        shared with the neighbor.
  void receiveSurfacewaterFlowLimited(int edge, double edgeSurfacewaterFlow);
  
  // Receive a flow limited message from a neighbor.  This message indicates
  // that the neighbor has calculated whether or not the flow needs to be
  // limited and what the final correct flow value is.  It does not necessarily
  // mean that the flow was limited, but that the flow limiting check has been
  // done and here is the final value, which may or may not have been limited.
  //
  // Parameters:
  //
  // edge                - My edge number for the edge shared with the
  //                       neighbor.
  // edgeGroundwaterFlow - Groundwater flow in cubic meters across the edge
  //                       shared with the neighbor.
  void receiveGroundwaterFlowLimited(int edge, double edgeGroundwaterFlow);
  
  // Check the ready state of all flows.  If all flows are calculated do the
  // flow limiting check for outward flows.  If all flow limiting checks are
  // done move water.
  void checkAllFlows();
  
  // Update the values of water state variables to reflect the result of all
  // flows.
  void moveWater();
  
  // Geometric coordinates of vertices (one based indexing).
  double vertexX[4];        // Meters.
  double vertexY[4];        // Meters.
  double vertexZSurface[4]; // Meters.
  double vertexZBedrock[4]; // Meters.
  
  // Geometric coordinates of edges (one based indexing).
  double edgeLength[4];  // Meters.
  double edgeNormalX[4]; // X component of normal unit vector.
  double edgeNormalY[4]; // Y component of normal unit vector.
  
  // Geometric coordinates of element center or entire element.
  double elementX;        // Meters.
  double elementY;        // Meters.
  double elementZSurface; // Meters.
  double elementZBedrock; // Meters.
  double elementArea;     // Square meters.
  
  // Neighbor element or boundary condition code (one based indexing).
  int neighbor[4]; // Array index into global chare array or NOFLOW, INFLOW, or OUTFLOW.
  
  // Neighbor element's edge number for our shared edge (one based indexing).
  int neighborReciprocalEdge[4]; // forall edge in {1, 2, 3} thisProxy[neighbor[edge]].neighbor[neighborReciprocalEdge[edge]] == thisIndex
  
  // How to interact with neighbors (one based indexing).
  InteractionEnum interaction[4];
  
  // Geometric coordinates of neighbor centers (one based indexing).
  double neighborX[4];        // Meters.
  double neighborY[4];        // Meters.
  double neighborZSurface[4]; // Meters.
  double neighborZBedrock[4]; // Meters.
  
  // Hydraulic parameters of neighbors.
  double neighborConductivity[4]; // Meters per second.
  double neighborManningsN[4];    // Seconds/(meters^(1/3)).
  
  // Catchment number that this element belongs to.
  int catchment; // ID number.
  
  // Hydraulic parameters of element.
  double conductivity; // Meters per second.
  double porosity;     // Unitless fraction.
  double manningsN;    // Seconds/(meters^(1/3)).
  
  // Water state variables.
  double surfacewaterDepth; // Meters.
  double surfacewaterError; // Meters.  Positive means water was created.  Negative means water was destroyed.
  double groundwaterHead;   // Meters.
  double groundwaterError;  // Meters.  Positive means water was created.  Negative means water was destroyed.
  
  // Per-edge temporary variables (one based indexing).
  double        surfacewaterFlow[4];      // Cubic meters.
  FlowReadyEnum surfacewaterFlowReady[4]; // Whether surfacewaterFlow has been updated for the current timestep.
  double        groundwaterFlow[4];       // Cubic meters.
  FlowReadyEnum groundwaterFlowReady[4];  // Whether groundwaterFlow  has been updated for the current timestep.
  
  // Timestep information.
  int    iteration;    // Iteration number to put on all messages this timestep.
  bool   timestepDone; // Flag indicating the current timestep is done.
  double dt;           // Current timestep duration in seconds.
  double dtNew;        // Suggested value for next timestep duration in seconds.
};

PUPbytes(MeshElement::InteractionEnum);
PUPbytes(MeshElement::FlowReadyEnum);

#endif // __MESH_ELEMENT_H__
