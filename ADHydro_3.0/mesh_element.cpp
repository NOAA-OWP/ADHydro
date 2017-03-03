#include "mesh_element.h"
#include "readonly.h"

bool MeshElement::checkInvariant()
{
    bool error = false; // Error flag.
    
    // FIXME implement
    /* FIXME notes
     * 
     * if !soilExists then soilThickness is set to zero
     * if !aquiferExists then aquiferThicnkness is set to zero and aquiferHead is set to -INFINITY
     * get groundwater state invariants from doPointProcessesAndSendOutflows
     */
    
    return error;
}

bool MeshElement::doPointProcessesAndSendOutflows(double referenceDate, double currentTime, double timestepEndTime)
{
    bool   error                = false;                                 // Error flag.
    double localSolarDateTime   = referenceDate + (currentTime / (24.0 * 60.0 * 60.0)) + (longitude / (2.0 * M_PI)); // (days) Julian date converted from UTC to local solar time.
    long   year;                                                         // For calculating yearlen, julian, and hourAngle.
    long   month;                                                        // For calculating hourAngle.
    long   day;                                                          // For calculating hourAngle.
    long   hour;                                                         // For passing to julianToGregorian, unused.
    long   minute;                                                       // For passing to julianToGregorian, unused.
    double second;                                                       // For passing to julianToGregorian, unused.
    int    yearlen;                                                      // (days) Year length.
    float  julian;                                                       // (days) Day of year including fractional day.
    double hourAngle;                                                    // (radians) How far the sun is east or west of zenith.  Positive means west.  Negative means east.  Used for calculating cosZ.
    double declinationOfSun;                                             // (radians) How far the sun is above the horizon.  Used for calculating cosZ.
    float  cosZ;                                                         // Cosine of the angle between the normal to the land surface and the sun.
    EvapoTranspirationSoilMoistureStruct evapoTranspirationSoilMoisture; // For passing soil moisture profile to Noah-MP.
    double originalEvapoTranspirationTotalWaterInDomain;                 // (mm) For mass balance check.
    double dt                   = timestepEndTime - currentTime;         // (s) Duration of timestep.
    float  floatDt              = dt;                                    // (s) Duration of timestep as a float for passing to Noah-MP.
    float  surfacewaterAdd;                                              // (mm) Water from Noah-MP that must be added to surface water.  Must be non-negative.
    float  evaporationFromCanopy;                                        // (mm) Water that Noah-MP already added to or removed from the canopy for evaporation or condensation.
                                                                         // Positive means water evaporated off of the canopy.  Negative means water condensed on to the canopy.
    float  evaporationFromSnow;                                          // (mm) Water that Noah-MP already added to or removed from the snowpack for evaporation or condensation.
                                                                         // Positive means water evaporated off of the snowpack.  Negative means water condensed on to the snowpack.
    float  evaporationFromGround;                                        // (mm) Water that must be added to or removed from the land surface for evaporation or condensation.
                                                                         // Positive means water evaporated off of the ground.  Negative means water condensed on to the ground.
    float  transpirationFromVegetation;                                  // (mm) Water that must be removed from the soil moisture for transpiration.  Positive means water transpired off of plants.  Must be non-negative.
    float  noahMPWaterCreated;                                           // (mm) Water that was created or destroyed by Noah-MP.  Positive means water was created.  Negative means water was destroyed.
    double precipitation;                                                // (m) Total quantity of water precipitated this timestep.
    double evaporation;                                                  // (m) Total quantity of water evaporated   this timestep.
    double transpiration;                                                // (m) Total quantity of water transpired   this timestep.
    double unsatisfiedEvaporation;                                       // (m) Remaining quantity of water needing to be evaporated.
    double unsatisfiedTranspiration;                                     // (m) Remaining quantity of water needing to be transpired.
    std::map<NeighborConnection, NeighborProxy>::iterator it;            // Iterator over neighbor proxies.
    double totalSurfaceOutflows = 0.0;                                   // (m^3/s) Total of nominal flow rates of surface outflows.
    double totalSoilOutflows    = 0.0;                                   // (m^3/s) Total of nominal flow rates of soil    outflows.
    double totalAquiferOutflows = 0.0;                                   // (m^3/s) Total of nominal flow rates of aquifer outflows.
    double surfaceOutflowFraction;                                       // (m^3/m^3) Fraction of total nominal flow rate that can be satisfied.
    double soilOutflowFraction;                                          // (m^3/m^3) Fraction of total nominal flow rate that can be satisfied.
    double aquiferOutflowFraction;                                       // (m^3/m^3) Fraction of total nominal flow rate that can be satisfied.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(1721425.5 <= referenceDate))
        {
            CkError("ERROR in MeshElement::doPointProcessesAndSendOutflows: referenceDate must not be before 1 CE.\n");
            error = true;
        }
        
        if (!(currentTime < timestepEndTime))
        {
            CkError("ERROR in MeshElement::doPointProcessesAndSendOutflows: currentTime must be less than timestepEndTime.\n");
            error = true;
        }
    }
    
    // Check invariants on groundwater state.
    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    {
        CkAssert(0.0 == soilRecharge);
        CkAssert(0.0 == aquiferRecharge);
        
        if (soilExists && aquiferExists)
        {
            CkAssert(SATURATED_AQUIFER == groundwaterMode || UNSATURATED_AQUIFER == groundwaterMode || PERCHED_WATER_TABLE == groundwaterMode);
        }
        else
        {
            CkAssert(NO_MULTILAYER == groundwaterMode);
        }
        
        if (SATURATED_AQUIFER == groundwaterMode)
        {
            CkAssert(aquiferWater.isFull());
            CkAssert(elementZ - soilThickness <= aquiferHead);
        }
        else if (UNSATURATED_AQUIFER == groundwaterMode)
        {
            CkAssert(!aquiferWater.isFull());
            CkAssert(elementZ - soilThickness > aquiferHead);
        }
        else if (PERCHED_WATER_TABLE == groundwaterMode)
        {
            CkAssert(!aquiferWater.isFull());
            CkAssert(elementZ - soilThickness > aquiferHead);
            CkAssert(perchedHead > aquiferHead);
        }
    }
    
    if (!error)
    {
        // Calculate year, month, and day.  Hour, minute, and second are unused.
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
        julian = localSolarDateTime - gregorianToJulian(year, 1, 1, 0, 0, 0.0);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        {
            CkAssert(0.0f <= julian && julian < yearlen);
        }
        
        // Calculate cosZ.
        // FIXME handle slope, aspect, and long distance shading effects on solar radiation by modifying how cosZ is calculated.
        hourAngle        = (localSolarDateTime - gregorianToJulian(year, month, day, 12, 0, 0.0)) * 2.0 * M_PI;
        declinationOfSun = -23.44 * (M_PI / 180.0) * cos(2.0 * M_PI * (julian + 10.0) / yearlen);
        cosZ             = sin(latitude) * sin(declinationOfSun) + cos(latitude) * cos(declinationOfSun) * cos(hourAngle);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        {
            CkAssert(-1.0f <= cosZ && 1.0f >= cosZ);
        }
        
        // If the sun is below the horizon it doesn't matter how far below.  Set cosZ to zero.
        if (0.0f > cosZ)
        {
            cosZ = 0.0f;
        }
        
        // Fill in the Noah-MP soil moisture struct from soil and aquifer state.
        fillInEvapoTranspirationSoilMoistureStruct(evapoTranspirationSoilMoisture);
        
        // Save the original amount of water stored in evapoTranspirationState for mass balance check.
        originalEvapoTranspirationTotalWaterInDomain = evapoTranspirationTotalWaterInDomain(&evapoTranspirationState);
        
        // Call Noah-MP.
        error = evapoTranspirationSoil(vegetationType, groundType, latitude, yearlen, julian, cosZ, floatDt, sqrt(elementArea), &evapoTranspirationForcing,
                                       &evapoTranspirationSoilMoisture, &evapoTranspirationState, &surfacewaterAdd, &evaporationFromCanopy, &evaporationFromSnow,
                                       &evaporationFromGround, &transpirationFromVegetation, &noahMPWaterCreated);
    }
    
    if (!error)
    {
        // Because Noah-MP uses single precision floats, its roundoff error is much higher than for doubles.  However, we can calculate the mass error using
        // doubles and account for it in surfaceWaterCreated.  The mass error is the amount of water at the end (water stored in evapoTranspirationState plus water
        // that came out in the form of surfacewaterAdd, evaporationFromCanopy, and evaporationFromSnow) minus the amount of water at the
        // beginning (water stored in evapoTranspirationState plus water that went in in the form of precipitation).  evaporationFromGround and
        // transpirationFromVegetation are not used in this computation because that water was not taken out by Noah-MP.  They are taken below.
        // evapoTranspirationSoil also tells us if it knowingly created or destroyed any water in noahMPWaterCreated.
        // However, we don't use that value because this calculation subsumes that quantity at higher precision.
        surfaceWaterCreated += (((double)evapoTranspirationTotalWaterInDomain(&evapoTranspirationState) + (double)surfacewaterAdd + (double)evaporationFromCanopy + (double)evaporationFromSnow) -
                                (originalEvapoTranspirationTotalWaterInDomain + evapoTranspirationForcing.prcp * dt)) / 1000.0; // Divide by a thousand to convert from millimeters to meters.
        
        // Add surfaceWater and record the quantities of precipitation, evaporation, and transpiration for this timestep.
        surfaceWater            += surfacewaterAdd / 1000.0;                                               // Divide by a thousand to convert from millimeters to meters.
        precipitation            = -evapoTranspirationForcing.prcp * dt / 1000.0;                          // Divide by a thousand to convert from millimeters to meters.
        evaporation              = ((double)evaporationFromCanopy + (double)evaporationFromSnow) / 1000.0; // Divide by a thousand to convert from millimeters to meters.
        transpiration            = 0.0;
        unsatisfiedEvaporation   = evaporationFromGround / 1000.0;                                         // Divide by a thousand to convert from millimeters to meters.
        unsatisfiedTranspiration = transpirationFromVegetation / 1000.0;                                   // Divide by a thousand to convert from millimeters to meters.
        
        // Take evaporation first from surfaceWater, and then if there isn't enough SurfaceWater take the rest from subsurface water.
        if (surfaceWater           >= unsatisfiedEvaporation)
        {
            evaporation            += unsatisfiedEvaporation;
            surfaceWater           -= unsatisfiedEvaporation;
            unsatisfiedEvaporation  = 0.0;
        }
        else
        {
            evaporation            += surfaceWater;
            unsatisfiedEvaporation -= surfaceWater;
            surfaceWater            = 0.0;
        }
        
        // Take remaining evaporation plus all transpiration from subsurface water.
        // We assume there is enough water in the capillary fringe to completely satisfy evapotranspiration.
        // Since we send the soil moisture profile into Noah-MP if there is little water left it should calculate correspondingly little evapotranspiration.
        // FIXME we could create functions to take evaporation and/or transpiration out of the soil and/or aquifer water data structures at specific depths.
        if (soilExists)
        {
            evaporation             += unsatisfiedEvaporation;
            soilRecharge            -= unsatisfiedEvaporation;
            unsatisfiedEvaporation   = 0.0;
            transpiration           += unsatisfiedTranspiration;
            soilRecharge            -= unsatisfiedTranspiration;
            unsatisfiedTranspiration = 0.0;
        }
        else if (aquiferExists)
        {
            evaporation             += unsatisfiedEvaporation;
            aquiferRecharge         -= unsatisfiedEvaporation;
            unsatisfiedEvaporation   = 0.0;
            transpiration           += unsatisfiedTranspiration;
            aquiferRecharge         -= unsatisfiedTranspiration;
            unsatisfiedTranspiration = 0.0;
        }
        else if ((2 <= Readonly::verbosityLevel && 0.1 < unsatisfiedEvaporation + unsatisfiedTranspiration) ||
                 (3 <= Readonly::verbosityLevel && 0.0 < unsatisfiedEvaporation + unsatisfiedTranspiration))
        {
            CkError("WARNING in MeshElement::doPointProcessesAndSendOutflows, element %lu: unsatisfied evapotranspiration of %le meters.\n", elementNumber, unsatisfiedEvaporation + unsatisfiedTranspiration);
        }
        
        // Record flow rates and cumulative flows.
        precipitationRate = precipitation / dt;
        evaporationRate   = evaporation   / dt;
        transpirationRate = transpiration / dt;
        
        if (0.0 != precipitation)
        {
            if (0.0 != precipitationCumulativeShortTerm && (precipitationCumulativeShortTerm / precipitation) > (precipitationCumulativeLongTerm / precipitationCumulativeShortTerm))
            {
                // The relative roundoff error of adding this timestep to short term is greater than adding short term to long term so move short term to long term.
                precipitationCumulativeLongTerm += precipitationCumulativeShortTerm;
                precipitationCumulativeShortTerm = 0.0;
            }
            
            precipitationCumulativeShortTerm += precipitation;
        }
        
        if (0.0 != evaporation)
        {
            // Absolute value must be used here and not for precipitation or transpiration because evaporation is the only one that can be either positive or negative.
            if (0.0 != evaporationCumulativeShortTerm && std::abs(evaporationCumulativeShortTerm / evaporation) > std::abs(evaporationCumulativeLongTerm / evaporationCumulativeShortTerm))
            {
                // The relative roundoff error of adding this timestep to short term is greater than adding short term to long term so move short term to long term.
                evaporationCumulativeLongTerm += evaporationCumulativeShortTerm;
                evaporationCumulativeShortTerm = 0.0;
            }
            
            evaporationCumulativeShortTerm += evaporation;
        }
        
        if (0.0 != transpiration)
        {
            if (0.0 != transpirationCumulativeShortTerm && (transpirationCumulativeShortTerm / transpiration) > (transpirationCumulativeLongTerm / transpirationCumulativeShortTerm))
            {
                // The relative roundoff error of adding this timestep to short term is greater than adding short term to long term so move short term to long term.
                transpirationCumulativeLongTerm += transpirationCumulativeShortTerm;
                transpirationCumulativeShortTerm = 0.0;
            }
            
            transpirationCumulativeShortTerm += transpiration;
        }
        
        // Move water for infiltration.
        if (soilExists)
        {
            soilRecharge += soilWater.vadoseZoneSolver(surfaceWater, (PERCHED_WATER_TABLE == groundwaterMode || NO_MULTILAYER == groundwaterMode) ? perchedHead : aquiferHead);
        }
        else if (aquiferExists)
        {
            if (impedanceConductivity * dt < surfaceWater)
            {
                aquiferRecharge += impedanceConductivity * dt;
                surfaceWater    -= impedanceConductivity * dt;
            }
            else
            {
                aquiferRecharge += surfaceWater;
                surfaceWater     = 0.0;
            }
        }
        
        // Add up total lateral outflows.
        for (it = neighbors.begin(); it != neighbors.end(); ++it)
        {
            switch(it->first.localEndpoint)
            {
                case MESH_SURFACE:
                    if (0.0 < it->second.getNominalFlowRate())
                    {
                        totalSurfaceOutflows += it->second.getNominalFlowRate();
                    }
                    break;
                case MESH_SOIL:
                    if (0.0 < it->second.getNominalFlowRate())
                    {
                        totalSoilOutflows    += it->second.getNominalFlowRate();
                    }
                    break;
                case MESH_AQUIFER:
                    if (0.0 < it->second.getNominalFlowRate())
                    {
                        totalAquiferOutflows += it->second.getNominalFlowRate();
                    }
                    break;
                case IRRIGATION_RECIPIENT:
                    // This can't be an outflow so do nothing.
                    break;
                case CHANNEL_SURFACE:
                case BOUNDARY_INFLOW:
                case BOUNDARY_OUTFLOW:
                case TRANSBASIN_INFLOW:
                case TRANSBASIN_OUTFLOW:
                case RESERVOIR_RELEASE:
                case RESERVOIR_RECIPIENT:
                case IRRIGATION_DIVERSION:
                default:
                    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                    {
                        CkAssert(false); // These endpoints are invalid here.
                    }
                    break;
            }
        }
        
        // Limit lateral outflows for water available and take water.
        if (0.0 == surfaceWater || 0.0 == totalSurfaceOutflows)
        {
            // I've been debating about having this check here.  It's not strictly necessary, but I think it's a good idea.  It serves two purposes.
            // First, if both surfaceWater and totalSurfaceOutflows are zero it prevents a divide by zero when calculating surfaceOutflowFraction in the next else if.
            // This actually isn't necessary because if totalSurfaceOutflows is zero there are no neighbors with an outflow so surfaceOutflowFraction is unused.
            // But I think it's bad coding style to leave an unprotected divide by zero.  The second purpose is for speed.
            // No surface water is by far the most common case so this will be just a comparison instead of math and then a comparison.
            // once again, not strictly necessary.  The other two cases would do the right thing without this check.
            surfaceOutflowFraction = 0.0;
        }
        else if (surfaceWater <= (totalSurfaceOutflows / elementArea) * dt)
        {
            surfaceOutflowFraction = surfaceWater / ((totalSurfaceOutflows / elementArea) * dt);
            surfaceWater           = 0.0;
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            {
                CkAssert(0.0 <= surfaceOutflowFraction && 1.0 >= surfaceOutflowFraction);
            }
            
        }
        else
        {
            surfaceOutflowFraction = 1.0;
            surfaceWater          -= (totalSurfaceOutflows / elementArea) * dt;
        }
        
        // For soil and aquifer if there is any positive pressure water don't limit outflows.  We assume there will be enough water in the capillary fringe to cover mass conservation.
        // FIXME add something to bridge the time when a perched water table gets subsumed into a saturated aquifer, but the aquifer flow rate hasn't been recalculated yet?
        if (soilExists && (PERCHED_WATER_TABLE == groundwaterMode || NO_MULTILAYER == groundwaterMode) && perchedHead > elementZ - soilThickness)
        {
            soilOutflowFraction    = 1.0;
            soilRecharge          -= (totalSoilOutflows / elementArea) * dt;
        }
        else
        {
            soilOutflowFraction    = 0.0;
        }
        
        if (aquiferExists && aquiferHead > elementZ - soilThickness - aquiferThickness)
        {
            // Also take water for leakage from the aquifer to deep groundwater.
            aquiferOutflowFraction = 1.0;
            aquiferRecharge       -= (totalAquiferOutflows / elementArea + deepConductivity) * dt;
            deepGroundwater       += deepConductivity * dt;
        }
        else
        {
            aquiferOutflowFraction = 0.0;
        }
    }
    
    // Send lateral outflow water.  Even if actual flow is zero a message must be sent to let the receiver know what the actual flow is.
    for (it = neighbors.begin(); !error && it != neighbors.end(); ++it)
    {
        switch(it->first.localEndpoint)
        {
            case MESH_SURFACE:
                if (0.0 < it->second.getNominalFlowRate())
                {
                    error = it->second.sendWaterTransfer(WaterTransfer(it->second.getNominalFlowRate() * dt * surfaceOutflowFraction, currentTime, timestepEndTime));
                }
                break;
            case MESH_SOIL:
                if (0.0 < it->second.getNominalFlowRate())
                {
                    error = it->second.sendWaterTransfer(WaterTransfer(it->second.getNominalFlowRate() * dt * soilOutflowFraction,    currentTime, timestepEndTime));
                }
                break;
            case MESH_AQUIFER:
                if (0.0 < it->second.getNominalFlowRate())
                {
                    error = it->second.sendWaterTransfer(WaterTransfer(it->second.getNominalFlowRate() * dt * aquiferOutflowFraction, currentTime, timestepEndTime));
                }
                break;
            case IRRIGATION_RECIPIENT:
                // This can't be an outflow so do nothing.
                break;
            case CHANNEL_SURFACE:
            case BOUNDARY_INFLOW:
            case BOUNDARY_OUTFLOW:
            case TRANSBASIN_INFLOW:
            case TRANSBASIN_OUTFLOW:
            case RESERVOIR_RELEASE:
            case RESERVOIR_RECIPIENT:
            case IRRIGATION_DIVERSION:
            default:
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                {
                    CkAssert(false); // These endpoints are invalid here.
                }
                break;
        }
    }
    
    return error;
}

