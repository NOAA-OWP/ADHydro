#ifndef __ALL_H__
#define __ALL_H__

// Either all.h or all_charm.h, but not both, should be included in all other
// files.  all.h should be used for non-Charm++ programs.  all_charm.h should
// be used for Charm++ programs.

#include <cmath>
#include <algorithm>

// The ADHYDRO_ASSERT macro is used to accomodate the fact that all.h will
// sometimes be used for non-Charm++ programs, but will be included in
// all_charm.h for Charm++ programs.  ADHYDRO_ASSERT is only meant to be used
// inside all.h or any code that might be used sometimes with Charm++ and
// sometimes not.  If you know whether some particular code uses Charm++ or not
// you should not use this macro. Just use assert or CkAssert instead of
// ADHYDRO_ASSERT.
#ifndef ADHYDRO_ASSERT
#include <cassert>

#define ADHYDRO_ASSERT assert
#endif // ADHYDRO_ASSERT

// Error checking, assertions, and invariants
//
// Error checking is the process of checking for invalid input.  Even bug free
// code needs to perform error checking because the code cannot control what it
// is given for input.  Code should handle invalid input gracefully.  Ideally
// it should not crash or seg fault on any input and should print a helpful
// error message.
// 
// Assertions check within the code for things that the code can guarantee to
// be true.  By definition, bug free code will never fire an assertion on any
// input.  This code does not satisfy that condition if any error checking is
// turned off.  Assertions are only needed for finding bugs and can be compiled
// out of production code.
// 
// Invariants are conditions that should always be true.  Invariants are
// usually written on complex data structures like a mesh or chanel network.
// Invariants can be used for both error checking and assertions.  For example,
// when reading in a mesh the code may check the invariant and throw an error
// if the input mesh does not satisfy the invariant.  Thereafter, the code may
// continue to check the invariant periodically as an assertion.
// 
// After error checking and assertions the rest of the code is written assuming
// that those checks pass.  For example, if statement cases may be omitted if
// they would be forbidden by an assertion.
#define DEBUG_LEVEL_LIBRARY_ERRORS               (1 << 0) // Error checking of return values of library calls.
#define DEBUG_LEVEL_USER_INPUT_SIMPLE            (1 << 1) // Error checking of simple conditions on user inputs.
#define DEBUG_LEVEL_USER_INPUT_INVARIANTS        (1 << 2) // Error checking of invariants        on user inputs.
#define DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE      (1 << 3) // Error checking of simple conditions on inputs to public  functions.
#define DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS  (1 << 4) // Error checking of invariants        on inputs to public  functions.
#define DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE     (1 << 5) // Assertion      of simple conditions on inputs to private functions.
#define DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS (1 << 6) // Assertion      of invariants        on inputs to private functions.
#define DEBUG_LEVEL_INTERNAL_SIMPLE              (1 << 7) // Assertion      of simple conditions within the code.
#define DEBUG_LEVEL_INTERNAL_INVARIANTS          (1 << 8) // Assertion      of invariants        within the code.

// No error checking or assertions.  Not recommended.
#define DEBUG_LEVEL_NONE (0)

// This is an appropriate debug level for production code released as an executable program.
#define DEBUG_LEVEL_PRODUCTION_EXECUTABLE (DEBUG_LEVEL_LIBRARY_ERRORS | DEBUG_LEVEL_USER_INPUT_SIMPLE | DEBUG_LEVEL_USER_INPUT_INVARIANTS)

// This is an appropriate debug level for production code released as a library.
#define DEBUG_LEVEL_PRODUCTION_LIBRARY (DEBUG_LEVEL_LIBRARY_ERRORS | DEBUG_LEVEL_USER_INPUT_SIMPLE | DEBUG_LEVEL_USER_INPUT_INVARIANTS | \
                                        DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE | DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

// This is an appropriate debug level for development code.
#define DEBUG_LEVEL_DEVELOPMENT (DEBUG_LEVEL_LIBRARY_ERRORS | DEBUG_LEVEL_USER_INPUT_SIMPLE | DEBUG_LEVEL_USER_INPUT_INVARIANTS |                       \
                                 DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE | DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS | DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE | \
                                 DEBUG_LEVEL_INTERNAL_SIMPLE | DEBUG_LEVEL_INTERNAL_INVARIANTS)

