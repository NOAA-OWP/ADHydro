#include "channel_element.h"
#include "readonly.h"

// FIXME stubs
static size_t numberOfChannelElements;
// FIXME end stubs

bool ChannelElement::checkInvariant() const
{
    bool                                                        error = false; // Error flag.
    std::map<NeighborConnection, NeighborProxy>::const_iterator it;            // Loop iterator.
    
    if (!(elementNumber < numberOfChannelElements))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: elementNumber must be less than numberOfChannelElements.\n", elementNumber);
        error = true;
    }
    
    if (!(STREAM == channelType || WATERBODY == channelType || ICEMASS == channelType))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: channelType must be STREAM or WATERBODY or ICEMASS.\n", elementNumber);
        error = true;
    }
    
    if (!(elementZBank >= elementZBed))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: elementZBank must be greater than or equal to elementZBed.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 < elementLength))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: elementLength must be greater than zero.\n", elementNumber);
        error = true;
    }
    
    if (!(-M_PI / 2.0 <= latitude && M_PI / 2.0 >= latitude))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: latitude must be greater than or equal to negative PI over two and less than or equal to PI over two.\n", elementNumber);
        error = true;
    }
    
    if (!(-M_PI * 2.0 <= longitude && M_PI * 2.0 >= longitude))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: longitude must be greater than or equal to negative two PI and less than or equal to two PI.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= baseWidth))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: baseWidth must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= sideSlope))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: sideSlope must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 < baseWidth || 0.0 < sideSlope))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: one of baseWidth or sideSlope must be greater than zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 < manningsN))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: manningsN must be greater than zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 < bedThickness))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: bedThickness must be greater than zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 <= bedConductivity))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: bedConductivity must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    error = checkEvapoTranspirationForcingStructInvariant(&evapoTranspirationForcing) || error;
    error = checkEvapoTranspirationStateStructInvariant(&evapoTranspirationState)     || error;
    
    if (!(0.0 <= surfaceWater))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: surfaceWater must be greater than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 >= precipitationRate))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: precipitationRate must be less than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 >= precipitationCumulativeShortTerm))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: precipitationCumulativeShortTerm must be less than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    if (!(0.0 >= precipitationCumulativeLongTerm))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: precipitationCumulativeLongTerm must be less than or equal to zero.\n", elementNumber);
        error = true;
    }
    
    for (it = neighbors.begin(); it != neighbors.end(); ++it)
    {
        error = it->first.checkInvariant()  || error;
        error = it->second.checkInvariant() || error;
        
        switch (it->first.localEndpoint)
        {
            case CHANNEL_SURFACE:
            case RESERVOIR_RELEASE:
            case IRRIGATION_DIVERSION:
                // These are valid and have no other conditions.
                break;
            case RESERVOIR_RECIPIENT:
                // This connection type is a one way flow where the two neighbors never communicate to calculate nominalFlowRate.  Instead, the sender just sends water.
                // We handle this at the recipient by setting nominalFlowRate to -INFINITY and expirationTime to INFINITY.  This is equivalent to saying that the pair has negotiated
                // that the remote element is the sender, the flow rate will be less than or equal to infinity, and they will meet again to renegotiate at the end of time.
                // This does not apply to natural boundary and transbasin inflows.  In those cases, there is no other neighbor and the recipient calculates nominalFlowRate.
                if (!(-INFINITY == it->second.getNominalFlowRate() && INFINITY == it->second.getExpirationTime()))
                {
                    CkError("ERROR in ChannelElement::checkInvariant, element %lu: For a one way inflow nominalFlowRate must be minus infinity and expirationTime must be infinity.\n", elementNumber);
                    error = true;
                }
                break;
            default:
                CkError("ERROR in ChannelElement::checkInvariant, element %lu: localEndpoint must be a channel element.\n", elementNumber);
                error = true;
                break;
        }
        
        if (!(elementNumber == it->first.localElementNumber))
        {
            CkError("ERROR in ChannelElement::checkInvariant, element %lu: localElementNumber must be elementNumber.\n", elementNumber);
            error = true;
        }
        
        if (BOUNDARY_INFLOW == it->first.remoteEndpoint || TRANSBASIN_INFLOW == it->first.remoteEndpoint)
        {
            if (!(0.0 >= it->second.getNominalFlowRate()))
            {
                CkError("ERROR in ChannelElement::checkInvariant, element %lu: For a boundary inflow nominalFlowRate must be less than or equal to zero.\n", elementNumber);
                error = true;
            }
        }
        else if (BOUNDARY_OUTFLOW == it->first.remoteEndpoint || TRANSBASIN_OUTFLOW == it->first.remoteEndpoint)
        {
            if (!(0.0 <= it->second.getNominalFlowRate()))
            {
                CkError("ERROR in ChannelElement::checkInvariant, element %lu: For a boundary outflow nominalFlowRate must be greater than or equal to zero.\n", elementNumber);
                error = true;
            }
        }
    }
    
    if (!(neighbors.size() >= neighborsFinished))
    {
        CkError("ERROR in ChannelElement::checkInvariant, element %lu: neighborsFinished must be less than or equal to neighbors.size().\n", elementNumber);
        error = true;
    }
    
    return error;
}