bool MeshElement::allInflowsHaveArrived(double currentTime, double timestepEndTime)
{
    bool                                                  arrived = true; // Flag will be set to false when one is found that has not arrived.
    std::map<NeighborConnection, NeighborProxy>::iterator it;             // Loop iterator.
    
    // Don't error check parameters because it's a simple pass-through to NeighborProxy::allWaterHasArrived and they will be checked inside that method.
    
    for (it = neighbors.begin(); arrived && it != neighbors.end(); ++it)
    {
        arrived = it->second.allWaterHasArrived(currentTime, timestepEndTime);
    }
    
    return arrived;
}

bool MeshElement::receiveInflowsAndUpdateState(double currentTime, double timestepEndTime)
{
    bool   error = false;                                     // Error flag.
    std::map<NeighborConnection, NeighborProxy>::iterator it; // Iterator over neighbor proxies.
    double dt    = timestepEndTime - currentTime;             // (s) duration of timestep.
    double impedanceFlow;                                     // (m) Water that makes it through the impedance layer.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(currentTime < timestepEndTime))
        {
            CkError("ERROR in MeshElement::receiveInflowsAndUpdateState: currentTime must be less than timestepEndTime.\n");
            error = true;
        }
    }
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
    {
        if (!allInflowsHaveArrived(currentTime, timestepEndTime))
        {
            CkError("ERROR in MeshElement::receiveInflowsAndUpdateState: It is an error to call receiveInflowsAndUpdateState when allInflowsHaveArrived is false.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Receive lateral inflow water.
        for (it = neighbors.begin(); it != neighbors.end(); ++it)
        {
            switch(it->first.localEndpoint)
            {
                case MESH_SURFACE:
                    if (0.0 > it->second.getNominalFlowRate())
                    {
                        surfaceWater    += it->second.receiveWater(timestepEndTime) / elementArea;
                    }
                    break;
                case MESH_SOIL:
                    if (0.0 > it->second.getNominalFlowRate())
                    {
                        soilRecharge    += it->second.receiveWater(timestepEndTime) / elementArea;
                    }
                    break;
                case MESH_AQUIFER:
                    if (0.0 > it->second.getNominalFlowRate())
                    {
                        aquiferRecharge += it->second.receiveWater(timestepEndTime) / elementArea;
                    }
                    break;
                case IRRIGATION_RECIPIENT:
                    if (0.0 > it->second.getNominalFlowRate())
                    {
                        surfaceWater    += it->second.receiveWater(timestepEndTime) / elementArea;
                    }
                    break;
                case CHANNEL_SURFACE:
                case BOUNDARY_INFLOW:
                case BOUNDARY_OUTFLOW:
                case TRANSBASIN_INFLOW:
                case TRANSBASIN_OUTFLOW:
                case RESERVOIR_RELEASE:
                case RESERVOIR_RECIPIENT:
                case IRRIGATION_DIVERSION:
                default:
                    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                    {
                        CkAssert(false); // These endpoints are invalid here.
                    }
                    break;
            }
        }
        
        // Move water through impedance layer.
        if (soilExists)
        {
            if (UNSATURATED_AQUIFER == groundwaterMode && impedanceConductivity * dt >= soilRecharge)
            {
                aquiferRecharge += soilRecharge;
                soilRecharge     = 0.0;
            }
            else
            {
                impedanceFlow = std::min(impedanceConductivity, soilWater.kThetaAtBottomOfSoil()) * dt;
                
                if (aquiferExists)
                {
                    aquiferRecharge += impedanceFlow;
                }
                else
                {
                    deepGroundwater += impedanceFlow;
                }
                
                soilRecharge -= impedanceFlow;
            }
        }
        
        // Run aquifer capillary fringe solver.
        // If groundwaterMode is SATURATED_AQUIFER there is no need to run this solver because the aquifer will always start and end full so no water movement.
        if (aquiferExists && SATURATED_AQUIFER != groundwaterMode)
        {
            aquiferRecharge += aquiferWater.capillaryFringeSolver(aquiferHead);
        }
        
        // Update water table heads and resolve recharge.
        if (SATURATED_AQUIFER == groundwaterMode)
        {
            if (0.0 <= aquiferRecharge)
            {
                soilRecharge   += aquiferRecharge;
                aquiferRecharge = 0.0;
                resolveSoilRechargeSaturatedAquifer();
            }
            else
            {
                groundwaterMode = PERCHED_WATER_TABLE;
                perchedHead     = aquiferHead;
                aquiferHead     = elementZ - soilThickness;
                resolveAquiferRechargeUnsaturatedAquifer();
                resolveSoilRechargePerchedWaterTable();
            }
        }
        else if (UNSATURATED_AQUIFER == groundwaterMode)
        {
            resolveAquiferRechargeUnsaturatedAquifer();
            
            if (0.0 < soilRecharge)
            {
                groundwaterMode = PERCHED_WATER_TABLE;
                perchedHead     = aquiferHead;
                resolveSoilRechargePerchedWaterTable();
            }
        }
        else if (PERCHED_WATER_TABLE == groundwaterMode)
        {
            resolveAquiferRechargeUnsaturatedAquifer();
            
            if (!aquiferWater.isFull())
            {
                resolveSoilRechargePerchedWaterTable();
            }
        }
        else if (soilExists)
        {
            resolveSoilRechargePerchedWaterTable();
        }
        else if (aquiferExists)
        {
            updateHead(aquiferHead, aquiferRecharge, elementZ);
            addOrRemoveWater(aquiferWater, aquiferRecharge);
            
            if (aquiferWater.isFull())
            {
                aquiferHead     = elementZ;
                surfaceWater   += aquiferRecharge;
                aquiferRecharge = 0.0;
            }
        }
        
        // Check invariants on groundwater state.
        if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
        {
            CkAssert(0.0 == soilRecharge);
            CkAssert(0.0 == aquiferRecharge);
            
            if (soilExists && aquiferExists)
            {
                CkAssert(SATURATED_AQUIFER == groundwaterMode || UNSATURATED_AQUIFER == groundwaterMode || PERCHED_WATER_TABLE == groundwaterMode);
            }
            else
            {
                CkAssert(NO_MULTILAYER == groundwaterMode);
            }
            
            if (SATURATED_AQUIFER == groundwaterMode)
            {
                CkAssert(aquiferWater.isFull());
                CkAssert(elementZ - soilThickness <= aquiferHead);
            }
            else if (UNSATURATED_AQUIFER == groundwaterMode)
            {
                CkAssert(!aquiferWater.isFull());
                CkAssert(elementZ - soilThickness > aquiferHead);
            }
            else if (PERCHED_WATER_TABLE == groundwaterMode)
            {
                CkAssert(!aquiferWater.isFull());
                CkAssert(elementZ - soilThickness > aquiferHead);
                CkAssert(perchedHead > aquiferHead);
            }
        }
    }
    
    return error;
}

void MeshElement::fillInEvapoTranspirationSoilMoistureStruct(EvapoTranspirationSoilMoistureStruct& evapoTranspirationSoilMoisture)
{
    // FIXME implement
}

void MeshElement::updateHead(double& head, double recharge, double maxHead)
{
    head += recharge; // FIXME adjust for specific yield
    head  = std::min(head, maxHead);
}

void MeshElement::addOrRemoveWater(WaterDataStructure& water, double& recharge)
{
    // FIXME implement
}

void MeshElement::resolveSoilRechargeSaturatedAquifer()
{
    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    {
        CkAssert(SATURATED_AQUIFER == groundwaterMode);
    }
    
    updateHead(aquiferHead, soilRecharge, elementZ);
    
    if (elementZ - soilThickness > aquiferHead)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        {
            CkAssert(0.0 > soilRecharge);
        }
        
        groundwaterMode = UNSATURATED_AQUIFER;
        addOrRemoveWater(aquiferWater, soilRecharge);
    }
    else
    {
        addOrRemoveWater(soilWater, soilRecharge);
        
        if (soilWater.isFull())
        {
            aquiferHead   = elementZ;
            surfaceWater += soilRecharge;
            soilRecharge  = 0.0;
        }
    }
}

