#include "channel_element.h"
#include "surfacewater.h"
#include "groundwater.h"

ChannelElement::ChannelElement()
{
  // Initialization will be done in runForever.
  thisProxy[thisIndex].runForever();
}

ChannelElement::ChannelElement(CkMigrateMessage* msg)
{
  // Do nothing.
}

void ChannelElement::pup(PUP::er &p)
{
  CBase_ChannelElement::pup(p);
  __sdag_pup(p);
  p | meshProxy;
  p | fileManagerProxy;
  p | elementX;
  p | elementY;
  p | elementZBank;
  p | elementZBed;
  p | elementLength;
  PUParray(p, channelNeighbors, channelNeighborsSize);
  PUParray(p, channelNeighborsReciprocalEdge, channelNeighborsSize);
  PUParray(p, channelNeighborsInteraction, channelNeighborsSize);
  PUParray(p, meshNeighbors, meshNeighborsSize);
  PUParray(p, meshNeighborsReciprocalEdge, meshNeighborsSize);
  PUParray(p, meshNeighborsInteraction, meshNeighborsSize);
  PUParray(p, channelNeighborsZBank, channelNeighborsSize);
  PUParray(p, channelNeighborsZBed, channelNeighborsSize);
  PUParray(p, channelNeighborsLength, channelNeighborsSize);
  PUParray(p, meshNeighborsZSurface, meshNeighborsSize);
  PUParray(p, meshNeighborsZBedrock, meshNeighborsSize);
  PUParray(p, meshNeighborsEdgeLength, meshNeighborsSize);
  PUParray(p, channelNeighborsChannelType, channelNeighborsSize);
  PUParray(p, channelNeighborsBaseWidth, channelNeighborsSize);
  PUParray(p, channelNeighborsSideSlope, channelNeighborsSize);
  PUParray(p, channelNeighborsManningsN, channelNeighborsSize);
  p | channelType;
  p | permanentCode;
  p | baseWidth;
  p | sideSlope;
  p | bedConductivity;
  p | bedThickness;
  p | manningsN;
  p | surfacewaterDepth;
  p | surfacewaterError;
  PUParray(p, channelNeighborsSurfacewaterFlowRateReady, channelNeighborsSize);
  PUParray(p, channelNeighborsSurfacewaterFlowRate, channelNeighborsSize);
  PUParray(p, channelNeighborsSurfacewaterCumulativeFlow, channelNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterFlowRateReady, meshNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterFlowRate, meshNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterCumulativeFlow, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterFlowRateReady, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterFlowRate, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterCumulativeFlow, meshNeighborsSize);
  p | groundwaterDone;
  p | surfacewaterDone;
  p | dt;
  p | dtNew;
}

