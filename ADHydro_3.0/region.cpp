#include "region.h"
#include "adhydro.h"

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "region.def.h"
#pragma GCC diagnostic warning "-Wsign-compare"

bool Region::checkInvariant() const
{
    bool                                             error = false; // Error flag.
    std::map<size_t,    MeshElement>::const_iterator itMesh;        // Loop iterator.
    std::map<size_t, ChannelElement>::const_iterator itChannel;     // Loop iterator.
    
    if (!(currentTime <= timestepEndTime && timestepEndTime <= Readonly::getCheckpointTime(nextCheckpointIndex))) // FIXME timestepEndTime <= next forcing time
    {
        CkError("ERROR in Region::checkInvariant, region %lu: currentTime must be less than or equal to timestepEndTime, which must be less than or equal to the next checkpoint time.\n", thisIndex);
        error = true;
    }
    
    for (itMesh = meshElements.begin(); itMesh != meshElements.end(); ++itMesh)
    {
        if (!(itMesh->second.getElementNumber() == itMesh->first))
        {
            CkError("ERROR in Region::checkInvariant, region %lu: mesh element %lu stored at map key %lu.\n", thisIndex, itMesh->second.getElementNumber(), itMesh->first);
            error = true;
        }
        
        error = itMesh->second.checkInvariant() || error;
    }
    
    for (itChannel = channelElements.begin(); itChannel != channelElements.end(); ++itChannel)
    {
        if (!(itChannel->second.getElementNumber() == itChannel->first))
        {
            CkError("ERROR in Region::checkInvariant, region %lu: channel element %lu stored at map key %lu.\n", thisIndex, itChannel->second.getElementNumber(), itChannel->first);
            error = true;
        }
        
        error = itChannel->second.checkInvariant() || error;
    }
    
    if (!(meshElements.size() + channelElements.size() >= elementsFinished))
    {
        CkError("ERROR in Region::checkInvariant, region %lu: elementsFinished must be less than or equal to meshElements.size() plus channelElements.size().\n", thisIndex);
        error = true;
    }
    
    return error;
}

void Region::receiveMessage(Message& message)
{
    std::map<size_t,    MeshElement>::iterator itMesh;    // Iterator for finding correct MeshElement.
    std::map<size_t, ChannelElement>::iterator itChannel; // Iterator for finding correct ChannelElement.
    
    // Don't error check parameter because it's a simple pass-through to MeshElement::receiveMessage or ChannelElement::receiveMessage and it will be checked inside that method.
    
    // Now that the message has arrived at its destination the old remote neighbor is now the local neighbor and the old local neighbor is now the remote neighbor.
    message.destination.reverse();
    
    // Pass the Message to the appropriate element.
    switch (message.destination.localEndpoint)
    {
        case MESH_SURFACE:
        case MESH_SOIL:
        case MESH_AQUIFER:
        case IRRIGATION_RECIPIENT:
            itMesh = meshElements.find(message.destination.localElementNumber);
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
            {
                if (!(meshElements.end() != itMesh))
                {
                    CkError("ERROR in Region::receiveMessage, Region %lu: trying to find mesh element %lu that I do not have.\n", thisIndex, message.destination.localElementNumber);
                    CkExit();
                }
            }
            
            if (itMesh->second.receiveMessage(message, elementsFinished, currentTime, timestepEndTime))
            {
                CkExit();
            }
            break;
        case CHANNEL_SURFACE:
        case RESERVOIR_RELEASE:
        case RESERVOIR_RECIPIENT:
        case IRRIGATION_DIVERSION:
            itChannel = channelElements.find(message.destination.localElementNumber);
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
            {
                if (!(channelElements.end() != itChannel))
                {
                    CkError("ERROR in Region::receiveMessage, Region %lu: trying to find channel element %lu that I do not have.\n", thisIndex, message.destination.localElementNumber);
                    CkExit();
                }
            }
            
            if (itChannel->second.receiveMessage(message, elementsFinished, currentTime, timestepEndTime))
            {
                CkExit();
            }
            break;
        default:
            if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
            {
                CkError("ERROR in Region::receiveMessage, Region %lu: invalid localEndpoint %d.\n", thisIndex, message.destination.localEndpoint);
                CkExit();
            }
            break;
    }
}
