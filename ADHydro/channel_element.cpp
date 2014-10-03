#include "channel_element.h"
#include "file_manager.h"
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
  p | channelType;
  p | permanentCode;
  p | baseWidth;
  p | sideSlope;
  p | bedConductivity;
  p | bedThickness;
  p | manningsN;
  p | surfacewaterDepth;
  p | surfacewaterError;
  p | groundwaterDone;
  p | surfacewaterDone;
  p | dt;
  p | dtNew;
  PUParray(p, channelNeighbors, channelNeighborsSize);
  PUParray(p, channelNeighborsReciprocalEdge, channelNeighborsSize);
  PUParray(p, channelNeighborsInteraction, channelNeighborsSize);
  PUParray(p, channelNeighborsInitialized, channelNeighborsSize);
  PUParray(p, channelNeighborsInvariantChecked, channelNeighborsSize);
  PUParray(p, channelNeighborsZBank, channelNeighborsSize);
  PUParray(p, channelNeighborsZBed, channelNeighborsSize);
  PUParray(p, channelNeighborsLength, channelNeighborsSize);
  PUParray(p, channelNeighborsChannelType, channelNeighborsSize);
  PUParray(p, channelNeighborsBaseWidth, channelNeighborsSize);
  PUParray(p, channelNeighborsSideSlope, channelNeighborsSize);
  PUParray(p, channelNeighborsManningsN, channelNeighborsSize);
  PUParray(p, channelNeighborsSurfacewaterFlowRateReady, channelNeighborsSize);
  PUParray(p, channelNeighborsSurfacewaterFlowRate, channelNeighborsSize);
  PUParray(p, channelNeighborsSurfacewaterCumulativeFlow, channelNeighborsSize);
  PUParray(p, meshNeighbors, meshNeighborsSize);
  PUParray(p, meshNeighborsReciprocalEdge, meshNeighborsSize);
  PUParray(p, meshNeighborsInteraction, meshNeighborsSize);
  PUParray(p, meshNeighborsInitialized, meshNeighborsSize);
  PUParray(p, meshNeighborsInvariantChecked, meshNeighborsSize);
  PUParray(p, meshNeighborsZSurface, meshNeighborsSize);
  PUParray(p, meshNeighborsZBedrock, meshNeighborsSize);
  PUParray(p, meshNeighborsZOffset, meshNeighborsSize);
  PUParray(p, meshNeighborsEdgeLength, meshNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterFlowRateReady, meshNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterFlowRate, meshNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterCumulativeFlow, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterFlowRateReady, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterFlowRate, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterCumulativeFlow, meshNeighborsSize);
}

bool ChannelElement::allInitialized()
{
  int  edge;               // Loop counter.
  bool initialized = true; // Flag to record whether we have found an uninitialized neighbor.
  
  for (edge = 0; initialized && edge < channelNeighborsSize; edge++)
    {
      initialized = channelNeighborsInitialized[edge];
    }
  
  for (edge = 0; initialized && edge < meshNeighborsSize; edge++)
    {
      initialized = meshNeighborsInitialized[edge];
    }
  
  return initialized;
}

bool ChannelElement::allInvariantChecked()
{
  int  edge;               // Loop counter.
  bool initialized = true; // Flag to record whether we have found an unchecked neighbor.
  
  for (edge = 0; initialized && edge < channelNeighborsSize; edge++)
    {
      initialized = channelNeighborsInvariantChecked[edge];
    }
  
  for (edge = 0; initialized && edge < meshNeighborsSize; edge++)
    {
      initialized = meshNeighborsInvariantChecked[edge];
    }
  
  return initialized;
}