void ChannelElement::handleInitialize(CProxy_MeshElement meshProxyInit, CProxy_FileManager fileManagerProxyInit, double elementXInit, double elementYInit,
                                      double elementZBankInit, double elementZBedInit, double elementLengthInit,
                                      int channelNeighborsInit[channelNeighborsSize], int channelNeighborsReciprocalEdgeInit[channelNeighborsSize],
                                      InteractionEnum channelNeighborsInteractionInit[channelNeighborsSize], int meshNeighborsInit[meshNeighborsSize],
                                      int meshNeighborsReciprocalEdgeInit[meshNeighborsSize], InteractionEnum meshNeighborsInteractionInit[meshNeighborsSize],
                                      double channelNeighborsZBankInit[channelNeighborsSize], double channelNeighborsZBedInit[channelNeighborsSize],
                                      double channelNeighborsLengthInit[channelNeighborsSize], double meshNeighborsZSurfaceInit[meshNeighborsSize],
                                      double meshNeighborsZBedrockInit[meshNeighborsSize], double meshNeighborsEdgeLengthInit[meshNeighborsSize],
                                      ChannelTypeEnum channelNeighborsChannelTypeInit[channelNeighborsSize],
                                      double channelNeighborsBaseWidthInit[channelNeighborsSize], double channelNeighborsSideSlopeInit[channelNeighborsSize],
                                      double channelNeighborsManningsNInit[channelNeighborsSize], ChannelTypeEnum channelTypeInit, int permanentCodeInit,
                                      double baseWidthInit, double sideSlopeInit, double bedConductivityInit, double bedThicknessInit, double manningsNInit,
                                      double surfacewaterDepthInit, double surfacewaterErrorInit)
{
  int edge; // Loop counter.
  
  // Parameter values will be error checked by the invariant.
  
  meshProxy        = meshProxyInit;
  fileManagerProxy = fileManagerProxyInit;
  elementX         = elementXInit;
  elementY         = elementYInit;
  elementZBank     = elementZBankInit;
  elementZBed      = elementZBedInit;
  elementLength    = elementLengthInit;
  
  for (edge = 0; edge < channelNeighborsSize; edge++)
    {
      channelNeighbors[edge]                           = channelNeighborsInit[edge];
      channelNeighborsReciprocalEdge[edge]             = channelNeighborsReciprocalEdgeInit[edge];
      channelNeighborsInteraction[edge]                = channelNeighborsInteractionInit[edge];
      channelNeighborsZBank[edge]                      = channelNeighborsZBankInit[edge];
      channelNeighborsZBed[edge]                       = channelNeighborsZBedInit[edge];
      channelNeighborsLength[edge]                     = channelNeighborsLengthInit[edge];
      channelNeighborsChannelType[edge]                = channelNeighborsChannelTypeInit[edge];
      channelNeighborsBaseWidth[edge]                  = channelNeighborsBaseWidthInit[edge];
      channelNeighborsSideSlope[edge]                  = channelNeighborsSideSlopeInit[edge];
      channelNeighborsManningsN[edge]                  = channelNeighborsManningsNInit[edge];
      channelNeighborsSurfacewaterFlowRateReady[edge]  = FLOW_RATE_NOT_READY;
      channelNeighborsSurfacewaterFlowRate[edge]       = 0.0;
      channelNeighborsSurfacewaterCumulativeFlow[edge] = 0.0;
    }
  
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
      meshNeighbors[edge]                           = meshNeighborsInit[edge];
      meshNeighborsReciprocalEdge[edge]             = meshNeighborsReciprocalEdgeInit[edge];
      meshNeighborsInteraction[edge]                = meshNeighborsInteractionInit[edge];
      meshNeighborsZSurface[edge]                   = meshNeighborsZSurfaceInit[edge];
      meshNeighborsZBedrock[edge]                   = meshNeighborsZBedrockInit[edge];
      meshNeighborsEdgeLength[edge]                 = meshNeighborsEdgeLengthInit[edge];
      meshNeighborsSurfacewaterFlowRateReady[edge]  = FLOW_RATE_NOT_READY;
      meshNeighborsSurfacewaterFlowRate[edge]       = 0.0;
      meshNeighborsSurfacewaterCumulativeFlow[edge] = 0.0;
      meshNeighborsGroundwaterFlowRateReady[edge]   = FLOW_RATE_NOT_READY;
      meshNeighborsGroundwaterFlowRate[edge]        = 0.0;
      meshNeighborsGroundwaterCumulativeFlow[edge]  = 0.0;
    }
  
  channelType       = channelTypeInit;
  permanentCode     = permanentCodeInit;
  baseWidth         = baseWidthInit;
  sideSlope         = sideSlopeInit;
  bedConductivity   = bedConductivityInit;
  bedThickness      = bedThicknessInit;
  manningsN         = manningsNInit;
  surfacewaterDepth = surfacewaterDepthInit;
  surfacewaterError = surfacewaterErrorInit;
  groundwaterDone   = false;
  surfacewaterDone  = false;
  dt                = 0.0;
  dtNew             = 2.0 * dt;
}

// Suppress warning enum value not handled in switch.
#pragma GCC diagnostic ignored "-Wswitch"
void ChannelElement::handleDoTimestep(CMK_REFNUM_TYPE iterationThisTimestep, double dtThisTimestep)
{
  int edge; // Loop counter.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtThisTimestep))
    {
      CkError("ERROR in ChannelElement::handleDoTimestep, element %d: dtThisTimestep must be greater than zero.\n", thisIndex);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // Initialize sequencing and timestep information.
  groundwaterDone  = false;
  surfacewaterDone = false;
  dt               = dtThisTimestep;
  dtNew            = 2.0 * dt;

  // Do point processes for snowmelt, rainfall, and evapo-transpiration.
  // FIXME implement these on channels
  
  // Initiate groundwater phase.
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
      if (!isBoundary(meshNeighbors[edge]))
        {
          // Set the flow rate state for this timestep to not ready.
          meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_NOT_READY;
          
          // Send my state to my neighbor.
          switch (meshNeighborsInteraction[edge])
          {
          // case I_CALCULATE_FLOW_RATE:
            // Do nothing.  I will calculate the flow rate after receiving a state message from my neighbor.
            // break;
          case NEIGHBOR_CALCULATES_FLOW_RATE:
            // Fallthrough.
          case BOTH_CALCULATE_FLOW_RATE:
            // Send state message.
            meshProxy[meshNeighbors[edge]].channelGroundwaterStateMessage(iterationThisTimestep, meshNeighborsReciprocalEdge[edge], surfacewaterDepth);
            break;
          }
        }
      else
        {
          // Only NOFLOW is a valid channel mesh neighbor boundary condition code.
          meshNeighborsGroundwaterFlowRate[edge]      = 0.0;
          meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
        }
    }
}

void ChannelElement::handleMeshGroundwaterStateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double neighborSurfacewaterDepth,
                                                       double neighborGroundwaterHead)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleMeshGroundwaterStateMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      error = true;
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(meshNeighborsZSurface[edge] >= neighborGroundwaterHead))
    {
      CkError("ERROR in ChannelElement::handleMeshGroundwaterStateMessage, element %d, edge %d: "
              "neighborGroundwaterHead must be less than or equal to neighborsZSurface.\n", thisIndex, edge);
      error = true;
    }

  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::handleMeshGroundwaterStateMessage, element %d, edge %d: "
              "neighborSurfacewaterDepth must be greater than or equal to zero.\n", thisIndex, edge);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // There is a race condition where a flow limited message can arrive before a state message.
      // In that case, the flow limited message already had the correct flow value and you can ignore the state message.
      if (FLOW_RATE_NOT_READY == meshNeighborsGroundwaterFlowRateReady[edge])
        {
          // Calculate groundwater flow rate.
          error = groundwaterMeshChannelFlowRate(&meshNeighborsGroundwaterFlowRate[edge], meshNeighborsEdgeLength[edge], meshNeighborsZSurface[edge],
                                                  meshNeighborsZBedrock[edge], neighborSurfacewaterDepth, neighborGroundwaterHead, elementZBank, elementZBed,
                                                  baseWidth, sideSlope, bedConductivity, bedThickness, surfacewaterDepth);
          
          // Use negative of value so that positive means flow out of the channel.
          meshNeighborsGroundwaterFlowRate[edge] *= -1.0;
          
          if (!error)
            {
              // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
              if (0.0 != meshNeighborsGroundwaterFlowRate[edge])
                {
                  meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      else
        {
          CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsGroundwaterFlowRateReady[edge]);
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  if (!error)
    {
      switch (meshNeighborsInteraction[edge])
      {
      case I_CALCULATE_FLOW_RATE:
        // Send flow message.
        meshProxy[meshNeighbors[edge]].channelGroundwaterFlowRateMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge],
                                                                         -meshNeighborsGroundwaterFlowRate[edge]);
        break;
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      case NEIGHBOR_CALCULATES_FLOW_RATE:
        // I should never receive a state message with the NEIGHBOR_CALCULATES_FLOW interaction.
        CkAssert(false);
        break;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        // case BOTH_CALCULATE_FLOW_RATE:
          // Do nothing.  My neighbor will calculate the same flow value.
          // break;
      }

      checkGroundwaterFlowRates(iterationThisMessage);
    }
  else
    {
      CkExit();
    }
}
#pragma GCC diagnostic warning "-Wswitch"

