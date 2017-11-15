#include "evapo_transpiration.h"
#include "adhydro.h"
#include <math.h>

#ifdef INTEL_COMPILER
#define READ_MP_VEG_PARAMETERS noahmp_veg_parameters_mp_read_mp_veg_parameters_
#define SOIL_VEG_GEN_PARM      module_sf_noahmpdrv_mp_soil_veg_gen_parm_
#define NOAHMP_OPTIONS         noahmp_routines_mp_noahmp_options_
#define NOAHMP_OPTIONS_GLACIER noahmp_glacier_routines_mp_noahmp_options_glacier_
#define REDPRM                 noahmp_routines_mp_redprm_
#define NOAHMP_SFLX            noahmp_routines_mp_noahmp_sflx_
#define NOAHMP_GLACIER         noahmp_glacier_routines_mp_noahmp_glacier_
#define NOAHMP_POROSITY        noahmp_globals_mp_smcmax_
#else // !INTEL_COMPILER
#define READ_MP_VEG_PARAMETERS __noahmp_veg_parameters_MOD_read_mp_veg_parameters
#define SOIL_VEG_GEN_PARM      __module_sf_noahmpdrv_MOD_soil_veg_gen_parm
#define NOAHMP_OPTIONS         __noahmp_routines_MOD_noahmp_options
#define NOAHMP_OPTIONS_GLACIER __noahmp_glacier_routines_MOD_noahmp_options_glacier
#define REDPRM                 __noahmp_routines_MOD_redprm
#define NOAHMP_SFLX            __noahmp_routines_MOD_noahmp_sflx
#define NOAHMP_GLACIER         __noahmp_glacier_routines_MOD_noahmp_glacier
#define NOAHMP_POROSITY        __noahmp_globals_MOD_smcmax
#endif // INTEL_COMPILER

#define NOAHMP_TFRZ (273.16f) // TFRZ from module_sf_noahmplsm.F

extern "C" void READ_MP_VEG_PARAMETERS(const char* landUse, const char* mpTableFile, size_t landUseSize, size_t mpTableFileSize);
extern "C" void SOIL_VEG_GEN_PARM(const char* landUse, const char* soil, const char* vegParmFile, const char* soilParmFile, const char* genParmFile,
                                  int* verbosityLevel, size_t landUseSize, size_t soilSize, size_t vegParmFileSize, size_t soilParmFileSize,
                                  size_t genParmFileSize);
extern "C" void NOAHMP_OPTIONS(int* dveg, int* optCrs, int* optBtr, int* optRun, int* optSfc, int* optFrz, int* optInf, int* optRad, int* optAlb, int* optSnf,
                               int* optTbot, int* optStc);
extern "C" void NOAHMP_OPTIONS_GLACIER(int* dveg, int* optCrs, int* optBtr, int* optRun, int* optSfc, int* optFrz, int* optInf, int* optRad, int* optAlb,
                                       int* optSnf, int* optTbot, int* optStc);
extern "C" void REDPRM(int* vegType, int* soilType, int* slopeType, float* zSoil, int* nSoil, int* isUrban);
extern "C" void NOAHMP_SFLX(int* iLoc, int* jLoc, float* lat, int* yearLen, float* julian, float* cosZ, float* dt, float* dx, float* dz8w, int* nSoil,
                            float* zSoil, int* nSnow, float* shdFac, float* shdMax, int* vegType, int* isUrban, int* ice, int* ist, int* isc, float* smcEq,
                            int* iz0tlnd, float* sfcTmp, float* sfcPrs, float* psfc, float* uu, float* vv, float* q2, float* qc, float* solDn, float* lwDn,
                            float* prcp, float* tBot, float* co2Air, float* o2Air, float* folN, float* fIceOld, float* pblh, float* zLvl, float* albOld,
                            float* snEqvO, float* stc, float* sh2o, float* smc, float* tah, float* eah, float* fWet, float* canLiq, float* canIce, float* tv,
                            float* tg, float* qsfc, float* qSnow, int* iSnow, float* zSnso, float* snowH, float* snEqv, float* snIce, float* snLiq, float* zwt,
                            float* wa, float* wt, float* wsLake, float* lfMass, float* rtMass, float* stMass, float* wood, float* stblCp, float* fastCp,
                            float* lai, float* sai, float* cm, float* ch, float* tauss, float* smcwtd, float* deepRech, float* rech, float* fsa, float* fsr,
                            float* fira, float* fsh, float* sSoil, float* fcev, float* fgev, float* fctr, float* ecan, float* etran, float* eDir, float* trad,
                            float* tgb, float* tgv, float* t2mv, float* t2mb, float* q2v, float* q2b, float* runSrf, float* runSub, float* apar, float* psn,
                            float* sav, float* sag, float* fSno, float* nee, float* gpp, float* npp, float* fVeg, float* albedo, float* qsnBot, float* ponding,
                            float* ponding1, float* ponding2, float* rsSun, float* rsSha, float* bGap, float* wGap, float* chv, float* chb, float* emissi,
                            float* shg, float* shc, float* shb, float* evg, float* evb, float* ghv, float* ghb, float* irg, float* irc, float* irb, float* tr,
                            float* evc, float* chLeaf, float* chuc, float* chv2, float* chb2, float* fpIce, float* qRain);
extern "C" void NOAHMP_GLACIER(int* iLoc, int* jLoc, float* cosZ, int* nSnow, int* nSoil, float* dt, float* sfcTmp, float* sfcPrs, float* uu, float* vv,
                               float* q2, float* solDn, float* prcp, float* lwDn, float* tBot, float* zLvl, float* fIceOld, float* zSoil, float* qSnow,
                               float* snEqvO, float* albOld, float* cm, float* ch, int* iSnow, float* snEqv, float* smc, float* zSnso, float* snowH,
                               float* snIce, float* snLiq, float* tg, float* stc, float* sh2o, float* tauss, float* qsfc, float* fsa, float* fsr, float* fira,
                               float* fsh, float* fgev, float* sSoil, float* trad, float* eDir, float* runSrf, float* runSub, float* sag, float* albedo,
                               float* qsnBot, float* ponding, float* ponding1, float* ponding2, float* t2m, float* q2e, float* emissi, float* fpIce,
                               float* ch2b);
extern float NOAHMP_POROSITY;

