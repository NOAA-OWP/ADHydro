#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>
#include <string.h>

// ADHydro outputs NetCDF files: geometry.nc, parameter.nc, state.nc, and
// display.nc.  In order to read these files in paraview, it is necessary to
// create Xdmf files.  The purpose of this program is to generate the Xdmf
// files.  The Xdmf files contain metadata that is redundant with the data in
// the NetCDF files so they are created from the NetCDF files.

int main(int argc, char*argv[])
{
 char *namestring;				// string to save the file names and directories of the output
 int err_status;				// error variable
 size_t nn;					// counter
 int ncid_geo, ncid_par, ncid_stt, ncid_disp;	// id of the *.nc files	
 int TmpvarID;                                  // Variable Id. Used for more than one variable, therefore temporary		
 size_t numInstances_geo, numInstances_par;	// # of instances in the *.nc files
 size_t numInstances_stt, numInstances_disp;
 double time;					// time of simulation in the state.nc file
 unsigned long long int Inst_geo, Inst_par;	// group # for geometry.nc and parameter.nc files -> read from state.nc file
 int tmpID;					// temporary net cdf ID
 size_t nMeshNgb;                               // # of Mesh Neighbors
 size_t nChannelVrtArr;                         // size of Channel Element Vertices Array
 size_t nSnowLayers;                            // size of dimension evapoTranspirationSnowLayers
 size_t nAllLayers;                             // size of dimension evapoTranspirationAllLayers
 int nMeshEle;			                // # of Mesh elements
 int nChannelEle;			        // # of Channel elements
 int nMeshNode;				        // # of mesh nodes
 int nChannelNode;				// # of channel nodes
 size_t tmp;                                    // used to avoid warnings
 FILE* meshStateOut;				// file output for mesh elements from state.nc with Xdmf format
 FILE* channelStateOut;				// file output for Channel elements from state.nc with Xdmf format
 FILE* meshDisplayOut;                          // file output for mesh elements from display.nc with Xdmf format
 FILE* channelDisplayOut;                       // file output for Channel elements from display.nc with Xdmf format
 if (argc != 2)
 {
	printf("Usage:\n");
	printf("./adhydro_create_xmf_file <One directory path for both .nc input and .xmf output files>\n");
	exit(0); 
 }
 
 namestring = malloc( sizeof( char )*( 1+ strlen(argv[1]) + strlen("/channels_display.xmf") ) );
 
 // Opening the geometry.nc file
 sprintf(namestring, "%s%s",argv[1],"/geometry.nc");
 err_status = nc_open(namestring, 0, &ncid_geo);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 // Opening the parameter.nc file
 sprintf(namestring, "%s%s",argv[1],"/parameter.nc");
 err_status = nc_open(namestring, 0, &ncid_par);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 // Opening the state.nc file
 sprintf(namestring, "%s%s",argv[1],"/state.nc");
 err_status = nc_open(namestring, 0, &ncid_stt);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 // Opening the display.nc file
 sprintf(namestring, "%s%s",argv[1],"/display.nc");
 err_status = nc_open(namestring, 0, &ncid_disp);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 
 // getting Instances' ID from geometry.nc file
 err_status = nc_inq_dimid (ncid_geo,"instances" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"instances\" id from ../geometry.nc\n");
     exit(0);
 }
 // # Instances  
 err_status = nc_inq_dimlen  (ncid_geo, tmpID, &numInstances_geo);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"instances\" from ../geometry.nc\n");
    exit(0);
  }
  // getting Instances' ID from parameter.nc file
 err_status = nc_inq_dimid (ncid_par,"instances" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"instances\" id from ../parameter.nc\n");
     exit(0);
 }
 // # Instances  
 err_status = nc_inq_dimlen  (ncid_par, tmpID, &numInstances_par);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"instances\" from ../parameter.nc\n");
    exit(0);
  }
 // getting Instances' ID from state.nc file
 err_status = nc_inq_dimid (ncid_stt,"instances" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"instances\" id from ../state.nc\n");
     exit(0);
 }
 // # Instances  
 err_status = nc_inq_dimlen  (ncid_stt, tmpID, &numInstances_stt);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"instances\" from ../state.nc\n");
    exit(0);
  }
 // getting Instances' ID from display.nc file
 err_status = nc_inq_dimid (ncid_disp,"instances" , &tmpID);
 if (err_status != NC_NOERR)
 {
     printf("Problem getting \"instances\" id from ../state.nc\n");
     exit(0);
 }
 // # Instances  
 err_status = nc_inq_dimlen  (ncid_disp, tmpID, &numInstances_disp);
 if (err_status != NC_NOERR)
 {
    printf("Problem getting # \"instances\" from ../state.nc\n");
    exit(0);
  }

 sprintf(namestring, "%s%s",argv[1],"/mesh_state.xmf");
 meshStateOut = fopen (namestring, "w");
 sprintf(namestring, "%s%s",argv[1],"/channels_state.xmf");
 channelStateOut = fopen (namestring, "w");
 sprintf(namestring, "%s%s",argv[1],"/mesh_display.xmf");
 meshDisplayOut = fopen (namestring, "w");
 sprintf(namestring, "%s%s",argv[1],"/channels_display.xmf");
 channelDisplayOut = fopen (namestring, "w");

 free(namestring);
 // Header for mesh state file 
