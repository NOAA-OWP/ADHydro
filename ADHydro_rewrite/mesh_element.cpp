#include "mesh_element.h"
#include "adhydro.h"
#include "surfacewater.h"
#include "groundwater.h"
#include "garto.h"

bool InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                                                     std::vector<MeshGroundwaterMeshNeighborProxy>::size_type
                                                                                     neighborProxyIndex, double elementX, double elementY,
                                                                                     double elementZSurface, double elementArea,
                                                                                     double elementSurfacewaterDepth, double neighborSurfacewaterDepth,
                                                                                     double neighborGroundwaterHead)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < meshNeighbors.size()))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborProxyIndex must be greater than or equal to "
              "zero and less than meshNeighbors.size().\n");
      error = true;
    }
  else if (!(meshNeighbors[neighborProxyIndex].neighborZSurface >= neighborGroundwaterHead))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborGroundwaterHead must be less than or equal "
              "to meshNeighbors[neighborProxyIndex].neighborZSurface.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: elementSurfacewaterDepth must be greater than or "
              "equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborSurfacewaterDepth must be greater than or "
              "equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      if (isBoundary(meshNeighbors[neighborProxyIndex].neighbor))
        {
          // FIXME what to do about inflowHeight?
          error = groundwaterMeshBoundaryFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                                  (BoundaryConditionEnum)meshNeighbors[neighborProxyIndex].neighbor, 0.0,
                                                  meshNeighbors[neighborProxyIndex].edgeLength, meshNeighbors[neighborProxyIndex].edgeNormalX,
                                                  meshNeighbors[neighborProxyIndex].edgeNormalY, layerZBottom, elementArea, slopeX, slopeY, conductivity,
                                                  porosity, groundwaterHead);
        }
      else
        {
          error = groundwaterMeshMeshFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                              meshNeighbors[neighborProxyIndex].edgeLength, elementX, elementY, elementZSurface, layerZBottom, elementArea,
                                              conductivity, porosity, elementSurfacewaterDepth, groundwaterHead, meshNeighbors[neighborProxyIndex].neighborX,
                                              meshNeighbors[neighborProxyIndex].neighborY, meshNeighbors[neighborProxyIndex].neighborZSurface,
                                              meshNeighbors[neighborProxyIndex].neighborLayerZBottom, meshNeighbors[neighborProxyIndex].neighborArea,
                                              meshNeighbors[neighborProxyIndex].neighborConductivity, meshNeighbors[neighborProxyIndex].neighborPorosity,
                                              neighborSurfacewaterDepth, neighborGroundwaterHead);
        }
    }
  
  // Calculate expiration time.
  if (!error)
    {
      meshNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor(double currentTime, double regionalDtLimit,
                                                                                        std::vector<MeshGroundwaterChannelNeighborProxy>::size_type
                                                                                        neighborProxyIndex, double elementZSurface,
                                                                                        double elementSurfacewaterDepth, double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < channelNeighbors.size()))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor: neighborProxyIndex must be greater than or equal "
              "to zero and less than channelNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor: elementSurfacewaterDepth must be greater than or "
              "equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor: neighborSurfacewaterDepth must be greater than or "
              "equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      // FIXME calculate dtNew
      error = groundwaterMeshChannelFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, channelNeighbors[neighborProxyIndex].edgeLength,
                                             elementZSurface, layerZBottom, elementSurfacewaterDepth, groundwaterHead,
                                             channelNeighbors[neighborProxyIndex].neighborZBank, channelNeighbors[neighborProxyIndex].neighborZBed,
                                             channelNeighbors[neighborProxyIndex].neighborBaseWidth, channelNeighbors[neighborProxyIndex].neighborSideSlope,
                                             channelNeighbors[neighborProxyIndex].neighborBedConductivity,
                                             channelNeighbors[neighborProxyIndex].neighborBedThickness, neighborSurfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      channelNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool InfiltrationAndGroundwater::fillInEvapoTranspirationSoilMoistureStruct(double elementZSurface, float zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS],
                                                                            EvapoTranspirationSoilMoistureStruct& evapoTranspirationSoilMoisture)
{
  bool   error = false;                                           // Error flag.
  int    ii;                                                      // Loop counter.
  double layerMiddleDepth;                                        // Meters.
  double distanceAboveWaterTable;                                 // Meters.
  double relativeSaturation;                                      // Unitless.
  double waterContent[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS]; // Water content for GARTO_INFILTRATION.
  double soilDepthZ[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];   // Soil depth in GARTO_INFILTRATION in meters, positive downward.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  for (ii = EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
    {
      if (!(zSnso[ii - 1] > zSnso[ii]))
        {
          CkError("ERROR in InfiltrationAndGroundwater::fillInEvapoTranspirationSoilMoistureStruct, soil layer %d: zSnso must be less than the layer above "
                  "it.\n", ii - EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS);
          error = true;
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      if (NO_INFILTRATION == infiltrationMethod)
        {
          evapoTranspirationSoilMoisture.zwt = 0.0f;

          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
            {
              evapoTranspirationSoilMoisture.smcEq[ii] = porosity * 0.01;
              evapoTranspirationSoilMoisture.sh2o[ii]  = porosity * 0.01;
              evapoTranspirationSoilMoisture.smc[ii]   = porosity * 0.01;
            }
        }
      else if (TRIVIAL_INFILTRATION == infiltrationMethod)
        {
          evapoTranspirationSoilMoisture.zwt = elementZSurface - groundwaterHead;

          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
            {
              layerMiddleDepth        = 0.5 * (zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] +
                                               zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1]);
              distanceAboveWaterTable = evapoTranspirationSoilMoisture.zwt + layerMiddleDepth; // Plus because layerMiddleDepth is negative.

              // FIXME use Van Genutchen or Brooks-Corey?
              if (0.1 >= distanceAboveWaterTable)
                {
                  relativeSaturation = 1.0;
                }
              else
                {
                  relativeSaturation = 1.0 - (log10(distanceAboveWaterTable) + 1.0) * 0.3;

                  if (0.01 > relativeSaturation)
                    {
                      relativeSaturation = 0.01;
                    }
                }

              evapoTranspirationSoilMoisture.smcEq[ii] = porosity * relativeSaturation;
              evapoTranspirationSoilMoisture.sh2o[ii]  = porosity * relativeSaturation;
              evapoTranspirationSoilMoisture.smc[ii]   = porosity * relativeSaturation;
            }
        }
      else if (GARTO_INFILTRATION == infiltrationMethod)
        {
          evapoTranspirationSoilMoisture.zwt = elementZSurface - groundwaterHead;
          
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
            {
              // FIXME is this necessary?  From the comments waterContent is an output of get_garto_domain_water_content, but Wencong put this here so is it
              // actually an in/out parameter, or did he just do this while get_garto_domain_water_content is commented out?
              waterContent[ii] = porosity;
              
              // soilDepthZ is positive downward, zSnso is negative downward.
              // FIXME this is wrong if the comment in garto.h is correct that it should contain each element's lower bound
              soilDepthZ[ii]   = -0.5 * (zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] + zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1]);
            }
          
          get_garto_domain_water_content((garto_domain*)vadoseZoneState, waterContent, soilDepthZ, EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS);
          
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
            {
              evapoTranspirationSoilMoisture.smcEq[ii] = waterContent[ii]; // FIXME no, GARTO is not necessarily at equilibrium.
              evapoTranspirationSoilMoisture.sh2o[ii]  = waterContent[ii];
              evapoTranspirationSoilMoisture.smc[ii]   = waterContent[ii];
            }
        }

      // For all methods make the lowest layer water content extend down forever.
      evapoTranspirationSoilMoisture.smcwtd = evapoTranspirationSoilMoisture.smc[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS - 1];
    }
  
  return error;
}