bool evapoTranspirationInit(const char* mpTableFile, const char* vegParmFile, const char* soilParmFile, const char* genParmFile)
{
  bool        error          = false;                   // Error flag.
  const char* landUse        = "USGS";                  // Land use data set.
  const char* soil           = "STAS";                  // Soil type data set.
  int         verbosityLevel = ADHydro::verbosityLevel; // For passing verbosity level into Noah-MP.  I'm not comfortable passing a class static variable by
                                                        // reference into foreign code.
  
  // =====================================options for different schemes================================
  // From module_sf_noahmplsm.F
  
  // options for dynamic vegetation:
  // 1 -> off (use table LAI; use FVEG = SHDFAC from input)
  // 2 -> on (together with OPT_CRS = 1)
  // 3 -> off (use table LAI; calculate FVEG)
  // 4 -> off (use table LAI; use maximum vegetation fraction)

  int dveg = 3;

  // options for canopy stomatal resistance
  // 1-> Ball-Berry; 2->Jarvis

  int optCrs = 1;

  // options for soil moisture factor for stomatal resistance
  // 1-> Noah (soil moisture)
  // 2-> CLM  (matric potential)
  // 3-> SSiB (matric potential)

  int optBtr = 1;

  // options for runoff and groundwater
  // 1 -> TOPMODEL with groundwater (Niu et al. 2007 JGR) ;
  // 2 -> TOPMODEL with an equilibrium water table (Niu et al. 2005 JGR) ;
  // 3 -> original surface and subsurface runoff (free drainage)
  // 4 -> BATS surface and subsurface runoff (free drainage)
  // 5 -> Miguez-Macho&Fan groundwater scheme (Miguez-Macho et al. 2007 JGR, lateral flow: Fan et al. 2007 JGR)

  int optRun = 1;

  // options for surface layer drag coeff (CH & CM)
  // 1->M-O ; 2->original Noah (Chen97); 3->MYJ consistent; 4->YSU consistent.

  int optSfc = 1;

  // options for supercooled liquid water (or ice fraction)
  // 1-> no iteration (Niu and Yang, 2006 JHM); 2: Koren's iteration

  int optFrz = 1;

  // options for frozen soil permeability
  // 1 -> linear effects, more permeable (Niu and Yang, 2006, JHM)
  // 2 -> nonlinear effects, less permeable (old)

  int optInf = 1;

  // options for radiation transfer
  // 1 -> modified two-stream (gap = F(solar angle, 3D structure ...)<1-FVEG)
  // 2 -> two-stream applied to grid-cell (gap = 0)
  // 3 -> two-stream applied to vegetated fraction (gap=1-FVEG)

  int optRad = 1;

  // options for ground snow surface albedo
  // 1-> BATS; 2 -> CLASS

  int optAlb = 2;

  // options for partitioning  precipitation into rainfall & snowfall
  // 1 -> Jordan (1991); 2 -> BATS: when SFCTMP<TFRZ+2.2 ; 3-> SFCTMP<TFRZ

  int optSnf = 1;

  // options for lower boundary condition of soil temperature
  // 1 -> zero heat flux from bottom (ZBOT and TBOT not used)
  // 2 -> TBOT at ZBOT (8m) read from a file (original Noah)

  int optTbot = 2;

  // options for snow/soil temperature time scheme (only layer 1)
  // 1 -> semi-implicit; 2 -> full implicit (original Noah)

  int optStc  = 1;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != mpTableFile))
    {
      CkError("ERROR in evapoTranspirationInit: mpTableFile must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != vegParmFile))
    {
      CkError("ERROR in evapoTranspirationInit: vegParmFile must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != soilParmFile))
    {
      CkError("ERROR in evapoTranspirationInit: soilParmFile must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != genParmFile))
    {
      CkError("ERROR in evapoTranspirationInit: genParmFile must not be NULL.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      READ_MP_VEG_PARAMETERS(landUse, mpTableFile, strlen(landUse), strlen(mpTableFile));
      SOIL_VEG_GEN_PARM(landUse, soil, vegParmFile, soilParmFile, genParmFile, &verbosityLevel, strlen(landUse), strlen(soil), strlen(vegParmFile),
                        strlen(soilParmFile), strlen(genParmFile));
      NOAHMP_OPTIONS(&dveg, &optCrs, &optBtr, &optRun, &optSfc, &optFrz, &optInf, &optRad, &optAlb, &optSnf, &optTbot, &optStc);
      NOAHMP_OPTIONS_GLACIER(&dveg, &optCrs, &optBtr, &optRun, &optSfc, &optFrz, &optInf, &optRad, &optAlb, &optSnf, &optTbot, &optStc);
    }
  
  return error;
}

float evapoTranspirationTotalWaterInDomain(EvapoTranspirationStateStruct* evapoTranspirationState)
{
  return evapoTranspirationState->canLiq + evapoTranspirationState->canIce + evapoTranspirationState->snEqv;
}

bool evapoTranspirationSoil(int vegType, int soilType, float lat, int yearLen, float julian, float cosZ, float dt, float dx,
                            EvapoTranspirationForcingStruct* evapoTranspirationForcing, EvapoTranspirationSoilMoistureStruct* evapoTranspirationSoilMoisture,
                            EvapoTranspirationStateStruct* evapoTranspirationState, float* surfacewaterAdd, float* evaporationFromCanopy,
                            float* evaporationFromSnow, float* evaporationFromGround, float* transpiration, float* waterError)
{
  bool  error = false; // Error flag.
  int   ii;            // Loop counter.
  
  // Input parameters for redprm function.  Some are also used for sflx function.
  int   slopeType = 8;                                         // I just arbitrarily chose a slope type with zero slope.  I think slope is only used to
                                                               // calculate runoff, which we ignore.
  float zSoil[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];      // Layer bottom depth in meters from soil surface of each soil layer.  Values are set below from
                                                               // zSnso.
  int   nSoil     = EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; // Always use four soil layers.
  int   isUrban   = 1;                                         // USGS vegetation type for urban land.
  
  // Input parameters to sflx function.
  int   iLoc    = 1;                                           // Grid location index, unused.
  int   jLoc    = 1;                                           // Grid location index, unused.
  int   nSnow   = EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS;   // Maximum number of snow layers.  Always pass 3.
  float shdFac  = 0.0f;                                        // Fraction of land area shaded by vegetation, 0.0 to 1.0.  Since we use dveg = 3 shdFac is
                                                               // unused.  Always pass 0.0.
  float shdMax  = 0.0f;                                        // Yearly maximum fraction of land area shaded by vegetation, 0.0 to 1.0.  Since we use dveg = 3
                                                               // shdMax is unused.  Always pass 0.0.
  int   ice     = 0;                                           // Flag to indicate permanent ice cover, 0 for no, 1 for yes.  Always pass 0.  If permanent ice
                                                               // cover call evapoTranspirationGlacier instead.
  int   ist     = 1;                                           // Flag to indicate permanent water cover, 1 for soil, 2 for lake.  Always pass 1.  If permanent
                                                               // water cover call evapoTranspirationWater instead.
  int   isc     = 4;                                           // Soil color type, 1 for lightest to 8 for darkest.  Always pass 4 unless we find a data source
                                                               // for soil color.
  int   iz0tlnd = 0;                                           // Unused.
  float co2Air  = 0.0004f * evapoTranspirationForcing->sfcPrs; // CO2 partial pressure in Pascal at surface.  Always pass 400 parts per million of surface
                                                               // pressure.
  float o2Air   = 0.21f * evapoTranspirationForcing->sfcPrs;   // O2 partial pressure in Pascal at surface.  Always pass 21 percent of surface pressure.
  float folN    = 3.0f;                                        // Foliage nitrogen percentage, 0.0 to 100.0.  Always pass 3.0 unless we find a data source for
                                                               // foliage nitrogen.
  float fIce[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];       // Frozen fraction of each snow layer, unitless.  Values are set below from snIce and snLiq.
  float zLvl    = evapoTranspirationForcing->dz8w;             // Thickness in meters of lowest atmosphere layer in forcing data.  Redundant with dz8w.
  
  // Input/output parameters to sflx function.
  float qsfc   = evapoTranspirationForcing->q2; // Water vapor mixing ratio at middle of lowest atmosphere layer in forcing data, unitless.  Redundant with q2.
  float qSnow  = NAN;                           // This is actually an output only variable.  Snowfall rate below the canopy in millimeters of water equivalent
                                                // per second.
  float wa;                                     // Water stored in aquifer in millimeters of water.  Value is set below from zwt.
  float wt;                                     // Water stored in aquifer and saturated soil in millimeters of water.  Value is set below from zwt, zSoil, and
                                                // NOAHMP_POROSITY.
  float wsLake = 0.0f;                          // Water stored in lakes in millimeters of water.  Because we separate waterbodies from the mesh, locations
                                                // with a soil surface have no included lake storage.
  
  // Output parameters to sflx function.  Set to NAN so we can detect if the values are used before being set.
  float fsa      = NAN; // Unused.
  float fsr      = NAN; // Unused.
  float fira     = NAN; // Unused.
  float fsh      = NAN; // Unused.
  float sSoil    = NAN; // Unused.
  float fcev     = NAN; // Unused.
  float fgev     = NAN; // Unused.
  float fctr     = NAN; // Unused.
  float eCan     = NAN; // Evaporation rate from canopy in millimeters of water equivalent per second.
  float eTran    = NAN; // Evaporation rate from transpiration in millimeters of water equivalent per second.
  float eDir     = NAN; // Evaporation rate from surface in millimeters of water equivalent per second.
  float tRad     = NAN; // Unused.
  float tgb      = NAN; // Unused.
  float tgv      = NAN; // Unused.
  float t2mv     = NAN; // Unused.
  float t2mb     = NAN; // Unused.
  float q2v      = NAN; // Unused.
  float q2b      = NAN; // Unused.
  float runSrf   = NAN; // Infiltration excess runoff in millimeters of water per second.
  float runSub   = NAN; // Saturation excess runoff in millimeters of water per second.
  float apar     = NAN; // Unused.
  float psn      = NAN; // Unused.
  float sav      = NAN; // Unused.
  float sag      = NAN; // Unused.
  float fSno     = NAN; // Unused.
  float nee      = NAN; // Unused.
  float gpp      = NAN; // Unused.
  float npp      = NAN; // Unused.
  float fVeg     = NAN; // Unused.
  float albedo   = NAN; // Unused.
  
  // There is some complexity with qSnBot and the ponding variables.  The big picture is that they contain the flow out the bottom of the snowpack including
  // snowmelt plus ranfall that lands on the snowpack and passes through without freezing.  In short, the quantity of water that flowed out the bottom of the
  // snowpack is always equal to qSnBot * dt + ponding + ponding1 + ponding2.  However, this flow is put into one of the four different variables depending on
  // the situation.  If the multi-layer snow simulation is turned on at the end of the timestep then the snowmelt rate is put into qSnBot and the ponding
  // variables are set to zero.  If the multi-layer snow simulation is turned off at the end of the timestep then the snowmelt quantity is put into one of the
  // ponding variables and the other ponding variables and qSnBot are set to zero.  Which ponding variable gets the water also depends on the situation. If the
  // multi-layer snow simulation is turned off at the beginning and end of the timestep then the water is put in ponding.  If the multi-layer snow simulation
  // is turned on at the beginning of the timestep, but is off at the end of the timestep then the water is put either in ponding1 or ponding2 depending on the
  // reason why the multi-layer snow simulation was turned off.  If in an individual layer in the multi-layer snow simulation has snIce go below 0.1 mm of snow
  // water equivalent that layer is eliminated.  The water is added to a neighboring layer if possible, but if that was the last layer then the multi-layer
  // snow simulation is turned off, snEqv is set to snIce, and the water in snLiq is put in ponding1.  If the height of the entire snow pack, snowH, goes below
  // 0.025 m then the multi-layer snow simulation is turned off, snEqv is set to snIce, and the water in snLiq is put in ponding2.
  float qSnBot   = NAN; // Flow rate out the bottom of the snowpack in millimeters of water per second.
  float ponding  = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  float ponding1 = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  float ponding2 = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  
  float rsSun    = NAN; // Unused.
  float rsSha    = NAN; // Unused.
  float bGap     = NAN; // Unused.
  float wGap     = NAN; // Unused.
  float chv      = NAN; // Unused.
  float chb      = NAN; // Unused.
  float emissi   = NAN; // Unused.
  float shg      = NAN; // Unused.
  float shc      = NAN; // Unused.
  float shb      = NAN; // Unused.
  float evg      = NAN; // Unused.
  float evb      = NAN; // Unused.
  float ghv      = NAN; // Unused.
  float ghb      = NAN; // Unused.
  float irg      = NAN; // Unused.
  float irc      = NAN; // Unused.
  float irb      = NAN; // Unused.
  float tr       = NAN; // Unused.
  float evc      = NAN; // Unused.
  float chLeaf   = NAN; // Unused.
  float chuc     = NAN; // Unused.
  float chv2     = NAN; // Unused.
  float chb2     = NAN; // Unused.
  float fpIce    = NAN; // Fraction of precipitation that is frozen, unitless.
  float qRain    = NAN; // Rainfall rate below the canopy in millimeters of water per second.
  
  // Derived output variables.
  float canIceOriginal;              // Quantity of canopy ice before timestep in millimeters of water equivalent.
  float canLiqOriginal;              // Quantity of canopy liquid before timestep in millimeters of water.
  float canWaterShouldBe;            // There are two situations where water can be erroneously created or destroyed in the canopy.  We don't want this
                                     // behavior so in this variable we calculate what (canIce + canLiq) should be and set it back.  If those situations do not
                                     // occur this instead performs a mass balance check.
  int   iSnowOriginal;               // Actual number of snow layers before timestep.
  float evaporationFromSurface;      // Quantity of evaporation from the surface in millimeters of water equivalent.  Positive means water evaporated off of
                                     // the surface.  Negative means water condensed on to the surface.  Surface evaporation sometimes comes from snow and is
                                     // taken out by Noah-MP, but if there is not enough snow we have to take the evaporation from the water in the ADHydro
                                     // state variables.
  float snowfallAboveCanopy;         // Quantity of snowfall above the canopy in millimeters of water equivalent.  Must be non-negative.
  float snowfallInterceptedByCanopy; // Quantity of snowfall intercepted by the canopy in millimeters of water equivalent.  Can be negative if snow is falling
                                     // off of the canopy.
  float snowfallBelowCanopy;         // Quantity of snowfall below the canopy in millimeters of water equivalent.  Must be non-negative.
  float snowmeltOnGround;            // Quantity of water that reaches the ground from the snow layer in millimeters of water.  Must be non-negative.
  float rainfallAboveCanopy;         // Quantity of rainfall above the canopy in millimeters of water.  Must be non-negative.
  float rainfallInterceptedByCanopy; // Quantity of rainfall intercepted by the canopy in millimeters of water.  Can be negative if rain is dripping from the
                                     // canopy.
  float rainfallBelowCanopy;         // Quantity of rainfall below the canopy in millimeters of water.  Must be non-negative.
  float rainfallInterceptedBySnow;   // Quantity of rainfall intercepted by the snow layer in millimeters of water.  Must be non-negative.
  float rainfallOnGround;            // Quantity of rainfall that reaches the ground in millimeters of water.  Must be non-negative.
  float snEqvOriginal;               // Quantity of water in the snow layer(s) before timestep in millimeters of water equivalent.  snEqvO is not always set to
                                     // this value after the timestep.
  float snEqvShouldBe;               // If snEqv falls below 0.001 Noah-MP sets it to zero, or if it rises above 2000.0 Noah-MP sets it to 2000.0.  We don't
                                     // want this behavior so in this variable we calculate what snEqv should be and set it back.  If snEqv is not changed this
                                     // instead performs a mass balance check.
  float runoff;                      // Quantity of water that runs off from the soil in millimeters of water.  We don't use Noah-MP's infiltration and
                                     // groundwater simulation so this is merely for a mass balance check.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Variables used for assertions.
  int   iLocOriginal         = iLoc;
  int   jLocOriginal         = jLoc;
  float latOriginal          = lat;
  int   yearLenOriginal      = yearLen;
  float julianOriginal       = julian;
  float cosZOriginal         = cosZ;
  float dtOriginal           = dt;
  float dxOriginal           = dx;
  float dz8wOriginal         = evapoTranspirationForcing->dz8w;
  int   nSoilOriginal        = nSoil;
  float zSoilOriginal[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];
  int   nSnowOriginal        = nSnow;
  float shdFacOriginal       = shdFac;
  float shdMaxOriginal       = shdMax;
  int   vegTypeOriginal      = vegType;
  int   isUrbanOriginal      = isUrban;
  int   iceOriginal          = ice;
  int   istOriginal          = ist;
  int   iscOriginal          = isc;
  float smcEqOriginal[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];
  int   iz0tlndOriginal      = iz0tlnd;
  float sfcTmpOriginal       = evapoTranspirationForcing->sfcTmp;
  float sfcPrsOriginal       = evapoTranspirationForcing->sfcPrs;
  float psfcOriginal         = evapoTranspirationForcing->psfc;
  float uuOriginal           = evapoTranspirationForcing->uu;
  float vvOriginal           = evapoTranspirationForcing->vv;
  float q2Original           = evapoTranspirationForcing->q2;
  float qcOriginal           = evapoTranspirationForcing->qc;
  float solDnOriginal        = evapoTranspirationForcing->solDn;
  float lwDnOriginal         = evapoTranspirationForcing->lwDn;
  float prcpOriginal         = evapoTranspirationForcing->prcp;
  float tBotOriginal         = evapoTranspirationForcing->tBot;
  float co2AirOriginal       = co2Air;
  float o2AirOriginal        = o2Air;
  float folNOriginal         = folN;
  float fIceOldOriginal[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
  float pblhOriginal         = evapoTranspirationForcing->pblh;
  float zLvlOriginal         = zLvl;
  float soilMoistureOriginal = 0.0f; // Total soil moisture before timestep in millimeters of water.
  float soilMoistureNew      = 0.0f; // Total soil moisture after timestep in millimeters of water.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(1 <= vegType && 27 >= vegType))
    {
      CkError("ERROR in evapoTranspirationSoil: USGS vegetation type must be greater than or equal to 1 and less than or equal to 27.\n");
      error = true;
    }

  if (16 == vegType)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in evapoTranspirationSoil: vegetation type is 'Water Bodies'.  Call evapoTranspirationWater instead.\n");
        }
    }

  if (24 == vegType)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in evapoTranspirationSoil: vegetation type is 'Snow or Ice'.  Call evapoTranspirationGlacier instead.\n");
        }
    }

  if (!(1 <= soilType && 19 >= soilType))
    {
      CkError("ERROR in evapoTranspirationSoil: STAS soil type must be greater than or equal to 1 and less than or equal to 19.\n");
      error = true;
    }

  if (14 == soilType)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in evapoTranspirationSoil: soil type is 'WATER'.  Call evapoTranspirationWater instead.\n");
        }
    }

  if (16 == soilType)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in evapoTranspirationSoil: soil type is 'OTHER(land-ice)'.  Call evapoTranspirationGlacier instead.\n");
        }
    }
  
  if (!(-M_PI / 2.0f <= lat && M_PI / 2.0f >= lat))
    {
      CkError("ERROR in evapoTranspirationSoil: lat must be greater than or equal to -pi/2 and less than or equal to pi/2.\n");
      error = true;
    }
  
  if (!(365 == yearLen || 366 == yearLen))
    {
      CkError("ERROR in evapoTranspirationSoil: yearLen must be 365 or 366.\n");
      error = true;
    }
  
  if (!(0.0f <= julian && julian <= yearLen))
    {
      CkError("ERROR in evapoTranspirationSoil: julian must be greater than or equal to zero and less than or equal to yearLen.\n");
      error = true;
    }
  
  if (!(0.0f <= cosZ && 1.0f >= cosZ))
    {
      CkError("ERROR in evapoTranspirationSoil: cosZ must be greater than or equal to zero and less than or equal to one.\n");
      error = true;
    }
  
  if (!(0.0f < dt))
    {
      CkError("ERROR in evapoTranspirationSoil: dt must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0f < dx))
    {
      CkError("ERROR in evapoTranspirationSoil: dx must be greater than zero.\n");
      error = true;
    }
  
  if (!(NULL != evapoTranspirationForcing))
    {
      CkError("ERROR in evapoTranspirationSoil: evapoTranspirationForcing must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != evapoTranspirationSoilMoisture))
    {
      CkError("ERROR in evapoTranspirationSoil: evapoTranspirationSoilMoisture must not be NULL.\n");
      error = true;
    }
  else
    {
      // Values of smcEq, sh2o, smc, and smcwtd will be error checked later.  They need to be compared against NOAHMP_POROSITY, which is not set until after we
      // call REDPRM.

      if (!(0.0f <= evapoTranspirationSoilMoisture->zwt))
        {
          CkError("ERROR in evapoTranspirationSoil: zwt must be greater than or equal to zero.\n");
          error = true;
        }
    }
  
  if (!(NULL != evapoTranspirationState))
    {
      CkError("ERROR in evapoTranspirationSoil: evapoTranspirationState must not be NULL.\n");
      error = true;
    }

  if (!(NULL != surfacewaterAdd))
    {
      CkError("ERROR in evapoTranspirationSoil: surfacewaterAdd must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *surfacewaterAdd = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evaporationFromCanopy))
    {
      CkError("ERROR in evapoTranspirationSoil: evaporationFromCanopy must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *evaporationFromCanopy = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evaporationFromSnow))
    {
      CkError("ERROR in evapoTranspirationSoil: evaporationFromSnow must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *evaporationFromSnow = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evaporationFromGround))
    {
      CkError("ERROR in evapoTranspirationSoil: evaporationFromGround must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *evaporationFromGround = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != transpiration))
    {
      CkError("ERROR in evapoTranspirationSoil: transpiration must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *transpiration = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != waterError))
    {
      CkError("ERROR in evapoTranspirationSoil: waterError must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *waterError = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (!error)
    {
      error = checkEvapoTranspirationForcingStructInvariant(evapoTranspirationForcing);
    }
  
  if (!error)
    {
      error = checkEvapoTranspirationStateStructInvariant(evapoTranspirationState);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  if (!error)
    {
      // Set variables that require error checking of pointers first.
      canIceOriginal = evapoTranspirationState->canIce;
      canLiqOriginal = evapoTranspirationState->canLiq;
      iSnowOriginal  = evapoTranspirationState->iSnow;
      snEqvOriginal  = evapoTranspirationState->snEqv;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          smcEqOriginal[ii] = evapoTranspirationSoilMoisture->smcEq[ii];
        }
      
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          fIceOldOriginal[ii] = evapoTranspirationState->fIceOld[ii];
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Set zSoil from zSnso.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          if (0 > evapoTranspirationState->iSnow)
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] -
                          evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1];
            }
          else
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
            }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          zSoilOriginal[ii] = zSoil[ii];
          
          // Calculate soil moisture at the beginning of the timestep.
          if (0 == ii)
            {
              soilMoistureOriginal += evapoTranspirationSoilMoisture->smc[ii] * -zSoil[ii] * 1000.0f; // * 1000.0f because zSoil is in meters and
                                                                                                      // soilMoistureOriginal is in millimeters.
            }
          else
            {
              soilMoistureOriginal += evapoTranspirationSoilMoisture->smc[ii] * (zSoil[ii - 1] - zSoil[ii]) * 1000.0f;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Calculate fIce at the beginning of the timestep.
      for (ii = evapoTranspirationState->iSnow + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          fIce[ii] = evapoTranspirationState->snIce[ii] / (evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]);
        }
      
      // Set Noah-MP globals.
      REDPRM(&vegType, &soilType, &slopeType, zSoil, &nSoil, &isUrban);
      
      // Calculate water in the aquifer and saturated soil.
      if (evapoTranspirationSoilMoisture->zwt > -zSoil[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS - 1])
        {
          // If the water table is deeper than the bottom of the lowest soil layer then the amount of water in the aquifer is the total capacity of the
          // aquifer, 5000 mm, minus the dry part, which is the distance of the water table below the bottom of the lowest soil layer times 1000 to convert
          // from meters to millimeters times the specific yield of 0.2.
          wa = 5000.0f - (evapoTranspirationSoilMoisture->zwt + zSoil[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS - 1]) * 1000.0f * 0.2f;
          
          // Prevent wa form being negative
          if (0.0f > wa)
            {
              wa = 0.0f;
            }
          
          // None of the water in the soil layers is in saturated soil so wt is the same as wa.
          wt = wa;
        }
      else
        {
          // If the water table is not deeper than the bottom of the lowest soil layer then the aquifer is completely full, and saturated soil includes the
          // distance of the water table above the bottom of the lowest soil layer times 1000 to convert from meters to millimeters times the porosity.  The
          // global variable NOAHMP_POROSITY gets set in REDPRM so we can't do this before here.  zSoil is negative so add zSoil to zwt to get the wetted
          // thickness, and zwt + zSoil is negative so subtract the result from wa to add water.
          wa = 5000.0f;
          wt = wa - (evapoTranspirationSoilMoisture->zwt + zSoil[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS - 1]) * 1000.0f * NOAHMP_POROSITY;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Include water in the aquifer in the mass balance check.
      soilMoistureOriginal += wa;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
      // Error check that none of the soil moisture variables are greater than NOAHMP_POROSITY.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          if (!(0.0f < evapoTranspirationSoilMoisture->smcEq[ii] && evapoTranspirationSoilMoisture->smcEq[ii] <= NOAHMP_POROSITY))
            {
              CkError("ERROR in evapoTranspirationSoil: smcEq must be greater than zero and less than or equal to NOAHMP_POROSITY.\n");
              error = true;
            }
          
          if (!(0.0f < evapoTranspirationSoilMoisture->smc[ii] && evapoTranspirationSoilMoisture->smc[ii] <= NOAHMP_POROSITY))
            {
              CkError("ERROR in evapoTranspirationSoil: smc must be greater than zero and less than or equal to NOAHMP_POROSITY.\n");
              error = true;
            }
          
          if (!(0.0f < evapoTranspirationSoilMoisture->sh2o[ii] && evapoTranspirationSoilMoisture->sh2o[ii] <= evapoTranspirationSoilMoisture->smc[ii]))
            {
              CkError("ERROR in evapoTranspirationSoil: sh2o must be greater than zero and less than or equal to smc[ii].\n");
              error = true;
            }
        }
      
      if (!(0.0f < evapoTranspirationSoilMoisture->smcwtd && evapoTranspirationSoilMoisture->smcwtd <= NOAHMP_POROSITY))
        {
          CkError("ERROR in evapoTranspirationSoil: smcwtd must be greater than zero and less than or equal to NOAHMP_POROSITY.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    }

  if (!error)
    {
      // Run Noah-MP.
      NOAHMP_SFLX(&iLoc, &jLoc, &lat, &yearLen, &julian, &cosZ, &dt, &dx, &evapoTranspirationForcing->dz8w, &nSoil, zSoil, &nSnow, &shdFac, &shdMax, &vegType,
                  &isUrban, &ice, &ist, &isc, evapoTranspirationSoilMoisture->smcEq, &iz0tlnd, &evapoTranspirationForcing->sfcTmp,
                  &evapoTranspirationForcing->sfcPrs, &evapoTranspirationForcing->psfc, &evapoTranspirationForcing->uu, &evapoTranspirationForcing->vv,
                  &evapoTranspirationForcing->q2, &evapoTranspirationForcing->qc, &evapoTranspirationForcing->solDn, &evapoTranspirationForcing->lwDn,
                  &evapoTranspirationForcing->prcp, &evapoTranspirationForcing->tBot, &co2Air, &o2Air, &folN, evapoTranspirationState->fIceOld,
                  &evapoTranspirationForcing->pblh, &zLvl, &evapoTranspirationState->albOld, &evapoTranspirationState->snEqvO, evapoTranspirationState->stc,
                  evapoTranspirationSoilMoisture->sh2o, evapoTranspirationSoilMoisture->smc, &evapoTranspirationState->tah, &evapoTranspirationState->eah,
                  &evapoTranspirationState->fWet, &evapoTranspirationState->canLiq, &evapoTranspirationState->canIce, &evapoTranspirationState->tv,
                  &evapoTranspirationState->tg, &qsfc, &qSnow, &evapoTranspirationState->iSnow, evapoTranspirationState->zSnso,
                  &evapoTranspirationState->snowH, &evapoTranspirationState->snEqv, evapoTranspirationState->snIce, evapoTranspirationState->snLiq,
                  &evapoTranspirationSoilMoisture->zwt, &wa, &wt, &wsLake, &evapoTranspirationState->lfMass, &evapoTranspirationState->rtMass,
                  &evapoTranspirationState->stMass, &evapoTranspirationState->wood, &evapoTranspirationState->stblCp, &evapoTranspirationState->fastCp,
                  &evapoTranspirationState->lai, &evapoTranspirationState->sai, &evapoTranspirationState->cm, &evapoTranspirationState->ch,
                  &evapoTranspirationState->tauss, &evapoTranspirationSoilMoisture->smcwtd, &evapoTranspirationState->deepRech, &evapoTranspirationState->rech,
                  &fsa, &fsr, &fira, &fsh, &sSoil, &fcev, &fgev, &fctr, &eCan, &eTran, &eDir, &tRad, &tgb, &tgv, &t2mv, &t2mb, &q2v, &q2b, &runSrf, &runSub,
                  &apar, &psn, &sav, &sag, &fSno, &nee, &gpp, &npp, &fVeg, &albedo, &qSnBot, &ponding, &ponding1, &ponding2, &rsSun, &rsSha, &bGap, &wGap,
                  &chv, &chb, &emissi, &shg, &shc, &shb, &evg, &evb, &ghv, &ghb, &irg, &irc, &irb, &tr, &evc, &chLeaf, &chuc, &chv2, &chb2, &fpIce, &qRain);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Verify that the input variables have not changed.
      CkAssert(iLocOriginal == iLoc && jLocOriginal == jLoc && latOriginal == lat && yearLenOriginal == yearLen && julianOriginal == julian &&
               cosZOriginal == cosZ && dtOriginal == dt && dxOriginal == dx && dz8wOriginal == evapoTranspirationForcing->dz8w && nSoilOriginal == nSoil &&
               zSoilOriginal[0] == zSoil[0] && zSoilOriginal[1] == zSoil[1] && zSoilOriginal[2] == zSoil[2] && zSoilOriginal[3] == zSoil[3] &&
               nSnowOriginal == nSnow && shdFacOriginal == shdFac && shdMaxOriginal == shdMax && vegTypeOriginal == vegType && isUrbanOriginal == isUrban &&
               iceOriginal == ice && istOriginal == ist && iscOriginal == isc && smcEqOriginal[0] == evapoTranspirationSoilMoisture->smcEq[0] &&
               smcEqOriginal[1] == evapoTranspirationSoilMoisture->smcEq[1] && smcEqOriginal[2] == evapoTranspirationSoilMoisture->smcEq[2] &&
               smcEqOriginal[3] == evapoTranspirationSoilMoisture->smcEq[3] && iz0tlndOriginal == iz0tlnd &&
               sfcTmpOriginal == evapoTranspirationForcing->sfcTmp && sfcPrsOriginal == evapoTranspirationForcing->sfcPrs &&
               psfcOriginal == evapoTranspirationForcing->psfc && uuOriginal == evapoTranspirationForcing->uu && vvOriginal == evapoTranspirationForcing->vv &&
               q2Original == evapoTranspirationForcing->q2 && qcOriginal == evapoTranspirationForcing->qc &&
               solDnOriginal == evapoTranspirationForcing->solDn && lwDnOriginal == evapoTranspirationForcing->lwDn &&
               prcpOriginal == evapoTranspirationForcing->prcp && tBotOriginal == evapoTranspirationForcing->tBot && co2AirOriginal == co2Air &&
               o2AirOriginal == o2Air && folNOriginal == folN && fIceOldOriginal[0] == evapoTranspirationState->fIceOld[0] &&
               fIceOldOriginal[1] == evapoTranspirationState->fIceOld[1] && fIceOldOriginal[2] == evapoTranspirationState->fIceOld[2] &&
               pblhOriginal == evapoTranspirationForcing->pblh && zLvlOriginal == zLvl);
      
      // Snowmelt that goes into ponding is taken from snEqv before its original value is stored in snEqvO.  This is only true for ponding, not ponding1 or
      // ponding2.  Therefore, if ponding is not equal to zero, snEqvOriginal will not equal snEqvO.  Instead, snEqvOriginal will equal snEqvO + ponding, and
      // we have to check for epsilon equal because of roundoff error. If ponding is zero then snEqvOriginal and snEqvO will be exactly equal.
      if (0.0f != ponding)
        {
          CkAssert(epsilonEqual(snEqvOriginal, evapoTranspirationState->snEqvO + ponding));
        }
      else
        {
          CkAssert(snEqvOriginal == evapoTranspirationState->snEqvO);
        }
      
      // Verify that the fraction of the precipitation that falls as snow is between 0 and 1, the snowfall and rainfall rates below the canopy are not negative,
      // and the snowmelt out the bottom of the snowpack is not negative.
      CkAssert(0.0f <= fpIce && 1.0f >= fpIce && 0.0f <= qSnow && 0.0f <= qRain && 0.0f <= qSnBot && 0.0f <= ponding && 0.0f <= ponding1 && 0.0f <= ponding2);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Store fIce from the beginning of the timestep in fIceOld.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          if (ii >= iSnowOriginal + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS)
            {
              evapoTranspirationState->fIceOld[ii] = fIce[ii];
            }
          else
            {
              evapoTranspirationState->fIceOld[ii] = 0.0f;
            }
        }
      
      // Calculate derived output variables.
      *evaporationFromCanopy = eCan * dt;
      evaporationFromSurface = eDir * dt;
      
      // Surface condensation is added to the snow layer if any snow exists or otherwise added to the ground as water.  Surface evaporation is taken first from
      // the snow layer up to the amount in snEqvO, and then any remaining is taken from the ground.
      if (0.0f > evaporationFromSurface)
        {
          if (0.0f < evapoTranspirationState->snEqvO)
            {
              *evaporationFromSnow   = evaporationFromSurface;
              *evaporationFromGround = 0.0f;
            }
          else
            {
              *evaporationFromSnow   = 0.0f;
              *evaporationFromGround = evaporationFromSurface;
            }
        }
      else if (evaporationFromSurface <= evapoTranspirationState->snEqvO)
        {
          *evaporationFromSnow   = evaporationFromSurface;
          *evaporationFromGround = 0.0f;
        }
      else
        {
          *evaporationFromSnow   = evapoTranspirationState->snEqvO;
          *evaporationFromGround = evaporationFromSurface - evapoTranspirationState->snEqvO;
        }
      
      snowfallAboveCanopy         = evapoTranspirationForcing->prcp * dt * fpIce;
      snowfallBelowCanopy         = qSnow * dt;
      snowfallInterceptedByCanopy = snowfallAboveCanopy - snowfallBelowCanopy;
      snowmeltOnGround            = qSnBot * dt + ponding + ponding1 + ponding2;
      rainfallAboveCanopy         = evapoTranspirationForcing->prcp * dt - snowfallAboveCanopy;
      rainfallBelowCanopy         = qRain * dt;
      rainfallInterceptedByCanopy = rainfallAboveCanopy - rainfallBelowCanopy;
      
      // If there is a snow layer at the end of the timestep it intercepts all of the rainfall.
      if (0 > evapoTranspirationState->iSnow)
        {
          rainfallInterceptedBySnow = rainfallBelowCanopy;
          rainfallOnGround          = 0.0f;
        }
      else
        {
          rainfallInterceptedBySnow = 0.0f;
          rainfallOnGround          = rainfallBelowCanopy;
        }
      
      // Do a mass balance check for the canopy.
      canWaterShouldBe = canIceOriginal + canLiqOriginal + snowfallInterceptedByCanopy + rainfallInterceptedByCanopy - *evaporationFromCanopy;

      // There is a mass balance bug that can happen to the canopy water.  When the canopy completely empties of water, the total outflow of
      // ((qSnow + qRain + eCan) * dt) can be greater than the total water available of (prcp * dt + canIce + canLiq).  This will result in canWaterShouldBe
      // being negative.  My first thought was to take the missing water back from snowfallBelowCanopy, rainfallBelowCanopy, and evaporationFromCanopy.
      // However, the mass balance check for the snow pack only works with the unaltered values of snowfallBelowCanopy and rainfallBelowCanopy so I would have
      // to take the missing water back from the snowpack too.  And what if the snowpack happened to disappear as well during the exact same timestep as the
      // canopy emptying.  It started to get complicated so I have decided to just create the water and record it in waterError.  The canopy completely
      // emptying should occur infrequently; at most once per storm event.  In the one case where I have seen this it only created 0.1 micron of water.
      // 
      // canWaterShouldBe can also go negative due to round-off error.  This is not a mass balance bug, but I can't proceed with canWaterShouldBe negative.
      // However, setting canWaterShouldBe to zero for negative roundoff errors, but not positive roundoff errors creates a water creating bias so it is
      // correct to record the created water in waterError.
      if (0.0f > canWaterShouldBe)
        {
          *waterError     -= canWaterShouldBe;
          canWaterShouldBe = 0.0f;
        }

      // If canIce or canLiq fall below 1e-6 mm then Noah-MP sets them to zero and the water is lost.  One pernicious aspect of this problem is that our
      // timesteps are smaller than what the Noah-MP code developers expected so we can see accumulations on the canopy of less than 1e-6 mm per timestep.
      // This water will be thrown away every timestep and nothing will ever accumulate.  This only happens for very light precipitation, and it only throws
      // away less than a nanometer of water each timestep so it's not too bad, but I wish they hadn't coded it this way.
      //
      // This problem went undetected for a long time because our epsilon for single precision floats is 1e-6 so when Noah-MP threw away less than 1e-6 it was
      // still epsilon equal.  The problem only became visible when canIce and canLiq were both set to zero during the same timestep and the water thrown away
      // added up to more than 1e-6.  So potentially, (canIce + canLiq) can be from (epsilon equal to (canWaterShouldBe - 2.0e-6)) up to
      // (epsilon equal to canWaterShouldBe).
      //
      // We can't really tell the difference between small amounts of water being thrown away and roundoff error so in every case we set canIce and canLiq to
      // equal canWaterShouldBe.  This should fix small amounts of water being thrown away without making roundoff error any worse.
      //
      // There was a situation where rainfallAboveCanopy and rainfallBelowCanopy were both large, but rainfallInterceptedByCanopy was small so its roundoff
      // error was large relative to the magnitude of canWaterShouldBe, and this check failed.  The same could occur for snow.  The solution is to use the
      // amount of precipitation to determine the acceptable epsilon.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonLessOrEqual(canWaterShouldBe - 2.0e-6f, evapoTranspirationState->canIce + evapoTranspirationState->canLiq, evapoTranspirationForcing->prcp) &&
               epsilonGreaterOrEqual(canWaterShouldBe, evapoTranspirationState->canIce + evapoTranspirationState->canLiq, evapoTranspirationForcing->prcp));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Determine the fraction of canopy ice.  This reuses the variable fIce for a different purpose because it is not used again in this function.
      if ((evapoTranspirationState->canIce + evapoTranspirationState->canLiq) > 0.0)
        {
          // If canIce and canLiq weren't set to zero maintain the same proportion of canopy ice.
          fIce[0] = evapoTranspirationState->canIce / (evapoTranspirationState->canIce + evapoTranspirationState->canLiq);
        }
      else if (NOAHMP_TFRZ < evapoTranspirationState->tv)
        {
          // If canIce and canLiq were set to zero make it all liquid if the temperature is above freezing.
          fIce[0] = 0.0f;
        }
      else
        {
          // If canIce and canLiq were set to zero make it all ice if the temperature is at or below freezing.
          fIce[0] = 1.0f;
        }
      
      // Set (canIce + canLiq) to equal canWaterShouldBe.
      evapoTranspirationState->canIce = canWaterShouldBe * fIce[0];
      evapoTranspirationState->canLiq = canWaterShouldBe - evapoTranspirationState->canIce;
      
      // Do a mass balance check for the snowpack.
      snEqvShouldBe = snEqvOriginal + snowfallBelowCanopy + rainfallInterceptedBySnow - *evaporationFromSnow - snowmeltOnGround;
      
      // snEqvShouldBe can go negative due to round-off error.  This is not a mass balance bug, but I can't proceed with snEqvShouldBe negative.
      // However, setting snEqvShouldBe to zero for negative roundoff errors, but not positive roundoff errors creates a water creating bias so it is
      // correct to record the created water in waterError.
      if (0.0f > snEqvShouldBe)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          *waterError  -= snEqvShouldBe;
          snEqvShouldBe = 0.0f;
        }
      
      runoff = (runSrf + runSub) * dt;
      
      // If snEqv falls below 0.001 mm, or snowH falls below 1e-6 m then Noah-MP sets both to zero and the water is lost.  If snEqv grows above 2000 mm then
      // Noah-MP sets it to 2000 and the water is added to runSub as glacier flow.  We are putting the water back.
      if (0.0f == evapoTranspirationState->snEqv)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(0.001f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snEqv = snEqvShouldBe;
          evapoTranspirationState->snowH = snEqvShouldBe / 1000.0f; // Divide by one thousand to convert from millimeters to meters.
        }
      else if (2000.0f < snEqvShouldBe)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(2000.0f, evapoTranspirationState->snEqv));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snowH += snEqvShouldBe - evapoTranspirationState->snEqv / 1000.0f; // Divide by one thousand to convert from millimeters to meters.
          
          if (0 > evapoTranspirationState->iSnow)
            {
              evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] = -evapoTranspirationState->snowH;
              
              for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
                {
                  evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] =
                      evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] + zSoil[ii];
                }
              
              evapoTranspirationState->snIce[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] += snEqvShouldBe - evapoTranspirationState->snEqv;
            }
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(evapoTranspirationState->snEqv + runoff, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          // Take the glacier flow back from runSub.
          runoff -= snEqvShouldBe - evapoTranspirationState->snEqv;
          
          // Put the water back in snEqv.
          evapoTranspirationState->snEqv = snEqvShouldBe;
        }
      else
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(evapoTranspirationState->snEqv, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      CkAssert(!checkEvapoTranspirationStateStructInvariant(evapoTranspirationState));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      
      *surfacewaterAdd = snowmeltOnGround + rainfallOnGround;
      *transpiration   = eTran * dt;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0f <= *surfacewaterAdd && 0.0f <= *transpiration);
      
      // Calculate soil moisture at the end of the timestep.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          if (0 == ii)
            {
              soilMoistureNew += evapoTranspirationSoilMoisture->smc[ii] * -zSoil[ii] * 1000.0f; // * 1000.0f because zSoil is in meters and soilMoistureNew is
                                                                                                 // in millimeters.
            }
          else
            {
              soilMoistureNew += evapoTranspirationSoilMoisture->smc[ii] * (zSoil[ii - 1] - zSoil[ii]) * 1000.0f;
            }
        }
      
      // Include water in the aquifer in the mass balance check.
      soilMoistureNew += wa;
      
      // Verify that soil moisture balances.  Epsilon needs to be based on the largest value used to calculate it.  However, there was a case where
      // soilMoistureOriginal was large with a lot of water subtracted from it and soilMoistureNew was small.  The values passed to the first epsilonEqual were
      // both small so epsilon was small and this check failed even though the difference was okay if epsilon were based on soilMoistureOriginal.  So I added
      // the second check That would use epsilon based on soilMoistureOriginal.
      CkAssert(epsilonEqual(soilMoistureOriginal - *evaporationFromGround - *transpiration + *surfacewaterAdd - runoff,  soilMoistureNew) ||
               epsilonEqual(soilMoistureOriginal,  *evaporationFromGround + *transpiration - *surfacewaterAdd + runoff + soilMoistureNew));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    } // End if (!error).
  
  return error;
}

bool evapoTranspirationWater(float lat, int yearLen, float julian, float cosZ, float dt, float dx, float wsLake,
                             EvapoTranspirationForcingStruct* evapoTranspirationForcing, EvapoTranspirationStateStruct* evapoTranspirationState,
                             float* surfacewaterAdd, float* evaporationFromSnow, float* evaporationFromGround, float* waterError)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
  // Input parameters for redprm function.  Some are also used for sflx function.
  int   vegType   = 16;                                        // 'Water Bodies' from VEGPARM.TBL.
  int   soilType  = 14;                                        // 'WATER' from SOILPARM.TBL.
  int   slopeType = 8;                                         // I just arbitrarily chose a slope type with zero slope.  I think slope is only used to
                                                               // calculate runoff, which we ignore.
  float zSoil[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];      // Layer bottom depth in meters from soil surface of each soil layer.  Values are set below from
                                                               // zSnso.
  int   nSoil     = EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; // Always use four soil layers.
  int   isUrban   = 1;                                         // USGS vegetation type for urban land.
  
  // Input parameters to sflx function.
  int   iLoc    = 1;                                           // Grid location index, unused.
  int   jLoc    = 1;                                           // Grid location index, unused.
  int   nSnow   = EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS;   // Maximum number of snow layers.  Always pass 3.
  float shdFac  = 0.0f;                                        // Fraction of land area shaded by vegetation, 0.0 to 1.0.  Always pass 0.0.
  float shdMax  = 0.0f;                                        // Yearly maximum fraction of land area shaded by vegetation, 0.0 to 1.0.  Always pass 0.0.
  int   ice     = 0;                                           // Flag to indicate permanent ice cover, 0 for no, 1 for yes.  Always pass 0.  If permanent ice
                                                               // cover call evapoTranspirationGlacier instead.
  int   ist     = 2;                                           // Flag to indicate permanent water cover, 1 for soil, 2 for lake.  Always pass 2.  If permanent
                                                               // soil cover call evapoTranspirationSoil instead.
  int   isc     = 4;                                           // Soil color type, 1 for lightest to 8 for darkest.  Always pass 4.
  float smcEq[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];      // Equlibrium water content of each soil layer, unitless.  Values are set below from
                                                               // NOAHMP_POROSITY.
  int   iz0tlnd = 0;                                           // Unused.
  float co2Air  = 0.0004f * evapoTranspirationForcing->sfcPrs; // CO2 partial pressure in Pascal at surface.  Always pass 400 parts per million of surface
                                                               // pressure.
  float o2Air   = 0.21f * evapoTranspirationForcing->sfcPrs;   // O2 partial pressure in Pascal at surface.  Always pass 21 percent of surface pressure.
  float folN    = 0.0f;                                        // Foliage nitrogen percentage, 0.0 to 100.0.  Always pass 0.0.
  float fIce[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];       // Frozen fraction of each snow layer, unitless.  Values are set below from snIce and snLiq.
  float zLvl    = evapoTranspirationForcing->dz8w;             // Thickness in meters of lowest atmosphere layer in forcing data.  Redundant with dz8w.
  
  // Input/output parameters to sflx function.
  float sh2o[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS]; // Liquid water content of each soil layer, unitless.  Values are set below from NOAHMP_POROSITY.
  float smc[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];  // Total water content, liquid and solid, of each soil layer, unitless.  Values are set below from
                                                         // NOAHMP_POROSITY.
  float qsfc  = evapoTranspirationForcing->q2;           // Water vapor mixing ratio at middle of lowest atmosphere layer in forcing data, unitless.  Redundant
                                                         // with q2.
  float qSnow = NAN;                                     // This is actually an output only variable.  Snowfall rate below the canopy in millimeters of water
                                                         // equivalent per second.
  float zwt   = 0.0f;                                    // Depth in meters to water table.  Specify depth as a positive number.  Always pass 0.0.
  float wa;                                              // Water stored in aquifer in millimeters of water.  Value is set below.
  float wt;                                              // Water stored in aquifer and saturated soil in millimeters of water.  Value is set below from zSoil,
                                                         // and NOAHMP_POROSITY.
  float smcwtd;                                          // Water content between the bottom of the lowest soil layer and water table, unitless.  Value is set
                                                         // below from NOAHMP_POROSITY.
  
  // Output parameters to sflx function.  Set to NAN so we can detect if the values are used before being set.
  float fsa      = NAN; // Unused.
  float fsr      = NAN; // Unused.
  float fira     = NAN; // Unused.
  float fsh      = NAN; // Unused.
  float sSoil    = NAN; // Unused.
  float fcev     = NAN; // Unused.
  float fgev     = NAN; // Unused.
  float fctr     = NAN; // Unused.
  float eCan     = NAN; // Evaporation rate from canopy in millimeters of water equivalent per second.
  float eTran    = NAN; // Evaporation rate from transpiration in millimeters of water equivalent per second.
  float eDir     = NAN; // Evaporation rate from surface in millimeters of water equivalent per second.
  float tRad     = NAN; // Unused.
  float tgb      = NAN; // Unused.
  float tgv      = NAN; // Unused.
  float t2mv     = NAN; // Unused.
  float t2mb     = NAN; // Unused.
  float q2v      = NAN; // Unused.
  float q2b      = NAN; // Unused.
  float runSrf   = NAN; // Infiltration excess runoff in millimeters of water per second.
  float runSub   = NAN; // Saturation excess runoff in millimeters of water per second.
  float apar     = NAN; // Unused.
  float psn      = NAN; // Unused.
  float sav      = NAN; // Unused.
  float sag      = NAN; // Unused.
  float fSno     = NAN; // Unused.
  float nee      = NAN; // Unused.
  float gpp      = NAN; // Unused.
  float npp      = NAN; // Unused.
  float fVeg     = NAN; // Unused.
  float albedo   = NAN; // Unused.
  
  // There is some complexity with qSnBot and the ponding variables.  The big picture is that they contain the flow out the bottom of the snowpack including
  // snowmelt plus ranfall that lands on the snowpack and passes through without freezing.  In short, the quantity of water that flowed out the bottom of the
  // snowpack is always equal to qSnBot * dt + ponding + ponding1 + ponding2.  However, this flow is put into one of the four different variables depending on
  // the situation.  If the multi-layer snow simulation is turned on at the end of the timestep then the snowmelt rate is put into qSnBot and the ponding
  // variables are set to zero.  If the multi-layer snow simulation is turned off at the end of the timestep then the snowmelt quantity is put into one of the
  // ponding variables and the other ponding variables and qSnBot are set to zero.  Which ponding variable gets the water also depends on the situation. If the
  // multi-layer snow simulation is turned off at the beginning and end of the timestep then the water is put in ponding.  If the multi-layer snow simulation
  // is turned on at the beginning of the timestep, but is off at the end of the timestep then the water is put either in ponding1 or ponding2 depending on the
  // reason why the multi-layer snow simulation was turned off.  If in an individual layer in the multi-layer snow simulation has snIce go below 0.1 mm of snow
  // water equivalent that layer is eliminated.  The water is added to a neighboring layer if possible, but if that was the last layer then the multi-layer
  // snow simulation is turned off, snEqv is set to snIce, and the water in snLiq is put in ponding1.  If the height of the entire snow pack, snowH, goes below
  // 0.025 m then the multi-layer snow simulation is turned off, snEqv is set to snIce, and the water in snLiq is put in ponding2.
  float qSnBot   = NAN; // Flow rate out the bottom of the snowpack in millimeters of water per second.
  float ponding  = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  float ponding1 = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  float ponding2 = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  
  float rsSun    = NAN; // Unused.
  float rsSha    = NAN; // Unused.
  float bGap     = NAN; // Unused.
  float wGap     = NAN; // Unused.
  float chv      = NAN; // Unused.
  float chb      = NAN; // Unused.
  float emissi   = NAN; // Unused.
  float shg      = NAN; // Unused.
  float shc      = NAN; // Unused.
  float shb      = NAN; // Unused.
  float evg      = NAN; // Unused.
  float evb      = NAN; // Unused.
  float ghv      = NAN; // Unused.
  float ghb      = NAN; // Unused.
  float irg      = NAN; // Unused.
  float irc      = NAN; // Unused.
  float irb      = NAN; // Unused.
  float tr       = NAN; // Unused.
  float evc      = NAN; // Unused.
  float chLeaf   = NAN; // Unused.
  float chuc     = NAN; // Unused.
  float chv2     = NAN; // Unused.
  float chb2     = NAN; // Unused.
  float fpIce    = NAN; // Fraction of precipitation that is frozen, unitless.
  float qRain    = NAN; // Rainfall rate below the canopy in millimeters of water per second.
  
  // Derived output variables.
  int   iSnowOriginal;             // Actual number of snow layers before timestep.
  float evaporationFromSurface;    // Quantity of evaporation from the surface in millimeters of water equivalent.  Positive means water evaporated off of the
                                   // surface.  Negative means water condensed on to the surface.  Surface evaporation sometimes comes from snow and is taken
                                   // out by Noah-MP, but if there is not enough snow we have to take the evaporation from the water in the ADHydro state
                                   // variables.
  float thrownAwaySnow;            // If a waterbody surface is above freezing Noah-MP throws away any snowfall.  This variable is used to put it back. It is
                                   // the quantity of snow that was thrown away in millimeters of water equivalent.  Must be non-negative.
  float snowfall;                  // Quantity of snowfall in millimeters of water equivalent.  Must be non-negative.
  float snowmeltOnGround;          // Quantity of water that reaches the ground from the snow layer in millimeters of water.  Must be non-negative.
  float rainfall;                  // Quantity of rainfall in millimeters of water.  Must be non-negative.
  float rainfallInterceptedBySnow; // Quantity of rainfall intercepted by the snow layer in millimeters of water.  Must be non-negative.
  float rainfallOnGround;          // Quantity of rainfall that reaches the ground in millimeters of water.  Must be non-negative.
  float snEqvOriginal;             // Quantity of water in the snow layer(s) before timestep in millimeters of water equivalent.  snEqvO is not always set to
                                   // this value after the timestep.
  float snEqvShouldBe;             // If snEqv falls below 0.001 Noah-MP sets it to zero, or if it rises above 2000.0 Noah-MP sets it to 2000.0.  We don't want
                                   // this behavior so in this variable we calculate what snEqv should be and set it back.  If snEqv is not changed this
                                   // instead performs a mass balance check.
  float runoff;                    // Quantity of water that runs off from the soil in millimeters of water.  We don't use Noah-MP's infiltration and
                                   // groundwater simulation so this is merely for a mass balance check.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Variables used for assertions.
  int   iLocOriginal         = iLoc;
  int   jLocOriginal         = jLoc;
  float latOriginal          = lat;
  int   yearLenOriginal      = yearLen;
  float julianOriginal       = julian;
  float cosZOriginal         = cosZ;
  float dtOriginal           = dt;
  float dxOriginal           = dx;
  float dz8wOriginal         = evapoTranspirationForcing->dz8w;
  int   nSoilOriginal        = nSoil;
  float zSoilOriginal[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];
  int   nSnowOriginal        = nSnow;
  float shdFacOriginal       = shdFac;
  float shdMaxOriginal       = shdMax;
  int   vegTypeOriginal      = vegType;
  int   isUrbanOriginal      = isUrban;
  int   iceOriginal          = ice;
  int   istOriginal          = ist;
  int   iscOriginal          = isc;
  float smcEqOriginal[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];
  int   iz0tlndOriginal      = iz0tlnd;
  float sfcTmpOriginal       = evapoTranspirationForcing->sfcTmp;
  float sfcPrsOriginal       = evapoTranspirationForcing->sfcPrs;
  float psfcOriginal         = evapoTranspirationForcing->psfc;
  float uuOriginal           = evapoTranspirationForcing->uu;
  float vvOriginal           = evapoTranspirationForcing->vv;
  float q2Original           = evapoTranspirationForcing->q2;
  float qcOriginal           = evapoTranspirationForcing->qc;
  float solDnOriginal        = evapoTranspirationForcing->solDn;
  float lwDnOriginal         = evapoTranspirationForcing->lwDn;
  float prcpOriginal         = evapoTranspirationForcing->prcp;
  float tBotOriginal         = evapoTranspirationForcing->tBot;
  float co2AirOriginal       = co2Air;
  float o2AirOriginal        = o2Air;
  float folNOriginal         = folN;
  float fIceOldOriginal[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
  float pblhOriginal         = evapoTranspirationForcing->pblh;
  float zLvlOriginal         = zLvl;
  float soilMoistureOriginal = 0.0f; // Total soil moisture before timestep in millimeters of water.
  float soilMoistureNew      = 0.0f; // Total soil moisture after timestep in millimeters of water.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(-M_PI / 2.0f <= lat && M_PI / 2.0f >= lat))
    {
      CkError("ERROR in evapoTranspirationWater: lat must be greater than or equal to -pi/2 and less than or equal to pi/2.\n");
      error = true;
    }
  
  if (!(365 == yearLen || 366 == yearLen))
    {
      CkError("ERROR in evapoTranspirationWater: yearLen must be 365 or 366.\n");
      error = true;
    }
  
  if (!(0.0f <= julian && julian <= yearLen))
    {
      CkError("ERROR in evapoTranspirationWater: julian must be greater than or equal to zero and less than or equal to yearLen.\n");
      error = true;
    }
  
  if (!(0.0f <= cosZ && 1.0f >= cosZ))
    {
      CkError("ERROR in evapoTranspirationWater: cosZ must be greater than or equal to zero and less than or equal to one.\n");
      error = true;
    }
  
  if (!(0.0f < dt))
    {
      CkError("ERROR in evapoTranspirationWater: dt must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0f < dx))
    {
      CkError("ERROR in evapoTranspirationWater: dx must be greater than zero.\n");
      error = true;
    }

  if (!(0.0f <= wsLake))
    {
      CkError("ERROR in evapoTranspirationWater: wsLake must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(NULL != evapoTranspirationForcing))
    {
      CkError("ERROR in evapoTranspirationWater: evapoTranspirationForcing must not be NULL.\n");
      error = true;
    }
  
  if (!(NULL != evapoTranspirationState))
    {
      CkError("ERROR in evapoTranspirationWater: evapoTranspirationState must not be NULL.\n");
      error = true;
    }
  else
    {
      if (!(0.0f == evapoTranspirationState->canLiq))
        {
          CkError("ERROR in evapoTranspirationWater: canLiq must be zero.\n");
          error = true;
        }

      if (!(0.0f == evapoTranspirationState->canIce))
        {
          CkError("ERROR in evapoTranspirationWater: canIce must be zero.\n");
          error = true;
        }
    }

  if (!(NULL != surfacewaterAdd))
    {
      CkError("ERROR in evapoTranspirationWater: surfacewaterAdd must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *surfacewaterAdd = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evaporationFromSnow))
    {
      CkError("ERROR in evapoTranspirationWater: evaporationFromSnow must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *evaporationFromSnow = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evaporationFromGround))
    {
      CkError("ERROR in evapoTranspirationWater: evaporationFromGround must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *evaporationFromGround = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != waterError))
    {
      CkError("ERROR in evapoTranspirationWater: waterError must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *waterError = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (!error)
    {
      error = checkEvapoTranspirationForcingStructInvariant(evapoTranspirationForcing);
    }
  
  if (!error)
    {
      error = checkEvapoTranspirationStateStructInvariant(evapoTranspirationState);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  if (!error)
    {
      // Set variables that require error checking of pointers first.
      iSnowOriginal  = evapoTranspirationState->iSnow;
      snEqvOriginal  = evapoTranspirationState->snEqv;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          fIceOldOriginal[ii] = evapoTranspirationState->fIceOld[ii];
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Set zSoil from zSnso.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          if (0 > evapoTranspirationState->iSnow)
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] -
                          evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1];
            }
          else
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
            }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          zSoilOriginal[ii] = zSoil[ii];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Calculate fIce at the beginning of the timestep.
      for (ii = evapoTranspirationState->iSnow + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          fIce[ii] = evapoTranspirationState->snIce[ii] / (evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]);
        }

      // Set Noah-MP globals.
      REDPRM(&vegType, &soilType, &slopeType, zSoil, &nSoil, &isUrban);
      
      // Set water content of the soil layers.  In waterbodies the watertable is always at the surface and everything is saturated.  The global variable
      // NOAHMP_POROSITY gets set in REDPRM so we can't do this before here.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          smcEq[ii] = NOAHMP_POROSITY;
          sh2o[ii]  = NOAHMP_POROSITY;
          smc[ii]   = NOAHMP_POROSITY;

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          smcEqOriginal[ii] = smcEq[ii];
          
          // Calculate soil moisture at the beginning of the timestep.
          if (0 == ii)
            {
              soilMoistureOriginal += smc[ii] * -zSoil[ii] * 1000.0f; // * 1000.0f because zSoil is in meters and soilMoistureOriginal is in millimeters.
            }
          else
            {
              soilMoistureOriginal += smc[ii] * (zSoil[ii - 1] - zSoil[ii]) * 1000.0f;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }
      
      // Calculate water in the aquifer and saturated soil.  zSoil is negative so subtract the result from wa to add water.
      wa     = 5000.0f;
      wt     = wa - zSoil[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS - 1] * 1000.0f * NOAHMP_POROSITY;
      smcwtd = NOAHMP_POROSITY;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Include water in the aquifer and channel surfacewater in the mass balance check.
      soilMoistureOriginal += wa + wsLake;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Run Noah-MP.
      NOAHMP_SFLX(&iLoc, &jLoc, &lat, &yearLen, &julian, &cosZ, &dt, &dx, &evapoTranspirationForcing->dz8w, &nSoil, zSoil, &nSnow, &shdFac, &shdMax, &vegType,
                  &isUrban, &ice, &ist, &isc, smcEq, &iz0tlnd, &evapoTranspirationForcing->sfcTmp, &evapoTranspirationForcing->sfcPrs,
                  &evapoTranspirationForcing->psfc, &evapoTranspirationForcing->uu, &evapoTranspirationForcing->vv, &evapoTranspirationForcing->q2,
                  &evapoTranspirationForcing->qc, &evapoTranspirationForcing->solDn, &evapoTranspirationForcing->lwDn, &evapoTranspirationForcing->prcp,
                  &evapoTranspirationForcing->tBot, &co2Air, &o2Air, &folN, evapoTranspirationState->fIceOld, &evapoTranspirationForcing->pblh, &zLvl,
                  &evapoTranspirationState->albOld, &evapoTranspirationState->snEqvO, evapoTranspirationState->stc, sh2o, smc, &evapoTranspirationState->tah,
                  &evapoTranspirationState->eah, &evapoTranspirationState->fWet, &evapoTranspirationState->canLiq, &evapoTranspirationState->canIce,
                  &evapoTranspirationState->tv, &evapoTranspirationState->tg, &qsfc, &qSnow, &evapoTranspirationState->iSnow, evapoTranspirationState->zSnso,
                  &evapoTranspirationState->snowH, &evapoTranspirationState->snEqv, evapoTranspirationState->snIce, evapoTranspirationState->snLiq, &zwt, &wa,
                  &wt, &wsLake, &evapoTranspirationState->lfMass, &evapoTranspirationState->rtMass, &evapoTranspirationState->stMass,
                  &evapoTranspirationState->wood, &evapoTranspirationState->stblCp, &evapoTranspirationState->fastCp, &evapoTranspirationState->lai,
                  &evapoTranspirationState->sai, &evapoTranspirationState->cm, &evapoTranspirationState->ch, &evapoTranspirationState->tauss, &smcwtd,
                  &evapoTranspirationState->deepRech, &evapoTranspirationState->rech, &fsa, &fsr, &fira, &fsh, &sSoil, &fcev, &fgev, &fctr, &eCan, &eTran,
                  &eDir, &tRad, &tgb, &tgv, &t2mv, &t2mb, &q2v, &q2b, &runSrf, &runSub, &apar, &psn, &sav, &sag, &fSno, &nee, &gpp, &npp, &fVeg, &albedo,
                  &qSnBot, &ponding, &ponding1, &ponding2, &rsSun, &rsSha, &bGap, &wGap, &chv, &chb, &emissi, &shg, &shc, &shb, &evg, &evb, &ghv, &ghb, &irg,
                  &irc, &irb, &tr, &evc, &chLeaf, &chuc, &chv2, &chb2, &fpIce, &qRain);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Verify that the input variables have not changed.
      CkAssert(iLocOriginal == iLoc && jLocOriginal == jLoc && latOriginal == lat && yearLenOriginal == yearLen && julianOriginal == julian &&
               cosZOriginal == cosZ && dtOriginal == dt && dxOriginal == dx && dz8wOriginal == evapoTranspirationForcing->dz8w && nSoilOriginal == nSoil &&
               zSoilOriginal[0] == zSoil[0] && zSoilOriginal[1] == zSoil[1] && zSoilOriginal[2] == zSoil[2] && zSoilOriginal[3] == zSoil[3] &&
               nSnowOriginal == nSnow && shdFacOriginal == shdFac && shdMaxOriginal == shdMax && vegTypeOriginal == vegType && isUrbanOriginal == isUrban &&
               iceOriginal == ice && istOriginal == ist && iscOriginal == isc && smcEqOriginal[0] == smcEq[0] && smcEqOriginal[1] == smcEq[1] &&
               smcEqOriginal[2] == smcEq[2] && smcEqOriginal[3] == smcEq[3] && iz0tlndOriginal == iz0tlnd &&
               sfcTmpOriginal == evapoTranspirationForcing->sfcTmp && sfcPrsOriginal == evapoTranspirationForcing->sfcPrs &&
               psfcOriginal == evapoTranspirationForcing->psfc && uuOriginal == evapoTranspirationForcing->uu && vvOriginal == evapoTranspirationForcing->vv &&
               q2Original == evapoTranspirationForcing->q2 && qcOriginal == evapoTranspirationForcing->qc &&
               solDnOriginal == evapoTranspirationForcing->solDn && lwDnOriginal == evapoTranspirationForcing->lwDn &&
               prcpOriginal == evapoTranspirationForcing->prcp && tBotOriginal == evapoTranspirationForcing->tBot && co2AirOriginal == co2Air &&
               o2AirOriginal == o2Air && folNOriginal == folN && fIceOldOriginal[0] == evapoTranspirationState->fIceOld[0] &&
               fIceOldOriginal[1] == evapoTranspirationState->fIceOld[1] && fIceOldOriginal[2] == evapoTranspirationState->fIceOld[2] &&
               pblhOriginal == evapoTranspirationForcing->pblh && zLvlOriginal == zLvl);
      
      // Snowmelt that goes into ponding is taken from snEqv before its original value is stored in snEqvO.  This is only true for ponding, not ponding1 or
      // ponding2.  Therefore, if ponding is not equal to zero, snEqvOriginal will not equal snEqvO.  Instead, snEqvOriginal will equal snEqvO + ponding, and
      // we have to check for epsilon equal because of roundoff error. If ponding is zero then snEqvOriginal and snEqvO will be exactly equal.
      if (0.0f != ponding)
        {
          CkAssert(epsilonEqual(snEqvOriginal, evapoTranspirationState->snEqvO + ponding));
        }
      else
        {
          CkAssert(snEqvOriginal == evapoTranspirationState->snEqvO);
        }
      
      // Verify that the fraction of the precipitation that falls as snow is between 0 and 1, the snowfall and rainfall rates below the canopy are not negative,
      // and the snowmelt out the bottom of the snowpack is not negative.
      CkAssert(0.0f <= fpIce && 1.0f >= fpIce && 0.0f <= qSnow && 0.0f <= qRain && 0.0f <= qSnBot && 0.0f <= ponding && 0.0f <= ponding1 && 0.0f <= ponding2);
      
      // Verify that there is no water, no evaporation, and no transpiration, in the non-existant canopy.
      CkAssert(0.0f == evapoTranspirationState->canLiq && 0.0f == evapoTranspirationState->canIce && 0.0f == eCan && 0.0f == eTran);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Verify that there is no snowfall interception in the non-existant canopy.
      if (NOAHMP_TFRZ < evapoTranspirationState->tg)
        {
          // There is something weird in the Noah-MP code.  If ist is 2 indicating a waterbody, and the ground temperature is above freezing, then any snow
          // that reaches the ground is set to zero.  It doesn't get melted and added to rainfall.  It just gets thrown away.  This occurs in
          // module_sf_noahmplsm.F line 6754 at the end of subroutine canwater.  After this point, thrownAwaySnow should be added to snEqv, but we don't add it
          // here because snEqv could have been set to 0.0 or 2000.0 for different reasons.  We need to preserve the value of snEqv and only add back in
          // thrownAwaySnow after checking for those cases.
          thrownAwaySnow = evapoTranspirationForcing->prcp * dt * fpIce;
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(0.0f == qSnow);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }
      else
        {
          thrownAwaySnow = 0.0f;
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // If qSnow does not get set to zero it should be equal to prcp * fpIce, but it could be epsilon equal because inside Noah-MP, precipitation is split
          // into convective precipitation and large-scale precipitation.  Even if all of the precipitation eventually becomes qSnow, adding those values back
          // together can cause roundoff error.  This problem doesn't occur in evapoTranspirationSoil because there we assume any difference is canopy
          // interception.  This problem doesn't occur in evapoTranspirationGlacier because that code does not split precipitation into convective
          // precipitation and large-scale precipitation.
          CkAssert(epsilonEqual(evapoTranspirationForcing->prcp * fpIce, qSnow));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }
      
      // Verify that there is no rainfall interception in the non-existant canopy.
      CkAssert(epsilonEqual(evapoTranspirationForcing->prcp * (1.0f - fpIce), qRain));
      
      // Store fIce from the beginning of the timestep in fIceOld.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          if (ii >= iSnowOriginal + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS)
            {
              evapoTranspirationState->fIceOld[ii] = fIce[ii];
            }
          else
            {
              evapoTranspirationState->fIceOld[ii] = 0.0f;
            }
        }
      
      // Calculate derived output variables.
      evaporationFromSurface = eDir * dt;
      
      // Surface condensation is added to the snow layer if any snow exists or otherwise added to the ground as water.  Surface evaporation is taken first from
      // the snow layer up to the amount in snEqvO, and then any remaining is taken from the ground.
      if (0.0f > evaporationFromSurface)
        {
          if (0.0f < evapoTranspirationState->snEqvO)
            {
              *evaporationFromSnow   = evaporationFromSurface;
              *evaporationFromGround = 0.0f;
            }
          else
            {
              *evaporationFromSnow   = 0.0f;
              *evaporationFromGround = evaporationFromSurface;
            }
        }
      else if (evaporationFromSurface <= evapoTranspirationState->snEqvO)
        {
          *evaporationFromSnow   = evaporationFromSurface;
          *evaporationFromGround = 0.0f;
        }
      else
        {
          *evaporationFromSnow   = evapoTranspirationState->snEqvO;
          *evaporationFromGround = evaporationFromSurface - evapoTranspirationState->snEqvO;
        }
      
      snowfall         = evapoTranspirationForcing->prcp * dt * fpIce;
      snowmeltOnGround = qSnBot * dt + ponding + ponding1 + ponding2;
      rainfall         = evapoTranspirationForcing->prcp * dt - snowfall;
      
      // If there is a snow layer at the end of the timestep it intercepts all of the rainfall.
      if (0 > evapoTranspirationState->iSnow)
        {
          rainfallInterceptedBySnow = rainfall;
          rainfallOnGround          = 0.0f;
        }
      else
        {
          rainfallInterceptedBySnow = 0.0f;
          rainfallOnGround          = rainfall;
        }
      
      // Do a mass balance check for the snowpack.
      snEqvShouldBe = snEqvOriginal + snowfall + rainfallInterceptedBySnow - *evaporationFromSnow - snowmeltOnGround;
      
      // snEqvShouldBe can go negative due to round-off error.  This is not a mass balance bug, but I can't proceed with snEqvShouldBe negative.
      // However, setting snEqvShouldBe to zero for negative roundoff errors, but not positive roundoff errors creates a water creating bias so it is
      // correct to record the created water in waterError.
      if (0.0f > snEqvShouldBe)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          *waterError  -= snEqvShouldBe;
          snEqvShouldBe = 0.0f;
        }
      
      runoff = (runSrf + runSub) * dt;
      
      // If snEqv falls below 0.001 mm, or snowH falls below 1e-6 m then Noah-MP sets both to zero and the water is lost.  If snEqv grows above 2000 mm then
      // Noah-MP sets it to 2000 and the water is added to runSub as glacier flow.  We are putting the water back.
      if (0.0f == evapoTranspirationState->snEqv)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(0.001f + thrownAwaySnow, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snEqv = snEqvShouldBe;
          evapoTranspirationState->snowH = snEqvShouldBe / 1000.0f; // Divide by one thousand to convert from millimeters to meters.
        }
      else if (2000.0f < snEqvShouldBe)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(2000.0f, evapoTranspirationState->snEqv));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snowH += snEqvShouldBe - evapoTranspirationState->snEqv / 1000.0f; // Divide by one thousand to convert from millimeters to meters.
          
          if (0 > evapoTranspirationState->iSnow)
            {
              evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] = -evapoTranspirationState->snowH;
              
              for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
                {
                  evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] =
                      evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] + zSoil[ii];
                }
              
              evapoTranspirationState->snIce[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] += snEqvShouldBe - evapoTranspirationState->snEqv;
            }
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(evapoTranspirationState->snEqv + runoff, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          // Take the glacier flow back from runSub.
          runoff -= snEqvShouldBe - evapoTranspirationState->snEqv;
          
          // Put the water back in snEqv.
          evapoTranspirationState->snEqv = snEqvShouldBe;
        }
      else
        {
          if (0.0f < thrownAwaySnow)
            {
              evapoTranspirationState->snEqv += thrownAwaySnow;
              evapoTranspirationState->snowH += thrownAwaySnow / 1000.0f; // Divide by one thousand to convert from millimeters to meters.

              if (0 > evapoTranspirationState->iSnow)
                {
                  evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] = -evapoTranspirationState->snowH;

                  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
                    {
                      evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] =
                          evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] + zSoil[ii];
                    }

                  evapoTranspirationState->snIce[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] += thrownAwaySnow;
                }
            }
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(evapoTranspirationState->snEqv, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      CkAssert(!checkEvapoTranspirationStateStructInvariant(evapoTranspirationState));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      
      *surfacewaterAdd = snowmeltOnGround + rainfallOnGround;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0f <= *surfacewaterAdd);
      
      // Calculate soil moisture at the end of the timestep.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          if (0 == ii)
            {
              soilMoistureNew += smc[ii] * -zSoil[ii] * 1000.0f; // * 1000.0f because zSoil is in meters and soilMoistureNew is in millimeters.
            }
          else
            {
              soilMoistureNew += smc[ii] * (zSoil[ii - 1] - zSoil[ii]) * 1000.0f;
            }
        }
      
      // Include water in the aquifer and channel surfacewater in the mass balance check.
      soilMoistureNew += wa + wsLake;
      
      // Verify that soil moisture balances.  Epsilon needs to be based on the largest value used to calculate it.  However, there was a case where
      // soilMoistureOriginal was large with a lot of water subtracted from it and soilMoistureNew was small.  The values passed to the first epsilonEqual were
      // both small so epsilon was small and this check failed even though the difference was okay if epsilon were based on soilMoistureOriginal.  So I added
      // the second check That would use epsilon based on soilMoistureOriginal.
      CkAssert(epsilonEqual(soilMoistureOriginal - *evaporationFromGround + *surfacewaterAdd - runoff,  soilMoistureNew) ||
               epsilonEqual(soilMoistureOriginal,  *evaporationFromGround - *surfacewaterAdd + runoff + soilMoistureNew));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    } // End if (!error).
  
  return error;
}

bool evapoTranspirationGlacier(float cosZ, float dt, EvapoTranspirationForcingStruct* evapoTranspirationForcing,
                               EvapoTranspirationStateStruct* evapoTranspirationState, float* surfacewaterAdd, float* evaporationFromSnow,
                               float* evaporationFromGround, float* waterError)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
  // Input parameters for redprm function.  Some are also used for sflx function.
  int   vegType   = 24;                                        // 'Snow or Ice' from VEGPARM.TBL.
  int   soilType  = 16;                                        // 'OTHER(land-ice)' from SOILPARM.TBL.
  int   slopeType = 8;                                         // I just arbitrarily chose a slope type with zero slope.  I think slope is only used to
                                                               // calculate runoff, which we ignore.
  float zSoil[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];      // Layer bottom depth in meters from soil surface of each soil layer.  Values are set below from
                                                               // zSnso.
  int   nSoil     = EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; // Always use four soil layers.
  int   isUrban   = 1;                                         // USGS vegetation type for urban land.
  
  // Input parameters to sflx function.
  int   iLoc    = 1;                                         // Grid location index, unused.
  int   jLoc    = 1;                                         // Grid location index, unused.
  int   nSnow   = EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; // Maximum number of snow layers.  Always pass 3.
  float fIce[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];     // Frozen fraction of each snow layer, unitless.  Values are set below from snIce and snLiq.
  
  // Input/output parameters to sflx function.
  float qSnow = NAN;                                     // This is actually an output only variable.  Snowfall rate below the canopy in millimeters of water
                                                         // equivalent per second.
  float smc[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];  // Total water content, liquid and solid, of each soil layer, unitless.  Values are set below from
                                                         // NOAHMP_POROSITY.
  float sh2o[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS]; // Liquid water content of each soil layer, unitless.  Values are set below.
  float qsfc  = evapoTranspirationForcing->q2;           // Water vapor mixing ratio at middle of lowest atmosphere layer in forcing data, unitless.  Redundant
                                                         // with q2.

  // Output parameters to sflx function.  Set to NAN so we can detect if the values are used before being set.
  float fsa      = NAN; // Unused.
  float fsr      = NAN; // Unused.
  float fira     = NAN; // Unused.
  float fsh      = NAN; // Unused.
  float fgev     = NAN; // Unused.
  float sSoil    = NAN; // Unused.
  float tRad     = NAN; // Unused.
  float eDir     = NAN; // Evaporation rate from surface in millimeters of water equivalent per second.
  float runSrf   = NAN; // Infiltration excess runoff in millimeters of water per second.
  float runSub   = NAN; // Saturation excess runoff in millimeters of water per second.
  float sag      = NAN; // Unused.
  float albedo   = NAN; // Unused.
  
  // There is some complexity with qSnBot and the ponding variables.  The big picture is that they contain the flow out the bottom of the snowpack including
  // snowmelt plus ranfall that lands on the snowpack and passes through without freezing.  In short, the quantity of water that flowed out the bottom of the
  // snowpack is always equal to qSnBot * dt + ponding + ponding1 + ponding2.  However, this flow is put into one of the four different variables depending on
  // the situation.  If the multi-layer snow simulation is turned on at the end of the timestep then the snowmelt rate is put into qSnBot and the ponding
  // variables are set to zero.  If the multi-layer snow simulation is turned off at the end of the timestep then the snowmelt quantity is put into one of the
  // ponding variables and the other ponding variables and qSnBot are set to zero.  Which ponding variable gets the water also depends on the situation. If the
  // multi-layer snow simulation is turned off at the beginning and end of the timestep then the water is put in ponding.  If the multi-layer snow simulation
  // is turned on at the beginning of the timestep, but is off at the end of the timestep then the water is put either in ponding1 or ponding2 depending on the
  // reason why the multi-layer snow simulation was turned off.  If in an individual layer in the multi-layer snow simulation has snIce go below 0.1 mm of snow
  // water equivalent that layer is eliminated.  The water is added to a neighboring layer if possible, but if that was the last layer then the multi-layer
  // snow simulation is turned off, snEqv is set to snIce, and the water in snLiq is put in ponding1.  If the height of the entire snow pack, snowH, goes below
  // 0.025 m then the multi-layer snow simulation is turned off, snEqv is set to snIce, and the water in snLiq is put in ponding2.
  float qSnBot   = NAN; // Flow rate out the bottom of the snowpack in millimeters of water per second.
  float ponding  = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  float ponding1 = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  float ponding2 = NAN; // Flow out the bottom of the snowpack in millimeters of water.
  
  float t2m      = NAN; // Unused.
  float q2e      = NAN; // Unused.
  float emissi   = NAN; // Unused.
  float fpIce    = NAN; // Fraction of precipitation that is frozen, unitless.
  float ch2b     = NAN; // Unused.
  
  // Derived output variables.
  int   iSnowOriginal;             // Actual number of snow layers before timestep.
  float evaporationFromSurface;    // Quantity of evaporation from the surface in millimeters of water equivalent.  Positive means water evaporated off of the
                                   // surface.  Negative means water condensed on to the surface.  Surface evaporation sometimes comes from snow and is taken
                                   // out by Noah-MP, but if there is not enough snow we have to take the evaporation from the water in the ADHydro state
                                   // variables.
  float snowfall;                  // Quantity of snowfall in millimeters of water equivalent.  Must be non-negative.
  float snowmeltOnGround;          // Quantity of water that reaches the ground from the snow layer in millimeters of water.  Must be non-negative.
  float rainfall;                  // Quantity of rainfall in millimeters of water.  Must be non-negative.
  float rainfallInterceptedBySnow; // Quantity of rainfall intercepted by the snow layer in millimeters of water.  Must be non-negative.
  float rainfallOnGround;          // Quantity of rainfall that reaches the ground in millimeters of water.  Must be non-negative.
  float snEqvOriginal;             // Quantity of water in the snow layer(s) before timestep in millimeters of water equivalent.  snEqvO is not always set to
                                   // this value after the timestep.
  float snEqvShouldBe;             // If snEqv falls below 0.001 Noah-MP sets it to zero, or if it rises above 2000.0 Noah-MP sets it to 2000.0.  We don't want
                                   // this behavior so in this variable we calculate what snEqv should be and set it back.  If snEqv is not changed this
                                   // instead performs a mass balance check.
  float runoff;                    // Quantity of water that runs off from the soil in millimeters of water.  We don't use Noah-MP's infiltration and
                                   // groundwater simulation so this is merely for a mass balance check.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Variables used for assertions.
  int   iLocOriginal         = iLoc;
  int   jLocOriginal         = jLoc;
  float cosZOriginal         = cosZ;
  int   nSnowOriginal        = nSnow;
  int   nSoilOriginal        = nSoil;
  float dtOriginal           = dt;
  float sfcTmpOriginal       = evapoTranspirationForcing->sfcTmp;
  float sfcPrsOriginal       = evapoTranspirationForcing->sfcPrs;
  float uuOriginal           = evapoTranspirationForcing->uu;
  float vvOriginal           = evapoTranspirationForcing->vv;
  float q2Original           = evapoTranspirationForcing->q2;
  float solDnOriginal        = evapoTranspirationForcing->solDn;
  float prcpOriginal         = evapoTranspirationForcing->prcp;
  float lwDnOriginal         = evapoTranspirationForcing->lwDn;
  float tBotOriginal         = evapoTranspirationForcing->tBot;
  float zLvlOriginal         = evapoTranspirationForcing->dz8w;
  float fIceOldOriginal[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
  float zSoilOriginal[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];
  float soilMoistureOriginal = 0.0f; // Total soil moisture before timestep in millimeters of water.
  float soilMoistureNew      = 0.0f; // Total soil moisture after timestep in millimeters of water.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                                    
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0f <= cosZ && 1.0f >= cosZ))
    {
      CkError("ERROR in evapoTranspirationGlacier: cosZ must be greater than or equal to zero and less than or equal to one.\n");
      error = true;
    }
  
  if (!(0.0f < dt))
    {
      CkError("ERROR in evapoTranspirationGlacier: dt must be greater than zero.\n");
      error = true;
    }
  
  if (!(NULL != evapoTranspirationForcing))
    {
      CkError("ERROR in evapoTranspirationGlacier: evapoTranspirationForcing must not be NULL.\n");
      error = true;
    }

  if (!(NULL != evapoTranspirationState))
    {
      CkError("ERROR in evapoTranspirationGlacier: evapoTranspirationState must not be NULL.\n");
      error = true;
    }
  else
    {
      if (!(0.0f == evapoTranspirationState->canLiq))
        {
          CkError("ERROR in evapoTranspirationGlacier: canLiq must be zero.\n");
          error = true;
        }

      if (!(0.0f == evapoTranspirationState->canIce))
        {
          CkError("ERROR in evapoTranspirationGlacier: canIce must be zero.\n");
          error = true;
        }
    }
  
  if (!(NULL != surfacewaterAdd))
    {
      CkError("ERROR in evapoTranspirationGlacier: surfacewaterAdd must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *surfacewaterAdd = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evaporationFromSnow))
    {
      CkError("ERROR in evapoTranspirationGlacier: evaporationFromSnow must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *evaporationFromSnow = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evaporationFromGround))
    {
      CkError("ERROR in evapoTranspirationGlacier: evaporationFromGround must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *evaporationFromGround = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != waterError))
    {
      CkError("ERROR in evapoTranspirationGlacier: water error must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *waterError = 0.0f;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (!error)
    {
      error = checkEvapoTranspirationForcingStructInvariant(evapoTranspirationForcing);
    }
  
  if (!error)
    {
      error = checkEvapoTranspirationStateStructInvariant(evapoTranspirationState);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  if (!error)
    {
      // Set variables that require error checking of pointers first.
      iSnowOriginal  = evapoTranspirationState->iSnow;
      snEqvOriginal  = evapoTranspirationState->snEqv;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          fIceOldOriginal[ii] = evapoTranspirationState->fIceOld[ii];
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Set zSoil from zSnso.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          if (0 > evapoTranspirationState->iSnow)
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] -
                          evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1];
            }
          else
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS];
            }
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          zSoilOriginal[ii] = zSoil[ii];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Calculate fIce at the beginning of the timestep.
      for (ii = evapoTranspirationState->iSnow + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          fIce[ii] = evapoTranspirationState->snIce[ii] / (evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]);
        }

      // Set Noah-MP globals.
      REDPRM(&vegType, &soilType, &slopeType, zSoil, &nSoil, &isUrban);
      
      // Set water content of the soil layers.  In glaciers the watertable is always at the surface and everything is saturated and frozen.  The global
      // variable NOAHMP_POROSITY gets set in REDPRM so we can't do this before here.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          sh2o[ii] = 0.0f;
          // FIXLATER Soil type 16 'OTHER(land-ice)' sets NOAHMP_POROSITY to 0.421, but after calling NOAHMP_GLACIER smc gets set to 1.0.  So I assume Noah-MP wants smc set to
          // 1.0 for glaciers.  Maybe we should use soil type 14 'WATER' like we do in evapoTranspirationWater.
          //smc[ii]  = NOAHMP_POROSITY;
          smc[ii]  = 1.0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // Calculate soil moisture at the beginning of the timestep.
          if (0 == ii)
            {
              soilMoistureOriginal += smc[ii] * -zSoil[ii] * 1000.0f; // * 1000.0f because zSoil is in meters and soilMoistureOriginal is in millimeters.
            }
          else
            {
              soilMoistureOriginal += smc[ii] * (zSoil[ii - 1] - zSoil[ii]) * 1000.0f;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }
      
      // Run Noah-MP.
      NOAHMP_GLACIER(&iLoc, &jLoc, &cosZ, &nSnow, &nSoil, &dt, &evapoTranspirationForcing->sfcTmp, &evapoTranspirationForcing->sfcPrs,
                     &evapoTranspirationForcing->uu, &evapoTranspirationForcing->vv, &evapoTranspirationForcing->q2, &evapoTranspirationForcing->solDn,
                     &evapoTranspirationForcing->prcp, &evapoTranspirationForcing->lwDn, &evapoTranspirationForcing->tBot, &evapoTranspirationForcing->dz8w,
                     evapoTranspirationState->fIceOld, zSoil, &qSnow, &evapoTranspirationState->snEqvO, &evapoTranspirationState->albOld,
                     &evapoTranspirationState->cm, &evapoTranspirationState->ch, &evapoTranspirationState->iSnow, &evapoTranspirationState->snEqv, smc,
                     evapoTranspirationState->zSnso, &evapoTranspirationState->snowH, evapoTranspirationState->snIce, evapoTranspirationState->snLiq,
                     &evapoTranspirationState->tg, evapoTranspirationState->stc, sh2o, &evapoTranspirationState->tauss, &qsfc, &fsa, &fsr, &fira, &fsh, &fgev,
                     &sSoil, &tRad, &eDir, &runSrf, &runSub, &sag, &albedo, &qSnBot, &ponding, &ponding1, &ponding2, &t2m, &q2e, &emissi, &fpIce, &ch2b);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Verify that the input variables have not changed.
      CkAssert(iLocOriginal == iLoc && jLocOriginal == jLoc && cosZOriginal == cosZ && nSnowOriginal == nSnow && nSoilOriginal == nSoil && dtOriginal == dt &&
               sfcTmpOriginal == evapoTranspirationForcing->sfcTmp && sfcPrsOriginal == evapoTranspirationForcing->sfcPrs &&
               uuOriginal == evapoTranspirationForcing->uu && vvOriginal == evapoTranspirationForcing->vv && q2Original == evapoTranspirationForcing->q2 &&
               solDnOriginal == evapoTranspirationForcing->solDn && prcpOriginal == evapoTranspirationForcing->prcp &&
               lwDnOriginal == evapoTranspirationForcing->lwDn && tBotOriginal == evapoTranspirationForcing->tBot &&
               zLvlOriginal == evapoTranspirationForcing->dz8w && fIceOldOriginal[0] == evapoTranspirationState->fIceOld[0] &&
               fIceOldOriginal[1] == evapoTranspirationState->fIceOld[1] && fIceOldOriginal[2] == evapoTranspirationState->fIceOld[2] &&
               zSoilOriginal[0] == zSoil[0] && zSoilOriginal[1] == zSoil[1] && zSoilOriginal[2] == zSoil[2] && zSoilOriginal[3] == zSoil[3]);
      
      // Snowmelt that goes into ponding is taken from snEqv before its original value is stored in snEqvO.  This is only true for ponding, not ponding1 or
      // ponding2.  Therefore, if ponding is not equal to zero, snEqvOriginal will not equal snEqvO.  Instead, snEqvOriginal will equal snEqvO + ponding, and
      // we have to check for epsilon equal because of roundoff error. If ponding is zero then snEqvOriginal and snEqvO will be exactly equal.
      if (0.0f != ponding)
        {
          CkAssert(epsilonEqual(snEqvOriginal, evapoTranspirationState->snEqvO + ponding));
        }
      else
        {
          CkAssert(snEqvOriginal == evapoTranspirationState->snEqvO);
        }
      
      // Verify that the fraction of the precipitation that falls as snow is between 0 and 1, the snowfall rate below the canopy is not negative, and the
      // snowmelt out the bottom of the snowpack is not negative.
      CkAssert(0.0f <= fpIce && 1.0f >= fpIce && 0.0f <= qSnow && 0.0f <= qSnBot && 0.0f <= ponding && 0.0f <= ponding1 && 0.0f <= ponding2);
      
      // Verify that there is no snowfall interception in the non-existant canopy.
      CkAssert(evapoTranspirationForcing->prcp * fpIce == qSnow);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Store fIce from the beginning of the timestep in fIceOld.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
        {
          if (ii >= iSnowOriginal + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS)
            {
              evapoTranspirationState->fIceOld[ii] = fIce[ii];
            }
          else
            {
              evapoTranspirationState->fIceOld[ii] = 0.0;
            }
        }
         
      // Calculate derived output variables.
      snowfall         = evapoTranspirationForcing->prcp * dt * fpIce;
      snowmeltOnGround = qSnBot * dt + ponding + ponding1 + ponding2;
      rainfall         = evapoTranspirationForcing->prcp * dt - snowfall;
      
      // If there is a snow layer at the end of the timestep it intercepts all of the rainfall.
      if (0 > evapoTranspirationState->iSnow)
        {
          rainfallInterceptedBySnow = rainfall;
          rainfallOnGround          = 0.0;
        }
      else
        {
          rainfallInterceptedBySnow = 0.0;
          rainfallOnGround          = rainfall;
        }
      
      evaporationFromSurface = eDir * dt;

      // Surface condensation is added to the snow layer if any snow exists or otherwise added to the ground as water.  Surface evaporation is taken first from
      // the snow layer up to the amount available, and then any remaining is taken from the ground.
      if (0.0f > evaporationFromSurface)
        {
          if (0.0f < evapoTranspirationState->snEqvO)
            {
              *evaporationFromSnow   = evaporationFromSurface;
              *evaporationFromGround = 0.0f;
            }
          else
            {
              *evaporationFromSnow   = 0.0f;
              *evaporationFromGround = evaporationFromSurface;
            }
        }
      // In NOAHMP_GLACIER, unlike NOAHMP_SFLX, evaporationFromGround is taken after precipitation is added to the snow layer.  So if it is snowing, the
      // maximum amount of water available for evaporation from snow is different.  I am unsure whether snowmeltOnGround is taken before or after evaporation.
      // I am doing it like this for now, and we'll see if any assertion is violated.
      else if (evaporationFromSurface <= evapoTranspirationState->snEqvO + snowfall + rainfallInterceptedBySnow - snowmeltOnGround)
        {
          *evaporationFromSnow   = evaporationFromSurface;
          *evaporationFromGround = 0.0f;
        }
      else
        {
          *evaporationFromSnow   = evapoTranspirationState->snEqvO + snowfall + rainfallInterceptedBySnow - snowmeltOnGround;
          *evaporationFromGround = evaporationFromSurface - *evaporationFromSnow;
        }
      
      // Do a mass balance check for the snowpack.
      snEqvShouldBe = snEqvOriginal + snowfall + rainfallInterceptedBySnow - *evaporationFromSnow - snowmeltOnGround;
      
      // snEqvShouldBe can go negative due to round-off error.  This is not a mass balance bug, but I can't proceed with snEqvShouldBe negative.
      // However, setting snEqvShouldBe to zero for negative roundoff errors, but not positive roundoff errors creates a water creating bias so it is
      // correct to record the created water in waterError.
      if (0.0f > snEqvShouldBe)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          *waterError  -= snEqvShouldBe;
          snEqvShouldBe = 0.0f;
        }
      
      runoff = (runSrf + runSub) * dt;
      
      // If snEqv falls below 0.001 mm, or snowH falls below 1e-6 m then Noah-MP sets both to zero and the water is lost.  If snEqv grows above 2000 mm then
      // Noah-MP sets it to 2000 and the water is added to runSub as glacier flow.  We are putting the water back.
      if (0.0 == evapoTranspirationState->snEqv)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(0.001f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snEqv = snEqvShouldBe;
          evapoTranspirationState->snowH = snEqvShouldBe / 1000.0f; // Divide by one thousand to convert from millimeters to meters.
        }
      else if (2000.0f < snEqvShouldBe)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(2000.0f, evapoTranspirationState->snEqv));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snowH += snEqvShouldBe - evapoTranspirationState->snEqv / 1000.0f; // Divide by one thousand to convert from millimeters to meters.
          
          if (0 > evapoTranspirationState->iSnow)
            {
              evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] = -evapoTranspirationState->snowH;
              
              for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
                {
                  evapoTranspirationState->zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] =
                      evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] + zSoil[ii];
                }
              
              evapoTranspirationState->snIce[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1] += snEqvShouldBe - evapoTranspirationState->snEqv;
            }
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(evapoTranspirationState->snEqv + runoff, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          // Take the glacier flow back from runSub.
          runoff -= snEqvShouldBe - evapoTranspirationState->snEqv;
          
          // Put the water back in snEqv.
          evapoTranspirationState->snEqv = snEqvShouldBe;
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      else
        {
          CkAssert(epsilonEqual(evapoTranspirationState->snEqv, snEqvShouldBe));
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      CkAssert(!checkEvapoTranspirationStateStructInvariant(evapoTranspirationState));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      
      *surfacewaterAdd = snowmeltOnGround + rainfallOnGround;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0f <= *surfacewaterAdd);
      
      // Calculate soil moisture at the end of the timestep.
      for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
        {
          if (0 == ii)
            {
              soilMoistureNew += smc[ii] * -zSoil[ii] * 1000.0f; // * 1000.0f because zSoil is in meters and soilMoistureNew is in millimeters.
            }
          else
            {
              soilMoistureNew += smc[ii] * (zSoil[ii - 1] - zSoil[ii]) * 1000.0f;
            }
        }
      
      // Verify that soil moisture balances.  Epsilon needs to be based on the largest value used to calculate it.  However, there was a case where
      // soilMoistureOriginal was large with a lot of water subtracted from it and soilMoistureNew was small.  The values passed to the first epsilonEqual were
      // both small so epsilon was small and this check failed even though the difference was okay if epsilon were based on soilMoistureOriginal.  So I added
      // the second check That would use epsilon based on soilMoistureOriginal.
      CkAssert(epsilonEqual(soilMoistureOriginal - *evaporationFromGround + *surfacewaterAdd - runoff,  soilMoistureNew) ||
               epsilonEqual(soilMoistureOriginal,  *evaporationFromGround - *surfacewaterAdd + runoff + soilMoistureNew));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    } // End if (!error).
  
  return error;
}

bool checkEvapoTranspirationForcingStructInvariant(EvapoTranspirationForcingStruct* evapoTranspirationForcing)
{
  bool error = false; // Error flag.
  
  if (!(0.0f < evapoTranspirationForcing->dz8w))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: dz8w must be greater than zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationForcing->sfcTmp))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: sfcTmp must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(-70.0f + ZERO_C_IN_KELVIN <= evapoTranspirationForcing->sfcTmp))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationForcingStructInvariant: sfcTmp below -70 degrees C.\n");
        }
    }
  else if (!(70.0f + ZERO_C_IN_KELVIN >= evapoTranspirationForcing->sfcTmp))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationForcingStructInvariant: sfcTmp above 70 degrees C.\n");
        }
    }

  if (!(0.0f <= evapoTranspirationForcing->sfcPrs))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: sfcPrs must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(35000.0f <= evapoTranspirationForcing->sfcPrs))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationForcingStructInvariant: sfcPrs below 35 kPa.\n");
        }
    }

  if (!(0.0f <= evapoTranspirationForcing->psfc))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: psfc must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(35000.0f <= evapoTranspirationForcing->psfc))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationForcingStructInvariant: psfc below 35 kPa.\n");
        }
    }
  
  if (!(100.0f >= fabs(evapoTranspirationForcing->uu)))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationForcingStructInvariant: magnitude of uu greater than 100 m/s.\n");
        }
    }
  
  if (!(100.0f >= fabs(evapoTranspirationForcing->vv)))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationForcingStructInvariant: magnitude of vv greater than 100 m/s.\n");
        }
    }
  
  if (!(0.0f <= evapoTranspirationForcing->q2 && 1.0f >= evapoTranspirationForcing->q2))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: q2 must be greater than or equal to zero and less than or equal to one.\n");
      error = true;
    }
  
  if (!(0.0f <= evapoTranspirationForcing->qc && 1.0f >= evapoTranspirationForcing->qc))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: qc must be greater than or equal to zero and less than or equal to one.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationForcing->solDn))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: solDn must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationForcing->lwDn))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: lwDn must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationForcing->prcp))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: prcp must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationForcing->tBot))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: tBot must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(-70.0f + ZERO_C_IN_KELVIN <= evapoTranspirationForcing->tBot))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationForcingStructInvariant: tBot below -70 degrees C.\n");
        }
    }
  else if (!(70.0f + ZERO_C_IN_KELVIN >= evapoTranspirationForcing->tBot))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationForcingStructInvariant: tBot above 70 degrees C.\n");
        }
    }

  if (!(0.0f <= evapoTranspirationForcing->pblh))
    {
      CkError("ERROR in checkEvapoTranspirationForcingStructInvariant: pblh must be greater than or equal to zero.\n");
      error = true;
    }
  
  return error;
}

