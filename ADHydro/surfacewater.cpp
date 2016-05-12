#include "surfacewater.h"
#include "adhydro.h"

#define COURANT_DIFFUSIVE (0.2)

bool surfacewaterMeshBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowXVelocity, double inflowYVelocity,
                                      double inflowHeight, double edgeLength, double edgeNormalX, double edgeNormalY, double elementArea,
                                      double surfacewaterDepth)
{
  bool   error = false; // Error flag.
  double dtTemp;        // Temporary variable for suggesting new timestep.
  
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
  if (!(NULL != dtNew && 0.0 < *dtNew))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!isBoundary(boundary))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: boundary must be a valid boundary condition value.\n");
      error = true;
    }
  
  if (!(0.0 <= inflowHeight))
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
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in surfacewaterMeshBoundaryFlowRate: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      if (INFLOW == boundary)
        {
          // Calculate flow rate.
          *flowRate = (inflowXVelocity * edgeNormalX + inflowYVelocity * edgeNormalY) * inflowHeight * edgeLength;
          
          // Force flow rate to have the correct sign.
          if (0.0 < *flowRate)
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in surfacewaterMeshBoundaryFlowRate: Outward flow at INFLOW boundary.  Setting flow to zero.\n");
                }
              
              *flowRate = 0.0;
            }
        }
      else if (OUTFLOW == boundary && PONDED_DEPTH < surfacewaterDepth)
        {
          // Calculate flow rate.
          *flowRate = sqrt(GRAVITY * surfacewaterDepth) * surfacewaterDepth * edgeLength;
          
          // Force flow rate to have the correct sign.
          if (0.0 > *flowRate)
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in surfacewaterMeshBoundaryFlowRate: Inward flow at OUTFLOW boundary.  Setting flow to zero.\n");
                }
              
              *flowRate = 0.0;
            }
          
          // Suggest new timestep.
          dtTemp = COURANT_DIFFUSIVE * sqrt(2.0 * elementArea) / (2.0 * sqrt(GRAVITY * surfacewaterDepth));
          
          if (*dtNew > dtTemp)
            {
              *dtNew = dtTemp;
            }
        }
      // else if (NOFLOW == boundary || (OUTFLOW == boundary && PONDED_DEPTH >= surfacewaterDepth)) flowRate has already been assigned to zero above.
    }
  
  return error;
}

bool surfacewaterChannelBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowVelocity, double inflowHeight,
                                         double elementLength, double baseWidth, double sideSlope, double surfacewaterDepth)
{
  bool   error = false;   // Error flag.
  double topWidth;        // Width of channel at water surface in meters.
  double area;            // Cross sectional area of water in trapezoidal channel in square meters.
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
  
  if (!(0.0 <= inflowVelocity))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: inflowVelocity must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= inflowHeight))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: inflowHeight must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementLength))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: elementLength must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= baseWidth))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: baseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= sideSlope))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: sideSlope must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(epsilonGreater(baseWidth, 0.0) || epsilonGreater(sideSlope, 0.0)))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: at least one of baseWidth or sideSlope must be epsilon greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in surfacewaterChannelBoundaryFlowRate: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      if (INFLOW == boundary)
        {
          // Calculate flow rate.
          area      = inflowHeight * (baseWidth + sideSlope * inflowHeight);
          *flowRate = -inflowVelocity * area; // Negative velocity because flow into the element is defined as negative flow.
          
          // Force flow rate to have the correct sign.
          if (0.0 < *flowRate)
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in surfacewaterChannelBoundaryFlowRate: Outward flow at INFLOW boundary.  Setting flow to zero.\n");
                }
              
              *flowRate = 0.0;
            }
        }
      else if (OUTFLOW == boundary && PONDED_DEPTH < surfacewaterDepth)
        {
          // Calculate flow rate.
          topWidth        = baseWidth + 2.0 * sideSlope * surfacewaterDepth;
          area            = surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth);
          outflowVelocity = sqrt(GRAVITY * area / topWidth);
          *flowRate       = outflowVelocity * area;
          
          // Force flow rate to have the correct sign.
          if (0.0 > *flowRate)
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in surfacewaterChannelBoundaryFlowRate: Inward flow at OUTFLOW boundary.  Setting flow to zero.\n");
                }
              
              *flowRate = 0.0;
            }
          
          // Suggest new timestep.
          dtTemp = COURANT_DIFFUSIVE * elementLength / (outflowVelocity + sqrt(GRAVITY * surfacewaterDepth));
          
          if (*dtNew > dtTemp)
            {
              *dtNew = dtTemp;
            }
        }
      // else if (NOFLOW == boundary || (OUTFLOW == boundary && PONDED_DEPTH >= surfacewaterDepth)) flowRate has already been assigned to zero above.
    }
  
  return error;
}

