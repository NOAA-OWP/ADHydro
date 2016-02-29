#include "region.h"
#include "adhydro.h"
#include "file_manager.h"
#include "garto.h"

RegionMessage::RegionMessage() :
  messageType(MESH_SURFACEWATER_MESH_NEIGHBOR), // Dummy values will be overwritten by pup_stl.h code.
  recipientElementNumber(0),
  recipientNeighborProxyIndex(0),
  senderSurfacewaterDepth(0.0),
  senderGroundwaterHead(0.0),
  water()
{
  // Initialization handled by initialization list.
}

RegionMessage::RegionMessage(RegionMessageTypeEnum messageTypeInit, int recipientElementNumberInit, int recipientNeighborProxyIndexInit,
                             double senderSurfacewaterDepthInit, double senderGroundwaterHeadInit, SimpleNeighborProxy::MaterialTransfer waterInit) :
  messageType(messageTypeInit),
  recipientElementNumber(recipientElementNumberInit),
  recipientNeighborProxyIndex(recipientNeighborProxyIndexInit),
  senderSurfacewaterDepth(senderSurfacewaterDepthInit),
  senderGroundwaterHead(senderGroundwaterHeadInit),
  water(waterInit)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(MESH_SURFACEWATER_MESH_NEIGHBOR    == messageTypeInit || MESH_SURFACEWATER_CHANNEL_NEIGHBOR    == messageTypeInit ||
        MESH_GROUNDWATER_MESH_NEIGHBOR     == messageTypeInit || MESH_GROUNDWATER_CHANNEL_NEIGHBOR     == messageTypeInit ||
        CHANNEL_SURFACEWATER_MESH_NEIGHBOR == messageTypeInit || CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR == messageTypeInit ||
        CHANNEL_GROUNDWATER_MESH_NEIGHBOR  == messageTypeInit))
    {
      CkError("ERROR in RegionMessage::RegionMessage: messageTypeInit must be a valid enum value.\n");
      CkExit();
    }
  
  if (MESH_SURFACEWATER_MESH_NEIGHBOR == messageTypeInit || MESH_SURFACEWATER_CHANNEL_NEIGHBOR == messageTypeInit ||
      MESH_GROUNDWATER_MESH_NEIGHBOR  == messageTypeInit || MESH_GROUNDWATER_CHANNEL_NEIGHBOR  == messageTypeInit)
    {
      if (!(0 <= recipientElementNumberInit && recipientElementNumberInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements))
        {
          CkError("ERROR in RegionMessage::RegionMessage: recipientElementNumberInit must be greater than or equal to zero and less than "
                  "globalNumberOfMeshElements.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0 <= recipientElementNumberInit && recipientElementNumberInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))
        {
          CkError("ERROR in RegionMessage::RegionMessage: recipientElementNumberInit must be greater than or equal to zero and less than "
                  "globalNumberOfChannelElements.\n");
          CkExit();
        }
    }
  
  if (!(0 <= recipientNeighborProxyIndexInit))
    {
      CkError("ERROR in RegionMessage::RegionMessage: recipientNeighborProxyIndexInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= senderSurfacewaterDepthInit))
    {
      CkError("ERROR in RegionMessage::RegionMessage: senderSurfacewaterDepthInit must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  waterInit.checkInvariant();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
}

void RegionMessage::pup(PUP::er &p)
{
  p | messageType;
  p | recipientElementNumber;
  p | recipientNeighborProxyIndex;
  p | senderSurfacewaterDepth;
  p | senderGroundwaterHead;
  p | water;
}

bool RegionMessage::checkInvariant()
{
  bool error = false; // Error flag.
  
  if (!(MESH_SURFACEWATER_MESH_NEIGHBOR    == messageType || MESH_SURFACEWATER_CHANNEL_NEIGHBOR    == messageType ||
        MESH_GROUNDWATER_MESH_NEIGHBOR     == messageType || MESH_GROUNDWATER_CHANNEL_NEIGHBOR     == messageType ||
        CHANNEL_SURFACEWATER_MESH_NEIGHBOR == messageType || CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR == messageType ||
        CHANNEL_GROUNDWATER_MESH_NEIGHBOR  == messageType))
    {
      CkError("ERROR in RegionMessage::checkInvariant: messageType must be a valid enum value.\n");
      error = true;
    }
  
  if (MESH_SURFACEWATER_MESH_NEIGHBOR == messageType || MESH_SURFACEWATER_CHANNEL_NEIGHBOR == messageType ||
      MESH_GROUNDWATER_MESH_NEIGHBOR  == messageType || MESH_GROUNDWATER_CHANNEL_NEIGHBOR  == messageType)
    {
      if (!(0 <= recipientElementNumber && recipientElementNumber < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements))
        {
          CkError("ERROR in RegionMessage::checkInvariant: recipientElementNumber must be greater than or equal to zero and less than "
                  "globalNumberOfMeshElements.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0 <= recipientElementNumber && recipientElementNumber < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))
        {
          CkError("ERROR in RegionMessage::checkInvariant: recipientElementNumber must be greater than or equal to zero and less than "
                  "globalNumberOfChannelElements.\n");
          CkExit();
        }
    }
  
  if (!(0 <= recipientNeighborProxyIndex))
    {
      CkError("ERROR in RegionMessage::checkInvariant: recipientNeighborProxyIndex must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= senderSurfacewaterDepth))
    {
      CkError("ERROR in RegionMessage::checkInvariant: senderSurfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  error = water.checkInvariant() || error;
  
  return error;
}

simpleNeighborInfo::simpleNeighborInfo() :
  expirationTime(0.0), // Dummy values will be overwritten by pup_stl.h code.
  nominalFlowRate(0.0),
  flowCumulativeShortTerm(0.0),
  flowCumulativeLongTerm(0.0),
  region(0),
  neighbor(0),
  edgeLength(0.0),
  edgeNormalX(0.0),
  edgeNormalY(0.0)
{
  // Initialization handled by initialization list.
}

simpleNeighborInfo::simpleNeighborInfo(double expirationTimeInit, double nominalFlowRateInit, double flowCumulativeShortTermInit,
                                       double flowCumulativeLongTermInit, int regionInit, int neighborInit, double edgeLengthInit, double edgeNormalXInit,
                                       double edgeNormalYInit) :
  expirationTime(expirationTimeInit),
  nominalFlowRate(nominalFlowRateInit),
  flowCumulativeShortTerm(flowCumulativeShortTermInit),
  flowCumulativeLongTerm(flowCumulativeLongTermInit),
  region(regionInit),
  neighbor(neighborInit),
  edgeLength(edgeLengthInit),
  edgeNormalX(edgeNormalXInit),
  edgeNormalY(edgeNormalYInit)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    if (!(0 <= regionInit && regionInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
      {
        CkError("ERROR in simpleNeighborInfo::simpleNeighborInfo: regionInit must be greater than or equal to zero and less than globalNumberOfRegions.\n");
        CkExit();
      }
    
    if (!(isBoundary(neighborInit) || (0 <= neighborInit && (neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements ||
                                                             neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))))
      {
        CkError("ERROR in simpleNeighborInfo::simpleNeighborInfo: neighborInit must be a boundary condition code or greater than or equal to zero and less "
                "than globalNumberOfMeshElements or globalNumberOfChannelElements.\n");
        CkExit();
      }
    
    if (!(0.0 < edgeLengthInit))
      {
        CkError("ERROR in simpleNeighborInfo::simpleNeighborInfo: edgeLengthInit must be greater than zero.\n");
        CkExit();
      }
    
    if (!epsilonEqual(1.0, edgeNormalXInit * edgeNormalXInit + edgeNormalYInit * edgeNormalYInit))
      {
        CkError("ERROR in simpleNeighborInfo::simpleNeighborInfo: edgeNormalXInit and edgeNormalYInit must be a unit vector.\n");
        CkExit();
      }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void simpleNeighborInfo::pup(PUP::er &p)
{
  p | expirationTime;
  p | nominalFlowRate;
  p | flowCumulativeShortTerm;
  p | flowCumulativeLongTerm;
  p | region;
  p | neighbor;
  p | edgeLength;
  p | edgeNormalX;
  p | edgeNormalY;
}

bool simpleNeighborInfo::checkInvariant()
{
  bool error = false; // Error flag.
  
  if (!(0 <= region && region < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in simpleNeighborInfo::checkInvariant: region must be greater than or equal to zero and less than globalNumberOfRegions.\n");
      error = true;
    }
  
  if (!(isBoundary(neighbor) || (0 <= neighbor && (neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements ||
                                                   neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))))
    {
      CkError("ERROR in simpleNeighborInfo::checkInvariant: neighbor must be a boundary condition code or greater than or equal to zero and less "
              "than globalNumberOfMeshElements or globalNumberOfChannelElements.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in simpleNeighborInfo::checkInvariant: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!epsilonEqual(1.0, edgeNormalX * edgeNormalX + edgeNormalY * edgeNormalY))
    {
      CkError("ERROR in simpleNeighborInfo::checkInvariant: edgeNormalX and edgeNormalY must be a unit vector.\n");
      error = true;
    }
  
  return error;
}

double Region::calculateZOffset(int meshElement, double meshVertexX[3], double meshVertexY[3], double meshElementX, double meshElementY,
                                double meshElementZSurface, double meshElementSlopeX, double meshElementSlopeY, int channelElement, double channelElementX,
                                double channelElementY, double channelElementZBank, ChannelTypeEnum channelType)
{
  int    ii;                        // Loop counter.
  bool   foundIntersection = false; // Whether the line segment from the center of the mesh element to the center of the channel element intersects any edges
                                    // of the mesh element.
  double zOffset;                   // The Z coordinate offset that is being calculated.
  
  // If the center of the channel element is not inside the mesh element only follow the slope of the mesh element to the edge of the mesh element.  We do this
  // by finding whether the line segment from the center of the mesh element to the center of the channel element intersects any of the mesh edges, and if it
  // does use the intersection point instead of the center of the channel element.
  for (ii = 0; !foundIntersection && ii < 3; ii++)
    {
      foundIntersection = getLineIntersection(meshElementX, meshElementY, channelElementX, channelElementY, meshVertexX[ii], meshVertexY[ii],
                                              meshVertexX[(ii + 1) % 3], meshVertexY[(ii + 1) % 3], &channelElementX, &channelElementY);
    }
  
  zOffset = (channelElementX - meshElementX) * meshElementSlopeX + (channelElementY - meshElementY) * meshElementSlopeY;
  
  // If the channel element is still higher than the mesh element after applying the offset we raise the offset to make them level.  We do not do this for
  // icemasses because there are often real situations where a glacier on a slope is higher than its neighboring mesh elements.
  if (ICEMASS != channelType && zOffset < channelElementZBank - meshElementZSurface)
    {
      if ((2 <= ADHydro::verbosityLevel && 100.0 < channelElementZBank - meshElementZSurface - zOffset) ||
          (3 <= ADHydro::verbosityLevel &&  10.0 < channelElementZBank - meshElementZSurface - zOffset) || 4 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in Region::calculateZOffset: mesh element %d is lower than neighboring channel element %d by %lf meters.  Raising zOffset to "
                  "make them level.\n", meshElement, channelElement, channelElementZBank - meshElementZSurface - zOffset);
        }
      
      zOffset = channelElementZBank - meshElementZSurface;
    }
  
  return zOffset;
}

Region::Region(double referenceDateInit, double currentTimeInit, double simulationEndTimeInit) :
  Element(referenceDateInit, currentTimeInit, simulationEndTimeInit),
  numberOfMeshElements(0),
  meshElements(),
  numberOfChannelElements(0),
  channelElements(),
  outgoingMessages(),
  pupItMeshAndItChannel(false),
  pupItNeighbor(false),
  itMesh(),
  itChannel(),
  itMeshSurfacewaterMeshNeighbor(),
  itMeshSurfacewaterChannelNeighbor(),
  itMeshGroundwaterMeshNeighbor(),
  itMeshGroundwaterChannelNeighbor(),
  itChannelSurfacewaterMeshNeighbor(),
  itChannelSurfacewaterChannelNeighbor(),
  itChannelGroundwaterMeshNeighbor(),
  regionalDtLimit(60.0), // FIXME calculate real value
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS) || (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  needToCheckInvariant(true),
#else // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS) || (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  needToCheckInvariant(false),
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS) || (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  nextCheckpointIndex(1 + (int)floor(currentTimeInit / ADHydro::checkpointPeriod)),
  nextOutputIndex(1 + (int)floor(currentTimeInit / ADHydro::outputPeriod)),
  simulationFinished(false)
{
  FileManager* fileManagerLocalBranch = ADHydro::fileManagerProxy.ckLocalBranch();            // Used for access to local public member variables.
  int          fileManagerLocalIndex  = thisIndex - fileManagerLocalBranch->localRegionStart; // Index of this element in local file manager arrays.

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  if (!(0 <= fileManagerLocalIndex && fileManagerLocalIndex < fileManagerLocalBranch->localNumberOfRegions &&
        NULL != fileManagerLocalBranch->regionNumberOfMeshElements && NULL != fileManagerLocalBranch->regionNumberOfChannelElements))
    {
      CkError("ERROR in Region::Region, region %d: initialization information not available from local file manager.\n", thisIndex);
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

  numberOfMeshElements    = fileManagerLocalBranch->regionNumberOfMeshElements[fileManagerLocalIndex];
  numberOfChannelElements = fileManagerLocalBranch->regionNumberOfChannelElements[fileManagerLocalIndex];

  // Initialization will be done in runUntilSimulationEnd.
  thisProxy[thisIndex].runUntilSimulationEnd();
}

Region::Region(CkMigrateMessage* msg) :
  Element(1721425.5, 0.0, 0.0), // Dummy values will be overwritten by pup.
  numberOfMeshElements(0),
  meshElements(),
  numberOfChannelElements(0),
  channelElements(),
  outgoingMessages(),
  pupItMeshAndItChannel(false),
  pupItNeighbor(false),
  itMesh(),
  itChannel(),
  itMeshSurfacewaterMeshNeighbor(),
  itMeshSurfacewaterChannelNeighbor(),
  itMeshGroundwaterMeshNeighbor(),
  itMeshGroundwaterChannelNeighbor(),
  itChannelSurfacewaterMeshNeighbor(),
  itChannelSurfacewaterChannelNeighbor(),
  itChannelGroundwaterMeshNeighbor(),
  regionalDtLimit(0.0),
  needToCheckInvariant(false),
  nextCheckpointIndex(0),
  nextOutputIndex(0),
  simulationFinished(false)
{
  // Initialization handled by initialization list.
}

void Region::pup(PUP::er &p)
{
  std::map<int, std::vector<RegionMessage> >::iterator it;    // Loop iterator.
  int                                                  index; // For pupping iterators by index.
  
  CBase_Region::pup(p);
  __sdag_pup(p);
  Element::pup(p);
  
  p | numberOfMeshElements;
  p | meshElements;
  p | numberOfChannelElements;
  p | channelElements;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // The vectors inside of outgoingMessages can only be non-empty within certain entry methods and are always emptied before the end of the entry method.
  // Because puping can only happen between entry methods there can never be anything in outgoingMessages that needs to be pupped at this time.
  for (it = outgoingMessages.begin(); it != outgoingMessages.end(); ++it)
    {
      CkAssert((*it).second.empty());
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // Pupping iterators by index.
  p | pupItMeshAndItChannel;
  p | pupItNeighbor;
  
  if (pupItMeshAndItChannel)
    {
      if (p.isUnpacking())
        {
          p | index;
          
          if (-1 == index)
            {
              itMesh = meshElements.end();
            }
          else
            {
              itMesh = meshElements.find(index);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Element must be found.
              CkAssert(itMesh != meshElements.end());
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              if (pupItNeighbor)
                {
                  p | index;
                  
                  if (-1 == index)
                    {
                      itMeshSurfacewaterMeshNeighbor = (*itMesh).second.meshNeighbors.end();
                    }
                  else
                    {
                      itMeshSurfacewaterMeshNeighbor = index + (*itMesh).second.meshNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (-1 == index)
                    {
                      itMeshSurfacewaterChannelNeighbor = (*itMesh).second.channelNeighbors.end();
                    }
                  else
                    {
                      itMeshSurfacewaterChannelNeighbor = index + (*itMesh).second.channelNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (-1 == index)
                    {
                      itMeshGroundwaterMeshNeighbor = (*itMesh).second.underground.meshNeighbors.end();
                    }
                  else
                    {
                      itMeshGroundwaterMeshNeighbor = index + (*itMesh).second.underground.meshNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (-1 == index)
                    {
                      itMeshGroundwaterChannelNeighbor = (*itMesh).second.underground.channelNeighbors.end();
                    }
                  else
                    {
                      itMeshGroundwaterChannelNeighbor = index + (*itMesh).second.underground.channelNeighbors.begin();
                    }
                }
            }
          
          p | index;
          
          if (-1 == index)
            {
              itChannel = channelElements.end();
            }
          else
            {
              itChannel = channelElements.find(index);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Element must be found.
              CkAssert(itChannel != channelElements.end());
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              if (pupItNeighbor)
                {
                  p | index;
                  
                  if (-1 == index)
                    {
                      itChannelSurfacewaterMeshNeighbor = (*itChannel).second.meshNeighbors.end();
                    }
                  else
                    {
                      itChannelSurfacewaterMeshNeighbor = index + (*itChannel).second.meshNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (-1 == index)
                    {
                      itChannelSurfacewaterChannelNeighbor = (*itChannel).second.channelNeighbors.end();
                    }
                  else
                    {
                      itChannelSurfacewaterChannelNeighbor = index + (*itChannel).second.channelNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (-1 == index)
                    {
                      itChannelGroundwaterMeshNeighbor = (*itChannel).second.undergroundMeshNeighbors.end();
                    }
                  else
                    {
                      itChannelGroundwaterMeshNeighbor = index + (*itChannel).second.undergroundMeshNeighbors.begin();
                    }
                }
            }
        }
      else // !p.isUnpacking()
        {
          if (itMesh == meshElements.end())
            {
              index = -1;
              p | index;
            }
          else
            {
              index = (*itMesh).first;
              p | index;
              
              if (pupItNeighbor)
                {
                  if (itMeshSurfacewaterMeshNeighbor == (*itMesh).second.meshNeighbors.end())
                    {
                      index = -1;
                    }
                  else
                    {
                      index = itMeshSurfacewaterMeshNeighbor - (*itMesh).second.meshNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (itMeshSurfacewaterChannelNeighbor == (*itMesh).second.channelNeighbors.end())
                    {
                      index = -1;
                    }
                  else
                    {
                      index = itMeshSurfacewaterChannelNeighbor - (*itMesh).second.channelNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (itMeshGroundwaterMeshNeighbor == (*itMesh).second.underground.meshNeighbors.end())
                    {
                      index = -1;
                    }
                  else
                    {
                      index = itMeshGroundwaterMeshNeighbor - (*itMesh).second.underground.meshNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (itMeshGroundwaterChannelNeighbor == (*itMesh).second.underground.channelNeighbors.end())
                    {
                      index = -1;
                    }
                  else
                    {
                      index = itMeshGroundwaterChannelNeighbor - (*itMesh).second.underground.channelNeighbors.begin();
                    }
                  
                  p | index;
                }
            }
          
          if (itChannel == channelElements.end())
            {
              index = -1;
              p | index;
            }
          else
            {
              index = (*itChannel).first;
              p | index;
              
              if (pupItNeighbor)
                {
                  if (itChannelSurfacewaterMeshNeighbor == (*itChannel).second.meshNeighbors.end())
                    {
                      index = -1;
                    }
                  else
                    {
                      index = itChannelSurfacewaterMeshNeighbor - (*itChannel).second.meshNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (itChannelSurfacewaterChannelNeighbor == (*itChannel).second.channelNeighbors.end())
                    {
                      index = -1;
                    }
                  else
                    {
                      index = itChannelSurfacewaterChannelNeighbor - (*itChannel).second.channelNeighbors.begin();
                    }
                  
                  p | index;
                  
                  if (itChannelGroundwaterMeshNeighbor == (*itChannel).second.undergroundMeshNeighbors.end())
                    {
                      index = -1;
                    }
                  else
                    {
                      index = itChannelGroundwaterMeshNeighbor - (*itChannel).second.undergroundMeshNeighbors.begin();
                    }
                  
                  p | index;
                }
            }
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      // Neighbor iterators can't be in use if ItMesh and itChannel aren't.
      CkAssert(!pupItNeighbor);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  p | regionalDtLimit;
  p | needToCheckInvariant;
  p | nextCheckpointIndex;
  p | nextOutputIndex;
  p | simulationFinished;
}

bool Region::checkInvariant()
{
  bool                                                 error = Element::checkInvariant(); // Error flag.
  std::map<int, std::vector<RegionMessage> >::iterator it;                                // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  for (itMesh = meshElements.begin(); itMesh != meshElements.end(); ++itMesh)
    {
      error = (*itMesh).second.checkInvariant() || error;
      
      // Send check invariant messages to neighbors.
      if (!error)
        {
          for (itMeshSurfacewaterMeshNeighbor  = (*itMesh).second.meshNeighbors.begin();
               itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
            {
              if (isBoundary((*itMeshSurfacewaterMeshNeighbor).neighbor))
                {
                  (*itMeshSurfacewaterMeshNeighbor).neighborInvariantChecked = true;
                }
              else
                {
                  (*itMeshSurfacewaterMeshNeighbor).neighborInvariantChecked = false;

                  thisProxy[(*itMeshSurfacewaterMeshNeighbor).region]
                            .sendMeshSurfacewaterMeshNeighborCheckInvariant(currentTime, (*itMesh).second.elementNumber, *itMeshSurfacewaterMeshNeighbor);
                }
            }

          for (itMeshSurfacewaterChannelNeighbor  = (*itMesh).second.channelNeighbors.begin();
               itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
            {
              (*itMeshSurfacewaterChannelNeighbor).neighborInvariantChecked = false;

              thisProxy[(*itMeshSurfacewaterChannelNeighbor).region]
                        .sendChannelSurfacewaterMeshNeighborCheckInvariant(currentTime, (*itMesh).second.elementNumber, *itMeshSurfacewaterChannelNeighbor);
            }

          for (itMeshGroundwaterMeshNeighbor  = (*itMesh).second.underground.meshNeighbors.begin();
               itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
            {
              if (isBoundary((*itMeshGroundwaterMeshNeighbor).neighbor))
                {
                  (*itMeshGroundwaterMeshNeighbor).neighborInvariantChecked = true;
                }
              else
                {
                  (*itMeshGroundwaterMeshNeighbor).neighborInvariantChecked = false;

                  thisProxy[(*itMeshGroundwaterMeshNeighbor).region]
                            .sendMeshGroundwaterMeshNeighborCheckInvariant(currentTime, (*itMesh).second.elementNumber, *itMeshGroundwaterMeshNeighbor);
                }
            }

          for (itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
               itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
            {
              (*itMeshGroundwaterChannelNeighbor).neighborInvariantChecked = false;

              thisProxy[(*itMeshGroundwaterChannelNeighbor).region]
                        .sendChannelGroundwaterMeshNeighborCheckInvariant(currentTime, (*itMesh).second.elementNumber, *itMeshGroundwaterChannelNeighbor);
            }
        }
    }
  
  for (itChannel = channelElements.begin(); itChannel != channelElements.end(); ++itChannel)
    {
      error = (*itChannel).second.checkInvariant() || error;
      
      // Send check invariant messages to neighbors.
      if (!error)
        {
          for (itChannelSurfacewaterMeshNeighbor  = (*itChannel).second.meshNeighbors.begin();
               itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
            {
              (*itChannelSurfacewaterMeshNeighbor).neighborInvariantChecked = false;

              thisProxy[(*itChannelSurfacewaterMeshNeighbor).region]
                        .sendMeshSurfacewaterChannelNeighborCheckInvariant(currentTime, (*itChannel).second.elementNumber, *itChannelSurfacewaterMeshNeighbor);
            }

          for (itChannelSurfacewaterChannelNeighbor  = (*itChannel).second.channelNeighbors.begin();
               itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
            {
              if (isBoundary((*itChannelSurfacewaterChannelNeighbor).neighbor))
                {
                  (*itChannelSurfacewaterChannelNeighbor).neighborInvariantChecked = true;
                }
              else
                {
                  (*itChannelSurfacewaterChannelNeighbor).neighborInvariantChecked = false;

                  thisProxy[(*itChannelSurfacewaterChannelNeighbor).region]
                            .sendChannelSurfacewaterChannelNeighborCheckInvariant(currentTime, (*itChannel).second.elementNumber, *itChannelSurfacewaterChannelNeighbor);
                }
            }

          for (itChannelGroundwaterMeshNeighbor  = (*itChannel).second.undergroundMeshNeighbors.begin();
               itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
            {
              (*itChannelGroundwaterMeshNeighbor).neighborInvariantChecked = false;

              thisProxy[(*itChannelGroundwaterMeshNeighbor).region]
                        .sendMeshGroundwaterChannelNeighborCheckInvariant(currentTime, (*itChannel).second.elementNumber, *itChannelGroundwaterMeshNeighbor);
            }
        }
    }
  
  // The vectors inside of outgoingMessages can only be non-empty within certain entry methods and are always emptied before the end of the entry method.
  // Because checking the invariant can only happen between entry methods there can never be anything in outgoingMessages at this time.
  for (it = outgoingMessages.begin(); it != outgoingMessages.end(); ++it)
    {
      if (!(*it).second.empty())
        {
          CkError("ERROR in Region::checkInvariant: outgoingMessages must be empty.\n");
          error = true;
        }
    }
  
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in Region::checkInvariant: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  return error;
}

void Region::handleMeshSurfacewaterMeshNeighborCheckInvariant(double messageTime, int neighbor, MeshSurfacewaterMeshNeighborProxy& neighborsProxy)
{
  if (messageTime < currentTime)
    {
      CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor check invariant message received from the past.  This should "
              "never happen.\n");
      CkExit();
    }
  else if (messageTime > currentTime)
    {
      // Message from the future.  Don't receive yet.  Send to self to receive later.
      thisProxy[thisIndex].sendMeshSurfacewaterMeshNeighborCheckInvariant(messageTime, neighbor, neighborsProxy);
    }
  else // if (messageTime == currentTime)
    {
      if (!(thisIndex == neighborsProxy.region))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor check invariant message received at incorrect region.\n");
          CkExit();
        }

      if (!(meshElements.find(neighborsProxy.neighbor) != meshElements.end()))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor check invariant message received at incorrect element.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighbor == neighbor))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor check invariant message received from incorrect neighbor.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].expirationTime == neighborsProxy.expirationTime))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor expirationTime does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].nominalFlowRate == -neighborsProxy.nominalFlowRate))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor nominalFlowRate does not match.\n");
          CkExit();
        }

      if (!epsilonEqual(meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                        -neighborsProxy.flowCumulativeShortTerm))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor flowCumulativeShortTerm does not match.  Value is %lg.  Difference is %lg.\n",
                  meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                  meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm +
                  neighborsProxy.flowCumulativeShortTerm);
          // FIXME There is a problem where cumulative flow can grow large and the roundoff error grows with it, but stays less than epsilon for the large
          // value, and then flow goes the other direction and cumulative flow shrinks and roundoff error is no longer epsilon less than the smaller value.
          // I'm not sure what I'm going to do about this, but for now I'm making this effectively a warning.
          //CkExit();
        }

      if (!epsilonEqual(meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeLongTerm,
                        -neighborsProxy.flowCumulativeLongTerm))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor flowCumulativeLongTerm does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementX == neighborsProxy.neighborX))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor neighborX does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementY == neighborsProxy.neighborY))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor neighborY does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementZSurface == neighborsProxy.neighborZSurface))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor neighborZSurface does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementArea == neighborsProxy.neighborArea))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor neighborArea does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeLength == neighborsProxy.edgeLength))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor edgeLength does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeNormalX == -neighborsProxy.edgeNormalX))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor edgeNormalX does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeNormalY == -neighborsProxy.edgeNormalY))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor edgeNormalY does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].manningsN == neighborsProxy.neighborManningsN))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborCheckInvariant: neighbor neighborManningsN does not match.\n");
          CkExit();
        }

      meshElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborInvariantChecked = true;
    }
}

void Region::handleMeshSurfacewaterChannelNeighborCheckInvariant(double messageTime, int neighbor, ChannelSurfacewaterMeshNeighborProxy& neighborsProxy)
{
  if (messageTime < currentTime)
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor check invariant message received from the past.  This should "
              "never happen.\n");
      CkExit();
    }
  else if (messageTime > currentTime)
    {
      // Message from the future.  Don't receive yet.  Send to self to receive later.
      thisProxy[thisIndex].sendMeshSurfacewaterChannelNeighborCheckInvariant(messageTime, neighbor, neighborsProxy);
    }
  else // if (messageTime == currentTime)
    {
      if (!(thisIndex == neighborsProxy.region))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor check invariant message received at incorrect region.\n");
          CkExit();
        }

      if (!(meshElements.find(neighborsProxy.neighbor) != meshElements.end()))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor check invariant message received at incorrect element.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].neighbor == neighbor))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor check invariant message received from incorrect neighbor.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].expirationTime == neighborsProxy.expirationTime))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor expirationTime does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].nominalFlowRate == -neighborsProxy.nominalFlowRate))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor nominalFlowRate does not match.\n");
          CkExit();
        }

      if (!epsilonEqual(meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                        -neighborsProxy.flowCumulativeShortTerm))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor flowCumulativeShortTerm does not match.  Value is %lg.  Difference is %lg\n",
                  meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                  meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm +
                  neighborsProxy.flowCumulativeShortTerm);
          // FIXME There is a problem where cumulative flow can grow large and the roundoff error grows with it, but stays less than epsilon for the large
          // value, and then flow goes the other direction and cumulative flow shrinks and roundoff error is no longer epsilon less than the smaller value.
          // I'm not sure what I'm going to do about this, but for now I'm making this effectively a warning.
          //CkExit();
        }

      if (!epsilonEqual(meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeLongTerm,
                        -neighborsProxy.flowCumulativeLongTerm))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor flowCumulativeLongTerm does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementZSurface == neighborsProxy.neighborZSurface))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor neighborZSurface does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborZOffset == neighborsProxy.neighborZOffset))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor neighborZOffset does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementArea == neighborsProxy.neighborArea))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor neighborArea does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeLength == neighborsProxy.edgeLength))
        {
          CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborCheckInvariant: neighbor edgeLength does not match.\n");
          CkExit();
        }

      meshElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborInvariantChecked = true;
    }
}

