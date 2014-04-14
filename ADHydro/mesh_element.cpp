#include "mesh_element.h"
#include "surfacewater.h"
#include "groundwater.h"
#include "all.h"
#include <assert.h>

// FIXME stubs
void doSnowMelt() {}
void doRainfall() {}
void doEvapoTranspiration() {}

// FIXME questions and to-do list items
// How to make it send the high priority messages out first?  We want all the messages going to other nodes to go out as soon as possible.
// Will it send out one MPI message per mesh edge rather than all of the ghost node information in a single message?
// As elements migrate to different nodes update interaction in a way that guarantees both sides agree on the interaction.
// When you implement groundwater/channel interaction include surfacewater depth in groundwater head.
// Assert all outward flow factors between 0 and 1, or even epsilon equal to 1?
// Scale channel dx w/ bankfull depth?

MeshElement::MeshElement()
{
  // FIXME set up member variables
  
  // FIXME hardcode simple mesh
  if (0 == thisIndex)
    {
      vertexX[1]                =   0.0;
      vertexX[2]                = 100.0;
      vertexX[3]                =   0.0;
      vertexY[1]                =   0.0;
      vertexY[2]                = 100.0;
      vertexY[3]                = 100.0;
      vertexZSurface[1]         =  10.0;
      vertexZSurface[2]         =   0.0;
      vertexZSurface[3]         =  10.0;
      vertexZBedrock[1]         =   0.0;
      vertexZBedrock[2]         = -10.0;
      vertexZBedrock[3]         =   0.0;
      neighbor[1]               = NOFLOW;
      neighbor[2]               = INFLOW;
      neighbor[3]               = 1;
      neighborReciprocalEdge[3] = 2;
      //interaction[3]            = BOTH_CALCULATE_FLOW;
      interaction[3]            = I_CALCULATE_FLOW;
    }
  else if (1 == thisIndex)
    {
      vertexX[1]                =   0.0;
      vertexX[2]                = 100.0;
      vertexX[3]                = 100.0;
      vertexY[1]                =   0.0;
      vertexY[2]                =   0.0;
      vertexY[3]                = 100.0;
      vertexZSurface[1]         =  10.0;
      vertexZSurface[2]         =   0.0;
      vertexZSurface[3]         =   0.0;
      vertexZBedrock[1]         =   0.0;
      vertexZBedrock[2]         = -10.0;
      vertexZBedrock[3]         = -10.0;
      neighbor[1]               = OUTFLOW;
      neighbor[2]               = 0;
      neighbor[3]               = NOFLOW;
      neighborReciprocalEdge[2] = 3;
      //interaction[2]            = BOTH_CALCULATE_FLOW;
      interaction[2]            = NEIGHBOR_CALCULATES_FLOW;
    }
  else
    {
      assert(false);
    }
  
  // Calculate derived values.
  
  // Length of each edge.
  // Edge 1 goes from vertex 2 to 3 (opposite vertex 1).
  // Edge 2 goes form vertex 3 to 1 (opposite vertex 2).
  // Edge 3 goes form vertex 1 to 2 (opposite vertex 3).
  edgeLength[1] = sqrt((vertexX[2] - vertexX[3]) * (vertexX[2] - vertexX[3]) + (vertexY[2] - vertexY[3]) * (vertexY[2] - vertexY[3]));
  edgeLength[2] = sqrt((vertexX[3] - vertexX[1]) * (vertexX[3] - vertexX[1]) + (vertexY[3] - vertexY[1]) * (vertexY[3] - vertexY[1]));
  edgeLength[3] = sqrt((vertexX[1] - vertexX[2]) * (vertexX[1] - vertexX[2]) + (vertexY[1] - vertexY[2]) * (vertexY[1] - vertexY[2]));

  // Unit normal vector of each edge.
  edgeNormalX[1] = (vertexY[3] - vertexY[2]) / edgeLength[1];
  edgeNormalY[1] = (vertexX[2] - vertexX[3]) / edgeLength[1];
  edgeNormalX[2] = (vertexY[1] - vertexY[3]) / edgeLength[2];
  edgeNormalY[2] = (vertexX[3] - vertexX[1]) / edgeLength[2];
  edgeNormalX[3] = (vertexY[2] - vertexY[1]) / edgeLength[3];
  edgeNormalY[3] = (vertexX[1] - vertexX[2]) / edgeLength[3];

  // Geometric coordinates of element center.
  elementX        = (vertexX[1]        + vertexX[2]        + vertexX[3])        / 3.0;
  elementY        = (vertexY[1]        + vertexY[2]        + vertexY[3])        / 3.0;
  elementZSurface = (vertexZSurface[1] + vertexZSurface[2] + vertexZSurface[3]) / 3.0;
  elementZBedrock = (vertexZBedrock[1] + vertexZBedrock[2] + vertexZBedrock[3]) / 3.0;

  // Area of element.
  elementArea = (vertexX[1] * (vertexY[2] - vertexY[3]) + vertexX[2] * (vertexY[3] - vertexY[1]) + vertexX[3] * (vertexY[1] - vertexY[2])) * 0.5;
  
  // Send calculated values to neighbors.
  // FIXME implement message
  if (0 == thisIndex)
    {
      neighborX[3]        = ( 0.0 + 100.0 + 100.0) / 3.0;
      neighborY[3]        = ( 0.0 +   0.0 + 100.0) / 3.0;
      neighborZSurface[3] = (10.0 +   0.0 +   0.0) / 3.0;
      neighborZBedrock[3] = ( 0.0 + -10.0 + -10.0) / 3.0;
      neighborConductivity[3] = 1.0e-3;
      neighborManningsN[3]    = 0.04;
    }
  else if (1 == thisIndex)
    {
      neighborX[2]        = ( 0.0 + 100.0 +   0.0) / 3.0;
      neighborY[2]        = ( 0.0 + 100.0 + 100.0) / 3.0;
      neighborZSurface[2] = (10.0 +   0.0 +  10.0) / 3.0;
      neighborZBedrock[2] = ( 0.0 + -10.0 +   0.0) / 3.0;
      neighborConductivity[2] = 1.0e-3;
      neighborManningsN[2]    = 0.04;
    }
  else
    {
      assert(false);
    }
  
  // Hydraulic parameters of element.
  conductivity = 1.0e-3;
  porosity     = 0.5;
  manningsN    = 0.04;
  
  // Water state variables.
  surfacewaterDepth = 0;
  groundwaterHead   = 0;
}

