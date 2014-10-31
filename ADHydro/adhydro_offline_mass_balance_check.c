#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>
#include <string.h>
#include "adhydro_offinlie_mass_balence_check.h"

int main(int argc, char*argv[])
{
 char*  namestring;				                // string to save the file names and directories of the output
 int    err_status;				                // error variable
 int    ncid_stt,ncid_geo, ncid_par;                            // id of the state.nc, geometry.nc, and parameter.nc files	
 size_t nInstances,nMeshEle, nChannelEle;                  	// # of: instances, mesh elements, and channel elements in the *.nc files		

 // from state.nc
 double dt;					                // size of time step of simulation in the state.nc file
 double prcp;
 int meshGroundwaterHeadID;
 int meshSurfacewaterDepthID;
 int cummEvapoMeshID;
 int canIceID;
 int canLiqID;
 int snEqvID;
 int channelSurfacewaterDepthID; 
 int channelEvaporationCumulativeID;
  
 // from geometry.nc
 size_t NmeshMeshNeighbors;                                     // # of meshMesh neighbors
 size_t NchannelChannelNeighbors;                               // # of channelChannel neighbors
 int    EleNeighID;                                             // meshMeshNeighbors variable ID 
 int    EleNeighgSfcFlowID;                                     // meshNeighborsSurfacewaterCumulativeFlow variable ID 
 int    EleNeighgGrdFlowID;                                     // meshNeighborsGroundwaterCumulativeFlow varaible ID
 int    ChanvarID;                                              // channelChannelNeighbors
 int    ChanNeighgSfcFlowID;                                    // channelNeighborsSurfacewaterCumulativeFlow
 int    meshElementAreaID;
 int    meshElementZBedrockID;
 int    channelElementLengthID;
 int    channelElementSlopeID;
 
 // From parameter.nc
 int meshPorosityID;
 
 double meshMeshNeighborsFlow;
 double channelChannelNeighborsFlow;
 double meshWaterbalence;
 double channelWaterbalence;
 size_t indices[3];                                             // array size 3
                                                                // indices [0] -> refers to current instance index
                                                                // indices [1] -> refers to current element index
                                                                // indice  [2] -> refers to current neighbor index
 
 size_t nn, ii;					                // counters
 int    tmpID;					                // temporary netcdf ID

  if (argc != 2)
 {
	printf("Usage:\n");
	printf("./adhydro_creat_xmf_file <Directory path the .nc output files from adhydro >\n");
	exit(0); 
 }
 
 namestring = malloc( sizeof( char )*( 1+ strlen(argv[1]) + strlen("/parameter.nc") ) );
 
 // Openning the state.nc file
 sprintf(namestring, "%s%s",argv[1],"/state.nc");
 err_status = nc_open(namestring, 0, &ncid_stt);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 
 // Openning the geometry.nc file
 sprintf(namestring, "%s%s",argv[1],"/geometry.nc");
 err_status = nc_open(namestring, 0, &ncid_geo);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 
 // Openning the parameter.nc file
 sprintf(namestring, "%s%s",argv[1],"/parameter.nc");
 err_status = nc_open(namestring, 0, &ncid_par);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 
 free (namestring);

 // From state.nc file 
 
 // getting Instances' ID from state.nc file
 err_status = nc_inq_dimid (ncid_stt,"instances" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"instances\" id from ../state.nc\n");
     exit(0);
 }
 // # Instances  
 err_status = nc_inq_dimlen  (ncid_stt, tmpID, &nInstances);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"instances\" from ../state.nc\n");
    exit(0);
  }
  
  // getting mesh Elements size ID from state.nc file
 err_status = nc_inq_dimid (ncid_stt,"meshElements" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"meshElements\" id from ../state.nc\n");
     exit(0);
 }
 // # mesh Elements  
 err_status = nc_inq_dimlen  (ncid_stt, tmpID, &nMeshEle);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"instances\" from ../state.nc\n");
    exit(0);
  }
  
   // getting channel Elements size ID from state.nc file
 err_status = nc_inq_dimid (ncid_stt,"channelElements" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"meshElements\" id from ../state.nc\n");
     exit(0);
 }
 // # of channel elements  
 err_status = nc_inq_dimlen  (ncid_stt, tmpID, &nChannelEle);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"channelElements\" from ../state.nc\n");
    exit(0);
  }
  // Id's only:
  
 // from State.nc
 err_status  = nc_inq_varid 	( ncid_stt, "meshNeighborsSurfacewaterCumulativeFlow", &EleNeighgSfcFlowID);
 if (err_status  != NC_NOERR)
 {
   printf("Problem getting the \"meshNeighborsSurfacewaterCumulativeFlow\" variable ID from ../geometry.nc\n");
   exit(0);
  }   

 err_status  = nc_inq_varid 	( ncid_stt, "meshNeighborsGroundwaterCumulativeFlow", &EleNeighgGrdFlowID);
 if (err_status  != NC_NOERR)
  {
    printf("Problem getting the \"meshNeighborsGroundwaterCumulativeFlow\" variable ID from ../geometry.nc\n");
    exit(0);
  }   

  err_status = nc_inq_varid 	( ncid_stt, "channelNeighborsSurfacewaterCumulativeFlow", &ChanNeighgSfcFlowID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"channelNeighborsSurfacewaterCumulativeFlow\" variable ID from ../geometry.nc\n");
     exit(0);
   }
   
 err_status = nc_inq_varid 	( ncid_stt, "meshGroundwaterHead", &meshGroundwaterHeadID);
 if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"meshSurfacewaterHead\" variable ID from ../state.nc\n");
    exit(0);
  }
 	
 err_status = nc_inq_varid 	( ncid_stt, "meshSurfacewaterDepth", &meshSurfacewaterDepthID);
 if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"meshSurfacewaterDepth\" variable ID from ../state.nc\n");
    exit(0);
   }
     
  err_status = nc_inq_varid 	( ncid_stt, "meshEvaporationCumulative", &cummEvapoMeshID);
  if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"evaporationCumulative\" variable ID from ../state.nc\n");
    exit(0);
  }
     
 err_status = nc_inq_varid 	( ncid_stt, "canIce", &canIceID);
 if (err_status != NC_NOERR)
  {
     printf("Problem getting the \"canIce\" variable ID from ../state.nc\n");
     exit(0);
   }
    
 err_status = nc_inq_varid 	( ncid_stt, "canLiq", &canLiqID);
 if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"canLiq\" variable ID from ../state.nc\n");
     exit(0);
   }
     
  err_status = nc_inq_varid 	( ncid_stt, "snEqv", &snEqvID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"snEqv\" variable ID from ../state.nc\n");
     exit(0);
   }
      
  err_status = nc_inq_varid 	( ncid_stt, "channelSurfacewaterDepth", &channelSurfacewaterDepthID);
  if (err_status != NC_NOERR)
   {
    printf("Problem getting the \"channelSurfacewaterDepth\" variable ID from ../state.nc\n");
    exit(0);
   }
     
 err_status = nc_inq_varid 	( ncid_stt, "channelEvaporationCumulative", &channelEvaporationCumulativeID);
 if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"channelEvaporationCumulative\" variable ID from ../state.nc\n");
    exit(0);
  }
        
  // From geometry.nc file
  
  // getting meshElement neighbor size ID from geometry.nc file
 err_status = nc_inq_dimid (ncid_geo,"meshMeshNeighbors" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"meshMeshNeighbors\" id from ../geometry.nc\n");
     exit(0);
 }
 // # meshElement beighbors  
 err_status = nc_inq_dimlen  (ncid_geo, tmpID, &NmeshMeshNeighbors);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"meshMeshNeighbors\" from ../geometry.nc\n");
    exit(0);
  }
  
  // getting channelChannel neighbor size ID from geometry.nc file
 err_status = nc_inq_dimid (ncid_geo,"channelChannelNeighbors" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"channelChannelNeighbors\" id from ../geometry.nc\n");
     exit(0);
 }
 // # channelElement beighbors  
 err_status = nc_inq_dimlen  (ncid_geo, tmpID, &NchannelChannelNeighbors);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"channelChannelNeighbors\" from ../geometry.nc\n");
    exit(0);
  }
 // Id's only:
 // Mesh elements
  err_status  = nc_inq_varid 	( ncid_geo, "meshMeshNeighbors", &EleNeighID);
  if (err_status  != NC_NOERR)
   {
    printf("Problem getting the \"meshMeshNeighbors\" variable ID from ../geometry.nc\n");
    exit(0);
   }
  
  err_status = nc_inq_varid 	( ncid_geo, "meshElementZBedrock", &meshElementZBedrockID);
  if (err_status != NC_NOERR)
   {
      printf("Problem getting the \"meshElementZBedrock\" variable ID from ../geometry.nc\n");
      exit(0);
   }
 
  err_status = nc_inq_varid 	( ncid_geo, "meshElementArea", &meshElementAreaID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"meshElementArea\" variable ID from ../geometry.nc\n");
     exit(0);
   }            
  // Channel elements
  err_status = nc_inq_varid 	( ncid_geo, "channelChannelNeighbors", &ChanvarID);
  if (err_status != NC_NOERR)
   {
     printf("Problem getting the \"channelChannelNeighbors\" variable ID from ../geometry.nc\n");
     exit(0);
   }
   
  err_status = nc_inq_varid 	( ncid_geo, "channelElementLength", &channelElementLengthID);
  if (err_status != NC_NOERR)
   {
      printf("Problem getting the \"channelElementLength\" variable ID from ../geometry.nc\n");
      exit(0);
   }
              
 err_status = nc_inq_varid 	( ncid_geo, "channelElementSlope", &channelElementSlopeID);
 if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"channelElementSlope\" variable ID from ../geometry.nc\n");
    exit(0);
  }
 err_status = nc_inq_varid 	( ncid_geo, "channelSurfacewaterDepth", &channelSurfacewaterDepthID);
 if (err_status != NC_NOERR)
  {
    printf("Problem getting the \"channelSurfacewaterDepth\" variable ID from ../geometry.nc\n");
    exit(0);
  }
  // from parameter.nc file:
    
  err_status = nc_inq_varid 	( ncid_par, "meshPorosity", &meshPorosityID);
  if (err_status != NC_NOERR)
   {
      printf("Problem getting the \"meshPorosity\" variable ID from ../parameter.nc\n");
      exit(0);
   }
   
    
 // Loop over timesteps
 for (nn = 0; nn < nInstances; nn++)
  {
  
       indices[0] = nn;
       channelWaterbalence = 0;
       meshWaterbalence = 0;
       
       err_status = nc_inq_varid 	( ncid_stt, "dt", &tmpID);
       if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"currentTime\" variable ID from ../state.nc\n");
      	    exit(0);
 	}
  
       err_status = nc_get_var1_double (ncid_stt,tmpID, &nn , &dt);
       if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"currentTime\" variable from ../state.nc\n");
      	    exit(0);
 	}
          
       err_status = nc_inq_varid 	( ncid_stt, "precipitation", &tmpID);
       if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"precipitation\" variable ID from ../state.nc\n");
      	    exit(0);
 	}
       
       err_status = nc_get_var1_double (ncid_stt,tmpID, &nn , &prcp);
       if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"precipitation\" variable from ../state.nc\n");
      	    exit(0);
 	}    
       
       
       for (ii = 0; ii < nMeshEle; ii++)  // Loop over elements
         {
           meshWaterbalence = 0;
           indices[1] = ii;
           meshMeshNeighborsFlow = ElemBoundaryflow(ncid_stt, ncid_geo, indices, EleNeighID, nMeshEle, NmeshMeshNeighbors, EleNeighgSfcFlowID, EleNeighgGrdFlowID );
           meshWaterbalence = meshEleWaterContent(ncid_stt, ncid_par, ncid_geo, meshElementZBedrockID, meshElementAreaID, meshPorosityID, 
                              meshSurfacewaterDepthID, meshGroundwaterHeadID, cummEvapoMeshID, canIceID, canLiqID, snEqvID, indices,  prcp);
           meshWaterbalence +=  meshMeshNeighborsFlow;

 	 }  // End of Loop over elements
      
       for (ii = 0; ii < nChannelEle; ii++)  // Loop over channels
         {   
 	     indices[1] = ii;
 	     channelWaterbalence = 0; 
 	     channelChannelNeighborsFlow = ChanBoundaryflow(ncid_stt, ncid_geo, indices, nChannelEle, NchannelChannelNeighbors, ChanvarID, ChanNeighgSfcFlowID);     
             channelWaterbalence = channelEleWaterContent (ncid_stt, ncid_geo, channelElementLengthID, channelElementSlopeID, channelSurfacewaterDepthID, channelEvaporationCumulativeID, indices, prcp);
             channelWaterbalence +=  channelChannelNeighborsFlow ;

        }   // End of Loop over channel elements     
 	
 	// balance =  meshWaterbalence + channelWaterbalence;
 	
  }  // end of Loop over timesteps
  err_status = nc_close(ncid_stt);
  err_status = nc_close(ncid_geo);
  err_status = nc_close(ncid_par);
 return 0; 
}
 
 double ElemBoundaryflow(int ncid_stt, int ncid_geo, size_t indices[3], size_t nMeshEle, int NmeshMeshNeighbors,  int EleNeighID, int EleNeighgSfcFlowID, int EleNeighgGrdFlowID)
 {
   int err;                                 // error variable
   int boundary;                            // counter
   int nNeighbor;                           // counter
   double tmp_d;                            // temporary variable used to save the values from the funtions nc_get...
   double meshMeshNeighborsFlow = 0;        // OutPut in meters of water.
           
 for (nNeighbor = 0; nNeighbor < NmeshMeshNeighbors; nNeighbor++)
  {  
        indices[2] = nNeighbor;
        err = nc_get_var1_int ( ncid_geo,EleNeighID, indices, &boundary); 	
        if (err != NC_NOERR)
         {
             printf("In ElemBoundaryflow: \n");
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
          
              meshMeshNeighborsFlow += tmp_d;
                      
              err = nc_get_var1_double ( ncid_stt,EleNeighgGrdFlowID, indices, &tmp_d); 	
              if (err != NC_NOERR)
               {
                     printf("In ElemBoundaryflow: \n");
                     printf("Problem getting the \"channelSurfacewaterDepth\" variable from ../state.nc\n");
                     exit(0);
                } 
              meshMeshNeighborsFlow += tmp_d;                      
          }
   }
   
 return meshMeshNeighborsFlow;
 }
 
