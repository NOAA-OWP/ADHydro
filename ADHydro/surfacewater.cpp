#include "surfacewater.h"

bool surfacewaterMeshBoundaryFlowRate(double* flowRate, BoundaryConditionEnum boundary, double inflowXVelocity, double inflowYVelocity, double inflowHeight,
                                      double edgeLength, double edgeNormalX, double edgeNormalY, double surfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: flowRate must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *flowRate = 0.0;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!isBoundary(boundary))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: boundary must be a valid boundary condition value.\n");
      error = true;
    }
  
  if (!(0 <= inflowHeight))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: inflowHeight must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(epsilonEqual(1.0, edgeNormalX * edgeNormalX + edgeNormalY * edgeNormalY)))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: edgeNormalX and edgeNormalY must make a unit vector.\n");
      error = true;
    }
  
  if (!(0 <= surfacewaterDepth))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      if (INFLOW == boundary)
        {
          *flowRate = (inflowXVelocity * edgeNormalX + inflowYVelocity * edgeNormalY) * inflowHeight * edgeLength;
          
          if (0.0 < *flowRate)
            {
              CkError("WARNING in surfacewaterMeshBoundaryFlowRate: Outward flow at INFLOW boundary.  Setting flow to zero.\n");
              *flowRate = 0.0;
            }
        }
      else if (OUTFLOW == boundary && PONDED_DEPTH < surfacewaterDepth)
        {
          *flowRate = sqrt(GRAVITY * surfacewaterDepth) * surfacewaterDepth * edgeLength;
          
          if (0.0 > *flowRate)
            {
              CkError("WARNING in surfacewaterMeshBoundaryFlowRate: Inward flow at OUTFLOW boundary.  Setting flow to zero.\n");
              *flowRate = 0.0;
            }
        }
      // else if (NOFLOW == boundary || (OUTFLOW == boundary && PONDED_DEPTH >= surfacewaterDepth))
      //   flowRate has already been assigned to zero above.
    }
  
  return error;
}

bool surfacewaterChannelBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowVelocity, double inflowHeight,
                                         double length, double baseWidth, double sideSlope, double waterDepth)
{
  bool   error = false;   // Error flag.
  double topWidth;        // Meters.  Width of channel at water surface.
  double area;            // Square meters.  Cross sectional area of trapezoidal channel.
  double outflowVelocity; // Meters per second.
  double dtTemp;          // Temporary variable for suggesting new timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: flowRate must not be NULL.\n");
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
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!isBoundary(boundary))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: boundary must be a valid boundary condition value.\n");
      error = true;
    }
  
  if (!(0 <= inflowVelocity))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: inflowVelocity must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= inflowHeight))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: inflowHeight must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 < length))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: length must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= baseWidth))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: baseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= sideSlope))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: sideSlope must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(epsilonGreater(baseWidth, 0.0) || epsilonGreater(sideSlope, 0.0)))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: at least one of baseWidth or sideSlope must be epsilon greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= waterDepth))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: waterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      if (INFLOW == boundary)
        {
          area      = inflowHeight * (baseWidth + sideSlope * inflowHeight);
          *flowRate = -inflowVelocity * area; // Negative velocity because inflow is defined as negative flow.
          
          if (0.0 < *flowRate)
            {
              CkError("WARNING in surfacewaterChannelBoundaryFlowRate: Outward flow at INFLOW boundary.  Setting flow to zero.\n");
              *flowRate = 0.0;
            }
        }
      else if (OUTFLOW == boundary && PONDED_DEPTH < waterDepth)
        {
          topWidth = baseWidth + 2.0 * sideSlope * waterDepth;
          area     = waterDepth * (baseWidth + sideSlope * waterDepth);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // We have already checked that waterDepth is greater than zero and at least one of baseWidth or sideSlope is greater than zero therefore we do not
          // have to protect against dividing by zero when we divide by topWidth or area.
          CkAssert(0.0 < topWidth && 0.0 < area);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          outflowVelocity = sqrt(GRAVITY * area / topWidth);
          *flowRate       = outflowVelocity * area;
          
          if (0.0 > *flowRate)
            {
              CkError("WARNING in surfacewaterChannelBoundaryFlowRate: Inward flow at OUTFLOW boundary.  Setting flow to zero.\n");
              *flowRate = 0.0;
            }
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // This denominator also cannot be zero.
          CkAssert(0.0 < *flowRate / area + sqrt(GRAVITY * waterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          // Suggest new timestep.
          dtTemp = 0.1 * length / (*flowRate / area + sqrt(GRAVITY * waterDepth));
          
          if (*dtNew > dtTemp)
            {
              *dtNew = dtTemp;
            }
        }
      // else if (NOFLOW == boundary || (OUTFLOW == boundary && PONDED_DEPTH >= surfacewaterDepth))
      //   flowRate has already been assigned to zero above.
    }
  
  return error;
}

