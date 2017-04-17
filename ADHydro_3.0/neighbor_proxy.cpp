#include "neighbor_proxy.h"
#include "readonly.h"

bool NeighborConnection::checkInvariant() const
{
    bool error = false; // Error flag.
    
    switch (localEndpoint)
    {
        case MESH_SURFACE:
        case MESH_SOIL:
        case MESH_AQUIFER:
            if (!(localElementNumber < Readonly::globalNumberOfMeshElements))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: localElementNumber must be less than Readonly::globalNumberOfMeshElements.\n");
                error = true;
            }
            
            switch (remoteEndpoint)
            {
                case MESH_SURFACE:
                case MESH_SOIL:
                case MESH_AQUIFER:
                    if (!(remoteElementNumber < Readonly::globalNumberOfMeshElements))
                    {
                        CkError("ERROR in NeighborConnection::checkInvariant: remoteElementNumber must be less than Readonly::globalNumberOfMeshElements.\n");
                        error = true;
                    }
                    
                    if (!(localElementNumber != remoteElementNumber))
                    {
                        CkError("ERROR in NeighborConnection::checkInvariant: invalid self-neighbor in mesh element %d.\n", localElementNumber);
                        error = true;
                    }
                    break;
                case CHANNEL_SURFACE:
                    if (!(remoteElementNumber < Readonly::globalNumberOfChannelElements))
                    {
                        CkError("ERROR in NeighborConnection::checkInvariant: remoteElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                        error = true;
                    }
                    break;
                case BOUNDARY_INFLOW:
                case BOUNDARY_OUTFLOW:
                case TRANSBASIN_INFLOW:
                case TRANSBASIN_OUTFLOW:
                    // These are valid and have no other conditions.
                    // FIXME do we want to use remoteElementNumber for something like multiple accounting buckets?
                    break;
                default:
                    CkError("ERROR in NeighborConnection::checkInvariant: invalid remoteEndpoint %d.\n", remoteEndpoint);
                    error = true;
                    break;
            }
            break;
        case CHANNEL_SURFACE:
            if (!(localElementNumber < Readonly::globalNumberOfChannelElements))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: localElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                error = true;
            }
            
            switch (remoteEndpoint)
            {
                case MESH_SURFACE:
                case MESH_SOIL:
                case MESH_AQUIFER:
                    if (!(remoteElementNumber < Readonly::globalNumberOfMeshElements))
                    {
                        CkError("ERROR in NeighborConnection::checkInvariant: remoteElementNumber must be less than Readonly::globalNumberOfMeshElements.\n");
                        error = true;
                    }
                    break;
                case CHANNEL_SURFACE:
                    if (!(remoteElementNumber < Readonly::globalNumberOfChannelElements))
                    {
                        CkError("ERROR in NeighborConnection::checkInvariant: remoteElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                        error = true;
                    }
                    
                    if (!(localElementNumber != remoteElementNumber))
                    {
                        CkError("ERROR in NeighborConnection::checkInvariant: invalid self-neighbor in channel element %d.\n", localElementNumber);
                        error = true;
                    }
                    break;
                case BOUNDARY_INFLOW:
                case BOUNDARY_OUTFLOW:
                case TRANSBASIN_INFLOW:
                case TRANSBASIN_OUTFLOW:
                    // These are valid and have no other conditions.
                    // FIXME do we want to use remoteElementNumber for something like multiple accounting buckets?
                    break;
                default:
                    CkError("ERROR in NeighborConnection::checkInvariant: invalid remoteEndpoint %d.\n", remoteEndpoint);
                    error = true;
                    break;
            }
            break;
        case RESERVOIR_RELEASE:
            if (!(localElementNumber < Readonly::globalNumberOfChannelElements))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: localElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                error = true;
            }
            
            if (!(RESERVOIR_RECIPIENT == remoteEndpoint))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: remoteEndpoint for a RESERVOIR_RELEASE must be a RESERVOIR_RECIPIENT.\n");
                error = true;
            }
            else
            {
                if (!(remoteElementNumber < Readonly::globalNumberOfChannelElements))
                {
                    CkError("ERROR in NeighborConnection::checkInvariant: remoteElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                    error = true;
                }
                
                if (!(localElementNumber != remoteElementNumber))
                {
                    CkError("ERROR in NeighborConnection::checkInvariant: invalid self-neighbor in channel element %d RESERVOIR_RELEASE.\n", localElementNumber);
                    error = true;
                }
            }
            break;
        case RESERVOIR_RECIPIENT:
            if (!(localElementNumber < Readonly::globalNumberOfChannelElements))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: localElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                error = true;
            }
            
            if (!(RESERVOIR_RELEASE == remoteEndpoint))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: remoteEndpoint for a RESERVOIR_RECIPIENT must be a RESERVOIR_RELEASE.\n");
                error = true;
            }
            else
            {
                if (!(remoteElementNumber < Readonly::globalNumberOfChannelElements))
                {
                    CkError("ERROR in NeighborConnection::checkInvariant: remoteElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                    error = true;
                }
                
                if (!(localElementNumber != remoteElementNumber))
                {
                    CkError("ERROR in NeighborConnection::checkInvariant: invalid self-neighbor in channel element %d RESERVOIR_RECIPIENT.\n", localElementNumber);
                    error = true;
                }
            }
            break;
        case IRRIGATION_DIVERSION:
            if (!(localElementNumber < Readonly::globalNumberOfChannelElements))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: localElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                error = true;
            }
            
            if (!(IRRIGATION_RECIPIENT == remoteEndpoint))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: remoteEndpoint for an IRRIGATION_DIVERSION must be an IRRIGATION_RECIPIENT.\n");
                error = true;
            }
            else
            {
                if (!(remoteElementNumber < Readonly::globalNumberOfMeshElements))
                {
                    CkError("ERROR in NeighborConnection::checkInvariant: remoteElementNumber must be less than Readonly::globalNumberOfMeshElements.\n");
                    error = true;
                }
            }
            break;
        case IRRIGATION_RECIPIENT:
            if (!(localElementNumber < Readonly::globalNumberOfMeshElements))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: localElementNumber must be less than Readonly::globalNumberOfMeshElements.\n");
                error = true;
            }
            
            if (!(IRRIGATION_DIVERSION == remoteEndpoint))
            {
                CkError("ERROR in NeighborConnection::checkInvariant: remoteEndpoint for an IRRIGATION_RECIPIENT must be an IRRIGATION_DIVERSION.\n");
                error = true;
            }
            else
            {
                if (!(remoteElementNumber < Readonly::globalNumberOfChannelElements))
                {
                    CkError("ERROR in NeighborConnection::checkInvariant: remoteElementNumber must be less than Readonly::globalNumberOfChannelElements.\n");
                    error = true;
                }
            }
            break;
        default:
            CkError("ERROR in NeighborConnection::checkInvariant: invalid localEndpoint %d.\n", localEndpoint);
            error = true;
            break;
    }
    
    return error;
}

