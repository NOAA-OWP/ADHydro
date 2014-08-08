#include "file_manager.h"
#include "all.h"
#include <netcdf.h>
#include <netcdf_par.h>

int FileManager::home(int item, int globalNumberOfItems)
{
  int numPes              = CkNumPes();                           // Number of processors.
  int numberOfFatOwners   = globalNumberOfItems % numPes;         // Number of file managers that own one extra item.
  int itemsPerFatOwner    = globalNumberOfItems / numPes + 1;     // Number of items in each file manager that owns one extra item.
  int itemsInAllFatOwners = numberOfFatOwners * itemsPerFatOwner; // Total number of items in all file managers that own one extra item.
  int itemsPerThinOwner   = globalNumberOfItems / numPes;         // Number of items in each file manager that does not own one extra item.
  int itemHome;                                                   // The index of the file manager that owns item.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= item && item < globalNumberOfItems))
    {
      CkError("ERROR in FileManager::home: item must be greater than or equal to zero and less than globalNumberOfItems.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (item < itemsInAllFatOwners)
    {
      // Item is owned by a fat owner.
      itemHome = item / itemsPerFatOwner;
    }
  else
    {
      // Item is owned by a thin owner.
      itemHome = (item - itemsInAllFatOwners) / itemsPerThinOwner + numberOfFatOwners;
    }
  
  return itemHome;
}

FileManager::FileManager(size_t directorySize, char* directory, int geometryGroup, int parameterGroup, int stateGroup, double time, double dt)
{
  bool error = false; // Error flag.
  
  // Hard coded mesh.
  // FIXME read from NetCDF file.
  globalNumberOfMeshNodes       = 13;
  globalNumberOfMeshElements    = 8;
  globalNumberOfChannelNodes    = 13;
  globalNumberOfChannelElements = 4;
  
  localStartAndNumber(&localMeshNodeStart,       &localNumberOfMeshNodes,       globalNumberOfMeshNodes);
  localStartAndNumber(&localMeshElementStart,    &localNumberOfMeshElements,    globalNumberOfMeshElements);
  localStartAndNumber(&localChannelNodeStart,    &localNumberOfChannelNodes,    globalNumberOfChannelNodes);
  localStartAndNumber(&localChannelElementStart, &localNumberOfChannelElements, globalNumberOfChannelElements);
  
  meshNodeX = new double[globalNumberOfMeshNodes];
  
  meshNodeX[0]  = -75.0;
  meshNodeX[1]  =  75.0;
  meshNodeX[2]  = -75.0;
  meshNodeX[3]  =   0.0;
  meshNodeX[4]  =  75.0;
  meshNodeX[5]  =   0.0;
  meshNodeX[6]  = -75.0;
  meshNodeX[7]  =   0.0;
  meshNodeX[8]  =  75.0;
  meshNodeX[9]  =   0.0;
  meshNodeX[10] = -75.0;
  meshNodeX[11] =   0.0;
  meshNodeX[12] =  75.0;
  
  meshNodeY = new double[globalNumberOfMeshNodes];
  
  meshNodeY[0]  = 200.0;
  meshNodeY[1]  = 200.0;
  meshNodeY[2]  = 150.0;
  meshNodeY[3]  = 150.0;
  meshNodeY[4]  = 150.0;
  meshNodeY[5]  = 100.0;
  meshNodeY[6]  =  75.0;
  meshNodeY[7]  =  75.0;
  meshNodeY[8]  =  75.0;
  meshNodeY[9]  =  50.0;
  meshNodeY[10] =   0.0;
  meshNodeY[11] =   0.0;
  meshNodeY[12] =   0.0;
  
  meshNodeZSurface = new double[globalNumberOfMeshNodes];
  
  meshNodeZSurface[0]  =  30.0;
  meshNodeZSurface[1]  =  30.0;
  meshNodeZSurface[2]  =  25.0;
  meshNodeZSurface[3]  =  15.0;
  meshNodeZSurface[4]  =  25.0;
  meshNodeZSurface[5]  =  10.0;
  meshNodeZSurface[6]  =  17.5;
  meshNodeZSurface[7]  =   7.5;
  meshNodeZSurface[8]  =  17.5;
  meshNodeZSurface[9]  =   5.0;
  meshNodeZSurface[10] =  10.0;
  meshNodeZSurface[11] =   0.0;
  meshNodeZSurface[12] =  10.0;
  
  meshNodeZBedrock = new double[globalNumberOfMeshNodes];
  
  meshNodeZBedrock[0]  =  25.0;
  meshNodeZBedrock[1]  =  25.0;
  meshNodeZBedrock[2]  =  20.0;
  meshNodeZBedrock[3]  =  10.0;
  meshNodeZBedrock[4]  =  20.0;
  meshNodeZBedrock[5]  =   5.0;
  meshNodeZBedrock[6]  =  12.5;
  meshNodeZBedrock[7]  =   2.5;
  meshNodeZBedrock[8]  =  12.5;
  meshNodeZBedrock[9]  =   0.0;
  meshNodeZBedrock[10] =   5.0;
  meshNodeZBedrock[11] =  -5.0;
  meshNodeZBedrock[12] =   5.0;
  
  meshElementVertices = new int[globalNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  meshElementVertices[0][0] =  3;
  meshElementVertices[0][1] =  2;
  meshElementVertices[0][2] =  6;
  meshElementVertices[1][0] =  3;
  meshElementVertices[1][1] =  6;
  meshElementVertices[1][2] =  7;
  meshElementVertices[2][0] =  7;
  meshElementVertices[2][1] =  6;
  meshElementVertices[2][2] = 10;
  meshElementVertices[3][0] =  7;
  meshElementVertices[3][1] = 10;
  meshElementVertices[3][2] = 11;
  meshElementVertices[4][0] =  4;
  meshElementVertices[4][1] =  3;
  meshElementVertices[4][2] =  8;
  meshElementVertices[5][0] =  3;
  meshElementVertices[5][1] =  7;
  meshElementVertices[5][2] =  8;
  meshElementVertices[6][0] =  8;
  meshElementVertices[6][1] =  7;
  meshElementVertices[6][2] = 12;
  meshElementVertices[7][0] =  7;
  meshElementVertices[7][1] = 11;
  meshElementVertices[7][2] = 12;
  
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
  
  channelNodeX = new double[globalNumberOfChannelNodes];
  
  channelNodeX[0]  = -75.0;
  channelNodeX[1]  =  75.0;
  channelNodeX[2]  = -75.0;
  channelNodeX[3]  =   0.0;
  channelNodeX[4]  =  75.0;
  channelNodeX[5]  =   0.0;
  channelNodeX[6]  = -75.0;
  channelNodeX[7]  =   0.0;
  channelNodeX[8]  =  75.0;
  channelNodeX[9]  =   0.0;
  channelNodeX[10] = -75.0;
  channelNodeX[11] =   0.0;
  channelNodeX[12] =  75.0;
  
  channelNodeY = new double[globalNumberOfChannelNodes];
  
  channelNodeY[0]  = 200.0;
  channelNodeY[1]  = 200.0;
  channelNodeY[2]  = 150.0;
  channelNodeY[3]  = 150.0;
  channelNodeY[4]  = 150.0;
  channelNodeY[5]  = 100.0;
  channelNodeY[6]  =  75.0;
  channelNodeY[7]  =  75.0;
  channelNodeY[8]  =  75.0;
  channelNodeY[9]  =  50.0;
  channelNodeY[10] =   0.0;
  channelNodeY[11] =   0.0;
  channelNodeY[12] =   0.0;
  
  channelNodeZBank = new double[globalNumberOfChannelNodes];
  
  channelNodeZBank[0]  =  30.0;
  channelNodeZBank[1]  =  30.0;
  channelNodeZBank[2]  =  25.0;
  channelNodeZBank[3]  =  15.0;
  channelNodeZBank[4]  =  25.0;
  channelNodeZBank[5]  =  10.0;
  channelNodeZBank[6]  =  17.5;
  channelNodeZBank[7]  =   7.5;
  channelNodeZBank[8]  =  17.5;
  channelNodeZBank[9]  =   5.0;
  channelNodeZBank[10] =  10.0;
  channelNodeZBank[11] =   0.0;
  channelNodeZBank[12] =  10.0;
  
  channelNodeZBed = new double[globalNumberOfChannelNodes];
  
  channelNodeZBed[0]  =  25.0;
  channelNodeZBed[1]  =  25.0;
  channelNodeZBed[2]  =  20.0;
  channelNodeZBed[3]  =  10.0;
  channelNodeZBed[4]  =  20.0;
  channelNodeZBed[5]  =   5.0;
  channelNodeZBed[6]  =  12.5;
  channelNodeZBed[7]  =   2.5;
  channelNodeZBed[8]  =  12.5;
  channelNodeZBed[9]  =   0.0;
  channelNodeZBed[10] =   5.0;
  channelNodeZBed[11] =  -5.0;
  channelNodeZBed[12] =   5.0;
  
  channelElementVertices = new int[globalNumberOfChannelElements][ChannelElement::channelVerticesSize + 2];
  
  channelElementVertices[0][0] =  3;
  channelElementVertices[0][1] =  5;
  channelElementVertices[0][2] =  0;
  channelElementVertices[0][3] =  2;
  channelElementVertices[0][4] =  3;
  channelElementVertices[0][5] =  4;
  channelElementVertices[0][6] =  1;
  channelElementVertices[1][0] =  2;
  channelElementVertices[1][1] =  5;
  channelElementVertices[1][2] =  3;
  channelElementVertices[1][3] =  5;
  channelElementVertices[1][4] =  5;
  channelElementVertices[1][5] =  5;
  channelElementVertices[1][6] =  5;
  channelElementVertices[2][0] =  2;
  channelElementVertices[2][1] =  5;
  channelElementVertices[2][2] =  5;
  channelElementVertices[2][3] =  9;
  channelElementVertices[2][4] =  9;
  channelElementVertices[2][5] =  9;
  channelElementVertices[2][6] =  9;
  channelElementVertices[3][0] =  2;
  channelElementVertices[3][1] =  5;
  channelElementVertices[3][2] =  9;
  channelElementVertices[3][3] = 11;
  channelElementVertices[3][4] = 11;
  channelElementVertices[3][5] = 11;
  channelElementVertices[3][6] = 11;
  
  channelVertexX         = NULL;
  channelVertexY         = NULL;
  channelVertexZBank     = NULL;
  channelVertexZBed      = NULL;
  
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
  
  meshElementUpdated    = new bool[localNumberOfMeshElements];
  channelElementUpdated = new bool[localNumberOfChannelElements];
  
  error = writeGeometry(directory, geometryGroup, true);
  
  if (!error)
    {
      error = writeParameter(directory, parameterGroup, true);
    }
  
  if (!error)
    {
      error = writeState(directory, stateGroup, true, time, dt, geometryGroup, parameterGroup);
    }
  
  if (error)
    {
      CkExit();
    }
}

void FileManager::localStartAndNumber(int* localItemStart, int* localNumberOfItems, int globalNumberOfItems)
{
  int numPes              = CkNumPes();                           // Number of processors.
  int myPe                = CkMyPe();                             // The processor of this file manager.
  int numberOfFatOwners   = globalNumberOfItems % numPes;         // Number of file managers that own one extra item.
  int itemsPerFatOwner    = globalNumberOfItems / numPes + 1;     // Number of items in each file manager that owns one extra item.
  int itemsInAllFatOwners = numberOfFatOwners * itemsPerFatOwner; // Total number of items in all file managers that own one extra item.
  int itemsPerThinOwner   = globalNumberOfItems / numPes;         // Number of items in each file manager that does not own one extra item.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(NULL != localItemStart && NULL != localNumberOfItems && 0 < globalNumberOfItems);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (myPe < numberOfFatOwners)
    {
      // I am a fat owner.
      *localItemStart     = myPe * itemsPerFatOwner;
      *localNumberOfItems = itemsPerFatOwner;
    }
  else
    {
      // I am a thin owner.
      *localItemStart     = (myPe - numberOfFatOwners) * itemsPerThinOwner + itemsInAllFatOwners;
      *localNumberOfItems = itemsPerThinOwner;
    }
}

bool FileManager::allUpdated()
{
  int  ii;             // Loop counter.
  bool updated = true; // Flag to record whether we have found an unupdated element.
  
  for (ii = 0; updated && ii < localNumberOfMeshElements; ii++)
  {
    updated = meshElementUpdated[ii];
  }
  
  for (ii = 0; updated && ii < localNumberOfChannelElements; ii++)
  {
    updated = channelElementUpdated[ii];
  }
  
  return updated;
}

bool FileManager::writeGeometry(const char* directory, int group, bool create)
{
  bool   error      = false;                     // Error flag.
  char*  nameString = NULL;                      // Temporary string for file and group names.
  size_t nameStringSize;                         // Size of buffer allocated for nameString.
  size_t numPrinted;                             // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;                            // Return value of NetCDF functions.
  int    fileID;                                 // ID of NetCDF file.
  bool   fileOpen   = false;                     // Whether fileID refers to an open file.
  int    groupID;                                // ID of group in NetCDF file.
  int    numberOfMeshNodesDimID;                 // ID of dimension in NetCDF file.
  int    numberOfMeshElementsDimID;              // ID of dimension in NetCDF file.
  int    numberOfMeshMeshNeighborsDimID;         // ID of dimension in NetCDF file.
  int    numberOfMeshChannelNeighborsDimID;      // ID of dimension in NetCDF file.
  int    numberOfChannelNodesDimID;              // ID of dimension in NetCDF file.
  int    numberOfChannelElementsDimID;           // ID of dimension in NetCDF file.
  int    sizeOfChannelElementVerticesArrayDimID; // ID of dimension in NetCDF file.
  int    numberOfChannelVerticesDimID;           // ID of dimension in NetCDF file.
  int    numberOfChannelChannelNeighborsDimID;   // ID of dimension in NetCDF file.
  int    numberOfChannelMeshNeighborsDimID;      // ID of dimension in NetCDF file.
  int    dimIDs[NC_MAX_VAR_DIMS];                // For passing dimension IDs.
  int    meshNodeXVarID;                         // ID of variable in NetCDF file.
  int    meshNodeYVarID;                         // ID of variable in NetCDF file.
  int    meshNodeZSurfaceVarID;                  // ID of variable in NetCDF file.
  int    meshNodeZBedrockVarID;                  // ID of variable in NetCDF file.
  int    meshElementVerticesVarID;               // ID of variable in NetCDF file.
  int    meshVertexXVarID;                       // ID of variable in NetCDF file.
  int    meshVertexYVarID;                       // ID of variable in NetCDF file.
  int    meshVertexZSurfaceVarID;                // ID of variable in NetCDF file.
  int    meshVertexZBedrockVarID;                // ID of variable in NetCDF file.
  int    meshElementXVarID;                      // ID of variable in NetCDF file.
  int    meshElementYVarID;                      // ID of variable in NetCDF file.
  int    meshElementZSurfaceVarID;               // ID of variable in NetCDF file.
  int    meshElementZBedrockVarID;               // ID of variable in NetCDF file.
  int    meshElementAreaVarID;                   // ID of variable in NetCDF file.
  int    meshElementSlopeXVarID;                 // ID of variable in NetCDF file.
  int    meshElementSlopeYVarID;                 // ID of variable in NetCDF file.
  int    meshMeshNeighborsVarID;                 // ID of variable in NetCDF file.
  int    meshMeshNeighborsChannelEdgeVarID;      // ID of variable in NetCDF file.
  int    meshMeshNeighborsEdgeLengthVarID;       // ID of variable in NetCDF file.
  int    meshMeshNeighborsEdgeNormalXVarID;      // ID of variable in NetCDF file.
  int    meshMeshNeighborsEdgeNormalYVarID;      // ID of variable in NetCDF file.
  int    meshChannelNeighborsVarID;              // ID of variable in NetCDF file.
  int    meshChannelNeighborsEdgeLengthVarID;    // ID of variable in NetCDF file.
  int    channelNodeXVarID;                      // ID of variable in NetCDF file.
  int    channelNodeYVarID;                      // ID of variable in NetCDF file.
  int    channelNodeZBankVarID;                  // ID of variable in NetCDF file.
  int    channelNodeZBedVarID;                   // ID of variable in NetCDF file.
  int    channelElementVerticesVarID;            // ID of variable in NetCDF file.
  int    channelVertexXVarID;                    // ID of variable in NetCDF file.
  int    channelVertexYVarID;                    // ID of variable in NetCDF file.
  int    channelVertexZBankVarID;                // ID of variable in NetCDF file.
  int    channelVertexZBedVarID;                 // ID of variable in NetCDF file.
  int    channelElementXVarID;                   // ID of variable in NetCDF file.
  int    channelElementYVarID;                   // ID of variable in NetCDF file.
  int    channelElementZBankVarID;               // ID of variable in NetCDF file.
  int    channelElementZBedVarID;                // ID of variable in NetCDF file.
  int    channelElementLengthVarID;              // ID of variable in NetCDF file.
  int    channelChannelNeighborsVarID;           // ID of variable in NetCDF file.
  int    channelMeshNeighborsVarID;              // ID of variable in NetCDF file.
  int    channelMeshNeighborsEdgeLengthVarID;    // ID of variable in NetCDF file.
  size_t start[NC_MAX_VAR_DIMS];                 // For specifying subarrays when writing to NetCDF file.
  size_t count[NC_MAX_VAR_DIMS];                 // For specifying subarrays when writing to NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::writeGeometry: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file and group name strings
      nameStringSize = strlen(directory) + strlen("/geometry.nc") + 1; // This will also have enough space to hold the group name because the maximum length
                                                                       // of an int printed as a string is 11, which is shorter than the length of
                                                                       // "/geometry.nc", which is 13.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // FIXME make directory?

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeGeometry: incorrect return value of snprintf when generating file name.  %d should be %d and less than %d.\n",
                  numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create or open file.
  if (!error)
    {
      if (create)
        {
          ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
        }
      else
        {
          ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to %s NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  create ? "create" : "open", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }

  // Create group name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%d", group);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeGeometry: incorrect return value of snprintf when generating group name.  "
                  "%d should be greater than 0 and less than %d.\n", numPrinted, nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create group.
  if (!error)
    {
      ncErrorCode = nc_def_grp(fileID, nameString, &groupID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create group %s in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshNodes", globalNumberOfMeshNodes, &numberOfMeshNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfMeshNodes in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshElements", globalNumberOfMeshElements, &numberOfMeshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfMeshElements in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshMeshNeighbors", MeshElement::meshNeighborsSize, &numberOfMeshMeshNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfMeshMeshNeighbors in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshChannelNeighbors", MeshElement::channelNeighborsSize, &numberOfMeshChannelNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfMeshChannelNeighbors in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelNodes", globalNumberOfChannelNodes, &numberOfChannelNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfChannelNodes in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelElements", globalNumberOfChannelElements, &numberOfChannelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfChannelElements in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "sizeOfChannelElementVerticesArray", ChannelElement::channelVerticesSize + 2, &sizeOfChannelElementVerticesArrayDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension sizeOfChannelElementVerticesArray in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelVertices", ChannelElement::channelVerticesSize, &numberOfChannelVerticesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfChannelVertices in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelChannelNeighbors", ChannelElement::channelNeighborsSize, &numberOfChannelChannelNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfChannelChannelNeighbors in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelMeshNeighbors", ChannelElement::meshNeighborsSize, &numberOfChannelMeshNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension numberOfChannelMeshNeighbors in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error && NULL != meshNodeX)
    {
      dimIDs[0]   = numberOfMeshNodesDimID;
      ncErrorCode = nc_def_var(groupID, "meshNodeX", NC_DOUBLE, 1, dimIDs, &meshNodeXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshNodeX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeY)
    {
      dimIDs[0]   = numberOfMeshNodesDimID;
      ncErrorCode = nc_def_var(groupID, "meshNodeY", NC_DOUBLE, 1, dimIDs, &meshNodeYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshNodeY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeZSurface)
    {
      dimIDs[0]   = numberOfMeshNodesDimID;
      ncErrorCode = nc_def_var(groupID, "meshNodeZSurface", NC_DOUBLE, 1, dimIDs, &meshNodeZSurfaceVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshNodeZSurface in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeZBedrock)
    {
      dimIDs[0]   = numberOfMeshNodesDimID;
      ncErrorCode = nc_def_var(groupID, "meshNodeZBedrock", NC_DOUBLE, 1, dimIDs, &meshNodeZBedrockVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshNodeZBedrock in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementVertices)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshElementVertices", NC_INT, 2, dimIDs, &meshElementVerticesVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshElementVertices in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexX)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshVertexX", NC_DOUBLE, 2, dimIDs, &meshVertexXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshVertexX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexY)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshVertexY", NC_DOUBLE, 2, dimIDs, &meshVertexYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshVertexY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexZSurface)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshVertexZSurface", NC_DOUBLE, 2, dimIDs, &meshVertexZSurfaceVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshVertexZSurface in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexZBedrock)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshVertexZBedrock", NC_DOUBLE, 2, dimIDs, &meshVertexZBedrockVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshVertexZBedrock in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementX)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshElementX", NC_DOUBLE, 1, dimIDs, &meshElementXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshElementX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementY)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshElementY", NC_DOUBLE, 1, dimIDs, &meshElementYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshElementY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementZSurface)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshElementZSurface", NC_DOUBLE, 1, dimIDs, &meshElementZSurfaceVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshElementZSurface in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementZBedrock)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshElementZBedrock", NC_DOUBLE, 1, dimIDs, &meshElementZBedrockVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshElementZBedrock in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementArea)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshElementArea", NC_DOUBLE, 1, dimIDs, &meshElementAreaVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshElementArea in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementSlopeX)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshElementSlopeX", NC_DOUBLE, 1, dimIDs, &meshElementSlopeXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshElementSlopeX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementSlopeY)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshElementSlopeY", NC_DOUBLE, 1, dimIDs, &meshElementSlopeYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshElementSlopeY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighbors)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshMeshNeighbors", NC_INT, 2, dimIDs, &meshMeshNeighborsVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshMeshNeighbors in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsChannelEdge)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshMeshNeighborsChannelEdge", NC_BYTE, 2, dimIDs, &meshMeshNeighborsChannelEdgeVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshMeshNeighborsChannelEdge in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeLength)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshMeshNeighborsEdgeLength", NC_DOUBLE, 2, dimIDs, &meshMeshNeighborsEdgeLengthVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshMeshNeighborsEdgeLength in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalX)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshMeshNeighborsEdgeNormalX", NC_DOUBLE, 2, dimIDs, &meshMeshNeighborsEdgeNormalXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalY)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshMeshNeighborsEdgeNormalY", NC_DOUBLE, 2, dimIDs, &meshMeshNeighborsEdgeNormalYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshChannelNeighbors)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshChannelNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshChannelNeighbors", NC_INT, 2, dimIDs, &meshChannelNeighborsVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshChannelNeighbors in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshChannelNeighborsEdgeLength)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      dimIDs[1]   = numberOfMeshChannelNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "meshChannelNeighborsEdgeLength", NC_DOUBLE, 2, dimIDs, &meshChannelNeighborsEdgeLengthVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable meshChannelNeighborsEdgeLength in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeX)
    {
      dimIDs[0]   = numberOfChannelNodesDimID;
      ncErrorCode = nc_def_var(groupID, "channelNodeX", NC_DOUBLE, 1, dimIDs, &channelNodeXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelNodeX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeY)
    {
      dimIDs[0]   = numberOfChannelNodesDimID;
      ncErrorCode = nc_def_var(groupID, "channelNodeY", NC_DOUBLE, 1, dimIDs, &channelNodeYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelNodeY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeZBank)
    {
      dimIDs[0]   = numberOfChannelNodesDimID;
      ncErrorCode = nc_def_var(groupID, "channelNodeZBank", NC_DOUBLE, 1, dimIDs, &channelNodeZBankVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelNodeZBank in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeZBed)
    {
      dimIDs[0]   = numberOfChannelNodesDimID;
      ncErrorCode = nc_def_var(groupID, "channelNodeZBed", NC_DOUBLE, 1, dimIDs, &channelNodeZBedVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelNodeZBed in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementVertices)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      dimIDs[1]   = sizeOfChannelElementVerticesArrayDimID;
      ncErrorCode = nc_def_var(groupID, "channelElementVertices", NC_INT, 2, dimIDs, &channelElementVerticesVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelElementVertices in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexX)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      dimIDs[1]   = numberOfChannelVerticesDimID;
      ncErrorCode = nc_def_var(groupID, "channelVertexX", NC_DOUBLE, 2, dimIDs, &channelVertexXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelVertexX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexY)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      dimIDs[1]   = numberOfChannelVerticesDimID;
      ncErrorCode = nc_def_var(groupID, "channelVertexY", NC_DOUBLE, 2, dimIDs, &channelVertexYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelVertexY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexZBank)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      dimIDs[1]   = numberOfChannelVerticesDimID;
      ncErrorCode = nc_def_var(groupID, "channelVertexZBank", NC_DOUBLE, 2, dimIDs, &channelVertexZBankVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelVertexZBank in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexZBed)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      dimIDs[1]   = numberOfChannelVerticesDimID;
      ncErrorCode = nc_def_var(groupID, "channelVertexZBed", NC_DOUBLE, 2, dimIDs, &channelVertexZBedVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelVertexZBed in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementX)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelElementX", NC_DOUBLE, 1, dimIDs, &channelElementXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelElementX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementY)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelElementY", NC_DOUBLE, 1, dimIDs, &channelElementYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelElementY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementZBank)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelElementZBank", NC_DOUBLE, 1, dimIDs, &channelElementZBankVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelElementZBank in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementZBed)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelElementZBed", NC_DOUBLE, 1, dimIDs, &channelElementZBedVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelElementZBed in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementLength)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelElementLength", NC_DOUBLE, 1, dimIDs, &channelElementLengthVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelElementLength in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelChannelNeighbors)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      dimIDs[1]   = numberOfChannelChannelNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "channelChannelNeighbors", NC_INT, 2, dimIDs, &channelChannelNeighborsVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelChannelNeighbors in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelMeshNeighbors)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      dimIDs[1]   = numberOfChannelMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "channelMeshNeighbors", NC_INT, 2, dimIDs, &channelMeshNeighborsVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelMeshNeighbors in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelMeshNeighborsEdgeLength)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      dimIDs[1]   = numberOfChannelMeshNeighborsDimID;
      ncErrorCode = nc_def_var(groupID, "channelMeshNeighborsEdgeLength", NC_DOUBLE, 2, dimIDs, &channelMeshNeighborsEdgeLengthVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create variable channelMeshNeighborsEdgeLength in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (!error && NULL != meshNodeX)
    {
      start[0]    = localMeshNodeStart;
      count[0]    = localNumberOfMeshNodes;
      ncErrorCode = nc_put_vara_double(groupID, meshNodeXVarID, start, count, meshNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshNodeX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeY)
    {
      start[0]    = localMeshNodeStart;
      count[0]    = localNumberOfMeshNodes;
      ncErrorCode = nc_put_vara_double(groupID, meshNodeYVarID, start, count, meshNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshNodeY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeZSurface)
    {
      start[0]    = localMeshNodeStart;
      count[0]    = localNumberOfMeshNodes;
      ncErrorCode = nc_put_vara_double(groupID, meshNodeZSurfaceVarID, start, count, meshNodeZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshNodeZSurface in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeZBedrock)
    {
      start[0]    = localMeshNodeStart;
      count[0]    = localNumberOfMeshNodes;
      ncErrorCode = nc_put_vara_double(groupID, meshNodeZBedrockVarID, start, count, meshNodeZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshNodeZBedrock in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementVertices)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_int(groupID, meshElementVerticesVarID, start, count, (int*)meshElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshElementVertices in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexX)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, meshVertexXVarID, start, count, (double*)meshVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshVertexX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexY)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, meshVertexYVarID, start, count, (double*)meshVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshVertexY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexZSurface)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, meshVertexZSurfaceVarID, start, count, (double*)meshVertexZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshVertexZSurface in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexZBedrock)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, meshVertexZBedrockVarID, start, count, (double*)meshVertexZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshVertexZBedrock in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementX)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshElementXVarID, start, count, meshElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshElementX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementY)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshElementYVarID, start, count, meshElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshElementY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementZSurface)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshElementZSurfaceVarID, start, count, meshElementZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshElementZSurface in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementZBedrock)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshElementZBedrockVarID, start, count, meshElementZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshElementZBedrock in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementArea)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshElementAreaVarID, start, count, meshElementArea);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshElementArea in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementSlopeX)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshElementSlopeXVarID, start, count, meshElementSlopeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshElementSlopeX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementSlopeY)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshElementSlopeYVarID, start, count, meshElementSlopeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshElementSlopeY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighbors)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_int(groupID, meshMeshNeighborsVarID, start, count, (int*)meshMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshMeshNeighbors in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsChannelEdge)
    {
      // Assumes bool is one byte when casting to signed char.
      CkAssert(sizeof(bool) == sizeof(signed char));
      
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_schar(groupID, meshMeshNeighborsChannelEdgeVarID, start, count, (signed char*)meshMeshNeighborsChannelEdge);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshMeshNeighborsChannelEdge in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeLength)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, meshMeshNeighborsEdgeLengthVarID, start, count, (double*)meshMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshMeshNeighborsEdgeLength in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalX)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, meshMeshNeighborsEdgeNormalXVarID, start, count, (double*)meshMeshNeighborsEdgeNormalX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalY)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, meshMeshNeighborsEdgeNormalYVarID, start, count, (double*)meshMeshNeighborsEdgeNormalY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshChannelNeighbors)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::channelNeighborsSize;
      ncErrorCode = nc_put_vara_int(groupID, meshChannelNeighborsVarID, start, count, (int*)meshChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshChannelNeighbors in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshChannelNeighborsEdgeLength)
    {
      start[0]    = localMeshElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfMeshElements;
      count[1]    = MeshElement::channelNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, meshChannelNeighborsEdgeLengthVarID, start, count, (double*)meshChannelNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable meshChannelNeighborsEdgeLength in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeX)
    {
      start[0]    = localChannelNodeStart;
      count[0]    = localNumberOfChannelNodes;
      ncErrorCode = nc_put_vara_double(groupID, channelNodeXVarID, start, count, channelNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelNodeX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeY)
    {
      start[0]    = localChannelNodeStart;
      count[0]    = localNumberOfChannelNodes;
      ncErrorCode = nc_put_vara_double(groupID, channelNodeYVarID, start, count, channelNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelNodeY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeZBank)
    {
      start[0]    = localChannelNodeStart;
      count[0]    = localNumberOfChannelNodes;
      ncErrorCode = nc_put_vara_double(groupID, channelNodeZBankVarID, start, count, channelNodeZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelNodeZBank in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeZBed)
    {
      start[0]    = localChannelNodeStart;
      count[0]    = localNumberOfChannelNodes;
      ncErrorCode = nc_put_vara_double(groupID, channelNodeZBedVarID, start, count, channelNodeZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelNodeZBed in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementVertices)
    {
      start[0]    = localChannelElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfChannelElements;
      count[1]    = ChannelElement::channelVerticesSize + 2;
      ncErrorCode = nc_put_vara_int(groupID, channelElementVerticesVarID, start, count, (int*)channelElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelElementVertices in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexX)
    {
      start[0]    = localChannelElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfChannelElements;
      count[1]    = ChannelElement::channelVerticesSize;
      ncErrorCode = nc_put_vara_double(groupID, channelVertexXVarID, start, count, (double*)channelVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelVertexX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexY)
    {
      start[0]    = localChannelElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfChannelElements;
      count[1]    = ChannelElement::channelVerticesSize;
      ncErrorCode = nc_put_vara_double(groupID, channelVertexYVarID, start, count, (double*)channelVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelVertexY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexZBank)
    {
      start[0]    = localChannelElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfChannelElements;
      count[1]    = ChannelElement::channelVerticesSize;
      ncErrorCode = nc_put_vara_double(groupID, channelVertexZBankVarID, start, count, (double*)channelVertexZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelVertexZBank in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexZBed)
    {
      start[0]    = localChannelElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfChannelElements;
      count[1]    = ChannelElement::channelVerticesSize;
      ncErrorCode = nc_put_vara_double(groupID, channelVertexZBedVarID, start, count, (double*)channelVertexZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelVertexZBed in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementX)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelElementXVarID, start, count, channelElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelElementX in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementY)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelElementYVarID, start, count, channelElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelElementY in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementZBank)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelElementZBankVarID, start, count, channelElementZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelElementZBank in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementZBed)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelElementZBedVarID, start, count, channelElementZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelElementZBed in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementLength)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelElementLengthVarID, start, count, channelElementLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelElementLength in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelChannelNeighbors)
    {
      start[0]    = localChannelElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfChannelElements;
      count[1]    = ChannelElement::channelNeighborsSize;
      ncErrorCode = nc_put_vara_int(groupID, channelChannelNeighborsVarID, start, count, (int*)channelChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelChannelNeighbors in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelMeshNeighbors)
    {
      start[0]    = localChannelElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfChannelElements;
      count[1]    = ChannelElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_int(groupID, channelMeshNeighborsVarID, start, count, (int*)channelMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelMeshNeighbors in NetCDF geometry file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelMeshNeighborsEdgeLength)
    {
      start[0]    = localChannelElementStart;
      start[1]    = 0;
      count[0]    = localNumberOfChannelElements;
      count[1]    = ChannelElement::meshNeighborsSize;
      ncErrorCode = nc_put_vara_double(groupID, channelMeshNeighborsEdgeLengthVarID, start, count, (double*)channelMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to write variable channelMeshNeighborsEdgeLength in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to close NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
    }

  return error;
}

bool FileManager::writeParameter(const char* directory, int group, bool create)
{
  bool   error      = false;           // Error flag.
  char*  nameString = NULL;            // Temporary string for file and group names.
  size_t nameStringSize;               // Size of buffer allocated for nameString.
  size_t numPrinted;                   // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;                  // Return value of NetCDF functions.
  int    fileID;                       // ID of NetCDF file.
  bool   fileOpen   = false;           // Whether fileID refers to an open file.
  int    groupID;                      // ID of group in NetCDF file.
  int    numberOfMeshElementsDimID;    // ID of dimension in NetCDF file.
  int    numberOfChannelElementsDimID; // ID of dimension in NetCDF file.
  int    dimIDs[NC_MAX_VAR_DIMS];      // For passing dimension IDs.
  int    meshCatchmentVarID;           // ID of variable in NetCDF file.
  int    meshConductivityVarID;        // ID of variable in NetCDF file.
  int    meshPorosityVarID;            // ID of variable in NetCDF file.
  int    meshManningsNVarID;           // ID of variable in NetCDF file.
  int    channelChannelTypeVarID;      // ID of variable in NetCDF file.
  int    channelPermanentCodeVarID;    // ID of variable in NetCDF file.
  int    channelBaseWidthVarID;        // ID of variable in NetCDF file.
  int    channelSideSlopeVarID;        // ID of variable in NetCDF file.
  int    channelBedConductivityVarID;  // ID of variable in NetCDF file.
  int    channelBedThicknessVarID;     // ID of variable in NetCDF file.
  int    channelManningsNVarID;        // ID of variable in NetCDF file.
  size_t start[NC_MAX_VAR_DIMS];       // For specifying subarrays when writing to NetCDF file.
  size_t count[NC_MAX_VAR_DIMS];       // For specifying subarrays when writing to NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::writeParameter: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // Allocate space for file and group name strings
      nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // This will also have enough space to hold the group name because the maximum length
                                                                        // of an int printed as a string is 11, which is shorter than the length of
                                                                        // "/parameter.nc", which is 13.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // FIXME make directory?

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeParameter: incorrect return value of snprintf when generating file name.  %d should be %d and less than %d.\n",
                  numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create or open file.
  if (!error)
    {
      if (create)
        {
          ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
        }
      else
        {
          ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to %s NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  create ? "create" : "open", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }

  // Create group name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%d", group);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeParameter: incorrect return value of snprintf when generating group name.  "
                  "%d should be greater than 0 and less than %d.\n", numPrinted, nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create group.
  if (!error)
    {
      ncErrorCode = nc_def_grp(fileID, nameString, &groupID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create group %s in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshElements", globalNumberOfMeshElements, &numberOfMeshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create dimension numberOfMeshElements in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelElements", globalNumberOfChannelElements, &numberOfChannelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create dimension numberOfChannelElements in NetCDF parameter file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error && NULL != meshCatchment)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshCatchment", NC_INT, 1, dimIDs, &meshCatchmentVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable meshCatchment in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshConductivity)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshConductivity", NC_DOUBLE, 1, dimIDs, &meshConductivityVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable meshConductivity in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshPorosity)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshPorosity", NC_DOUBLE, 1, dimIDs, &meshPorosityVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable meshPorosity in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshManningsN)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshManningsN", NC_DOUBLE, 1, dimIDs, &meshManningsNVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable meshManningsN in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelChannelType)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelChannelType", NC_INT, 1, dimIDs, &channelChannelTypeVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable channelChannelType in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelPermanentCode)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelPermanentCode", NC_INT, 1, dimIDs, &channelPermanentCodeVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable channelPermanentCode in NetCDF parameter file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBaseWidth)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelBaseWidth", NC_DOUBLE, 1, dimIDs, &channelBaseWidthVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable channelBaseWidth in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSideSlope)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelSideSlope", NC_DOUBLE, 1, dimIDs, &channelSideSlopeVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable channelSideSlope in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBedConductivity)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelBedConductivity", NC_DOUBLE, 1, dimIDs, &channelBedConductivityVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable channelBedConductivity in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBedThickness)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelBedThickness", NC_DOUBLE, 1, dimIDs, &channelBedThicknessVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable channelBedThickness in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelManningsN)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelManningsN", NC_DOUBLE, 1, dimIDs, &channelManningsNVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to create variable channelManningsN in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (!error && NULL != meshCatchment)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_int(groupID, meshCatchmentVarID, start, count, meshCatchment);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable meshCatchment in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshConductivity)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshConductivityVarID, start, count, meshConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable meshConductivity in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshPorosity)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshPorosityVarID, start, count, meshPorosity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable meshPorosity in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshManningsN)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshManningsNVarID, start, count, meshManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable meshManningsN in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelChannelType)
    {
      // Assumes ChannelTypeEnum is four bytes when casting to int.
      CkAssert(sizeof(ChannelTypeEnum) == sizeof(int));
      
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_int(groupID, channelChannelTypeVarID, start, count, (int*)channelChannelType);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable channelChannelType in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelPermanentCode)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_int(groupID, channelPermanentCodeVarID, start, count, channelPermanentCode);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable channelPermanentCode in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBaseWidth)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelBaseWidthVarID, start, count, channelBaseWidth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable channelBaseWidth in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSideSlope)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelSideSlopeVarID, start, count, channelSideSlope);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable channelSideSlope in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBedConductivity)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelBedConductivityVarID, start, count, channelBedConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable channelBedConductivity in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBedThickness)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelBedThicknessVarID, start, count, channelBedThickness);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable channelBedThickness in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelManningsN)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelManningsNVarID, start, count, channelManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to write variable channelManningsN in NetCDF parameter file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeParameter: unable to close NetCDF parameter file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
    }

  return error;
}

