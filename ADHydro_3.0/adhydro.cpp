#include "adhydro.h"
#include "adhydro.def.h"
#include "INIReader.h"

ADHydro::ADHydro(CkArgMsg* msg)
{
    bool        error         = false;                                   // Error flag.
    const char* superfileName = (1 < msg->argc) ? (msg->argv[1]) : (""); // The first command line argument protected against non-existance.
    INIReader   superfile(superfileName);                                // Superfile reader object.
    std::string noahMPDirectoryPath;                                     // Directory path to use with default filenames if file paths not specified.
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
        
        if (!isnan(Readonly::simulationStartTime))
        {
            Readonly::zeroExpirationTime = true;
        }
        
        if (0 == Readonly::checkpointGroupSize)
        {
            Readonly::checkpointGroupSize = 1;
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            // FIXME check that referenceDate and simulationStartTime are no earlier than 1 CE.  Our Julian to Gregorian conversion routines don't work before 1 CE.
            // Error check referenceDate and simulationStartTime after they are possibly loaded from file.
            
            if (!(0.0 < Readonly::simulationDuration))
            {
                CkError("ERROR in ADHydro::ADHydro: simulationDuration must be greater than zero.\n");
                error = true;
            }
            
            if (!(0.0 < Readonly::checkpointPeriod))
            {
                CkError("ERROR in ADHydro::ADHydro: checkpointPeriod must be greater than zero.\n");
                error = true;
            }
        }
    }
    
    if (!error)
    {
        // Create manager groups.
        initializationManagerProxy = CProxy_InitializationManager::ckNew();
        checkpointManagerProxy     = CProxy_CheckpointManager::ckNew();
    }
    
    if (error)
    {
        CkExit();
    }
}

// Global readonly variables.
CProxy_InitializationManager ADHydro::initializationManagerProxy;
CProxy_CheckpointManager     ADHydro::checkpointManagerProxy;
CProxy_Region                ADHydro::regionProxy;