void Region::handleMeshGroundwaterMeshNeighborCheckInvariant(double messageTime, int neighbor, MeshGroundwaterMeshNeighborProxy& neighborsProxy)
{
  if (messageTime < currentTime)
    {
      CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor check invariant message received from the past.  This should "
              "never happen.\n");
      CkExit();
    }
  else if (messageTime > currentTime)
    {
      // Message from the future.  Don't receive yet.  Send to self to receive later.
      thisProxy[thisIndex].sendMeshGroundwaterMeshNeighborCheckInvariant(messageTime, neighbor, neighborsProxy);
    }
  else // if (messageTime == currentTime)
    {
      if (!(thisIndex == neighborsProxy.region))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor check invariant message received at incorrect region.\n");
          CkExit();
        }

      if (!(meshElements.find(neighborsProxy.neighbor) != meshElements.end()))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor check invariant message received at incorrect element.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighbor == neighbor))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor check invariant message received from incorrect neighbor.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].expirationTime ==
            neighborsProxy.expirationTime))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor expirationTime does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].nominalFlowRate ==
            -neighborsProxy.nominalFlowRate))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor nominalFlowRate does not match.\n");
          CkExit();
        }

      if (!epsilonEqual(meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                        -neighborsProxy.flowCumulativeShortTerm))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor flowCumulativeShortTerm does not match.  Value is %lg.  Difference is %lg.\n",
                  meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                  meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm +
                  neighborsProxy.flowCumulativeShortTerm);
          // FIXME There is a problem where cumulative flow can grow large and the roundoff error grows with it, but stays less than epsilon for the large
          // value, and then flow goes the other direction and cumulative flow shrinks and roundoff error is no longer epsilon less than the smaller value.
          // I'm not sure what I'm going to do about this, but for now I'm making this effectively a warning.
          //CkExit();
        }

      if (!epsilonEqual(meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeLongTerm,
                        -neighborsProxy.flowCumulativeLongTerm))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor flowCumulativeLongTerm does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementX == neighborsProxy.neighborX))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor neighborX does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementY == neighborsProxy.neighborY))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor neighborY does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementZSurface == neighborsProxy.neighborZSurface))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor neighborZSurface does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.layerZBottom == neighborsProxy.neighborLayerZBottom))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor neighborLayerZBottom does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementArea == neighborsProxy.neighborArea))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor neighborArea does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeLength == neighborsProxy.edgeLength))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor edgeLength does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeNormalX == -neighborsProxy.edgeNormalX))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor edgeNormalX does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeNormalY == -neighborsProxy.edgeNormalY))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor edgeNormalY does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.conductivity == neighborsProxy.neighborConductivity))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor neighborConductivity does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.porosity == neighborsProxy.neighborPorosity))
        {
          CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborCheckInvariant: neighbor neighborPorosity does not match.\n");
          CkExit();
        }

      meshElements[neighborsProxy.neighbor].underground.meshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborInvariantChecked = true;
    }
}