void ChannelElement::handleMeshGroundwaterFlowRateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeGroundwaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleMeshGroundwaterFlowRateMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // I should only receive a flow rate message with the NEIGHBOR_CALCULATES_FLOW interaction.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(NEIGHBOR_CALCULATES_FLOW_RATE == meshNeighborsInteraction[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // There is a race condition where a flow limited message can arrive before a flow rate message.
  // In that case, the flow limited message already had the correct flow value and you can ignore the flow rate message.
  if (FLOW_RATE_NOT_READY == meshNeighborsGroundwaterFlowRateReady[edge])
    {
      // Save the received flow rate value.
      meshNeighborsGroundwaterFlowRate[edge] = edgeGroundwaterFlowRate;

      // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
      if (0.0 != meshNeighborsGroundwaterFlowRate[edge])
        {
          meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
        }
      else
        {
          meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsGroundwaterFlowRateReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  checkGroundwaterFlowRates(iterationThisMessage);
}

void ChannelElement::handleMeshGroundwaterFlowRateLimitedMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeGroundwaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleMeshGroundwaterFlowRateLimitedMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      CkExit();
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeGroundwaterFlowRate &&
             (FLOW_RATE_NOT_READY == meshNeighborsGroundwaterFlowRateReady[edge] || edgeGroundwaterFlowRate >= meshNeighborsGroundwaterFlowRate[edge])))
    {
      CkError("ERROR in ChannelElement::handleMeshGroundwaterFlowRateLimitedMessage, element %d, edge %d: "
              "A flow limiting message must be for an inflow and it must only reduce the magnitude of the flow.\n", thisIndex, edge);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(FLOW_RATE_NOT_READY == meshNeighborsGroundwaterFlowRateReady[edge] || FLOW_RATE_CALCULATED == meshNeighborsGroundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // Save the received flow rate value.
  meshNeighborsGroundwaterFlowRate[edge] = edgeGroundwaterFlowRate;

  // The flow limiting check was done by neighbor.
  meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;

  checkGroundwaterFlowRates(iterationThisMessage);
}

void ChannelElement::checkGroundwaterFlowRates(CMK_REFNUM_TYPE iterationThisMessage)
{
  int    edge;                            // Loop counter.
  bool   allCalculated           = true;  // Whether all flows have been calculated.
  bool   outwardNotLimited       = false; // Whether at least one outward flow is not limited.
  double totalOutwardFlowRate    = 0.0;   // Sum of all outward flows.
  double outwardFlowRateFraction = 1.0;   // Fraction of all outward flows that can be satisfied.
  bool   allLimited              = true;  // Whether all flow limiting checks have been done.
  
  // We need to limit outward flows if all flows are at least calculated (not FLOW_NOT_READY) and there is at least one outward flow that is not limited.
  for (edge = 0; allCalculated && edge < meshNeighborsSize; edge++)
    {
      if (FLOW_RATE_NOT_READY == meshNeighborsGroundwaterFlowRateReady[edge])
        {
          allCalculated = false;
          allLimited    = false;
        }
      else if (FLOW_RATE_CALCULATED == meshNeighborsGroundwaterFlowRateReady[edge] && 0.0 < meshNeighborsGroundwaterFlowRate[edge])
        {
          outwardNotLimited     = true;
          totalOutwardFlowRate += meshNeighborsGroundwaterFlowRate[edge];
        }
    }
  
  if (allCalculated && outwardNotLimited)
    {
      // Check if total outward flow is greater than water available.
      if (surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth) * elementLength < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = (surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth) * elementLength) / (totalOutwardFlowRate * dt);
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
      // FIXME Our code doesn't assure this.
      //CkAssert(epsilonEqual(1.0, outwardFlowRateFraction));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // Limit outward flows.
      for (edge = 0; edge < meshNeighborsSize; edge++)
        {
          if (0.0 < meshNeighborsGroundwaterFlowRate[edge])
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(FLOW_RATE_CALCULATED == meshNeighborsGroundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              meshNeighborsGroundwaterFlowRate[edge]     *= outwardFlowRateFraction;
              meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(meshNeighbors[edge]))
                {
                  meshProxy[meshNeighbors[edge]].channelGroundwaterFlowRateLimitedMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge],
                                                                                          -meshNeighborsGroundwaterFlowRate[edge]);
                }
            }
        }
    }
  
  // If the flow rate limiting check is done for all flow rates then we have the final value for all flow rates and can move groundwater now.
  for (edge = 0; allLimited && edge < meshNeighborsSize; edge++)
    {
      allLimited = (FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsGroundwaterFlowRateReady[edge]);
    }
  
  if (allLimited)
    {
      moveGroundwater(iterationThisMessage);
    }
}

