#include "mesh_element.h"
#include "surfacewater.h"
#include "groundwater.h"

// FIXME questions and to-do list items
// How to make it send the high priority messages out first?  We want all the messages going to other nodes to go out as soon as possible.
// Will it send out one MPI message per mesh edge rather than all of the ghost node information in a single message?
// As elements migrate to different nodes update interaction in a way that guarantees both sides agree on the interaction.
// When you implement groundwater/channel interaction include surfacewater depth in groundwater head.
// Scale channel dx w/ bankfull depth?

MeshElement::MeshElement()
{
  // Do nothing.  Initialization of member variables is handled in receiveInitialize.
}

MeshElement::MeshElement(CkMigrateMessage* msg)
{
  // Do nothing.
}

void MeshElement::pup(PUP::er &p)
{
  CBase_MeshElement::pup(p);
  __sdag_pup(p);
  PUParray(p, vertexX, 3);
  PUParray(p, vertexY, 3);
  PUParray(p, vertexZSurface, 3);
  PUParray(p, vertexZBedrock, 3);
  PUParray(p, edgeLength, 3);
  PUParray(p, edgeNormalX, 3);
  PUParray(p, edgeNormalY, 3);
  p | elementX;
  p | elementY;
  p | elementZSurface;
  p | elementZBedrock;
  p | elementArea;
  PUParray(p, neighbor, 3);
  PUParray(p, neighborReciprocalEdge, 3);
  PUParray(p, interaction, 3);
  PUParray(p, neighborX, 3);
  PUParray(p, neighborY, 3);
  PUParray(p, neighborZSurface, 3);
  PUParray(p, neighborZBedrock, 3);
  PUParray(p, neighborConductivity, 3);
  PUParray(p, neighborManningsN, 3);
  p | catchment;
  p | conductivity;
  p | porosity;
  p | manningsN;
  p | surfacewaterDepth;
  p | surfacewaterError;
  p | groundwaterHead;
  p | groundwaterError;
  PUParray(p, surfacewaterFlow, 3);
  PUParray(p, surfacewaterFlowReady, 3);
  PUParray(p, groundwaterFlow, 3);
  PUParray(p, groundwaterFlowReady, 3);
  p | iteration;
  p | timestepDone;
  p | dt;
  p | dtNew;
}

