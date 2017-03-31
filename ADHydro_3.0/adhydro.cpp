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
        // FIXME put this in a chare group so it gets done on every PE.
        // Initialize Noah-MP.
        error = evapoTranspirationInit(Readonly::noahMPMpTableFilePath.c_str(),  Readonly::noahMPVegParmFilePath.c_str(), Readonly::noahMPSoilParmFilePath.c_str(), Readonly::noahMPGenParmFilePath.c_str());
    }
    
    if (!error)
    {
        // FIXME do this in the initialization manager
        // Create the chare array of regions.
        ADHydro::regionProxy = CProxy_Region::ckNew(2);
        
        // FIXME initialize for real
        EvapoTranspirationStateStruct evapoTranspirationState;
        
        evapoTranspirationState.fIceOld[0] = 0;
        evapoTranspirationState.fIceOld[1] = 0;
        evapoTranspirationState.fIceOld[2] = 0;
        evapoTranspirationState.albOld     = 1;
        evapoTranspirationState.snEqvO     = 0;
        evapoTranspirationState.stc[0]     = 0;
        evapoTranspirationState.stc[1]     = 0;
        evapoTranspirationState.stc[2]     = 0;
        evapoTranspirationState.stc[3]     = 300;
        evapoTranspirationState.stc[4]     = 300;
        evapoTranspirationState.stc[5]     = 300;
        evapoTranspirationState.stc[6]     = 300;
        evapoTranspirationState.tah        = 300;
        evapoTranspirationState.eah        = 2000;
        evapoTranspirationState.fWet       = 0;
        evapoTranspirationState.canLiq     = 0;
        evapoTranspirationState.canIce     = 0;
        evapoTranspirationState.tv         = 300;
        evapoTranspirationState.tg         = 300;
        evapoTranspirationState.iSnow      = 0;
        evapoTranspirationState.zSnso[0]   = 0;
        evapoTranspirationState.zSnso[1]   = 0;
        evapoTranspirationState.zSnso[2]   = 0;
        evapoTranspirationState.zSnso[3]   = -.05;
        evapoTranspirationState.zSnso[4]   = -.2;
        evapoTranspirationState.zSnso[5]   = -.5;
        evapoTranspirationState.zSnso[6]   = -1;
        evapoTranspirationState.snowH      = 0;
        evapoTranspirationState.snEqv      = 0;
        evapoTranspirationState.snIce[0]   = 0;
        evapoTranspirationState.snIce[1]   = 0;
        evapoTranspirationState.snIce[2]   = 0;
        evapoTranspirationState.snLiq[0]   = 0;
        evapoTranspirationState.snLiq[1]   = 0;
        evapoTranspirationState.snLiq[2]   = 0;
        evapoTranspirationState.lfMass     = 100000;
        evapoTranspirationState.rtMass     = 100000;
        evapoTranspirationState.stMass     = 100000;
        evapoTranspirationState.wood       = 200000;
        evapoTranspirationState.stblCp     = 200000;
        evapoTranspirationState.fastCp     = 200000;
        evapoTranspirationState.lai        = 4.6;
        evapoTranspirationState.sai        = 0.6;
        evapoTranspirationState.cm         = 0.002;
        evapoTranspirationState.ch         = 0.002;
        evapoTranspirationState.tauss      = 0;
        evapoTranspirationState.deepRech   = 0;
        evapoTranspirationState.rech       = 0;
        
        std::map<NeighborConnection, NeighborProxy> neighbors;
        
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SURFACE, 0, MESH_SURFACE,    1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SOIL,    0, MESH_SOIL,       1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_AQUIFER, 0, MESH_AQUIFER,    1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SURFACE, 0, CHANNEL_SURFACE, 0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SOIL   , 0, CHANNEL_SURFACE, 0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_AQUIFER, 0, CHANNEL_SURFACE, 0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        
        ADHydro::regionProxy[0].sendInitializeMeshElement(MeshElement(0, 0, 0.0, 0.0, 0.0, 100, 0.0, 0.0, 11, 2, 0.16, true, 3.38E-6, true, 9.74E-7, &evapoTranspirationState, 0.0, 0.0, UNSATURATED_AQUIFER, 0.0,
                                                                      SimpleGroundwater(0.1, 1.41E-5, 0.421, 0.0426, 0.0, 0.9), 0.0, -0.9, SimpleGroundwater(0.8, 4.66E-5, 0.339, 0.0279, 0.0, 0.8), 0.0, 0.0, 0.0, 0.0, 0.0, neighbors));
        
        neighbors.clear();
        
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, MESH_SURFACE,     0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, MESH_SOIL   ,     0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, MESH_AQUIFER,     0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, CHANNEL_SURFACE,  1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, BOUNDARY_OUTFLOW, 0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        
        ADHydro::regionProxy[0].sendInitializeChannelElement(ChannelElement(0, STREAM, 0, 0.0, 0.0, 0.0, -1.0, 10.0, 0.0, 0.0, 1.0, 1.0, 0.038, 1.0, 1.41E-5, &evapoTranspirationState, 0.0, 0.0, 0.0, 0.0, neighbors));
        
        neighbors.clear();
        
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SURFACE, 1, MESH_SURFACE,    0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SOIL,    1, MESH_SOIL,       0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_AQUIFER, 1, MESH_AQUIFER,    0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SURFACE, 1, CHANNEL_SURFACE, 1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SOIL   , 1, CHANNEL_SURFACE, 1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_AQUIFER, 1, CHANNEL_SURFACE, 1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        
        ADHydro::regionProxy[1].sendInitializeMeshElement(MeshElement(1, 1, 0.0, 0.0, 0.0, 100, 0.0, 0.0, 11, 2, 0.16, true, 3.38E-6, true, 9.74E-7, &evapoTranspirationState, 0.0, 0.0, UNSATURATED_AQUIFER, 0.0,
                                                                      SimpleGroundwater(0.1, 1.41E-5, 0.421, 0.0426, 0.0, 0.9), 0.0, -0.9, SimpleGroundwater(0.8, 4.66E-5, 0.339, 0.0279, 0.0, 0.8), 0.0, 0.0, 0.0, 0.0, 0.0, neighbors));
        
        neighbors.clear();
        
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, MESH_SURFACE,    1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, MESH_SOIL   ,    1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, MESH_AQUIFER,    1), NeighborProxy(1, 0.0, 0.0, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, CHANNEL_SURFACE, 0), NeighborProxy(0, 0.0, 0.0, 0.0, 0.0)));
        
        ADHydro::regionProxy[1].sendInitializeChannelElement(ChannelElement(1, STREAM, 1, 0.0, 0.0, 0.0, -1.0, 10.0, 0.0, 0.0, 1.0, 1.0, 0.038, 1.0, 1.41E-5, &evapoTranspirationState, 0.0, 0.0, 0.0, 0.0, neighbors));
    }
    
    if (error)
    {
        CkExit();
    }
}

// Global readonly variables.
CProxy_Region ADHydro::regionProxy;
