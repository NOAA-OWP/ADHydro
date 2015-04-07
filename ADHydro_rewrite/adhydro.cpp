#include "adhydro.h"

// Global read-only variables.
double ADHydro::centralMeridian;
double ADHydro::falseEasting;
double ADHydro::falseNorthing;
int    ADHydro::verbosityLevel;

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

ADHydro::ADHydro(CkArgMsg* msg)
{
  thisProxy.FIXMEdeleteme();
}

#include "adhydro.def.h"
