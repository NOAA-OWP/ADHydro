#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>
#include <string.h>

// ADHydro outputs necdf files: status.nc, geometry.nc, and parameter.nc.
// In order to read these files in paraview, it is necessary to create
// Xdmf files.
// The purpose of this program is to generate the Xdmf files from the .nc
// files. All the output information are in the .nc files. 
// 06 - 09 - 2014
// Leticia Pureza 
int main(int argc, char*argv[])
{
 char *namestring;				// string to save the file names and directories
 int err_status;				// error variable
 int n;						// counter
 int ncid_stt;					// id of the state.nc file			
 int numgrps_stt;				// # of groups in the state.nc file
 int *grpid_stt = NULL;				// id of each group of the state.nc file
 double time;					// time of the group in the state.nc file
 int grp_geo, grp_par;				// group # for geometry.nc and parameter.nc files -> read from state.nc file
 char grp_stt_name[NC_MAX_NAME + 1];		// name of the groups in the state.nc file
 int neleID;					// id for the number of elements dimension
 int nnodeID;					// id for the number of nodes dimension
 size_t nele;					// # of elements
 size_t nnode;					// # of nodes
 FILE* output;					// file output with Xdmf format
 
 if (argc != 2)
 {
	printf("Usage:\n");
	printf("./adhydro_creat_xmf_file <directory of output files>\n");
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
 output = fopen (namestring, "w");
 
 free(namestring);
  
fprintf(output,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(output,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(output,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(output,"  <Domain>\n");
fprintf(output,"    <Grid CollectionType=\"Temporal\" GridType=\"Collection\" Name=\"Mesh\">\n\n"); 
 
 
 for (n = 0; n < numgrps_stt; n++)
 {
 	// time step
 	
 	// getting the group name from the group id for the state.nc file
 	err_status = nc_inq_grpname(grpid_stt[n], grp_stt_name);
	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting group name from ../state.nc\n");
      	    exit(0);
 	}

 	err_status = nc_get_att_double (grpid_stt[n], NC_GLOBAL, "time", &time);
 	if (err_status != NC_NOERR)
 	{
 	     printf("Problem getting attributes of ../state.nc\n");
 	     exit(0);
 	}
	
 	err_status = nc_get_att_int (grpid_stt[n], NC_GLOBAL, "geometryGroup", &grp_geo);
 	
 	if (err_status != NC_NOERR)
 	{
 	     printf("Problem getting attributes of ../state.nc\n");
      	     exit(0);
 	}
	
	err_status = nc_get_att_int (grpid_stt[n], NC_GLOBAL, "parameterGroup", &grp_par);
 	if (err_status != NC_NOERR)
 	{
     	    printf("Problem getting attributes of state.nc\n");
	    exit(0);
 	}

 	err_status = nc_inq_dimid (grpid_stt[ n ],"numberOfMeshElements" , &neleID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting dimmesion id from ../state.nc\n");
   	    exit(0);
 	}
 	
 	// # elements  
 	err_status = nc_inq_dimlen  (grpid_stt[ n ], neleID, &nele);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting dimmesion from ../state.nc\n");
      	    exit(0);
 	}
  	
  	err_status = nc_inq_dimid (grpid_stt[ n ],"numberOfMeshNodes" , &nnodeID);
  	if (err_status != NC_NOERR)
 	{
      	   printf("Problem getting dimmesion id from ../state.nc\n");
           exit(0);
 	}
 	
 	// dimension size  
 	err_status = nc_inq_dimlen  (grpid_stt[ n ], nnodeID, &nnode);
  	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting dimmesion from ../state.nc\n");
      	    exit(0);
 	}
	
// for all groups
fprintf(output,"      <Grid GridType=\"Uniform\">\n");
fprintf(output,"        <Time Value=\"%f\"/>\n",time);
fprintf(output,"        <Topology NumberOfElements=\"%i\" Type=\"Triangle\">\n",nele);
fprintf(output,"          <DataItem DataType=\"Int\" Dimensions=\"%i 3\" Format=\"HDF\">geometry.nc:/%i/meshElementNodeIndices</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Topology>\n");
fprintf(output,"        <Geometry Type=\"XYZ\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshNodeXYZSurfaceCoordinates</DataItem>\n",nnode,grp_geo);
fprintf(output,"        </Geometry>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Node\" Name=\"NodeZBedrock\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\" 8\">geometry.nc:/%i/meshNodeZBedrockCoordinates</DataItem>\n",nnode,grp_geo);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeLength\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshEdgeLength</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeNormalX\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshEdgeNormalX</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeNormalY\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshEdgeNormalY</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementX\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementX</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementY\">\n");
fprintf(output,"          \n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementY</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementZSurface\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementZSurface</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementZBedrock\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementZBedrock</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementArea\">\n");  
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementArea</DataItem>\n",nele,grp_geo);
fprintf(output,"        </Attribute>\n"); 
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementCatchment\">\n");
fprintf(output,"          <DataItem DataType=\"Int\" Dimensions=\"%i\" Format=\"HDF\">parameter.nc:/%i/meshElementCatchment</DataItem>\n",nele,grp_par); 
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementConductivity\">\n"); 
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshElementConductivity</DataItem>\n",nele,grp_par);
fprintf(output,"        </Attribute>\n"); 
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementPorosity\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshElementPorosity</DataItem>\n",nele,grp_par); 
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementManningsN\">\n"); 
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshElementManningsN</DataItem>\n",nele,grp_par);			
fprintf(output,"        </Attribute>\n"); 
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementSurfacewaterDepth\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshElementSurfacewaterDepth</DataItem>\n",nele,grp_stt_name); 
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementSurfacewaterError\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshElementSurfacewaterError</DataItem>\n",nele,grp_stt_name);
fprintf(output,"        </Attribute>\n"); 
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementGroundwaterHead\">\n");
fprintf(output,"           <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshElementGroundwaterHead</DataItem>\n",nele,grp_stt_name);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementGroundwaterError\">\n"); 
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshElementGroundwaterError</DataItem>\n",nele,grp_stt_name);
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementGroundwaterRecharge\">\n"); 
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshElementGroundwaterRecharge</DataItem>\n",nele,grp_stt_name); 
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeSurfacewaterFlowRate\">\n"); 
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshEdgeSurfacewaterFlowRate</DataItem>\n",nele,grp_stt_name);
fprintf(output,"        </Attribute>\n"); 
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeSurfacewaterCumulativeFlow\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshEdgeSurfacewaterCumulativeFlow</DataItem>\n",nele,grp_stt_name); 
fprintf(output,"        </Attribute>\n");
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeGroundwaterFlowRate\">\n"); 
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshEdgeGroundwaterFlowRate</DataItem>\n",nele,grp_stt_name);
fprintf(output,"        </Attribute>\n"); 
fprintf(output,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeGroundwaterCumulativeFlow\">\n");
fprintf(output,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">state.nc:/%s/meshEdgeGroundwaterCumulativeFlow</DataItem>\n",nele,grp_stt_name); 
fprintf(output,"        </Attribute>\n");
fprintf(output,"      </Grid>\n\n");
 
 }
 
 fprintf(output,"    </Grid>\n");
fprintf(output,"  </Domain>\n");
fprintf(output,"</Xdmf>\n");
 
 free(grpid_stt);
 
 err_status = nc_close(ncid_stt);
 if (err_status != NC_NOERR)
 	{
      	   printf("Problem closing ../state.nc\n");
           exit(0);
 	}
 	
 return 0;
 }
  
