#ifndef __SURFACEWATER_H__
#define __SURFACEWATER_H__

#include "all.h"

// Calculate the surfacewater flow rate in cubic meters per second across a
// mesh boundary edge.  Positive means flow out of the element across the edge.
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
bool surfacewaterMeshBoundaryFlowRate(double* flowRate, BoundaryConditionEnum boundary, double inflowXVelocity, double inflowYVelocity, double inflowHeight,
                                      double edgeLength, double edgeNormalX, double edgeNormalY, double surfacewaterDepth);

// Calculate the surfacewater flow rate in cubic meters per second across a
// channel boundary edge.  Positive means flow out of the element across the
// edge.  Negative means flow into the element across the edge.
//
// If boundary is NOFLOW, flowRate is forced to be zero.  If boundary is
// INFLOW, flowRate is forced to be non-positive.  If boundary is OUTFLOW,
// flowRate is forced to be non-negative.
//
// The governing equation for outflow is:
//
// assume free outflow boundary condtion: u = sqrt(g * A/T);
// where   u = velocity,                  [m/s];
//         g = gravitational accerlation, [9.81 m/s^2];
//         A = flow cross section area,   [m^2];
//         T = channel top width,         [m];
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate       - Scalar passed by reference will be filled in with the flow
//                  rate in cubic meters per second.
// dtNew          - Scalar passed by reference containing the suggested value
//                  for the next timestep duration.  May be updated to be
//                  shorter.
// boundary       - What type of boundary.
// inflowVelocity - Water velocity in meters per second for INFLOW boundary.
//                  Ignored for NOFLOW or OUTFLOW boundary.
// inflowHeight   - Flow height in meters for INFLOW boundary.  Ignored for
//                  NOFLOW or OUTFLOW boundary.
// length         - Length of element in meters.
// baseWidth      - Base width of element in meters.
// sideSlope      - Side slope of element, unitless.
// WaterDepth     - Water depth of element in meters.
bool surfacewaterChannelBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowVelocity, double inflowHeight,
                                         double length, double baseWidth, double sideSlope, double waterDepth);

// Calculate the surfacewater flow rate in cubic meters per second between a
// mesh element and its mesh neighbor.  Positive means flow out of the element
// into the neighbor.  Negative means flow into the element out of the
// neighbor.
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
// elementZSurface           - Surface Z coordinate of element center in
//                             meters.
// elementArea               - Area of element in square meters.
// elementManningsN          - Surface roughness of element.
// elementSurfacewaterDepth  - Surfacewater depth of element in meters.
// neighborX                 - X coordinate of neighbor center in meters.
// neighborY                 - Y coordinate of neighbor center in meters.
// neighborZSurface          - Surface Z coordinate of neighbor center in
//                             meters.
// neighborManningsN         - Surface roughness of neighbor.
// neighborSurfacewaterDepth - Surfacewater depth of neighbor in meters.
bool surfacewaterMeshMeshFlowRate(double* flowRate, double* dtNew, double edgeLength, double elementX, double elementY, double elementZSurface,
                                  double elementArea, double elementManningsN, double elementSurfacewaterDepth, double neighborX, double neighborY,
                                  double neighborZSurface, double neighborManningsN, double neighborSurfacewaterDepth);

// Calculate the surfacewater flow rate in cubic meters per second between a
// mesh element and a channel element.  Positive means flow out of the mesh
// element into the channel element.  Negative means flow out of the channel
// element into the mesh element.
//
// For now we are assuming flow is one way from the mesh element into the
// channel and the flow rate is the same as if it were a mesh outflow boundary.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate          - Scalar passed by reference will be filled in with the
//                     flow rate in cubic meters per second.
// edgeLength        - Length of edge in meters.
// surfacewaterDepth - Surfacewater depth of element in meters.
bool surfacewaterMeshChannelFlowRate(double* flowRate, double edgeLength, double surfacewaterDepth);

// Calculate the surfacewater flow rate in cubic meters per second between a
// channel element and its channel neighbor.  Positive means flow out of the
// element into the neighbor.  Negative means flow into the element out of the
// neighbor.
//
// Currently, this function only calculates diffusive wave flow.
//
// The governing equation for diffusive wave is:
//
//        Q   = - R^(2/3) * A * Z_x / (n * sqrt(abs(Z_x)));
// where  Q   = flow rate (flux),   in [m^3/s];
//        R   = hydraulic radius,   in [m];
//        A   = cross section area, in [m^2];
//        Z   = water surface elevation, in [m];
//        Z_x = slope of water surface elevation, [-].
//        n   = Manning's roughness coefficint, in [s/m^(-1/3)];
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate           - Scalar passed by reference will be filled in with the
//                      flow rate in cubic meters per second.
// dtNew              - Scalar passed by reference containing the suggested
//                      value for the next timestep duration.  May be updated
//                      to be shorter.
// elementZBed        - Channel bed Z coordinate of element in meters.
// elementWaterDepth  - Water depth of element in meters.
// neighborZBed       - Channel bed Z coordinate of neighbor in meters.
// neighborWaterDepth - Water depth of neighbor in meters.
// distance           - Distance along channel between element and neighbor.
// manningsN          - Mannings n to use in flow calculation.
// area               - Wetted cross sectional area to use in flow calculation.
// radius             - Hydraulic radius to use in flow calculation.
bool surfacewaterChannelChannelFlowRate(double* flowRate, double* dtNew, double elementZBed, double elementWaterDepth, double neighborZBed,
                                        double neighborWaterDepth, double distance, double manningsN, double area, double radius);

#endif // __SURFACEWATER_H__
