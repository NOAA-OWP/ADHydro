#include "evapo_transpiration.h"
#include "all.h"
#include <math.h>

#ifdef INTEL_COMPILER
#define READ_MP_VEG_PARAMETERS noahmp_veg_parameters_mp_read_mp_veg_parameters_
#define SOIL_VEG_GEN_PARM      module_sf_noahmpdrv_mp_soil_veg_gen_parm_
#define NOAHMP_OPTIONS         noahmp_routines_mp_noahmp_options_
#define NOAHMP_OPTIONS_GLACIER noahmp_glacier_routines_mp_noahmp_options_glacier_
#define REDPRM                 noahmp_routines_mp_redprm_
#define NOAHMP_SFLX            noahmp_routines_mp_noahmp_sflx_
#define NOAHMP_GLACIER         noahmp_glacier_routines_mp_noahmp_glacier_
#else // INTEL_COMPILER
#define READ_MP_VEG_PARAMETERS __noahmp_veg_parameters_MOD_read_mp_veg_parameters
#define SOIL_VEG_GEN_PARM      __module_sf_noahmpdrv_MOD_soil_veg_gen_parm
#define NOAHMP_OPTIONS         __noahmp_routines_MOD_noahmp_options
#define NOAHMP_OPTIONS_GLACIER __noahmp_glacier_routines_MOD_noahmp_options_glacier
#define REDPRM                 __noahmp_routines_MOD_redprm
#define NOAHMP_SFLX            __noahmp_routines_MOD_noahmp_sflx
#define NOAHMP_GLACIER         __noahmp_glacier_routines_MOD_noahmp_glacier
#endif // INTEL_COMPILER

extern "C" void READ_MP_VEG_PARAMETERS(const char* landUse, const char* mpTableFile, size_t landUseSize, size_t mpTableFileSize);
extern "C" void SOIL_VEG_GEN_PARM(const char* landUse, const char* soil, const char* vegParmFile, const char* soilParmFile, const char* genParmFile,
                                  size_t landUseSize, size_t soilSize, size_t vegParmFileSize, size_t soilParmFileSize, size_t genParmFileSize);
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
                            float* evc, float* chLeaf, float* chuc, float* chv2, float* chb2, float* fpIce);
extern "C" void NOAHMP_GLACIER(int* iLoc, int* jLoc, float* cosZ, int* nSnow, int* nSoil, float* dt, float* sfcTmp, float* sfcPrs, float* uu, float* vv,
                               float* q2, float* solDn, float* prcp, float* lwDn, float* tBot, float* zLvl, float* fIceOld, float* zSoil, float* qSnow,
                               float* snEqvO, float* albOld, float* cm, float* ch, int* iSnow, float* snEqv, float* smc, float* zSnso, float* snowH,
                               float* snIce, float* snLiq, float* tg, float* stc, float* sh2o, float* tauss, float* qsfc, float* fsa, float* fsr, float* fira,
                               float* fsh, float* fgev, float* sSoil, float* trad, float* eDir, float* runSrf, float* runSub, float* sag, float* albedo,
                               float* qsnBot, float* ponding, float* ponding1, float* ponding2, float* t2m, float* q2e, float* emissi, float* fpIce,
                               float* ch2b);

