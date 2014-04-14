#ifndef __GROUNDWATER_H__
#define __GROUNDWATER_H__

#include "all.h"

// Calculate the groundwater flow rate in cubic meters per second across a
// boundary edge.  Positive means flow out of the element across the edge.
// Negative means flow into the element across the edge.
//
// If boundary is NOFLOW, flowRate is forced to be zero.  If boundary is
// INFLOW, flowRate is forced to be non-positive.  If boundary is OUTFLOW,
// flowRate is forced to be non-negative.
//
// Parameters:
//
// flowRate        - Scalar passed by reference will be filled in with the flow
//                   rate in cubic meters per second.
// boundary        - What type of boundary.
// vertexX         - Array containing x coordinates of element vertices in
//                   meters.
// vertexY         - Array containing y coordinates of element vertices in
//                   meters.
// vertexZSurface  - Array containing surface z coordinates of element vertices
//                   in meters.
// edgeLength      - Length of edge in meters.
// edgeNormalX     - X component of edge normal unit vector.
// edgeNormalY     - Y component of edge normal unit vector.
// elementZBedrock - Bedrock z coordinate of element center in meters.
// elementArea     - Area of element in square meters.
// conductivity    - Hydraulic conductivity of element in meters per second.
// groundwaterHead - Groundwater head of element in meters.
bool groundwaterBoundaryFlowRate(double* flowRate, BoundaryConditionEnum boundary, double vertexX[4], double vertexY[4], double vertexZSurface[4],
                                 double edgeLength, double edgeNormalX, double edgeNormalY, double elementZBedrock, double elementArea, double conductivity,
                                 double groundwaterHead);

// Calculate the groundwater flow rate in cubic meters per second between an
// element and its neighbor.  Positive means flow out of the element into the
// neighbor.  Negative means flow into the element out of the neighbor.
//
// The governing equation for 2D groundwater flow in an unconfined aquifer is
// the Boussinesq equation:
//
// Sy(dH/dt) = -divergent(-K*h*gradient(H)) + R;
// Sy        = Specific yield, can be approximated by porosity;
// H         = groundwater pressure head, [m];
// h         = groundwater depth, [m] = H - zb;
// K         = hydraulic conductivity, [m/s];
// R         = Recharge rate,[m/s]; (But we pass the recharge rate in meters.)
// flux      = -K*h*gradient(H), in [m^2/s];
//
// Parameters:
//
// flowRate                  - Scalar passed by reference will be filled in
//                             with the flow rate in cubic meters per second.
// edgeLength                - Length of edge in meters.
// elementX                  - X coordinate of element center in meters.
// elementY                  - Y coordinate of element center in meters.
// elementZSurface           - Surface z coordinate of element center in
//                             meters.
// elementZBedrock           - Bedrock z coordinate of element center in
//                             meters.
// elementConductivity       - Hydraulic conductivity of element in meters per
//                             second.
// elementSurfacewaterDepth  - Surfacewater depth of element in meters.
// elementGroundwaterHead    - Groundwater head of element in meters.
// neighborX                 - X coordinate of neighbor center in meters.
// neighborY                 - Y coordinate of neighbor center in meters.
// neighborZSurface          - Surface z coordinate of neighbor center in
//                             meters.
// neighborZBedrock          - Bedrock z coordinate of neighbor center in
//                             meters.
// neighborConductivity      - Hydraulic conductivity of neighbor in meters per
//                             second.
// neighborSurfacewaterDepth - Surfacewater depth of neighbor in meters.
// neighborGroundwaterHead   - Groundwater head of neighbor in meters.
bool groundwaterElementNeighborFlowRate(double* flowRate, double edgeLength, double elementX, double elementY, double elementZSurface, double elementZBedrock,
                                        double elementConductivity, double elementSurfacewaterDepth, double elementGroundwaterHead, double neighborX,
                                        double neighborY, double neighborZSurface, double neighborZBedrock, double neighborConductivity,
                                        double neighborSurfacewaterDepth, double neighborGroundwaterHead);

#endif // __GROUNDWATER_H__
