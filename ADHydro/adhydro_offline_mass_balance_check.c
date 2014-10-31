#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>
#include <string.h>
#include "adhydro_offline_mass_balance_check.h"

int main(int argc, char*argv[])
{
// Propose:
// Offline check of the water balance of adhydro simulation.
// Calculates the amount of water in all mesh and channel elements for each time step.
// Prits off values on 2 files:
// channelBalanceCheck.txt
// meshBalanceCheck.txt
// They contain:
// 1 # of time steps   # of elements
// 2 current time
// 3 cumWaterContent, cumNeighborsFlow, cumWaterContent + cumFlow
// 2 and 3 repeats for each time step.
// Files are being saved in the same folder as the output .nc files from adhydro.
// #####################################################################################################################
// ######### IMPORTANT: To choose the folder where the output files should be saved, go to lines 114 and 117 ###########
// #####################################################################################################################

 char*  filePathstring;				                // string to save the file names and directories of the output
 int    err_status;				                // error variable
 int    ncid_stt,ncid_geo, ncid_par;                            // id of the state.nc, geometry.nc, and parameter.nc files	
 size_t nInstances,nMeshEle, nChannelEle;                  	// # of: instances, mesh elements, and channel elements in the *.nc files		

 // from state.nc
 double currentTime;					       // current time of simulation in the state.nc file
 int currentTimeID;                                            // current time variable ID
 int meshGroundwaterHeadID;                                    // meshGroundwaterHead variable ID
 int meshSurfacewaterDepthID;                                  // meshSurfacewaterDepth varaible ID
 int meshPrecipitaitonCumulativeID;                            // meshPrecipitationCumulative varaible ID
 int meshEvaporationCumulativeID;                              // meshEvaporationCumulative varaible ID
 int canIceID;                                                 // canopy ice varaible ID
 int canLiqID;                                                 // canopyliquid varaible ID
 int snEqvID;                                                  // snow water equivalent varaible ID
 int channelSurfacewaterDepthID;                               // channelSurface water Depth varaible ID
 int channelEvaporationCumulativeID;                           // channelEvaporationCumulative ID
 double channelEvaporationCumulative;                          // channelEvaporationCumulative value
                                                               // Needed as a output of channelEleWaterContent() to avoid code repetition
 // from geometry.nc
 size_t  NmeshMeshNeighbors;                                     // # of meshMesh neighbors
 size_t  NchannelChannelNeighbors;                               // # of channelChannel neighbors
 int     EleNeighID;                                             // mesh Mesh Neighbors variable ID 
 int     EleNeighgSfcFlowID;                                     // mesh Element NeighborsSurfacewaterCumulativeFlow variable ID 
 int     EleNeighgGrdFlowID;                                     // meshElementNeighborsGroundwaterCumulativeFlow varaible ID
 int     ChanNeighID;                                            // channelChannelNeighbors
 int     ChanNeighgSfcFlowID;                                    // channelElementNeighborsSurfacewaterCumulativeFlow
 int     meshElementAreaID;                                      // meshElementArea variable ID
 int     meshElementZBedrockID;                                  // meshElementZBedrock varaible ID
 int     channelElementLengthID;                                 // channelElementLength variable ID
 double* meshElementZBedrock;                                    // meshElementZBedrock vector of values for all mesh elements
 double* meshElementArea;                                        // meshElementArea vector of values for all mesh elements
 double* channelElementLength;                                   // channelElementLength vector of values for all channels elements
 
 // From parameter.nc
 int     meshPorosityID;                                         // meshPorosity varaible ID 
 double* meshPorosity;                                           // meshPorosity vector of values for all mesh elements
 int     channelElementSlopeID;                                  // channelElementSlope variable ID
 double* channelElementSlope;                                    // channelElementSlope vector of values for all channel elements


 double cumMeshMeshNeighborsFlow;
 double cumChannelChannelNeighborsFlow;
 double cumMeshWaterContent;
 double cumChannelWaterContent;
 size_t indices[2];                                             // array size 2
                                                                // indices [0] -> refers to current instance index
                                                                // indices [1] -> refers to current element index
 size_t flowIndices[3];                                         // array size 3
                                                                // indices [0] -> refers to current instance index
                                                                // indices [1] -> refers to current element index
                                                                // indice  [2] -> refers to current neighbor index                                                             
 
 size_t nn, ii;					                // counters
 FILE  *meshOutput, *channelOutput;

  if (argc != 2)
 {
	printf("Usage:\n");
	printf("./adhydro_offline_mass_balance_check <Directory path the .nc output files from adhydro >\n");
	exit(0); 
 }

 // Initialization
 // Netcdf related:
 initialize( argv, &ncid_stt, &ncid_geo, &ncid_par, &nInstances, &nMeshEle, &nChannelEle, &currentTimeID, &meshSurfacewaterDepthID, &meshGroundwaterHeadID, &meshPrecipitaitonCumulativeID, &meshEvaporationCumulativeID,
             &canIceID, &canLiqID, &snEqvID, &EleNeighgSfcFlowID, &EleNeighgGrdFlowID, &channelSurfacewaterDepthID, &channelEvaporationCumulativeID, &ChanNeighgSfcFlowID, &NmeshMeshNeighbors, &NchannelChannelNeighbors, 
             &EleNeighID, &meshElementZBedrockID, &meshElementAreaID, &ChanNeighID, &channelElementLengthID, &meshPorosityID, &channelElementSlopeID );
 // Initialization
 // Vectors
 meshElementZBedrock  = malloc(nMeshEle*sizeof(double));
 meshElementArea      = malloc(nMeshEle*sizeof(double));
 meshPorosity         = malloc(nMeshEle*sizeof(double));
 channelElementLength = malloc(nChannelEle*sizeof(double));
 channelElementSlope  = malloc(nChannelEle*sizeof(double));
 
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
  }
  // Initialization
  // Files
  filePathstring = malloc( sizeof( char )*( 1+ strlen(argv[1]) + strlen("/channelBalanceCheck.txt") ) );
  
  sprintf(filePathstring, "%s%s",argv[1],"/meshBalanceCheck.txt");
  meshOutput = fopen(filePathstring,"w");
 
  sprintf(filePathstring, "%s%s",argv[1],"/channelBalanceCheck.txt");
  channelOutput = fopen(filePathstring,"w");
 
  free (filePathstring);

  fprintf(meshOutput, "%zu  %zu\n", nInstances, nMeshEle);
  fprintf(channelOutput, "%zu  %zu\n", nInstances, nChannelEle);

 // Loop over timesteps
 for (nn = 0; nn < nInstances; nn++)
  {
       indices[0]     = nn;
       flowIndices[0] = nn;
       cumMeshMeshNeighborsFlow        = 0;
       cumChannelChannelNeighborsFlow  = 0;
       cumMeshWaterContent             = 0;
       cumChannelWaterContent          = 0;
       
       getElementAndChannelGeometry( ncid_stt, ncid_geo, nn, nMeshEle, nChannelEle, meshElementZBedrockID, meshElementAreaID, channelElementLengthID,  meshElementZBedrock, meshElementArea, channelElementLength );
       getElementAndChannelParameter( ncid_stt, ncid_par, nn, nMeshEle, nChannelEle, meshPorosityID, channelElementSlopeID, meshPorosity, channelElementSlope );
       
       
       err_status = nc_get_var1_double (ncid_stt,currentTimeID, &nn , &currentTime);
       if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"currentTime\" variable from ../state.nc\n");
      	    exit(0);
 	}        

       fprintf(meshOutput, "Current time\t%f\n", currentTime);
       fprintf(channelOutput, "Current time\t%f\n", currentTime);

       for (ii = 0; ii < nMeshEle; ii++)  // Loop over the mesh element
         {
           indices[1]     = ii;          
           cumMeshWaterContent+= meshEleWaterContent(ncid_stt, ncid_par, ncid_geo, indices, meshElementZBedrock[ii], meshElementArea[ii], meshPorosity[ii],
                                                     meshSurfacewaterDepthID, meshGroundwaterHeadID, canIceID, canLiqID, snEqvID);
           cumMeshMeshNeighborsFlow += ElemBoundaryflow(ncid_stt, ncid_geo, indices, meshElementArea[ii], meshPrecipitaitonCumulativeID, meshEvaporationCumulativeID, nMeshEle, NmeshMeshNeighbors, EleNeighID, EleNeighgSfcFlowID, EleNeighgGrdFlowID );
 	 }  // End of Loop over elements

       fprintf(meshOutput, "%f\t%f\t%f\n", cumMeshWaterContent, cumMeshMeshNeighborsFlow, cumMeshWaterContent + cumMeshMeshNeighborsFlow );
       for (ii = 0; ii < nChannelEle; ii++)  // Loop over the channel 
         {   
 	     indices[1]     = ii;
             cumChannelWaterContent+= channelEleWaterContent (ncid_stt, ncid_geo, ncid_par, indices, channelElementLength[ii], channelElementSlope[ii], channelSurfacewaterDepthID, channelEvaporationCumulativeID, &channelEvaporationCumulative);
             cumChannelChannelNeighborsFlow += ChanBoundaryflow(ncid_stt, ncid_geo, indices, nChannelEle, NchannelChannelNeighbors, ChanNeighID, ChanNeighgSfcFlowID, channelEvaporationCumulative);     
 
        }   // End of Loop over channel elements     

 	fprintf(channelOutput, "%f\t%f\t%f \n", cumChannelWaterContent, cumChannelChannelNeighborsFlow, cumChannelWaterContent + cumChannelChannelNeighborsFlow);
 	
  }  // end of Loop over timesteps
  
  err_status = nc_close(ncid_stt);
  err_status = nc_close(ncid_geo);
  err_status = nc_close(ncid_par);
  fclose(meshOutput);
  fclose(channelOutput);
  
 return 0; 
}
 

 void initialize(char* argv[], int* ncid_stt, int* ncid_geo, int* ncid_par, size_t* nInstances, size_t* nMeshEle, size_t* nChannelEle, int* currentTimeID, int* meshSurfacewaterDepthID, 
                 int* meshGroundwaterHeadID, int* meshPrecipitaitonCumulativeID, int* meshEvaporationCumulativeID, int* canIceID, int* canLiqID, int* snEqvID, int* EleNeighgSfcFlowID, int* EleNeighgGrdFlowID, int* channelSurfacewaterDepthID,
                 int* channelEvaporationCumulativeID, int* ChanNeighgSfcFlowID, size_t* NmeshMeshNeighbors, size_t* NchannelChannelNeighbors, int* EleNeighID, int* meshElementZBedrockID, int* meshElementAreaID,
                 int* ChanNeighID, int* channelElementLengthID, int* meshPorosityID, int* channelElementSlopeID)
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
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }

 // Openning the geometry.nc file
 sprintf(namestring, "%s%s",argv[1],"/geometry.nc");
 err_status = nc_open(namestring, 0, ncid_geo);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 
 // Openning the parameter.nc file
 sprintf(namestring, "%s%s",argv[1],"/parameter.nc");
 err_status = nc_open(namestring, 0, ncid_par);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }

 // From state.nc file 
 
 // getting Instances' ID from state.nc file
 err_status = nc_inq_dimid (*ncid_stt,"instances" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"instances\" id from ../state.nc\n");
     exit(0);
 }
 // # Instances  
 err_status = nc_inq_dimlen  (*ncid_stt, tmpID, nInstances);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"instances\" from ../state.nc\n");
    exit(0);
  }

  // getting mesh Elements size ID from state.nc file
 err_status = nc_inq_dimid (*ncid_stt,"meshElements" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"meshElements\" id from ../state.nc\n");
     exit(0);
 }
 // # mesh Elements  
 err_status = nc_inq_dimlen  (*ncid_stt, tmpID, nMeshEle);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"instances\" from ../state.nc\n");
    exit(0);
  }

   // getting channel Elements size ID from state.nc file
 err_status = nc_inq_dimid (*ncid_stt,"channelElements" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"channelElements\" id from ../state.nc\n");
     exit(0);
 }
 // # of channel elements  
 err_status = nc_inq_dimlen  (*ncid_stt, tmpID, nChannelEle);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"channelElements\" from ../state.nc\n");
    exit(0);
  }
  
  // Ids only:
 // from state.nc
 err_status = nc_inq_varid 	( *ncid_stt, "currentTime", currentTimeID);
 if (err_status != NC_NOERR)
 {
   printf("Problem getting the \"currentTime\" variable ID from ../state.nc\n");
   exit(0);
 }
  
 err_status = nc_inq_varid( *ncid_stt, "meshSurfacewaterDepth", meshSurfacewaterDepthID);
 if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"meshSurfacewaterDepth\" variable ID from ../state.nc\n");
    exit(0);
   }
 
 err_status = nc_inq_varid( *ncid_stt, "meshGroundwaterHead", meshGroundwaterHeadID);
 if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"meshSurfacewaterHead\" variable ID from ../state.nc\n");
    exit(0);
  }
  
  err_status = nc_inq_varid ( *ncid_stt, "meshPrecipitationCumulative", meshPrecipitaitonCumulativeID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"meshPrecipitationCumulative\" variable ID from ../state.nc\n");
     exit(0);
  } 
  
  err_status = nc_inq_varid ( *ncid_stt, "meshEvaporationCumulative", meshEvaporationCumulativeID);
  if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"evaporationCumulative\" variable ID from ../state.nc\n");
    exit(0);
  }
     
 err_status = nc_inq_varid ( *ncid_stt, "meshCanopyIce", canIceID);
 if (err_status != NC_NOERR)
  {
     printf("Problem getting the \"canIce\" variable ID from ../state.nc\n");
     exit(0);
   }
    
 err_status = nc_inq_varid ( *ncid_stt, "meshCanopyLiquid", canLiqID);
 if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"canLiq\" variable ID from ../state.nc\n");
     exit(0);
   }
     
  err_status = nc_inq_varid ( *ncid_stt, "meshSnowWaterEquivalent", snEqvID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"snEqv\" variable ID from ../state.nc\n");
     exit(0);
   }
   
 err_status  = nc_inq_varid ( *ncid_stt, "meshElementNeighborsSurfacewaterCumulativeFlow", EleNeighgSfcFlowID);
 if (err_status  != NC_NOERR)
 {
   printf("Problem getting the \"meshElementNeighborsSurfacewaterCumulativeFlow\" variable ID from ../geometry.nc\n");
   exit(0);
  }   

 err_status  = nc_inq_varid ( *ncid_stt, "meshElementNeighborsGroundwaterCumulativeFlow", EleNeighgGrdFlowID);
 if (err_status  != NC_NOERR)
  {
    printf("Problem getting the \"meshElementNeighborsGroundwaterCumulativeFlow\" variable ID from ../geometry.nc\n");
    exit(0);
  }   

  err_status = nc_inq_varid ( *ncid_stt, "channelSurfacewaterDepth", channelSurfacewaterDepthID);
  if (err_status != NC_NOERR)
   {
    printf("Problem getting the \"channelSurfacewaterDepth\" variable ID from ../state.nc\n");
    exit(0);
   }
     
 err_status = nc_inq_varid ( *ncid_stt, "channelEvaporationCumulative", channelEvaporationCumulativeID);
 if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"channelEvaporationCumulative\" variable ID from ../state.nc\n");
    exit(0);
  }
 
 err_status = nc_inq_varid ( *ncid_stt, "channelElementNeighborsSurfacewaterCumulativeFlow", ChanNeighgSfcFlowID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"channelElementNeighborsSurfacewaterCumulativeFlow\" variable ID from ../geometry.nc\n");
     exit(0);
   }
 
  // From geometry.nc file
  
  // getting meshElement neighbor size ID from geometry.nc file
 err_status = nc_inq_dimid (*ncid_geo,"meshMeshNeighbors" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"meshMeshNeighbors\" id from ../geometry.nc\n");
     exit(0);
 }
 // # meshElement beighbors  
 err_status = nc_inq_dimlen  (*ncid_geo, tmpID, NmeshMeshNeighbors);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"meshMeshNeighbors\" from ../geometry.nc\n");
    exit(0);
  }
  
  // getting channelChannel neighbor size ID from geometry.nc file
 err_status = nc_inq_dimid (*ncid_geo,"channelChannelNeighbors" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"channelChannelNeighbors\" id from ../geometry.nc\n");
     exit(0);
 }
 // # channelElement beighbors  
 err_status = nc_inq_dimlen  (*ncid_geo, tmpID, NchannelChannelNeighbors);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"channelChannelNeighbors\" from ../geometry.nc\n");
    exit(0);
  }
 // Ids only:
 // Mesh elements
  err_status  = nc_inq_varid 	( *ncid_geo, "meshMeshNeighbors", EleNeighID);
  if (err_status  != NC_NOERR)
   {
    printf("Problem getting the \"meshMeshNeighbors\" variable ID from ../geometry.nc\n");
    exit(0);
   }
  
  err_status = nc_inq_varid 	( *ncid_geo, "meshElementZBedrock", meshElementZBedrockID);
  if (err_status != NC_NOERR)
   {
      printf("Problem getting the \"meshElementZBedrock\" variable ID from ../geometry.nc\n");
      exit(0);
   }
 
  err_status = nc_inq_varid 	( *ncid_geo, "meshElementArea", meshElementAreaID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"meshElementArea\" variable ID from ../geometry.nc\n");
     exit(0);
   }
                 
  // Channel elements
  err_status = nc_inq_varid 	( *ncid_geo, "channelChannelNeighbors", ChanNeighID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"channelChannelNeighbors\" variable ID from ../geometry.nc\n");
     exit(0);
   }
   
  err_status = nc_inq_varid 	( *ncid_geo, "channelElementLength", channelElementLengthID);
  if (err_status != NC_NOERR)
   {
      printf("Problem getting the \"channelElementLength\" variable ID from ../geometry.nc\n");
      exit(0);
   }
             
  // from parameter.nc file:
  err_status = nc_inq_varid 	( *ncid_par, "meshPorosity", meshPorosityID);
  if (err_status != NC_NOERR)
   {
      printf("Problem getting the \"meshPorosity\" variable ID from ../parameter.nc\n");
      exit(0);
   }

  err_status = nc_inq_varid 	( *ncid_par, "channelSideSlope", channelElementSlopeID);
  
  if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"channelSideSlope\" variable ID from ../parameter.nc\n");
    exit(0);
  }

}

 void getElementAndChannelGeometry(int ncid_stt, int ncid_geo, size_t stateInstance, size_t nMeshEle, size_t nChannelEle, int meshElementZBedrockID, int meshElementAreaID, int channelElementLengthID, double* meshElementZBedrock,
                                  double* meshElementArea, double* channelElementLength )
 {
    int      err_status;                 // error variable
    int      geometryInstanceID;
    unsigned long long int geometryInstance;
    unsigned long long int geometryInstanceOld;
    double   tmp;
    size_t   indices[2];
    int nn;
    
    //tmp_indice[0] = stateInstance;
    
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
   
      indices[0] = geometryInstance;
        
      for (nn = 0; nn < nMeshEle; nn++)
       {
         indices[1] = nn;
         
         err_status = nc_get_var1_double ( ncid_geo,meshElementZBedrockID, indices, &tmp); 	
         if (err_status != NC_NOERR)
          {
            printf ("In getElementAndChannelGeometry:\n");
            printf("Problem getting the \"meshElementZBedrock\" variable from ../geometry.nc\n");
            exit(0);
          }           
         meshElementZBedrock[nn] = tmp;
         
        err_status = nc_get_var1_double ( ncid_geo,meshElementAreaID, indices, &tmp); 	
        if (err_status != NC_NOERR)
         {
           printf ("In getElementAndChannelGeometry:\n");
           printf("Problem getting the \"meshElementArea\" variable from ../geometry.nc\n");
           exit(0);
         }
         
         meshElementArea [nn] = tmp;      
     }
    
    for (nn = 0; nn < nChannelEle; nn++)
     {
        indices[1] = nn;
        err_status = nc_get_var1_double ( ncid_geo,channelElementLengthID, indices, &tmp); 	
        if (err_status != NC_NOERR)
          {
             printf ("In getElementAndChannelGeometry:\n");
             printf("Problem getting the \"channelElementLenght\" variable from ../geometry.nc\n");
             exit(0);
          }   
        channelElementLength[nn] = tmp;
        
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
       err_status = nc_get_var1_ulonglong ( ncid_stt,geometryInstanceID, &stateInstance, &geometryInstance); 	
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
            indices[0] = geometryInstance +1;
            
            for (nn = 0; nn < nMeshEle; nn++)
             {
                indices[1] = nn;
         
                err_status = nc_get_var1_double ( ncid_geo,meshElementZBedrockID, indices, &tmp); 	
                if (err_status != NC_NOERR)
                 {
                    printf ("In getElementAndChannelGeometry:\n");
                    printf("Problem getting the \"meshElementZBedrock\" variable from ../geometry.nc\n");
                    exit(0);
                 }           
            
                meshElementZBedrock[nn] = tmp;
         
                err_status = nc_get_var1_double ( ncid_geo,meshElementAreaID, indices, &tmp); 	
                if (err_status != NC_NOERR)
                 {
                   printf ("In getElementAndChannelGeometry:\n");
                   printf("Problem getting the \"meshElementArea\" variable from ../geometry.nc\n");
                   exit(0);
                 }
         
               meshElementArea [nn] = tmp;      
            }
    
          for (nn = 0; nn < nChannelEle; nn++)
           {
              indices[1] = nn;
              err_status = nc_get_var1_double ( ncid_geo,channelElementLengthID, indices, &tmp); 	
              if (err_status != NC_NOERR)
               {
                   printf ("In getElementAndChannelGeometry:\n");
                   printf("Problem getting the \"channelElementLenght\" variable from ../geometry.nc\n");
                   exit(0);
              }   
              channelElementLength[nn] = tmp;
        
           }
       }
       
       else
          return;
    }

   
 }
 
 void getElementAndChannelParameter( int ncid_stt, int ncid_par, size_t stateInstance, size_t nMeshEle, size_t nChannelEle, int meshPorosityID, int channelElementSlopeID, double* meshPorosity, double* channelElementSlope )
 {
    int      err_status;                 // error variable
    int      parameterInstanceID;
    unsigned long long int parameterInstance;
    unsigned long long int parameterInstanceOld;
    double   tmp;
    size_t   indices[2];
    int nn;
    
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
   
      indices[0] = parameterInstance;
        
      for (nn = 0; nn < nMeshEle; nn++)
       {
         indices[1] = nn;
         
         err_status = nc_get_var1_double ( ncid_par,meshPorosityID, indices, &tmp); 	
         if (err_status != NC_NOERR)
          {
            printf ("In getElementAndChannelParameter:\n");
            printf("Problem getting the \"meshPorosity\" variable from ../parameter.nc\n");
            exit(0);
          }           
         meshPorosity[nn] = tmp;
         
     }
    
    for (nn = 0; nn < nChannelEle; nn++)
     {
        indices[1] = nn;
        err_status = nc_get_var1_double ( ncid_par,channelElementSlopeID, indices, &tmp); 	
        if (err_status != NC_NOERR)
          {
             printf ("In getElementAndChannelParameter:\n");
             printf("Problem getting the \"channelElementLenght\" variable from ../parameter.nc\n");
             exit(0);
          }   
        channelElementSlope[nn] = tmp;
        
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
            indices[0] = parameterInstance +1;
            
            for (nn = 0; nn < nMeshEle; nn++)
             {
                indices[1] = nn;
         
                err_status = nc_get_var1_double ( ncid_par,meshPorosityID, indices, &tmp); 	
                if (err_status != NC_NOERR)
                 {
                    printf ("In getElementAndChannelGeometry:\n");
                    printf("Problem getting the \"meshElementZBedrock\" variable from ../parameter.nc\n");
                    exit(0);
                 }           
            
                meshPorosity[nn] = tmp;
            }
    
          for (nn = 0; nn < nChannelEle; nn++)
           {
              indices[1] = nn;
              err_status = nc_get_var1_double ( ncid_par,channelElementSlopeID, indices, &tmp); 	
              if (err_status != NC_NOERR)
               {
                   printf ("In getElementAndChannelGeometry:\n");
                   printf("Problem getting the \"channelElementLenght\" variable from ../parameter.nc\n");
                   exit(0);
              }   
              channelElementSlope[nn] = tmp;
        
           }
       }     
       else
          return;
    }

 }
 
 double meshEleWaterContent(int ncid_stt, int ncid_par, int ncid_geo, size_t indices[2], double meshElementZBedrock, double meshElementArea, double meshPorosity,
                            int meshSurfacewaterDepthID, int meshGroundwaterHeadID, int canIceID, int canLiqID, int snEqvID)
 {
   int    err_status;                 // error variable
   double canLiq;                     // Cummulative canopy liquid water equivalent in meters of water
   double canIce;                     // Cummulative canopy ice water equivalent in meters of water
   double snEqv;                      // Cummulative Snow water equivalent in meters of water
   double meshSurfacewaterDepth;      // Mesh Surface Water Depth in meters
   double meshGroundwaterHead;        // Mesh Ground Water Head in meters
                  
    
     err_status = nc_get_var1_double ( ncid_stt, meshSurfacewaterDepthID, indices, &meshSurfacewaterDepth); 	
     if (err_status != NC_NOERR)
       {
           printf ("In meshEleWaterContent:\n");
           printf("Problem getting the \"meshSurfacewaterDepth\" variable from ../state.nc\n");
           exit(0);
      }
            
     err_status = nc_get_var1_double ( ncid_stt,meshGroundwaterHeadID, indices, &meshGroundwaterHead); 	
     if (err_status != NC_NOERR)
       {
           printf ("In meshEleWaterContent:\n");
           printf("Problem getting the \"meshSurfacewaterHead\" variable from ../state.nc\n");
           exit(0);
       }
 	   
     err_status = nc_get_var1_double ( ncid_stt,canIceID, indices, &canIce); 	
     if (err_status != NC_NOERR)
       {
           printf ("In meshEleWaterContent:\n");
           printf("Problem getting the \"canIce\" variable from ../state.nc\n");
           exit(0);
       }
     
     err_status = nc_get_var1_double ( ncid_stt,canLiqID, indices, &canLiq); 	
     if (err_status != NC_NOERR)
       {
           printf ("In meshEleWaterContent:\n");
           printf("Problem getting the \"canLiq\" variable from ../state.nc\n");
           exit(0);
       }
      
     err_status = nc_get_var1_double ( ncid_stt,snEqvID, indices, &snEqv); 	
     if (err_status != NC_NOERR)
       {
           printf ("In meshEleWaterContent:\n");
           printf("Problem getting the \"snEqv\" variable from ../state.nc\n");
           exit(0);
       }

     meshSurfacewaterDepth *= meshElementArea;
     meshGroundwaterHead =  (meshGroundwaterHead - meshElementZBedrock)*meshPorosity*meshElementArea;
     canIce = canIce*meshElementArea;
     canLiq = canLiq*meshElementArea;
     snEqv = snEqv*meshElementArea;
     
     return (meshGroundwaterHead + meshSurfacewaterDepth + snEqv + canLiq + canIce );
     
 }
 
 double channelEleWaterContent (int ncid_stt, int ncid_geo, int ncid_par, size_t indices[2], double channelElementLength, double channelElementSlope, int channelSurfacewaterDepthID, int cummEvapoID, double* cumEvapo)
 {
   int    err_status;                    // error variable
   double channelArea;                   // Channel Area in Square Meters
   double channelSurfacewaterDepth;      // Channel Surface Water Depth in meters 
   
      
       err_status = nc_get_var1_double ( ncid_stt,channelSurfacewaterDepthID, indices, &channelSurfacewaterDepth); 	
       if (err_status != NC_NOERR)
         {
             printf ("In channelEleWaterContent:\n");
             printf("Problem getting the \"channelSurfacewaterDepth\" variable from ../state.nc\n");
             exit(0);
         }    
      
       channelArea = channelElementSlope*channelSurfacewaterDepth;
 	             
       channelSurfacewaterDepth = channelSurfacewaterDepth*channelArea*channelElementLength;
 	  
     err_status = nc_get_var1_double ( ncid_stt,cummEvapoID, indices, cumEvapo); 	
     if (err_status != NC_NOERR)
       {
           printf ("In channelEleWaterContent:\n");
           printf("Problem getting the \"channelEvaporationCumulative\" variable from ../state.nc\n");
           exit(0);
       }
     *cumEvapo = *cumEvapo*channelArea;

     return (channelSurfacewaterDepth);
 }
 
 double ElemBoundaryflow(int ncid_stt, int ncid_geo, size_t indices[3], double meshElementArea, int meshPrecipitaitonCumulativeID, int CummEvapoID, size_t nMeshEle, int NmeshMeshNeighbors,  
                         int EleNeighID, int EleNeighgSfcFlowID, int EleNeighgGrdFlowID)
 {
   int err;                                 // error variable
   int boundary;                            // counter
   int nNeighbor;                           // counter
   double PrcpCum;                          // cumulative precipitation in meters of water -> Inflow
   double CumEvapo;                         // cumulative evaporation in meters of water   -> Outflow
   double tmp_d;                            // temporary variable used to save the values from the funtions nc_get...
   double meshMeshNeighborsinFlow = 0;      // OutPut in cubic meters of water.
   double meshMeshNeighborsoutFlow = 0;     // OutPut in cubic meters of water.
   size_t tmp_indices[3];                   // Indices used to retrive parameters and geometry values as they don't change with time.
   
   tmp_indices[0] = 0;
   tmp_indices[1] = indices[1];
   
   
  err = nc_get_var1_double ( ncid_stt, meshPrecipitaitonCumulativeID, indices, &PrcpCum);
  if (err != NC_NOERR)
   {
      printf ("In ElemBoundaryflow:\n");
      printf("Problem getting the \"meshPrecipitationCumulative\" variable ID from ../state.nc\n");
      exit(0);
   }      
      
  PrcpCum = PrcpCum*meshElementArea; 
         
  err = nc_get_var1_double ( ncid_stt,CummEvapoID, indices, &CumEvapo); 	
  if (err != NC_NOERR)
   {
     printf ("In ElemBoundaryflow:\n");
     printf("Problem getting the \"evaporationCumulative\" variable from ../state.nc\n");
     exit(0);
  }
   
  CumEvapo = CumEvapo*meshElementArea;
 	           
 for (nNeighbor = 0; nNeighbor < NmeshMeshNeighbors; nNeighbor++)
  {  
        tmp_indices[2] = nNeighbor;
        
        err = nc_get_var1_int ( ncid_geo,EleNeighID, tmp_indices, &boundary); 	
        if (err != NC_NOERR)
         {
             printf ("In ElemBoundaryflow:\n");
             printf("Problem getting the \"meshMeshNeighbors\" variable from ../geometry.nc\n");
             exit(0);
         }
 	           
        if (boundary < 0)
         {
             err = nc_get_var1_double ( ncid_stt,EleNeighgSfcFlowID, indices, &tmp_d); 	
             if (err != NC_NOERR)
              {
                     printf("In ElemBoundaryflow: \n");
                     printf("Problem getting the \"channelSurfacewaterDepth\" variable from ../state.nc\n");
                     exit(0);
              } 
              
              if (tmp_d < 0)
                    meshMeshNeighborsinFlow += tmp_d;
              else
                    meshMeshNeighborsoutFlow+= tmp_d;  
              
              err = nc_get_var1_double ( ncid_stt,EleNeighgGrdFlowID, indices, &tmp_d); 	
              if (err != NC_NOERR)
               {
                     printf("In ElemBoundaryflow: \n");
                     printf("Problem getting the \"channelSurfacewaterDepth\" variable from ../state.nc\n");
                     exit(0);
                } 
             if (tmp_d < 0)
                    meshMeshNeighborsinFlow += tmp_d;
              else
                    meshMeshNeighborsoutFlow+= tmp_d;       
          }
   }
 // PrcpCum is inflow, therefore considered negative
 // Outflow is outflow, therefore considered positive
 // CumEvapo < 0 means water removed
 // PrcpCum > 0 means water added
 return  meshMeshNeighborsoutFlow - CumEvapo + meshMeshNeighborsinFlow - PrcpCum;
 }
 
