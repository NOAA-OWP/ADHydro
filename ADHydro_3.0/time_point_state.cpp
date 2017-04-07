#include "time_point_state.h"

TimePointState::TimePointState(size_t globalNumberOfMeshElements, size_t localNumberOfMeshElements, size_t localMeshElementStart, size_t maximumNumberOfMeshNeighbors,
                               size_t globalNumberOfChannelElements, size_t localNumberOfChannelElements, size_t localChannelElementStart, size_t maximumNumberOfChannelNeighbors) :
                               globalNumberOfMeshElements(globalNumberOfMeshElements), localNumberOfMeshElements(localNumberOfMeshElements), localMeshElementStart(localMeshElementStart),
                               maximumNumberOfMeshNeighbors(maximumNumberOfMeshNeighbors), globalNumberOfChannelElements(globalNumberOfChannelElements), localNumberOfChannelElements(localNumberOfChannelElements),
                               localChannelElementStart(localChannelElementStart), maximumNumberOfChannelNeighbors(maximumNumberOfChannelNeighbors), elementsReceived(0),
                               #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                               meshStateReceived((0 == localNumberOfMeshElements) ? (NULL) : (new bool[localNumberOfMeshElements])),
                               channelStateReceived((0 == localNumberOfChannelElements) ? (NULL) : (new bool[localNumberOfChannelElements])),
                               #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                               meshEvapoTranspirationState(    (0 == localNumberOfMeshElements)                                ? (NULL) : (new EvapoTranspirationStateBlob[localNumberOfMeshElements])),
                               meshSurfaceWater(               (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshSurfaceWaterCreated(        (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshGroundwaterMode(            (0 == localNumberOfMeshElements)                                ? (NULL) : (new         GroundwaterModeEnum[localNumberOfMeshElements])),
                               meshPerchedHead(                (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshSoilWater(                  (0 == localNumberOfMeshElements)                                ? (NULL) : (new        GroundwaterStateBlob[localNumberOfMeshElements])),
                               meshSoilWaterCreated(           (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshAquiferHead(                (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshAquiferWater(               (0 == localNumberOfMeshElements)                                ? (NULL) : (new        GroundwaterStateBlob[localNumberOfMeshElements])),
                               meshAquiferWaterCreated(        (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshDeepGroundwater(            (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshPrecipitationRate(          (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshPrecipitationCumulative(    (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshEvaporationRate(            (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshEvaporationCumulative(      (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshTranspirationRate(          (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshTranspirationCumulative(    (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshCanopyWaterEquivalent(      (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshSnowWaterEquivalent(        (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshRootZoneWater(              (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshTotalGroundwater(           (0 == localNumberOfMeshElements)                                ? (NULL) : (new                      double[localNumberOfMeshElements])),
                               meshNeighborLocalEndpoint(      (0 == localNumberOfMeshElements * maximumNumberOfMeshNeighbors) ? (NULL) : (new        NeighborEndpointEnum[localNumberOfMeshElements * maximumNumberOfMeshNeighbors])),
                               meshNeighborRemoteEndpoint(     (0 == localNumberOfMeshElements * maximumNumberOfMeshNeighbors) ? (NULL) : (new        NeighborEndpointEnum[localNumberOfMeshElements * maximumNumberOfMeshNeighbors])),
                               meshNeighborRemoteElementNumber((0 == localNumberOfMeshElements * maximumNumberOfMeshNeighbors) ? (NULL) : (new                      size_t[localNumberOfMeshElements * maximumNumberOfMeshNeighbors])),
                               meshNeighborNominalFlowRate(    (0 == localNumberOfMeshElements * maximumNumberOfMeshNeighbors) ? (NULL) : (new                      double[localNumberOfMeshElements * maximumNumberOfMeshNeighbors])),
                               meshNeighborExpirationTime(     (0 == localNumberOfMeshElements * maximumNumberOfMeshNeighbors) ? (NULL) : (new                      double[localNumberOfMeshElements * maximumNumberOfMeshNeighbors])),
                               meshNeighborInflowCumulative(   (0 == localNumberOfMeshElements * maximumNumberOfMeshNeighbors) ? (NULL) : (new                      double[localNumberOfMeshElements * maximumNumberOfMeshNeighbors])),
                               meshNeighborOutflowCumulative(  (0 == localNumberOfMeshElements * maximumNumberOfMeshNeighbors) ? (NULL) : (new                      double[localNumberOfMeshElements * maximumNumberOfMeshNeighbors])),
                               channelEvapoTranspirationState(    (0 == localNumberOfChannelElements)                                   ? (NULL) : (new EvapoTranspirationStateBlob[localNumberOfChannelElements])),
                               channelSurfaceWater(               (0 == localNumberOfChannelElements)                                   ? (NULL) : (new                      double[localNumberOfChannelElements])),
                               channelSurfaceWaterCreated(        (0 == localNumberOfChannelElements)                                   ? (NULL) : (new                      double[localNumberOfChannelElements])),
                               channelPrecipitationRate(          (0 == localNumberOfChannelElements)                                   ? (NULL) : (new                      double[localNumberOfChannelElements])),
                               channelPrecipitationCumulative(    (0 == localNumberOfChannelElements)                                   ? (NULL) : (new                      double[localNumberOfChannelElements])),
                               channelEvaporationRate(            (0 == localNumberOfChannelElements)                                   ? (NULL) : (new                      double[localNumberOfChannelElements])),
                               channelEvaporationCumulative(      (0 == localNumberOfChannelElements)                                   ? (NULL) : (new                      double[localNumberOfChannelElements])),
                               channelSnowWaterEquivalent(        (0 == localNumberOfChannelElements)                                   ? (NULL) : (new                      double[localNumberOfChannelElements])),
                               channelNeighborLocalEndpoint(      (0 == localNumberOfChannelElements * maximumNumberOfChannelNeighbors) ? (NULL) : (new        NeighborEndpointEnum[localNumberOfChannelElements * maximumNumberOfChannelNeighbors])),
                               channelNeighborRemoteEndpoint(     (0 == localNumberOfChannelElements * maximumNumberOfChannelNeighbors) ? (NULL) : (new        NeighborEndpointEnum[localNumberOfChannelElements * maximumNumberOfChannelNeighbors])),
                               channelNeighborRemoteElementNumber((0 == localNumberOfChannelElements * maximumNumberOfChannelNeighbors) ? (NULL) : (new                      size_t[localNumberOfChannelElements * maximumNumberOfChannelNeighbors])),
                               channelNeighborNominalFlowRate(    (0 == localNumberOfChannelElements * maximumNumberOfChannelNeighbors) ? (NULL) : (new                      double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors])),
                               channelNeighborExpirationTime(     (0 == localNumberOfChannelElements * maximumNumberOfChannelNeighbors) ? (NULL) : (new                      double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors])),
                               channelNeighborInflowCumulative(   (0 == localNumberOfChannelElements * maximumNumberOfChannelNeighbors) ? (NULL) : (new                      double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors])),
                               channelNeighborOutflowCumulative(  (0 == localNumberOfChannelElements * maximumNumberOfChannelNeighbors) ? (NULL) : (new                      double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors]))
{
    size_t ii; // Loop counter.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(localMeshElementStart + localNumberOfMeshElements <= globalNumberOfMeshElements))
        {
            CkError("ERROR in TimePointState::TimePointState: localMeshElementStart plus localNumberOfMeshElements must be less than or equal to globalNumberOfMeshElements.\n");
            CkExit();
        }
        
        if (!(localChannelElementStart + localNumberOfChannelElements <= globalNumberOfChannelElements))
        {
            CkError("ERROR in TimePointState::TimePointState: localChannelElementStart plus localNumberOfChannelElements must be less than or equal to globalNumberOfChannelElements.\n");
            CkExit();
        }
    }
    
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    // Initialize received flags to false.
    for (ii = 0; ii < localNumberOfMeshElements; ++ii)
    {
        meshStateReceived[ii] = false;
    }
    
    for (ii = 0; ii < localNumberOfChannelElements; ++ii)
    {
        channelStateReceived[ii] = false;
    }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    
    for (ii = 0; ii < localNumberOfMeshElements * maximumNumberOfMeshNeighbors; ++ii)
    {
        meshNeighborLocalEndpoint[      ii] = NO_NEIGHBOR;
        meshNeighborRemoteEndpoint[     ii] = NO_NEIGHBOR;
        meshNeighborRemoteElementNumber[ii] = 0;
        meshNeighborNominalFlowRate[    ii] = 0.0;
        meshNeighborExpirationTime[     ii] = 0.0;
        meshNeighborInflowCumulative[   ii] = 0.0;
        meshNeighborOutflowCumulative[  ii] = 0.0;
    }
    
    for (ii = 0; ii < localNumberOfChannelElements * maximumNumberOfChannelNeighbors; ++ii)
    {
        channelNeighborLocalEndpoint[      ii] = NO_NEIGHBOR;
        channelNeighborRemoteEndpoint[     ii] = NO_NEIGHBOR;
        channelNeighborRemoteElementNumber[ii] = 0;
        channelNeighborNominalFlowRate[    ii] = 0.0;
        channelNeighborExpirationTime[     ii] = 0.0;
        channelNeighborInflowCumulative[   ii] = 0.0;
        channelNeighborOutflowCumulative[  ii] = 0.0;
    }
}

TimePointState::~TimePointState()
{
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    delete[] meshStateReceived;
    delete[] channelStateReceived;
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    delete[] meshEvapoTranspirationState;
    delete[] meshSurfaceWater;
    delete[] meshSurfaceWaterCreated;
    delete[] meshGroundwaterMode;
    delete[] meshPerchedHead;
    delete[] meshSoilWater;
    delete[] meshSoilWaterCreated;
    delete[] meshAquiferHead;
    delete[] meshAquiferWater;
    delete[] meshAquiferWaterCreated;
    delete[] meshDeepGroundwater;
    delete[] meshPrecipitationRate;
    delete[] meshPrecipitationCumulative;
    delete[] meshEvaporationRate;
    delete[] meshEvaporationCumulative;
    delete[] meshTranspirationRate;
    delete[] meshTranspirationCumulative;
    delete[] meshCanopyWaterEquivalent;
    delete[] meshSnowWaterEquivalent;
    delete[] meshRootZoneWater;
    delete[] meshTotalGroundwater;
    delete[] meshNeighborLocalEndpoint;
    delete[] meshNeighborRemoteEndpoint;
    delete[] meshNeighborRemoteElementNumber;
    delete[] meshNeighborNominalFlowRate;
    delete[] meshNeighborExpirationTime;
    delete[] meshNeighborInflowCumulative;
    delete[] meshNeighborOutflowCumulative;
    delete[] channelEvapoTranspirationState;
    delete[] channelSurfaceWater;
    delete[] channelSurfaceWaterCreated;
    delete[] channelPrecipitationRate;
    delete[] channelPrecipitationCumulative;
    delete[] channelEvaporationRate;
    delete[] channelEvaporationCumulative;
    delete[] channelSnowWaterEquivalent;
    delete[] channelNeighborLocalEndpoint;
    delete[] channelNeighborRemoteEndpoint;
    delete[] channelNeighborRemoteElementNumber;
    delete[] channelNeighborNominalFlowRate;
    delete[] channelNeighborExpirationTime;
    delete[] channelNeighborInflowCumulative;
    delete[] channelNeighborOutflowCumulative;
}

bool TimePointState::receiveMeshState(const MeshState& state)
{
    bool                                       error      = false;                                       // Error flag.
    int                                        ii;                                                       // Loop counter.
    std::vector<NeighborState>::const_iterator it;                                                       // Loop iterator.
    size_t                                     localIndex = state.elementNumber - localMeshElementStart; // The array index in the TimePointState for this data.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(localMeshElementStart <= state.elementNumber && state.elementNumber < localMeshElementStart + localNumberOfMeshElements))
        {
            CkError("ERROR in TimePointState::receiveMeshState: received state from a mesh element I do not own.\n");
            error = true;
        }
        
        if (!(state.neighbors.size() <= maximumNumberOfMeshNeighbors))
        {
            CkError("ERROR in TimePointState::receiveMeshState: received state from a mesh element with more neighbors than maximumNumberOfMeshNeighbors.\n");
            error = true;
        }
    }
    
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    if (!error)
    {
        if (!(!meshStateReceived[localIndex]))
        {
            CkError("ERROR in TimePointState::receiveMeshState: received duplicate state from a mesh element.\n");
            error = true;
        }
        else
        {
            meshStateReceived[localIndex] = true;
        }
    }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    
    if (!error)
    {
        elementsReceived++;
        meshEvapoTranspirationState[localIndex].noahMPStateBlob            = state.evapoTranspirationState;
        meshSurfaceWater[           localIndex]                            = state.surfaceWater;
        meshSurfaceWaterCreated[    localIndex]                            = state.surfaceWaterCreated;
        meshGroundwaterMode[        localIndex]                            = state.groundwaterMode;
        meshPerchedHead[            localIndex]                            = state.perchedHead;
        meshSoilWater[              localIndex].simpleGroundwaterStateBlob = state.soilWater;
        meshSoilWaterCreated[       localIndex]                            = state.soilWaterCreated;
        meshAquiferHead[            localIndex]                            = state.aquiferHead;
        meshAquiferWater[           localIndex].simpleGroundwaterStateBlob = state.aquiferWater;
        meshAquiferWaterCreated[    localIndex]                            = state.aquiferWaterCreated;
        meshDeepGroundwater[        localIndex]                            = state.deepGroundwater;
        meshPrecipitationRate[      localIndex]                            = state.precipitationRate;
        meshPrecipitationCumulative[localIndex]                            = state.precipitationCumulative;
        meshEvaporationRate[        localIndex]                            = state.evaporationRate;
        meshEvaporationCumulative[  localIndex]                            = state.evaporationCumulative;
        meshTranspirationRate[      localIndex]                            = state.transpirationRate;
        meshTranspirationCumulative[localIndex]                            = state.transpirationCumulative;
        meshCanopyWaterEquivalent[  localIndex]                            = (state.evapoTranspirationState.canLiq + state.evapoTranspirationState.canIce) / 1000.0; // Divide by 1000.0 to convert from millimeters to meters.
        meshSnowWaterEquivalent[    localIndex]                            = state.evapoTranspirationState.snEqv / 1000.0;                                           // Divide by 1000.0 to convert from millimeters to meters.
        meshRootZoneWater[          localIndex]                            = state.soilWater.waterAboveDepth(state.soilWater.getThickness()); // FIXME define the root zone.
        meshTotalGroundwater[       localIndex]                            = state.soilWater.waterAboveDepth(state.soilWater.getThickness()) + state.aquiferWater.waterAboveDepth(state.aquiferWater.getThickness());
        
        for (it = state.neighbors.begin(), ii = 0; it != state.neighbors.end(); ++it, ++ii)
        {
            meshNeighborLocalEndpoint[      localIndex * maximumNumberOfMeshNeighbors + ii] = it->localEndpoint;
            meshNeighborRemoteEndpoint[     localIndex * maximumNumberOfMeshNeighbors + ii] = it->remoteEndpoint;
            meshNeighborRemoteElementNumber[localIndex * maximumNumberOfMeshNeighbors + ii] = it->remoteElementNumber;
            meshNeighborNominalFlowRate[    localIndex * maximumNumberOfMeshNeighbors + ii] = it->nominalFlowRate;
            meshNeighborExpirationTime[     localIndex * maximumNumberOfMeshNeighbors + ii] = it->expirationTime;
            meshNeighborInflowCumulative[   localIndex * maximumNumberOfMeshNeighbors + ii] = it->inflowCumulative;
            meshNeighborOutflowCumulative[  localIndex * maximumNumberOfMeshNeighbors + ii] = it->outflowCumulative;
        }
    }
    
    return error;
}

bool TimePointState::receiveChannelState(const ChannelState& state)
{
    bool                                       error      = false;                                       // Error flag.
    int                                        ii;                                                       // Loop counter.
    std::vector<NeighborState>::const_iterator it;                                                       // Loop iterator.
    size_t                                     localIndex = state.elementNumber - localMeshElementStart; // The array index in the TimePointState for this data.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(localChannelElementStart <= state.elementNumber && state.elementNumber < localChannelElementStart + localNumberOfChannelElements))
        {
            CkError("ERROR in TimePointState::receiveChannelState: received state from a channel element I do not own.\n");
            error = true;
        }
        
        if (!(state.neighbors.size() <= maximumNumberOfChannelNeighbors))
        {
            CkError("ERROR in TimePointState::receiveChannelState: received state from a channel element with more neighbors than maximumNumberOfChannelNeighbors.\n");
            error = true;
        }
    }
    
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    if (!error)
    {
        if (!(!channelStateReceived[localIndex]))
        {
            CkError("ERROR in TimePointState::receiveChannelState: received duplicate state from a channel element.\n");
            error = true;
        }
        else
        {
            channelStateReceived[localIndex] = true;
        }
    }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    
    if (!error)
    {
        elementsReceived++;
        channelEvapoTranspirationState[localIndex].noahMPStateBlob = state.evapoTranspirationState;
        channelSurfaceWater[           localIndex]                 = state.surfaceWater;
        channelSurfaceWaterCreated[    localIndex]                 = state.surfaceWaterCreated;
        channelPrecipitationRate[      localIndex]                 = state.precipitationRate;
        channelPrecipitationCumulative[localIndex]                 = state.precipitationCumulative;
        channelEvaporationRate[        localIndex]                 = state.evaporationRate;
        channelEvaporationCumulative[  localIndex]                 = state.evaporationCumulative;
        channelSnowWaterEquivalent[    localIndex]                 = state.evapoTranspirationState.snEqv / 1000.0; // Divide by 1000.0 to convert from millimeters to meters.
        
        for (it = state.neighbors.begin(), ii = 0; it != state.neighbors.end(); ++it, ++ii)
        {
            channelNeighborLocalEndpoint[      localIndex * maximumNumberOfChannelNeighbors + ii] = it->localEndpoint;
            channelNeighborRemoteEndpoint[     localIndex * maximumNumberOfChannelNeighbors + ii] = it->remoteEndpoint;
            channelNeighborRemoteElementNumber[localIndex * maximumNumberOfChannelNeighbors + ii] = it->remoteElementNumber;
            channelNeighborNominalFlowRate[    localIndex * maximumNumberOfChannelNeighbors + ii] = it->nominalFlowRate;
            channelNeighborExpirationTime[     localIndex * maximumNumberOfChannelNeighbors + ii] = it->expirationTime;
            channelNeighborInflowCumulative[   localIndex * maximumNumberOfChannelNeighbors + ii] = it->inflowCumulative;
            channelNeighborOutflowCumulative[  localIndex * maximumNumberOfChannelNeighbors + ii] = it->outflowCumulative;
        }
    }
    
    return error;
}
