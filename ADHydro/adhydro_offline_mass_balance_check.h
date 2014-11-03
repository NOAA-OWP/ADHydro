#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>


void initialize(char* argv[], int* ncid_stt, int* ncid_geo, int* ncid_par, size_t* nInstances, size_t* nMeshEle, size_t* nChannelEle, int* currentTimeID, int* meshSurfacewaterDepthID,int* meshGroundwaterHeadID, 
                 int* meshPrecipitaitonCumulativeID, int* meshEvaporationCumulativeID, int* canIceID, int* canLiqID, int* snEqvID, int* EleNeighgSfcFlowID, int* EleNeighgGrdFlowID, int* channelSurfacewaterDepthID,
                 int* channelEvaporationCumulativeID, int* ChanNeighgSfcFlowID, size_t* NmeshMeshNeighbors,size_t* NchannelChannelNeighbors, int* EleNeighID, int* meshElementZBedrockID, int* meshElementAreaID,
                 int* ChanNeighID, int* channelElementLengthID, int* meshPorosityID, int* channelElementSlopeID);
// Proporse:
// To open netcdf files and to extract Id's of variables as well as a few time independent variables.
//
// Input:
// argv[]                          -> command line argument
//
// Output:
// ncid_stt                        scalar -> state.nc file ID	
// ncid_geo                        scalar -> geometry.nc file ID
// ncid_par                        scalar -> parameter.nc file ID
// nInstances                      scalar -> # of instances in the *.nc files
// nMeshEle                        scalar -> # of mesh elements in the *.nc files
// nChannelEle                     scalar -> # of channel elements in the *.nc files
// currentTimeID                   scalar -> current Time varaible ID
// meshSurfacewaterDepthID         scalar -> mesh Surface water Depth variable ID                    from state.nc
// meshGroundwaterHeadID           scalar -> mesh Ground water Head variable ID                      from state.nc
// meshPrecipitaitonCumulativeID   scalar -> mesh Precipitation Cumulative variable ID               from state.nc
// meshEvaporationCumulativeID     scalar -> mesh Evaporation Cumulative varaible ID                 from state.nc
// canIceID                        scalar -> mesh canopy Ice variable ID                             from state.nc
// canLiqID                        scalar -> mesh canopy Liquid variable ID                          from state.nc
// snEqvID                         scalar -> mesh snow water Equivalent varaible ID                  from state.nc
// EleNeighgSfcFlowID              scalar -> mesh Element Neighbor Surface Flow variable ID          from state.nc
// EleNeighgGrdFlowID              scalar -> mesh Element Neighbor Ground Flow variable ID           from state.nc
// channelSurfacewaterDepthID      scalar -> channel Surface water Depth varaible ID                 from state.nc
// channelEvaporationCumulativeID  scalar -> channel Evaporation Cumulative varaible ID              from state.nc
// ChanNeighgSfcFlowID             scalar -> Channel Neighbor Surface Flow varaible ID               from state.nc
// NmeshMeshNeighbors              scalar -> # of mesh Mesh Neighbors in the state.nc files 
// NchannelChannelNeighbors        scalar -> # of channel Channel Neighbors in the state.nc files
// EleNeighID                      scalar -> mesh element neighbor varaible ID                       from geometry.nc
// meshElementZBedrockID           scalar -> mesh Element Z Bedrock varaible ID                      from geometry.nc
// meshElementAreaID               scalar -> mesh Element Area varaible ID                           from geometry.nc
// ChanNeighID                     scalar -> Channel Neighbor varaible ID                            from geometry.nc
// channelElementLengthID          scalar -> channel Element Length varaible ID                      from geometry.nc
// meshPorosityID                  scalar -> mesh Porosity varaible ID                               from parameter.nc
// channelElementSlopeID           scalar -> channel Element Slope variable ID                       from parameter.nc

void getElementAndChannelGeometry(int ncid_stt, int ncid_geo, size_t stateInstance, size_t nMeshEle, size_t nChannelEle, int meshElementZBedrockID, int meshElementAreaID, int channelElementLengthID, double* meshElementZBedrock,
                                  double* meshElementArea, double* channelElementLength );