bool ChannelElement::receiveMessage(const Message& message, size_t& elementsFinished, double currentTime, double timestepEndTime)
{
    bool                                                  error = false;                               // Error flag.
    std::map<NeighborConnection, NeighborProxy>::iterator it    = neighbors.find(message.destination); // Iterator for finding correct NeighborProxy.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        error = message.checkInvariant();
        
        if (!(neighbors.end() != it))
        {
            CkError("ERROR in ChannelElement::receiveNeighborAttributes: received a NeighborMessage with a NeighborConnection that I do not have.\n");
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

bool ChannelElement::sendNeighborAttributes(std::map<size_t, std::vector<NeighborMessage> >& outgoingMessages, size_t& elementsFinished)
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

bool ChannelElement::calculateNominalFlowRates(std::map<size_t, std::vector<StateMessage> >& outgoingMessages, size_t& elementsFinished, double currentTime)
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

double ChannelElement::minimumExpirationTime()
{
    double                                                minimumTime = INFINITY; // Return value gets set to the minimum of expirationTime for all NeighborProxies.
    std::map<NeighborConnection, NeighborProxy>::iterator it;                     // Loop iterator.
    
    for (it = neighbors.begin(); it != neighbors.end(); ++it)
    {
        minimumTime = std::min(minimumTime, it->second.getExpirationTime());
    }
    
    return minimumTime;
}

bool ChannelElement::doPointProcessesAndSendOutflows(std::map<size_t, std::vector<WaterMessage> >& outgoingMessages, size_t& elementsFinished, double currentTime, double timestepEndTime)
{
    bool   error                = false;                         // Error flag.
    double localSolarDateTime   = Readonly::referenceDate + (currentTime / (24.0 * 60.0 * 60.0)) + (longitude / (2.0 * M_PI)); // (days) Julian date converted from UTC to local solar time.
    long   year;                                                 // For calculating yearlen, julian, and hourAngle.
    long   month;                                                // For calculating hourAngle.
    long   day;                                                  // For calculating hourAngle.
    long   hour;                                                 // For passing to julianToGregorian, unused.
    long   minute;                                               // For passing to julianToGregorian, unused.
    double second;                                               // For passing to julianToGregorian, unused.
    int    yearlen;                                              // (days) Year length.
    float  julian;                                               // (days) Day of year including fractional day.
    double hourAngle;                                            // (radians) How far the sun is east or west of zenith.  Positive means west.  Negative means east.  Used for calculating cosZ.
    double declinationOfSun;                                     // (radians) How far the sun is above the horizon.  Used for calculating cosZ.
    float  cosZ;                                                 // Cosine of the angle between the normal to the land surface and the sun.
    double originalEvapoTranspirationTotalWaterInDomain;         // (mm) For mass balance check.
    double dt                   = timestepEndTime - currentTime; // (s) Duration of timestep.
    float  surfacewaterAdd;                                      // (mm) Water from Noah-MP that must be added to surface water.  Must be non-negative.
    float  evaporationFromSnow;                                  // (mm) Water that Noah-MP already added to or removed from the snowpack for evaporation or condensation.
                                                                 // Positive means water evaporated off of the snowpack.  Negative means water condensed on to the snowpack.
    float  evaporationFromGround;                                // (mm) Water that must be added to or removed from the land surface for evaporation or condensation.
                                                                 // Positive means water evaporated off of the ground.  Negative means water condensed on to the ground.
    float  noahMPWaterCreated;                                   // (mm) Water that was created or destroyed by Noah-MP.  Positive means water was created.  Negative means water was destroyed.
    double topWidth             = baseWidth + 2.0 * sideSlope * surfaceWater; // (m) Width of the water top surface.
    double topArea              = topWidth * elementLength;      // (m^2) Surface area of the water top surface.
    double crossSectionalArea   = (baseWidth + sideSlope * surfaceWater) * surfaceWater; // (m^2) Wetted cross sectional area of the channel.
    double precipitation;                                        // (m) Total quantity of water precipitated this timestep.
    double evaporation;                                          // (m) Total quantity of water evaporated   this timestep.
    double unsatisfiedEvaporation;                               // (m) Remaining quantity of water needing to be evaporated.
    std::map<NeighborConnection, NeighborProxy>::iterator it;    // Iterator over neighbor proxies.
    double totalSurfaceOutflows = 0.0;                           // (m^3/s) Total of nominal flow rates of surface outflows.
    double surfaceOutflowFraction;                               // (m^3/m^3) Fraction of total nominal flow rate that can be satisfied.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(currentTime < timestepEndTime))
        {
            CkError("ERROR in ChannelElement::doPointProcessesAndSendOutflows: currentTime must be less than timestepEndTime.\n");
            error = true;
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
        
        // Save the original amount of water stored in evapoTranspirationState for mass balance check.
        originalEvapoTranspirationTotalWaterInDomain = evapoTranspirationTotalWaterInDomain(&evapoTranspirationState);
        
        // Call Noah-MP.
        // FIXME Currently this is double counting area.  Streams overlap with adjacent mesh elements and both precipitate and evaporate that area.
        if (ICEMASS == channelType)
        {
            error = evapoTranspirationGlacier(cosZ, dt, &evapoTranspirationForcing, &evapoTranspirationState, &surfacewaterAdd, &evaporationFromSnow, &evaporationFromGround, &noahMPWaterCreated);
        }
        else
        {
            error = evapoTranspirationWater(latitude, yearlen, julian, cosZ, dt, elementLength, surfaceWater * 1000.0, &evapoTranspirationForcing,
                                            &evapoTranspirationState, &surfacewaterAdd, &evaporationFromSnow, &evaporationFromGround, &noahMPWaterCreated);
        }
    }
    
    if (!error)
    {
        // Because Noah-MP uses single precision floats, its roundoff error is much higher than for doubles.  However, we can calculate the mass error using doubles and account for it in surfaceWaterCreated.  The mass error
        // is the amount of water at the end (water stored in evapoTranspirationState plus water that came out in the form of surfacewaterAdd and evaporationFromSnow) minus the amount of water at the beginning (water stored
        // in evapoTranspirationState plus water that went in in the form of precipitation).  evaporationFromGround is not used in this computation because that water was not taken out by Noah-MP. It is taken below.
        // Noah-MP also tells us if it knowingly created or destroyed any water in noahMPWaterCreated.  However, we don't use that value because this calculation subsumes that quantity at higher precision.
        surfaceWaterCreated += ((((double)evapoTranspirationTotalWaterInDomain(&evapoTranspirationState) + (double)surfacewaterAdd + (double)evaporationFromSnow) -
                                 (originalEvapoTranspirationTotalWaterInDomain + evapoTranspirationForcing.prcp * dt)) / 1000.0) * topArea; // Divide by a thousand to convert from millimeters to meters.
        
        // Add surfaceWater and record the quantities of precipitation and evaporation for this timestep.
        crossSectionalArea      += (surfacewaterAdd / 1000.0) * topWidth;         // Divide by a thousand to convert from millimeters to meters.
        precipitation            = -evapoTranspirationForcing.prcp * dt / 1000.0; // Divide by a thousand to convert from millimeters to meters.
        evaporation              = evaporationFromSnow / 1000.0;                  // Divide by a thousand to convert from millimeters to meters.
        unsatisfiedEvaporation   = evaporationFromGround / 1000.0;                // Divide by a thousand to convert from millimeters to meters.
        
        // Take evaporation from surfaceWater.  If there isn't enough surfaceWater print a warning and reduce the quantity of evaporation.
        if (crossSectionalArea     >= unsatisfiedEvaporation * topWidth)
        {
            evaporation            += unsatisfiedEvaporation;
            crossSectionalArea     -= unsatisfiedEvaporation * topWidth;
            unsatisfiedEvaporation  = 0.0;
        }
        else
        {
            evaporation            += crossSectionalArea / topWidth;
            unsatisfiedEvaporation -= crossSectionalArea / topWidth;
            crossSectionalArea      = 0.0;
            
            if ((2 <= Readonly::verbosityLevel && 0.1 < unsatisfiedEvaporation) || (3 <= Readonly::verbosityLevel && 0.0 < unsatisfiedEvaporation))
            {
                CkError("WARNING in ChannelElement::doPointProcessesAndSendOutflows, element %lu: unsatisfied evapotranspiration of %le meters.\n", elementNumber, unsatisfiedEvaporation);
            }
        }
        
        // Record flow rates and cumulative flows.
        precipitationRate = precipitation / dt;
        evaporationRate   = evaporation   / dt;
        
        if (0.0 != precipitation)
        {
            if (0.0 != precipitationCumulativeShortTerm && (precipitationCumulativeShortTerm / (precipitation * topArea)) > (precipitationCumulativeLongTerm / precipitationCumulativeShortTerm))
            {
                // The relative roundoff error of adding this timestep to short term is greater than adding short term to long term so move short term to long term.
                precipitationCumulativeLongTerm += precipitationCumulativeShortTerm;
                precipitationCumulativeShortTerm = 0.0;
            }
            
            precipitationCumulativeShortTerm += precipitation * topArea;
        }
        
        if (0.0 != evaporation)
        {
            // Absolute value must be used here and not for precipitation or transpiration because evaporation is the only one that can be either positive or negative.
            if (0.0 != evaporationCumulativeShortTerm && std::abs(evaporationCumulativeShortTerm / (evaporation * topArea)) > std::abs(evaporationCumulativeLongTerm / evaporationCumulativeShortTerm))
            {
                // The relative roundoff error of adding this timestep to short term is greater than adding short term to long term so move short term to long term.
                evaporationCumulativeLongTerm += evaporationCumulativeShortTerm;
                evaporationCumulativeShortTerm = 0.0;
            }
            
            evaporationCumulativeShortTerm += evaporation * topArea;
        }
        
        // Add up total lateral outflows.
        for (it = neighbors.begin(); it != neighbors.end(); ++it)
        {
            switch(it->first.localEndpoint)
            {
                case CHANNEL_SURFACE:
                case RESERVOIR_RELEASE:
                case IRRIGATION_DIVERSION:
                    if (0.0 < it->second.getNominalFlowRate())
                    {
                        totalSurfaceOutflows += it->second.getNominalFlowRate();
                    }
                    break;
                case RESERVOIR_RECIPIENT:
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
        if (0.0 == crossSectionalArea || 0.0 == totalSurfaceOutflows)
        {
            // I've been debating about having this check here.  It's not strictly necessary, but I think it's a good idea.  It serves two purposes.
            // First, if both surfaceWater and totalSurfaceOutflows are zero it prevents a divide by zero when calculating surfaceOutflowFraction in the next else if.
            // This actually isn't necessary because if totalSurfaceOutflows is zero there are no neighbors with an outflow so surfaceOutflowFraction is unused.
            // But I think it's bad coding style to leave an unprotected divide by zero.  The second purpose is for speed.
            // No surface water is by far the most common case so this will be just a comparison instead of math and then a comparison.
            // once again, not strictly necessary.  The other two cases would do the right thing without this check.
            surfaceOutflowFraction = 0.0;
        }
        else if (crossSectionalArea <= (totalSurfaceOutflows / elementLength) * dt)
        {
            surfaceOutflowFraction = crossSectionalArea / ((totalSurfaceOutflows / elementLength) * dt);
            crossSectionalArea     = 0.0;
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            {
                CkAssert(0.0 <= surfaceOutflowFraction && 1.0 >= surfaceOutflowFraction);
            }
            
        }
        else
        {
            surfaceOutflowFraction = 1.0;
            crossSectionalArea    -= (totalSurfaceOutflows / elementLength) * dt;
        }
        
        // Convert cross sectional area back to water depth.
        calculateSurfaceWaterDepthFromCrossSectionalArea(crossSectionalArea);
    
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
            case CHANNEL_SURFACE:
            case RESERVOIR_RELEASE:
            case IRRIGATION_DIVERSION:
                if (0.0 < it->second.getNominalFlowRate())
                {
                    error = it->second.sendWater(outgoingMessages, WaterMessage(it->first, WaterTransfer(it->second.getNominalFlowRate() * dt * surfaceOutflowFraction, currentTime, timestepEndTime)));
                }
                break;
            case RESERVOIR_RECIPIENT:
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

bool ChannelElement::receiveInflowsAndUpdateState(double currentTime, double timestepEndTime)
{
    bool                                                  error              = false;                                                 // Error flag.
    std::map<NeighborConnection, NeighborProxy>::iterator it;                                                                         // Iterator over neighbor proxies.
    double                                                crossSectionalArea = (baseWidth + sideSlope * surfaceWater) * surfaceWater; // (m^2) Wetted cross sectional area of the channel.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(currentTime < timestepEndTime))
        {
            CkError("ERROR in ChannelElement::receiveInflowsAndUpdateState: currentTime must be less than timestepEndTime.\n");
            error = true;
        }
        
        if (!(neighbors.size() == neighborsFinished))
        {
            CkError("ERROR in ChannelElement::receiveInflowsAndUpdateState: It is an error to call receiveInflowsAndUpdateState when not all NeighborProxies are finished receiving water.\n");
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
                case CHANNEL_SURFACE:
                case RESERVOIR_RECIPIENT:
                    if (0.0 > it->second.getNominalFlowRate())
                    {
                        crossSectionalArea += it->second.receiveWater(it->first, currentTime, timestepEndTime) / elementLength;
                    }
                    break;
                case RESERVOIR_RELEASE:
                case IRRIGATION_DIVERSION:
                    // This can't be an inflow so do nothing.
                    break;
                default:
                    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
                    {
                        CkAssert(false); // All other endpoints are invalid here.
                    }
                    break;
            }
        }
        
        // Convert cross sectional area back to water depth.
        calculateSurfaceWaterDepthFromCrossSectionalArea(crossSectionalArea);
        
        // If in drainDownMode cut off surface water to channel bank depth.
        if (Readonly::drainDownMode && surfaceWater > elementZBank - elementZBed)
        {
            surfaceWater         = elementZBank - elementZBed;
            surfaceWaterCreated -= (crossSectionalArea - ((baseWidth + sideSlope * surfaceWater) * surfaceWater)) * elementLength;
        }
    }
    
    return error;
}

void ChannelElement::calculateSurfaceWaterDepthFromCrossSectionalArea(double crossSectionalArea)
{
    double bOverTwoS; // Temporary value reused in computation.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
        CkAssert(0.0 <= crossSectionalArea);
    }
    
    if (0.0 < sideSlope)
    {
        // Trapezoidal or triangular channel.
        bOverTwoS = baseWidth / (2.0 * sideSlope);
        
        surfaceWater = sqrt(crossSectionalArea / sideSlope + bOverTwoS * bOverTwoS) - bOverTwoS;
        
        // For very small areas I wonder if surfaceWater might be able to come out negative due to roundoff error.  I wasn't able to convince myself that it couldn't possibly happen so I'm leaving this in.
        // I don't add anything to surfaceWaterCreated because I already know that crossSectionalArea wasn't negative so it's not really missing water.
        if (0.0 > surfaceWater)
        {
            if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            {
                CkAssert(epsilonEqual(0.0, surfaceWater));
            }
            
            surfaceWater = 0.0;
        }
    }
    else
    {
        // Rectangular channel.  By invariant baseWidth and sideSlope can't both be zero.
        surfaceWater = crossSectionalArea / baseWidth;
    }
}
