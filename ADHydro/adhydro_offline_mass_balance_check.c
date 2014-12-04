#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>
#include <string.h>
#include <assert.h>
#include "adhydro_offline_mass_balance_check.h"
// From Adhydro - all.h:
#define  NOFLOW  (-1) // This must be -1 because Triangle and TauDEM both use -1 to indicate no neighbor.
#define  INFLOW  (-2) // Others must be non-positive because positive numbers are used for neighbor array indices.
#define  OUTFLOW (-3)


int main(int argc, char*argv[])
{
// Propose:
// Offline check of the water balance of adhydro simulation.
// Calculates the amount of water in all mesh and channel elements for each time step.
// Prints off values on a files:
// adhydroOffilineMassBalanceCheck.txt
// It contains:
// 1 current time
// 2 total water value, total flows (+ is out) value, originally had value
// 1 and 2 repeats for each time step.
// File is being saved in the same folder as the output .nc files from adhydro.
// #####################################################################################################################
// #########    IMPORTANT: To choose the folder where the output files should be saved, go to lines 161      ###########      
// #####################################################################################################################

 char*  filePathstring;				                // string to save the file names and directories of the output
 int    err_status;				                // error variable
 int    ncid_stt,ncid_geo, ncid_par;                            // id of the state.nc, geometry.nc, and parameter.nc files	
 size_t nInstances,nMeshEle, nChannelEle;                  	// # of: instances, mesh elements, and channel elements in the *.nc files		
 size_t  NmeshMeshNeighbors;                                    // # of meshMesh neighbors
 size_t  NchannelChannelNeighbors;                              // # of channelChannel neighbors
 // from state.nc
 int currentTimeID;                                            // current time variable ID
 int meshGroundwaterHeadID;                                    // meshGroundwaterHead variable ID
 int meshSurfacewaterDepthID;                                  // meshSurfacewaterDepth varaible ID
 int meshPrecipitationCumulativeID;                            // meshPrecipitationCumulative varaible ID
 int meshEvaporationCumulativeID;                              // meshEvaporationCumulative varaible ID
 int canIceID;                                                 // canopy ice varaible ID
 int canLiqID;                                                 // canopyliquid varaible ID
 int snEqvID;                                                  // snow water equivalent varaible ID
 int channelSurfacewaterDepthID;                               // channelSurface water Depth varaible ID
 int channelEvaporationCumulativeID;                           // channelEvaporationCumulative ID
 int channelPrecipitationCumulativeID;                         // PrecipitationCumulative varaible ID
 double currentTime;					       // current time of simulation in the state.nc file
 double* meshGroundwaterHead;                                  // meshGroundwaterHead array[nMeshEle]
 double* meshSurfacewaterDepth;                                // meshSurfacewaterDepth array[nMeshEle]
 double* meshPrecipitationCumulative;                          // meshPrecipitationCumulative array[nMeshEle]
 double* meshEvaporationCumulative;                            // meshEvaporationCumulative array[nMeshEle]
 float*  meshCanIce;                                           // canopyliquid array[nMeshEle]
 float*  meshCanLiq;                                           // canopyice array[nMeshEle]
 float*  meshSnEqv;                                            // snow water equivalent array[nChannelEle]
 double* channelSurfacewaterDepth;                             // channelSurface array[nChannelEle]
 double* channelEvaporationCumulative;                         // channelEvaporationCumulative array[nChannelEle]
 double* channelPrecipitationCumulative;                       // channelPrecipitationCumulative array[nChannelEle*NchannelChannelNeighbors]
 
 // Flow varaibles
 int     EleNeighSfcFlowID;                                     // mesh Element NeighborsSurfacewaterCumulativeFlow variable ID 
 int     EleNeighGrdFlowID;                                     // meshElementNeighborsGroundwaterCumulativeFlow varaible ID
 int     ChanNeighSfcFlowID;                                    // channelElementNeighborsSurfacewaterCumulativeFlow
 double* EleNeighSfcFlow;                                       // mesh Element NeighborsSurfacewaterCumulativeFlow array[nMeshEle*NmeshMeshNeighbors]
 double* EleNeighGrdFlow;                                       // mesh Element NeighborsGroundwaterCumulativeFlow array[nMeshEle*NmeshMeshNeighbors]
 double* ChanNeighSfcFlow;                                      // channelElementNeighborsSurfacewaterCumulativeFlow array[nChannelEle*NchannelChannelNeighbors]
 
 // From geometry.nc
 int     EleNeighID;                                             // mesh Mesh Neighbors variable ID      
 int     meshElementAreaID;                                      // meshElementArea variable ID
 int     meshElementZBedrockID;                                  // meshElementZBedrock varaible ID
 int     ChanNeighID;                                            // channel Channel Neighbors variable ID 
 int     channelElementLengthID;                                 // channelElementLength variable ID
 int* meshMeshNeighbors;                                         // mesh Mesh Neighbors array[nMeshEle*NmeshMeshNeighbors] -> Index describing flow boundary  
 double* meshElementArea;                                        // meshElementArea array[nMeshEle]
 double* meshElementZBedrock;                                    // meshElementZBedrock array[nMeshEle]
 int* channelChannelNeighbors;                                   // channel Channel Neighbors array[nChannelEle*NchannelChannelNeighbors] -> Index describing flow boundary
 double* channelElementLength;                                   // channelElementLength array[nChannelEle]

 // From parameter.nc
 int     meshPorosityID;                                         // meshPorosity varaible ID 
 int     channelElementSlopeID;                                  // channelElementSlope variable ID
 int     channelBaseWidthID;                                     // channel Base Width variable ID
 double* meshPorosity;                                           // meshPorosity array[nMeshEle]
 double* channelElementSlope;                                    // channelElementSlope array[nChannelEle]
 double* channelBaseWidth;                                       // channelBaseWidth array[nChannelEle]
 
 // Cumulative varaibles
 double cumMeshWaterContent;                                     // Cumulative water content in cubic meters for all mesh per time step. Gets reset every time step
 double cumChannelWaterContent;                                  // Cumulative water content in cubic meters for all channels per time step. Gets reset every time step
 double cumMeshMeshNeighborsFlow;                                // Cumulative flow in cubic meters for all mesh per time step. Gets reset every time step  
 double cumChannelChannelNeighborsFlow;                          // Cumulative flow in cubic meters for all channels per time step. Gets reset every time step
 double cumMeshMeshNeighborsFlowOld;                             // Cumulative flow in cubic meters for all mesh from all previous time steps
 double cumChannelChannelNeighborsFlowOld;                       // Cumulative flow in cubic meters for all channels from all previous time steps
 size_t indices[2];                                              // array size 2
                                                                 // indices [0] -> refers to current instance index
                                                                 // indices [1] -> refers to current element index
 
 
 size_t nn, ii;					                // counters
 FILE  *output;                                                 // output file  

  if (argc != 2)
 {
	printf("Usage:\n");
	printf("./adhydro_offline_mass_balance_check <Directory path the .nc output files from adhydro >\n");
	exit(0); 
 }

 // Initialization
 // Netcdf related:
 initialize( argv, &ncid_stt, &ncid_geo, &ncid_par, &nInstances, &nMeshEle, &nChannelEle, &currentTimeID, &meshSurfacewaterDepthID, &meshGroundwaterHeadID, &meshPrecipitationCumulativeID, &meshEvaporationCumulativeID,
             &canIceID, &canLiqID, &snEqvID, &EleNeighSfcFlowID, &EleNeighGrdFlowID, &channelSurfacewaterDepthID, &channelPrecipitationCumulativeID, &channelEvaporationCumulativeID, &ChanNeighSfcFlowID, &NmeshMeshNeighbors,
             &NchannelChannelNeighbors, &EleNeighID, &meshElementZBedrockID, &meshElementAreaID, &ChanNeighID, &channelElementLengthID, &meshPorosityID, &channelBaseWidthID, &channelElementSlopeID );
 // Allocating memory
 // Arrays
 
 // State varaibles:
 meshGroundwaterHead            = malloc(nMeshEle*sizeof(double));
 meshSurfacewaterDepth          = malloc(nMeshEle*sizeof(double));
 meshEvaporationCumulative      = malloc(nMeshEle*sizeof(double));
 meshPrecipitationCumulative     = malloc(nMeshEle*sizeof(double));
 meshCanIce                     = malloc(nMeshEle*sizeof(float));
 meshCanLiq                     = malloc(nMeshEle*sizeof(float));
 meshSnEqv                      = malloc(nMeshEle*sizeof(float));
 channelSurfacewaterDepth       = malloc(nChannelEle*sizeof(double));
 channelEvaporationCumulative   = malloc(nChannelEle*sizeof(double));
 channelPrecipitationCumulative = malloc(nChannelEle*sizeof(double));
 // state Flows:
 EleNeighSfcFlow = malloc(NmeshMeshNeighbors*nMeshEle*sizeof(double));
 EleNeighGrdFlow = malloc(NmeshMeshNeighbors*nMeshEle*sizeof(double));
 ChanNeighSfcFlow = malloc(nChannelEle * NchannelChannelNeighbors * (sizeof(double)));
 
 // geometry variables:
 meshElementZBedrock  = malloc(nMeshEle*sizeof(double));
 meshElementArea      = malloc(nMeshEle*sizeof(double));
 channelElementLength = malloc(nChannelEle*sizeof(double));
 meshMeshNeighbors = malloc(NmeshMeshNeighbors*nMeshEle*sizeof(int));
 channelChannelNeighbors = malloc(NchannelChannelNeighbors*nChannelEle*sizeof(int));
 
 // parameter variables:
 meshPorosity         = malloc(nMeshEle*sizeof(double));
 channelElementSlope  = malloc(nChannelEle*sizeof(double));
 channelBaseWidth     = malloc(nChannelEle*sizeof(double));
 
 // Initialization of geometry and parameter arrays
 for (ii = 0; ii < nMeshEle; ii++)  // Loop over the mesh element
   {
      meshElementArea    [ii] = -1;
      meshPorosity       [ii] = -1;
      meshElementZBedrock[ii] = -1;
  }    

  for (ii = 0; ii < nChannelEle; ii++)  // Loop over the channel element
  {
      channelElementLength[ii] = -1;
      channelElementSlope [ii] = -1;
      channelBaseWidth    [ii] = -1;
  }
  // Initialization
  // Files
  filePathstring = malloc( sizeof( char )*( 1+ strlen(argv[1]) + strlen("/adhydroOffilineMassBalanceCheck.txt") ) );
  sprintf(filePathstring, "%s%s",argv[1],"/adhydroOffilineMassBalanceCheck.txt");
  output = fopen(filePathstring,"w");
 
  free (filePathstring);
 
 cumMeshMeshNeighborsFlowOld         = 0.0;
 cumChannelChannelNeighborsFlowOld   = 0.0;
 // Loop over instances
 for (nn = 0; nn < nInstances; nn++)
  {
       indices[0]     = nn;
       cumMeshMeshNeighborsFlow        = 0;
       cumChannelChannelNeighborsFlow  = 0;
       cumMeshWaterContent             = 0;
       cumChannelWaterContent          = 0;
       
       getElementAndChannelGeometry( ncid_stt, ncid_geo, nn, nMeshEle, nChannelEle, NmeshMeshNeighbors, NchannelChannelNeighbors, meshElementZBedrockID, meshElementAreaID, EleNeighID, channelElementLengthID,  ChanNeighID,
                                     meshElementZBedrock, meshElementArea, channelElementLength, meshMeshNeighbors, channelChannelNeighbors );
                                     
       getElementAndChannelParameter( ncid_stt, ncid_par, nn, nMeshEle, nChannelEle, meshPorosityID, channelElementSlopeID, channelBaseWidthID, meshPorosity, channelElementSlope, channelBaseWidth );     
    
       getStateVars(ncid_stt, nn , nMeshEle, nChannelEle, NmeshMeshNeighbors, NchannelChannelNeighbors, meshSurfacewaterDepthID, meshGroundwaterHeadID, canIceID, canLiqID, snEqvID, meshEvaporationCumulativeID,
           meshPrecipitationCumulativeID, EleNeighSfcFlowID, EleNeighGrdFlowID, channelSurfacewaterDepthID, channelEvaporationCumulativeID, channelPrecipitationCumulativeID,  ChanNeighSfcFlowID, meshSurfacewaterDepth,
           meshGroundwaterHead, meshCanIce, meshCanLiq, meshSnEqv, meshEvaporationCumulative, meshPrecipitationCumulative,  EleNeighSfcFlow, EleNeighGrdFlow, channelSurfacewaterDepth, channelEvaporationCumulative,
           channelPrecipitationCumulative, ChanNeighSfcFlow);
       
     
       err_status = nc_get_var1_double (ncid_stt,currentTimeID, &nn , &currentTime);
       if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"currentTime\" variable from ../state.nc\n");
      	    exit(0);
 	}        

       fprintf(output, "Current time\t%f\n", currentTime);
              printf("instance %lu\n", nn);
       for (ii = 0; ii < nMeshEle; ii++)  // Loop over the mesh element
         {
           indices[1]     = ii;          
           cumMeshWaterContent+= meshEleWaterContent(meshElementZBedrock[ii], meshElementArea[ii], meshPorosity[ii],meshSurfacewaterDepth[ii], meshGroundwaterHead[ii], meshCanIce[ii], meshCanLiq[ii], meshSnEqv[ii]);
                                  
           cumMeshMeshNeighborsFlow += ElemBoundaryflow(ii, nMeshEle, NmeshMeshNeighbors, meshElementArea[ii], meshMeshNeighbors, meshPrecipitationCumulative[ii], meshEvaporationCumulative[ii], 
                                                         EleNeighSfcFlow, EleNeighGrdFlow );
 	 }  // End of Loop over elements

       for (ii = 0; ii < nChannelEle; ii++)  // Loop over the channel 
         {   
 	     indices[1]     = ii;
             cumChannelWaterContent+= channelEleWaterContent (channelElementLength[ii], channelElementSlope[ii], channelBaseWidth[ii], channelSurfacewaterDepth[ii]);
             
             cumChannelChannelNeighborsFlow += ChanBoundaryflow(ii, nChannelEle, NchannelChannelNeighbors, channelChannelNeighbors, ChanNeighSfcFlow, channelEvaporationCumulative[ii], 
                                                             channelPrecipitationCumulative[ii]);     

        }   // End of Loop over channel elements     

 	fprintf(output, "total water %lf, total flows (+ is out) %lf, originally had %lf \n", (cumMeshWaterContent + cumChannelWaterContent), (cumMeshMeshNeighborsFlowOld + cumMeshMeshNeighborsFlow +  
 	        cumChannelChannelNeighborsFlowOld + cumChannelChannelNeighborsFlow), (cumMeshWaterContent + cumChannelWaterContent) + (cumMeshMeshNeighborsFlowOld + cumMeshMeshNeighborsFlow +  
 	        cumChannelChannelNeighborsFlowOld + cumChannelChannelNeighborsFlow) );
 
        cumMeshMeshNeighborsFlowOld         += cumMeshMeshNeighborsFlow;
        cumChannelChannelNeighborsFlowOld   += cumChannelChannelNeighborsFlow;
 	
  }  // end of Loop over timesteps
  
  err_status = nc_close(ncid_stt);
  err_status = nc_close(ncid_geo);
  err_status = nc_close(ncid_par);
  fclose(output);
  
 return 0; 
}
 

 void initialize(char* argv[], int* ncid_stt, int* ncid_geo, int* ncid_par, size_t* nInstances, size_t* nMeshEle, size_t* nChannelEle, int* currentTimeID, int* meshSurfacewaterDepthID, int* meshGroundwaterHeadID,
                 int* meshPrecipitationCumulativeID, int* meshEvaporationCumulativeID, int* canIceID, int* canLiqID, int* snEqvID, int* EleNeighSfcFlowID, int* EleNeighGrdFlowID, int* channelSurfacewaterDepthID, 
                 int* channelPrecipitationCumulativeID, int* channelEvaporationCumulativeID, int* ChanNeighSfcFlowID, size_t* NmeshMeshNeighbors, size_t* NchannelChannelNeighbors, int* EleNeighID, int* meshElementZBedrockID, 
                 int* meshElementAreaID, int* ChanNeighID, int* channelElementLengthID, int* meshPorosityID, int* channelBaseWidthID, int* channelElementSlopeID)
 {
 
 char*  namestring;				                // string to save the file names and directories of the output
 int    err_status;				                // error variable
 int    tmpID;					                // temporary netcdf ID
 
 namestring = malloc( sizeof( char )*( 1+ strlen(argv[1]) + strlen("/channelBalanceCheck.txt") ) );
 
 // Openning the state.nc file

 sprintf(namestring, "%s%s",argv[1],"/state.nc");
 err_status = nc_open(namestring, 0, ncid_stt);
 if (err_status != NC_NOERR)
 {
        printf("In initialize: \n");
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }

 // Openning the geometry.nc file
 sprintf(namestring, "%s%s",argv[1],"/geometry.nc");
 err_status = nc_open(namestring, 0, ncid_geo);
 if (err_status != NC_NOERR)
 {
        printf("In initialize: \n");
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 
 // Openning the parameter.nc file
 sprintf(namestring, "%s%s",argv[1],"/parameter.nc");
 err_status = nc_open(namestring, 0, ncid_par);
 if (err_status != NC_NOERR)
 {
        printf("In initialize: \n");
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }

 // From state.nc file 
 
 // getting Instances' ID from state.nc file
 err_status = nc_inq_dimid (*ncid_stt,"instances" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("In initialize: \n");
     printf("Problem getting \"instances\" id from ../state.nc\n");
     exit(0);
 }
 // # Instances  
 err_status = nc_inq_dimlen  (*ncid_stt, tmpID, nInstances);
 if (err_status != NC_NOERR)
 {
    printf("In initialize: \n");
    printf("Problem getting # \"instances\" from ../state.nc\n");
    exit(0);
  }

  // getting mesh Elements size ID from state.nc file
 err_status = nc_inq_dimid (*ncid_stt,"meshElements" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("In initialize: \n");
     printf("Problem getting \"meshElements\" id from ../state.nc\n");
     exit(0);
 }
 // # mesh Elements  
 err_status = nc_inq_dimlen  (*ncid_stt, tmpID, nMeshEle);
 if (err_status != NC_NOERR)
 {
    printf("In initialize: \n");
    printf("Problem getting # \"instances\" from ../state.nc\n");
    exit(0);
  }

   // getting channel Elements size ID from state.nc file
 err_status = nc_inq_dimid (*ncid_stt,"channelElements" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("In initialize: \n");
     printf("Problem getting \"channelElements\" id from ../state.nc\n");
     exit(0);
 }
 // # of channel elements  
 err_status = nc_inq_dimlen  (*ncid_stt, tmpID, nChannelEle);
 if (err_status != NC_NOERR)
 {
    printf("In initialize: \n");
    printf("Problem getting # \"channelElements\" from ../state.nc\n");
    exit(0);
  }
  
  // Ids only:
 // from state.nc
 err_status = nc_inq_varid 	( *ncid_stt, "currentTime", currentTimeID);
 if (err_status != NC_NOERR)
 {
   printf("In initialize: \n");
   printf("Problem getting the \"currentTime\" variable ID from ../state.nc\n");
   exit(0);
 }
  
 err_status = nc_inq_varid( *ncid_stt, "meshSurfacewaterDepth", meshSurfacewaterDepthID);
 if (err_status != NC_NOERR)
  {
    printf("In initialize: \n");
    printf("Problem getting the \"meshSurfacewaterDepth\" variable ID from ../state.nc\n");
    exit(0);
   }
 
 err_status = nc_inq_varid( *ncid_stt, "meshGroundwaterHead", meshGroundwaterHeadID);
 if (err_status != NC_NOERR)
  {
    printf("In initialize: \n");
    printf("Problem getting the \"meshSurfacewaterHead\" variable ID from ../state.nc\n");
    exit(0);
  }
  
  err_status = nc_inq_varid ( *ncid_stt, "meshPrecipitationCumulative", meshPrecipitationCumulativeID);
  if (err_status != NC_NOERR)
   {
     printf("In initialize: \n");
     printf("Problem getting the \"meshPrecipitationCumulative\" variable ID from ../state.nc\n");
     exit(0);
  } 
  
  err_status = nc_inq_varid ( *ncid_stt, "meshEvaporationCumulative", meshEvaporationCumulativeID);
  if (err_status != NC_NOERR)
  {
    printf("In initialize: \n");
    printf("Problem getting the \"evaporationCumulative\" variable ID from ../state.nc\n");
    exit(0);
  }
     
 err_status = nc_inq_varid ( *ncid_stt, "meshCanIce", canIceID);
 if (err_status != NC_NOERR)
  {
     printf("In initialize: \n");
     printf("Problem getting the \"meshCanIce\" variable ID from ../state.nc\n");
     exit(0);
   }
    
 err_status = nc_inq_varid ( *ncid_stt, "meshCanLiq", canLiqID);
 if (err_status != NC_NOERR)
   {
     printf("In initialize: \n");
     printf("Problem getting the \"canLiq\" variable ID from ../state.nc\n");
     exit(0);
   }
     
  err_status = nc_inq_varid ( *ncid_stt, "meshSnEqv", snEqvID);
  if (err_status != NC_NOERR)
   {
     printf("In initialize: \n");
     printf("Problem getting the \"meshSnEqv\" variable ID from ../state.nc\n");
     exit(0);
   }
   
 err_status  = nc_inq_varid ( *ncid_stt, "meshMeshNeighborsSurfacewaterCumulativeFlow", EleNeighSfcFlowID);
 if (err_status  != NC_NOERR)
 {
   printf("In initialize: \n");
   printf("Problem getting the \"meshMeshNeighborsSurfacewaterCumulativeFlow\" variable ID from ../state.nc\n");
   exit(0);
  }   

 err_status  = nc_inq_varid ( *ncid_stt, "meshMeshNeighborsGroundwaterCumulativeFlow", EleNeighGrdFlowID);
 if (err_status  != NC_NOERR)
  {
    printf("In initialize: \n");
    printf("Problem getting the \"meshMeshNeighborsGroundwaterCumulativeFlow\" variable ID from ../geometry.nc\n");
    exit(0);
  }   

  err_status = nc_inq_varid ( *ncid_stt, "channelSurfacewaterDepth", channelSurfacewaterDepthID);
  if (err_status != NC_NOERR)
   {
    printf("In initialize: \n");
    printf("Problem getting the \"channelSurfacewaterDepth\" variable ID from ../state.nc\n");
    exit(0);
   }
     
 err_status = nc_inq_varid ( *ncid_stt, "channelEvaporationCumulative", channelEvaporationCumulativeID);
 if (err_status != NC_NOERR)
  {
    printf("In initialize: \n");
    printf("Problem getting the \"channelEvaporationCumulative\" variable ID from ../state.nc\n");
    exit(0);
  }
  
  err_status = nc_inq_varid ( *ncid_stt, "channelPrecipitationCumulative", channelPrecipitationCumulativeID);
 if (err_status != NC_NOERR)
  {
    printf("In initialize: \n");
    printf("Problem getting the \"channelPrecipitationCumulative\" variable ID from ../state.nc\n");
    exit(0);
  }
  
  
 
 err_status = nc_inq_varid ( *ncid_stt, "channelChannelNeighborsSurfacewaterCumulativeFlow", ChanNeighSfcFlowID);
  if (err_status != NC_NOERR)
   {
     printf("In initialize: \n");
     printf("Problem getting the \"channelChannelNeighborsSurfacewaterCumulativeFlow\" variable ID from ../geometry.nc\n");
     exit(0);
   }
 
  // From geometry.nc file
  
  // getting meshElement neighbor size ID from geometry.nc file
 err_status = nc_inq_dimid (*ncid_geo,"meshMeshNeighborsSize" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("In initialize: \n");
     printf("Problem getting \"meshMeshNeighborsSize\" id from ../geometry.nc\n");
     exit(0);
 }
 // # meshElement beighbors  
 err_status = nc_inq_dimlen  (*ncid_geo, tmpID, NmeshMeshNeighbors);
 if (err_status != NC_NOERR)
 {
    printf("In initialize: \n");
    printf("Problem getting # \"meshMeshNeighborsSize\" from ../geometry.nc\n");
    exit(0);
  }
  
  // getting channelChannel neighbor size ID from geometry.nc file
 err_status = nc_inq_dimid (*ncid_geo,"channelChannelNeighborsSize" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("In initialize: \n");
     printf("Problem getting \"channelChannelNeighborsSize\" id from ../geometry.nc\n");
     exit(0);
 }
 // # channelElement beighbors  
 err_status = nc_inq_dimlen  (*ncid_geo, tmpID, NchannelChannelNeighbors);
 if (err_status != NC_NOERR)
 {
    printf("In initialize: \n");
    printf("Problem getting # \"channelChannelNeighbors\" from ../geometry.nc\n");
    exit(0);
  }
 // Ids only:
 // Mesh elements
  err_status  = nc_inq_varid 	( *ncid_geo, "meshMeshNeighbors", EleNeighID);
  if (err_status  != NC_NOERR)
   {
    printf("In initialize: \n");
    printf("Problem getting the \"meshMeshNeighbors\" variable ID from ../geometry.nc\n");
    exit(0);
   }
  
  err_status = nc_inq_varid 	( *ncid_geo, "meshElementZBedrock", meshElementZBedrockID);
  if (err_status != NC_NOERR)
   {
      printf("In initialize: \n");
      printf("Problem getting the \"meshElementZBedrock\" variable ID from ../geometry.nc\n");
      exit(0);
   }
 
  err_status = nc_inq_varid 	( *ncid_geo, "meshElementArea", meshElementAreaID);
  if (err_status != NC_NOERR)
   {
     printf("In initialize: \n");
     printf("Problem getting the \"meshElementArea\" variable ID from ../geometry.nc\n");
     exit(0);
   }
                 
  // Channel elements
  err_status = nc_inq_varid 	( *ncid_geo, "channelChannelNeighbors", ChanNeighID);
  if (err_status != NC_NOERR)
   {
     printf("In initialize: \n");
     printf("Problem getting the \"channelChannelNeighbors\" variable ID from ../geometry.nc\n");
     exit(0);
   }
   
  err_status = nc_inq_varid 	( *ncid_geo, "channelElementLength", channelElementLengthID);
  if (err_status != NC_NOERR)
   {
      printf("In initialize: \n");
      printf("Problem getting the \"channelElementLength\" variable ID from ../geometry.nc\n");
      exit(0);
   }
             
  // from parameter.nc file:
  err_status = nc_inq_varid 	( *ncid_par, "meshPorosity", meshPorosityID);
  if (err_status != NC_NOERR)
   {
      printf("In initialize: \n");
      printf("Problem getting the \"meshPorosity\" variable ID from ../parameter.nc\n");
      exit(0);
   }

  err_status = nc_inq_varid 	( *ncid_par, "channelBaseWidth", channelBaseWidthID);
  if (err_status != NC_NOERR)
   {
      printf("In initialize: \n");
      printf("Problem getting the \"channelBaseWidth\" variable ID from ../parameter.nc\n");
      exit(0);
   }

  err_status = nc_inq_varid 	( *ncid_par, "channelSideSlope", channelElementSlopeID);
  if (err_status != NC_NOERR)
  {
    printf("In initialize: \n");
    printf("Problem getting the \"channelSideSlope\" variable ID from ../parameter.nc\n");
    exit(0);
  }

}

void getElementAndChannelGeometry(int ncid_stt, int ncid_geo, size_t stateInstance, size_t nMeshEle, size_t nChannelEle,  size_t nMeshNgb, size_t nChannelNgb, int meshElementZBedrockID, int meshElementAreaID, int EleNeighID, 
                                  int channelElementLengthID, int ChanNeighID, double meshElementZBedrock[nMeshEle], double meshElementArea[nMeshEle], double channelElementLength[nChannelEle], 
                                  int meshMeshNeighbors[nMeshEle*nMeshNgb], int channelChannelNeighbors[nChannelEle*nChannelNgb]);
 {
    int      err_status;                 // error variable
    int      geometryInstanceID;
    unsigned long long int geometryInstance;
    unsigned long long int geometryInstanceOld;
    size_t   start[3];
    size_t   count[3];
 
   if (meshElementArea[0] < 0) // It has no values yet
    {
        err_status = nc_inq_varid 	( ncid_stt, "geometryInstance", &geometryInstanceID);
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelGeometry:\n");
           printf("Problem getting the \"geometryInstance\" variable ID from ../state.nc\n");
           exit(0);
        }

       err_status = nc_get_var1_ulonglong ( ncid_stt,geometryInstanceID, &stateInstance, &geometryInstance); 	
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelGeometry:\n");
           printf("Problem getting the \"geometryInstance\" variable from ../state.nc\n");
           exit(0);
        }   
       
      start[0] = geometryInstance;
      start[1] = 0;
      start[2] = 0;
      count[0] = 1;
      count[1] = nMeshEle;
      count[2] = nMeshNgb;    
      
      err_status =  nc_get_vara_double ( ncid_geo,meshElementZBedrockID,start, count, meshElementZBedrock);	
      if (err_status != NC_NOERR)
          {
            printf ("In getElementAndChannelGeometry:\n");
            printf("Problem getting the \"meshElementZBedrock\" variable from ../geometry.nc\n");
            exit(0);
          }   
      
      err_status =  nc_get_vara_double ( ncid_geo,meshElementAreaID,start, count, meshElementArea);	
      if (err_status != NC_NOERR)
         {
           printf ("In getElementAndChannelGeometry:\n");
           printf("Problem getting the \"meshElementArea\" variable from ../geometry.nc\n");
           exit(0);
         } 
     
       err_status =  nc_get_vara_int ( ncid_geo,EleNeighID, start, count, meshMeshNeighbors); 	
       if (err_status != NC_NOERR)
       {
             printf ("In getElementAndChannelGeometry:\n");
             printf("Problem getting the \"meshMeshNeighbors\" variable from ../geometry.nc\n");
             exit(0);
       }
       
     
      count[1] = nChannelEle;
      count[2] = nChannelNgb;
      
      err_status =  nc_get_vara_double ( ncid_geo,channelElementLengthID,start, count, channelElementLength);	
      if (err_status != NC_NOERR)
       {
             printf ("In getElementAndChannelGeometry:\n");
             printf("Problem getting the \"channelElementLenght\" variable from ../geometry.nc\n");
             exit(0);
       }   
  
       err_status = nc_get_vara_int ( ncid_geo, ChanNeighID, start, count, channelChannelNeighbors); 	
       if (err_status != NC_NOERR)
          {
             printf ("In getElementAndChannelGeometry:\n");
               printf("Problem getting the \"channelChannelNeighbors\" variable from ../geometry.nc\n");
               exit(0);
          }
  
   }
   else // If alread have a value
    {
       err_status = nc_inq_varid 	( ncid_stt, "geometryInstance", &geometryInstanceID);
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelGeometry:\n");
           printf("Problem getting the \"geometryInstance\" variable ID from ../state.nc\n");
           exit(0);
        }
       err_status = nc_get_var1_ulonglong ( ncid_stt, geometryInstanceID, &stateInstance, &geometryInstance); 	
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelGeometry:\n");
           printf("Problem getting the \"geometryInstance\" variable from ../state.nc\n");
           exit(0);
        }
        stateInstance -= 1;
       err_status = nc_get_var1_ulonglong ( ncid_stt,geometryInstanceID, &stateInstance, &geometryInstanceOld); 	
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelGeometry:\n");
           printf("Problem getting the \"geometryInstance\" variable from ../state.nc\n");
           exit(0);
        }
        
        if (geometryInstance != geometryInstanceOld)
         {
            
            start[0] = geometryInstance;
            start[1] = 0; 
            start[2] = 0;
            count[0] = 1; 
            count[1] = nMeshEle;
            count[2] = nMeshNgb;    
      
           err_status =  nc_get_vara_double ( ncid_geo,meshElementZBedrockID,start, count, meshElementZBedrock);	
           if (err_status != NC_NOERR)
             {
                printf ("In getElementAndChannelGeometry:\n");
                printf("Problem getting the \"meshElementZBedrock\" variable from ../geometry.nc\n");
                exit(0);
             }   
      
          err_status =  nc_get_vara_double ( ncid_geo,meshElementAreaID,start, count, meshElementArea);	
          if (err_status != NC_NOERR)
           {
               printf ("In getElementAndChannelGeometry:\n");
               printf("Problem getting the \"meshElementArea\" variable from ../geometry.nc\n");
               exit(0);
           } 
     
          err_status =  nc_get_vara_int ( ncid_geo,EleNeighID, start, count, meshMeshNeighbors); 	
          if (err_status != NC_NOERR)
          {
                printf ("In getElementAndChannelGeometry:\n");
                printf("Problem getting the \"meshMeshNeighbors\" variable from ../geometry.nc\n");
                exit(0);
            }
       
     
         count[1] = nChannelEle;
         count[2] = nChannelNgb;
        
         err_status =  nc_get_vara_double ( ncid_geo,channelElementLengthID,start, count, channelElementLength);	
         if (err_status != NC_NOERR)
          {
                printf ("In getElementAndChannelGeometry:\n");
                printf("Problem getting the \"channelElementLenght\" variable from ../geometry.nc\n");
                exit(0);
           }   
  
        err_status = nc_get_vara_int ( ncid_geo, ChanNeighID, start, count, channelChannelNeighbors); 	
        if (err_status != NC_NOERR)
          {
               printf ("In getElementAndChannelGeometry:\n");
               printf("Problem getting the \"channelChannelNeighbors\" variable from ../geometry.nc\n");
               exit(0);
          }
       }
       
      else    
         return;    
    }   
 }
 
