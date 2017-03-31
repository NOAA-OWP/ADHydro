#include "mesh_element.h"
#include "readonly.h"

// FIXME stubs
static size_t numberOfMeshElements = 2;
// FIXME end stubs

bool MeshElement::checkInvariant() const
{
    bool                                                        error = false; // Error flag.
    std::map<NeighborConnection, NeighborProxy>::const_iterator it;            // Loop iterator.
    
    if (!(elementNumber < numberOfMeshElements))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: elementNumber must be less than numberOfMeshElements.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 < elementArea))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: elementArea must be greater than zero.\n", elementNumber);
        error = true;
    }
    
    if (!(-M_PI / 2.0 <= latitude && M_PI / 2.0 >= latitude))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: latitude must be greater than or equal to negative PI over two and less than or equal to PI over two.\n", elementNumber);
        error = true;
    }
    
    if (!(-M_PI * 2.0 <= longitude && M_PI * 2.0 >= longitude))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: longitude must be greater than or equal to negative two PI and less than or equal to two PI.\n", elementNumber);
        error = true;
    }
    
    if (!(1 <= vegetationType && 27 >= vegetationType))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: vegetationType must be greater than or equal to 1 and less than or equal to 27.\n", elementNumber);
        error = true;
    }
    else if (16 == vegetationType)
    {
        if (2 <= Readonly::verbosityLevel)
        {
            CkError("WARNING in MeshElement::checkInvariant, element %lu: vegetationType is 'Water Bodies'.  Should be a waterbody instead.\n", elementNumber);
        }
    }
    else if (24 == vegetationType)
    {
        if (2 <= Readonly::verbosityLevel)
        {
            CkError("WARNING in MeshElement::checkInvariant, element %lu: vegetationType is 'Snow or Ice'.  Should be an icemass instead.\n", elementNumber);
        }
    }
    
    if (!(1 <= groundType && 19 >= groundType))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: groundType must be greater than or equal to 1 and less than or equal to 19.\n", elementNumber);
        error = true;
    }
    else if (14 == groundType)
    {
        if (2 <= Readonly::verbosityLevel)
        {
            CkError("WARNING in MeshElement::checkInvariant, element %lu: groundType is 'WATER'.  Should be a waterbody instead.\n", elementNumber);
        }
    }
    else if (16 == groundType)
    {
        if (2 <= Readonly::verbosityLevel)
        {
            CkError("WARNING in MeshElement::checkInvariant, element %lu: groundType is 'OTHER(land-ice)'.  Should be an icemass instead.\n", elementNumber);
        }
    }
    
    if (!(0.0 < manningsN))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: manningsN must be greater than zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= impedanceConductivity))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: impedanceConductivity must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= deepConductivity))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: deepConductivity must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    error = checkEvapoTranspirationForcingStructInvariant(&evapoTranspirationForcing) || error;
    error = checkEvapoTranspirationStateStructInvariant(&evapoTranspirationState)     || error;
    
    if (!(0.0 <= surfaceWater))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: surfaceWater must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    switch (groundwaterMode)
    {
        case SATURATED_AQUIFER:
            if (!(soilExists && aquiferExists))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is SATURATED_AQUIFER then both soil and aquifer must exist.\n", elementNumber);
                error = true;
            }
            
            if (!(aquiferWater.isFull()))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is SATURATED_AQUIFER then aquiferWater must be full.\n", elementNumber);
                error = true;
            }
            
            if (!(elementZ - soilThickness() <= aquiferHead && aquiferHead <= elementZ))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is SATURATED_AQUIFER then aquiferHead must be greater than or equal to the top of the aquifer "
                        "and less than or equal to the land surface.\n", elementNumber);
                error = true;
            }
            break;
        case UNSATURATED_AQUIFER:
            if (!(soilExists && aquiferExists))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is UNSATURATED_AQUIFER then both soil and aquifer must exist.\n", elementNumber);
                error = true;
            }
            
            if (!(!aquiferWater.isFull()))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is UNSATURATED_AQUIFER then aquiferWater must not be full.\n", elementNumber);
                error = true;
            }
            
            if (!(elementZ - soilThickness() > aquiferHead))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is UNSATURATED_AQUIFER then aquiferHead must be less than the top of the aquifer.\n", elementNumber);
                error = true;
            }
            break;
        case PERCHED_WATER_TABLE:
            if (!(soilExists && aquiferExists))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is PERCHED_WATER_TABLE then both soil and aquifer must exist.\n", elementNumber);
                error = true;
            }
            
            if (!(!aquiferWater.isFull()))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is PERCHED_WATER_TABLE then aquiferWater must not be full.\n", elementNumber);
                error = true;
            }
            
            if (!(aquiferHead <= perchedHead && perchedHead <= elementZ))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is PERCHED_WATER_TABLE then perchedHead must be greater than or equal to aquiferHead and less than or equal to the land surface.\n", elementNumber);
                error = true;
            }
            
            if (!(elementZ - soilThickness() > aquiferHead))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is PERCHED_WATER_TABLE then aquiferHead must be less than the top of the aquifer.\n", elementNumber);
                error = true;
            }
            break;
        case NO_MULTILAYER:
            if (soilExists)
            {
                if (!(!aquiferExists))
                {
                    CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is NO_MULTILAYER then soil and aquifer cannot both exist.\n", elementNumber);
                    error = true;
                }
                
                if (!(perchedHead <= elementZ))
                {
                    CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is NO_MULTILAYER and soilExists then perchedHead must be less than or equal to the land surface.\n", elementNumber);
                    error = true;
                }
                
                if (!(-INFINITY == aquiferHead))
                {
                    CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is NO_MULTILAYER and soilExists then aquiferHead must be -INFINITY.\n", elementNumber);
                    error = true;
                }
            }
            else if (aquiferExists)
            {
                if (!(aquiferHead <= elementZ))
                {
                    CkError("ERROR in MeshElement::checkInvariant, element %lu: if groundwaterMode is NO_MULTILAYER and aquiferExists then aquiferHead must be less than or equal to the land surface.\n", elementNumber);
                    error = true;
                }
            }
            break;
        default:
            CkError("ERROR in MeshElement::checkInvariant, element %lu: invalid groundwaterMode %d.\n", elementNumber, groundwaterMode);
            error = true;
            break;
    }
    
    error = soilWater.checkInvariant() || error;
    
    // soilRecharge can change within a timestep, but must always return to zero at the end of a timestep, and checkInvariant can only be called between timesteps.
    if (!(0.0 == soilRecharge))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: soilRecharge must be zero at the time the invariant is checked.\n", elementNumber);
        error = true;
    }
    
    error = aquiferWater.checkInvariant() || error;
    
    // aquiferRecharge can change within a timestep, but must always return to zero at the end of a timestep, and checkInvariant can only be called between timesteps.
    if (!(0.0 == aquiferRecharge))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: aquiferRecharge must be zero at the time the invariant is checked.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= deepGroundwater))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: deepGroundwater must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 >= precipitationRate))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: precipitationRate must be less than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 >= precipitationCumulativeShortTerm))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: precipitationCumulativeShortTerm must be less than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 >= precipitationCumulativeLongTerm))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: precipitationCumulativeLongTerm must be less than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= transpirationRate))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: transpirationRate must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= transpirationCumulativeShortTerm))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: transpirationCumulativeShortTerm must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= transpirationCumulativeLongTerm))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: transpirationCumulativeLongTerm must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    for (it = neighbors.begin(); it != neighbors.end(); ++it)
    {
        error = it->first.checkInvariant()  || error;
        error = it->second.checkInvariant() || error;
        
        switch (it->first.localEndpoint)
        {
            case MESH_SURFACE:
                // This is valid and has no other conditions.
                break;
            case MESH_SOIL:
                if (!soilExists)
                {
                    CkError("ERROR in MeshElement::checkInvariant, element %lu: if localEndpoint is MESH_SOIL then soil must exist.\n", elementNumber);
                    error = true;
                }
                break;
            case MESH_AQUIFER:
                if (!aquiferExists)
                {
                    CkError("ERROR in MeshElement::checkInvariant, element %lu: if localEndpoint is MESH_AQUIFER then aquifer must exist.\n", elementNumber);
                    error = true;
                }
                break;
            case IRRIGATION_RECIPIENT:
                // This connection type is a one way flow where the two neighbors never communicate to calculate nominalFlowRate.  Instead, the sender just sends water.
                // We handle this at the recipient by setting nominalFlowRate to -INFINITY and expirationTime to INFINITY.  This is equivalent to saying that the pair has negotiated
                // that the remote element is the sender, the flow rate will be less than or equal to infinity, and they will meet again to renegotiate at the end of time.
                // This does not apply to natural boundary and transbasin inflows.  In those cases, there is no other neighbor and the recipient calculates nominalFlowRate.
                if (!(-INFINITY == it->second.getNominalFlowRate() && INFINITY == it->second.getExpirationTime()))
                {
                    CkError("ERROR in MeshElement::checkInvariant, element %lu: For a one way inflow nominalFlowRate must be minus infinity and expirationTime must be infinity.\n", elementNumber);
                    error = true;
                }
                break;
            default:
                CkError("ERROR in MeshElement::checkInvariant, element %lu: localEndpoint must be a mesh element.\n", elementNumber);
                error = true;
                break;
        }
        
        if (!(elementNumber == it->first.localElementNumber))
        {
            CkError("ERROR in MeshElement::checkInvariant, element %lu: localElementNumber must be elementNumber.\n", elementNumber);
            error = true;
        }
        
        if (BOUNDARY_INFLOW == it->first.remoteEndpoint || TRANSBASIN_INFLOW == it->first.remoteEndpoint)
        {
            if (!(0.0 >= it->second.getNominalFlowRate()))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: For a boundary inflow nominalFlowRate must be less than or equal to zero.\n", elementNumber);
                error = true;
            }
        }
        else if (BOUNDARY_OUTFLOW == it->first.remoteEndpoint || TRANSBASIN_OUTFLOW == it->first.remoteEndpoint)
        {
            if (!(0.0 <= it->second.getNominalFlowRate()))
            {
                CkError("ERROR in MeshElement::checkInvariant, element %lu: For a boundary outflow nominalFlowRate must be greater than or equal to zero.\n", elementNumber);
                error = true;
            }
        }
    }
    
    if (!(neighbors.size() >= neighborsFinished))
    {
        CkError("ERROR in MeshElement::checkInvariant, element %lu: neighborsFinished must be less than or equal to neighbors.size().\n", elementNumber);
        error = true;
    }
    
    return error;
}

