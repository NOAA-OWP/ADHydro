#include "all.h"
#include <stdio.h>
#include <string>
#include <netcdf.h>

// ADHydro outputs NetCDF files: geometry.nc, parameter.nc, state.nc, and
// display.nc.  In order to read these files in paraview, it is necessary to
// create XDMF files.  The purpose of this program is to generate the XDMF
// files.  The XDMF files contain metadata that is redundant with the data in
// the NetCDF files so they are created from the NetCDF files.

// FIXME this program is a little light on error checking.  It only checks if the files can be opened.
// FIXME we could have a more flexible way to specify the locations of all eight files that will be touched.

int openNetCDFFile(std::string directoryPath, std::string fileName)
{
  int         ncErrorCode;
  std::string filePath;
  int         fileID;
  
  filePath    = directoryPath + "/" + fileName;
  ncErrorCode = nc_open(filePath.c_str(), NC_NOWRITE, &fileID);
  
  if (NC_NOERR != ncErrorCode)
    {
      printf("ERROR: Cannot open NetCDF file %s for read.\n", filePath.c_str());
      exit(-1);
    }
  
  return fileID;
}

FILE* openXDMFFile(std::string directoryPath, std::string fileName)
{
  std::string filePath;
  FILE*       fileID;
  
  filePath = directoryPath + "/" + fileName;
  fileID   = fopen(filePath.c_str(), "w");
  
  if (NULL == fileID)
    {
      printf("ERROR: Cannot open XDMF file %s for write.\n", filePath.c_str());
      exit(-1);
    }
  
  return fileID;
}

