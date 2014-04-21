#include "surfacewater.h"
#include <charm++.h>

// Comment in .h file.
bool surfacewaterBoundaryFlowRate(double* flowRate, BoundaryConditionEnum boundary, double inflowXVelocity, double inflowYVelocity, double inflowHeight,
                                  double edgeLength, double edgeNormalX, double edgeNormalY, double surfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != flowRate))
    {
      CkError("ERROR: flowRate must not be NULL.\n");
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
      CkError("ERROR: boundary must be a valid boundary condition value.\n");
      error = true;
    }
  
  if (!(0 <= inflowHeight))
    {
      CkError("ERROR: inflowHeight must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(epsilonEqual(1.0, edgeNormalX * edgeNormalX + edgeNormalY * edgeNormalY)))
    {
      CkError("ERROR: edgeNormalX and edgeNormalY must make a unit vector.\n");
      error = true;
    }
  
  if (!(0 <= surfacewaterDepth))
    {
      CkError("ERROR: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // If boundary is NOFLOW flowRate has already been assigned to zero above.
      if (INFLOW == boundary)
        {
          *flowRate = (inflowXVelocity * edgeNormalX + inflowYVelocity * edgeNormalY) * inflowHeight * edgeLength;
          
          if (0.0 < *flowRate)
            {
              CkError("WARNING: Outward flow at INFLOW boundary.  Setting flow to zero.\n");
              *flowRate = 0.0;
            }
        }
      else if (OUTFLOW == boundary && PONDED_DEPTH < surfacewaterDepth)
        {
          *flowRate = sqrt(GRAVITY * surfacewaterDepth) * surfacewaterDepth * edgeLength;
          
          if (0.0 > *flowRate)
            {
              CkError("WARNING: Inward flow at OUTFLOW boundary.  Setting flow to zero.\n");
              *flowRate = 0.0;
            }
        }
    }
  
  return error;
}

// Comment in .h file.
bool surfacewaterElementNeighborFlowRate(double* flowRate, double* dtNew, double edgeLength, double elementX, double elementY, double elementZSurface,
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
      CkError("ERROR: flowRate must not be NULL.\n");
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
      CkError("ERROR: dtNew must not be NULL and must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < elementManningsN))
    {
      CkError("ERROR: elementManningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR: elementSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborManningsN))
    {
      CkError("ERROR: neighborManningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR: neighborSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < distance))
    {
      CkError("ERROR: Distance between element and neighbor centers must be greater than zero.\n");
      error = true;
    }
  
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // If there is no slope or the higher element is dry there is no flow.
  if (!error && ((elementSurfacewaterHead > neighborSurfacewaterHead && PONDED_DEPTH < elementSurfacewaterDepth) ||
                 (elementSurfacewaterHead < neighborSurfacewaterHead && PONDED_DEPTH < neighborSurfacewaterDepth)))
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 < averageDepth && 0.0 != headSlope);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      *flowRate = (pow(averageDepth, 5.0 / 3.0) / (averageManningsN * sqrt(fabs(headSlope)))) * headSlope * edgeLength;

      // Suggest new timestep.
      // FIXME if this calculation is only done by one of the neighbors only that neighbor's area will be used to limit the timestep.
      dtTemp = 0.1 * sqrt(2.0 * elementArea) / (pow(averageDepth, 2.0 / 3.0) * sqrt(fabs(headSlope)) / averageManningsN + sqrt(GRAVITY * averageDepth));

      if (*dtNew > dtTemp)
        {
          *dtNew = dtTemp;
        }
    }

  return error;
}