bool surfacewaterMeshMeshFlowRate(double* flowRate, double* dtNew, double edgeLength, double elementX, double elementY, double elementZSurface,
                                  double elementArea, double elementManningsN, double elementSurfacewaterDepth, double neighborX, double neighborY,
                                  double neighborZSurface, double neighborArea, double neighborManningsN, double neighborSurfacewaterDepth)
{
  bool   error                    = false;                                                           // Error flag.
  double distance                 = sqrt((elementX - neighborX) * (elementX - neighborX) +           // Distance between element and neighbor centers in
                                         (elementY - neighborY) * (elementY - neighborY));           // meters.
  double averageArea              = 0.5 * (elementArea + neighborArea);                              // Area to use in flow calculation in square meters.
  double averageManningsN         = 0.5 * (elementManningsN + neighborManningsN);                    // Manning's n to use in flow calculation.
  double averageDepth             = 0.5 * (elementSurfacewaterDepth + neighborSurfacewaterDepth);    // Depth to use in flow calculation in meters.
  double elementSurfacewaterHead  = elementSurfacewaterDepth  + elementZSurface;                     // Surfacewater head of element in meters.
  double neighborSurfacewaterHead = neighborSurfacewaterDepth + neighborZSurface;                    // Surfacewater head of neighbor in meters.
  double headSlope                = (elementSurfacewaterHead - neighborSurfacewaterHead) / distance; // Slope of water surface, unitless.
  double dtTemp;                                                                                     // Temporary variable for suggesting new timestep in
                                                                                                     // seconds.
  
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
  
  if (!(0.0 < distance))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: Distance between element and neighbor centers must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: elementArea must be greater than zero.\n");
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
  
  if (!(0.0 < neighborArea))
    {
      CkError("ERROR in surfacewaterMeshMeshFlowRate: neighborArea must be greater than zero.\n");
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
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error && ((elementSurfacewaterHead > neighborSurfacewaterHead && PONDED_DEPTH < elementSurfacewaterDepth) ||
                 (elementSurfacewaterHead < neighborSurfacewaterHead && PONDED_DEPTH < neighborSurfacewaterDepth)))
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 < averageDepth && 0.0 != headSlope);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      *flowRate = (pow(averageDepth, 5.0 / 3.0) / (averageManningsN * sqrt(fabs(headSlope)))) * headSlope * edgeLength;

      // Suggest new timestep.
      dtTemp = COURANT_DIFFUSIVE * sqrt(2.0 * averageArea) / (pow(averageDepth, 2.0 / 3.0) * sqrt(fabs(headSlope)) / averageManningsN + sqrt(GRAVITY *
                                                                                                                                             averageDepth));

      if (*dtNew > dtTemp)
        {
          *dtNew = dtTemp;
        }
    }
  // else if there is no slope or the higher element is dry there is no flow.  flowRate has already been assigned to zero above.

  return error;
}

