#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

#include "file_manager.h"
#include "all.h"

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "mesh_element.decl.h"
#pragma GCC diagnostic warning "-Wsign-compare"

// A MeshElement object represents one triangle in the simulation mesh.
// A chare array of MeshElement objects represents the entire simulation mesh.
class MeshElement : public CBase_MeshElement
{
  MeshElement_SDAG_CODE
  
public:
  
  // Constructor.  The constructor initializes the object by reading from the
  // FileManager.  The FileManager files must already be opened for read.  The
  // constructor does not perform an invariant check on the input because at
  // the end of an invariant check it contributes to an empty reduction as a
  // barrier.  The calling program should send a checkInvariant message to the
  // entire chare array after initialization.  Exit on error.
  MeshElement(CProxy_FileManager fileManagerProxyInit);
  
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

private:
  
  // Initialize the member variables of this element. Initialization data comes
  // from the FileManager passed to the constructor.  Files must already be
  // opened for read.  Contribute to an empty reduction as a barrier when
  // complete or exit on error.
  void initialize();
  
  // Step forward one timestep.  Performs point processes and starts the
  // surfacewater, groundwater, and channels algorithm.  Exit on error.
  //
  // Parameters:
  //
  // iterationThisTimestep - Iteration number to put on all messages this
  //                         timestep.
  // dtThisTimestep        - Duration for this timestep in seconds.
  void handleDoTimestep(int iterationThisTimestep, double dtThisTimestep);
  
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
  
  // Calculate flow rate for all boundary condition edges.  Exit on error.
  void handleCalculateBoundaryConditionFlowRate();
  
  // Receive a state message from a neighbor, calculate flow rate across the
  // edge shared with the neighbor, and possibly send a flow rate message back
  // to the neighbor.  Exit on error.
  //
  // Parameters:
  //
  // edge                      - My edge number for the edge shared with the
  //                             neighbor.
  // neighborSurfacewaterDepth - Surfacewater depth of the neighbor in meters.
  // neighborGroundwaterHead   - Groundwater head of the neighbor in meters.
  void handleStateMessage(int edge, double neighborSurfacewaterDepth, double neighborGroundwaterHead);
  
  // Receive a flow rate message from a neighbor.  This message indicates that
  // the neighbor has calculated the flow rate across the shared edge, but the
  // flow rate might still need to be limited.  Exit on error.
  //
  // Parameters:
  //
  // edge                     - My edge number for the edge shared with the
  //                            neighbor.
  // edgeSurfacewaterFlowRate - Surfacewater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  // edgeGroundwaterFlowRate  - Groundwater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  void handleFlowRateMessage(int edge, double edgeSurfacewaterFlowRate, double edgeGroundwaterFlowRate);
  
  // Receive a flow rate limited message from a neighbor.  This message
  // indicates that the neighbor has calculated whether or not the flow rate
  // needs to be limited and what the final correct value is.  It does not
  // necessarily mean that the flow rate was limited, but that the flow rate
  // limiting check has been done and here is the final value, which may or may
  // not have been limited.  Exit on error.
  //
  // Parameters:
  //
  // edge                     - My edge number for the edge shared with the
  //                            neighbor.
  // edgeSurfacewaterFlowRate - Surfacewater flow rate in cubic meters across
  //                            the edge shared with the neighbor.
  void handleSurfacewaterFlowRateLimitedMessage(int edge, double edgeSurfacewaterFlowRate);
  
  // Check the ready state of all flow rates.  If all flow rates are calculated
  // do the flow rate limiting check for surfacewater outward flow rates, and
  // if all flow rate limiting checks are done move surfacewater.
  void checkAllFlowRates();
  
  // Update the values of water state variables to reflect the result of all
  // surfacewater flow rates.  Contribute to dtNew reduction when complete.
  void moveSurfacewater();
  
  // Perform an output phase sending data to the FileManager passed to the
  // constructor.  Files must already be opened for write.  Contribute to
  // an empty reduction as a barrier when complete or exit on error.
  void handleOutput();
  
  // Check invariant on member variables and send information to neighbors to
  // check relationship invariant.  When all relationship invariants have been
  // checked the structured dagger code will contribute to an empty reduction
  // as a barrier.  Exit if invariant is violated.
  void handleCheckInvariant();
  
