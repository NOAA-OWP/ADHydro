#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

double ElemBoundaryflow(int ncid_stt, int ncid_geo, size_t indices[3], size_t nMeshEle, int NmeshMeshNeighbors,  int EleNeighID, int EleNeighgSfcFlowID, int EleNeighgGrdFlowID);
// Proporse: Check if the mesh element has a flow boundary and returns how much water has entered or left the element. 
//           Positive values mean Inflow, Negative values means Outfloe. 0 is returned if element is not a flow boundary.
//
// Input: 
// ncid_stt              -> id for the opened file state.nc
// ncid_geo              -> id for the opened file geometry.nc
// indices [3]           -> Allocated  size 3 array. Already filled.
//                          indices [0] -> refers to current instance index from main
//                          indices [1] -> refers to current element index  from main
//                          indice  [2] -> refers to current neighbor index from ElemBoundaryflow
// nMeshEle              -> # of mesh elements
// NmeshMeshNeighbors    -> # of mesh mesh neighbors
// EleNeighID            -> ncid for the variable meshMeshNeighbors from geometry.nc
// EleNeighgSfcFlowID    -> ncid for the variable meshNeighborsSurfacewaterCumulativeFlow from state.nc
// EleNeighgGrdFlowID    -> ncid for the variable meshNeighborsGroundwaterCumulativeFlow from state.nc
//
// Output:
// Scalar. Total flow in meter of water from the mesh element.

double ChanBoundaryflow(int ncid_stt, int ncid_geo, size_t indices[3], size_t nChannelEle, int NchannelChannelNeighbors, int ChanvarID,  int ChanNeighgSfcFlowID);
// Proporse: Check if the channel element has a flow boundary and returns how much water has entered or left the element. 
//           Positive values mean Inflow, Negative values means Outfloe. 0 is returned if element is not a flow boundary.
//
// Input: 
// ncid_stt                    -> id for the opened file state.nc
// ncid_geo                    -> id for the opened file geometry.nc
// indices [3]                 -> Allocated  size 3 array. Already filled.
//                                indices [0] -> refers to current instance index from main
//                                indices [1] -> refers to current element index  from main
//                                indice  [2] -> refers to current neighbor index from ChanBoundaryflow
// nChannelEle                 -> # of channel elements
// NchannelChannelNeighbors    -> # of channel channel neighbors
// ChanvarID                   -> ncid for the variable meshMeshNeighbors from geometry.nc
// ChanNeighgSfcFlowID         -> ncid for the variable meshNeighborsSurfacewaterCumulativeFlow from state.nc
//
// Output:
// Scalar. Total flow in meter of water from the channel element.

double meshEleWaterContent(int ncid_stt, int ncid_par, int ncid_geo, int meshElementZBedrockID, int meshElementAreaID, int meshPorosityID,
                            int meshSurfacewaterDepthID, int meshGroundwaterHeadID, int CummEvapoID, int canIceID, int canLiqID, int snEqvID, 
                            size_t indices[3], double precip);
// Proporse: Calculate the water balence in meter of water of a mesh element for a given time step.
//           Positive means current ammount of water in the element. Must be non negative.
// Input: 
// ncid_stt                    -> id for the opened file state.nc
// ncid_par                    -> id for the opened file parameter.nc
// ncid_geo                    -> id for the opened file geometry.nc
// meshElementZBedrockID       -> id for the variable meshElementZBedrock from geometry.nc
// meshElementAreaID           -> id for the varaible meshElementAreaID from geometry.nc
// meshPorosityID              -> id for the varaible meshPorosity from parameter.nc
// meshSurfacewaterDepthID     -> id for the varaible meshSurfacewaterDepth from state.nc
// meshGroundwaterHeadID       -> id for the varaible meshGroundwaterHead from state.nc
// CummEvapoID                 -> id for the varaible evaporationCumulative from state.nc
// canIceID                    -> id for the varaible canIce from state.nc
// canLiqID                    -> id for the varaible canLiq from state.nc
// snEqvID                     -> id for the varaible snEqv from state.nc
// indices[3]                  -> Allocated  size 3 array. Already filled.
//                                indices [0] -> refers to current instance index from main
//                                indices [1] -> refers to current element index  from main
//                                indice  [2] -> not used
// precip                      -> value of precipitation in meters of water for the given time step.
//
// Outputs:
// The ammount of water in the mesh element for this time step.
// Given by: (meshGroundwaterHead + meshSurfacewaterDepth + precip + snEqv - canLiq - canIce - CummEvapo)
// All values account for element porosity and/or element area

double channelEleWaterContent (int ncid_stt, int ncid_geo, int channelElementLengthID, int channelElementSlopeID, int channelSurfacewaterDepthID, int cummEvapoID, size_t* indices, double precip);
// Proporse: Calculate the water balence in meter of water of a channel element for a given time step.
//           Positive means current ammount of water in the channel. Must be non negative.
// Input: 
// ncid_stt                    -> id for the opened file state.nc
// ncid_geo                    -> id for the opened file geometry.nc
// channelElementLengthID      -> id for the variable channelElementLength from geometry.nc
// channelElementSlopeID       -> id for the varaible channelElementSlope  from geometry.nc
// channelSurfacewaterDepthID  -> id for the varaible channelSurfacewaterDepth from state.nc
// CummEvapoID                 -> id for the varaible evaporationCumulative from state.nc
// indices[3]                  -> Allocated  size 3 array. Already filled.
//                                indices [0] -> refers to current instance index from main
//                                indices [1] -> refers to current element index  from main
//                                indice  [2] -> not used
// precip                      -> value of precipitation in meters of water for the given time step.
//
// Outputs:
// The ammount of water in the channel element for this time step.
// Given by: (channelSurfacewaterDepth + *precip - cummEvapo )
// All values account for element area


