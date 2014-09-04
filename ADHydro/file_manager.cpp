#include "file_manager.h"
#include "evapo_transpiration.h"
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

FileManager::FileManager()
{
  globalNumberOfMeshNodes        = 0;
  localMeshNodeStart             = 0;
  localNumberOfMeshNodes         = 0;
  globalNumberOfMeshElements     = 0;
  localMeshElementStart          = 0;
  localNumberOfMeshElements      = 0;
  globalNumberOfChannelNodes     = 0;
  localChannelNodeStart          = 0;
  localNumberOfChannelNodes      = 0;
  globalNumberOfChannelElements  = 0;
  localChannelElementStart       = 0;
  localNumberOfChannelElements   = 0;
  meshNodeX                      = NULL;
  meshNodeY                      = NULL;
  meshNodeZSurface               = NULL;
  meshNodeZBedrock               = NULL;
  meshElementVertices            = NULL;
  meshVertexX                    = NULL;
  meshVertexY                    = NULL;
  meshVertexZSurface             = NULL;
  meshVertexZBedrock             = NULL;
  meshElementX                   = NULL;
  meshElementY                   = NULL;
  meshElementZSurface            = NULL;
  meshElementZBedrock            = NULL;
  meshElementArea                = NULL;
  meshElementSlopeX              = NULL;
  meshElementSlopeY              = NULL;
  meshCatchment                  = NULL;
  meshConductivity               = NULL;
  meshPorosity                   = NULL;
  meshManningsN                  = NULL;
  meshSurfacewaterDepth          = NULL;
  meshSurfacewaterError          = NULL;
  meshGroundwaterHead            = NULL;
  meshGroundwaterError           = NULL;
  meshMeshNeighbors              = NULL;
  meshMeshNeighborsChannelEdge   = NULL;
  meshMeshNeighborsEdgeLength    = NULL;
  meshMeshNeighborsEdgeNormalX   = NULL;
  meshMeshNeighborsEdgeNormalY   = NULL;
  meshChannelNeighbors           = NULL;
  meshChannelNeighborsEdgeLength = NULL;
  channelNodeX                   = NULL;
  channelNodeY                   = NULL;
  channelNodeZBank               = NULL;
  channelNodeZBed                = NULL;
  channelElementVertices         = NULL;
  channelVertexX                 = NULL;
  channelVertexY                 = NULL;
  channelVertexZBank             = NULL;
  channelVertexZBed              = NULL;
  channelElementX                = NULL;
  channelElementY                = NULL;
  channelElementZBank            = NULL;
  channelElementZBed             = NULL;
  channelElementLength           = NULL;
  channelChannelType             = NULL;
  channelPermanentCode           = NULL;
  channelBaseWidth               = NULL;
  channelSideSlope               = NULL;
  channelBedConductivity         = NULL;
  channelBedThickness            = NULL;
  channelManningsN               = NULL;
  channelSurfacewaterDepth       = NULL;
  channelSurfacewaterError       = NULL;
  channelChannelNeighbors        = NULL;
  channelMeshNeighbors           = NULL;
  channelMeshNeighborsEdgeLength = NULL;
  meshVertexUpdated              = NULL;
  meshElementUpdated             = NULL;
  channelElementUpdated          = NULL;
  currentTime                    = 0.0;
  dt                             = 1.0;
  iteration                      = 1;
}

FileManager::~FileManager()
{
  if (NULL != meshNodeX)
    {
      delete[] meshNodeX;
    }
  
  if (NULL != meshNodeY)
    {
      delete[] meshNodeY;
    }
  
  if (NULL != meshNodeZSurface)
    {
      delete[] meshNodeZSurface;
    }
  
  if (NULL != meshNodeZBedrock)
    {
      delete[] meshNodeZBedrock;
    }
  
  if (NULL != meshElementVertices)
    {
      delete[] meshElementVertices;
    }
  
  if (NULL != meshVertexX)
    {
      delete[] meshVertexX;
    }
  
  if (NULL != meshVertexY)
    {
      delete[] meshVertexY;
    }
  
  if (NULL != meshVertexZSurface)
    {
      delete[] meshVertexZSurface;
    }
  
  if (NULL != meshVertexZBedrock)
    {
      delete[] meshVertexZBedrock;
    }
  
  if (NULL != meshElementX)
    {
      delete[] meshElementX;
    }
  
  if (NULL != meshElementY)
    {
      delete[] meshElementY;
    }
  
  if (NULL != meshElementZSurface)
    {
      delete[] meshElementZSurface;
    }
  
  if (NULL != meshElementZBedrock)
    {
      delete[] meshElementZBedrock;
    }
  
  if (NULL != meshElementArea)
    {
      delete[] meshElementArea;
    }
  
  if (NULL != meshElementSlopeX)
    {
      delete[] meshElementSlopeX;
    }
  
  if (NULL != meshElementSlopeY)
    {
      delete[] meshElementSlopeY;
    }
  
  if (NULL != meshCatchment)
    {
      delete[] meshCatchment;
    }
  
  if (NULL != meshConductivity)
    {
      delete[] meshConductivity;
    }
  
  if (NULL != meshPorosity)
    {
      delete[] meshPorosity;
    }
  
  if (NULL != meshManningsN)
    {
      delete[] meshManningsN;
    }
  
  if (NULL != meshSurfacewaterDepth)
    {
      delete[] meshSurfacewaterDepth;
    }
  
  if (NULL != meshSurfacewaterError)
    {
      delete[] meshSurfacewaterError;
    }
  
  if (NULL != meshGroundwaterHead)
    {
      delete[] meshGroundwaterHead;
    }
  
  if (NULL != meshGroundwaterError)
    {
      delete[] meshGroundwaterError;
    }
  
  if (NULL != meshMeshNeighbors)
    {
      delete[] meshMeshNeighbors;
    }
  
  if (NULL != meshMeshNeighborsChannelEdge)
    {
      delete[] meshMeshNeighborsChannelEdge;
    }
  
  if (NULL != meshMeshNeighborsEdgeLength)
    {
      delete[] meshMeshNeighborsEdgeLength;
    }
  
  if (NULL != meshMeshNeighborsEdgeNormalX)
    {
      delete[] meshMeshNeighborsEdgeNormalX;
    }
  
  if (NULL != meshMeshNeighborsEdgeNormalY)
    {
      delete[] meshMeshNeighborsEdgeNormalY;
    }
  
  if (NULL != meshChannelNeighbors)
    {
      delete[] meshChannelNeighbors;
    }
  
  if (NULL != meshChannelNeighborsEdgeLength)
    {
      delete[] meshChannelNeighborsEdgeLength;
    }
  
  if (NULL != channelNodeX)
    {
      delete[] channelNodeX;
    }
  
  if (NULL != channelNodeY)
    {
      delete[] channelNodeY;
    }
  
  if (NULL != channelNodeZBank)
    {
      delete[] channelNodeZBank;
    }
  
  if (NULL != channelNodeZBed)
    {
      delete[] channelNodeZBed;
    }
  
  if (NULL != channelElementVertices)
    {
      delete[] channelElementVertices;
    }
  
  if (NULL != channelVertexX)
    {
      delete[] channelVertexX;
    }
  
  if (NULL != channelVertexY)
    {
      delete[] channelVertexY;
    }
  
  if (NULL != channelVertexZBank)
    {
      delete[] channelVertexZBank;
    }
  
  if (NULL != channelVertexZBed)
    {
      delete[] channelVertexZBed;
    }
  
  if (NULL != channelElementX)
    {
      delete[] channelElementX;
    }
  
  if (NULL != channelElementY)
    {
      delete[] channelElementY;
    }
  
  if (NULL != channelElementZBank)
    {
      delete[] channelElementZBank;
    }
  
  if (NULL != channelElementZBed)
    {
      delete[] channelElementZBed;
    }
  
  if (NULL != channelElementLength)
    {
      delete[] channelElementLength;
    }
  
  if (NULL != channelChannelType)
    {
      delete[] channelChannelType;
    }
  
  if (NULL != channelPermanentCode)
    {
      delete[] channelPermanentCode;
    }
  
  if (NULL != channelBaseWidth)
    {
      delete[] channelBaseWidth;
    }
  
  if (NULL != channelSideSlope)
    {
      delete[] channelSideSlope;
    }
  
  if (NULL != channelBedConductivity)
    {
      delete[] channelBedConductivity;
    }
  
  if (NULL != channelBedThickness)
    {
      delete[] channelBedThickness;
    }
  
  if (NULL != channelManningsN)
    {
      delete[] channelManningsN;
    }
  
  if (NULL != channelSurfacewaterDepth)
    {
      delete[] channelSurfacewaterDepth;
    }
  
  if (NULL != channelSurfacewaterError)
    {
      delete[] channelSurfacewaterError;
    }
  
  if (NULL != channelChannelNeighbors)
    {
      delete[] channelChannelNeighbors;
    }
  
  if (NULL != channelMeshNeighbors)
    {
      delete[] channelMeshNeighbors;
    }
  
  if (NULL != channelMeshNeighborsEdgeLength)
    {
      delete[] channelMeshNeighborsEdgeLength;
    }
  
  if (NULL != meshVertexUpdated)
    {
      delete[] meshVertexUpdated;
    }
  
  if (NULL != meshElementUpdated)
    {
      delete[] meshElementUpdated;
    }
  
  if (NULL != channelElementUpdated)
    {
      delete[] channelElementUpdated;
    }
}

