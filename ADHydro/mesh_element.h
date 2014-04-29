#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

// Flags to indicate how to interact with neighbors.
// This must be declared before #include "mesh_element.decl.h" to be used as a
// parameter for the entry method sendInitialize.  Also, it cannot be a member
// of MeshElement like it should be because C++ doesn't allow scope qualified
// forward declarations like enum MeshElement::InteractionEnum;
enum InteractionEnum
{
  I_CALCULATE_FLOW_RATE,
  NEIGHBOR_CALCULATES_FLOW_RATE,
  BOTH_CALCULATE_FLOW_RATE,
};

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
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);

  // Flags to indicate whether temporary flow rate variables have been updated
  // for the current timestep.
  enum FlowRateReadyEnum
  {
    FLOW_RATE_NOT_READY,
    FLOW_RATE_CALCULATED,
    FLOW_RATE_LIMITING_CHECK_DONE
  };

private:
  
  // Initialize the member variables of this element.  This is not done in the
  // constructor because Charm++ passes the same constructor parameters to all
  // of the elements of an array while we need to initialize each element
  // differently.
  //
  // Parameters:
  //
  // vertexXInit             - Member variable initial value.
  // vertexYInit             - Member variable initial value.
  // vertexZSurfaceInit      - Member variable initial value.
  // vertexZBedrockInit      - Member variable initial value.
  // neighborInit            - Member variable initial value.
  // interactionInit         - Member variable initial value.
  // catchmentInit           - Member variable initial value.
  // conductivityInit        - Member variable initial value.
  // porosityInit            - Member variable initial value.
  // manningsNInit           - Member variable initial value.
  // surfacewaterDepthInit   - Member variable initial value.
  // surfacewaterErrorInit   - Member variable initial value.
  // groundwaterHeadInit     - Member variable initial value.
  // groundwaterErrorInit    - Member variable initial value.
  // groundwaterRechargeInit - Member variable initial value.
  void receiveInitialize(double vertexXInit[3], double vertexYInit[3], double vertexZSurfaceInit[3], double vertexZBedrockInit[3],
                         int neighborInit[3], InteractionEnum interactionInit[3], int catchmentInit, double conductivityInit, double porosityInit,
                         double manningsNInit, double surfacewaterDepthInit, double surfacewaterErrorInit, double groundwaterHeadInit,
                         double groundwaterErrorInit, double groundwaterRechargeInit);
  
  // Initialize the member variables of this element with readonly derived
  // values from neighbors.
  //
  // Parameters:
  //
  // neighborIndex               - Neighbor element sending me initialization
  //                               information.
  // neighborEdge                - Neighbor elemnt's edge number for our shared
  //                               edge.
  // neighborElementX            - Member variable initial value.
  // neighborElementY            - Member variable initial value.
  // neighborElementZSurface     - Member variable initial value.
  // neighborElementZBedrock     - Member variable initial value.
  // neighborElementConductivity - Member variable initial value.
  // neighborElementManningsN    - Member variable initial value.
  void receiveInitializeNeighbor(int neighborIndex, int neighborEdge, double neighborElementX, double neighborElementY,
                                 double neighborElementZSurface, double neighborElementZBedrock, double neighborElementConductivity,
                                 double neighborElementManningsN);
  
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
  
  // Update the values of water state variables to reflect the result of all
  // groundwater flow rates and groundwater recharge.
  void moveGroundwater();
  
  // Send the state messages that start the surfacewater, groundwater, and
  // channels algorithm.
  void sendState();
  
  // Calculate flow rate for all boundary condition edges.
  void receiveCalculateBoundaryConditionFlowRate();
  
  // Receive a state message from a neighbor, calculate flow rate across the
  // edge shared with the neighbor, and possibly send a flow rate message back
  // to the neighbor.
  //
  // Parameters:
  //
  // edge                      - My edge number for the edge shared with the
  //                             neighbor.
  // neighborSurfacewaterDepth - Surfacewater depth of the neighbor in meters.
  // neighborGroundwaterHead   - Groundwater head of the neighbor in meters.
  void receiveState(int edge, double neighborSurfacewaterDepth, double neighborGroundwaterHead);
  
  // Receive a flow rate message from a neighbor.  This message indicates that
  // the neighbor has calculated the flow rate across the shared edge, but the
  // flow rate might still need to be limited.
  //
  // Parameters:
  //
  // edge                     - My edge number for the edge shared with the
  //                            neighbor.
  // edgeSurfacewaterFlowRate - Surfacewater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  // edgeGroundwaterFlowRate  - Groundwater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  void receiveFlowRate(int edge, double edgeSurfacewaterFlowRate, double edgeGroundwaterFlowRate);
  
  // Receive a flow rate limited message from a neighbor.  This message
  // indicates that the neighbor has calculated whether or not the flow rate
  // needs to be limited and what the final correct value is.  It does not
  // necessarily mean that the flow rate was limited, but that the flow rate
  // limiting check has been done and here is the final value, which may or may
  // not have been limited.
  //
  // Parameters:
  //
  // edge                     - My edge number for the edge shared with the
  //                            neighbor.
  // edgeSurfacewaterFlowRate - Surfacewater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  void receiveSurfacewaterFlowRateLimited(int edge, double edgeSurfacewaterFlowRate);
  
  // Check the ready state of all flow rates.  If all flow rates are calculated
  // do the flow rate limiting check for outward flow rates, and if all flow
  // rate limiting checks are done move surfacewater.
  void checkAllFlowRates();
  
  // Update the values of water state variables to reflect the result of all
  // surfacewater flow rates.
  void moveSurfacewater();
  
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
  
  // Whether you have received readonly derived values from neighbors.
  // Set to true if neighbor is a boundary condition code.
  bool neighborInitialized[3];
  
  // Catchment number that this element belongs to.
  int catchment; // ID number.
  
  // Hydraulic parameters of element.
  double conductivity; // Meters per second.
  double porosity;     // Unitless fraction.
  double manningsN;    // Seconds/(meters^(1/3)).
  
  // Water state variables.
  double surfacewaterDepth;   // Meters of height and meters of water.
  double surfacewaterError;   // Meters of water.  Positive means water was created.  Negative means water was destroyed.
  double groundwaterHead;     // Meters of height.
  double groundwaterError;    // Meters of water.  Positive means water was created.  Negative means water was destroyed.
  double groundwaterRecharge; // Meters of water.
  
  // Water flow temporary variables.
  double            surfacewaterFlowRate[3];      // Cubic meters of water per second.
  FlowRateReadyEnum surfacewaterFlowRateReady[3]; // Whether surfacewaterFlowRate has been updated for the current timestep.
  double            groundwaterFlowRate[3];       // Cubic meters of water per second.
  FlowRateReadyEnum groundwaterFlowRateReady[3];  // Whether groundwaterFlowRate  has been updated for the current timestep.
  
  // Timestep information.
  int    iteration;    // Iteration number to put on all messages this timestep.
  bool   timestepDone; // Flag indicating the current timestep is done.
  double dt;           // Current timestep duration in seconds.
  double dtNew;        // Suggested value for next timestep duration in seconds.
};

PUPbytes(InteractionEnum);
PUPbytes(MeshElement::FlowRateReadyEnum);

#endif // __MESH_ELEMENT_H__
