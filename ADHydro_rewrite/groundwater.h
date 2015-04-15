#ifndef __GROUNDWATER_H__
#define __GROUNDWATER_H__

#include "all_charm.h"

// Calculate the groundwater flow rate in cubic meters per second across a mesh
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
// flowRate        - Scalar passed by reference will be filled in with the flow
//                   rate in cubic meters per second.
// dtNew           - Scalar passed by reference containing the suggested value
//                   for the next timestep duration in seconds.  May be updated
//                   to be shorter.
// boundary        - What type of boundary.
// inflowHeight    - Flow height in meters for INFLOW boundary.  Ignored for
//                   NOFLOW or OUTFLOW boundary.
// edgeLength      - Length of edge in meters.
// edgeNormalX     - X component of edge normal unit vector.
// edgeNormalY     - Y component of edge normal unit vector.
// elementZBedrock - Bedrock Z coordinate of element center in meters.
// elementArea     - Area of element in square meters.
// elementSlopeX   - Surface slope in X direction, unitless.
// elementSlopeY   - Surface slope in Y direction, unitless.
// conductivity    - Hydraulic conductivity of element in meters per second.
// porosity        - Porosity, unitless.
// groundwaterHead - Groundwater head of element in meters.
bool groundwaterMeshBoundaryFlowRate(double* flowRate, double* dtNew, BoundaryConditionEnum boundary, double inflowHeight, double edgeLength,
                                     double edgeNormalX, double edgeNormalY, double elementZBedrock, double elementArea, double elementSlopeX,
                                     double elementSlopeY, double conductivity, double porosity, double groundwaterHead);

// Calculate the groundwater flow rate in cubic meters per second between a
// mesh element and its mesh neighbor.  Positive means flow out of the element
// into the neighbor.  Negative means flow into the element out of the
// neighbor.
//
// The governing equation for 2D groundwater flow in an unconfined aquifer is
// the Boussinesq equation:
//
// Sy(dH/dt) = -divergent(-h*K*grad(H))+R;
// Sy        = Specific yield, can be approximated by porosity,                 [-];
// H         = groundwater pressure head,                                       [m];
// h         = groundwater depth = pressure head-layer bottom elevation = H-zb, [m];
// K         = hydraulic conductivity,                                          [m/s];
// R         = Recharge rate,                                                   [m/s];
//
// Therefore:
//
// f         = flux per unit edge length = -h*K*grad(H),                        [m^2/s];
// Q         = flow rate (flux) = f*edge length,                                [m^3/s];
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
// elementZBedrock           - Bedrock Z coordinate of element center in
//                             meters.
// elementArea               - Area of element in square meters.
// elementConductivity       - Hydraulic conductivity of element in meters per
//                             second.
// elementPorosity           - Porosity of element, unitless.
// elementSurfacewaterDepth  - Surfacewater depth of element in meters.
// elementGroundwaterHead    - Groundwater head of element in meters.
// neighborX                 - X coordinate of neighbor center in meters.
// neighborY                 - Y coordinate of neighbor center in meters.
// neighborZSurface          - Surface Z coordinate of neighbor center in
//                             meters.
// neighborZBedrock          - Bedrock Z coordinate of neighbor center in
//                             meters.
// neighborArea              - Area of neighbor in square meters.
// neighborConductivity      - Hydraulic conductivity of neighbor in meters per
//                             second.
// neighborPorosity          - Porosity of neighbor, unitless.
// neighborSurfacewaterDepth - Surfacewater depth of neighbor in meters.
// neighborGroundwaterHead   - Groundwater head of neighbor in meters.
bool groundwaterMeshMeshFlowRate(double* flowRate, double* dtNew, double edgeLength, double elementX, double elementY, double elementZSurface,
                                 double elementZBedrock, double elementArea, double elementConductivity, double elementPorosity,
                                 double elementSurfacewaterDepth, double elementGroundwaterHead, double neighborX, double neighborY, double neighborZSurface,
                                 double neighborZBedrock, double neighborArea, double neighborConductivity, double neighborPorosity,
                                 double neighborSurfacewaterDepth, double neighborGroundwaterHead);

// Calculate the groundwater flow rate in cubic meters per second between a
// mesh element and a channel element.  Positive means flow out of the mesh
// element into the channel element.  Negative means flow out of the channel
// element into the mesh element.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// flowRate                 - Scalar passed by reference will be filled in with
//                            the flow rate in cubic meters per second.
// edgeLength               - Length of intersection of mesh and channel
//                            elements in meters.
// meshZSurface             - Surface Z coordinate of mesh element in meters.
// meshZBedrock             - Bedrock Z coordinate of mesh element in meters.
// meshSurfacewaterDepth    - Surfacewater depth of mesh element in meters.
// meshGroundwaterHead      - Groundwater head of mesh element in meters.
// channelZBank             - Bank Z coordinate of channel element in meters.
// channelZBed              - Bed Z coordinate of channel element in meters.
// channelBaseWidth         - Base width of channel element in meters.
// channelSideSlope         - Side slope of channel element, unitless.
// channelBedConductivity   - Conductivity of channel bed in meters per second.
// channelBedThickness      - Thickness of channel bed in meters.
// channelSurfacewaterDepth - Surfacewater depth of channel element in meters.
// FIXME figure out how to calculate dtNew
bool groundwaterMeshChannelFlowRate(double* flowRate, double edgeLength, double meshZSurface, double meshZBedrock, double meshSurfacewaterDepth,
                                    double meshGroundwaterHead, double channelZBank, double channelZBed, double channelBaseWidth, double channelSideSlope,
                                    double channelBedConductivity, double channelBedThickness, double channelSurfacewaterDepth);

#endif // __GROUNDWATER_H__
