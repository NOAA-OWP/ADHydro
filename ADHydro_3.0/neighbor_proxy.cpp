#include "neighbor_proxy.h"

bool NeighborConnection::checkInvariant()
{
    bool error           = false; // Error flag.
    bool localIsMesh     = false; // Flag for detecting self-neighbors.
    bool localIsChannel  = false; // Flag for detecting self-neighbors.
    bool remoteIsMesh    = false; // Flag for detecting self-neighbors.
    bool remoteIsChannel = false; // Flag for detecting self-neighbors.
    
    switch (localEndpoint)
    {
        case MESH_SURFACE:
        case MESH_SOIL:
        case MESH_AQUIFER:
        case IRRIGATION_RECIPIENT:
            // FIXME localElementNumber must be a valid mesh element
            localIsMesh = true;
            break;
        case CHANNEL_SURFACE:
        case RESERVOIR_RELEASE:
        case RESERVOIR_RECIPIENT:
        case IRRIGATION_DIVERSION:
            // FIXME localElementNumber must be a valid channel element
            localIsChannel = true;
            break;
        case BOUNDARY_INFLOW:
        case BOUNDARY_OUTFLOW:
        case TRANSBASIN_INFLOW:
        case TRANSBASIN_OUTFLOW:
        default:
            CkError("ERROR in NeighborConnection::checkInvariant: invalid localEndpoint %d.\n", localEndpoint);
            error = true;
            break;
    }
    
    switch (remoteEndpoint)
    {
        case MESH_SURFACE:
        case MESH_SOIL:
        case MESH_AQUIFER:
        case IRRIGATION_RECIPIENT:
            // FIXME remoteElementNumber must be a valid mesh element
            remoteIsMesh = true;
            break;
        case CHANNEL_SURFACE:
        case RESERVOIR_RELEASE:
        case RESERVOIR_RECIPIENT:
        case IRRIGATION_DIVERSION:
            // FIXME remoteElementNumber must be a valid channel element
            remoteIsChannel = true;
            break;
        case BOUNDARY_INFLOW:
        case BOUNDARY_OUTFLOW:
        case TRANSBASIN_INFLOW:
        case TRANSBASIN_OUTFLOW:
            // FIXME do we want to use remoteElementNumber for something like multiple accounting buckets?
            break;
        default:
            CkError("ERROR in NeighborConnection::checkInvariant: invalid remoteEndpoint %d.\n", localEndpoint);
            error = true;
            break;
    }
    
    if (((localIsMesh && remoteIsMesh) || (localIsChannel && remoteIsChannel)) && localElementNumber == remoteElementNumber)
    {
        CkError("ERROR in NeighborConnection::checkInvariant: invalid self-neighbor in %s element %d.\n", (localIsMesh ? "mesh" : "channel"), localElementNumber);
        error = true;
    }
    
    return error;
}

bool WaterTransfer::checkInvariant()
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

bool NeighborProxy::checkInvariant()
{
    bool                               error = false;                 // Error flag.
    std::list<WaterTransfer>::iterator it    = incomingWater.begin(); // Loop iterator.
    double                             previousEndTime;               // To check that the elements of incomingWater are sorted and non-overlapping.
    
    error = connection.checkInvariant();
    
    // These connection types are one way flows where the two neighbors never communicate to calculate nominalFlowRate.  Instead, the sender just sends water.
    // We handle this at the recipient by setting nominalFlowRate to -INFINITY and expirationTime to INFINITY.  This is equivalent to saying that the pair has negotiated
    // that the remote element is the sender, the flow rate will be less than or equal to infinity, and they will meet again to renegotiate at the end of time.
    // This does not apply to natural boundary and transbasin inflows.  In those cases, there is no other neighbor and the recipient calculates nominalFlowRate.
    if (RESERVOIR_RECIPIENT == connection.localEndpoint || IRRIGATION_RECIPIENT == connection.localEndpoint)
    {
        if (!(-INFINITY == nominalFlowRate && INFINITY == expirationTime))
        {
            CkError("ERROR in NeighborProxy::checkInvariant: For a one way flow nominalFlowRate must be minus infinity and expirationTime must be infinity.\n");
            error = true;
        }
    }
    
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
        
        if (!(incomingWater.back().endTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::checkInvariant: the last endTime in incomingWater must be less than or equal to expirationTime.\n");
            error = true;
        }
    }
    
    while (it != incomingWater.end())
    {
        if (!(-nominalFlowRate * (it->endTime - it->startTime) >= it->water))
        {
            CkError("ERROR in NeighborProxy::checkInvariant: water received at a rate greater than nominalFlowRate.\n");
            error = true;
        }
        
        error           = it->checkInvariant() || error;
        previousEndTime = it->endTime;
        ++it;
        
        if (it != incomingWater.end())
        {
            if (!(previousEndTime <= it->startTime))
            {
                CkError("ERROR in NeighborProxy::checkInvariant: elements of incomingWater must be sorted and non-overlapping.\n");
                error = true;
            }
        }
    }
    
    return error;
}

