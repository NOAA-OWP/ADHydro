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
      itemHome = ((item - itemsInAllFatOwners) / itemsPerThinOwner) + numberOfFatOwners;
    }
  
  return itemHome;
}

FileManager::FileManager(size_t directorySize, char* directory)
{
  int    ii, jj;                                             // Loop counters.
  bool   needToGetVertexData                        = false; // Whether we need to get any vertex data.
  // FIXME Hard coded mesh, read from NetCDF file instead.
  double globalNodeX[13]                            = {-75.0, 75.0, -75.0, 0.0, 75.0, 0.0, -75.0, 0.0, 75.0, 0.0, -75.0, 0.0, 75.0};
  double globalNodeY[13]                            = {200.0, 200.0, 150.0, 150.0, 150.0, 100.0, 75.0, 75.0, 75.0, 50.0, 0.0, 0.0, 0.0};
  double globalNodeZ[13]                            = {30.0, 30.0, 25.0, 15.0, 25.0, 10.0, 17.5, 7.5, 17.5, 5.0, 10.0, 0.0, 10.0};
  int    globalMeshElementVertices[8][3]            = {{3, 2, 6}, {3, 6, 7}, {7, 6, 10}, {7, 10, 11}, {4, 3, 8}, {3, 7, 8}, {8, 7, 12}, {7, 11, 12}};
  int    globalMeshMeshNeighbors[8][3]              = {{NOFLOW, 1, NOFLOW}, {2, 5, 0}, {NOFLOW, 3, 1}, {NOFLOW, 7, 2}, {5, NOFLOW, NOFLOW}, {6, 4, 1},
                                                       {7, NOFLOW, 5}, {NOFLOW, 6, 3}};
  bool   globalMeshMeshNeighborsChannelEdge[8][3]   = {{false, false, false}, {false, true, false}, {false, false, false}, {false, true, false},
                                                       {false, false, false}, {false, false, true}, {false, false, false}, {false, false, true}};
  int    globalMeshChannelNeighbors[8][2]           = {{0 ,NOFLOW}, {1 ,2}, {NOFLOW ,NOFLOW}, {2 ,3}, {0 ,NOFLOW}, {1 ,2}, {NOFLOW ,NOFLOW}, {2 ,3}};
  double globalMeshChannelNeighborsEdgeLength[8][2] = {{75.0, 1.0}, {50.0, 25.0}, {1.0, 1.0}, {25.0, 50.0}, {75.0, 1.0}, {50.0, 25.0}, {1.0, 1.0},
                                                       {25.0, 50.0}};
  int    globalChannelElementVertices[4][7]         = {{3, 5, 0, 2, 3, 4, 1}, {2, 5, 3, 5, 5, 5, 5}, {2, 5, 5, 9, 9, 9, 9}, {2, 5, 9, 11, 11, 11, 11}};
  double globalChannelElementX[4]                   = {0.0, 0.0, 0.0, 0.0};
  double globalChannelElementY[4]                   = {175.0, 125.0, 75.0, 25.0};
  double globalChannelElementZ[4]                   = {15.0, 12.5, 7.5, 2.5};
  int    globalChannelChannelNeighbors[4][2]        = {{1, NOFLOW}, {0, 2}, {1, 3}, {2, OUTFLOW}};
  int    globalChannelMeshNeighbors[4][4]           = {{0, 4, NOFLOW, NOFLOW}, {1, 5, NOFLOW, NOFLOW}, {1, 5, 3, 7}, {3, 7, NOFLOW, NOFLOW}};
  double globalChannelMeshNeighborsEdgeLength[4][4] = {{75.0, 75.0, 1.0, 1.0}, {50.0, 50.0, 1.0, 1.0}, {25.0, 25.0, 25.0, 25.0}, {50.0, 50.0, 1.0, 1.0}};
  
  globalNumberOfMeshNodes       = 13;
  globalNumberOfMeshElements    = 8;
  globalNumberOfChannelNodes    = 13;
  globalNumberOfChannelElements = 4;
  
  localStartAndNumber(&localMeshNodeStart,       &localNumberOfMeshNodes,       globalNumberOfMeshNodes);
  localStartAndNumber(&localMeshElementStart,    &localNumberOfMeshElements,    globalNumberOfMeshElements);
  localStartAndNumber(&localChannelNodeStart,    &localNumberOfChannelNodes,    globalNumberOfChannelNodes);
  localStartAndNumber(&localChannelElementStart, &localNumberOfChannelElements, globalNumberOfChannelElements);
  
  meshNodeX = new double[localNumberOfMeshNodes];
  
  for (ii = 0; ii < localNumberOfMeshNodes; ii++)
    {
      meshNodeX[ii] = globalNodeX[ii + localMeshNodeStart];
    }
  
  meshNodeY = new double[localNumberOfMeshNodes];
  
  for (ii = 0; ii < localNumberOfMeshNodes; ii++)
    {
      meshNodeY[ii] = globalNodeY[ii + localMeshNodeStart];
    }
  
  meshNodeZSurface = new double[localNumberOfMeshNodes];
  
  for (ii = 0; ii < localNumberOfMeshNodes; ii++)
    {
      meshNodeZSurface[ii] = globalNodeZ[ii + localMeshNodeStart];
    }
  
  meshNodeZBedrock = new double[localNumberOfMeshNodes];
  
  for (ii = 0; ii < localNumberOfMeshNodes; ii++)
    {
      meshNodeZBedrock[ii] = globalNodeZ[ii + localMeshNodeStart] - 5.0;
    }
  
  meshElementVertices = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
        {
          meshElementVertices[ii][jj] = globalMeshElementVertices[ii + localMeshElementStart][jj];
        }
    }
  
  // Will be derived from node data.
  meshVertexX         = NULL;
  meshVertexY         = NULL;
  meshVertexZSurface  = NULL;
  meshVertexZBedrock  = NULL;
  meshElementX        = NULL;
  meshElementY        = NULL;
  meshElementZSurface = NULL;
  meshElementZBedrock = NULL;
  meshElementArea     = NULL;
  meshElementSlopeX   = NULL;
  meshElementSlopeY   = NULL;
  
  meshCatchment = new int[localNumberOfMeshElements];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      meshCatchment[ii] = 1;
    }
  
  meshConductivity = new double[localNumberOfMeshElements];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      meshConductivity[ii] = 5.55e-4;
    }
  
  meshPorosity = new double[localNumberOfMeshElements];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      meshPorosity[ii] = 0.5;
    }
  
  meshManningsN = new double[localNumberOfMeshElements];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      meshManningsN[ii] = 0.038;
    }
  
  meshSurfacewaterDepth =  new double[localNumberOfMeshElements];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      meshSurfacewaterDepth[ii] = 0.1;
    }
  
  meshSurfacewaterError = new double[localNumberOfMeshElements];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      meshSurfacewaterError[ii] = 0.0;
    }
  
  // FIXME decide how to do meshGroundwaterHead.
  meshGroundwaterHead = NULL;
  
  meshGroundwaterError = new double[localNumberOfMeshElements];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      meshGroundwaterError[ii] = 0.0;
    }
  
  meshMeshNeighbors = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
        {
          meshMeshNeighbors[ii][jj] = globalMeshMeshNeighbors[ii + localMeshElementStart][jj];
        }
    }
  
  meshMeshNeighborsChannelEdge = new bool[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
        {
          meshMeshNeighborsChannelEdge[ii][jj] = globalMeshMeshNeighborsChannelEdge[ii + localMeshElementStart][jj];
        }
    }
  
  // Will be derived from node data.
  meshMeshNeighborsEdgeLength  = NULL;
  meshMeshNeighborsEdgeNormalX = NULL;
  meshMeshNeighborsEdgeNormalY = NULL;
  
  meshChannelNeighbors = new int[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      for (jj = 0; jj < MeshElement::channelNeighborsSize; jj++)
        {
          meshChannelNeighbors[ii][jj] = globalMeshChannelNeighbors[ii + localMeshElementStart][jj];
        }
    }
  
  meshChannelNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
  
  for (ii = 0; ii < localNumberOfMeshElements; ii++)
    {
      for (jj = 0; jj < MeshElement::channelNeighborsSize; jj++)
        {
          meshChannelNeighborsEdgeLength[ii][jj] = globalMeshChannelNeighborsEdgeLength[ii + localMeshElementStart][jj];
        }
    }
  
  channelNodeX = new double[localNumberOfChannelNodes];
  
  for (ii = 0; ii < localNumberOfChannelNodes; ii++)
    {
      channelNodeX[ii] = globalNodeX[ii + localChannelNodeStart];
    }
  
  channelNodeY = new double[localNumberOfChannelNodes];
  
  for (ii = 0; ii < localNumberOfChannelNodes; ii++)
    {
      channelNodeY[ii] = globalNodeY[ii + localChannelNodeStart];
    }
  
  channelNodeZBank = new double[localNumberOfChannelNodes];
  
  for (ii = 0; ii < localNumberOfChannelNodes; ii++)
    {
      channelNodeZBank[ii] = globalNodeZ[ii + localChannelNodeStart];
    }
  
  channelNodeZBed = new double[localNumberOfChannelNodes];
  
  for (ii = 0; ii < localNumberOfChannelNodes; ii++)
    {
      channelNodeZBed[ii] = globalNodeZ[ii + localChannelNodeStart] - 5.0;
    }
  
  channelElementVertices = new int[localNumberOfChannelElements][ChannelElement::channelVerticesSize + 2];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      for (jj = 0; jj < ChannelElement::channelVerticesSize + 2; jj++)
        {
          channelElementVertices[ii][jj] = globalChannelElementVertices[ii + localChannelElementStart][jj];
        }
    }
  
  // Unused.
  channelVertexX         = NULL;
  channelVertexY         = NULL;
  channelVertexZBank     = NULL;
  channelVertexZBed      = NULL;
  
  channelElementX = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelElementX[ii] = globalChannelElementX[ii + localChannelElementStart];
    }
  
  channelElementY = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelElementY[ii] = globalChannelElementY[ii + localChannelElementStart];
    }
  
  channelElementZBank = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelElementZBank[ii] = globalChannelElementZ[ii + localChannelElementStart];
    }
  
  channelElementZBed = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelElementZBed[ii] = globalChannelElementZ[ii + localChannelElementStart] - 2.5;
    }
  
  channelElementLength = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelElementLength[ii] = 50.0;
    }
  
  channelChannelType = new ChannelTypeEnum[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      if (0 == ii + localChannelElementStart)
        {
          channelChannelType[ii] = WATERBODY;
        }
      else
        {
          channelChannelType[ii] = STREAM;
        }
    }
  
  channelPermanentCode = new int[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelPermanentCode[ii] = 1;
    }
  
  channelBaseWidth = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      if (0 == ii + localChannelElementStart)
        {
          channelBaseWidth[ii] = 150.0;
        }
      else
        {
          channelBaseWidth[ii] = 1.0;
        }
    }
  
  channelSideSlope = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelSideSlope[ii] = 1.0;
    }
  
  channelBedConductivity = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelBedConductivity[ii] = 5.55e-4;
    }
  
  channelBedThickness = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelBedThickness[ii] = 1.0;
    }
  
  channelManningsN = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelManningsN[ii] = 0.038;
    }
  
  channelSurfacewaterDepth = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelSurfacewaterDepth[ii] = 0.0;
    }
  
  channelSurfacewaterError = new double[localNumberOfChannelElements];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      channelSurfacewaterError[ii] = 0.0;
    }
  
  channelChannelNeighbors = new int[localNumberOfChannelElements][ChannelElement::channelNeighborsSize];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      for (jj = 0; jj < ChannelElement::channelNeighborsSize; jj++)
        {
          channelChannelNeighbors[ii][jj] = globalChannelChannelNeighbors[ii + localChannelElementStart][jj];
        }
    }
  
  channelMeshNeighbors = new int[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      for (jj = 0; jj < ChannelElement::meshNeighborsSize; jj++)
        {
          channelMeshNeighbors[ii][jj] = globalChannelMeshNeighbors[ii + localChannelElementStart][jj];
        }
    }
  
  channelMeshNeighborsEdgeLength = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
  
  for (ii = 0; ii < localNumberOfChannelElements; ii++)
    {
      for (jj = 0; jj < ChannelElement::meshNeighborsSize; jj++)
        {
          channelMeshNeighborsEdgeLength[ii][jj] = globalChannelMeshNeighborsEdgeLength[ii + localChannelElementStart][jj];
        }
    }
  
  meshVertexUpdated     = NULL; // Will be allocated and freed if and when we send messages to update vertices from nodes.
  meshElementUpdated    = new bool[localNumberOfMeshElements];
  channelElementUpdated = new bool[localNumberOfChannelElements];

  // Get vertex data form node data.
  if (NULL == meshVertexX && NULL != meshNodeX && NULL != meshElementVertices)
    {
      meshVertexX         = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      needToGetVertexData = true;
    }
  
  if (NULL == meshVertexY && NULL != meshNodeY && NULL != meshElementVertices)
    {
      meshVertexY         = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      needToGetVertexData = true;
    }
  
  if (NULL == meshVertexZSurface && NULL != meshNodeZSurface && NULL != meshElementVertices)
    {
      meshVertexZSurface  = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      needToGetVertexData = true;
    }
  
  if (NULL == meshVertexZBedrock && NULL != meshNodeZBedrock && NULL != meshElementVertices)
    {
      meshVertexZBedrock  = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      needToGetVertexData = true;
    }
  
  // We don't try to get channel vertex data because it is unused.
  
  if (needToGetVertexData)
    {
      meshVertexUpdated = new bool[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
      
      for(ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              meshVertexUpdated[ii][jj] = false;
              
              // FIXME improve efficiency.  Don't send message to myself.  Don't send duplicate messages for the same node.
              thisProxy[home(meshElementVertices[ii][jj], globalNumberOfMeshNodes)].getMeshVertexDataMessage(CkMyPe(), ii + localMeshElementStart, jj,
                                                                                                             meshElementVertices[ii][jj]);
            }
        }
      
      thisProxy[CkMyPe()].waitForVertexData();
    }
  else
    {
      contribute();
    }
}

