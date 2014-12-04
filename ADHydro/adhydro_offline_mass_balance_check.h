#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>


void initialize(char* argv[], int* ncid_stt, int* ncid_geo, int* ncid_par, size_t* nInstances, size_t* nMeshEle, size_t* nChannelEle, int* currentTimeID, int* meshSurfacewaterDepthID,int* meshGroundwaterHeadID, 
                 int* meshPrecipitaitonCumulativeID, int* meshEvaporationCumulativeID, int* canIceID, int* canLiqID, int* snEqvID, int* EleNeighgSfcFlowID, int* EleNeighgGrdFlowID, int* channelSurfacewaterDepthID, 
                 int* channelprecipitationCumulativeID, int* channelEvaporationCumulativeID, int* ChanNeighgSfcFlowID, size_t* NmeshMeshNeighbors,size_t* NchannelChannelNeighbors, int* EleNeighID, int* meshElementZBedrockID,
                 int* meshElementAreaID, int* ChanNeighID, int* channelElementLengthID, int* meshPorosityID, int* channelBaseWidthID, int* channelElementSlopeID);
// Proporse:
// To open netcdf files and to extract Id's of variables as well as a few time independent variables.
//
// Input:
// argv[]                          -> command line argument
//
// Output:
// ncid_stt                          scalar -> state.nc file ID	
// ncid_geo                          scalar -> geometry.nc file ID
// ncid_par                          scalar -> parameter.nc file ID
// nInstances                        scalar -> # of instances in the *.nc files
// nMeshEle                          scalar -> # of mesh elements in the *.nc files
// nChannelEle                       scalar -> # of channel elements in the *.nc files
// currentTimeID                     scalar -> current Time varaible ID
// meshSurfacewaterDepthID           scalar -> mesh Surface water Depth variable ID                    from state.nc
// meshGroundwaterHeadID             scalar -> mesh Ground water Head variable ID                      from state.nc
// meshPrecipitaitonCumulativeID     scalar -> mesh Precipitation Cumulative variable ID               from state.nc
// meshEvaporationCumulativeID       scalar -> mesh Evaporation Cumulative varaible ID                 from state.nc
// canIceID                          scalar -> mesh canopy Ice variable ID                             from state.nc
// canLiqID                          scalar -> mesh canopy Liquid variable ID                          from state.nc
// snEqvID                           scalar -> mesh snow water Equivalent varaible ID                  from state.nc
// EleNeighgSfcFlowID                scalar -> mesh Element Neighbor Surface Flow variable ID          from state.nc
// EleNeighgGrdFlowID                scalar -> mesh Element Neighbor Ground Flow variable ID           from state.nc
// channelSurfacewaterDepthID        scalar -> channel Surface water Depth varaible ID                 from state.nc
// channelPrecipitationCumulativeID  scalar -> channel Evaporation Cumulative varaible ID              from state.nc
// channelEvaporationCumulativeID    scalar -> channel Evaporation Cumulative varaible ID              from state.nc
// ChanNeighgSfcFlowID               scalar -> Channel Neighbor Surface Flow varaible ID               from state.nc
// NmeshMeshNeighbors                scalar -> # of mesh Mesh Neighbors in the state.nc files 
// NchannelChannelNeighbors          scalar -> # of channel Channel Neighbors in the state.nc files
// EleNeighID                        scalar -> mesh element neighbor varaible ID                       from geometry.nc
// meshElementZBedrockID             scalar -> mesh Element Z Bedrock varaible ID                      from geometry.nc
// meshElementAreaID                 scalar -> mesh Element Area varaible ID                           from geometry.nc
// ChanNeighID                       scalar -> Channel Neighbor varaible ID                            from geometry.nc
// channelElementLengthID            scalar -> channel Element Length varaible ID                      from geometry.nc
// meshPorosityID                    scalar -> mesh Porosity varaible ID                               from parameter.nc
// channelBaseWidthID                scalar -> channel Element base width variable ID                  from parameter.nc
// channelElementSlopeID             scalar -> channel Element Slope variable ID                       from parameter.nc

void getElementAndChannelGeometry(int ncid_stt, int ncid_geo, size_t stateInstance, size_t nMeshEle, size_t nChannelEle,  size_t nMeshNgb, size_t nChannelNgb, int meshElementZBedrockID, int meshElementAreaID, int EleNeighID, 
                                  int channelElementLengthID, int ChanNeighID, double meshElementZBedrock[nMeshEle], double meshElementArea[nMeshEle], double channelElementLength[nChannelEle], 
                                  int meshMeshNeighbors[nMeshEle*nMeshNgb], int channelChannelNeighbors[nChannelEle*nChannelNgb]);