void ChannelElement::handleInitialize(CProxy_MeshElement meshProxyInit, CProxy_FileManager fileManagerProxyInit)
{
  bool         error                  = false;                                                        // Error flag.
  int          edge;                                                                                  // Loop counter.
  FileManager* fileManagerLocalBranch = fileManagerProxyInit.ckLocalBranch();                         // Used for access to local public member variables.
  int          fileManagerLocalIndex  = thisIndex - fileManagerLocalBranch->localChannelElementStart; // Index of this element in file manager arrays.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(0 <= fileManagerLocalIndex && fileManagerLocalIndex < fileManagerLocalBranch->localNumberOfChannelElements))
    {
      CkError("ERROR in ChannelElement::handleInitialize, element %d: initialization information not available from local file manager.\n", thisIndex);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  if (!error)
    {
      meshProxy        = meshProxyInit;
      fileManagerProxy = fileManagerProxyInit;
      
      if (NULL != fileManagerLocalBranch->channelElementX)
        {
          elementX = fileManagerLocalBranch->channelElementX[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: elementX initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelElementY)
        {
          elementY = fileManagerLocalBranch->channelElementY[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: elementY initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelElementZBank)
        {
          elementZBank = fileManagerLocalBranch->channelElementZBank[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: elementZBank initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelElementZBed)
        {
          elementZBed = fileManagerLocalBranch->channelElementZBed[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: elementZBed initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelElementLength)
        {
          elementLength = fileManagerLocalBranch->channelElementLength[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: elementLength initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelChannelType)
        {
          channelType = fileManagerLocalBranch->channelChannelType[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: channelType initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelPermanentCode)
        {
          permanentCode = fileManagerLocalBranch->channelPermanentCode[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: permanentCode initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelBaseWidth)
        {
          baseWidth = fileManagerLocalBranch->channelBaseWidth[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: baseWidth initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelSideSlope)
        {
          sideSlope = fileManagerLocalBranch->channelSideSlope[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: sideSlope initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelBedConductivity)
        {
          bedConductivity = fileManagerLocalBranch->channelBedConductivity[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: bedConductivity initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelBedThickness)
        {
          bedThickness = fileManagerLocalBranch->channelBedThickness[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: bedThickness initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelManningsN)
        {
          manningsN = fileManagerLocalBranch->channelManningsN[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: manningsN initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelSurfacewaterDepth)
        {
          surfacewaterDepth = fileManagerLocalBranch->channelSurfacewaterDepth[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: surfacewaterDepth initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelSurfacewaterError)
        {
          surfacewaterError = fileManagerLocalBranch->channelSurfacewaterError[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: surfacewaterError initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      groundwaterDone          = true;
      surfacewaterDone         = true;
      dt                       = 1.0;
      dtNew                    = 1.0;
      
      if (NULL != fileManagerLocalBranch->channelChannelNeighbors)
        {
          for (edge = 0; !error && edge < channelNeighborsSize; edge++)
            {
              channelNeighbors[edge]                           = fileManagerLocalBranch->channelChannelNeighbors[fileManagerLocalIndex][edge];
              channelNeighborsInvariantChecked[edge]           = true;
              channelNeighborsSurfacewaterFlowRateReady[edge]  = FLOW_RATE_LIMITING_CHECK_DONE;
              channelNeighborsSurfacewaterFlowRate[edge]       = 0.0;
              channelNeighborsSurfacewaterCumulativeFlow[edge] = 0.0;
              
              if (isBoundary(channelNeighbors[edge]))
                {
                  channelNeighborsInitialized[edge] = true;
                  
                  // Unused, but initialize for completeness.
                  channelNeighborsReciprocalEdge[edge] = 0;
                  channelNeighborsInteraction[edge]    = BOTH_CALCULATE_FLOW_RATE;
                  channelNeighborsZBank[edge]          = 0.0;
                  channelNeighborsZBed[edge]           = 0.0;
                  channelNeighborsLength[edge]         = 1.0;
                  channelNeighborsChannelType[edge]    = STREAM;
                  channelNeighborsBaseWidth[edge]      = 1.0;
                  channelNeighborsSideSlope[edge]      = 1.0;
                  channelNeighborsManningsN[edge]      = 1.0;
                }
              else if (0 <= channelNeighbors[edge] && channelNeighbors[edge] < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
                {
                  channelNeighborsInitialized[edge] = false;
                  
                  thisProxy[channelNeighbors[edge]].initializeChannelNeighbor(thisIndex, edge, elementZBank, elementZBed, elementLength, channelType,
                                                                              baseWidth, sideSlope, manningsN);
                }
              else
                {
                  // We have to finish initialization before checking the invariant so the invariant hasn't been checked yet so we have to check this here.
                  CkError("ERROR in ChannelElement::handleInitialize, element %d, edge %d: channelNeighbors must be a boundary condition code or a valid "
                          "array index.\n", thisIndex, edge);
                  error = true;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: channelNeighbors initialization information not available from local file "
                  "manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelMeshNeighbors)
        {
          for (edge = 0; !error && edge < meshNeighborsSize; edge++)
            {
              meshNeighbors[edge]                           = fileManagerLocalBranch->channelMeshNeighbors[fileManagerLocalIndex][edge];
              meshNeighborsInvariantChecked[edge]           = true;
              meshNeighborsSurfacewaterFlowRateReady[edge]  = FLOW_RATE_LIMITING_CHECK_DONE;
              meshNeighborsSurfacewaterFlowRate[edge]       = 0.0;
              meshNeighborsSurfacewaterCumulativeFlow[edge] = 0.0;
              meshNeighborsGroundwaterFlowRateReady[edge]   = FLOW_RATE_LIMITING_CHECK_DONE;
              meshNeighborsGroundwaterFlowRate[edge]        = 0.0;
              meshNeighborsGroundwaterCumulativeFlow[edge]  = 0.0;
              
              if (isBoundary(meshNeighbors[edge]))
                {
                  meshNeighborsInitialized[edge] = true;
                  
                  // Unused, but initialize for completeness.
                  meshNeighborsReciprocalEdge[edge] = 0;
                  meshNeighborsInteraction[edge]    = BOTH_CALCULATE_FLOW_RATE;
                  meshNeighborsZSurface[edge]       = 0.0;
                  meshNeighborsZBedrock[edge]       = 0.0;
                  meshNeighborsZOffset[edge]        = 0.0;
                }
              else if (0 <= meshNeighbors[edge] && meshNeighbors[edge] < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
                {
                  meshNeighborsInitialized[edge] = false;
                  
                  meshProxy[meshNeighbors[edge]].initializeChannelNeighbor(thisIndex, edge, elementX, elementY, elementZBank, elementZBed, baseWidth,
                                                                           sideSlope, bedConductivity, bedThickness);
                }
              else
                {
                  // We have to finish initialization before checking the invariant so the invariant hasn't been checked yet so we have to check this here.
                  CkError("ERROR in ChannelElement::handleInitialize, element %d, edge %d: meshNeighbors must be a boundary condition code or a valid "
                          "array index.\n", thisIndex, edge);
                  error = true;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: meshNeighbors initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->channelMeshNeighborsEdgeLength)
        {
          for (edge = 0; edge < meshNeighborsSize; edge++)
            {
              meshNeighborsEdgeLength[edge] = fileManagerLocalBranch->channelMeshNeighborsEdgeLength[fileManagerLocalIndex][edge];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in ChannelElement::handleInitialize, element %d: meshNeighborsEdgeLength initialization information not available from local file "
                  "manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Error checking of initialization values is done in the invariant.
  
  if (error)
    {
      CkExit();
    }
}

void ChannelElement::handleInitializeChannelNeighbor(int neighbor, int neighborReciprocalEdge, double neighborZBank, double neighborZBed,
                                                     double neighborLength, ChannelTypeEnum neighborChannelType, double neighborBaseWidth,
                                                     double neighborSideSlope, double neighborManningsN)
{
  bool error = false; // Error flag.
  int  edge  = 0;     // Loop counter.
  
  while (edge < channelNeighborsSize - 1 && channelNeighbors[edge] != neighbor)
    {
      edge++;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(channelNeighbors[edge] == neighbor))
    {
      CkError("ERROR in ChannelElement::handleInitializeChannelNeighbor, element %d: received an initialization message from an element that is not my "
              "neighbor.\n", thisIndex);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  if (!error)
    {
      channelNeighborsInitialized[edge]     = true;
      channelNeighborsReciprocalEdge[edge]  = neighborReciprocalEdge;
      channelNeighborsInteraction[edge]     = BOTH_CALCULATE_FLOW_RATE;
      channelNeighborsZBank[edge]           = neighborZBank;
      channelNeighborsZBed[edge]            = neighborZBed;
      channelNeighborsLength[edge]          = neighborLength;
      channelNeighborsChannelType[edge]     = neighborChannelType;
      channelNeighborsBaseWidth[edge]       = neighborBaseWidth;
      channelNeighborsSideSlope[edge]       = neighborSideSlope;
      channelNeighborsManningsN[edge]       = neighborManningsN;
    }
  
  // Error checking of initialization values is done in the invariant.
  
  if (error)
    {
      CkExit();
    }
}

void ChannelElement::handleInitializeMeshNeighbor(int neighbor, int neighborReciprocalEdge, double neighborX, double neighborY, double neighborZSurface,
                                                  double neighborZBedrock, double neighborSlopeX, double neighborSlopeY)
{
  bool error = false; // Error flag.
  int  edge  = 0;     // Loop counter.
  
  while (edge < meshNeighborsSize - 1 && meshNeighbors[edge] != neighbor)
    {
      edge++;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(meshNeighbors[edge] == neighbor))
    {
      CkError("ERROR in ChannelElement::handleInitializeMeshNeighbor, element %d: received an initialization message from an element that is not my "
              "neighbor.\n", thisIndex);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  if (!error)
    {
      meshNeighborsInitialized[edge]    = true;
      meshNeighborsReciprocalEdge[edge] = neighborReciprocalEdge;
      meshNeighborsInteraction[edge]    = BOTH_CALCULATE_FLOW_RATE;
      meshNeighborsZSurface[edge]       = neighborZSurface;
      meshNeighborsZBedrock[edge]       = neighborZBedrock;
      meshNeighborsZOffset[edge]        = (elementX - neighborX) * neighborSlopeX + (elementY - neighborY) * neighborSlopeY;
    }
  
  // Error checking of initialization values is done in the invariant.
  
  if (error)
    {
      CkExit();
    }
}

void ChannelElement::handleForcingDataMessage()
{
  // FIXME add forcing data for channels.
  
  contribute();
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
          error = groundwaterMeshChannelFlowRate(&meshNeighborsGroundwaterFlowRate[edge], meshNeighborsEdgeLength[edge],
                                                 meshNeighborsZSurface[edge] + meshNeighborsZOffset[edge],
                                                 meshNeighborsZBedrock[edge] + meshNeighborsZOffset[edge], neighborSurfacewaterDepth,
                                                 neighborGroundwaterHead + meshNeighborsZOffset[edge], elementZBank, elementZBed, baseWidth, sideSlope,
                                                 bedConductivity, bedThickness, surfacewaterDepth);
          
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
          error = surfacewaterMeshChannelFlowRate(&meshNeighborsSurfacewaterFlowRate[edge], meshNeighborsEdgeLength[edge],
                                                  meshNeighborsZSurface[edge] + meshNeighborsZOffset[edge], neighborSurfacewaterDepth, elementZBank,
                                                  elementZBed, surfacewaterDepth);
          
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

          surfacewaterError -= surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth) * elementLength;
          surfacewaterDepth  = 0.0;
        }
    }
  else
    {
      // Rectangular channel.  By invariant baseWidth and sideSlope can't both be zero.
      surfacewaterDepth = area / baseWidth;
    }
}

void ChannelElement::handleCheckInvariant()
{
  bool error = false; // Error flag.
  int  edge;          // Loop counter.
  
  if (!(elementZBank >= elementZBed))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: elementZBank must be greater than or equal to elementZBed.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < elementLength))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: elementLength must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(STREAM == channelType || WATERBODY == channelType || ICEMASS == channelType))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: channelType must be a valid enum value.\n", thisIndex);
      error = true;
    }
  
  if (!(0 < permanentCode))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: permanentCode must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 <= baseWidth))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: baseWidth must be greater than or equal to zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 <= sideSlope))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: sideSlope must be greater than or equal to zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < baseWidth || 0.0 < sideSlope))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: at least one of baseWidth or SideSlope must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < bedConductivity))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: bedConductivity must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < bedThickness))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: bedThickness must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < manningsN))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: manningsN must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: surfacewaterDepth must be greater than or equal to zero.\n", thisIndex);
      error = true;
    }
  
  if (!groundwaterDone)
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: groundwaterDone must be true when not processing a timestep.\n", thisIndex);
      error = true;
    }
  
  if (!surfacewaterDone)
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: surfacewaterDone must be true when not processing a timestep.\n", thisIndex);
      error = true;
    }
    
  if (!(0.0 < dt))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: dt must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < dtNew))
    {
      CkError("ERROR in ChannelElement::handleCheckInvariant, element %d: dtNew must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  for (edge = 0; edge < channelNeighborsSize; edge++)
    {
      if (!(isBoundary(channelNeighbors[edge]) || (0 <= channelNeighbors[edge] &&
                                                   channelNeighbors[edge] < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: channelNeighbors must be a boundary condition code or a valid array "
                  "index.\n", thisIndex, edge);
          error = true;
        }
      
      if (!(0 <= channelNeighborsReciprocalEdge[edge] && channelNeighborsReciprocalEdge[edge] < channelNeighborsSize))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: channelNeighborsReciprocalEdge must be a valid array index.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(I_CALCULATE_FLOW_RATE    == channelNeighborsInteraction[edge] || NEIGHBOR_CALCULATES_FLOW_RATE == channelNeighborsInteraction[edge] ||
            BOTH_CALCULATE_FLOW_RATE == channelNeighborsInteraction[edge]))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: channelNeighborsInteraction must be a valid enum value.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!channelNeighborsInitialized[edge])
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: channelNeighborsInitialized must be true before checking invariant.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsSurfacewaterFlowRateReady[edge]))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: channelNeighborsSurfacewaterFlowRateReady must be ready when not "
                  "processing a timestep.\n", thisIndex, edge);
          error = true;
        }
    }
  
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
      if (!(NOFLOW == meshNeighbors[edge] || (0 <= meshNeighbors[edge] &&
                                              meshNeighbors[edge] < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: meshNeighbors must be NOFLOW or a valid array index.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(0 <= meshNeighborsReciprocalEdge[edge] && meshNeighborsReciprocalEdge[edge] < MeshElement::channelNeighborsSize))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: meshNeighborsReciprocalEdge must be a valid array index.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(I_CALCULATE_FLOW_RATE    == meshNeighborsInteraction[edge] || NEIGHBOR_CALCULATES_FLOW_RATE == meshNeighborsInteraction[edge] ||
            BOTH_CALCULATE_FLOW_RATE == meshNeighborsInteraction[edge]))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: meshNeighborsInteraction must be a valid enum value.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!meshNeighborsInitialized[edge])
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: meshNeighborsInitialized must be true before checking invariant.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(0.0 < meshNeighborsEdgeLength[edge]))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: meshNeighborsEdgeLength must be greater than zero.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsSurfacewaterFlowRateReady[edge]))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: meshNeighborsSurfacewaterFlowRateReady must be ready when not processing "
                  "a timestep.\n", thisIndex, edge);
          error = true;
        }
      
      if (!(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsGroundwaterFlowRateReady[edge]))
        {
          CkError("ERROR in ChannelElement::handleCheckInvariant, element %d, edge %d: meshNeighborsGroundwaterFlowRateReady must be ready when not processing "
                  "a timestep.\n", thisIndex, edge);
          error = true;
        }
    }
  
  if (!error)
    {
      for (edge = 0; edge < channelNeighborsSize; edge++)
        {
          if (isBoundary(channelNeighbors[edge]))
            {
              channelNeighborsInvariantChecked[edge] = true;
            }
          else
            {
              channelNeighborsInvariantChecked[edge] = false;
              
              thisProxy[channelNeighbors[edge]].checkChannelNeighborInvariant(thisIndex, channelNeighborsReciprocalEdge[edge], edge,
                                                                              channelNeighborsInteraction[edge], elementZBank, elementZBed, elementLength,
                                                                              channelType, baseWidth, sideSlope, manningsN,
                                                                              channelNeighborsSurfacewaterFlowRate[edge],
                                                                              channelNeighborsSurfacewaterCumulativeFlow[edge], dt);
            }
        }
      
      for (edge = 0; edge < meshNeighborsSize; edge++)
        {
          if (isBoundary(meshNeighbors[edge]))
            {
              meshNeighborsInvariantChecked[edge] = true;
            }
          else
            {
              meshNeighborsInvariantChecked[edge] = false;
              
              meshProxy[meshNeighbors[edge]].checkChannelNeighborInvariant(thisIndex, meshNeighborsReciprocalEdge[edge], edge, meshNeighborsInteraction[edge],
                                                                           elementX, elementY, elementZBank, elementZBed, meshNeighborsZOffset[edge],
                                                                           meshNeighborsEdgeLength[edge], baseWidth, sideSlope, bedConductivity, bedThickness,
                                                                           meshNeighborsSurfacewaterFlowRate[edge],
                                                                           meshNeighborsSurfacewaterCumulativeFlow[edge],
                                                                           meshNeighborsGroundwaterFlowRate[edge],
                                                                           meshNeighborsGroundwaterCumulativeFlow[edge], dt);
            }
        }
    }
  else
    {
      CkExit();
    }
}