void getElementAndChannelParameter( int ncid_stt, int ncid_par, size_t stateInstance, size_t nMeshEle, size_t nChannelEle, int meshPorosityID, int channelElementSlopeID, int channelBaseWidthID, 
                                      double meshPorosity[nMeshEle], double channelElementSlope[nChannelEle], double channelBaseWidth[nChannelEle])
 {
    int      err_status;                 // error variable
    int      parameterInstanceID;
    unsigned long long int parameterInstance;
    unsigned long long int parameterInstanceOld;
    size_t   start[2];
    size_t   count[2];
    
   if (meshPorosity[0] < 0) // It has no values yet
    {
       err_status = nc_inq_varid 	( ncid_stt, "parameterInstance", &parameterInstanceID);
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelParameter:\n");
           printf("Problem getting the \"geometryInstance\" variable ID from ../state.nc\n");
           exit(0);
        }
       
       err_status = nc_get_var1_ulonglong ( ncid_stt,parameterInstanceID, &stateInstance, &parameterInstance); 	
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelParameter:\n");
           printf("Problem getting the \"geometryInstance\" variable from ../state.nc\n");
           exit(0);
        }   
   
      start[0] = parameterInstance;
      start[1] = 0;
      count[0] = 1;
      count[1] = nMeshEle;
   
   err_status =  nc_get_vara_double ( ncid_par,meshPorosityID,start, count, meshPorosity);	
   if (err_status != NC_NOERR)
          {
            printf ("In getElementAndChannelParameter:\n");
            printf("Problem getting the \"meshPorosity\" variable from ../parameter.nc\n");
            exit(0);
          }   
   
   count[1] = nChannelEle;
   err_status =  nc_get_vara_double ( ncid_par,channelElementSlopeID,start, count, channelElementSlope);	
   if (err_status != NC_NOERR)
          {
             printf ("In getElementAndChannelParameter:\n");
             printf("Problem getting the \"channelElementLenght\" variable from ../parameter.nc\n");
             exit(0);
          }
   err_status =  nc_get_vara_double ( ncid_par,channelBaseWidthID,start, count, channelBaseWidth);	
        if (err_status != NC_NOERR)
          {
             printf ("In getElementAndChannelParameter:\n");
             printf("Problem getting the \"channelBaseWidth\" variable from ../parameter.nc\n");
             exit(0);
          }   

   }
   else // If alread have a value
    {
       err_status = nc_inq_varid 	( ncid_stt, "parameterInstance", &parameterInstanceID);
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelParameter:\n");
           printf("Problem getting the \"parameterInstance\" variable ID from ../state.nc\n");
           exit(0);
        }
       err_status = nc_get_var1_ulonglong ( ncid_stt,parameterInstanceID, &stateInstance, &parameterInstance); 	
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelParameter:\n");
           printf("Problem getting the \"parameterInstance\" variable from ../state.nc\n");
           exit(0);
        }
       
       stateInstance -= 1;
       err_status = nc_get_var1_ulonglong ( ncid_stt,parameterInstanceID, &stateInstance, &parameterInstanceOld); 	
       if (err_status != NC_NOERR)
        {
           printf ("In getElementAndChannelParameter:\n");
           printf("Problem getting the \"parameterInstance\" variable from ../state.nc\n");
           exit(0);
        }
        
        if (parameterInstance != parameterInstanceOld)
         {
            printf("Inside if that shouldnt be\n");
            
            start[0] = parameterInstance;
            start[1] = 0;
            count[0] = 1;
            count[1] = nMeshEle;
            
            err_status =  nc_get_vara_double ( ncid_par,meshPorosityID,start, count, meshPorosity);	
            if (err_status != NC_NOERR)
             {
                printf ("In getElementAndChannelParameter:\n");
                printf("Problem getting the \"meshPorosity\" variable from ../parameter.nc\n");
                exit(0);
             }   
   
          count[1] = nChannelEle;
    
          err_status =  nc_get_vara_double ( ncid_par,channelElementSlopeID,start, count, channelElementSlope);	
          if (err_status != NC_NOERR)
          {
               printf ("In getElementAndChannelParameter:\n");
               printf("Problem getting the \"channelElementLenght\" variable from ../parameter.nc\n");
               exit(0);
          }
          err_status =  nc_get_vara_double ( ncid_par,channelBaseWidthID,start, count, channelBaseWidth);	
          if (err_status != NC_NOERR)
            {
               printf ("In getElementAndChannelParameter:\n");
               printf("Problem getting the \"channelBaseWidth\" variable from ../parameter.nc\n");
               exit(0);
          }   
       }           
       else
          return;
    }

 }
 
 double meshEleWaterContent(double meshElementZBedrock, double meshElementArea, double meshPorosity, double meshSurfacewaterDepth , double meshGroundwaterHead, float meshCanIce, float meshCanLiq, float meshSnEqv)
 {

   double meshSurfacewaterDepth1 = meshSurfacewaterDepth;      // Mesh Surface Water Depth in meters
   double meshGroundwaterHead1 = meshGroundwaterHead;          // Mesh Ground Water Head in meters
   double meshCanLiq1 = meshCanLiq;                            // Cummulative canopy liquid water equivalent in meters of water
   double meshCanIce1 = meshCanIce;                            // Cummulative canopy ice water equivalent in meters of water
   double meshSnEqv1  = meshSnEqv;                             // Cummulative Snow water equivalent in meters of water
                   
     meshSurfacewaterDepth1 *= meshElementArea;
     meshGroundwaterHead1 =  (meshGroundwaterHead1 - meshElementZBedrock)*meshPorosity*meshElementArea;
     meshCanIce1 *= meshElementArea/1000.0;
     meshCanLiq1 *= meshElementArea/1000.0;
     meshSnEqv1  *= meshElementArea/1000.0;
     
     return (meshGroundwaterHead1 + meshSurfacewaterDepth1 + meshSnEqv1 + meshCanLiq1 + meshCanIce1 );
     
 }
 
 double channelEleWaterContent (double channelElementLength, double channelElementSlope, double channelBaseWidth, double channelSurfacewaterDepth)
 {
   double channelArea;              // Channel Area in Square Meters
   double channelSurfacewater;      // Channel Surface Water Depth in cubic meters 
   
       
       channelArea = (channelElementSlope*channelSurfacewaterDepth + channelBaseWidth )*channelSurfacewaterDepth;           
       channelSurfacewater = channelArea*channelElementLength;
    
     return (channelSurfacewater);
 }
 