double ChanBoundaryflow(int ncid_stt, int ncid_geo, size_t indices[3], size_t nChannelEle, int NchannelChannelNeighbors, int ChanNeighID, int ChanNeighgSfcFlowID, double cumEvapo)
 {
   int err;                                     // err variable
   int boundary;                                // counter
   int nNeighbor;                               // counter
   double tmp_d;                                // temporary variable used to save the values from the funtions nc_get...
   double channelChannelNeighborsFlow = 0;      // OutPut in cubic meters of water
   size_t tmp_indices[3];
   
   tmp_indices[0] = 0;
   tmp_indices[1] = indices[1];
   
  for (nNeighbor = 0; nNeighbor < NchannelChannelNeighbors; nNeighbor++)
    {  
       indices[2] = nNeighbor;
       tmp_indices[2] = indices[2];
       
       err = nc_get_var1_int ( ncid_geo, ChanNeighID, tmp_indices, &boundary); 	
       if (err != NC_NOERR)
          {
               printf("In ChanBoundaryflow: \n");
               printf("Problem getting the \"channelChannelNeighbors\" variable from ../geometry.nc\n");
               exit(0);
          }
 	         
       if (boundary < 0)
          {
               err = nc_get_var1_double ( ncid_stt,ChanNeighgSfcFlowID, indices, &tmp_d); 	
               if (err != NC_NOERR)
                  {
                      printf("In ChanBoundaryflow: \n");
                      printf("Problem getting the \"channelSurfacewaterDepth\" variable from ../state.nc\n");
                      exit(0);
                  } 
           
               channelChannelNeighborsFlow += tmp_d;                            
         }
   }
// Outflow is outflow, therefore considered positive
// CumEvapo < 0 means water removed
  return channelChannelNeighborsFlow - cumEvapo;
 }
 
 