void MeshElement::receiveInitialize(MeshElementInitStruct& initialValues)
{
  bool error = false; // Error flag.
  int  edge;          // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  for (edge = 0; edge < 3; edge++)
    {
      if (!(initialValues.vertexZSurface[edge] >= initialValues.vertexZBedrock[edge]))
        {
          CkError("ERROR: vertexZSurface must be greater than or equal to vertexZBedrock.\n");
          error = true;
        }
      
      if (!((0 <= initialValues.neighbor[edge] /* FIXME && initialValues.neighbor[edge] < ADHydro::meshProxySize */ ) ||
            isBoundary(initialValues.neighbor[edge])))
        {
          CkError("ERROR: neighbor must be a valid meshProxy array index or boundary condition code.\n");
          error = true;
        }
      
      if (!((0 <= initialValues.neighborReciprocalEdge[edge] && 3 > initialValues.neighborReciprocalEdge[edge]) || isBoundary(initialValues.neighbor[edge])))
        {
          CkError("ERROR: neighborReciprocalEdge must be zero, one, or two if neighbor is not a boundary condition code.\n");
          error = true;
        }
      
      if (!((I_CALCULATE_FLOW    == initialValues.interaction[edge] || NEIGHBOR_CALCULATES_FLOW == initialValues.interaction[edge] ||
             BOTH_CALCULATE_FLOW == initialValues.interaction[edge]) || isBoundary(initialValues.neighbor[edge])))
        {
          CkError("ERROR: interaction must be a valid enum value if neighbor is not a boundary condition code.\n");
          error = true;
        }
    }
  
  if (!(0.0 < initialValues.conductivity))
    {
      CkError("ERROR: conductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < initialValues.porosity))
    {
      CkError("ERROR: porosity must be greater than zero.\n");
      error = true;
    }

  if (!(0.0 < initialValues.manningsN))
    {
      CkError("ERROR: manningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= initialValues.surfacewaterDepth))
    {
      CkError("ERROR: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Store passed parameter values.
      for (edge = 0; edge < 3; edge++)
        {
          vertexX[edge]                = initialValues.vertexX[edge];
          vertexY[edge]                = initialValues.vertexY[edge];
          vertexZSurface[edge]         = initialValues.vertexZSurface[edge];
          vertexZBedrock[edge]         = initialValues.vertexZBedrock[edge];
          neighbor[edge]               = initialValues.neighbor[edge];
          neighborReciprocalEdge[edge] = initialValues.neighborReciprocalEdge[edge];
          interaction[edge]            = initialValues.interaction[edge];
        }

      catchment         = initialValues.catchment;
      conductivity      = initialValues.conductivity;
      porosity          = initialValues.porosity;
      manningsN         = initialValues.manningsN;
      surfacewaterDepth = initialValues.surfacewaterDepth;
      surfacewaterError = initialValues.surfacewaterError;
      groundwaterHead   = initialValues.groundwaterHead;
      groundwaterError  = initialValues.groundwaterError;

      // Calculate derived values.

      // Length of each edge.
      // Edge 0 goes from vertex 1 to 2 (opposite vertex 0).
      // Edge 1 goes form vertex 2 to 0 (opposite vertex 1).
      // Edge 2 goes form vertex 0 to 1 (opposite vertex 2).
      edgeLength[0] = sqrt((vertexX[1] - vertexX[2]) * (vertexX[1] - vertexX[2]) + (vertexY[1] - vertexY[2]) * (vertexY[1] - vertexY[2]));
      edgeLength[1] = sqrt((vertexX[2] - vertexX[0]) * (vertexX[2] - vertexX[0]) + (vertexY[2] - vertexY[0]) * (vertexY[2] - vertexY[0]));
      edgeLength[2] = sqrt((vertexX[0] - vertexX[1]) * (vertexX[0] - vertexX[1]) + (vertexY[0] - vertexY[1]) * (vertexY[0] - vertexY[1]));
      
      for (edge = 0; edge < 3; edge++)
        {
          if (!(0.0 < edgeLength[edge]))
            {
              CkError("ERROR: edgeLength must be greater than zero.\n");
              error = true;
            }
        }
    }

  if (!error)
    {
      // Unit normal vector of each edge.
      edgeNormalX[0] = (vertexY[2] - vertexY[1]) / edgeLength[0];
      edgeNormalY[0] = (vertexX[1] - vertexX[2]) / edgeLength[0];
      edgeNormalX[1] = (vertexY[0] - vertexY[2]) / edgeLength[1];
      edgeNormalY[1] = (vertexX[2] - vertexX[0]) / edgeLength[1];
      edgeNormalX[2] = (vertexY[1] - vertexY[0]) / edgeLength[2];
      edgeNormalY[2] = (vertexX[0] - vertexX[1]) / edgeLength[2];

      // Geometric coordinates of element center or entire element.
      elementX        = (vertexX[0]        + vertexX[1]        + vertexX[2])        / 3.0;
      elementY        = (vertexY[0]        + vertexY[1]        + vertexY[2])        / 3.0;
      elementZSurface = (vertexZSurface[0] + vertexZSurface[1] + vertexZSurface[2]) / 3.0;
      elementZBedrock = (vertexZBedrock[0] + vertexZBedrock[1] + vertexZBedrock[2]) / 3.0;
      elementArea = (vertexX[0] * (vertexY[1] - vertexY[2]) + vertexX[1] * (vertexY[2] - vertexY[0]) + vertexX[2] * (vertexY[0] - vertexY[1])) * 0.5;
      
      if (!(elementZBedrock <= initialValues.groundwaterHead && initialValues.groundwaterHead <= elementZSurface))
        {
          CkError("ERROR: groundwaterHead must be between elementZBedrock and elementZSurface.\n");
          error = true;
        }
      
      if (!(0.0 < elementArea))
        {
          CkError("ERROR: elementArea must be greater than zero.\n");
          error = true;
        }
    }
  
  if (!error)
    {
      // Send calculated values to neighbors.
      // FIXME implement message
      if (0 == thisIndex)
        {
          neighborX[2]        = ( 0.0 + 100.0 + 100.0) / 3.0;
          neighborY[2]        = ( 0.0 +   0.0 + 100.0) / 3.0;
          neighborZSurface[2] = (10.0 +   0.0 +   0.0) / 3.0;
          neighborZBedrock[2] = ( 0.0 + -10.0 + -10.0) / 3.0;
          neighborConductivity[2] = 1.0e-3;
          neighborManningsN[2]    = 0.04;
        }
      else if (1 == thisIndex)
        {
          neighborX[1]        = ( 0.0 + 100.0 +   0.0) / 3.0;
          neighborY[1]        = ( 0.0 + 100.0 + 100.0) / 3.0;
          neighborZSurface[1] = (10.0 +   0.0 +  10.0) / 3.0;
          neighborZBedrock[1] = ( 0.0 + -10.0 +   0.0) / 3.0;
          neighborConductivity[1] = 1.0e-3;
          neighborManningsN[1]    = 0.04;
        }
      else
        {
          CkAssert(false);
        }
    }
  else
    {
      CkExit();
    }
}

void MeshElement::receiveDoTimestep(int iterationThisTimestep, double dtThisTimestep)
{
  int edge; // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtThisTimestep))
    {
      CkError("ERROR: dtThisTimestep must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // Save the new timestep information.
  iteration    = iterationThisTimestep;
  timestepDone = false;
  dt           = dtThisTimestep;
  dtNew        = 2.0 * dt;

  // Do point processes.
  doSnowMelt();
  doRainfall();
  doEvapoTranspiration();
  doInfiltration();

  // Set the flow states to not ready for this timestep.
  for (edge = 0; edge < 3; edge++)
    {
      surfacewaterFlowReady[edge] = FLOW_NOT_READY;
      groundwaterFlowReady[edge]  = FLOW_NOT_READY;
    }

  // Start surfacewater, groundwater, and channels algorithm.
  sendState();

  // FIXME Should we make calculateBoundaryConditionFlow a low priority message so that other state messages can go out without waiting for that computation?
  // FIXME Is it enough computation to make it worth doing that?  Try it both ways to find out.
  //calculateBoundaryConditionFlow();
  thisProxy[thisIndex].sendCalculateBoundaryConditionFlow(iteration);
}

void MeshElement::doSnowMelt()
{
  // FIXME implement
}

void MeshElement::doRainfall()
{
  // FIXME implement
}

void MeshElement::doEvapoTranspiration()
{
  // FIXME implement
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
  
  for (edge = 0; edge < 3; edge++)
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
            thisProxy[neighbor[edge]].sendState(iteration, neighborReciprocalEdge[edge], surfacewaterDepth, groundwaterHead);
            break;
          }
        }
    }
}

void MeshElement::receiveCalculateBoundaryConditionFlow()
{
  bool error = false; // Error flag.
  int  edge;          // Loop counter.

  for (edge = 0; !error && edge < 3; edge++)
    {
      if (isBoundary(neighbor[edge]))
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(FLOW_NOT_READY == surfacewaterFlowReady[edge] && FLOW_NOT_READY == groundwaterFlowReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          // Calculate surfacewater flow rate.
          // FIXME figure out what to do about inflow boundary velocity and height
          error = surfacewaterBoundaryFlowRate(&surfacewaterFlow[edge], (BoundaryConditionEnum)neighbor[edge], 1.0, 1.0, 1.0, edgeLength[edge],
                                               edgeNormalX[edge], edgeNormalY[edge], surfacewaterDepth);
          
          if (!error)
            {
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
              error = groundwaterBoundaryFlowRate(&groundwaterFlow[edge], (BoundaryConditionEnum)neighbor[edge], vertexX, vertexY, vertexZSurface,
                                                  edgeLength[edge], edgeNormalX[edge], edgeNormalY[edge], elementZBedrock, elementArea, conductivity,
                                                  groundwaterHead);
            }
          
          if (!error)
            {
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
    }

  if (!error)
    {
      checkAllFlows();
    }
  else
    {
      CkExit();
    }
}

void MeshElement::receiveState(int edge, double neighborSurfacewaterDepth, double neighborGroundwaterHead)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && 3 > edge))
    {
      CkError("ERROR: edge must be zero, one, or two.\n");
      error = true;
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(neighborZBedrock[edge] <= neighborGroundwaterHead && neighborGroundwaterHead <= neighborZSurface[edge]))
    {
      CkError("ERROR: neighborGroundwaterHead must be between neighborZBedrock and neighborZSurface.\n");
      error = true;
    }

  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR: neighborSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // There is a race condition where a flow limited message can arrive before a state message.
      // In that case, the flow limited message has the correct flow value and you can ignore the state message.
      if (FLOW_NOT_READY == surfacewaterFlowReady[edge])
        {
          // Calculate surfacewater flow rate.
          error = surfacewaterElementNeighborFlowRate(&surfacewaterFlow[edge], &dtNew, edgeLength[edge], elementX, elementY, elementZSurface, elementArea,
                                                      manningsN, surfacewaterDepth, neighborX[edge], neighborY[edge], neighborZSurface[edge],
                                                      neighborManningsN[edge], neighborSurfacewaterDepth);

          if (!error)
            {
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
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      else
        {
          CkAssert(FLOW_LIMITING_CHECK_DONE == surfacewaterFlowReady[edge]);
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  if (!error)
    {
      if (FLOW_NOT_READY == groundwaterFlowReady[edge])
        {
          // Calculate groundwater flow rate.
          error = groundwaterElementNeighborFlowRate(&groundwaterFlow[edge], edgeLength[edge], elementX, elementY, elementZSurface, elementZBedrock,
                                                     conductivity, surfacewaterDepth, groundwaterHead, neighborX[edge], neighborY[edge],
                                                     neighborZSurface[edge], neighborZBedrock[edge], neighborConductivity[edge], neighborSurfacewaterDepth,
                                                     neighborGroundwaterHead);

          if (!error)
            {
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
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      else
        {
          CkAssert(FLOW_LIMITING_CHECK_DONE == groundwaterFlowReady[edge]);
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }

  if (!error)
    {
      switch (interaction[edge])
      {
      case I_CALCULATE_FLOW:
        // Send flow message.
        thisProxy[neighbor[edge]].sendFlow(iteration, neighborReciprocalEdge[edge], -surfacewaterFlow[edge], -groundwaterFlow[edge]);
        break;
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      case NEIGHBOR_CALCULATES_FLOW:
        // I should never receive a state message with the NEIGHBOR_CALCULATES_FLOW interaction.
        CkAssert(false);
        break;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        // case BOTH_CALCULATE_FLOW:
          // Do nothing.  My neighbor will calculate the same flow values.
          // break;
      }

      checkAllFlows();
    }
  else
    {
      CkExit();
    }
}

void MeshElement::receiveFlow(int edge, double edgeSurfacewaterFlow, double edgeGroundwaterFlow)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && 3 > edge))
    {
      CkError("ERROR: edge must be zero, one, or two.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // I should only receive a flow message with the NEIGHBOR_CALCULATES_FLOW interaction.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(NEIGHBOR_CALCULATES_FLOW == interaction[edge]);
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
      CkAssert(FLOW_LIMITING_CHECK_DONE == surfacewaterFlowReady[edge]);
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
      CkAssert(FLOW_LIMITING_CHECK_DONE == groundwaterFlowReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  checkAllFlows();
}

void MeshElement::receiveSurfacewaterFlowLimited(int edge, double edgeSurfacewaterFlow)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  bool error = false; // Error flag.
  
  if (!(0 <= edge && 3 > edge))
    {
      CkError("ERROR: edge must be zero, one, or two.\n");
      error = true;
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeSurfacewaterFlow && (FLOW_NOT_READY == surfacewaterFlowReady[edge] || edgeSurfacewaterFlow >= surfacewaterFlow[edge])))
    {
      CkError("ERROR: A flow limiting message must be for an inflow and it must only reduce the magnitude of the flow.\n");
      error = true;
    }

  if (error)
    {
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(FLOW_NOT_READY == surfacewaterFlowReady[edge] || FLOW_CALCULATED == surfacewaterFlowReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // Save the received flow value.  The flow limiting check was done by neighbor.
  surfacewaterFlow[edge]      = edgeSurfacewaterFlow;
  surfacewaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
  
  checkAllFlows();
}

void MeshElement::receiveGroundwaterFlowLimited(int edge, double edgeGroundwaterFlow)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  bool error = false; // Error flag.
  
  if (!(0 <= edge && 3 > edge))
    {
      CkError("ERROR: edge must be zero, one, or two.\n");
      error = true;
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeGroundwaterFlow && (FLOW_NOT_READY == groundwaterFlowReady[edge] || edgeGroundwaterFlow >= groundwaterFlow[edge])))
    {
      CkError("ERROR: A flow limiting message must be for an inflow and it must only reduce the magnitude of the flow.\n");
      error = true;
    }

  if (error)
    {
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(FLOW_NOT_READY == groundwaterFlowReady[edge] || FLOW_CALCULATED == groundwaterFlowReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // Save the received flow value.  The flow limiting check was done by neighbor.
  groundwaterFlow[edge]      = edgeGroundwaterFlow;
  groundwaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
  
  checkAllFlows();
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
  for (edge = 0; edge < 3; edge++)
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
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 <= surfacewaterOutwardFlowFraction && 1.0 >= surfacewaterOutwardFlowFraction);
      // FIXME Our code doesn't assure this.
      //CkAssert(epsilonEqual(1.0, surfacewaterOutwardFlowFraction));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // Limit outward flows.
      for (edge = 0; edge < 3; edge++)
        {
          if (0.0 < surfacewaterFlow[edge])
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(FLOW_CALCULATED == surfacewaterFlowReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              surfacewaterFlow[edge]     *= surfacewaterOutwardFlowFraction;
              surfacewaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(neighbor[edge]))
                {
                  thisProxy[neighbor[edge]].sendSurfacewaterFlowLimited(iteration, neighborReciprocalEdge[edge], -surfacewaterFlow[edge]);
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
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 <= groundwaterOutwardFlowFraction && 1.0 >= groundwaterOutwardFlowFraction);
      // FIXME Our code doesn't assure this.
      //CkAssert(epsilonEqual(1.0, groundwaterOutwardFlowFraction));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // Limit outward flows.
      for (edge = 0; edge < 3; edge++)
        {
          if (0.0 < groundwaterFlow[edge])
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(FLOW_CALCULATED == groundwaterFlowReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              groundwaterFlow[edge]     *= groundwaterOutwardFlowFraction;
              groundwaterFlowReady[edge] = FLOW_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(neighbor[edge]))
                {
                  thisProxy[neighbor[edge]].sendGroundwaterFlowLimited(iteration, neighborReciprocalEdge[edge], -groundwaterFlow[edge]);
                }
            }
        }
    }
  
  // If the flow limiting check is done for all flows then we have the final value for all flows and can move water now.
  for (edge = 0; allLimited && edge < 3; edge++)
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
  for (edge = 0; edge < 3; edge++)
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
  // FIXME should we try to take the water back from the error accumulator later?
  if (0.0 > surfacewaterDepth)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonEqual(0.0, surfacewaterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      surfacewaterError -= surfacewaterDepth;
      surfacewaterDepth  = 0.0;
    }
  
  if (groundwaterHead < elementZBedrock)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonEqual(groundwaterHead, elementZBedrock));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      groundwaterError += elementZBedrock - groundwaterHead;
      groundwaterHead   = elementZBedrock;
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
  
  // Timestep done.
  timestepDone = true;
  
  // Perform min reduction on dtNew.  This also serves as a barrier at the end of the timestep.
  contribute(sizeof(double), &dtNew, CkReduction::min_double);
  
  // FIXME remove
  CkPrintf("element %d, pe = %d, surfacewaterDepth = %lf, groundwaterHead = %lf\n", thisIndex, CkMyPe(), surfacewaterDepth, groundwaterHead);
  migrateMe((CkMyPe() + 1) % CkNumPes());
}

#include "mesh_element.def.h"
