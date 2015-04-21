#include "channel_element.h"
#include "adhydro.h"
#include "surfacewater.h"
#include "groundwater.h"

bool ChannelElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                                          std::vector<ChannelSurfacewaterMeshNeighborProxy>::size_type neighborProxyIndex,
                                                                          double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < meshNeighbors.size()))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: neighborProxyIndex must be greater than or equal to zero and "
              "less than meshNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      error = surfacewaterMeshChannelFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                              meshNeighbors[neighborProxyIndex].edgeLength,
                                              meshNeighbors[neighborProxyIndex].neighborZSurface + meshNeighbors[neighborProxyIndex].neighborZOffset,
                                              meshNeighbors[neighborProxyIndex].neighborArea, neighborSurfacewaterDepth, elementZBank, elementZBed, baseWidth,
                                              sideSlope, surfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      meshNeighbors[neighborProxyIndex].nominalFlowRate *= -1.0; // Use negative of flow rate so that positive means flow out of the channel.
      meshNeighbors[neighborProxyIndex].expirationTime   = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool ChannelElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor(double currentTime, double regionalDtLimit,
                                                                             std::vector<ChannelSurfacewaterChannelNeighborProxy>::size_type
                                                                             neighborProxyIndex, double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < channelNeighbors.size()))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: neighborProxyIndex must be greater than or equal to zero and "
              "less than channelNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      if (isBoundary(channelNeighbors[neighborProxyIndex].neighbor))
        {
          // FIXME what to do about inflowVelocity and inflowHeight?
          error = surfacewaterChannelBoundaryFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                                      (BoundaryConditionEnum)channelNeighbors[neighborProxyIndex].neighbor, 0.0, 0.0, elementLength, baseWidth,
                                                      sideSlope, surfacewaterDepth);
        }
      else
        {
          error = surfacewaterChannelChannelFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit, channelType, elementZBank, elementZBed,
                                                     elementLength, baseWidth, sideSlope, manningsN, surfacewaterDepth,
                                                     channelNeighbors[neighborProxyIndex].neighborChannelType,
                                                     channelNeighbors[neighborProxyIndex].neighborZBank, channelNeighbors[neighborProxyIndex].neighborZBed,
                                                     channelNeighbors[neighborProxyIndex].neighborLength,
                                                     channelNeighbors[neighborProxyIndex].neighborBaseWidth,
                                                     channelNeighbors[neighborProxyIndex].neighborSideSlope,
                                                     channelNeighbors[neighborProxyIndex].neighborManningsN, neighborSurfacewaterDepth);
        }
    }
  
  // Calculate expiration time.
  if (!error)
    {
      channelNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                                         std::vector<ChannelGroundwaterMeshNeighborProxy>::size_type neighborProxyIndex,
                                                                         double neighborSurfacewaterDepth, double neighborGroundwaterHead)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < undergroundMeshNeighbors.size()))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborProxyIndex must be greater than or equal to zero and "
              "less than undergroundMeshNeighbors.size().\n");
      error = true;
    }
  else if (!(undergroundMeshNeighbors[neighborProxyIndex].neighborZSurface >= neighborGroundwaterHead))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborGroundwaterHead must be less than or equal to "
              "undergroundMeshNeighbors[neighborProxyIndex].neighborZSurface.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      // FIXME figure out how to calculate dtNew
      error = groundwaterMeshChannelFlowRate(&undergroundMeshNeighbors[neighborProxyIndex].nominalFlowRate,
                                             undergroundMeshNeighbors[neighborProxyIndex].edgeLength,
                                             undergroundMeshNeighbors[neighborProxyIndex].neighborZSurface +
                                             undergroundMeshNeighbors[neighborProxyIndex].neighborZOffset,
                                             undergroundMeshNeighbors[neighborProxyIndex].neighborLayerZBottom +
                                             undergroundMeshNeighbors[neighborProxyIndex].neighborZOffset, neighborSurfacewaterDepth,
                                             neighborGroundwaterHead + undergroundMeshNeighbors[neighborProxyIndex].neighborZOffset, elementZBank, elementZBed,
                                             baseWidth, sideSlope, bedConductivity, bedThickness, surfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      undergroundMeshNeighbors[neighborProxyIndex].nominalFlowRate *= -1.0; // Use negative of flow rate so that positive means flow out of the channel.
      undergroundMeshNeighbors[neighborProxyIndex].expirationTime   = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool ChannelElement::doPointProcessesAndSendOutflows(double referenceDate, double currentTime, double timestepEndTime)
{
  bool   error                   = false;                                                           // Error flag.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;                            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;                         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh;                 // Loop iterator.
  double dt                      = timestepEndTime - currentTime;                                   // Seconds.
  double totalOutwardFlowRate    = 0.0;                                                             // Sum of all outward flow rates in cubic meters per
                                                                                                    // second.
  double outwardFlowRateFraction = 1.0;                                                             // Fraction of all outward flow rates that can be
                                                                                                    // satisfied, unitless.
  double area                    = surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth); // Wetted cross sectional area of channel in square meters.
  double waterSent;                                                                                 // Cubic meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(1721425.5 <= referenceDate))
    {
      CkError("ERROR in ChannelElement::doPointProcessesAndSendOutflows: referenceDate must not be before 1 CE.\n");
      error = true;
    }
  
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in ChannelElement::doPointProcessesAndSendOutflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // FIXME Do point processes for rainfall, snowmelt, and evapo-transpiration without double counting area.
  
  if (!error)
    {
      // Limit surfacewater and groundwater outflows.
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 < (*itMesh).nominalFlowRate)
            {
              totalOutwardFlowRate += (*itMesh).nominalFlowRate;
            }
        }

      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 < (*itChannel).nominalFlowRate)
            {
              totalOutwardFlowRate += (*itChannel).nominalFlowRate;
            }
        }

      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 < (*itUndergroundMesh).nominalFlowRate)
            {
              totalOutwardFlowRate += (*itUndergroundMesh).nominalFlowRate;
            }
        }

      if (area * elementLength < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = area * elementLength / (totalOutwardFlowRate * dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Send surfacewater and groundwater outflows taking water from area.
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 < (*itMesh).nominalFlowRate)
            {
              waterSent  = (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction;
              area      -= waterSent / elementLength;

              // FIXME call the region's function to send the water.
              //region->sendWater(MeshSurfacewaterMeshNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction));
            }
        }

      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 < (*itChannel).nominalFlowRate)
            {
              waterSent  = (*itChannel).nominalFlowRate * dt * outwardFlowRateFraction;
              area      -= waterSent / elementLength;

              // FIXME call the region's function to send the water.
              //region->sendWater(MeshSurfacewaterChannelNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction));
            }
        }

      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 < (*itUndergroundMesh).nominalFlowRate)
            {
              waterSent  = (*itUndergroundMesh).nominalFlowRate * dt * outwardFlowRateFraction;
              area      -= waterSent / elementLength;

              // FIXME call the region's function to send the water.
              //region->sendWater(MeshSurfacewaterMeshNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction));
            }
        }
      
      // Even though we are limiting outflows, area can go below zero due to roundoff error.
      if (0.0 > area)
        {
    #if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0, area));
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          surfacewaterError -= area * elementLength;
          area               = 0.0;
        }

      // Convert cross sectional area back to water depth.
      calculateSurfacewaterDepthFromArea(area);
    }
  
  return error;
}

