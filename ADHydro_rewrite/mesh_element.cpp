#include "mesh_element.h"
#include "adhydro.h"
#include "surfacewater.h"
#include "groundwater.h"

void InfiltrationAndGroundwater::fillInEvapoTranspirationSoilMoistureStruct(double elementZSurface, float zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS],
                                                                            EvapoTranspirationSoilMoistureStruct& evapoTranspirationSoilMoisture)
{
  int    ii;                      // Loop counter.
  double layerMiddleDepth;        // Meters.
  double distanceAboveWaterTable; // Meters.
  double relativeSaturation;      // Unitless.
  
  // FIXME error check inputs.
  
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
          layerMiddleDepth        = 0.5 * (zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] + zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1]);
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
  
  // For all methods make the lowest layer water content extend down forever.
  evapoTranspirationSoilMoisture.smcwtd = evapoTranspirationSoilMoisture.smc[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS - 1];
}

double InfiltrationAndGroundwater::evaporate(double unsatisfiedEvaporation)
{
  double evaporation = 0.0; // Return value.
  
  // FIXME error check inputs.

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

  return evaporation;
}

double InfiltrationAndGroundwater::transpire(double unsatisfiedTranspiration)
{
  double transpiration = 0.0; // Return value.
  
  // FIXME error check inputs.

  // if (NO_INFILTRATION == infiltrationMethod) return zero.
  if (TRIVIAL_INFILTRATION == infiltrationMethod)
    {
      // With no vadose zone state transpiration water comes from the same place as evaporation water.
      transpiration = evaporate(unsatisfiedTranspiration);
    }

  return transpiration;
}

bool InfiltrationAndGroundwater::calculateNominalFlowRateWithMeshNeighbor(double currentTime, double regionalExpirationTimeLimit, int neighborProxyIndex,
                                                                          double elementX, double elementY, double elementZSurface, double elementArea,
                                                                          double elementSurfacewaterDepth, double neighborSurfacewaterDepth,
                                                                          double neighborGroundwaterHead)
{
  bool   error = false;    // Error flag.
  double dtNew = INFINITY; // Duration before nominal flow rate expiration in seconds.
  
  // FIXME error check inputs.
  
  if (isBoundary(meshNeighbors[neighborProxyIndex].neighbor))
    {
      // Calculate nominal flow rate.
      // FIXME what to do about inflowHeight?
      error = groundwaterMeshBoundaryFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &dtNew,
                                              (BoundaryConditionEnum)meshNeighbors[neighborProxyIndex].neighbor, 0.0,
                                              meshNeighbors[neighborProxyIndex].edgeLength, meshNeighbors[neighborProxyIndex].edgeNormalX,
                                              meshNeighbors[neighborProxyIndex].edgeNormalY, layerZBottom, elementArea, slopeX, slopeY, conductivity, porosity,
                                              groundwaterHead);
    }
  else
    {
      // Calculate nominal flow rate.
      error = groundwaterMeshMeshFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &dtNew, meshNeighbors[neighborProxyIndex].edgeLength, elementX,
                                          elementY, elementZSurface, layerZBottom, elementArea, conductivity, porosity, elementSurfacewaterDepth,
                                          groundwaterHead, meshNeighbors[neighborProxyIndex].neighborX, meshNeighbors[neighborProxyIndex].neighborY,
                                          meshNeighbors[neighborProxyIndex].neighborZSurface, meshNeighbors[neighborProxyIndex].neighborLayerZBottom,
                                          meshNeighbors[neighborProxyIndex].neighborArea, meshNeighbors[neighborProxyIndex].neighborConductivity,
                                          meshNeighbors[neighborProxyIndex].neighborPorosity, neighborSurfacewaterDepth, neighborGroundwaterHead);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      meshNeighbors[neighborProxyIndex].expirationTime = currentTime + dtNew;
      
      // Limit expiration time to the regional limit.
      if (meshNeighbors[neighborProxyIndex].expirationTime > regionalExpirationTimeLimit)
        {
          meshNeighbors[neighborProxyIndex].expirationTime = regionalExpirationTimeLimit;
        }
    }
  
  return error;
}