fprintf(meshStateOut,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(meshStateOut,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(meshStateOut,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(meshStateOut,"  <Domain>\n");
fprintf(meshStateOut,"    <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"MeshState\">\n"); 
  // Header for channel state file 
fprintf(channelStateOut,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(channelStateOut,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(channelStateOut,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(channelStateOut,"  <Domain>\n");
fprintf(channelStateOut,"    <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"ChannelsState\">\n"); 
// Header for mesh display file 
fprintf(meshDisplayOut,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(meshDisplayOut,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(meshDisplayOut,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(meshDisplayOut,"  <Domain>\n");
fprintf(meshDisplayOut,"    <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"MeshDisplay\">\n"); 
 // Header for channel display file 
fprintf(channelDisplayOut,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(channelDisplayOut,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(channelDisplayOut,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(channelDisplayOut,"  <Domain>\n");
fprintf(channelDisplayOut,"    <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"ChannelsDisplay\">\n"); 

 for (nn = 0; nn < numInstances_stt; nn++)
 {
 	// time step
 	err_status = nc_inq_varid 	( ncid_stt, "currentTime", &TmpvarID);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"currentTime\" variable ID from ../state.nc\n");
      	    exit(0);
 	}

 	err_status = nc_get_var1_double (ncid_stt,TmpvarID, &nn , &time	);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"currentTime\" variable from ../state.nc\n");
      	    exit(0);
 	}	
 	// geometry instance
 	err_status = nc_inq_varid 	( ncid_stt, "geometryInstance", &TmpvarID);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"geometryInstance\" variable ID from ../state.nc\n");
      	    exit(0);
 	}

 	err_status = nc_get_var1_ulonglong (ncid_stt,TmpvarID, &nn , &Inst_geo);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"geometryInstance\" variable from ../state.nc\n");
      	    exit(0);
 	}
 	// parameter instance
 	err_status = nc_inq_varid 	( ncid_stt, "parameterInstance", &TmpvarID);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"parameterInstance\" variable ID from ../state.nc\n");
      	    exit(0);
 	}

 	err_status = nc_get_var1_ulonglong (ncid_stt,TmpvarID, &nn , &Inst_par);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"parameterInstance\" variable from ../state.nc\n");
      	    exit(0);
 	}
 	
 	// numberOfMeshElements ID  
 	err_status = nc_inq_varid ( ncid_geo, "numberOfMeshElements", &TmpvarID);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"numberOfMeshElements\" variable ID from ../geometry.nc\n");
      	    exit(0);
 	}
 	tmp = Inst_geo;
 	err_status = nc_get_var1_int (ncid_geo,TmpvarID, &tmp , &nMeshEle);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"numberOfMeshElements\" variable ID from ../geometry.nc\n");
      	    exit(0);
 	}
 	// numberOfChannelElements ID  
 	err_status = nc_inq_varid ( ncid_geo, "numberOfChannelElements", &TmpvarID);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"numberOfChannelElements\" variable ID from ../geometry.nc\n");
      	    exit(0);
 	}
 	err_status = nc_get_var1_int (ncid_geo,TmpvarID, &tmp , &nChannelEle);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"numberOfChannelElements\" variable ID from ../geometry.nc\n");
      	    exit(0);
 	}		
 	// number Of Mesh nodes ID  
 	err_status = nc_inq_varid ( ncid_geo, "numberOfMeshNodes", &TmpvarID);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"numberOfMeshNodes\" variable ID from ../geometry.nc\n");
      	    exit(0);
 	}
 	err_status = nc_get_var1_int (ncid_geo, TmpvarID, &tmp , &nMeshNode);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"numberOfMeshNodes\" variable ID from ../geometry.nc\n");
      	    exit(0);
 	}	
        // number Of Channel nodes ID  
        err_status = nc_inq_varid ( ncid_geo, "numberOfChannelNodes", &TmpvarID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfChannelNodes\" variable ID from ../geometry.nc\n");
            exit(0);
        }
        err_status = nc_get_var1_int (ncid_geo, TmpvarID, &tmp , &nChannelNode);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfChannelNodes\" variable ID from ../geometry.nc\n");
            exit(0);
        }       
 	
 	// meshMeshNeighborsSize dimension ID  
 	err_status = nc_inq_dimid (ncid_geo,"meshMeshNeighborsSize" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting meshMeshNeighborsSize dimesion id from ../geometry.nc\n");
   	    exit(0);
 	}
 	// # meshMeshNeighborsSize
 	err_status = nc_inq_dimlen  (ncid_geo, tmpID, &nMeshNgb);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting dimmesion from ../geometry.nc\n");
      	    exit(0);
 	}
 	// channelElementVerticesSize dimension ID  
 	err_status = nc_inq_dimid (ncid_geo,"channelElementVerticesSize" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting channelElementVerticesSize dimmesion id from ../geometry.nc\n");
   	    exit(0);
 	}
 	// channelElementVerticesSize varaible
 	err_status = nc_inq_dimlen  (ncid_geo, tmpID, &nChannelVrtArr);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting channelElementVerticesSize dimmesion from ../geometry.nc\n");
      	    exit(0);
 	}	
 	
 	// dimension evapoTranspirationSnowLayers ID  
 	err_status = nc_inq_dimid ( ncid_stt, "evapoTranspirationSnowLayers", &tmpID);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"evapoTranspirationSnowLayers\" variable ID from ../state.nc\n");
      	    exit(0);
 	}
 	// dimension size evapoTranspirationSnowLayers 
 	err_status = nc_inq_dimlen  (ncid_stt, tmpID, &nSnowLayers);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"evapoTranspirationSnowLayers\" variable ID from ../state.nc\n");
      	    exit(0);
 	}	
     	
     	// dimension evapoTranspirationAllLayers ID
 	err_status = nc_inq_dimid ( ncid_stt, "evapoTranspirationAllLayers", &tmpID);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"evapoTranspirationAllLayers\" variable ID from ../state.nc\n");
      	    exit(0);
 	}
 	// dimension size evapoTranspirationAllLayers 
 	err_status = nc_inq_dimlen  (ncid_stt, tmpID, &nAllLayers);
 	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting the \"evapoTranspirationAllLayers\" variable ID from ../state.nc\n");
      	    exit(0);
 	}
     	
