#ifndef __EVAPO_TRANSPIRATION_H__
#define __EVAPO_TRANSPIRATION_H__

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <charm++.h>
#pragma GCC diagnostic warning "-Wsign-compare"

// The evapo-transpiration module has a concept of snow layers and soil layers
// that is separate from the rest of the ADHydro simulation.  There can be up
// to three snow layers, and there are always four soil layers.  The interface
// includes some array variables that store values for snow and/or soil layers.
// Arrays of size three hold snow layers.  Arrays of size four hold soil
// layers.  Arrays of size seven hold both snow and soil layers.  In these
// arrays the first entry is the top layer followed by lower layers.  For snow
// layers if less than the maximum number of layers exist then the existing
// layers come first, top to bottom, followed by unused entries in the array
// for layers that do not exist.  For array variables that contain both snow
// and soil layers the existing snow layers come first, top to bottom, followed
// by unused entries in the array for snow layers that do not exist, followed
// by the soil layers, top to bottom.

// This struct holds the inout state variables that are simulated by the
// evapo-transpiration module.  The calling code must initialize these values
// before they are first used.  Then it should just pass the values produced by
// one call back in to the next call.
typedef struct
{
  float albOld;   // Snow albedo, unitless.
  float stc[7];   // Temperature in Kelvin of each snow and soil layer.
  float tah;      // Canopy air temperature in Kelvin.
  float eah;      // Canopy air water vapor pressure in Pascal.
  float fWet;     // Wetted or snowed fraction of canopy, unitless.
  float canLiq;   // Intercepted liquid water in canopy in millimeters of water
                  // equivalent.
  float canIce;   // Intercepted solid water in canopy in millimeters of water
                  // equivalent.
  float tv;       // Vegetation temperature in Kelvin.
  float tg;       // Ground temperature in Kelvin.
  int   iSnow;    // Actual number of snow layers.
  float zSnso[7]; // Layer bottom depth from snow surface in meters of each
                  // snow and soil layer.
  float snIce[3]; // Solid water in each snow layer in millimeters of water
                  // equivalent.
  float snLiq[3]; // Liquid water in each snow layer in millimeters of water
                  // equivalent.
  float lfMass;   // Leaf mass in grams per square meter.
  float rtMass;   // Fine root mass in grams per square meter.
  float stMass;   // Stem mass in grams per square meter.
  float wood;     // Wood mass including woody roots in grams per square meter.
  float stblCp;   // Stable carbon in deep soil in grams per square meter.
  float fastCp;   // Short lived carbon in shallow soil in grams per square
                  // meter.
  float lai;      // Leaf area index, unitless.
  float sai;      // Stem area index, unitless.
  float cm;       // Momentum drag coefficient.
  float ch;       // Sensible heat exchange coefficient.
  float tauss;    // Non-dimensional snow age.
  float deepRech; // Recharge in meters of water to or from the water table
                  // when deep.
  float rech;     // Recharge in meters of water to or from the water table
                  // when shallow.
} EvapoTranspirationStateStruct;

PUPbytes(EvapoTranspirationStateStruct);

// Call this once to initialize the evapo-transpiration module before calling
// any of the other functions.
//
// Parameters:
//
// directory - The directory from which to read the .TBL files.
bool evapoTranspirationInit(const char* directory);

// Calculate evapo-transpiration for a location with a soil surface.  The soil
// might be currently covered with surfacewater or snow, but it is not
// permanently covered with surfacewater or ice such as a waterbody or glacier.
//
// Parameters:
//
// lat                     - Latitude in radians.
// yearLen                 - Number of days in the current year, 365 or 366.
// julian                  - Julian day of year.  Time in days including
//                           fractional day since midnight at the beginning of
//                           January 1 of the current year.
// cosZ                    - Cosine of the solar zenith angle, 0.0 to 1.0.
// dt                      - Time step in seconds.
// dx                      - Horizontal grid scale in meters.  Pass square root
//                           of element area for non-square grid cells.
// dz8w                    - Thickness in meters of lowest atmosphere layer in
//                           forcing data.  The following other variables are
//                           values from the middle of the lowest atmosphere
//                           layer, i.e. half of this height: psfc, uu, vv, q2.
// shdFac                  - Fraction of land area shaded by vegetation, 0.0 to
//                           1.0.
// shdMax                  - Yearly maximum fraction of land area shaded by
//                           vegetation, 0.0 to 1.0.
// smceq                   - Equlibrium water content of each soil layer,
//                           unitless.
// sfcTmp                  - Air temperature in Kelvin at surface.
// sfcPrs                  - Air pressure in Pascal at surface.
// psfc                    - Air pressure in Pascal at middle of lowest
//                           atmosphere layer in forcing data.
// uu                      - Eastward wind speed in meters per second at middle
//                           of lowest atmosphere layer in forcing data.
// vv                      - Northward wind speed in meters per second at
//                           middle of lowest atmosphere layer in forcing data.
// q2                      - Water vapor mixing ratio at middle of lowest
//                           atmosphere layer in forcing data, unitless.
// qc                      - Liquid water mixing ratio in clouds, unitless.
// solDn                   - Downward short wave radiation in Watts per square
//                           meter at the top of the canopy.
// lwDn                    - Downward long wave radiation in Watts per square
//                           meter at the top of the canopy.
// prcp                    - Precipitation rate in millimeters of water per
//                           second at the top of the canopy.
// tBot                    - Boundary condition for soil temperature in Kelvin
//                           at the bottom of the lowest soil layer.
// plbh                    - Planetary boundary layer height in meters.
// sh2o                    - Liquid water content of each soil layer, unitless.
// smc                     - Total water content, liquid and solid, of each
//                           soil layer, unitless.
// zwt                     - Depth in meters to water table.
// wa                      - Water stored in aquifer in millimeters of water.
// wt                      - Water stored in aquifer and saturated soil in
//                           millimeters of water.
// wsLake                  - Water stored in lakes in millimeters of water.
// smcwtd                  - Water content between the bottom of the lowest
//                           soil layer and water table, unitless.
// evapoTranspirationState - State that is simulated by the evapo-transpiration
//                           module passed by reference.  Must be initialized
//                           before first use.  Then will be filled in with new
//                           values that should be passed back in to the next
//                           call.
bool evapoTranspirationSoil(int vegType, int soilType, double soilThickness, float lat, int yearLen, float julian, float cosZ, float dt, float dx, float dz8w,
                            float shdFac, float shdMax, float smceq[4], float sfcTmp, float sfcPrs, float psfc, float uu, float vv, float q2, float qc,
                            float solDn, float lwDn, float prcp, float tBot, float pblh, float sh2o[4], float smc[4], float zwt, float wa, float wt,
                            float wsLake, float smcwtd, EvapoTranspirationStateStruct* evapoTranspirationState);

// FIXME add functions for evapoTranspirationWater and evapoTranspirationGlacier

#endif // __EVAPO_TRANSPIRATION_H__