void Region::handleMeshGroundwaterChannelNeighborCheckInvariant(double messageTime, int neighbor, ChannelGroundwaterMeshNeighborProxy& neighborsProxy)
{
  if (messageTime < currentTime)
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor check invariant message received from the past.  This should "
              "never happen.\n");
      CkExit();
    }
  else if (messageTime > currentTime)
    {
      // Message from the future.  Don't receive yet.  Send to self to receive later.
      thisProxy[thisIndex].sendMeshGroundwaterChannelNeighborCheckInvariant(messageTime, neighbor, neighborsProxy);
    }
  else // if (messageTime == currentTime)
    {
      if (!(thisIndex == neighborsProxy.region))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor check invariant message received at incorrect region.\n");
          CkExit();
        }

      if (!(meshElements.find(neighborsProxy.neighbor) != meshElements.end()))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor check invariant message received at incorrect element.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].neighbor == neighbor))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor check invariant message received from incorrect neighbor.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].expirationTime ==
            neighborsProxy.expirationTime))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor expirationTime does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].nominalFlowRate ==
            -neighborsProxy.nominalFlowRate))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor nominalFlowRate does not match.\n");
          CkExit();
        }

      if (!epsilonEqual(meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                        -neighborsProxy.flowCumulativeShortTerm))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor flowCumulativeShortTerm does not match.  Value is %lg.  Difference is %lg.\n",
                  meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                  meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm +
                  neighborsProxy.flowCumulativeShortTerm);
          // FIXME There is a problem where cumulative flow can grow large and the roundoff error grows with it, but stays less than epsilon for the large
          // value, and then flow goes the other direction and cumulative flow shrinks and roundoff error is no longer epsilon less than the smaller value.
          // I'm not sure what I'm going to do about this, but for now I'm making this effectively a warning.
          //CkExit();
        }

      if (!epsilonEqual(meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeLongTerm,
                        -neighborsProxy.flowCumulativeLongTerm))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor flowCumulativeLongTerm does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].elementZSurface == neighborsProxy.neighborZSurface))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor neighborZSurface does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.layerZBottom == neighborsProxy.neighborLayerZBottom))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor neighborLayerZBottom does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborZOffset ==
            neighborsProxy.neighborZOffset))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor neighborZOffset does not match.\n");
          CkExit();
        }

      if (!(meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeLength == neighborsProxy.edgeLength))
        {
          CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborCheckInvariant: neighbor edgeLength does not match.\n");
          CkExit();
        }

      meshElements[neighborsProxy.neighbor].underground.channelNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborInvariantChecked = true;
    }
}

void Region::handleChannelSurfacewaterMeshNeighborCheckInvariant(double messageTime, int neighbor, MeshSurfacewaterChannelNeighborProxy& neighborsProxy)
{
  if (messageTime < currentTime)
    {
      CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor check invariant message received from the past.  This should "
              "never happen.\n");
      CkExit();
    }
  else if (messageTime > currentTime)
    {
      // Message from the future.  Don't receive yet.  Send to self to receive later.
      thisProxy[thisIndex].sendChannelSurfacewaterMeshNeighborCheckInvariant(messageTime, neighbor, neighborsProxy);
    }
  else // if (messageTime == currentTime)
    {
      if (!(thisIndex == neighborsProxy.region))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor check invariant message received at incorrect region.\n");
          CkExit();
        }

      if (!(channelElements.find(neighborsProxy.neighbor) != channelElements.end()))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor check invariant message received at incorrect element.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighbor == neighbor))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor check invariant message received from incorrect neighbor.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].expirationTime == neighborsProxy.expirationTime))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor expirationTime does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].nominalFlowRate == -neighborsProxy.nominalFlowRate))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor nominalFlowRate does not match.\n");
          CkExit();
        }

      if (!epsilonEqual(channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                        -neighborsProxy.flowCumulativeShortTerm))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor flowCumulativeShortTerm does not match.  Value is %lg.  Difference is %lg.\n",
                  channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                  channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm +
                  neighborsProxy.flowCumulativeShortTerm);
          // FIXME There is a problem where cumulative flow can grow large and the roundoff error grows with it, but stays less than epsilon for the large
          // value, and then flow goes the other direction and cumulative flow shrinks and roundoff error is no longer epsilon less than the smaller value.
          // I'm not sure what I'm going to do about this, but for now I'm making this effectively a warning.
          //CkExit();
        }

      if (!epsilonEqual(channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeLongTerm,
                        -neighborsProxy.flowCumulativeLongTerm))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor flowCumulativeLongTerm does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].elementZBank == neighborsProxy.neighborZBank))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor neighborZBank does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].elementZBed == neighborsProxy.neighborZBed))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor neighborZSurface does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborZOffset == neighborsProxy.neighborZOffset))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor neighborZOffset does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeLength == neighborsProxy.edgeLength))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor edgeLength does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].baseWidth == neighborsProxy.neighborBaseWidth))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor neighborBaseWidth does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].sideSlope == neighborsProxy.neighborSideSlope))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborCheckInvariant: neighbor neighborSideSlope does not match.\n");
          CkExit();
        }

      channelElements[neighborsProxy.neighbor].meshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborInvariantChecked = true;
    }
}

void Region::handleChannelSurfacewaterChannelNeighborCheckInvariant(double messageTime, int neighbor, ChannelSurfacewaterChannelNeighborProxy& neighborsProxy)
{
  if (messageTime < currentTime)
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor check invariant message received from the past.  This should "
              "never happen.\n");
      CkExit();
    }
  else if (messageTime > currentTime)
    {
      // Message from the future.  Don't receive yet.  Send to self to receive later.
      thisProxy[thisIndex].sendChannelSurfacewaterChannelNeighborCheckInvariant(messageTime, neighbor, neighborsProxy);
    }
  else // if (messageTime == currentTime)
    {
      if (!(thisIndex == neighborsProxy.region))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor check invariant message received at incorrect region.\n");
          CkExit();
        }

      if (!(channelElements.find(neighborsProxy.neighbor) != channelElements.end()))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor check invariant message received at incorrect element.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].neighbor == neighbor))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor check invariant message received from incorrect neighbor.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].expirationTime == neighborsProxy.expirationTime))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor expirationTime does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].nominalFlowRate == -neighborsProxy.nominalFlowRate))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor nominalFlowRate does not match.\n");
          CkExit();
        }

      if (!epsilonEqual(channelElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                        -neighborsProxy.flowCumulativeShortTerm))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor flowCumulativeShortTerm does not match.  Value is %lg.  Difference is %lg.\n",
                  channelElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                  channelElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm +
                  neighborsProxy.flowCumulativeShortTerm);
          // FIXME There is a problem where cumulative flow can grow large and the roundoff error grows with it, but stays less than epsilon for the large
          // value, and then flow goes the other direction and cumulative flow shrinks and roundoff error is no longer epsilon less than the smaller value.
          // I'm not sure what I'm going to do about this, but for now I'm making this effectively a warning.
          //CkExit();
        }

      if (!epsilonEqual(channelElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeLongTerm,
                        -neighborsProxy.flowCumulativeLongTerm))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor flowCumulativeLongTerm does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].channelType == neighborsProxy.neighborChannelType))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor neighborChannelType does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].elementZBank == neighborsProxy.neighborZBank))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor neighborZBank does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].elementZBed == neighborsProxy.neighborZBed))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor neighborZBed does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].elementLength == neighborsProxy.neighborLength))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor neighborLength does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].baseWidth == neighborsProxy.neighborBaseWidth))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor neighborBaseWidth does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].sideSlope == neighborsProxy.neighborSideSlope))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor neighborSideSlope does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].manningsN == neighborsProxy.neighborManningsN))
        {
          CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborCheckInvariant: neighbor neighborManningsN does not match.\n");
          CkExit();
        }

      channelElements[neighborsProxy.neighbor].channelNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborInvariantChecked = true;
    }
}

