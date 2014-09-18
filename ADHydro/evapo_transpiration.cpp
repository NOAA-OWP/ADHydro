#include "evapo_transpiration.h"
#include "all.h"
#include <math.h>

extern "C" void __noahmp_veg_parameters_MOD_read_mp_veg_parameters(const char* landUse, const char* mpTableFile, size_t landUseSize, size_t mpTableFileSize);
extern "C" void __module_sf_noahmpdrv_MOD_soil_veg_gen_parm(const char* landUse, const char* soil, const char* vegParmFile, const char* soilParmFile,
                                                            const char* genParmFile, size_t landUseSize, size_t soilSize, size_t vegParmFileSize,
                                                            size_t soilParmFileSize, size_t genParmFileSize);
extern "C" void __noahmp_routines_MOD_noahmp_options(int* dveg, int* optCrs, int* optBtr, int* optRun, int* optSfc, int* optFrz, int* optInf, int* optRad,
                                                     int* optAlb, int* optSnf, int* optTbot, int* optStc);
extern "C" void __noahmp_glacier_routines_MOD_noahmp_options_glacier(int* dveg, int* optCrs, int* optBtr, int* optRun, int* optSfc, int* optFrz, int* optInf,
                                                                     int* optRad, int* optAlb, int* optSnf, int* optTbot, int* optStc);
extern "C" void __noahmp_routines_MOD_redprm(int* vegType, int* soilType, int* slopeType, float* zSoil, int* nSoil, int* isUrban);
extern "C" void __noahmp_routines_MOD_noahmp_sflx(int* iLoc, int* jLoc, float* lat, int* yearLen, float* julian, float* cosZ, float* dt, float* dx,
                                                  float* dz8w, int* nSoil, float* zSoil, int* nSnow, float* shdFac, float* shdMax, int* vegType, int* isUrban,
                                                  int* ice, int* ist, int* isc, float* smcEq, int* iz0tlnd, float* sfcTmp, float* sfcPrs, float* psfc,
                                                  float* uu, float* vv, float* q2, float* qc, float* solDn, float* lwDn, float* prcp, float* tBot,
                                                  float* co2Air, float* o2Air, float* folN, float* fIceOld, float* pblh, float* zLvl, float* albOld,
                                                  float* snEqvO, float* stc, float* sh2o, float* smc, float* tah, float* eah, float* fWet, float* canLiq,
                                                  float* canIce, float* tv, float* tg, float* qsfc, float* qSnow, int* iSnow, float* zSnso, float* snowH,
                                                  float* snEqv, float* snIce, float* snLiq, float* zwt, float* wa, float* wt, float* wsLake, float* lfMass,
                                                  float* rtMass, float* stMass, float* wood, float* stblCp, float* fastCp, float* lai, float* sai, float* cm,
                                                  float* ch, float* tauss, float* smcwtd, float* deepRech, float* rech, float* fsa, float* fsr, float* fira,
                                                  float* fsh, float* sSoil, float* fcev, float* fgev, float* fctr, float* ecan, float* etran, float* eDir,
                                                  float* trad, float* tgb, float* tgv, float* t2mv, float* t2mb, float* q2v, float* q2b, float* runSrf,
                                                  float* runSub, float* apar, float* psn, float* sav, float* sag, float* fSno, float* nee, float* gpp,
                                                  float* npp, float* fVeg, float* albedo, float* qsnBot, float* ponding, float* ponding1, float* ponding2,
                                                  float* rsSun, float* rsSha, float* bGap, float* wGap, float* chv, float* chb, float* emissi, float* shg,
                                                  float* shc, float* shb, float* evg, float* evb, float* ghv, float* ghb, float* irg, float* irc, float* irb,
                                                  float* tr, float* evc, float* chLeaf, float* chuc, float* chv2, float* chb2, float* fpIce);
