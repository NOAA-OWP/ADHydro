#include "region.h"

void Region::initiateNominalFlowRateCalculation()
{
  std::map<int, std::vector<RegionMessageStruct> >::iterator it; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  // Must be in the NEED_TO_CALCULATE_EXTERNAL_NOMINAL_FLOW_RATES state.
  CkAssert(NEED_TO_CALCULATE_EXTERNAL_NOMINAL_FLOW_RATES == stateMachineState);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  
  // Loop over all neighbor proxies of all elements.  If the neighbor's region is not me it is an external neighbor.  If the flow rate has expired put the
  // element's state in the list of state messages to send to the neighbor's region.
  for (itMesh = meshElements.begin(); itMesh != meshElements.end(); ++itMesh)
    {
      for (itMeshSurfacewaterMeshNeighbor  = (*itMesh).second.meshNeighbors.begin();
           itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          if ((*itMeshSurfacewaterMeshNeighbor).region != thisIndex && (*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshSurfacewaterMeshNeighbor).region].push_back(
                  RegionMessageStruct(MESH_SURFACEWATER_MESH_NEIGHBOR, (*itMeshSurfacewaterMeshNeighbor).neighbor,
                                      (*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy, (*itMesh).second.surfacewaterDepth, 0.0));
            }
        }
      
      for (itMeshSurfacewaterChannelNeighbor  = (*itMesh).second.channelNeighbors.begin();
           itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
        {
          if ((*itMeshSurfacewaterChannelNeighbor).region != thisIndex && (*itMeshSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshSurfacewaterChannelNeighbor).region].push_back(
                  RegionMessageStruct(CHANNEL_SURFACEWATER_MESH_NEIGHBOR, (*itMeshSurfacewaterChannelNeighbor).neighbor,
                                      (*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy, (*itMesh).second.surfacewaterDepth, 0.0));
            }
        }
      
      for (itMeshGroundwaterMeshNeighbor  = (*itMesh).second.underground.meshNeighbors.begin();
           itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          if ((*itMeshGroundwaterMeshNeighbor).region != thisIndex && (*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshGroundwaterMeshNeighbor).region].push_back(
                  RegionMessageStruct(MESH_GROUNDWATER_MESH_NEIGHBOR, (*itMeshGroundwaterMeshNeighbor).neighbor,
                                      (*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy, (*itMesh).second.surfacewaterDepth,
                                      (*itMesh).second.underground.groundwaterHead));
            }
        }
      
      for (itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
           itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
        {
          if ((*itMeshGroundwaterChannelNeighbor).region != thisIndex && (*itMeshGroundwaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshGroundwaterChannelNeighbor).region].push_back(
                  RegionMessageStruct(CHANNEL_GROUNDWATER_MESH_NEIGHBOR, (*itMeshGroundwaterChannelNeighbor).neighbor,
                                      (*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy, (*itMesh).second.surfacewaterDepth,
                                      (*itMesh).second.underground.groundwaterHead));
            }
        }
    }
  
  for (itChannel = channelElements.begin(); itChannel != channelElements.end(); ++itChannel)
    {
      for (itChannelSurfacewaterMeshNeighbor  = (*itChannel).second.meshNeighbors.begin();
           itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
        {
          if ((*itChannelSurfacewaterMeshNeighbor).region != thisIndex && (*itChannelSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelSurfacewaterMeshNeighbor).region].push_back(
                  RegionMessageStruct(MESH_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannelSurfacewaterMeshNeighbor).neighbor,
                                      (*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy, (*itChannel).second.surfacewaterDepth, 0.0));
            }
        }
      
      for (itChannelSurfacewaterChannelNeighbor  = (*itChannel).second.channelNeighbors.begin();
           itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          if ((*itChannelSurfacewaterChannelNeighbor).region != thisIndex && (*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelSurfacewaterChannelNeighbor).region].push_back(
                  RegionMessageStruct(CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannelSurfacewaterChannelNeighbor).neighbor,
                                      (*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy, (*itChannel).second.surfacewaterDepth, 0.0));
            }
        }
      
      for (itChannelGroundwaterMeshNeighbor  = (*itChannel).second.undergroundMeshNeighbors.begin();
           itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
        {
          if ((*itChannelGroundwaterMeshNeighbor).region != thisIndex && (*itChannelGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelGroundwaterMeshNeighbor).region].push_back(
                  RegionMessageStruct(MESH_GROUNDWATER_CHANNEL_NEIGHBOR, (*itChannelGroundwaterMeshNeighbor).neighbor,
                                      (*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy, (*itChannel).second.surfacewaterDepth, 0.0));
            }
        }
    }
  
  // Send aggregated messages to other regions and clear outgoing message buffers.
  for (it = outgoingMessages.begin(); it != outgoingMessages.end(); ++it)
    {
      thisProxy[(*it).first].sendStateMessages(currentTime, (*it).second);
      (*it).second.clear();
    }
  
  // Set up iterators for the incremental scan that detects when all external nominal flow rates are calculated because all state messages have arrived.
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
  
  // The incremental scan also finds the minimum of nextSyncTime and all external nominal flow rate expiration times for use in selecting the next timestep.
  minimumExternalExpirationTime = nextSyncTime;
  
  // Scan now just in case they are already all calculated.  I think the only way that can happen is if the region has no external neighbors, but that could
  // happen.
  checkIfAllExternalNominalFlowRatesAreCalculated();
}

void Region::checkIfAllExternalNominalFlowRatesAreCalculated()
{
  bool allCalculated = true; // Stays true until we find one that is not calculated.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  // Must be in the NEED_TO_CALCULATE_EXTERNAL_NOMINAL_FLOW_RATES state.
  CkAssert(NEED_TO_CALCULATE_EXTERNAL_NOMINAL_FLOW_RATES == stateMachineState);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  
  while (allCalculated && itMesh != meshElements.end())
    {
      while (allCalculated && itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end())
        {
          if ((*itMeshSurfacewaterMeshNeighbor).region != thisIndex && (*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              allCalculated = false;
            }
          else
            {
              if (minimumExternalExpirationTime > (*itMeshSurfacewaterMeshNeighbor).expirationTime)
                {
                  minimumExternalExpirationTime = (*itMeshSurfacewaterMeshNeighbor).expirationTime;
                }

              ++itMeshSurfacewaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end())
        {
          if ((*itMeshSurfacewaterChannelNeighbor).region != thisIndex && (*itMeshSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              allCalculated = false;
            }
          else
            {
              if (minimumExternalExpirationTime > (*itMeshSurfacewaterChannelNeighbor).expirationTime)
                {
                  minimumExternalExpirationTime = (*itMeshSurfacewaterChannelNeighbor).expirationTime;
                }
              
              ++itMeshSurfacewaterChannelNeighbor;
            }
        }
      
      while (allCalculated && itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end())
        {
          if ((*itMeshGroundwaterMeshNeighbor).region != thisIndex && (*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              allCalculated = false;
            }
          else
            {
              if (minimumExternalExpirationTime > (*itMeshGroundwaterMeshNeighbor).expirationTime)
                {
                  minimumExternalExpirationTime = (*itMeshGroundwaterMeshNeighbor).expirationTime;
                }
              
              ++itMeshGroundwaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end())
        {
          if ((*itMeshGroundwaterChannelNeighbor).region != thisIndex && (*itMeshGroundwaterChannelNeighbor).expirationTime == currentTime)
            {
              allCalculated = false;
            }
          else
            {
              if (minimumExternalExpirationTime > (*itMeshGroundwaterChannelNeighbor).expirationTime)
                {
                  minimumExternalExpirationTime = (*itMeshGroundwaterChannelNeighbor).expirationTime;
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
          if ((*itChannelSurfacewaterMeshNeighbor).region != thisIndex && (*itChannelSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              allCalculated = false;
            }
          else
            {
              if (minimumExternalExpirationTime > (*itChannelSurfacewaterMeshNeighbor).expirationTime)
                {
                  minimumExternalExpirationTime = (*itChannelSurfacewaterMeshNeighbor).expirationTime;
                }

              ++itChannelSurfacewaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end())
        {
          if ((*itChannelSurfacewaterChannelNeighbor).region != thisIndex && (*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              allCalculated = false;
            }
          else
            {
              if (minimumExternalExpirationTime > (*itChannelSurfacewaterChannelNeighbor).expirationTime)
                {
                  minimumExternalExpirationTime = (*itChannelSurfacewaterChannelNeighbor).expirationTime;
                }
              
              ++itChannelSurfacewaterChannelNeighbor;
            }
        }
      
      while (allCalculated && itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end())
        {
          if ((*itChannelGroundwaterMeshNeighbor).region != thisIndex && (*itChannelGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              allCalculated = false;
            }
          else
            {
              if (minimumExternalExpirationTime > (*itChannelGroundwaterMeshNeighbor).expirationTime)
                {
                  minimumExternalExpirationTime = (*itChannelGroundwaterMeshNeighbor).expirationTime;
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
      stateMachineState = NEED_TO_RECEIVE_INFLOWS;
    }
}

void Region::processStateMessages(double senderCurrentTime, std::vector<RegionMessageStruct>& stateMessages)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(senderCurrentTime >= currentTime))
    {
      CkError("ERROR in Region::processStateMessages, region %d: senderCurrentTime must be greater than or equal to currentTime.\n", thisIndex);
      CkExit();
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
      // FIXME calculate nominal flow rates.
      
      checkIfAllExternalNominalFlowRatesAreCalculated();
    }
}

#include "region.def.h"