// Proporse:
// Get the Geometry values needed for both mesh and channel calcultion. Accounts for possible change of geometry with time.
//
// Input:
// ncid_stt                                   scalar -> id of the state.nc file	
// ncid_geo                                   scalar -> id of the geometry.nc file
// stateInstance                              scalar -> current state instance
// nMeshEle                                   scalar -> # of mesh elements in the *.nc files
// nChannelEle                                scalar -> # of channel elements in the *.nc files
// meshElementZBedrockID                      scalar -> mesh Element Z Bedrock varaible ID                      from geometry.nc
// meshElementAreaID                          scalar -> mesh Element Area varaible ID                           from geometry.nc
// channelElementLengthID                     scalar -> channel Element Length varaible ID                      from geometry.nc 
// meshElementZBedrock     Asigned Vector [nMeshEle] -> value of meshElementZBedrock for all mesh elements; -1 if no value.
// meshElementArea         Asigned Vector [nMeshEle] -> value of meshElementArea for all mesh elements; -1 if no value.
// channelElementLength    Asigned Vector [nChannelEle] -> value of channelElementLength for all channel elements; -1 if no value.
//
// Output:                       
// meshElementZBedrock     Asigned Vector [nMeshEle] -> value of meshElementZBedrock for all mesh elements
// meshElementArea         Asigned Vector [nMeshEle] -> value of meshElementArea for all mesh elements
// channelElementLength    Asigned Vector [nChannelEle] -> value of channelElementLength for all channel elements

void getElementAndChannelParameter( int ncid_stt, int ncid_par, size_t stateInstance, size_t nMeshEle, size_t nChannelEle, int meshPorosityID, int channelElementSlopeID, double* meshPorosity, double* channelElementSlope );
// Proporse:
// Get the parameter values needed for both mesh and channel water content calcultion. Accounts for possible change of parameter with time.
//
// Input:
// ncid_stt                                   scalar -> id of the state.nc file	
// ncid_par                                   scalar -> id of the parameter.nc file
// stateInstance                              scalar -> current state instance
// nMeshEle                                   scalar -> # of mesh elements in the *.nc files
// nChannelEle                                scalar -> # of channel elements in the *.nc files
// meshPorosityID                             scalar -> mesh Porosity varaible ID                               from parameter.nc
// channelElementSlopeID                      scalar -> channel Element Slope variable ID                       from parameter.nc
// meshPorosity            Asigned Vector [nMeshEle]    -> value of meshPorosity for all mesh elements; -1 if no value.
// channelElementSlope     Asigned Vector [nChannelEle] -> value of channelElementSlope for all channel elements; -1 if no value.
//
// Output:
// meshPorosity            Asigned Vector [nMeshEle] -> value of meshPorosity for all mesh elements
// channelElementSlope     Asigned Vector [nChannelEle] -> value of channelElementSlope for all channel elements

double meshEleWaterContent(int ncid_stt, int ncid_par, int ncid_geo, size_t indices[2], double meshElementZBedrock, double meshElementArea, double meshPorosity,
                           int meshSurfacewaterDepthID, int meshGroundwaterHeadID, int canIceID, int canLiqID, int snEqvID);
// Proporse: Calculate the amount of water in cubic meters of a sigle mesh element for a given time step.
//           Positive means current ammount of water in the element. Must be non negative.
// Input: 
// ncid_stt                    -> id for the opened file state.nc
// ncid_par                    -> id for the opened file parameter.nc
// ncid_geo                    -> id for the opened file geometry.nc
// indices[2]                  -> Allocated  size 2 array. Already filled.
//                                indices [0] -> refers to current instance index from main
//                                indices [1] -> refers to current element index  from main
// meshElementZBedrock         -> variable meshElementZBedrock for the same element index 
// meshElementArea             -> variable meshElementArea for the same element index
// meshPorosity                -> varaible meshPorosity for the same element index
// meshSurfacewaterDepthID     -> id for the varaible meshSurfacewaterDepth from state.nc
// meshGroundwaterHeadID       -> id for the varaible meshGroundwaterHead from state.nc
// CummEvapoID                 -> id for the varaible meshPrecipitationCumulative from state.nc
// CummEvapoID                 -> id for the varaible mesh evaporationCumulative from state.nc
// canIceID                    -> id for the varaible meshCanopyIce from state.nc
// canLiqID                    -> id for the varaible meshCanopyLiq from state.nc
// snEqvID                     -> id for the varaible meshSnowWaterEquivalent from state.nc
//
// Outputs:
// The ammount of water in the mesh element for this time step in cubic meters.
// Given by: (meshGroundwaterHead + meshSurfacewaterDepth + precip + snEqv + canLiq + canIce )
// The result should be greater than or equal to zero.

