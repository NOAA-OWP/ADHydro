#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

// This must be declared before #include "mesh_element.decl.h" to be used as a
// parameter for the entry method sendInitialize.  Also, it cannot be a member
// of MeshElement like it should be because C++ doesn't allow scope qualified
// forward declarations like struct MeshElement::InitStruct;
struct MeshElementInitStruct;

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
  
  // Initialize the member variables of this element.  This is not done in the
  // constructor because Charm++ passes the same constructor parameters to all
  // of the elements of an array while we need to initialize each element
  // differently.
  //
  // Parameters:
  //
  // initialValues - Member variable initial values.
  void receiveInitialize(MeshElementInitStruct& initialValues);
  
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
  
  // Neighbor element or boundary condition code.
  int neighbor[3]; // Array index into global chare array or NOFLOW, INFLOW, or OUTFLOW.
  
  // Neighbor element's edge number for our shared edge.
  int neighborReciprocalEdge[3]; // forall edge in {0, 1, 2}, thisProxy[neighbor[edge]].neighbor[neighborReciprocalEdge[edge]] == thisIndex
  
  // How to interact with neighbors.
  InteractionEnum interaction[3];
  
  // Geometric coordinates of neighbor centers.
  double neighborX[3];        // Meters.
  double neighborY[3];        // Meters.
  double neighborZSurface[3]; // Meters.
  double neighborZBedrock[3]; // Meters.
  
  // Hydraulic parameters of neighbors.
  double neighborConductivity[3]; // Meters per second.
  double neighborManningsN[3];    // Seconds/(meters^(1/3)).
  
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
  
  // Per-edge temporary variables.
  double        surfacewaterFlow[3];      // Cubic meters.
  FlowReadyEnum surfacewaterFlowReady[3]; // Whether surfacewaterFlow has been updated for the current timestep.
  double        groundwaterFlow[3];       // Cubic meters.
  FlowReadyEnum groundwaterFlowReady[3];  // Whether groundwaterFlow  has been updated for the current timestep.
  
  // Timestep information.
  int    iteration;    // Iteration number to put on all messages this timestep.
  bool   timestepDone; // Flag indicating the current timestep is done.
  double dt;           // Current timestep duration in seconds.
  double dtNew;        // Suggested value for next timestep duration in seconds.
};

// Structure for passing values to initialize MeshElement member variables.
struct MeshElementInitStruct
{
  double                       vertexX[3];
  double                       vertexY[3];
  double                       vertexZSurface[3];
  double                       vertexZBedrock[3];
  int                          neighbor[3];
  int                          neighborReciprocalEdge[3];
  MeshElement::InteractionEnum interaction[3];
  int                          catchment;
  double                       conductivity;
  double                       porosity;
  double                       manningsN;
  double                       surfacewaterDepth;
  double                       surfacewaterError;
  double                       groundwaterHead;
  double                       groundwaterError;
};

PUPbytes(MeshElement::InteractionEnum);
PUPbytes(MeshElement::FlowReadyEnum);
PUPbytes(MeshElementInitStruct);

#endif // __MESH_ELEMENT_H__
