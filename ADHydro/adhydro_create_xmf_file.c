#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>
#include <string.h>

// ADHydro outputs necdf files: status.nc, geometry.nc, and parameter.nc.
// In order to read these files in paraview, it is necessary to create
// Xdmf files.
// The purpose of this program is to generate the Xdmf files from the state.nc
// file. All the output information is in the state.nc file. 

int main(int argc, char*argv[])
{
 char *namestring;				// string to save the file names and directories of the output
 int err_status;				// error variable
 int nn;					// counter
 int ncid_stt;					// id of the state.nc file			
 int numgrps_stt;				// # of groups in the state.nc file
 int *grpid_stt = NULL;				// id of each group of the state.nc file
 double time;					// time of the group in the state.nc file
 int grp_geo, grp_par;				// group # for geometry.nc and parameter.nc files -> read from state.nc file
 char grp_stt_name[NC_MAX_NAME + 1];		// name of the groups in the state.nc file
 int tmpID;					// temporary net cdf ID
 size_t nMeshNgb;                               // # of Mesh Neighbors
 size_t nchannelVrtArr;                         // size of Channel Element Vertices Array
 size_t nMeshEle;			        // # of Mesh elements
 size_t nChannelEle;			        // # of Channel elements
 size_t nnode;					// # of nodes
 FILE* meshOut;					// file output for mesh elements with Xdmf format
 FILE* ChannelOut;				// file output for Channel elements with Xdmf format
 if (argc != 2)
 {
	printf("Usage:\n");
	printf("./adhydro_creat_xmf_file <One directory path for both .nc input and .xmf output files>\n");
	exit(0); 
 }
 
 namestring = malloc( sizeof( char )*( 1+ strlen(argv[1]) + strlen("/state.nc") ) );
  
 // Openning the state.nc file
 sprintf(namestring, "%s%s",argv[1],"/state.nc");
 err_status = nc_open(namestring, 0, &ncid_stt);
 if (err_status != NC_NOERR)
 {
        printf("Problem openning file %s\n", namestring);
        exit(0);
 }
 
 // Getting the # of groups and the groups Ids 
 err_status = nc_inq_grps(ncid_stt, &numgrps_stt, NULL);
 if (err_status != NC_NOERR)
 {
      printf("Problem getting the # of groups of %s\n",namestring );
      exit(0);
 }
 
 grpid_stt = malloc(sizeof(int)*numgrps_stt);
 err_status = nc_inq_grps(ncid_stt, NULL, grpid_stt);
 if (err_status != NC_NOERR)
 {
      printf("Problem getting the groups ids %s\n",namestring );
      exit(0);
 }
 
 sprintf(namestring, "%s%s",argv[1],"/mesh.xmf");
 meshOut = fopen (namestring, "w");
 sprintf(namestring, "%s%s",argv[1],"/channels.xmf");
 ChannelOut = fopen (namestring, "w");

 free(namestring);
 // Heather for mesh file 
fprintf(meshOut,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(meshOut,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(meshOut,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(meshOut,"  <Domain>\n");
fprintf(meshOut,"    <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"Mesh\">\n"); 
  // Heather for channel file 
fprintf(ChannelOut,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(ChannelOut,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(ChannelOut,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(ChannelOut,"  <Domain>\n");
fprintf(ChannelOut,"    <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"Channels\">\n"); 

 for (nn = 0; nn < numgrps_stt; nn++)
 {
 	// time step
 	
 	// getting the group name from the group id for the state.nc file
 	err_status = nc_inq_grpname(grpid_stt[nn], grp_stt_name);
	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting group name from ../state.nc\n");
      	    exit(0);
 	}

 	err_status = nc_get_att_double (grpid_stt[nn], NC_GLOBAL, "time", &time);
 	if (err_status != NC_NOERR)
 	{
 	     printf("Problem getting attributes of ../state.nc\n");
 	     exit(0);
 	}
	
 	err_status = nc_get_att_int (grpid_stt[nn], NC_GLOBAL, "geometryGroup", &grp_geo);

 	if (err_status != NC_NOERR)
 	{
 	     printf("Problem getting attributes of ../state.nc\n");
      	     exit(0);
 	}
	
	err_status = nc_get_att_int (grpid_stt[nn], NC_GLOBAL, "parameterGroup", &grp_par);
 	if (err_status != NC_NOERR)
 	{
     	    printf("Problem getting attributes of state.nc\n");
	    exit(0);
 	}
 	// dimension ID  
 	err_status = nc_inq_dimid (grpid_stt[ nn ],"numberOfMeshElements" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting dimmesion id from ../state.nc\n");
   	    exit(0);
 	}
 	
 	// # mesh elements  
 	err_status = nc_inq_dimlen  (grpid_stt[ nn ], tmpID, &nMeshEle);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting dimmesion from ../state.nc\n");
      	    exit(0);
 	}
 	// dimension ID  
 	err_status = nc_inq_dimid (grpid_stt[ nn ],"numberOfMeshMeshNeighbors" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting dimmesion id from ../state.nc\n");
   	    exit(0);
 	}
 	
 	// # mesh elements  
 	err_status = nc_inq_dimlen  (grpid_stt[ nn ], tmpID, &nMeshNgb);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting dimmesion from ../state.nc\n");
      	    exit(0);
 	}
 	
 	// dimension ID  
  	err_status = nc_inq_dimid (grpid_stt[ nn ],"numberOfChannelElements" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting dimmesion id from ../state.nc\n");
   	    exit(0);
 	}
 	
 	// # channels elements  
 	err_status = nc_inq_dimlen  (grpid_stt[ nn ], tmpID, &nChannelEle);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting dimmesion from ../state.nc\n");
      	    exit(0);
 	}
 	
 	// dimension ID  
  	err_status = nc_inq_dimid (grpid_stt[ nn ],"sizeOfChannelElementVerticesArray" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting dimmesion id from ../state.nc\n");
   	    exit(0);
 	}
 	
 	// size of channels vertice elements array  
 	err_status = nc_inq_dimlen  (grpid_stt[ nn ], tmpID, &nchannelVrtArr);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting dimmesion from ../state.nc\n");
      	    exit(0);
 	}

	// dimension ID  
  	err_status = nc_inq_dimid (grpid_stt[ nn ],"numberOfMeshNodes" , &tmpID);
  	if (err_status != NC_NOERR)
 	{
      	   printf("Problem getting dimmesion id from ../state.nc\n");
           exit(0);
 	}
 	
 	// # node elements  
 	err_status = nc_inq_dimlen  (grpid_stt[ nn ], tmpID, &nnode);
  	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting dimmesion from ../state.nc\n");
      	    exit(0);
 	}
	
// for all groups: Mesh file
fprintf(meshOut,"      <Grid GridType=\"Uniform\">\n");
fprintf(meshOut,"        <Time Value=\"%f\"/>\n",time);
fprintf(meshOut,"        <Topology NumberOfElements=\"%zu\" Type=\"Triangle\">\n",nMeshEle);
fprintf(meshOut,"          <DataItem DataType=\"Int\" Dimensions=\"%zu %zu\" Format=\"HDF\">geometry.nc:/%i/meshElementVertices</DataItem>\n",nMeshEle,nMeshNgb,grp_geo);
fprintf(meshOut,"        </Topology>\n");
fprintf(meshOut,"        <Geometry Type=\"X_Y_Z\">\n");
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshNodeX</DataItem>\n",nnode,grp_geo);   
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshNodeY</DataItem>\n",nnode,grp_geo);
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshNodeZSurface</DataItem>\n",nnode,grp_geo);                                          
fprintf(meshOut,"        </Geometry>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Node\" Name=\"meshNodeZBedrock\">\n");        
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshNodeZBedrock</DataItem>\n",nnode,grp_geo);
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementX\">\n");
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementX</DataItem>\n",nMeshEle,grp_geo);
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementY\">\n");
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementY</DataItem>\n",nMeshEle,grp_geo);                  
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementZSurface\">\n");
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementZSurface</DataItem>\n",nMeshEle,grp_geo);                  
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementZBedrock\">\n");
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementZBedrock</DataItem>\n",nMeshEle,grp_geo);
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementArea\">\n");  
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementArea</DataItem>\n",nMeshEle,grp_geo);
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementSlopeX\">\n");  
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementSlopeX</DataItem>\n",nMeshEle,grp_geo);
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementSlopeY\">\n");  
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementSlopeY</DataItem>\n",nMeshEle,grp_geo);
fprintf(meshOut,"        </Attribute>\n"); 
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshCatchment\">\n");
fprintf(meshOut,"          <DataItem DataType=\"Int\" Dimensions=\"%zu\" Format=\"HDF\">parameter.nc:/%i/meshCatchment</DataItem>\n",nMeshEle,grp_par); 
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshConductivity\">\n"); 
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshConductivity</DataItem>\n",nMeshEle,grp_par);
fprintf(meshOut,"        </Attribute>\n"); 
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshPorosity\">\n");
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshPorosity</DataItem>\n",nMeshEle,grp_par); 
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshManningsN\">\n"); 
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshManningsN</DataItem>\n",nMeshEle,grp_par);			
fprintf(meshOut,"        </Attribute>\n"); 
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterDepth\">\n");
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshSurfacewaterDepth</DataItem>\n",nMeshEle,grp_stt_name); 
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterError\">\n"); 
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshSurfacewaterError</DataItem>\n",nMeshEle,grp_stt_name);
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterHead\">\n");
fprintf(meshOut,"           <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshGroundwaterHead</DataItem>\n",nMeshEle,grp_stt_name);
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterError\">\n"); 
fprintf(meshOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshGroundwaterError</DataItem>\n",nMeshEle,grp_stt_name);
fprintf(meshOut,"        </Attribute>\n");     
fprintf(meshOut,"      </Grid>\n");    

// for all groups: Channel file
fprintf(ChannelOut,"      <Grid GridType=\"Uniform\">\n");
fprintf(ChannelOut,"        <Time Value=\"%f\"/>\n",time);
fprintf(ChannelOut,"         <Topology NumberOfElements=\"%zu\" Type=\"Mixed\">\n",nChannelEle);
fprintf(ChannelOut,"          <DataItem DataType=\"Int\" Dimensions=\"%zu %zu\" Format=\"HDF\">geometry.nc:/%i/meshElementVertices</DataItem>\n",nChannelEle,nchannelVrtArr, grp_geo);
fprintf(ChannelOut,"        </Topology>\n");
fprintf(ChannelOut,"        <Geometry Type=\"X_Y_Z\">\n");
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelNodeX</DataItem>\n",nnode,grp_geo);   
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelNodeY</DataItem>\n",nnode,grp_geo);
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelNodeZBank</DataItem>\n",nnode,grp_geo);                                          
fprintf(ChannelOut,"        </Geometry>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Node\" Name=\"channelNodeZBed\">\n");        
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelNodeZBed</DataItem>\n",nnode,grp_geo);
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementX\">\n");
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelElementX</DataItem>\n",nChannelEle,grp_geo);
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementY\">\n");
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelElementY</DataItem>\n",nChannelEle,grp_geo);                  
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementZBank\">\n");
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelElementZBank</DataItem>\n",nChannelEle,grp_geo);                  
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementZBed\">\n");
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelElementZBed</DataItem>\n",nChannelEle,grp_geo);
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementLength\">\n");  
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/channelElementLength</DataItem>\n",nChannelEle,grp_geo);
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelChannelType\">\n");  
fprintf(ChannelOut,"          <DataItem DataType=\"Int\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/channelChannelType</DataItem>\n",nChannelEle,grp_par);
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelPermanentCode\">\n");  
fprintf(ChannelOut,"          <DataItem DataType=\"Int\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/channelPermanentCode</DataItem>\n",nChannelEle,grp_par);
fprintf(ChannelOut,"        </Attribute>\n"); 
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBaseWidth\">\n");
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/channelBaseWidth</DataItem>\n",nChannelEle,grp_par); 
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSideSlope\">\n"); 
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/channelSideSlope</DataItem>\n",nChannelEle,grp_par);
fprintf(ChannelOut,"        </Attribute>\n"); 
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBedConductivity\">\n");
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/channelBedConductivity</DataItem>\n",nChannelEle,grp_par); 
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBedThickness\">\n"); 
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/channelBedThickness</DataItem>\n",nChannelEle,grp_par);			
fprintf(ChannelOut,"        </Attribute>\n"); 
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelManningsN\">\n");
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/channelManningsN</DataItem>\n",nChannelEle,grp_par); 
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSurfacewaterDepth\">\n"); 
fprintf(ChannelOut,"          <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/channelSurfacewaterDepth</DataItem>\n",nChannelEle,grp_stt_name);
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSurfacewaterError\">\n");
fprintf(ChannelOut,"           <DataItem DataType=\"Float\" Dimensions=\"%zu\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/channelSurfacewaterError</DataItem>\n",nChannelEle,grp_stt_name);
fprintf(ChannelOut,"        </Attribute>\n");     
fprintf(ChannelOut,"      </Grid>\n");    

 }
 
fprintf(meshOut,"    </Grid>\n");
fprintf(meshOut,"  </Domain>\n");
fprintf(meshOut,"</Xdmf>\n");

fprintf(ChannelOut,"    </Grid>\n");
fprintf(ChannelOut,"  </Domain>\n");
fprintf(ChannelOut,"</Xdmf>\n");

 free(grpid_stt);
 
 err_status = nc_close(ncid_stt);
 if (err_status != NC_NOERR)
 	{
      	   printf("Problem closing ../state.nc\n");
           exit(0);
 	}
 	
 return 0;
 }
  