MeshElement::MeshElement(CkMigrateMessage* msg)
{
  // Do nothing.
}

void MeshElement::doTimestep(double dtThisTimestep)
{
  int edge; // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME how to return an error?
  assert(0.0 < dtThisTimestep);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Set the flow states to not ready for this timestep.
  for (edge = 1; edge <= 3; edge++)
    {
      surfacewaterFlowReady[edge] = FLOW_NOT_READY;
      groundwaterFlowReady[edge]  = FLOW_NOT_READY;
    }
  
  // Save the new timestep duration.
  dt    = dtThisTimestep;
  dtNew = 2.0 * dt;
  
  // Do point processes.
  doSnowMelt();
  doRainfall();
  doInfiltration();
  doEvapoTranspiration();
  
  // Allow receipt of messages for the current timestep.
  // FIXME implement
  
  // Start surfacewater, groundwater, and channels algorithm.
  sendState();
  
  // FIXME Should we make calculateBoundaryConditionFlow a low priority message so that other state messages can go out without waiting for that computation?
  // FIXME Is it enough computation to make it worth doing that?  Try it both ways to find out.
  //calculateBoundaryConditionFlow();
  thisProxy[thisIndex].calculateBoundaryConditionFlow();
}

void MeshElement::calculateBoundaryConditionFlow()
{
  int edge; // Loop counter.

  for (edge = 1; edge <= 3; edge++)
    {
      if (isBoundary(neighbor[edge]))
        {
          // Calculate surfacewater flow rate.
          // FIXME check error condition
          // FIXME figure out what to do about inflow boundary velocity and height
          surfacewaterBoundaryFlowRate(&surfacewaterFlow[edge], (BoundaryConditionEnum)neighbor[edge], 1.0, 1.0, 1.0, edgeLength[edge], edgeNormalX[edge],
                                       edgeNormalY[edge], surfacewaterDepth);
          
          // Convert from cubic meters per second to cubic meters.
          surfacewaterFlow[edge] *= dt;
          
          // Outflows may need to be limited.  Inflows and noflows will not.
          if (0.0 < surfacewaterFlow[edge])
            {
              surfacewaterFlowReady[edge] = FLOW_CALCULATED;
            }
          else
            {
              surfacewaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
            }
          
          // Calculate groundwater flow rate.
          // FIXME check error condition
          groundwaterBoundaryFlowRate(&groundwaterFlow[edge], (BoundaryConditionEnum)neighbor[edge], vertexX, vertexY, vertexZSurface, edgeLength[edge],
                                      edgeNormalX[edge], edgeNormalY[edge], elementZBedrock, elementArea, conductivity, groundwaterHead);
          
          // Convert from cubic meters per second to cubic meters.
          groundwaterFlow[edge] *= dt;
          
          // Outflows may need to be limited.  Inflows and noflows will not.
          if (0.0 < groundwaterFlow[edge])
            {
              groundwaterFlowReady[edge] = FLOW_CALCULATED;
            }
          else
            {
              groundwaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
            }
        }
    }

  checkAllFlows();
}