bool evapoTranspirationInit(const char* directory)
{
  bool        error        = false;  // Error flag.
  const char* landUse      = "USGS"; // Land use data set.
  const char* soil         = "STAS"; // Soil type data set.
  char*       mpTableFile  = NULL;   // File name for MPTABLE.TBL file.
  char*       vegParmFile  = NULL;   // File name for VEGPARM.TBL file.
  char*       soilParmFile = NULL;   // File name for SOILPARM.TBL file.
  char*       genParmFile  = NULL;   // File name for GENPARM.TBL file.
  size_t      fileStringSize;        // Size of buffer allocated for file name strings.
  size_t      numPrinted;            // Used to check that snprintf printed the correct number of characters.
  
  // =====================================options for different schemes================================
  // From module_sf_noahmplsm.F
  
  // options for dynamic vegetation:
  // 1 -> off (use table LAI; use FVEG = SHDFAC from input)
  // 2 -> on (together with OPT_CRS = 1)
  // 3 -> off (use table LAI; calculate FVEG)
  // 4 -> off (use table LAI; use maximum vegetation fraction)

  int dveg = 4;

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
  if (!(NULL != directory))
    {
      CkError("ERROR in evapoTranspirationInit: directory must not be null.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // Allocate space for file name strings.
      fileStringSize = strlen(directory) + strlen("/SOILPARM.TBL") + 1; // The longest file name is SOILPARM.TBL.  +1 for null terminating character.
      mpTableFile    = new char[fileStringSize];
      vegParmFile    = new char[fileStringSize];
      soilParmFile   = new char[fileStringSize];
      genParmFile    = new char[fileStringSize];

      // Create file name.
      numPrinted = snprintf(mpTableFile, fileStringSize, "%s/MPTABLE.TBL", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/MPTABLE.TBL") == numPrinted && numPrinted < fileStringSize))
        {
          CkError("ERROR in evapoTranspirationInit: incorrect return value of snprintf when generating mp table file name %s.  "
                  "%d should be equal to %d and less than %d.\n", mpTableFile, numPrinted, strlen(directory) + strlen("/MPTABLE.TBL"), fileStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(vegParmFile, fileStringSize, "%s/VEGPARM.TBL", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/VEGPARM.TBL") == numPrinted && numPrinted < fileStringSize))
        {
          CkError("ERROR in evapoTranspirationInit: incorrect return value of snprintf when generating veg parm file name %s.  "
                  "%d should be equal to %d and less than %d.\n", vegParmFile, numPrinted, strlen(directory) + strlen("/VEGPARM.TBL"), fileStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(soilParmFile, fileStringSize, "%s/SOILPARM.TBL", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/SOILPARM.TBL") == numPrinted && numPrinted < fileStringSize))
        {
          CkError("ERROR in evapoTranspirationInit: incorrect return value of snprintf when generating soil parm file name %s.  "
                  "%d should be equal to %d and less than %d.\n", soilParmFile, numPrinted, strlen(directory) + strlen("/SOILPARM.TBL"), fileStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(genParmFile, fileStringSize, "%s/GENPARM.TBL", directory);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(strlen(directory) + strlen("/GENPARM.TBL") == numPrinted && numPrinted < fileStringSize))
        {
          CkError("ERROR in evapoTranspirationInit: incorrect return value of snprintf when generating veg parm file name %s.  "
                  "%d should be equal to %d and less than %d.\n", genParmFile, numPrinted, strlen(directory) + strlen("/GENPARM.TBL"), fileStringSize);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      READ_MP_VEG_PARAMETERS(landUse, mpTableFile, strlen(landUse), strlen(mpTableFile));
      SOIL_VEG_GEN_PARM(landUse, soil, vegParmFile, soilParmFile, genParmFile, strlen(landUse), strlen(soil), strlen(vegParmFile), strlen(soilParmFile),
                        strlen(genParmFile));
      NOAHMP_OPTIONS(&dveg, &optCrs, &optBtr, &optRun, &optSfc, &optFrz, &optInf, &optRad, &optAlb, &optSnf, &optTbot, &optStc);
      NOAHMP_OPTIONS_GLACIER(&dveg, &optCrs, &optBtr, &optRun, &optSfc, &optFrz, &optInf, &optRad, &optAlb, &optSnf, &optTbot, &optStc);
    }
  
  if (NULL != mpTableFile)
    {
      delete[] mpTableFile;
    }
  
  if (NULL != vegParmFile)
    {
      delete[] vegParmFile;
    }
  
  if (NULL != soilParmFile)
    {
      delete[] soilParmFile;
    }
  
  if (NULL != genParmFile)
    {
      delete[] genParmFile;
    }
  
  return error;
}

bool evapoTranspirationSoil(int vegType, int soilType, float lat, int yearLen, float julian, float cosZ, float dt, float dx, float dz8w, float shdFac,
                            float shdMax, float smcEq[4], float sfcTmp, float sfcPrs, float psfc, float uu, float vv, float q2, float qc, float solDn,
                            float lwDn, float prcp, float tBot, float pblh, float sh2o[4], float smc[4], float zwt, float wa, float wt,  float smcwtd,
                            EvapoTranspirationStateStruct* evapoTranspirationState, float* surfacewaterAdd, float* evaporationFromCanopy,
                            float* evaporationFromSnow, float* evaporationFromGround, float* transpiration, float* waterError)
{
  bool  error = false; // Error flag.
  int   ii;            // Loop counter.
  float epsilon;       // For near equality testing.
  
  // Input parameters for redprm function.  Some are also used for sflx function.
  int   slopeType = 8; // I just arbitrarily chose a slope type with zero slope.  I think slope is only used to calculate runoff, which we ignore.
  float zSoil[4];      // Layer bottom depth in meters from soil surface of each soil layer.  Values are set below from zSnso.
  int   nSoil     = 4; // Always use four soil layers.
  int   isUrban   = 1; // USGS vegetation type for urban land.
  
  // Input parameters to sflx function.
  int   iLoc    = 1;                // Grid location index, unused.
  int   jLoc    = 1;                // Grid location index, unused.
  int   nSnow   = 3;                // Maximum number of snow layers.  Always pass 3.
  int   ice     = 0;                // Flag to indicate permanent ice cover, 0 for no, 1 for yes.  Always pass 0.  If permanent ice cover call
                                    // evapoTranspirationGlacier instead.
  int   ist     = 1;                // Flag to indicate permanent water cover, 1 for soil, 2 for lake.  Always pass 1.  If permanent water cover call
                                    // evapoTranspirationWater instead.
  int   isc     = 4;                // Soil color type, 1 for lightest to 8 for darkest.  Always pass 4 unless we find a data source for soil color.
  int   iz0tlnd = 0;                // Unused.
  float co2Air  = 0.0004f * sfcPrs; // CO2 partial pressure in Pascal at surface.  Always pass 400 parts per million of surface pressure.
  float o2Air   = 0.21f * sfcPrs;   // O2 partial pressure in Pascal at surface.  Always pass 21 percent of surface pressure.
  float folN    = 3.0f;             // Foliage nitrogen percentage, 0.0 to 100.0.  Always pass 3.0 unless we find a data source for foliage nitrogen.
  float fIce[3];                    // Frozen fraction of each snow layer, unitless.  Values are set below from snIce and snLiq.
  float zLvl    = dz8w;             // Thickness in meters of lowest atmosphere layer in forcing data.  Redundant with dz8w.
  
  // Input/output parameters to sflx function.
  float qsfc   = q2;   // Water vapor mixing ratio at middle of lowest atmosphere layer in forcing data, unitless.  Redundant with q2.
  float qSnow  = NAN;  // This is actually an output only variable.  Snowfall rate below the canopy in millimeters of water equivalent per second.
  float wsLake = 0.0f; // Water stored in lakes in millimeters of water.  Because we separate waterbodies from the mesh locations with a soil surface have no
                       // included lake storage.
  
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
  float runSrf   = NAN; // Unused.
  float runSub   = NAN; // Unused.
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
  float qSnBot   = NAN; // Snowmelt rate from the bottom of the snow pack in millimeters of water per second.
  float ponding  = NAN; // Unused.
  float ponding1 = NAN; // Unused.
  float ponding2 = NAN; // Unused.
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
  
  // Derived output variables.
  float canIceOriginal = evapoTranspirationState->canIce; // Quantity of canopy ice before timestep in millimeters of water equivalent.
  float changeInCanopyIce;                                // Change in canopy ice during timestep in millimeters of water equivalent.  Positive means the
                                                          // amount of canopy ice increased.
  float canLiqOriginal = evapoTranspirationState->canLiq; // Quantity of canopy liquid before timestep in millimeters of water.
  float changeInCanopyLiquid;                             // Change in canopy liquid during timestep in millimeters of water.  Positive means the
                                                          // amount of canopy liquid increased.
  int   iSnowOriginal  = evapoTranspirationState->iSnow;  // Actual number of snow layers before timestep.
  float evaporationFromSurface;                           // Quantity of evaporation from the surface in millimeters of water equivalent.  Positive means water
                                                          // evaporated off of the surface.  Negative means water condensed on to the surface.
                                                          // Surface evaporation sometimes comes from snow and is taken out by Noah-MP, but if there is not
                                                          // enough snow we have to take the evaporation from the water in the ADHydro state variables.
  float snowfallAboveCanopy;                              // Quantity of snowfall above the canopy in millimeters of water equivalent.  Must be non-negative.
  float snowfallInterceptedByCanopy;                      // Quantity of snowfall intercepted by the canopy in millimeters of water equivalent.
                                                          // Can be negative if snow is falling off of the canopy.
  float snowfallBelowCanopy;                              // Quantity of snowfall below the canopy in millimeters of water equivalent.  Must be non-negative.
  float snowmeltOnGround;                                 // Quantity of water that reaches the ground from the snow layer in millimeters of water.  Must be
                                                          // non-negative.
  float rainfallAboveCanopy;                              // Quantity of rainfall above the canopy in millimeters of water.  Must be non-negative.
  float rainfallInterceptedByCanopy;                      // Quantity of rainfall intercepted by the canopy in millimeters of water.
                                                          // Can be negative if rain is dripping from the canopy.
  float rainfallBelowCanopy;                              // Quantity of rainfall below the canopy in millimeters of water.  Must be non-negative.
  float rainfallInterceptedBySnow;                        // Quantity of rainfall intercepted by the snow layer in millimeters of water.  Must be non-negative.
  float rainfallOnGround;                                 // Quantity of rainfall that reaches the ground in millimeters of water.  Must be non-negative.
  float snEqvShouldBe;                                    // If snEqv falls below 0.001 Noah-MP sets it to zero, or if it rises above 2000.0 Noah-MP sets it to
                                                          // 2000.0.  We don't want this behavior so in this variable we calculate what snEqv should be and set
                                                          // it back.  If snEqv is not changed this instead performs a mass balance check.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Variables used for assertions.
  int   iLocOriginal       = iLoc;
  int   jLocOriginal       = jLoc;
  float latOriginal        = lat;
  int   yearLenOriginal    = yearLen;
  float julianOriginal     = julian;
  float cosZOriginal       = cosZ;
  float dtOriginal         = dt;
  float dxOriginal         = dx;
  float dz8wOriginal       = dz8w;
  int   nSoilOriginal      = nSoil;
  float zSoilOriginal[4];
  int   nSnowOriginal      = nSnow;
  float shdFacOriginal     = shdFac;
  float shdMaxOriginal     = shdMax;
  int   vegTypeOriginal    = vegType;
  int   isUrbanOriginal    = isUrban;
  int   iceOriginal        = ice;
  int   istOriginal        = ist;
  int   iscOriginal        = isc;
  float smcEqOriginal[4]   = {smcEq[0], smcEq[1], smcEq[2], smcEq[3]};
  int   iz0tlndOriginal    = iz0tlnd;
  float sfcTmpOriginal     = sfcTmp;
  float sfcPrsOriginal     = sfcPrs;
  float psfcOriginal       = psfc;
  float uuOriginal         = uu;
  float vvOriginal         = vv;
  float q2Original         = q2;
  float qcOriginal         = qc;
  float solDnOriginal      = solDn;
  float lwDnOriginal       = lwDn;
  float prcpOriginal       = prcp;
  float tBotOriginal       = tBot;
  float co2AirOriginal     = co2Air;
  float o2AirOriginal      = o2Air;
  float folNOriginal       = folN;
  float fIceOldOriginal[3] = {evapoTranspirationState->fIceOld[0], evapoTranspirationState->fIceOld[1], evapoTranspirationState->fIceOld[2]};
  float pblhOriginal       = pblh;
  float zLvlOriginal       = zLvl;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(1 <= vegType && 27 >= vegType))
    {
      CkError("ERROR in evapoTranspirationSoil: USGS vegetation type must be greater than or equal to 1 and less than or equal to 27.\n");
      error = true;
    }
  
  if (!(1 <= soilType && 19 >= soilType))
    {
      CkError("ERROR in evapoTranspirationSoil: STAS soil type must be greater than or equal to 1 and less than or equal to 19.\n");
      error = true;
    }
  
  if (!(NULL != evapoTranspirationState))
    {
      CkError("ERROR in evapoTranspirationSoil: evapoTranspirationState must not be NULL.\n");
      error = true;
    }
  
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME do other checks on inputs.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (!error)
    {
      error = checkEvapoTranspirationStateStructInvariant(evapoTranspirationState);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  if (!error)
    {
      // Set zSoil from zSnso.
      for (ii = 0; ii < 4; ii++)
        {
          if (0 > evapoTranspirationState->iSnow)
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + 3] - evapoTranspirationState->zSnso[2];
            }
          else
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + 3];
            }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          zSoilOriginal[ii] = zSoil[ii];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Calculate fIce at the beginning of the timestep.
      for (ii = evapoTranspirationState->iSnow + 3; ii < 3; ii++)
        {
          fIce[ii] = evapoTranspirationState->snIce[ii] / (evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]);
        }
      
      REDPRM(&vegType, &soilType, &slopeType, zSoil, &nSoil, &isUrban);
      NOAHMP_SFLX(&iLoc, &jLoc, &lat, &yearLen, &julian, &cosZ, &dt, &dx, &dz8w, &nSoil, zSoil, &nSnow, &shdFac, &shdMax, &vegType, &isUrban, &ice, &ist, &isc,
                  smcEq, &iz0tlnd, &sfcTmp, &sfcPrs, &psfc, &uu, &vv, &q2, &qc, &solDn, &lwDn, &prcp, &tBot, &co2Air, &o2Air, &folN,
                  evapoTranspirationState->fIceOld, &pblh, &zLvl, &evapoTranspirationState->albOld, &evapoTranspirationState->snEqvO,
                  evapoTranspirationState->stc, sh2o, smc, &evapoTranspirationState->tah, &evapoTranspirationState->eah, &evapoTranspirationState->fWet,
                  &evapoTranspirationState->canLiq, &evapoTranspirationState->canIce, &evapoTranspirationState->tv, &evapoTranspirationState->tg, &qsfc,
                  &qSnow, &evapoTranspirationState->iSnow, evapoTranspirationState->zSnso, &evapoTranspirationState->snowH, &evapoTranspirationState->snEqv,
                  evapoTranspirationState->snIce, evapoTranspirationState->snLiq, &zwt, &wa, &wt, &wsLake, &evapoTranspirationState->lfMass,
                  &evapoTranspirationState->rtMass, &evapoTranspirationState->stMass, &evapoTranspirationState->wood, &evapoTranspirationState->stblCp,
                  &evapoTranspirationState->fastCp, &evapoTranspirationState->lai, &evapoTranspirationState->sai, &evapoTranspirationState->cm,
                  &evapoTranspirationState->ch, &evapoTranspirationState->tauss, &smcwtd, &evapoTranspirationState->deepRech, &evapoTranspirationState->rech,
                  &fsa, &fsr, &fira, &fsh, &sSoil, &fcev, &fgev, &fctr, &eCan, &eTran, &eDir, &tRad, &tgb, &tgv, &t2mv, &t2mb, &q2v, &q2b, &runSrf, &runSub,
                  &apar, &psn, &sav, &sag, &fSno, &nee, &gpp, &npp, &fVeg, &albedo, &qSnBot, &ponding, &ponding1, &ponding2, &rsSun, &rsSha, &bGap, &wGap,
                  &chv, &chb, &emissi, &shg, &shc, &shb, &evg, &evb, &ghv, &ghb, &irg, &irc, &irb, &tr, &evc, &chLeaf, &chuc, &chv2, &chb2, &fpIce);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Verify that the input variables have not changed.
      CkAssert(iLocOriginal == iLoc && jLocOriginal == jLoc && latOriginal == lat && yearLenOriginal == yearLen && julianOriginal == julian &&
               cosZOriginal == cosZ && dtOriginal == dt && dxOriginal == dx && dz8wOriginal == dz8w && nSoilOriginal == nSoil &&
               zSoilOriginal[0] == zSoil[0] && zSoilOriginal[1] == zSoil[1] && zSoilOriginal[2] == zSoil[2] && zSoilOriginal[3] == zSoil[3] &&
               nSnowOriginal == nSnow && shdFacOriginal == shdFac && shdMaxOriginal == shdMax && vegTypeOriginal == vegType && isUrbanOriginal == isUrban &&
               iceOriginal == ice && istOriginal == ist && iscOriginal == isc && smcEqOriginal[0] == smcEq[0] && smcEqOriginal[1] == smcEq[1] &&
               smcEqOriginal[2] == smcEq[2] && smcEqOriginal[3] == smcEq[3] && iz0tlndOriginal == iz0tlnd && sfcTmpOriginal == sfcTmp &&
               sfcPrsOriginal == sfcPrs && psfcOriginal == psfc && uuOriginal == uu && vvOriginal == vv && q2Original == q2 && qcOriginal == qc &&
               solDnOriginal == solDn && lwDnOriginal == lwDn && prcpOriginal == prcp && tBotOriginal == tBot && co2AirOriginal == co2Air &&
               o2AirOriginal == o2Air && folNOriginal == folN && fIceOldOriginal[0] == evapoTranspirationState->fIceOld[0] &&
               fIceOldOriginal[1] == evapoTranspirationState->fIceOld[1] && fIceOldOriginal[2] == evapoTranspirationState->fIceOld[2] &&
               pblhOriginal == pblh && zLvlOriginal == zLvl);
      
      // Verify that the fraction of the precipitation that falls as snow is between 0 and 1, the snowfall rate below the canopy is not negative, and the
      // snowmelt out the bottom of the snowpack is not negative.
      CkAssert(0.0f <= fpIce && 1.0f >= fpIce && 0.0f <= qSnow && 0.0f <= qSnBot);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Store fIce from the beginning of the timestep in fIceOld.
      for (ii = 0; ii < 3; ii++)
        {
          if (ii >= iSnowOriginal + 3)
            {
              evapoTranspirationState->fIceOld[ii] = fIce[ii];
            }
          else
            {
              evapoTranspirationState->fIceOld[ii] = 0.0f;
            }
        }
      
      // Calculate derived output variables.
      changeInCanopyIce      = evapoTranspirationState->canIce - canIceOriginal;
      changeInCanopyLiquid   = evapoTranspirationState->canLiq - canLiqOriginal;
      *evaporationFromCanopy = eCan * dt;
      evaporationFromSurface = eDir * dt;
      
      // Surface evaporation is taken first from the snow layer up to the amount in snEqv at the beginning of the timestep, which is now in snEqvO.
      if (evaporationFromSurface <= evapoTranspirationState->snEqvO)
        {
          *evaporationFromSnow   = evaporationFromSurface;
          *evaporationFromGround = 0.0f;
        }
      else
        {
          *evaporationFromSnow   = evapoTranspirationState->snEqvO;
          *evaporationFromGround = evaporationFromSurface - evapoTranspirationState->snEqvO;
        }
      
      snowfallAboveCanopy         = prcp * dt * fpIce;
      snowfallBelowCanopy         = qSnow * dt;
      snowfallInterceptedByCanopy = snowfallAboveCanopy - snowfallBelowCanopy;
      snowmeltOnGround            = qSnBot * dt;
      rainfallAboveCanopy         = prcp * dt - snowfallAboveCanopy;
      rainfallInterceptedByCanopy = changeInCanopyIce + changeInCanopyLiquid + *evaporationFromCanopy - snowfallInterceptedByCanopy;
      rainfallBelowCanopy         = rainfallAboveCanopy - rainfallInterceptedByCanopy;
      
      // Because rainfallBelowCanopy is a derived value it can be slightly negative due to roundoff error.
      // If it is try to take the water from canLiq or canIce.
      if (0.0f > rainfallBelowCanopy)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // We want to test that rainfallBelowCanopy is epsilon equal to zero, but it can be the result of subtracting two big numbers that are close to each
          // other so we have to base epsilon on the largest of the numbers used to generate it.
          epsilon = 1.0f;
          
          if (epsilon < prcp * dt)
            {
              epsilon = prcp * dt;
            }
          
          if (epsilon < evapoTranspirationState->canIce)
            {
              epsilon = evapoTranspirationState->canIce;
            }
          
          if (epsilon < evapoTranspirationState->canLiq)
            {
              epsilon = evapoTranspirationState->canLiq;
            }
          
          if (epsilon < fabs(*evaporationFromCanopy))
            {
              epsilon = fabs(*evaporationFromCanopy);
            }
          
          if (epsilon < snowfallBelowCanopy)
            {
              epsilon = snowfallBelowCanopy;
            }
          
          epsilon *= -1.0e-6f;
          
          CkAssert(epsilon <= rainfallBelowCanopy);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          if (evapoTranspirationState->canLiq >= -rainfallBelowCanopy)
            {
              evapoTranspirationState->canLiq += rainfallBelowCanopy;
              rainfallBelowCanopy              = 0.0f;
            }
          else
            {
              rainfallBelowCanopy             += evapoTranspirationState->canLiq;
              evapoTranspirationState->canLiq  = 0.0f;

              if (evapoTranspirationState->canIce >= -rainfallBelowCanopy)
                {
                  evapoTranspirationState->canIce += rainfallBelowCanopy;
                  rainfallBelowCanopy              = 0.0f;
                }
              else
                {
                  rainfallBelowCanopy             += evapoTranspirationState->canIce;
                  evapoTranspirationState->canIce  = 0.0f;
                  *waterError                     -= rainfallBelowCanopy;
                  rainfallBelowCanopy              = 0.0f;
                }
            }
        }
      
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
      
      // When the total snow height gets less than 2.5mm the multi-layer snow simulation turns off.  When this happens all of the liquid water in snLiq becomes
      // snowmelt on the ground and snEqv gets set to just the portion in snIce.  However, melting/freezing between snIce and snLiq also happens during the
      // timestep so we can't use the beginning timestep value of snLiq to determine how much to add to snowmeltOnGround.  We have to use the final value of
      // snEqv to back out the value of snowmeltOnGround.
      if (0 > iSnowOriginal && 0 == evapoTranspirationState->iSnow)
        {
          snowmeltOnGround = evapoTranspirationState->snEqvO + snowfallBelowCanopy - *evaporationFromSnow - evapoTranspirationState->snEqv;
        }
      
      // If snEqv falls below 0.001 mm then Noah-MP sets it to zero and the water is lost.  If snEqv grows above 2000 mm then Noah-MP sets it to 2000 and the
      // water is lost.  We are calculating what snEqv should be and putting the water back.
      snEqvShouldBe = evapoTranspirationState->snEqvO + snowfallBelowCanopy + rainfallInterceptedBySnow - *evaporationFromSnow - snowmeltOnGround;
      
      if (0.0f == evapoTranspirationState->snEqv)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(0.001f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snEqv = snEqvShouldBe;
        }
      else if (epsilonEqual(2000.0f, evapoTranspirationState->snEqv))
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonLessOrEqual(2000.0f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          if (0 > evapoTranspirationState->iSnow)
            {
              evapoTranspirationState->zSnso[2] = -evapoTranspirationState->snowH;
              
              for (ii = 0; ii < 4; ii++)
                {
                  evapoTranspirationState->zSnso[ii + 3] = evapoTranspirationState->zSnso[2] + zSoil[ii];
                }
              
              evapoTranspirationState->snIce[2] += snEqvShouldBe - evapoTranspirationState->snEqv;
            }
          
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
      
      // FIXME if the multi-layer snow simulation is turned off Noah-MP doesn't calculate any snowmelt regardless of temperature.
      // if 0 == iSnow && 0 == snowmeltOnGround and the temperature is above freezing do a simple snow melt calculation.
      
      *surfacewaterAdd = snowmeltOnGround + rainfallOnGround;
      *transpiration   = eTran * dt;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0f <= *surfacewaterAdd && 0.0f <= *transpiration);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    } // End if (!error).
  
  return error;
}

bool evapoTranspirationWater(float lat, int yearLen, float julian, float cosZ, float dt, float dx, float dz8w, float sfcTmp, float sfcPrs, float psfc,
                             float uu, float vv, float q2, float qc, float solDn, float lwDn, float prcp, float tBot, float pblh, float wsLake,
                             EvapoTranspirationStateStruct* evapoTranspirationState, float* surfacewaterAdd, float* evaporationFromSnow,
                             float* evaporationFromGround, float* waterError)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
  // Input parameters for redprm function.  Some are also used for sflx function.
  int   vegType   = 16; // 'Water Bodies' from VEGPARM.TBL.
  int   soilType  = 14; // 'WATER' from SOILPARM.TBL.
  int   slopeType = 8;  // I just arbitrarily chose a slope type with zero slope.  I think slope is only used to calculate runoff, which we ignore.
  float zSoil[4];       // Layer bottom depth in meters from soil surface of each soil layer.  Values are set below from zSnso.
  int   nSoil     = 4;  // Always use four soil layers.
  int   isUrban   = 1;  // USGS vegetation type for urban land.
  
  // Input parameters to sflx function.
  int   iLoc     = 1;                        // Grid location index, unused.
  int   jLoc     = 1;                        // Grid location index, unused.
  int   nSnow    = 3;                        // Maximum number of snow layers.  Always pass 3.
  float shdFac   = 0.0f;                     // Fraction of land area shaded by vegetation, 0.0 to 1.0.  Always pass 0.0.
  float shdMax   = 0.0f;                     // Yearly maximum fraction of land area shaded by vegetation, 0.0 to 1.0.  Always pass 0.0.
  int   ice      = 0;                        // Flag to indicate permanent ice cover, 0 for no, 1 for yes.  Always pass 0.  If permanent ice cover call
                                             // evapoTranspirationGlacier instead.
  int   ist      = 2;                        // Flag to indicate permanent water cover, 1 for soil, 2 for lake.  Always pass 2.  If permanent soil cover call
                                             // evapoTranspirationSoil instead.
  int   isc      = 4;                        // Soil color type, 1 for lightest to 8 for darkest.  Always pass 4.
  float smcEq[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Equlibrium water content of each soil layer, unitless.  Always pass 1.0.
  int   iz0tlnd  = 0;                        // Unused.
  float co2Air   = 0.0004f * sfcPrs;         // CO2 partial pressure in Pascal at surface.  Always pass 400 parts per million of surface pressure.
  float o2Air    = 0.21f * sfcPrs;           // O2 partial pressure in Pascal at surface.  Always pass 21 percent of surface pressure.
  float folN     = 0.0f;                     // Foliage nitrogen percentage, 0.0 to 100.0.  Always pass 0.0.
  float fIce[3];                             // Frozen fraction of each snow layer, unitless.  Values are set below from snIce and snLiq.
  float zLvl     = dz8w;                     // Thickness in meters of lowest atmosphere layer in forcing data.  Redundant with dz8w.
  
  // Input/output parameters to sflx function.
  float sh2o[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Liquid water content of each soil layer, unitless.  Always pass 1.0.
  float smc[4]  = {1.0f, 1.0f, 1.0f, 1.0f}; // Total water content, liquid and solid, of each soil layer, unitless.  Always pass 1.0.
  float qsfc    = q2;                       // Water vapor mixing ratio at middle of lowest atmosphere layer in forcing data, unitless.  Redundant with q2.
  float qSnow   = NAN;                      // This is actually an output only variable.  Snowfall rate below the canopy in millimeters of water equivalent per
                                            // second.
  float zwt     = 0.0f;                     // Depth in meters to water table.  Specify depth as a positive number.  Always pass 0.0.
  float wa      = 0.0f;                     // Water stored in aquifer in millimeters of water.  Always pass 0.0.
  float wt      = 0.0f;                     // Water stored in aquifer and saturated soil in millimeters of water.  Always pass 0.0.
  float smcwtd  = 1.0f;                     // Water content between the bottom of the lowest soil layer and water table, unitless.  Always pass 1.0.
  
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
  float runSrf   = NAN; // Unused.
  float runSub   = NAN; // Unused.
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
  float qSnBot   = NAN; // Snowmelt rate from the bottom of the snow pack in millimeters of water per second.
  float ponding  = NAN; // Unused.
  float ponding1 = NAN; // Unused.
  float ponding2 = NAN; // Unused.
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
  
  // Derived output variables.
  int   iSnowOriginal  = evapoTranspirationState->iSnow; // Actual number of snow layers before timestep.
  float evaporationFromSurface;                          // Quantity of evaporation from the surface in millimeters of water equivalent.  Positive means water
                                                         // evaporated off of the surface.  Negative means water condensed on to the surface.
                                                         // Surface evaporation sometimes comes from snow and is taken out by Noah-MP, but if there is not
                                                         // enough snow we have to take the evaporation from the water in the ADHydro state variables.
  float snowfall;                                        // Quantity of snowfall in millimeters of water equivalent.  Must be non-negative.
  float snowmeltOnGround;                                 // Quantity of water that reaches the ground from the snow layer in millimeters of water.  Must be
                                                          // non-negative.
  float rainfall;                                        // Quantity of rainfall in millimeters of water.  Must be non-negative.
  float rainfallInterceptedBySnow;                        // Quantity of rainfall intercepted by the snow layer in millimeters of water.  Must be non-negative.
  float rainfallOnGround;                                 // Quantity of rainfall that reaches the ground in millimeters of water.  Must be non-negative.
  float snEqvShouldBe;                                    // If snEqv falls below 0.001 Noah-MP sets it to zero, or if it rises above 2000.0 Noah-MP sets it to
                                                          // 2000.0.  We don't want this behavior so in this variable we calculate what snEqv should be and set
                                                          // it back.  If snEqv is not changed this instead performs a mass balance check.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Variables used for assertions.
  int   iLocOriginal       = iLoc;
  int   jLocOriginal       = jLoc;
  float latOriginal        = lat;
  int   yearLenOriginal    = yearLen;
  float julianOriginal     = julian;
  float cosZOriginal       = cosZ;
  float dtOriginal         = dt;
  float dxOriginal         = dx;
  float dz8wOriginal       = dz8w;
  int   nSoilOriginal      = nSoil;
  float zSoilOriginal[4];
  int   nSnowOriginal      = nSnow;
  float shdFacOriginal     = shdFac;
  float shdMaxOriginal     = shdMax;
  int   vegTypeOriginal    = vegType;
  int   isUrbanOriginal    = isUrban;
  int   iceOriginal        = ice;
  int   istOriginal        = ist;
  int   iscOriginal        = isc;
  float smcEqOriginal[4]   = {smcEq[0], smcEq[1], smcEq[2], smcEq[3]};
  int   iz0tlndOriginal    = iz0tlnd;
  float sfcTmpOriginal     = sfcTmp;
  float sfcPrsOriginal     = sfcPrs;
  float psfcOriginal       = psfc;
  float uuOriginal         = uu;
  float vvOriginal         = vv;
  float q2Original         = q2;
  float qcOriginal         = qc;
  float solDnOriginal      = solDn;
  float lwDnOriginal       = lwDn;
  float prcpOriginal       = prcp;
  float tBotOriginal       = tBot;
  float co2AirOriginal     = co2Air;
  float o2AirOriginal      = o2Air;
  float folNOriginal       = folN;
  float fIceOldOriginal[3] = {evapoTranspirationState->fIceOld[0], evapoTranspirationState->fIceOld[1], evapoTranspirationState->fIceOld[2]};
  float pblhOriginal       = pblh;
  float zLvlOriginal       = zLvl;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evapoTranspirationState))
    {
      CkError("ERROR in evapoTranspirationWater: evapoTranspirationState must not be NULL.\n");
      error = true;
    }
  else
    {
      if (!(0.0 == evapoTranspirationState->canLiq))
        {
          CkError("ERROR in evapoTranspirationWater: canLiq must be zero.\n");
          error = true;
        }

      if (!(0.0 == evapoTranspirationState->canIce))
        {
          CkError("ERROR in evapoTranspirationWater: canIce must be zero.\n");
          error = true;
        }
    }
  
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
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME do other checks on inputs.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (!error)
    {
      error = checkEvapoTranspirationStateStructInvariant(evapoTranspirationState);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  if (!error)
    {
      // Set zSoil from zSnso.
      for (ii = 0; ii < 4; ii++)
        {
          if (0 > evapoTranspirationState->iSnow)
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + 3] - evapoTranspirationState->zSnso[2];
            }
          else
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + 3];
            }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          zSoilOriginal[ii] = zSoil[ii];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Calculate fIce at the beginning of the timestep.
      for (ii = evapoTranspirationState->iSnow + 3; ii < 3; ii++)
        {
          fIce[ii] = evapoTranspirationState->snIce[ii] / (evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]);
        }

      REDPRM(&vegType, &soilType, &slopeType, zSoil, &nSoil, &isUrban);
      NOAHMP_SFLX(&iLoc, &jLoc, &lat, &yearLen, &julian, &cosZ, &dt, &dx, &dz8w, &nSoil, zSoil, &nSnow, &shdFac, &shdMax, &vegType, &isUrban, &ice, &ist, &isc,
                  smcEq, &iz0tlnd, &sfcTmp, &sfcPrs, &psfc, &uu, &vv, &q2, &qc, &solDn, &lwDn, &prcp, &tBot, &co2Air, &o2Air, &folN,
                  evapoTranspirationState->fIceOld, &pblh, &zLvl, &evapoTranspirationState->albOld, &evapoTranspirationState->snEqvO,
                  evapoTranspirationState->stc, sh2o, smc, &evapoTranspirationState->tah, &evapoTranspirationState->eah, &evapoTranspirationState->fWet,
                  &evapoTranspirationState->canLiq, &evapoTranspirationState->canIce, &evapoTranspirationState->tv, &evapoTranspirationState->tg, &qsfc,
                  &qSnow, &evapoTranspirationState->iSnow, evapoTranspirationState->zSnso, &evapoTranspirationState->snowH, &evapoTranspirationState->snEqv,
                  evapoTranspirationState->snIce, evapoTranspirationState->snLiq, &zwt, &wa, &wt, &wsLake, &evapoTranspirationState->lfMass,
                  &evapoTranspirationState->rtMass, &evapoTranspirationState->stMass, &evapoTranspirationState->wood, &evapoTranspirationState->stblCp,
                  &evapoTranspirationState->fastCp, &evapoTranspirationState->lai, &evapoTranspirationState->sai, &evapoTranspirationState->cm,
                  &evapoTranspirationState->ch, &evapoTranspirationState->tauss, &smcwtd, &evapoTranspirationState->deepRech, &evapoTranspirationState->rech,
                  &fsa, &fsr, &fira, &fsh, &sSoil, &fcev, &fgev, &fctr, &eCan, &eTran, &eDir, &tRad, &tgb, &tgv, &t2mv, &t2mb, &q2v, &q2b, &runSrf, &runSub,
                  &apar, &psn, &sav, &sag, &fSno, &nee, &gpp, &npp, &fVeg, &albedo, &qSnBot, &ponding, &ponding1, &ponding2, &rsSun, &rsSha, &bGap, &wGap,
                  &chv, &chb, &emissi, &shg, &shc, &shb, &evg, &evb, &ghv, &ghb, &irg, &irc, &irb, &tr, &evc, &chLeaf, &chuc, &chv2, &chb2, &fpIce);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Verify that the input variables have not changed.
      CkAssert(iLocOriginal == iLoc && jLocOriginal == jLoc && latOriginal == lat && yearLenOriginal == yearLen && julianOriginal == julian &&
               cosZOriginal == cosZ && dtOriginal == dt && dxOriginal == dx && dz8wOriginal == dz8w && nSoilOriginal == nSoil &&
               zSoilOriginal[0] == zSoil[0] && zSoilOriginal[1] == zSoil[1] && zSoilOriginal[2] == zSoil[2] && zSoilOriginal[3] == zSoil[3] &&
               nSnowOriginal == nSnow && shdFacOriginal == shdFac && shdMaxOriginal == shdMax && vegTypeOriginal == vegType && isUrbanOriginal == isUrban &&
               iceOriginal == ice && istOriginal == ist && iscOriginal == isc && smcEqOriginal[0] == smcEq[0] && smcEqOriginal[1] == smcEq[1] &&
               smcEqOriginal[2] == smcEq[2] && smcEqOriginal[3] == smcEq[3] && iz0tlndOriginal == iz0tlnd && sfcTmpOriginal == sfcTmp &&
               sfcPrsOriginal == sfcPrs && psfcOriginal == psfc && uuOriginal == uu && vvOriginal == vv && q2Original == q2 && qcOriginal == qc &&
               solDnOriginal == solDn && lwDnOriginal == lwDn && prcpOriginal == prcp && tBotOriginal == tBot && co2AirOriginal == co2Air &&
               o2AirOriginal == o2Air && folNOriginal == folN && fIceOldOriginal[0] == evapoTranspirationState->fIceOld[0] &&
               fIceOldOriginal[1] == evapoTranspirationState->fIceOld[1] && fIceOldOriginal[2] == evapoTranspirationState->fIceOld[2] &&
               pblhOriginal == pblh && zLvlOriginal == zLvl);
      
      // Verify that the fraction of the precipitation that falls as snow is between 0 and 1, the snowfall rate below the canopy is not negative, and the
      // snowmelt out the bottom of the snowpack is not negative.
      CkAssert(0.0f <= fpIce && 1.0f >= fpIce && 0.0f <= qSnow && 0.0f <= qSnBot);
      
      // Verify that there is no water, no evaporation, no transpiration, and no snowfall interception in the non-existant canopy.
      CkAssert(0.0 == evapoTranspirationState->canLiq && 0.0 == evapoTranspirationState->canIce && 0.0 == eCan && 0.0 == eTran && prcp * fpIce == qSnow);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Store fIce from the beginning of the timestep in fIceOld.
      for (ii = 0; ii < 3; ii++)
        {
          if (ii >= iSnowOriginal + 3)
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
      
      // Surface evaporation is taken first from the snow layer up to the amount in snEqv at the beginning of the timestep, which is now in snEqvO.
      if (evaporationFromSurface <= evapoTranspirationState->snEqvO)
        {
          *evaporationFromSnow   = evaporationFromSurface;
          *evaporationFromGround = 0.0f;
        }
      else
        {
          *evaporationFromSnow   = evapoTranspirationState->snEqvO;
          *evaporationFromGround = evaporationFromSurface - evapoTranspirationState->snEqvO;
        }
      
      snowfall         = prcp * dt * fpIce;
      snowmeltOnGround = qSnBot * dt;
      rainfall         = prcp * dt - snowfall;
      
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
      
      // When the total snow height gets less than 2.5mm the multi-layer snow simulation turns off.  When this happens all of the liquid water in snLiq becomes
      // snowmelt on the ground and snEqv gets set to just the portion in snIce.  However, melting/freezing between snIce and snLiq also happens during the
      // timestep so we can't use the beginning timestep value of snLiq to determine how much to add to snowmeltOnGround.  We have to use the final value of
      // snEqv to back out the value of snowmeltOnGround.
      if (0 > iSnowOriginal && 0 == evapoTranspirationState->iSnow)
        {
          snowmeltOnGround = evapoTranspirationState->snEqvO + snowfall - *evaporationFromSnow - evapoTranspirationState->snEqv;
        }
      
      // If snEqv falls below 0.001 mm then Noah-MP sets it to zero and the water is lost.  If snEqv grows above 2000 mm then Noah-MP sets it to 2000 and the
      // water is lost.  We are calculating what snEqv should be and putting the water back.
      snEqvShouldBe = evapoTranspirationState->snEqvO + snowfall + rainfallInterceptedBySnow - *evaporationFromSnow - snowmeltOnGround;
      
      if (0.0f == evapoTranspirationState->snEqv)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(0.001f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snEqv = snEqvShouldBe;
        }
      else if (epsilonEqual(2000.0f, evapoTranspirationState->snEqv))
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonLessOrEqual(2000.0f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          if (0 > evapoTranspirationState->iSnow)
            {
              evapoTranspirationState->zSnso[2] = -evapoTranspirationState->snowH;
              
              for (ii = 0; ii < 4; ii++)
                {
                  evapoTranspirationState->zSnso[ii + 3] = evapoTranspirationState->zSnso[2] + zSoil[ii];
                }
              
              evapoTranspirationState->snIce[2] += snEqvShouldBe - evapoTranspirationState->snEqv;
            }
          
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
      
      // FIXME if the multi-layer snow simulation is turned off Noah-MP doesn't calculate any snowmelt regardless of temperature.
      // if 0 == iSnow && 0 == snowmeltOnGround and the temperature is above freezing do a simple snow melt calculation.
      
      *surfacewaterAdd = snowmeltOnGround + rainfallOnGround;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0f <= *surfacewaterAdd);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    } // End if (!error).
  
  return error;
}

bool evapoTranspirationGlacier(float cosZ, float dt, float sfcTmp, float sfcPrs, float uu, float vv, float q2, float solDn, float lwDn, float prcp, float tBot,
                               float zLvl, EvapoTranspirationStateStruct* evapoTranspirationState, float* surfacewaterAdd, float* evaporationFromSnow,
                               float* evaporationFromGround, float* waterError)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
  // Input parameters for redprm function.  Some are also used for sflx function.
  int   vegType   = 16; // 'Water Bodies' from VEGPARM.TBL.
  int   soilType  = 14; // 'WATER' from SOILPARM.TBL.
  int   slopeType = 8;  // I just arbitrarily chose a slope type with zero slope.  I think slope is only used to calculate runoff, which we ignore.
  float zSoil[4];       // Layer bottom depth in meters from soil surface of each soil layer.  Values are set below from zSnso.
  int   nSoil     = 4;  // Always use four soil layers.
  int   isUrban   = 1;  // USGS vegetation type for urban land.
  
  // Input parameters to sflx function.
  int   iLoc    = 1; // Grid location index, unused.
  int   jLoc    = 1; // Grid location index, unused.
  int   nSnow   = 3; // Maximum number of snow layers.  Always pass 3.
  float fIce[3];     // Frozen fraction of each snow layer, unitless.  Values are set below from snIce and snLiq.
  
  // Input/output parameters to sflx function.
  float qSnow   = NAN;                      // This is actually an output only variable.  Snowfall rate below the canopy in millimeters of water equivalent per
                                            // second.
  float smc[4]  = {1.0f, 1.0f, 1.0f, 1.0f}; // Total water content, liquid and solid, of each soil layer, unitless.  Always pass 1.0.
  float sh2o[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // Liquid water content of each soil layer, unitless.  Always pass 0.0.
  float qsfc    = q2;                       // Water vapor mixing ratio at middle of lowest atmosphere layer in forcing data, unitless.  Redundant with q2.

  // Output parameters to sflx function.  Set to NAN so we can detect if the values are used before being set.
  float fsa      = NAN; // Unused.
  float fsr      = NAN; // Unused.
  float fira     = NAN; // Unused.
  float fsh      = NAN; // Unused.
  float fgev     = NAN; // Unused.
  float sSoil    = NAN; // Unused.
  float tRad     = NAN; // Unused.
  float eDir     = NAN; // Evaporation rate from surface in millimeters of water equivalent per second.
  float runSrf   = NAN; // Unused.
  float runSub   = NAN; // Unused.
  float sag      = NAN; // Unused.
  float albedo   = NAN; // Unused.
  float qSnBot   = NAN; // Snowmelt rate from the bottom of the snow pack in millimeters of water per second.
  float ponding  = NAN; // Unused.
  float ponding1 = NAN; // Unused.
  float ponding2 = NAN; // Unused.
  float t2m      = NAN; // Unused.
  float q2e      = NAN; // Unused.
  float emissi   = NAN; // Unused.
  float fpIce    = NAN; // Fraction of precipitation that is frozen, unitless.
  float ch2b     = NAN; // Unused.
  
  // Derived output variables.
  int   iSnowOriginal  = evapoTranspirationState->iSnow; // Actual number of snow layers before timestep.
  float evaporationFromSurface;                          // Quantity of evaporation from the surface in millimeters of water equivalent.  Positive means water
                                                         // evaporated off of the surface.  Negative means water condensed on to the surface.
                                                         // Surface evaporation sometimes comes from snow and is taken out by Noah-MP, but if there is not
                                                         // enough snow we have to take the evaporation from the water in the ADHydro state variables.
  float snowfall;                                        // Quantity of snowfall in millimeters of water equivalent.  Must be non-negative.
  float snowmeltOnGround;                                 // Quantity of water that reaches the ground from the snow layer in millimeters of water.  Must be
                                                          // non-negative.
  float rainfall;                                        // Quantity of rainfall in millimeters of water.  Must be non-negative.
  float rainfallInterceptedBySnow;                        // Quantity of rainfall intercepted by the snow layer in millimeters of water.  Must be non-negative.
  float rainfallOnGround;                                 // Quantity of rainfall that reaches the ground in millimeters of water.  Must be non-negative.
  float snEqvShouldBe;                                    // If snEqv falls below 0.001 Noah-MP sets it to zero, or if it rises above 2000.0 Noah-MP sets it to
                                                          // 2000.0.  We don't want this behavior so in this variable we calculate what snEqv should be and set
                                                          // it back.  If snEqv is not changed this instead performs a mass balance check.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Variables used for assertions.
  int   iLocOriginal       = iLoc;
  int   jLocOriginal       = jLoc;
  float cosZOriginal       = cosZ;
  int   nSnowOriginal      = nSnow;
  int   nSoilOriginal      = nSoil;
  float dtOriginal         = dt;
  float sfcTmpOriginal     = sfcTmp;
  float sfcPrsOriginal     = sfcPrs;
  float uuOriginal         = uu;
  float vvOriginal         = vv;
  float q2Original         = q2;
  float solDnOriginal      = solDn;
  float prcpOriginal       = prcp;
  float lwDnOriginal       = lwDn;
  float tBotOriginal       = tBot;
  float zLvlOriginal       = zLvl;
  float fIceOldOriginal[3] = {evapoTranspirationState->fIceOld[0], evapoTranspirationState->fIceOld[1], evapoTranspirationState->fIceOld[2]};
  float zSoilOriginal[4];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                                                    
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NULL != evapoTranspirationState))
    {
      CkError("ERROR in evapoTranspirationGlacier: evapoTranspirationState must not be NULL.\n");
      error = true;
    }
  else
    {
      if (!(0.0 == evapoTranspirationState->canLiq))
        {
          CkError("ERROR in evapoTranspirationGlacier: canLiq must be zero.\n");
          error = true;
        }

      if (!(0.0 == evapoTranspirationState->canIce))
        {
          CkError("ERROR in evapoTranspirationGlacier: canIce must be zero.\n");
          error = true;
        }
    }
  
  if (!(NULL != waterError))
    {
      CkError("ERROR in evapoTranspirationGlacier: water error must not be NULL.\n");
      error = true;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *waterError = 0.0;
    }
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME do other checks on inputs.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (!error)
    {
      error = checkEvapoTranspirationStateStructInvariant(evapoTranspirationState);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  if (!error)
    {
      // Set zSoil from zSnso.
      for (ii = 0; ii < 4; ii++)
        {
          if (0 > evapoTranspirationState->iSnow)
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + 3] - evapoTranspirationState->zSnso[2];
            }
          else
            {
              zSoil[ii] = evapoTranspirationState->zSnso[ii + 3];
            }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          zSoilOriginal[ii] = zSoil[ii];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Calculate fIce at the beginning of the timestep.
      for (ii = evapoTranspirationState->iSnow + 3; ii < 3; ii++)
        {
          fIce[ii] = evapoTranspirationState->snIce[ii] / (evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]);
        }

      REDPRM(&vegType, &soilType, &slopeType, zSoil, &nSoil, &isUrban);
      NOAHMP_GLACIER(&iLoc, &jLoc, &cosZ, &nSnow, &nSoil, &dt, &sfcTmp, &sfcPrs, &uu, &vv, &q2, &solDn, &prcp, &lwDn, &tBot, &zLvl,
                     evapoTranspirationState->fIceOld, zSoil, &qSnow, &evapoTranspirationState->snEqvO, &evapoTranspirationState->albOld,
                     &evapoTranspirationState->cm, &evapoTranspirationState->ch, &evapoTranspirationState->iSnow, &evapoTranspirationState->snEqv, smc,
                     evapoTranspirationState->zSnso, &evapoTranspirationState->snowH, evapoTranspirationState->snIce, evapoTranspirationState->snLiq,
                     &evapoTranspirationState->tg, evapoTranspirationState->stc, sh2o, &evapoTranspirationState->tauss, &qsfc, &fsa, &fsr, &fira, &fsh, &fgev,
                     &sSoil, &tRad, &eDir, &runSrf, &runSub, &sag, &albedo, &qSnBot, &ponding, &ponding1, &ponding2, &t2m, &q2e, &emissi, &fpIce, &ch2b);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Verify that the input variables have not changed.
      CkAssert(iLocOriginal == iLoc && jLocOriginal == jLoc && cosZOriginal == cosZ && nSnowOriginal == nSnow && nSoilOriginal == nSoil && dtOriginal == dt &&
               sfcTmpOriginal == sfcTmp && sfcPrsOriginal == sfcPrs && uuOriginal == uu && vvOriginal == vv && q2Original == q2 && solDnOriginal == solDn &&
               prcpOriginal == prcp && lwDnOriginal == lwDn && tBotOriginal == tBot && zLvlOriginal == zLvl &&
               fIceOldOriginal[0] == evapoTranspirationState->fIceOld[0] && fIceOldOriginal[1] == evapoTranspirationState->fIceOld[1] &&
               fIceOldOriginal[2] == evapoTranspirationState->fIceOld[2] && zSoilOriginal[0] == zSoil[0] && zSoilOriginal[1] == zSoil[1] &&
               zSoilOriginal[2] == zSoil[2] && zSoilOriginal[3] == zSoil[3]);
      
      // Verify that the fraction of the precipitation that falls as snow is between 0 and 1, the snowfall rate below the canopy is not negative, and the
      // snowmelt out the bottom of the snowpack is not negative.
      CkAssert(0.0f <= fpIce && 1.0f >= fpIce && 0.0f <= qSnow && 0.0f <= qSnBot);
      
      // Verify that there is no snowfall interception in the non-existant canopy.
      CkAssert(prcp * fpIce == qSnow);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      // Store fIce from the beginning of the timestep in fIceOld.
      for (ii = 0; ii < 3; ii++)
        {
          if (ii >= iSnowOriginal + 3)
            {
              evapoTranspirationState->fIceOld[ii] = fIce[ii];
            }
          else
            {
              evapoTranspirationState->fIceOld[ii] = 0.0;
            }
        }
         
      // Calculate derived output variables.
      evaporationFromSurface = eDir * dt;

      // Surface evaporation is taken first from the snow layer up to the amount in snEqv at the beginning of the timestep, which is now in snEqvO.
      if (evaporationFromSurface <= evapoTranspirationState->snEqvO)
        {
          *evaporationFromSnow   = evaporationFromSurface;
          *evaporationFromGround = 0.0;
        }
      else
        {
          *evaporationFromSnow   = evapoTranspirationState->snEqvO;
          *evaporationFromGround = evaporationFromSurface - evapoTranspirationState->snEqvO;
        }
      
      snowfall         = prcp * dt * fpIce;
      snowmeltOnGround = qSnBot * dt;
      rainfall         = prcp * dt - snowfall;
      
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
      
      // When the total snow height gets less than 2.5mm the multi-layer snow simulation turns off.  When this happens all of the liquid water in snLiq becomes
      // snowmelt on the ground and snEqv gets set to just the portion in snIce.  However, melting/freezing between snIce and snLiq also happens during the
      // timestep so we can't use the beginning timestep value of snLiq to determine how much to add to snowmeltOnGround.  We have to use the final value of
      // snEqv to back out the value of snowmeltOnGround.
      if (0 > iSnowOriginal && 0 == evapoTranspirationState->iSnow)
        {
          snowmeltOnGround = evapoTranspirationState->snEqvO + snowfall - *evaporationFromSnow - evapoTranspirationState->snEqv;
        }
      
      // If snEqv falls below 0.001 mm then NOAM-MP sets it to zero and the water is lost.  We are calculating what snEqv should be and putting the water back.
      snEqvShouldBe = evapoTranspirationState->snEqvO + snowfall + rainfallInterceptedBySnow - *evaporationFromSnow - snowmeltOnGround;
      
      if (0.0 == evapoTranspirationState->snEqv)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(0.001f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snEqv = snEqvShouldBe;
        }
      else if (epsilonEqual(2000.0f, evapoTranspirationState->snEqv))
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonLessOrEqual(2000.0f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          if (0 > evapoTranspirationState->iSnow)
            {
              evapoTranspirationState->zSnso[2] = -evapoTranspirationState->snowH;
              
              for (ii = 0; ii < 4; ii++)
                {
                  evapoTranspirationState->zSnso[ii + 3] = evapoTranspirationState->zSnso[2] + zSoil[ii];
                }
              
              evapoTranspirationState->snIce[2] += snEqvShouldBe - evapoTranspirationState->snEqv;
            }
          
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
      
      // FIXME if the multi-layer snow simulation is turned off Noah-MP doesn't calculate any snowmelt regardless of temperature.
      // if 0 == iSnow && 0 == snowmeltOnGround and the temperature is above freezing do a simple snow melt calculation.
      
      *surfacewaterAdd = snowmeltOnGround + rainfallOnGround;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0f <= *surfacewaterAdd);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    } // End if (!error).
  
  return error;
}