bool InfiltrationAndGroundwater::calculateNominalFlowRateWithChannelNeighbor(double currentTime, double regionalExpirationTimeLimit, int neighborProxyIndex,
                                                                             double elementZSurface, double elementSurfacewaterDepth,
                                                                             double neighborSurfacewaterDepth)
{
  bool   error = false;    // Error flag.
  double dtNew = INFINITY; // Duration before nominal flow rate expiration in seconds.
  
  // FIXME error check inputs.
  
  // Calculate nominal flow rate.
  // FIXME calculate dtNew
  error = groundwaterMeshChannelFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, channelNeighbors[neighborProxyIndex].edgeLength,
                                         elementZSurface, layerZBottom, elementSurfacewaterDepth, groundwaterHead,
                                         channelNeighbors[neighborProxyIndex].neighborZBank, channelNeighbors[neighborProxyIndex].neighborZBed,
                                         channelNeighbors[neighborProxyIndex].neighborBaseWidth, channelNeighbors[neighborProxyIndex].neighborSideSlope,
                                         channelNeighbors[neighborProxyIndex].neighborBedConductivity,
                                         channelNeighbors[neighborProxyIndex].neighborBedThickness, neighborSurfacewaterDepth);
  
  // Calculate expiration time.
  if (!error)
    {
      channelNeighbors[neighborProxyIndex].expirationTime = currentTime + dtNew;
      
      // Limit expiration time to the regional limit.
      if (channelNeighbors[neighborProxyIndex].expirationTime > regionalExpirationTimeLimit)
        {
          channelNeighbors[neighborProxyIndex].expirationTime = regionalExpirationTimeLimit;
        }
    }
  
  return error;
}

void InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows(double currentTime, double timestepEndTime, double elementZSurface,
                                                                          double elementArea, double& surfacewaterDepth)
{
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  double dt                      = timestepEndTime - currentTime;       // Seconds
  double groundwaterAvailable    = 0.0;                                 // Groundwater available to satisfy outflows in cubic meters of water.
  double totalOutwardFlowRate    = 0.0;                                 // Sum of all outward flow rates in cubic meters per second.
  double outwardFlowRateFraction = 1.0;                                 // Fraction of all outward flow rates that can be satisfied, unitless.
  double waterSent;                                                     // Cubic meters.
  
  // FIXME error check inputs.
  
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
    }
}

bool InfiltrationAndGroundwater::allInflowsArrived(double currentTime, double timestepEndTime)
{
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  bool                                                       allArrived = true; // Whether all material has arrived from all neighbors.
  
  // FIXME error check inputs.
  
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

void InfiltrationAndGroundwater::receiveInflows(double currentTime, double timestepEndTime, double elementArea)
{
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  
  // FIXME error check inputs.
  
  // If timestepEndTime is not in the future we can't call getMaterial.  advanceTime shouldn't be called in this situation, but if it is we don't have to do
  // anything to advance time by zero seconds.
  if (currentTime < timestepEndTime)
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
}

bool MeshElement::calculateNominalFlowRateWithMeshNeighbor(double currentTime, double regionalExpirationTimeLimit, int neighborProxyIndex,
                                                           double neighborSurfacewaterDepth)
{
  bool   error = false;    // Error flag.
  double dtNew = INFINITY; // Duration before nominal flow rate expiration in seconds.
  
  // FIXME error check inputs.
  
  if (isBoundary(meshNeighbors[neighborProxyIndex].neighbor))
    {
      // Calculate nominal flow rate.
      // FIXME what to do about inflowXVelocity, inflowYVelocity, and inflowHeight?
      error = surfacewaterMeshBoundaryFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &dtNew,
                                               (BoundaryConditionEnum)meshNeighbors[neighborProxyIndex].neighbor, 0.0, 0.0, 0.0,
                                               meshNeighbors[neighborProxyIndex].edgeLength, meshNeighbors[neighborProxyIndex].edgeNormalX,
                                               meshNeighbors[neighborProxyIndex].edgeNormalY, elementArea, surfacewaterDepth);
    }
  else
    {
      // Calculate nominal flow rate.
      error = surfacewaterMeshMeshFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &dtNew, meshNeighbors[neighborProxyIndex].edgeLength, elementX,
                                           elementY, elementZSurface, elementArea, manningsN, surfacewaterDepth, meshNeighbors[neighborProxyIndex].neighborX,
                                           meshNeighbors[neighborProxyIndex].neighborY, meshNeighbors[neighborProxyIndex].neighborZSurface,
                                           meshNeighbors[neighborProxyIndex].neighborArea, meshNeighbors[neighborProxyIndex].neighborManningsN,
                                           neighborSurfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      meshNeighbors[neighborProxyIndex].expirationTime = currentTime + dtNew;
      
      // Limit expiration time to the regional limit.
      if (meshNeighbors[neighborProxyIndex].expirationTime > regionalExpirationTimeLimit)
        {
          meshNeighbors[neighborProxyIndex].expirationTime = regionalExpirationTimeLimit;
        }
    }
  
  return error;
}

