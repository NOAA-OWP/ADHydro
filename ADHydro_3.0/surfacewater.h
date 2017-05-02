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
// dtNew             - Scalar passed by reference containing the suggested
//                     value for the next timestep duration in seconds.  May be
//                     updated to be shorter.
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
// elementArea       - Area of element in square meters.
// surfacewaterDepth - Surfacewater depth of element in meters.
bool surfacewaterMeshBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowXVelocity, double inflowYVelocity,
                                      double inflowHeight, double edgeLength, double edgeNormalX, double edgeNormalY, double elementArea,
                                      double surfacewaterDepth);

// Calculate the surfacewater flow rate in cubic meters per second across a
// channel boundary edge.  Positive means flow out of the element across the
// edge.  Negative means flow into the element across the edge.
//
// If boundary is NOFLOW, flowRate is forced to be zero.  If boundary is
// INFLOW, flowRate is forced to be non-positive.  If boundary is OUTFLOW,
// flowRate is forced to be non-negative.
//
// The governing equation for outflow is a free outflow boundary condtion:
//
// u = sqrt(g*A/T);               [m/s]
// u = velocity,                  [m/s];
// g = gravitational accerlation, [9.81 m/s^2];
// A = flow cross section area,   [m^2];
// T = channel top width,         [m];
//
// Therefore:
//
// Q = flow rate (flux) = u*A,    [m^3/s];
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate          - Scalar passed by reference will be filled in with the
//                     flow rate in cubic meters per second.
// dtNew             - Scalar passed by reference containing the suggested
//                     value for the next timestep duration in seconds.  May be
//                     updated to be shorter.
// boundary          - What type of boundary.
// inflowVelocity    - Water velocity in meters per second for INFLOW boundary.
//                     Ignored for NOFLOW or OUTFLOW boundary.
// inflowHeight      - Flow height in meters for INFLOW boundary.  Ignored for
//                     NOFLOW or OUTFLOW boundary.
// elementLength     - Length of element in meters.
// baseWidth         - Base width of element in meters.
// sideSlope         - Side slope of element, unitless.
// surfacewaterDepth - Water depth of element in meters.
bool surfacewaterChannelBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowVelocity, double inflowHeight,
                                         double elementLength, double baseWidth, double sideSlope, double surfacewaterDepth);

// Calculate the surfacewater flow rate in cubic meters per second between a
// mesh element and its mesh neighbor.  Positive means flow out of the element
// into the neighbor.  Negative means flow into the element out of the
// neighbor.
//
// The governing equation is the diffusive wave equation:
//
// (dh/dt) = -divergent(-h*k*grad(Z))+qr = -divergernt(-h^(5/3)*grad(Z)/(manning_n*(grad(Z))^1/2))+qr;
//  h      = overland water depth,                                          [m];
//  Z      = water surface elevation = water depth+ground elevation = h+zg, [m];
//  qr     = rainfall rate,                                                 [m/s];
//  k      = conductance = h^(2/3)/(manning_n*(grad(Z))^1/2),               [m/s];
//
//  Therefore:
//
//  f      = flux per unit edge length = -h*k*grad(Z),                      [m^2/s];
//  Q      = flow rate (flux) = f*edge length,                              [m^3/s];
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate                  - Scalar passed by reference will be filled in
//                             with the flow rate in cubic meters per second.
// dtNew                     - Scalar passed by reference containing the
//                             suggested value for the next timestep duration
//                             in seconds.  May be updated to be shorter.
// edgeLength                - Length of common edge in meters.
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
// neighborArea              - Area of neighbor in square meters.
// neighborManningsN         - Surface roughness of neighbor.
// neighborSurfacewaterDepth - Surfacewater depth of neighbor in meters.
bool surfacewaterMeshMeshFlowRate(double* flowRate, double* dtNew, double edgeLength, double elementX, double elementY, double elementZSurface,
                                  double elementArea, double elementManningsN, double elementSurfacewaterDepth, double neighborX, double neighborY,
                                  double neighborZSurface, double neighborArea, double neighborManningsN, double neighborSurfacewaterDepth);