bool surfacewaterMeshChannelFlowRate(double* flowRate, double* dtNew, double edgeLength, double meshZSurface, double meshArea, double meshSurfacewaterDepth,
                                     double channelZBank, double channelZBed, double channelBaseWidth, double channelSideSlope,
                                     double channelSurfacewaterDepth)
{
  bool   error                   = false;                                  // Error flag.
  double meshSurfacewaterHead    = meshSurfacewaterDepth + meshZSurface;   // The elevation in meters of the mesh surfacewater.
  double channelSurfacewaterHead = channelSurfacewaterDepth + channelZBed; // The elevation in meters of the channel surfacewater.
  double weirElevation           = (meshZSurface > channelZBank) ?         // The elevation in meters of the thing that water has to flow over.
                                   meshZSurface : channelZBank;
  double channelTopWidth         = channelBaseWidth +                      // Width of channel at water surface in meters.
                                   2.0 * channelSideSlope * channelSurfacewaterDepth;
  double dtTemp;                                                           // Temporary variable for suggesting new timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: flowRate must not be NULL.\n");
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
      CkError("ERROR in surfacewaterMeshChannelFlowRate: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < meshArea))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: meshArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= meshSurfacewaterDepth))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: meshSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(channelZBank >= channelZBed))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: channelZBank must be greater than or equal to channelZBed");
      error = true;
    }
  
  if (!(0 <= channelBaseWidth))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: channelBaseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= channelSideSlope))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: channelSideSlope must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(epsilonGreater(channelBaseWidth, 0.0) || epsilonGreater(channelSideSlope, 0.0)))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: at least one of channelBaseWidth or channelSideSlope must be epsilon greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= channelSurfacewaterDepth))
    {
      CkError("ERROR in surfacewaterMeshChannelFlowRate: channelSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      if (meshSurfacewaterHead > channelSurfacewaterHead && PONDED_DEPTH < meshSurfacewaterDepth)
        {
          // Flow from the mesh to the channel.  If the channel water is over bank set weirElevation no lower than channelSurfacewaterHead.
          if (weirElevation < channelSurfacewaterHead)
            {
              weirElevation = channelSurfacewaterHead;
            }
          
          meshSurfacewaterDepth = meshSurfacewaterHead - weirElevation;
          
          if (0.0 < meshSurfacewaterDepth)
            {
              // Assume critical velocity flow over a broad crested weir.
              *flowRate = sqrt(GRAVITY * meshSurfacewaterDepth) * meshSurfacewaterDepth * edgeLength;
              
              // Suggest new timestep.
              dtTemp = COURANT_DIFFUSIVE * sqrt(2.0 * meshArea) / (2.0 * sqrt(GRAVITY * meshSurfacewaterDepth));
              
              if (*dtNew > dtTemp)
                {
                  *dtNew = dtTemp;
                }
            }
          // else if water elevation is below the weir there is no flow.  flowRate has already been assigned to zero above.
        }
      else if (channelSurfacewaterHead > meshSurfacewaterHead && PONDED_DEPTH < channelSurfacewaterDepth)
        {
          // Flow from the channel to the mesh.  If the mesh water is over bank set weirElevation no lower than meshSurfacewaterHead.
          if (weirElevation < meshSurfacewaterHead)
            {
              weirElevation = meshSurfacewaterHead;
            }
          
          channelSurfacewaterDepth = channelSurfacewaterHead - weirElevation;
          
          if (0.0 < channelSurfacewaterDepth)
            {
              // Assume critical velocity flow over a broad crested weir.  Flow from channel to mesh is negative.
              *flowRate = -sqrt(GRAVITY * channelSurfacewaterDepth) * channelSurfacewaterDepth * edgeLength;
              
              // Suggest new timestep.
              dtTemp = COURANT_DIFFUSIVE * channelTopWidth / (2.0 * sqrt(GRAVITY * channelSurfacewaterDepth));
              
              if (*dtNew > dtTemp)
                {
                  *dtNew = dtTemp;
                }
            }
          // else if water elevation is below the weir there is no flow.  flowRate has already been assigned to zero above.
        }
      // else if water elevations are equal there is no flow.  flowRate has already been assigned to zero above.
    }
  
  return error;
}

