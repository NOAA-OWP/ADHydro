#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>
#include <string.h>

// Program to convert netcdf files to txt
// IMPORTANT: Considers that the name of the groups
// 	      in the  netcdf files are numbers and 
//	      therefore are used as index of an array
// 06 - 09 - 2014
// Leticia Pureza 
int main(int argc, char*argv[])
{
 char *namestring;
 char tmp;
 int err_status,n;
 int ncid_stt, ncid_geo;
 int numgrps_stt, numgrps_geo;
 int *grpid_stt = NULL, *grpid_geo = NULL;
 double time;
 int grp_geo, grp_par, grp_stt;
 int neleID, nnodeID;
 size_t nele, nnode;
 
 if (argc != 2)
 {
	printf("Path to the directory of the input files not included or too many parameters passed.\n");
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
      printf("Problem getting the groups ids of state.nc\n" );
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
 
 // Getting the # of groups and the groups Ids 
 err_status = nc_inq_grps(ncid_geo, &numgrps_geo, NULL);
 if (err_status != NC_NOERR)
 {
       printf("Problem getting the # of groups of %s\n",namestring);
       exit(0);
 }
 
 grpid_geo = malloc(sizeof(int)*numgrps_geo);
 err_status = nc_inq_grps(ncid_geo, NULL, grpid_geo);
 if (err_status != NC_NOERR)
 {
       printf("Problem getting the # of groups of %s\n",namestring);
      exit(0);
 }

 sprintf(namestring, "%s%s",argv[1],"mesh.xmf");
 FILE * fp;
 fp = fopen (namestring, "w");
 
 free(namestring);
  
fprintf(fp,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(fp,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(fp,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(fp,"  <Domain>\n");
fprintf(fp,"    <Grid CollectionType=\"Temporal\" GridType=\"Collection\" Name=\"Mesh\">\n\n"); 
 
 
 for (n = 0; n < numgrps_stt; n++)
 {
 	// time step
 	err_status = nc_get_att_double (grpid_stt[n], NC_GLOBAL, "time", &time);
 	if (err_status != NC_NOERR)
 	{
 	     printf("Problem getting attributes of state.nc\n");
 	     exit(0);
 	}
	
 	err_status = nc_get_att_int (grpid_stt[n], NC_GLOBAL, "geometryGroup", &grp_geo);
 	
 	if (err_status != NC_NOERR)
 	{
 	     printf("Problem getting attributes of state.nc\n");
      	     exit(0);
 	}
	
	err_status = nc_get_att_int (grpid_stt[n], NC_GLOBAL, "parameterGroup", &grp_par);
 	if (err_status != NC_NOERR)
 	{
     	    printf("Problem getting attributes of state.nc\n");
	    exit(0);
 	}

 	err_status = nc_inq_dimid (grpid_geo[ grp_geo ],"numberOfMeshElements" , &neleID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting dimmesion id from geometry.nc\n");
   	    exit(0);
 	}
 	
 	// # elements  
 	err_status = nc_inq_dimlen  (grpid_geo[ grp_geo], neleID, &nele);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting dimmesion from geometry.nc\n");
      	    exit(0);
 	}
  	
  	err_status = nc_inq_dimid (grpid_geo[ grp_geo ],"numberOfMeshNodes" , &nnodeID);
  	if (err_status != NC_NOERR)
 	{
      	   printf("Problem getting dimmesion id from geometry.nc\n");
           exit(0);
 	}
 	
 	// dimension size  
 	err_status = nc_inq_dimlen  (grpid_geo[ grp_geo ], nnodeID, &nnode);
  	if (err_status != NC_NOERR)
 	{
    	    printf("Problem getting dimmesion from geometry.nc\n");
      	    exit(0);
 	}

// for all groups
fprintf(fp,"      <Grid GridType=\"Uniform\">\n");
fprintf(fp,"        <Time Value=\"%f\"/>\n",time);
fprintf(fp,"        <Topology NumberOfElements=\"%i\" Type=\"Triangle\">\n",nele);
fprintf(fp,"          <DataItem DataType=\"Int\" Dimensions=\"%i 3\" Format=\"HDF\">geometry.nc:/%i/meshElementNodeIndices</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Topology>\n");
fprintf(fp,"        <Geometry Type=\"XYZ\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshNodeXYZSurfaceCoordinates</DataItem>\n",nnode,grp_geo);
fprintf(fp,"        </Geometry>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Node\" Name=\"NodeZBedrock\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\" 8\">geometry.nc:/%i/meshNodeZBedrockCoordinates</DataItem>\n",nnode,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeLength\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshEdgeLength</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeNormalX\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshEdgeNormalX</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeNormalY\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshEdgeNormalY</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementX\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementX</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementY\">\n");
fprintf(fp,"          \n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementY</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementZSurface\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementZSurface</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementZBedrock\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementZBedrock</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementArea\">\n");  
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementArea</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n"); 
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementCatchment\">\n");
fprintf(fp,"          <DataItem DataType=\"Int\" Dimensions=\"%i\" Format=\"HDF\">parameter.nc:/%i/meshElementCatchment</DataItem>\n",nele,grp_par); 
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementConductivity\">\n"); 
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshElementConductivity</DataItem>\n",nele,grp_par);
fprintf(fp,"        </Attribute>\n"); 
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementPorosity\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshElementPorosity</DataItem>\n",nele,grp_par); 
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementManningsN\">\n"); 
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">parameter.nc:/%i/meshElementManningsN</DataItem>\n",nele,grp_par);			
fprintf(fp,"        </Attribute>\n"); 
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementSurfacewaterDepth\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshElementSurfacewaterDepth</DataItem>\n",nele,n); 
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementSurfacewaterError\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshElementSurfacewaterError</DataItem>\n",nele,n);
fprintf(fp,"        </Attribute>\n"); 
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementGroundwaterHead\">\n");
fprintf(fp,"           <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshElementGroundwaterHead</DataItem>\n",nele,n);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementGroundwaterError\">\n"); 
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshElementGroundwaterError</DataItem>\n",nele,n);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementGroundwaterRecharge\">\n"); 
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshElementGroundwaterRecharge</DataItem>\n",nele,n); 
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeSurfacewaterFlowRate\">\n"); 
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshEdgeSurfacewaterFlowRate</DataItem>\n",nele,n);
fprintf(fp,"        </Attribute>\n"); 
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeSurfacewaterCumulativeFlow\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshEdgeSurfacewaterCumulativeFlow</DataItem>\n",nele,n); 
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeGroundwaterFlowRate\">\n"); 
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshEdgeGroundwaterFlowRate</DataItem>\n",nele,n);
fprintf(fp,"        </Attribute>\n"); 
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeGroundwaterCumulativeFlow\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">state.nc:/%i/meshEdgeGroundwaterCumulativeFlow</DataItem>\n",nele,n); 
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"      </Grid>\n\n");
 
 }
 
 fprintf(fp,"    </Grid>\n");
fprintf(fp,"  </Domain>\n");
fprintf(fp,"</Xdmf>\n");
 
 free(grpid_geo);
 free(grpid_stt);
 
 err_status = nc_close(ncid_stt);
 if (err_status != NC_NOERR)
 	{
      	   printf("Problem closing ../state.nc\n");
           exit(0);
 	}

 if (err_status != NC_NOERR)
 	{
      	   printf("Problem closing ../geometry.nc\n");
           exit(0);
 	}
 	
 return 0;
 }
  
