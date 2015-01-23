#include "mesh_element.h"
#include "file_manager.h"
#include "surfacewater.h"
#include "groundwater.h"
#include <math.h>

// FIXME questions and to-do list items
// How to make it send the high priority messages out first?  We want all the messages going to other nodes to go out as soon as possible.
// Will it send out one MPI message per mesh edge rather than all of the ghost node information in a single message?  Yes, I believe so.
// As elements migrate to different nodes update interaction in a way that guarantees both sides agree on the interaction.
// Think about implications of file manager files being open or closed when checkpointing.

double MeshElement::calculateZOffset(int meshElement, double meshVertexX[meshNeighborsSize], double meshVertexY[meshNeighborsSize], double meshElementX,
                                     double meshElementY, double meshElementZSurface, double meshElementSlopeX, double meshElementSlopeY, int channelElement,
                                     double channelElementX, double channelElementY, double channelElementZBank, ChannelTypeEnum channelType)
{
  int    ii;                        // Loop counter.
  bool   foundIntersection = false; // Whether the line segment from the center of the mesh element to the center of the channel element intersects any edges
                                    // of the mesh element.
  double zOffset;                   // The Z coordinate offset that is being calculated.
  
  // If the center of the channel element is not inside the mesh element only follow the slope of the mesh element to the edge of the mesh element.  We do this
  // by finding whether the line segment from the center of the mesh element to the center of the channel element intersects any of the mesh edges, and if it
  // does use the intersection point instead of the center of the channel element.
  for (ii = 0; !foundIntersection && ii < meshNeighborsSize; ii++)
    {
      foundIntersection = get_line_intersection(meshElementX, meshElementY, channelElementX, channelElementY, meshVertexX[ii], meshVertexY[ii],
                                                meshVertexX[(ii + 1) % meshNeighborsSize], meshVertexY[(ii + 1) % meshNeighborsSize], &channelElementX,
                                                &channelElementY);
    }
  
  zOffset = (channelElementX - meshElementX) * meshElementSlopeX + (channelElementY - meshElementY) * meshElementSlopeY;
  
  // If the channel element is still higher than the mesh element after applying the offset we raise the offset to make them level.  We do not do this for
  // icemasses because there are often real situations where a glacier on a slope is higher than its neighboring mesh elements.
  if (ICEMASS != channelType && zOffset < channelElementZBank - meshElementZSurface)
    {
      if (10.0 < channelElementZBank - meshElementZSurface - zOffset)
        {
          CkError("WARNING in MeshElement::calculateZOffset: mesh element %d is lower than neighboring channel element %d by %lf meters.  Raising zOffset to "
                  "make them level.\n", meshElement, channelElement, channelElementZBank - meshElementZSurface - zOffset);
        }
      
      zOffset = channelElementZBank - meshElementZSurface;
    }
  
  return zOffset;
}

MeshElement::MeshElement()
{
  // Initialization will be done in runForever.
  thisProxy[thisIndex].runForever();
}

MeshElement::MeshElement(CkMigrateMessage* msg)
{
  // Do nothing.
}

void MeshElement::pup(PUP::er &p)
{
  CBase_MeshElement::pup(p);
  __sdag_pup(p);
  p | channelProxy;
  p | fileManagerProxy;
  PUParray(p, vertexX, meshNeighborsSize);
  PUParray(p, vertexY, meshNeighborsSize);
  p | elementX;
  p | elementY;
  p | elementZSurface;
  p | elementZBedrock;
  p | elementArea;
  p | elementSlopeX;
  p | elementSlopeY;
  p | catchment;
  p | vegetationType;
  p | soilType;
  p | conductivity;
  p | porosity;
  p | manningsN;
  p | surfacewaterDepth;
  p | surfacewaterError;
  p | groundwaterHead;
  p | groundwaterError;
  p | precipitation;
  p | precipitationCumulative;
  p | evaporation;
  p | evaporationCumulative;
  p | surfacewaterInfiltration;
  p | groundwaterRecharge;
  p | evapoTranspirationState;
  // FIXME PUP garParameters and garDomain
  p | atmosphereLayerThickness;
  p | shadedFraction;
  p | shadedFractionMaximum;
  p | surfaceTemperature;
  p | surfacePressure;
  p | atomsphereLayerPressure;
  p | eastWindSpeed;
  p | northWindSpeed;
  p | atmosphereLayerMixingRatio;
  p | cloudMixingRatio;
  p | shortWaveRadiationDown;
  p | longWaveRadiationDown;
  p | precipitationRate;
  p | soilBottomTemperature;
  p | planetaryBoundaryLayerHeight;
  p | groundwaterDone;
  p | infiltrationDone;
  p | surfacewaterDone;
  p | dt;
  p | dtNew;
  PUParray(p, meshNeighbors, meshNeighborsSize);
  PUParray(p, meshNeighborsChannelEdge, meshNeighborsSize);
  PUParray(p, meshNeighborsReciprocalEdge, meshNeighborsSize);
  PUParray(p, meshNeighborsInteraction, meshNeighborsSize);
  PUParray(p, meshNeighborsInitialized, meshNeighborsSize);
  PUParray(p, meshNeighborsInvariantChecked, meshNeighborsSize);
  PUParray(p, meshNeighborsX, meshNeighborsSize);
  PUParray(p, meshNeighborsY, meshNeighborsSize);
  PUParray(p, meshNeighborsZSurface, meshNeighborsSize);
  PUParray(p, meshNeighborsZBedrock, meshNeighborsSize);
  PUParray(p, meshNeighborsArea, meshNeighborsSize);
  PUParray(p, meshNeighborsEdgeLength, meshNeighborsSize);
  PUParray(p, meshNeighborsEdgeNormalX, meshNeighborsSize);
  PUParray(p, meshNeighborsEdgeNormalY, meshNeighborsSize);
  PUParray(p, meshNeighborsConductivity, meshNeighborsSize);
  PUParray(p, meshNeighborsManningsN, meshNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterFlowRateReady, meshNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterFlowRate, meshNeighborsSize);
  PUParray(p, meshNeighborsSurfacewaterCumulativeFlow, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterFlowRateReady, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterFlowRate, meshNeighborsSize);
  PUParray(p, meshNeighborsGroundwaterCumulativeFlow, meshNeighborsSize);
  PUParray(p, channelNeighbors, channelNeighborsSize);
  PUParray(p, channelNeighborsReciprocalEdge, channelNeighborsSize);
  PUParray(p, channelNeighborsInteraction, channelNeighborsSize);
  PUParray(p, channelNeighborsInitialized, channelNeighborsSize);
  PUParray(p, channelNeighborsInvariantChecked, channelNeighborsSize);
  PUParray(p, channelNeighborsZBank, channelNeighborsSize);
  PUParray(p, channelNeighborsZBed, channelNeighborsSize);
  PUParray(p, channelNeighborsZOffset, channelNeighborsSize);
  PUParray(p, channelNeighborsEdgeLength, channelNeighborsSize);
  PUParray(p, channelNeighborsBaseWidth, channelNeighborsSize);
  PUParray(p, channelNeighborsSideSlope, channelNeighborsSize);
  PUParray(p, channelNeighborsBedConductivity, channelNeighborsSize);
  PUParray(p, channelNeighborsBedThickness, channelNeighborsSize);
  PUParray(p, channelNeighborsSurfacewaterFlowRateReady, channelNeighborsSize);
  PUParray(p, channelNeighborsSurfacewaterFlowRate, channelNeighborsSize);
  PUParray(p, channelNeighborsSurfacewaterCumulativeFlow, channelNeighborsSize);
  PUParray(p, channelNeighborsGroundwaterFlowRateReady, channelNeighborsSize);
  PUParray(p, channelNeighborsGroundwaterFlowRate, channelNeighborsSize);
  PUParray(p, channelNeighborsGroundwaterCumulativeFlow, channelNeighborsSize);
}

bool MeshElement::allInitialized()
{
  int  edge;               // Loop counter.
  bool initialized = true; // Flag to record whether we have found an uninitialized neighbor.
  
  for (edge = 0; initialized && edge < meshNeighborsSize; edge++)
    {
      initialized = meshNeighborsInitialized[edge];
    }
  
  for (edge = 0; initialized && edge < channelNeighborsSize; edge++)
    {
      initialized = channelNeighborsInitialized[edge];
    }
  
  return initialized;
}

bool MeshElement::allInvariantChecked()
{
  int  edge;               // Loop counter.
  bool initialized = true; // Flag to record whether we have found an unchecked neighbor.
  
  for (edge = 0; initialized && edge < meshNeighborsSize; edge++)
    {
      initialized = meshNeighborsInvariantChecked[edge];
    }
  
  for (edge = 0; initialized && edge < channelNeighborsSize; edge++)
    {
      initialized = channelNeighborsInvariantChecked[edge];
    }
  
  return initialized;
}

