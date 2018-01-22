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
    inline ForcingManager() : jultime(NULL), jultimeSize(0), jultimeIndex(0), nextForcingTime(0.0), t2(NULL), psfc(NULL), u(NULL), v(NULL), qVapor(NULL), qCloud(NULL), swDown(NULL), gLw(NULL), tPrec(NULL),
                              tslb(NULL), pblh(NULL), t2_c(NULL), psfc_c(NULL), u_c(NULL), v_c(NULL), qVapor_c(NULL), qCloud_c(NULL), swDown_c(NULL), gLw_c(NULL), tPrec_c(NULL), tslb_c(NULL), pblh_c(NULL)
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
        delete[] t2;
        delete[] psfc;
        delete[] u;
        delete[] v;
        delete[] qVapor;
        delete[] qCloud;
        delete[] swDown;
        delete[] gLw;
        delete[] tPrec;
        delete[] tslb;
        delete[] pblh;
        delete[] t2_c;
        delete[] psfc_c;
        delete[] u_c;
        delete[] v_c;
        delete[] qVapor_c;
        delete[] qCloud_c;
        delete[] swDown_c;
        delete[] gLw_c;
        delete[] tPrec_c;
        delete[] tslb_c;
        delete[] pblh_c;
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
    
    // Arrays for reading forcing data from file.
    float* t2;       // Used to read air temperature at 2m height forcing for mesh elements.
    float* psfc;     // Used to read surface pressure forcing for mesh elements.
    float* u;        // Used to read wind speed U component forcing for mesh elements.
    float* v;        // Used to read wind speed V component forcing for mesh elements.
    float* qVapor;   // Used to read water vapor mixing ratio forcing for mesh elements.
    float* qCloud;   // Used to read cloud water mixing ratio forcing for mesh elements.
    float* swDown;   // Used to read downward shortwave flux forcing for mesh elements.
    float* gLw;      // Used to read downward longwave flux forcing for mesh elements.
    float* tPrec;    // Used to read total precipitation forcing for mesh elements.
    float* tslb;     // Used to read soil temperature at the deepest layer forcing for mesh elements.
    float* pblh;     // Used to read planetary boundary layer height forcing for mesh elements.
    float* t2_c;     // Used to read air temperature at 2m height forcing for channel elements.
    float* psfc_c;   // Used to read surface pressure forcing for channel elements.
    float* u_c;      // Used to read wind speed U component forcing for channel elements.
    float* v_c;      // Used to read wind speed V component forcing for channel elements.
    float* qVapor_c; // Used to read water vapor mixing ratio forcing for channel elements.
    float* qCloud_c; // Used to read cloud water mixing ratio forcing for channel elements.
    float* swDown_c; // Used to read downward shortwave flux forcing for channel elements.
    float* gLw_c;    // Used to read downward longwave flux forcing for channel elements.
    float* tPrec_c;  // Used to read total precipitation forcing for channel elements.
    float* tslb_c;   // Used to read soil temperature at the deepest layer forcing for channel elements.
    float* pblh_c;   // Used to read planetary boundary layer height forcing for channel elements.
};

#endif // __FORCING_MANAGER_H__