double channelEleWaterContent (int ncid_stt, int ncid_geo,  int ncid_par, size_t indices[2], double channelElementLength, double channelElementSlope, int channelSurfacewaterDepthID, int cummEvapoID, double* cumEvapo);
// Proporse: Calculate the water balence in cubic meters of water of a channel element for a given time step.
//           Positive means current ammount of water in the channel. Must be non negative.
// Input: 
// ncid_stt                    -> id for the opened file state.nc
// ncid_geo                    -> id for the opened file geometry.nc
// indices[2]                  -> Allocated  size 3 array. Already filled.
//                                indices [0] -> refers to current instance index from main
//                                indices [1] -> refers to current element index  from main
// channelElementLength        -> channel Element Length in meters 
// channelElementSlope         -> Channel Element Slope for the same channel element index
// channelSurfacewaterDepthID  -> id for the varaible channelSurfacewaterDepth from state.nc
// CummEvapoID                 -> id for the varaible evaporationCumulative from state.nc
//
// Outputs:
// The ammount of water in the channel element for this time step in cubic meters.
// The result should be greater than or equal to zero.

double ElemBoundaryflow(int ncid_stt, int ncid_geo, size_t indices[2], double meshElementArea, int meshPrecipitaitonCumulativeID, int CummEvapoID, size_t nMeshEle, int NmeshMeshNeighbors,  int EleNeighID,
                        int EleNeighgSfcFlowID, int EleNeighgGrdFlowID);
// Proporse: Check if the mesh element has a flow boundary and returns how much water has entered or left the element. 
//           Positive values mean Inflow, Negative values means Outfloe. 0 is returned if element is not a flow boundary.
//
// Input: 
// ncid_stt              -> id for the opened file state.nc
// ncid_geo              -> id for the opened file geometry.nc
// indices [3]           -> Allocated  size 3 array. First 2 elements of array have to be already filled.
//                          indices [0] -> refers to current instance 
//                          indices [1] -> refers to current element 
//                          indice  [2] -> refers to current neighbor index filled in ElemBoundaryflow
// nMeshEle              -> # of mesh elements
// NmeshMeshNeighbors    -> # of mesh mesh neighbors
// EleNeighID            -> ncid for the variable meshMeshNeighbors from geometry.nc
// EleNeighgSfcFlowID    -> ncid for the variable meshNeighborsSurfacewaterCumulativeFlow from state.nc
// EleNeighgGrdFlowID    -> ncid for the variable meshNeighborsGroundwaterCumulativeFlow from state.nc
//
// Output:
// Scalar. Total flow in cubic meters of water from the mesh element including evaporation and precipitation.
//         Given by  meshMeshNeighborsoutFlow - CumEvapo + meshMeshNeighborsinFlow - PrcpCum
// PrcpCum is inflow, therefore considered negative
// Outflow is outflow, therefore considered positive
// CumEvapo < 0 means water removed
// PrcpCum > 0 means water added

double ChanBoundaryflow(int ncid_stt, int ncid_geo, size_t indices[2], size_t nChannelEle, int NchannelChannelNeighbors, int ChanvarID, int ChanNeighgSfcFlowID, double cumEvapo);
// Proporse: Check if the channel element has a flow boundary and returns how much water has entered or left the element. 
//           Positive values mean Inflow, Negative values means Outfloe. 0 is returned if element is not a flow boundary.
//
// Input: 
// ncid_stt                    -> id for the opened file state.nc
// ncid_geo                    -> id for the opened file geometry.nc
// indices [2]                 -> Allocated  size 3 array. First 2 elements of array have to be already filled.
//                                indices [0] -> refers to current instance index 
//                                indices [1] -> refers to current element index  
// nChannelEle                 -> # of channel elements
// NchannelChannelNeighbors    -> # of channel channel neighbors
// ChanvarID                   -> ncid for the variable meshMeshNeighbors from geometry.nc
// ChanNeighgSfcFlowID         -> ncid for the variable meshNeighborsSurfacewaterCumulativeFlow from state.nc
//
// Output:
// Scalar. Total flow in cubic meters of water from the channel element.
// Given by: (channelSurfacewaterDepth - cummEvapo )
// Outflow is outflow, therefore considered positive
// CumEvapo < 0 means water removed