bool MeshElement::receiveMessage(const Message& message, size_t& elementsFinished, double currentTime, double timestepEndTime)
{
    bool                                                  error = false;                               // Error flag.
    std::map<NeighborConnection, NeighborProxy>::iterator it    = neighbors.find(message.destination); // Iterator for finding correct NeighborProxy.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = message.checkInvariant();
        
        if (!(neighbors.end() != it))
        {
            CkError("ERROR in MeshElement::receiveNeighborAttributes: received a NeighborMessage with a NeighborConnection that I do not have.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        error = message.receive(it->second, neighborsFinished, currentTime, timestepEndTime);
    }
    
    // Check if this element is finished
    if (!error && neighbors.size() == neighborsFinished)
    {
        ++elementsFinished;
    }
    
    return error;
}

bool MeshElement::sendNeighborAttributes(std::map<size_t, std::vector<NeighborMessage> >& outgoingMessages, size_t& elementsFinished)
{
    bool                                                  error = false; // Error flag.
    std::map<NeighborConnection, NeighborProxy>::iterator it;            // Loop iterator.
    
    // Don't error check parameters because it's a simple pass-through to NeighborProxy::calculateNominalFlowRate and it will be checked inside that method.
    
    neighborsFinished = 0;
    
    for (it = neighbors.begin(); !error && it != neighbors.end(); ++it)
    {
        error = it->second.sendNeighborMessage(outgoingMessages, neighborsFinished, NeighborMessage(it->first, NeighborAttributes(0)));
    }
    
    // Check if this element is finished.
    if (!error && neighbors.size() == neighborsFinished)
    {
        ++elementsFinished;
    }
    
    return error;
}

bool MeshElement::calculateNominalFlowRates(std::map<size_t, std::vector<StateMessage> >& outgoingMessages, size_t& elementsFinished, double currentTime)
{
    bool                                                  error = false; // Error flag.
    std::map<NeighborConnection, NeighborProxy>::iterator it;            // Loop iterator.
    
    // Don't error check parameters because it's a simple pass-through to NeighborProxy::calculateNominalFlowRate and it will be checked inside that method.
    
    neighborsFinished = 0;
    
    for (it = neighbors.begin(); !error && it != neighbors.end(); ++it)
    {
        error = it->second.calculateNominalFlowRate(outgoingMessages, neighborsFinished, it->first, currentTime);
    }
    
    // Check if this element is finished.
    if (!error && neighbors.size() == neighborsFinished)
    {
        ++elementsFinished;
    }
    
    return error;
}

double MeshElement::minimumExpirationTime()
{
    double                                                minimumTime = INFINITY; // Return value gets set to the minimum of expirationTime for all NeighborProxies.
    std::map<NeighborConnection, NeighborProxy>::iterator it;                     // Loop iterator.
    
    for (it = neighbors.begin(); it != neighbors.end(); ++it)
    {
        minimumTime = std::min(minimumTime, it->second.getExpirationTime());
    }
    
    return minimumTime;
}

bool MeshElement::doPointProcessesAndSendOutflows(std::map<size_t, std::vector<WaterMessage> >& outgoingMessages, size_t& elementsFinished, double currentTime, double timestepEndTime)
{
    bool   error                = false;                                 // Error flag.
    double localSolarDateTime   = Readonly::referenceDate + (currentTime / (24.0 * 60.0 * 60.0)) + (longitude / (2.0 * M_PI)); // (days) Julian date converted from UTC to local solar time.
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
        if (!(currentTime < timestepEndTime))
        {
            CkError("ERROR in MeshElement::doPointProcessesAndSendOutflows: currentTime must be less than timestepEndTime.\n");
            error = true;
        }
    }
    
    // Check invariants on groundwater state.  The processing below that modifies these is fairly complicated.
    // I feel that an extra check on these at the beginning and end of that processing is warranted in addition to the normal checkInvariant calls.
    // FIXME maybe this could be eliminated later.
    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    {
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
            CkAssert(elementZ - soilThickness() <= aquiferHead && aquiferHead <= elementZ);
        }
        else if (UNSATURATED_AQUIFER == groundwaterMode)
        {
            CkAssert(!aquiferWater.isFull());
            CkAssert(elementZ - soilThickness() > aquiferHead);
        }
        else if (PERCHED_WATER_TABLE == groundwaterMode)
        {
            CkAssert(!aquiferWater.isFull());
            CkAssert(aquiferHead <= perchedHead && perchedHead <= elementZ);
            CkAssert(elementZ - soilThickness() > aquiferHead);
        }
        else if (NO_MULTILAYER == groundwaterMode && soilExists)
        {
            CkAssert(perchedHead <= elementZ);
            CkAssert(-INFINITY == aquiferHead);
        }
        else if (NO_MULTILAYER == groundwaterMode && aquiferExists)
        {
            CkAssert(aquiferHead <= elementZ);
        }
        
        CkAssert(0.0 == soilRecharge);
        CkAssert(0.0 == aquiferRecharge);
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
        error = evapoTranspirationSoil(vegetationType, groundType, latitude, yearlen, julian, cosZ, dt, sqrt(elementArea), &evapoTranspirationForcing,
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
        
        // Take evaporation first from surfaceWater, and then if there isn't enough surfaceWater take the rest from subsurface water.
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
        else if ((2 <= Readonly::verbosityLevel && 0.1 < unsatisfiedEvaporation + unsatisfiedTranspiration) || (3 <= Readonly::verbosityLevel && 0.0 < unsatisfiedEvaporation + unsatisfiedTranspiration))
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
            error = soilWater.doTimestep(soilRecharge, surfaceWater, elementZ - ((PERCHED_WATER_TABLE == groundwaterMode || NO_MULTILAYER == groundwaterMode) ? perchedHead : aquiferHead), dt);
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
    }
    
    if (!error)
    {
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
                default:
                    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                    {
                        CkAssert(false); // All other endpoints are invalid here.
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
        if (soilExists && (PERCHED_WATER_TABLE == groundwaterMode || NO_MULTILAYER == groundwaterMode) && perchedHead > elementZ - soilThickness())
        {
            soilOutflowFraction    = 1.0;
            soilRecharge          -= (totalSoilOutflows / elementArea) * dt;
        }
        else
        {
            soilOutflowFraction    = 0.0;
        }
        
        if (aquiferExists && aquiferHead > elementZ - soilThickness() - aquiferThickness())
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
    
        neighborsFinished = 0;
    }
    
    for (it = neighbors.begin(); !error && it != neighbors.end(); ++it)
    {
        // Count up the neighbors that don't need to receive any water messages.
        if (0.0 <= it->second.getNominalFlowRate() || it->second.allWaterHasArrived(it->first, currentTime, timestepEndTime))
        {
            ++neighborsFinished;
        }
        
        // Send lateral outflow water.  Even if actual flow is zero a message must be sent to let the receiver know what the actual flow is.
        switch(it->first.localEndpoint)
        {
            case MESH_SURFACE:
                if (0.0 < it->second.getNominalFlowRate())
                {
                    error = it->second.sendWater(outgoingMessages, WaterMessage(it->first, WaterTransfer(it->second.getNominalFlowRate() * dt * surfaceOutflowFraction, currentTime, timestepEndTime)));
                }
                break;
            case MESH_SOIL:
                if (0.0 < it->second.getNominalFlowRate())
                {
                    error = it->second.sendWater(outgoingMessages, WaterMessage(it->first, WaterTransfer(it->second.getNominalFlowRate() * dt * soilOutflowFraction,    currentTime, timestepEndTime)));
                }
                break;
            case MESH_AQUIFER:
                if (0.0 < it->second.getNominalFlowRate())
                {
                    error = it->second.sendWater(outgoingMessages, WaterMessage(it->first, WaterTransfer(it->second.getNominalFlowRate() * dt * aquiferOutflowFraction, currentTime, timestepEndTime)));
                }
                break;
            case IRRIGATION_RECIPIENT:
                // This can't be an outflow so do nothing.
                break;
            default:
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                {
                    CkAssert(false); // All other endpoints are invalid here.
                }
                break;
        }
    }
    
    // Check if this element is finished.
    if (!error && neighbors.size() == neighborsFinished)
    {
        ++elementsFinished;
    }
    
    return error;
}

bool MeshElement::receiveInflowsAndUpdateState(double currentTime, double timestepEndTime)
{
    bool                                                  error = false;                         // Error flag.
    std::map<NeighborConnection, NeighborProxy>::iterator it;                                    // Iterator over neighbor proxies.
    double                                                dt    = timestepEndTime - currentTime; // (s) duration of timestep.
    double                                                impedanceFlow;                         // (m) Water that makes it through the impedance layer.
    double                                                dummy = 0.0;                           // aquiferWater.doTimestep needs a reference parameter for surfaceWater, but we always pass zero and it is then unmodified.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(currentTime < timestepEndTime))
        {
            CkError("ERROR in MeshElement::receiveInflowsAndUpdateState: currentTime must be less than timestepEndTime.\n");
            error = true;
        }
        
        if (!(neighbors.size() == neighborsFinished))
        {
            CkError("ERROR in MeshElement::receiveInflowsAndUpdateState: It is an error to call receiveInflowsAndUpdateState when not all NeighborProxies are finished receiveing water.\n");
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
                case IRRIGATION_RECIPIENT:
                    if (0.0 > it->second.getNominalFlowRate())
                    {
                        surfaceWater    += it->second.receiveWater(it->first, currentTime, timestepEndTime) / elementArea;
                    }
                    break;
                case MESH_SOIL:
                    if (0.0 > it->second.getNominalFlowRate())
                    {
                        soilRecharge    += it->second.receiveWater(it->first, currentTime, timestepEndTime) / elementArea;
                    }
                    break;
                case MESH_AQUIFER:
                    if (0.0 > it->second.getNominalFlowRate())
                    {
                        aquiferRecharge += it->second.receiveWater(it->first, currentTime, timestepEndTime) / elementArea;
                    }
                    break;
                default:
                    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                    {
                        CkAssert(false); // All other endpoints are invalid here.
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
                impedanceFlow = std::min(impedanceConductivity, soilWater.conductivityAtDepth(soilThickness())) * dt;
                
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
            error = aquiferWater.doTimestep(aquiferRecharge, dummy, elementZ - soilThickness() - aquiferHead, dt);
        }
    }
    
    if (!error)
    {
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
                aquiferHead     = elementZ - soilThickness();
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
        else if (soilExists) // NO_MULTILAYER == groundwaterMode.
        {
            resolveSoilRechargePerchedWaterTable();
        }
        else if (aquiferExists) // NO_MULTILAYER == groundwaterMode.
        {
            updateHead(aquiferHead, aquiferRecharge, elementZ);
            aquiferWater.addOrRemoveWater(aquiferRecharge, aquiferWaterCreated);
            
            if (aquiferWater.isFull())
            {
                aquiferHead     = elementZ;
                surfaceWater   += aquiferRecharge;
                aquiferRecharge = 0.0;
            }
        }
        
        // Check invariants on groundwater state.  The processing below that modifies these is fairly complicated.
        // I feel that an extra check on these at the beginning and end of that processing is warranted in addition to the normal checkInvariant calls.
        // FIXME maybe this could be eliminated later.
        if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
        {
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
                CkAssert(elementZ - soilThickness() <= aquiferHead && aquiferHead <= elementZ);
            }
            else if (UNSATURATED_AQUIFER == groundwaterMode)
            {
                CkAssert(!aquiferWater.isFull());
                CkAssert(elementZ - soilThickness() > aquiferHead);
            }
            else if (PERCHED_WATER_TABLE == groundwaterMode)
            {
                CkAssert(!aquiferWater.isFull());
                CkAssert(aquiferHead <= perchedHead && perchedHead <= elementZ);
                CkAssert(elementZ - soilThickness() > aquiferHead);
            }
            else if (NO_MULTILAYER == groundwaterMode && soilExists)
            {
                CkAssert(perchedHead <= elementZ);
                CkAssert(-INFINITY == aquiferHead);
            }
            else if (NO_MULTILAYER == groundwaterMode && aquiferExists)
            {
                CkAssert(aquiferHead <= elementZ);
            }
            
            CkAssert(0.0 == soilRecharge);
            CkAssert(0.0 == aquiferRecharge);
        }
    }
    
    return error;
}