void ChannelElement::handleCheckChannelNeighborInvariant(int neighbor, int edge, int neighborEdge, InteractionEnum neighborInteraction, double neighborZBank,
                                                         double neighborZBed, double neighborLength, double neighborChannelType, double neighborBaseWidth,
                                                         double neighborSideSlope, double neighborManningsN, double neighborSurfacewaterFlowRate,
                                                         double neighborSurfacewaterCumulativeFlow, double neighborDt)
{
  bool error = false; // Error flag.
  
  if (!(neighbor == channelNeighbors[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: element %d thinks he is my neighbor on this edge, but he is "
              "not.\n", thisIndex, edge, neighbor);
      error = true;
    }
  
  if (!(neighborEdge == channelNeighborsReciprocalEdge[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsReciprocalEdge is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!((I_CALCULATE_FLOW_RATE         == neighborInteraction && NEIGHBOR_CALCULATES_FLOW_RATE == channelNeighborsInteraction[edge]) ||
        (NEIGHBOR_CALCULATES_FLOW_RATE == neighborInteraction && I_CALCULATE_FLOW_RATE         == channelNeighborsInteraction[edge]) ||
        (BOTH_CALCULATE_FLOW_RATE      == neighborInteraction && BOTH_CALCULATE_FLOW_RATE      == channelNeighborsInteraction[edge])))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsInteraction is incompatible.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZBank == channelNeighborsZBank[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsZBank is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZBed == channelNeighborsZBed[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsZBed is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborLength == channelNeighborsLength[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsLength is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborChannelType == channelNeighborsChannelType[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsChannelType is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborBaseWidth == channelNeighborsBaseWidth[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsBaseWidth is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSideSlope == channelNeighborsSideSlope[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsSideSlope is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborManningsN == channelNeighborsManningsN[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsManningsN is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSurfacewaterFlowRate == -channelNeighborsSurfacewaterFlowRate[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsSurfacewaterFlowRate is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSurfacewaterCumulativeFlow == -channelNeighborsSurfacewaterCumulativeFlow[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsSurfacewaterCumulativeFlow is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborDt == dt))
    {
      CkError("ERROR in ChannelElement::handleCheckChannelNeighborInvariant, element %d, edge %d: dt is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!error)
    {
      channelNeighborsInvariantChecked[edge] = true;
    }
  else
    {
      CkExit();
    }
}

void ChannelElement::handleCheckMeshNeighborInvariant(int neighbor, int edge, int neighborEdge, InteractionEnum neighborInteraction, double neighborX,
                                                      double neighborY, double neighborZSurface, double neighborZBedrock, double neighborZOffset,
                                                      double neighborSlopeX, double neighborSlopeY, double neighborEdgeLength,
                                                      double neighborSurfacewaterFlowRate, double neighborSurfacewaterCumulativeFlow,
                                                      double neighborGroundwaterFlowRate, double neighborGroundwaterCumulativeFlow, double neighborDt)
{
  bool error = false; // Error flag.
  
  if (!(neighbor == meshNeighbors[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: element %d thinks he is my neighbor on this edge, but he is "
              "not.\n", thisIndex, edge, neighbor);
      error = true;
    }
  
  if (!(neighborEdge == meshNeighborsReciprocalEdge[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsReciprocalEdge is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!((I_CALCULATE_FLOW_RATE         == neighborInteraction && NEIGHBOR_CALCULATES_FLOW_RATE == meshNeighborsInteraction[edge]) ||
        (NEIGHBOR_CALCULATES_FLOW_RATE == neighborInteraction && I_CALCULATE_FLOW_RATE         == meshNeighborsInteraction[edge]) ||
        (BOTH_CALCULATE_FLOW_RATE      == neighborInteraction && BOTH_CALCULATE_FLOW_RATE      == meshNeighborsInteraction[edge])))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsInteraction is incompatible.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZSurface == meshNeighborsZSurface[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsZSurface is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZBedrock == meshNeighborsZBedrock[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsZBedrock is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZOffset == meshNeighborsZOffset[edge] &&
        (elementX - neighborX) * neighborSlopeX + (elementY - neighborY) * neighborSlopeY == meshNeighborsZOffset[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsZOffset is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborEdgeLength == meshNeighborsEdgeLength[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsEdgeLength is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSurfacewaterFlowRate == -meshNeighborsSurfacewaterFlowRate[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsSurfacewaterFlowRate is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSurfacewaterCumulativeFlow == -meshNeighborsSurfacewaterCumulativeFlow[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsSurfacewaterCumulativeFlow is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborGroundwaterFlowRate == -meshNeighborsGroundwaterFlowRate[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsGroundwaterFlowRate is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborGroundwaterCumulativeFlow == -meshNeighborsGroundwaterCumulativeFlow[edge]))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsGroundwaterCumulativeFlow is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborDt == dt))
    {
      CkError("ERROR in ChannelElement::handleCheckMeshNeighborInvariant, element %d, edge %d: dt is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!error)
    {
      meshNeighborsInvariantChecked[edge] = true;
    }
  else
    {
      CkExit();
    }
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "channel_element.def.h"
#pragma GCC diagnostic warning "-Wunused-variable"
