#include "groundwater.h"
#include <charm++.h>

// Comment in .h file.
bool groundwaterBoundaryFlowRate(double* flowRate, BoundaryConditionEnum boundary, double vertexX[4], double vertexY[4], double vertexZSurface[4],
                                 double edgeLength, double edgeNormalX, double edgeNormalY, double elementZBedrock, double elementArea, double conductivity,
                                 double groundwaterHead)
{
  bool   error = false; // Error flag.
  double slopeX;        // X component of slope off the edge of the mesh.
  double slopeY;        // Y component of slope off the edge of the mesh.
  
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
  
  if (!(NULL != vertexX))
    {
      CkError("ERROR: vertexX must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != vertexY))
    {
      CkError("ERROR: vertexY must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != vertexZSurface))
    {
      CkError("ERROR: vertexZSurface must not be NULL.\n");
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
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < conductivity))
    {
      CkError("ERROR: conductivity must be greater than zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If boundary is NOFLOW flowRate has already been assigned to zero above.
  if (!error && (INFLOW == boundary || OUTFLOW == boundary) && groundwaterHead > elementZBedrock)
    {
      // Calculate assumed slope off the edge of the mesh.
      // FIXME do we want to use vertexZBedrock coordinates to calculate assumed slope of water table?
      slopeX = ((vertexY[3] - vertexY[1]) * (vertexZSurface[2] - vertexZSurface[1]) +
                (vertexY[1] - vertexY[2]) * (vertexZSurface[3] - vertexZSurface[1])) / (2.0 * elementArea);
      slopeY = ((vertexX[1] - vertexX[3]) * (vertexZSurface[2] - vertexZSurface[1]) +
                (vertexX[2] - vertexX[1]) * (vertexZSurface[3] - vertexZSurface[1])) / (2.0 * elementArea);

      *flowRate = conductivity * (groundwaterHead - elementZBedrock) * -(slopeX * edgeNormalX + slopeY * edgeNormalY) * edgeLength;

      if (INFLOW == boundary && 0.0 < *flowRate)
        {
          CkError("WARNING: Outward flow at INFLOW boundary.  Setting flow to zero.\n");
          *flowRate = 0.0;
        }
      else if (OUTFLOW == boundary && 0.0 > *flowRate)
        {
          CkError("WARNING: Inward flow at OUTFLOW boundary.  Setting flow to zero.\n");
          *flowRate = 0.0;
        }
    }

  return error;
}

// Comment in .h file.
bool groundwaterElementNeighborFlowRate(double* flowRate, double edgeLength, double elementX, double elementY, double elementZSurface, double elementZBedrock,
                                        double elementConductivity, double elementSurfacewaterDepth, double elementGroundwaterHead, double neighborX,
                                        double neighborY, double neighborZSurface, double neighborZBedrock, double neighborConductivity,
                                        double neighborSurfacewaterDepth, double neighborGroundwaterHead)
{
  bool   error                     = false;                                                         // Error flag.
  double elementGroundwaterHeight  = elementGroundwaterHead  - elementZBedrock;                     // Groundwater height of element.
  double neighborGroundwaterHeight = neighborGroundwaterHead - neighborZBedrock;                    // Groundwater height of neighbor.
  double averageConductivity       = 0.5 * (elementConductivity + neighborConductivity);            // Conductivity to use in flow calculation.
  double averageHeight             = 0.5 * (elementGroundwaterHeight + neighborGroundwaterHeight);  // Height to use in flow calculation.
  double distance                  = sqrt((elementX - neighborX) * (elementX - neighborX) + (elementY - neighborY) * (elementY - neighborY));
                                                                                                    // Distance between element and neighbor centers.
  double headSlope                 = (elementGroundwaterHead - neighborGroundwaterHead) / distance; // Slope of water table.
  
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
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(elementZSurface >= elementZBedrock))
    {
      CkError("ERROR: elementZSurface must be greater than or equal to elementZBedrock");
      error = true;
    }
  
  if (!(elementZSurface >= elementGroundwaterHead))
    {
      CkError("ERROR: elementZSurface must be greater than or equal to elementGroundwaterHead");
      error = true;
    }
  
  if (!(0.0 < elementConductivity))
    {
      CkError("ERROR: elementConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR: elementSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(neighborZSurface >= neighborZBedrock))
    {
      CkError("ERROR: neighborZSurface must be greater than or equal to neighborZBedrock");
      error = true;
    }
  
  if (!(neighborZSurface >= neighborGroundwaterHead))
    {
      CkError("ERROR: neighborZSurface must be greater than or equal to neighborGroundwaterHead");
      error = true;
    }
  
  if (!(0.0 < neighborConductivity))
    {
      CkError("ERROR: neighborConductivity must be greater than zero.\n");
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
      // If groundwater head is below bedrock set height to zero.
      if (0.0 > elementGroundwaterHeight)
        {
          elementGroundwaterHeight = 0.0;
        }

      if (0.0 > neighborGroundwaterHeight)
        {
          neighborGroundwaterHeight = 0.0;
        }

      // If groundwater head is at the surface add surfacewater depth.
      if (elementGroundwaterHead == elementZSurface)
        {
          elementGroundwaterHead += elementSurfacewaterDepth;
        }

      if (neighborGroundwaterHead == neighborZSurface)
        {
          neighborGroundwaterHead += neighborSurfacewaterDepth;
        }

      // If there is no slope or the higher element is dry there is no flow.
      if ((elementGroundwaterHead > neighborGroundwaterHead && 0.0 < elementGroundwaterHeight) ||
          (elementGroundwaterHead < neighborGroundwaterHead && 0.0 < neighborGroundwaterHeight))
        {
          *flowRate = averageConductivity * averageHeight * headSlope * edgeLength;
        }
    }

  return error;
}
