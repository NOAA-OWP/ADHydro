#include "adhydro.h"
#include "INIReader.h"

// Global readonly variables.
std::string                                        ADHydro::evapoTranspirationInitMpTableFilePath;
std::string                                        ADHydro::evapoTranspirationInitVegParmFilePath;
std::string                                        ADHydro::evapoTranspirationInitSoilParmFilePath;
std::string                                        ADHydro::evapoTranspirationInitGenParmFilePath;
bool                                               ADHydro::initializeFromASCIIFiles;
std::string                                        ADHydro::ASCIIInputMeshNodeFilePath;
std::string                                        ADHydro::ASCIIInputMeshZFilePath;
std::string                                        ADHydro::ASCIIInputMeshElementFilePath;
std::string                                        ADHydro::ASCIIInputMeshNeighborFilePath;
std::string                                        ADHydro::ASCIIInputMeshLandCoverFilePath;
std::string                                        ADHydro::ASCIIInputMeshSoilTypeFilePath;
std::string                                        ADHydro::ASCIIInputMeshGeolTypeFilePath;
std::string                                        ADHydro::ASCIIInputMeshEdgeFilePath;
std::string                                        ADHydro::ASCIIInputChannelNodeFilePath;
std::string                                        ADHydro::ASCIIInputChannelZFilePath;
std::string                                        ADHydro::ASCIIInputChannelElementFilePath;
std::string                                        ADHydro::ASCIIInputChannelPruneFilePath;
std::string                                        ADHydro::adhydroInputGeometryFilePath;
std::string                                        ADHydro::adhydroInputParameterFilePath;
std::string                                        ADHydro::adhydroInputStateFilePath;
std::string                                        ADHydro::adhydroInputForcingFilePath;
std::string                                        ADHydro::adhydroOutputGeometryFilePath;
std::string                                        ADHydro::adhydroOutputParameterFilePath;
std::string                                        ADHydro::adhydroOutputStateFilePath;
std::string                                        ADHydro::adhydroOutputDisplayFilePath;
double                                             ADHydro::centralMeridian;
double                                             ADHydro::falseEasting;
double                                             ADHydro::falseNorthing;
double                                             ADHydro::referenceDate;
double                                             ADHydro::currentTime;
double                                             ADHydro::simulationDuration;
double                                             ADHydro::checkpointPeriod;
double                                             ADHydro::outputPeriod;
InfiltrationAndGroundwater::InfiltrationMethodEnum ADHydro::infiltrationMethod;
bool                                               ADHydro::drainDownMode;
double                                             ADHydro::drainDownTime;
bool                                               ADHydro::doMeshMassage;
int                                                ADHydro::verbosityLevel;
CProxy_FileManager                                 ADHydro::fileManagerProxy;
CProxy_Region                                      ADHydro::regionProxy;

