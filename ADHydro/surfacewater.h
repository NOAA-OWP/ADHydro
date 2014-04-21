#ifndef __SURFACEWATER_H__
#define __SURFACEWATER_H__

#include "all.h"

// Calculate the surfacewater flow rate in cubic meters per second across a
// boundary edge.  Positive means flow out of the element across the edge.
// Negative means flow into the element across the edge.
//
// If boundary is NOFLOW, flowRate is forced to be zero.  If boundary is
// INFLOW, flowRate is forced to be non-positive.  If boundary is OUTFLOW,
// flowRate is forced to be non-negative.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate          - Scalar passed by reference will be filled in with the
//                     flow rate in cubic meters per second.
// boundary          - What type of boundary.
// inflowXVelocity   - X component of water velocity in meters per second for
//                     INFLOW boundary.  Ignored for NOFLOW or OUTFLOW
//                     boundary.
// inflowYVelocity   - Y component of water velocity in meters per second for
//                     INFLOW boundary.  Ignored for NOFLOW or OUTFLOW
//                     boundary.
// inflowHeight      - Flow height in meters for INFLOW boundary.  Ignored for
//                     NOFLOW or OUTFLOW boundary.
// edgeLength        - Length of edge in meters.
// edgeNormalX       - X component of edge normal unit vector.
// edgeNormalY       - Y component of edge normal unit vector.
// surfacewaterDepth - Surfacewater depth of element in meters.
bool surfacewaterBoundaryFlowRate(double* flowRate, BoundaryConditionEnum boundary, double inflowXVelocity, double inflowYVelocity, double inflowHeight,
                                  double edgeLength, double edgeNormalX, double edgeNormalY, double surfacewaterDepth);

// Calculate the surfacewater flow rate in cubic meters per second between an
// element and its neighbor.  Positive means flow out of the element into the
// neighbor.  Negative means flow into the element out of the neighbor.
//
// The governing equation for diffusive wave is:
// 
// (dh/dt) = - divergent(-h*k*grad(Z))+qr = -divergernt(- h^(5/3)*grad(Z)/(manning_n*(grad(Z))^1/2))+qr
//  h      = overland water depth
//  Z      = water surface elevation = water depth + ground elevation(h+zg)
//  qr     = rainfall rate [m/s]
//  k      = conductance = h^(2/3)/(manning_n*(grad(Z))^1/2) in [m/s]
//  the flux across edge = -h*k*grad(Z) in [m^2/s]
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate                  - Scalar passed by reference will be filled in
//                             with the flow rate in cubic meters per second.
// dtNew                     - Scalar passed by reference containing the
//                             suggested value for the next timestep duration.
//                             May be updated to be shorter.
// edgeLength                - Length of edge in meters.
// elementX                  - X coordinate of element center in meters.
// elementY                  - Y coordinate of element center in meters.
// elementZSurface           - Surface z coordinate of element center in
//                             meters.
// elementManningsN          - Surface roughness of element.
// elementSurfacewaterDepth  - Surfacewater depth of element in meters.
// neighborX                 - X coordinate of neighbor center in meters.
// neighborY                 - Y coordinate of neighbor center in meters.
// neighborZSurface          - Surface z coordinate of neighbor center in
//                             meters.
// neighborManningsN         - Surface roughness of neighbor.
// neighborSurfacewaterDepth - Surfacewater depth of neighbor in meters.
bool surfacewaterElementNeighborFlowRate(double* flowRate, double* dtNew, double edgeLength, double elementX, double elementY, double elementZSurface,
                                         double elementArea, double elementManningsN, double elementSurfacewaterDepth, double neighborX, double neighborY,
                                         double neighborZSurface, double neighborManningsN, double neighborSurfacewaterDepth);

#endif // __SURFACEWATER_H__