// for all groups: Mesh file
fprintf(meshStateOut,"      <Grid GridType=\"Uniform\">\n");
fprintf(meshStateOut,"        <Time Value=\"%f\"/>\n",time);
fprintf(meshStateOut,"        <Topology NumberOfElements=\"%d\" Type=\"Triangle\">\n",nMeshEle); 
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nMeshEle,nMeshNgb);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 3\" Format=\"XML\">%llu 0 0 1 1 1 1 %d %zu</DataItem>\n",Inst_geo,nMeshEle, nMeshNgb);
fprintf(meshStateOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d %zu\" Format=\"HDF\">geometry.nc:/meshElementVertices</DataItem>\n",numInstances_geo,nMeshEle, nMeshNgb);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Topology>\n");
fprintf(meshStateOut,"        <Geometry Type=\"X_Y_Z\">\n");
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeX</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeY</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeZSurface</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Geometry>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementX\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementX</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");                
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementY\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementY</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementZSurface\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementZSurface</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementZBedrock\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementZBedrock</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");      
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementArea\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementArea</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");     
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementSlopeX\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementSlopeX</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");     
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementSlopeY\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementSlopeY</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");     
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshCatchment\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\">parameter.nc:/meshCatchment</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");     
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshConductivity\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshConductivity</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshPorosity\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshPorosity</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshManningsN\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshManningsN</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterDepth\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshSurfacewaterDepth</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterError\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshSurfacewaterError</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterHead\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshGroundwaterHead</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterError\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshGroundwaterError</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshPrecipitation\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshPrecipitation</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshPrecipitationCumulative\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshPrecipitationCumulative</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshEvaporation\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshEvaporation</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshEvaporationCumulative\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshEvaporationCumulative</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshTranspiration\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshTranspiration</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshTranspirationCumulative\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshTranspirationCumulative</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterInfiltration\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshSurfacewaterInfiltration</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterRecharge\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshGroundwaterRecharge</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
/* FIXME -> meshFIceOld(instances, meshElements, evapoTranspirationSnowLayers)  Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(meshStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"meshFIceOld\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nMeshEle, nSnowLayers);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2 \" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshFIceOld</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshAlbOld\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshAlbOld</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSnEqvO\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshSnEqvO</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
/* FIXME -> meshStc(instances, meshElements, evapoTranspirationAllLayers)  Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(meshStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"meshStc\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nMeshEle, nSnowLayers);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshStc</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshTah\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshTah</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshEah\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshEah</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshFWet\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshFWet</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshCanLiq\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshCanLiq</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshCanIce\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshCanIce</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshTv\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshTv</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshTg\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshTg</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshISnow\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\" >state.nc:/meshISnow</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
/*  FIXME -> meshZSnso(instances, meshElements, evapoTranspirationAllLayers) Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(meshStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"meshZSnso\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nMeshEle, nAllLayers);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshZSnso</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSnowH\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshSnowH</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSnEqv\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshSnEqv</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
/* FIXME -> meshSnIce(instances, meshElements, evapoTranspirationSnowLayers)  Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(meshStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"meshSnIce\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nMeshEle, nSnowLayers);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshSnIce</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
/* FIXME -> meshSnLiq(instances, meshElements, evapoTranspirationSnowLayers)  Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(meshStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"meshSnLiq\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nMeshEle, nSnowLayers);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshSnLiq</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshLfMass\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshLfMass</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshRtMass\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshRtMass</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshStMass\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshStMass</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshWood\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshWood</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshStblCp\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshStblCp</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshFastCp\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshFastCp</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshLai\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshLai</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSai\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshSai</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshCm\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshCm</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshCh\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshCh</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshTauss\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshTauss</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshDeepRech\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshDeepRech</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshRech\">\n");        
fprintf(meshStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/meshRech</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshStateOut,"          </DataItem>\n");
fprintf(meshStateOut,"        </Attribute>\n");
fprintf(meshStateOut,"      </Grid>\n");    

// for all groups: Channel file
fprintf(channelStateOut,"      <Grid GridType=\"Uniform\">\n");
fprintf(channelStateOut,"        <Time Value=\"%f\"/>\n",time);
fprintf(channelStateOut,"        <Topology NumberOfElements=\"%d\" Type=\"Mixed\">\n",nChannelEle);
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nChannelEle,nChannelVrtArr);
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 3\" Format=\"XML\">%llu 0 0 1 1 1 1 %d %zu</DataItem>\n",Inst_geo,nChannelEle, nChannelVrtArr);   
fprintf(channelStateOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d %zu\" Format=\"HDF\">geometry.nc:/channelElementVertices</DataItem>\n",numInstances_geo,nChannelEle, nChannelVrtArr);   
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Topology>\n");
fprintf(channelStateOut,"        <Geometry Type=\"X_Y_Z\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelNode);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelNode);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeX</DataItem>\n",numInstances_geo,nChannelNode);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelNode);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelNode);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeY</DataItem>\n",numInstances_geo,nChannelNode);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelNode);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelNode);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeZBank</DataItem>\n",numInstances_geo,nChannelNode);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Geometry>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementX\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementX</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementY\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementY</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementZBank\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementZBank</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementZBed\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementZBed</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementLength\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementLength</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelChannelType\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\">parameter.nc:/channelChannelType</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelReachCode\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelReachCode</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBaseWidth\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBaseWidth</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSideSlope\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelSideSlope</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBedConductivity\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBedConductivity</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBedThickness\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBedThickness</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelManningsN\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelManningsN</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSurfacewaterDepth\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/channelSurfacewaterDepth</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSurfacewaterError\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/channelSurfacewaterError</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelPrecipitation\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/channelPrecipitation</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelPrecipitationCumulative\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/channelPrecipitationCumulative</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelEvaporation\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/channelEvaporation</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelEvaporationCumulative\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/channelEvaporationCumulative</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
/* FIXME -> channelFIceOld(instances, channelElements, evapoTranspirationSnowLayers)  Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(channelStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"channelFIceOld\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nChannelEle, nSnowLayers);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelFIceOld</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelAlbOld\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelAlbOld</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSnEqvO\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelSnEqvO</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
/*FIXME -> channelStc(instances, channelElements, evapoTranspirationAllLayers)  Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(channelStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"channelStc\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nChannelEle, nAllLayers);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelStc</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelTah\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelTah</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelEah\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelEah</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelFWet\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelFWet</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelCanLiq\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelCanLiq</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelCanIce\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelCanIce</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelTv\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelTv</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelTg\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelTg</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelISnow\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\">state.nc:/channelISnow</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
/*FIXME -> channelZSnso(instances, channelElements, evapoTranspirationAllLayers)   Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(channelStateOut,"        <Attribute AttributeType=\"vector\" Center=\"Cell\" Name=\"channelZSnso\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nChannelEle, nAllLayers);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelZSnso</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSnowH\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelSnowH</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSnEqv\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelSnEqv</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
/*FIXME -> channelSnIce(instances, channelElements, evapoTranspirationSnowLayers)   Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(channelStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"channelSnIce\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nChannelEle, nSnowLayers);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelSnIce</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
/* FIXME -> channelSnLiq(instances, channelElements, evapoTranspirationSnowLayers)   Using AttributeType= Vector and DataItem ItemType = Collection, but Type= HyperSlab */
// Potential source of problems in the future 
fprintf(channelStateOut,"        <Attribute AttributeType=\"Vector\" Center=\"Cell\" Name=\"channelSnLiq\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"Collection\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nChannelEle, nSnowLayers);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelSnLiq</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelLfMass\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelLfMass</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelRtMass\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelRtMass</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelStMass\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelStMass</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelWood\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelWood</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelStblCp\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelStblCp</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelFastCp\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelFastCp</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelLai\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelLai</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSai\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelSai</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelCm\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelCm</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelCh\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelCh</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelTauss\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelTauss</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelDeepRech\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelDeepRech</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelRech\">\n");
fprintf(channelStateOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelStateOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelStateOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">state.nc:/channelRech</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(channelStateOut,"          </DataItem>\n");
fprintf(channelStateOut,"        </Attribute>\n");
fprintf(channelStateOut,"      </Grid>\n");    

 }

 for (nn = 0; nn < numInstances_disp; nn++)
 {
        // time step
        err_status = nc_inq_varid       ( ncid_disp, "currentTime", &TmpvarID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"currentTime\" variable ID from ../display.nc\n");
            exit(0);
        }

        err_status = nc_get_var1_double (ncid_disp,TmpvarID, &nn , &time );
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"currentTime\" variable from ../display.nc\n");
            exit(0);
        }       
        // geometry instance
        err_status = nc_inq_varid       ( ncid_disp, "geometryInstance", &TmpvarID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"geometryInstance\" variable ID from ../display.nc\n");
            exit(0);
        }

        err_status = nc_get_var1_ulonglong (ncid_disp,TmpvarID, &nn , &Inst_geo);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"geometryInstance\" variable from ../display.nc\n");
            exit(0);
        }
        // parameter instance
        err_status = nc_inq_varid       ( ncid_disp, "parameterInstance", &TmpvarID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"parameterInstance\" variable ID from ../display.nc\n");
            exit(0);
        }

        err_status = nc_get_var1_ulonglong (ncid_disp,TmpvarID, &nn , &Inst_par);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"parameterInstance\" variable from ../display.nc\n");
            exit(0);
        }
        
        // numberOfMeshElements ID  
        err_status = nc_inq_varid ( ncid_geo, "numberOfMeshElements", &TmpvarID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfMeshElements\" variable ID from ../geometry.nc\n");
            exit(0);
        }
        tmp = Inst_geo;
        err_status = nc_get_var1_int (ncid_geo,TmpvarID, &tmp , &nMeshEle);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfMeshElements\" variable ID from ../geometry.nc\n");
            exit(0);
        }
        // numberOfChannelElements ID  
        err_status = nc_inq_varid ( ncid_geo, "numberOfChannelElements", &TmpvarID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfChannelElements\" variable ID from ../geometry.nc\n");
            exit(0);
        }
        err_status = nc_get_var1_int (ncid_geo,TmpvarID, &tmp , &nChannelEle);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfChannelElements\" variable ID from ../geometry.nc\n");
            exit(0);
        }               
        // number Of Mesh nodes ID  
        err_status = nc_inq_varid ( ncid_geo, "numberOfMeshNodes", &TmpvarID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfMeshNodes\" variable ID from ../geometry.nc\n");
            exit(0);
        }
        err_status = nc_get_var1_int (ncid_geo, TmpvarID, &tmp , &nMeshNode);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfMeshNodes\" variable ID from ../geometry.nc\n");
            exit(0);
        }       
        // number Of Channel nodes ID  
        err_status = nc_inq_varid ( ncid_geo, "numberOfChannelNodes", &TmpvarID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfChannelNodes\" variable ID from ../geometry.nc\n");
            exit(0);
        }
        err_status = nc_get_var1_int (ncid_geo, TmpvarID, &tmp , &nChannelNode);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting the \"numberOfChannelNodes\" variable ID from ../geometry.nc\n");
            exit(0);
        }       
        
        // meshMeshNeighborsSize dimension ID  
        err_status = nc_inq_dimid (ncid_geo,"meshMeshNeighborsSize" , &tmpID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting meshMeshNeighborsSize dimesion id from ../geometry.nc\n");
            exit(0);
        }
        // # meshMeshNeighborsSize
        err_status = nc_inq_dimlen  (ncid_geo, tmpID, &nMeshNgb);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting dimmesion from ../geometry.nc\n");
            exit(0);
        }
        // channelElementVerticesSize dimension ID  
        err_status = nc_inq_dimid (ncid_geo,"channelElementVerticesSize" , &tmpID);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting channelElementVerticesSize dimmesion id from ../geometry.nc\n");
            exit(0);
        }
        // channelElementVerticesSize varaible
        err_status = nc_inq_dimlen  (ncid_geo, tmpID, &nChannelVrtArr);
        if (err_status != NC_NOERR)
        {
            printf("Problem getting channelElementVerticesSize dimmesion from ../geometry.nc\n");
            exit(0);
        }       
        