// Proporse:
// Get the Geometry values needed for both mesh and channel calcultion. Accounts for possible change of geometry with time.
//
// Input:
// ncid_stt                                   scalar -> id of the state.nc file	
// ncid_geo                                   scalar -> id of the geometry.nc file
// stateInstance                              scalar -> current state instance
// nMeshEle                                   scalar -> # of mesh elements in the *.nc files
// nChannelEle                                scalar -> # of channel elements in the *.nc files
// nMeshNgb                                   scalar -> # of mesh mesh neighbors in the *.nc files
// nChannelNgb                                scalar -> # of channel channel neighbors in the *.nc files
// meshElementZBedrockID                      scalar -> mesh Element Z Bedrock varaible ID                      from geometry.nc
// meshElementAreaID                          scalar -> mesh Element Area varaible ID                           from geometry.nc
// EleNeighID                                 scalar -> mesh mesh neighbors variable ID                         from geometry.nc
// channelElementLengthID                     scalar -> channel Element Length variable ID                      from geometry.nc 
// ChanNeighID                                scalar -> channel channel neighbors variable ID                   from geometry.nc 
// meshElementZBedrock     Asigned Array [nMeshEle]             -> value of meshElementZBedrock for all mesh elements;     -1 if no value.
// meshElementArea         Asigned Array [nMeshEle]             -> value of meshElementArea for all mesh elements;         -1 if no value.
// channelElementLength    Asigned Array [nChannelEle]          -> value of channelElementLength for all channel elements; -1 if no value.
// meshMeshNeighbors       Asigned Array [nMeshEle*nMeshNgb]    -> mesh mesh neighbors boundary index
// channelChannelNeighbors Asigned Array [nChannelEle*nMeshNgb] -> channel channel neighbors boundary index
//
// Output:                       
// meshElementZBedrock     Asigned Array [nMeshEle] -> value of meshElementZBedrock for all mesh elements
// meshElementArea         Asigned Array [nMeshEle] -> value of meshElementArea for all mesh elements
// channelElementLength    Asigned Array [nChannelEle] -> value of channelElementLength for all channel elements
// meshMeshNeighbors       Asigned Array [nMeshEle*nMeshNgb]    -> mesh mesh neighbors boundary index
// channelChannelNeighbors Asigned Array [nChannelEle*nMeshNgb] -> channel channel neighbors boundary index

void getElementAndChannelParameter( int ncid_stt, int ncid_par, size_t stateInstance, size_t nMeshEle, size_t nChannelEle, int meshPorosityID, int channelElementSlopeID, int channelBaseWidthID, 
                                      double meshPorosity[nMeshEle], double channelElementSlope[nChannelEle], double channelBaseWidth[nChannelEle]) ;
// Proporse:
// Get the parameter values needed for both mesh and channel water content calcultion. Accounts for possible change of parameter instance.
//
// Input:
// ncid_stt                                   scalar -> id of the state.nc file	
// ncid_par                                   scalar -> id of the parameter.nc file
// stateInstance                              scalar -> current state instance
// nMeshEle                                   scalar -> # of mesh elements in the *.nc files
// nChannelEle                                scalar -> # of channel elements in the *.nc files
// meshPorosityID                             scalar -> mesh Porosity varaible ID                               from parameter.nc
// channelElementSlopeID                      scalar -> channel Element Slope variable ID                       from parameter.nc
// channelBaseWidthID                         scalar -> channel Element base width variable ID                       from parameter.nc
// meshPorosity            Asigned Array [nMeshEle]    -> value of meshPorosity for all mesh elements; -1 if no value.
// channelElementSlope     Asigned Array [nChannelEle] -> value of channelElementSlope for all channel elements; -1 if no value.
// channelBaseWidth        Asigned Array [nChannelEle] -> value of channelElementSlope for all channel elements; -1 if no value.
//
// Output:
// meshPorosity            Asigned Array [nMeshEle] -> value of meshPorosity for all mesh elements
// channelElementSlope     Asigned Array [nChannelEle] -> value of channelElementSlope for all channel elements
// channelBaseWidth        Asigned Array [nChannelEle] -> value of channelBaseWidth for all channel elements

