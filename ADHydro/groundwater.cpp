#include "groundwater.h"
#include "adhydro.h"

//#define COURANT_DIFFUSIVE (0.2)
//NJF Testing courant change for surface water channels.  Probably don't need to change this, but am just in case
#define COURANT_DIFFUSIVE (0.04) 

bool groundwaterMeshBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowHeight, double edgeLength,
                                     double edgeNormalX, double edgeNormalY, double elementZBedrock, double elementArea, double elementSlopeX,
                                     double elementSlopeY, double conductivity, double porosity, double groundwaterHead)
{
  bool   error = false; // Error flag.
  double dtTemp;        // Temporary variable for suggesting new timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: flowRate must not be NULL.\n");
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
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!isBoundary(boundary))
    {
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: boundary must be a valid boundary condition value.\n");
      error = true;
    }
  
  if (!(0.0 <= inflowHeight))
    {
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: inflowHeight must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(epsilonEqual(1.0, edgeNormalX * edgeNormalX + edgeNormalY * edgeNormalY)))
    {
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: edgeNormalX and edgeNormalY must make a unit vector.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < conductivity))
    {
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: conductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < porosity))
    {
      CkError("ERROR in groundwaterMeshBoundaryFlowRate: porosity must be greater than zero.\n");
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
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in groundwaterMeshBoundaryFlowRate: Outward flow at INFLOW boundary.  Setting flow to zero.\n");
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
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in groundwaterMeshBoundaryFlowRate: Inward flow at OUTFLOW boundary.  Setting flow to zero.\n");
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
                                 double elementSurfacewaterDepth, double elementGroundwaterHead, double neighborX, double neighborY, double neighborZSurface,
                                 double neighborZBedrock, double neighborArea, double neighborConductivity, double neighborPorosity,
                                 double neighborSurfacewaterDepth, double neighborGroundwaterHead)
{
  bool   error                     = false;                                                 // Error flag.
  double distance                  = sqrt((elementX - neighborX) * (elementX - neighborX) + // Distance between element and neighbor centers in meters.
                                          (elementY - neighborY) * (elementY - neighborY));
  double averageArea               = 0.5 * (elementArea + neighborArea);                    // Area to use in calculating dtNew in square meters.
  double averageConductivity       = 0.5 * (elementConductivity + neighborConductivity);    // Conductivity to use in flow calculation in meters per second.
  double averagePorosity           = 0.5 * (elementPorosity + neighborPorosity);            // Porosity to use in calculating dtNew, unitless.
  double elementGroundwaterHeight  = elementGroundwaterHead  - elementZBedrock;             // Groundwater height of element in meters.
  double neighborGroundwaterHeight = neighborGroundwaterHead - neighborZBedrock;            // Groundwater height of neighbor in meters.
  double averageHeight;                                                                     // Height to use in flow calculation in meters.
  double headSlope;                                                                         // Slope of water table to use in flow calculation, unitless.
  double dtTemp;                                                                            // Temporary variable for suggesting new timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: flowRate must not be NULL.\n");
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
      CkError("ERROR in groundwaterMeshMeshFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < distance))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: Distance between element and neighbor centers must be greater than zero.\n");
      error = true;
    }
  
  if (!(elementZSurface >= elementZBedrock))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: elementZSurface must be greater than or equal to elementZBedrock");
      error = true;
    }
  
  if (!(elementZSurface >= elementGroundwaterHead))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: elementZSurface must be greater than or equal to elementGroundwaterHead");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementConductivity))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: elementConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementPorosity))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: elementPorosity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: elementSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(neighborZSurface >= neighborZBedrock))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: neighborZSurface must be greater than or equal to neighborZBedrock");
      error = true;
    }
  
  if (!(neighborZSurface >= neighborGroundwaterHead))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: neighborZSurface must be greater than or equal to neighborGroundwaterHead");
      error = true;
    }
  
  if (!(0.0 < neighborArea))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: neighborArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborConductivity))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: neighborConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborPorosity))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: neighborPorosity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in groundwaterMeshMeshFlowRate: neighborSurfacewaterDepth must be greater than or equal to zero.\n");
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
      
      // If groundwater head is at the surface add surfacewater depth.
      if (elementGroundwaterHead == elementZSurface)
        {
          elementGroundwaterHead += elementSurfacewaterDepth;
        }

      if (neighborGroundwaterHead == neighborZSurface)
        {
          neighborGroundwaterHead += neighborSurfacewaterDepth;
        }

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

bool groundwaterMeshChannelFlowRate(double* flowRate, double edgeLength, double meshZSurface, double meshZBedrock, double meshSurfacewaterDepth,
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
      CkError("ERROR in groundwaterMeshChannelFlowRate: flowRate must not be NULL.\n");
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
      CkError("ERROR in groundwaterMeshChannelFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(meshZSurface >= meshZBedrock))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: meshZSurface must be greater than or equal to meshZBedrock");
      error = true;
    }
  
  if (!(meshZSurface >= meshGroundwaterHead))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: meshZSurface must be greater than or equal to meshGroundwaterHead");
      error = true;
    }
  
  if (!(0.0 <= meshSurfacewaterDepth))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: meshSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(channelZBank >= channelZBed))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: channelZBank must be greater than or equal to channelZBed");
      error = true;
    }
  
  if (!(0 <= channelBaseWidth))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: channelBaseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= channelSideSlope))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: channelSideSlope must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(epsilonGreater(channelBaseWidth, 0.0) || epsilonGreater(channelSideSlope, 0.0)))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: at least one of channelBaseWidth or channelSideSlope must be epsilon greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < channelBedConductivity))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: channelBedConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < channelBedThickness))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: channelBedThickness must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= channelSurfacewaterDepth))
    {
      CkError("ERROR in groundwaterMeshChannelFlowRate: channelSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // If groundwater is at the surface add surfacewater depth to find the real head.
      if (meshGroundwaterHead == meshZSurface)
        {
          meshGroundwaterHead += meshSurfacewaterDepth;
        }

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