bool checkEvapoTranspirationStateStructInvariant(EvapoTranspirationStateStruct* evapoTranspirationState)
{
  bool  error          = false; // Error flag.
  int   ii;                     // Loop counter.
  float totalSnowWater = 0.0f;  // Total water in all snow layers in millimeters of water equivalent.

  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
    {
      if (!(0.0f <= evapoTranspirationState->fIceOld[ii] && evapoTranspirationState->fIceOld[ii] <= 1.0f))
        {
          CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snow layer %d: fIceOld must be greater than or equal to zero and less than or equal "
                  "to one.\n", ii);
          error = true;
        }
    }

  if (!(0.0f <= evapoTranspirationState->albOld && evapoTranspirationState->albOld <= 1.0f))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: albOld must be greater than or equal to zero and less than or equal to one.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->snEqvO))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: snEqvO must be greater than or equal to zero.\n");
      error = true;
    }

  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
    {
      if (ii < evapoTranspirationState->iSnow + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS)
        {
          // Snow layer does not exist.
          if (!(0.0f == evapoTranspirationState->stc[ii]))
            {
              CkError("ERROR in checkEvapoTranspirationStateStructInvariant, non-existant snow layer %d: stc must be equal to zero.\n", ii);
              error = true;
            }
        }
      else
        {
          // Snow or soil layer does exist.
          if (!(0.0f <= evapoTranspirationState->stc[ii]))
            {
              CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snow/soil layer %d: stc must be greater than or equal to zero.\n", ii);
              error = true;
            }
          else if (!(-70.0f + ZERO_C_IN_KELVIN <= evapoTranspirationState->stc[ii]))
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in checkEvapoTranspirationStateStructInvariant, snow/soil layer %d: stc below -70 degrees C.\n", ii);
                }
            }
          else if (!(70.0f + ZERO_C_IN_KELVIN >= evapoTranspirationState->stc[ii]))
            {
              if (2 <= ADHydro::verbosityLevel)
                {
                  CkError("WARNING in checkEvapoTranspirationStateStructInvariant, snow/soil layer %d: stc above 70 degrees C.\n", ii);
                }
            }
        }
    }

  if (!(0.0f <= evapoTranspirationState->tah))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: tah must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(-70.0f + ZERO_C_IN_KELVIN <= evapoTranspirationState->tah))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tah below -70 degrees C.\n");
        }
    }
  else if (!(70.0f + ZERO_C_IN_KELVIN >= evapoTranspirationState->tah))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tah above 70 degrees C.\n");
        }
    }

  if (!(0.0f <= evapoTranspirationState->eah))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: eah must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->fWet && evapoTranspirationState->fWet <= 1.0f))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: fWet must be greater than or equal to zero and less than or equal to one.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->canLiq))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: canLiq must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->canIce))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: canIce must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->tv))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: tv must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(-70.0f + ZERO_C_IN_KELVIN <= evapoTranspirationState->tv))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tv below -70 degrees C.\n");
        }
    }
  else if (!(70.0f + ZERO_C_IN_KELVIN >= evapoTranspirationState->tv))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tv above 70 degrees C.\n");
        }
    }

  if (!(0.0f <= evapoTranspirationState->tg))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: tg must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(-70.0f + ZERO_C_IN_KELVIN <= evapoTranspirationState->tg))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tg below -70 degrees C.\n");
        }
    }
  else if (!(70.0f + ZERO_C_IN_KELVIN >= evapoTranspirationState->tg))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tg above 70 degrees C.\n");
        }
    }

  if (!(-EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS <= evapoTranspirationState->iSnow && evapoTranspirationState->iSnow <= 0))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: iSnow must be greater than or equal to minus three and less than or equal to zero.\n");
      error = true;
    }

  for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii++)
    {
      if (ii < evapoTranspirationState->iSnow + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS)
        {
          // Snow layer does not exist.
          if (!(0.0f == evapoTranspirationState->zSnso[ii]))
            {
              CkError("ERROR in checkEvapoTranspirationStateStructInvariant, non-existant snow layer %d: zSnso must be equal to zero.\n", ii);
              error = true;
            }

          if (!(0.0f == evapoTranspirationState->snIce[ii]))
            {
              CkError("ERROR in checkEvapoTranspirationStateStructInvariant, non-existant snow layer %d: snIce must be equal to zero.\n", ii);
              error = true;
            }

          if (!(0.0f == evapoTranspirationState->snLiq[ii]))
            {
              CkError("ERROR in checkEvapoTranspirationStateStructInvariant, non-existant snow layer %d: snLiq must be equal to zero.\n", ii);
              error = true;
            }
        }
      else
        {
          // Snow layer does exist.
          if (ii == evapoTranspirationState->iSnow + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS)
            {
              // Top snow layer.
              if (!(0.0f > evapoTranspirationState->zSnso[ii]))
                {
                  CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snow layer %d: zSnso must be less than zero.\n", ii);
                  error = true;
                }
            }
          else
            {
              // Not top snow layer.
              if (!(evapoTranspirationState->zSnso[ii - 1] > evapoTranspirationState->zSnso[ii]))
                {
                  CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snow layer %d: zSnso must be less than the layer above it.\n", ii);
                  error = true;
                }
            }

          if (!(0.0f < evapoTranspirationState->snIce[ii]))
            {
              CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snow layer %d: snIce must be greater than zero.\n", ii);
              error = true;
            }

          if (!(0.0f <= evapoTranspirationState->snLiq[ii]))
            {
              CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snow layer %d: snLiq must be greater than or equal to zero.\n", ii);
              error = true;
            }

          totalSnowWater += evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii];
        }
    }

  for (ii = EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
    {
      if (!(evapoTranspirationState->zSnso[ii - 1] > evapoTranspirationState->zSnso[ii]))
        {
          CkError("ERROR in checkEvapoTranspirationStateStructInvariant, soil layer %d: zSnso must be less than the layer above it.\n",
                  ii - EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS);
          error = true;
        }
    }

  if (!(0.0f <= evapoTranspirationState->snowH))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: snowH must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->snEqv))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: snEqv must be greater than or equal to zero.\n");
      error = true;
    }

  // If multi-layer snow simulation is turned on snowH and snEqv must be consistent with zSnso, snIce, and snLiq.
  if (0 > evapoTranspirationState->iSnow)
    {
      if (!epsilonEqual(-evapoTranspirationState->zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1], evapoTranspirationState->snowH))
        {
          CkError("ERROR in checkEvapoTranspirationStateStructInvariant: snowH must be consistent with zSnso.\n");
          error = true;
        }

      if (!epsilonEqual(totalSnowWater, evapoTranspirationState->snEqv))
        {
          CkError("ERROR in checkEvapoTranspirationStateStructInvariant: snEqv must be consistent with snIce and snLiq.\n");
          error = true;
        }
    }

  if (!(0.0f <= evapoTranspirationState->lfMass))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: lfMass must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->rtMass))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: rtMass must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->stMass))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: stMass must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->wood))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: wood must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->stblCp))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: stblCp must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->fastCp))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: fastCp must be greater than or equal to zero.\n");
      error = true;
    }

  if (!(0.0f <= evapoTranspirationState->lai))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: lai must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(6.0f >= evapoTranspirationState->lai))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationStateStructInvariant: lai above six.\n");
        }
    }

  if (!(0.0f <= evapoTranspirationState->sai))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: sai must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(1.6f >= evapoTranspirationState->sai))
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in checkEvapoTranspirationStateStructInvariant: sai above 1.6.\n");
        }
    }

  return error;
}
