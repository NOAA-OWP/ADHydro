#ifndef __EVAPO_TRANSPIRATION_H__
#define __EVAPO_TRANSPIRATION_H__

#include "all.h"

// The evapo-transpiration module has a concept of snow layers and soil layers
// that is separate from the rest of the ADHydro simulation.  There can be up
// to three snow layers, and there are always four soil layers.  The interface
// includes some array variables that store values for snow and/or soil layers.
// Arrays of size three hold snow layers.  Arrays of size four hold soil
// layers.  Arrays of size seven hold both snow and soil layers.  For all of
// these arrays layers are ordered top to bottom.  For array variables that
// contain snow layers if less than the maximum number of layers exist then the
// existing layers are at the end of the array.  That is, unused array elements
// come first followed by existing layers top to bottom.  For array variables
// that contain both snow and soil layers unused array elements come first
// followed by existing snow layers top to bottom followed by soil layers top
// to bottom.
#define EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS (3)
#define EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS (4)
#define EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS (EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS + EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS)

// This struct holds input variables that come from forcing data.
typedef struct
{
  float dz8w;   // Thickness in meters of lowest atmosphere layer.  The following other variables are values from the middle of the lowest atmosphere layer,
                // i.e. half of this height: psfc, uu, vv, q2.
  float sfcTmp; // Air temperature in Kelvin at surface.
  float sfcPrs; // Air pressure in Pascal at surface.
  float psfc;   // Air pressure in Pascal at middle of lowest atmosphere layer.
  float uu;     // Eastward wind speed in meters per second at middle of lowest atmosphere layer.
  float vv;     // Northward wind speed in meters per second at middle of lowest atmosphere layer.
  float q2;     // Water vapor mixing ratio at middle of lowest atmosphere layer, unitless.
  float qc;     // Liquid water mixing ratio in clouds, unitless.
  float solDn;  // Downward short wave radiation in Watts per square meter at the top of the canopy.
  float lwDn;   // Downward long wave radiation in Watts per square meter at the top of the canopy.
  float prcp;   // Precipitation rate in millimeters of water per second at the top of the canopy.  Positive means water added to the element.  Must be
                // non-negative.  This is the opposite sign from precipitation variables in elements, but this is the sign that Noah-MP uses.
  float tBot;   // Soil temperature in Kelvin at bottom of lowest soil layer.
  float pblh;   // Planetary boundary layer height in meters.
} EvapoTranspirationForcingStruct;

#ifdef __CHARMC__
PUPbytes(EvapoTranspirationForcingStruct);
#endif // __CHARMC__

// This struct holds input variables that come from soil moisture.
typedef struct
{
  float smcEq[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS]; // Equlibrium water content of each soil layer, unitless.
  float sh2o[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];  // Liquid water content of each soil layer, unitless.
  float smc[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];   // Total water content, liquid and solid, of each soil layer, unitless.
  float zwt;                                              // Depth in meters to water table.  Specify depth as a positive number.
  float smcwtd;                                           // Water content between the bottom of the lowest soil layer and water table, unitless.
} EvapoTranspirationSoilMoistureStruct;

#ifdef __CHARMC__
PUPbytes(EvapoTranspirationSoilMoistureStruct);
#endif // __CHARMC__