// Calculate the surfacewater flow rate in cubic meters per second between a
// stream element and its stream neighbor.  Positive means flow out of the
// element into the neighbor.  Negative means flow into the element out of the
// neighbor.
//
// Currently, this function only calculates diffusive wave flow.
//
// The governing equation is the diffusive wave equation:
//
// Q   = -R^(2/3)*A*Z_x/(n*sqrt(abs(Z_x)));
// Q   = flow rate (flux),                 [m^3/s];
// R   = hydraulic radius,                 [m];
// A   = cross section area,               [m^2];
// Z   = water surface elevation,          [m];
// Z_x = slope of water surface elevation, [-];
// n   = Manning's roughness coefficint,   [s/m^(1/3)];
//
// Parameters:
//
// flowRate                  - Scalar passed by reference will be filled in
//                             with the flow rate in cubic meters per second.
// dtNew                     - Scalar passed by reference containing the
//                             suggested value for the next timestep duration
//                             in seconds.  May be updated to be shorter.
// elementZBed               - Bed Z coordinate of element in meters.
// elementLength             - Length of element in meters.  Can be zero if it
//                             is an imaginary stream element representing the
//                             mouth of a waterbody.
// elementBaseWidth          - Base width of element in meters.
// elementSideSlope          - Side slope of element, unitless.
// elementManningsN          - Surface roughness of element.
// elementSurfacewaterDepth  - Surfacewater depth of element in meters.
// neighborZBed              - Bed Z coordinate of neighbor in meters.
// neighborLength            - Length of neighbor in meters.
// neighborBaseWidth         - Base width of neighbor in meters.
// neighborSideSlope         - Side slope of neighbor, unitless.
// neighborManningsN         - Surface roughness of neighbor.
// neighborSurfacewaterDepth - Surfacewater depth of neighbor in meters.
void surfacewaterStreamStreamFlowRate(double* flowRate, double* dtNew, double elementZBed, double elementLength, double elementBaseWidth,
                                      double elementSideSlope, double elementManningsN, double elementSurfacewaterDepth, double neighborZBed,
                                      double neighborLength, double neighborBaseWidth, double neighborSideSlope, double neighborManningsN,
                                      double neighborSurfacewaterDepth)
{
  double distance                 = 0.5 * (elementLength + neighborLength);                          // Distance along channel between element and neighbor in
                                                                                                     // meters.
  double averageManningsN         = 0.5 * (elementManningsN + neighborManningsN);                    // Manning's n to use in flow calculation.
  double averageDepth             = 0.5 * (elementSurfacewaterDepth + neighborSurfacewaterDepth);    // Depth to use in flow calculation in meters.
  double elementSurfacewaterHead  = elementSurfacewaterDepth + elementZBed;                          // Surfacewater head of element in meters.
  double neighborSurfacewaterHead = neighborSurfacewaterDepth + neighborZBed;                        // Surfacewater head of neighbor in meters.
  double headSlope                = (elementSurfacewaterHead - neighborSurfacewaterHead) / distance; // Slope of water surface, unitless.
  double elementArea;                                                                                // Cross sectional area of water in trapezoidal channel in
                                                                                                     // square meters.
  double neighborArea;                                                                               // Cross sectional area of water in trapezoidal channel in
                                                                                                     // square meters.
  double averageArea;                                                                                // Area to use in flow calculation in square meters.
  double elementRadius;                                                                              // Hydraulic radius of element in meters.
  double neighborRadius;                                                                             // Hydraulic radius of neighbor in meters.
  double averageRadius;                                                                              // Hydraulic radius to use in flow calculation in meters.
  double velocityDirection;                                                                          // Direction of flow, unitless.
  double velocityMagnitude;                                                                          // Magnitude of flow velocity in meters per second.
  double criticalVelocity;                                                                           // Critical flow velocity in meters per second.
  double dtTemp;                                                                                     // Temporary variable for suggesting new timestep in
                                                                                                     // seconds.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != flowRate && NULL != dtNew && 0.0 < *dtNew && 0.0 <= elementLength && 0.0 <= elementBaseWidth && 0.0 <= elementSideSlope &&
           (epsilonLess(0.0, elementBaseWidth) || epsilonLess(0.0, elementSideSlope)) && 0.0 < elementManningsN && 0.0 <= elementSurfacewaterDepth &&
           0.0 < neighborLength && 0.0 <= neighborBaseWidth && 0.0 <= neighborSideSlope &&
           (epsilonLess(0.0, neighborBaseWidth) || epsilonLess(0.0, neighborSideSlope)) && 0.0 < neighborManningsN && 0.0 <= neighborSurfacewaterDepth);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if ((elementSurfacewaterHead > neighborSurfacewaterHead && PONDED_DEPTH < elementSurfacewaterDepth) ||
      (elementSurfacewaterHead < neighborSurfacewaterHead && PONDED_DEPTH < neighborSurfacewaterDepth))
    {
      elementArea  = elementSurfacewaterDepth * (elementBaseWidth + elementSideSlope * elementSurfacewaterDepth);
      neighborArea = neighborSurfacewaterDepth * (neighborBaseWidth + neighborSideSlope * neighborSurfacewaterDepth);
      averageArea  = 0.5 * (elementArea + neighborArea);
      
      if (0.0 < elementSurfacewaterDepth)
        {
          elementRadius = elementArea / (elementBaseWidth + 2.0 * elementSurfacewaterDepth * sqrt(1.0 + elementSideSlope * elementSideSlope));
        }
      else
        {
          elementRadius = 0.0;
        }
      
      if (0.0 < neighborSurfacewaterDepth)
        {
          neighborRadius = neighborArea / (neighborBaseWidth + 2.0 * neighborSurfacewaterDepth * sqrt(1.0 + neighborSideSlope * neighborSideSlope));
        }
      else
        {
          neighborRadius = 0.0;
        }
      
      averageRadius     = 0.5 * (elementRadius + neighborRadius);
      velocityDirection = (0.0 <= headSlope) ? 1.0 : -1.0;
      velocityMagnitude = pow(averageRadius, 2.0 / 3.0) * sqrt(fabs(headSlope)) / averageManningsN;
      criticalVelocity  = sqrt(GRAVITY * averageDepth);
      
      // diffusive wave flow assumes small velocities.  Cap the velocity at the critical velocity.
      // FIXME discuss what we want to do here.  The really right thing to do is implement dynamic wave.
      if (velocityMagnitude > criticalVelocity)
        {
          velocityMagnitude = criticalVelocity;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 < averageDepth && 0.0 != headSlope && 0.0 < averageArea && 0.0 < averageRadius && 0.0 < criticalVelocity);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Calculate flow rate.
      *flowRate = velocityDirection * velocityMagnitude * averageArea;
      
      // Suggest new timestep.
      dtTemp = COURANT_DIFFUSIVE * distance / (velocityMagnitude + criticalVelocity);

      if (*dtNew > dtTemp)
        {
          *dtNew = dtTemp;
        }
    }
  else // if there is no slope or the higher element is dry there is no flow.
    {
      *flowRate = 0.0;
    }
}