// for all groups: Mesh file
fprintf(meshDisplayOut,"      <Grid GridType=\"Uniform\">\n");
fprintf(meshDisplayOut,"        <Time Value=\"%f\"/>\n",time);
fprintf(meshDisplayOut,"        <Topology NumberOfElements=\"%d\" Type=\"Triangle\">\n",nMeshEle); 
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nMeshEle,nMeshNgb);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 3\" Format=\"XML\">%llu 0 0 1 1 1 1 %d %zu</DataItem>\n",Inst_geo,nMeshEle, nMeshNgb);
fprintf(meshDisplayOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d %zu\" Format=\"HDF\">geometry.nc:/meshElementVertices</DataItem>\n",numInstances_geo,nMeshEle, nMeshNgb);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Topology>\n");
fprintf(meshDisplayOut,"        <Geometry Type=\"X_Y_Z\">\n");
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeX</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeY</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeZSurface</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Geometry>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementX\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementX</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");                
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementY\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementY</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementZSurface\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementZSurface</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementZBedrock\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementZBedrock</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");      
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementArea\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementArea</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");     
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementSlopeX\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementSlopeX</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");     
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementSlopeY\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementSlopeY</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");     
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshCatchment\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\">parameter.nc:/meshCatchment</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");     
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshConductivity\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshConductivity</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshPorosity\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshPorosity</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshManningsN\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshManningsN</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterDepth\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshSurfacewaterDepth</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterError\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshSurfacewaterError</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterHead\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshGroundwaterHead</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterError\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshGroundwaterError</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshPrecipitation\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshPrecipitation</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshPrecipitationCumulative\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshPrecipitationCumulative</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshEvaporation\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshEvaporation</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshEvaporationCumulative\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshEvaporationCumulative</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshTranspiration\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshTranspiration</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshTranspirationCumulative\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/meshTranspirationCumulative</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSnEqv\">\n");        
fprintf(meshDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">display.nc:/meshSnEqv</DataItem>\n",numInstances_disp,nMeshEle);
fprintf(meshDisplayOut,"          </DataItem>\n");
fprintf(meshDisplayOut,"        </Attribute>\n");
fprintf(meshDisplayOut,"      </Grid>\n");    