// This debug level is paranoid and expensive, but could help localize bugs.
#define DEBUG_LEVEL_ALL (DEBUG_LEVEL_LIBRARY_ERRORS | DEBUG_LEVEL_USER_INPUT_SIMPLE | DEBUG_LEVEL_USER_INPUT_INVARIANTS |                       \
                         DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE | DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS | DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE | \
                         DEBUG_LEVEL_PRIVATE_FUNCTIONS_INVARIANTS | DEBUG_LEVEL_INTERNAL_SIMPLE | DEBUG_LEVEL_INTERNAL_INVARIANTS)

// Set this macro to the debug level to use.
#define DEBUG_LEVEL (DEBUG_LEVEL_DEVELOPMENT)

// Constants.
#define GRAVITY               (9.81)      // Meters per second squared.
#define POLAR_RADIUS_OF_EARTH (6356752.3) // Meters.
#define ZERO_C_IN_KELVIN      (273.15)    // Kelvin.
//NJF Change ponded depth to 1 cm to improve numerical stability especially in channels. Previous 1 mm (0.001 M)
//#define PONDED_DEPTH          (0.01)     // Meters.  Water can be ponded due to micro-topography.  Surfacewater depth below this will have no flow.
//PONDED_DEPTH has been redefined in terms of MESH_RETENTION_DEPTH and CHANNEL_RETENTION_DEPTH
#define MESH_RETENTION_DEPTH    (0.001)    // Meters.  Depth of water retained on mesh elements due to micro-topography.  Surfacewater depth below this will have no flow.
#define CHANNEL_RETENTION_DEPTH (0.01)     // Meters.  Depth of water retained in channel elements.  Surfacewater depth below this will have no flow.

// Special cases of element boundaries.
enum BoundaryConditionEnum
{
  NOFLOW  = -1, // This must be -1 because Triangle and TauDEM both use -1 to indicate no neighbor.
  INFLOW  = -2, // Others must be non-positive because positive numbers are used for neighbor array indices.
  OUTFLOW = -3,
};

// Returns: true if x is a boundary condition code, false otherwise.
inline bool isBoundary(int x)
{
  return (NOFLOW == x || INFLOW == x || OUTFLOW == x);
}

// Used for the type of channel links.
enum ChannelTypeEnum
{
  NOT_USED      = 0, // Not all link numbers are used.
  STREAM        = 1, // Stream link.
  WATERBODY     = 2, // Waterbody link.
  ICEMASS       = 3, // Equivalent to WATERBODY, but made of ice.
  PRUNED_STREAM = 4, // A pruned link was once used but is no longer used.
};

// Utility functions for epsilon-equality testing of doubles. Two doubles are
// considered epsilon-equal if they are within epsilon.  Epsilon is the larger
// of 10^-10 or ten orders of magnitude smaller than one of the two numbers
// being compared.  This is necessary because large values stored in a double
// might have less resolution than ten digits past the decimal point, but they
// will always have ten significant digits.  Which of the two numbers being
// compared is used to calculate epsilon is chosen arbitrarily.  If the numbers
// are similar the epsilon values will be similar and it won't matter.  If the
// numbers are different they won't be epsilon-equal and the exact value of
// epsilon won't matter.

// Returns: the epsilon value to use for epsilon-equality testing with x.
inline double epsilon(double x)
{
  return 1.0e-10 * std::max(std::fabs(x), 1.0);
}

// Returns: true if a is less than and not epsilon-equal to b, false otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonLess(double a, double b, double epsilonProxy = 0.0)
{
  return a < b - epsilon(std::max(std::fabs(b), std::fabs(epsilonProxy)));
}

// Returns: true if a is greater than and not epsilon-equal to b, false
//          otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonGreater(double a, double b, double epsilonProxy = 0.0)
{
  return a > b + epsilon(std::max(std::fabs(b), std::fabs(epsilonProxy)));
}

// Returns: true if a is less than or epsilon-equal to b, false otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonLessOrEqual(double a, double b, double epsilonProxy = 0.0)
{
  return !epsilonGreater(a, b, epsilonProxy);
}

// Returns: true if a is greater than or epsilon-equal to b, false otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonGreaterOrEqual(double a, double b, double epsilonProxy = 0.0)
{
  return !epsilonLess(a, b, epsilonProxy);
}

// Returns: true if a is epsilon-equal to b, false otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonEqual(double a, double b, double epsilonProxy = 0.0)
{
  return !epsilonLess(a, b, epsilonProxy) && !epsilonGreater(a, b, epsilonProxy);
}