// Calculate the surfacewater flow rate in cubic meters per second between a
// waterbody element and its stream neighbor.  Positive means flow out of the
// waterbody into the stream.  Negative means flow into the waterbody out of
// the stream.
//
// This function calculates the flow between an imaginary stream element
// representing the mouth of the waterbody and the stream neighbor.  The mouth
// of the waterbody is assumed to have the same base width, side slope, and
// Manning's n as the stream neighbor.  Its bed is assumed to be as far below
// the bank of the waterbody as the channel depth of the stream neighbor,
// although never below the waterbody's bed.  Its water depth is calculated as
// the elevation of the water in the waterbody minus the elevation of its bed,
// although never less than zero.  Its length is set to zero so that the
// distance from the mouth of the waterbody to the center of the stream
// neighbor will just be half of the length of the stream neighbor.
//
// Parameters:
//
// flowRate                   - Scalar passed by reference will be filled in
//                              with the flow rate in cubic meters per second.
// dtNew                      - Scalar passed by reference containing the
//                              suggested value for the next timestep duration
//                              in seconds.  May be updated to be shorter.
// waterbodyZBank             - Bank Z coordinate of waterbody in meters.
// waterbodyZBed              - Bed Z coordinate of waterbody in meters.
// waterbodySurfacewaterDepth - Surfacewater depth of stream in meters.
// streamZBank                - Bank Z coordinate of stream in meters.
// streamZBed                 - Bed Z coordinate of stream in meters.
// streamLength               - Length of stream in meters.
// streamBaseWidth            - Base width of stream in meters.
// streamSideSlope            - Side slope of stream, unitless.
// streamManningsN            - Surface roughness of stream.
// streamSurfacewaterDepth    - Surfacewater depth of stream in meters.
void surfacewaterWaterbodyStreamFlowRate(double* flowRate, double* dtNew, double waterbodyZBank, double waterbodyZBed, double waterbodySurfacewaterDepth,
                                         double streamZBank, double streamZBed, double streamLength, double streamBaseWidth, double streamSideSlope,
                                         double streamManningsN, double streamSurfacewaterDepth)
{
  double waterbodyMouthZBed;              // Channel bed Z coordinate of waterbody mouth in meters.
  double waterbodyMouthSurfacewaterDepth; // Surfacewater depth of waterbody mouth in meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != flowRate && NULL != dtNew && 0.0 < *dtNew && waterbodyZBank >= waterbodyZBed && 0.0 <= waterbodySurfacewaterDepth &&
           streamZBank >= streamZBed && 0.0 < streamLength && 0.0 <= streamBaseWidth && 0.0 <= streamSideSlope &&
           (epsilonLess(0.0, streamBaseWidth) || epsilonLess(0.0, streamSideSlope)) && 0.0 < streamManningsN && 0.0 <= streamSurfacewaterDepth);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Find the bed elevation and surfacewater depth of the mouth of the waterbody.
  waterbodyMouthZBed = waterbodyZBank - (streamZBank - streamZBed);
  
  if (waterbodyMouthZBed < waterbodyZBed)
    {
      waterbodyMouthZBed = waterbodyZBed;
    }
  
  waterbodyMouthSurfacewaterDepth = waterbodySurfacewaterDepth - (waterbodyMouthZBed - waterbodyZBed);
  
  if (0.0 > waterbodyMouthSurfacewaterDepth)
    {
      waterbodyMouthSurfacewaterDepth = 0.0;
    }
  
  // Calculate flow rate between an imaginary stream element representing the waterbody mouth and the stream neighbor.
  surfacewaterStreamStreamFlowRate(flowRate, dtNew, waterbodyMouthZBed, 0.0, streamBaseWidth, streamSideSlope, streamManningsN,
                                   waterbodyMouthSurfacewaterDepth, streamZBed, streamLength, streamBaseWidth, streamSideSlope, streamManningsN,
                                   streamSurfacewaterDepth);
}

