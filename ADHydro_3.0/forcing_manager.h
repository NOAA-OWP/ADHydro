#ifndef __FORCING_MANAGER_H__
#define __FORCING_MANAGER_H__

#include "readonly.h"
#include "all.h"
#include "forcing_manager.decl.h"

// ForcingManager is a Charm++ group that reads forcing data from file and sends it to the elements.
class ForcingManager : public CBase_ForcingManager
{
    ForcingManager_SDAG_CODE
    
public:
    
    // Constructor.
    inline ForcingManager() : jultime(NULL), jultimeSize(0), jultimeIndex(0), nextForcingTime(0.0)
    {
        if (readForcingTimes())
        {
            CkExit();
        }
        
        // We have to call readForcingTimes to initialize member variables before we will pass the invariant.
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (checkInvariant())
            {
                CkExit();
            }
        }
        
        // Tell myself I'm ready to send the first forcing data.  After the first one, Regions will tell me when they are ready for the next one.
        thisProxy[CkMyPe()].readyForForcing(nextForcingTime);
        thisProxy[CkMyPe()].runUntilSimulationEnd();
    }
    
    // Destructor.
    inline ~ForcingManager()
    {
        delete[] jultime;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
private:
    
    // Initialize member variables to the correct forcing instance for the start of the simulation.
    //
    // Returns: true if there is an error, false otherwise.
    bool readForcingTimes();
    
    // Read from the forcing file the instance at jultimeIndex.  Then update jultimeIndex and nextForcingTime to the next instance in the forcing file.
    // When advancing to the next instance protect against jultime being not monotonically increasing and against running off the end of the array.
    // Then send the forcing data and nextForcingTime out to the appropriate elements.
    //
    // Returns: true if there is an error, false otherwise.
    bool readAndSendForcing();
    
    // Returns: (s) the simulation time of jultime[index] or INFINITY if index is off the end of the array.
    //          We round this value to the nearest second because we have had problems with roundoff error setting forcing times to 59.999999... seconds.
    //
    // Parameters:
    //
    // index - The entry of jultime to get as a simulation time.
    inline double getForcingTime(size_t index)
    {
        double newTime; // Return value.
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
        {
            CkAssert(index <= jultimeSize);
        }
        
        if (index >= jultimeSize)
        {
            newTime = INFINITY;
        }
        else
        {
            newTime = round((jultime[index] - Readonly::referenceDate) * ONE_DAY_IN_SECONDS);
        }
        
        return newTime;
    }
    
    // There are times when we want to increment to the next forcing data entry.
    // In all of those cases, we want to skip over any entries that are not monotonically increasing in time when rounded to the nearest second.
    // To avoid duplicate code this is pulled out into a separate function.
    //
    // Returns: the index of the "next" forcing data entry after jultimeIndex while skipping over entries that are not monotonically
    //          increasing in time when rounded to the nearest second.  Can return jultimeSize if there are no more entries in the array.
    //
    // Parameters:
    //
    // newForcingTime - (s) Scalar passed by reference will be filled in with getForcingTime() of the returned index.
    //                  Can be passed in as NULL, in which case it is ignored.  The caller can call getForcingTime separately to get
    //                  this value, but we have to calculate it to find newIndex so no reason to make the caller calculate it again.
    inline size_t skipEntriesNotMonotonicallyIncreasingInTime(double* newForcingTime = NULL)
    {
        size_t  newIndex    = jultimeIndex;                                                   // Return value.
        double  tempForcingTime;                                                              // (s) temporary variable to use if newForcingTime is NULL.
        double& forcingTime = ((NULL != newForcingTime) ? *newForcingTime : tempForcingTime); // (s) reference to fill in value into newForcingTime or use a temporary variable if newForcingTime is NULL.
        
        while (!((forcingTime = getForcingTime(++newIndex)) > nextForcingTime) && newIndex < jultimeSize)
        {
            if (2 <= Readonly::verbosityLevel)
            {
                CkError("WARNING in ForcingManager::skipEntriesNotMonotonicallyIncreasingInTime: forcing file entry %lu is not monotonically increasing in time when rounded to the nearest second.\n", newIndex);
            }
        }
        
        return newIndex;
    }
    
    // Simulation time of forcing instances.
    double*      jultime;         // Array of Julian dates of all instances in forcing file.
    size_t       jultimeSize;     // The size of the allocated array pointed to by jultime.
    size_t       jultimeIndex;    // The next instance to use in the forcing file.
    double       nextForcingTime; // (s) The simulation time of the instance at jultimeIndex or INFINITY if jultimeIndex is off the end of the array.
    const double simulationEndTime = Readonly::simulationStartTime + Readonly::simulationDuration;
                                  // This is partly for efficiency so we don't do the addition over and over and partly because Charm++ is having trouble parsing Readonly:: in the .ci file.
};

#endif // __FORCING_MANAGER_H__