void MeshElement::handleInitialize(CProxy_ChannelElement channelProxyInit, CProxy_FileManager fileManagerProxyInit)
{
  bool         error                  = false;                                                     // Error flag.
  int          ii, edge;                                                                           // Loop counters.
  FileManager* fileManagerLocalBranch = fileManagerProxyInit.ckLocalBranch();                      // Used for access to local public member variables.
  int          fileManagerLocalIndex  = thisIndex - fileManagerLocalBranch->localMeshElementStart; // Index of this element in file manager arrays.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(0 <= fileManagerLocalIndex && fileManagerLocalIndex < fileManagerLocalBranch->localNumberOfMeshElements))
    {
      CkError("ERROR in MeshElement::handleInitialize, element %d: initialization information not available from local file manager.\n", thisIndex);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  if (!error)
    {
      channelProxy     = channelProxyInit;
      fileManagerProxy = fileManagerProxyInit;
      
      if (NULL != fileManagerLocalBranch->meshVertexX)
        {
          for (edge = 0; edge < meshNeighborsSize; edge++)
            {
              vertexX[edge] = fileManagerLocalBranch->meshVertexX[fileManagerLocalIndex][edge];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: vertexX initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshVertexY)
        {
          for (edge = 0; edge < meshNeighborsSize; edge++)
            {
              vertexY[edge] = fileManagerLocalBranch->meshVertexY[fileManagerLocalIndex][edge];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: vertexY initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshElementX)
        {
          elementX = fileManagerLocalBranch->meshElementX[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: elementX initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshElementY)
        {
          elementY = fileManagerLocalBranch->meshElementY[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: elementY initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshElementZSurface)
        {
          elementZSurface = fileManagerLocalBranch->meshElementZSurface[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: elementZSurface initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshElementZBedrock)
        {
          elementZBedrock = fileManagerLocalBranch->meshElementZBedrock[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: elementZBedrock initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshElementArea)
        {
          elementArea = fileManagerLocalBranch->meshElementArea[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: elementArea initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshElementSlopeX)
        {
          elementSlopeX = fileManagerLocalBranch->meshElementSlopeX[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: elementSlopeX initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshElementSlopeY)
        {
          elementSlopeY = fileManagerLocalBranch->meshElementSlopeY[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: elementSlopeY initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshCatchment)
        {
          catchment = fileManagerLocalBranch->meshCatchment[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: catchment initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshVegetationType)
        {
          vegetationType = fileManagerLocalBranch->meshVegetationType[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: vegetationType initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSoilType)
        {
          soilType = fileManagerLocalBranch->meshSoilType[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: soilType initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshConductivity)
        {
          conductivity = fileManagerLocalBranch->meshConductivity[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: conductivity initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshPorosity)
        {
          porosity = fileManagerLocalBranch->meshPorosity[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: porosity initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshManningsN)
        {
          manningsN = fileManagerLocalBranch->meshManningsN[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: manningsN initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSurfacewaterDepth)
        {
          surfacewaterDepth = fileManagerLocalBranch->meshSurfacewaterDepth[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: surfacewaterDepth initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSurfacewaterError)
        {
          surfacewaterError = fileManagerLocalBranch->meshSurfacewaterError[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: surfacewaterError initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshGroundwaterHead)
        {
          groundwaterHead = fileManagerLocalBranch->meshGroundwaterHead[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: groundwaterHead initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshGroundwaterError)
        {
          groundwaterError = fileManagerLocalBranch->meshGroundwaterError[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: groundwaterError initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      precipitation            = 0.0;
      precipitationCumulative  = 0.0;
      evaporation              = 0.0;
      evaporationCumulative    = 0.0;
      surfacewaterInfiltration = 0.0;
      groundwaterRecharge      = 0.0;
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshFIceOld)
        {
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
            {
              evapoTranspirationState.fIceOld[ii] = fileManagerLocalBranch->meshFIceOld[fileManagerLocalIndex][ii];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: fIceOld initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshAlbOld)
        {
          evapoTranspirationState.albOld = fileManagerLocalBranch->meshAlbOld[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: albOld initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSnEqvO)
        {
          evapoTranspirationState.snEqvO = fileManagerLocalBranch->meshSnEqvO[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: snEqvO initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshStc)
        {
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
            {
              evapoTranspirationState.stc[ii] = fileManagerLocalBranch->meshStc[fileManagerLocalIndex][ii];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: stc initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshTah)
        {
          evapoTranspirationState.tah = fileManagerLocalBranch->meshTah[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: tah initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshEah)
        {
          evapoTranspirationState.eah = fileManagerLocalBranch->meshEah[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: eah initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshFWet)
        {
          evapoTranspirationState.fWet = fileManagerLocalBranch->meshFWet[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: fWet initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshCanLiq)
        {
          evapoTranspirationState.canLiq = fileManagerLocalBranch->meshCanLiq[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: canLiq initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshCanIce)
        {
          evapoTranspirationState.canIce = fileManagerLocalBranch->meshCanIce[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: canIce initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshTv)
        {
          evapoTranspirationState.tv = fileManagerLocalBranch->meshTv[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: tv initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshTg)
        {
          evapoTranspirationState.tg = fileManagerLocalBranch->meshTg[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: tg initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshISnow)
        {
          evapoTranspirationState.iSnow = fileManagerLocalBranch->meshISnow[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: iSnow initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshZSnso)
        {
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
            {
              evapoTranspirationState.zSnso[ii] = fileManagerLocalBranch->meshZSnso[fileManagerLocalIndex][ii];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: zSnso initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSnowH)
        {
          evapoTranspirationState.snowH = fileManagerLocalBranch->meshSnowH[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: snowH initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSnEqv)
        {
          evapoTranspirationState.snEqv = fileManagerLocalBranch->meshSnEqv[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: snEqv initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSnIce)
        {
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
            {
              evapoTranspirationState.snIce[ii] = fileManagerLocalBranch->meshSnIce[fileManagerLocalIndex][ii];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: snIce initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSnLiq)
        {
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
            {
              evapoTranspirationState.snLiq[ii] = fileManagerLocalBranch->meshSnLiq[fileManagerLocalIndex][ii];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: snLiq initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshLfMass)
        {
          evapoTranspirationState.lfMass = fileManagerLocalBranch->meshLfMass[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: lfMass initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshRtMass)
        {
          evapoTranspirationState.rtMass = fileManagerLocalBranch->meshRtMass[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: rtMass initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshStMass)
        {
          evapoTranspirationState.stMass = fileManagerLocalBranch->meshStMass[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: stMass initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshWood)
        {
          evapoTranspirationState.wood = fileManagerLocalBranch->meshWood[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: wood initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshStblCp)
        {
          evapoTranspirationState.stblCp = fileManagerLocalBranch->meshStblCp[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: stblCp initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshFastCp)
        {
          evapoTranspirationState.fastCp = fileManagerLocalBranch->meshFastCp[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: fastCp initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshLai)
        {
          evapoTranspirationState.lai = fileManagerLocalBranch->meshLai[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: lai initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshSai)
        {
          evapoTranspirationState.sai = fileManagerLocalBranch->meshSai[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: sai initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshCm)
        {
          evapoTranspirationState.cm = fileManagerLocalBranch->meshCm[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: cm initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshCh)
        {
          evapoTranspirationState.ch = fileManagerLocalBranch->meshCh[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: ch initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshTauss)
        {
          evapoTranspirationState.tauss = fileManagerLocalBranch->meshTauss[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: tauss initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshDeepRech)
        {
          evapoTranspirationState.deepRech = fileManagerLocalBranch->meshDeepRech[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: deepRech initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshRech)
        {
          evapoTranspirationState.rech = fileManagerLocalBranch->meshRech[fileManagerLocalIndex];
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: rech initialization information not available from local file manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // FIXME, wencong, initialize gar_domain here ?? Use some fake numbers for testing.
  if (!error)
    {
      // gar_parameters_alloc(&parameters, num_bins, conductivity, porosity, residual_saturation, van_genutchen,  vg_alpha, vg_n, bc_lambda, bc_psib);
      error = gar_parameters_alloc(&garParameters, 10, conductivity, porosity, 0.01, false,  0.01, 1.2, 0.5, 0.2);
    }
  
  if (!error)
    {
      // gar_domain_alloc(&domain, parameters, layer_top_depth, layer_bottom_depth, yes_groundwater, initial_water_content, yes_groundwater, water_table);
      error = gar_domain_alloc(&garDomain, garParameters, 0.0, elementZSurface - elementZBedrock, true, 0.2, true, elementZSurface - groundwaterHead);
    }
  // End of gar_domain initialization.
    
  // Forcing data will be initialized when the sdag code forces the object to receive a forcing data message before processing any timesteps.
  
  if (!error)
    {
      groundwaterDone  = true;
      infiltrationDone = true;
      surfacewaterDone = true;
      dt               = 1.0;
      dtNew            = 1.0;
      
      if (NULL != fileManagerLocalBranch->meshMeshNeighbors && NULL != fileManagerLocalBranch->meshMeshNeighborsChannelEdge)
        {
          for (edge = 0; !error && edge < meshNeighborsSize; edge++)
            {
              meshNeighbors[edge]                           = fileManagerLocalBranch->meshMeshNeighbors[fileManagerLocalIndex][edge];
              meshNeighborsChannelEdge[edge]                = fileManagerLocalBranch->meshMeshNeighborsChannelEdge[fileManagerLocalIndex][edge];
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
                  meshNeighborsX[edge]              = 0.0;
                  meshNeighborsY[edge]              = 0.0;
                  meshNeighborsZSurface[edge]       = 0.0;
                  meshNeighborsZBedrock[edge]       = 0.0;
                  meshNeighborsArea[edge]           = 1.0;
                  meshNeighborsConductivity[edge]   = 1.0;
                  meshNeighborsManningsN[edge]      = 1.0;
                }
              else if (0 <= meshNeighbors[edge] && meshNeighbors[edge] < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)
                {
                  meshNeighborsInitialized[edge] = false;
                  
                  thisProxy[meshNeighbors[edge]].initializeMeshNeighbor(thisIndex, edge, elementX, elementY, elementZSurface, elementZBedrock, elementArea,
                                                                        conductivity, manningsN);
                }
              else
                {
                  // We have to finish initialization before checking the invariant so the invariant hasn't been checked yet so we have to check this here.
                  CkError("ERROR in MeshElement::handleInitialize, element %d, edge %d: meshNeighbors must be a boundary condition code or a valid "
                          "array index.\n", thisIndex, edge);
                  error = true;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: meshNeighbors initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshMeshNeighborsEdgeLength)
        {
          for (edge = 0; edge < meshNeighborsSize; edge++)
            {
              meshNeighborsEdgeLength[edge] = fileManagerLocalBranch->meshMeshNeighborsEdgeLength[fileManagerLocalIndex][edge];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: meshNeighborsEdgeLength initialization information not available from local file "
                  "manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshMeshNeighborsEdgeNormalX)
        {
          for (edge = 0; edge < meshNeighborsSize; edge++)
            {
              meshNeighborsEdgeNormalX[edge] = fileManagerLocalBranch->meshMeshNeighborsEdgeNormalX[fileManagerLocalIndex][edge];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: meshNeighborsEdgeNormalX initialization information not available from local file "
                  "manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshMeshNeighborsEdgeNormalY)
        {
          for (edge = 0; edge < meshNeighborsSize; edge++)
            {
              meshNeighborsEdgeNormalY[edge] = fileManagerLocalBranch->meshMeshNeighborsEdgeNormalY[fileManagerLocalIndex][edge];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: meshNeighborsEdgeNormalY initialization information not available from local file "
                  "manager.\n", thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshChannelNeighbors)
        {
          for (edge = 0; !error && edge < channelNeighborsSize; edge++)
            {
              channelNeighbors[edge]                           = fileManagerLocalBranch->meshChannelNeighbors[fileManagerLocalIndex][edge];
              channelNeighborsInvariantChecked[edge]           = true;
              channelNeighborsSurfacewaterFlowRateReady[edge]  = FLOW_RATE_LIMITING_CHECK_DONE;
              channelNeighborsSurfacewaterFlowRate[edge]       = 0.0;
              channelNeighborsSurfacewaterCumulativeFlow[edge] = 0.0;
              channelNeighborsGroundwaterFlowRateReady[edge]   = FLOW_RATE_LIMITING_CHECK_DONE;
              channelNeighborsGroundwaterFlowRate[edge]        = 0.0;
              channelNeighborsGroundwaterCumulativeFlow[edge]  = 0.0;
              
              if (isBoundary(channelNeighbors[edge]))
                {
                  channelNeighborsInitialized[edge] = true;
                  
                  // Unused, but initialize for completeness.
                  channelNeighborsReciprocalEdge[edge]  = 0;
                  channelNeighborsInteraction[edge]     = BOTH_CALCULATE_FLOW_RATE;
                  channelNeighborsZBank[edge]           = 0.0;
                  channelNeighborsZBed[edge]            = 0.0;
                  channelNeighborsZOffset[edge]         = 0.0;
                  channelNeighborsBaseWidth[edge]       = 1.0;
                  channelNeighborsSideSlope[edge]       = 1.0;
                  channelNeighborsBedConductivity[edge] = 1.0;
                  channelNeighborsBedThickness[edge]    = 1.0;
                }
              else if (0 <= channelNeighbors[edge] && channelNeighbors[edge] < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)
                {
                  channelNeighborsInitialized[edge] = false;
                  
                  channelProxy[channelNeighbors[edge]].initializeMeshNeighbor(thisIndex, edge, vertexX, vertexY, elementX, elementY, elementZSurface,
                                                                              elementZBedrock, elementSlopeX, elementSlopeY);
                }
              else
                {
                  // We have to finish initialization before checking the invariant so the invariant hasn't been checked yet so we have to check this here.
                  CkError("ERROR in MeshElement::handleInitialize, element %d, edge %d: channelNeighbors must be a boundary condition code or a valid "
                          "array index.\n", thisIndex, edge);
                  error = true;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: channelNeighbors initialization information not available from local file manager.\n",
                  thisIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  if (!error)
    {
      if (NULL != fileManagerLocalBranch->meshChannelNeighborsEdgeLength)
        {
          for (edge = 0; edge < channelNeighborsSize; edge++)
            {
              channelNeighborsEdgeLength[edge] = fileManagerLocalBranch->meshChannelNeighborsEdgeLength[fileManagerLocalIndex][edge];
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          CkError("ERROR in MeshElement::handleInitialize, element %d: channelNeighborsEdgeLength initialization information not available from local file "
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

void MeshElement::handleInitializeMeshNeighbor(int neighbor, int neighborReciprocalEdge, double neighborX, double neighborY, double neighborZSurface,
                                               double neighborZBedrock, double neighborArea, double neighborConductivity, double neighborManningsN)
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
      CkError("ERROR in MeshElement::handleInitializeMeshNeighbor, element %d: received an initialization message from element %d that is not my neighbor.\n",
              thisIndex, neighbor);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  if (!error)
    {
      meshNeighborsInitialized[edge]    = true;
      meshNeighborsReciprocalEdge[edge] = neighborReciprocalEdge;
      meshNeighborsInteraction[edge]    = BOTH_CALCULATE_FLOW_RATE;
      meshNeighborsX[edge]              = neighborX;
      meshNeighborsY[edge]              = neighborY;
      meshNeighborsZSurface[edge]       = neighborZSurface;
      meshNeighborsZBedrock[edge]       = neighborZBedrock;
      meshNeighborsArea[edge]           = neighborArea;
      meshNeighborsConductivity[edge]   = neighborConductivity;
      meshNeighborsManningsN[edge]      = neighborManningsN;
    }
  
  // Error checking of initialization values is done in the invariant.
  
  if (error)
    {
      CkExit();
    }
}

void MeshElement::handleInitializeChannelNeighbor(int neighbor, int neighborReciprocalEdge, double neighborX, double neighborY, double neighborZBank,
                                                  double neighborZBed, ChannelTypeEnum neighborChannelType, double neighborBaseWidth, double neighborSideSlope,
                                                  double neighborBedConductivity, double neighborBedThickness)
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
      CkError("ERROR in MeshElement::handleInitializeChannelNeighbor, element %d: received an initialization message from element %d that is not my "
              "neighbor.\n", thisIndex, neighbor);
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
      channelNeighborsZOffset[edge]         = calculateZOffset(thisIndex, vertexX, vertexY, elementX, elementY, elementZSurface, elementSlopeX, elementSlopeY,
                                                               neighbor, neighborX, neighborY, neighborZBank, neighborChannelType);
      channelNeighborsBaseWidth[edge]       = neighborBaseWidth;
      channelNeighborsSideSlope[edge]       = neighborSideSlope;
      channelNeighborsBedConductivity[edge] = neighborBedConductivity;
      channelNeighborsBedThickness[edge]    = neighborBedThickness;
    }
  
  // Error checking of initialization values is done in the invariant.
  
  if (error)
    {
      CkExit();
    }
}

void MeshElement::handleForcingDataMessage(float atmosphereLayerThicknessNew, float shadedFractionNew, float shadedFractionMaximumNew,
                                           float surfaceTemperatureNew, float surfacePressureNew, float atomsphereLayerPressureNew, float eastWindSpeedNew,
                                           float northWindSpeedNew, float atmosphereLayerMixingRatioNew, float cloudMixingRatioNew,
                                           float shortWaveRadiationDownNew, float longWaveRadiationDownNew, float precipitationRateNew,
                                           float soilBottomTemperatureNew, float planetaryBoundaryLayerHeightNew)
{
  // FIXME error checking on inputs.
  
  atmosphereLayerThickness     = atmosphereLayerThicknessNew;
  shadedFraction               = shadedFractionNew;
  shadedFractionMaximum        = shadedFractionMaximumNew;
  surfaceTemperature           = surfaceTemperatureNew;
  surfacePressure              = surfacePressureNew;
  atomsphereLayerPressure      = atomsphereLayerPressureNew;
  eastWindSpeed                = eastWindSpeedNew;
  northWindSpeed               = northWindSpeedNew;
  atmosphereLayerMixingRatio   = atmosphereLayerMixingRatioNew;
  cloudMixingRatio             = cloudMixingRatioNew;
  shortWaveRadiationDown       = shortWaveRadiationDownNew;
  longWaveRadiationDown        = longWaveRadiationDownNew;
  precipitationRate            = precipitationRateNew;
  soilBottomTemperature        = soilBottomTemperatureNew;
  planetaryBoundaryLayerHeight = planetaryBoundaryLayerHeightNew;
  
  contribute();
}

// Suppress warning enum value not handled in switch.
#pragma GCC diagnostic ignored "-Wswitch"
void MeshElement::handleDoTimestep(size_t iterationThisTimestep, double date, double dtThisTimestep)
{
  bool   error = false;                                    // Error flag.
  int    ii, edge;                                         // Loop counters.
  long   year;                                             // For calculating input to evapoTranspirationSoil.
  long   month;                                            // For calculating input to evapoTranspirationSoil.
  long   day;                                              // For calculating input to evapoTranspirationSoil.
  long   hour;                                             // For calculating input to evapoTranspirationSoil.
  long   minute;                                           // For calculating input to evapoTranspirationSoil.
  double second;                                           // For calculating input to evapoTranspirationSoil.
  double declinationOfSun;                                 // For calculating input to evapoTranspirationSoil in radians.
  double hourAngle;                                        // For calculating input to evapoTranspirationSoil in radians.
  double layerMiddleDepth;                                 // For calculating input to evapoTranspirationSoil.
  double distanceAboveWaterTable;                          // For calculating input to evapoTranspirationSoil.
  double relativeSaturation;                               // For calculating input to evapoTranspirationSoil.
  double latitude;                                         // Input to evapoTranspirationSoil in radians.
  double longitude;                                        // Input to evapoTranspirationSoil in radians.
  int    yearlen;                                          // Input to evapoTranspirationSoil in days.
  float  julian;                                           // Input to evapoTranspirationSoil in days.
  float  cosZ;                                             // Input to evapoTranspirationSoil, unitless.
  float  smcEq[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS]; // Input to evapoTranspirationSoil.
  float  sh2o[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];  // Input to evapoTranspirationSoil.
  float  smc[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];   // Input to evapoTranspirationSoil.
  float  surfacewaterAdd;                                  // Output of evapoTranspirationSoil.
  float  evaporationFromCanopy;                            // Output of evapoTranspirationSoil.
  float  evaporationFromSnow;                              // Output of evapoTranspirationSoil.
  float  evaporationFromGround;                            // Output of evapoTranspirationSoil.
  float  transpiration;                                    // Output of evapoTranspirationSoil.
  float  waterError;                                       // Output of evapoTranspirationSoil.
  double unsatisfiedEvaporation;                           // Unsatisfied evaporation in meters of water.  Positive means water evaporated off of the ground.
                                                           // Negative means water condensed on to the ground.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtThisTimestep))
    {
      CkError("ERROR in MeshElement::handleDoTimestep, element %d: dtThisTimestep must be greater than zero.\n", thisIndex);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Initialize sequencing and timestep information.
      groundwaterDone  = false;
      infiltrationDone = false;
      surfacewaterDone = false;
      dt               = dtThisTimestep;
      dtNew            = 2.0 * dt;

      // Do point processes for rainfall, snowmelt, and evapo-transpiration.

      // Calculate yearlen, julian, and cosZ from absolute time.
      julianToGregorian(date, &year, &month, &day, &hour, &minute, &second);
      
      // Determine if it is a leap year.
      if (0 == year % 400)
        {
          yearlen = 366;
        }
      else if (0 == year % 100)
        {
          yearlen = 365;
        }
      else if (0 == year % 4)
        {
          yearlen = 366;
        }
      else
        {
          yearlen = 365;
        }
      
      // Calculate the ordinal day of the year by subtracting the Julian date of Jan 1 beginning midnight.
      julian = date - gregorianToJulian(year, 1, 1, 0, 0, 0);
      
      // Calculate cosZ.
      latitude         = (elementY - FALSE_NORTHING) / POLAR_RADIUS_OF_EARTH;
      longitude        = CENTRAL_MERIDIAN + (elementX - FALSE_EASTING) / (POLAR_RADIUS_OF_EARTH * cos(latitude));
      declinationOfSun = -23.44 * M_PI / 180.0 * cos(2 * M_PI * (julian + 10) / yearlen);
      hourAngle        = (date - gregorianToJulian(year, month, day, 12, 0, 0)) * 2 * M_PI + longitude; // This calculates the hour angle at longitude zero,
                                                                                                        // and then adds the longitude of this location.
      cosZ             = sin(latitude) * sin(declinationOfSun) + cos(latitude) * cos(declinationOfSun) * cos(hourAngle);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(-1.0f <= cosZ && 1.0f >= cosZ);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      if (0.0f > cosZ)
        {
          cosZ = 0.0f;
        }
      
      // FIXME get real values for soil moisture from infiltration state.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          layerMiddleDepth        = 0.5 * (evapoTranspirationState.zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] +
                                    evapoTranspirationState.zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1]); // Depth as a negative number.
          distanceAboveWaterTable = elementZSurface + layerMiddleDepth - groundwaterHead;

          if (0.1 > distanceAboveWaterTable)
            {
              relativeSaturation = 1.0;
            }
          else
            {
              relativeSaturation = 1.0 - (log10(distanceAboveWaterTable) + 1.0) * 0.3;

              if (0.0 > relativeSaturation)
                {
                  relativeSaturation = 0.0;
                }
            }

          smcEq[ii] = porosity * relativeSaturation;
          sh2o[ii]  = porosity * relativeSaturation;
          smc[ii]   = porosity * relativeSaturation;
        }

      error = evapoTranspirationSoil(vegetationType, soilType, latitude, yearlen, julian, cosZ, dt, sqrt(elementArea), atmosphereLayerThickness,
                                     shadedFraction, shadedFractionMaximum, smcEq, surfaceTemperature + ZERO_C_IN_KELVIN, surfacePressure,
                                     atomsphereLayerPressure, eastWindSpeed, northWindSpeed, atmosphereLayerMixingRatio, cloudMixingRatio,
                                     shortWaveRadiationDown, longWaveRadiationDown, precipitationRate * 1000.0f, soilBottomTemperature + ZERO_C_IN_KELVIN,
                                     planetaryBoundaryLayerHeight, sh2o, smc, elementZSurface - groundwaterHead, smc[3], &evapoTranspirationState,
                                     &surfacewaterAdd, &evaporationFromCanopy, &evaporationFromSnow, &evaporationFromGround, &transpiration, &waterError);
    }
  
  if (!error)
    {
      // Move water and record flows for precipitation and evaporation.
      surfacewaterDepth += surfacewaterAdd / 1000.0;
      precipitation      = precipitationRate * dt;
      evaporation        = -((double)evaporationFromCanopy + evaporationFromSnow) / 1000.0;
      
      // Take evaporationFromGround first from surfacewater, and then if there isn't enough surfacewater from groundwater.  If there isn't enough groundwater
      // print a warning and reduce the quantity of evaporation.
      unsatisfiedEvaporation = evaporationFromGround / 1000.0;
      
      if (surfacewaterDepth >= unsatisfiedEvaporation)
        {
          surfacewaterDepth -= unsatisfiedEvaporation;
          evaporation       -= unsatisfiedEvaporation;
        }
      else
        {
          unsatisfiedEvaporation -= surfacewaterDepth;
          evaporation            -= surfacewaterDepth;
          surfacewaterDepth       = 0.0;
          
          if (groundwaterHead - unsatisfiedEvaporation / porosity >= elementZBedrock)
            {
              groundwaterHead -= unsatisfiedEvaporation / porosity;
              evaporation     -= unsatisfiedEvaporation;
            }
          else
            {
              unsatisfiedEvaporation = (unsatisfiedEvaporation / porosity - groundwaterHead + elementZBedrock) * porosity;
              evaporation           -= (groundwaterHead - elementZBedrock) * porosity;
              groundwaterHead        = elementZBedrock;
              
              CkError("WARNING in MeshElement::handleDoTimestep, element %d: unsatisfied evaporation from ground of %le meters.\n",
                      thisIndex, unsatisfiedEvaporation);
            }
        }
      
      // Take transpiration first from groundwater, and then if there isn't enough groundwater from surfacewater.  If there isn't enough surfacewater
      // print a warning and reduce the quantity of transpiration.
      unsatisfiedEvaporation = transpiration / 1000.0;
      
      if (groundwaterHead - unsatisfiedEvaporation / porosity >= elementZBedrock)
        {
          groundwaterHead -= unsatisfiedEvaporation / porosity;
          evaporation     -= unsatisfiedEvaporation;
        }
      else
        {
          unsatisfiedEvaporation = (unsatisfiedEvaporation / porosity - groundwaterHead + elementZBedrock) * porosity;
          evaporation           -= (groundwaterHead - elementZBedrock) * porosity;
          groundwaterHead        = elementZBedrock;
          
          if (surfacewaterDepth >= unsatisfiedEvaporation)
            {
              surfacewaterDepth -= unsatisfiedEvaporation;
              evaporation       -= unsatisfiedEvaporation;
            }
          else
            {
              unsatisfiedEvaporation -= surfacewaterDepth;
              evaporation            -= surfacewaterDepth;
              surfacewaterDepth       = 0.0;
              
              CkError("WARNING in MeshElement::handleDoTimestep, element %d: unsatisfied transpiration of %le meters.\n", thisIndex, unsatisfiedEvaporation);
            }
        }
      
      // Record cumulative flows and water error.
      precipitationCumulative += precipitation;
      evaporationCumulative   += evaporation;
      surfacewaterError       += (waterError / 1000.0) * elementArea;
      
      // Initiate groundwater phase.
      for (edge = 0; edge < meshNeighborsSize; edge++)
        {
          // Set the flow rate state for this timestep to not ready.
          meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_NOT_READY;

          if (!isBoundary(meshNeighbors[edge]))
            {
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
                thisProxy[meshNeighbors[edge]].meshGroundwaterStateMessage(iterationThisTimestep, meshNeighborsReciprocalEdge[edge], surfacewaterDepth,
                                                                           groundwaterHead);
                break;
              }
            }
        }

      for (edge = 0; edge < channelNeighborsSize; edge++)
        {
          if (!isBoundary(channelNeighbors[edge]))
            {
              // Set the flow rate state for this timestep to not ready.
              channelNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_NOT_READY;

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
                channelProxy[channelNeighbors[edge]].meshGroundwaterStateMessage(iterationThisTimestep, channelNeighborsReciprocalEdge[edge], surfacewaterDepth,
                                                                                 groundwaterHead);
                break;
              }
            }
          else
            {
              // Only NOFLOW is a valid mesh channel neighbor boundary condition code.
              channelNeighborsGroundwaterFlowRate[edge]      = 0.0;
              channelNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
            }
        }

      // Send myself a message to calculate boundary conditions.
      // This is sent as a message instead of doing it immediately to better overlap communication and computation.
      thisProxy[thisIndex].calculateGroundwaterBoundaryConditionsMessage(iterationThisTimestep);
    }
  
  if (error)
    {
      CkExit();
    }
}
#pragma GCC diagnostic warning "-Wswitch"

void MeshElement::handleCalculateGroundwaterBoundaryConditionsMessage(size_t iterationThisMessage)
{
  bool error = false; // Error flag.
  int  edge;          // Loop counter.

  for (edge = 0; !error && edge < meshNeighborsSize; edge++)
    {
      if (isBoundary(meshNeighbors[edge]))
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(FLOW_RATE_NOT_READY == meshNeighborsGroundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          // Calculate groundwater flow rate.
          error = groundwaterMeshBoundaryFlowRate(&meshNeighborsGroundwaterFlowRate[edge], (BoundaryConditionEnum)meshNeighbors[edge],
                                                  meshNeighborsEdgeLength[edge], meshNeighborsEdgeNormalX[edge], meshNeighborsEdgeNormalY[edge],
                                                  elementZBedrock, elementArea, elementSlopeX, elementSlopeY, conductivity, groundwaterHead);
          
          if (!error)
            {
              // Outflows may need to be limited.  Inflows and noflows will not.
              if (0.0 < meshNeighborsGroundwaterFlowRate[edge])
                {
                  meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  meshNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
                }
            }
        }
    }
  
  if (!error)
    {
      // Calculate infiltration.  FIXME trivial infiltration, improve.
     
      // Calculate the amount that infiltrates.
      surfacewaterInfiltration = conductivity * dt; // Meters of water.
      if (surfacewaterInfiltration > surfacewaterDepth)
        {
          surfacewaterInfiltration = surfacewaterDepth;
        }
      
      // Infiltration goes instantly to groundwater.
      groundwaterRecharge = surfacewaterInfiltration;
      infiltrationDone    = true;
      
      // FIXME, Bob, I'm commenting out the GARTO infiltration in the version checked in to the repository.  Because the GARTO state isn't PUPed yet we can't
      // run with GARTO and load balancing.  Therefore, I'm leaving as the verison in the repository a version that can run on multiple processors with load
      // balancing.  Wencong, feel free to uncomment this for your testing, but I think we shouldn't uncomment it in the version checked in ot the repository
      // until it is fully working.
      // FIXME, wencong, testing GARTO infiltration.
      // gar_timestep(gar_domain* domain, double dt, double* surfacewater_depth, double water_table, double* groundwater_recharge)
      //groundwaterRecharge = 0.0;
      //error               = gar_timestep(garDomain, dt, &surfacewaterDepth, elementZSurface - groundwaterHead, &groundwaterRecharge);
      //infiltrationDone    = true;
  
      // Testing 1D soil temperature.
      // soil_temperature_timestep(double* soil_temperature, double* soil_depth_z, int num_elements, double input_heat_flux, double dt, 
                              //gar_domain* domain_gar, int infiltration_type)
      //error = soil_temperature_timestep(soil_temperature, soil_depth_z, num_elements_t, 0.5* (shortWaveRadiationDown + longWaveRadiationDown), dt, domain, 1);

      checkGroundwaterFlowRates(iterationThisMessage);
    }
  else
    {
      CkExit();
    }
}

// Suppress warning enum value not handled in switch.
#pragma GCC diagnostic ignored "-Wswitch"
void MeshElement::handleMeshGroundwaterStateMessage(size_t iterationThisMessage, int edge, double neighborSurfacewaterDepth,
                                                    double neighborGroundwaterHead)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleMeshGroundwaterStateMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      error = true;
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(meshNeighborsZSurface[edge] >= neighborGroundwaterHead))
    {
      CkError("ERROR in MeshElement::handleMeshGroundwaterStateMessage, element %d, edge %d: "
              "neighborGroundwaterHead must be less than or equal to neighborsZSurface.\n", thisIndex, edge);
      error = true;
    }

  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in MeshElement::handleMeshGroundwaterStateMessage, element %d, edge %d: "
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
          error = groundwaterMeshMeshFlowRate(&meshNeighborsGroundwaterFlowRate[edge], meshNeighborsEdgeLength[edge], elementX, elementY, elementZSurface,
                                              elementZBedrock, conductivity, surfacewaterDepth, groundwaterHead, meshNeighborsX[edge], meshNeighborsY[edge],
                                              meshNeighborsZSurface[edge], meshNeighborsZBedrock[edge], meshNeighborsConductivity[edge],
                                              neighborSurfacewaterDepth, neighborGroundwaterHead);
          
          
          
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
        thisProxy[meshNeighbors[edge]].meshGroundwaterFlowRateMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge],
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

void MeshElement::handleChannelGroundwaterStateMessage(size_t iterationThisMessage, int edge, double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleChannelGroundwaterStateMessage, element %d, edge %d: edge must be between zero and channelNeighborsSize.\n",
              thisIndex, edge);
      error = true;
    }

  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in MeshElement::handleChannelGroundwaterStateMessage, element %d, edge %d: "
              "neighborSurfacewaterDepth must be greater than or equal to zero.\n", thisIndex, edge);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // There is a race condition where a flow limited message can arrive before a state message.
      // In that case, the flow limited message already had the correct flow value and you can ignore the state message.
      if (FLOW_RATE_NOT_READY == channelNeighborsGroundwaterFlowRateReady[edge])
        {
          // Calculate groundwater flow rate.
          error = groundwaterMeshChannelFlowRate(&channelNeighborsGroundwaterFlowRate[edge], channelNeighborsEdgeLength[edge],
                                                 elementZSurface + channelNeighborsZOffset[edge], elementZBedrock + channelNeighborsZOffset[edge],
                                                 surfacewaterDepth, groundwaterHead + channelNeighborsZOffset[edge], channelNeighborsZBank[edge],
                                                 channelNeighborsZBed[edge], channelNeighborsBaseWidth[edge], channelNeighborsSideSlope[edge],
                                                 channelNeighborsBedConductivity[edge], channelNeighborsBedThickness[edge], neighborSurfacewaterDepth);
          
          if (!error)
            {
              // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
              if (0.0 != channelNeighborsGroundwaterFlowRate[edge])
                {
                  channelNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  channelNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
                }
            }
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      else
        {
          CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsGroundwaterFlowRateReady[edge]);
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  if (!error)
    {
      switch (channelNeighborsInteraction[edge])
      {
      case I_CALCULATE_FLOW_RATE:
        // Send flow message.
        channelProxy[channelNeighbors[edge]].meshGroundwaterFlowRateMessage(iterationThisMessage, channelNeighborsReciprocalEdge[edge],
                                                                            -channelNeighborsGroundwaterFlowRate[edge]);
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

void MeshElement::handleMeshGroundwaterFlowRateMessage(size_t iterationThisMessage, int edge, double edgeGroundwaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleMeshGroundwaterFlowRateMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
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

void MeshElement::handleChannelGroundwaterFlowRateMessage(size_t iterationThisMessage, int edge, double edgeGroundwaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleChannelGroundwaterFlowRateMessage, element %d, edge %d: edge must be between zero and channelNeighborsSize.\n",
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
  if (FLOW_RATE_NOT_READY == channelNeighborsGroundwaterFlowRateReady[edge])
    {
      // Save the received flow rate value.
      channelNeighborsGroundwaterFlowRate[edge] = edgeGroundwaterFlowRate;

      // Outflows may need to be limited.  Inflows may be limited by neighbor.  Noflows will not.
      if (0.0 != channelNeighborsGroundwaterFlowRate[edge])
        {
          channelNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
        }
      else
        {
          channelNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsGroundwaterFlowRateReady[edge]);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  checkGroundwaterFlowRates(iterationThisMessage);
}

void MeshElement::handleMeshGroundwaterFlowRateLimitedMessage(size_t iterationThisMessage, int edge, double edgeGroundwaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleMeshGroundwaterFlowRateLimitedMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      CkExit();
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeGroundwaterFlowRate &&
             (FLOW_RATE_NOT_READY == meshNeighborsGroundwaterFlowRateReady[edge] || edgeGroundwaterFlowRate >= meshNeighborsGroundwaterFlowRate[edge])))
    {
      CkError("ERROR in MeshElement::handleMeshGroundwaterFlowRateLimitedMessage, element %d, edge %d: "
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

void MeshElement::handleChannelGroundwaterFlowRateLimitedMessage(size_t iterationThisMessage, int edge, double edgeGroundwaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleChannelGroundwaterFlowRateLimitedMessage, element %d, edge %d: "
              "edge must be between zero and channelNeighborsSize.\n", thisIndex, edge);
      CkExit();
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeGroundwaterFlowRate &&
             (FLOW_RATE_NOT_READY == channelNeighborsGroundwaterFlowRateReady[edge] || edgeGroundwaterFlowRate >= channelNeighborsGroundwaterFlowRate[edge])))
    {
      CkError("ERROR in MeshElement::handleChannelGroundwaterFlowRateLimitedMessage, element %d, edge %d: "
              "A flow limiting message must be for an inflow and it must only reduce the magnitude of the flow.\n", thisIndex, edge);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(FLOW_RATE_NOT_READY == channelNeighborsGroundwaterFlowRateReady[edge] || FLOW_RATE_CALCULATED == channelNeighborsGroundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  // Save the received flow rate value.
  channelNeighborsGroundwaterFlowRate[edge] = edgeGroundwaterFlowRate;

  // The flow limiting check was done by neighbor.
  channelNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;

  checkGroundwaterFlowRates(iterationThisMessage);
}

void MeshElement::checkGroundwaterFlowRates(size_t iterationThisMessage)
{
  int    edge;                            // Loop counter.
  bool   allCalculated           = true;  // Whether all flows have been calculated.
  bool   outwardNotLimited       = false; // Whether at least one outward flow is not limited.
  double totalOutwardFlowRate    = 0.0;   // Sum of all outward flows.
  double outwardFlowRateFraction = 1.0;   // Fraction of all outward flows that can be satisfied.
  double groundwaterAvailable    = 0.0;   // Available groundwater in meters of water.
  bool   allLimited              = true;  // Whether infiltration and all flow limiting checks have been done.
  
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
  
  for (edge = 0; allCalculated && edge < channelNeighborsSize; edge++)
    {
      if (FLOW_RATE_NOT_READY == channelNeighborsGroundwaterFlowRateReady[edge])
        {
          allCalculated = false;
          allLimited    = false;
        }
      else if (FLOW_RATE_CALCULATED == channelNeighborsGroundwaterFlowRateReady[edge] && 0.0 < channelNeighborsGroundwaterFlowRate[edge])
        {
          outwardNotLimited     = true;
          totalOutwardFlowRate += channelNeighborsGroundwaterFlowRate[edge];
        }
    }
  
  if (allCalculated && outwardNotLimited)
    {
      // Check if total outward flow is greater than water available.
      if (groundwaterHead > elementZBedrock)
        {
          groundwaterAvailable = (groundwaterHead - elementZBedrock) * elementArea * porosity;
        }
      
      if (groundwaterAvailable < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = groundwaterAvailable / (totalOutwardFlowRate * dt);
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
                  thisProxy[meshNeighbors[edge]].meshGroundwaterFlowRateLimitedMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge],
                                                                                       -meshNeighborsGroundwaterFlowRate[edge]);
                }
            }
        }
      
      for (edge = 0; edge < channelNeighborsSize; edge++)
        {
          if (0.0 < channelNeighborsGroundwaterFlowRate[edge])
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(FLOW_RATE_CALCULATED == channelNeighborsGroundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              channelNeighborsGroundwaterFlowRate[edge]     *= outwardFlowRateFraction;
              channelNeighborsGroundwaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
              
              // Send flow limited message.
              if (!isBoundary(channelNeighbors[edge]))
                {
                  channelProxy[channelNeighbors[edge]].meshGroundwaterFlowRateLimitedMessage(iterationThisMessage, channelNeighborsReciprocalEdge[edge],
                                                                                             -channelNeighborsGroundwaterFlowRate[edge]);
                }
            }
        }
    }
  
  // If infiltration is done and the flow rate limiting check is done for all flow rates then we have the final value for all flow rates and can move
  // groundwater now.
  for (edge = 0; infiltrationDone && allLimited && edge < meshNeighborsSize; edge++)
    {
      allLimited = (FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsGroundwaterFlowRateReady[edge]);
    }
  
  for (edge = 0; infiltrationDone && allLimited && edge < channelNeighborsSize; edge++)
    {
      allLimited = (FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsGroundwaterFlowRateReady[edge]);
    }
  
  if (infiltrationDone && allLimited)
    {
      moveGroundwater(iterationThisMessage);
    }
}

// Suppress warning enum value not handled in switch.
#pragma GCC diagnostic ignored "-Wswitch"
void MeshElement::moveGroundwater(size_t iterationThisMessage)
{
  int    edge;   // Loop counter.
  double dtTemp; // Temporary variable for suggesting new timestep.
  
  // Remove infiltrated surfacewater.
  surfacewaterDepth -= surfacewaterInfiltration;
  
  // Calculate net groundwater recharge.
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsGroundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      groundwaterRecharge                          -= meshNeighborsGroundwaterFlowRate[edge] * dt / elementArea;
      meshNeighborsGroundwaterCumulativeFlow[edge] += meshNeighborsGroundwaterFlowRate[edge] * dt;
    }
  
  for (edge = 0; edge < channelNeighborsSize; edge++)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsGroundwaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      groundwaterRecharge                             -= channelNeighborsGroundwaterFlowRate[edge] * dt / elementArea;
      channelNeighborsGroundwaterCumulativeFlow[edge] += channelNeighborsGroundwaterFlowRate[edge] * dt;
    }
  
  // Update groundwaterHead and get or put water from the infiltration state.
  // FIXME currently there is no infiltration state and groundwater head also represents the presence of water.
  groundwaterHead += groundwaterRecharge / porosity;
  
  /* FIXME here's the old code for updating groundwater head
              // Resolve groundwater_recharge by moving groundwater_head and putting or taking water from the infiltration domain.
              groundwater_head[ii] += groundwater_recharge[ii] / infiltration_specific_yield(ii, mesh->elements_xyz[ii][3] - groundwater_head[ii]);

              // Cap groundwater_head at the surface.
              if (groundwater_head[ii] > mesh->elements_xyz[ii][3])
                {
                  groundwater_head[ii] = mesh->elements_xyz[ii][3];
                }

              if (epsilon_less(0.0, groundwater_recharge[ii]))
                {
                  // If there is excess water put it immediately into the groundwater front of the infiltration domain.
                  infiltration_add_groundwater(ii, &groundwater_recharge[ii]);

                  if (epsilon_less(0.0, groundwater_recharge[ii]))
                    {
                      // Not all of the water could fit in to the infiltration domain because the domain is full.
                      // Put the excess water on the surface and the groundwater head moves up to the surface.
                      // The real groundwater head is at the top of the surfacewater, but we set the variable to be at the surface and inside
                      // groundwater_timestep we add surfacewater_depth because if we set groundwater_head to be at the top of the surfacewater
                      // we would need to update it any time the value of surfacewater_depth changed.
                      surfacewater_depth[ii]   += groundwater_recharge[ii];
                      groundwater_recharge[ii]  = 0.0;
                      groundwater_head[ii]      = mesh->elements_xyz[ii][3];
                    }
                }
              else if (epsilon_greater(0.0, groundwater_recharge[ii]))
                {
                  // If there is a water deficit take it immediately from the groundwater front of the infiltration domain.
                  infiltration_take_groundwater(ii, mesh->elements_xyz[ii][3] - groundwater_head[ii], &groundwater_recharge[ii]);

                  // If there is still a deficit leave it to be resolved next time.  The water table will drop further allowing us to get more water out.
                }
                
                
                
                
          // If use_groundwater is FALSE then treat the water put into groundwater_recharge as if it passed out of a groundwater outflow boundary.
          for (ii = 1; ii <= mesh->num_elements; ii++)
            {
              groundwater_volume_out += groundwater_recharge[ii] * mesh->elements_area[ii];
              groundwater_recharge[ii] = 0.0;
            }
   */
  
  // FIXME, here is the new code for updating groundwater head with GARTO infiltration.
  /*
  groundwaterHead += groundwaterRecharge / porosity; // Or groundwaterRecharge / gar_specific_yield(garDomain, elementZSurface - groundwaterHead);
  // Cap groundwater_head at the surface.
  if (groundwaterHead > elementZSurface)
    {
      groundwaterHead = elementZSurface;
    }
  
  if (epsilonLess(0.0, groundwaterRecharge))
    {
      // If there is excess water put it immediately into the groundwater front of the infiltration domain.
      gar_add_groundwater(garDomain, &groundwaterRecharge);
      if (epsilonLess(0.0, groundwaterRecharge))
        {
          // Not all of the water could fit in to the infiltration domain because the domain is full.
          // Put the excess water on the surface and the groundwater head moves up to the surface.
          // The real groundwater head is at the top of the surfacewater, but we set the variable to be at the surface and inside
          // groundwater_timestep we add surfacewater_depth because if we set groundwater_head to be at the top of the surfacewater
          // we would need to update it any time the value of surfacewater_depth changed.
          surfacewaterDepth    += groundwaterRecharge;
          groundwaterRecharge   = 0.0;
           groundwaterHead      = elementZSurface;
       }
    }
  else if (epsilonGreater(0.0, groundwaterRecharge))
    {
      // If there is a water deficit take it immediately from the groundwater front of the infiltration domain.
      gar_take_groundwater(garDomain, elementZSurface - groundwaterHead, &groundwaterRecharge);
      // If there is still a deficit leave it to be resolved next time.  The water table will drop further allowing us to get more water out.
    }
  // End of updating groundwater head with GARTO infiltration.
  */
  
  if (elementZBedrock > groundwaterHead)
    {
      // Even though we are limiting outward flows, groundwaterhead can go below bedrock due to roundoff error.
      // FIXME should we try to take the water back from the error accumulator later?
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonEqual(elementZBedrock, groundwaterHead));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      groundwaterError += (elementZBedrock - groundwaterHead) * elementArea * porosity;
      groundwaterHead   = elementZBedrock;
    }
  else if (elementZSurface < groundwaterHead)
    {
      // If groundwater rises above the surface put the excess in the surfacewater.
      surfacewaterDepth += (groundwaterHead - elementZSurface) * porosity;
      groundwaterHead    = elementZSurface;
    }
  
  // Suggest new timestep.
  if (elementZBedrock < groundwaterHead)
    {
      dtTemp = 0.2 * porosity * elementArea / (4.0 * conductivity * (groundwaterHead - elementZBedrock));

      if (dtNew > dtTemp)
        {
          dtNew = dtTemp;
        }
    }
  
  groundwaterDone = true;
  
  // Initiate surfacewater phase.
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
      // Set the flow rate state for this timestep to not ready.
      meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_NOT_READY;
      
      if (!(isBoundary(meshNeighbors[edge]) || meshNeighborsChannelEdge[edge]))
        {
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
            thisProxy[meshNeighbors[edge]].meshSurfacewaterStateMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge], surfacewaterDepth);
            break;
          }
        }
    }
  
  for (edge = 0; edge < channelNeighborsSize; edge++)
    {
      if (!isBoundary(channelNeighbors[edge]))
        {
          // Set the flow rate state for this timestep to not ready.
          channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_NOT_READY;
          
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
            channelProxy[channelNeighbors[edge]].meshSurfacewaterStateMessage(iterationThisMessage, channelNeighborsReciprocalEdge[edge], surfacewaterDepth);
            break;
          }
        }
      else
        {
          // Only NOFLOW is a valid mesh channel neighbor boundary condition code.
          channelNeighborsSurfacewaterFlowRate[edge]      = 0.0;
          channelNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
        }
    }
  
  // Send myself a message to calculate boundary conditions.
  // This is sent as a message instead of doing it immediately to better overlap communication and computation.
  thisProxy[thisIndex].calculateSurfacewaterBoundaryConditionsMessage(iterationThisMessage);
}
#pragma GCC diagnostic warning "-Wswitch"

void MeshElement::handleCalculateSurfacewaterBoundaryConditionsMessage(size_t iterationThisMessage)
{
  bool                  error = false; // Error flag.
  int                   edge;          // Loop counter.
  BoundaryConditionEnum boundary;      // Boundary condition code to pass to the function to calculate flow rate.

  for (edge = 0; !error && edge < meshNeighborsSize; edge++)
    {
      if (isBoundary(meshNeighbors[edge]) || meshNeighborsChannelEdge[edge])
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(FLOW_RATE_NOT_READY == meshNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          if (isBoundary(meshNeighbors[edge]))
            {
              boundary = (BoundaryConditionEnum)meshNeighbors[edge];
            }
          else
            {
              // Channel edge.  No direct flow to/from mesh neighbor on other side of channel.
              boundary = NOFLOW;
            }
          
          // Calculate surfacewater flow rate.
          // FIXME figure out what to do about inflow boundary velocity and height
          error = surfacewaterMeshBoundaryFlowRate(&meshNeighborsSurfacewaterFlowRate[edge], boundary, 0.0, 0.0, 0.0, meshNeighborsEdgeLength[edge],
                                                   meshNeighborsEdgeNormalX[edge], meshNeighborsEdgeNormalY[edge], surfacewaterDepth);
          
          if (!error)
            {
              // Outflows may need to be limited.  Inflows and noflows will not.
              if (0.0 < meshNeighborsSurfacewaterFlowRate[edge])
                {
                  meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_CALCULATED;
                }
              else
                {
                  meshNeighborsSurfacewaterFlowRateReady[edge] = FLOW_RATE_LIMITING_CHECK_DONE;
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
void MeshElement::handleMeshSurfacewaterStateMessage(size_t iterationThisMessage, int edge, double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleMeshSurfacewaterStateMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
              thisIndex, edge);
      error = true;
    }

  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in MeshElement::handleMeshSurfacewaterStateMessage, element %d, edge %d: "
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
          error = surfacewaterMeshMeshFlowRate(&meshNeighborsSurfacewaterFlowRate[edge], &dtNew, meshNeighborsEdgeLength[edge], elementX, elementY,
                                               elementZSurface, elementArea, manningsN, surfacewaterDepth, meshNeighborsX[edge], meshNeighborsY[edge],
                                               meshNeighborsZSurface[edge], meshNeighborsArea[edge], meshNeighborsManningsN[edge], neighborSurfacewaterDepth);
          
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
        thisProxy[meshNeighbors[edge]].meshSurfacewaterFlowRateMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge],
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

void MeshElement::handleChannelSurfacewaterStateMessage(size_t iterationThisMessage, int edge, double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleChannelSurfacewaterStateMessage, element %d, edge %d: edge must be between zero and channelNeighborsSize.\n",
              thisIndex, edge);
      error = true;
    }

  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in MeshElement::handleChannelSurfacewaterStateMessage, element %d, edge %d: "
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
          error = surfacewaterMeshChannelFlowRate(&channelNeighborsSurfacewaterFlowRate[edge], channelNeighborsEdgeLength[edge],
                                                  elementZSurface + channelNeighborsZOffset[edge], surfacewaterDepth, channelNeighborsZBank[edge],
                                                  channelNeighborsZBed[edge], neighborSurfacewaterDepth);
          
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
        channelProxy[channelNeighbors[edge]].meshSurfacewaterFlowRateMessage(iterationThisMessage, channelNeighborsReciprocalEdge[edge],
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
#pragma GCC diagnostic warning "-Wswitch"

void MeshElement::handleMeshSurfacewaterFlowRateMessage(size_t iterationThisMessage, int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleMeshSurfacewaterFlowRateMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
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

void MeshElement::handleChannelSurfacewaterFlowRateMessage(size_t iterationThisMessage, int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleChannelSurfacewaterFlowRateMessage, element %d, edge %d: edge must be between zero and channelNeighborsSize.\n",
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

void MeshElement::handleMeshSurfacewaterFlowRateLimitedMessage(size_t iterationThisMessage, int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && meshNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleMeshSurfacewaterFlowRateLimitedMessage, element %d, edge %d: edge must be between zero and meshNeighborsSize.\n",
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

void MeshElement::handleChannelSurfacewaterFlowRateLimitedMessage(size_t iterationThisMessage, int edge, double edgeSurfacewaterFlowRate)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= edge && channelNeighborsSize > edge))
    {
      CkError("ERROR in MeshElement::handleChannelSurfacewaterFlowRateLimitedMessage, element %d, edge %d: edge must be between zero and "
              "channelNeighborsSize.\n", thisIndex, edge);
      CkExit();
    }
  // Must be else if because cannot use edge unless it passes the previous test.
  else if (!(0.0 >= edgeSurfacewaterFlowRate &&
             (FLOW_RATE_NOT_READY == channelNeighborsSurfacewaterFlowRateReady[edge] ||
              edgeSurfacewaterFlowRate >= channelNeighborsSurfacewaterFlowRate[edge])))
    {
      CkError("ERROR in MeshElement::handleChannelSurfacewaterFlowRateLimitedMessage, element %d, edge %d: "
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

void MeshElement::checkSurfacewaterFlowRates(size_t iterationThisMessage)
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
  
  if (allCalculated && outwardNotLimited)
    {
      // Check if total outward flow is greater than water available.
      if (surfacewaterDepth * elementArea < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = (surfacewaterDepth * elementArea) / (totalOutwardFlowRate * dt);
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
      // FIXME Our code doesn't assure this.
      //CkAssert(epsilonEqual(1.0, outwardFlowRateFraction));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // Limit outward flows.
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
              if (!(isBoundary(meshNeighbors[edge]) || meshNeighborsChannelEdge[edge]))
                {
                  thisProxy[meshNeighbors[edge]].meshSurfacewaterFlowRateLimitedMessage(iterationThisMessage, meshNeighborsReciprocalEdge[edge],
                                                                                        -meshNeighborsSurfacewaterFlowRate[edge]);
                }
            }
        }
      
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
                  channelProxy[channelNeighbors[edge]].meshSurfacewaterFlowRateLimitedMessage(iterationThisMessage, channelNeighborsReciprocalEdge[edge],
                                                                                              -channelNeighborsSurfacewaterFlowRate[edge]);
                }
            }
        }
    }
  
  // If the flow rate limiting check is done for all flow rates then we have the final value for all flow rates and can move surfacewater now.
  for (edge = 0; allLimited && edge < meshNeighborsSize; edge++)
    {
      allLimited = (FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsSurfacewaterFlowRateReady[edge]);
    }
  
  for (edge = 0; allLimited && edge < channelNeighborsSize; edge++)
    {
      allLimited = (FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsSurfacewaterFlowRateReady[edge]);
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
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      surfacewaterDepth                             -= meshNeighborsSurfacewaterFlowRate[edge] * dt / elementArea;
      meshNeighborsSurfacewaterCumulativeFlow[edge] += meshNeighborsSurfacewaterFlowRate[edge] * dt;
    }
  
  for (edge = 0; edge < channelNeighborsSize; edge++)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsSurfacewaterFlowRateReady[edge]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      surfacewaterDepth                                -= channelNeighborsSurfacewaterFlowRate[edge] * dt / elementArea;
      channelNeighborsSurfacewaterCumulativeFlow[edge] += channelNeighborsSurfacewaterFlowRate[edge] * dt;
    }
  
  // Even though we are limiting outward flows, surfacewaterDepth can go below zero due to roundoff error.
  // FIXME should we try to take the water back from the error accumulator later?
  if (0.0 > surfacewaterDepth)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonEqual(0.0, surfacewaterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      surfacewaterError -= surfacewaterDepth * elementArea;
      surfacewaterDepth  = 0.0;
    }
  
  surfacewaterDone = true;
  
  // Perform min reduction on dtNew.  This also serves as a barrier at the end of the timestep.
  contribute(sizeof(double), &dtNew, CkReduction::min_double);
}

void MeshElement::handleCheckInvariant()
{
  bool error = false; // Error flag.
  int  edge;          // Loop counter.
  
  if (!(elementZSurface >= elementZBedrock))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: elementZSurface must be greater than or equal to elementZBedrock.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: elementArea must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(1 <= vegetationType && 27 >= vegetationType))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: USGS vegetation type must be greater than or equal to 1 and less than or equal to "
              "27.\n", thisIndex);
      error = true;
    }
  
  if (!(1 <= soilType && 19 >= soilType))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: STAS soil type must be greater than or equal to 1 and less than or equal to 19.\n",
              thisIndex);
      error = true;
    }
  
  if (!(0.0 < conductivity))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: conductivity must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < porosity))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: porosity must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < manningsN))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: manningsN must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: surfacewaterDepth must be greater than or equal to zero.\n", thisIndex);
      error = true;
    }
  
  if (!(elementZSurface >= groundwaterHead))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: groundwaterHead must be less than or equal to elementZSurface.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 <= precipitation))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: precipitation must be greater than or equal to zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 <= precipitationCumulative))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: precipitationCumulative must be greater than or equal to zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 <= surfacewaterInfiltration))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: surfacewaterInfiltration must be greater than or equal to zero.\n", thisIndex);
      error = true;
    }
  
  if (checkEvapoTranspirationStateStructInvariant(&evapoTranspirationState))
    {
      error = true;
    }
  
  // FIXME check invariant on GARTO data
  // FIXME check invariant on forcing data
  
  if (!groundwaterDone)
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: groundwaterDone must be true when not processing a timestep.\n", thisIndex);
      error = true;
    }
  
  if (!infiltrationDone)
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: infiltrationDone must be true when not processing a timestep.\n", thisIndex);
      error = true;
    }
  
  if (!surfacewaterDone)
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: surfacewaterDone must be true when not processing a timestep.\n", thisIndex);
      error = true;
    }
    
  if (!(0.0 < dt))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: dt must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < dtNew))
    {
      CkError("ERROR in MeshElement::handleCheckInvariant, element %d: dtNew must be greater than zero.\n", thisIndex);
      error = true;
    }
  
  for (edge = 0; edge < meshNeighborsSize; edge++)
    {
      if (!(isBoundary(meshNeighbors[edge]) || (0 <= meshNeighbors[edge] &&
                                                meshNeighbors[edge] < fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: meshNeighbors must be a boundary condition code or a valid array index.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(0 <= meshNeighborsReciprocalEdge[edge] && meshNeighborsReciprocalEdge[edge] < meshNeighborsSize))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: meshNeighborsReciprocalEdge must be a valid array index.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(I_CALCULATE_FLOW_RATE    == meshNeighborsInteraction[edge] || NEIGHBOR_CALCULATES_FLOW_RATE == meshNeighborsInteraction[edge] ||
            BOTH_CALCULATE_FLOW_RATE == meshNeighborsInteraction[edge]))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: meshNeighborsInteraction must be a valid enum value.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!meshNeighborsInitialized[edge])
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: meshNeighborsInitialized must be true before checking invariant.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(0.0 < meshNeighborsEdgeLength[edge]))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: meshNeighborsEdgeLength must be greater than zero.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(epsilonEqual(1.0, meshNeighborsEdgeNormalX[edge] * meshNeighborsEdgeNormalX[edge] +
                              meshNeighborsEdgeNormalY[edge] * meshNeighborsEdgeNormalY[edge])))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: meshNeighborsEdgeNormalX and meshNeighborsEdgeNormalY must make a unit "
                  "vector.\n", thisIndex, edge);
          error = true;
        }
      
      if (!(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsSurfacewaterFlowRateReady[edge]))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: meshNeighborsSurfacewaterFlowRateReady must be ready when not processing "
                  "a timestep.\n", thisIndex, edge);
          error = true;
        }
      
      if (!(FLOW_RATE_LIMITING_CHECK_DONE == meshNeighborsGroundwaterFlowRateReady[edge]))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: meshNeighborsGroundwaterFlowRateReady must be ready when not processing "
                  "a timestep.\n", thisIndex, edge);
          error = true;
        }
    }
  
  for (edge = 0; edge < channelNeighborsSize; edge++)
    {
      // FIXME add check that all channel neighbors have to be packed to the front of the array.
      if (!(NOFLOW == channelNeighbors[edge] || (0 <= channelNeighbors[edge] &&
                                                 channelNeighbors[edge] < fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: channelNeighbors must be NOFLOW or a valid array index.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(0 <= channelNeighborsReciprocalEdge[edge] && channelNeighborsReciprocalEdge[edge] < ChannelElement::meshNeighborsSize))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: channelNeighborsReciprocalEdge must be a valid array index.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(I_CALCULATE_FLOW_RATE    == channelNeighborsInteraction[edge] || NEIGHBOR_CALCULATES_FLOW_RATE == channelNeighborsInteraction[edge] ||
            BOTH_CALCULATE_FLOW_RATE == channelNeighborsInteraction[edge]))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: channelNeighborsInteraction must be a valid enum value.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!channelNeighborsInitialized[edge])
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: channelNeighborsInitialized must be true before checking invariant.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(0.0 < channelNeighborsEdgeLength[edge]))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: channelNeighborsEdgeLength must be greater than zero.\n",
                  thisIndex, edge);
          error = true;
        }
      
      if (!(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsSurfacewaterFlowRateReady[edge]))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: channelNeighborsSurfacewaterFlowRateReady must be ready when not "
                  "processing a timestep.\n", thisIndex, edge);
          error = true;
        }
      
      if (!(FLOW_RATE_LIMITING_CHECK_DONE == channelNeighborsGroundwaterFlowRateReady[edge]))
        {
          CkError("ERROR in MeshElement::handleCheckInvariant, element %d, edge %d: channelNeighborsGroundwaterFlowRateReady must be ready when not "
                  "processing a timestep.\n", thisIndex, edge);
          error = true;
        }
    }
  
  if (!error)
    {
      for (edge = 0; edge < meshNeighborsSize; edge++)
        {
          if (isBoundary(meshNeighbors[edge]))
            {
              meshNeighborsInvariantChecked[edge] = true;
            }
          else
            {
              meshNeighborsInvariantChecked[edge] = false;
              
              thisProxy[meshNeighbors[edge]].checkMeshNeighborInvariant(thisIndex, meshNeighborsReciprocalEdge[edge], edge, meshNeighborsChannelEdge[edge],
                                                                        meshNeighborsInteraction[edge], elementX, elementY, elementZSurface, elementZBedrock,
                                                                        elementArea, meshNeighborsEdgeLength[edge], meshNeighborsEdgeNormalX[edge],
                                                                        meshNeighborsEdgeNormalY[edge], conductivity, manningsN,
                                                                        meshNeighborsSurfacewaterFlowRate[edge], meshNeighborsSurfacewaterCumulativeFlow[edge],
                                                                        meshNeighborsGroundwaterFlowRate[edge], meshNeighborsGroundwaterCumulativeFlow[edge],
                                                                        dt);
            }
        }
      
      for (edge = 0; edge < channelNeighborsSize; edge++)
        {
          if (isBoundary(channelNeighbors[edge]))
            {
              channelNeighborsInvariantChecked[edge] = true;
            }
          else
            {
              channelNeighborsInvariantChecked[edge] = false;
              
              channelProxy[channelNeighbors[edge]].checkMeshNeighborInvariant(thisIndex, channelNeighborsReciprocalEdge[edge], edge,
                                                                              channelNeighborsInteraction[edge], elementX, elementY, elementZSurface,
                                                                              elementZBedrock, channelNeighborsZOffset[edge], elementSlopeX, elementSlopeY,
                                                                              channelNeighborsEdgeLength[edge], channelNeighborsSurfacewaterFlowRate[edge],
                                                                              channelNeighborsSurfacewaterCumulativeFlow[edge],
                                                                              channelNeighborsGroundwaterFlowRate[edge],
                                                                              channelNeighborsGroundwaterCumulativeFlow[edge], dt);
            }
        }
    }
  else
    {
      CkExit();
    }
}

void MeshElement::handleCheckMeshNeighborInvariant(int neighbor, int edge, int neighborEdge, bool neighborChannelEdge, InteractionEnum neighborInteraction,
                                                   double neighborX, double neighborY, double neighborZSurface, double neighborZBedrock, double neighborArea,
                                                   double neighborEdgeLength, double neighborEdgeNormalX, double neighborEdgeNormalY,
                                                   double neighborConductivity, double neighborManningsN, double neighborSurfacewaterFlowRate,
                                                   double neighborSurfacewaterCumulativeFlow, double neighborGroundwaterFlowRate,
                                                   double neighborGroundwaterCumulativeFlow, double neighborDt)
{
  bool error = false; // Error flag.
  
  if (!(neighbor == meshNeighbors[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: element %d thinks he is my neighbor on this edge, but he is "
              "not.\n", thisIndex, edge, neighbor);
      error = true;
    }
  
  if (!(neighborChannelEdge == meshNeighborsChannelEdge[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsChannelEdge is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborEdge == meshNeighborsReciprocalEdge[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsReciprocalEdge is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!((I_CALCULATE_FLOW_RATE         == neighborInteraction && NEIGHBOR_CALCULATES_FLOW_RATE == meshNeighborsInteraction[edge]) ||
        (NEIGHBOR_CALCULATES_FLOW_RATE == neighborInteraction && I_CALCULATE_FLOW_RATE         == meshNeighborsInteraction[edge]) ||
        (BOTH_CALCULATE_FLOW_RATE      == neighborInteraction && BOTH_CALCULATE_FLOW_RATE      == meshNeighborsInteraction[edge])))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsInteraction is incompatible.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborX == meshNeighborsX[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsX is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborY == meshNeighborsY[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsY is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZSurface == meshNeighborsZSurface[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsZSurface is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZBedrock == meshNeighborsZBedrock[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsZBedrock is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborArea == meshNeighborsArea[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsArea is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborEdgeLength == meshNeighborsEdgeLength[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsEdgeLength is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborEdgeNormalX == -meshNeighborsEdgeNormalX[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsEdgeNormalX is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborEdgeNormalY == -meshNeighborsEdgeNormalY[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsEdgeNormalY is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborConductivity == meshNeighborsConductivity[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsConductivity is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborManningsN == meshNeighborsManningsN[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsManningsN is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSurfacewaterFlowRate == -meshNeighborsSurfacewaterFlowRate[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsSurfacewaterFlowRate is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSurfacewaterCumulativeFlow == -meshNeighborsSurfacewaterCumulativeFlow[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsSurfacewaterCumulativeFlow is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborGroundwaterFlowRate == -meshNeighborsGroundwaterFlowRate[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsGroundwaterFlowRate is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborGroundwaterCumulativeFlow == -meshNeighborsGroundwaterCumulativeFlow[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: meshNeighborsGroundwaterCumulativeFlow is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborDt == dt))
    {
      CkError("ERROR in MeshElement::handleCheckMeshNeighborInvariant, element %d, edge %d: dt is incorrect.\n", thisIndex, edge);
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

void MeshElement::handleCheckChannelNeighborInvariant(int neighbor, int edge, int neighborEdge, InteractionEnum neighborInteraction, double neighborX,
                                                      double neighborY, double neighborZBank, double neighborZBed, double neighborZOffset,
                                                      double neighborEdgeLength, double neighborBaseWidth, double neighborSideSlope,
                                                      double neighborBedConductivity, double neighborBedThickness, double neighborSurfacewaterFlowRate,
                                                      double neighborSurfacewaterCumulativeFlow, double neighborGroundwaterFlowRate,
                                                      double neighborGroundwaterCumulativeFlow, double neighborDt)
{
  bool error = false; // Error flag.
  
  if (!(neighbor == channelNeighbors[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: element %d thinks he is my neighbor on this edge, but he is "
              "not.\n", thisIndex, edge, neighbor);
      error = true;
    }
  
  if (!(neighborEdge == channelNeighborsReciprocalEdge[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsReciprocalEdge is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!((I_CALCULATE_FLOW_RATE         == neighborInteraction && NEIGHBOR_CALCULATES_FLOW_RATE == channelNeighborsInteraction[edge]) ||
        (NEIGHBOR_CALCULATES_FLOW_RATE == neighborInteraction && I_CALCULATE_FLOW_RATE         == channelNeighborsInteraction[edge]) ||
        (BOTH_CALCULATE_FLOW_RATE      == neighborInteraction && BOTH_CALCULATE_FLOW_RATE      == channelNeighborsInteraction[edge])))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsInteraction is incompatible.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZBank == channelNeighborsZBank[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsZBank is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZBed == channelNeighborsZBed[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsZBed is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborZOffset == channelNeighborsZOffset[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsZOffset is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborEdgeLength == channelNeighborsEdgeLength[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsEdgeLength is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborBaseWidth == channelNeighborsBaseWidth[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsBaseWidth is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSideSlope == channelNeighborsSideSlope[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsSideSlope is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborBedConductivity == channelNeighborsBedConductivity[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsBedConductivity is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborBedThickness == channelNeighborsBedThickness[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsBed is incorrect.\n", thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSurfacewaterFlowRate == -channelNeighborsSurfacewaterFlowRate[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsSurfacewaterFlowRate is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborSurfacewaterCumulativeFlow == -channelNeighborsSurfacewaterCumulativeFlow[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsSurfacewaterCumulativeFlow is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborGroundwaterFlowRate == -channelNeighborsGroundwaterFlowRate[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsGroundwaterFlowRate is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborGroundwaterCumulativeFlow == -channelNeighborsGroundwaterCumulativeFlow[edge]))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: channelNeighborsGroundwaterCumulativeFlow is incorrect.\n",
              thisIndex, edge);
      error = true;
    }
  
  if (!(neighborDt == dt))
    {
      CkError("ERROR in MeshElement::handleCheckChannelNeighborInvariant, element %d, edge %d: dt is incorrect.\n", thisIndex, edge);
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

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "mesh_element.def.h"
#pragma GCC diagnostic warning "-Wunused-variable"