// Suppress warning enum value not handled in switch.
#pragma GCC diagnostic ignored "-Wswitch"
void ChannelElement::moveGroundwater(CMK_REFNUM_TYPE iterationThisMessage)
{
  int    edge; // Loop counter.
  double area; // Cross sectional area of water in trapezoidal channel in square meters.
  
  // Convert water depth to cross sectional area.
  area = surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth);
  
  // Calculate new value of area.
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsGroundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      area                                         -= meshNeighborsGroundwaterFlowRate[edge] * dt / elementLength;
      meshNeighborsGroundwaterCumulativeFlow[edge] += meshNeighborsGroundwaterFlowRate[edge] * dt;
    }
  
  // Even though we are limiting outward flows, area can go below zero due to roundoff error.
  // FIXME should we try to take the water back from the error accumulator later?
  if (0.0 > area)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonEqual(0.0, area));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      surfacewaterError -= area * elementLength;
      area               = 0.0;
    }
  
  // Convert cross sectional area back to water depth.
  calculateSurfacewaterDepthFromArea(area);
  
  groundwaterDone = true;
  
  // Initiate surfacewater phase.
  for (edge = 0; edge < channelNeighborsSize; edge++)
    {
      // Set the flow rate state for this timestep to not ready.
      channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_NOT_READY;
      
      if (!isBoundary(channelNeighbors[edge]))
        {
          // Send my state to my neighbor.
          switch (channelNeighborsInteraction[edge])
          {
          // case I_CALCULATE_FLOW_RATE:
            // Do nothing.  I will calculate the flow rate after receiving a state message from my neighbor.
            // break;
          case NEIGHBOR_CALCULATES_FLOW_RATE:
            // Fallthrough.
          case BOTH_CALCULATE_FLOW_RATE:
            // Send state message.
            thisProxy[channelNeighbors[edge]].channelSurfacewaterStateMessage(iterationThisMessage, channelNeighborsReciprocalEdge[edge], surfacewaterDepth);
            break;
          }
        }
    }
  
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
      if (!isBoundary(meshNeighbors[edge]))
        {
          // Set the flow rate state for this timestep to not ready.
          meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_NOT_READY;
          
          // Send my state to my neighbor.
          switch (meshNeighborsInteraction[edge])
          {
          // case I_CALCULATE_FLOW_RATE:
            // Do nothing.  I will calculate the flow rate after receiving a state message from my neighbor.
            // break;
          case NEIGHBOR_CALCULATES_FLOW_RATE:
            // Fallthrough.
          case BOTH_CALCULATE_FLOW_RATE:
            // Send state message.
            meshProxy[meshNeighbors[edge]].channelSurfacewaterStateMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge], surfacewaterDepth);
            break;
          }
        }
      else
        {
          // Only NOFLOW is a valid channel mesh neighbor boundary condition code.
          meshNeighborsSurfacewaterFlowRate[edge]      = 0.0;
          meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
        }
    }
  
  // Send myself a message to calculate boundary conditions.
  // This is sent as a message instead of doing it immediately to better overlap communication and computation.
  thisProxy[thisIndex].calculateSurfacewaterBoundaryConditionsMessage(iterationThisMessage);
}
#pragma GCC diagnostic warning "-Wswitch"

void ChannelElement::handleCalculateSurfacewaterBoundaryConditionsMessage(CMK_REFNUM_TYPE iterationThisMessage)
{
  bool error = false; // Error flag.
  int  edge;          // Loop counter.

  for (edge = 0; !error && edge < channelNeighborsSize; edge++)
    {
      if (isBoundary(channelNeighbors[edge]))
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(FLOW_RATE_NOT_READY == channelNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          // Calculate surfacewater flow rate.
          // FIXME figure out what to do about inflow boundary velocity and height
          error = surfacewaterChannelBoundaryFlowRate(&channelNeighborsSurfacewaterFlowRate[edge], &dtNew, (BoundaryConditionEnum)channelNeighbors[edge], 0.0,
                                                      0.0, elementLength, baseWidth, sideSlope, surfacewaterDepth);
          
          if (!error)
            {
              // Outflows may need to be limited.  Inflows and noflows will not.
              if (0.0 < channelNeighborsSurfacewaterFlowRate[edge])
                {
                  channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
                }
            }
        }
    }

  if (!error)
    {
      checkSurfacewaterFlowRates(iterationThisMessage);
    }
  else
    {
      CkExit();
    }
}