void FileManager::finishInitialization(size_t directorySize, char* directory, int geometryGroup, int parameterGroup, int stateGroup, double time, double dt)
{
  bool   error = false; // Error flag.
  int    element;       // Loop counter.
  int    vertex;        // Loop counter.
  double value;         // For calculating derived values.

  if (NULL == meshElementX && NULL != meshVertexX)
    {
      meshElementX = new double[localNumberOfMeshElements];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          value = 0.0;

          for (vertex = 0; vertex < MeshElement::meshNeighborsSize; vertex++)
            {
              value += meshVertexX[element][vertex];
            }

          meshElementX[element] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementY && NULL != meshVertexY)
    {
      meshElementY = new double[localNumberOfMeshElements];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          value = 0.0;

          for (vertex = 0; vertex < MeshElement::meshNeighborsSize; vertex++)
            {
              value += meshVertexY[element][vertex];
            }

          meshElementY[element] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementZSurface && NULL != meshVertexZSurface)
    {
      meshElementZSurface = new double[localNumberOfMeshElements];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          value = 0.0;

          for (vertex = 0; vertex < MeshElement::meshNeighborsSize; vertex++)
            {
              value += meshVertexZSurface[element][vertex];
            }

          meshElementZSurface[element] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementZBedrock && NULL != meshVertexZBedrock)
    {
      meshElementZBedrock = new double[localNumberOfMeshElements];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          value = 0.0;

          for (vertex = 0; vertex < MeshElement::meshNeighborsSize; vertex++)
            {
              value += meshVertexZBedrock[element][vertex];
            }

          meshElementZBedrock[element] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementArea && NULL != meshVertexX && NULL != meshVertexY)
    {
      meshElementArea = new double[localNumberOfMeshElements];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (vertex = 0; vertex < MeshElement::meshNeighborsSize; vertex++)
            {
              value += meshVertexX[element][vertex] * (meshVertexY[element][(vertex + 1) % MeshElement::meshNeighborsSize] -
                                                       meshVertexY[element][(vertex + 2) % MeshElement::meshNeighborsSize]);
            }

          meshElementArea[element] = value * 0.5;
        }
    }

  if (NULL == meshElementSlopeX && NULL != meshVertexX && NULL != meshVertexY && NULL != meshVertexZSurface)
    {
      meshElementSlopeX = new double[localNumberOfMeshElements];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (vertex = 0; vertex < MeshElement::meshNeighborsSize - 1; vertex++)
            {
              value += (meshVertexY[element][(vertex + 2) % MeshElement::meshNeighborsSize] - meshVertexY[element][vertex]) *
                       (meshVertexZSurface[element][(vertex + 1) % MeshElement::meshNeighborsSize] - meshVertexZSurface[element][0]);
            }

          meshElementSlopeX[element] = value / (2.0 * meshElementArea[element]);
        }
    }

  if (NULL == meshElementSlopeY && NULL != meshVertexX && NULL != meshVertexY && NULL != meshVertexZSurface)
    {
      meshElementSlopeY = new double[localNumberOfMeshElements];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (vertex = 0; vertex < MeshElement::meshNeighborsSize - 1; vertex++)
            {
              value += (meshVertexX[element][vertex] - meshVertexX[element][(vertex + 2) % MeshElement::meshNeighborsSize]) *
                       (meshVertexZSurface[element][(vertex + 1) % MeshElement::meshNeighborsSize] - meshVertexZSurface[element][0]);
            }

          meshElementSlopeY[element] = value / (2.0 * meshElementArea[element]);
        }
    }

  if (NULL == meshMeshNeighborsEdgeLength && NULL != meshVertexX && NULL != meshVertexY)
    {
      meshMeshNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          for (vertex = 0; vertex < MeshElement::meshNeighborsSize; vertex++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeLength[element][vertex] = sqrt((meshVertexX[element][(vertex + 1) % MeshElement::meshNeighborsSize] -
                                                                   meshVertexX[element][(vertex + 2) % MeshElement::meshNeighborsSize]) *
                                                                  (meshVertexX[element][(vertex + 1) % MeshElement::meshNeighborsSize] -
                                                                   meshVertexX[element][(vertex + 2) % MeshElement::meshNeighborsSize]) +
                                                                  (meshVertexY[element][(vertex + 1) % MeshElement::meshNeighborsSize] -
                                                                   meshVertexY[element][(vertex + 2) % MeshElement::meshNeighborsSize]) *
                                                                  (meshVertexY[element][(vertex + 1) % MeshElement::meshNeighborsSize] -
                                                                   meshVertexY[element][(vertex + 2) % MeshElement::meshNeighborsSize]));
            }
        }
    }

  if (NULL == meshMeshNeighborsEdgeNormalX && NULL != meshVertexY && NULL != meshMeshNeighborsEdgeLength)
    {
      meshMeshNeighborsEdgeNormalX = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          for (vertex = 0; vertex < MeshElement::meshNeighborsSize; vertex++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeNormalX[element][vertex] = (meshVertexY[element][(vertex + 2) % MeshElement::meshNeighborsSize] -
                                                               meshVertexY[element][(vertex + 1) % MeshElement::meshNeighborsSize]) /
                                                              meshMeshNeighborsEdgeLength[element][vertex];
            }
        }
    }

  if (NULL == meshMeshNeighborsEdgeNormalY && NULL != meshVertexX && NULL != meshMeshNeighborsEdgeLength)
    {
      meshMeshNeighborsEdgeNormalY = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          for (vertex = 0; vertex < MeshElement::meshNeighborsSize; vertex++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeNormalY[element][vertex] = (meshVertexX[element][(vertex + 1) % MeshElement::meshNeighborsSize] -
                                                               meshVertexX[element][(vertex + 2) % MeshElement::meshNeighborsSize]) /
                                                              meshMeshNeighborsEdgeLength[element][vertex];
            }
        }
    }

  // FIXME remove, part of the hardcoded mesh
  if (NULL == meshGroundwaterHead && NULL != meshElementZSurface)
    {
      meshGroundwaterHead = new double[localNumberOfMeshElements];

      for (element = 0; element < localNumberOfMeshElements; element++)
        {
          meshGroundwaterHead[element] = meshElementZSurface[element];
        }
    }
  
  error = writeGeometry(directory, geometryGroup, true);
  
  if (!error)
    {
      error = writeParameter(directory, parameterGroup, true);
    }
  
  if (!error)
    {
      error = writeState(directory, stateGroup, true, time, dt, geometryGroup, parameterGroup);
    }
  
  if (!error)
    {
      contribute();
    }
  else
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

bool FileManager::allVerticesUpdated()
{
  int  ii, jj;         // Loop counters.
  bool updated = true; // Flag to record whether we have found an unupdated vertex.
  
  for (ii = 0; updated && ii < localNumberOfMeshElements; ii++)
  {
      for (jj = 0; updated && jj < MeshElement::meshNeighborsSize; jj++)
        {
          updated = meshVertexUpdated[ii][jj];
        }
  }
  
  return updated;
}

bool FileManager::allElementsUpdated()
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