double InfiltrationAndGroundwater::evaporate(double unsatisfiedEvaporation)
{
  double evaporation = 0.0; // Return value.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= unsatisfiedEvaporation))
    {
      CkError("ERROR in InfiltrationAndGroundwater::evaporate: unsatisfiedEvaporation must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // if (NO_INFILTRATION == infiltrationMethod) return zero.
  if (TRIVIAL_INFILTRATION == infiltrationMethod)
    {
      if (DEEP_AQUIFER == groundwaterMethod)
        {
          evaporation          = unsatisfiedEvaporation;
          groundwaterRecharge -= unsatisfiedEvaporation;
        }
      else if (SHALLOW_AQUIFER == groundwaterMethod)
        {
          if (groundwaterHead - unsatisfiedEvaporation / porosity >= layerZBottom)
            {
              evaporation      = unsatisfiedEvaporation;
              groundwaterHead -= unsatisfiedEvaporation / porosity;
            }
          else
            {
              evaporation     = (groundwaterHead - layerZBottom) * porosity;
              groundwaterHead = layerZBottom;
            }
        }
    }
  else if (GARTO_INFILTRATION == infiltrationMethod)
    {
      evaporation = garto_evapotranspiration((garto_domain*)vadoseZoneState, unsatisfiedEvaporation, 0.0, 0.0); // FIXME root_depth?
    }
    
  return evaporation;
}

double InfiltrationAndGroundwater::transpire(double unsatisfiedTranspiration)
{
  double transpiration = 0.0; // Return value.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= unsatisfiedTranspiration))
    {
      CkError("ERROR in InfiltrationAndGroundwater::transpire: unsatisfiedTranspiration must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // if (NO_INFILTRATION == infiltrationMethod) return zero.
  if (TRIVIAL_INFILTRATION == infiltrationMethod)
    {
      // With no vadose zone state transpiration water comes from the same place as evaporation water.
      transpiration = evaporate(unsatisfiedTranspiration);
    }
  else if (GARTO_INFILTRATION == infiltrationMethod)
    {
      transpiration = garto_evapotranspiration((garto_domain*)vadoseZoneState, 0.0, unsatisfiedTranspiration, 0.0); // FIXME root_depth?
    }
  return transpiration;
}

bool InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows(double currentTime, double timestepEndTime, double elementZSurface,
                                                                          double elementArea, double& surfacewaterDepth)
{
  bool   error                   = false;                               // Error flag.
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  double dt                      = timestepEndTime - currentTime;       // Seconds
  double groundwaterAvailable    = 0.0;                                 // Groundwater available to satisfy outflows in cubic meters of water.
  double totalOutwardFlowRate    = 0.0;                                 // Sum of all outward flow rates in cubic meters per second.
  double outwardFlowRateFraction = 1.0;                                 // Fraction of all outward flow rates that can be satisfied, unitless.
  double waterSent;                                                     // Cubic meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // Do infiltration.
      // if (NO_INFILTRATION == infiltrationMethod) do nothing.
      if (TRIVIAL_INFILTRATION == infiltrationMethod)
        {
          if (surfacewaterDepth >= conductivity * dt)
            {
              groundwaterRecharge += conductivity * dt;
              surfacewaterDepth   -= conductivity * dt;
            }
          else
            {
              groundwaterRecharge += surfacewaterDepth;
              surfacewaterDepth    = 0.0;
            }
        }
      else if (GARTO_INFILTRATION == infiltrationMethod)
        {
          garto_timestep((garto_domain*)vadoseZoneState, dt, &surfacewaterDepth, elementZSurface - groundwaterHead, &groundwaterRecharge);
        }

      // Do groundwater outflows and resolve groundwaterRecharge.
      // if (NO_AQUIFER == groundwaterMethod) do nothing.
      if (DEEP_AQUIFER == groundwaterMethod)
        {
          // FIXME send groundwaterRecharge to an aquifer storage bucket.  For now, water just accumulates in groundwaterRecharge.
        }
      else if (SHALLOW_AQUIFER == groundwaterMethod)
        {
          // Limit groundwater outflows.
          if (groundwaterHead > layerZBottom)
            {
              groundwaterAvailable = (groundwaterRecharge + (groundwaterHead - layerZBottom) * porosity) * elementArea;
            }

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

          if (groundwaterAvailable < totalOutwardFlowRate * dt)
            {
              outwardFlowRateFraction = groundwaterAvailable / (totalOutwardFlowRate * dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            }

          // Send groundwater outflows taking water from groundwaterRecharge.
          for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
            {
              if (0.0 < (*itMesh).nominalFlowRate)
                {
                  waterSent            = (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction;
                  groundwaterRecharge -= waterSent / elementArea;

                  // FIXME call the region's function to send the water.
                  //region->sendGroundwaterToMeshElement((*itMesh).region, (*itMesh).neighbor, (*itMesh).reciprocalNeighborProxy,
                  //                                     MeshGroundwaterMeshNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, waterSent));
                }
            }

          for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
            {
              if (0.0 < (*itChannel).nominalFlowRate)
                {
                  waterSent            = (*itChannel).nominalFlowRate * dt * outwardFlowRateFraction;
                  groundwaterRecharge -= waterSent / elementArea;

                  // FIXME call the region's function to send the water.
                  //region->sendWaterToChannelElement((*itChannel).region, (*itChannel).neighbor, (*itChannel).reciprocalNeighborProxy,
                  //                                  MeshGroundwaterChannelNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, waterSent));
                }
            }

          // Update groundwaterHead based on net groundwaterRecharge and take or put groundwaterRecharge water back into the domain, or if the domain is full put it
          // back in surfacewaterDepth.
          // NO_INFILTRATION cannot be used with SHALLOW_AQUIFER.
          if (TRIVIAL_INFILTRATION == infiltrationMethod)
            {
              groundwaterHead += groundwaterRecharge / porosity;

              if (groundwaterHead < layerZBottom)
                {
                  // Even though we are limiting outflows, groundwaterHead can go below layerZBottom due to roundoff error.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                  CkAssert(epsilonEqual(groundwaterHead, layerZBottom));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                  groundwaterError += (layerZBottom - groundwaterHead) * porosity;
                  groundwaterHead   = layerZBottom;
                }
              else if (groundwaterHead > elementZSurface)
                {
                  // Exfiltration.
                  surfacewaterDepth += (groundwaterHead - elementZSurface) * porosity;
                  groundwaterHead    = elementZSurface;
                }
            }
          else if (GARTO_INFILTRATION == infiltrationMethod)
            {
              groundwaterHead += groundwaterRecharge / garto_specific_yield((garto_domain*)vadoseZoneState, elementZSurface - groundwaterHead);

              // Cap groundwaterHead at the surface.  Do not limit groundwaterHead from going below layerZBottom.
              if (groundwaterHead > elementZSurface)
                {
                  groundwaterHead = elementZSurface;
                }
              
              if (epsilonLess(0.0, groundwaterRecharge))
                {
                  // If there is excess water put it immediately into the groundwater front of the GARTO domain.
                  garto_add_groundwater((garto_domain*)vadoseZoneState, &groundwaterRecharge);

                  if (epsilonLess(0.0, groundwaterRecharge))
                    {
                      // Not all of the water could fit in to the GARTO domain because the domain is full.  Put the excess water on the surface and the
                      // groundwater head moves up to the surface.  The real groundwater head is at the top of the surfacewater, but we set groundwaterHead to
                      // be at the surface and add surfacewaterDepth as needed because if we set groundwaterHead to be at the top of the surfacewater we would
                      // need to update it any time the value of surfacewaterDepth changed.
                      surfacewaterDepth   += groundwaterRecharge;
                      groundwaterRecharge  = 0.0;
                      groundwaterHead      = elementZSurface;
                    }
                }
              else if (epsilonGreater(0.0, groundwaterRecharge))
                {
                  // If there is a water deficit take it immediately from the groundwater front of the GARTO domain.
                  garto_take_groundwater((garto_domain*)vadoseZoneState, elementZSurface - groundwaterHead, &groundwaterRecharge);

                  // If there is still a deficit leave it to be resolved next time.  The water table will drop further allowing us to get more water out.
                }
            }
        }
    }
  
  return error;
}

bool InfiltrationAndGroundwater::allInflowsArrived(double currentTime, double timestepEndTime)
{
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  bool                                                       allArrived = true; // Whether all material has arrived from all neighbors.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in InfiltrationAndGroundwater::allInflowsArrived: currentTime must be less than or equal to timestepEndTime.\n");
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
    }
  
  return allArrived;
}

bool InfiltrationAndGroundwater::receiveInflows(double currentTime, double timestepEndTime, double elementArea)
{
  bool error = false;                                                   // Error flag.
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in InfiltrationAndGroundwater::receiveInflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in InfiltrationAndGroundwater::receiveInflows: elementArea must be greater than zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call getMaterial.  advanceTime shouldn't be called in this situation, but if it is we don't have to do
  // anything to receive inflows for the next zero seconds.
  if (!error && currentTime < timestepEndTime)
    {
      // Receive all inflows.
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              groundwaterRecharge += (*itMesh).getMaterial(currentTime, timestepEndTime) / elementArea;
            }
        }
      
      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              groundwaterRecharge += (*itChannel).getMaterial(currentTime, timestepEndTime) / elementArea;
            }
        }
    }
  
  return error;
}