// Suppress warning enum value not handled in switch.
#pragma GCC diagnostic ignored "-Wswitch"
void ChannelElement::handleChannelSurfacewaterStateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double neighborSurfacewaterDepth)
{
  bool   error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleChannelSurfacewaterStateMessage, element %d, edge %d: edge must be between zero and channelNeighborsSize.\n",
              thisIndex, edge);
      error = true;
    }

  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::handleChannelSurfacewaterStateMessage, element %d, edge %d: "
              "neighborSurfacewaterDepth must be greater than or equal to zero.\n", thisIndex, edge);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // There is a race condition where a flow limited message can arrive before a state message.
      // In that case, the flow limited message already had the correct flow value and you can ignore the state message.
      if (FLOW_RATE_NOT_READY == channelNeighborsSurfacewaterFlowRateReady[edge])
        {
          // Calculate surfacewater flow rate.
          error = surfacewaterChannelChannelFlowRate(&channelNeighborsSurfacewaterFlowRate[edge], &dtNew, channelType, elementZBank, elementZBed,
                                                     elementLength, baseWidth, sideSlope, manningsN, surfacewaterDepth, channelNeighborsChannelType[edge],
                                                     channelNeighborsZBank[edge], channelNeighborsZBed[edge], channelNeighborsLength[edge],
                                                     channelNeighborsBaseWidth[edge], channelNeighborsSideSlope[edge], channelNeighborsManningsN[edge],
                                                     neighborSurfacewaterDepth);
          
          if (!error)
            {
              // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
              if (0.0 != channelNeighborsSurfacewaterFlowRate[edge])
                {
                  channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      else
        {
          CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsSurfacewaterFlowRateReady[edge]);
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  if (!error)
    {
      switch (channelNeighborsInteraction[edge])
      {
      case I_CALCULATE_FLOW_RATE:
        // Send flow message.
        thisProxy[channelNeighbors[edge]].channelSurfacewaterFlowRateMessage(iterationThisMessage, channelNeighborsReciprocalEdge[edge],
                                                                             -channelNeighborsSurfacewaterFlowRate[edge]);
        break;
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      case NEIGHBOR_CALCULATES_FLOW_RATE:
        // I should never receive a state message with the NEIGHBOR_CALCULATES_FLOW interaction.
        CkAssert(false);
        break;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        // case BOTH_CALCULATE_FLOW_RATE:
          // Do nothing.  My neighbor will calculate the same flow value.
          // break;
      }

      checkSurfacewaterFlowRates(iterationThisMessage);
    }
  else
    {
      CkExit();
    }
}

void ChannelElement::handleMeshSurfacewaterStateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleMeshSurfacewaterStateMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      error = true;
    }

  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::handleMeshSurfacewaterStateMessage, element %d, edge %d: "
              "neighborSurfacewaterDepth must be greater than or equal to zero.\n", thisIndex, edge);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // There is a race condition where a flow limited message can arrive before a state message.
      // In that case, the flow limited message already had the correct flow value and you can ignore the state message.
      if (FLOW_RATE_NOT_READY == meshNeighborsSurfacewaterFlowRateReady[edge])
        {
          // Calculate surfacewater flow rate.
          error = surfacewaterMeshChannelFlowRate(&meshNeighborsSurfacewaterFlowRate[edge], meshNeighborsEdgeLength[edge], meshNeighborsZSurface[edge],
                                                  neighborSurfacewaterDepth, elementZBank, elementZBed, surfacewaterDepth);
          
          // Use negative of value so that positive means flow out of the channel.
          meshNeighborsSurfacewaterFlowRate[edge] *= -1.0;
          
          if (!error)
            {
              // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
              if (0.0 != meshNeighborsSurfacewaterFlowRate[edge])
                {
                  meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      else
        {
          CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsSurfacewaterFlowRateReady[edge]);
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  if (!error)
    {
      switch (meshNeighborsInteraction[edge])
      {
      case I_CALCULATE_FLOW_RATE:
        // Send flow message.
        meshProxy[meshNeighbors[edge]].channelSurfacewaterFlowRateMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge],
                                                                          -meshNeighborsSurfacewaterFlowRate[edge]);
        break;
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      case NEIGHBOR_CALCULATES_FLOW_RATE:
        // I should never receive a state message with the NEIGHBOR_CALCULATES_FLOW interaction.
        CkAssert(false);
        break;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        // case BOTH_CALCULATE_FLOW_RATE:
          // Do nothing.  My neighbor will calculate the same flow value.
          // break;
      }

      checkSurfacewaterFlowRates(iterationThisMessage);
    }
  else
    {
      CkExit();
    }
}
#pragma GCC diagnostic warning "-Wswitch"