// Utility functions for epsilon-equality testing of floats.  Same as the
// functions for doubles, but using 10^-6 for epsilon instead of 10^-10.

// Returns: the epsilon value to use for epsilon-equality testing with x.
inline float epsilon(float x)
{
  return 1.0e-6f * std::max(std::fabs(x), 1.0f);
}

// Returns: true if a is less than and not epsilon-equal to b, false otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonLess(float a, float b, float epsilonProxy = 0.0)
{
  return a < b - epsilon(std::max(std::fabs(b), std::fabs(epsilonProxy)));
}

// Returns: true if a is greater than and not epsilon-equal to b, false
//          otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonGreater(float a, float b, float epsilonProxy = 0.0)
{
  return a > b + epsilon(std::max(std::fabs(b), std::fabs(epsilonProxy)));
}

// Returns: true if a is less than or epsilon-equal to b, false otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonLessOrEqual(float a, float b, float epsilonProxy = 0.0)
{
  return !epsilonGreater(a, b, epsilonProxy);
}

// Returns: true if a is greater than or epsilon-equal to b, false otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonGreaterOrEqual(float a, float b, float epsilonProxy = 0.0)
{
  return !epsilonLess(a, b, epsilonProxy);
}

// Returns: true if a is epsilon-equal to b, false otherwise.
//
// epsilonProxy sets a floor on the magnitude used to generate epsilon.
inline bool epsilonEqual(float a, float b, float epsilonProxy = 0.0)
{
  return !epsilonLess(a, b, epsilonProxy) && !epsilonGreater(a, b, epsilonProxy);
}

// Utility functions for converting dates.

// Convert Gregorian date to Julian date.
//
// Gregorian dates are specified as year, month, day, hour, minute, second.  A
// Gregorian day runs from midnight to the following midnight.  Julian dates
// are specified as time in days, including fractional day, since noon, January
// 1, 4713 BCE.  A Julian day runs from noon to the following noon.
//
// This function does not work properly for years before 1 CE.  It is an error
// to pass in less than 1 for year.
//
// Copied from algorithm 199 in Collected algorithms of the CACM.
// Author: Robert G. Tantzen, Translator: Nat Howard
// Modified by RCS 10/14 for coding standards.
//
// Returns: the Julian date equal to the input Gregorian date.
//
// Parameters:
//
// year   - Gregorian year.  You must include the full year number.  E.g. 14 is
//          14 CE, not 2014 CE.  Must be positive.
// month  - Gregorian month, 1 to 12.
// day    - Gregorian day, 1 to 31.
// hour   - Gregorian hour, 0 to 23.
// minute - Gregorian minute, 0 to 59.
// second - Gregorian second including fractional second, 0 to 59.999999...
inline double gregorianToJulian(long year, long month, long day, long hour, long minute, double second)
{
  long   century;              // Number of centuries.
  long   yearInCentury;        // Tens and ones digit of the year, 0 to 99.
  long   julianDay;            // Julian day not including fractional day.
  double secondsSinceMidnight; // Number of seconds since midnight, 0.0 to 24.0 * 3600.0.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  ADHYDRO_ASSERT(1L <= year && 1L <= month && 12L >= month && 1L <= day && 31L >= day && 0L <= hour && 23L >= hour && 0L <= minute && 59L >= minute &&
                 0.0 <= second && 60.0 > second);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  if (month > 2L)
    {
      month -= 3L;
    }
  else
    {
      month += 9L;
      year--;
    }

  century              = year / 100L;
  yearInCentury        = year - (century * 100L);
  julianDay            = (146097L * century) / 4L + (1461L * yearInCentury) / 4L + (153L * month + 2L) / 5L + day + 1721119L;
  secondsSinceMidnight = hour * 3600.0 + minute * 60.0 + second;

  if (secondsSinceMidnight < 12.0 * 3600.0)
    {
      secondsSinceMidnight += 12.0 * 3600.0;
      julianDay--;
    }
  else
    {
      secondsSinceMidnight -= 12.0 * 3600.0;
    }

  return (julianDay + secondsSinceMidnight / (24.0 * 3600.0));
}

