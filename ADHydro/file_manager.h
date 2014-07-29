#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "file_manager.decl.h"
#pragma GCC diagnostic warning "-Wsign-compare"

#include "mesh_element.h"
#include "channel_element.h"

typedef int    intarraymmn[MeshElement::meshNeighborsSize];       // Fixed size array of ints.     Size is mesh    mesh neighbors.
typedef bool   boolarraymmn[MeshElement::meshNeighborsSize];      // Fixed size array of bools.    Size is mesh    mesh neighbors.
typedef double doublearraymmn[MeshElement::meshNeighborsSize];    // Fixed size array of doubles.  Size is mesh    mesh neighbors.
typedef int    intarraymcn[MeshElement::channelNeighborsSize];    // Fixed size array of ints.     Size is mesh    channel neighbors.
typedef double doublearraymcn[MeshElement::channelNeighborsSize]; // Fixed size array of doubles.  Size is mesh    channel neighbors.
typedef int    intarrayccn[ChannelElement::channelNeighborsSize]; // Fixed size array of ints.     Size is channel channel neighbors.
typedef int    intarraycmn[ChannelElement::meshNeighborsSize];    // Fixed size array of ints.     Size is channel mesh neighbors.
typedef double doublearraycmn[ChannelElement::meshNeighborsSize]; // Fixed size array of doubles.  Size is channel mesh neighbors.

// FIXME comment
class FileManager : public CBase_FileManager
{
public:

  // Constructor.
  FileManager();

  int globalNumberOfMeshElements;    // FIXME comment
  int localMeshElementStart;         // FIXME comment
  int localNumberOfMeshElements;     // FIXME comment
  int globalNumberOfChannelElements; // FIXME comment
  int localChannelElementStart;      // FIXME comment
  int localNumberOfChannelElements;  // FIXME comment
  
  doublearraymmn*  meshVertexX;
  doublearraymmn*  meshVertexY;
  doublearraymmn*  meshVertexZSurface;
  doublearraymmn*  meshVertexZBedrock;
  double*          meshElementX;
  double*          meshElementY;
  double*          meshElementZSurface;
  double*          meshElementZBedrock;
  double*          meshElementArea;
  double*          meshElementSlopeX;
  double*          meshElementSlopeY;
  int*             meshCatchment;
  double*          meshConductivity;
  double*          meshPorosity;
  double*          meshManningsN;
  double*          meshSurfacewaterDepth;
  double*          meshSurfacewaterError;
  double*          meshGroundwaterHead;
  double*          meshGroundwaterError;
  intarraymmn*     meshMeshNeighbors;
  boolarraymmn*    meshMeshNeighborsChannelEdge;
  doublearraymmn*  meshMeshNeighborsEdgeLength;
  doublearraymmn*  meshMeshNeighborsEdgeNormalX;
  doublearraymmn*  meshMeshNeighborsEdgeNormalY;
  intarraymcn*     meshChannelNeighbors;
  doublearraymcn*  meshChannelNeighborsEdgeLength;
  double*          channelElementX;
  double*          channelElementY;
  double*          channelElementZBank;
  double*          channelElementZBed;
  double*          channelElementLength;
  ChannelTypeEnum* channelChannelType;
  int*             channelPermanentCode;
  double*          channelBaseWidth;
  double*          channelSideSlope;
  double*          channelBedConductivity;
  double*          channelBedThickness;
  double*          channelManningsN;
  double*          channelSurfacewaterDepth;
  double*          channelSurfacewaterError;
  intarrayccn*     channelChannelNeighbors;
  intarraycmn*     channelMeshNeighbors;
  doublearraycmn*  channelMeshNeighborsEdgeLength;
};

#endif // __FILE_MANAGER_H__
