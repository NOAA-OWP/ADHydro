#include "region.h"

void Region::sendStateToExternalNeighbors()
{
  std::map<int, std::vector<RegionMessageStruct> >::iterator it; // Loop iterator.
  
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
          if ((*itMeshSurfacewaterMeshNeighbor).region == thisIndex)
            {
              (*itMeshSurfacewaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshSurfacewaterMeshNeighbor).region]
                               .push_back(RegionMessageStruct(MESH_SURFACEWATER_MESH_NEIGHBOR, (*itMeshSurfacewaterMeshNeighbor).neighbor,
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
                               .push_back(RegionMessageStruct(CHANNEL_SURFACEWATER_MESH_NEIGHBOR, (*itMeshSurfacewaterChannelNeighbor).neighbor,
                                                              (*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy,
                                                              (*itMesh).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itMeshGroundwaterMeshNeighbor  = (*itMesh).second.underground.meshNeighbors.begin();
           itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          if ((*itMeshGroundwaterMeshNeighbor).region == thisIndex)
            {
              (*itMeshGroundwaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshGroundwaterMeshNeighbor).region]
                               .push_back(RegionMessageStruct(MESH_GROUNDWATER_MESH_NEIGHBOR, (*itMeshGroundwaterMeshNeighbor).neighbor,
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
                               .push_back(RegionMessageStruct(CHANNEL_GROUNDWATER_MESH_NEIGHBOR, (*itMeshGroundwaterChannelNeighbor).neighbor,
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
                               .push_back(RegionMessageStruct(MESH_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannelSurfacewaterMeshNeighbor).neighbor,
                                                              (*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy,
                                                              (*itChannel).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itChannelSurfacewaterChannelNeighbor  = (*itChannel).second.channelNeighbors.begin();
           itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          if ((*itChannelSurfacewaterChannelNeighbor).region == thisIndex)
            {
              (*itChannelSurfacewaterChannelNeighbor).expirationTime = currentTime;
            }
          else if ((*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelSurfacewaterChannelNeighbor).region]
                               .push_back(RegionMessageStruct(CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannelSurfacewaterChannelNeighbor).neighbor,
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
                               .push_back(RegionMessageStruct(MESH_GROUNDWATER_CHANNEL_NEIGHBOR, (*itChannelGroundwaterMeshNeighbor).neighbor,
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
          thisProxy[(*it).first].sendStateMessages(currentTime, (*it).second);
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
  
  // Loop over all neighbor proxies of all elements.  If the neighbor's region is me it is an internal neighbor, otherwise it is an external neighbor.  For
  // internal neighbors, we always calculate a new nominal flow rate each timestep because we can be sure the neighbors are synced up, and it is inexpensive
  // since it doesn't require any messages.  In these loops we calculate the nominal flow rates for internal neighbors.
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      for (itMeshSurfacewaterMeshNeighbor  = (*itMesh).second.meshNeighbors.begin(); !error &&
           itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          if ((*itMeshSurfacewaterMeshNeighbor).region == thisIndex && (*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
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
          if ((*itMeshGroundwaterMeshNeighbor).region == thisIndex && (*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
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
          if ((*itChannelSurfacewaterChannelNeighbor).region == thisIndex && (*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
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
      itMesh    = meshElements.begin();
      itChannel = channelElements.begin();

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
  
  return allCalculated;
}

void Region::processStateMessages(double senderCurrentTime, std::vector<RegionMessageStruct>& stateMessages)
{
  bool                                       error = false; // Error flag.
  std::vector<RegionMessageStruct>::iterator it;            // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(senderCurrentTime >= currentTime))
    {
      CkError("ERROR in Region::processStateMessages, region %d: senderCurrentTime must be greater than or equal to currentTime.\n", thisIndex);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  // FIXME check RegionMessageStruct invariant.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  if (senderCurrentTime > currentTime)
    {
      // This is a message from the future, don't receive it yet.
      thisProxy[thisIndex].sendStateMessages(senderCurrentTime, stateMessages);
    }
  else
    {
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

            error = meshElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, regionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case MESH_SURFACEWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, regionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case MESH_GROUNDWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].underground.meshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].underground.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, regionalDtLimit,
                (*it).recipientNeighborProxyIndex, meshElements[(*it).recipientElementNumber].elementX, meshElements[(*it).recipientElementNumber].elementY,
                meshElements[(*it).recipientElementNumber].elementZSurface, meshElements[(*it).recipientElementNumber].elementArea,
                meshElements[(*it).recipientElementNumber].surfacewaterDepth, (*it).senderSurfacewaterDepth, (*it).senderGroundwaterHead);
            break;
          case MESH_GROUNDWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].underground.channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].underground.calculateNominalFlowRateWithGroundwaterChannelNeighbor(currentTime, regionalDtLimit,
                (*it).recipientNeighborProxyIndex, meshElements[(*it).recipientElementNumber].elementZSurface,
                meshElements[(*it).recipientElementNumber].surfacewaterDepth, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_SURFACEWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].meshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, regionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, regionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_GROUNDWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].undergroundMeshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, regionalDtLimit,
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
  bool                                                       error = false; // Error flag.
  std::map<int, std::vector<RegionMessageStruct> >::iterator it;            // Loop iterator.
  
  
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
      itMesh    = meshElements.begin();
      itChannel = channelElements.begin();

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
    }
  
  if (error)
    {
      CkExit();
    }
}

#include "region.def.h"
