#include "file_manager.h"
#include "all.h"

FileManager::FileManager()
{
  // Hard coded mesh.
  // FIXME read from NetCDF file.
  globalNumberOfMeshElements    = 8;
  localMeshElementStart         = 0;
  localNumberOfMeshElements     = 8;
  globalNumberOfChannelElements = 4;
  localChannelElementStart      = 0;
  localNumberOfChannelElements  = 4;
  
  meshVertexX = new double[globalNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  meshVertexX[0][0] =   0.0;
  meshVertexX[0][1] = -75.0;
  meshVertexX[0][2] = -75.0;
  meshVertexX[1][0] =   0.0;
  meshVertexX[1][1] = -75.0;
  meshVertexX[1][2] =   0.0;
  meshVertexX[2][0] =   0.0;
  meshVertexX[2][1] = -75.0;
  meshVertexX[2][2] = -75.0;
  meshVertexX[3][0] =   0.0;
  meshVertexX[3][1] = -75.0;
  meshVertexX[3][2] =   0.0;
  meshVertexX[4][0] =  75.0;
  meshVertexX[4][1] =   0.0;
  meshVertexX[4][2] =  75.0;
  meshVertexX[5][0] =   0.0;
  meshVertexX[5][1] =   0.0;
  meshVertexX[5][2] =  75.0;
  meshVertexX[6][0] =  75.0;
  meshVertexX[6][1] =   0.0;
  meshVertexX[6][2] =  75.0;
  meshVertexX[7][0] =   0.0;
  meshVertexX[7][1] =   0.0;
  meshVertexX[7][2] =  75.0;
  
  meshVertexY = new double[globalNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  meshVertexY[0][0] = 150.0;
  meshVertexY[0][1] = 150.0;
  meshVertexY[0][2] =  75.0;
  meshVertexY[1][0] = 150.0;
  meshVertexY[1][1] =  75.0;
  meshVertexY[1][2] =  75.0;
  meshVertexY[2][0] =  75.0;
  meshVertexY[2][1] =  75.0;
  meshVertexY[2][2] =   0.0;
  meshVertexY[3][0] =  75.0;
  meshVertexY[3][1] =   0.0;
  meshVertexY[3][2] =   0.0;
  meshVertexY[4][0] = 150.0;
  meshVertexY[4][1] = 150.0;
  meshVertexY[4][2] =  75.0;
  meshVertexY[5][0] = 150.0;
  meshVertexY[5][1] =  75.0;
  meshVertexY[5][2] =  75.0;
  meshVertexY[6][0] =  75.0;
  meshVertexY[6][1] =  75.0;
  meshVertexY[6][2] =   0.0;
  meshVertexY[7][0] =  75.0;
  meshVertexY[7][1] =   0.0;
  meshVertexY[7][2] =   0.0;
  
  meshVertexZSurface = new double[globalNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  meshVertexZSurface[0][0] =  15.0;
  meshVertexZSurface[0][1] =  25.0;
  meshVertexZSurface[0][2] =  17.5;
  meshVertexZSurface[1][0] =  15.0;
  meshVertexZSurface[1][1] =  17.5;
  meshVertexZSurface[1][2] =   7.5;
  meshVertexZSurface[2][0] =   7.5;
  meshVertexZSurface[2][1] =  17.5;
  meshVertexZSurface[2][2] =  10.0;
  meshVertexZSurface[3][0] =   7.5;
  meshVertexZSurface[3][1] =  10.0;
  meshVertexZSurface[3][2] =   0.0;
  meshVertexZSurface[4][0] =  25.0;
  meshVertexZSurface[4][1] =  15.0;
  meshVertexZSurface[4][2] =  17.5;
  meshVertexZSurface[5][0] =  15.0;
  meshVertexZSurface[5][1] =   7.5;
  meshVertexZSurface[5][2] =  17.5;
  meshVertexZSurface[6][0] =  17.5;
  meshVertexZSurface[6][1] =   7.5;
  meshVertexZSurface[6][2] =  10.0;
  meshVertexZSurface[7][0] =   7.5;
  meshVertexZSurface[7][1] =   0.0;
  meshVertexZSurface[7][2] =  10.0;
  
  meshVertexZBedrock = new double[globalNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  meshVertexZBedrock[0][0] =  10.0;
  meshVertexZBedrock[0][1] =  20.0;
  meshVertexZBedrock[0][2] =  12.5;
  meshVertexZBedrock[1][0] =  10.0;
  meshVertexZBedrock[1][1] =  12.5;
  meshVertexZBedrock[1][2] =   2.5;
  meshVertexZBedrock[2][0] =   2.5;
  meshVertexZBedrock[2][1] =  12.5;
  meshVertexZBedrock[2][2] =   5.0;
  meshVertexZBedrock[3][0] =   2.5;
  meshVertexZBedrock[3][1] =   5.0;
  meshVertexZBedrock[3][2] =  -5.0;
  meshVertexZBedrock[4][0] =  20.0;
  meshVertexZBedrock[4][1] =  10.0;
  meshVertexZBedrock[4][2] =  12.5;
  meshVertexZBedrock[5][0] =  10.0;
  meshVertexZBedrock[5][1] =   2.5;
  meshVertexZBedrock[5][2] =  12.5;
  meshVertexZBedrock[6][0] =  12.5;
  meshVertexZBedrock[6][1] =   2.5;
  meshVertexZBedrock[6][2] =   5.0;
  meshVertexZBedrock[7][0] =   2.5;
  meshVertexZBedrock[7][1] =  -5.0;
  meshVertexZBedrock[7][2] =   5.0;
  
  // All of these will be calcluated from the vertices.
  
  meshElementX        = NULL;
  meshElementY        = NULL;
  meshElementZSurface = NULL;
  meshElementZBedrock = NULL;
  meshElementArea     = NULL;
  meshElementSlopeX   = NULL;
  meshElementSlopeY   = NULL;
  
  
  meshCatchment = new int[globalNumberOfMeshElements];
  
  meshCatchment[0] = 1;
  meshCatchment[1] = 1;
  meshCatchment[2] = 1;
  meshCatchment[3] = 1;
  meshCatchment[4] = 1;
  meshCatchment[5] = 1;
  meshCatchment[6] = 1;
  meshCatchment[7] = 1;
  
  meshConductivity = new double[globalNumberOfMeshElements];
  
  meshConductivity[0] = 5.55e-4;
  meshConductivity[1] = 5.55e-4;
  meshConductivity[2] = 5.55e-4;
  meshConductivity[3] = 5.55e-4;
  meshConductivity[4] = 5.55e-4;
  meshConductivity[5] = 5.55e-4;
  meshConductivity[6] = 5.55e-4;
  meshConductivity[7] = 5.55e-4;
  
  meshPorosity = new double[globalNumberOfMeshElements];
  
  meshPorosity[0] = 0.5;
  meshPorosity[1] = 0.5;
  meshPorosity[2] = 0.5;
  meshPorosity[3] = 0.5;
  meshPorosity[4] = 0.5;
  meshPorosity[5] = 0.5;
  meshPorosity[6] = 0.5;
  meshPorosity[7] = 0.5;
  
  meshManningsN = new double[globalNumberOfMeshElements];
  
  meshManningsN[0] = 0.038;
  meshManningsN[1] = 0.038;
  meshManningsN[2] = 0.038;
  meshManningsN[3] = 0.038;
  meshManningsN[4] = 0.038;
  meshManningsN[5] = 0.038;
  meshManningsN[6] = 0.038;
  meshManningsN[7] = 0.038;
  
  meshSurfacewaterDepth =  new double[globalNumberOfMeshElements];
  
  meshSurfacewaterDepth[0] = 0.1;
  meshSurfacewaterDepth[1] = 0.1;
  meshSurfacewaterDepth[2] = 0.1;
  meshSurfacewaterDepth[3] = 0.1;
  meshSurfacewaterDepth[4] = 0.1;
  meshSurfacewaterDepth[5] = 0.1;
  meshSurfacewaterDepth[6] = 0.1;
  meshSurfacewaterDepth[7] = 0.1;
  
  meshSurfacewaterError = new double[globalNumberOfMeshElements];
  
  meshSurfacewaterError[0] = 0.0;
  meshSurfacewaterError[1] = 0.0;
  meshSurfacewaterError[2] = 0.0;
  meshSurfacewaterError[3] = 0.0;
  meshSurfacewaterError[4] = 0.0;
  meshSurfacewaterError[5] = 0.0;
  meshSurfacewaterError[6] = 0.0;
  meshSurfacewaterError[7] = 0.0;
  
  meshGroundwaterHead = new double[globalNumberOfMeshElements];
  
  meshGroundwaterHead[0] = (meshVertexZSurface[0][0] + meshVertexZSurface[0][1] + meshVertexZSurface[0][2]) / 3.0;
  meshGroundwaterHead[1] = (meshVertexZSurface[1][0] + meshVertexZSurface[1][1] + meshVertexZSurface[1][2]) / 3.0;
  meshGroundwaterHead[2] = (meshVertexZSurface[2][0] + meshVertexZSurface[2][1] + meshVertexZSurface[2][2]) / 3.0;
  meshGroundwaterHead[3] = (meshVertexZSurface[3][0] + meshVertexZSurface[3][1] + meshVertexZSurface[3][2]) / 3.0;
  meshGroundwaterHead[4] = (meshVertexZSurface[4][0] + meshVertexZSurface[4][1] + meshVertexZSurface[4][2]) / 3.0;
  meshGroundwaterHead[5] = (meshVertexZSurface[5][0] + meshVertexZSurface[5][1] + meshVertexZSurface[5][2]) / 3.0;
  meshGroundwaterHead[6] = (meshVertexZSurface[6][0] + meshVertexZSurface[6][1] + meshVertexZSurface[6][2]) / 3.0;
  meshGroundwaterHead[7] = (meshVertexZSurface[7][0] + meshVertexZSurface[7][1] + meshVertexZSurface[7][2]) / 3.0;
  
  meshGroundwaterError = new double[globalNumberOfMeshElements];
  
  meshGroundwaterError[0] = 0.0;
  meshGroundwaterError[1] = 0.0;
  meshGroundwaterError[2] = 0.0;
  meshGroundwaterError[3] = 0.0;
  meshGroundwaterError[4] = 0.0;
  meshGroundwaterError[5] = 0.0;
  meshGroundwaterError[6] = 0.0;
  meshGroundwaterError[7] = 0.0;
  
  meshMeshNeighbors = new int[globalNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  meshMeshNeighbors[0][0] = NOFLOW;
  meshMeshNeighbors[0][1] = 1;
  meshMeshNeighbors[0][2] = NOFLOW;
  meshMeshNeighbors[1][0] = 2;
  meshMeshNeighbors[1][1] = 5;
  meshMeshNeighbors[1][2] = 0;
  meshMeshNeighbors[2][0] = NOFLOW;
  meshMeshNeighbors[2][1] = 3;
  meshMeshNeighbors[2][2] = 1;
  meshMeshNeighbors[3][0] = NOFLOW;
  meshMeshNeighbors[3][1] = 7;
  meshMeshNeighbors[3][2] = 2;
  meshMeshNeighbors[4][0] = 5;
  meshMeshNeighbors[4][1] = NOFLOW;
  meshMeshNeighbors[4][2] = NOFLOW;
  meshMeshNeighbors[5][0] = 6;
  meshMeshNeighbors[5][1] = 4;
  meshMeshNeighbors[5][2] = 1;
  meshMeshNeighbors[6][0] = 7;
  meshMeshNeighbors[6][1] = NOFLOW;
  meshMeshNeighbors[6][2] = 5;
  meshMeshNeighbors[7][0] = NOFLOW;
  meshMeshNeighbors[7][1] = 6;
  meshMeshNeighbors[7][2] = 3;
  
  meshMeshNeighborsChannelEdge = new bool[globalNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  meshMeshNeighborsChannelEdge[0][0] = false;
  meshMeshNeighborsChannelEdge[0][1] = false;
  meshMeshNeighborsChannelEdge[0][2] = false;
  meshMeshNeighborsChannelEdge[1][0] = false;
  meshMeshNeighborsChannelEdge[1][1] = true;
  meshMeshNeighborsChannelEdge[1][2] = false;
  meshMeshNeighborsChannelEdge[2][0] = false;
  meshMeshNeighborsChannelEdge[2][1] = false;
  meshMeshNeighborsChannelEdge[2][2] = false;
  meshMeshNeighborsChannelEdge[3][0] = false;
  meshMeshNeighborsChannelEdge[3][1] = true;
  meshMeshNeighborsChannelEdge[3][2] = false;
  meshMeshNeighborsChannelEdge[4][0] = false;
  meshMeshNeighborsChannelEdge[4][1] = false;
  meshMeshNeighborsChannelEdge[4][2] = false;
  meshMeshNeighborsChannelEdge[5][0] = false;
  meshMeshNeighborsChannelEdge[5][1] = false;
  meshMeshNeighborsChannelEdge[5][2] = true;
  meshMeshNeighborsChannelEdge[6][0] = false;
  meshMeshNeighborsChannelEdge[6][1] = false;
  meshMeshNeighborsChannelEdge[6][2] = false;
  meshMeshNeighborsChannelEdge[7][0] = false;
  meshMeshNeighborsChannelEdge[7][1] = false;
  meshMeshNeighborsChannelEdge[7][2] = true;
  
  // All of these will be calcluated from the vertices.
  
  meshMeshNeighborsEdgeLength  = NULL;
  meshMeshNeighborsEdgeNormalX = NULL;
  meshMeshNeighborsEdgeNormalY = NULL;
  
  meshChannelNeighbors = new int[globalNumberOfMeshElements][MeshElement::channelNeighborsSize];
  
  meshChannelNeighbors[0][0] = 0;
  meshChannelNeighbors[0][1] = NOFLOW;
  meshChannelNeighbors[1][0] = 1;
  meshChannelNeighbors[1][1] = 2;
  meshChannelNeighbors[2][0] = NOFLOW;
  meshChannelNeighbors[2][1] = NOFLOW;
  meshChannelNeighbors[3][0] = 2;
  meshChannelNeighbors[3][1] = 3;
  meshChannelNeighbors[4][0] = 0;
  meshChannelNeighbors[4][1] = NOFLOW;
  meshChannelNeighbors[5][0] = 1;
  meshChannelNeighbors[5][1] = 2;
  meshChannelNeighbors[6][0] = NOFLOW;
  meshChannelNeighbors[6][1] = NOFLOW;
  meshChannelNeighbors[7][0] = 2;
  meshChannelNeighbors[7][1] = 3;
  
  meshChannelNeighborsEdgeLength = new double[globalNumberOfMeshElements][MeshElement::channelNeighborsSize];
  
  meshChannelNeighborsEdgeLength[0][0] = 75.0;
  meshChannelNeighborsEdgeLength[0][1] =  1.0;
  meshChannelNeighborsEdgeLength[1][0] = 50.0;
  meshChannelNeighborsEdgeLength[1][1] = 25.0;
  meshChannelNeighborsEdgeLength[2][0] =  1.0;
  meshChannelNeighborsEdgeLength[2][1] =  1.0;
  meshChannelNeighborsEdgeLength[3][0] = 25.0;
  meshChannelNeighborsEdgeLength[3][1] = 50.0;
  meshChannelNeighborsEdgeLength[4][0] = 75.0;
  meshChannelNeighborsEdgeLength[4][1] =  1.0;
  meshChannelNeighborsEdgeLength[5][0] = 50.0;
  meshChannelNeighborsEdgeLength[5][1] = 25.0;
  meshChannelNeighborsEdgeLength[6][0] =  1.0;
  meshChannelNeighborsEdgeLength[6][1] =  1.0;
  meshChannelNeighborsEdgeLength[7][0] = 25.0;
  meshChannelNeighborsEdgeLength[7][1] = 50.0;
  
  channelElementX = new double[globalNumberOfChannelElements];
  
  channelElementX[0] = 0.0;
  channelElementX[1] = 0.0;
  channelElementX[2] = 0.0;
  channelElementX[3] = 0.0;
  
  channelElementY = new double[globalNumberOfChannelElements];
  
  channelElementY[0] = 175.0;
  channelElementY[1] = 125.0;
  channelElementY[2] =  75.0;
  channelElementY[3] =  25.0;
  
  channelElementZBank = new double[globalNumberOfChannelElements];
  
  channelElementZBank[0] = 15.0;
  channelElementZBank[1] = 12.5;
  channelElementZBank[2] =  7.5;
  channelElementZBank[3] =  2.5;
  
  channelElementZBed = new double[globalNumberOfChannelElements];
  
  channelElementZBed[0] = 12.5;
  channelElementZBed[1] = 10.0;
  channelElementZBed[2] =  5.0;
  channelElementZBed[3] =  0.0;
  
  channelElementLength = new double[globalNumberOfChannelElements];
  
  channelElementLength[0] = 50.0;
  channelElementLength[1] = 50.0;
  channelElementLength[2] = 50.0;
  channelElementLength[3] = 50.0;
  
  channelChannelType = new ChannelTypeEnum[globalNumberOfChannelElements];
  
  channelChannelType[0] = WATERBODY;
  channelChannelType[1] = STREAM;
  channelChannelType[2] = STREAM;
  channelChannelType[3] = STREAM;
  
  channelPermanentCode = new int[globalNumberOfChannelElements];
  
  channelPermanentCode[0] = 1;
  channelPermanentCode[1] = 1;
  channelPermanentCode[2] = 1;
  channelPermanentCode[3] = 1;
  
  channelBaseWidth = new double[globalNumberOfChannelElements];
  
  channelBaseWidth[0] = 150.0;
  channelBaseWidth[1] =   1.0;
  channelBaseWidth[2] =   1.0;
  channelBaseWidth[3] =   1.0;
  
  channelSideSlope = new double[globalNumberOfChannelElements];
  
  channelSideSlope[0] = 1.0;
  channelSideSlope[1] = 1.0;
  channelSideSlope[2] = 1.0;
  channelSideSlope[3] = 1.0;
  
  channelBedConductivity = new double[globalNumberOfChannelElements];
  
  channelBedConductivity[0] = 5.55e-4;
  channelBedConductivity[1] = 5.55e-4;
  channelBedConductivity[2] = 5.55e-4;
  channelBedConductivity[3] = 5.55e-4;
  
  channelBedThickness = new double[globalNumberOfChannelElements];
  
  channelBedThickness[0] = 1.0;
  channelBedThickness[1] = 1.0;
  channelBedThickness[2] = 1.0;
  channelBedThickness[3] = 1.0;
  
  channelManningsN = new double[globalNumberOfChannelElements];
  
  channelManningsN[0] = 0.038;
  channelManningsN[1] = 0.038;
  channelManningsN[2] = 0.038;
  channelManningsN[3] = 0.038;
  
  channelSurfacewaterDepth = new double[globalNumberOfChannelElements];
  
  channelSurfacewaterDepth[0] = 0.0;
  channelSurfacewaterDepth[1] = 0.0;
  channelSurfacewaterDepth[2] = 0.0;
  channelSurfacewaterDepth[3] = 0.0;
  
  channelSurfacewaterError = new double[globalNumberOfChannelElements];
  
  channelSurfacewaterError[0] = 0.0;
  channelSurfacewaterError[1] = 0.0;
  channelSurfacewaterError[2] = 0.0;
  channelSurfacewaterError[3] = 0.0;
  
  channelChannelNeighbors = new int[globalNumberOfChannelElements][ChannelElement::channelNeighborsSize];
  
  channelChannelNeighbors[0][0] = 1;
  channelChannelNeighbors[0][1] = NOFLOW;
  channelChannelNeighbors[1][0] = 0;
  channelChannelNeighbors[1][1] = 2;
  channelChannelNeighbors[2][0] = 1;
  channelChannelNeighbors[2][1] = 3;
  channelChannelNeighbors[3][0] = 2;
  channelChannelNeighbors[3][1] = OUTFLOW;
  
  channelMeshNeighbors = new int[globalNumberOfChannelElements][ChannelElement::meshNeighborsSize];
  
  channelMeshNeighbors[0][0] = 0;
  channelMeshNeighbors[0][1] = 4;
  channelMeshNeighbors[0][2] = NOFLOW;
  channelMeshNeighbors[0][3] = NOFLOW;
  channelMeshNeighbors[1][0] = 1;
  channelMeshNeighbors[1][1] = 5;
  channelMeshNeighbors[1][2] = NOFLOW;
  channelMeshNeighbors[1][3] = NOFLOW;
  channelMeshNeighbors[2][0] = 1;
  channelMeshNeighbors[2][1] = 5;
  channelMeshNeighbors[2][2] = 3;
  channelMeshNeighbors[2][3] = 7;
  channelMeshNeighbors[3][0] = 3;
  channelMeshNeighbors[3][1] = 7;
  channelMeshNeighbors[3][2] = NOFLOW;
  channelMeshNeighbors[3][3] = NOFLOW;
  
  channelMeshNeighborsEdgeLength = new double[globalNumberOfChannelElements][ChannelElement::meshNeighborsSize];
  
  channelMeshNeighborsEdgeLength[0][0] = 75.0;
  channelMeshNeighborsEdgeLength[0][1] = 75.0;
  channelMeshNeighborsEdgeLength[0][2] =  1.0;
  channelMeshNeighborsEdgeLength[0][3] =  1.0;
  channelMeshNeighborsEdgeLength[1][0] = 50.0;
  channelMeshNeighborsEdgeLength[1][1] = 50.0;
  channelMeshNeighborsEdgeLength[1][2] =  1.0;
  channelMeshNeighborsEdgeLength[1][3] =  1.0;
  channelMeshNeighborsEdgeLength[2][0] = 25.0;
  channelMeshNeighborsEdgeLength[2][1] = 25.0;
  channelMeshNeighborsEdgeLength[2][2] = 25.0;
  channelMeshNeighborsEdgeLength[2][3] = 25.0;
  channelMeshNeighborsEdgeLength[3][0] = 50.0;
  channelMeshNeighborsEdgeLength[3][1] = 50.0;
  channelMeshNeighborsEdgeLength[3][2] =  1.0;
  channelMeshNeighborsEdgeLength[3][3] =  1.0;
}

#include "file_manager.def.h"
