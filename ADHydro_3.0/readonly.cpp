#include "readonly.h"
#include "all.h"

bool Readonly::checkInvariant()
{
    bool                error                       = false;               // Error flag.
    const static double originalReferenceDate       = referenceDate;       // For checking that readonly values are never changed.
    const static double originalSimulationStartTime = simulationStartTime; // For checking that readonly values are never changed.
    const static double originalSimulationDuration  = simulationDuration;  // For checking that readonly values are never changed.
    const static double originalCheckpointPeriod    = checkpointPeriod;    // For checking that readonly values are never changed.
    const static size_t originalCheckpointGroupSize = checkpointGroupSize; // For checking that readonly values are never changed.
    const static bool   originalDrainDownMode       = drainDownMode;       // For checking that readonly values are never changed.
    const static size_t originalVerbosityLevel      = verbosityLevel;      // For checking that readonly values are never changed.
    
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
    
    if (!(originalCheckpointGroupSize == checkpointGroupSize))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: checkpointGroupSize changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalDrainDownMode == drainDownMode))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: drainDownMode changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    if (!(originalVerbosityLevel == verbosityLevel))
    {
        ADHYDRO_ERROR("ERROR in Readonly::checkInvariant: verbosityLevel changed, which is not allowed for a readonly variable.\n");
        error = true;
    }
    
    return error;
}

// Global variables that are used as Charm++ readonly variables.
double Readonly::referenceDate;
double Readonly::simulationStartTime;
double Readonly::simulationDuration;
double Readonly::checkpointPeriod;
size_t Readonly::checkpointGroupSize;
bool   Readonly::drainDownMode;
size_t Readonly::verbosityLevel;