bool MeshElement::calculateNominalFlowRateWithChannelNeighbor(double currentTime, double regionalExpirationTimeLimit, int neighborProxyIndex,
                                                              double neighborSurfacewaterDepth)
{
  bool   error = false;    // Error flag.
  double dtNew = INFINITY; // Duration before nominal flow rate expiration in seconds.
  
  // FIXME error check inputs.
  
  // Calculate nominal flow rate.
  error = surfacewaterMeshChannelFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, &dtNew, channelNeighbors[neighborProxyIndex].edgeLength,
                                          elementZSurface, elementArea, surfacewaterDepth, channelNeighbors[neighborProxyIndex].neighborZBank,
                                          channelNeighbors[neighborProxyIndex].neighborZBed, channelNeighbors[neighborProxyIndex].neighborBaseWidth,
                                          channelNeighbors[neighborProxyIndex].neighborSideSlope, neighborSurfacewaterDepth);
  
  // Calculate expiration time.
  if (!error)
    {
      channelNeighbors[neighborProxyIndex].expirationTime = currentTime + dtNew;
      
      // Limit expiration time to the regional limit.
      if (channelNeighbors[neighborProxyIndex].expirationTime > regionalExpirationTimeLimit)
        {
          channelNeighbors[neighborProxyIndex].expirationTime = regionalExpirationTimeLimit;
        }
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
  
  // FIXME error check inputs.
  
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
  
  // The number of "hours" that the sun is east or west of straight overhead.  The value is actually in radians with each hour being Pi/12 radians.  Positive
  // means west.  Negative means east.
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
              CkError("WARNING in MeshElement::doPointProcessesAndSendSurfacewaterOutflows, element %d: unsatisfied evaporation from ground of %le meters.\n",
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
              CkError("WARNING in MeshElement::doPointProcessesAndSendSurfacewaterOutflows, element %d: unsatisfied transpiration of %le meters.\n",
                      elementNumber, unsatisfiedEvaporation);
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

      if (surfacewaterDepth < 0.0)
        {
          // Even though we are limiting outflows, surfacewaterDepth can go below zero due to roundoff error.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(surfacewaterDepth, 0.0));
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
  
  // FIXME error check inputs.
  
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

void MeshElement::receiveInflows(double currentTime, double timestepEndTime)
{
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  
  // FIXME error check inputs.
  
  // If timestepEndTime is not in the future we can't call getMaterial.  advanceTime shouldn't be called in this situation, but if it is we don't have to do
  // anything to advance time by zero seconds.
  if (currentTime < timestepEndTime)
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
}