bool NeighborProxy::sendWaterTransfer(WaterTransfer water)
{
    bool error = false; // Error flag.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
    {
        error = water.checkInvariant();
        
        if (!(0.0 < nominalFlowRate))
        {
            CkError("ERROR in NeighborProxy::sendWaterTransfer: when sending water nominalFlowRate must be an outflow.\n");
            error = true;
        }
        
        if (!(water.endTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::sendWaterTransfer: water.endTime must be less than or equal to expirationTime.\n");
            error = true;
        }
        
        if (!(nominalFlowRate * (water.endTime - water.startTime) >= water.water))
        {
            CkError("ERROR in NeighborProxy::sendWaterTransfer: water sent at a rate greater than nominalFlowRate.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Record cumulative flow.
        if (0.0 != water.water)
        {
            if (0.0 != outflowCumulativeShortTerm && (outflowCumulativeShortTerm / water.water) > (outflowCumulativeLongTerm / outflowCumulativeShortTerm))
            {
                // The relative roundoff error of adding this timestep to short term is greater than adding short term to long term so move short term to long term.
                outflowCumulativeLongTerm += outflowCumulativeShortTerm;
                outflowCumulativeShortTerm = 0.0;
            }
            
            outflowCumulativeShortTerm += water.water;
        }
        
        // FIXME Send message
    }
    
    return error;
}

bool NeighborProxy::receiveWaterTransfer(WaterTransfer water)
{
    bool                                       error = false;                  // Error flag.
    std::list<WaterTransfer>::reverse_iterator it    = incomingWater.rbegin(); // Loop iterator.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
    {
        error = water.checkInvariant();
        
        if (!(0.0 > nominalFlowRate))
        {
            CkError("ERROR in NeighborProxy::receiveWaterTransfer: when receiving water nominalFlowRate must be an inflow.\n");
            error = true;
        }
        
        if (!(water.endTime <= expirationTime))
        {
            CkError("ERROR in NeighborProxy::receiveWaterTransfer: water.endTime must be less than or equal to expirationTime.\n");
            error = true;
        }
        
        if (!(-nominalFlowRate * (water.endTime - water.startTime) >= water.water))
        {
            CkError("ERROR in NeighborProxy::receiveWaterTransfer: water received at a rate greater than nominalFlowRate.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Find the last element in the list that ends no later than when this new element starts.
        while (it != incomingWater.rend() && it->endTime > water.startTime)
        {
            ++it;
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
        {
            // Check that the next one in the list does not overlap the new element.
            if (it.base() != incomingWater.end())
            {
                if (!(water.endTime <= it.base()->startTime))
                {
                    CkError("ERROR in NeighborProxy::receiveWaterTransfer: elements of incomingWater must be non-overlapping.\n");
                    error = true;
                }
            }
        }
    }
    
    if (!error)
    {
        incomingWater.insert(it.base(), water);
    }
    
    return error;
}

bool NeighborProxy::allWaterHasArrived(double currentTime, double timestepEndTime)
{
    std::list<WaterTransfer>::iterator it          = incomingWater.begin(); // Loop iterator.
    double                             lastEndTime = currentTime;           // The last endTime for which water has arrived.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(currentTime < timestepEndTime))
        {
            CkError("ERROR in NeighborProxy::allWaterHasArrived: currentTime must be less than timestepEndTime.\n");
            CkExit();
        }
        
        if (!(incomingWater.empty() || currentTime <= incomingWater.front().startTime))
        {
            CkError("ERROR in NeighborProxy::allWaterHasArrived: currentTime must be less than or equal to the first startTime in incomingWater.\n");
            CkExit();
        }
    }
    
    while (it != incomingWater.end() && lastEndTime < timestepEndTime && lastEndTime == it->startTime)
    {
        lastEndTime = it->endTime;
        ++it;
    }
    
    return (lastEndTime >= timestepEndTime);
}

double NeighborProxy::receiveWater(double timestepEndTime)
{
    double water = 0.0;     // (m^3) The water that is being received.
    double partialQuantity; // (m^3) Part of a WaterTransfer that will be received.
    
    while (!incomingWater.empty() && incomingWater.front().startTime < timestepEndTime)
    {
        if (incomingWater.front().endTime <= timestepEndTime)
        {
            // Get this entire WaterTransfer.
            water += incomingWater.front().water;
            incomingWater.pop_front();
        }
        else
        {
            // Get part of this transfer up to timestepEndTime.
            partialQuantity = incomingWater.front().water * (timestepEndTime - incomingWater.front().startTime) / (incomingWater.front().endTime - incomingWater.front().startTime);
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
            {
                CkAssert(0.0 <= partialQuantity && partialQuantity < incomingWater.front().water);
            }
            
            water                          += partialQuantity;
            incomingWater.front().water    -= partialQuantity;
            incomingWater.front().startTime = timestepEndTime;
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
