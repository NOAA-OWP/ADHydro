#include "readonly.h"
#include "all.h"

bool Readonly::checkInvariant()
{
    bool                     error                           = false;                   // Error flag.
    const static std::string originalNoahMPMpTableFilePath   = noahMPMpTableFilePath;   // For checking that readonly values are never changed.
    const static std::string originalNoahMPVegParmFilePath   = noahMPVegParmFilePath;   // For checking that readonly values are never changed.
    const static std::string originalNoahMPSoilParmFilePath  = noahMPSoilParmFilePath;  // For checking that readonly values are never changed.
    const static std::string originalNoahMPGenParmFilePath   = noahMPGenParmFilePath;   // For checking that readonly values are never changed.
    const static std::string originalForcingFilePath         = forcingFilePath;         // For checking that readonly values are never changed.
    const static double      originalReferenceDate           = referenceDate;           // For checking that readonly values are never changed.
    const static double      originalSimulationStartTime     = simulationStartTime;     // For checking that readonly values are never changed.
    const static double      originalSimulationDuration      = simulationDuration;      // For checking that readonly values are never changed.
    const static double      originalCheckpointPeriod        = checkpointPeriod;        // For checking that readonly values are never changed.
    const static size_t      originalCheckpointGroupSize     = checkpointGroupSize;     // For checking that readonly values are never changed.
    const static std::string originalCheckpointDirectoryPath = checkpointDirectoryPath; // For checking that readonly values are never changed.
    const static bool        originalDrainDownMode           = drainDownMode;           // For checking that readonly values are never changed.
    const static bool        originalZeroExpirationTime      = zeroExpirationTime;      // For checking that readonly values are never changed.
    const static bool        originalZeroCumulativeFlow      = zeroCumulativeFlow;      // For checking that readonly values are never changed.
    const static bool        originalZeroWaterCreated        = zeroWaterCreated;        // For checking that readonly values are never changed.
    const static size_t      originalVerbosityLevel          = verbosityLevel;          // For checking that readonly values are never changed.
    
    if (!(originalNoahMPMpTableFilePath == noahMPMpTableFilePath))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: noahMPMpTableFilePath changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalNoahMPVegParmFilePath == noahMPVegParmFilePath))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: noahMPVegParmFilePath changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalNoahMPSoilParmFilePath == noahMPSoilParmFilePath))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: noahMPSoilParmFilePath changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalNoahMPGenParmFilePath == noahMPGenParmFilePath))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: noahMPGenParmFilePath changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
     
    if (!(originalForcingFilePath == forcingFilePath))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: forcingFilePath changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
   
    if (!(1721425.5 <= referenceDate))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: referenceDate must be on or after 1 CE (1721425.5).\n");
        error = true;
    }
    
    if (!(originalReferenceDate == referenceDate))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: referenceDate changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(1721425.5 <= referenceDate + (simulationStartTime / ONE_DAY_IN_SECONDS)))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: simulationStartTime must be on or after 1 CE (1721425.5).\n");
        error = true;
    }
    
    if (!(originalSimulationStartTime == simulationStartTime))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: simulationStartTime changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(0.0 < simulationDuration))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: simulationDuration must be greater than zero.\n");
        error = true;
    }
    
    if (!(originalSimulationDuration == simulationDuration))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: simulationDuration changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(0.0 < checkpointPeriod))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: checkpointPeriod must be greater than zero.\n");
        error = true;
    }
    
    if (!(originalCheckpointPeriod == checkpointPeriod))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: checkpointPeriod changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(0 < checkpointGroupSize))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: checkpointGroupSize must be greater than zero.\n");
        error = true;
    }
    
    if (!(originalCheckpointGroupSize == checkpointGroupSize))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: checkpointGroupSize changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalCheckpointDirectoryPath == checkpointDirectoryPath))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: checkpointDirectoryPath changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalDrainDownMode == drainDownMode))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: drainDownMode changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalZeroExpirationTime == zeroExpirationTime))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: zeroExpirationTime changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalZeroCumulativeFlow == zeroCumulativeFlow))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: zeroCumulativeFlow changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalZeroWaterCreated == zeroWaterCreated))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: zeroWaterCreated changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalVerbosityLevel == verbosityLevel))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: verbosityLevel changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(localMeshElementStart + localNumberOfMeshElements <= globalNumberOfMeshElements))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: localMeshElementStart plus localNumberOfMeshElements must be less than or equal to globalNumberOfMeshElements.\n");
        error = true;
    }
    
    if (!(localChannelElementStart + localNumberOfChannelElements <= globalNumberOfChannelElements))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: localChannelElementStart plus localNumberOfChannelElements must be less than or equal to globalNumberOfChannelElements.\n");
        error = true;
    }
    
    if (!(localRegionStart + localNumberOfRegions <= globalNumberOfRegions))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: localRegionStart plus localNumberOfRegions must be less than or equal to globalNumberOfRegions.\n");
        error = true;
    }
    
    return error;
}