bool FileManager::writeState(const char* directory, int group, bool create, double time, double dt, int geometryGroup, int parameterGroup)
{
  bool   error      = false;                     // Error flag.
  char*  nameString = NULL;                      // Temporary string for file and group names.
  size_t nameStringSize;                         // Size of buffer allocated for nameString.
  size_t numPrinted;                             // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;                            // Return value of NetCDF functions.
  int    fileID;                                 // ID of NetCDF file.
  bool   fileOpen   = false;                     // Whether fileID refers to an open file.
  int    groupID;                                // ID of group in NetCDF file.
  int    numberOfMeshNodesDimID;                 // ID of dimension in NetCDF file.
  int    numberOfMeshElementsDimID;              // ID of dimension in NetCDF file.
  int    numberOfMeshMeshNeighborsDimID;         // ID of dimension in NetCDF file.
  int    numberOfMeshChannelNeighborsDimID;      // ID of dimension in NetCDF file.
  int    numberOfChannelNodesDimID;              // ID of dimension in NetCDF file.
  int    numberOfChannelElementsDimID;           // ID of dimension in NetCDF file.
  int    sizeOfChannelElementVerticesArrayDimID; // ID of dimension in NetCDF file.
  int    numberOfChannelVerticesDimID;           // ID of dimension in NetCDF file.
  int    numberOfChannelChannelNeighborsDimID;   // ID of dimension in NetCDF file.
  int    numberOfChannelMeshNeighborsDimID;      // ID of dimension in NetCDF file.
  int    dimIDs[NC_MAX_VAR_DIMS];                // For passing dimension IDs.
  int    meshSurfacewaterDepthVarID;             // ID of variable in NetCDF file.
  int    meshSurfacewaterErrorVarID;             // ID of variable in NetCDF file.
  int    meshGroundwaterHeadVarID;               // ID of variable in NetCDF file.
  int    meshGroundwaterErrorVarID;              // ID of variable in NetCDF file.
  int    channelSurfacewaterDepthVarID;          // ID of variable in NetCDF file.
  int    channelSurfacewaterErrorVarID;          // ID of variable in NetCDF file.
  size_t start[NC_MAX_VAR_DIMS];                 // For specifying subarrays when writing to NetCDF file.
  size_t count[NC_MAX_VAR_DIMS];                 // For specifying subarrays when writing to NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::writeState: directory must not be null.\n");
      error = true;
    }
  
  if (!(0.0 < dt))
    {
      CkError("ERROR in FileManager::writeState: dt must be greater than zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // Allocate space for file and group name strings
      nameStringSize = strlen(directory) + strlen("/state.nc  ") + 1; // Extra spaces are added at the end of "/state.nc  " in order to have enough space to
                                                                      // hold the group name because the maximum length of an int printed as a string is 11,
                                                                      // which is the same length as "/state.nc  ".  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // FIXME make directory?

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeState: incorrect return value of snprintf when generating file name.  %d should be %d and less than %d.\n",
                  numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create or open file.
  if (!error)
    {
      if (create)
        {
          ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
        }
      else
        {
          ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to %s NetCDF state file %s.  NetCDF error message: %s.\n",
                  create ? "create" : "open", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }

  // Create group name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%d", group);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 < numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeState: incorrect return value of snprintf when generating group name.  "
                  "%d should be greater than 0 and less than %d.\n", numPrinted, nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create group.
  if (!error)
    {
      ncErrorCode = nc_def_grp(fileID, nameString, &groupID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create group %s in NetCDF state file.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshNodes", globalNumberOfMeshNodes, &numberOfMeshNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfMeshNodes in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshElements", globalNumberOfMeshElements, &numberOfMeshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfMeshElements in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshMeshNeighbors", MeshElement::meshNeighborsSize, &numberOfMeshMeshNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfMeshMeshNeighbors in NetCDF state file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfMeshChannelNeighbors", MeshElement::channelNeighborsSize, &numberOfMeshChannelNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfMeshChannelNeighbors in NetCDF state file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelNodes", globalNumberOfChannelNodes, &numberOfChannelNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfChannelNodes in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelElements", globalNumberOfChannelElements, &numberOfChannelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfChannelElements in NetCDF state file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "sizeOfChannelElementVerticesArray", ChannelElement::channelVerticesSize + 2, &sizeOfChannelElementVerticesArrayDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeGeometry: unable to create dimension sizeOfChannelElementVerticesArray in NetCDF geometry file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelVertices", ChannelElement::channelVerticesSize, &numberOfChannelVerticesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfChannelVertices in NetCDF state file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelChannelNeighbors", ChannelElement::channelNeighborsSize, &numberOfChannelChannelNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfChannelChannelNeighbors in NetCDF state file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(groupID, "numberOfChannelMeshNeighbors", ChannelElement::meshNeighborsSize, &numberOfChannelMeshNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create dimension numberOfChannelMeshNeighbors in NetCDF state file.  "
                  "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create attributes.
  if (!error)
    {
      ncErrorCode = nc_put_att_double(groupID, NC_GLOBAL, "time", NC_DOUBLE, 1, &time);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create attribute time in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_put_att_double(groupID, NC_GLOBAL, "dt", NC_DOUBLE, 1, &dt);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create attribute dt in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_put_att_int(groupID, NC_GLOBAL, "geometryGroup", NC_INT, 1, &geometryGroup);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create attribute geometryGroup in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_put_att_int(groupID, NC_GLOBAL, "parameterGroup", NC_INT, 1, &parameterGroup);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create attribute parameterGroup in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error && NULL != meshSurfacewaterDepth)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshSurfacewaterDepth", NC_DOUBLE, 1, dimIDs, &meshSurfacewaterDepthVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create variable meshSurfacewaterDepth in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshSurfacewaterError)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshSurfacewaterError", NC_DOUBLE, 1, dimIDs, &meshSurfacewaterErrorVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create variable meshSurfacewaterError in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshGroundwaterHead)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshGroundwaterHead", NC_DOUBLE, 1, dimIDs, &meshGroundwaterHeadVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create variable meshGroundwaterHead in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshGroundwaterError)
    {
      dimIDs[0]   = numberOfMeshElementsDimID;
      ncErrorCode = nc_def_var(groupID, "meshGroundwaterError", NC_DOUBLE, 1, dimIDs, &meshGroundwaterErrorVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create variable meshGroundwaterError in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSurfacewaterDepth)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelSurfacewaterDepth", NC_DOUBLE, 1, dimIDs, &channelSurfacewaterDepthVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create variable channelSurfacewaterDepth in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSurfacewaterError)
    {
      dimIDs[0]   = numberOfChannelElementsDimID;
      ncErrorCode = nc_def_var(groupID, "channelSurfacewaterError", NC_DOUBLE, 1, dimIDs, &channelSurfacewaterErrorVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to create variable channelSurfacewaterError in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (!error && NULL != meshSurfacewaterDepth)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshSurfacewaterDepthVarID, start, count, meshSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to write variable meshSurfacewaterDepth in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshSurfacewaterError)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshSurfacewaterErrorVarID, start, count, meshSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to write variable meshSurfacewaterError in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshGroundwaterHead)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshGroundwaterHeadVarID, start, count, meshGroundwaterHead);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to write variable meshGroundwaterHead in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshGroundwaterError)
    {
      start[0]    = localMeshElementStart;
      count[0]    = localNumberOfMeshElements;
      ncErrorCode = nc_put_vara_double(groupID, meshGroundwaterErrorVarID, start, count, meshGroundwaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to write variable meshGroundwaterError in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSurfacewaterDepth)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelSurfacewaterDepthVarID, start, count, channelSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to write variable channelSurfacewaterDepth in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSurfacewaterError)
    {
      start[0]    = localChannelElementStart;
      count[0]    = localNumberOfChannelElements;
      ncErrorCode = nc_put_vara_double(groupID, channelSurfacewaterErrorVarID, start, count, channelSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to write variable channelSurfacewaterError in NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeState: unable to close NetCDF state file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
    }

  return error;
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "file_manager.def.h"
#pragma GCC diagnostic warning "-Wsign-compare"
#pragma GCC diagnostic warning "-Wunused-variable"
