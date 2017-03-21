#include "region.h"

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "region.def.h"
#pragma GCC diagnostic warning "-Wsign-compare"

bool Region::checkInvariant() const
{
    bool                                             error = false; // Error flag.
    std::map<size_t,    MeshElement>::const_iterator itMesh;        // Loop iterator.
    std::map<size_t, ChannelElement>::const_iterator itChannel;     // Loop iterator.
    
    if (!(currentTime <= timestepEndTime && timestepEndTime <= simulationEndTime)) // FIXME timestepEndTime <= nextSyncTime <= simulationEndTime
    {
        CkError("ERROR in Region::checkInvariant: currentTime must be less than or equal to timestepEndTime, which must be less than or equal to simulationEndTime.\n");
        error = true;
    }
    
    for (itMesh = meshElements.begin(); itMesh != meshElements.end(); ++itMesh)
    {
        error = itMesh->second.checkInvariant() || error;
    }
    
    for (itChannel = channelElements.begin(); itChannel != channelElements.end(); ++itChannel)
    {
        error = itChannel->second.checkInvariant() || error;
    }
    
    return error;
}

bool Region::allNominalFlowRatesCalculated()
{
    bool   calculated;                           // Return value flag.
    double oldTimestepEndTime = timestepEndTime; // Temporary for putting the value of timestepEndTime back to prevent side effect.
    
    // FIXME Currently this is implemented by calling selectTimestep and checking if timestepEndTime is greater than currentTime.
    // This could be done more efficiently by having counters that get incremented each time a flow rate is calculated rather than iterating over all NeighborProxies each time.
    // Calling selectTimestep also has a side effect of modifying timestepEndTime.  We put the old value of timestepEndTime back at the end to avoid this side effect.
    selectTimestep();
    calculated      = (currentTime < timestepEndTime);
    timestepEndTime = oldTimestepEndTime;
    
    return calculated;
}

void Region::receiveState(std::vector<StateMessage>& messages)
{
    std::vector<StateMessage>::iterator        it;        // Loop iterator.
    std::map<size_t,    MeshElement>::iterator itMesh;    // Iterator for finding correct MeshElement.
    std::map<size_t, ChannelElement>::iterator itChannel; // Iterator for finding correct ChannelElement.
    
    // Don't error check parameters because it's a simple pass-through to MeshElement::receiveState or ChannelElement::receiveState and it will be checked inside that method.
    
    // Loop over messages passing the StateMessage objects to elements.
    for (it = messages.begin(); it != messages.end(); ++it)
    {
        // Now that the message has arrived at its destination the old remote neighbor is now the local neighbor and the old local neighbor is now the remote neighbor.
        it->destination.reverse();
        
        // Pass the StateMessage to the appropriate element.
        switch (it->destination.localEndpoint)
        {
            case MESH_SURFACE:
            case MESH_SOIL:
            case MESH_AQUIFER:
                itMesh = meshElements.find(it->destination.localElementNumber);
                
                if (meshElements.end() != itMesh)
                {
                    if (itMesh->second.receiveState(*it))
                    {
                        CkExit();
                    }
                }
                else if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
                {
                    CkError("ERROR in Region::receiveState: received a state message for mesh element %lu that I do not have.\n", it->destination.localElementNumber);
                    CkExit();
                }
                break;
            case CHANNEL_SURFACE:
                itChannel = channelElements.find(it->destination.localElementNumber);
                
                if (channelElements.end() != itChannel)
                {
                    if (itChannel->second.receiveState(*it))
                    {
                        CkExit();
                    }
                }
                else if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
                {
                    CkError("ERROR in Region::receiveState: received a state message for channel element %lu that I do not have.\n", it->destination.localElementNumber);
                    CkExit();
                }
                break;
            default:
                if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
                {
                    CkError("ERROR in Region::receiveState: invalid localEndpoint %d.\n", it->destination.localEndpoint);
                    CkExit();
                }
                break;
        }
    }
}

