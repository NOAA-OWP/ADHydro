#ifndef __ALL_H__
#define __ALL_H__

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <charm++.h>
#pragma GCC diagnostic warning "-Wsign-compare"

// Error checking, assertions, and invariants
//
// Error checking is the process of checking for invalid input.  Even bug free
// code needs to perform error checking because the code cannot control what it
// is given for input.  Code should handle invalid input gracefully.  Ideally
// it should not crash or seg fault on any input and should print a helpful
// error message.
// 
// Assertions check within the code for things that the code should be
// guaranteeing to be true.  By definition, bug free code will never fire an
// assertion on any input.  This code does not satisfy that condition if any
// error checking is turned off.  Assertions are only needed for finding bugs
// and can be compiled out of production code.
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
#define PONDED_DEPTH          (0.01)      // Meters.  Water can be ponded due to micro-topography.  Surfacewater depth below this will have no flow.

// Using NetCDF parallel collective I/O with Charm++ causes a problematic
// interaction.  Setting this flag causes a workaround to be used.  See the
// comment of FileManager::resizeUnlimitedDimensions in file_manager.h for more
// details.
// I think I figured out what was going on and it was a problem in my code.
// Remove this after verifying it works okay on Mt Moran and Yellowstone.
//#define NETCDF_COLLECTIVE_IO_WORKAROUND

// Special cases of element boundaries.
enum BoundaryConditionEnum
{
  NOFLOW  = -1, // This must be -1 because Triangle and TauDEM both use -1 to indicate no neighbor.
  INFLOW  = -2, // Others must be non-positive because positive numbers are used for neighbor array indices.
  OUTFLOW = -3
};

// Returns: true if x is a boundary condition code, false otherwise.
inline bool isBoundary(int x)
{
  return (NOFLOW == x || INFLOW == x || OUTFLOW == x);
}

// Flags to indicate how to interact with neighbors.
enum InteractionEnum
{
  I_CALCULATE_FLOW_RATE,
  NEIGHBOR_CALCULATES_FLOW_RATE,
  BOTH_CALCULATE_FLOW_RATE,
};

PUPbytes(InteractionEnum);

// Flags to indicate whether flow rate variables have been updated.
enum FlowRateReadyEnum
{
  FLOW_RATE_NOT_READY,
  FLOW_RATE_CALCULATED,
  FLOW_RATE_LIMITING_CHECK_DONE,
};

PUPbytes(FlowRateReadyEnum);

// Used for the type of channel links.
enum ChannelTypeEnum
{
  NOT_USED                           = 0, // Not all link numbers are used.
  STREAM                             = 1, // Stream link.
  WATERBODY                          = 2, // Waterbody link.
  ICEMASS                            = 3, // Equivalent to WATERBODY, but made of ice.
  PRUNED_STREAM                      = 4, // A pruned link was once used but is no longer needed.
};

PUPbytes(ChannelTypeEnum);

// Utility functions for near equality testing of doubles. Two doubles are
// considered near equal if they are within epsilon.  Epsilon is the larger of
// 10^-10 or ten orders of magnitude smaller than the number being checked
// because large doubles might have less resolution than ten digits past the
// decimal point, but they will always have ten significant digits.

// Returns: the epsilon value to use for near-equality testing with x.
inline double epsilon(double x)
{
  double eps = 1.0e-10;
  
  // Don't use library fabs and max to avoid including headers in all.h.
  if (0.0 > x)
    {
      x *= -1;
    }
  
  if (1.0 < x)
    {
      eps *= x;
    }
  
  return eps;
}

// Returns: true if a is less than and not near equal to b, false otherwise.
inline bool epsilonLess(double a, double b)
{
  return a < b - epsilon(b);
}

// Returns: true if a is greater than and not near equal to b, false otherwise.
inline bool epsilonGreater(double a, double b)
{
  return a > b + epsilon(b);
}

// Returns: true if a is less than or near equal to b, false otherwise.
inline bool epsilonLessOrEqual(double a, double b)
{
  return !epsilonGreater(a, b);
}

// Returns: true if a is greater than or near equal to b, false otherwise.
inline bool epsilonGreaterOrEqual(double a, double b)
{
  return !epsilonLess(a, b);
}

// Returns: true if a is near equal to b, false otherwise.
inline bool epsilonEqual(double a, double b)
{
  return !epsilonLess(a, b) && !epsilonGreater(a, b);
}

// Utility functions for near equality testing of floats.  Same as the
// functions for doubles, but using 10^-6 for epsilon instead of 10^-10.

// Returns: the epsilon value to use for near-equality testing with x.
inline float epsilon(float x)
{
  float eps = 1.0e-6f;
  
  // Don't use library fabs and max to avoid including headers in all.h.
  if (0.0 > x)
    {
      x *= -1;
    }
  
  if (1.0 < x)
    {
      eps *= x;
    }
  
  return eps;
}

// Returns: true if a is less than and not near equal to b, false otherwise.
inline bool epsilonLess(float a, float b)
{
  return a < b - epsilon(b);
}

// Returns: true if a is greater than and not near equal to b, false otherwise.
inline bool epsilonGreater(float a, float b)
{
  return a > b + epsilon(b);
}

// Returns: true if a is less than or near equal to b, false otherwise.
inline bool epsilonLessOrEqual(float a, float b)
{
  return !epsilonGreater(a, b);
}

// Returns: true if a is greater than or near equal to b, false otherwise.
inline bool epsilonGreaterOrEqual(float a, float b)
{
  return !epsilonLess(a, b);
}

// Returns: true if a is near equal to b, false otherwise.
inline bool epsilonEqual(float a, float b)
{
  return !epsilonLess(a, b) && !epsilonGreater(a, b);
}

#endif // __ALL_H__