bool checkEvapoTranspirationStateStructInvariant(EvapoTranspirationStateStruct* evapoTranspirationState)
{
  bool  error          = false; // Error flag.
  int   ii;                     // Loop counter.
  float totalSnowWater = 0.0f;  // Total water in all snow layers in millimeters of water equivalent.

  for (ii = 0; ii < 3; ii++)
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

  for (ii = 0; ii < 7; ii++)
    {
      if (ii < evapoTranspirationState->iSnow + 3)
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
          else if (!(-60.0f + ZERO_C_IN_KELVIN <= evapoTranspirationState->stc[ii]))
            {
              CkError("WARNING in checkEvapoTranspirationStateStructInvariant, snow/soil layer %d: stc below -60 degrees C.\n", ii);
            }
          else if (!(60.0f + ZERO_C_IN_KELVIN >= evapoTranspirationState->stc[ii]))
            {
              CkError("WARNING in checkEvapoTranspirationStateStructInvariant, snow/soil layer %d: stc above 60 degrees C.\n", ii);
            }
        }
    }

  if (!(0.0f <= evapoTranspirationState->tah))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: tah must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(-60.0f + ZERO_C_IN_KELVIN <= evapoTranspirationState->tah))
    {
      CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tah below -60 degrees C.\n");
    }
  else if (!(60.0f + ZERO_C_IN_KELVIN >= evapoTranspirationState->tah))
    {
      CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tah above 60 degrees C.\n");
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
  else if (!(-60.0f + ZERO_C_IN_KELVIN <= evapoTranspirationState->tv))
    {
      CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tv below -60 degrees C.\n");
    }
  else if (!(60.0f + ZERO_C_IN_KELVIN >= evapoTranspirationState->tv))
    {
      CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tv above 60 degrees C.\n");
    }

  if (!(0.0f <= evapoTranspirationState->tg))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: tg must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(-60.0f + ZERO_C_IN_KELVIN <= evapoTranspirationState->tg))
    {
      CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tg below -60 degrees C.\n");
    }
  else if (!(60.0f + ZERO_C_IN_KELVIN >= evapoTranspirationState->tg))
    {
      CkError("WARNING in checkEvapoTranspirationStateStructInvariant: tg above 60 degrees C.\n");
    }

  if (!(-3 <= evapoTranspirationState->iSnow && evapoTranspirationState->iSnow <= 0))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: iSnow must be greater than or equal to minus three and less than or equal to zero.\n");
      error = true;
    }

  for (ii = 0; ii < 3; ii++)
    {
      if (ii < evapoTranspirationState->iSnow + 3)
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
          if (ii == evapoTranspirationState->iSnow + 3)
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

  for (ii = 3; ii < 7; ii++)
    {
      if (!(evapoTranspirationState->zSnso[ii - 1] > evapoTranspirationState->zSnso[ii]))
        {
          CkError("ERROR in checkEvapoTranspirationStateStructInvariant, soil layer %d: zSnso must be less than the layer above it.\n", ii - 3);
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
      if (!epsilonEqual(-evapoTranspirationState->zSnso[2], evapoTranspirationState->snowH))
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
      CkError("WARNING in checkEvapoTranspirationStateStructInvariant: lai above six.\n");
    }

  if (!(0.0f <= evapoTranspirationState->sai))
    {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant: sai must be greater than or equal to zero.\n");
      error = true;
    }
  else if (!(1.0f >= evapoTranspirationState->sai))
    {
      CkError("WARNING in checkEvapoTranspirationStateStructInvariant: sai above one.\n");
    }

  return error;
}