bool surfacewaterMeshMeshFlowRate(double* flowRate, double* dtNew, double edgeLength, double elementX, double elementY, double elementZSurface,
                                  double elementArea, double elementManningsN, double elementSurfacewaterDepth, double neighborX, double neighborY,
                                  double neighborZSurface, double neighborManningsN, double neighborSurfacewaterDepth)
{
  bool   error                    = false;                                                           // Error flag.
  double elementSurfacewaterHead  = elementSurfacewaterDepth  + elementZSurface;                     // Surfacewater head of element.
  double neighborSurfacewaterHead = neighborSurfacewaterDepth + neighborZSurface;                    // Surfacewater head of neighbor.
  double averageManningsN         = 0.5 * (elementManningsN + neighborManningsN);                    // Manning's n to use in flow calculation.
  double averageDepth             = 0.5 * (elementSurfacewaterDepth + neighborSurfacewaterDepth);    // Depth to use in flow calculation.
  double distance                 = sqrt((elementX - neighborX) * (elementX - neighborX) + (elementY - neighborY) * (elementY - neighborY));
                                                                                                     // Distance between element and neighbor centers.
  double headSlope                = (elementSurfacewaterHead - neighborSurfacewaterHead) / distance; // Slope of water surface.
  double dtTemp;                                                                                     // Temporary variable for suggesting new timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: flowRate must not be NULL.\n");
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
      CkError("ERROR in surfacewaterMeshMeshFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementManningsN))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: elementManningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: elementSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborManningsN))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: neighborManningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: neighborSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < distance))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: Distance between element and neighbor centers must be greater than zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error && ((elementSurfacewaterHead > neighborSurfacewaterHead && PONDED_DEPTH < elementSurfacewaterDepth) ||
                 (elementSurfacewaterHead < neighborSurfacewaterHead && PONDED_DEPTH < neighborSurfacewaterDepth)))
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 < averageDepth && 0.0 != headSlope);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      *flowRate = (pow(averageDepth, 5.0 / 3.0) / (averageManningsN * sqrt(fabs(headSlope)))) * headSlope * edgeLength;

      // Suggest new timestep.
      // FIXME if this calculation is only done by one of the neighbors only that neighbor's area will be used to limit the timestep, but neighbors should have similar sizes.
      dtTemp = 0.1 * sqrt(2.0 * elementArea) / (pow(averageDepth, 2.0 / 3.0) * sqrt(fabs(headSlope)) / averageManningsN + sqrt(GRAVITY * averageDepth));

      if (*dtNew > dtTemp)
        {
          *dtNew = dtTemp;
        }
    }
  // else if there is no slope or the higher element is dry there is no flow.

  return error;
}

bool surfacewaterMeshChannelFlowRate(double* flowRate, double edgeLength, double surfacewaterDepth)
{
  return surfacewaterMeshBoundaryFlowRate(flowRate, OUTFLOW, 0.0, 0.0, 0.0, edgeLength, 1.0, 0.0, surfacewaterDepth);
}

bool surfacewaterChannelChannelFlowRate(double* flowRate, double* dtNew, double elementZBed, double elementWaterDepth, double neighborZBed,
                                        double neighborWaterDepth, double distance, double manningsN, double area, double radius)
{
  bool   error             = false;                                                      // Error flag.
  double elementWaterHead  = elementWaterDepth  + elementZBed;                           // Water head of element.
  double neighborWaterHead = neighborWaterDepth + neighborZBed;                          // Water head of neighbor.
  double averageDepth      = 0.5 * (elementWaterDepth + neighborWaterDepth);             // Depth to use in flow calculation.
  double headSlope         = (elementWaterHead - neighborWaterHead) / distance;          // Slope of water surface.
  double velocityDirection = (0.0 <= headSlope) ? 1.0 : -1.0;                            // Direction of flow
  double velocityMagnitude = pow(radius, 2.0 / 3.0) * sqrt(fabs(headSlope)) / manningsN; // Magnitude of flow velocity.
  double criticalVelocity  = sqrt(GRAVITY * averageDepth);                               // Critical flow velocity.
  double dtTemp;                                                                         // Temporary variable for suggesting new timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: flowRate must not be NULL.\n");
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
      CkError("ERROR in surfacewaterChannelChannelFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= elementWaterDepth))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: elementWaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborWaterDepth))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: neighborWaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < distance))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: distance must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < manningsN))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: manningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= area))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: area must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= radius))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: radius must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // If there is no slope or the higher element is dry there is no flow.
  if (!error && ((elementWaterHead > neighborWaterHead && PONDED_DEPTH < elementWaterDepth) ||
                 (elementWaterHead < neighborWaterHead && PONDED_DEPTH < neighborWaterDepth)))
    {
      // diffusive wave flow assumes small velocities.  Cap the velocity at the critical velocity.  FIXME discuss what we want to do here.
      if (velocityMagnitude > criticalVelocity)
        {
          velocityMagnitude = criticalVelocity;
        }
      
      *flowRate = velocityDirection * velocityMagnitude * area;
      
      // Suggest new timestep.
      if (0.0 < area)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // This denominator cannot be zero.
          CkAssert(0.0 < fabs(*flowRate) / area + sqrt(GRAVITY * averageDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          dtTemp = 0.1 * distance / (fabs(*flowRate) / area + sqrt(GRAVITY * averageDepth));

          if (*dtNew > dtTemp)
            {
              *dtNew = dtTemp;
            }
        }
    }

  return error;
}
