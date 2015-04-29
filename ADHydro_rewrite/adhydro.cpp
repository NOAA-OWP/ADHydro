#include "adhydro.h"

// Global read-only variables.
double        ADHydro::centralMeridian;
double        ADHydro::falseEasting;
double        ADHydro::falseNorthing;
bool          ADHydro::drainDownMode;
double        ADHydro::drainDownTime;
int           ADHydro::verbosityLevel;
CProxy_Region ADHydro::regionProxy;

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
  int          ii;                                                                                                                          // Loop counter.
  const int    numberOfDts               = 15;                                                                                              // Unitless.
  const double allowableDts[numberOfDts] = {1.0, 2.0, 3.0, 5.0, 10.0, 15.0, 30.0, 60.0, 120.0, 180.0, 300.0, 600.0, 900.0, 1800.0, 3600.0}; // Seconds.
  double       selectedDt;                                                                                                                  // Seconds.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < dtNew))
    {
      CkError("ERROR in ADHydro::newExpirationTime: dtNew must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (dtNew < allowableDts[0])
    {
      selectedDt = allowableDts[0];
      
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
  
  ii = floor(currentTime / selectedDt);
  
  while ((ii + 1) * selectedDt <= currentTime + dtNew)
    {
      ++ii;
    }
  
  return ii * selectedDt;
}

ADHydro::ADHydro(CkArgMsg* msg)
{
  evapoTranspirationInit("/user2/rsteinke/Desktop/ADHydro/HRLDAS-v3.6/Run/MPTABLE.TBL",
                         "/user2/rsteinke/Desktop/ADHydro/HRLDAS-v3.6/Run/VEGPARM.TBL",
                         "/user2/rsteinke/Desktop/ADHydro/HRLDAS-v3.6/Run/SOILPARM.TBL",
                         "/user2/rsteinke/Desktop/ADHydro/HRLDAS-v3.6/Run/GENPARM.TBL");
  
  regionProxy = CProxy_Region::ckNew(gregorianToJulian(2000, 1, 1, 0, 0, 0), 0.0, 25.0, 3);
  
  regionProxy.ckSetReductionClient(new CkCallback(CkCallback::ckExit));
}

#include "adhydro.def.h"