void MeshElement::fillInEvapoTranspirationSoilMoistureStruct(EvapoTranspirationSoilMoistureStruct& evapoTranspirationSoilMoisture)
{
    int    ii;               // Loop counter.
    double layerMiddleDepth; // (m) The depth of the middle of a Noah-MP layer expressed as a positive number.
    double waterContent;     // (m^3/m^3) The water content of a Noah-MP layer.
    
    // Set water content for layers.
    for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ++ii)
    {
        layerMiddleDepth = -0.5 * (evapoTranspirationState.zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] +     // zSnso is negative.
                                   evapoTranspirationState.zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1]); // layerMiddleDepth is positive.
        
        if (soilThickness() > layerMiddleDepth)
        {
            waterContent = soilWater.waterContentAtDepth(layerMiddleDepth);
        }
        else if (soilThickness() + aquiferThickness() > layerMiddleDepth)
        {
            waterContent = aquiferWater.waterContentAtDepth(layerMiddleDepth - soilThickness());
        }
        else if (aquiferExists)
        {
            waterContent = aquiferWater.waterContentAtDepth(aquiferThickness());
        }
        else if (soilExists)
        {
            waterContent = soilWater.waterContentAtDepth(soilThickness());
        }
        else
        {
            waterContent = 0.01; // Noah-MP can't handle water content of zero so even if there's no soil or aquifer we need to fill in something positive.
        }
        
        // Set minimum residual water content.
        if (0.01 > waterContent)
        {
            waterContent = 0.01;
        }
        
        evapoTranspirationSoilMoisture.smcEq[ii] = waterContent;
        evapoTranspirationSoilMoisture.sh2o[ii]  = waterContent;
        evapoTranspirationSoilMoisture.smc[ii]   = waterContent;
    }
    
    // Set water table depth.
    if (PERCHED_WATER_TABLE == groundwaterMode || (NO_MULTILAYER == groundwaterMode && soilExists))
    {
        evapoTranspirationSoilMoisture.zwt = elementZ - perchedHead;
    }
    else if (aquiferExists)
    {
        evapoTranspirationSoilMoisture.zwt = elementZ - aquiferHead;
    }
    else
    {
        evapoTranspirationSoilMoisture.zwt = 0.0;
    }
    
    // Make the lowest layer water content extend down forever.
    evapoTranspirationSoilMoisture.smcwtd = evapoTranspirationSoilMoisture.smc[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS - 1];
}

