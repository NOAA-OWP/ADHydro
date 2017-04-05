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
        if (!(localNumberOfMeshElements <= globalNumberOfMeshElements))
        {
            CkError("ERROR in TimePointState::TimePointState: localNumberOfMeshElements must be less than or equal to globalNumberOfMeshElements.\n");
            CkExit();
        }
        
        if (!((0 != localNumberOfMeshElements && localMeshElementStart < globalNumberOfMeshElements) || (0 == localNumberOfMeshElements && 0 == localMeshElementStart)))
        {
            CkError("ERROR in TimePointState::TimePointState: localMeshElementStart must be less than globalNumberOfMeshElements or zero if localNumberOfMeshElements is zero.\n");
            CkExit();
        }
        
        if (!(localNumberOfChannelElements <= globalNumberOfChannelElements))
        {
            CkError("ERROR in TimePointState::TimePointState: localNumberOfChannelElements must be less than or equal to globalNumberOfChannelElements.\n");
            CkExit();
        }
        
        if (!((0 != localNumberOfChannelElements && localChannelElementStart < globalNumberOfChannelElements) || (0 == localNumberOfChannelElements && 0 == localChannelElementStart)))
        {
            CkError("ERROR in TimePointState::TimePointState: localChannelElementStart must be less than globalNumberOfChannelElements or zero if localNumberOfChannelElements is zero.\n");
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
