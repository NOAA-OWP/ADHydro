#ifndef __READONLY_H__
#define __READONLY_H__

#include <string>

// Readonly is a class for holding global variables that are used as Charm++ readonly variables.
// It also holds some static functions that have no better home.
class Readonly
{
public:
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    static bool checkInvariant();
    
    // Returns: The home processor of a numbered item such as an element or region.  Exit on error.
    //
    // Parameters:
    //
    // item                - The item number.
    // globalNumberOfItems - The total number of the type of item.
    // numPes              - The total number of processors.
    static size_t home(size_t item, size_t globalNumberOfItems, size_t numPes);
    
    // Calculate the range of numbered items that are homed on this processor.  Exit on error.
    //
    // Parameters:
    //
    // localItemStart      - Will be filled in with the first item number homed on this processor.
    // localNumberOfItems  - Will be filled in with the number of items homed on this processor.
    // globalNumberOfItems - The total number of the type of item.
    // numPes              - The total number of processors.
    // myPe                - The number of this processor.
    static void localStartAndNumber(size_t& localItemStart, size_t& localNumberOfItems, size_t globalNumberOfItems, size_t numPes, size_t myPe);
    
    // The simulation runs more efficiently if multiple expiration times expire simultaneously.
    // In order to increase the likelihood of that happening we force expiration times to expire at certain discrete times.
    // To choose the expiration time, first choose the number closest to but not over dtNew from the following divisors of sixty.
    // Not all divisors of sixty are present in the list because some pairs of divisors have a bad synodic period.
    //
    // 1, 2, 3, 5, 10, 15, 30, 60
    //
    // If dtNew is one minute or more then use the same divisors as minutes instead of seconds.
    // If dtNew is less than one second choose one second divided by a power of two.
    // The expiration time must be a multiple of this number.
    // Choose the expiration time as the latest multiple of this number less than or equal to currentTime + dtNew.
    //
    // Examples:
    //
    // currentTime = 0.0, dtNew = 3.1, newExpirationTime = 3.0
    // currentTime = 0.0, dtNew = 4.3, newExpirationTime = 3.0
    // currentTime = 2.0, dtNew = 3.1, newExpirationTime = 3.0
    // currentTime = 2.0, dtNew = 4.3, newExpirationTime = 6.0
    //
    // Returns: (s) simulation time for the new expiration time of a nominal flow rate.  Exit on error.
    //
    // Parameters:
    //
    // currentTime - (s) Current simulation time.
    // dtNew       - (s) Desired timestep duration.
    static double newExpirationTime(double currentTime, double dtNew);
    
    // Calculate latitude and longitude from X,Y coordinates of a sinusoidal projection.
    // This function uses the less accurate spherical world equations.
    // FIXME I don't know if we are going to use this or use GDAL for a more accurate value.
    //
    // Returns: true if there is an error, false otherwise.  Exit on error.
    //
    // Parameters:
    //
    // latitude        - (radians) Will be filled in with latitude.
    // longitude       - (radians) Will be filled in with longitude.
    // centralMeridian - (radians) The central meridian of the sinusoidal projection.
    // falseEasting    - (m) The false easting of the sinusoidal projection.
    // falseNorthing   - (m) the false northing of the sinusoidal projection.
    // x               - (m) X coordinate in the sinusoidal projection.
    // y               - (m) Y coordinate in the sinusoidal projection.
    static void getLatLongSinusoidal(double& latitude, double& longitude, double centralMeridian, double falseEasting, double falseNorthing, double x, double y);
    
    // Returns: (s) the simulation time of a given checkpoint.  Exit on Error
    //
    // Parameters:
    //
    // checkpointIndex - The index number of the checkpoint from one to getNumberOfCheckpoints.
    static double getCheckpointTime(size_t checkpointIndex);
    
    // Returns: the total number of checkpoints for the entire run.
    static size_t getNumberOfCheckpoints();
    
    // Global readonly variables.  For usage see comments in the example superfile.
    static std::string noahMPMpTableFilePath;   // For initializing Noah-MP.
    static std::string noahMPVegParmFilePath;   // For initializing Noah-MP.
    static std::string noahMPSoilParmFilePath;  // For initializing Noah-MP.
    static std::string noahMPGenParmFilePath;   // For initializing Noah-MP.
    static double      referenceDate;           // (days) Julian date when currentTime is zero.  The current date and time of the simulation is the Julian date equal to referenceDate + (currentTime / (24.0 * 60.0 * 60.0)).  Time zone is UTC.
    static double      simulationStartTime;     // (s) Time when the simulation starts specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    static double      simulationDuration;      // (s) Time duration that the simulation will run.  Must be positive.  The simulation ends when currentTime is simulationStartTime + simulationDuration.
    static double      checkpointPeriod;        // (s) Time duration between state checkpoints.  Must be positive.  Checkpoints occur at simulationStartTime + checkpointPeriod, simulationStartTime + 2 * checkpointPeriod, etc.
                                                // There is always a checkpoint at the end of the simulation even if it is not on a multiple of checkpointPeriod.
    static size_t      checkpointGroupSize;     // The number of state checkpoints that are accumulated and outputed at the same time.  Increasing this number can reduce time spent on I/O.
    static std::string checkpointDirectoryPath; // Directory in which to store checkpoint files.
    static bool        drainDownMode;           // If true, do not allow channels to have more water than bank-full.  Excess water is discarded.
    static bool        zeroExpirationTime;      // If true, set all nominal flow rates to expired at the beginning of the simulation.
    static bool        zeroCumulativeFlow;      // If true, set all cumulative flows to zero at the beginning of the simulation.
    static bool        zeroWaterCreated;        // If true, set all water created to zero at the beginning of the simulation.
    static size_t      verbosityLevel;          // Controls the amount of messages printed to the console.
    
    // Variables for number of items.
    // FIXME Once we implement mesh adaption, these variables won't be read-only.
    // I don't really have a good plan for how we are going to coordinate item homes once we start changing the number of elements.
    static size_t globalNumberOfMeshElements;
    static size_t localNumberOfMeshElements;
    static size_t localMeshElementStart;
    static size_t maximumNumberOfMeshNeighbors;
    static size_t globalNumberOfChannelElements;
    static size_t localNumberOfChannelElements;
    static size_t localChannelElementStart;
    static size_t maximumNumberOfChannelNeighbors;
    static size_t globalNumberOfRegions;
    static size_t localNumberOfRegions;
    static size_t localRegionStart;
};

#endif // __READONLY_H__