double ElemBoundaryflow( int currentEle, size_t nMeshEle, int NmeshMeshNeighbors, double meshElementArea, int meshMeshNeighbors [nMeshEle*NmeshMeshNeighbors], double meshPrecipitationCumulative, 
                         double meshEvaporationCumulative, double EleNeighSfcFlow[nMeshEle*NmeshMeshNeighbors], double EleNeighGrdFlow[nMeshEle*NmeshMeshNeighbors])
 {
   int boundary;                            // counter
   int nNeighbor;                           // counter
   int matrixOffset;
   double PrcpCum;                          // cumulative precipitation in cubic meters of water -> Inflow
   double CumEvapo;                         // cumulative evaporation in cubic meters of water   -> Outflow
   double EleSfcFlow;                       // element neighbor surface cumulative flow from state.nc 
   double EleGrdFlow;                       // element neighbor ground cumulative flow from state.nc 
   double meshNeighborsinFlow = 0;          // OutPut in cubic meters of water.
   double meshNeighborsoutFlow = 0;         // OutPut in cubic meters of water.
//   double interMeshFlow        = 0;       // OutPut in cubic meters of water. -> Should be = 0.

      
  PrcpCum = meshPrecipitationCumulative*meshElementArea; 
         
  CumEvapo = meshEvaporationCumulative*meshElementArea;
 
    
 for (nNeighbor = 0; nNeighbor < NmeshMeshNeighbors; nNeighbor++)
  {  
       matrixOffset = currentEle*NmeshMeshNeighbors + nNeighbor;
       boundary = meshMeshNeighbors[matrixOffset];
 
       if (boundary < 0.0)    
        {   
            EleSfcFlow = EleNeighSfcFlow[matrixOffset];
            EleGrdFlow = EleNeighGrdFlow[matrixOffset];
            
            if (NOFLOW == boundary)
             {
                 assert (EleSfcFlow == 0.0 );
                 assert (EleGrdFlow == 0.0) ;      
            }
          
            else if (INFLOW == boundary)
            {  
                 meshNeighborsinFlow += EleSfcFlow;
                 meshNeighborsinFlow += EleGrdFlow;
             }
          
             else if (OUTFLOW == boundary)
             {
                 meshNeighborsoutFlow+= EleSfcFlow; 
                 meshNeighborsoutFlow+= EleGrdFlow; 
             }
                   
       }   
   }
 // PrcpCum is inflow, therefore considered negative
 // Outflow is outflow, therefore considered positive
 // CumEvapo < 0 means water removed
 // PrcpCum > 0 means water added
 
 return  meshNeighborsoutFlow - CumEvapo + meshNeighborsinFlow - PrcpCum;
 }
 
