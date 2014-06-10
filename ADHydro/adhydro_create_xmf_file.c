#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

// Program to convert netcdf files to txt
// IMPORTANT: Considers that the name of the groups
// 	      in the  netcdf files are numbers and 
//	      therefore are used as index of an array
// 06 - 09 - 2014
// Leticia Pureza 
int main(int argc, char*argv[])
{
 char *namestring;
 int status,n;
 int ncid_stt, ncid_geo, ncid_par;
 int numgrps_stt, numgrps_geo, numgrps_par;
 int *grpid_stt = NULL, *grpid_geo = NULL;
 double time;
 int grp_geo, grp_par, grp_stt;
 int neleID, nnodeID;
 size_t nele, nnode;
 
 namestring = malloc( sizeof( char )*( 1+ strlen(argv[1]) + strlen("parameter.nc") ) );

 sprintf(namestring, "%s%s",argv[1],"mesh.xmf");
 FILE * fp;
 fp = fopen (namestring, "w");
   
 // Openning the state.nc file
 sprintf(namestring, "%s%s",argv[1],"state.nc");
 status = nc_open(namestring, 0, &ncid_stt);

 // Getting the # of groups and the groups Ids 
 status = nc_inq_grps(ncid_stt, &numgrps_stt, NULL);
 printf("# groups %i\n", numgrps_stt);
 grpid_stt = malloc(sizeof(int)*numgrps_stt);
 status = nc_inq_grps(ncid_stt, NULL, grpid_stt);
 
  // Openning the geometry.nc file
 sprintf(namestring, "%s%s",argv[1],"geometry.nc");
 status = nc_open(namestring, 0, &ncid_geo);
 
 // Getting the # of groups and the groups Ids 
 status = nc_inq_grps(ncid_geo, &numgrps_geo, NULL);
 grpid_geo = malloc(sizeof(int)*numgrps_geo);
 status = nc_inq_grps(ncid_geo, NULL, grpid_geo);
 
  // Openning the parameter.nc file
 sprintf(namestring, "%s%s",argv[1],"parameter.nc");
 status = nc_open(namestring, 0, &ncid_par);
 
 // Getting the # of groups and the groups Ids 
 //status = nc_inq_grps(ncid_par, &numgrps_par, NULL);
 //grpid_par = malloc(sizeof(int)*numgrps_par);
 //status = nc_inq_grps(ncid_par, NULL, grpid_par);
 
 free(namestring);
  
fprintf(fp,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
fprintf(fp,"<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
fprintf(fp,"<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
fprintf(fp,"  <Domain>\n");
 
 
 for (n = 0; n < numgrps_stt; n++)
 {
 	// time step
 	printf("group %i ",n);
 	status = nc_get_att_double (grpid_stt[n], NC_GLOBAL, "time", &time);
 	printf("time %f\n", time);

 	status = nc_get_att_int (grpid_stt[n], NC_GLOBAL, "geometryGroup", &grp_geo);
 	printf("group geo %i\n", grp_geo);
	
	status = nc_get_att_int (grpid_stt[n], NC_GLOBAL, "parameterGroup", &grp_par);
 	printf("group par %i\n", grp_par);
 

 	status = nc_inq_dimid (grpid_geo[ grp_geo ],"numberOfMeshElements" , &neleID);
 	// # elements  
 	status = nc_inq_dimlen  (grpid_geo[ grp_geo], neleID, &nele);
  	printf("# ele %i\n", nele);
  	
  	status = nc_inq_dimid (grpid_geo[ grp_geo ],"numberOfMeshNodes" , &nnodeID);
  	
 	// dimension size  
 	status = nc_inq_dimlen  (grpid_geo[ grp_geo ], nnodeID, &nnode);
  	printf("# nodes %i\n", nnode);

// for all groups
fprintf(fp,"    <Grid CollectionType=\"Temporal\" GridType=\"Collection\" Name=\"Mesh\">\n\n"); 
fprintf(fp,"      <Grid GridType=\"Uniform\">\n");
fprintf(fp,"        <Time Value=\"%f\"/>\n",time);
fprintf(fp,"        <Topology NumberOfElements=\"%i\" Type=\"Triangle\">\n",nele);
fprintf(fp,"          <DataItem DataType=\"Int\" Dimensions=\"%i 3\" Format=\"HDF\">geometry.nc:/%i/meshElementNodeIndices</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Topology>\n");
fprintf(fp,"        <Geometry Type=\"XYZ\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshNodeXYZSurfaceCoordinates</DataItem>\n",nnode,grp_geo);
fprintf(fp,"        </Geometry>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Node\" Name=\"ZBedrock\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\" 8\">geometry.nc:/%i/meshNodeZBedrockCoordinates</DataItem>\n",nnode,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeLength\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshEdgeLength</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeNormalX\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i 3\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshEdgeNormalX</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Edge\" Name=\"EdgeNormalY\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementY</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementX\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementX</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementY\">\n");
fprintf(fp,"          \n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementY</DataItem>\n",nele,grp_geo);
fprintf(fp,"        </Attribute>\n");
fprintf(fp,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"ElementZSurface\">\n");
fprintf(fp,"          <DataItem DataType=\"Float\" Dimensions=\"%i\" Format=\"HDF\" Precision=\"8\">geometry.nc:/%i/meshElementZSurface</DataItem>\n");
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
fprintf(fp,"      </Grid>\n\n");
 
 }
 
 fprintf(fp,"    </Grid>\n");
fprintf(fp,"  </Domain>\n");
fprintf(fp,"</Xdmf>\n");
 
 free(grpid_geo);
 free(grpid_stt);
 
 return 0;
 }
  