bool ChannelElement::allInflowsArrived(double currentTime, double timestepEndTime)
{
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh; // Loop iterator.
  bool allArrived = true;                                                           // Whether all material has arrived from all neighbors.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in ChannelElement::allInflowsArrived: currentTime must be less than or equal to timestepEndTime.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call allMaterialHasArrived.  allInflowsArrived shouldn't be called in this situation, but if it is return
  // that all inflows have arrived for the next zero seconds.
  if (currentTime < timestepEndTime)
    {
      for (itMesh = meshNeighbors.begin(); allArrived && itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              allArrived = (*itMesh).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
      
      for (itChannel = channelNeighbors.begin(); allArrived && itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              allArrived = (*itChannel).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
      
      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); allArrived && itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 > (*itUndergroundMesh).nominalFlowRate)
            {
              allArrived = (*itUndergroundMesh).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
    }
  
  return allArrived;
}

bool ChannelElement::receiveInflows(double currentTime, double timestepEndTime)
{
  bool   error = false;                                                             // Error flag.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh; // Loop iterator.
  double area  = surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth);   // Wetted cross sectional area of channel in square meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in ChannelElement::receiveInflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call getMaterial.  advanceTime shouldn't be called in this situation, but if it is we don't have to do
  // anything to receive inflows for the next zero seconds.
  if (!error && currentTime < timestepEndTime)
    {
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              area += (*itMesh).getMaterial(currentTime, timestepEndTime) / elementLength;
            }
        }
      
      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              area += (*itChannel).getMaterial(currentTime, timestepEndTime) / elementLength;
            }
        }
      
      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 > (*itUndergroundMesh).nominalFlowRate)
            {
              area += (*itUndergroundMesh).getMaterial(currentTime, timestepEndTime) / elementLength;
            }
        }

      // Convert cross sectional area back to water depth.
      calculateSurfacewaterDepthFromArea(area);

      // If in drainDownMode cut off surface water to channel bank depth.
      if (ADHydro::drainDownMode && surfacewaterDepth > (elementZBank - elementZBed))
        {
          surfacewaterDepth = (elementZBank - elementZBed);
        }
    }
  
  return error;
}

void ChannelElement::calculateSurfacewaterDepthFromArea(double area)
{
  double bOverTwoS; // Temporary value reused in computation.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(0.0 <= area);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  if (0.0 < sideSlope)
    {
      // Trapezoidal or triangular channel.
      bOverTwoS = baseWidth / (2.0 * sideSlope);

      surfacewaterDepth = sqrt(area / sideSlope + bOverTwoS * bOverTwoS) - bOverTwoS;
      
      // For very small areas I wonder if surfacewaterDepth might be able to come out negative due to roundoff error.  I wasn't able to convince myself that it
      // couldn't possibly happen so I'm leaving this in.  I don't add anything to surfacewaterError because I already know that area wasn't negative so it's
      // not really missing water.
      if (0.0 > surfacewaterDepth)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0, surfacewaterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          surfacewaterDepth = 0.0;
        }
    }
  else
    {
      // Rectangular channel.  By invariant baseWidth and sideSlope can't both be zero.
      surfacewaterDepth = area / baseWidth;
    }
}