double ChanBoundaryflow(int currentElem, size_t nChannelEle,int NchannelChannelNeighbors,int channelChannelNeighbors[nChannelEle*NchannelChannelNeighbors], 
                         double ChanNeighSfcFlow[nChannelEle*NchannelChannelNeighbors], double cumEvapo, double cumPrcp)
 {
   int boundary;                                // counter
   int nNeighbor;                               // counter
   int matrixOffset;
   double ChanNeighFlow;                    // 
   double channelNeighborsInFlow = 0;    // OutPut in cubic meters of water
   double channelNeighborsOutFlow = 0;   // OutPut in cubic meters of water

   
 // FIXME the area for evapo and prcp must be the top of the channel area  
   /*  cumEvapo = cumEvapo*channelArea;
     cumPrcp  = cumPrcp*channelArea; */
  
  for (nNeighbor = 0; nNeighbor < NchannelChannelNeighbors; nNeighbor++)
    {  
      //printf("element %d, ngb %d\n", currentElem, nNeighbor);
      matrixOffset = currentElem*NchannelChannelNeighbors + nNeighbor;
      boundary     = channelChannelNeighbors[matrixOffset];
      
       if (boundary < 0)
          {
              ChanNeighFlow = ChanNeighSfcFlow[matrixOffset];

               if (NOFLOW == boundary)
                {
                 //assert (ChanNeighFlow == 0.0);
                 if(ChanNeighFlow!= 0.0)
                  {
                    printf("ChanNeighFlow was supposed to be 0.0, but it is %f\n",ChanNeighFlow );
                  }
               }
          
               else if (INFLOW == boundary)
               {  
                 channelNeighborsInFlow += ChanNeighFlow;
                }
          
               else if (OUTFLOW == boundary)
               {
                 channelNeighborsOutFlow += ChanNeighFlow; 
               }   
         }
   }
   
   assert (cumEvapo == 0.0);
   assert (cumPrcp == 0.0);
// Outflow is outflow, therefore considered positive
// CumEvapo < 0 means water removed
  return channelNeighborsOutFlow - cumEvapo + channelNeighborsInFlow - cumPrcp;
 }
 
