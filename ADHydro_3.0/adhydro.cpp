#include "adhydro.h"
#include "readonly.h"
#include "adhydro.def.h"
#include "INIReader.h"

ADHydro::ADHydro(CkArgMsg* msg)
{
    bool        error         = false;                                   // Error flag.
    const char* superfileName = (1 < msg->argc) ? (msg->argv[1]) : (""); // The first command line argument protected against non-existance.
    INIReader   superfile(superfileName);                                // Superfile reader object.
    std::string noahMPDirectoryPath;                                     // Directory path to use with default filenames if file paths not specified.
    std::string adhydroInputDirectoryPath;                               // Directory path to use with default filenames if file paths not specified.
    long        referenceDateYear;                                       // For converting Gregorian date to Julian date.
    long        referenceDateMonth;                                      // For converting Gregorian date to Julian date.
    long        referenceDateDay;                                        // For converting Gregorian date to Julian date.
    long        referenceDateHour;                                       // For converting Gregorian date to Julian date.
    long        referenceDateMinute;                                     // For converting Gregorian date to Julian date.
    double      referenceDateSecond;                                     // For converting Gregorian date to Julian date.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    {
        // If the superfile cannot be read print usage message and exit.
        if (!(0 == superfile.ParseError()))
        {
            if (0 > superfile.ParseError())
            {
                CkError("ERROR in ADHydro::ADHydro: cannot open superfile %s.\n", superfileName);
            }
            else
            {
                CkError("ERROR in ADHydro::ADHydro: parse error on line number %d of superfile %s.\n", superfile.ParseError(), superfileName);
            }
            
            CkError("\nUsage:\n\nadhydro <superfile>\n\nE.g.:\n\nadhydro ../input/example_superfile.ini\n\n");
            error = true;
        }
    }
    
    if (!error)
    {
        // Get readonly variables from the superfile.
        noahMPDirectoryPath              = superfile.Get(    "", "noahMPDirectoryPath", ".");
        Readonly::noahMPMpTableFilePath  = superfile.Get(    "", "noahMPMpTableFilePath",  noahMPDirectoryPath + "/MPTABLE.TBL");
        Readonly::noahMPVegParmFilePath  = superfile.Get(    "", "noahMPVegParmFilePath",  noahMPDirectoryPath + "/VEGPARM.TBL");
        Readonly::noahMPSoilParmFilePath = superfile.Get(    "", "noahMPSoilParmFilePath", noahMPDirectoryPath + "/SOILPARM.TBL");
        Readonly::noahMPGenParmFilePath  = superfile.Get(    "", "noahMPGenParmFilePath",  noahMPDirectoryPath + "/GENPARM.TBL");
        adhydroInputDirectoryPath        = superfile.Get(    "", "adhydroInputDirectoryPath", ".");
        Readonly::forcingFilePath        = superfile.Get(    "", "adhydroInputForcingFilePath", adhydroInputDirectoryPath + "/forcing.nc");
        Readonly::referenceDate          = superfile.GetReal("", "referenceDateJulian", NAN);
        
        // If there is no referenceDateJulian read a Gregorian date and convert to Julian date.
        if (isnan(Readonly::referenceDate))
        {
            referenceDateYear   = superfile.GetInteger("", "referenceDateYear",   -1);
            referenceDateMonth  = superfile.GetInteger("", "referenceDateMonth",  -1);
            referenceDateDay    = superfile.GetInteger("", "referenceDateDay",    -1);
            referenceDateHour   = superfile.GetInteger("", "referenceDateHour",   -1);
            referenceDateMinute = superfile.GetInteger("", "referenceDateMinute", -1);
            referenceDateSecond = superfile.GetReal(   "", "referenceDateSecond", -1.0);
            
            if (1 <= referenceDateYear && 1 <= referenceDateMonth && 12 >= referenceDateMonth && 1 <= referenceDateDay && 31 >= referenceDateDay && 0 <= referenceDateHour && 23 >= referenceDateHour &&
                0 <= referenceDateMinute && 59 >= referenceDateMinute && 0.0 <= referenceDateSecond && 60.0 > referenceDateSecond)
            {
                Readonly::referenceDate = gregorianToJulian(referenceDateYear, referenceDateMonth, referenceDateDay, referenceDateHour, referenceDateMinute, referenceDateSecond);
            }
            else if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
            {
                if (!(-1 == referenceDateYear && -1 == referenceDateMonth && -1 == referenceDateDay && -1 == referenceDateHour && -1 == referenceDateMinute && -1.0 == referenceDateSecond))
                {
                    CkError("ERROR in ADHydro::ADHydro: Some of referenceDateYear, Month, Day, Hour, Minute, Second were specified, but not all were specified and within valid ranges.  See comments in the example superfile for valid ranges.\n");
                    error = true;
                }
            }
        }
    }
    
    if (!error)
    {
        // Get readonly variables from the superfile.
        Readonly::simulationStartTime     = superfile.GetReal(   "", "simulationStartTime",     NAN);
        Readonly::simulationDuration      = superfile.GetReal(   "", "simulationDuration",      NAN);
        Readonly::checkpointPeriod        = superfile.GetReal(   "", "checkpointPeriod",        INFINITY);
        Readonly::checkpointGroupSize     = superfile.GetInteger("", "checkpointGroupSize",     1);
        Readonly::checkpointDirectoryPath = superfile.Get(       "", "checkpointDirectoryPath", ".");
        Readonly::drainDownMode           = superfile.GetBoolean("", "drainDownMode",           false);
        Readonly::zeroExpirationTime      = superfile.GetBoolean("", "zeroExpirationTime",      false);
        Readonly::zeroCumulativeFlow      = superfile.GetBoolean("", "zeroCumulativeFlow",      false);
        Readonly::zeroWaterCreated        = superfile.GetBoolean("", "zeroWaterCreated",        false);
        Readonly::verbosityLevel          = superfile.GetInteger("", "verbosityLevel",          2);
        
        // At this point, Readonly::referenceDate, Readonly::simulationStartTime, and Readonly::simulationDuration could all be NAN.
        // If Readonly::referenceDate or Readonly::simulationStartTime are NAN then the initialization manager will read them from file.
        // If Readonly::simulationDuration is NAN it is an error, which will be caught when the initialization manager calls Readonly::checkInvariant().
        
        // If simulationStartTime was changed then expiration times are invalid and need to be recalculated.
        if (!isnan(Readonly::simulationStartTime))
        {
            Readonly::zeroExpirationTime = true;
        }
        
        // The minimum checkpointGroupSize is one.  Since the variable is unsigned the simplest thing to do is treat zero as one and then no condition produces an error.
        if (0 == Readonly::checkpointGroupSize)
        {
            Readonly::checkpointGroupSize = 1;
        }
        
        // Create Region array.  This requires knowing the number of regions, which must be read from file.
        // The initialization managers will re-open the file to read lots of other stuff, but getting this one value here simplifies the code,
        // and opening and closing the file one extra time isn't a high performance cost.
        // FIXME read the real value
        Readonly::globalNumberOfRegions = 2;
        
        regionProxy = CProxy_Region::ckNew(Readonly::globalNumberOfRegions);
        
        // Create manager groups.
        initializationManagerProxy = CProxy_InitializationManager::ckNew();
        checkpointManagerProxy     = CProxy_CheckpointManager::ckNew();
        forcingManagerProxy        = CProxy_ForcingManager::ckNew();
    }
    
    if (error)
    {
        CkExit();
    }
}

// Global readonly variables.
CProxy_InitializationManager ADHydro::initializationManagerProxy;
CProxy_CheckpointManager     ADHydro::checkpointManagerProxy;
CProxy_ForcingManager        ADHydro::forcingManagerProxy;
CProxy_Region                ADHydro::regionProxy;