void Region::handleChannelGroundwaterMeshNeighborCheckInvariant(double messageTime, int neighbor, MeshGroundwaterChannelNeighborProxy& neighborsProxy)
{
  if (messageTime < currentTime)
    {
      CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor check invariant message received from the past.  This should "
              "never happen.\n");
      CkExit();
    }
  else if (messageTime > currentTime)
    {
      // Message from the future.  Don't receive yet.  Send to self to receive later.
      thisProxy[thisIndex].sendChannelGroundwaterMeshNeighborCheckInvariant(messageTime, neighbor, neighborsProxy);
    }
  else // if (messageTime == currentTime)
    {
      if (!(thisIndex == neighborsProxy.region))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor check invariant message received at incorrect region.\n");
          CkExit();
        }

      if (!(channelElements.find(neighborsProxy.neighbor) != channelElements.end()))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor check invariant message received at incorrect element.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighbor == neighbor))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor check invariant message received from incorrect neighbor.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].expirationTime ==
            neighborsProxy.expirationTime))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor expirationTime does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].nominalFlowRate ==
            -neighborsProxy.nominalFlowRate))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor nominalFlowRate does not match.\n");
          CkExit();
        }

      if (!epsilonEqual(channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                        -neighborsProxy.flowCumulativeShortTerm))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor flowCumulativeShortTerm does not match.  Value is %lg.  Difference is %lg.\n",
                  channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm,
                  channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeShortTerm +
                  neighborsProxy.flowCumulativeShortTerm);
          // FIXME There is a problem where cumulative flow can grow large and the roundoff error grows with it, but stays less than epsilon for the large
          // value, and then flow goes the other direction and cumulative flow shrinks and roundoff error is no longer epsilon less than the smaller value.
          // I'm not sure what I'm going to do about this, but for now I'm making this effectively a warning.
          //CkExit();
        }

      if (!epsilonEqual(channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].flowCumulativeLongTerm,
                        -neighborsProxy.flowCumulativeLongTerm))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor flowCumulativeLongTerm does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].elementZBank == neighborsProxy.neighborZBank))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor neighborZBank does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].elementZBed == neighborsProxy.neighborZBed))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor neighborZBed does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborZOffset ==
            neighborsProxy.neighborZOffset))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor neighborZOffset does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].edgeLength == neighborsProxy.edgeLength))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor edgeLength does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].baseWidth == neighborsProxy.neighborBaseWidth))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor neighborBaseWidth does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].sideSlope == neighborsProxy.neighborSideSlope))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor neighborSideSlope does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].bedConductivity == neighborsProxy.neighborBedConductivity))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor neighborBedConductivity does not match.\n");
          CkExit();
        }

      if (!(channelElements[neighborsProxy.neighbor].bedThickness == neighborsProxy.neighborBedThickness))
        {
          CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborCheckInvariant: neighbor neighborBedThickness does not match.\n");
          CkExit();
        }

      channelElements[neighborsProxy.neighbor].undergroundMeshNeighbors[neighborsProxy.reciprocalNeighborProxy].neighborInvariantChecked = true;
    }
}

bool Region::allNeighborInvariantsChecked()
{
  bool allChecked = true; // Stays true until we find one that is not checked.
  
  // FIXLATER I could modify this to make it an incremental scan like allNominalFlowRatesCalculated.  Performance isn't as important for invariant checking code
  // that gets turned off for performance critical runs.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  for (itMesh = meshElements.begin(); allChecked && itMesh != meshElements.end(); ++itMesh)
    {
      for (itMeshSurfacewaterMeshNeighbor = (*itMesh).second.meshNeighbors.begin();
           allChecked && itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          allChecked = (*itMeshSurfacewaterMeshNeighbor).neighborInvariantChecked;
        }
      
      for (itMeshSurfacewaterChannelNeighbor = (*itMesh).second.channelNeighbors.begin();
           allChecked && itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
        {
          allChecked = (*itMeshSurfacewaterChannelNeighbor).neighborInvariantChecked;
        }
      
      for (itMeshGroundwaterMeshNeighbor = (*itMesh).second.underground.meshNeighbors.begin();
           allChecked && itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          allChecked = (*itMeshGroundwaterMeshNeighbor).neighborInvariantChecked;
        }
      
      for (itMeshGroundwaterChannelNeighbor = (*itMesh).second.underground.channelNeighbors.begin();
           allChecked && itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
        {
          allChecked = (*itMeshGroundwaterChannelNeighbor).neighborInvariantChecked;
        }
    }
  
  for (itChannel = channelElements.begin(); allChecked && itChannel != channelElements.end(); ++itChannel)
    {
      for (itChannelSurfacewaterMeshNeighbor = (*itChannel).second.meshNeighbors.begin();
           allChecked && itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
        {
          allChecked = (*itChannelSurfacewaterMeshNeighbor).neighborInvariantChecked;
        }
      
      for (itChannelSurfacewaterChannelNeighbor = (*itChannel).second.channelNeighbors.begin();
           allChecked && itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          allChecked = (*itChannelSurfacewaterChannelNeighbor).neighborInvariantChecked;
        }
      
      for (itChannelGroundwaterMeshNeighbor = (*itChannel).second.undergroundMeshNeighbors.begin();
           allChecked && itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
        {
          allChecked = (*itChannelGroundwaterMeshNeighbor).neighborInvariantChecked;
        }
    }
  
  return allChecked;
}

void Region::handleInitializeMeshElement(int elementNumberInit, int catchmentInit, int vegetationTypeInit, int soilTypeInit, double vertexXInit[3],
                                         double vertexYInit[3], double elementXInit, double elementYInit, double elementZSurfaceInit, double layerZBottomInit,
                                         double elementAreaInit, double slopeXInit, double slopeYInit, double latitudeInit, double longitudeInit,
                                         double manningsNInit, double conductivityInit, double porosityInit, double surfacewaterDepthInit,
                                         double surfacewaterErrorInit, double groundwaterHeadInit, double groundwaterRechargeInit, double groundwaterErrorInit,
                                         double precipitationRateInit, double precipitationCumulativeShortTermInit, double precipitationCumulativeLongTermInit,
                                         double evaporationRateInit, double evaporationCumulativeShortTermInit, double evaporationCumulativeLongTermInit,
                                         double transpirationRateInit, double transpirationCumulativeShortTermInit, double transpirationCumulativeLongTermInit,
                                         EvapoTranspirationForcingStruct& evapoTranspirationForcingInit,
                                         EvapoTranspirationStateStruct& evapoTranspirationStateInit,
                                         InfiltrationAndGroundwater::GroundwaterMethodEnum groundwaterMethodInit,
                                         InfiltrationAndGroundwater::VadoseZone vadoseZoneInit, std::vector<simpleNeighborInfo> surfacewaterMeshNeighbors,
                                         std::vector<simpleNeighborInfo> surfacewaterChannelNeighbors,
                                         std::vector<simpleNeighborInfo> groundwaterMeshNeighbors, std::vector<simpleNeighborInfo> groundwaterChannelNeighbors)
{
  std::vector<simpleNeighborInfo>::iterator it; // Loop iterator.
  
  // Most parameters are error checked in the MeshElement constructor.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  for (it = surfacewaterMeshNeighbors.begin(); it != surfacewaterMeshNeighbors.end(); ++it)
    {
      if ((*it).checkInvariant())
        {
          CkExit();
        }
    }

  for (it = surfacewaterChannelNeighbors.begin(); it != surfacewaterChannelNeighbors.end(); ++it)
    {
      if ((*it).checkInvariant())
        {
          CkExit();
        }
    }

  for (it = groundwaterMeshNeighbors.begin(); it != groundwaterMeshNeighbors.end(); ++it)
    {
      if ((*it).checkInvariant())
        {
          CkExit();
        }
    }

  for (it = groundwaterChannelNeighbors.begin(); it != groundwaterChannelNeighbors.end(); ++it)
    {
      if ((*it).checkInvariant())
        {
          CkExit();
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  // Insert element in this region's elements vector.
  meshElements.insert(std::pair<int, MeshElement>(elementNumberInit,
                                                  MeshElement(elementNumberInit, catchmentInit, vegetationTypeInit, soilTypeInit, vertexXInit, vertexYInit,
                                                              elementXInit, elementYInit, elementZSurfaceInit, layerZBottomInit, elementAreaInit, slopeXInit,
                                                              slopeYInit, latitudeInit, longitudeInit, manningsNInit, conductivityInit, porosityInit,
                                                              surfacewaterDepthInit, surfacewaterErrorInit, groundwaterHeadInit, groundwaterRechargeInit,
                                                              groundwaterErrorInit, precipitationRateInit, precipitationCumulativeShortTermInit,
                                                              precipitationCumulativeLongTermInit, evaporationRateInit, evaporationCumulativeShortTermInit,
                                                              evaporationCumulativeLongTermInit, transpirationRateInit, transpirationCumulativeShortTermInit,
                                                              transpirationCumulativeLongTermInit, evapoTranspirationForcingInit, evapoTranspirationStateInit,
                                                              groundwaterMethodInit, vadoseZoneInit)));
  
  // Insert neighbor proxies and send init messages to neighbors.
  for (it = surfacewaterMeshNeighbors.begin(); it != surfacewaterMeshNeighbors.end(); ++it)
    {
      meshElements[elementNumberInit].meshNeighbors.push_back(
          MeshSurfacewaterMeshNeighborProxy((*it).expirationTime, (*it).nominalFlowRate, (*it).flowCumulativeShortTerm, (*it).flowCumulativeLongTerm,
                                            (*it).region, (*it).neighbor, 0, 0.0, 0.0, 0.0, isBoundary((*it).neighbor) ? 0.0 : 1.0, (*it).edgeLength,
                                            (*it).edgeNormalX, (*it).edgeNormalY, isBoundary((*it).neighbor) ? 0.0 : 1.0));
      
      if (isBoundary((*it).neighbor))
        {
          meshElements[elementNumberInit].meshNeighbors.back().neighborInitialized = true;
        }
      else
        {
          thisProxy[(*it).region].sendMeshSurfacewaterMeshNeighborInitMessage(
              (*it).neighbor, elementNumberInit, meshElements[elementNumberInit].meshNeighbors.size() - 1, elementXInit, elementYInit, elementZSurfaceInit,
              elementAreaInit, manningsNInit);
        }
    }
  
  for (it = surfacewaterChannelNeighbors.begin(); it != surfacewaterChannelNeighbors.end(); ++it)
    {
      meshElements[elementNumberInit].channelNeighbors.push_back(
          MeshSurfacewaterChannelNeighborProxy((*it).expirationTime, (*it).nominalFlowRate, (*it).flowCumulativeShortTerm, (*it).flowCumulativeLongTerm,
                                               (*it).region, (*it).neighbor, 0, 0.0, 0.0, 0.0, (*it).edgeLength, 1.0, 0.0));
      
      thisProxy[(*it).region].sendChannelSurfacewaterMeshNeighborInitMessage(
          (*it).neighbor, elementNumberInit, meshElements[elementNumberInit].channelNeighbors.size() - 1, vertexXInit, vertexYInit, elementXInit, elementYInit,
          elementZSurfaceInit, elementAreaInit, slopeXInit, slopeYInit);
    }
  
  for (it = groundwaterMeshNeighbors.begin(); it != groundwaterMeshNeighbors.end(); ++it)
    {
      meshElements[elementNumberInit].underground.meshNeighbors.push_back(
          MeshGroundwaterMeshNeighborProxy((*it).expirationTime, (*it).nominalFlowRate, (*it).flowCumulativeShortTerm, (*it).flowCumulativeLongTerm,
                                           (*it).region, (*it).neighbor, 0, 0.0, 0.0, 0.0, 0.0, isBoundary((*it).neighbor) ? 0.0 : 1.0, (*it).edgeLength,
                                           (*it).edgeNormalX, (*it).edgeNormalY, isBoundary((*it).neighbor) ? 0.0 : 1.0,
                                           isBoundary((*it).neighbor) ? 0.0 : 1.0));
      
      if (isBoundary((*it).neighbor))
        {
          meshElements[elementNumberInit].underground.meshNeighbors.back().neighborInitialized = true;
        }
      else
        {
          thisProxy[(*it).region].sendMeshGroundwaterMeshNeighborInitMessage(
              (*it).neighbor, elementNumberInit, meshElements[elementNumberInit].underground.meshNeighbors.size() - 1, elementXInit, elementYInit,
              elementZSurfaceInit, layerZBottomInit, elementAreaInit, conductivityInit, porosityInit);
        }
    }
  
  for (it = groundwaterChannelNeighbors.begin(); it != groundwaterChannelNeighbors.end(); ++it)
    {
      meshElements[elementNumberInit].underground.channelNeighbors.push_back(
          MeshGroundwaterChannelNeighborProxy((*it).expirationTime, (*it).nominalFlowRate, (*it).flowCumulativeShortTerm, (*it).flowCumulativeLongTerm,
                                              (*it).region, (*it).neighbor, 0, 0.0, 0.0, 0.0, (*it).edgeLength, 1.0, 0.0, 1.0, 1.0));
      
      thisProxy[(*it).region].sendChannelGroundwaterMeshNeighborInitMessage(
          (*it).neighbor, elementNumberInit, meshElements[elementNumberInit].underground.channelNeighbors.size() - 1, vertexXInit, vertexYInit, elementXInit,
          elementYInit, elementZSurfaceInit, layerZBottomInit, slopeXInit, slopeYInit);
    }
}

void Region::handleInitializeChannelElement(int elementNumberInit, ChannelTypeEnum channelTypeInit, long long reachCodeInit, double elementXInit,
                                            double elementYInit, double elementZBankInit, double elementZBedInit, double elementLengthInit,
                                            double latitudeInit, double longitudeInit, double baseWidthInit, double sideSlopeInit, double bedConductivityInit,
                                            double bedThicknessInit, double manningsNInit, double surfacewaterDepthInit, double surfacewaterErrorInit,
                                            double precipitationRateInit, double precipitationCumulativeShortTermInit,
                                            double precipitationCumulativeLongTermInit, double evaporationRateInit, double evaporationCumulativeShortTermInit,
                                            double evaporationCumulativeLongTermInit, EvapoTranspirationForcingStruct& evapoTranspirationForcingInit,
                                            EvapoTranspirationStateStruct& evapoTranspirationStateInit,
                                            std::vector<simpleNeighborInfo> surfacewaterMeshNeighbors,
                                            std::vector<simpleNeighborInfo> surfacewaterChannelNeighbors,
                                            std::vector<simpleNeighborInfo> groundwaterMeshNeighbors)
{
  std::vector<simpleNeighborInfo>::iterator it; // Loop iterator.
  
  // Most parameters are error checked in the ChannelElement constructor.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  for (it = surfacewaterMeshNeighbors.begin(); it != surfacewaterMeshNeighbors.end(); ++it)
    {
      if ((*it).checkInvariant())
        {
          CkExit();
        }
    }

  for (it = surfacewaterChannelNeighbors.begin(); it != surfacewaterChannelNeighbors.end(); ++it)
    {
      if ((*it).checkInvariant())
        {
          CkExit();
        }
    }

  for (it = groundwaterMeshNeighbors.begin(); it != groundwaterMeshNeighbors.end(); ++it)
    {
      if ((*it).checkInvariant())
        {
          CkExit();
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  // Insert element in this region's elements vector.
  channelElements.insert(std::pair<int, ChannelElement>(elementNumberInit,
                                                        ChannelElement(elementNumberInit, channelTypeInit, reachCodeInit, elementXInit, elementYInit,
                                                                       elementZBankInit, elementZBedInit, elementLengthInit, latitudeInit, longitudeInit,
                                                                       baseWidthInit, sideSlopeInit, bedConductivityInit, bedThicknessInit, manningsNInit,
                                                                       surfacewaterDepthInit, surfacewaterErrorInit, precipitationRateInit,
                                                                       precipitationCumulativeShortTermInit, precipitationCumulativeLongTermInit,
                                                                       evaporationRateInit, evaporationCumulativeShortTermInit,
                                                                       evaporationCumulativeLongTermInit, evapoTranspirationForcingInit, evapoTranspirationStateInit)));
  
  // Insert neighbor proxies and send init messages to neighbors.
  for (it = surfacewaterMeshNeighbors.begin(); it != surfacewaterMeshNeighbors.end(); ++it)
    {
      channelElements[elementNumberInit].meshNeighbors.push_back(
          ChannelSurfacewaterMeshNeighborProxy((*it).expirationTime, (*it).nominalFlowRate, (*it).flowCumulativeShortTerm, (*it).flowCumulativeLongTerm,
                                               (*it).region, (*it).neighbor, 0, 0.0, 0.0, 1.0, (*it).edgeLength));
      
      thisProxy[(*it).region].sendMeshSurfacewaterChannelNeighborInitMessage(
              (*it).neighbor, elementNumberInit, channelElements[elementNumberInit].meshNeighbors.size() - 1, channelTypeInit, elementXInit, elementYInit,
              elementZBankInit, elementZBedInit, baseWidthInit, sideSlopeInit);
    }
  
  for (it = surfacewaterChannelNeighbors.begin(); it != surfacewaterChannelNeighbors.end(); ++it)
    {
      channelElements[elementNumberInit].channelNeighbors.push_back(
          ChannelSurfacewaterChannelNeighborProxy((*it).expirationTime, (*it).nominalFlowRate, (*it).flowCumulativeShortTerm, (*it).flowCumulativeLongTerm,
                                                  (*it).region, (*it).neighbor, 0, isBoundary((*it).neighbor) ? NOT_USED : STREAM, 0.0, 0.0,
                                                  isBoundary((*it).neighbor) ? 0.0 : 1.0, isBoundary((*it).neighbor) ? 0.0 : 1.0, 0.0,
                                                  isBoundary((*it).neighbor) ? 0.0 : 1.0));
      
      if (isBoundary((*it).neighbor))
        {
          channelElements[elementNumberInit].channelNeighbors.back().neighborInitialized = true;
        }
      else
        {
          thisProxy[(*it).region].sendChannelSurfacewaterChannelNeighborInitMessage(
              (*it).neighbor, elementNumberInit, channelElements[elementNumberInit].channelNeighbors.size() - 1, channelTypeInit, elementZBankInit,
              elementZBedInit, elementLengthInit, baseWidthInit, sideSlopeInit, manningsNInit);
        }
    }
  
  for (it = groundwaterMeshNeighbors.begin(); it != groundwaterMeshNeighbors.end(); ++it)
    {
      channelElements[elementNumberInit].undergroundMeshNeighbors.push_back(
          ChannelGroundwaterMeshNeighborProxy((*it).expirationTime, (*it).nominalFlowRate, (*it).flowCumulativeShortTerm, (*it).flowCumulativeLongTerm,
                                              (*it).region, (*it).neighbor, 0, 0.0, 0.0, 0.0, (*it).edgeLength));
      
      thisProxy[(*it).region].sendMeshGroundwaterChannelNeighborInitMessage(
          (*it).neighbor, elementNumberInit, channelElements[elementNumberInit].undergroundMeshNeighbors.size() - 1, channelTypeInit, elementXInit, elementYInit,
          elementZBankInit, elementZBedInit, baseWidthInit, sideSlopeInit, bedConductivityInit, bedThicknessInit);
    }
}

void Region::handleMeshSurfacewaterMeshNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, double neighborX, double neighborY,
                                                           double neighborZSurface, double neighborArea, double neighborManningsN)
{
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(meshElements.find(element) != meshElements.end()))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborInitMessage: neighbor init message received at incorrect region.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborInitMessage: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborArea))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborInitMessage: neighborArea must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborManningsN))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborInitMessage: neighborManningsN must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  it = meshElements[element].meshNeighbors.begin();
  
  while (it != meshElements[element].meshNeighbors.end() && (*it).neighbor != neighbor)
    {
      ++it;
    }
  
  if (it == meshElements[element].meshNeighbors.end())
    {
      CkError("ERROR in Region::handleMeshSurfacewaterMeshNeighborInitMessage, region %d, element %d, neighbor %d: neighbor not found.\n", thisIndex, element,
              neighbor);
      CkExit();
    }
  else
    {
      (*it).reciprocalNeighborProxy = reciprocalNeighborProxy;
      (*it).neighborX               = neighborX;
      (*it).neighborY               = neighborY;
      (*it).neighborZSurface        = neighborZSurface;
      (*it).neighborArea            = neighborArea;
      (*it).neighborManningsN       = neighborManningsN;
      (*it).neighborInitialized     = true;
    }
}

void Region::handleMeshSurfacewaterChannelNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, ChannelTypeEnum neighborChannelType,
                                                              double neighborX, double neighborY, double neighborZBank, double neighborZBed,
                                                              double neighborBaseWidth, double neighborSideSlope)
{
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(meshElements.find(element) != meshElements.end()))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborInitMessage: neighbor init message received at incorrect region.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborInitMessage: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      CkExit();
    }

  if (!(STREAM == neighborChannelType || WATERBODY == neighborChannelType || ICEMASS == neighborChannelType))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborInitMessage: neighborChannelType must be STREAM or WATERBODY or ICEMASS.\n");
      CkExit();
    }
  
  if (!(neighborZBank >= neighborZBed))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborInitMessage: neighborZBank must be greater than or equal to neighborZBed.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborBaseWidth))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborInitMessage: neighborBaseWidth must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborSideSlope))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborInitMessage: neighborSideSlope must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBaseWidth || 0.0 < neighborSideSlope))
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborInitMessage: one of neighborBaseWidth or neighborSideSlope must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  it = meshElements[element].channelNeighbors.begin();
  
  while (it != meshElements[element].channelNeighbors.end() && (*it).neighbor != neighbor)
    {
      ++it;
    }
  
  if (it == meshElements[element].channelNeighbors.end())
    {
      CkError("ERROR in Region::handleMeshSurfacewaterChannelNeighborInitMessage, region %d, element %d, neighbor %d: neighbor not found.\n", thisIndex,
              element, neighbor);
      CkExit();
    }
  else
    {
      (*it).reciprocalNeighborProxy = reciprocalNeighborProxy;
      (*it).neighborZBank           = neighborZBank;
      (*it).neighborZBed            = neighborZBed;
      (*it).neighborZOffset         = calculateZOffset(element, meshElements[element].vertexX, meshElements[element].vertexY, meshElements[element].elementX,
                                                       meshElements[element].elementY, meshElements[element].elementZSurface,
                                                       meshElements[element].underground.slopeX, meshElements[element].underground.slopeY, neighbor, neighborX,
                                                       neighborY, neighborZBank, neighborChannelType);
      (*it).neighborBaseWidth       = neighborBaseWidth;
      (*it).neighborSideSlope       = neighborSideSlope;
      (*it).neighborInitialized     = true;
    }
}

void Region::handleMeshGroundwaterMeshNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, double neighborX, double neighborY,
                                                          double neighborZSurface, double neighborLayerZBottom, double neighborArea,
                                                          double neighborConductivity, double neighborPorosity)
{
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(meshElements.find(element) != meshElements.end()))
    {
      CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborInitMessage: neighbor init message received at incorrect region.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborInitMessage: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(neighborZSurface >= neighborLayerZBottom))
    {
      CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborInitMessage: neighborZSurface must be greater than or equal to neighborLayerZBottom.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborArea))
    {
      CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborInitMessage: neighborArea must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborConductivity))
    {
      CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborInitMessage: neighborConductivity must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborPorosity))
    {
      CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborInitMessage: neighborPorosity must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  it = meshElements[element].underground.meshNeighbors.begin();
  
  while (it != meshElements[element].underground.meshNeighbors.end() && (*it).neighbor != neighbor)
    {
      ++it;
    }
  
  if (it == meshElements[element].underground.meshNeighbors.end())
    {
      CkError("ERROR in Region::handleMeshGroundwaterMeshNeighborInitMessage, region %d, element %d, neighbor %d: neighbor not found.\n", thisIndex, element,
              neighbor);
      CkExit();
    }
  else
    {
      (*it).reciprocalNeighborProxy = reciprocalNeighborProxy;
      (*it).neighborX               = neighborX;
      (*it).neighborY               = neighborY;
      (*it).neighborZSurface        = neighborZSurface;
      (*it).neighborLayerZBottom    = neighborLayerZBottom;
      (*it).neighborArea            = neighborArea;
      (*it).neighborConductivity    = neighborConductivity;
      (*it).neighborPorosity        = neighborPorosity;
      (*it).neighborInitialized     = true;
    }
}

void Region::handleMeshGroundwaterChannelNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, ChannelTypeEnum neighborChannelType,
                                                             double neighborX, double neighborY, double neighborZBank, double neighborZBed,
                                                             double neighborBaseWidth, double neighborSideSlope, double neighborBedConductivity,
                                                             double neighborBedThickness)
{
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(meshElements.find(element) != meshElements.end()))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: neighbor init message received at incorrect region.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      CkExit();
    }

  if (!(STREAM == neighborChannelType || WATERBODY == neighborChannelType || ICEMASS == neighborChannelType))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: neighborChannelType must be STREAM or WATERBODY or ICEMASS.\n");
      CkExit();
    }
  
  if (!(neighborZBank >= neighborZBed))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: neighborZBank must be greater than or equal to neighborZBed.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborBaseWidth))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: neighborBaseWidth must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborSideSlope))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: neighborSideSlope must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBaseWidth || 0.0 < neighborSideSlope))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: one of neighborBaseWidth or neighborSideSlope must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBedConductivity))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: neighborBedConductivity must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBedThickness))
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage: neighborBedThickness must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  it = meshElements[element].underground.channelNeighbors.begin();
  
  while (it != meshElements[element].underground.channelNeighbors.end() && (*it).neighbor != neighbor)
    {
      ++it;
    }
  
  if (it == meshElements[element].underground.channelNeighbors.end())
    {
      CkError("ERROR in Region::handleMeshGroundwaterChannelNeighborInitMessage, region %d, element %d, neighbor %d: neighbor not found.\n", thisIndex,
              element, neighbor);
      CkExit();
    }
  else
    {
      (*it).reciprocalNeighborProxy = reciprocalNeighborProxy;
      (*it).neighborZBank           = neighborZBank;
      (*it).neighborZBed            = neighborZBed;
      (*it).neighborZOffset         = calculateZOffset(element, meshElements[element].vertexX, meshElements[element].vertexY, meshElements[element].elementX,
                                                       meshElements[element].elementY, meshElements[element].elementZSurface,
                                                       meshElements[element].underground.slopeX, meshElements[element].underground.slopeY, neighbor, neighborX,
                                                       neighborY, neighborZBank, neighborChannelType);
      (*it).neighborBaseWidth       = neighborBaseWidth;
      (*it).neighborSideSlope       = neighborSideSlope;
      (*it).neighborBedConductivity = neighborBedConductivity;
      (*it).neighborBedThickness    = neighborBedThickness;
      (*it).neighborInitialized     = true;
    }
}

void Region::handleChannelSurfacewaterMeshNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, double neighborVertexX[3],
                                                              double neighborVertexY[3], double neighborX, double neighborY, double neighborZSurface,
                                                              double neighborArea, double neighborSlopeX, double neighborSlopeY)
{
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(channelElements.find(element) != channelElements.end()))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborInitMessage: neighbor init message received at incorrect region.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborInitMessage: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborArea))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborInitMessage: neighborArea must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  it = channelElements[element].meshNeighbors.begin();
  
  while (it != channelElements[element].meshNeighbors.end() && (*it).neighbor != neighbor)
    {
      ++it;
    }
  
  if (it == channelElements[element].meshNeighbors.end())
    {
      CkError("ERROR in Region::handleChannelSurfacewaterMeshNeighborInitMessage, region %d, element %d, neighbor %d: neighbor not found.\n", thisIndex,
              element, neighbor);
      CkExit();
    }
  else
    {
      (*it).reciprocalNeighborProxy = reciprocalNeighborProxy;
      (*it).neighborZSurface        = neighborZSurface;
      (*it).neighborZOffset         = calculateZOffset(neighbor, neighborVertexX, neighborVertexY, neighborX, neighborY, neighborZSurface, neighborSlopeX,
                                                       neighborSlopeY, element, channelElements[element].elementX, channelElements[element].elementY,
                                                       channelElements[element].elementZBank, channelElements[element].channelType);
      (*it).neighborArea            = neighborArea;
      (*it).neighborInitialized     = true;
    }
}

void Region::handleChannelSurfacewaterChannelNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, ChannelTypeEnum neighborChannelType,
                                                                 double neighborZBank, double neighborZBed, double neighborLength, double neighborBaseWidth,
                                                                 double neighborSideSlope, double neighborManningsN)
{
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(channelElements.find(element) != channelElements.end()))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: neighbor init message received at incorrect region.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      CkExit();
    }

  if (!(STREAM == neighborChannelType || WATERBODY == neighborChannelType || ICEMASS == neighborChannelType))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: neighborChannelType must be STREAM or WATERBODY or ICEMASS.\n");
      CkExit();
    }
  
  if (!(neighborZBank >= neighborZBed))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: neighborZBank must be greater than or equal to neighborZBed.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborLength))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: neighborLength must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborBaseWidth))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: neighborBaseWidth must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborSideSlope))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: neighborSideSlope must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBaseWidth || 0.0 < neighborSideSlope))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: one of neighborBaseWidth or neighborSideSlope must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborManningsN))
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage: neighborManningsN must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  it = channelElements[element].channelNeighbors.begin();
  
  while (it != channelElements[element].channelNeighbors.end() && (*it).neighbor != neighbor)
    {
      ++it;
    }
  
  if (it == channelElements[element].channelNeighbors.end())
    {
      CkError("ERROR in Region::handleChannelSurfacewaterChannelNeighborInitMessage, region %d, element %d, neighbor %d: neighbor not found.\n", thisIndex,
              element, neighbor);
      CkExit();
    }
  else
    {
      (*it).reciprocalNeighborProxy = reciprocalNeighborProxy;
      (*it).neighborChannelType     = neighborChannelType;
      (*it).neighborZBank           = neighborZBank;
      (*it).neighborZBed            = neighborZBed;
      (*it).neighborLength          = neighborLength;
      (*it).neighborBaseWidth       = neighborBaseWidth;
      (*it).neighborSideSlope       = neighborSideSlope;
      (*it).neighborManningsN       = neighborManningsN;
      (*it).neighborInitialized     = true;
    }
}

void Region::handleChannelGroundwaterMeshNeighborInitMessage(int element, int neighbor, int reciprocalNeighborProxy, double neighborVertexX[3],
                                                             double neighborVertexY[3], double neighborX, double neighborY, double neighborZSurface,
                                                             double neighborLayerZBottom, double neighborSlopeX, double neighborSlopeY)
{
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(channelElements.find(element) != channelElements.end()))
    {
      CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborInitMessage: neighbor init message received at incorrect region.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborInitMessage: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(neighborZSurface >= neighborLayerZBottom))
    {
      CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborInitMessage: neighborZSurface must be greater than or equal to neighborLayerZBottom.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  it = channelElements[element].undergroundMeshNeighbors.begin();
  
  while (it != channelElements[element].undergroundMeshNeighbors.end() && (*it).neighbor != neighbor)
    {
      ++it;
    }
  
  if (it == channelElements[element].undergroundMeshNeighbors.end())
    {
      CkError("ERROR in Region::handleChannelGroundwaterMeshNeighborInitMessage, region %d, element %d, neighbor %d: neighbor not found.\n", thisIndex,
              element, neighbor);
      CkExit();
    }
  else
    {
      (*it).reciprocalNeighborProxy = reciprocalNeighborProxy;
      (*it).neighborZSurface        = neighborZSurface;
      (*it).neighborLayerZBottom    = neighborLayerZBottom;
      (*it).neighborZOffset         = calculateZOffset(neighbor, neighborVertexX, neighborVertexY, neighborX, neighborY, neighborZSurface, neighborSlopeX,
                                                       neighborSlopeY, element, channelElements[element].elementX, channelElements[element].elementY,
                                                       channelElements[element].elementZBank, channelElements[element].channelType);
      (*it).neighborInitialized     = true;
    }
}