void getStateVars(int ncid_stt, size_t stateInstance, size_t nMeshEle, size_t nChannelEle, size_t nMeshNgb, size_t nChannelNgb, int meshSurfacewaterDepthID, int meshGroundwaterHeadID, int canIceID, int canLiqID, int snEqvID, 
                  int meshCummEvapoID, int meshCummPrcpID, int EleNeighSfcFlowID, int EleNeighGrdFlowID, int channelSurfacewaterDepthID, int channelCummEvapoID, int channelCummPrcpID, int ChanNeighSfcFlowID,
                  double meshSurfacewaterDepth[nMeshEle], double meshGroundwaterHead[nMeshEle], float meshCanIce[nMeshEle], float meshCanLiq[nMeshEle], float meshSnEqv[nMeshEle], double meshCummEvapo[nMeshEle], 
                  double meshCummPrcp[nMeshEle], double EleNeighSfcFlow[nMeshEle*nMeshNgb], double EleNeighGrdFlow[nMeshEle*nMeshNgb], double channelSurfacewaterDepth[nChannelEle], double channelCummEvapo[nChannelEle], 
                  double channelCummPrcp[nChannelEle], double ChanNeighSfcFlow[nChannelEle*nChannelNgb] );
// Proporse:
// Get the state values for all elements and all channels for each time step.
//
// Inputs:
//
// ncid_stt                             scalar -> id of the state.nc file	
// stateInstance                        scalar -> current state instance
// nMeshEle                             scalar -> # of mesh elements in the *.nc files
// nChannelEle                          scalar -> # of channel elements in the *.nc files
// nMeshNgb                             scalar -> # of mesh mesh neighbors in the *.nc files
// nChannelNgb                          scalar -> # of channel channel neighbors in the *.nc files
// meshSurfacewaterDepthID              scalar -> mesh Surface water Depth variable ID                    
// meshGroundwaterHeadID                scalar -> mesh Ground water Head variable ID                                 
// canIceID                             scalar -> mesh canopy Ice variable ID                             
// canLiqID                             scalar -> mesh canopy Liquid variable ID                          
// snEqvID                              scalar -> mesh snow water Equivalent varaible ID                  
// meshCummEvapoID          scalar -> mesh Evaporation Cumulative varaible ID      
// meshCummPrcpID        scalar -> mesh Precipitation Cumulative variable ID               
// EleNeighgSfcFlowID                   scalar -> mesh Element Neighbor Surface Flow variable ID          
// EleNeighgGrdFlowID                   scalar -> mesh Element Neighbor Ground Flow variable ID 
// channelSurfacewaterDepthID           scalar -> channel Surface water Depth varaible ID                 
// channelCummEvapoID                   scalar -> channel Evaporation Cumulative varaible ID              
// channelCummPrcpID                    scalar -> channel Evaporation Cumulative varaible ID              
// ChanNeighgSfcFlowID                  scalar -> Channel Neighbor Surface Flow varaible ID          
// meshSurfacewaterDepth        Asigned Array [nMeshEle]             -> meshSurfacewaterDepth for all mesh elements
// meshGroundwaterHead          Asigned Array [nMeshEle]             -> meshGroundwaterHead for all mesh elements
// meshCanIce                   Asigned Array [nMeshEle              ->   meshCanIce for all mesh elements
// meshCanLiq                   Asigned Array [nMeshEle]             ->   meshCanIce for all mesh elements
// meshSnEqv                    Asigned Array [nMeshEle]             ->   mesh Snow water Equivalent for all mesh elements
// meshCummEvapo                Asigned Array [nMeshEle]             ->   mesh Cumulative Evaporation for all mesh elements
// meshCummPrcp                 Asigned Array [nMeshEle]             ->   mesh Cumulative Precipitation for all mesh elements
// EleNeighSfcFlow              Asigned Array [nMeshEle*nMeshNgb]    ->   Element Neighbor Surface Flow for all elements and theirs neighbors
// EleNeighGrdFlow              Asigned Array [nMeshEle*nMeshNgb]    ->   Element Neighbor Ground water Flow for all elements and theirs neighbors
// channelSurfacewaterDepth     Asigned Array [nChannelEle]          ->   channelSurfacewaterDepth for all channel elements
// channelCummEvapo             Asigned Array [nChannelEle]          ->   channel Cumulative Evaporation for all channel elements
// channelCummPrcp              Asigned Array [nChannelEle]          ->   channel Cumulative Precipitation for all channel elements
// ChanNeighSfcFlow             Asigned Array [nChannelEle*nChannelNgb]    ->   Channel Neighbor Surface Flow for all channels and theirs neighbors
//
// Outputs:
// meshSurfacewaterDepth        Asigned Array [nMeshEle]             -> value of meshSurfacewaterDepth for all mesh elements
// meshGroundwaterHead          Asigned Array [nMeshEle]             -> value of meshGroundwaterHead for all mesh elements
// meshCanIce                   Asigned Array [nMeshEle              -> value of meshCanIce for all mesh elements
// meshCanLiq                   Asigned Array [nMeshEle]             -> value of meshCanIce for all mesh elements
// meshSnEqv                    Asigned Array [nMeshEle]             -> value of mesh Snow water Equivalent for all mesh elements
// meshCummEvapo                Asigned Array [nMeshEle]             -> value of mesh Cumulative Evaporation for all mesh elements
// meshCummPrcp                 Asigned Array [nMeshEle]             -> value of mesh Cumulative Precipitation for all mesh elements
// EleNeighSfcFlow              Asigned Array [nMeshEle*nMeshNgb]    -> value of Element Neighbor Surface Flow for all elements and theirs neighbors
// EleNeighGrdFlow              Asigned Array [nMeshEle*nMeshNgb]    -> value of Element Neighbor Ground water Flow for all elements and theirs neighbors
// channelSurfacewaterDepth     Asigned Array [nChannelEle]          -> value of channelSurfacewaterDepth for all channel elements
// channelCummEvapo             Asigned Array [nChannelEle]          -> value of channel Cumulative Evaporation for all channel elements
// channelCummPrcp              Asigned Array [nChannelEle]          -> value of channel Cumulative Precipitation for all channel elements
// ChanNeighSfcFlow             Asigned Array [nChannelEle*nChannelNgb]    -> value of Channel Neighbor Surface Flow for all channels and theirs neighbors