void writeXMLHeader(FILE* outputFile, const char* gridName)
{
  fprintf(outputFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(outputFile, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\">\n");
  fprintf(outputFile, "<Xdmf Version=\"2.0\" xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n");
  fprintf(outputFile, "  <Domain>\n");
  fprintf(outputFile, "    <Grid GridType=\"Collection\" CollectionType=\"Temporal\" Name=\"%s\">\n", gridName); 
}

void writeGridHeader(FILE* outputFile, double currentTime)
{
  fprintf(outputFile, "      <Grid GridType=\"Uniform\">\n");
  fprintf(outputFile, "        <Time Value=\"%lf\"/>\n", currentTime);
}

void writeGeometryDataItem(FILE* outputFile, const char* sourceFileName, const char* nodeName, int numberOfNodes, unsigned long long instance, size_t numberOfInstances)
{
  fprintf(outputFile, "          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n", numberOfNodes);   
  fprintf(outputFile, "            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n", instance, numberOfNodes);   
  fprintf(outputFile, "            <DataItem DataType=\"Float\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"8\">%s:/%s</DataItem>\n", numberOfInstances, numberOfNodes, sourceFileName, nodeName);
  fprintf(outputFile, "          </DataItem>\n");
}

void writeTopologyAndGeometry(FILE* outputFile, const char* sourceFileName, const char* topologyType, const char* verticesName, const char* nodeXName, const char* nodeYName, const char* nodeZName,
                              int numberOfNodes, int numberOfElements, size_t verticesSize, unsigned long long instance, size_t numberOfInstances)
{
  fprintf(outputFile, "        <Topology NumberOfElements=\"%d\" Type=\"%s\">\n", numberOfElements, topologyType); 
  fprintf(outputFile, "          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d %zu\" Type=\"HyperSlab\">\n", numberOfElements, verticesSize);
  fprintf(outputFile, "            <DataItem Dimensions=\"3 3\" Format=\"XML\">%llu 0 0 1 1 1 1 %d %zu</DataItem>\n", instance, numberOfElements, verticesSize);
  fprintf(outputFile, "            <DataItem DataType=\"Int\" Dimensions=\"%zu %d %zu\" Format=\"HDF\">%s:/%s</DataItem>\n", numberOfInstances, numberOfElements, verticesSize, sourceFileName, verticesName);
  fprintf(outputFile, "          </DataItem>\n");
  fprintf(outputFile, "        </Topology>\n");
  fprintf(outputFile, "        <Geometry Type=\"X_Y_Z\">\n");
  writeGeometryDataItem(outputFile, sourceFileName, nodeXName, numberOfNodes, instance, numberOfInstances);
  writeGeometryDataItem(outputFile, sourceFileName, nodeYName, numberOfNodes, instance, numberOfInstances);
  writeGeometryDataItem(outputFile, sourceFileName, nodeZName, numberOfNodes, instance, numberOfInstances);
  fprintf(outputFile, "        </Geometry>\n");
}

void writeScalarAttribute(FILE* outputFile, const char* sourceFileName, const char* attributeName, const char* dataType, int precision, int numberOfElements, unsigned long long instance, size_t numberOfInstances)
{
  fprintf(outputFile, "        <Attribute AttributeType=\"Scalar\" Center=\"Cell\" Name=\"%s\">\n", attributeName);
  fprintf(outputFile, "          <DataItem ItemType=\"HyperSlab\" Dimensions=\"%d\" Type=\"HyperSlab\">\n", numberOfElements);
  fprintf(outputFile, "            <DataItem Dimensions=\"3 2\" Format=\"XML\">%llu 0 1 1 1 %d</DataItem>\n", instance, numberOfElements);
  fprintf(outputFile, "            <DataItem DataType=\"%s\" Dimensions=\"%zu %d\" Format=\"HDF\" Precision=\"%d\">%s:/%s</DataItem>\n", dataType, numberOfInstances, numberOfElements, precision, sourceFileName, attributeName);
  fprintf(outputFile, "          </DataItem>\n");
  fprintf(outputFile, "        </Attribute>\n");
}

void writeGridFooter(FILE* outputFile)
{
  fprintf(outputFile, "      </Grid>\n");    
}

void writeXMLFooter(FILE* outputFile)
{
  fprintf(outputFile, "    </Grid>\n");
  fprintf(outputFile, "  </Domain>\n");
  fprintf(outputFile, "</Xdmf>\n");
}

int main(int argc, char** argv)
{
  int                geometryFileID;
  int                parameterFileID;
  int                stateFileID;
  int                displayFileID;
  FILE*              meshStateOut;
  FILE*              channelStateOut;
  FILE*              meshDisplayOut;
  FILE*              channelDisplayOut;
  int                dimensionVariableID;
  size_t             index[1];
  size_t             numberOfGeometryInstances;
  unsigned long long geometryInstance;
  size_t             numberOfParameterInstances;
  unsigned long long parameterInstance;
  size_t             numberOfStateInstances;
  unsigned long long stateInstance;
  size_t             numberOfDisplayInstances;
  unsigned long long displayInstance;
  double             currentTime;
  int                numberOfMeshNodes;
  int                numberOfMeshElements;
  size_t             meshMeshNeighborsSize;
  int                numberOfChannelNodes;
  int                numberOfChannelElements;
  size_t             channelVerticesXDMFSize;
  
  if (2 != argc)
    {
      printf("Usage:\n\nadhydro_create_xdmf_file <directory path for both .nc input files and .xdmf output files>\n");
      exit(-1);
    }
  
  // Open NetCDF input files.
  geometryFileID  = openNetCDFFile(argv[1], "geometry.nc");
  parameterFileID = openNetCDFFile(argv[1], "parameter.nc");
  stateFileID     = openNetCDFFile(argv[1], "state.nc");
  displayFileID   = openNetCDFFile(argv[1], "display.nc");
  
  // Open XDMF output files.
  meshStateOut      = openXDMFFile(argv[1], "mesh_state.xdmf");
  channelStateOut   = openXDMFFile(argv[1], "channel_state.xdmf");
  meshDisplayOut    = openXDMFFile(argv[1], "mesh_display.xdmf");
  channelDisplayOut = openXDMFFile(argv[1], "channel_display.xdmf");
  
  // Read dimensions from geometry.nc.
  nc_inq_dimid(geometryFileID, "instances", &dimensionVariableID);
  nc_inq_dimlen(geometryFileID, dimensionVariableID, &numberOfGeometryInstances);
  nc_inq_dimid(geometryFileID, "meshMeshNeighborsSize", &dimensionVariableID);
  nc_inq_dimlen(geometryFileID, dimensionVariableID, &meshMeshNeighborsSize);
  nc_inq_dimid(geometryFileID, "channelVerticesXDMFSize", &dimensionVariableID);
  nc_inq_dimlen(geometryFileID, dimensionVariableID, &channelVerticesXDMFSize);
  
  // Read dimensions from parameter.nc.
  nc_inq_dimid(parameterFileID, "instances", &dimensionVariableID);
  nc_inq_dimlen(parameterFileID, dimensionVariableID, &numberOfParameterInstances);
  
  // Read dimensions from state.nc.
  nc_inq_dimid(stateFileID, "instances", &dimensionVariableID);
  nc_inq_dimlen(stateFileID, dimensionVariableID, &numberOfStateInstances);
  
  // Read dimensions from display.nc.
  nc_inq_dimid(displayFileID, "instances", &dimensionVariableID);
  nc_inq_dimlen(displayFileID, dimensionVariableID, &numberOfDisplayInstances);
  
  writeXMLHeader(meshStateOut,      "MeshState");
  writeXMLHeader(channelStateOut,   "ChannelState");
  writeXMLHeader(meshDisplayOut,    "MeshDisplay");
  writeXMLHeader(channelDisplayOut, "ChannelDisplay");
  
  // Write to meshStateOut and channelStateOut for each state file instance.
  for (stateInstance = 0; stateInstance < numberOfStateInstances; ++stateInstance)
    {
      // Read variables from state.nc.
      index[0] = stateInstance;
      nc_inq_varid(stateFileID, "geometryInstance", &dimensionVariableID);
      nc_get_var1_ulonglong(stateFileID, dimensionVariableID, index, &geometryInstance);
      nc_inq_varid(stateFileID, "parameterInstance", &dimensionVariableID);
      nc_get_var1_ulonglong(stateFileID, dimensionVariableID, index, &parameterInstance);
      nc_inq_varid(stateFileID, "currentTime", &dimensionVariableID);
      nc_get_var1_double(stateFileID, dimensionVariableID, index, &currentTime);
      
      // Read variables from geometry.nc.
      index[0] = geometryInstance;
      nc_inq_varid(geometryFileID, "numberOfMeshNodes", &dimensionVariableID);
      nc_get_var1_int(geometryFileID, dimensionVariableID, index, &numberOfMeshNodes);
      nc_inq_varid(geometryFileID, "numberOfMeshElements", &dimensionVariableID);
      nc_get_var1_int(geometryFileID, dimensionVariableID, index, &numberOfMeshElements);
      nc_inq_varid(geometryFileID, "numberOfChannelNodes", &dimensionVariableID);
      nc_get_var1_int(geometryFileID, dimensionVariableID, index, &numberOfChannelNodes);
      nc_inq_varid(geometryFileID, "numberOfChannelElements", &dimensionVariableID);
      nc_get_var1_int(geometryFileID, dimensionVariableID, index, &numberOfChannelElements);
      
      // Write mesh metadata.
      writeGridHeader(meshStateOut, currentTime);
      writeTopologyAndGeometry(meshStateOut, "geometry.nc", "Triangle", "meshElementVertices", "meshNodeX", "meshNodeY", "meshNodeZSurface",
                               numberOfMeshNodes, numberOfMeshElements, meshMeshNeighborsSize, geometryInstance, numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshElementX",                         "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshElementY",                         "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshElementZSurface",                  "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshElementSoilDepth",                 "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshElementLayerZBottom",              "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshElementArea",                      "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshElementSlopeX",                    "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshElementSlopeY",                    "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshLatitude",                         "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "geometry.nc",  "meshLongitude",                        "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshRegion",                           "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshCatchment",                        "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshVegetationType",                   "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshSoilType",                         "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshAlluvium",                         "Int",   1, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshManningsN",                        "Float", 8, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshConductivity",                     "Float", 8, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshPorosity",                         "Float", 8, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "parameter.nc", "meshGroundwaterMethod",                "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshSurfacewaterDepth",                "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshSurfacewaterError",                "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshGroundwaterHead",                  "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshGroundwaterRecharge",              "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshGroundwaterError",                 "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshPrecipitationRate",                "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshPrecipitationCumulativeShortTerm", "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshPrecipitationCumulativeLongTerm",  "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshEvaporationRate",                  "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshEvaporationCumulativeShortTerm",   "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshEvaporationCumulativeLongTerm",    "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshTranspirationRate",                "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshTranspirationCumulativeShortTerm", "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(meshStateOut, "state.nc",     "meshTranspirationCumulativeLongTerm",  "Float", 8, numberOfMeshElements, stateInstance,     numberOfStateInstances);
      writeGridFooter(meshStateOut);

      // Write channel metadata.
      writeGridHeader(channelStateOut, currentTime);
      writeTopologyAndGeometry(channelStateOut, "geometry.nc", "Mixed", "channelElementVertices", "channelNodeX", "channelNodeY", "channelNodeZBank",
                               numberOfChannelNodes, numberOfChannelElements, channelVerticesXDMFSize, geometryInstance, numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "geometry.nc",  "channelElementX",                         "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "geometry.nc",  "channelElementY",                         "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "geometry.nc",  "channelElementZBank",                     "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "geometry.nc",  "channelElementBankFullDepth",             "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "geometry.nc",  "channelElementZBed",                      "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "geometry.nc",  "channelElementLength",                    "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "geometry.nc",  "channelLatitude",                         "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "geometry.nc",  "channelLongitude",                        "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelStateOut, "parameter.nc", "channelRegion",                           "Int",   4, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelStateOut, "parameter.nc", "channelChannelType",                      "Int",   4, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelStateOut, "parameter.nc", "channelReachCode",                        "Int",   8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelStateOut, "parameter.nc", "channelBaseWidth",                        "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelStateOut, "parameter.nc", "channelSideSlope",                        "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelStateOut, "parameter.nc", "channelBedConductivity",                  "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelStateOut, "parameter.nc", "channelBedThickness",                     "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelStateOut, "parameter.nc", "channelManningsN",                        "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelStateOut, "state.nc",     "channelSurfacewaterDepth",                "Float", 8, numberOfChannelElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(channelStateOut, "state.nc",     "channelSurfacewaterError",                "Float", 8, numberOfChannelElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(channelStateOut, "state.nc",     "channelPrecipitationRate",                "Float", 8, numberOfChannelElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(channelStateOut, "state.nc",     "channelPrecipitationCumulativeShortTerm", "Float", 8, numberOfChannelElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(channelStateOut, "state.nc",     "channelPrecipitationCumulativeLongTerm",  "Float", 8, numberOfChannelElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(channelStateOut, "state.nc",     "channelEvaporationRate",                  "Float", 8, numberOfChannelElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(channelStateOut, "state.nc",     "channelEvaporationCumulativeShortTerm",   "Float", 8, numberOfChannelElements, stateInstance,     numberOfStateInstances);
      writeScalarAttribute(channelStateOut, "state.nc",     "channelEvaporationCumulativeLongTerm",    "Float", 8, numberOfChannelElements, stateInstance,     numberOfStateInstances);
      writeGridFooter(channelStateOut);
    } // End write to meshStateOut and channelStateOut for each state file instance.
  
  // Write to meshDisplayOut and channelDisplayOut for each display file instance.
  for (displayInstance = 0; displayInstance < numberOfDisplayInstances; ++displayInstance)
    {
      // Read variables from display.nc.
      index[0] = displayInstance;
      nc_inq_varid(displayFileID, "geometryInstance", &dimensionVariableID);
      nc_get_var1_ulonglong(displayFileID, dimensionVariableID, index, &geometryInstance);
      nc_inq_varid(displayFileID, "parameterInstance", &dimensionVariableID);
      nc_get_var1_ulonglong(displayFileID, dimensionVariableID, index, &parameterInstance);
      nc_inq_varid(displayFileID, "currentTime", &dimensionVariableID);
      nc_get_var1_double(displayFileID, dimensionVariableID, index, &currentTime);
      
      // Read variables from geometry.nc.
      index[0] = geometryInstance;
      nc_inq_varid(geometryFileID, "numberOfMeshNodes", &dimensionVariableID);
      nc_get_var1_int(geometryFileID, dimensionVariableID, index, &numberOfMeshNodes);
      nc_inq_varid(geometryFileID, "numberOfMeshElements", &dimensionVariableID);
      nc_get_var1_int(geometryFileID, dimensionVariableID, index, &numberOfMeshElements);
      nc_inq_varid(geometryFileID, "numberOfChannelNodes", &dimensionVariableID);
      nc_get_var1_int(geometryFileID, dimensionVariableID, index, &numberOfChannelNodes);
      nc_inq_varid(geometryFileID, "numberOfChannelElements", &dimensionVariableID);
      nc_get_var1_int(geometryFileID, dimensionVariableID, index, &numberOfChannelElements);
      
      // Write mesh metadata.
      writeGridHeader(meshDisplayOut, currentTime);
      writeTopologyAndGeometry(meshDisplayOut, "geometry.nc", "Triangle", "meshElementVertices", "meshNodeX", "meshNodeY", "meshNodeZSurface",
                               numberOfMeshNodes, numberOfMeshElements, meshMeshNeighborsSize, geometryInstance, numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshElementX",                         "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshElementY",                         "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshElementZSurface",                  "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshElementSoilDepth",                 "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshElementLayerZBottom",              "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshElementArea",                      "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshElementSlopeX",                    "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshElementSlopeY",                    "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshLatitude",                         "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "geometry.nc",  "meshLongitude",                        "Float", 8, numberOfMeshElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshRegion",                           "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshCatchment",                        "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshVegetationType",                   "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshSoilType",                         "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshAlluvium",                         "Int",   1, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshManningsN",                        "Float", 8, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshConductivity",                     "Float", 8, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshPorosity",                         "Float", 8, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "parameter.nc", "meshGroundwaterMethod",                "Int",   4, numberOfMeshElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshSurfacewaterDepth",                "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshSurfacewaterError",                "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshGroundwaterHead",                  "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshGroundwaterRecharge",              "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshGroundwaterError",                 "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshPrecipitationRate",                "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshPrecipitationCumulativeShortTerm", "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshPrecipitationCumulativeLongTerm",  "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshEvaporationRate",                  "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshEvaporationCumulativeShortTerm",   "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshEvaporationCumulativeLongTerm",    "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshTranspirationRate",                "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshTranspirationCumulativeShortTerm", "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshTranspirationCumulativeLongTerm",  "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshCanopyWater",                      "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(meshDisplayOut, "display.nc",   "meshSnowWaterEquivalent",              "Float", 8, numberOfMeshElements, displayInstance,   numberOfDisplayInstances);
      writeGridFooter(meshDisplayOut);

      // Write channel metadata.
      writeGridHeader(channelDisplayOut, currentTime);
      writeTopologyAndGeometry(channelDisplayOut, "geometry.nc", "Mixed", "channelElementVertices", "channelNodeX", "channelNodeY", "channelNodeZBank",
                               numberOfChannelNodes, numberOfChannelElements, channelVerticesXDMFSize, geometryInstance, numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "geometry.nc",  "channelElementX",                         "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "geometry.nc",  "channelElementY",                         "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "geometry.nc",  "channelElementZBank",                     "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "geometry.nc",  "channelElementBankFullDepth",             "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "geometry.nc",  "channelElementZBed",                      "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "geometry.nc",  "channelElementLength",                    "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "geometry.nc",  "channelLatitude",                         "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "geometry.nc",  "channelLongitude",                        "Float", 8, numberOfChannelElements, geometryInstance,  numberOfGeometryInstances);
      writeScalarAttribute(channelDisplayOut, "parameter.nc", "channelRegion",                           "Int",   4, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelDisplayOut, "parameter.nc", "channelChannelType",                      "Int",   4, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelDisplayOut, "parameter.nc", "channelReachCode",                        "Int",   8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelDisplayOut, "parameter.nc", "channelBaseWidth",                        "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelDisplayOut, "parameter.nc", "channelSideSlope",                        "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelDisplayOut, "parameter.nc", "channelBedConductivity",                  "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelDisplayOut, "parameter.nc", "channelBedThickness",                     "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelDisplayOut, "parameter.nc", "channelManningsN",                        "Float", 8, numberOfChannelElements, parameterInstance, numberOfParameterInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelSurfacewaterDepth",                "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelSurfacewaterError",                "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelPrecipitationRate",                "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelPrecipitationCumulativeShortTerm", "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelPrecipitationCumulativeLongTerm",  "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelEvaporationRate",                  "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelEvaporationCumulativeShortTerm",   "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelEvaporationCumulativeLongTerm",    "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeScalarAttribute(channelDisplayOut, "display.nc",   "channelSnowWaterEquivalent",              "Float", 8, numberOfChannelElements, displayInstance,   numberOfDisplayInstances);
      writeGridFooter(channelDisplayOut);
    } // End write to meshDisplayOut and channelDisplayOut for each display file instance.
  
  writeXMLFooter(meshStateOut);
  writeXMLFooter(channelStateOut);
  writeXMLFooter(meshDisplayOut);
  writeXMLFooter(channelDisplayOut);
  
  // Close XDMF output files.
  fclose(meshStateOut);
  fclose(channelStateOut);
  fclose(meshDisplayOut);
  fclose(channelDisplayOut);
  
  // Close NetCDF input files.
  nc_close(geometryFileID);
  nc_close(parameterFileID);
  nc_close(stateFileID);
  nc_close(displayFileID);
  
  return 0;
}
