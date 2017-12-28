#include "groundwater.h"
#include "readonly.h"

#define COURANT_DIFFUSIVE (0.2)

bool groundwaterMeshBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowHeight, double edgeLength,
                                     double edgeNormalX, double edgeNormalY, double elementZBedrock, double elementArea, double elementSlopeX,
                                     double elementSlopeY, double conductivity, double porosity, double groundwaterHead)
{
  bool   error = false; // Error flag.
  double dtTemp;        // Temporary variable for suggesting new timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: flowRate must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *flowRate = 0.0;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != dtNew && 0.0 < *dtNew))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!isBoundary(boundary))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: boundary must be a valid boundary condition value.\n");
      error = true;
    }
  
  if (!(0.0 <= inflowHeight))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: inflowHeight must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(epsilonEqual(1.0, edgeNormalX * edgeNormalX + edgeNormalY * edgeNormalY)))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: edgeNormalX and edgeNormalY must make a unit vector.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < conductivity))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: conductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < porosity))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshBoundaryFlowRate: porosity must be greater than zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      if (INFLOW == boundary)
        {
          // Calculate flow rate.
          *flowRate = conductivity * -(elementSlopeX * edgeNormalX + elementSlopeY * edgeNormalY) * inflowHeight * edgeLength;

          // Force flow rate to have the correct sign.
          if (0.0 < *flowRate)
            {
              if (2 <= Readonly::verbosityLevel)
                {
                  ADHYDRO_ERROR("WARNING in groundwaterMeshBoundaryFlowRate: Outward flow at INFLOW boundary.  Setting flow to zero.\n");
                }
              
              *flowRate = 0.0;
            }
        }
      else if (OUTFLOW == boundary && elementZBedrock < groundwaterHead)
        {
          // Calculate flow rate.
          *flowRate = conductivity * -(elementSlopeX * edgeNormalX + elementSlopeY * edgeNormalY) * (groundwaterHead - elementZBedrock) * edgeLength;

          // Force flow rate to have the correct sign.
          if (0.0 > *flowRate)
            {
              if (2 <= Readonly::verbosityLevel)
                {
                  ADHYDRO_ERROR("WARNING in groundwaterMeshBoundaryFlowRate: Inward flow at OUTFLOW boundary.  Setting flow to zero.\n");
                }

              *flowRate = 0.0;
            }

          // Suggest new timestep.
          dtTemp = COURANT_DIFFUSIVE * porosity * 2.0 * elementArea / (conductivity * (groundwaterHead - elementZBedrock));

          if (*dtNew > dtTemp)
            {
              *dtNew = dtTemp;
            }
        }
    }
  // else if (NOFLOW == boundary || OUTFLOW == boundary && elementZBedrock >= groundwaterHead)) flowRate has already been assigned to zero above.

  return error;
}

bool groundwaterMeshMeshFlowRate(double* flowRate, double* dtNew, double edgeLength, double elementX, double elementY, double elementZSurface,
                                 double elementZBedrock, double elementArea, double elementConductivity, double elementPorosity,
                                 double elementGroundwaterHead, double neighborX, double neighborY, double neighborZSurface,
                                 double neighborZBedrock, double neighborArea, double neighborConductivity, double neighborPorosity, double neighborGroundwaterHead)
{
  bool   error                     = false;                                                 // Error flag.
  double distance                  = sqrt((elementX - neighborX) * (elementX - neighborX) + // Distance between element and neighbor centers in meters.
                                          (elementY - neighborY) * (elementY - neighborY));
  double averageArea               = 0.5 * (elementArea + neighborArea);                    // Area to use in calculating dtNew in square meters.
  double averageConductivity       = 0.5 * (elementConductivity + neighborConductivity);    // Conductivity to use in flow calculation in meters per second.
  double averagePorosity           = 0.5 * (elementPorosity + neighborPorosity);            // Porosity to use in calculating dtNew, unitless.
  double elementGroundwaterHeight  = ((elementGroundwaterHead  < elementZSurface)  ? elementGroundwaterHead  : elementZSurface)  - elementZBedrock;  // Groundwater height of element in meters.
  double neighborGroundwaterHeight = ((neighborGroundwaterHead < neighborZSurface) ? neighborGroundwaterHead : neighborZSurface) - neighborZBedrock; // Groundwater height of neighbor in meters.
  double averageHeight;                                                                     // Height to use in flow calculation in meters.
  double headSlope;                                                                         // Slope of water table to use in flow calculation, unitless.
  double dtTemp;                                                                            // Temporary variable for suggesting new timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: flowRate must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *flowRate = 0.0;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != dtNew && 0.0 < *dtNew))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < distance))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: Distance between element and neighbor centers must be greater than zero.\n");
      error = true;
    }
  
  if (!(elementZSurface >= elementZBedrock))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: elementZSurface must be greater than or equal to elementZBedrock");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementConductivity))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: elementConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementPorosity))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: elementPorosity must be greater than zero.\n");
      error = true;
    }
  
  if (!(neighborZSurface >= neighborZBedrock))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: neighborZSurface must be greater than or equal to neighborZBedrock");
      error = true;
    }
  
  if (!(0.0 < neighborArea))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: neighborArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborConductivity))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: neighborConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborPorosity))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshMeshFlowRate: neighborPorosity must be greater than zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // If groundwater head is below bedrock set height to zero.
      if (0.0 > elementGroundwaterHeight)
        {
          elementGroundwaterHeight = 0.0;
        }

      if (0.0 > neighborGroundwaterHeight)
        {
          neighborGroundwaterHeight = 0.0;
        }

      averageHeight = 0.5 * (elementGroundwaterHeight + neighborGroundwaterHeight);

      headSlope = (elementGroundwaterHead - neighborGroundwaterHead) / distance;
      
      // Calculate flow rate.
      if ((elementGroundwaterHead > neighborGroundwaterHead && 0.0 < elementGroundwaterHeight) ||
          (elementGroundwaterHead < neighborGroundwaterHead && 0.0 < neighborGroundwaterHeight))
        {
          *flowRate = averageConductivity * averageHeight * headSlope * edgeLength;
          
          // Suggest new timestep.
          dtTemp = COURANT_DIFFUSIVE * averagePorosity * 2.0 * averageArea / (averageConductivity * fabs(elementGroundwaterHead - neighborGroundwaterHead));
          
          if (*dtNew > dtTemp)
            {
              *dtNew = dtTemp;
            }
        }
      // else if there is no slope or the higher element is dry there is no flow.  flowRate has already been assigned to zero above.
    }

  return error;
}

