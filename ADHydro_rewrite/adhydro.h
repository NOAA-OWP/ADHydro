#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "adhydro.decl.h"
#pragma GCC diagnostic warning "-Wsign-compare"

#include "all_charm.h"

// FIXME comment
class ADHydro : public CBase_ADHydro
{
  ADHydro_SDAG_CODE
  
public:
  
  // Global read-only variables to specify map projection.  Currently we only support a sinusoidal projection.
  static double centralMeridian; // Radians.
  static double falseEasting;    // Meters.
  static double falseNorthing;   // Meters.
  
  // Global read-only variables to specify whether to run the simulation in
  // drain down mode.  In this mode, there will be no precipitation, the water
  // level in the channels will be capped at bank-full depth, and calendar date
  // and time will stand still.  Forcing data, sun angle, and other things that
  // depend on absolute time will always be taken from a single point in time
  // specified by drainDownTime.
  static bool   drainDownMode; // If true, run in drain down mode.
  static double drainDownTime; // Seconds since Element::referenceDate.
  
  // Global read-only variable specifies how much to print to the display.
  // Current verbosity levels are:
  //
  // 0 - Error messages only.
  // 1 - Messages about general simulation progress.
  // 2 - Warning messages about situations that may be a problem.
  // 3 - Messages about general simulation progress or warning messages about
  //     situations that may be a problem that just create too many messages
  //     for levels one and two.
  // 4 - Warning messages about situations that are probably ok.
  static int verbosityLevel;
  
  // Calculate latitude and longitude from X,Y coordinates.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // x         - X coordinate in meters.
  // y         - Y coordinate in meters.
  // latitude  - Scalar passed by reference will be filled in with latitude in
  //             radians.
  // longitude - Scalar passed by reference will be filled in with longitude in
  //             radians.
  static bool getLatLong(double x, double y, double& latitude, double& longitude);
  
  // Returns: simulation time in seconds since Element::referenceDate for the
  //          new expiration time of a nominal flow rate.
  //
  // The simulation runs more efficiently if multiple expiration times expire
  // simultaneously.  In order to increase the likelihood of that happening we
  // Force expiration times to expire at certain discrete times.  First, choose
  // the number closest to but not over dtNew from the following divisors of
  // sixty.  Not all divisors of sixty are present in the list because some
  // pairs of divisors have a bad synodic period.
  //
  // 1, 2, 3, 5, 10, 15, 30, 60
  //
  // If dtNew is between one second and one minute the number is in seconds.
  // If dtNew is one minute or more the number is in minutes.  If dtNew is less
  // than one second choose one second divided by a power of two.  The
  // expiration time must be a multiple of this number.  Choose the expiration
  // time as the latest multiple of this number less than or equal to
  // currentTime + dtNew.
  //
  // Examples:
  //
  // currentTime = 0.0, dtNew = 4.3, newExpirationTime = 3.0
  // currentTime = 2.0, dtNew = 3.1, newExpirationTime = 3.0
  // currentTime = 2.0, dtNew = 4.3, newExpirationTime = 6.0
  static double newExpirationTime(double currentTime, double dtNew);
  
  // Constructor.  This is the mainchare constructor where the program starts.
  //
  // Parameters:
  //
  // msg - Charm++ command line arguments message.
  ADHydro(CkArgMsg* msg);
};

#endif // __ADHYDRO_H__