// Calculate the surfacewater flow rate in cubic meters per second between a
// waterbody element and its waterbody neighbor.  Positive means flow out of
// the element into the neighbor.  Negative means flow into the element out of
// the neighbor.
//
// FIXLATER For now we are calculating flow as if the higher of the two water
// levels were flowing over a broad crested weir.  The weir elevation is set at
// the lower of elementZBank and neighborZBank.  The idea is that if the two
// elements were indicated as separate waterbodies in the source data they
// probably have some kind of bank in between them.  The weir elevation is also
// set no lower than either elementZBed, neighborZBed, or the water level in
// the element that water is flowing in to.  This calculation is correct if
// only one of the water levels is over the weir, but if both are we should
// really be using a different equation rather than assuming the higher water
// level is flowing over a weir at the elevation of the lower water level.
//
// Parameters:
//
// flowRate                  - Scalar passed by reference will be filled in
//                             with the flow rate in cubic meters per second.
// dtNew                     - Scalar passed by reference containing the
//                             suggested value for the next timestep duration
//                             in seconds.  May be updated to be shorter.
// elementZBank              - Bank Z coordinate of element in meters.
// elementZBed               - Bed Z coordinate of element in meters.
// elementLength             - Length of element in meters.
// elementBaseWidth          - Base width of element in meters.
// elementSideSlope          - Side slope of element, unitless.
// elementSurfacewaterDepth  - Surfacewater depth of element in meters.
// neighborZBank             - Bank Z coordinate of neighbor in meters.
// neighborZBed              - Bed Z coordinate of neighbor in meters.
// neighborLength            - Length of neighbor in meters.
// neighborBaseWidth         - Base width of neighbor in meters.
// neighborSideSlope         - Side slope of neighbor, unitless.
// neighborSurfacewaterDepth - Surfacewater depth of neighbor in meters.
void surfacewaterWaterbodyWaterbodyFlowRate(double* flowRate, double* dtNew, double elementZBank, double elementZBed, double elementLength,
                                            double elementBaseWidth, double elementSideSlope, double elementSurfacewaterDepth, double neighborZBank,
                                            double neighborZBed, double neighborLength, double neighborBaseWidth, double neighborSideSlope,
                                            double neighborSurfacewaterDepth)
{
  double elementSurfacewaterElevation  = elementZBed + elementSurfacewaterDepth;                                    // The elevation in meters of the element
                                                                                                                    // surfacewater.
  double neighborSurfacewaterElevation = neighborZBed + neighborSurfacewaterDepth;                                  // The elevation in meters of the neighbor
                                                                                                                    // surfacewater.
  double weirElevation                 = (elementZBank < neighborZBank) ? elementZBank : neighborZBank;             // The elevation in meters of the thing
                                                                                                                    // that water has to flow over.
  double elementTopWidth               = elementBaseWidth + 2.0 * elementSideSlope * elementSurfacewaterDepth;      // The width of element in meters.
  double neighborTopWidth              = neighborBaseWidth + 2.0 * neighborSideSlope * neighborSurfacewaterDepth;   // The width of neighbor in meters.
  double edgeLength                    = (elementTopWidth < neighborTopWidth) ? elementTopWidth : neighborTopWidth; // The width of the narrower waterbody in
                                                                                                                    // meters.
  double dtTemp;                                                                                                    // Temporary variable for suggesting new
                                                                                                                    // timestep.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != flowRate && NULL != dtNew && 0.0 < *dtNew && elementZBank >= elementZBed && 0.0 < elementLength && 0.0 <= elementBaseWidth &&
           0.0 <= elementSideSlope && (epsilonLess(0.0, elementBaseWidth) || epsilonLess(0.0, elementSideSlope)) && 0.0 <= elementSurfacewaterDepth &&
           neighborZBank >= neighborZBed && 0.0 < neighborLength && 0.0 <= neighborBaseWidth && 0.0 <= neighborSideSlope &&
           (epsilonLess(0.0, neighborBaseWidth) || epsilonLess(0.0, neighborSideSlope)) && 0.0 <= neighborSurfacewaterDepth);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Set weirElevation no lower than elementZBed.
  if (weirElevation < elementZBed)
    {
      weirElevation = elementZBed;
    }
  
  // Set weirElevation no lower than neighborZBed.
  if (weirElevation < neighborZBed)
    {
      weirElevation = neighborZBed;
    }
  
  if (elementSurfacewaterElevation > neighborSurfacewaterElevation)
    {
      // Flow from element to neighbor.  Set weirElevation no lower than neighborSurfacewaterElevation.
      if (weirElevation < neighborSurfacewaterElevation)
        {
          weirElevation = neighborSurfacewaterElevation;
        }
      
      elementSurfacewaterDepth = elementSurfacewaterElevation - weirElevation;
      
      if (0.0 < elementSurfacewaterDepth)
        {
          // Assume critical velocity flow over a broad crested weir.
          *flowRate = sqrt(GRAVITY * elementSurfacewaterDepth) * elementSurfacewaterDepth * edgeLength;
          
          // Suggest new timestep.
          dtTemp = COURANT_DIFFUSIVE * elementLength / (2.0 * sqrt(GRAVITY * elementSurfacewaterDepth));
          
          if (*dtNew > dtTemp)
            {
              *dtNew = dtTemp;
            }
        }
      else // if water elevation is below the weir there is no flow.
        {
          *flowRate = 0.0;
        }
    }
  else if (neighborSurfacewaterElevation > elementSurfacewaterElevation)
    {
      // Flow from neighbor to element.  Set weirElevation no lower than elementSurfacewaterElevation.
      if (weirElevation < elementSurfacewaterElevation)
        {
          weirElevation = elementSurfacewaterElevation;
        }
      
      neighborSurfacewaterDepth = neighborSurfacewaterElevation - weirElevation;
      
      if (0.0 < neighborSurfacewaterDepth)
        {
          // Assume critical velocity flow over a broad crested weir.  Flow from neighbor to element is negative.
          *flowRate = -sqrt(GRAVITY * neighborSurfacewaterDepth) * neighborSurfacewaterDepth * edgeLength;
          
          // Suggest new timestep.
          dtTemp = COURANT_DIFFUSIVE * neighborLength / (2.0 * sqrt(GRAVITY * neighborSurfacewaterDepth));
          
          if (*dtNew > dtTemp)
            {
              *dtNew = dtTemp;
            }
        }
      else // if water elevation is below the weir there is no flow.
        {
          *flowRate = 0.0;
        }
    }
  else // if water elevations are equal there is no flow.
    {
      *flowRate = 0.0;
    }
}

