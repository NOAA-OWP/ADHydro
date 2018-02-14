#include "neighbor_proxy.h"
#include "surfacewater.h"
#include "groundwater.h"
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
    
    if (!(elementZTop >= elementZBottom))
    {
        CkError("ERROR in NeighborAttributes::checkInvariant: elementZTop must be greater than or equal to elementZBottom.\n");
        error = true;
    }
    
    if (!(0.0 < areaOrLength))
    {
        CkError("ERROR in NeighborAttributes::checkInvariant: areaOrLength must be greater than zero.\n");
        error = true;
    }
    
    if (!(0.0 < manningsN))
    {
        CkError("ERROR in NeighborAttributes::checkInvariant: manningsN must be greater than zero.\n");
        error = true;
    }
    
    if (!(0.0 <= conductivity))
    {
        CkError("ERROR in NeighborAttributes::checkInvariant: conductivity must be greater than or equal to zero.\n");
        error = true;
    }
    
    if (!(0.0 < porosityOrBedThickness))
    {
        CkError("ERROR in NeighborAttributes::checkInvariant: porosityOrBedThickness must be greater than zero.\n");
        error = true;
    }
    
    if (!(STREAM == channelType || WATERBODY == channelType || ICEMASS == channelType))
    {
        CkError("ERROR in NeighborAttributes::checkInvariant: channelType must be STREAM or WATERBODY or ICEMASS.\n");
        error = true;
    }
    
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

bool NeighborMessage::receive(NeighborProxy& proxy, size_t& neighborsFinished, const NeighborAttributes& localAttributes, double localDepthOrHead, double currentTime, double timestepEndTime) const
{
    double newZOffset = 0.0; // If necessary, calculate a new zOffset.  This is done here just because it is a place where all of the required information is available.
                             // FIXME implement the full calculation from v2.0
                             // FIXME problem with zTop of aquifer not being zSurface
    
    if ((MESH_SURFACE == destination.localEndpoint || MESH_SOIL == destination.localEndpoint || MESH_AQUIFER == destination.localEndpoint) &&
        CHANNEL_SURFACE == destination.remoteEndpoint && ICEMASS != attributes.channelType && attributes.elementZTop > localAttributes.elementZTop)
    {
        newZOffset = attributes.elementZTop - localAttributes.elementZTop;
    }
    else if ((MESH_SURFACE == destination.remoteEndpoint || MESH_SOIL == destination.remoteEndpoint || MESH_AQUIFER == destination.remoteEndpoint) &&
             CHANNEL_SURFACE == destination.localEndpoint && ICEMASS != localAttributes.channelType && localAttributes.elementZTop > attributes.elementZTop)
    {
        newZOffset = localAttributes.elementZTop - attributes.elementZTop;
    }
    
    return proxy.receiveNeighborAttributes(neighborsFinished, newZOffset, attributes);
}

bool StateMessage::receive(NeighborProxy& proxy, size_t& neighborsFinished, const NeighborAttributes& localAttributes, double localDepthOrHead, double currentTime, double timestepEndTime) const
{
    return proxy.receiveStateMessage(neighborsFinished, *this, localAttributes, localDepthOrHead, currentTime);
}

bool WaterMessage::receive(NeighborProxy& proxy, size_t& neighborsFinished, const NeighborAttributes& localAttributes, double localDepthOrHead, double currentTime, double timestepEndTime) const
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
    
    if (!(0.0 < edgeLength))
    {
        CkError("ERROR in NeighborProxy::checkInvariant: edgeLength must be greater than zero.\n");
        error = true;
    }
    
    if (!(epsilonEqual(1.0, edgeNormalX * edgeNormalX + edgeNormalY * edgeNormalY)))
    {
        CkError("ERROR in NeighborProxy::checkInvariant: edgeNormalX and edgeNormalY must make a unit vector.\n");
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
            // There is no remote neighbor, or the remote neighbor doesn't need my attributes.  Don't send the message and mark attributes as initialized and the NeighborProxy as finished.
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

bool NeighborProxy::receiveNeighborAttributes(size_t& neighborsFinished, double newZOffset, const NeighborAttributes& remoteAttributes)
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
        zOffset               = newZOffset;
        attributes            = remoteAttributes;
        attributesInitialized = true;
        ++neighborsFinished; // When a NeighborProxy receives attributes it is finished with initialization.
    }
    
    return error;
}