double ChanBoundaryflow(int ncid_stt, int ncid_geo, size_t indices[3], size_t nChannelEle, int NchannelChannelNeighbors, int ChanvarID, int ChanNeighgSfcFlowID)
 {
   int err;                                     // err variable
   int boundary;                                // counter
   int nNeighbor;                               // counter
   double tmp_d;                                // temporary variable used to save the values from the funtions nc_get...
   double channelChannelNeighborsFlow = 0;      // OutPut in Meters of water
        
  for (nNeighbor = 0; nNeighbor < NchannelChannelNeighbors; nNeighbor++)
    {  
       indices[2] = nNeighbor;
       err = nc_get_var1_int ( ncid_geo,ChanvarID, indices, &boundary); 	
       if (err != NC_NOERR)
          {
               printf("In ChanBoundaryflow: \n");
               printf("Problem getting the \"meshMeshNeighbors\" variable from ../geometry.nc\n");
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
        
  return channelChannelNeighborsFlow;
 }
 
 double meshEleWaterContent(int ncid_stt, int ncid_par, int ncid_geo, int meshElementZBedrockID, int meshElementAreaID, int meshPorosityID, 
                            int meshSurfacewaterDepthID, int meshGroundwaterHeadID, int CummEvapoID, int canIceID, int canLiqID, int snEqvID, 
                            size_t indices[3], double precip)
 {
   int    err_status;                 // error variable
   double meshElementZBedrock;        // mesh Element Z Bedrock in meters
   double meshPorosity;               // mesh Porosity
   double meshElementArea;            // mesh element area in Square meters.  
   double canLiq;                     // Cummulative canopy liquid water equivalent in meters of water
   double canIce;                     // Cummulative canopy ice water equivalent in meters of water
   double CummEvapo;                  // Cummulative evaporation from the mesh in meters of water
   double snEqv;                      // Cummulative Snow water equivalent in meters of water
   double meshSurfacewaterDepth;      // Mesh Surface Water Depth in meters
   double meshGroundwaterHead;        // Mesh Ground Water Head in meters
    
    err_status = nc_get_var1_double ( ncid_geo,meshElementZBedrockID, indices, &meshElementZBedrock); 	
    if (err_status != NC_NOERR)
      {
          printf("Problem getting the \"meshElementZBedrock\" variable from ../geometry.nc\n");
          exit(0);
      }           
        
         
     err_status = nc_get_var1_double ( ncid_geo,meshElementAreaID, indices, &meshElementArea); 	
     if (err_status != NC_NOERR)
       {
          printf("Problem getting the \"meshElementArea\" variable from ../geometry.nc\n");
          exit(0);
       } 
     
     err_status = nc_get_var1_double ( ncid_par,meshPorosityID, indices, &meshPorosity); 	
     if (err_status != NC_NOERR)
      {
          printf("Problem getting the \"meshPorosity\" variable from ../parameter.nc\n");
          exit(0);
     }
          
                     
     err_status = nc_get_var1_double ( ncid_stt,meshSurfacewaterDepthID, indices, &meshSurfacewaterDepth); 	
     if (err_status != NC_NOERR)
       {
           printf("Problem getting the \"meshSurfacewaterDepth\" variable from ../state.nc\n");
           exit(0);
      }
            
     meshSurfacewaterDepth *= meshElementArea;
 	  
     err_status = nc_get_var1_double ( ncid_stt,meshGroundwaterHeadID, indices, &meshGroundwaterHead); 	
     if (err_status != NC_NOERR)
       {
           printf("Problem getting the \"meshSurfacewaterHead\" variable from ../state.nc\n");
           exit(0);
       }
 	   
     meshGroundwaterHead =  (meshGroundwaterHead - meshElementZBedrock)* (meshPorosity*meshElementArea);
     
     precip = precip*meshElementArea;
     
     // ET per element
     
         
     err_status = nc_get_var1_double ( ncid_stt,CummEvapoID, indices, &CummEvapo); 	
     if (err_status != NC_NOERR)
       {
           printf("Problem getting the \"evaporationCumulative\" variable from ../state.nc\n");
           exit(0);
       }
     CummEvapo = CummEvapo*meshElementArea;
     
      
 	  
     err_status = nc_get_var1_double ( ncid_stt,canIceID, indices, &canIce); 	
     if (err_status != NC_NOERR)
       {
           printf("Problem getting the \"canIce\" variable from ../state.nc\n");
           exit(0);
       }
     canIce = canIce*meshElementArea;
     
 	  
     err_status = nc_get_var1_double ( ncid_stt,canLiqID, indices, &canLiq); 	
     if (err_status != NC_NOERR)
       {
           printf("Problem getting the \"canLiq\" variable from ../state.nc\n");
           exit(0);
       }
     canLiq = canLiq*meshElementArea;
     
      
     err_status = nc_get_var1_double ( ncid_stt,snEqvID, indices, &snEqv); 	
     if (err_status != NC_NOERR)
       {
           printf("Problem getting the \"snEqv\" variable from ../state.nc\n");
           exit(0);
       }
     snEqv = snEqv*meshElementArea;
     
     return (meshGroundwaterHead + meshSurfacewaterDepth + precip + snEqv - canLiq - canIce - CummEvapo);
     
 }
 
 double channelEleWaterContent (int ncid_stt, int ncid_geo, int channelElementLengthID, int channelElementSlopeID, int channelSurfacewaterDepthID, int cummEvapoID, size_t* indices, double precip)
 {
   int    err_status;                    // error variable
   double channelElementLength;          // channel Element Length in meters
   double channelElementSlope;           // Channel Element Slope
   double channelArea;                   // Channel Area in Square Meters
   double channelSurfacewaterDepth;      // Channel Surface Water Depth in meters 
   double cummEvapo;                     // Cummulative evaporation in meters of water
   
      err_status = nc_get_var1_double ( ncid_geo,channelElementLengthID, indices, &channelElementLength); 	
      if (err_status != NC_NOERR)
        {
             printf("Problem getting the \"channelElementLenght\" variable from ../geometry.nc\n");
             exit(0);
        }     
                
       err_status = nc_get_var1_double ( ncid_geo,channelElementSlopeID, indices, &channelElementSlope); 	
       if (err_status != NC_NOERR)
         {
             printf("Problem getting the \"channelElementSlope\" variable from ../geometry.nc\n");
             exit(0);
         } 
   
       err_status = nc_get_var1_double ( ncid_stt,channelSurfacewaterDepthID, indices, &channelSurfacewaterDepth); 	
       if (err_status != NC_NOERR)
         {
             printf("Problem getting the \"channelSurfacewaterDepth\" variable from ../state.nc\n");
             exit(0);
         }    
 	             
       channelArea = channelElementSlope*channelSurfacewaterDepth;
 	             
       channelSurfacewaterDepth = channelSurfacewaterDepth*channelArea*channelElementLength;
       
       precip *= channelArea;
      
 	  
     err_status = nc_get_var1_double ( ncid_stt,cummEvapoID, indices, &cummEvapo); 	
     if (err_status != NC_NOERR)
       {
           printf("Problem getting the \"channelEvaporationCumulative\" variable from ../state.nc\n");
           exit(0);
       }
     cummEvapo = cummEvapo*channelArea;
     
     return (channelSurfacewaterDepth + precip - cummEvapo );
 }
 
 