bool Region::allNeighborsInitialized()
{
  bool allInitialized = true; // Stays true until we find one that is not initialized.
  
  // FIXLATER I could modify this to make it an incremental scan like allNominalFlowRatesCalculated.  It's only done once at initialization, not once per timestep
  // so the performance benefit would be less.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  for (itMesh = meshElements.begin(); allInitialized && itMesh != meshElements.end(); ++itMesh)
    {
      for (itMeshSurfacewaterMeshNeighbor = (*itMesh).second.meshNeighbors.begin();
           allInitialized && itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          allInitialized = (*itMeshSurfacewaterMeshNeighbor).neighborInitialized;
        }
      
      for (itMeshSurfacewaterChannelNeighbor = (*itMesh).second.channelNeighbors.begin();
           allInitialized && itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
        {
          allInitialized = (*itMeshSurfacewaterChannelNeighbor).neighborInitialized;
        }
      
      for (itMeshGroundwaterMeshNeighbor = (*itMesh).second.underground.meshNeighbors.begin();
           allInitialized && itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          allInitialized = (*itMeshGroundwaterMeshNeighbor).neighborInitialized;
        }
      
      for (itMeshGroundwaterChannelNeighbor = (*itMesh).second.underground.channelNeighbors.begin();
           allInitialized && itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
        {
          allInitialized = (*itMeshGroundwaterChannelNeighbor).neighborInitialized;
        }
    }
  
  for (itChannel = channelElements.begin(); allInitialized && itChannel != channelElements.end(); ++itChannel)
    {
      for (itChannelSurfacewaterMeshNeighbor = (*itChannel).second.meshNeighbors.begin();
           allInitialized && itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
        {
          allInitialized = (*itChannelSurfacewaterMeshNeighbor).neighborInitialized;
        }
      
      for (itChannelSurfacewaterChannelNeighbor = (*itChannel).second.channelNeighbors.begin();
           allInitialized && itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          allInitialized = (*itChannelSurfacewaterChannelNeighbor).neighborInitialized;
        }
      
      for (itChannelGroundwaterMeshNeighbor = (*itChannel).second.undergroundMeshNeighbors.begin();
           allInitialized && itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
        {
          allInitialized = (*itChannelGroundwaterMeshNeighbor).neighborInitialized;
        }
    }
  
  return allInitialized;
}

void Region::sendStateToExternalNeighbors()
{
  std::map<int, std::vector<RegionMessage> >::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // Loop over all neighbor proxies of all elements.  If the neighbor's region is me it is an internal neighbor, otherwise it is an external neighbor.  For
  // internal neighbors, we always calculate a new nominal flow rate each timestep because we can be sure the neighbors are synced up, and it is inexpensive
  // since it doesn't require any messages.  The new nominal flow rate will be calculated after all messages have gone out to better overlap computation and
  // communication.  Therefore, at this point all we do for internal neighbors is set the nominal flow rate to expired.  For external neighbors, if the nominal
  // flow rate has expired put the element's state in the list of state messages to send to the neighbor's region.
  for (itMesh = meshElements.begin(); itMesh != meshElements.end(); ++itMesh)
    {
      for (itMeshSurfacewaterMeshNeighbor  = (*itMesh).second.meshNeighbors.begin();
           itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          if ((*itMeshSurfacewaterMeshNeighbor).region == thisIndex || isBoundary((*itMeshSurfacewaterMeshNeighbor).neighbor))
            {
              (*itMeshSurfacewaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshSurfacewaterMeshNeighbor).region]
                               .push_back(RegionMessage(MESH_SURFACEWATER_MESH_NEIGHBOR, (*itMeshSurfacewaterMeshNeighbor).neighbor,
                                                        (*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy,
                                                        (*itMesh).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itMeshSurfacewaterChannelNeighbor  = (*itMesh).second.channelNeighbors.begin();
           itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
        {
          if ((*itMeshSurfacewaterChannelNeighbor).region == thisIndex)
            {
              (*itMeshSurfacewaterChannelNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshSurfacewaterChannelNeighbor).region]
                               .push_back(RegionMessage(CHANNEL_SURFACEWATER_MESH_NEIGHBOR, (*itMeshSurfacewaterChannelNeighbor).neighbor,
                                                        (*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy,
                                                        (*itMesh).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itMeshGroundwaterMeshNeighbor  = (*itMesh).second.underground.meshNeighbors.begin();
           itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          if ((*itMeshGroundwaterMeshNeighbor).region == thisIndex || isBoundary((*itMeshGroundwaterMeshNeighbor).neighbor))
            {
              (*itMeshGroundwaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshGroundwaterMeshNeighbor).region]
                               .push_back(RegionMessage(MESH_GROUNDWATER_MESH_NEIGHBOR, (*itMeshGroundwaterMeshNeighbor).neighbor,
                                                        (*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy,
                                                        (*itMesh).second.surfacewaterDepth, (*itMesh).second.underground.groundwaterHead,
                                                        SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
           itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
        {
          if ((*itMeshGroundwaterChannelNeighbor).region == thisIndex)
            {
              (*itMeshGroundwaterChannelNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshGroundwaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshGroundwaterChannelNeighbor).region]
                               .push_back(RegionMessage(CHANNEL_GROUNDWATER_MESH_NEIGHBOR, (*itMeshGroundwaterChannelNeighbor).neighbor,
                                                        (*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy,
                                                        (*itMesh).second.surfacewaterDepth, (*itMesh).second.underground.groundwaterHead,
                                                        SimpleNeighborProxy::MaterialTransfer()));
            }
        }
    }
  
  for (itChannel = channelElements.begin(); itChannel != channelElements.end(); ++itChannel)
    {
      for (itChannelSurfacewaterMeshNeighbor  = (*itChannel).second.meshNeighbors.begin();
           itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
        {
          if ((*itChannelSurfacewaterMeshNeighbor).region == thisIndex)
            {
              (*itChannelSurfacewaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itChannelSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelSurfacewaterMeshNeighbor).region]
                               .push_back(RegionMessage(MESH_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannelSurfacewaterMeshNeighbor).neighbor,
                                                        (*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy,
                                                        (*itChannel).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itChannelSurfacewaterChannelNeighbor  = (*itChannel).second.channelNeighbors.begin();
           itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          if ((*itChannelSurfacewaterChannelNeighbor).region == thisIndex || isBoundary((*itChannelSurfacewaterChannelNeighbor).neighbor))
            {
              (*itChannelSurfacewaterChannelNeighbor).expirationTime = currentTime;
            }
          else if ((*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelSurfacewaterChannelNeighbor).region]
                               .push_back(RegionMessage(CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannelSurfacewaterChannelNeighbor).neighbor,
                                                        (*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy,
                                                        (*itChannel).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itChannelGroundwaterMeshNeighbor  = (*itChannel).second.undergroundMeshNeighbors.begin();
           itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
        {
          if ((*itChannelGroundwaterMeshNeighbor).region == thisIndex)
            {
              (*itChannelGroundwaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itChannelGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelGroundwaterMeshNeighbor).region]
                               .push_back(RegionMessage(MESH_GROUNDWATER_CHANNEL_NEIGHBOR, (*itChannelGroundwaterMeshNeighbor).neighbor,
                                                        (*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy,
                                                        (*itChannel).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
    }
  
  // Send aggregated messages to other regions and clear outgoing message buffers.
  for (it = outgoingMessages.begin(); it != outgoingMessages.end(); ++it)
    {
      if (!(*it).second.empty())
        {
          thisProxy[(*it).first].sendStateMessages(currentTime, regionalDtLimit, (*it).second);
          (*it).second.clear();
        }
    }
  
  // Send myself a message to calculate nominal flow rates of internal neighbors.  This is implemented as a low priority message so that all other regions can
  // send outgoing messages before I calculate to better overlap computation and communication.
  thisProxy[thisIndex].calculateNominalFlowRatesForInternalNeighbors();
}

void Region::handleCalculateNominalFlowRatesForInternalNeighbors()
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  // Loop over all neighbor proxies of all elements.  If the neighbor's region is me it is an internal neighbor, otherwise it is an external neighbor.  For
  // internal neighbors, we always calculate a new nominal flow rate each timestep because we can be sure the neighbors are synced up, and it is inexpensive
  // since it doesn't require any messages.  In these loops we calculate the nominal flow rates for internal neighbors.
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      for (itMeshSurfacewaterMeshNeighbor  = (*itMesh).second.meshNeighbors.begin(); !error &&
           itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          if (isBoundary((*itMeshSurfacewaterMeshNeighbor).neighbor))
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The boundary condition must not be calculated yet.
              CkAssert((*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              error = (*itMesh).second.calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, regionalDtLimit,
                  itMeshSurfacewaterMeshNeighbor - (*itMesh).second.meshNeighbors.begin(), 0.0);
            }
          else if ((*itMeshSurfacewaterMeshNeighbor).region == thisIndex && (*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(meshElements[(*itMeshSurfacewaterMeshNeighbor).neighbor]
                                    .meshNeighbors[(*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, regionalDtLimit,
                  itMeshSurfacewaterMeshNeighbor - (*itMesh).second.meshNeighbors.begin(),
                  meshElements[(*itMeshSurfacewaterMeshNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  meshElements[(*itMeshSurfacewaterMeshNeighbor).neighbor]
                               .meshNeighbors[(*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy]
                                              .nominalFlowRate = -(*itMeshSurfacewaterMeshNeighbor).nominalFlowRate;
                  meshElements[(*itMeshSurfacewaterMeshNeighbor).neighbor]
                               .meshNeighbors[(*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy]
                                              .expirationTime = (*itMeshSurfacewaterMeshNeighbor).expirationTime;
                }
            }
        }
      
      for (itMeshSurfacewaterChannelNeighbor  = (*itMesh).second.channelNeighbors.begin(); !error &&
           itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
        {
          if ((*itMeshSurfacewaterChannelNeighbor).region == thisIndex && (*itMeshSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(channelElements[(*itMeshSurfacewaterChannelNeighbor).neighbor]
                                       .meshNeighbors[(*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, regionalDtLimit,
                  itMeshSurfacewaterChannelNeighbor - (*itMesh).second.channelNeighbors.begin(),
                  channelElements[(*itMeshSurfacewaterChannelNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  channelElements[(*itMeshSurfacewaterChannelNeighbor).neighbor]
                                  .meshNeighbors[(*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy]
                                                 .nominalFlowRate = -(*itMeshSurfacewaterChannelNeighbor).nominalFlowRate;
                  channelElements[(*itMeshSurfacewaterChannelNeighbor).neighbor]
                                  .meshNeighbors[(*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy]
                                                 .expirationTime = (*itMeshSurfacewaterChannelNeighbor).expirationTime;
                }
            }
        }
      
      for (itMeshGroundwaterMeshNeighbor  = (*itMesh).second.underground.meshNeighbors.begin(); !error &&
           itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          if (isBoundary((*itMeshGroundwaterMeshNeighbor).neighbor))
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The boundary condition must not be calculated yet.
              CkAssert((*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.underground.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, regionalDtLimit,
                  itMeshGroundwaterMeshNeighbor - (*itMesh).second.underground.meshNeighbors.begin(), (*itMesh).second.elementX, (*itMesh).second.elementY,
                  (*itMesh).second.elementZSurface, (*itMesh).second.elementArea, (*itMesh).second.surfacewaterDepth, 0.0,
                  (*itMeshGroundwaterMeshNeighbor).neighborZSurface);
            }
          else if ((*itMeshGroundwaterMeshNeighbor).region == thisIndex && (*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor]
                                    .underground.meshNeighbors[(*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.underground.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, regionalDtLimit,
                  itMeshGroundwaterMeshNeighbor - (*itMesh).second.underground.meshNeighbors.begin(), (*itMesh).second.elementX, (*itMesh).second.elementY,
                  (*itMesh).second.elementZSurface, (*itMesh).second.elementArea, (*itMesh).second.surfacewaterDepth,
                  meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor].surfacewaterDepth,
                  meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor].underground.groundwaterHead);

              if (!error)
                {
                  meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor]
                               .underground.meshNeighbors[(*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy]
                                                          .nominalFlowRate = -(*itMeshGroundwaterMeshNeighbor).nominalFlowRate;
                  meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor]
                               .underground.meshNeighbors[(*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy]
                                                          .expirationTime = (*itMeshGroundwaterMeshNeighbor).expirationTime;
                }
            }
        }
      
      for (itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin(); !error &&
           itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
        {
          if ((*itMeshGroundwaterChannelNeighbor).region == thisIndex && (*itMeshGroundwaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(channelElements[(*itMeshGroundwaterChannelNeighbor).neighbor]
                                       .undergroundMeshNeighbors[(*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.underground.calculateNominalFlowRateWithGroundwaterChannelNeighbor(currentTime, regionalDtLimit,
                  itMeshGroundwaterChannelNeighbor - (*itMesh).second.underground.channelNeighbors.begin(), (*itMesh).second.elementZSurface,
                  (*itMesh).second.surfacewaterDepth, channelElements[(*itMeshGroundwaterChannelNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  channelElements[(*itMeshGroundwaterChannelNeighbor).neighbor]
                                  .undergroundMeshNeighbors[(*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy]
                                                            .nominalFlowRate = -(*itMeshGroundwaterChannelNeighbor).nominalFlowRate;
                  channelElements[(*itMeshGroundwaterChannelNeighbor).neighbor]
                                  .undergroundMeshNeighbors[(*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy]
                                                            .expirationTime = (*itMeshGroundwaterChannelNeighbor).expirationTime;
                }
            }
        }
    }
  
  for (itChannel = channelElements.begin(); !error && itChannel != channelElements.end(); ++itChannel)
    {
      for (itChannelSurfacewaterMeshNeighbor  = (*itChannel).second.meshNeighbors.begin(); !error &&
           itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
        {
          if ((*itChannelSurfacewaterMeshNeighbor).region == thisIndex && (*itChannelSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(meshElements[(*itChannelSurfacewaterMeshNeighbor).neighbor]
                                    .channelNeighbors[(*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itChannel).second.calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, regionalDtLimit,
                  itChannelSurfacewaterMeshNeighbor - (*itChannel).second.meshNeighbors.begin(),
                  meshElements[(*itChannelSurfacewaterMeshNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  meshElements[(*itChannelSurfacewaterMeshNeighbor).neighbor]
                               .channelNeighbors[(*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy]
                                                 .nominalFlowRate = -(*itChannelSurfacewaterMeshNeighbor).nominalFlowRate;
                  meshElements[(*itChannelSurfacewaterMeshNeighbor).neighbor]
                               .channelNeighbors[(*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy]
                                                 .expirationTime = (*itChannelSurfacewaterMeshNeighbor).expirationTime;
                }
            }
        }
      
      for (itChannelSurfacewaterChannelNeighbor  = (*itChannel).second.channelNeighbors.begin(); !error &&
           itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          if (isBoundary((*itChannelSurfacewaterChannelNeighbor).neighbor))
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The boundary condition must not be calculated yet.
              CkAssert((*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itChannel).second.calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, regionalDtLimit,
                  itChannelSurfacewaterChannelNeighbor - (*itChannel).second.channelNeighbors.begin(), 0.0);
            }
          else if ((*itChannelSurfacewaterChannelNeighbor).region == thisIndex && (*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(channelElements[(*itChannelSurfacewaterChannelNeighbor).neighbor]
                                       .channelNeighbors[(*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itChannel).second.calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, regionalDtLimit,
                  itChannelSurfacewaterChannelNeighbor - (*itChannel).second.channelNeighbors.begin(),
                  channelElements[(*itChannelSurfacewaterChannelNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  channelElements[(*itChannelSurfacewaterChannelNeighbor).neighbor]
                                  .channelNeighbors[(*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy]
                                                    .nominalFlowRate = -(*itChannelSurfacewaterChannelNeighbor).nominalFlowRate;
                  channelElements[(*itChannelSurfacewaterChannelNeighbor).neighbor]
                                  .channelNeighbors[(*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy]
                                                    .expirationTime = (*itChannelSurfacewaterChannelNeighbor).expirationTime;
                }
            }
        }
      
      for (itChannelGroundwaterMeshNeighbor  = (*itChannel).second.undergroundMeshNeighbors.begin(); !error &&
           itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
        {
          if ((*itChannelGroundwaterMeshNeighbor).region == thisIndex && (*itChannelGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor]
                                    .underground.channelNeighbors[(*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itChannel).second.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, regionalDtLimit,
                  itChannelGroundwaterMeshNeighbor - (*itChannel).second.undergroundMeshNeighbors.begin(),
                  meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor].surfacewaterDepth,
                  meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor].underground.groundwaterHead);

              if (!error)
                {
                  meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor]
                               .underground.channelNeighbors[(*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy]
                                                             .nominalFlowRate = -(*itChannelGroundwaterMeshNeighbor).nominalFlowRate;
                  meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor]
                               .underground.channelNeighbors[(*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy]
                                                             .expirationTime = (*itChannelGroundwaterMeshNeighbor).expirationTime;
                }
            }
        }
    }
  
  if (!error)
    {
      // Set up iterators for the incremental scan that detects when all nominal flow rates are calculated because all state messages have arrived.
      pupItMeshAndItChannel = true;
      pupItNeighbor         = true;
      itMesh                = meshElements.begin();
      itChannel             = channelElements.begin();

      if (itMesh != meshElements.end())
        {
          itMeshSurfacewaterMeshNeighbor    = (*itMesh).second.meshNeighbors.begin();
          itMeshSurfacewaterChannelNeighbor = (*itMesh).second.channelNeighbors.begin();
          itMeshGroundwaterMeshNeighbor     = (*itMesh).second.underground.meshNeighbors.begin();
          itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
        }

      if (itChannel != channelElements.end())
        {
          itChannelSurfacewaterMeshNeighbor    = (*itChannel).second.meshNeighbors.begin();
          itChannelSurfacewaterChannelNeighbor = (*itChannel).second.channelNeighbors.begin();
          itChannelGroundwaterMeshNeighbor     = (*itChannel).second.undergroundMeshNeighbors.begin();
        }

      // The incremental scan also selects the next timestep end time as the minimum of nextSyncTime and all nominal flow rate expiration times.
      timestepEndTime = nextSyncTime;
    }
  
  if (error)
    {
      CkExit();
    }
}

bool Region::allNominalFlowRatesCalculated()
{
  bool allCalculated = true; // Stays true until we find one that is not calculated.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must be in use at this time.
  CkAssert(pupItMeshAndItChannel && pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  while (allCalculated && itMesh != meshElements.end())
    {
      while (allCalculated && itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end())
        {
          if ((*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itMeshSurfacewaterMeshNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itMeshSurfacewaterMeshNeighbor).expirationTime)
                {
                  timestepEndTime = (*itMeshSurfacewaterMeshNeighbor).expirationTime;
                }

              ++itMeshSurfacewaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end())
        {
          if ((*itMeshSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itMeshSurfacewaterChannelNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itMeshSurfacewaterChannelNeighbor).expirationTime)
                {
                  timestepEndTime = (*itMeshSurfacewaterChannelNeighbor).expirationTime;
                }
              
              ++itMeshSurfacewaterChannelNeighbor;
            }
        }
      
      while (allCalculated && itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end())
        {
          if ((*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itMeshGroundwaterMeshNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itMeshGroundwaterMeshNeighbor).expirationTime)
                {
                  timestepEndTime = (*itMeshGroundwaterMeshNeighbor).expirationTime;
                }
              
              ++itMeshGroundwaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end())
        {
          if ((*itMeshGroundwaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itMeshGroundwaterChannelNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itMeshGroundwaterChannelNeighbor).expirationTime)
                {
                  timestepEndTime = (*itMeshGroundwaterChannelNeighbor).expirationTime;
                }
              
              ++itMeshGroundwaterChannelNeighbor;
            }
        }
      
      if (allCalculated)
        {
          ++itMesh;
          
          if (itMesh != meshElements.end())
            {
              itMeshSurfacewaterMeshNeighbor    = (*itMesh).second.meshNeighbors.begin();
              itMeshSurfacewaterChannelNeighbor = (*itMesh).second.channelNeighbors.begin();
              itMeshGroundwaterMeshNeighbor     = (*itMesh).second.underground.meshNeighbors.begin();
              itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
            }
        }
    }
  
  while (allCalculated && itChannel != channelElements.end())
    {
      while (allCalculated && itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end())
        {
          if ((*itChannelSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itChannelSurfacewaterMeshNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itChannelSurfacewaterMeshNeighbor).expirationTime)
                {
                  timestepEndTime = (*itChannelSurfacewaterMeshNeighbor).expirationTime;
                }

              ++itChannelSurfacewaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end())
        {
          if ((*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itChannelSurfacewaterChannelNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itChannelSurfacewaterChannelNeighbor).expirationTime)
                {
                  timestepEndTime = (*itChannelSurfacewaterChannelNeighbor).expirationTime;
                }
              
              ++itChannelSurfacewaterChannelNeighbor;
            }
        }
      
      while (allCalculated && itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end())
        {
          if ((*itChannelGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itChannelGroundwaterMeshNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itChannelGroundwaterMeshNeighbor).expirationTime)
                {
                  timestepEndTime = (*itChannelGroundwaterMeshNeighbor).expirationTime;
                }
              
              ++itChannelGroundwaterMeshNeighbor;
            }
        }
      
      if (allCalculated)
        {
          ++itChannel;
          
          if (itChannel != channelElements.end())
            {
              itChannelSurfacewaterMeshNeighbor    = (*itChannel).second.meshNeighbors.begin();
              itChannelSurfacewaterChannelNeighbor = (*itChannel).second.channelNeighbors.begin();
              itChannelGroundwaterMeshNeighbor     = (*itChannel).second.undergroundMeshNeighbors.begin();
            }
        }
    }
  
  if (allCalculated)
    {
      pupItMeshAndItChannel = false;
      pupItNeighbor         = false;
    }
  
  return allCalculated;
}

void Region::processStateMessages(double senderCurrentTime, double senderRegionalDtLimit, std::vector<RegionMessage>& stateMessages)
{
  bool                                 error = false; // Error flag.
  std::vector<RegionMessage>::iterator it;            // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(senderCurrentTime >= currentTime))
    {
      CkError("ERROR in Region::processStateMessages, region %d: senderCurrentTime must be greater than or equal to currentTime.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < senderRegionalDtLimit))
    {
      CkError("ERROR in Region::processStateMessages, region %d: senderRegionalDtLimit must be greater than zero.\n", thisIndex);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  for (it = stateMessages.begin(); it != stateMessages.end(); ++it)
    {
      error = (*it).checkInvariant() || error;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  if (senderCurrentTime > currentTime)
    {
      // This is a message from the future, don't receive it yet.
      thisProxy[thisIndex].sendStateMessages(senderCurrentTime, senderRegionalDtLimit, stateMessages);
    }
  else
    {
      // Use the minimum of my or my neighbor's regionalDtLimit.
      if (senderRegionalDtLimit > regionalDtLimit)
        {
          senderRegionalDtLimit = regionalDtLimit;
        }
      
      // Calculate nominal flow rates.
      for (it = stateMessages.begin(); !error && it != stateMessages.end(); ++it)
        {
          switch ((*it).messageType)
          {
          case MESH_SURFACEWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].meshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case MESH_SURFACEWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case MESH_GROUNDWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].underground.meshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].underground.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime,
                senderRegionalDtLimit, (*it).recipientNeighborProxyIndex, meshElements[(*it).recipientElementNumber].elementX,
                meshElements[(*it).recipientElementNumber].elementY, meshElements[(*it).recipientElementNumber].elementZSurface,
                meshElements[(*it).recipientElementNumber].elementArea, meshElements[(*it).recipientElementNumber].surfacewaterDepth,
                (*it).senderSurfacewaterDepth, (*it).senderGroundwaterHead);
            break;
          case MESH_GROUNDWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].underground.channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].underground.calculateNominalFlowRateWithGroundwaterChannelNeighbor(currentTime,
                senderRegionalDtLimit, (*it).recipientNeighborProxyIndex, meshElements[(*it).recipientElementNumber].elementZSurface,
                meshElements[(*it).recipientElementNumber].surfacewaterDepth, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_SURFACEWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].meshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_GROUNDWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].undergroundMeshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth, (*it).senderGroundwaterHead);
            break;
          }
        }
    }
  
  if (error)
    {
      CkExit();
    }
}

void Region::doPointProcessesAndSendOutflows()
{
  bool                                                 error = false; // Error flag.
  std::map<int, std::vector<RegionMessage> >::iterator it;            // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      error = (*itMesh).second.doPointProcessesAndSendOutflows(referenceDate, currentTime, timestepEndTime, *this);
    }
  
  for (itChannel = channelElements.begin(); !error && itChannel != channelElements.end(); ++itChannel)
    {
      error = (*itChannel).second.doPointProcessesAndSendOutflows(referenceDate, currentTime, timestepEndTime, *this);
    }
  
  if (!error)
    {
      // Send aggregated messages to other regions and clear outgoing message buffers.
      for (it = outgoingMessages.begin(); it != outgoingMessages.end(); ++it)
        {
          if (!(*it).second.empty())
            {
              thisProxy[(*it).first].sendWaterMessages((*it).second);
              (*it).second.clear();
            }
        }
      
      // Set up iterators for the incremental scan that detects when all inflow water messages have arrived.
      pupItMeshAndItChannel = true;
      itMesh                = meshElements.begin();
      itChannel             = channelElements.begin();
    }
  
  if (error)
    {
      CkExit();
    }
}

bool Region::allInflowsArrived()
{
  bool allArrived = true; // Stays true until we find one that has not arrived.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Mesh and channel iterators must be in use at this time, neighbor iterators must not.
  CkAssert(pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  while (allArrived && itMesh != meshElements.end())
    {
      if (!(*itMesh).second.allInflowsArrived(currentTime, timestepEndTime))
        {
          allArrived = false;
        }
      else
        {
          ++itMesh;
        }
    }
  
  while (allArrived && itChannel != channelElements.end())
    {
      if (!(*itChannel).second.allInflowsArrived(currentTime, timestepEndTime))
        {
          allArrived = false;
        }
      else
        {
          ++itChannel;
        }
    }
  
  if (allArrived)
    {
      pupItMeshAndItChannel = false;
    }
  
  return allArrived;
}

bool Region::receiveWater(RegionMessage waterMessage)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  error = waterMessage.checkInvariant();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  if (!error)
    {
      switch (waterMessage.messageType)
      {
      case MESH_SURFACEWATER_MESH_NEIGHBOR:
        error = meshElements[waterMessage.recipientElementNumber].meshNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                .insertMaterial(waterMessage.water);
        break;
      case MESH_SURFACEWATER_CHANNEL_NEIGHBOR:
        error = meshElements[waterMessage.recipientElementNumber].channelNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                   .insertMaterial(waterMessage.water);
        break;
      case MESH_GROUNDWATER_MESH_NEIGHBOR:
        error = meshElements[waterMessage.recipientElementNumber].underground.meshNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                            .insertMaterial(waterMessage.water);
        break;
      case MESH_GROUNDWATER_CHANNEL_NEIGHBOR:
        error = meshElements[waterMessage.recipientElementNumber].underground.channelNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                               .insertMaterial(waterMessage.water);
        break;
      case CHANNEL_SURFACEWATER_MESH_NEIGHBOR:
        error = channelElements[waterMessage.recipientElementNumber].meshNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                   .insertMaterial(waterMessage.water);
        break;
      case CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR:
        error = channelElements[waterMessage.recipientElementNumber].channelNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                      .insertMaterial(waterMessage.water);
        break;
      case CHANNEL_GROUNDWATER_MESH_NEIGHBOR:
        error = channelElements[waterMessage.recipientElementNumber].undergroundMeshNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                              .insertMaterial(waterMessage.water);
        break;
      }
    }

  return error;
}

bool Region::sendWater(int recipientRegion, RegionMessage waterMessage)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= recipientRegion && recipientRegion < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in Region::sendWater: recipientRegion must be greater than or equal to zero and less than globalNumberOfRegions.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  error = waterMessage.checkInvariant() || error;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  if (!error)
    {
      if (recipientRegion == thisIndex)
        {
          error = receiveWater(waterMessage);
        }
      else
        {
          outgoingMessages[recipientRegion].push_back(waterMessage);
        }
    }

  return error;
}

void Region::processWaterMessages(std::vector<RegionMessage>& waterMessages)
{
  bool                                 error = false; // Error flag.
  std::vector<RegionMessage>::iterator it;            // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  for (it = waterMessages.begin(); it != waterMessages.end(); ++it)
    {
      error = (*it).checkInvariant() || error;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  // Place water in recipient's incoming material list.
  for (it = waterMessages.begin(); !error && it != waterMessages.end(); ++it)
    {
      error = receiveWater(*it);
    }

  if (error)
    {
      CkExit();
    }
}

void Region::receiveInflowsAndAdvanceTime()
{
  bool error = false;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      error = (*itMesh).second.receiveInflows(currentTime, timestepEndTime);
    }
  
  for (itChannel = channelElements.begin(); !error && itChannel != channelElements.end(); ++itChannel)
    {
      error = (*itChannel).second.receiveInflows(currentTime, timestepEndTime);
    }
  
  if (!error)
    {
      currentTime = timestepEndTime;
    }
  
  if (error)
    {
      CkExit();
    }
}

void Region::sendStateMessages()
{
  // FIXME these could become member variables like outgoingMessages.  The benefit of that would be that the vectors don't need to be constructed every time.
  std::map<int, std::vector<ElementStateMessage> >           meshElementStateMessages;    // Aggregator for messages going out to file managers.
                                                                                          // Key is PE number.
  std::map<int, std::vector<ElementStateMessage> >           channelElementStateMessages; // Aggregator for messages going out to file managers.
                                                                                          // Key is PE number.
  std::map<int, std::vector<ElementStateMessage> >::iterator it;                          // Loop iterator
  FileManager*                                               fileManagerLocalBranch = ADHydro::fileManagerProxy.ckLocalBranch();
                                                                                          // Used for access to local public member variables.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  for (itMesh = meshElements.begin(); itMesh != meshElements.end(); ++itMesh)
    {
      ElementStateMessage state((*itMesh).second.elementNumber, (*itMesh).second.surfacewaterDepth, (*itMesh).second.surfacewaterError,
                                (*itMesh).second.underground.groundwaterHead, (*itMesh).second.underground.groundwaterRecharge,
                                (*itMesh).second.underground.groundwaterError, (*itMesh).second.precipitationRate,
                                (*itMesh).second.precipitationCumulativeShortTerm, (*itMesh).second.precipitationCumulativeLongTerm,
                                (*itMesh).second.evaporationRate, (*itMesh).second.evaporationCumulativeShortTerm,
                                (*itMesh).second.evaporationCumulativeLongTerm, (*itMesh).second.transpirationRate,
                                (*itMesh).second.transpirationCumulativeShortTerm, (*itMesh).second.transpirationCumulativeLongTerm,
                                (*itMesh).second.evapoTranspirationState);
      
      for (itMeshSurfacewaterMeshNeighbor  = (*itMesh).second.meshNeighbors.begin();
           itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          state.surfacewaterMeshNeighbors.push_back(simpleNeighborInfo(
              (*itMeshSurfacewaterMeshNeighbor).expirationTime, (*itMeshSurfacewaterMeshNeighbor).nominalFlowRate,
              (*itMeshSurfacewaterMeshNeighbor).flowCumulativeShortTerm, (*itMeshSurfacewaterMeshNeighbor).flowCumulativeLongTerm, 0,
              (*itMeshSurfacewaterMeshNeighbor).neighbor, 1.0, 1.0, 0.0));
        }

      for (itMeshGroundwaterMeshNeighbor  = (*itMesh).second.underground.meshNeighbors.begin();
           itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          state.groundwaterMeshNeighbors.push_back(simpleNeighborInfo(
              (*itMeshGroundwaterMeshNeighbor).expirationTime, (*itMeshGroundwaterMeshNeighbor).nominalFlowRate,
              (*itMeshGroundwaterMeshNeighbor).flowCumulativeShortTerm, (*itMeshGroundwaterMeshNeighbor).flowCumulativeLongTerm, 0,
              (*itMeshGroundwaterMeshNeighbor).neighbor, 1.0, 1.0, 0.0));
        }

      for (itMeshSurfacewaterChannelNeighbor  = (*itMesh).second.channelNeighbors.begin();
           itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
        {
          state.surfacewaterChannelNeighbors.push_back(simpleNeighborInfo(
              (*itMeshSurfacewaterChannelNeighbor).expirationTime, (*itMeshSurfacewaterChannelNeighbor).nominalFlowRate,
              (*itMeshSurfacewaterChannelNeighbor).flowCumulativeShortTerm, (*itMeshSurfacewaterChannelNeighbor).flowCumulativeLongTerm, 0,
              (*itMeshSurfacewaterChannelNeighbor).neighbor, 1.0, 1.0, 0.0));
        }

      for (itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
           itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
        {
          state.groundwaterChannelNeighbors.push_back(simpleNeighborInfo(
              (*itMeshGroundwaterChannelNeighbor).expirationTime, (*itMeshGroundwaterChannelNeighbor).nominalFlowRate,
              (*itMeshGroundwaterChannelNeighbor).flowCumulativeShortTerm, (*itMeshGroundwaterChannelNeighbor).flowCumulativeLongTerm, 0,
              (*itMeshGroundwaterChannelNeighbor).neighbor, 1.0, 1.0, 0.0));
        }
      
      meshElementStateMessages[FileManager::home((*itMesh).second.elementNumber, fileManagerLocalBranch->globalNumberOfMeshElements)].push_back(state);
    }
  
  for (itChannel = channelElements.begin(); itChannel != channelElements.end(); ++itChannel)
    {
      ElementStateMessage state((*itChannel).second.elementNumber, (*itChannel).second.surfacewaterDepth, (*itChannel).second.surfacewaterError, 0.0, 0.0, 0.0,
                                (*itChannel).second.precipitationRate, (*itChannel).second.precipitationCumulativeShortTerm,
                                (*itChannel).second.precipitationCumulativeLongTerm, (*itChannel).second.evaporationRate,
                                (*itChannel).second.evaporationCumulativeShortTerm, (*itChannel).second.evaporationCumulativeLongTerm, 0.0, 0.0, 0.0,
                                (*itChannel).second.evapoTranspirationState);
      
      for (itChannelSurfacewaterMeshNeighbor  = (*itChannel).second.meshNeighbors.begin();
           itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
        {
          state.surfacewaterMeshNeighbors.push_back(simpleNeighborInfo(
              (*itChannelSurfacewaterMeshNeighbor).expirationTime, (*itChannelSurfacewaterMeshNeighbor).nominalFlowRate,
              (*itChannelSurfacewaterMeshNeighbor).flowCumulativeShortTerm, (*itChannelSurfacewaterMeshNeighbor).flowCumulativeLongTerm, 0,
              (*itChannelSurfacewaterMeshNeighbor).neighbor, 1.0, 1.0, 0.0));
        }

      for (itChannelGroundwaterMeshNeighbor  = (*itChannel).second.undergroundMeshNeighbors.begin();
           itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
        {
          state.groundwaterMeshNeighbors.push_back(simpleNeighborInfo(
              (*itChannelGroundwaterMeshNeighbor).expirationTime, (*itChannelGroundwaterMeshNeighbor).nominalFlowRate,
              (*itChannelGroundwaterMeshNeighbor).flowCumulativeShortTerm, (*itChannelGroundwaterMeshNeighbor).flowCumulativeLongTerm, 0,
              (*itChannelGroundwaterMeshNeighbor).neighbor, 1.0, 1.0, 0.0));
        }

      for (itChannelSurfacewaterChannelNeighbor  = (*itChannel).second.channelNeighbors.begin();
           itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          state.surfacewaterChannelNeighbors.push_back(simpleNeighborInfo(
              (*itChannelSurfacewaterChannelNeighbor).expirationTime, (*itChannelSurfacewaterChannelNeighbor).nominalFlowRate,
              (*itChannelSurfacewaterChannelNeighbor).flowCumulativeShortTerm, (*itChannelSurfacewaterChannelNeighbor).flowCumulativeLongTerm, 0,
              (*itChannelSurfacewaterChannelNeighbor).neighbor, 1.0, 1.0, 0.0));
        }
      
      channelElementStateMessages[FileManager::home((*itChannel).second.elementNumber, fileManagerLocalBranch->globalNumberOfChannelElements)].push_back(state);
    }
  
  // Send messages to file managers.  First loop over all of the PEs in the mesh messages map.
  for (it = meshElementStateMessages.begin(); it != meshElementStateMessages.end(); ++it)
    {
      if ((*it).second.size() > 0 || channelElementStateMessages[(*it).first].size() > 0)
        {
          ADHydro::fileManagerProxy[(*it).first].sendElementStateMessages(currentTime, (*it).second, channelElementStateMessages[(*it).first]);
        }
    }
  
  // Now find any PEs in the channel messages map that weren't in the mesh messages map.
  for (it = channelElementStateMessages.begin(); it != channelElementStateMessages.end(); ++it)
    {
      if ((*it).second.size() > 0 && meshElementStateMessages.end() == meshElementStateMessages.find((*it).first))
        {
          ADHydro::fileManagerProxy[(*it).first].sendElementStateMessages(currentTime, std::vector<ElementStateMessage>(), (*it).second);
        }
    }
}

bool Region::massBalance(double& waterInDomain, double& externalFlows, double& waterError)
{
  bool error = false; // Error flag.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= waterInDomain))
    {
      CkError("ERROR in Region::massBalance: waterInDomain must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Iterators must not be in use at this time.
  CkAssert(!pupItMeshAndItChannel && !pupItNeighbor);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      error = (*itMesh).second.massBalance(waterInDomain, externalFlows, waterError);
    }
  
  for (itChannel = channelElements.begin(); !error && itChannel != channelElements.end(); ++itChannel)
    {
      error = (*itChannel).second.massBalance(waterInDomain, externalFlows, waterError);
    }

  return error;
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "region.def.h"
#pragma GCC diagnostic warning "-Wsign-compare"