bool NeighborConnection::operator<(const NeighborConnection& other) const
{
    bool lessThan; // Return value.
    
    if (remoteElementNumber < other.remoteElementNumber)
    {
        lessThan = true;
    }
    else if (remoteElementNumber > other.remoteElementNumber)
    {
        lessThan = false;
    }
    else if (remoteEndpoint < other.remoteEndpoint)
    {
        lessThan = true;
    }
    else if (remoteEndpoint > other.remoteEndpoint)
    {
        lessThan = false;
    }
    else if (localElementNumber < other.localElementNumber)
    {
        lessThan = true;
    }
    else if (localElementNumber > other.localElementNumber)
    {
        lessThan = false;
    }
    else if (localEndpoint < other.localEndpoint)
    {
        lessThan = true;
    }
    else if (localEndpoint > other.localEndpoint)
    {
        lessThan = false;
    }
    else
    {
        lessThan = false;
    }
    
    return lessThan;
}

bool NeighborAttributes::checkInvariant() const
{
    bool error = false; // Error flag.
    
    return error;
}

bool StateTransfer::checkInvariant() const
{
    bool error = false; // Error flag.
    
    return error;
}

bool WaterTransfer::checkInvariant() const
{
    bool error = false; // Error flag.
    
    if (!(0.0 <= water))
    {
        CkError("ERROR in WaterTransfer::checkInvariant: water must be greater than or equal to zero.\n");
        error = true;
    }
    
    if (!(startTime < endTime))
    {
        CkError("ERROR in WaterTransfer::checkInvariant: startTime must be less than endTime.\n");
        error = true;
    }
    
    return error;
}