void FileManager::initializeHardcodedMesh()
{
  int    ii, jj; // Loop counters.
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
  
  currentTime = 0.0;
  dt          = 1.0;
  iteration   = 1;
  
  meshVertexUpdated     = NULL; // Will be allocated and freed if and when we send messages to update vertices from nodes.
  meshElementUpdated    = new bool[localNumberOfMeshElements];
  channelElementUpdated = new bool[localNumberOfChannelElements];
  
  // Have to call evapoTranspirationInit once on each Pe. This is a convenient place to do that.
  if (evapoTranspirationInit("."))
    {
      CkExit();
    }
  
  contribute();
}

void FileManager::initializeFromNetCDFFiles(size_t directorySize, const char* directory)
{
  bool   error         = false;  // Error flag.
  char*  nameString    = NULL;   // Temporary string for file names.
  size_t nameStringSize;         // Size of buffer allocated for nameString.
  size_t numPrinted;             // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;            // Return value of NetCDF functions.
  int    stateFileID;            // ID of NetCDF file.
  bool   stateFileOpen = false;  // Whether stateFileID refers to an open file.
  int    fileID;                 // ID of NetCDF file.
  bool   fileOpen      = false;  // Whether fileID refers to an open file.
  int    dimID;                  // ID of dimension in NetCDF file.
  int    varID;                  // ID of variable in NetCDF file.
  size_t geometryInstance;       // Instance index for geometry file.
  size_t parameterInstance;      // Instance index for parameter file.
  size_t stateInstance;          // Instance index for state file.
  size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when writing to NetCDF file.
  size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when writing to NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::initializeFromNetCDFFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file name strings.
      nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // The longest file name is parameter.nc.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: incorrect return value of snprintf when generating state file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &stateFileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to open NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          stateFileOpen = true;
        }
    }
  
  // Get the number of existing instances.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(stateFileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get dimension instances in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(stateFileID, dimID, &stateInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get length of dimension instances in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      if (0 < stateInstance)
        {
          // We're not creating a new instance so use the last instance with index one less than the dimension length.
          stateInstance--;
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      else
        {
          // We're not creating a new instance so it's an error if there's not an existing one.
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: not creating a new instance and no existing instance in NetCDF state file %s.\n",
                  nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Read variables.
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "iteration", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable iteration in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_ushort(stateFileID, varID, start, count, &iteration);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable iteration in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "currentTime", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable currentTime in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_double(stateFileID, varID, start, count, &currentTime);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable currentTime in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "dt", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable dt in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_double(stateFileID, varID, start, count, &dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable dt in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "geometryInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable geometryInstance in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      // Assumes size_t is eight bytes when casting to unsigned long long.
      CkAssert(sizeof(size_t) == sizeof(unsigned long long));
      
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_ulonglong(stateFileID, varID, start, count, (unsigned long long *)&geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable geometryInstance in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "parameterInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable parameterInstance in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      // Assumes size_t is eight bytes when casting to unsigned long long.
      CkAssert(sizeof(size_t) == sizeof(unsigned long long));
      
      start[0]    = stateInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_ulonglong(stateFileID, varID, start, count, (unsigned long long *)&parameterInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable parameterInstance in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: incorrect return value of snprintf when generating geometry file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to open NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Read variables.
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "numberOfMeshNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable numberOfMeshNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = geometryInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_int(fileID, varID, start, count, &globalNumberOfMeshNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable numberOfMeshNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "numberOfMeshElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable numberOfMeshElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = geometryInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_int(fileID, varID, start, count, &globalNumberOfMeshElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable numberOfMeshElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "numberOfChannelNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable numberOfChannelNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = geometryInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_int(fileID, varID, start, count, &globalNumberOfChannelNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable numberOfChannelNodes in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "numberOfChannelElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to get variable numberOfChannelElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  if (!error)
    {
      start[0]    = geometryInstance;
      count[0]    = 1;
      ncErrorCode = nc_get_vara_int(fileID, varID, start, count, &globalNumberOfChannelElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable numberOfChannelElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      localStartAndNumber(&localMeshNodeStart,       &localNumberOfMeshNodes,       globalNumberOfMeshNodes);
      localStartAndNumber(&localMeshElementStart,    &localNumberOfMeshElements,    globalNumberOfMeshElements);
      localStartAndNumber(&localChannelNodeStart,    &localNumberOfChannelNodes,    globalNumberOfChannelNodes);
      localStartAndNumber(&localChannelElementStart, &localNumberOfChannelElements, globalNumberOfChannelElements);

      ncErrorCode = nc_inq_varid(fileID, "meshNodeX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshNodeX   = new double[localNumberOfMeshNodes];
          start[0]    = geometryInstance;
          start[1]    = localMeshNodeStart;
          count[0]    = 1;
          count[1]    = localNumberOfMeshNodes;
          ncErrorCode = nc_get_vara_double(fileID, varID, start, count, meshNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshNodeX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshNodeY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshNodeY   = new double[localNumberOfMeshNodes];
          start[0]    = geometryInstance;
          start[1]    = localMeshNodeStart;
          count[0]    = 1;
          count[1]    = localNumberOfMeshNodes;
          ncErrorCode = nc_get_vara_double(fileID, varID, start, count, meshNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshNodeY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshNodeZSurface", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshNodeZSurface = new double[localNumberOfMeshNodes];
          start[0]         = geometryInstance;
          start[1]         = localMeshNodeStart;
          count[0]         = 1;
          count[1]         = localNumberOfMeshNodes;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, meshNodeZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshNodeZSurface in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshNodeZBedrock", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshNodeZBedrock = new double[localNumberOfMeshNodes];
          start[0]         = geometryInstance;
          start[1]         = localMeshNodeStart;
          count[0]         = 1;
          count[1]         = localNumberOfMeshNodes;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, meshNodeZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshNodeZBedrock in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementVertices", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementVertices = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]            = geometryInstance;
          start[1]            = localMeshElementStart;
          start[2]            = 0;
          count[0]            = 1;
          count[1]            = localNumberOfMeshElements;
          count[2]            = MeshElement::meshNeighborsSize;
          ncErrorCode         = nc_get_vara_int(fileID, varID, start, count, (int*)meshElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshElementVertices in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshVertexX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshVertexX = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]    = geometryInstance;
          start[1]    = localMeshElementStart;
          start[2]    = 0;
          count[0]    = 1;
          count[1]    = localNumberOfMeshElements;
          count[2]    = MeshElement::meshNeighborsSize;
          ncErrorCode = nc_get_vara_double(fileID, varID, start, count, (double*)meshVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshVertexX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshVertexY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshVertexY = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]    = geometryInstance;
          start[1]    = localMeshElementStart;
          start[2]    = 0;
          count[0]    = 1;
          count[1]    = localNumberOfMeshElements;
          count[2]    = MeshElement::meshNeighborsSize;
          ncErrorCode = nc_get_vara_double(fileID, varID, start, count, (double*)meshVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshVertexY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshVertexZSurface", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshVertexZSurface = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]           = geometryInstance;
          start[1]           = localMeshElementStart;
          start[2]           = 0;
          count[0]           = 1;
          count[1]           = localNumberOfMeshElements;
          count[2]           = MeshElement::meshNeighborsSize;
          ncErrorCode        = nc_get_vara_double(fileID, varID, start, count, (double*)meshVertexZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshVertexZSurface in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshVertexZBedrock", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshVertexZBedrock = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]           = geometryInstance;
          start[1]           = localMeshElementStart;
          start[2]           = 0;
          count[0]           = 1;
          count[1]           = localNumberOfMeshElements;
          count[2]           = MeshElement::meshNeighborsSize;
          ncErrorCode        = nc_get_vara_double(fileID, varID, start, count, (double*)meshVertexZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshVertexZBedrock in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementX = new double[localNumberOfMeshElements];
          start[0]     = geometryInstance;
          start[1]     = localMeshElementStart;
          count[0]     = 1;
          count[1]     = localNumberOfMeshElements;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, meshElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshElementX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementY = new double[localNumberOfMeshElements];
          start[0]     = geometryInstance;
          start[1]     = localMeshElementStart;
          count[0]     = 1;
          count[1]     = localNumberOfMeshElements;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, meshElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshElementY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementZSurface", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementZSurface = new double[localNumberOfMeshElements];
          start[0]            = geometryInstance;
          start[1]            = localMeshElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfMeshElements;
          ncErrorCode         = nc_get_vara_double(fileID, varID, start, count, meshElementZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshElementZSurface in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementZBedrock", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementZBedrock = new double[localNumberOfMeshElements];
          start[0]            = geometryInstance;
          start[1]            = localMeshElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfMeshElements;
          ncErrorCode         = nc_get_vara_double(fileID, varID, start, count, meshElementZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshElementZBedrock in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementArea", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementArea = new double[localNumberOfMeshElements];
          start[0]        = geometryInstance;
          start[1]        = localMeshElementStart;
          count[0]        = 1;
          count[1]        = localNumberOfMeshElements;
          ncErrorCode     = nc_get_vara_double(fileID, varID, start, count, meshElementArea);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshElementArea in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementSlopeX = new double[localNumberOfMeshElements];
          start[0]          = geometryInstance;
          start[1]          = localMeshElementStart;
          count[0]          = 1;
          count[1]          = localNumberOfMeshElements;
          ncErrorCode       = nc_get_vara_double(fileID, varID, start, count, meshElementSlopeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshElementSlopeX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshElementSlopeY = new double[localNumberOfMeshElements];
          start[0]          = geometryInstance;
          start[1]          = localMeshElementStart;
          count[0]          = 1;
          count[1]          = localNumberOfMeshElements;
          ncErrorCode       = nc_get_vara_double(fileID, varID, start, count, meshElementSlopeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshElementSlopeY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighbors", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshMeshNeighbors = new int[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]          = geometryInstance;
          start[1]          = localMeshElementStart;
          start[2]          = 0;
          count[0]          = 1;
          count[1]          = localNumberOfMeshElements;
          count[2]          = MeshElement::meshNeighborsSize;
          ncErrorCode       = nc_get_vara_int(fileID, varID, start, count, (int*)meshMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshMeshNeighbors in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsChannelEdge", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          // Assumes bool is one byte when casting to signed char.
          CkAssert(sizeof(bool) == sizeof(signed char));
          
          meshMeshNeighborsChannelEdge = new bool[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]                     = geometryInstance;
          start[1]                     = localMeshElementStart;
          start[2]                     = 0;
          count[0]                     = 1;
          count[1]                     = localNumberOfMeshElements;
          count[2]                     = MeshElement::meshNeighborsSize;
          ncErrorCode                  = nc_get_vara_schar(fileID, varID, start, count, (signed char*)meshMeshNeighborsChannelEdge);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshMeshNeighborsChannelEdge in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeLength", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshMeshNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]                    = geometryInstance;
          start[1]                    = localMeshElementStart;
          start[2]                    = 0;
          count[0]                    = 1;
          count[1]                    = localNumberOfMeshElements;
          count[2]                    = MeshElement::meshNeighborsSize;
          ncErrorCode                 = nc_get_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshMeshNeighborsEdgeNormalX = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]                     = geometryInstance;
          start[1]                     = localMeshElementStart;
          start[2]                     = 0;
          count[0]                     = 1;
          count[1]                     = localNumberOfMeshElements;
          count[2]                     = MeshElement::meshNeighborsSize;
          ncErrorCode                  = nc_get_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeNormalX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshMeshNeighborsEdgeNormalY = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          start[0]                     = geometryInstance;
          start[1]                     = localMeshElementStart;
          start[2]                     = 0;
          count[0]                     = 1;
          count[1]                     = localNumberOfMeshElements;
          count[2]                     = MeshElement::meshNeighborsSize;
          ncErrorCode                  = nc_get_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeNormalY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighbors", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshChannelNeighbors = new int[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
          start[0]             = geometryInstance;
          start[1]             = localMeshElementStart;
          start[2]             = 0;
          count[0]             = 1;
          count[1]             = localNumberOfMeshElements;
          count[2]             = MeshElement::channelNeighborsSize;
          ncErrorCode          = nc_get_vara_int(fileID, varID, start, count, (int*)meshChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshChannelNeighbors in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighborsEdgeLength", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshChannelNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::channelNeighborsSize];
          start[0]                       = geometryInstance;
          start[1]                       = localMeshElementStart;
          start[2]                       = 0;
          count[0]                       = 1;
          count[1]                       = localNumberOfMeshElements;
          count[2]                       = MeshElement::channelNeighborsSize;
          ncErrorCode                    = nc_get_vara_double(fileID, varID, start, count, (double*)meshChannelNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshChannelNeighborsEdgeLength in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelNodeX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelNodeX = new double[localNumberOfChannelNodes];
          start[0]     = geometryInstance;
          start[1]     = localChannelNodeStart;
          count[0]     = 1;
          count[1]     = localNumberOfChannelNodes;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, channelNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelNodeX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelNodeY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelNodeY = new double[localNumberOfChannelNodes];
          start[0]     = geometryInstance;
          start[1]     = localChannelNodeStart;
          count[0]     = 1;
          count[1]     = localNumberOfChannelNodes;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, channelNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelNodeY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelNodeZBank", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelNodeZBank = new double[localNumberOfChannelNodes];
          start[0]         = geometryInstance;
          start[1]         = localChannelNodeStart;
          count[0]         = 1;
          count[1]         = localNumberOfChannelNodes;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, channelNodeZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelNodeZBank in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelNodeZBed", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelNodeZBed = new double[localNumberOfChannelNodes];
          start[0]        = geometryInstance;
          start[1]        = localChannelNodeStart;
          count[0]        = 1;
          count[1]        = localNumberOfChannelNodes;
          ncErrorCode     = nc_get_vara_double(fileID, varID, start, count, channelNodeZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelNodeZBed in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementVertices", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementVertices = new int[localNumberOfChannelElements][ChannelElement::channelVerticesSize + 2];
          start[0]               = geometryInstance;
          start[1]               = localChannelElementStart;
          start[2]               = 0;
          count[0]               = 1;
          count[1]               = localNumberOfChannelElements;
          count[2]               = ChannelElement::channelVerticesSize + 2;
          ncErrorCode            = nc_get_vara_int(fileID, varID, start, count, (int*)channelElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelElementVertices in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelVertexX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelVertexX = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          start[0]       = geometryInstance;
          start[1]       = localChannelElementStart;
          start[2]       = 0;
          count[0]       = 1;
          count[1]       = localNumberOfChannelElements;
          count[2]       = ChannelElement::channelVerticesSize;
          ncErrorCode    = nc_get_vara_double(fileID, varID, start, count, (double*)channelVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelVertexX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelVertexY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelVertexY = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          start[0]       = geometryInstance;
          start[1]       = localChannelElementStart;
          start[2]       = 0;
          count[0]       = 1;
          count[1]       = localNumberOfChannelElements;
          count[2]       = ChannelElement::channelVerticesSize;
          ncErrorCode    = nc_get_vara_double(fileID, varID, start, count, (double*)channelVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelVertexY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelVertexZBank", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelVertexZBank = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          start[0]           = geometryInstance;
          start[1]           = localChannelElementStart;
          start[2]           = 0;
          count[0]           = 1;
          count[1]           = localNumberOfChannelElements;
          count[2]           = ChannelElement::channelVerticesSize;
          ncErrorCode        = nc_get_vara_double(fileID, varID, start, count, (double*)channelVertexZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelVertexZBank in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelVertexZBed", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelVertexZBed = new double[localNumberOfChannelElements][ChannelElement::channelVerticesSize];
          start[0]          = geometryInstance;
          start[1]          = localChannelElementStart;
          start[2]          = 0;
          count[0]          = 1;
          count[1]          = localNumberOfChannelElements;
          count[2]          = ChannelElement::channelVerticesSize;
          ncErrorCode       = nc_get_vara_double(fileID, varID, start, count, (double*)channelVertexZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelVertexZBed in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementX", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementX = new double[localNumberOfChannelElements];
          start[0]        = geometryInstance;
          start[1]        = localChannelElementStart;
          count[0]        = 1;
          count[1]        = localNumberOfChannelElements;
          ncErrorCode     = nc_get_vara_double(fileID, varID, start, count, channelElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelElementX in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementY", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementY = new double[localNumberOfChannelElements];
          start[0]        = geometryInstance;
          start[1]        = localChannelElementStart;
          count[0]        = 1;
          count[1]        = localNumberOfChannelElements;
          ncErrorCode     = nc_get_vara_double(fileID, varID, start, count, channelElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelElementY in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementZBank", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementZBank = new double[localNumberOfChannelElements];
          start[0]            = geometryInstance;
          start[1]            = localChannelElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfChannelElements;
          ncErrorCode         = nc_get_vara_double(fileID, varID, start, count, channelElementZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelElementZBank in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementZBed", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementZBed = new double[localNumberOfChannelElements];
          start[0]           = geometryInstance;
          start[1]           = localChannelElementStart;
          count[0]           = 1;
          count[1]           = localNumberOfChannelElements;
          ncErrorCode        = nc_get_vara_double(fileID, varID, start, count, channelElementZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelElementZBed in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelElementLength", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelElementLength = new double[localNumberOfChannelElements];
          start[0]             = geometryInstance;
          start[1]             = localChannelElementStart;
          count[0]             = 1;
          count[1]             = localNumberOfChannelElements;
          ncErrorCode          = nc_get_vara_double(fileID, varID, start, count, channelElementLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelElementLength in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelChannelNeighbors", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelChannelNeighbors = new int[localNumberOfChannelElements][ChannelElement::channelNeighborsSize];
          start[0]                = geometryInstance;
          start[1]                = localChannelElementStart;
          start[2]                = 0;
          count[0]                = 1;
          count[1]                = localNumberOfChannelElements;
          count[2]                = ChannelElement::channelNeighborsSize;
          ncErrorCode             = nc_get_vara_int(fileID, varID, start, count, (int*)channelChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelChannelNeighbors in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighbors", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelMeshNeighbors = new int[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
          start[0]             = geometryInstance;
          start[1]             = localChannelElementStart;
          start[2]             = 0;
          count[0]             = 1;
          count[1]             = localNumberOfChannelElements;
          count[2]             = ChannelElement::meshNeighborsSize;
          ncErrorCode          = nc_get_vara_int(fileID, varID, start, count, (int*)channelMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelMeshNeighbors in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighborsEdgeLength", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelMeshNeighborsEdgeLength = new double[localNumberOfChannelElements][ChannelElement::meshNeighborsSize];
          start[0]                       = geometryInstance;
          start[1]                       = localChannelElementStart;
          start[2]                       = 0;
          count[0]                       = 1;
          count[1]                       = localNumberOfChannelElements;
          count[2]                       = ChannelElement::meshNeighborsSize;
          ncErrorCode                    = nc_get_vara_double(fileID, varID, start, count, (double*)channelMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to close NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: incorrect return value of snprintf when generating parameter file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to open NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Read variables.
  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshCatchment", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshCatchment = new int[localNumberOfMeshElements];
          start[0]      = parameterInstance;
          start[1]      = localMeshElementStart;
          count[0]      = 1;
          count[1]      = localNumberOfMeshElements;
          ncErrorCode   = nc_get_vara_int(fileID, varID, start, count, meshCatchment);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshCatchment in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshConductivity", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshConductivity = new double[localNumberOfMeshElements];
          start[0]         = parameterInstance;
          start[1]         = localMeshElementStart;
          count[0]         = 1;
          count[1]         = localNumberOfMeshElements;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, meshConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshConductivity in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshPorosity", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshPorosity = new double[localNumberOfMeshElements];
          start[0]     = parameterInstance;
          start[1]     = localMeshElementStart;
          count[0]     = 1;
          count[1]     = localNumberOfMeshElements;
          ncErrorCode  = nc_get_vara_double(fileID, varID, start, count, meshPorosity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshPorosity in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "meshManningsN", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshManningsN = new double[localNumberOfMeshElements];
          start[0]      = parameterInstance;
          start[1]      = localMeshElementStart;
          count[0]      = 1;
          count[1]      = localNumberOfMeshElements;
          ncErrorCode   = nc_get_vara_double(fileID, varID, start, count, meshManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshManningsN in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelChannelType", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          // Assumes ChannelTypeEnum is four bytes when casting to int.
          CkAssert(sizeof(ChannelTypeEnum) == sizeof(int));
          
          channelChannelType = new ChannelTypeEnum[localNumberOfChannelElements];
          start[0]           = parameterInstance;
          start[1]           = localChannelElementStart;
          count[0]           = 1;
          count[1]           = localNumberOfChannelElements;
          ncErrorCode        = nc_get_vara_int(fileID, varID, start, count, (int*)channelChannelType);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelChannelType in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelPermanentCode", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelPermanentCode = new int[localNumberOfChannelElements];
          start[0]             = parameterInstance;
          start[1]             = localChannelElementStart;
          count[0]             = 1;
          count[1]             = localNumberOfChannelElements;
          ncErrorCode          = nc_get_vara_int(fileID, varID, start, count, channelPermanentCode);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelPermanentCode in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelBaseWidth", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelBaseWidth = new double[localNumberOfChannelElements];
          start[0]         = parameterInstance;
          start[1]         = localChannelElementStart;
          count[0]         = 1;
          count[1]         = localNumberOfChannelElements;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, channelBaseWidth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelBaseWidth in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelSideSlope", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelSideSlope = new double[localNumberOfChannelElements];
          start[0]         = parameterInstance;
          start[1]         = localChannelElementStart;
          count[0]         = 1;
          count[1]         = localNumberOfChannelElements;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, channelSideSlope);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelSideSlope in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelBedConductivity", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelBedConductivity = new double[localNumberOfChannelElements];
          start[0]               = parameterInstance;
          start[1]               = localChannelElementStart;
          count[0]               = 1;
          count[1]               = localNumberOfChannelElements;
          ncErrorCode            = nc_get_vara_double(fileID, varID, start, count, channelBedConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelBedConductivity in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelBedThickness", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelBedThickness = new double[localNumberOfChannelElements];
          start[0]            = parameterInstance;
          start[1]            = localChannelElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfChannelElements;
          ncErrorCode         = nc_get_vara_double(fileID, varID, start, count, channelBedThickness);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelBedThickness in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(fileID, "channelManningsN", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelManningsN = new double[localNumberOfChannelElements];
          start[0]         = parameterInstance;
          start[1]         = localChannelElementStart;
          count[0]         = 1;
          count[1]         = localNumberOfChannelElements;
          ncErrorCode      = nc_get_vara_double(fileID, varID, start, count, channelManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelManningsN in NetCDF parameter file %s.  "
                      "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to close NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read variables.
  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "meshSurfacewaterDepth", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshSurfacewaterDepth = new double[localNumberOfMeshElements];
          start[0]              = stateInstance;
          start[1]              = localMeshElementStart;
          count[0]              = 1;
          count[1]              = localNumberOfMeshElements;
          ncErrorCode           = nc_get_vara_double(stateFileID, varID, start, count, meshSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshSurfacewaterDepth in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "meshSurfacewaterError", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshSurfacewaterError = new double[localNumberOfMeshElements];
          start[0]              = stateInstance;
          start[1]              = localMeshElementStart;
          count[0]              = 1;
          count[1]              = localNumberOfMeshElements;
          ncErrorCode           = nc_get_vara_double(stateFileID, varID, start, count, meshSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshSurfacewaterError in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "meshGroundwaterHead", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshGroundwaterHead = new double[localNumberOfMeshElements];
          start[0]            = stateInstance;
          start[1]            = localMeshElementStart;
          count[0]            = 1;
          count[1]            = localNumberOfMeshElements;
          ncErrorCode         = nc_get_vara_double(stateFileID, varID, start, count, meshGroundwaterHead);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshGroundwaterHead in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "meshGroundwaterError", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          meshGroundwaterError = new double[localNumberOfMeshElements];
          start[0]             = stateInstance;
          start[1]             = localMeshElementStart;
          count[0]             = 1;
          count[1]             = localNumberOfMeshElements;
          ncErrorCode          = nc_get_vara_double(stateFileID, varID, start, count, meshGroundwaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable meshGroundwaterError in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "channelSurfacewaterDepth", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelSurfacewaterDepth = new double[localNumberOfChannelElements];
          start[0]                 = stateInstance;
          start[1]                 = localChannelElementStart;
          count[0]                 = 1;
          count[1]                 = localNumberOfChannelElements;
          ncErrorCode              = nc_get_vara_double(stateFileID, varID, start, count, channelSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelSurfacewaterDepth in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (!error)
    {
      ncErrorCode = nc_inq_varid(stateFileID, "channelSurfacewaterError", &varID);

      if (NC_NOERR == ncErrorCode)
        {
          channelSurfacewaterError = new double[localNumberOfChannelElements];
          start[0]                 = stateInstance;
          start[1]                 = localChannelElementStart;
          count[0]                 = 1;
          count[1]                 = localNumberOfChannelElements;
          ncErrorCode              = nc_get_vara_double(stateFileID, varID, start, count, channelSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to read variable channelSurfacewaterError in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Close file.
  if (stateFileOpen)
    {
      ncErrorCode   = nc_close(stateFileID);
      stateFileOpen = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::initializeFromNetCDFFiles: unable to close NetCDF state file.  NetCDF error message: %s.\n",
                  nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      meshElementUpdated    = new bool[localNumberOfMeshElements];
      channelElementUpdated = new bool[localNumberOfChannelElements];
    }
  
  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
    }
  
  // Have to call evapoTranspirationInit once on each Pe. This is a convenient place to do that.
  if (!error)
    {
      error = evapoTranspirationInit(directory);
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

void FileManager::calculateDerivedValues()
{
  int  ii, jj;                      // Loop counters.
  bool needToGetVertexData = false; // Whether we need to get any vertex data.

  // Get vertex data from node data.
  if (NULL != meshElementVertices)
    {
      if (NULL == meshVertexX && NULL != meshNodeX)
        {
          meshVertexX         = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetVertexData = true;
        }

      if (NULL == meshVertexY && NULL != meshNodeY)
        {
          meshVertexY         = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetVertexData = true;
        }

      if (NULL == meshVertexZSurface && NULL != meshNodeZSurface)
        {
          meshVertexZSurface  = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetVertexData = true;
        }

      if (NULL == meshVertexZBedrock && NULL != meshNodeZBedrock)
        {
          meshVertexZBedrock  = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];
          needToGetVertexData = true;
        }
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

              // FIXME improve efficiency.  Don't send messages to myself.  Don't send duplicate messages for the same node.
              thisProxy[home(meshElementVertices[ii][jj], globalNumberOfMeshNodes)].getMeshVertexDataMessage(CkMyPe(), ii + localMeshElementStart, jj,
                                                                                                             meshElementVertices[ii][jj]);
            }
        }

      thisProxy[CkMyPe()].waitForVertexData();
    }
  else
    {
      finishCalculateDerivedValues();
    }
}

void FileManager::getMeshVertexDataMessage(int requester, int element, int vertex, int node)
{
  double x        = 0.0;
  double y        = 0.0;
  double zSurface = 0.0;
  double zBedrock = 0.0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= requester && requester < CkNumPes()))
    {
      CkError("ERROR in FileManager::getMeshVertexDataMessage: requester must be greater than or equal to zero and less than CkNumPes().\n");
      CkExit();
    }

  if (!(0 <= element && element < globalNumberOfMeshElements))
    {
      CkError("ERROR in FileManager::getMeshVertexDataMessage: element must be greater than or equal to zero and less than "
              "globalNumberOfMeshElements.\n");
      CkExit();
    }

  if (!(0 <= vertex && vertex < MeshElement::meshNeighborsSize))
    {
      CkError("ERROR in FileManager::getMeshVertexDataMessage: vertex must be greater than or equal to zero and less than "
              "MeshElement::meshNeighborsSize.\n");
      CkExit();
    }

  if (!(localMeshNodeStart <= node && node < localMeshNodeStart + localNumberOfMeshNodes))
    {
      CkError("ERROR in FileManager::getMeshVertexDataMessage: node data not owned by this local branch.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (NULL != meshNodeX)
    {
      x = meshNodeX[node - localMeshNodeStart];
    }

  if (NULL != meshNodeY)
    {
      y = meshNodeY[node - localMeshNodeStart];
    }

  if (NULL != meshNodeZSurface)
    {
      zSurface = meshNodeZSurface[node - localMeshNodeStart];
    }

  if (NULL != meshNodeZBedrock)
    {
      zBedrock = meshNodeZBedrock[node - localMeshNodeStart];
    }

  thisProxy[requester].meshVertexDataMessage(element, vertex, x, y, zSurface, zBedrock);
}

void FileManager::readForcingData(CProxy_MeshElement meshProxy)
{
  int ii; // Loop counter.
  
  // FIXME implement
  
  for (ii = localMeshElementStart; ii < localMeshElementStart + localNumberOfMeshElements; ii++)
    {
      meshProxy[ii].forcingDataMessage(20.0, 0.8, 0.8, 300.0, 101300.0, 101175.0, 0.0, 0.0,0.02, 0.0, 800.0, 200.0, 0.001, 300.0, 30000.0);
    }
}

void FileManager::createFiles(size_t directorySize, const char* directory)
{
  bool   error      = false;                     // Error flag.
  char*  nameString = NULL;                      // Temporary string for file names.
  size_t nameStringSize;                         // Size of buffer allocated for nameString.
  size_t numPrinted;                             // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;                            // Return value of NetCDF functions.
  int    fileID;                                 // ID of NetCDF file.
  bool   fileOpen   = false;                     // Whether fileID refers to an open file.
  int    instancesDimID;                         // ID of dimension in NetCDF file.
  int    meshNodesDimID;                         // ID of dimension in NetCDF file.
  int    meshElementsDimID;                      // ID of dimension in NetCDF file.
  int    meshMeshNeighborsDimID;                 // ID of dimension in NetCDF file.
  int    meshChannelNeighborsDimID;              // ID of dimension in NetCDF file.
  int    channelNodesDimID;                      // ID of dimension in NetCDF file.
  int    channelElementsDimID;                   // ID of dimension in NetCDF file.
  int    sizeOfChannelElementVerticesArrayDimID; // ID of dimension in NetCDF file.
  int    channelVerticesDimID;                   // ID of dimension in NetCDF file.
  int    channelChannelNeighborsDimID;           // ID of dimension in NetCDF file.
  int    channelMeshNeighborsDimID;              // ID of dimension in NetCDF file.
  int    dimIDs[NC_MAX_VAR_DIMS];                // For passing dimension IDs.
  int    varID;                                  // ID of variable in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::createFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file name strings.
      nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // The longest file name is parameter.nc.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // FIXME make directory?

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::createFiles: incorrect return value of snprintf when generating geometry file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file.
  if (!error)
    {
      ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "instances", NC_UNLIMITED, &instancesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension instances in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshNodes", NC_UNLIMITED, &meshNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshNodes in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshElements in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshMeshNeighbors", MeshElement::meshNeighborsSize, &meshMeshNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshMeshNeighbors in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshChannelNeighbors", MeshElement::channelNeighborsSize, &meshChannelNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshChannelNeighbors in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelNodes", NC_UNLIMITED, &channelNodesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelNodes in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelElements in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "sizeOfChannelElementVerticesArray", ChannelElement::channelVerticesSize + 2, &sizeOfChannelElementVerticesArrayDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension sizeOfChannelElementVerticesArray in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelVertices", ChannelElement::channelVerticesSize, &channelVerticesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelVertices in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelChannelNeighbors", ChannelElement::channelNeighborsSize, &channelChannelNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelChannelNeighbors in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelMeshNeighbors", ChannelElement::meshNeighborsSize, &channelMeshNeighborsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelMeshNeighbors in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "numberOfMeshNodes", NC_INT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable numberOfMeshNodes in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "numberOfMeshElements", NC_INT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable numberOfMeshElements in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "numberOfChannelNodes", NC_INT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable numberOfChannelNodes in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "numberOfChannelElements", NC_INT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable numberOfChannelElements in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshNodesDimID;
      ncErrorCode = nc_def_var(fileID, "meshNodeX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshNodesDimID;
      ncErrorCode = nc_def_var(fileID, "meshNodeY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeZSurface)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshNodesDimID;
      ncErrorCode = nc_def_var(fileID, "meshNodeZSurface", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshNodeZSurface in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshNodeZBedrock)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshNodesDimID;
      ncErrorCode = nc_def_var(fileID, "meshNodeZBedrock", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshNodeZBedrock in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementVertices)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementVertices", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementVertices in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshVertexX", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshVertexX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshVertexY", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshVertexY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexZSurface)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshVertexZSurface", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshVertexZSurface in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshVertexZBedrock)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshVertexZBedrock", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshVertexZBedrock in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementZSurface)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementZSurface", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementZSurface in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementZBedrock)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementZBedrock", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementZBedrock in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementArea)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementArea", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementArea in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementSlopeX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementSlopeX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementSlopeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshElementSlopeY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshElementSlopeY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshElementSlopeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighbors)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighbors", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighbors in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsChannelEdge)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighborsChannelEdge", NC_BYTE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighborsChannelEdge in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeLength)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighborsEdgeLength", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighborsEdgeNormalX", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshMeshNeighborsEdgeNormalY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshMeshNeighborsEdgeNormalY", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshChannelNeighbors)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshChannelNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshChannelNeighbors", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshChannelNeighbors in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshChannelNeighborsEdgeLength)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      dimIDs[2]   = meshChannelNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "meshChannelNeighborsEdgeLength", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshChannelNeighborsEdgeLength in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelNodesDimID;
      ncErrorCode = nc_def_var(fileID, "channelNodeX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelNodesDimID;
      ncErrorCode = nc_def_var(fileID, "channelNodeY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeZBank)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelNodesDimID;
      ncErrorCode = nc_def_var(fileID, "channelNodeZBank", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelNodeZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelNodeZBed)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelNodesDimID;
      ncErrorCode = nc_def_var(fileID, "channelNodeZBed", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelNodeZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementVertices)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = sizeOfChannelElementVerticesArrayDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementVertices", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementVertices in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelVerticesDimID;
      ncErrorCode = nc_def_var(fileID, "channelVertexX", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelVertexX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelVerticesDimID;
      ncErrorCode = nc_def_var(fileID, "channelVertexY", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelVertexY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexZBank)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelVerticesDimID;
      ncErrorCode = nc_def_var(fileID, "channelVertexZBank", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelVertexZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelVertexZBed)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelVerticesDimID;
      ncErrorCode = nc_def_var(fileID, "channelVertexZBed", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelVertexZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementX)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementX", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementY)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementY", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementZBank)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementZBank", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementZBed)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementZBed", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelElementLength)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelElementLength", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelElementLength in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelChannelNeighbors)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelChannelNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "channelChannelNeighbors", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelChannelNeighbors in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelMeshNeighbors)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "channelMeshNeighbors", NC_INT, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelMeshNeighbors in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelMeshNeighborsEdgeLength)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      dimIDs[2]   = channelMeshNeighborsDimID;
      ncErrorCode = nc_def_var(fileID, "channelMeshNeighborsEdgeLength", NC_DOUBLE, 3, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to close NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::createFiles: incorrect return value of snprintf when generating parameter file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file.
  if (!error)
    {
      ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "instances", NC_UNLIMITED, &instancesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension instances in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshElements in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelElements in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error && NULL != meshCatchment)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshCatchment", NC_INT, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshCatchment in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshConductivity)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshConductivity", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshConductivity in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshPorosity)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshPorosity", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshPorosity in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshManningsN)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshManningsN", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshManningsN in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelChannelType)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelChannelType", NC_INT, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelChannelType in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelPermanentCode)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelPermanentCode", NC_INT, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelPermanentCode in NetCDF parameter file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBaseWidth)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelBaseWidth", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelBaseWidth in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSideSlope)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelSideSlope", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelSideSlope in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBedConductivity)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelBedConductivity", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelBedConductivity in NetCDF parameter file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelBedThickness)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelBedThickness", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelBedThickness in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelManningsN)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelManningsN", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelManningsN in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to close NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::createFiles: incorrect return value of snprintf when generating state file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file.
  if (!error)
    {
      ncErrorCode = nc_create_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Create dimensions.
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "instances", NC_UNLIMITED, &instancesDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension instances in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "meshElements", NC_UNLIMITED, &meshElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension meshElements in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_def_dim(fileID, "channelElements", NC_UNLIMITED, &channelElementsDimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create dimension channelElements in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create variables.
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "iteration", NC_USHORT, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable iteration in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "currentTime", NC_DOUBLE, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable currentTime in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "dt", NC_DOUBLE, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "geometryInstance", NC_UINT64, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable geometryInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      dimIDs[0]   = instancesDimID;
      ncErrorCode = nc_def_var(fileID, "parameterInstance", NC_UINT64, 1, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable parameterInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshSurfacewaterDepth)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshSurfacewaterDepth", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshSurfacewaterDepth in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshSurfacewaterError)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshSurfacewaterError", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshSurfacewaterError in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshGroundwaterHead)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshGroundwaterHead", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshGroundwaterHead in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != meshGroundwaterError)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = meshElementsDimID;
      ncErrorCode = nc_def_var(fileID, "meshGroundwaterError", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable meshGroundwaterError in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSurfacewaterDepth)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelSurfacewaterDepth", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelSurfacewaterDepth in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error && NULL != channelSurfacewaterError)
    {
      dimIDs[0]   = instancesDimID;
      dimIDs[1]   = channelElementsDimID;
      ncErrorCode = nc_def_var(fileID, "channelSurfacewaterError", NC_DOUBLE, 2, dimIDs, &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to create variable channelSurfacewaterError in NetCDF state file %s.  "
                  "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::createFiles: unable to close NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
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

void FileManager::resizeUnlimitedDimensions(size_t directorySize, const char* directory, bool writeGeometry, bool writeParameter, bool writeState)
{
  bool               error      = false;       // Error flag.
  char*              nameString = NULL;        // Temporary string for file names.
  size_t             nameStringSize;           // Size of buffer allocated for nameString.
  size_t             numPrinted;               // Used to check that snprintf printed the correct number of characters.
  int                ncErrorCode;              // Return value of NetCDF functions.
  int                fileID;                   // ID of NetCDF file.
  bool               fileOpen   = false;       // Whether fileID refers to an open file.
  int                dimID;                    // ID of dimension in NetCDF file.
  int                varID;                    // ID of variable in NetCDF file.
  size_t             geometryInstance;         // Instance index for geometry file.
  size_t             parameterInstance;        // Instance index for parameter file.
  size_t             stateInstance;            // Instance index for state file.
  size_t             start[NC_MAX_VAR_DIMS];   // For specifying subarrays when writing to NetCDF file.
  int                intZero = 0;              // For writing dummy value to resize dimension.
  double             doubleZero = 0.0;         // For writing dummy value to resize dimension.
  signed char        signedCharZero = 0;       // For writing dummy value to resize dimension.
  unsigned short     unsignedShortZero = 0;    // For writing dummy value to resize dimension.
  unsigned long long unsignedLongLongZero = 0; // For writing dummy value to resize dimension.

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
  // This function should only be called if using the NetCDF collective I/O workaround.
  CkAssert(false);
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

  if (0 == CkMyPe())
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      if (!(NULL != directory))
        {
          CkError("ERROR in FileManager::resizeUnlimitedDimensions: directory must not be null.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

      // Allocate space for file name strings.
      if (!error)
        {
          nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // The longest file name is parameter.nc.  +1 for null terminating character.
          nameString     = new char[nameStringSize];
        }

      if (writeGeometry)
        {
          // Create file name.
          if (!error)
            {
              numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: incorrect return value of snprintf when generating geometry file name %s.  "
                          "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Open file.
          if (!error)
            {
              ncErrorCode = nc_open(nameString, NC_NETCDF4 | NC_WRITE, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to open NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
              else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                {
                  fileOpen = true;
                }
            }

          // Get the number of existing instances.
          if (!error)
            {
              ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get dimension instances in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_dimlen(fileID, dimID, &geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get length of dimension instances in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Write variables.
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfMeshNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable numberOfMeshNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = geometryInstance;
              ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable numberOfMeshNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfMeshElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable numberOfMeshElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = geometryInstance;
              ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable numberOfMeshElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfChannelNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable numberOfChannelNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = geometryInstance;
              ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable numberOfChannelNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfChannelElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable numberOfChannelElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = geometryInstance;
              ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable numberOfChannelElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (NULL != meshNodeX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshNodeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshNodeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshNodeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshNodeY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshNodeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshNodeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshNodeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshNodeZSurface)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshNodeZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshNodeZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshNodeZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshNodeZBedrock)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshNodeZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshNodeZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshNodeZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementVertices)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementVertices", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementVertices in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementVertices in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshVertexX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshVertexX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshVertexX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshVertexY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshVertexY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshVertexY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshVertexY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshVertexZSurface)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshVertexZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshVertexZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshVertexZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshVertexZBedrock)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshVertexZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshVertexZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshVertexZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementZSurface)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementZSurface in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementZBedrock)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementZBedrock in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementArea)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementArea", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementArea in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementArea in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementSlopeX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementSlopeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementSlopeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshElementSlopeY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshElementSlopeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshElementSlopeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighbors)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighborsChannelEdge)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsChannelEdge", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighborsChannelEdge in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_schar(fileID, varID, start, &signedCharZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighborsChannelEdge in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighborsEdgeLength)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighborsEdgeLength in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighborsEdgeLength in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighborsEdgeNormalX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshMeshNeighborsEdgeNormalY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshChannelNeighbors)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshChannelNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::channelNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshChannelNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshChannelNeighborsEdgeLength)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshChannelNeighborsEdgeLength in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  start[2]    = MeshElement::channelNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshChannelNeighborsEdgeLength in NetCDF geometry "
                              "file %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelNodeX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelNodeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelNodeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelNodeX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelNodeY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelNodeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelNodeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelNodeY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelNodeZBank)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelNodeZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelNodeZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelNodeZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelNodeZBed)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelNodeZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelNodeZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelNodes - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelNodeZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementVertices)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementVertices", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementVertices in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize + 2 - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementVertices in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelVertexX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelVertexX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelVertexX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelVertexX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelVertexY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelVertexY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelVertexY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelVertexY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelVertexZBank)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelVertexZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelVertexZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelVertexZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelVertexZBed)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelVertexZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelVertexZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelVerticesSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelVertexZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementX)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementX in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementY)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementY in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementZBank)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementZBank in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementZBed)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementZBed in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelElementLength)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelElementLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelElementLength in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelElementLength in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelChannelNeighbors)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelChannelNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelChannelNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::channelNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelChannelNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelMeshNeighbors)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelMeshNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelMeshNeighbors in NetCDF geometry file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelMeshNeighborsEdgeLength)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelMeshNeighborsEdgeLength in NetCDF geometry file "
                              "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = geometryInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  start[2]    = ChannelElement::meshNeighborsSize - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelMeshNeighborsEdgeLength in NetCDF geometry "
                              "file %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          // Close file.
          if (fileOpen)
            {
              ncErrorCode = nc_close(fileID);
              fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to close NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        } // End if (writeGeometry).

      if (writeParameter)
        {
          // Create file name.
          if (!error)
            {
              numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: incorrect return value of snprintf when generating parameter file name %s.  "
                          "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Open file.
          if (!error)
            {
              ncErrorCode = nc_open(nameString, NC_NETCDF4 | NC_WRITE, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to open NetCDF parameter file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
              else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                {
                  fileOpen = true;
                }
            }

          // Get the number of existing instances.
          if (!error)
            {
              ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get dimension instances in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_dimlen(fileID, dimID, &parameterInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get length of dimension instances in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Write variables.
          if (NULL != meshCatchment)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshCatchment", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshCatchment in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshCatchment in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshConductivity)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshConductivity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshConductivity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshConductivity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshPorosity)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshPorosity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshPorosity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshPorosity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshManningsN)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshManningsN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshManningsN in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshManningsN in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelChannelType)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelChannelType", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelChannelType in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelChannelType in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelPermanentCode)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelPermanentCode", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelPermenentCode in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_int(fileID, varID, start, &intZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelPermanentCode in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelBaseWidth)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelBaseWidth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelBaseWidth in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelBaseWidth in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelSideSlope)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelSideSlope", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelSideSlope in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelSideSlope in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelBedConductivity)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelBedConductivity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelBedConductivity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelBedConductivity in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelBedThickness)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelBedThickness", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelBedThickness in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelBedThickness in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelManningsN)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelManningsN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelManningsN in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = parameterInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelManningsN in NetCDF parameter file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          // Close file.
          if (fileOpen)
            {
              ncErrorCode = nc_close(fileID);
              fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to close NetCDF parameter file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        } // End if (writeParameter).

      if (writeState)
        {
          // Create file name.
          if (!error)
            {
              numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: incorrect return value of snprintf when generating state file name %s.  "
                          "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Open file.
          if (!error)
            {
              ncErrorCode = nc_open(nameString, NC_NETCDF4 | NC_WRITE, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to open NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
              else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                {
                  fileOpen = true;
                }
            }

          // Get the number of existing instances.
          if (!error)
            {
              ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get dimension instances in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_dimlen(fileID, dimID, &stateInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get length of dimension instances in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          // Write variables.
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "iteration", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable iteration in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_ushort(fileID, varID, start, &unsignedShortZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable iteration in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "currentTime", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable currentTime in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable currentTime in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "dt", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "geometryInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable geometryInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_ulonglong(fileID, varID, start, &unsignedLongLongZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable geometryInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "parameterInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable parameterInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              start[0]    = stateInstance;
              ncErrorCode = nc_put_var1_ulonglong(fileID, varID, start, &unsignedLongLongZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable parameterInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (NULL != meshSurfacewaterDepth)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshSurfacewaterDepth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshSurfacewaterDepth in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshSurfacewaterDepth in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshSurfacewaterError)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshSurfacewaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshSurfacewaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshSurfacewaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshGroundwaterHead)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshGroundwaterHead", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshGroundwaterHead in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshGroundwaterHead in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != meshGroundwaterError)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "meshGroundwaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable meshGroundwaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfMeshElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable meshGroundwaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelSurfacewaterDepth)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelSurfacewaterDepth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelSurfacewaterDepth in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelSurfacewaterDepth in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          if (NULL != channelSurfacewaterError)
            {
              if (!error)
                {
                  ncErrorCode = nc_inq_varid(fileID, "channelSurfacewaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to get variable channelSurfacewaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }

              if (!error)
                {
                  start[0]    = stateInstance;
                  start[1]    = globalNumberOfChannelElements - 1;
                  ncErrorCode = nc_put_var1_double(fileID, varID, start, &doubleZero);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to write variable channelSurfacewaterError in NetCDF state file %s.  "
                              "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                }
            }

          // Close file.
          if (fileOpen)
            {
              ncErrorCode = nc_close(fileID);
              fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::resizeUnlimitedDimensions: unable to close NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        } // End if (writeState).

      // Delete nameString.
      if (NULL != nameString)
        {
          delete[] nameString;
        }

      if (!error)
        {
          thisProxy.writeFiles(directorySize, directory, writeGeometry, writeParameter, writeState);
        }
      else
        {
          CkExit();
        }
    } // End if (0 == CkMyPe()).
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      // This function should only be called on file manager zero.
      CkAssert(false);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
}

void FileManager::writeFiles(size_t directorySize, const char* directory, bool writeGeometry, bool writeParameter, bool writeState)
{
  bool   error      = false;     // Error flag.
  char*  nameString = NULL;      // Temporary string for file names.
  size_t nameStringSize;         // Size of buffer allocated for nameString.
  size_t numPrinted;             // Used to check that snprintf printed the correct number of characters.
  int    ncErrorCode;            // Return value of NetCDF functions.
  int    fileID;                 // ID of NetCDF file.
  bool   fileOpen   = false;     // Whether fileID refers to an open file.
  int    dimID;                  // ID of dimension in NetCDF file.
  int    varID;                  // ID of variable in NetCDF file.
  size_t geometryInstance;       // Instance index for geometry file.
  size_t parameterInstance;      // Instance index for parameter file.
  size_t stateInstance;          // Instance index for state file.
  size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when writing to NetCDF file.
  size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when writing to NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != directory))
    {
      CkError("ERROR in FileManager::writeFiles: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file name strings.
      nameStringSize = strlen(directory) + strlen("/parameter.nc") + 1; // The longest file name is parameter.nc.  +1 for null terminating character.
      nameString     = new char[nameStringSize];

      // Create file name.
      numPrinted = snprintf(nameString, nameStringSize, "%s/geometry.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/geometry.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeFiles: incorrect return value of snprintf when generating geometry file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/geometry.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to open NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get dimension instances in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(fileID, dimID, &geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get length of dimension instances in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (writeGeometry)
    {
#ifdef NETCDF_COLLECTIVE_IO_WORKAROUND
      // The new instance was already created in resizeUnlimitedDimensions so use the last existing instance, not a new one.
      geometryInstance--;
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
      
      if (0 == CkMyPe())
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfMeshNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable numberOfMeshNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable numberOfMeshNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = geometryInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, &globalNumberOfMeshNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable numberOfMeshNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfMeshElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable numberOfMeshElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable numberOfMeshElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = geometryInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, &globalNumberOfMeshElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable numberOfMeshElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfChannelNodes", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable numberOfChannelNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable numberOfChannelNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = geometryInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, &globalNumberOfChannelNodes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable numberOfChannelNodes in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "numberOfChannelElements", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable numberOfChannelElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable numberOfChannelElements in NetCDF geometry file "
                          "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = geometryInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, &globalNumberOfChannelElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable numberOfChannelElements in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        } // End if (0 == CkMyPe()).
      
      if (NULL != meshNodeX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshNodeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshNodeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshNodeY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshNodeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshNodeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshNodeZSurface)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshNodeZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshNodeZSurface in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshNodeZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshNodeZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshNodeZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshNodeZBedrock)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshNodeZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshNodeZBedrock in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshNodeZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshNodeZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshNodeZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementVertices)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementVertices", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)meshElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshVertexX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshVertexX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshVertexY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshVertexY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshVertexZSurface)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshVertexZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshVertexZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshVertexZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshVertexZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshVertexZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshVertexZBedrock)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshVertexZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshVertexZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshVertexZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshVertexZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshVertexZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementZSurface)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementZSurface", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementZSurface);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementZSurface in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementZBedrock)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementZBedrock", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementZBedrock);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementZBedrock in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementArea)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementArea", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementArea in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementArea in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementArea);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementArea in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementSlopeX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementSlopeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementSlopeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementSlopeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementSlopeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshElementSlopeY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshElementSlopeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshElementSlopeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshElementSlopeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshElementSlopeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshElementSlopeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighbors)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)meshMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighborsChannelEdge)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsChannelEdge", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighborsChannelEdge in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighborsChannelEdge in NetCDF geometry file"
                          " %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              // Assumes bool is one byte when casting to signed char.
              CkAssert(sizeof(bool) == sizeof(signed char));
              
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_schar(fileID, varID, start, count, (signed char*)meshMeshNeighborsChannelEdge);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighborsChannelEdge in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighborsEdgeLength)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighborsEdgeLength in NetCDF geometry file "
                          "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighborsEdgeNormalX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file"
                          " %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeNormalX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighborsEdgeNormalX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshMeshNeighborsEdgeNormalY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshMeshNeighborsEdgeNormalY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file"
                          " %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshMeshNeighborsEdgeNormalY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshMeshNeighborsEdgeNormalY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshChannelNeighbors)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::channelNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)meshChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshChannelNeighborsEdgeLength)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshChannelNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshChannelNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshChannelNeighborsEdgeLength in NetCDF geometry "
                          "file %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localMeshElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              count[2]    = MeshElement::channelNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)meshChannelNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshChannelNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelNodeX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelNodeX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelNodeX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelNodeX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelNodeX in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelNodeY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelNodeY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelNodeY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelNodeY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelNodeY in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelNodeZBank)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelNodeZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelNodeZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelNodeZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelNodeZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelNodeZBank in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelNodeZBed)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelNodeZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelNodeZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelNodeZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelNodeStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelNodes;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelNodeZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelNodeZBed in NetCDF geometry file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementVertices)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementVertices", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize + 2;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)channelElementVertices);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementVertices in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelVertexX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelVertexX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelVertexX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelVertexY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelVertexY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelVertexY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelVertexZBank)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelVertexZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelVertexZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelVertexZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelVertexZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelVertexZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelVertexZBed)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelVertexZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelVertexZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelVertexZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelVerticesSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelVertexZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelVertexZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementX)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementX", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementX in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementY)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementY", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementY in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementZBank)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementZBank", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementZBank);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementZBank in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementZBed)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementZBed", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementZBed);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementZBed in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelElementLength)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelElementLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelElementLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelElementLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelElementLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelElementLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelChannelNeighbors)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelChannelNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::channelNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)channelChannelNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelChannelNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelMeshNeighbors)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighbors", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)channelMeshNeighbors);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelMeshNeighbors in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelMeshNeighborsEdgeLength)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelMeshNeighborsEdgeLength", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelMeshNeighborsEdgeLength in NetCDF geometry "
                          "file %s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = geometryInstance;
              start[1]    = localChannelElementStart;
              start[2]    = 0;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              count[2]    = ChannelElement::meshNeighborsSize;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, (double*)channelMeshNeighborsEdgeLength);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelMeshNeighborsEdgeLength in NetCDF geometry file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
    }
  else // if (!writeGeometry)
    {
      if (!error)
        {
          if (0 < geometryInstance)
            {
              // We're not creating a new instance so use the last instance with index one less than the dimension length.
              geometryInstance--;
              
              // FIXME check if the number of nodes and elements is consisitent?
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else
            {
              // We're not creating a new instance so it's an error if there's not an existing one.
              CkError("ERROR in FileManager::writeFiles: not creating a new instance and no existing instance in NetCDF geometry file %s.\n", nameString);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to close NetCDF geometry file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/parameter.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/parameter.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeFiles: incorrect return value of snprintf when generating parameter file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/parameter.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to open NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get dimension instances in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(fileID, dimID, &parameterInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get length of dimension instances in NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (writeParameter)
    {
#ifdef NETCDF_COLLECTIVE_IO_WORKAROUND
      // The new instance was already created in resizeUnlimitedDimensions so use the last existing instance, not a new one.
      parameterInstance--;
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
      
      if (NULL != meshCatchment)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshCatchment", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshCatchment in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshCatchment in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, meshCatchment);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshCatchment in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshConductivity)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshConductivity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshPorosity)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshPorosity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshPorosity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshPorosity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshPorosity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshPorosity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshManningsN)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshManningsN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelChannelType)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelChannelType", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelChannelType in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelChannelType in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              // Assumes ChannelTypeEnum is four bytes when casting to int.
              CkAssert(sizeof(ChannelTypeEnum) == sizeof(int));
              
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, (int*)channelChannelType);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelChannelType in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelPermanentCode)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelPermanentCode", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelPermenentCode in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelPermenentCode in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_int(fileID, varID, start, count, channelPermanentCode);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelPermanentCode in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelBaseWidth)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelBaseWidth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelBaseWidth in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelBaseWidth in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelBaseWidth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelBaseWidth in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelSideSlope)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelSideSlope", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelSideSlope in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelSideSlope in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelSideSlope);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelSideSlope in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelBedConductivity)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelBedConductivity", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelBedConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelBedConductivity in NetCDF parameter file "
                          "%s.  NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelBedConductivity);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelBedConductivity in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelBedThickness)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelBedThickness", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelBedThickness in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelBedThickness in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelBedThickness);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelBedThickness in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelManningsN)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelManningsN", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = parameterInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelManningsN);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelManningsN in NetCDF parameter file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
    }
  else // if (!writeParameter)
    {
      if (!error)
        {
          if (0 < parameterInstance)
            {
              // We're not creating a new instance so use the last instance with index one less than the dimension length.
              parameterInstance--;
              
              // FIXME check if the number of nodes and elements is consisitent?
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          else
            {
              // We're not creating a new instance so it's an error if there's not an existing one.
              CkError("ERROR in FileManager::writeFiles: not creating a new instance and no existing instance in NetCDF parameter file %s.\n", nameString);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
    }
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to close NetCDF parameter file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s/state.nc", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/state.nc") == numPrinted && numPrinted < nameStringSize))
        {
          CkError("ERROR in FileManager::writeFiles: incorrect return value of snprintf when generating state file name %s.  "
                  "%d should be equal to %d and less than %d.\n", nameString, numPrinted, strlen(directory) + strlen("/state.nc"), nameStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      ncErrorCode = nc_open_par(nameString, NC_NETCDF4 | NC_MPIIO | NC_WRITE, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to open NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
      else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          fileOpen = true;
        }
    }
  
  // Get the number of existing instances.
  if (!error)
    {
      ncErrorCode = nc_inq_dimid(fileID, "instances", &dimID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get dimension instances in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(fileID, dimID, &stateInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to get length of dimension instances in NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Write variables.
  if (writeState)
    {
#ifdef NETCDF_COLLECTIVE_IO_WORKAROUND
      // The new instance was already created in resizeUnlimitedDimensions so use the last existing instance, not a new one.
      stateInstance--;
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
      
      if (0 == CkMyPe())
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "iteration", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable iteration in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable iteration in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = stateInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_ushort(fileID, varID, start, count, &iteration);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable iteration in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "currentTime", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable currentTime in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable currentTime in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = stateInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, &currentTime);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable currentTime in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "dt", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable dt in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              start[0]    = stateInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, &dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable dt in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "geometryInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable geometryInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable geometryInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              // Assumes size_t is eight bytes when casting to unsigned long long.
              CkAssert(sizeof(size_t) == sizeof(unsigned long long));
              
              start[0]    = stateInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_ulonglong(fileID, varID, start, count, (unsigned long long *)&geometryInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable geometryInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "parameterInstance", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable parameterInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable parameterInstance in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND

          if (!error)
            {
              // Assumes size_t is eight bytes when casting to unsigned long long.
              CkAssert(sizeof(size_t) == sizeof(unsigned long long));
              
              start[0]    = stateInstance;
              count[0]    = 1;
              ncErrorCode = nc_put_vara_ulonglong(fileID, varID, start, count, (unsigned long long *)&parameterInstance);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable parameterInstance in NetCDF state file %s.  NetCDF error message: %s.\n",
                          nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        } // End if (0 == CkMyPe()).
      
      if (NULL != meshSurfacewaterDepth)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshSurfacewaterDepth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshSurfacewaterError)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshSurfacewaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshGroundwaterHead)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshGroundwaterHead", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshGroundwaterHead in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshGroundwaterHead in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshGroundwaterHead);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshGroundwaterHead in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != meshGroundwaterError)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "meshGroundwaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable meshGroundwaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable meshGroundwaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localMeshElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfMeshElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, meshGroundwaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable meshGroundwaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelSurfacewaterDepth)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelSurfacewaterDepth", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelSurfacewaterDepth);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelSurfacewaterDepth in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      if (NULL != channelSurfacewaterError)
        {
          if (!error)
            {
              ncErrorCode = nc_inq_varid(fileID, "channelSurfacewaterError", &varID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to get variable channelSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }

#ifndef NETCDF_COLLECTIVE_IO_WORKAROUND
          if (!error)
            {
              ncErrorCode = nc_var_par_access(fileID, varID, NC_COLLECTIVE);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to set collective access for variable channelSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
#endif // NETCDF_COLLECTIVE_IO_WORKAROUND
          
          if (!error)
            {
              start[0]    = stateInstance;
              start[1]    = localChannelElementStart;
              count[0]    = 1;
              count[1]    = localNumberOfChannelElements;
              ncErrorCode = nc_put_vara_double(fileID, varID, start, count, channelSurfacewaterError);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in FileManager::writeFiles: unable to write variable channelSurfacewaterError in NetCDF state file %s.  "
                          "NetCDF error message: %s.\n", nameString, nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
    } // End if (writeState).
  
  // Close file.
  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);
      fileOpen    = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          CkError("ERROR in FileManager::writeFiles: unable to close NetCDF state file %s.  NetCDF error message: %s.\n",
                  nameString, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Delete nameString.
  if (NULL != nameString)
    {
      delete[] nameString;
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

void FileManager::handleMeshVertexDataMessage(int element, int vertex, double x, double y, double zSurface, double zBedrock)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(localMeshElementStart <= element && element < localMeshElementStart + localNumberOfMeshElements))
    {
      CkError("ERROR in FileManager::meshVertexDataMessage: element data not owned by this local branch.\n");
      CkExit();
    }

  if (!(0 <= vertex && vertex < MeshElement::meshNeighborsSize))
    {
      CkError("ERROR in FileManager::meshVertexDataMessage: vertex must be greater than or equal to zero and less than "
          "MeshElement::meshNeighborsSize.\n");
      CkExit();
    }

  if (!(zSurface >= zBedrock))
    {
      CkError("ERROR in FileManager::meshVertexDataMessage: zSurface must be greater than or equal to zBedrock.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (NULL != meshNodeX)
    {
      meshVertexX[element - localMeshElementStart][vertex] = x;
    }

  if (NULL != meshNodeY)
    {
      meshVertexY[element - localMeshElementStart][vertex] = y;
    }

  if (NULL != meshNodeZSurface)
    {
      meshVertexZSurface[element - localMeshElementStart][vertex] = zSurface;
    }

  if (NULL != meshNodeZBedrock)
    {
      meshVertexZBedrock[element - localMeshElementStart][vertex] = zBedrock;
    }

  meshVertexUpdated[element - localMeshElementStart][vertex] = true;
}

void FileManager::finishCalculateDerivedValues()
{
  int    ii, jj; // Loop counters.
  double value;  // For calculating derived values.

  if (NULL == meshElementX && NULL != meshVertexX)
    {
      meshElementX = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexX[ii][jj];
            }

          meshElementX[ii] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementY && NULL != meshVertexY)
    {
      meshElementY = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexY[ii][jj];
            }

          meshElementY[ii] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementZSurface && NULL != meshVertexZSurface)
    {
      meshElementZSurface = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexZSurface[ii][jj];
            }

          meshElementZSurface[ii] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementZBedrock && NULL != meshVertexZBedrock)
    {
      meshElementZBedrock = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexZBedrock[ii][jj];
            }

          meshElementZBedrock[ii] = value / MeshElement::meshNeighborsSize;
        }
    }

  if (NULL == meshElementArea && NULL != meshVertexX && NULL != meshVertexY)
    {
      meshElementArea = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              value += meshVertexX[ii][jj] * (meshVertexY[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                              meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize]);
            }

          meshElementArea[ii] = value * 0.5;
        }
    }

  if (NULL == meshElementSlopeX && NULL != meshVertexX && NULL != meshVertexY && NULL != meshVertexZSurface)
    {
      meshElementSlopeX = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize - 1; jj++)
            {
              value += (meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize] - meshVertexY[ii][jj]) *
                       (meshVertexZSurface[ii][(jj + 1) % MeshElement::meshNeighborsSize] - meshVertexZSurface[ii][0]);
            }

          meshElementSlopeX[ii] = value / (2.0 * meshElementArea[ii]);
        }
    }

  if (NULL == meshElementSlopeY && NULL != meshVertexX && NULL != meshVertexY && NULL != meshVertexZSurface)
    {
      meshElementSlopeY = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          // This works for triangles.  Don't know about other shapes.
          value = 0.0;

          for (jj = 0; jj < MeshElement::meshNeighborsSize - 1; jj++)
            {
              value += (meshVertexX[ii][jj] - meshVertexX[ii][(jj + 2) % MeshElement::meshNeighborsSize]) *
                       (meshVertexZSurface[ii][(jj + 1) % MeshElement::meshNeighborsSize] - meshVertexZSurface[ii][0]);
            }

          meshElementSlopeY[ii] = value / (2.0 * meshElementArea[ii]);
        }
    }

  if (NULL == meshMeshNeighborsEdgeLength && NULL != meshVertexX && NULL != meshVertexY)
    {
      meshMeshNeighborsEdgeLength = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeLength[ii][jj] = sqrt((meshVertexX[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                          meshVertexX[ii][(jj + 2) % MeshElement::meshNeighborsSize]) *
                                                         (meshVertexX[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                          meshVertexX[ii][(jj + 2) % MeshElement::meshNeighborsSize]) +
                                                         (meshVertexY[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                          meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize]) *
                                                         (meshVertexY[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                          meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize]));
            }
        }
    }

  if (NULL == meshMeshNeighborsEdgeNormalX && NULL != meshVertexY && NULL != meshMeshNeighborsEdgeLength)
    {
      meshMeshNeighborsEdgeNormalX = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeNormalX[ii][jj] = (meshVertexY[ii][(jj + 2) % MeshElement::meshNeighborsSize] -
                                                      meshVertexY[ii][(jj + 1) % MeshElement::meshNeighborsSize]) / meshMeshNeighborsEdgeLength[ii][jj];
            }
        }
    }

  if (NULL == meshMeshNeighborsEdgeNormalY && NULL != meshVertexX && NULL != meshMeshNeighborsEdgeLength)
    {
      meshMeshNeighborsEdgeNormalY = new double[localNumberOfMeshElements][MeshElement::meshNeighborsSize];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          for (jj = 0; jj < MeshElement::meshNeighborsSize; jj++)
            {
              // This works for triangles.  Don't know about other shapes.
              meshMeshNeighborsEdgeNormalY[ii][jj] = (meshVertexX[ii][(jj + 1) % MeshElement::meshNeighborsSize] -
                                                      meshVertexX[ii][(jj + 2) % MeshElement::meshNeighborsSize]) / meshMeshNeighborsEdgeLength[ii][jj];
            }
        }
    }

  // FIXME remove, part of the hardcoded mesh
  if (NULL == meshGroundwaterHead && NULL != meshElementZSurface)
    {
      meshGroundwaterHead = new double[localNumberOfMeshElements];

      for (ii = 0; ii < localNumberOfMeshElements; ii++)
        {
          meshGroundwaterHead[ii] = meshElementZSurface[ii];
        }
    }
  
  contribute();
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "file_manager.def.h"
#pragma GCC diagnostic warning "-Wsign-compare"
#pragma GCC diagnostic warning "-Wunused-variable"