void ChannelElement::handleChannelSurfacewaterFlowRateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleChannelSurfacewaterFlowRateMessage, element %d, edge %d: edge must be between zero and channelNeighborsSize.\n",
              thisIndex, edge);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // I should only receive a flow rate message with the NEIGHBOR_CALCULATES_FLOW interaction.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(NEIGHBOR_CALCULATES_FLOW_RATE == channelNeighborsInteraction[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // There is a race condition where a flow limited message can arrive before a flow rate message.
  // In that case, the flow limited message already had the correct flow value and you can ignore the flow rate message.
  if (FLOW_RATE_NOT_READY == channelNeighborsSurfacewaterFlowRateReady[edge])
    {
      // Save the received flow rate value.
      channelNeighborsSurfacewaterFlowRate[edge] = edgeSurfacewaterFlowRate;

      // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
      if (0.0 != channelNeighborsSurfacewaterFlowRate[edge])
        {
          channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
        }
      else
        {
          channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsSurfacewaterFlowRateReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  checkSurfacewaterFlowRates(iterationThisMessage);
}

void ChannelElement::handleMeshSurfacewaterFlowRateMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleMeshSurfacewaterFlowRateMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // I should only receive a flow rate message with the NEIGHBOR_CALCULATES_FLOW interaction.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(NEIGHBOR_CALCULATES_FLOW_RATE == meshNeighborsInteraction[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // There is a race condition where a flow limited message can arrive before a flow rate message.
  // In that case, the flow limited message already had the correct flow value and you can ignore the flow rate message.
  if (FLOW_RATE_NOT_READY == meshNeighborsSurfacewaterFlowRateReady[edge])
    {
      // Save the received flow rate value.
      meshNeighborsSurfacewaterFlowRate[edge] = edgeSurfacewaterFlowRate;

      // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
      if (0.0 != meshNeighborsSurfacewaterFlowRate[edge])
        {
          meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
        }
      else
        {
          meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsSurfacewaterFlowRateReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  checkSurfacewaterFlowRates(iterationThisMessage);
}

void ChannelElement::handleChannelSurfacewaterFlowRateLimitedMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleChannelSurfacewaterFlowRateLimitedMessage, element %d, edge %d: edge must be between zero and "
              "channelNeighborsSize.\n", thisIndex, edge);
      CkExit();
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeSurfacewaterFlowRate &&
             (FLOW_RATE_NOT_READY == channelNeighborsSurfacewaterFlowRateReady[edge] ||
              edgeSurfacewaterFlowRate >= channelNeighborsSurfacewaterFlowRate[edge])))
    {
      CkError("ERROR in ChannelElement::handleChannelSurfacewaterFlowRateLimitedMessage, element %d, edge %d: "
              "A flow limiting message must be for an inflow and it must only reduce the magnitude of the flow.\n", thisIndex, edge);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(FLOW_RATE_NOT_READY == channelNeighborsSurfacewaterFlowRateReady[edge] || FLOW_RATE_CALCULATED == channelNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // Save the received flow rate value.
  channelNeighborsSurfacewaterFlowRate[edge] = edgeSurfacewaterFlowRate;

  // The flow limiting check was done by neighbor.
  channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;

  checkSurfacewaterFlowRates(iterationThisMessage);
}

void ChannelElement::handleMeshSurfacewaterFlowRateLimitedMessage(CMK_REFNUM_TYPE iterationThisMessage, int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in ChannelElement::handleMeshSurfacewaterFlowRateLimitedMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      CkExit();
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeSurfacewaterFlowRate &&
             (FLOW_RATE_NOT_READY == meshNeighborsSurfacewaterFlowRateReady[edge] || edgeSurfacewaterFlowRate >= meshNeighborsSurfacewaterFlowRate[edge])))
    {
      CkError("ERROR in ChannelElement::handleMeshSurfacewaterFlowRateLimitedMessage, element %d, edge %d: "
              "A flow limiting message must be for an inflow and it must only reduce the magnitude of the flow.\n", thisIndex, edge);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(FLOW_RATE_NOT_READY == meshNeighborsSurfacewaterFlowRateReady[edge] || FLOW_RATE_CALCULATED == meshNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // Save the received flow rate value.
  meshNeighborsSurfacewaterFlowRate[edge] = edgeSurfacewaterFlowRate;

  // The flow limiting check was done by neighbor.
  meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;

  checkSurfacewaterFlowRates(iterationThisMessage);
}

void ChannelElement::checkSurfacewaterFlowRates(CMK_REFNUM_TYPE iterationThisMessage)
{
  int    edge;                            // Loop counter.
  bool   allCalculated           = true;  // Whether all flows have been calculated.
  bool   outwardNotLimited       = false; // Whether at least one outward flow is not limited.
  double totalOutwardFlowRate    = 0.0;   // Sum of all outward flows.
  double outwardFlowRateFraction = 1.0;   // Fraction of all outward flows that can be satisfied.
  bool   allLimited              = true;  // Whether all flow limiting checks have been done.
  
  // We need to limit outward flows if all flows are at least calculated (not FLOW_NOT_READY) and there is at least one outward flow that is not limited.
  for (edge = 0; allCalculated && edge < channelNeighborsSize; edge++)
    {
      if (FLOW_RATE_NOT_READY == channelNeighborsSurfacewaterFlowRateReady[edge])
        {
          allCalculated = false;
          allLimited    = false;
        }
      else if (FLOW_RATE_CALCULATED == channelNeighborsSurfacewaterFlowRateReady[edge] && 0.0 < channelNeighborsSurfacewaterFlowRate[edge])
        {
          outwardNotLimited     = true;
          totalOutwardFlowRate += channelNeighborsSurfacewaterFlowRate[edge];
        }
    }
  
  for (edge = 0; allCalculated && edge < meshNeighborsSize; edge++)
    {
      if (FLOW_RATE_NOT_READY == meshNeighborsSurfacewaterFlowRateReady[edge])
        {
          allCalculated = false;
          allLimited    = false;
        }
      else if (FLOW_RATE_CALCULATED == meshNeighborsSurfacewaterFlowRateReady[edge] && 0.0 < meshNeighborsSurfacewaterFlowRate[edge])
        {
          outwardNotLimited     = true;
          totalOutwardFlowRate += meshNeighborsSurfacewaterFlowRate[edge];
        }
    }
  
  if (allCalculated && outwardNotLimited)
    {
      // Check if total outward flow is greater than water available.
      if (surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth) * elementLength < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = (surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth) * elementLength) / (totalOutwardFlowRate * dt);
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
      // FIXME Our code doesn't assure this.
      //CkAssert(epsilonEqual(1.0, outwardFlowRateFraction));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // Limit outward flows.
      for (edge = 0; edge < channelNeighborsSize; edge++)
        {
          if (0.0 < channelNeighborsSurfacewaterFlowRate[edge])
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(FLOW_RATE_CALCULATED == channelNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              channelNeighborsSurfacewaterFlowRate[edge]     *= outwardFlowRateFraction;
              channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(channelNeighbors[edge]))
                {
                  thisProxy[channelNeighbors[edge]].channelSurfacewaterFlowRateLimitedMessage(iterationThisMessage, channelNeighborsReciprocalEdge[edge],
                                                                                              -channelNeighborsSurfacewaterFlowRate[edge]);
                }
            }
        }

      for (edge = 0; edge < meshNeighborsSize; edge++)
        {
          if (0.0 < meshNeighborsSurfacewaterFlowRate[edge])
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(FLOW_RATE_CALCULATED == meshNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              meshNeighborsSurfacewaterFlowRate[edge]     *= outwardFlowRateFraction;
              meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(meshNeighbors[edge]))
                {
                  meshProxy[meshNeighbors[edge]].channelSurfacewaterFlowRateLimitedMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge],
                                                                                           -meshNeighborsSurfacewaterFlowRate[edge]);
                }
            }
        }
    }
  
  // If the flow rate limiting check is done for all flow rates then we have the final value for all flow rates and can move surfacewater now.
  for (edge = 0; allLimited && edge < channelNeighborsSize; edge++)
    {
      allLimited = (FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsSurfacewaterFlowRateReady[edge]);
    }
  
  for (edge = 0; allLimited && edge < meshNeighborsSize; edge++)
    {
      allLimited = (FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsSurfacewaterFlowRateReady[edge]);
    }
  
  if (allLimited)
    {
      moveSurfacewater();
    }
}