size_t Readonly::home(size_t item, size_t globalNumberOfItems, size_t numPes)
{
    size_t numberOfFatOwners   = globalNumberOfItems % numPes;         // Number of processors that own one extra item.
    size_t itemsPerFatOwner    = globalNumberOfItems / numPes + 1;     // Number of items in each processor that owns one extra item.
    size_t itemsInAllFatOwners = numberOfFatOwners * itemsPerFatOwner; // Total number of items in all processors that own one extra item.
    size_t itemsPerThinOwner   = globalNumberOfItems / numPes;         // Number of items in each processor that does not own one extra item.
    size_t itemHome;                                                   // The number of the processor that owns item.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(item < globalNumberOfItems))
        {
            ADHYDRO_ERROR("ERROR in Readonly::home: item must be less than globalNumberOfItems.\n");
            ADHYDRO_EXIT(-1);
        }
        
        if (!(0 < numPes))
        {
            ADHYDRO_ERROR("ERROR in Readonly::home: numPes must be greater than zero.\n");
            ADHYDRO_EXIT(-1);
        }
    }
    
    if (item < itemsInAllFatOwners)
    {
        // Item is owned by a fat owner.
        itemHome = item / itemsPerFatOwner;
    }
    else
    {
        // Item is owned by a thin owner.
        itemHome = (item - itemsInAllFatOwners) / itemsPerThinOwner + numberOfFatOwners;
    }
    
    return itemHome;
}

void Readonly::localStartAndNumber(size_t& localItemStart, size_t& localNumberOfItems, size_t globalNumberOfItems, size_t numPes, size_t myPe)
{
    size_t numberOfFatOwners   = globalNumberOfItems % numPes;         // Number of processors that own one extra item.
    size_t itemsPerFatOwner    = globalNumberOfItems / numPes + 1;     // Number of items in each processor that owns one extra item.
    size_t itemsInAllFatOwners = numberOfFatOwners * itemsPerFatOwner; // Total number of items in all processors that own one extra item.
    size_t itemsPerThinOwner   = globalNumberOfItems / numPes;         // Number of items in each processor that does not own one extra item.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(myPe < numPes))
        {
            ADHYDRO_ERROR("ERROR in Readonly::localStartAndNumber: myPe must be less than numPes.\n");
            ADHYDRO_EXIT(-1);
        }
    }
    
    if (myPe < numberOfFatOwners)
    {
        // I am a fat owner.
        localItemStart     = myPe * itemsPerFatOwner;
        localNumberOfItems = itemsPerFatOwner;
    }
    else
    {
        // I am a thin owner.
        localItemStart     = (myPe - numberOfFatOwners) * itemsPerThinOwner + itemsInAllFatOwners;
        localNumberOfItems = itemsPerThinOwner;
    }
}

double Readonly::newExpirationTime(double currentTime, double dtNew)
{
    int          ii;                             // Loop counter.
    const int    numberOfDts               = 15; // Size of list in allowableDts.
    const double allowableDts[numberOfDts] = {1.0, 2.0, 3.0, 5.0, 10.0, 15.0, 30.0, 1.0 * 60.0, 2.0 * 60.0, 3.0 * 60.0, 5.0 * 60.0, 10.0 * 60.0, 15.0 * 60.0, 30.0 * 60.0, 60.0 * 60.0}; // (s).
    double       selectedDt;                     // (s) Selected value from allowableDts.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(0.0 < dtNew && INFINITY > dtNew))
        {
            ADHYDRO_ERROR("ERROR in Readonly::newExpirationTime: dtNew must be greater than zero and less than INFINITY.\n");
            ADHYDRO_EXIT(-1);
        }
    }
    
    if (dtNew < allowableDts[0])
    {
        selectedDt = allowableDts[0] * 0.5;
        
        while (dtNew < selectedDt)
        {
            selectedDt *= 0.5;
        }
    }
    else
    {
        ii = 0;
        
        while (ii + 1 < numberOfDts && dtNew >= allowableDts[ii + 1])
        {
            ++ii;
        }
        
        selectedDt = allowableDts[ii];
    }
    
    ii = floor(currentTime / selectedDt) + 1;
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    {
        ADHYDRO_ASSERT(ii * selectedDt <= currentTime + dtNew);
    }
    
    while ((ii + 1) * selectedDt <= currentTime + dtNew)
    {
        ++ii;
    }
    
    return ii * selectedDt;
}

