#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

#include "region.h"
#include "file_manager.h"
#include "adhydro.decl.h"

// An ADHydro object is the main chare of the program.  Execution starts in its
// constructor.
class ADHydro : public CBase_ADHydro
{
  ADHydro_SDAG_CODE
  
public:
  
  // Global readonly variables.  For usage see comments in the example superfile.
  static std::string                                        evapoTranspirationInitMpTableFilePath;
  static std::string                                        evapoTranspirationInitVegParmFilePath;
  static std::string                                        evapoTranspirationInitSoilParmFilePath;
  static std::string                                        evapoTranspirationInitGenParmFilePath;
  static bool                                               initializeFromASCIIFiles; // Flag.
  static std::string                                        ASCIIInputMeshNodeFilePath;
  static std::string                                        ASCIIInputMeshZFilePath;
  static std::string                                        ASCIIInputMeshElementFilePath;
  static std::string                                        ASCIIInputMeshNeighborFilePath;
  static std::string                                        ASCIIInputMeshLandCoverFilePath;
  static std::string                                        ASCIIInputMeshSoilTypeFilePath;
  static std::string                                        ASCIIInputMeshGeolTypeFilePath;
  static std::string                                        ASCIIInputMeshEdgeFilePath;
  static std::string                                        ASCIIInputChannelNodeFilePath;
  static std::string                                        ASCIIInputChannelZFilePath;
  static std::string                                        ASCIIInputChannelElementFilePath;
  static std::string                                        ASCIIInputChannelPruneFilePath;
  static std::string                                        adhydroInputGeometryFilePath;
  static std::string                                        adhydroInputParameterFilePath;
  static std::string                                        adhydroInputStateFilePath;
  static std::string                                        adhydroInputForcingFilePath;
  static int                                                adhydroInputStateInstance;
  static std::string                                        adhydroOutputGeometryFilePath;
  static std::string                                        adhydroOutputParameterFilePath;
  static std::string                                        adhydroOutputStateFilePath;
  static std::string                                        adhydroOutputDisplayFilePath;
  static double                                             centralMeridian;    // Radians.
  static double                                             falseEasting;       // Meters.
  static double                                             falseNorthing;      // Meters.
  static double                                             referenceDate;      // Julian date.
  static double                                             currentTime;        // Seconds.
  static double                                             simulationDuration; // Seconds.
  static double                                             checkpointPeriod;   // Seconds.
  static double                                             outputPeriod;       // Seconds.
  static InfiltrationAndGroundwater::InfiltrationMethodEnum infiltrationMethod;
  static bool                                               drainDownMode;      // Flag.
  static double                                             drainDownTime;      // Seconds.
  static bool                                               doMeshMassage;      // Flag.
  static bool                                               zeroExpirationTime; // Flag.
  static bool                                               zeroCumulativeFlow; // Flag.
  static bool                                               zeroWaterError;     // Flag.
  static int                                                verbosityLevel;     // Unitless.
  static CProxy_FileManager                                 fileManagerProxy;
  static CProxy_Region                                      regionProxy;
  
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
  
  // The simulation runs more efficiently if multiple expiration times expire
  // simultaneously.  In order to increase the likelihood of that happening we
  // force expiration times to expire at certain discrete times.  To choose the
  // expiration time, first choose the number closest to but not over dtNew
  // from the following divisors of sixty.  Not all divisors of sixty are
  // present in the list because some pairs of divisors have a bad synodic
  // period.
  //
  // 1, 2, 3, 5, 10, 15, 30, 60
  //
  // If dtNew is one minute or more then use the same divisors as minutes
  // instead of seconds.  If dtNew is less than one second choose one second
  // divided by a power of two.  The expiration time must be a multiple of this
  // number.  Choose the expiration time as the latest multiple of this number
  // less than or equal to currentTime + dtNew.
  //
  // Examples:
  //
  // currentTime = 0.0, dtNew = 3.1, newExpirationTime = 3.0
  // currentTime = 0.0, dtNew = 4.3, newExpirationTime = 3.0
  // currentTime = 2.0, dtNew = 3.1, newExpirationTime = 3.0
  // currentTime = 2.0, dtNew = 4.3, newExpirationTime = 6.0
  // 
  // Returns: simulation time in seconds since Element::referenceDate for the
  //          new expiration time of a nominal flow rate.
  //
  // Parameters:
  //
  // currentTime - Current simulation time in seconds since
  //               Element::referenceDate.
  // dtNew       - Desired timestep duration in seconds.
  static double newExpirationTime(double currentTime, double dtNew);
  
  // Constructor.  This is the mainchare constructor where the program starts.
  //
  // Parameters:
  //
  // msg - Charm++ command line arguments message.
  ADHydro(CkArgMsg* msg);
};

#endif // __ADHYDRO_H__
