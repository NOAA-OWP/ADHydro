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
 char *namestring;				                // string to save the file names and directories of the output
 int err_status;				                // error variable
 size_t nn;					                // counter
 int ncid_stt, ncid_geo, ncid_par;				// id of the state.nc file	
 int TmpvarID;                                                  // Variable Id. Used for more than one variable, therefore temporary		
 size_t numInstances_stt, numInstances_geo, numInstances_par;	// # of instances in the *.nc files
 double time;					                // time of simulation in the state.nc file
 unsigned long long int Inst_geo, Inst_par;			           	// group # for geometry.nc and parameter.nc files -> read from state.nc file
 int tmpID;					                // temporary net cdf ID
 size_t nMeshNgb;                                               // # of Mesh Neighbors
 size_t nchannelVrtArr;                                         // size of Channel Element Vertices Array
 int nMeshEle;			                                // # of Mesh elements
 int nChannelEle;			                        // # of Channel elements
 int nMeshNode;				                        // # of mesh nodes
 size_t nchannelNodes;				                // # of channel nodes
 size_t tmp;                                                    // used to avoid warnings
 FILE* meshOut;					                // file output for mesh elements with Xdmf format
 FILE* ChannelOut;				                // file output for Channel elements with Xdmf format
 if (argc != 2)
 {
	printf("Usage:\n");
	printf("./adhydro_creat_xmf_file <One directory path for both .nc input and .xmf output files>\n");
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
 err_status = nc_inq_dimid (ncid_geo,"instances" , &tmpID);
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

 sprintf(namestring, "%s%s",argv[1],"/mesh1.xmf");
 meshOut = fopen (namestring, "w");
 sprintf(namestring, "%s%s",argv[1],"/channels1.xmf");
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
    	    printf("Problem getting the \"currentTime\" variable from ../state.nc\n");
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
    	    printf("Problem getting the \"currentTime\" variable ID from ../geometry.nc\n");
      	    exit(0);
 	}	
 	
 	// meshMeshNeighbors dimension ID  
 	err_status = nc_inq_dimid (ncid_geo,"meshMeshNeighbors" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting meshMeshNeighbors dimmesion id from ../geometry.nc\n");
   	    exit(0);
 	}
 	// # meshMeshNeighbors
 	err_status = nc_inq_dimlen  (ncid_geo, tmpID, &nMeshNgb);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting dimmesion from ../geometry.nc\n");
      	    exit(0);
 	}
 	// sizeOfChannelElementVerticesArray dimension ID  
 	err_status = nc_inq_dimid (ncid_geo,"sizeOfChannelElementVerticesArray" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting sizeOfChannelElementVerticesArray dimmesion id from ../geometry.nc\n");
   	    exit(0);
 	}
 	// # meshMeshNeighbors
 	err_status = nc_inq_dimlen  (ncid_geo, tmpID, &nchannelVrtArr);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting sizeOfChannelElementVerticesArray dimmesion from ../geometry.nc\n");
      	    exit(0);
 	}	
        // sizeOfChannelElementVerticesArray dimension ID  
 	err_status = nc_inq_dimid (ncid_geo,"channelNodes" , &tmpID);
 	if (err_status != NC_NOERR)
 	{
      	    printf("Problem getting channelNodes dimmesion id from ../geometry.nc\n");
   	    exit(0);
 	}
 	// # sizeOfChannelElementVerticesArray
 	err_status = nc_inq_dimlen  (ncid_geo, tmpID, &nchannelNodes);
  	if (err_status != NC_NOERR)
 	{
	    printf("Problem getting channelNodes dimmesion from ../geometry.nc\n");
      	    exit(0);
 	}	
     	