void Readonly::getLatLongSinusoidal(double& latitude, double& longitude, double centralMeridian, double falseEasting, double falseNorthing, double x, double y)
{
    latitude  = (y - falseNorthing) / POLAR_RADIUS_OF_EARTH;
    longitude = centralMeridian + (x - falseEasting) / (POLAR_RADIUS_OF_EARTH * cos(latitude));
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(-M_PI * 2.0 <= centralMeridian && M_PI * 2.0 >= centralMeridian))
        {
            ADHYDRO_ERROR("ERROR in Readonly::getLatLongSinusoidal: centralMeridian must be greater than or equal to negative two PI and less than or equal to two PI.\n");
            ADHYDRO_EXIT(-1);
        }
        
        if (!(-M_PI / 2.0 <= latitude && M_PI / 2.0 >= latitude))
        {
            ADHYDRO_ERROR("ERROR in Readonly::getLatLongSinusoidal: x and y must produce a latitude greater than or equal to negative PI over two and less than or equal to PI over two.\n");
            ADHYDRO_EXIT(-1);
        }
        
        if (!(-M_PI * 2.0 <= longitude && M_PI * 2.0 >= longitude))
        {
            ADHYDRO_ERROR("ERROR in Readonly::getLatLongSinusoidal: x and y must produce a longitude greater than or equal to negative two PI and less than or equal to two PI.\n");
            ADHYDRO_EXIT(-1);
        }
    }
}

double Readonly::getCheckpointTime(size_t checkpointIndex)
{
    double checkpointTime; // Return value.
    size_t numberOfCheckpoints = getNumberOfCheckpoints();
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(checkpointIndex <= numberOfCheckpoints))
        {
            ADHYDRO_ERROR("ERROR in Readonly::getCheckpointTime: checkpointIndex must be less than or equal to getNumberOfCheckpoints.\n");
            ADHYDRO_EXIT(-1);
        }
    }
    
    if (checkpointIndex < numberOfCheckpoints)
    {
        checkpointTime = simulationStartTime + checkpointPeriod * checkpointIndex;
    }
    else
    {
        // For the last checkpoint use simulationDuration rather than a multiple of checkpointPeriod.
        // simulationDuration might not be an exact multiple of checkpointPeriod, and even if it is, this avoids roundoff error.
        checkpointTime = simulationStartTime + simulationDuration;
    }
    
    return checkpointTime;
}

size_t Readonly::getNumberOfCheckpoints()
{
    size_t numberOfCheckpoints = std::ceil(simulationDuration / checkpointPeriod); // Return value.  Total number of checkpoints for the entire run.
    
    // If checkpointPeriod is infinity then numberOfCheckpoints will be zero.  In that case, we want to checkpoint once at the end of the simulation.
    if (0 == numberOfCheckpoints)
    {
        numberOfCheckpoints = 1;
    }
    
    return numberOfCheckpoints;
}

// Global readonly variables.
std::string Readonly::noahMPMpTableFilePath;
std::string Readonly::noahMPVegParmFilePath;
std::string Readonly::noahMPSoilParmFilePath;
std::string Readonly::noahMPGenParmFilePath;
std::string Readonly::forcingFilePath;
double      Readonly::referenceDate;
double      Readonly::simulationStartTime;
double      Readonly::simulationDuration;
double      Readonly::checkpointPeriod;
size_t      Readonly::checkpointGroupSize;
std::string Readonly::checkpointDirectoryPath;
bool        Readonly::drainDownMode;
bool        Readonly::zeroExpirationTime;
bool        Readonly::zeroCumulativeFlow;
bool        Readonly::zeroWaterCreated;
size_t      Readonly::verbosityLevel;

// Variables for number of items.
size_t Readonly::globalNumberOfMeshElements;
size_t Readonly::localNumberOfMeshElements;
size_t Readonly::localMeshElementStart;
size_t Readonly::maximumNumberOfMeshNeighbors;
size_t Readonly::globalNumberOfChannelElements;
size_t Readonly::localNumberOfChannelElements;
size_t Readonly::localChannelElementStart;
size_t Readonly::maximumNumberOfChannelNeighbors;
size_t Readonly::globalNumberOfRegions;
size_t Readonly::localNumberOfRegions;
size_t Readonly::localRegionStart;
