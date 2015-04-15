#ifndef __CHANNEL_ELEMENT_H__
#define __CHANNEL_ELEMENT_H__

#include "neighbor_proxy.h"

// Proxy for a channel element's surfacewater neighbor that is a mesh element.
class ChannelSurfacewaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // FIXME comment and implement
  void sendWater(MaterialTransfer water);
  
  int region;
  int neighbor;
  int reciprocalNeighborProxy;
};

// Proxy for a channel element's surfacewater neighbor that is a channel element.
class ChannelSurfacewaterChannelNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // FIXME comment and implement
  void sendWater(MaterialTransfer water);
  
  int region;
  int neighbor;
  int reciprocalNeighborProxy;
};

// Proxy for a channel element's groundwater neighbor that is a mesh element.
class ChannelGroundwaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // FIXME comment and implement
  void sendWater(MaterialTransfer water);
  
  int region;
  int neighbor;
  int reciprocalNeighborProxy;
};

class ChannelElement
{
public:
  
  // Identification parameters.
  int elementNumber;  // Channel element ID number of this element.
  int catchment;      // Catchment ID number that this element belongs to.
  
  // Water state variables.
  double surfacewaterDepth; // Meters.
  double surfacewaterError; // Meters.  Positive means water was created.  Negative means water was destroyed.
  
  // Neighbors.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>    meshNeighbors;
  std::vector<ChannelSurfacewaterChannelNeighborProxy> channelNeighbors;
  std::vector<ChannelGroundwaterMeshNeighborProxy>     undergroundMeshNeighbors;
};

#endif // __CHANNEL_ELEMENT_H__