bool MeshElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                                       std::vector<MeshSurfacewaterMeshNeighborProxy>::size_type neighborProxyIndex,
                                                                       double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < meshNeighbors.size()))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: neighborProxyIndex must be greater than or equal to zero and less "
              "than meshNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      if (isBoundary(meshNeighbors[neighborProxyIndex].neighbor))
        {
          // FIXME what to do about inflowXVelocity, inflowYVelocity, and inflowHeight?
          error = surfacewaterMeshBoundaryFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                                   (BoundaryConditionEnum)meshNeighbors[neighborProxyIndex].neighbor, 0.0, 0.0, 0.0,
                                                   meshNeighbors[neighborProxyIndex].edgeLength, meshNeighbors[neighborProxyIndex].edgeNormalX,
                                                   meshNeighbors[neighborProxyIndex].edgeNormalY, elementArea, surfacewaterDepth);
        }
      else
        {
          error = surfacewaterMeshMeshFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                               meshNeighbors[neighborProxyIndex].edgeLength, elementX, elementY, elementZSurface, elementArea, manningsN,
                                               surfacewaterDepth, meshNeighbors[neighborProxyIndex].neighborX, meshNeighbors[neighborProxyIndex].neighborY,
                                               meshNeighbors[neighborProxyIndex].neighborZSurface, meshNeighbors[neighborProxyIndex].neighborArea,
                                               meshNeighbors[neighborProxyIndex].neighborManningsN, neighborSurfacewaterDepth);
        }
    }
  
  // Calculate expiration time.
  if (!error)
    {
      meshNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool MeshElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor(double currentTime, double regionalDtLimit,
                                                                          std::vector<MeshSurfacewaterChannelNeighborProxy>::size_type neighborProxyIndex,
                                                                          double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < channelNeighbors.size()))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: neighborProxyIndex must be greater than or equal to zero and "
              "less than channelNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      error = surfacewaterMeshChannelFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                              channelNeighbors[neighborProxyIndex].edgeLength, elementZSurface, elementArea, surfacewaterDepth,
                                              channelNeighbors[neighborProxyIndex].neighborZBank, channelNeighbors[neighborProxyIndex].neighborZBed,
                                              channelNeighbors[neighborProxyIndex].neighborBaseWidth, channelNeighbors[neighborProxyIndex].neighborSideSlope,
                                              neighborSurfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      channelNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool MeshElement::doPointProcessesAndSendOutflows(double referenceDate, double currentTime, double timestepEndTime)
{
  bool   error                   = false;                                // Error flag.
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  double localSolarDateTime      = referenceDate + (ADHydro::drainDownMode ? ADHydro::drainDownTime : currentTime) / (24.0 * 60.0 * 60.0) +
                                   longitude / (2 * M_PI);               // The time and date to use for the sun angle as a Julian date converted from UTC to
                                                                         // local solar time.  If we are using drainDownMode calendar date and time stands
                                                                         // still at the time specified by ADHydro::drainDownTime.
  long   year;                                                           // For calculating yearlen, julian, and hourAngle.
  long   month;                                                          // For calculating hourAngle.
  long   day;                                                            // For calculating hourAngle.
  long   hour;                                                           // For passing to julianToGregorian, unused.
  long   minute;                                                         // For passing to julianToGregorian, unused.
  double second;                                                         // For passing to julianToGregorian, unused.
  int    yearlen;                                                        // Input to evapoTranspirationSoil in days.
  float  julian;                                                         // Input to evapoTranspirationSoil in days.
  double hourAngle;                                                      // For calculating cosZ.  In radians.
  double declinationOfSun;                                               // For calculating cosZ.  In radians.
  float  cosZ;                                                           // Input to evapoTranspirationSoil, unitless.
  EvapoTranspirationSoilMoistureStruct evapoTranspirationSoilMoisture;   // Input to evapoTranspirationSoil.
  double dt                      = timestepEndTime - currentTime;        // Seconds.
  float  surfacewaterAdd;                                                // Output of evapoTranspirationSoil in millimeters.
  float  evaporationFromCanopy;                                          // Output of evapoTranspirationSoil in millimeters.
  float  evaporationFromSnow;                                            // Output of evapoTranspirationSoil in millimeters.
  float  evaporationFromGround;                                          // Output of evapoTranspirationSoil in millimeters.
  float  transpirationFromVegetation;                                    // Output of evapoTranspirationSoil in millimeters.
  float  waterError;                                                     // Output of evapoTranspirationSoil in millimeters.
  double evaporation;                                                    // Meters.
  double transpiration;                                                  // Meters.
  double unsatisfiedEvaporation;                                         // Meters.
  double groundwaterEvaporation;                                         // Meters.
  double totalOutwardFlowRate    = 0.0;                                  // Sum of all outward flow rates in cubic meters per second.
  double outwardFlowRateFraction = 1.0;                                  // Fraction of all outward flow rates that can be satisfied, unitless.
  double waterSent;                                                      // Cubic meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(1721425.5 <= referenceDate))
    {
      CkError("ERROR in MeshElement::doPointProcessesAndSendOutflows: referenceDate must not be before 1 CE.\n");
      error = true;
    }
  
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in MeshElement::doPointProcessesAndSendOutflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // Calculate year, month, and day.
      julianToGregorian(localSolarDateTime, &year, &month, &day, &hour, &minute, &second);

      // Determine if it is a leap year.
      if (0 == year % 400)
        {
          yearlen = 366;
        }
      else if (0 == year % 100)
        {
          yearlen = 365;
        }
      else if (0 == year % 4)
        {
          yearlen = 366;
        }
      else
        {
          yearlen = 365;
        }

      // Calculate the ordinal day of the year by subtracting the Julian date of Jan 1 beginning midnight.
      julian = localSolarDateTime - gregorianToJulian(year, 1, 1, 0, 0, 0);

      // FIXME if this element is shaded set cosZ to zero.

      // FIXME handle slope and aspect effects on solar radiation.

      // The number of "hours" that the sun is east or west of straight overhead.  The value is actually in radians with each hour being Pi/12 radians.
      // Positive means west.  Negative means east.
      hourAngle = (localSolarDateTime - gregorianToJulian(year, month, day, 12, 0, 0)) * 2.0 * M_PI;

      // Calculate cosZ.
      declinationOfSun = -23.44 * M_PI / 180.0 * cos(2.0 * M_PI * (julian + 10.0) / yearlen);
      cosZ             = sin(latitude) * sin(declinationOfSun) + cos(latitude) * cos(declinationOfSun) * cos(hourAngle);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(-1.0f <= cosZ && 1.0f >= cosZ);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // If the sun is below the horizon it doesn't matter how far below.  Set cosZ to zero.
      if (0.0f > cosZ)
        {
          cosZ = 0.0f;
        }

      // Fill in the Noah-MP soil moisture struct from groundwater and vadose zone state.
      underground.fillInEvapoTranspirationSoilMoistureStruct(elementZSurface, evapoTranspirationState.zSnso, evapoTranspirationSoilMoisture);

      // Do point processes for rainfall, snowmelt, and evapo-transpiration.
      error = evapoTranspirationSoil(vegetationType, soilType, latitude, yearlen, julian, cosZ, dt, sqrt(elementArea), &evapoTranspirationForcing,
                                     &evapoTranspirationSoilMoisture, &evapoTranspirationState, &surfacewaterAdd, &evaporationFromCanopy, &evaporationFromSnow,
                                     &evaporationFromGround, &transpirationFromVegetation, &waterError);
    }
  
  if (!error)
    {
      // Move water and record flows for precipitation, evaporation, and transpiration.
      surfacewaterDepth += surfacewaterAdd / 1000.0;
      evaporation        = ((double)evaporationFromCanopy + evaporationFromSnow) / 1000.0;
      transpiration      = 0.0;
      
      // Take evaporationFromGround first from surfacewater, and then if there isn't enough surfacewater from groundwater.  If there isn't enough groundwater
      // print a warning and reduce the quantity of evaporation.
      unsatisfiedEvaporation = evaporationFromGround / 1000.0;
      
      if (surfacewaterDepth >= unsatisfiedEvaporation)
        {
          evaporation       += unsatisfiedEvaporation;
          surfacewaterDepth -= unsatisfiedEvaporation;
        }
      else
        {
          unsatisfiedEvaporation -= surfacewaterDepth;
          evaporation            += surfacewaterDepth;
          surfacewaterDepth       = 0.0;
          groundwaterEvaporation  = underground.evaporate(unsatisfiedEvaporation);
          unsatisfiedEvaporation -= groundwaterEvaporation;
          evaporation            += groundwaterEvaporation;

          if ((2 <= ADHydro::verbosityLevel && 1.0 < unsatisfiedEvaporation) || (3 <= ADHydro::verbosityLevel && 0.0 < unsatisfiedEvaporation))
            {
              CkError("WARNING in MeshElement::doPointProcessesAndSendOutflows, element %d: unsatisfied evaporation from ground of %le meters.\n",
                      elementNumber, unsatisfiedEvaporation);
            }
        }
      
      // Take transpiration first from groundwater, and then if there isn't enough groundwater from surfacewater.  If there isn't enough surfacewater print a
      // warning and reduce the quantity of transpiration.
      unsatisfiedEvaporation  = transpirationFromVegetation / 1000.0;
      groundwaterEvaporation  = underground.transpire(unsatisfiedEvaporation);
      unsatisfiedEvaporation -= groundwaterEvaporation;
      transpiration          += groundwaterEvaporation;

      if (surfacewaterDepth >= unsatisfiedEvaporation)
        {
          transpiration     += unsatisfiedEvaporation;
          surfacewaterDepth -= unsatisfiedEvaporation;
        }
      else
        {
          unsatisfiedEvaporation -= surfacewaterDepth;
          transpiration          += surfacewaterDepth;
          surfacewaterDepth       = 0.0;

          if ((2 <= ADHydro::verbosityLevel && 1.0 < unsatisfiedEvaporation) || 3 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in MeshElement::doPointProcessesAndSendOutflows, element %d: unsatisfied transpiration of %le meters.\n", elementNumber,
                      unsatisfiedEvaporation);
            }
        }
      
      // Record cumulative flows and water error.
      precipitationRate                 = -evapoTranspirationForcing.prcp / 1000.0;
      precipitationCumulativeShortTerm += precipitationRate * dt;
      evaporationRate                   = evaporation / dt;
      evaporationCumulativeShortTerm   += evaporation;
      transpirationRate                 = transpiration / dt;
      transpirationCumulativeShortTerm += transpiration;
      surfacewaterError                += waterError / 1000.0;
      
      // If the roundoff error of adding one timestep's water to CumulativeShortTerm is greater than the roundoff error of adding CumulativeShortTerm to
      // CumulativeLongTerm then move CumulativeShortTerm to CumulativeLongTerm.
      // FIXME implement
      
      // Do point process for infiltration and send groundwater outflows.
      underground.doInfiltrationAndSendGroundwaterOutflows(currentTime, timestepEndTime, elementZSurface, elementArea, surfacewaterDepth);
      
      // Limit surfacewater outflows.
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

      if (surfacewaterDepth * elementArea < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = surfacewaterDepth * elementArea / (totalOutwardFlowRate * dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Send surfacewater outflows taking water from surfacewaterDepth.
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 < (*itMesh).nominalFlowRate)
            {
              waterSent          = (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction;
              surfacewaterDepth -= waterSent / elementArea;

              // FIXME call the region's function to send the water.
              //region->sendWater(MeshSurfacewaterMeshNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction));
            }
        }

      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 < (*itChannel).nominalFlowRate)
            {
              waterSent          = (*itChannel).nominalFlowRate * dt * outwardFlowRateFraction;
              surfacewaterDepth -= waterSent / elementArea;

              // FIXME call the region's function to send the water.
              //region->sendWater(MeshSurfacewaterChannelNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction));
            }
        }

      // Even though we are limiting outflows, surfacewaterDepth can go below zero due to roundoff error.
      if (0.0 > surfacewaterDepth)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0, surfacewaterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          surfacewaterError -= surfacewaterDepth;
          surfacewaterDepth  = 0.0;
        }
    }
  
  return error;
}

bool MeshElement::allInflowsArrived(double currentTime, double timestepEndTime)
{
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  bool allArrived = underground.allInflowsArrived(currentTime, timestepEndTime); // Whether all material has arrived from all neighbors.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in MeshElement::allInflowsArrived: currentTime must be less than or equal to timestepEndTime.\n");
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
    }
  
  return allArrived;
}

bool MeshElement::receiveInflows(double currentTime, double timestepEndTime)
{
  bool                                                        error = false; // Error flag.
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator    itMesh;        // Loop iterator.
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator itChannel;     // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in MeshElement::receiveInflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call getMaterial.  advanceTime shouldn't be called in this situation, but if it is we don't have to do
  // anything to advance time by zero seconds.
  if (!error && currentTime < timestepEndTime)
    {
      // Receive all inflows.
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              surfacewaterDepth += (*itMesh).getMaterial(currentTime, timestepEndTime) / elementArea;
            }
        }
      
      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              surfacewaterDepth += (*itChannel).getMaterial(currentTime, timestepEndTime) / elementArea;
            }
        }
      
      underground.receiveInflows(currentTime, timestepEndTime, elementArea);
    }
  
  return error;
}
