#include "forcing_manager.h"
#include "adhydro.h"
#include "initialization_manager.h"
#include "forcing_manager.def.h"
#include <netcdf.h>
#include <netcdf_par.h>

bool ForcingManager::checkInvariant() const
{
    bool error = false; // Error flag.
    
    if (!(NULL != jultime))
    {
        CkError("ERROR in ForcingManager::checkInvariant: jultime must not be NULL.\n");
        error = true;
    }
    
    if (!(jultimeIndex <= jultimeSize))
    {
        CkError("ERROR in ForcingManager::checkInvariant: jultimeIndex must be less than or equal to jultimeSize.\n");
        error = true;
    }
    
    if (jultimeIndex == jultimeSize && !(INFINITY == nextForcingTime))
    {
        CkError("ERROR in ForcingManager::checkInvariant: if jultimeIndex is equal to jultimeSize then nextForcingTime must be INFINITY.\n");
        error = true;
    }
    
    return error;
}

bool ForcingManager::readForcingTimes()
{
    bool   error    = false; // Error flag.
    int    ncErrorCode;      // Return value of NetCDF functions.
    int    fileID;           // ID of NetCDF file.
    bool   fileOpen = false; // Whether fileID refers to an open file.
    int    variableID;       // ID of variable in NetCDF file.
    int    dimensionID;      // ID of dimension in NetCDF file.
    bool   done     = false; // Flag to signal when we have found the right forcing index.
    size_t newIndex;         // Possible new value for jultimeIndex.
    double newForcingTime;   // (s) Possible new value for nextForcingTime.
    
    // Open the forcing file.
    ncErrorCode = nc_open_par(Readonly::forcingFilePath.c_str(), NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
    
    if (NC_NOERR == ncErrorCode)
    {
        fileOpen = true;
    }
    else if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    {
        CkError("ERROR in ForcingManager::readForcingTimes: unable to open NetCDF forcing file %s.  NetCDF error message: %s.\n", Readonly::forcingFilePath.c_str(), nc_strerror(ncErrorCode));
        error = true;
    }
    
    // Get the dimension ID of the time dimension.
    if (!error)
    {
        ncErrorCode = nc_inq_dimid(fileID, "Time", &dimensionID);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NC_NOERR == ncErrorCode))
            {
                CkError("ERROR in ForcingManager::readForcingTimes: unable to get dimension Time in NetCDF forcing file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    // Get the dimension length of the time dimension.
    if (!error)
    {
        ncErrorCode = nc_inq_dimlen(fileID, dimensionID, &jultimeSize);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NC_NOERR == ncErrorCode))
            {
                CkError("ERROR in ForcingManager::readForcingTimes: unable to get length of dimension Time in NetCDF forcing file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                error = true;
            }
            else if (!(0 < jultimeSize))
            {
                CkError("ERROR in ForcingManager::readForcingTimes: No forcing data in NetCDF forcing file.\n");
                error = true;
            }
        }
    }
    
    // Get the variable ID of the julian dates.
    if (!error)
    {
        ncErrorCode = nc_inq_varid(fileID, "JULTIME", &variableID);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NC_NOERR == ncErrorCode))
            {
                CkError("ERROR in ForcingManager::readForcingTimes: unable to get variable JULTIME in NetCDF forcing file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    // Get the julian date data.
    if (!error)
    {
        jultime = new double[jultimeSize];
        
        ncErrorCode = nc_get_var(fileID, variableID, jultime);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NC_NOERR == ncErrorCode))
            {
                CkError("ERROR in ForcingManager::readForcingTimes: unable to read variable JULTIME in NetCDF forcing file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    // Set jultimeIndex and nextForcingTime to the forcing data to use at the start of the simulation.
    // We do a linear scan from the beginning of the array to find the last entry that is no later than Readonly::simulationStartTime.
    // It's possible that the first entry in the array is after Readonly::simulationStartTime.  In this case, we print a warning and use that entry.
    // If we find entries that are not monotonically increasing in time we print a warning and those entries are ignored.
    if (!error)
    {
        jultimeIndex    = 0;
        nextForcingTime = getForcingTime(jultimeIndex);
        
        while (!done)
        {
            // To protect against entries that are not monotonically increasing, find the next index strictly later than jultimeIndex when converted to a simulation time including roundoff to the nearest second.
            newIndex = skipEntriesNotMonotonicallyIncreasingInTime(&newForcingTime);
            
            if (!(newForcingTime <= Readonly::simulationStartTime) || newIndex == jultimeSize)
            {
                // If we found one later than Readonly::simulationStartTime or hit the end of the array then we are done, and we will use the entry previously stored in jultimeIndex.
                done = true;
            }
            else
            {
                // Otherwise, the new one is later than jultimeIndex and no later than Readonly::simulationStartTime so save it and keep scanning.
                jultimeIndex    = newIndex;
                nextForcingTime = newForcingTime;
            }
        }
        
        // Print a warning if we didn't find an entry no later than Readonly::simulationStartTime.
        // In that case, the first entry will still be used.
        if (2 <= Readonly::verbosityLevel && !(nextForcingTime <= Readonly::simulationStartTime))
        {
            CkError("WARNING in ForcingManager::readForcingTimes: first entry in forcing file is after simulation start time.\n");
        }
    }
    
    // Close the forcing file.
    if (fileOpen)
    {
        ncErrorCode = nc_close(fileID);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NC_NOERR == ncErrorCode))
            {
                CkError("ERROR in ForcingManager::readForcingTimes: could not close NetCDF forcing file %s.  NetCDF error message: %s.\n", Readonly::forcingFilePath.c_str(), nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    return error;
}

bool ForcingManager::readAndSendForcing()
{
    bool   error    = false; // Error flag.
    int    ncErrorCode;      // Return value of NetCDF functions.
    int    fileID;           // ID of NetCDF file.
    bool   fileOpen = false; // Whether fileID refers to an open file.
    long   year;             // For printing date and time of forcing data.
    long   month;            // For printing date and time of forcing data.
    long   day;              // For printing date and time of forcing data.
    long   hour;             // For printing date and time of forcing data.
    long   minute;           // For printing date and time of forcing data.
    double second;           // For printing date and time of forcing data.
    size_t newIndex;         // Possible new value for jultimeIndex.
    double newForcingTime;   // (s) Possible new value for nextForcingTime.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(jultimeIndex < jultimeSize))
        {
            CkError("ERROR in ForcingManager::readAndSendForcing: someone just asked for forcing data beyond the end of the array, which is an error.\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Print out that we are sending the forcing.
        if (0 == CkMyPe() && 1 <= Readonly::verbosityLevel)
        {
            julianToGregorian(jultime[jultimeIndex], &year, &month, &day, &hour, &minute, &second);
            CkPrintf("Reading forcing data for %02ld/%02ld/%04ld %02ld:%02ld:%02.0lf\n", month, day, year, hour, minute, second);
        }
        
        // Open the forcing file.
        ncErrorCode = nc_open_par(Readonly::forcingFilePath.c_str(), NC_NETCDF4 | NC_MPIIO, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
        
        if (NC_NOERR == ncErrorCode)
        {
            fileOpen = true;
        }
        else if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            CkError("ERROR in ForcingManager::readAndSendForcing: unable to open NetCDF forcing file %s.  NetCDF error message: %s.\n", Readonly::forcingFilePath.c_str(), nc_strerror(ncErrorCode));
            error = true;
        }
    }
    
    if (!error)
    {
        // Load the forcing.
        // FIXME implement
        
        // Find the next forcing data to use.  The time of this forcing data will be sent to regions to let them know when to stop and expect more forcing.
        // To protect against entries that are not monotonically increasing, find the next index strictly later than jultimeIndex when converted to a simulation time including roundoff to the nearest second.
        newIndex = skipEntriesNotMonotonicallyIncreasingInTime(&newForcingTime);
        
        // Print a warning if we are sending the last forcing time in the file.
        if (2 <= Readonly::verbosityLevel && newIndex == jultimeSize)
        {
            CkError("WARNING in ForcingManager::readAndSendForcing: reading the last entry in the forcing file.  No more forcing will be loaded in the future for this run.\n");
        }
        
        // Send the forcing.
        // FIXME implement
        std::map<size_t, std::pair<std::map<size_t, EvapoTranspirationForcingStruct>, std::map<size_t, EvapoTranspirationForcingStruct> > >           forcing;
        std::map<size_t, std::pair<std::map<size_t, EvapoTranspirationForcingStruct>, std::map<size_t, EvapoTranspirationForcingStruct> > >::iterator it;
        InitializationManager*                                                                                                                        initializationManager = ADHydro::initializationManagerProxy.ckLocalBranch();
        
        for (size_t ii = 0; ii < Readonly::localNumberOfMeshElements; ++ii)
        {
            EvapoTranspirationForcingStruct& forcingStruct = forcing[initializationManager->meshRegion[ii]].first[ii + Readonly::localMeshElementStart];
            
            forcingStruct.dz8w   = 20.0f;
            forcingStruct.sfcTmp = 300.0f;
            forcingStruct.sfcPrs = 101300.0f;
            forcingStruct.psfc   = 101180.0f;
            forcingStruct.uu     = 0.0f;
            forcingStruct.vv     = 0.0f;
            forcingStruct.q2     = 0.01f;
            forcingStruct.qc     = 0.01f;
            forcingStruct.solDn  = 500.0f;
            forcingStruct.lwDn   = 300.0f;
            forcingStruct.prcp   = (0 == ii + Readonly::localMeshElementStart) ? 1.0E-3f : 0.0f;
            forcingStruct.tBot   = 300.0f;
            forcingStruct.pblh   = 1000.0f;
        }
        
        for (size_t ii = 0; ii < Readonly::localNumberOfChannelElements; ++ii)
        {
            EvapoTranspirationForcingStruct& forcingStruct = forcing[initializationManager->meshRegion[ii]].first[ii + Readonly::localMeshElementStart];
            
            forcingStruct.dz8w   = 20.0f;
            forcingStruct.sfcTmp = 300.0f;
            forcingStruct.sfcPrs = 101300.0f;
            forcingStruct.psfc   = 101180.0f;
            forcingStruct.uu     = 0.0f;
            forcingStruct.vv     = 0.0f;
            forcingStruct.q2     = 0.01f;
            forcingStruct.qc     = 0.01f;
            forcingStruct.solDn  = 500.0f;
            forcingStruct.lwDn   = 300.0f;
            forcingStruct.prcp   = (0 == ii + Readonly::localMeshElementStart) ? 1.0E-3f : 0.0f;
            forcingStruct.tBot   = 300.0f;
            forcingStruct.pblh   = 1000.0f;
        }
        
        for (it = forcing.begin(); it != forcing.end(); ++it)
        {
            ADHydro::regionProxy[it->first].sendForcing(nextForcingTime, newForcingTime, it->second.first, it->second.second);
        }
        
        // Increment jultimeIndex and update nextForcingTime.
        jultimeIndex    = newIndex;
        nextForcingTime = newForcingTime;
    }
    
    // Close the forcing file.
    if (fileOpen)
    {
        ncErrorCode = nc_close(fileID);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NC_NOERR == ncErrorCode))
            {
                CkError("ERROR in ForcingManager::readAndSendForcing: could not close NetCDF forcing file %s.  NetCDF error message: %s.\n", Readonly::forcingFilePath.c_str(), nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    return error;
}