// for all groups: Channel file
fprintf(channelDisplayOut,"      <Grid GridType=\"Uniform\">\n");
fprintf(channelDisplayOut,"        <Time Value=\"%f\"/>\n",time);
fprintf(channelDisplayOut,"        <Topology NumberOfElements=\"%d\" Type=\"Mixed\">\n",nChannelEle);
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nChannelEle,nChannelVrtArr);
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 3\" Format=\"XML\">%llu 0 0 1 1 1 1 %d %zu</DataItem>\n",Inst_geo,nChannelEle, nChannelVrtArr);   
fprintf(channelDisplayOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d %zu\" Format=\"HDF\">geometry.nc:/channelElementVertices</DataItem>\n",numInstances_geo,nChannelEle, nChannelVrtArr);   
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Topology>\n");
fprintf(channelDisplayOut,"        <Geometry Type=\"X_Y_Z\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelNode);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelNode);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeX</DataItem>\n",numInstances_geo,nChannelNode);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelNode);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelNode);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeY</DataItem>\n",numInstances_geo,nChannelNode);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelNode);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelNode);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeZBank</DataItem>\n",numInstances_geo,nChannelNode);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Geometry>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementX\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementX</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementY\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementY</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementZBank\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementZBank</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementZBed\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementZBed</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementLength\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementLength</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelChannelType\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\">parameter.nc:/channelChannelType</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelReachCode\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelReachCode</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBaseWidth\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBaseWidth</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSideSlope\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelSideSlope</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBedConductivity\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBedConductivity</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBedThickness\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBedThickness</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelManningsN\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelManningsN</DataItem>\n",numInstances_par,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSurfacewaterDepth\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/channelSurfacewaterDepth</DataItem>\n",numInstances_disp,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSurfacewaterError\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/channelSurfacewaterError</DataItem>\n",numInstances_disp,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelPrecipitation\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/channelPrecipitation</DataItem>\n",numInstances_disp,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelPrecipitationCumulative\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/channelPrecipitationCumulative</DataItem>\n",numInstances_disp,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelEvaporation\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/channelEvaporation</DataItem>\n",numInstances_disp,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelEvaporationCumulative\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">display.nc:/channelEvaporationCumulative</DataItem>\n",numInstances_disp,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSnEqv\">\n");
fprintf(channelDisplayOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(channelDisplayOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(channelDisplayOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"4\">display.nc:/channelSnEqv</DataItem>\n",numInstances_disp,nChannelEle);
fprintf(channelDisplayOut,"          </DataItem>\n");
fprintf(channelDisplayOut,"        </Attribute>\n");
fprintf(channelDisplayOut,"      </Grid>\n");    

 }
 
fprintf(meshStateOut,"    </Grid>\n");
fprintf(meshStateOut,"  </Domain>\n");
fprintf(meshStateOut,"</Xdmf>\n");

fprintf(channelStateOut,"    </Grid>\n");
fprintf(channelStateOut,"  </Domain>\n");
fprintf(channelStateOut,"</Xdmf>\n");

fprintf(meshDisplayOut,"    </Grid>\n");
fprintf(meshDisplayOut,"  </Domain>\n");
fprintf(meshDisplayOut,"</Xdmf>\n");

fprintf(channelDisplayOut,"    </Grid>\n");
fprintf(channelDisplayOut,"  </Domain>\n");
fprintf(channelDisplayOut,"</Xdmf>\n");

 
 err_status = nc_close(ncid_geo);
 if (err_status != NC_NOERR)
 	{
      	   printf("Problem closing ../geometry.nc\n");
           exit(0);
 	}
  err_status = nc_close(ncid_par);
 if (err_status != NC_NOERR)
 	{
      	   printf("Problem closing ../parameter.nc\n");
           exit(0);
 	}
 err_status = nc_close(ncid_stt);
 if (err_status != NC_NOERR)
        {
           printf("Problem closing ../state.nc\n");
           exit(0);
        }
 err_status = nc_close(ncid_disp);
 if (err_status != NC_NOERR)
        {
           printf("Problem closing ../display.nc\n");
           exit(0);
        }
 fclose(meshStateOut);
 fclose(channelStateOut);
 fclose(meshDisplayOut);
 fclose(channelDisplayOut);

 return 0;
 }
  