bool groundwaterMeshChannelFlowRate(double* flowRate, double edgeLength, double meshZSurface, double meshZBedrock,
                                    double meshGroundwaterHead, double channelZBank, double channelZBed, double channelBaseWidth, double channelSideSlope,
                                    double channelBedConductivity, double channelBedThickness, double channelSurfacewaterDepth)
{
  bool   error                   = false;                                  // Error flag.
  double channelSurfacewaterHead = channelSurfacewaterDepth + channelZBed; // Surfacewater head in channel.
  double wettedPerimeterHighPoint;                                         // For calculating wetted perimeter.
  double wettedPerimeterLowPoint;                                          // For calculating wetted perimeter.
  double wettedPerimeter;                                                  // Perimeter of channel through which water can flow in meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: flowRate must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *flowRate = 0.0;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < edgeLength))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(meshZSurface >= meshZBedrock))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: meshZSurface must be greater than or equal to meshZBedrock");
      error = true;
    }
  
  if (!(channelZBank >= channelZBed))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: channelZBank must be greater than or equal to channelZBed");
      error = true;
    }
  
  if (!(0 <= channelBaseWidth))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: channelBaseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= channelSideSlope))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: channelSideSlope must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(epsilonGreater(channelBaseWidth, 0.0) || epsilonGreater(channelSideSlope, 0.0)))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: at least one of channelBaseWidth or channelSideSlope must be epsilon greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < channelBedConductivity))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: channelBedConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < channelBedThickness))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: channelBedThickness must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= channelSurfacewaterDepth))
    {
      ADHYDRO_ERROR("ERROR in groundwaterMeshChannelFlowRate: channelSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // If channel bank is below the surface use the surface elevation instead.
      if (channelZBank < meshZSurface)
        {
          channelZBank = meshZSurface;
        }

      if (meshGroundwaterHead > channelSurfacewaterHead)
        {
          // Flow direction is into the channel.

          // Limit head difference at bedrock.
          if (channelSurfacewaterHead < meshZBedrock)
            {
              channelSurfacewaterHead = meshZBedrock;
            }

          // Determine the high point of the wetted perimeter.
          if (channelZBank < meshGroundwaterHead)
            {
              wettedPerimeterHighPoint = channelZBank - channelZBed; // Meters above channel bed.
            }
          else
            {
              wettedPerimeterHighPoint = meshGroundwaterHead - channelZBed; // Meters above channel bed.
            }
        }
      else
        {
          // Flow direction is out of the channel.

          // Limit head difference at bedrock.
          if (meshGroundwaterHead < meshZBedrock)
            {
              meshGroundwaterHead = meshZBedrock;
            }

          // Limit head difference at bottom of channel bed.
          if (meshGroundwaterHead < channelZBed - channelBedThickness)
            {
              meshGroundwaterHead = channelZBed - channelBedThickness;
            }

          // Determine the high point of the wetted perimeter.
          if (channelZBank < channelSurfacewaterHead)
            {
              wettedPerimeterHighPoint = channelZBank - channelZBed; // Meters above channel bed.
            }
          else
            {
              wettedPerimeterHighPoint = channelSurfacewaterHead - channelZBed; // Meters above channel bed.
            }
        }

      // Determine the low point of the wetted perimeter.
      if (channelZBed < meshZBedrock)
        {
          wettedPerimeterLowPoint = meshZBedrock - channelZBed; // Meters above channel bed.
        }
      else
        {
          wettedPerimeterLowPoint = 0.0; // Meters above channel bed.
        }

      if (wettedPerimeterHighPoint > wettedPerimeterLowPoint)
        {
          // Determine the wetted perimeter.
          if (0.0 == wettedPerimeterLowPoint)
            {
              // Slope length of channel side.
              wettedPerimeter = wettedPerimeterHighPoint * sqrt(1 + channelSideSlope * channelSideSlope);
              
              // Add near side (half) of channel base.
              wettedPerimeter += 0.5 * channelBaseWidth;
              
              // If channel bed is close to bedrock flow might be limited by perpendicular height from wettedPerimeterHighPoint to bedrock.
              if (wettedPerimeter > wettedPerimeterHighPoint + channelZBed - meshZBedrock)
                {
                  wettedPerimeter = wettedPerimeterHighPoint + channelZBed - meshZBedrock;
                }
            }
          else
            {
              // Flow is limited by perpendicular height from wettedPerimeterHighPoint to wettedPerimeterLowPoint.
              wettedPerimeter = wettedPerimeterHighPoint - wettedPerimeterLowPoint;
            }

          // Calculate flow rate.
          *flowRate = channelBedConductivity * ((meshGroundwaterHead - channelSurfacewaterHead) / channelBedThickness) * edgeLength * wettedPerimeter;
        }
      // else if there is no wetted perimeter there is no flow.  flowRate has already been assigned to zero above. 
    }

  return error;
}