bool NeighborMessage::receive(NeighborProxy& proxy, size_t& neighborsFinished, double currentTime, double timestepEndTime) const
{
    return proxy.receiveNeighborAttributes(neighborsFinished, this->attributes);
}

bool StateMessage::receive(NeighborProxy& proxy, size_t& neighborsFinished, double currentTime, double timestepEndTime) const
{
    return proxy.receiveStateMessage(neighborsFinished, *this, currentTime);
}

bool WaterMessage::receive(NeighborProxy& proxy, size_t& neighborsFinished, double currentTime, double timestepEndTime) const
{
    return proxy.receiveWaterMessage(neighborsFinished, *this, currentTime, timestepEndTime);
}

bool NeighborProxy::checkInvariant() const
{
    bool                                    error = false; // Error flag.
    std::set<WaterTransfer>::const_iterator it;            // Loop iterator.
    
    if (!(neighborRegion < Readonly::globalNumberOfRegions))
    {
        CkError("ERROR in NeighborProxy::checkInvariant: neighborRegion must be less than Readonly::globalNumberOfRegions.\n");
        error = true;
    }
    
    error = attributes.checkInvariant() || error;
    
    if (!(0.0 >= inflowCumulativeShortTerm))
    {
        CkError("ERROR in NeighborProxy::checkInvariant: inflowCumulativeShortTerm must be less than or equal to zero.\n");
        error = true;
    }
    
    if (!(0.0 >= inflowCumulativeLongTerm))
    {
        CkError("ERROR in NeighborProxy::checkInvariant: inflowCumulativeLongTerm must be less than or equal to zero.\n");
        error = true;
    }
    
    if (!(0.0 <= outflowCumulativeShortTerm))
    {
        CkError("ERROR in NeighborProxy::checkInvariant: outflowCumulativeShortTerm must be greater than or equal to zero.\n");
        error = true;
    }
    
    if (!(0.0 <= outflowCumulativeLongTerm))
    {
        CkError("ERROR in NeighborProxy::checkInvariant: outflowCumulativeLongTerm must be greater than or equal to zero.\n");
        error = true;
    }
    
    if (!incomingWater.empty())
    {
        if (!(0.0 > nominalFlowRate))
        {
            CkError("ERROR in NeighborProxy::checkInvariant: If incomingWater is not empty nominalFlowRate must be an inflow.\n");
            error = true;
        }
        
        if (!(incomingWater.rbegin()->endTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::checkInvariant: the last endTime in incomingWater must be less than or equal to expirationTime.\n");
            error = true;
        }
        
        for (it = incomingWater.begin(); it != incomingWater.end(); ++it)
        {
            error = it->checkInvariant() || error;
            
            if (!(-nominalFlowRate * (it->endTime - it->startTime) >= it->water)) // FIXME do I need to do epsilon on this check?
            {
                CkError("ERROR in NeighborProxy::checkInvariant: water received at a rate greater than nominalFlowRate.\n");
                error = true;
            }
        }
    }
    
    return error;
}

bool NeighborProxy::sendNeighborMessage(std::map<size_t, std::vector<NeighborMessage> >& outgoingMessages, size_t& neighborsFinished, const NeighborMessage& message)
{
    bool error = false; // Error flag.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = message.checkInvariant();
    }
    
    if (!error)
    {
        if (BOUNDARY_INFLOW      == message.destination.remoteEndpoint || BOUNDARY_OUTFLOW     == message.destination.remoteEndpoint ||
            TRANSBASIN_INFLOW    == message.destination.remoteEndpoint || TRANSBASIN_OUTFLOW   == message.destination.remoteEndpoint ||
            RESERVOIR_RELEASE    == message.destination.remoteEndpoint || RESERVOIR_RECIPIENT  == message.destination.remoteEndpoint ||
            IRRIGATION_DIVERSION == message.destination.remoteEndpoint || IRRIGATION_RECIPIENT == message.destination.remoteEndpoint)
        {
            // There is no remote neighbor.  Don't send the message and mark attributes as initialized and the NeighborProxy as finished.
            attributesInitialized = true;
            ++neighborsFinished;
        }
        else
        {
            // Send the message.
            outgoingMessages[neighborRegion].push_back(message);
        }
    }
    
    return error;
}