// This struct holds input/output state variables that are simulated by the
// evapo-transpiration module.  The calling code must initialize these values
// before they are first used.  Then it should just pass the values produced by
// one call back in to the next call.
typedef struct
{
  float fIceOld[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
                  // Ice fraction of each snow layer at the beginning of the last timestep, unitless.
  float albOld;   // Snow albedo, unitless.
  float snEqvO;   // The value of snEqv at the beginning of the last timestep in millimeters of water equivalent.
  float stc[EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS];
                  // Temperature of each snow and soil layer in Kelvin.
  float tah;      // Canopy air temperature in Kelvin.
  float eah;      // Canopy air water vapor pressure in Pascal.
  float fWet;     // Wetted or snowed fraction of canopy, unitless.
  float canLiq;   // Intercepted liquid water in canopy in millimeters of water equivalent.
  float canIce;   // Intercepted solid water in canopy in millimeters of water equivalent.
  float tv;       // Vegetation temperature in Kelvin.
  float tg;       // Ground temperature in Kelvin.
  int   iSnow;    // Actual number of snow layers.
  float zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS];
                  // Layer bottom depth in meters from snow surface of each snow and soil layer.  Specify depths as negative numbers.
  float snowH;    // Total snow height in meters.
  float snEqv;    // Total quantity of snow in millimeters of water equivalent.
  float snIce[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
                  // Solid water in each snow layer in millimeters of water equivalent.
  float snLiq[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
                  // Liquid water in each snow layer in millimeters of water equivalent.
  float lfMass;   // Leaf mass in grams per square meter.
  float rtMass;   // Fine root mass in grams per square meter.
  float stMass;   // Stem mass in grams per square meter.
  float wood;     // Wood mass including woody roots in grams per square meter.
  float stblCp;   // Stable carbon in deep soil in grams per square meter.
  float fastCp;   // Short lived carbon in shallow soil in grams per square meter.
  float lai;      // Leaf area index, unitless.
  float sai;      // Stem area index, unitless.
  float cm;       // Momentum drag coefficient, unitless.
  float ch;       // Sensible heat exchange coefficient, unitless.
  float tauss;    // Non-dimensional snow age, unitless.
  float deepRech; // Recharge in meters of water to or from the water table when deep.
  float rech;     // Recharge in meters of water to or from the water table when shallow.
} EvapoTranspirationStateStruct;

#ifdef __CHARMC__
PUPbytes(EvapoTranspirationStateStruct);
#endif // __CHARMC__

// Call this once to initialize the evapo-transpiration module before calling
// any of the other functions.  It initializes globals so in a parallel
// application it must be called once from each shared memory domain.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// mpTableFile  - The path to the MPTABLE.TBL file.
// vegParmFile  - The path to the VEGPARM.TBL file.
// soilParmFile - The path to the SOILPARM.TBL file.
// genParmFile  - The path to the GENPARM.TBL file.
bool evapoTranspirationInit(const char* mpTableFile, const char* vegParmFile, const char* soilParmFile, const char* genParmFile);

// Returns: the total water in the evapo-transpiration domain in millimeters of
//          water.
//
// Parameters:
//
// evapoTranspirationState - a pointer to the state of the evapo-transpiration
//                           domain.
float evapoTranspirationTotalWaterInDomain(const EvapoTranspirationStateStruct* evapoTranspirationState);

// Calculate evapo-transpiration for a location with a soil surface.  The soil
// might be currently covered with surfacewater or snow, but it is not
// permanently covered with surfacewater or ice such as a waterbody or icemass.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// vegType  - Vegetation type from VEGPARM.TBL file.
// soilType - Soil type form SOILPARM.TBL file.
// lat      - Latitude in radians.
// yearLen  - Number of days in the current year, 365 or 366.
// julian   - Ordinal day of year.  Time in days including fractional day since
//            midnight at the beginning of January 1 of the current year.  E.g.
//            noon on Jan 1 is 0.5.  6:00 AM on Jan 2 is 1.25.
// cosZ     - Cosine of the solar zenith angle, 0.0 to 1.0.  1.0 means the sun
//            is directly overhead.  0.0 means the sun is at or below the
//            horizon.
// dt       - Time step in seconds.
// dx       - Horizontal grid scale in meters.  Pass square root of element
//            area for non-square elements.
// evapoTranspirationForcing
//          - input variables that come from forcing data passed by reference
//            but unmodified.
// evapoTranspirationSoilMoisture
//          - input variables that come from soil moisture passed by reference
//            but unmodified.
// evapoTranspirationState
//          - State that is simulated by the evapo-transpiration module passed
//            by reference.  Must be initialized before first use.  Then will
//            be filled in with new values that should be passed back in to the
//            next call.
// surfacewaterAdd
//          - Scalar passed by reference will be filled in with the total
//            amount of water in millimeters of water to add to the
//            surfacewater of the element being processed.  This includes rain
//            and snowmelt that reaches the ground surface.  This water should
//            be added in lieu of prcp * dt because some of that precipitation
//            might be intercepted by the canopy or snow layer.  Must be
//            non-negative.
// evaporationFromCanopy
//          - Scalar passed by reference will be filled in with the amount of
//            evaporation from the canopy in millimeters of water.  Positive
//            means water evaporated off of the canopy.  Negative means water
//            condensed on to the canopy.  This water has already been added to
//            or removed from canLiq and canIce in evapoTranspirationState.
//            The quantity is merely provided for mass balance calculations.
// evaporationFromSnow
//          - Scalar passed by reference will be filled in with the amount of
//            evaporation from the snow layer in millimeters of water.
//            Positive means water evaporated off of the snow.  Negative means
//            water condensed on to the snow.  This water has already been
//            added to or removed from snEqv in evapoTranspirationState.  The
//            quantity is merely provided for mass balance calculations.
// evaporationFromGround
//          - Scalar passed by reference will be filled in with the amount of
//            evaporation from the ground in millimeters of water.  Positive
//            means water evaporated off of the ground.  Negative means water
//            condensed on to the ground.  This water needs to be added to or
//            removed from the surfacewater or groundwater of the element being
//            processed.
// transpiration
//          - Scalar passed by reference will be filled in with the amount of
//            transpiration through plants in millimeters of water.  Positive
//            means water transpired off of the plants.  Must be non-negative.
//            This water needs to be removed from the groundwater or
//            surfacewater of the element being processed.
// waterError
//          - Scalar passed by reference will be filled in with the water error
//            in Millimeters of water.  Positive means water was created.
//            Negative means water was destroyed.
bool evapoTranspirationSoil(int vegType, int soilType, float lat, int yearLen, float julian, float cosZ, float dt, float dx,
                            EvapoTranspirationForcingStruct* evapoTranspirationForcing, EvapoTranspirationSoilMoistureStruct* evapoTranspirationSoilMoisture,
                            EvapoTranspirationStateStruct* evapoTranspirationState, float* surfacewaterAdd, float* evaporationFromCanopy,
                            float* evaporationFromSnow, float* evaporationFromGround, float* transpiration, float* waterError);

// Calculate evapo-transpiration for a location with a water surface.  This
// should be used for places permanently covered with surfacewater such as
// waterbodies, not mesh elements that temporarily have surfacewater.  The
// waterbody might be currently covered with snow, but it is not permanently
// covered with ice such as an icemass.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// lat     - Latitude in radians.
// yearLen - Number of days in the current year, 365 or 366.
// julian  - Ordinal day of year.  Time in days including fractional day since
//           midnight at the beginning of January 1 of the current year.  E.g.
//           noon on Jan 1 is 0.5.  6:00 AM on Jan 2 is 1.25.
// cosZ    - Cosine of the solar zenith angle, 0.0 to 1.0.  1.0 means the sun
//           is directly overhead.  0.0 means the sun is at or below the
//           horizon.
// dt      - Time step in seconds.
// dx      - Horizontal grid scale in meters.  Pass square root of element
//           area for non-square elements.
// wsLake  - Water stored in lakes in millimeters of water.
// evapoTranspirationForcing
//         - input variables that come from forcing data passed by reference
//           but unmodified.
// evapoTranspirationState
//         - State that is simulated by the evapo-transpiration module passed
//           by reference.  Must be initialized before first use.  Then will
//           be filled in with new values that should be passed back in to the
//           next call.
// surfacewaterAdd
//         - Scalar passed by reference will be filled in with the total
//           amount of water in millimeters of water to add to the
//           surfacewater of the element being processed.  This includes rain
//           and snowmelt that reaches the ground surface.  This water should
//           be added in lieu of prcp * dt because some of that precipitation
//           might be intercepted by the canopy or snow layer.  Must be
//           non-negative.
// evaporationFromSnow
//         - Scalar passed by reference will be filled in with the amount of
//           evaporation from the snow layer in millimeters of water.
//           Positive means water evaporated off of the snow.  Negative means
//           water condensed on to the snow.  This water has already been
//           added to or removed from snEqv in evapoTranspirationState.  The
//           quantity is merely provided for mass balance calculations.
// evaporationFromGround
//         - Scalar passed by reference will be filled in with the amount of
//           evaporation from the ground in millimeters of water.  Positive
//           means water evaporated off of the ground.  Negative means water
//           condensed on to the ground.  This water needs to be added to or
//           removed from the surfacewater or groundwater of the element being
//           processed.
// waterError
//         - Scalar passed by reference will be filled in with the water error
//           in Millimeters of water.  Positive means water was created.
//           Negative means water was destroyed.
bool evapoTranspirationWater(float lat, int yearLen, float julian, float cosZ, float dt, float dx, float wsLake,
                             EvapoTranspirationForcingStruct* evapoTranspirationForcing, EvapoTranspirationStateStruct* evapoTranspirationState,
                             float* surfacewaterAdd, float* evaporationFromSnow, float* evaporationFromGround, float* waterError);

// Calculate evapo-transpiration for a location with a glacier surface.  This
// should be used for places permanently covered with ice such as icemasses,
// not mesh elements or waterbodies that temporarily have snow.
//
// Returns: true if there is an error, false otherwise.
//
// Parameters:
//
// cosZ - Cosine of the solar zenith angle, 0.0 to 1.0.  1.0 means the sun
//        is directly overhead.  0.0 means the sun is at or below the
//        horizon.
// dt   - Time step in seconds.
// evapoTranspirationForcing
//      - input variables that come from forcing data passed by reference
//        but unmodified.
// evapoTranspirationState
//      - State that is simulated by the evapo-transpiration module passed
//        by reference.  Must be initialized before first use.  Then will
//        be filled in with new values that should be passed back in to the
//        next call.
// surfacewaterAdd
//      - Scalar passed by reference will be filled in with the total
//        amount of water in millimeters of water to add to the
//        surfacewater of the element being processed.  This includes rain
//        and snowmelt that reaches the ground surface.  This water should
//        be added in lieu of prcp * dt because some of that precipitation
//        might be intercepted by the canopy or snow layer.  Must be
//        non-negative.
// evaporationFromSnow
//      - Scalar passed by reference will be filled in with the amount of
//        evaporation from the snow layer in millimeters of water.
//        Positive means water evaporated off of the snow.  Negative means
//        water condensed on to the snow.  This water has already been
//        added to or removed from snEqv in evapoTranspirationState.  The
//        quantity is merely provided for mass balance calculations.
// evaporationFromGround
//      - Scalar passed by reference will be filled in with the amount of
//        evaporation from the ground in millimeters of water.  Positive
//        means water evaporated off of the ground.  Negative means water
//        condensed on to the ground.  This water needs to be added to or
//        removed from the surfacewater or groundwater of the element being
//        processed.
// waterError
//      - Scalar passed by reference will be filled in with the water error
//        in Millimeters of water.  Positive means water was created.
//        Negative means water was destroyed.
bool evapoTranspirationGlacier(float cosZ, float dt, EvapoTranspirationForcingStruct* evapoTranspirationForcing,
                               EvapoTranspirationStateStruct* evapoTranspirationState, float* surfacewaterAdd, float* evaporationFromSnow,
                               float* evaporationFromGround, float* waterError);

// Check invariant conditions on an EvapoTranspirationForcingStruct.
//
// Returns: true if the invariant is violated, false otherwise.
//
// Parameters:
//
// evapoTranspirationForcing - Struct to check passed by reference.
bool checkEvapoTranspirationForcingStructInvariant(const EvapoTranspirationForcingStruct* evapoTranspirationForcing);

// Check invariant conditions on an EvapoTranspirationSoilMoistureStruct.
//
// Returns: true if the invariant is violated, false otherwise.
//
// Parameters:
//
// evapoTranspirationSoilMoisture - Struct to check passed by reference.
// porosity                       - Porosity is not in the soil moisture
//                                  struct, but several of the members must not
//                                  exceed porosity.  If you do not know the
//                                  porosity you can still check for internal
//                                  consistency by taking the default that the
//                                  porosity must not exceed one.
bool checkEvapoTranspirationSoilMoistureStructInvariant(const EvapoTranspirationSoilMoistureStruct* evapoTranspirationSoilMoisture, float porosity = 1.0);

// Check invariant conditions on an EvapoTranspirationStateStruct.
//
// Returns: true if the invariant is violated, false otherwise.
//
// Parameters:
//
// evapoTranspirationState - Struct to check passed by reference.
bool checkEvapoTranspirationStateStructInvariant(const EvapoTranspirationStateStruct* evapoTranspirationState);

#endif // __EVAPO_TRANSPIRATION_H__