// Calculate the surfacewater flow rate in cubic meters per second between a
// mesh element and a channel element.  Positive means flow out of the mesh
// element into the channel element.  Negative means flow out of the channel
// element into the mesh element.
//
// FIXLATER For now we are calculating flow as if the higher of the mesh or
// channel water levels were flowing over a broad crested weir.  The weir
// elevation is set at the highest of meshZSurface, channelZBank, or the water
// level in the element that water is flowing in to.  This calculation is
// correct if only one of the water levels is over the weir (the higher of
// meshZSurface or channelZBank), but if both are we should really be using a
// different equation rather than assuming the higher water level is flowing
// over a weir at the elevation of the lower water level.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate                 - Scalar passed by reference will be filled in with
//                            the flow rate in cubic meters per second.
// dtNew                    - Scalar passed by reference containing the
//                            suggested value for the next timestep duration
//                            in seconds.  May be updated to be shorter.
// edgeLength               - Length of common edge in meters.
// meshZSurface             - Surface Z coordinate of mesh element in meters.
// meshArea                 - Area of mesh element in square meters.
// meshSurfacewaterDepth    - Surfacewater depth of mesh element in meters.
// channelZBank             - Bank Z coordinate of channel element in meters.
// channelZBed              - Bed Z coordinate of channel element in meters.
// channelBaseWidth         - Base width of channel element in meters.
// channelSideSlope         - Side slope of channel element, unitless.
// channelSurfacewaterDepth - Surfacewater depth of channel element in meters.
bool surfacewaterMeshChannelFlowRate(double* flowRate, double* dtNew, double edgeLength, double meshZSurface, double meshArea, double meshSurfacewaterDepth,
                                     double channelZBank, double channelZBed, double channelBaseWidth, double channelSideSlope,
                                     double channelSurfacewaterDepth);

// Calculate the surfacewater flow rate in cubic meters per second between a
// channel element and its channel neighbor.  Positive means flow out of the
// element into the neighbor.  Negative means flow into the element out of
// the neighbor.
//
// The equation for calculating the flow rate depends on the channel types of
// the two elements.  For details see comments of private functions in
// surfacewater.cpp.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate                  - Scalar passed by reference will be filled in
//                             with the flow rate in cubic meters per second.
// dtNew                     - Scalar passed by reference containing the
//                             suggested value for the next timestep duration
//                             in seconds.  May be updated to be shorter.
// elementChannelType        - Channel type of element.
// elementZBank              - Bank Z coordinate of element in meters.
// elementZBed               - Bed Z coordinate of element in meters.
// elementLength             - Length of element in meters.
// elementBaseWidth          - Base width of element in meters.
// elementSideSlope          - Side slope of element, unitless.
// elementManningsN          - Surface roughness of element.
// elementSurfacewaterDepth  - Surfacewater depth of element in meters.
// neighborChannelType       - Channel type of neighbor.
// neighborZBank             - Bank Z coordinate of neighbor in meters.
// neighborZBed              - Bed Z coordinate of neighbor in meters.
// neighborLength            - Length of neighbor in meters.
// neighborBaseWidth         - Base width of neighbor in meters.
// neighborSideSlope         - Side slope of neighbor, unitless.
// neighborManningsN         - Surface roughness of neighbor.
// neighborSurfacewaterDepth - Surfacewater depth of neighbor in meters.
bool surfacewaterChannelChannelFlowRate(double* flowRate, double* dtNew, ChannelTypeEnum elementChannelType, double elementZBank, double elementZBed,
                                        double elementLength, double elementBaseWidth, double elementSideSlope, double elementManningsN,
                                        double elementSurfacewaterDepth, ChannelTypeEnum neighborChannelType, double neighborZBank, double neighborZBed,
                                        double neighborLength, double neighborBaseWidth, double neighborSideSlope, double neighborManningsN,
                                        double neighborSurfacewaterDepth);

#endif // __SURFACEWATER_H__