void MeshElement::updateHead(double& head, double recharge, double maxHead)
{
    head += recharge; // FIXME adjust for specific yield
    head  = std::min(head, maxHead);
}

void MeshElement::resolveSoilRechargeSaturatedAquifer()
{
    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    {
        CkAssert(SATURATED_AQUIFER == groundwaterMode);
    }
    
    updateHead(aquiferHead, soilRecharge, elementZ);
    
    if (elementZ - soilThickness() > aquiferHead)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        {
            CkAssert(0.0 > soilRecharge);
        }
        
        groundwaterMode = UNSATURATED_AQUIFER;
        aquiferWater.addOrRemoveWater(soilRecharge, aquiferWaterCreated);
    }
    else
    {
        soilWater.addOrRemoveWater(soilRecharge, soilWaterCreated);
        
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
        CkAssert(PERCHED_WATER_TABLE == groundwaterMode || (NO_MULTILAYER == groundwaterMode && soilExists));
    }
    
    updateHead(perchedHead, soilRecharge, elementZ);
    soilWater.addOrRemoveWater(soilRecharge, soilWaterCreated);
    
    if (soilWater.isFull())
    {
        perchedHead   = elementZ;
        surfaceWater += soilRecharge;
        soilRecharge  = 0.0;
    }
    else if (perchedHead <= aquiferHead)
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
    
    updateHead(aquiferHead, aquiferRecharge, elementZ - soilThickness() - epsilon(0.0));
    aquiferWater.addOrRemoveWater(aquiferRecharge, aquiferWaterCreated);
    
    if (aquiferWater.isFull())
    {
        if (PERCHED_WATER_TABLE == groundwaterMode && elementZ - soilThickness() < perchedHead)
        {
            aquiferHead = perchedHead;
        }
        else
        {
            aquiferHead = elementZ - soilThickness();
        }
        
        groundwaterMode = SATURATED_AQUIFER;
        soilRecharge   += aquiferRecharge;
        aquiferRecharge = 0.0;
        resolveSoilRechargeSaturatedAquifer();
    }
}