bool NeighborProxy::receiveNeighborAttributes(size_t& neighborsFinished, const NeighborAttributes& remoteAttributes)
{
    bool error = false; // Error flag.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = remoteAttributes.checkInvariant();
        
        if (!(!attributesInitialized))
        {
            CkError("ERROR in NeighborProxy::receiveNeighborAttributes: received attributes after attributes were already initialized.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        attributes            = remoteAttributes;
        attributesInitialized = true;
        ++neighborsFinished; // When a NeighborProxy receives attributes it is finished with initialization.
    }
    
    return error;
}

bool NeighborProxy::calculateNominalFlowRate(std::map<size_t, std::vector<StateMessage> >& outgoingMessages, size_t& neighborsFinished, const NeighborConnection& connection, double currentTime)
{
    bool error   = false; // Error flag.
    bool inflow  = false; // Flag for whether the flow is an inflow.
    bool outflow = false; // Flag for whether the flow is an outflow.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = connection.checkInvariant();
        
        if (!(currentTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::calculateNominalFlowRate: currentTime must be less than or equal to expirationTime.\n");
            error = true;
        }
    }
    
    // Only recalculate nominalFlowRate if it has expired.
    if (!error && currentTime == expirationTime)
    {
        // FIXME implement.  If the neighbor is in another region send a message with your state.
        // If the neighbor is in the same region go get its state and calculate right away.
        // If this is an inflow or outflow that has no neighbor element calculate right away.
        // For now I'm just hardcoding something simple.  Mesh elements always flow to channel elements.
        // Between elements of the same type the higher number element flows to the lower number one.
        // The flow rate is always 0.1 m^3/s and the expiration time is always 1 s.
        switch (connection.localEndpoint)
        {
            case MESH_SURFACE:
            case MESH_SOIL:
            case MESH_AQUIFER:
                if (BOUNDARY_INFLOW == connection.remoteEndpoint || TRANSBASIN_INFLOW == connection.remoteEndpoint)
                {
                    inflow = true;
                }
                else if ((MESH_SURFACE == connection.remoteEndpoint || MESH_SOIL == connection.remoteEndpoint || MESH_AQUIFER == connection.remoteEndpoint) && connection.localElementNumber < connection.remoteElementNumber)
                {
                    inflow = true;
                }
                else
                {
                    outflow = true;
                }
                break;
            case CHANNEL_SURFACE:
                if (BOUNDARY_OUTFLOW == connection.remoteEndpoint || TRANSBASIN_OUTFLOW == connection.remoteEndpoint)
                {
                    outflow = true;
                }
                else if (CHANNEL_SURFACE == connection.remoteEndpoint && connection.localElementNumber > connection.remoteElementNumber)
                {
                    outflow = true;
                }
                else
                {
                    inflow = true;
                }
                break;
            case RESERVOIR_RELEASE:
            case IRRIGATION_DIVERSION:
                outflow = true;
                break;
            default:
                // For IRRIGATION_RECIPIENT and RESERVOIR_RECIPIENT never recalculate nominalFlowRate and expirationTime.
                break;
        }
        
        if (outflow)
        {
            nominalFlowRate = 0.1;
            expirationTime  = currentTime + 1.0;
        }
        else if (inflow)
        {
            nominalFlowRate = -0.1;
            expirationTime  = currentTime + 1.0;
        }
        
        // If you don't need to wait for a message mark the NeighborProxy as finished.
        ++neighborsFinished;
    }
    else
    {
        // If the nominalFlowRate for this neighbor hasn't expired, it is finished.
        ++neighborsFinished;
    }
    
    return error;
}

bool NeighborProxy::receiveStateMessage(size_t& neighborsFinished, const StateMessage& state, double currentTime)
{
    bool error = false; // Error flag.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = state.checkInvariant();
        
        if (!(currentTime < expirationTime))
        {
            CkError("ERROR in NeighborProxy::receiveStateTransfer: received state when I do not need to recalculate nominalFlowRate.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // FIXME implement
        ++neighborsFinished; // When a NeighborProxy receives state it is finished with calculating nominalFlowRate.
    }
    
    return error;
}

bool NeighborProxy::sendWater(std::map<size_t, std::vector<WaterMessage> >& outgoingMessages, const WaterMessage& water)
{
    bool error = false; // Error flag.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = water.checkInvariant();
        
        if (!(0.0 < nominalFlowRate))
        {
            CkError("ERROR in NeighborProxy::sendWater: when sending water nominalFlowRate must be an outflow.\n");
            error = true;
        }
        
        if (!(water.water.endTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::sendWater: water.water.endTime must be less than or equal to expirationTime.\n");
            error = true;
        }
        
        if (!(nominalFlowRate * (water.water.endTime - water.water.startTime) >= water.water.water))
        {
            CkError("ERROR in NeighborProxy::sendWater: water sent at a rate greater than nominalFlowRate.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Record cumulative flow.
        if (0.0 != water.water.water)
        {
            if (0.0 != outflowCumulativeShortTerm && (outflowCumulativeShortTerm / water.water.water) > (outflowCumulativeLongTerm / outflowCumulativeShortTerm))
            {
                // The relative roundoff error of adding this timestep to short term is greater than adding short term to long term so move short term to long term.
                outflowCumulativeLongTerm += outflowCumulativeShortTerm;
                outflowCumulativeShortTerm = 0.0;
            }
            
            outflowCumulativeShortTerm += water.water.water;
        }
        
        // Send the water.  If the remote endpoint is a boundary or transbasin outflow there is no recipient element so don't send a message.
        if (BOUNDARY_OUTFLOW != water.destination.remoteEndpoint && TRANSBASIN_OUTFLOW != water.destination.remoteEndpoint)
        {
            outgoingMessages[neighborRegion].push_back(water);
        }
    }
    
    return error;
}

bool NeighborProxy::receiveWaterMessage(size_t& neighborsFinished, const WaterMessage& water, double currentTime, double timestepEndTime)
{
    bool error = false;   // Error flag.
    bool alreadyFinished; // Flag to indicate that allWaterHasArrived was true before this message arrived.
    bool waterInserted;   // Flag to indicate if water was correctly inserted.  It might not be if there was already an overlapping time range in incomingWater.  Unneeded if we use std::list instead.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = water.checkInvariant();
        
        if (!(0.0 > nominalFlowRate))
        {
            CkError("ERROR in NeighborProxy::receiveWaterTransfer: when receiving water nominalFlowRate must be an inflow.\n");
            error = true;
        }
        
        if (!(water.water.endTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::receiveWaterTransfer: water.water.endTime must be less than or equal to expirationTime.\n");
            error = true;
        }
        
        if (!(-nominalFlowRate * (water.water.endTime - water.water.startTime) >= water.water.water))
        {
            CkError("ERROR in NeighborProxy::receiveWaterTransfer: water received at a rate greater than nominalFlowRate.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // It's possible for a neighbor proxy to already have all of its water up through timestepEndTime and then receive another water message for after timestepEndTime.
        // In this case we don't want to increment neighborsFinished when allWaterHasArrived is true at the end.
        // FIXME do I somehow want to avoid calling allWaterHasArrived twice?
        alreadyFinished = allWaterHasArrived(water.destination, currentTime, timestepEndTime);
        waterInserted   = incomingWater.insert(water.water).second;
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (!waterInserted)
            {
                CkError("ERROR in NeighborProxy::receiveWaterTransfer: elements of incomingWater must be non-overlapping.\n");
                error = true;
            }
        }
    }
    
    if (!error && !alreadyFinished && allWaterHasArrived(water.destination, currentTime, timestepEndTime))
    {
        ++neighborsFinished;
    }
    
    return error;
}

bool NeighborProxy::allWaterHasArrived(const NeighborConnection& connection, double currentTime, double timestepEndTime)
{
    std::set<WaterTransfer>::iterator it;                        // Loop iterator.
    double                            lastEndTime = currentTime; // The last endTime for which water has arrived.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (connection.checkInvariant())
        {
            CkExit();
        }
        
        if (!(0.0 > nominalFlowRate))
        {
            CkError("ERROR in NeighborProxy::allWaterHasArrived: when calling allWaterHasArrived nominalFlowRate must be an inflow.\n");
            CkExit();
        }
        
        if (!(currentTime < timestepEndTime && timestepEndTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::allWaterHasArrived: currentTime must be less than timestepEndTime, which must be less than or equal to expirationTime.\n");
            CkExit();
        }
        
        if (!(incomingWater.empty() || currentTime <= incomingWater.begin()->startTime))
        {
            CkError("ERROR in NeighborProxy::allWaterHasArrived: currentTime must be less than or equal to the first startTime in incomingWater.\n");
            CkExit();
        }
    }
    
    // If the remote endpoint is a boundary or transbasin inflow then water is always available.
    if (BOUNDARY_INFLOW == connection.remoteEndpoint || TRANSBASIN_INFLOW == connection.remoteEndpoint)
    {
        lastEndTime = timestepEndTime;
    }
    else
    {
        // Check that there are no time gaps in incomingWater.
        for (it = incomingWater.begin(); it != incomingWater.end() && lastEndTime < timestepEndTime && lastEndTime == it->startTime; ++it)
        {
            lastEndTime = it->endTime;
        }
    }
    
    return (lastEndTime >= timestepEndTime);
}

double NeighborProxy::receiveWater(const NeighborConnection& connection, double currentTime, double timestepEndTime)
{
    double        water = 0.0;      // (m^3) Return value.
    double        partialQuantity;  // (m^3) Part of a WaterTransfer that will be received.
    WaterTransfer newWater;         // Used to modify an element in incomingWater by erasing and re-inserting with different values.  Unneeded if we use std::list instead.
    bool          newWaterInserted; // Flag to indicate if newWater was correctly inserted.  It might not be if there was already an overlapping time range in incomingWater.  Unneeded if we use std::list instead.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (connection.checkInvariant())
        {
            CkExit();
        }
        
        if (!(0.0 > nominalFlowRate))
        {
            CkError("ERROR in NeighborProxy::receiveWater: when calling receiveWater nominalFlowRate must be an inflow.\n");
            CkExit();
        }
        
        if (!(currentTime < timestepEndTime && timestepEndTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::receiveWater: currentTime must be less than timestepEndTime, which must be less than or equal to expirationTime.\n");
            CkExit();
        }
        
        if (!(incomingWater.empty() || currentTime == incomingWater.begin()->startTime))
        {
            CkError("ERROR in NeighborProxy::receiveWater: currentTime must be equal to the first startTime in incomingWater.\n");
            CkExit();
        }
    }
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
    {
        if (!allWaterHasArrived(connection, currentTime, timestepEndTime))
        {
            CkError("ERROR in NeighborProxy::receiveWater: It is an error to call receiveWater when allWaterHasArrived is false.\n");
            CkExit();
        }
    }
    
    // If the remote endpoint is a boundary or transbasin inflow then water is always available.
    if (BOUNDARY_INFLOW == connection.remoteEndpoint || TRANSBASIN_INFLOW == connection.remoteEndpoint)
    {
        water = -nominalFlowRate * (timestepEndTime - currentTime);
    }
    else
    {
        // Get all of the WaterTransfers in incomingWater up to timestepEndTime.
        while (!incomingWater.empty() && incomingWater.begin()->startTime < timestepEndTime)
        {
            if (incomingWater.begin()->endTime <= timestepEndTime)
            {
                // Get this entire WaterTransfer.
                water += incomingWater.begin()->water;
                incomingWater.erase(incomingWater.begin());
            }
            else
            {
                // Get part of this transfer up to timestepEndTime.
                partialQuantity = incomingWater.begin()->water * (timestepEndTime - incomingWater.begin()->startTime) / (incomingWater.begin()->endTime - incomingWater.begin()->startTime);
                
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                {
                    CkAssert(0.0 <= partialQuantity && partialQuantity < incomingWater.begin()->water);
                }
                
                water             += partialQuantity;
                newWater.water     = incomingWater.begin()->water - partialQuantity;
                newWater.startTime = timestepEndTime;
                newWater.endTime   = incomingWater.begin()->endTime;
                incomingWater.erase(incomingWater.begin());
                newWaterInserted   = incomingWater.insert(newWater).second;
                
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                {
                    CkAssert(!newWater.checkInvariant());
                    CkAssert(newWaterInserted);
                }
            }
        }
    }
    
    // Record cumulative flow.
    if (0.0 != water)
    {
        if (0.0 != inflowCumulativeShortTerm && (inflowCumulativeShortTerm / water) > (inflowCumulativeLongTerm / inflowCumulativeShortTerm))
        {
            // The relative roundoff error of adding this timestep to short term is greater than adding short term to long term so move short term to long term.
            inflowCumulativeLongTerm += inflowCumulativeShortTerm;
            inflowCumulativeShortTerm = 0.0;
        }
        
        inflowCumulativeShortTerm -= water;
    }
    
    return water;
}