// Convert Julian date to Gregorian date.
//
// Julian dates are specified as time in days, including fractional day, since
// noon, January 1, 4713 BCE.  A Julian day runs from noon to the following
// noon.  Gregorian dates are specified as year, month, day, hour, minute,
// second.  A Gregorian day runs from midnight to the following midnight.
//
// This function does not work properly for years before 1 CE.  It is an error
// to pass in less than 1721425.5 for julian.  This is the julian date of
// beginning midnight, Jan 1, 1 CE.
//
// Copied from algorithm 199 in Collected algorithms of the CACM.
// Author: Robert G. Tantzen, Translator: Nat Howard
// Modified by FLO 4/99 to account for nagging round off error.
// Modified by RCS 10/14 for coding standards.
//
// Parameters:
//
// julian - Julian date.  Must be greater than or equal to 1721425.5.
// year   - Scalar passed by reference will be filled in with Gregorian year.
// month  - Scalar passed by reference will be filled in with Gregorian month,
//          1 to 12.
// day    - Scalar passed by reference will be filled in with Gregorian day,
//          1 to 31.
// hour   - Scalar passed by reference will be filled in with Gregorian hour,
//          0 to 23.
// minute - Scalar passed by reference will be filled in with Gregorian minute,
//          0 to 59.
// second - Scalar passed by reference will be filled in with Gregorian second
//          including fractional second, 0 to 59.999999...
inline void julianToGregorian(double julian, long* year, long* month, long* day, long* hour, long* minute, double* second)
{
  long   julianDay     = (long)julian;       // Julian day not including fractional day.
  double fractionalDay = julian - julianDay; // Fractional day since noon.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  ADHYDRO_ASSERT(1721425.5 <= julian && NULL != year && NULL != month && NULL != day && NULL != hour && NULL != minute && NULL != second);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  if (fractionalDay >= 0.5)
    {
      fractionalDay -= 0.5;
      julianDay++;
    }
  else
    {
      fractionalDay += 0.5;
    }

  julianDay -= 1721119L;
  *year      = (4L * julianDay - 1L) / 146097L;
  julianDay  = 4L * julianDay - 1L - 146097L * *year;
  *day       = julianDay / 4L;
  julianDay  = (4L * *day + 3L) / 1461L;
  *day       = 4L * *day + 3L - 1461L * julianDay;
  *day       = (*day + 4L) / 4L;
  *month     = (5L * *day - 3L) / 153L;
  *day       = 5L * *day - 3 - 153L * *month;
  *day       = (*day + 5L) / 5L;
  *year      = 100L * *year + julianDay;

  if (*month < 10L)
    {
      *month += 3L;
    }
  else
    {
      *month -= 9L;
      (*year)++;
    }

  *second  = fractionalDay * 24.0 * 3600.0;
  *hour    = (long) (*second / 3600.0);
  *second -= *hour * 3600.0;
  *minute  = (long) (*second / 60.0);
  *second -= *minute * 60.0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  ADHYDRO_ASSERT(1L <= *year && 1L <= *month && 12L >= *month && 1L <= *day && 31L >= *day && 0L <= *hour && 23L >= *hour && 0L <= *minute && 59L >= *minute &&
                 0.0 <= *second && 60.0 > *second);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
}

// Utility functions for geometric calculations.