bool ADHydro::getLatLong(double x, double y, double& latitude, double& longitude)
{
  bool error = false; // Error flag.
  
  latitude  = (y - falseNorthing) / POLAR_RADIUS_OF_EARTH;
  longitude = centralMeridian + (x - falseEasting) / (POLAR_RADIUS_OF_EARTH * cos(latitude));
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(-M_PI / 2.0 <= latitude && M_PI / 2.0 >= latitude))
    {
      CkError("ERROR in ADHydro::getLatLong: x and y must produce a latitude greater than or equal to negative PI over two and less than or equal to PI over "
              "two.\n");
      error = true;
    }
  
  if (!(-M_PI * 2.0 <= longitude && M_PI * 2.0 >= longitude))
    {
      CkError("ERROR in ADHydro::getLatLong: x and y must produce a longitude greater than or equal to negative two PI and less than or equal to two PI.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  return error;
}

double ADHydro::newExpirationTime(double currentTime, double dtNew)
{
  int          ii;                                                     // Loop counter.
  const int    numberOfDts               = 15;                         // Unitless.
  const double allowableDts[numberOfDts] = {1.0, 2.0, 3.0, 5.0, 10.0, 15.0, 30.0, 60.0, 2.0 * 60.0, 3.0 * 60.0, 5.0 * 60.0, 10.0 * 60.0, 15.0 * 60.0,
                                            30.0 * 60.0, 60.0 * 60.0}; // Seconds.
  double       selectedDt;                                             // Seconds.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtNew))
    {
      CkError("ERROR in ADHydro::newExpirationTime: dtNew must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (dtNew < allowableDts[0])
    {
      selectedDt = allowableDts[0] * 0.5;
      
      while (dtNew < selectedDt)
        {
          selectedDt *= 0.5;
        }
    }
  else
    {
      ii = 0;
      
      while (ii + 1 < numberOfDts && dtNew >= allowableDts[ii + 1])
        {
          ++ii;
        }
      
      selectedDt = allowableDts[ii];
    }
  
  ii = floor(currentTime / selectedDt) + 1;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  CkAssert(ii * selectedDt <= currentTime + dtNew);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  while ((ii + 1) * selectedDt <= currentTime + dtNew)
    {
      ++ii;
    }
  
  return ii * selectedDt;
}

ADHydro::ADHydro(CkArgMsg* msg)
{
  const char* superfileName = (1 < msg->argc) ? (msg->argv[1]) : (""); // The first command line argument protected against non-existance.
  INIReader   superfile(superfileName);                                // Superfile reader object.
  std::string evapoTranspirationInitDirectoryPath;                     // Directory path to use with default filenames if file paths not specified.
  std::string ASCIIInputDirectoryPath;                                 // Directory path to use with default filenames if file paths not specified.
  std::string ASCIIInputFileBasename;                                  // Directory path to use with default filenames if file paths not specified.
  std::string adhydroInputDirectoryPath;                               // Directory path to use with default filenames if file paths not specified.
  std::string adhydroOutputDirectoryPath;                              // Directory path to use with default filenames if file paths not specified.
  long        referenceDateYear;                                       // For converting Gregorian date to Julian date.
  long        referenceDateMonth;                                      // For converting Gregorian date to Julian date.
  long        referenceDateDay;                                        // For converting Gregorian date to Julian date.
  long        referenceDateHour;                                       // For converting Gregorian date to Julian date.
  long        referenceDateMinute;                                     // For converting Gregorian date to Julian date.
  double      referenceDateSecond;                                     // For converting Gregorian date to Julian date.
  std::string infiltrationMethodString;                                // For converting text to enum value.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  // If the superfile cannot be read print usage message and exit.
  if (0 != superfile.ParseError())
    {
      if (0 > superfile.ParseError())
        {
          CkError("ERROR in ADHydro::ADHydro: cannot open superfile %s.\n", superfileName);
        }
      else
        {
          CkError("ERROR in ADHydro::ADHydro: parse error on line number %d of superfile %s.\n", superfile.ParseError(), superfileName);
        }
      
      CkError("\nUsage:\n\nadhydro <superfile>\n\n");
      CkError("E.g.:\n\nadhydro ../input/example_superfile_v2.0.ini\n\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
  
  // Get readonly variables from the superfile.
  evapoTranspirationInitDirectoryPath    = superfile.Get("", "evapoTranspirationInitDirectoryPath", ".");
  evapoTranspirationInitMpTableFilePath  = superfile.Get("", "evapoTranspirationInitMpTableFilePath",  evapoTranspirationInitDirectoryPath + "/MPTABLE.TBL");
  evapoTranspirationInitVegParmFilePath  = superfile.Get("", "evapoTranspirationInitVegParmFilePath",  evapoTranspirationInitDirectoryPath + "/VEGPARM.TBL");
  evapoTranspirationInitSoilParmFilePath = superfile.Get("", "evapoTranspirationInitSoilParmFilePath", evapoTranspirationInitDirectoryPath + "/SOILPARM.TBL");
  evapoTranspirationInitGenParmFilePath  = superfile.Get("", "evapoTranspirationInitGenParmFilePath",  evapoTranspirationInitDirectoryPath + "/GENPARM.TBL");
  initializeFromASCIIFiles               = superfile.GetBoolean("", "initializeFromASCIIFiles", false);
  ASCIIInputDirectoryPath                = superfile.Get("", "ASCIIInputDirectoryPath", ".");
  ASCIIInputFileBasename                 = superfile.Get("", "ASCIIInputFileBasename", "mesh.1");
  ASCIIInputMeshNodeFilePath             = superfile.Get("", "ASCIIInputMeshNodeFilePath",       ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".node");
  ASCIIInputMeshZFilePath                = superfile.Get("", "ASCIIInputMeshZFilePath",          ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".z");
  ASCIIInputMeshElementFilePath          = superfile.Get("", "ASCIIInputMeshElementFilePath",    ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".ele");
  ASCIIInputMeshNeighborFilePath         = superfile.Get("", "ASCIIInputMeshNeighborFilePath",   ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".neigh");
  ASCIIInputMeshLandCoverFilePath        = superfile.Get("", "ASCIIInputMeshLandCoverFilePath",  ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".landCover");
  ASCIIInputMeshSoilTypeFilePath         = superfile.Get("", "ASCIIInputMeshSoilTypeFilePath",   ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".soilType");
  ASCIIInputMeshGeolTypeFilePath         = superfile.Get("", "ASCIIInputMeshGeolTypeFilePath",   ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".geolType");
  ASCIIInputMeshEdgeFilePath             = superfile.Get("", "ASCIIInputMeshEdgeFilePath",       ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".edge");
  ASCIIInputChannelNodeFilePath          = superfile.Get("", "ASCIIInputChannelNodeFilePath",    ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".chan.node");
  ASCIIInputChannelZFilePath             = superfile.Get("", "ASCIIInputChannelZFilePath",       ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".chan.z");
  ASCIIInputChannelElementFilePath       = superfile.Get("", "ASCIIInputChannelElementFilePath", ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".chan.ele");
  ASCIIInputChannelPruneFilePath         = superfile.Get("", "ASCIIInputChannelPruneFilePath",   ASCIIInputDirectoryPath + "/" + ASCIIInputFileBasename + ".chan.prune");
  adhydroInputDirectoryPath              = superfile.Get("", "adhydroInputDirectoryPath", ".");
  adhydroInputGeometryFilePath           = superfile.Get("", "adhydroInputGeometryFilePath",  adhydroInputDirectoryPath + "/geometry.nc");
  adhydroInputParameterFilePath          = superfile.Get("", "adhydroInputParameterFilePath", adhydroInputDirectoryPath + "/parameter.nc");
  adhydroInputStateFilePath              = superfile.Get("", "adhydroInputStateFilePath",     adhydroInputDirectoryPath + "/state.nc");
  adhydroInputForcingFilePath            = superfile.Get("", "adhydroInputForcingFilePath",   adhydroInputDirectoryPath + "/forcing.nc");
  adhydroOutputDirectoryPath             = superfile.Get("", "adhydroOutputDirectoryPath", ".");
  adhydroOutputGeometryFilePath          = superfile.Get("", "adhydroOutputGeometryFilePath",  adhydroOutputDirectoryPath + "/geometry.nc");
  adhydroOutputParameterFilePath         = superfile.Get("", "adhydroOutputParameterFilePath", adhydroOutputDirectoryPath + "/parameter.nc");
  adhydroOutputStateFilePath             = superfile.Get("", "adhydroOutputStateFilePath",     adhydroOutputDirectoryPath + "/state.nc");
  adhydroOutputDisplayFilePath           = superfile.Get("", "adhydroOutputDisplayFilePath",   adhydroOutputDirectoryPath + "/display.nc");
  
  centralMeridian = superfile.GetReal("", "centralMeridianRadians", NAN);
  
  if (isnan(centralMeridian))
    {
      centralMeridian = superfile.GetReal("", "centralMeridianDegrees", NAN) * M_PI / 180.0;
    }
  
  falseEasting  = superfile.GetReal("", "falseEasting", NAN);
  falseNorthing = superfile.GetReal("", "falseNorthing", NAN);
  
  referenceDate = superfile.GetReal("", "referenceDateJulian", NAN);
  
  // If there is no referenceDate read a Gregorian date and convert to Julian date.
  if (isnan(referenceDate))
    {
      referenceDateYear   = superfile.GetInteger("", "referenceDateYear", -1);
      referenceDateMonth  = superfile.GetInteger("", "referenceDateMonth", -1);
      referenceDateDay    = superfile.GetInteger("", "referenceDateDay", -1);
      referenceDateHour   = superfile.GetInteger("", "referenceDateHour", -1);
      referenceDateMinute = superfile.GetInteger("", "referenceDateMinute", -1);
      referenceDateSecond = superfile.GetReal("", "referenceDateSecond", -1.0);
      
      if (1 <= referenceDateYear && 1 <= referenceDateMonth && 12 >= referenceDateMonth && 1 <= referenceDateDay && 31 >= referenceDateDay &&
          0 <= referenceDateHour && 23 >= referenceDateHour && 0 <= referenceDateMinute && 59 >= referenceDateMinute && 0.0 <= referenceDateSecond &&
          60.0 > referenceDateSecond)
        {
          referenceDate = gregorianToJulian(referenceDateYear, referenceDateMonth, referenceDateDay, referenceDateHour, referenceDateMinute,
                                            referenceDateSecond);
        }
    }
  
  currentTime        = superfile.GetReal("", "currentTime", NAN);
  simulationDuration = superfile.GetReal("", "simulationDuration", 0.0);
  checkpointPeriod   = superfile.GetReal("", "checkpointPeriod", INFINITY);
  outputPeriod       = superfile.GetReal("", "outputPeriod", INFINITY);
  
  infiltrationMethodString = superfile.Get("", "infiltrationMethod", "NO_INFILTRATION");
  
  if ("NO_INFILTRATION" == infiltrationMethodString)
    {
      infiltrationMethod = InfiltrationAndGroundwater::NO_INFILTRATION;
    }
  else if ("TRIVIAL_INFILTRATION" == infiltrationMethodString)
    {
      infiltrationMethod = InfiltrationAndGroundwater::TRIVIAL_INFILTRATION;
    }
  else if ("GARTO_INFILTRATION" == infiltrationMethodString)
    {
      infiltrationMethod = InfiltrationAndGroundwater::GARTO_INFILTRATION;
    }
  else
    {
      CkError("ERROR in ADHydro::ADHydro: infiltrationMethod in superfile must be a valid enum value.\n");
      CkExit();
    }
  
  drainDownMode      = superfile.GetBoolean("", "drainDownMode", false);
  drainDownTime      = superfile.GetReal("", "drainDownTime", 0.0);
  doMeshMassage      = superfile.GetBoolean("", "doMeshMassage", false);
  verbosityLevel     = superfile.GetInteger("", "verbosityLevel", 2);
  
  // Create file manager.
  fileManagerProxy = CProxy_FileManager::ckNew();
}

#include "adhydro.def.h"