bool surfacewaterChannelChannelFlowRate(double* flowRate, double* dtNew, ChannelTypeEnum elementChannelType, double elementZBank, double elementZBed,
                                        double elementLength, double elementBaseWidth, double elementSideSlope, double elementManningsN,
                                        double elementSurfacewaterDepth, ChannelTypeEnum neighborChannelType, double neighborZBank, double neighborZBed,
                                        double neighborLength, double neighborBaseWidth, double neighborSideSlope, double neighborManningsN,
                                        double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
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
  
  if (!(STREAM == elementChannelType || WATERBODY == elementChannelType || ICEMASS == elementChannelType))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: elementChannelType must be a valid channel type value.\n");
      error = true;
    }
  
  if (!(elementZBank >= elementZBed))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: elementZBank must be greater than or equal to elementZBed");
      error = true;
    }
  
  if (!(0.0 < elementLength))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: elementLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= elementBaseWidth))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: elementBaseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= elementSideSlope))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: elementSideSlope must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(epsilonGreater(elementBaseWidth, 0.0) || epsilonGreater(elementSideSlope, 0.0)))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: at least one of elementBaseWidth or elementSideSlope must be epsilon greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementManningsN))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: elementManningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: elementSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(STREAM == neighborChannelType || WATERBODY == neighborChannelType || ICEMASS == neighborChannelType))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: neighborChannelType must be a valid channel type value.\n");
      error = true;
    }
  
  if (!(neighborZBank >= neighborZBed))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: neighborZBank must be greater than or equal to neighborZBed");
      error = true;
    }
  
  if (!(0.0 < neighborLength))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: neighborLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborBaseWidth))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: neighborBaseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborSideSlope))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: neighborSideSlope must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(epsilonGreater(neighborBaseWidth, 0.0) || epsilonGreater(neighborSideSlope, 0.0)))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: at least one of neighborBaseWidth or neighborSideSlope must be epsilon greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborManningsN))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: neighborManningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in surfacewaterChannelChannelFlowRate: neighborSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      if (STREAM == elementChannelType && STREAM == neighborChannelType)
        {
          surfacewaterStreamStreamFlowRate(flowRate, dtNew, elementZBed, elementLength, elementBaseWidth, elementSideSlope, elementManningsN,
                                           elementSurfacewaterDepth, neighborZBed, neighborLength, neighborBaseWidth, neighborSideSlope, neighborManningsN,
                                           neighborSurfacewaterDepth);
        }
      else if ((WATERBODY == elementChannelType || ICEMASS == elementChannelType) && STREAM == neighborChannelType)
        {
          surfacewaterWaterbodyStreamFlowRate(flowRate, dtNew, elementZBank, elementZBed, elementSurfacewaterDepth, neighborZBank, neighborZBed,
                                              neighborLength, neighborBaseWidth, neighborSideSlope, neighborManningsN, neighborSurfacewaterDepth);
        }
      else if (STREAM == elementChannelType && (WATERBODY == neighborChannelType || ICEMASS == neighborChannelType))
        {
          surfacewaterWaterbodyStreamFlowRate(flowRate, dtNew, neighborZBank, neighborZBed, neighborSurfacewaterDepth, elementZBank, elementZBed,
                                              elementLength, elementBaseWidth, elementSideSlope, elementManningsN, elementSurfacewaterDepth);
          *flowRate *= -1.0; // Make flow out of the stream element be positive and flow out of the waterbody neighbor be negative.
        }
      else if ((WATERBODY == elementChannelType || ICEMASS == elementChannelType) && (WATERBODY == neighborChannelType || ICEMASS == neighborChannelType))
        {
          surfacewaterWaterbodyWaterbodyFlowRate(flowRate, dtNew, elementZBank, elementZBed, elementLength, elementBaseWidth, elementSideSlope,
                                                 elementSurfacewaterDepth, neighborZBank, neighborZBed, neighborLength, neighborBaseWidth, neighborSideSlope,
                                                 neighborSurfacewaterDepth);
        }
      // else this would be something like an internal boundary condition, which we haven't implemented yet.  flowRate has already been assigned to zero above.
    }
  
  return error;
}