  // Check relationship invariant on information from a neighbor.  When all
  // relationship invariants have been checked the structured dagger code will
  // contribute to an empty reduction as a barrier. Exit if invariant is
  // violated.
  //
  // Parameters:
  //
  // neighborIndex                      - Neighbor value for invariant check.
  // neighborsNeighborInteraction       - Neighbor value for invariant check.
  // neighborEdge                       - Neighbor value for invariant check.
  // neighborsNeighborReciprocalEdge    - Neighbor value for invariant check.
  // neighborVertexX1                   - Neighbor value for invariant check.
  // neighborVertexX2                   - Neighbor value for invariant check.
  // neighborVertexY1                   - Neighbor value for invariant check.
  // neighborVertexY2                   - Neighbor value for invariant check.
  // neighborVertexZSurface1            - Neighbor value for invariant check.
  // neighborVertexZSurface2            - Neighbor value for invariant check.
  // neighborVertexZBedrock1            - Neighbor value for invariant check.
  // neighborVertexZBedrock2            - Neighbor value for invariant check.
  // neighborElementX                   - Neighbor value for invariant check.
  // neighborElementY                   - Neighbor value for invariant check.
  // neighborElementZSurface            - Neighbor value for invariant check.
  // neighborElementZBedrock            - Neighbor value for invariant check.
  // neighborElementConductivity        - Neighbor value for invariant check.
  // neighborElementManningsN           - Neighbor value for invariant check.
  // neighborSurfacewaterFlowRate       - Neighbor value for invariant check.
  // neighborSurfacewaterCumulativeFlow - Neighbor value for invariant check.
  // neighborGroundwaterFlowRate        - Neighbor value for invariant check.
  // neighborGroundwaterCumulativeFlow  - Neighbor value for invariant check.
  // neighborIteration                  - Neighbor value for invariant check.
  // neighborDt                         - Neighbor value for invariant check.
  void handleCheckInvariantNeighbor(int neighborIndex, InteractionEnum neighborsNeighborInteraction, int neighborEdge,
                                    int neighborsNeighborReciprocalEdge, double neighborVertexX1, double neighborVertexX2, double neighborVertexY1,
                                    double neighborVertexY2, double neighborVertexZSurface1, double neighborVertexZSurface2,
                                    double neighborVertexZBedrock1, double neighborVertexZBedrock2, double neighborElementX,
                                    double neighborElementY, double neighborElementZSurface, double neighborElementZBedrock,
                                    double neighborElementConductivity, double neighborElementManningsN, double neighborSurfacewaterFlowRate,
                                    double neighborSurfacewaterCumulativeFlow, double neighborGroundwaterFlowRate,
                                    double neighborGroundwaterCumulativeFlow, int neighborIteration, double neighborDt);
  
  // File manager for I/O.
  CProxy_FileManager fileManagerProxy;
  
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
  
  // How to interact with neighbors.
  InteractionEnum neighborInteraction[3];
  
  // Neighbor element's edge number for our shared edge.
  int neighborReciprocalEdge[3]; // forall edge in {0, 1, 2}, thisProxy[neighbor[edge]].neighbor[neighborReciprocalEdge[edge]] == thisIndex
  
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
  double surfacewaterDepth;   // Meters of height and meters of water.
  double surfacewaterError;   // Meters of water.  Positive means water was created.  Negative means water was destroyed.
  double groundwaterHead;     // Meters of height.
  double groundwaterError;    // Meters of water.  Positive means water was created.  Negative means water was destroyed.
  double groundwaterRecharge; // Meters of water.
  
  // Water flow state variables.
  double            surfacewaterFlowRate[3];       // Cubic meters of water per second.
  FlowRateReadyEnum surfacewaterFlowRateReady[3];  // Whether surfacewaterFlowRate has been updated for the current timestep.
  double            surfacewaterCumulativeFlow[3]; // Cubic meters of water.  Cumulative flow across edge since last set to zero.
                                                   // Gets set to zero at initialization and each I/O phase.
  double            groundwaterFlowRate[3];        // Cubic meters of water per second.
  FlowRateReadyEnum groundwaterFlowRateReady[3];   // Whether groundwaterFlowRate  has been updated for the current timestep.
  double            groundwaterCumulativeFlow[3];  // Cubic meters of water.  Cumulative flow across edge since last set to zero.
                                                   // Gets set to zero at initialization and each I/O phase.
  
  // Sequencing and timestep information.
  bool   timestepDone;                // Flag indicating the current timestep is done, or undefined if not in a timestep.
  bool   neighborInvariantChecked[3]; // Flags indicating if the relationship invariant with each neighbor has been checked, or undefined if not checking
                                      // invariant.
  int    iteration;                   // Iteration number to put on all messages this timestep or most recent timestep.
  double dt;                          // Current or most recent timestep duration in seconds.
  double dtNew;                       // Suggested value for next timestep duration in seconds.
};

#endif // __MESH_ELEMENT_H__