void MeshElement::resolveSoilRechargePerchedWaterTable()
{
    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    {
        CkAssert(PERCHED_WATER_TABLE == groundwaterMode || NO_MULTILAYER == groundwaterMode);
    }
    
    updateHead(perchedHead, soilRecharge, elementZ);
    addOrRemoveWater(soilWater, soilRecharge);
    
    if (soilWater.isFull())
    {
        perchedHead   = elementZ;
        surfaceWater += soilRecharge;
        soilRecharge  = 0.0;
    }
    
    if (perchedHead <= aquiferHead)
    {
        groundwaterMode = UNSATURATED_AQUIFER;
    }
}

void MeshElement::resolveAquiferRechargeUnsaturatedAquifer()
{
    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    {
        CkAssert(UNSATURATED_AQUIFER == groundwaterMode || PERCHED_WATER_TABLE == groundwaterMode);
    }
    
    updateHead(aquiferHead, aquiferRecharge, elementZ - soilThickness - epsilon(0.0));
    addOrRemoveWater(aquiferWater, aquiferRecharge);
    
    if (aquiferWater.isFull())
    {
        if (PERCHED_WATER_TABLE == groundwaterMode && elementZ - soilThickness < perchedHead)
        {
            aquiferHead = perchedHead;
        }
        else
        {
            aquiferHead = elementZ - soilThickness;
        }
        
        groundwaterMode = SATURATED_AQUIFER;
        soilRecharge   += aquiferRecharge;
        aquiferRecharge = 0.0;
        resolveSoilRechargeSaturatedAquifer();
    }
}