// for all groups: Mesh file
fprintf(meshOut,"      <Grid GridType=\"Uniform\">\n");
fprintf(meshOut,"        <Time Value=\"%f\"/>\n",time);
fprintf(meshOut,"        <Topology NumberOfElements=\"%d\" Type=\"Triangle\">\n",nMeshEle); 
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nMeshEle,nMeshNgb);
fprintf(meshOut,"            <DataItem Dimensions=\"3 3\" Format=\"XML\">%llu 0 0 1 1 1 1 %d %zu</DataItem>\n",Inst_geo,nMeshEle, nMeshNgb);
fprintf(meshOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d %zu\" Format=\"HDF\">geometry.nc:/meshElementVertices</DataItem>\n",numInstances_geo,nMeshEle, nMeshNgb);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Topology>\n");
fprintf(meshOut,"        <Geometry Type=\"X_Y_Z\">\n");
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeX</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeY</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);   
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeZSurface</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Geometry>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Node\" Name=\"meshNodeZBedrock\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshNode);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshNode);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshNodeZBedrock</DataItem>\n",numInstances_geo,nMeshNode);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");                
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementX\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementX</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");                
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementY\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementY</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementZSurface\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementZSurface</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementZBedrock\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementZBedrock</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");      
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementArea\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementArea</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");     
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementSlopeX\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementSlopeX</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");     
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshElementSlopeY\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/meshElementSlopeY</DataItem>\n",numInstances_geo,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");     
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshCatchment\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\">parameter.nc:/meshCatchment</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");     
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshConductivity\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshConductivity</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshPorosity\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshPorosity</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshManningsN\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/meshManningsN</DataItem>\n",numInstances_par,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterDepth\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshSurfacewaterDepth</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshSurfacewaterError\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshSurfacewaterError</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterHead\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshGroundwaterHead</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"meshGroundwaterError\">\n");        
fprintf(meshOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nMeshEle);
fprintf(meshOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nMeshEle);   
fprintf(meshOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/meshGroundwaterError</DataItem>\n",numInstances_stt,nMeshEle);
fprintf(meshOut,"          </DataItem>\n");
fprintf(meshOut,"        </Attribute>\n");
fprintf(meshOut,"      </Grid>\n");    

// for all groups: Channel file
fprintf(ChannelOut,"      <Grid GridType=\"Uniform\">\n");
fprintf(ChannelOut,"        <Time Value=\"%f\"/>\n",time);
fprintf(ChannelOut,"        <Topology NumberOfElements=\"%d\" Type=\"Mixed\">\n",nChannelEle);
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n",nChannelEle,nchannelVrtArr);
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 3\" Format=\"XML\">%llu 0 0 1 1 1 1 %d %zu</DataItem>\n",Inst_geo,nChannelEle, nchannelVrtArr);   
fprintf(ChannelOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d %zu\" Format=\"HDF\">geometry.nc:/channelElementVertices</DataItem>\n",numInstances_geo,nChannelEle, nchannelVrtArr);   
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Topology>\n");
fprintf(ChannelOut,"        <Geometry Type=\"X_Y_Z\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%zu\" Type=\"HyperSlab\">\n",nchannelNodes);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %zu</DataItem>\n",Inst_geo,nchannelNodes);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeX</DataItem>\n",numInstances_geo,nchannelNodes);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%zu\" Type=\"HyperSlab\">\n",nchannelNodes);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %zu</DataItem>\n",Inst_geo,nchannelNodes);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeY</DataItem>\n",numInstances_geo,nchannelNodes);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%zu\" Type=\"HyperSlab\">\n",nchannelNodes);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %zu</DataItem>\n",Inst_geo,nchannelNodes);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeZBank</DataItem>\n",numInstances_geo,nchannelNodes);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Geometry>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Node\" Name=\"channelNodeZBed\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%zu\" Type=\"HyperSlab\">\n",nchannelNodes);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %zu</DataItem>\n",Inst_geo,nchannelNodes);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %zu\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelNodeZBed</DataItem>\n",numInstances_geo,nchannelNodes);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementX\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementX</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementY\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementY</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementZBank\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementZBank</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementZBed\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementZBed</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelElementLength\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">geometry.nc:/channelElementLength</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelChannelType\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_geo,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\">parameter.nc:/channelChannelType</DataItem>\n",numInstances_geo,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelPermanentCode\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Int\" Dimensions=\"%zu %d\" Format=\"HDF\">parameter.nc:/channelPermanentCode</DataItem>\n",numInstances_par,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBaseWidth\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBaseWidth</DataItem>\n",numInstances_par,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSideSlope\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelSideSlope</DataItem>\n",numInstances_par,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBedConductivity\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBedConductivity</DataItem>\n",numInstances_par,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelBedThickness\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelBedThickness</DataItem>\n",numInstances_par,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelManningsN\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n",Inst_par,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">parameter.nc:/channelManningsN</DataItem>\n",numInstances_par,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSurfacewaterDepth\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/channelSurfacewaterDepth</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"channelSurfacewaterError\">\n");
fprintf(ChannelOut,"          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n",nChannelEle);   
fprintf(ChannelOut,"            <DataItem Dimensions=\"3 2\" Format=\"XML\">%zu 0 1 1 1 %d</DataItem>\n",nn,nChannelEle);
fprintf(ChannelOut,"            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">state.nc:/channelSurfacewaterError</DataItem>\n",numInstances_stt,nChannelEle);
fprintf(ChannelOut,"          </DataItem>\n");
fprintf(ChannelOut,"        </Attribute>\n");
fprintf(ChannelOut,"      </Grid>\n");    

 }

fprintf(meshOut,"    </Grid>\n");
fprintf(meshOut,"  </Domain>\n");
fprintf(meshOut,"</Xdmf>\n");

fprintf(ChannelOut,"    </Grid>\n");
fprintf(ChannelOut,"  </Domain>\n");
fprintf(ChannelOut,"</Xdmf>\n");

 
 err_status = nc_close(ncid_stt);
 if (err_status != NC_NOERR)
 	{
      	   printf("Problem closing ../state.nc\n");
           exit(0);
 	}
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
 free(meshOut);
 free(ChannelOut);

 return 0;
 }
  
