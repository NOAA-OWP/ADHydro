#ifndef __READONLY_H__
#define __READONLY_H__

#include <cstddef>

// This is a do-nothing class for holding global variables that are used as Charm++ readonly variables.
class Readonly
{
public:
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    static bool   checkInvariant();
    
    static double referenceDate;       // (days) Julian date when currentTime is zero.  The current date and time of the simulation is the Julian date equal to referenceDate + (currentTime / (24.0 * 60.0 * 60.0)). Time zone is UTC.
    static double simulationStartTime; // (s) Time when the simulation starts specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    static double simulationDuration;  // (s) Time duration that the simulation will run.  Must be positive.  The simulation ends when currentTime is simulationStartTime + simulationDuration.
    static double checkpointPeriod;    // (s) Time duration between state checkpoints.  Must be positive.  Checkpoints occur at simulationStartTime + checkpointPeriod, simulationStartTime + 2 * checkpointPeriod, etc.
                                       // There is always a checkpoint at the end of the simulation even if it is not on a multiple of checkpointPeriod.
    static size_t checkpointGroupSize; // The number of state checkpoints that are accumulated and outputed at the same time.  Zero is treated as one.  Increasing this number can reduce time spent on I/O.
    static bool   drainDownMode;       // If true, do not allow channels to have more water than bank-full.  Excess water is discarded.
    static size_t verbosityLevel;      // Controls the amount of messages printed to the console.
};

#endif // __READONLY_H__