void MeshElement::receiveState(int edge, double neighborSurfacewaterDepth, double neighborGroundwaterHead)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME how to return an error?
  assert(1 <= edge && 3 >= edge && 0.0 <= neighborSurfacewaterDepth && neighborZBedrock[edge] <= neighborGroundwaterHead &&
         neighborGroundwaterHead <= neighborZSurface[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // There is a race condition where a flow limited message can arrive before a state message.
  // In that case, the flow limited message has the correct flow value and you can ignore the state message.
  if (FLOW_NOT_READY == surfacewaterFlowReady[edge])
    {
      // Calculate surfacewater flow rate.
      // FIXME check error condition
      surfacewaterElementNeighborFlowRate(&surfacewaterFlow[edge], &dtNew, edgeLength[edge], elementX, elementY, elementZSurface, elementArea, manningsN,
                                          surfacewaterDepth, neighborX[edge], neighborY[edge], neighborZSurface[edge], neighborManningsN[edge],
                                          neighborSurfacewaterDepth);
      
      // Convert from cubic meters per second to cubic meters.
      surfacewaterFlow[edge] *= dt;
      
      // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
      if (0.0 != surfacewaterFlow[edge])
        {
          surfacewaterFlowReady[edge] = FLOW_CALCULATED;
        }
      else
        {
          surfacewaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      assert(FLOW_LIMITING_CHECK_DONE == surfacewaterFlowReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  if (FLOW_NOT_READY == groundwaterFlowReady[edge])
    {
      // Calculate groundwater flow rate.
      // FIXME check error condition
      groundwaterElementNeighborFlowRate(&groundwaterFlow[edge], edgeLength[edge], elementX, elementY, elementZSurface, elementZBedrock, conductivity,
                                         surfacewaterDepth, groundwaterHead, neighborX[edge], neighborY[edge], neighborZSurface[edge], neighborZBedrock[edge],
                                         neighborConductivity[edge], neighborSurfacewaterDepth, neighborGroundwaterHead);
      
      // Convert from cubic meters per second to cubic meters.
      groundwaterFlow[edge] *= dt;
      
      // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
      if (0.0 != groundwaterFlow[edge])
        {
          groundwaterFlowReady[edge] = FLOW_CALCULATED;
        }
      else
        {
          groundwaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      assert(FLOW_LIMITING_CHECK_DONE == groundwaterFlowReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  switch (interaction[edge])
  {
  case I_CALCULATE_FLOW:
    // Send flow message.
    thisProxy[neighbor[edge]].receiveFlow(neighborReciprocalEdge[edge], -surfacewaterFlow[edge], -groundwaterFlow[edge]);
    break;
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  case NEIGHBOR_CALCULATES_FLOW:
    // I should never receive a state message with the NEIGHBOR_CALCULATES_FLOW interaction.
    assert(false);
    break;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // case BOTH_CALCULATE_FLOW:
    // Do nothing.  My neighbor will calculate the same flow values.
    // break;
  }
  
  checkAllFlows();
}

void MeshElement::receiveFlow(int edge, double edgeSurfacewaterFlow, double edgeGroundwaterFlow)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME how to return an error?
  assert(1 <= edge && 3 >= edge);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // I should only receive a flow message with the NEIGHBOR_CALCULATES_FLOW interaction.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  assert(NEIGHBOR_CALCULATES_FLOW == interaction[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // There is a race condition where a flow limited message can arrive before a flow message.
  // In that case, the flow limited message has the correct flow value and you can ignore the flow message.
  if (FLOW_NOT_READY == surfacewaterFlowReady[edge])
    {
      // Save received flow value.
      surfacewaterFlow[edge] = edgeSurfacewaterFlow;
      
      // Outflows may need to be limited.  Inflows may be limited by neighbor.  No flows will not.
      if (0.0 != surfacewaterFlow[edge])
        {
          surfacewaterFlowReady[edge] = FLOW_CALCULATED;
        }
      else
        {
          surfacewaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      assert(FLOW_LIMITING_CHECK_DONE == surfacewaterFlowReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  if (FLOW_NOT_READY == groundwaterFlowReady[edge])
    {
      // Save received flow value.
      groundwaterFlow[edge] = edgeGroundwaterFlow;
      
      // Outflows may need to be limited.  Inflows may be limited by neighbor.  No flows will not.
      if (0.0 != groundwaterFlow[edge])
        {
          groundwaterFlowReady[edge] = FLOW_CALCULATED;
        }
      else
        {
          groundwaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      assert(FLOW_LIMITING_CHECK_DONE == groundwaterFlowReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  checkAllFlows();
}

void MeshElement::receiveSurfacewaterFlowLimited(int edge, double edgeSurfacewaterFlow)
{
  // You should only receive this if it is an inflow and it should only reduce the magnitude of the flow.
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME how to return an error?
  assert(1 <= edge && 3 >= edge && 0.0 >= edgeSurfacewaterFlow && edgeSurfacewaterFlow >= surfacewaterFlow[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Save the received flow value.  The flow limiting check was done by neighbor.
  surfacewaterFlow[edge]      = edgeSurfacewaterFlow;
  surfacewaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
  
  checkAllFlows();
}

void MeshElement::receiveGroundwaterFlowLimited(int edge, double edgeGroundwaterFlow)
{
  // You should only receive this if it is an inflow and it should only reduce the magnitude of the flow.
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME how to return an error?
  assert(1 <= edge && 3 >= edge && 0.0 >= edgeGroundwaterFlow && edgeGroundwaterFlow >= groundwaterFlow[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Save the received flow value.  The flow limiting check was done by neighbor.
  groundwaterFlow[edge]      = edgeGroundwaterFlow;
  groundwaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
  
  checkAllFlows();
}

void MeshElement::doInfiltration()
{
  // FIXME trivial infiltration
  
  // Calculate the amount that infiltrates.
  double infiltrationAmount = conductivity * dt; // Meters of water.
  
  if (infiltrationAmount > surfacewaterDepth)
    {
      infiltrationAmount = surfacewaterDepth;
    }
  
  // Subtract that amount from surfacewater and add it to groundwater.
  surfacewaterDepth -= infiltrationAmount;
  groundwaterHead   += infiltrationAmount / porosity;
  
  // If groundwater rises above the surface put the extra water in surfacewater.
  if (groundwaterHead > elementZSurface)
    {
      surfacewaterDepth += (groundwaterHead - elementZSurface) * porosity;
      groundwaterHead    = elementZSurface;
    }
}

void MeshElement::sendState()
{
  int edge; // Loop counter.
  
  for (edge = 1; edge <= 3; edge++)
    {
      if (!isBoundary(neighbor[edge]))
        {
          switch (interaction[edge])
          {
          // case I_CALCULATE_FLOW:
            // Do nothing.  I will calculate the flow after receiving a state message from my neighbor.
            // break;
          case NEIGHBOR_CALCULATES_FLOW:
            // Fallthrough.
          case BOTH_CALCULATE_FLOW:
            // Send state message.
            thisProxy[neighbor[edge]].receiveState(neighborReciprocalEdge[edge], surfacewaterDepth, groundwaterHead);
            break;
          }
        }
    }
}

void MeshElement::checkAllFlows()
{
  int    edge;                                    // Loop counter.
  bool   surfacewaterAllCalculated       = true;  // Whether all surfacewater flows have been calculated.
  bool   groundwaterAllCalculated        = true;  // Whether all groundwater  flows have been calculated.
  bool   surfacewaterOutwardNotLimited   = false; // Whether at least one surfacewater outward flow is not limited.
  bool   groundwaterOutwardNotLimited    = false; // Whether at least one groundwater  outward flow is not limited.
  double surfacewaterTotalOutwardFlow    = 0.0;   // Sum of all surfacewater outward flows.
  double groundwaterTotalOutwardFlow     = 0.0;   // Sum of all groundwater  outward flows.
  double surfacewaterOutwardFlowFraction = 1.0;   // Fraction of all surfacewater outward flows that can be satisfied.
  double groundwaterOutwardFlowFraction  = 1.0;   // Fraction of all groundwater  outward flows that can be satisfied.
  bool   allLimited                      = true;  // Whether all flow limiting checks have been done.
  
  // We need to limit outward flows if all flows are at least calculated (not FLOW_NOT_READY) and there is at least one outward flow that is not limited.
  for (edge = 1; edge <= 3; edge++)
    {
      if (FLOW_NOT_READY == surfacewaterFlowReady[edge])
        {
          surfacewaterAllCalculated = false;
        }
      else if (FLOW_CALCULATED == surfacewaterFlowReady[edge] && 0.0 < surfacewaterFlow[edge])
        {
          surfacewaterOutwardNotLimited = true;
          surfacewaterTotalOutwardFlow += surfacewaterFlow[edge];
        }
      
      if (FLOW_NOT_READY == groundwaterFlowReady[edge])
        {
          groundwaterAllCalculated = false;
        }
      else if (FLOW_CALCULATED == groundwaterFlowReady[edge] && 0.0 < groundwaterFlow[edge])
        {
          groundwaterOutwardNotLimited = true;
          groundwaterTotalOutwardFlow += groundwaterFlow[edge];
        }
    }
  
  if (surfacewaterAllCalculated && surfacewaterOutwardNotLimited)
    {
      // Check if total outward flow is greater than water available.
      if (surfacewaterDepth * elementArea < surfacewaterTotalOutwardFlow)
        {
          surfacewaterOutwardFlowFraction = surfacewaterDepth * elementArea / surfacewaterTotalOutwardFlow;
        }
      
      // Limit outward flows.
      for (edge = 1; edge <= 3; edge++)
        {
          if (0.0 < surfacewaterFlow[edge])
            {
              surfacewaterFlow[edge]     *= surfacewaterOutwardFlowFraction;
              surfacewaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(neighbor[edge]))
                {
                  thisProxy[neighbor[edge]].receiveSurfacewaterFlowLimited(neighborReciprocalEdge[edge], -surfacewaterFlow[edge]);
                }
            }
        }
    }
  
  if (groundwaterAllCalculated && groundwaterOutwardNotLimited)
    {
      // Check if total outward flow is greater than water available.
      if ((groundwaterHead - elementZBedrock) * porosity * elementArea < groundwaterTotalOutwardFlow)
        {
          groundwaterOutwardFlowFraction = (groundwaterHead - elementZBedrock) * porosity * elementArea / groundwaterTotalOutwardFlow;
        }
      
      // Limit outward flows.
      for (edge = 1; edge <= 3; edge++)
        {
          if (0.0 < groundwaterFlow[edge])
            {
              groundwaterFlow[edge]     *= groundwaterOutwardFlowFraction;
              groundwaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(neighbor[edge]))
                {
                  thisProxy[neighbor[edge]].receiveGroundwaterFlowLimited(neighborReciprocalEdge[edge], -groundwaterFlow[edge]);
                }
            }
        }
    }
  
  // If the flow limiting check is done for all flows then we have the final value for all flows and can move water now.
  for (edge = 1; allLimited && edge <= 3; edge++)
    {
      allLimited = (FLOW_LIMITING_CHECK_DONE == surfacewaterFlowReady[edge] && FLOW_LIMITING_CHECK_DONE == groundwaterFlowReady[edge]);
    }
  
  if (allLimited)
    {
      moveWater();
    }
}

void MeshElement::moveWater()
{
  int    edge;   // Loop counter.
  double dtTemp; // Temporary variable for suggesting new timestep.
  
  // Calculate new values of surfacewaterDepth and groundwaterHead.
  for (edge = 1; edge <= 3; edge++)
    {
      surfacewaterDepth -= surfacewaterFlow[edge] /  elementArea;
      groundwaterHead   -= groundwaterFlow[edge]  / (elementArea * porosity);
    }
  
  // If groundwater rises above the surface put the extra water in surfacewater.
  if (groundwaterHead > elementZSurface)
    {
      surfacewaterDepth += (groundwaterHead - elementZSurface) * porosity;
      groundwaterHead    = elementZSurface;
    }
  
  // Even though we are limiting outward flows, surfacewaterDepth and groundwaterHead can go below their lower limits due to roundoff error.
  // FIXME this should be a real error if it is not within epsilon.
  // FIXME put the difference in an error accumulator.
  // FIXME should we try to take the water back later?
  if (0.0 > surfacewaterDepth)
    {
      surfacewaterDepth = 0.0;
    }
  
  if (groundwaterHead < elementZBedrock)
    {
      groundwaterHead = elementZBedrock;
    }
  
  // Suggest new timestep.
  if (groundwaterHead > elementZBedrock)
    {
      dtTemp = 0.2 * porosity * elementArea / (4.0 * conductivity * (groundwaterHead - elementZBedrock));

      if (dtNew > dtTemp)
        {
          dtNew = dtTemp;
        }
    }
  
  // FIXME remove
  printf("element %d: surfacewaterDepth = %lf, groundwaterHead = %lf\n", thisIndex, surfacewaterDepth, groundwaterHead);
  
  // Timestep done.  Perform min reduction on dtNew.  This also serves as a barrier at the end of the timestep.
  contribute(sizeof(double), &dtNew, CkReduction::min_double);
}

#include "mesh_element.def.h"