void Region::selectTimestep()
{
    std::map<size_t,    MeshElement>::iterator itMesh;    // Loop iterator.
    std::map<size_t, ChannelElement>::iterator itChannel; // Loop iterator.
    
    // Initially set timestepEndTime to be a time that we know we cannot exceed, either the next forcing read, checkpoint write, or simulationEndTime.
    timestepEndTime = simulationEndTime; // FIXME forcing read or checkpoint write.
    
    // Set timestepEndTime to be no later than any expirationTime.
    for (itMesh = meshElements.begin(); itMesh != meshElements.end(); ++itMesh)
    {
        timestepEndTime = std::min(timestepEndTime, itMesh->second.minimumExpirationTime());
    }
    
    for (itChannel = channelElements.begin(); itChannel != channelElements.end(); ++itChannel)
    {
        timestepEndTime = std::min(timestepEndTime, itChannel->second.minimumExpirationTime());
    }
}

bool Region::allInflowsHaveArrived()
{
    bool                                       arrived = true; // Return value flag will be set to false when one is found that has not arrived.
    std::map<size_t,    MeshElement>::iterator itMesh;         // Loop iterator.
    std::map<size_t, ChannelElement>::iterator itChannel;      // Loop iterator.
    
    // This could be done more efficiently by having counters that get incremented each time a NeighborProxy gets all of its water rather than iterating over all NeighborProxies each time.
    
    for (itMesh = meshElements.begin(); arrived && itMesh != meshElements.end(); ++itMesh)
    {
        arrived = itMesh->second.allInflowsHaveArrived(currentTime, timestepEndTime);
    }
    
    for (itChannel = channelElements.begin(); arrived && itChannel != channelElements.end(); ++itChannel)
    {
        arrived = itChannel->second.allInflowsHaveArrived(currentTime, timestepEndTime);
    }
    
    return arrived;
}

void Region::receiveWater(std::vector<WaterMessage>& messages)
{
    std::vector<WaterMessage>::iterator        it;        // Loop iterator.
    std::map<size_t,    MeshElement>::iterator itMesh;    // Iterator for finding correct MeshElement.
    std::map<size_t, ChannelElement>::iterator itChannel; // Iterator for finding correct ChannelElement.
    
    // Don't error check parameters because it's a simple pass-through to MeshElement::receiveWater or ChannelElement::receiveWater and it will be checked inside that method.
    
    // Loop over messages passing the WaterMessage objects to elements.
    for (it = messages.begin(); it != messages.end(); ++it)
    {
        // Now that the message has arrived at its destination the old remote neighbor is now the local neighbor and the old local neighbor is now the remote neighbor.
        it->destination.reverse();
        
        // Pass the WaterMessage to the appropriate element.
        switch (it->destination.localEndpoint)
        {
            case MESH_SURFACE:
            case MESH_SOIL:
            case MESH_AQUIFER:
                itMesh = meshElements.find(it->destination.localElementNumber);
                
                if (meshElements.end() != itMesh)
                {
                    if (itMesh->second.receiveWater(*it))
                    {
                        CkExit();
                    }
                }
                else if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
                {
                    CkError("ERROR in Region::receiveWater: received a water message for mesh element %lu that I do not have.\n", it->destination.localElementNumber);
                    CkExit();
                }
                break;
            case CHANNEL_SURFACE:
                itChannel = channelElements.find(it->destination.localElementNumber);
                
                if (channelElements.end() != itChannel)
                {
                    if (itChannel->second.receiveWater(*it))
                    {
                        CkExit();
                    }
                }
                else if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
                {
                    CkError("ERROR in Region::receiveWater: received a water message for channel element %lu that I do not have.\n", it->destination.localElementNumber);
                    CkExit();
                }
                break;
            default:
                if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
                {
                    CkError("ERROR in Region::receiveWater: invalid localEndpoint %d.\n", it->destination.localEndpoint);
                    CkExit();
                }
                break;
        }
    }
}
