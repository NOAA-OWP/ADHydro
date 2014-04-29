#include "mesh_element.h"
#include "adhydro.h"
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
  PUParray(p, neighborInitialized, 3);
  p | catchment;
  p | conductivity;
  p | porosity;
  p | manningsN;
  p | surfacewaterDepth;
  p | surfacewaterError;
  p | groundwaterHead;
  p | groundwaterError;
  p | groundwaterRecharge;
  PUParray(p, surfacewaterFlowRate, 3);
  PUParray(p, surfacewaterFlowRateReady, 3);
  PUParray(p, groundwaterFlowRate, 3);
  PUParray(p, groundwaterFlowRateReady, 3);
  p | iteration;
  p | timestepDone;
  p | dt;
  p | dtNew;
}

void MeshElement::receiveInitialize(double vertexXInit[3], double vertexYInit[3], double vertexZSurfaceInit[3], double vertexZBedrockInit[3],
                                    int neighborInit[3], InteractionEnum interactionInit[3], int catchmentInit, double conductivityInit, double porosityInit,
                                    double manningsNInit, double surfacewaterDepthInit, double surfacewaterErrorInit, double groundwaterHeadInit,
                                    double groundwaterErrorInit, double groundwaterRechargeInit)
{
  bool error = false; // Error flag.
  int  edge;          // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  for (edge = 0; edge < 3; edge++)
    {
      if (!(vertexZSurfaceInit[edge] >= vertexZBedrockInit[edge]))
        {
          CkError("ERROR: vertexZSurfaceInit must be greater than or equal to vertexZBedrockInit.\n");
          error = true;
        }
      
      if (!((0 <= neighborInit[edge] && neighborInit[edge] < ADHydro::meshProxySize) ||
            isBoundary(neighborInit[edge])))
        {
          CkError("ERROR: neighborInit must be a valid meshProxy array index or boundary condition code.\n");
          error = true;
        }
      
      if (!((I_CALCULATE_FLOW_RATE    == interactionInit[edge] || NEIGHBOR_CALCULATES_FLOW_RATE == interactionInit[edge] ||
             BOTH_CALCULATE_FLOW_RATE == interactionInit[edge]) || isBoundary(neighborInit[edge])))
        {
          CkError("ERROR: interactionInit must be a valid enum value if neighborInit is not a boundary condition code.\n");
          error = true;
        }
    }
  
  if (!(0.0 < conductivityInit))
    {
      CkError("ERROR: conductivityInit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < porosityInit))
    {
      CkError("ERROR: porosityInit must be greater than zero.\n");
      error = true;
    }

  if (!(0.0 < manningsNInit))
    {
      CkError("ERROR: manningsNInit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepthInit))
    {
      CkError("ERROR: surfacewaterDepthInit must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      for (edge = 0; edge < 3; edge++)
        {
          // Store passed parameter values.
          vertexX[edge]                = vertexXInit[edge];
          vertexY[edge]                = vertexYInit[edge];
          vertexZSurface[edge]         = vertexZSurfaceInit[edge];
          vertexZBedrock[edge]         = vertexZBedrockInit[edge];
          neighbor[edge]               = neighborInit[edge];
          interaction[edge]            = interactionInit[edge];
          
          // Initialize flow rates to zero.  Not strictly necessary for
          // surfacewater, but groundwaterFlowRate from the previous timestep
          // is used in infiltration before it is calculated.
          surfacewaterFlowRate[edge]      = 0.0;
          surfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
          groundwaterFlowRate[edge]       = 0.0;
          groundwaterFlowRateReady[edge]  = FLOW_RATE_LIMITING_CHECK_DONE;
        }

      // Store passed parameter values.
      catchment           = catchmentInit;
      conductivity        = conductivityInit;
      porosity            = porosityInit;
      manningsN           = manningsNInit;
      surfacewaterDepth   = surfacewaterDepthInit;
      surfacewaterError   = surfacewaterErrorInit;
      groundwaterHead     = groundwaterHeadInit;
      groundwaterError    = groundwaterErrorInit;
      groundwaterRecharge = groundwaterRechargeInit;

      // Calculate derived values.

      // Length of each edge.
      // Edge 0 goes from vertex 1 to 2 (opposite vertex 0).
      // Edge 1 goes form vertex 2 to 0 (opposite vertex 1).
      // Edge 2 goes form vertex 0 to 1 (opposite vertex 2).
      edgeLength[0] = sqrt((vertexX[1] - vertexX[2]) * (vertexX[1] - vertexX[2]) + (vertexY[1] - vertexY[2]) * (vertexY[1] - vertexY[2]));
      edgeLength[1] = sqrt((vertexX[2] - vertexX[0]) * (vertexX[2] - vertexX[0]) + (vertexY[2] - vertexY[0]) * (vertexY[2] - vertexY[0]));
      edgeLength[2] = sqrt((vertexX[0] - vertexX[1]) * (vertexX[0] - vertexX[1]) + (vertexY[0] - vertexY[1]) * (vertexY[0] - vertexY[1]));
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      for (edge = 0; edge < 3; edge++)
        {
          if (!(0.0 < edgeLength[edge]))
            {
              CkError("ERROR: edgeLength must be greater than zero.\n");
              error = true;
            }
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
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
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      if (!(elementZBedrock <= groundwaterHeadInit && groundwaterHeadInit <= elementZSurface))
        {
          CkError("ERROR: groundwaterHeadInit must be between elementZBedrock and elementZSurface.\n");
          error = true;
        }
      
      if (!(0.0 < elementArea))
        {
          CkError("ERROR: elementArea must be greater than zero.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    }
  
  if (!error)
    {
      // Send derived values to neighbors.
      for (edge = 0; edge < 3; edge++)
        {
          if (isBoundary(neighbor[edge]))
            {
              neighborInitialized[edge] = true;
            }
          else
            {
              neighborInitialized[edge] = false;
              
              thisProxy[neighbor[edge]].sendInitializeNeighbor(thisIndex, edge, elementX, elementY, elementZSurface, elementZBedrock, conductivity, manningsN);
            }
        }
    }
  else
    {
      CkExit();
    }
}

void MeshElement::receiveInitializeNeighbor(int neighborIndex, int neighborEdge, double neighborElementX, double neighborElementY,
                                            double neighborElementZSurface, double neighborElementZBedrock, double neighborElementConductivity,
                                            double neighborElementManningsN)
{
  bool error = false; // Error flag.
  int  edge  = 0;     // My edge number for our shared edge.
  
  while (neighbor[edge] != neighborIndex && edge < 2)
    {
      edge++;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= neighborIndex && neighborIndex < ADHydro::meshProxySize))
    {
      CkError("ERROR: neighborIndex must be a valid meshProxy array index.\n");
      error = true;
    }

  if (!(neighbor[edge] == neighborIndex))
    {
      CkError("ERROR: Received an initialize neighbor message from an element that is not my neighbor.\n");
      error = true;
    }

  if (!(0 <= neighborEdge && 3 > neighborEdge))
    {
      CkError("ERROR: neighborEdge must be zero, one, or two.\n");
      error = true;
    }

  if (!(neighborElementZSurface >= neighborElementZBedrock))
    {
      CkError("ERROR: neighborElementZSurface must be greater than or equal to neighborElementZBedrock.\n");
      error = true;
    }

  if (!(0.0 < neighborElementConductivity))
    {
      CkError("ERROR: neighborElementConductivity must be greater than zero.\n");
      error = true;
    }

  if (!(0.0 < neighborElementManningsN))
    {
      CkError("ERROR: neighborElementManningsN must be greater than zero.\n");
      error = true;
    }

  if (error)
    {
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  neighborReciprocalEdge[edge] = neighborEdge;
  neighborX[edge]              = neighborElementX;
  neighborY[edge]              = neighborElementY;
  neighborZSurface[edge]       = neighborElementZSurface;
  neighborZBedrock[edge]       = neighborElementZBedrock;
  neighborConductivity[edge]   = neighborElementConductivity;
  neighborManningsN[edge]      = neighborElementManningsN;
  neighborInitialized[edge]    = true;
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
  
  // Move groundwater including groundwater recharge from infiltration.
  moveGroundwater();

  // Set the flow rate states to not ready for this timestep.
  for (edge = 0; edge < 3; edge++)
    {
      surfacewaterFlowRateReady[edge] = FLOW_RATE_NOT_READY;
      groundwaterFlowRateReady[edge]  = FLOW_RATE_NOT_READY;
    }

  // Start surfacewater, groundwater, and channels algorithm.
  sendState();

  // FIXME Should we make calculateBoundaryConditionFlow a low priority message so that other state messages can go out without waiting for that computation?
  // FIXME Is it enough computation to make it worth doing that?  Try it both ways to find out.
  //calculateBoundaryConditionFlow();
  thisProxy[thisIndex].sendCalculateBoundaryConditionFlowRate(iteration);
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
  
  // Subtract that amount from surfacewaterDepth and add it to groundwaterRecharge.
  surfacewaterDepth   -= infiltrationAmount;
  groundwaterRecharge += infiltrationAmount;
}

void MeshElement::moveGroundwater()
{
  int    edge;   // Loop counter.
  double dtTemp; // Temporary variable for suggesting new timestep.
  
  // Put groundwaterRecharge in groundwater.
  groundwaterHead    += groundwaterRecharge / porosity;
  groundwaterRecharge = 0.0;
  
  // Move water for groundwater flows.
  for (edge = 0; edge < 3; edge++)
    {
      groundwaterHead -= (groundwaterFlowRate[edge] * dt)  / (elementArea * porosity);
    }
  
  // If groundwater rises above the surface put the extra water in surfacewater.
  if (groundwaterHead > elementZSurface)
    {
      surfacewaterDepth += (groundwaterHead - elementZSurface) * porosity;
      groundwaterHead    = elementZSurface;
    }
  // If groundwater falls below bedrock set it to bedrock and record the water error
  else if (groundwaterHead < elementZBedrock)
    {
      groundwaterError += (elementZBedrock - groundwaterHead) * porosity;
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
          // case I_CALCULATE_FLOW_RATE:
            // Do nothing.  I will calculate the flow after receiving a state message from my neighbor.
            // break;
          case NEIGHBOR_CALCULATES_FLOW_RATE:
            // Fallthrough.
          case BOTH_CALCULATE_FLOW_RATE:
            // Send state message.
            thisProxy[neighbor[edge]].sendState(iteration, neighborReciprocalEdge[edge], surfacewaterDepth, groundwaterHead);
            break;
          }
        }
    }
}

void MeshElement::receiveCalculateBoundaryConditionFlowRate()
{
  bool error = false; // Error flag.
  int  edge;          // Loop counter.

  for (edge = 0; !error && edge < 3; edge++)
    {
      if (isBoundary(neighbor[edge]))
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(FLOW_RATE_NOT_READY == surfacewaterFlowRateReady[edge] && FLOW_RATE_NOT_READY == groundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          // Calculate surfacewater flow rate.
          // FIXME figure out what to do about inflow boundary velocity and height
          error = surfacewaterBoundaryFlowRate(&surfacewaterFlowRate[edge], (BoundaryConditionEnum)neighbor[edge], 0.0, 0.0, 0.0, edgeLength[edge],
                                               edgeNormalX[edge], edgeNormalY[edge], surfacewaterDepth);
          
          if (!error)
            {
              // Outflows may need to be limited.  Inflows and noflows will not.
              if (0.0 < surfacewaterFlowRate[edge])
                {
                  surfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  surfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
                }

              // Calculate groundwater flow rate.
              error = groundwaterBoundaryFlowRate(&groundwaterFlowRate[edge], (BoundaryConditionEnum)neighbor[edge], vertexX, vertexY, vertexZSurface,
                                                  edgeLength[edge], edgeNormalX[edge], edgeNormalY[edge], elementZBedrock, elementArea, conductivity,
                                                  groundwaterHead);
              
              // Groundwater flows are not limited.
              groundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
            }
        }
    }

  if (!error)
    {
      checkAllFlowRates();
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
      if (FLOW_RATE_NOT_READY == surfacewaterFlowRateReady[edge])
        {
          // Calculate surfacewater flow rate.
          error = surfacewaterElementNeighborFlowRate(&surfacewaterFlowRate[edge], &dtNew, edgeLength[edge], elementX, elementY, elementZSurface, elementArea,
                                                      manningsN, surfacewaterDepth, neighborX[edge], neighborY[edge], neighborZSurface[edge],
                                                      neighborManningsN[edge], neighborSurfacewaterDepth);

          if (!error)
            {
              // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
              if (0.0 != surfacewaterFlowRate[edge])
                {
                  surfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  surfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      else
        {
          CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == surfacewaterFlowRateReady[edge]);
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  if (!error)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(FLOW_RATE_NOT_READY == groundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // Calculate groundwater flow rate.
      error = groundwaterElementNeighborFlowRate(&groundwaterFlowRate[edge], edgeLength[edge], elementX, elementY, elementZSurface, elementZBedrock,
                                                 conductivity, surfacewaterDepth, groundwaterHead, neighborX[edge], neighborY[edge],
                                                 neighborZSurface[edge], neighborZBedrock[edge], neighborConductivity[edge], neighborSurfacewaterDepth,
                                                 neighborGroundwaterHead);

      // Groundwater flows are not limited.
      groundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
    }

  if (!error)
    {
      switch (interaction[edge])
      {
      case I_CALCULATE_FLOW_RATE:
        // Send flow message.
        thisProxy[neighbor[edge]].sendFlowRate(iteration, neighborReciprocalEdge[edge], -surfacewaterFlowRate[edge], -groundwaterFlowRate[edge]);
        break;
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      case NEIGHBOR_CALCULATES_FLOW_RATE:
        // I should never receive a state message with the NEIGHBOR_CALCULATES_FLOW interaction.
        CkAssert(false);
        break;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        // case BOTH_CALCULATE_FLOW_RATE:
          // Do nothing.  My neighbor will calculate the same flow values.
          // break;
      }

      checkAllFlowRates();
    }
  else
    {
      CkExit();
    }
}

void MeshElement::receiveFlowRate(int edge, double edgeSurfacewaterFlowRate, double edgeGroundwaterFlowRate)
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
  CkAssert(NEIGHBOR_CALCULATES_FLOW_RATE == interaction[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // There is a race condition where a flow limited message can arrive before a flow message.
  // In that case, the flow limited message has the correct flow value and you can ignore the flow message.
  if (FLOW_RATE_NOT_READY == surfacewaterFlowRateReady[edge])
    {
      // Save received flow value.
      surfacewaterFlowRate[edge] = edgeSurfacewaterFlowRate;
      
      // Outflows may need to be limited.  Inflows may be limited by neighbor.  No flows will not.
      if (0.0 != surfacewaterFlowRate[edge])
        {
          surfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
        }
      else
        {
          surfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == surfacewaterFlowRateReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(FLOW_RATE_NOT_READY == groundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // Save received flow value.
  groundwaterFlowRate[edge] = edgeGroundwaterFlowRate;
  
  // Groundwater flows are not limited.
  groundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
  
  checkAllFlowRates();
}

void MeshElement::receiveSurfacewaterFlowRateLimited(int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  bool error = false; // Error flag.
  
  if (!(0 <= edge && 3 > edge))
    {
      CkError("ERROR: edge must be zero, one, or two.\n");
      error = true;
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeSurfacewaterFlowRate &&
             (FLOW_RATE_NOT_READY == surfacewaterFlowRateReady[edge] || edgeSurfacewaterFlowRate >= surfacewaterFlowRate[edge])))
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
  CkAssert(FLOW_RATE_NOT_READY == surfacewaterFlowRateReady[edge] || FLOW_RATE_CALCULATED == surfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // Save the received flow value.  The flow limiting check was done by neighbor.
  surfacewaterFlowRate[edge]      = edgeSurfacewaterFlowRate;
  surfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
  
  checkAllFlowRates();
}

void MeshElement::checkAllFlowRates()
{
  int    edge;                                        // Loop counter.
  bool   surfacewaterAllCalculated           = true;  // Whether all surfacewater flows have been calculated.
  bool   surfacewaterOutwardNotLimited       = false; // Whether at least one surfacewater outward flow is not limited.
  double surfacewaterTotalOutwardFlowRate    = 0.0;   // Sum of all surfacewater outward flows.
  double surfacewaterOutwardFlowRateFraction = 1.0;   // Fraction of all surfacewater outward flows that can be satisfied.
  bool   allLimited                          = true;  // Whether all flow limiting checks have been done.
  
  // For surfacewater, we need to limit outward flows if all flows are at least calculated (not FLOW_NOT_READY) and there is at least one outward flow that is
  // not limited.  We are not currently doing outward flow limiting checks for groundwater and may never do them.  The primary reason is that groundwater
  // movement and groundwater recharge from infiltration must be done together.  To do this we calculate the groundwater rate this timestep and use that rate
  // to move groundwater in the infiltration step next timestep.  This creates problems for limiting groundwater outward flows without another round of
  // messages.  Another reason is that groundwater moves so slowly that we are very unlikely to overshoot and move more groundwater than an element has.
  for (edge = 0; edge < 3; edge++)
    {
      if (FLOW_RATE_NOT_READY == surfacewaterFlowRateReady[edge])
        {
          surfacewaterAllCalculated = false;
        }
      else if (FLOW_RATE_CALCULATED == surfacewaterFlowRateReady[edge] && 0.0 < surfacewaterFlowRate[edge])
        {
          surfacewaterOutwardNotLimited     = true;
          surfacewaterTotalOutwardFlowRate += surfacewaterFlowRate[edge];
        }
    }
  
  if (surfacewaterAllCalculated && surfacewaterOutwardNotLimited)
    {
      // Check if total outward flow is greater than water available.
      if (surfacewaterDepth * elementArea < surfacewaterTotalOutwardFlowRate * dt)
        {
          surfacewaterOutwardFlowRateFraction = (surfacewaterDepth * elementArea) / (surfacewaterTotalOutwardFlowRate * dt);
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 <= surfacewaterOutwardFlowRateFraction && 1.0 >= surfacewaterOutwardFlowRateFraction);
      // FIXME Our code doesn't assure this.
      //CkAssert(epsilonEqual(1.0, surfacewaterOutwardFlowRateFraction));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // Limit outward flows.
      for (edge = 0; edge < 3; edge++)
        {
          if (0.0 < surfacewaterFlowRate[edge])
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(FLOW_RATE_CALCULATED == surfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              surfacewaterFlowRate[edge]     *= surfacewaterOutwardFlowRateFraction;
              surfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(neighbor[edge]))
                {
                  thisProxy[neighbor[edge]].sendSurfacewaterFlowRateLimited(iteration, neighborReciprocalEdge[edge], -surfacewaterFlowRate[edge]);
                }
            }
        }
    }
  
  // If the flow rate limiting check is done for all flow rates then we have the final value for all flow rates and can move surfacewater now.
  for (edge = 0; allLimited && edge < 3; edge++)
    {
      allLimited = (FLOW_RATE_LIMITING_CHECK_DONE == surfacewaterFlowRateReady[edge] && FLOW_RATE_LIMITING_CHECK_DONE == groundwaterFlowRateReady[edge]);
    }
  
  if (allLimited)
    {
      moveSurfacewater();
    }
}

void MeshElement::moveSurfacewater()
{
  int edge; // Loop counter.
  
  // Calculate new value of surfacewaterDepth.
  for (edge = 0; edge < 3; edge++)
    {
      surfacewaterDepth -= (surfacewaterFlowRate[edge] * dt) / elementArea;
    }
  
  // Even though we are limiting outward flows, surfacewaterDepth can go below zero due to roundoff error.
  // FIXME should we try to take the water back from the error accumulator later?
  if (0.0 > surfacewaterDepth)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonEqual(0.0, surfacewaterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      surfacewaterError -= surfacewaterDepth;
      surfacewaterDepth  = 0.0;
    }
  
  // Timestep done.
  timestepDone = true;
  
  // Perform min reduction on dtNew.  This also serves as a barrier at the end of the timestep.
  contribute(sizeof(double), &dtNew, CkReduction::min_double);
  
  // FIXME remove
  CkPrintf("element %d, pe = %d, surfacewaterDepth = %lf, groundwaterHead = %lf\n", thisIndex, CkMyPe(), surfacewaterDepth, groundwaterHead);
}

#include "mesh_element.def.h"