// Calculate whether two line segments intersect, and if they do calculate the
// intersection point.  If the two line segments are parallel this function
// always returns no intersection even if the line segments are collinear and
// overlapping.  This is fine for us because we are using it to detect the
// intersection between an edge of a mesh element and a line segment from the
// center of that mesh element to its neighbor.  The center of the mesh element
// can never lie on one if its edges so the line segments cannot be collinear.
//
// This code is taken from:
//
// http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
//
// Modified by RCS 1/15 to change floats to doubles, make it inline, and add
// comments, but didn't clean up other aspects that don't fit our coding
// standards.
// Modified by RCS 3/15 for coding standards.
//
// Returns: true if the line segments intersect, false otherwise.
//
// Parameters:
//
// segmentOneBeginX - The X coordinate of the begin endpoint of the first line
//                    segment.
// segmentOneBeginY - The Y coordinate of the begin endpoint of the first line
//                    segment.
// segmentOneEndX   - The X coordinate of the end endpoint of the first line
//                    segment.
// segmentOneEndY   - The Y coordinate of the end endpoint of the first line
//                    segment.
// segmentTwoBeginX - The X coordinate of the begin endpoint of the second line
//                    segment.
// segmentTwoBeginY - The Y coordinate of the begin endpoint of the second line
//                    segment.
// segmentTwoEndX   - The X coordinate of the end endpoint of the second line
//                    segment.
// segmentTwoEndY   - The Y coordinate of the end endpoint of the second line
//                    segment.
// intersectionX    - Scalar passed by reference.  If the line segments
//                    intersect this will be filled in with the X coordinate of
//                    the intersection point.  Otherwise it will be unmodified.
//                    Or it can be passed in as NULL in which case it will be
//                    ignored.
// intersectionY    - Scalar passed by reference.  If the line segments
//                    intersect this will be filled in with the Y coordinate of
//                    the intersection point.  Otherwise it will be unmodified.
//                    Or it can be passed in as NULL in which case it will be
//                    ignored.
inline bool getLineIntersection(double segmentOneBeginX, double segmentOneBeginY, double segmentOneEndX, double segmentOneEndY, double segmentTwoBeginX,
                                double segmentTwoBeginY, double segmentTwoEndX, double segmentTwoEndY, double *intersectionX, double *intersectionY)
{
  // Calculate the span of each line segment in the X and Y directions.
  double segmentOneSpanX = segmentOneEndX - segmentOneBeginX;
  double segmentOneSpanY = segmentOneEndY - segmentOneBeginY;
  double segmentTwoSpanX = segmentTwoEndX - segmentTwoBeginX;
  double segmentTwoSpanY = segmentTwoEndY - segmentTwoBeginY;
  
  // Calculate the fractional distance along each line segment of the intersection point.  Zero means the intersection is at the begin endpoint of the segment.
  // One means it is at the end endpoint of the line segment.  Between zero and one means it is between the endpoints of the line segment.  If it is outside
  // the range zero to one then there is an intersection with the infinite line containing the line segment, but outside the span of the line segment.  If the
  // denominator is zero the lines are parallel.  In that case, testing for intersection by checking against the range zero to one will return no intersection
  // because the fraction value will be infinite or not a number.
  double segmentOneFraction = ( segmentTwoSpanX * (segmentOneBeginY - segmentTwoBeginY) - segmentTwoSpanY * (segmentOneBeginX - segmentTwoBeginX)) /
                              (-segmentTwoSpanX * segmentOneSpanY + segmentOneSpanX * segmentTwoSpanY);
  double segmentTwoFraction = (-segmentOneSpanY * (segmentOneBeginX - segmentTwoBeginX) + segmentOneSpanX * (segmentOneBeginY - segmentTwoBeginY)) /
                              (-segmentTwoSpanX * segmentOneSpanY + segmentOneSpanX * segmentTwoSpanY);
  bool   intersection       = false;

  if (0.0 <= segmentOneFraction && 1.0 >= segmentOneFraction && 0.0 <= segmentTwoFraction && 1.0 >= segmentTwoFraction)
    {
      intersection = true;
      
      // intersection detected
      if (intersectionX != NULL)
        {
          *intersectionX = segmentOneBeginX + segmentOneSpanX * segmentOneFraction;
        }

      if (intersectionY != NULL)
        {
          *intersectionY = segmentOneBeginY + segmentOneSpanY * segmentOneFraction;
        }
    }

  return intersection;
}

// Get the area of a polygon.
//
// This code is taken from:
//
// http://www.mathopenref.com/coordpolygonarea2.html
//
// The vertices must be ordered in either a clockwise or counterclockwise
// traversal around the polygon.  The algorithm does not handle
// self-intersecting polygons correctly.
//
// Returns: the area of the polygon in square units of whatever the units are
//          of xCoordinates and yCoordinates.
//
// Parameters:
//
// xCoordinates - An array of the X coordinates of the vertices of the polygon.
// yCoordinates - An array of the Y coordinates of the vertices of the polygon.
//                Must be in the same order as xCoordinates.
// size         - The number of vertices.  Also the size of both the
//                xCoordinates and yCoordinates arrays.
inline double getAreaOfPolygon(double* xCoordinates, double* yCoordinates, int size)
{
  int    ii;                        // Loop counter.
  int    previousVertex = size - 1; // The last vertex is the 'previous' one to the first
  double area           = 0.0;      // Accumulator for area.
  
  for (ii = 0; ii < size; ++ii)
    {
      area          += (xCoordinates[previousVertex] + xCoordinates[ii]) * (yCoordinates[previousVertex] - yCoordinates[ii]);
      previousVertex = ii; // The current value of ii is previous to the next value of ii.
    }
  
  return fabs(0.5 * area);
}

#endif // __ALL_H__