// FIXME extern "C" noahmp_glacier

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
      __noahmp_veg_parameters_MOD_read_mp_veg_parameters(landUse, mpTableFile, strlen(landUse), strlen(mpTableFile));
      __module_sf_noahmpdrv_MOD_soil_veg_gen_parm(landUse, soil, vegParmFile, soilParmFile, genParmFile, strlen(landUse), strlen(soil), strlen(vegParmFile),
                                                  strlen(soilParmFile), strlen(genParmFile));
      __noahmp_routines_MOD_noahmp_options(&dveg, &optCrs, &optBtr, &optRun, &optSfc, &optFrz, &optInf, &optRad, &optAlb, &optSnf, &optTbot, &optStc);
      __noahmp_glacier_routines_MOD_noahmp_options_glacier(&dveg, &optCrs, &optBtr, &optRun, &optSfc, &optFrz, &optInf, &optRad, &optAlb, &optSnf, &optTbot,
                                                           &optStc);
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
                            float lwDn, float prcp, float tBot, float pblh, float sh2o[4], float smc[4], float zwt, float wa, float wt, float wsLake,
                            float smcwtd, EvapoTranspirationStateStruct* evapoTranspirationState, float* waterError)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
  // Input parameters for redprm function.  Some are also used for sflx function.
  int   slopeType = 8; // I just arbitrarily chose a slope type with zero slope.  I think slope is only used to calculate runoff, which we ignore.
  float zSoil[4];      // Layer bottom depth in meters from soil surface of each soil layer.  Values are set below from zSnso.
  int   nSoil     = 4; // Always use four soil layers.
  int   isUrban   = 1; // USGS vegetation type for urban land.
  
  // Input parameters to sflx function.
  int   iLoc    = 1;               // Grid location index, unused.
  int   jLoc    = 1;               // Grid location index, unused.
  int   nSnow   = 3;               // Maximum number of snow layers.  Always pass 3.
  int   ice     = 0;               // Flag to indicate permanent ice cover, 0 for no, 1 for yes.  Always pass 0.  If permanent ice cover call
                                   // evapoTranspirationGlacier instead.
  int   ist     = 1;               // Flag to indicate permanent water cover, 1 for soil, 2 for lake.  Always pass 1.  If permanent water cover call
                                   // evapoTranspirationWater instead.
  int   isc     = 4;               // Soil color type, 1 for lightest to 8 for darkest.  Always pass 4 unless we find a data source for soil color.
  int   iz0tlnd = 0;               // Unused.
  float co2Air  = 0.0004 * sfcPrs; // CO2 partial pressure in Pascal at surface.  Always pass 400 parts per million of surface pressure.
  float o2Air   = 0.21 * sfcPrs;   // O2 partial pressure in Pascal at surface.  Always pass 21 percent of surface pressure.
  float folN    = 3.0;             // Foliage nitrogen percentage, 0.0 to 100.0.  Always pass 3.0 unless we find a data source for foliage nitrogen.
  float fIce[3];                   // Frozen fraction of each snow layer, unitless.  Values are set below from snIce and snLiq.
  float zLvl    = dz8w;            // Thickness in meters of lowest atmosphere layer in forcing data.  Redundant with dz8w.
  
  // Input/output parameters to sflx function.
  float qsfc   = q2;  // Water vapor mixing ratio at middle of lowest atmosphere layer in forcing data, unitless.  Redundant with q2.
  float qSnow  = NAN; // This is actually an output only variable.  Snowfall rate below the canopy in millimeters of water equivalent per second.

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
  float changeInCanopyIce;                                // Change in canopy ice during timestep in millimeters of water equivalent.
  float canLiqOriginal = evapoTranspirationState->canLiq; // Quantity of canopy liquid before timestep in millimeters of water.
  float changeInCanopyLiquid;                             // Change in canopy liquid during timestep in millimeters of water.
  int   iSnowOriginal  = evapoTranspirationState->iSnow;  // Actual number of snow layers before timestep.
  float evaporationFromCanopy;                            // Quantity of evaporation from canopy in millimeters of water equivalent.
  float evaporationFromSurface;                           // Quantity of evaporation from surface in millimeters of water equivalent.
                                                          // Surface evaporation sometimes comes from snow and is taken out by Noah-MP, but if there is not
                                                          // enough snow we have to take the evaporation from the water in the ADHydro state variables.
  float evaporationFromSnow;                              // Quantity of evaporation that Noah-MP takes from the snow in millimeters of water equivalent.
  float evaporationFromGround;                            // Quantity of evaporation that we need to take from the ADHydro state variables in millimeters of
                                                          // water equivalent.
  float snowfallAboveCanopy;                              // Quantity of snowfall above the canopy in millimeters of water equivalent.
  float snowfallInterceptedByCanopy;                      // Quantity of snowfall intercepted by the canopy in millimeters of water equivalent.
                                                          // Can be negative if snow is falling off of the canopy.
  float snowfallBelowCanopy;                              // Quantity of snowfall below the canopy in millimeters of water equivalent.
  float snowmeltOnGround;                                 // Quantity of water that reaches the ground from the snow layer in millimeters of water.
  float rainfallAboveCanopy;                              // Quantity of rainfall above the canopy in millimeters of water.
  float rainfallInterceptedByCanopy;                      // Quantity of rainfall intercepted by the canopy in millimeters of water.
                                                          // Can be negative if rain is dripping from the canopy.
  float rainfallBelowCanopy;                              // Quantity of rainfall below the canopy in millimeters of water.
  float rainfallInterceptedBySnow;                        // Quantity of rainfall intercepted by the snow layer in millimeters of water.
  float rainfallOnGround;                                 // Quantity of rainfall that reaches the ground in millimeters of water.
  float snEqvShouldBe;                                    // If snEqv falls below 0.001 Noah-MP sets it to zero.  We don't want this behavior so in this
                                                          // variable we calculate what snEqv should be and set it back if it gets set to zero.
                                                          // If snEqv is not set to zero this instead performs a mass balance check.
  
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
  
  if (!(NULL != waterError))
    {
      CkError("ERROR in evapoTranspirationSoil: water error must not be NULL.\n");
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
  // FIXME replace with call to invariant check.
  
  // Verify that snEqv is equal to the total water equivalent in the snow layers.
  if (0 > evapoTranspirationState->iSnow)
    {
      snEqvShouldBe = 0.0;

      for (ii = evapoTranspirationState->iSnow + 3; ii < 3; ii++)
        {
          if (!(0.0 < evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]))
            {
              CkError("ERROR in evapoTranspirationSoil: snow layer %d exists with no water in it.\n", ii);
              error = true;
            }
          
          snEqvShouldBe += evapoTranspirationState->snIce[ii];
          snEqvShouldBe += evapoTranspirationState->snLiq[ii];
        }

      if (!(evapoTranspirationState->snEqv == snEqvShouldBe))
        {
          CkError("ERROR in evapoTranspirationSoil: snEqv must be equal to the total of snIce and snLiq.\n");
          error = true;
        }
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
    }
  
  if (!error)
    {
      __noahmp_routines_MOD_redprm(&vegType, &soilType, &slopeType, zSoil, &nSoil, &isUrban);
      __noahmp_routines_MOD_noahmp_sflx(&iLoc, &jLoc, &lat, &yearLen, &julian, &cosZ, &dt, &dx, &dz8w, &nSoil, zSoil, &nSnow, &shdFac, &shdMax, &vegType,
                                        &isUrban, &ice, &ist, &isc, smcEq, &iz0tlnd, &sfcTmp, &sfcPrs, &psfc, &uu, &vv, &q2, &qc, &solDn, &lwDn, &prcp, &tBot,
                                        &co2Air, &o2Air, &folN, evapoTranspirationState->fIceOld, &pblh, &zLvl, &evapoTranspirationState->albOld,
                                        &evapoTranspirationState->snEqvO, evapoTranspirationState->stc, sh2o, smc, &evapoTranspirationState->tah,
                                        &evapoTranspirationState->eah, &evapoTranspirationState->fWet, &evapoTranspirationState->canLiq,
                                        &evapoTranspirationState->canIce, &evapoTranspirationState->tv, &evapoTranspirationState->tg, &qsfc, &qSnow,
                                        &evapoTranspirationState->iSnow, evapoTranspirationState->zSnso, &evapoTranspirationState->snowH,
                                        &evapoTranspirationState->snEqv, evapoTranspirationState->snIce, evapoTranspirationState->snLiq, &zwt, &wa, &wt,
                                        &wsLake, &evapoTranspirationState->lfMass, &evapoTranspirationState->rtMass, &evapoTranspirationState->stMass,
                                        &evapoTranspirationState->wood, &evapoTranspirationState->stblCp, &evapoTranspirationState->fastCp,
                                        &evapoTranspirationState->lai, &evapoTranspirationState->sai, &evapoTranspirationState->cm,
                                        &evapoTranspirationState->ch, &evapoTranspirationState->tauss, &smcwtd, &evapoTranspirationState->deepRech,
                                        &evapoTranspirationState->rech, &fsa, &fsr, &fira, &fsh, &sSoil, &fcev, &fgev, &fctr, &eCan, &eTran, &eDir, &tRad,
                                        &tgb, &tgv, &t2mv, &t2mb, &q2v, &q2b, &runSrf, &runSub, &apar, &psn, &sav, &sag, &fSno, &nee, &gpp, &npp, &fVeg,
                                        &albedo, &qSnBot, &ponding, &ponding1, &ponding2, &rsSun, &rsSha, &bGap, &wGap, &chv, &chb, &emissi, &shg, &shc, &shb,
                                        &evg, &evb, &ghv, &ghb, &irg, &irc, &irb, &tr, &evc, &chLeaf, &chuc, &chv2, &chb2, &fpIce);
      
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
              evapoTranspirationState->fIceOld[ii] = 0.0;
            }
        }
      
      // Calculate derived output variables.
      changeInCanopyIce      = evapoTranspirationState->canIce - canIceOriginal;
      changeInCanopyLiquid   = evapoTranspirationState->canLiq - canLiqOriginal;
      evaporationFromCanopy  = eCan * dt;
      evaporationFromSurface = eDir * dt;
      
      // Surface evaporation is taken first from the snow layer up to the amount in snEqv at the beginning of the timestep, which is now in snEqvO.
      if (evaporationFromSurface <= evapoTranspirationState->snEqvO)
        {
          evaporationFromSnow   = evaporationFromSurface;
          evaporationFromGround = 0.0;
        }
      else
        {
          evaporationFromSnow   = evapoTranspirationState->snEqvO;
          evaporationFromGround = evaporationFromSurface - evapoTranspirationState->snEqvO;
        }
      
      snowfallAboveCanopy         = prcp * dt * fpIce;
      snowfallBelowCanopy         = qSnow * dt;
      snowfallInterceptedByCanopy = snowfallAboveCanopy - snowfallBelowCanopy;
      snowmeltOnGround            = qSnBot * dt;
      rainfallAboveCanopy         = prcp * dt - snowfallAboveCanopy;
      rainfallInterceptedByCanopy = changeInCanopyIce + changeInCanopyLiquid + evaporationFromCanopy - snowfallInterceptedByCanopy;
      rainfallBelowCanopy         = rainfallAboveCanopy - rainfallInterceptedByCanopy;
      
      // Because rainfallBelowCanopy is a derived value it can be slightly negative due to roundoff error.
      // If it is try to take the water from canLiq or canIce.
      if (0.0 > rainfallBelowCanopy)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0f, rainfallBelowCanopy));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          if (evapoTranspirationState->canLiq >= -rainfallBelowCanopy)
            {
              evapoTranspirationState->canLiq += rainfallBelowCanopy;
              rainfallBelowCanopy              = 0.0;
            }
          else
            {
              rainfallBelowCanopy             += evapoTranspirationState->canLiq;
              evapoTranspirationState->canLiq  = 0.0;

              if (evapoTranspirationState->canIce >= -rainfallBelowCanopy)
                {
                  evapoTranspirationState->canIce += rainfallBelowCanopy;
                  rainfallBelowCanopy              = 0.0;
                }
              else
                {
                  rainfallBelowCanopy             += evapoTranspirationState->canIce;
                  evapoTranspirationState->canIce  = 0.0;
                  *waterError                     -= rainfallBelowCanopy;
                  rainfallBelowCanopy              = 0.0;
                }
            }
        }
      
      // If there is a snow layer at the end of the timestep it intercepts all of the rainfall.
      if (0 > evapoTranspirationState->iSnow)
        {
          rainfallInterceptedBySnow = rainfallBelowCanopy;
          rainfallOnGround          = 0.0;
        }
      else
        {
          rainfallInterceptedBySnow = 0.0;
          rainfallOnGround          = rainfallBelowCanopy;
        }
      
      // When the total snow height gets less than 2.5mm the multi-layer snow simulation turns off.  When this happens all of the liquid water in snLiq becomes
      // snowmelt on the ground and snEqv gets set to just the portion in snIce.  However, melting/freezing between snIce and snLiq also happens during the
      // timestep so we can't use the beginning timestep value of snLiq to determine how much to add to snowmeltOnGround.  We have to use the final value of
      // snEqv to back out the value of snowmeltOnGround.
      if (0 > iSnowOriginal && 0 == evapoTranspirationState->iSnow)
        {
          snowmeltOnGround = evapoTranspirationState->snEqvO + snowfallBelowCanopy - evaporationFromSnow - evapoTranspirationState->snEqv;
        }
      
      // If snEqv falls below 0.001 mm then NOAM-MP sets it to zero and the water is lost.  We are calculating what snEqv should be and putting the water back.
      snEqvShouldBe = evapoTranspirationState->snEqvO + snowfallBelowCanopy + rainfallInterceptedBySnow - evaporationFromSnow - snowmeltOnGround;
      
      if (0.0 == evapoTranspirationState->snEqv)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonGreaterOrEqual(0.001f, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          evapoTranspirationState->snEqv = snEqvShouldBe;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(epsilonEqual(evapoTranspirationState->snEqv, snEqvShouldBe));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      CkAssert(!checkEvapoTranspirationStateStructInvariant(evapoTranspirationState));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
    } // End if (!error).
  
  return error;
}

