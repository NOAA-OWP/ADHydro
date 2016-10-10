#include "adhydro.h"
#include "INIReader.h"

// Global readonly variables.
std::string               ADHydro::adhydroOutputDirectoryPath;
double                    ADHydro::referenceDate;
double                    ADHydro::simulationStartTime;
double                    ADHydro::simulationDuration;
double                    ADHydro::outputPeriod;
size_t                    ADHydro::outputGroupSize;
int                       ADHydro::verbosityLevel;
CProxy_OutputManagerCharm ADHydro::outputManagerProxy;

ADHydro::ADHydro(CkArgMsg* msg)
{
  bool        error         = false;                                   // Error flag.
  const char* superfileName = (1 < msg->argc) ? (msg->argv[1]) : (""); // The first command line argument protected against non-existance.
  INIReader   superfile(superfileName);                                // Superfile reader object.
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
      adhydroOutputDirectoryPath = superfile.Get("", "adhydroOutputDirectoryPath", ".");

      referenceDate = superfile.GetReal("", "referenceDateJulian", NAN);

      // If there is no referenceDateJulian read a Gregorian date and convert to Julian date.
      if (isnan(referenceDate))
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
              referenceDate = gregorianToJulian(referenceDateYear, referenceDateMonth, referenceDateDay, referenceDateHour, referenceDateMinute, referenceDateSecond);
            }
          else if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
            {
              if (!(-1 == referenceDateYear && -1 == referenceDateMonth && -1 == referenceDateDay && -1 == referenceDateHour && -1 == referenceDateMinute && -1.0 == referenceDateSecond))
                {
                  CkError("ERROR in ADHydro::ADHydro: Some of referenceDateYear, Month, Day, Hour, Minute, Second were specified, but not all were specified and within valid ranges.  "
                      "See comments in the example superfile for valid ranges.\n");
                  error = true;
                }
            }
        }
    }

  if (!error)
    {
      simulationStartTime = superfile.GetReal(   "", "simulationStartTime", NAN);
      simulationDuration  = superfile.GetReal(   "", "simulationDuration",  0.0);
      outputPeriod        = superfile.GetReal(   "", "outputPeriod",        INFINITY);
      outputGroupSize     = superfile.GetInteger("", "outputGroupSize",     1);

      if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
          // FIXME check that referenceDate and simulationStartTime are no earlier than 1 CE.  Our Julian to Gregorian conversion routines don't work before 1 CE.
          // Error check referenceDate and simulationStartTime after they are possibly loaded from file.

          if (!(0.0 <= simulationDuration))
            {
              CkError("ERROR in ADHydro::ADHydro: simulationDuration must be greater than or equal to zero.\n");
              error = true;
            }

          if (!(0.0 < outputPeriod))
            {
              CkError("ERROR in ADHydro::ADHydro: outputPeriod must be greater than zero.\n");
              error = true;
            }

          if (!(0 < outputGroupSize))
            {
              CkError("ERROR in ADHydro::ADHydro: outputGroupSize must be greater than zero.\n");
              error = true;
            }
        }
    }

  if (!error)
    {
      verbosityLevel = superfile.GetInteger("", "verbosityLevel", 2);

      // Create output manager.
      outputManagerProxy = CProxy_OutputManagerCharm::ckNew(FILE_MANAGER_NETCDF);

      // FIXME call this after referenceDate and simulationStartTime are possibly loaded from file.
      outputManagerProxy.sendInitialize();

      // FIXME remove
      /**/
      MeshElementStateCharm meshState(0, 1, 0, 0);
      outputManagerProxy[0].sendMeshElementState(meshState);
      meshState.elementNumber = 1;
      outputManagerProxy[0].sendMeshElementState(meshState);
      meshState.elementNumber = 2;
      outputManagerProxy[0].sendMeshElementState(meshState);
      meshState.elementNumber = 3;
      outputManagerProxy[1 % CkNumPes()].sendMeshElementState(meshState);
      meshState.elementNumber = 4;
      outputManagerProxy[1 % CkNumPes()].sendMeshElementState(meshState);
      ChannelElementStateCharm channelState(0, 1, 0);
      outputManagerProxy[0].sendChannelElementState(channelState);
      channelState.elementNumber = 1;
      outputManagerProxy[0].sendChannelElementState(channelState);
      channelState.elementNumber = 2;
      outputManagerProxy[1 % CkNumPes()].sendChannelElementState(channelState);
      channelState.elementNumber = 3;
      outputManagerProxy[1 % CkNumPes()].sendChannelElementState(channelState);
      /**/
    }

  if (error)
    {
      CkExit();
    }
}

#include "adhydro.def.h"