double meshEleWaterContent(double meshElementZBedrock, double meshElementArea, double meshPorosity, double meshSurfacewaterDepth , double meshGroundwaterHead, float meshCanIce, float meshCanLiq, float meshSnEqv);
// Proporse: Calculate the amount of water in cubic meters of a sigle mesh element for a given time step.
//           Positive means current ammount of water in the element. Must be non negative.
// Input: 
// ncid_stt                    -> id for the opened file state.nc
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

double channelEleWaterContent (double channelElementLength, double channelElementSlope, double channelBaseWidth, double channelSurfacewaterDepth);
// Proporse: Calculate the water balence in cubic meters of water of a channel element for a given time step.
//           Positive means current ammount of water in the channel. Must be non negative.
// Input: 
// ncid_stt                    -> id for the opened file state.nc
// indices[2]                  -> Allocated  size 3 array. Already filled.
//                                indices [0] -> refers to current instance index from main
//                                indices [1] -> refers to current element index  from main
// channelElementLength        -> channel Element Length in meters 
// channelElementSlope         -> Channel Element Slope for the same channel element index
// channelSurfacewaterDepthID  -> id for the varaible channelSurfacewaterDepth from state.nc
// CummEvapoID                 -> id for the varaible evaporationCumulative from state.nc
// CummPrcpID                  -> id for the varaible precipitationCumulative from state.nc
//
// Outputs:
// The ammount of water in the channel element for this time step in cubic meters.
// The result should be greater than or equal to zero.
// cumEvapo and cumPrcp -> Evaporation and precipitation for the given element and time step. Value in cubic meters or water.

double ElemBoundaryflow( int currentEle, size_t nMeshEle, int NmeshMeshNeighbors, double meshElementArea, int meshMeshNeighbors [nMeshEle*NmeshMeshNeighbors], double meshPrecipitationCumulative, 
                         double meshEvaporationCumulative, double EleNeighSfcFlow[nMeshEle*NmeshMeshNeighbors], double EleNeighGrdFlow[nMeshEle*NmeshMeshNeighbors]);
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

double ChanBoundaryflow(int currentElem, size_t nChannelEle,int NchannelChannelNeighbors,int channelChannelNeighbors[nChannelEle*NchannelChannelNeighbors], 
                         double ChanNeighSfcFlow[nChannelEle*NchannelChannelNeighbors], double cumEvapo, double cumPrcp);
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
// cumEvapo                    -> evaporationCumulative from state.nc
// cumPrcp                     -> precipitationCumulative from state.nc
//
// Output:
// Scalar. Total flow in cubic meters of water from the channel element.
// Given by: (channelSurfacewaterDepth - cummEvapo + cumpPrcp)
// Outflow is outflow, therefore considered positive
// CumEvapo < 0 means water removed