void ChannelElement::moveSurfacewater()
{
  int    edge; // Loop counter.
  double area; // Cross sectional area of water in trapezoidal channel in square meters.
  
  // Convert water depth to cross sectional area.
  area = surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth);
  
  // Calculate new value of area.
  for (edge = 0; edge < channelNeighborsSize; edge++)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      area                                             -= channelNeighborsSurfacewaterFlowRate[edge] * dt / elementLength;
      channelNeighborsSurfacewaterCumulativeFlow[edge] += channelNeighborsSurfacewaterFlowRate[edge] * dt;
    }
  
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      area                                          -= meshNeighborsSurfacewaterFlowRate[edge] * dt / elementLength;
      meshNeighborsSurfacewaterCumulativeFlow[edge] += meshNeighborsSurfacewaterFlowRate[edge] * dt;
    }
  
  // Even though we are limiting outward flows, area can go below zero due to roundoff error.
  // FIXME should we try to take the water back from the error accumulator later?
  if (0.0 > area)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonEqual(0.0, area));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      surfacewaterError -= area * elementLength;
      area               = 0.0;
    }
  
  // Convert cross sectional area back to water depth.
  calculateSurfacewaterDepthFromArea(area);
  
  surfacewaterDone = true;
  
  // Perform min reduction on dtNew.  This also serves as a barrier at the end of the timestep.
  contribute(sizeof(double), &dtNew, CkReduction::min_double);
}

void ChannelElement::calculateSurfacewaterDepthFromArea(double area)
{
  double bOverTwoS; // Temporary value reused in computation.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(0.0 <= area);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  if (0.0 < sideSlope)
    {
      // Trapezoidal or triangular channel.
      bOverTwoS = baseWidth / (2.0 * sideSlope);

      surfacewaterDepth = sqrt(area / sideSlope + bOverTwoS * bOverTwoS) - bOverTwoS;
      
      // For very small areas surfacewaterDepth might be able to come out negative due to roundoff error.
      if (0.0 > surfacewaterDepth)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0, surfacewaterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          surfacewaterDepth = 0.0;
        }
    }
  else
    {
      // Rectangular channel.  By invariant baseWidth and sideSlope can't both be zero.
      surfacewaterDepth = area / baseWidth;
    }
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "channel_element.def.h"
#pragma GCC diagnostic warning "-Wsign-compare"
#pragma GCC diagnostic warning "-Wunused-variable"