bool NeighborProxy::calculateNominalFlowRate(std::map<size_t, std::vector<StateMessage> >& outgoingMessages, size_t& neighborsFinished, const StateMessage& state, const NeighborAttributes& localAttributes, double currentTime)
{
    bool error = false; // Error flag.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = state.checkInvariant();
        error = localAttributes.checkInvariant() || error;
        
        if (!(currentTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::calculateNominalFlowRate: currentTime must be less than or equal to expirationTime.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Only recalculate nominalFlowRate if it has expired.
        if (currentTime == expirationTime)
        {
            if (BOUNDARY_INFLOW   == state.destination.remoteEndpoint || BOUNDARY_OUTFLOW     == state.destination.remoteEndpoint ||
                TRANSBASIN_INFLOW == state.destination.remoteEndpoint || TRANSBASIN_OUTFLOW   == state.destination.remoteEndpoint ||
                RESERVOIR_RELEASE == state.destination.localEndpoint  || IRRIGATION_DIVERSION == state.destination.localEndpoint)
            {
                // I can unilaterally calculate the flow rate without communicating with anybody.
                error = nominalFlowRateCalculation(state.destination.localEndpoint, state.destination.remoteEndpoint, localAttributes, state.depthOrHead, 0.0, currentTime);
                ++neighborsFinished;
            }
            else if (RESERVOIR_RECIPIENT == state.destination.localEndpoint || IRRIGATION_RECIPIENT == state.destination.localEndpoint)
            {
                // This sould actually never happen since expirationTime should be INFINITY.  Report an error.
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                {
                    CkError("ERROR in NeighborProxy::calculateNominalFlowRate: Trying to recalculate flow rate for RESERVOIR_RECIPIENT or IRRIGATION_RECIPIENT, which should never happen.\n");
                    error = true;
                }
            }
            else
            {
                // Send my state to my neighbor.
                // FIXME short circuit if neighbor is on this PE?
                outgoingMessages[neighborRegion].push_back(state);
            }
        }
        else
        {
            // If the nominalFlowRate for this neighbor hasn't expired, it is finished.
            ++neighborsFinished;
        }
    }
    
    return error;
}

bool NeighborProxy::receiveStateMessage(size_t& neighborsFinished, const StateMessage& state, const NeighborAttributes& localAttributes, double localDepthOrHead, double currentTime)
{
    bool error = false; // Error flag.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = state.checkInvariant();
        error = localAttributes.checkInvariant() || error;
        
        if (!(currentTime == expirationTime))
        {
            CkError("ERROR in NeighborProxy::receiveStateMessage: currentTime must be equal to expirationTime.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        error = nominalFlowRateCalculation(state.destination.localEndpoint, state.destination.remoteEndpoint, localAttributes, localDepthOrHead, state.depthOrHead, currentTime);
        ++neighborsFinished;
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
                    CkAssert(0.0 <= partialQuantity && partialQuantity <= incomingWater.begin()->water);
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

#define GLOBAL_DT_LIMIT (60.0) // FIXME do something better with this.
#define INFLOW_VELOCITY (0.0)
#define INFLOW_HEIGHT   (0.0)

bool NeighborProxy::nominalFlowRateCalculation(NeighborEndpointEnum localEndpoint, NeighborEndpointEnum remoteEndpoint, const NeighborAttributes& localAttributes,
                                               double localDepthOrHead, double remoteDepthOrHead, double currentTime)
{
    bool   error = false;           // Error flag.
    double dtNew = GLOBAL_DT_LIMIT; // (s) Desired duration until the next expiration time.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
        CkAssert(!localAttributes.checkInvariant() && currentTime == expirationTime);
    }
    
    if (MESH_SURFACE == localEndpoint && MESH_SURFACE == remoteEndpoint)
    {
        error = surfacewaterMeshMeshFlowRate(&nominalFlowRate, &dtNew, edgeLength, localAttributes.elementX, localAttributes.elementY, localAttributes.elementZTop,
                                             localAttributes.areaOrLength, localAttributes.manningsN, localDepthOrHead, attributes.elementX, attributes.elementY,
                                             attributes.elementZTop, attributes.areaOrLength, attributes.manningsN, remoteDepthOrHead);
    }
    else if ((MESH_SURFACE == localEndpoint && MESH_SOIL    == remoteEndpoint) ||
             (MESH_SURFACE == localEndpoint && MESH_AQUIFER == remoteEndpoint))
    {
        // Element is the soil neighbor.  Neighbor X and Y are the midpoint between the soil and surface where we assume the soil ends.
        // NeighborZBedrock is the surface elevation.  NeighborZSurface is the surface elevation plus the thickness of the soil layer.
        // NeighborConductivity and Porosity are the soil values.
        error = groundwaterMeshMeshFlowRate(&nominalFlowRate, &dtNew, edgeLength, attributes.elementX, attributes.elementY, attributes.elementZTop, attributes.elementZBottom, attributes.areaOrLength,
                                            attributes.conductivity, attributes.porosityOrBedThickness, remoteDepthOrHead, 0.5 * (localAttributes.elementX + attributes.elementX),
                                            0.5 * (localAttributes.elementY + attributes.elementY), localAttributes.elementZTop + (attributes.elementZTop - attributes.elementZBottom),
                                            localAttributes.elementZTop, localAttributes.areaOrLength, attributes.conductivity, attributes.porosityOrBedThickness, localAttributes.elementZTop + localDepthOrHead);
        
        // Reverse the direction of flow because the local element is neighbor in the previous calculation.
        nominalFlowRate *= -1.0;
    }
    else if (MESH_SURFACE == localEndpoint && CHANNEL_SURFACE == remoteEndpoint)
    {
        error = surfacewaterMeshChannelFlowRate(&nominalFlowRate, &dtNew, edgeLength, localAttributes.elementZTop + zOffset, localAttributes.areaOrLength, localDepthOrHead,
                                                attributes.elementZTop, attributes.elementZBottom, attributes.slopeXOrBaseWidth, attributes.slopeYOrSideSlope, remoteDepthOrHead);
    }
    else if (MESH_SURFACE == localEndpoint && BOUNDARY_INFLOW == remoteEndpoint)
    {
        error = surfacewaterMeshBoundaryFlowRate(&nominalFlowRate, &dtNew, INFLOW, INFLOW_VELOCITY, INFLOW_VELOCITY, INFLOW_HEIGHT, edgeLength, edgeNormalX, edgeNormalY, localAttributes.areaOrLength, localDepthOrHead);
    }
    else if (MESH_SURFACE == localEndpoint && BOUNDARY_OUTFLOW == remoteEndpoint)
    {
        error = surfacewaterMeshBoundaryFlowRate(&nominalFlowRate, &dtNew, OUTFLOW, INFLOW_VELOCITY, INFLOW_VELOCITY, INFLOW_HEIGHT, edgeLength, edgeNormalX, edgeNormalY, localAttributes.areaOrLength, localDepthOrHead);
    }
    else if (MESH_SURFACE == localEndpoint && TRANSBASIN_INFLOW == remoteEndpoint)
    {
        // FIXME transbasin flows not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (MESH_SURFACE == localEndpoint && TRANSBASIN_OUTFLOW == remoteEndpoint)
    {
        // FIXME transbasin flows not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (MESH_SOIL == localEndpoint && MESH_SURFACE == remoteEndpoint)
    {
        // Element is the soil neighbor.  Neighbor X and Y are the midpoint between the soil and surface where we assume the soil ends.
        // NeighborZBedrock is the surface elevation.  NeighborZSurface is the surface elevation plus the thickness of the soil layer.
        // NeighborConductivity and Porosity are the soil values.
        error = groundwaterMeshMeshFlowRate(&nominalFlowRate, &dtNew, edgeLength, localAttributes.elementX, localAttributes.elementY, localAttributes.elementZTop, localAttributes.elementZBottom, localAttributes.areaOrLength,
                                            localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead, 0.5 * (localAttributes.elementX + attributes.elementX),
                                            0.5 * (localAttributes.elementY + attributes.elementY), attributes.elementZTop + (localAttributes.elementZTop - localAttributes.elementZBottom),
                                            attributes.elementZTop, attributes.areaOrLength, localAttributes.conductivity, localAttributes.porosityOrBedThickness, attributes.elementZTop + remoteDepthOrHead);
    }
    else if ((MESH_SOIL == localEndpoint && MESH_SOIL    == remoteEndpoint) ||
             (MESH_SOIL == localEndpoint && MESH_AQUIFER == remoteEndpoint))
    {
        error = groundwaterMeshMeshFlowRate(&nominalFlowRate, &dtNew, edgeLength, localAttributes.elementX, localAttributes.elementY, localAttributes.elementZTop, localAttributes.elementZBottom,
                                            localAttributes.areaOrLength, localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead, attributes.elementX, attributes.elementY,
                                            attributes.elementZTop, attributes.elementZBottom, attributes.areaOrLength, attributes.conductivity, attributes.porosityOrBedThickness, remoteDepthOrHead);
    }
    else if (MESH_SOIL == localEndpoint && CHANNEL_SURFACE == remoteEndpoint)
    {
        error = groundwaterMeshChannelFlowRate(&nominalFlowRate, edgeLength, localAttributes.elementZTop + zOffset, localAttributes.elementZBottom + zOffset, localDepthOrHead + zOffset, attributes.elementZTop,
                                               attributes.elementZBottom, attributes.slopeXOrBaseWidth, attributes.slopeYOrSideSlope, attributes.conductivity, attributes.porosityOrBedThickness, remoteDepthOrHead);
    }
    else if (MESH_SOIL == localEndpoint && BOUNDARY_INFLOW == remoteEndpoint)
    {
        error = groundwaterMeshBoundaryFlowRate(&nominalFlowRate, &dtNew, INFLOW, INFLOW_HEIGHT, edgeLength, edgeNormalX, edgeNormalY, localAttributes.elementZBottom, localAttributes.areaOrLength,
                                                localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead);
    }
    else if (MESH_SOIL == localEndpoint && BOUNDARY_OUTFLOW == remoteEndpoint)
    {
        error = groundwaterMeshBoundaryFlowRate(&nominalFlowRate, &dtNew, OUTFLOW, INFLOW_HEIGHT, edgeLength, edgeNormalX, edgeNormalY, localAttributes.elementZBottom, localAttributes.areaOrLength,
                                                localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead);
    }
    else if (MESH_SOIL == localEndpoint && TRANSBASIN_INFLOW == remoteEndpoint)
    {
        // FIXME transbasin flows not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (MESH_SOIL == localEndpoint && TRANSBASIN_OUTFLOW == remoteEndpoint)
    {
        // FIXME transbasin flows not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (MESH_AQUIFER == localEndpoint && MESH_SURFACE == remoteEndpoint)
    {
        // Element is the soil neighbor.  Neighbor X and Y are the midpoint between the soil and surface where we assume the soil ends.
        // NeighborZBedrock is the surface elevation.  NeighborZSurface is the surface elevation plus the thickness of the soil layer.
        // NeighborConductivity and Porosity are the soil values.
        error = groundwaterMeshMeshFlowRate(&nominalFlowRate, &dtNew, edgeLength, localAttributes.elementX, localAttributes.elementY, localAttributes.elementZTop, localAttributes.elementZBottom, localAttributes.areaOrLength,
                                            localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead, 0.5 * (localAttributes.elementX + attributes.elementX),
                                            0.5 * (localAttributes.elementY + attributes.elementY), attributes.elementZTop + (localAttributes.elementZTop - localAttributes.elementZBottom),
                                            attributes.elementZTop, attributes.areaOrLength, localAttributes.conductivity, localAttributes.porosityOrBedThickness, attributes.elementZTop + remoteDepthOrHead);
    }
    else if ((MESH_AQUIFER == localEndpoint && MESH_SOIL == remoteEndpoint) ||
             (MESH_AQUIFER == localEndpoint && MESH_AQUIFER == remoteEndpoint))
    {
        error = groundwaterMeshMeshFlowRate(&nominalFlowRate, &dtNew, edgeLength, localAttributes.elementX, localAttributes.elementY, localAttributes.elementZTop, localAttributes.elementZBottom,
                                            localAttributes.areaOrLength, localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead, attributes.elementX, attributes.elementY,
                                            attributes.elementZTop, attributes.elementZBottom, attributes.areaOrLength, attributes.conductivity, attributes.porosityOrBedThickness, remoteDepthOrHead);
    }
    else if (MESH_AQUIFER == localEndpoint && CHANNEL_SURFACE == remoteEndpoint)
    {
        error = groundwaterMeshChannelFlowRate(&nominalFlowRate, edgeLength, localAttributes.elementZTop + zOffset, localAttributes.elementZBottom + zOffset, localDepthOrHead + zOffset, attributes.elementZTop,
                                               attributes.elementZBottom, attributes.slopeXOrBaseWidth, attributes.slopeYOrSideSlope, attributes.conductivity, attributes.porosityOrBedThickness, remoteDepthOrHead);
    }
    else if (MESH_AQUIFER == localEndpoint && BOUNDARY_INFLOW == remoteEndpoint)
    {
        error = groundwaterMeshBoundaryFlowRate(&nominalFlowRate, &dtNew, INFLOW, INFLOW_HEIGHT, edgeLength, edgeNormalX, edgeNormalY, localAttributes.elementZBottom, localAttributes.areaOrLength,
                                                localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead);
    }
    else if (MESH_AQUIFER == localEndpoint && BOUNDARY_OUTFLOW == remoteEndpoint)
    {
        error = groundwaterMeshBoundaryFlowRate(&nominalFlowRate, &dtNew, OUTFLOW, INFLOW_HEIGHT, edgeLength, edgeNormalX, edgeNormalY, localAttributes.elementZBottom, localAttributes.areaOrLength,
                                                localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead);
    }
    else if (MESH_AQUIFER == localEndpoint && TRANSBASIN_INFLOW == remoteEndpoint)
    {
        // FIXME transbasin flows not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (MESH_AQUIFER == localEndpoint && TRANSBASIN_OUTFLOW == remoteEndpoint)
    {
        // FIXME transbasin flows not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (CHANNEL_SURFACE == localEndpoint && MESH_SURFACE == remoteEndpoint)
    {
        error = surfacewaterMeshChannelFlowRate(&nominalFlowRate, &dtNew, edgeLength, attributes.elementZTop + zOffset, attributes.areaOrLength, remoteDepthOrHead, localAttributes.elementZTop,
                                                localAttributes.elementZBottom, localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localDepthOrHead);
        
        // Reverse the direction of flow because the local element is neighbor in the previous calculation.
        nominalFlowRate *= -1.0;
    }
    else if ((CHANNEL_SURFACE == localEndpoint && MESH_SOIL    == remoteEndpoint) ||
             (CHANNEL_SURFACE == localEndpoint && MESH_AQUIFER == remoteEndpoint))
    {
        error = groundwaterMeshChannelFlowRate(&nominalFlowRate, edgeLength, attributes.elementZTop + zOffset, attributes.elementZBottom + zOffset, remoteDepthOrHead + zOffset, localAttributes.elementZTop, localAttributes.elementZBottom,
                                               localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localAttributes.conductivity, localAttributes.porosityOrBedThickness, localDepthOrHead);
        
        // Reverse the direction of flow because the local element is neighbor in the previous calculation.
        nominalFlowRate *= -1.0;
    }
    else if (CHANNEL_SURFACE == localEndpoint && CHANNEL_SURFACE == remoteEndpoint)
    {
        error = surfacewaterChannelChannelFlowRate(&nominalFlowRate, &dtNew, localAttributes.channelType, localAttributes.elementZTop, localAttributes.elementZBottom, localAttributes.areaOrLength,
                                                   localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localAttributes.manningsN, localDepthOrHead, attributes.channelType, attributes.elementZTop,
                                                   attributes.elementZBottom, attributes.areaOrLength, attributes.slopeXOrBaseWidth, attributes.slopeYOrSideSlope, attributes.manningsN, remoteDepthOrHead);
    }
    else if (CHANNEL_SURFACE == localEndpoint && BOUNDARY_INFLOW == remoteEndpoint)
    {
        error = surfacewaterChannelBoundaryFlowRate(&nominalFlowRate, &dtNew, INFLOW, INFLOW_VELOCITY, INFLOW_HEIGHT, localAttributes.areaOrLength, localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localDepthOrHead);
    }
    else if (CHANNEL_SURFACE == localEndpoint && BOUNDARY_OUTFLOW == remoteEndpoint)
    {
        error = surfacewaterChannelBoundaryFlowRate(&nominalFlowRate, &dtNew, OUTFLOW, INFLOW_VELOCITY, INFLOW_HEIGHT, localAttributes.areaOrLength, localAttributes.slopeXOrBaseWidth, localAttributes.slopeYOrSideSlope, localDepthOrHead);
    }
    else if (CHANNEL_SURFACE == localEndpoint && TRANSBASIN_INFLOW == remoteEndpoint)
    {
        // FIXME transbasin flows not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (CHANNEL_SURFACE == localEndpoint && TRANSBASIN_OUTFLOW == remoteEndpoint)
    {
        // FIXME transbasin flows not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (RESERVOIR_RELEASE == localEndpoint)
    {
        // FIXME water management not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (IRRIGATION_DIVERSION == localEndpoint)
    {
        // FIXME water management not yet implemented
        nominalFlowRate = 0.0;
        dtNew           = INFINITY;
    }
    else if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
        CkAssert(false); // All other endpoints are invalid here.
    }
    
    if (!error)
    {
        if (INFINITY == dtNew)
        {
            expirationTime = INFINITY;
        }
        else
        {
            expirationTime = Readonly::newExpirationTime(currentTime, dtNew);
        }
    }
    
    return error;
}