bool checkEvapoTranspirationStateStructInvariant(EvapoTranspirationStateStruct* struc)
{
   bool error = false;
   int  nn ;
   float tmp_f;
   
   for (nn = 0; nn < 3; nn++) // nn = # of snow layers
   {
     if ( struc->fIceOld[nn] < 0) // Also not greater than 1.
      {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, fIceOld[%i]: Ice fraction of snow layer must be greater than 0\n", nn);
        error = true;
      }
   }
   
   if (struc->albOld < 0 || struc->albOld > 1)
   {
     CkError("ERROR in EcheckEvapoTranspirationStateStructInvariant, albOld: Snow albedo must be in the range 0-1\n");
        error = true;
   }

   if (struc->snEqvO < 0)
   {
     CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snEqvO: The value of SWE must be greater than or equal to 0\n");
        error = true;
   }

   for (nn = 0; nn < 7; nn++) // nn = # of snow layers + # of soil layers
   {
     if ( struc->stc[nn] < 0)
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, stc[%i]: Snow layer or soil layer temperature [K] must be greater than 0\n", nn);
        error = true;
       }
     else if  ( struc->stc[nn] < 217.15)
     {
       CkError("WARNING in checkEvapoTranspirationStateStructInvariant, stc[%i]: Snow layer or soil layer temperature [K] is samller than 217.15 K\n", nn);
     }
   }

   if ( struc->tah < 0)
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, tah: Canopy air temperature [K] must be greater than 0\n");
        error = true;
       }
   else if (struc->tah < 217.15)
     {
       CkError("WARNING in checkEvapoTranspirationStateStructInvariant, tah: Canopy air temperature [K] is samller than 217.15 K\n");
       //  217.15 K is the record low temperature for the states: WY,CO, NM, UT, AZ
     }
  
  if ( struc->eah < 0)
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, eah: Canopy water vapor pressure [Pa] is samller than 0 Pa\n");
        error = true;
       }
   if ( struc->fWet < 0) // and not greater than 1
     {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant, fWet: Wetted or snowed fraction of canopy must be greater than or equal to 0\n");
      error = true;
     }
   if ( struc->canLiq < 0)
     {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant, canLiq: intercepted liquid water in canopy must be greater than or equal to 0\n");
      error = true;
     } 
   if ( struc->canIce < 0)
     {
      CkError("ERROR in checkEvapoTranspirationStateStructInvariant, canIce: intercepted solid water in canopy must be greater than or equal to 0\n");
      error = true;
     }
   if ( struc->tv < 0)
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, tv: Vegetation temperature [K] must be greater than 0 K\n");
        error = true;
       }
   else if (struc->tv < 217.15)
     {
       CkError("WARNING in checkEvapoTranspirationStateStructInvariant, tv: Vegetation temperature [K] is smaller than 217.15 K\n");
       //  217.15 K is the record low temperature for the states: WY,CO, NM, UT, AZ
     }
   if ( struc->tg < 0)
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, tg: Ground temperature [K] must be greater than 0 K\n");
        error = true;
       }
   else if (struc->tg < 217.15)
     {
       CkError("WARNING in checkEvapoTranspirationStateStructInvariant, tg: Ground temperature [K] is samller than 217.15 K\n");
        //  217->15 K is the record low temperature for the states: WY,CO, NM, UT, AZ
     }
   if ( struc->lfMass < 0) 
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, lfMass: Leaf mass [g/m^2] must be greater than or equal to 0\n");
        error = true;
       }
   if ( struc->rtMass < 0) 
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, rtMass: Root mass [g/m^2] must be greater than or equal to 0\n");
        error = true;
       }
   if ( struc->stMass < 0) 
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, stMass: Stem mass [g/m^2] must be greater than or equal to 0\n");
        error = true;
       }
   if ( struc->wood < 0)
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, wood: Wood mass including woody roots [g/m^2] must be greater than or equal to 0\n ");
        error = true;
       }
   if ( struc->stblCp < 0)
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, stblCp: Stable carbon in deep soil [g/m^2] must be greater than or equal to 0\n ");
        error = true;
       }
   if ( struc->fastCp < 0)
       {
        CkError("ERROR in checkEvapoTranspirationStateStructInvariant, fastCp: Short lived carbon in shallow soil [g/m^2] must be greater than or equal to 0 \n");
        error = true;
       }
       // stblCp and fastCp not less than zero
   if ( struc->lai < 0 || struc->lai > 6) 
      {
       CkError("WARNING in checkEvapoTranspirationStateStructInvariant, lai: Leaf area index must be in the range 0-6\n");
      }
   if ( struc-> sai < 0 || struc->lai > 0.6)
      {
       CkError("WARNING in checkEvapoTranspirationStateStructInvariant, sai: Stem area index must be in the range 0-0.6\n");
      }
   
   if (struc -> iSnow == 0) // Meaning there is not soil layer formed
    {
       if ( struc ->snEqv < 0 )
       {
           CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snEqv: SWE [mm] must be greater than or equal to 0 \n");
           error = true;
       }
       for (nn = 0; nn < 3; nn++)  
       { 
          if (struc->snIce[nn] != 0)
          {
               CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snIce[%i]: snIce [mm] must be equal to 0 \n", nn);
               error = true;
          }
          
          if (struc->snLiq[nn] != 0)
          {
               CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snLiq[%i]: snLiq [mm] must be equal to 0 \n", nn);
               error = true;
          }
          
          if (struc->zSnso[nn] != 0)
          {
               CkError("ERROR in checkEvapoTranspirationStateStructInvariant, zSnso[%i]: snow layer height [m] must be equal to 0 \n", nn);
               error = true;
          }
       }
    }
    else if (struc -> iSnow < 0)
    {
       if (struc->snowH <= 0)
       {
           CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snowH: snow height [m] must be greater than 0 when iSnow != 0\n");
           error = true;
       }
       
       if ( !epsilonEqual(struc->snowH, struc->zSnso[2] )  ) 
       {
           CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snowH: snow height [m] must be equal to zSnso[2] when iSnow != 0\n");
           error = true;
       }
       tmp_f = 0;
       for (nn = 0; nn > 3; nn++)  
       {
          if (struc->snIce[nn] < 0)
          {
               CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snIce[%i]: snIce [mm] must be equal to or greater than 0\n ", nn);
               error = true;
          }
          
          if (struc->snLiq[nn] < 0)
          {
               CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snLiq[%i]: snLiq [mm] must be equal to or greater than 0\n ", nn);
               error = true;
          }
          
          if (struc->zSnso[nn] < 0)
          {
               CkError("ERROR in checkEvapoTranspirationStateStructInvariant, zSnso[%i]: Snow layer height [m] must be equal to or greater than 0 \n", nn);
               error = true;
          }
          
          tmp_f += struc-> snIce[nn];
          tmp_f += struc-> snLiq[nn];
       }
 
       if ( !epsilonEqual(struc ->snEqv, tmp_f ) ) 
       {
           CkError("ERROR in checkEvapoTranspirationStateStructInvariant, snEqv: SWE [mm] must be equal to snIce[3] + snLiq[3] when iSnow != 0\n");
           error = true;
       }
   }
   else // if (struc -> iSnow > 0)
    {
       CkError("ERROR in checkEvapoTranspirationStateStructInvariant, iSnow: # of snow layers must be smaller than 0\n");
       error = true;
    }
     
return error;
}