void getStateVars(int ncid_stt, size_t stateInstance, size_t nMeshEle, size_t nChannelEle, size_t nMeshNgb, size_t nChannelNgb, int meshSurfacewaterDepthID, int meshGroundwaterHeadID, int canIceID, int canLiqID, int snEqvID, 
                  int meshCummEvapoID, int meshCummPrcpID, int EleNeighSfcFlowID, int EleNeighGrdFlowID, int channelSurfacewaterDepthID, int channelCummEvapoID, int channelCummPrcpID, int ChanNeighSfcFlowID,
                  double meshSurfacewaterDepth[nMeshEle], double meshGroundwaterHead[nMeshEle], float meshCanIce[nMeshEle], float meshCanLiq[nMeshEle], float meshSnEqv[nMeshEle], double meshCummEvapo[nMeshEle], 
                  double meshCummPrcp[nMeshEle], double EleNeighSfcFlow[nMeshEle*nMeshNgb], double EleNeighGrdFlow[nMeshEle*nMeshNgb], double channelSurfacewaterDepth[nChannelEle], double channelCummEvapo[nChannelEle], 
                  double channelCummPrcp[nChannelEle], double ChanNeighSfcFlow[nChannelEle*nChannelNgb] )
 {
    int      err_status; 
    size_t   start[3];
    size_t   count[3];        
     
      start[0] = stateInstance;
      start[1] = 0;
      start[2] = 0;
      
      count[0] = 1;
      count[1] = nMeshEle;
      count[2] = nMeshNgb;
      
     
     err_status =  nc_get_vara_double ( ncid_stt,meshSurfacewaterDepthID,start, count, meshSurfacewaterDepth);	
     if (err_status != NC_NOERR)
       {
           printf ("In getStateVars:\n");
           printf("Problem getting the \"meshSurfacewaterDepth\" variable from ../state.nc\n");
           exit(0);
      }
            
     err_status =  nc_get_vara_double ( ncid_stt,meshGroundwaterHeadID, start, count, meshGroundwaterHead); 	
     if (err_status != NC_NOERR)
       {
           printf ("In getStateVars:\n");
           printf("Problem getting the \"meshSurfacewaterHead\" variable from ../state.nc\n");
           exit(0);
       }
 	   
     err_status =  nc_get_vara_float  ( ncid_stt,canIceID, start, count, meshCanIce); 	
     if (err_status != NC_NOERR)
       {
           printf ("In getStateVars:\n");
           printf("Problem getting the \"canIce\" variable from ../state.nc\n");
           exit(0);
       }
     
     err_status =  nc_get_vara_float ( ncid_stt,canLiqID,start, count, meshCanLiq); 	
     if (err_status != NC_NOERR)
       {
           printf ("In getStateVars:\n");
           printf("Problem getting the \"canLiq\" variable from ../state.nc\n");
           exit(0);
       }
      
     err_status =  nc_get_vara_float ( ncid_stt,snEqvID,start, count, meshSnEqv); 	
     if (err_status != NC_NOERR)
       {
           printf ("In getStateVars:\n");
           printf("Problem getting the \"snEqv\" variable from ../state.nc\n");
           exit(0);
       }
    
    err_status =  nc_get_vara_double ( ncid_stt,meshCummEvapoID, start, count, meshCummEvapo); 	
    if (err_status != NC_NOERR)
     {
       printf ("In getStateVars:\n");
       printf("Problem getting the \"evaporationCumulative\" variable from ../state.nc\n");
       exit(0);
    }
  
    err_status =  nc_get_vara_double ( ncid_stt,meshCummPrcpID, start, count, meshCummPrcp); 	
    if (err_status != NC_NOERR) 
     {
       printf ("In getStateVars:\n");
        printf("Problem getting the \"meshPrecipitationCumulative\" variable ID from ../state.nc\n");
        exit(0);
     }      
    
    err_status =  nc_get_vara_double ( ncid_stt,EleNeighSfcFlowID,  start, count, EleNeighSfcFlow); 	
    if (err_status != NC_NOERR)
     {
          printf ("In getStateVars:\n");
          printf("Problem getting the \"Element Neighbor Surface water Flow\" variable from ../state.nc\n");
          exit(0);
     } 

    err_status =  nc_get_vara_double ( ncid_stt,EleNeighGrdFlowID,  start, count, EleNeighGrdFlow); 	
    if (err_status != NC_NOERR)
      {
          printf ("In getStateVars:\n");
          printf("Problem getting the \"Element Neighbor Gorubnd water Flow\" variable from ../state.nc\n");
          exit(0);
      }
    
    count[1] = nChannelEle;
    count[2] = nChannelNgb;
      
    err_status =  nc_get_vara_double ( ncid_stt,channelSurfacewaterDepthID,start, count, channelSurfacewaterDepth); 	
    if (err_status != NC_NOERR)
      {
          printf ("In getStateVars:\n");
          printf("Problem getting the \"channelSurfacewaterDepth\" variable from ../state.nc\n");
          exit(0);
    } 
    
    err_status =  nc_get_vara_double ( ncid_stt,channelCummEvapoID, start, count, channelCummEvapo); 	
    if (err_status != NC_NOERR)
     {
       printf ("In getStateVars:\n");
       printf("Problem getting the \"evaporationCumulative\" variable from ../state.nc\n");
       exit(0);
    }
  
    err_status =  nc_get_vara_double ( ncid_stt,channelCummPrcpID, start, count, channelCummPrcp); 	
    if (err_status != NC_NOERR) 
     {
        printf ("In getStateVars:\n");
        printf("Problem getting the \"meshPrecipitationCumulative\" variable ID from ../state.nc\n");
        exit(0);
     } 
     
    err_status =  nc_get_vara_double ( ncid_stt, ChanNeighSfcFlowID, start, count, ChanNeighSfcFlow); 	
    if (err_status != NC_NOERR)
     {
        printf ("In getStateVars:\n");
        printf("Problem getting the \"channelSurfacewaterDepth\" variable from ../state.nc\n");
        exit(0);
      }     
    
 }
