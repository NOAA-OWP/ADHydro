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
                                                  int* ice, int* ist, int* isc, float* smceq, int* iz0tlnd, float* sfcTmp, float* sfcPrs, float* psfc,
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

bool evapoTranspirationSoil(int vegType, int soilType, double soilThickness, float lat, int yearLen, float julian, float cosZ, float dt, float dx, float dz8w,
                            float shdFac, float shdMax, float smceq[4], float sfcTmp, float sfcPrs, float psfc, float uu, float vv, float q2, float qc,
                            float solDn, float lwDn, float prcp, float tBot, float pblh, float sh2o[4], float smc[4], float zwt, float wa, float wt,
                            float wsLake, float smcwtd, EvapoTranspirationStateStruct* evapoTranspirationState)
{
  bool error = false; // Error flag.
  int  ii;            // Loop counter.
  
  // Input parameters for redprm function.  Some are also used for sflx function.
  int   slopeType = 8; // I just arbitrarily chose a slope type with zero slope.  I think slope is only used to calculate runoff, which we ignore. FIXME
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
  float fIceOld[3];                // Frozen fraction of each snow layer, unitless.  Values are set below from snIce and snLiq.
  float zLvl    = dz8w;            // Thickness in meters of lowest atmosphere layer in forcing data.  Redundant with dz8w.
  
  // Input/output parameters to sflx function.
  float qsfc   = q2;  // This is actually an input only variable.  Water vapor mixing ratio, unitless, at middle of lowest atmosphere layer in forcing data.
                      // Redundant with q2.
  float qSnow  = NAN; // This is actually an output only variable.  Snowfall rate below the canopy in millimeters of water equivalent per second.

  // Output parameters to sflx function.  Set to NAN so we can detect if the values are used before being set.  FIXME comment
  float fsa      = NAN;
  float fsr      = NAN;
  float fira     = NAN;
  float fsh      = NAN;
  float sSoil    = NAN;
  float fcev     = NAN;
  float fgev     = NAN;
  float fctr     = NAN;
  float eCan     = NAN;
  float eTran    = NAN;
  float eDir     = NAN;
  float tRad     = NAN;
  float tgb      = NAN;
  float tgv      = NAN;
  float t2mv     = NAN;
  float t2mb     = NAN;
  float q2v      = NAN;
  float q2b      = NAN;
  float runSrf   = NAN;
  float runSub   = NAN;
  float apar     = NAN;
  float psn      = NAN;
  float sav      = NAN;
  float sag      = NAN;
  float fSno     = NAN;
  float nee      = NAN;
  float gpp      = NAN;
  float npp      = NAN;
  float fVeg     = NAN;
  float albedo   = NAN;
  float qSnBot   = NAN;
  float ponding  = NAN;
  float ponding1 = NAN;
  float ponding2 = NAN;
  float rsSun    = NAN;
  float rsSha    = NAN;
  float bGap     = NAN;
  float wGap     = NAN;
  float chv      = NAN;
  float chb      = NAN;
  float emissi   = NAN;
  float shg      = NAN;
  float shc      = NAN;
  float shb      = NAN;
  float evg      = NAN;
  float evb      = NAN;
  float ghv      = NAN;
  float ghb      = NAN;
  float irg      = NAN;
  float irc      = NAN;
  float irb      = NAN;
  float tr       = NAN;
  float evc      = NAN;
  float chLeaf   = NAN;
  float chuc     = NAN;
  float chv2     = NAN;
  float chb2     = NAN;
  float fpIce    = NAN;
  
  // Derived output variables.
  float evaporationFromCanopy;                            // Quantity of evaporation from canopy in millimeters of water equivalent.
  float canIceOriginal = evapoTranspirationState->canIce; // Quantity of canopy ice before timestep in millimeters of water equivalent.
  float changeInCanopyIce;                                // Change in canopy ice during timestep in millimeters of water equivalent.
  float canLiqOriginal = evapoTranspirationState->canLiq; // Quantity of canopy liquid before timestep in millimeters of water.
  float changeInCanopyLiquid;                             // Change in canopy liquid during timestep in millimeters of water.
  float snowAboveCanopy;                                  // Quantity of snowfall above the canopy in millimeters of water equivalent.
  float snowInterceptedByCanopy;                          // Quantity of snowfall intercepted by the canopy in millimeters of water equivalent.
                                                          // Can be negative if snow is falling off of the canopy.
  float snowBelowCanopy;                                  // Quantity of snowfall below the canopy in millimeters of water equivalent.
  float rainAboveCanopy;                                  // Quantity of rainfall above the canopy in millimeters of water.
  float rainInterceptedByCanopy;                          // Quantity of rainfall intercepted by the canopy in millimeters of water.
                                                          // Can be negative if rain is dripping from the canopy.
  float rainBelowCanopy;                                  // Quantity of rainfall below the canopy in millimeters of water.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  // Variables used for assertions.
  int   iLocOriginal     = iLoc;
  int   jLocOriginal     = jLoc;
  float latOriginal      = lat;
  int   yearLenOriginal  = yearLen;
  float julianOriginal   = julian;
  float cosZOriginal     = cosZ;
  float dtOriginal       = dt;
  float dxOriginal       = dx;
  float dz8wOriginal     = dz8w;
  int   nSoilOriginal    = nSoil;
  float zSoilOriginal[4] = {zSoil[0], zSoil[1], zSoil[2], zSoil[3]};
  int   nSnowOriginal    = nSnow;
  float shdFacOriginal   = shdFac;
  float shdMaxOriginal   = shdMax;
  int   vegTypeOriginal  = vegType;
  int   isUrbanOriginal  = isUrban;
  int   iceOriginal      = ice;
  int   istOriginal      = ist;
  int   iscOriginal      = isc;
  float smceqOriginal[4] = {smceq[0], smceq[1], smceq[2], smceq[3]};
  int   iz0tlndOriginal  = iz0tlnd;
  float sfcTmpOriginal   = sfcTmp;
  float sfcPrsOriginal   = sfcPrs;
  float psfcOriginal     = psfc;
  float uuOriginal       = uu;
  float vvOriginal       = vv;
  float q2Original       = q2;
  float qcOriginal       = qc;
  float solDnOriginal    = solDn;
  float lwDnOriginal     = lwDn;
  float prcpOriginal     = prcp;
  float tBotOriginal     = tBot;
  float co2AirOriginal   = co2Air;
  float o2AirOriginal    = o2Air;
  float folNOriginal     = folN;
  float fIceOldOriginal[3];
  float pblhOriginal     = pblh;
  float zLvlOriginal     = zLvl;
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
  
  // FIXME do other checks on inputs.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

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
    }
  
  // Set fIceOld from snIce and snLiq.
  for (ii = 0; ii < 3; ii++)
    {
      if (ii - 2 > evapoTranspirationState->iSnow)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
          if (!(0.0 < evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]))
            {
              CkError("ERROR in evapoTranspirationSoil: snow layer %d exists with no water in it.\n", ii - 2);
              error = true;
            }
          else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
            {
              fIceOld[ii] = evapoTranspirationState->snIce[ii] / (evapoTranspirationState->snIce[ii] + evapoTranspirationState->snLiq[ii]);
            }
        }
      else
        {
          fIceOld[ii] = 0.0;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      fIceOldOriginal[ii] = fIceOld[ii];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  if (!error)
    {
      __noahmp_routines_MOD_redprm(&vegType, &soilType, &slopeType, zSoil, &nSoil, &isUrban);
      __noahmp_routines_MOD_noahmp_sflx(&iLoc, &jLoc, &lat, &yearLen, &julian, &cosZ, &dt, &dx, &dz8w, &nSoil, zSoil, &nSnow, &shdFac, &shdMax, &vegType,
                                        &isUrban, &ice, &ist, &isc, smceq, &iz0tlnd, &sfcTmp, &sfcPrs, &psfc, &uu, &vv, &q2, &qc, &solDn, &lwDn, &prcp, &tBot,
                                        &co2Air, &o2Air, &folN, fIceOld, &pblh, &zLvl, &evapoTranspirationState->albOld, &evapoTranspirationState->snEqvO,
                                        evapoTranspirationState->stc, sh2o, smc, &evapoTranspirationState->tah, &evapoTranspirationState->eah,
                                        &evapoTranspirationState->fWet, &evapoTranspirationState->canLiq, &evapoTranspirationState->canIce,
                                        &evapoTranspirationState->tv, &evapoTranspirationState->tg, &qsfc, &qSnow, &evapoTranspirationState->iSnow,
                                        evapoTranspirationState->zSnso, &evapoTranspirationState->snowH, &evapoTranspirationState->snEqv,
                                        evapoTranspirationState->snIce, evapoTranspirationState->snLiq, &zwt, &wa, &wt, &wsLake,
                                        &evapoTranspirationState->lfMass, &evapoTranspirationState->rtMass, &evapoTranspirationState->stMass,
                                        &evapoTranspirationState->wood, &evapoTranspirationState->stblCp, &evapoTranspirationState->fastCp,
                                        &evapoTranspirationState->lai, &evapoTranspirationState->sai, &evapoTranspirationState->cm,
                                        &evapoTranspirationState->ch, &evapoTranspirationState->tauss, &smcwtd, &evapoTranspirationState->deepRech,
                                        &evapoTranspirationState->rech, &fsa, &fsr, &fira, &fsh, &sSoil, &fcev, &fgev, &fctr, &eCan, &eTran, &eDir, &tRad,
                                        &tgb, &tgv, &t2mv, &t2mb, &q2v, &q2b, &runSrf, &runSub, &apar, &psn, &sav, &sag, &fSno, &nee, &gpp, &npp, &fVeg,
                                        &albedo, &qSnBot, &ponding, &ponding1, &ponding2, &rsSun, &rsSha, &bGap, &wGap, &chv, &chb, &emissi, &shg, &shc, &shb,
                                        &evg, &evb, &ghv, &ghb, &irg, &irc, &irb, &tr, &evc, &chLeaf, &chuc, &chv2, &chb2, &fpIce);
      
      // Calculate derived output variables.
      evaporationFromCanopy   = eCan * dt;
      changeInCanopyIce       = evapoTranspirationState->canIce - canIceOriginal;
      changeInCanopyLiquid    = evapoTranspirationState->canLiq - canLiqOriginal;
      snowAboveCanopy         = prcp * dt * fpIce;
      snowBelowCanopy         = qSnow * dt;
      snowInterceptedByCanopy = snowAboveCanopy - snowBelowCanopy;
      rainAboveCanopy         = prcp * dt - snowAboveCanopy;
      rainInterceptedByCanopy = evaporationFromCanopy + changeInCanopyIce + changeInCanopyLiquid - snowInterceptedByCanopy;
      rainBelowCanopy         = rainAboveCanopy - rainInterceptedByCanopy;
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      // Verify that the input variables have not changed.
      CkAssert(iLocOriginal == iLoc && jLocOriginal == jLoc && latOriginal == lat && yearLenOriginal == yearLen && julianOriginal == julian &&
               cosZOriginal == cosZ && dtOriginal == dt && dxOriginal == dx && dz8wOriginal == dz8w && nSoilOriginal == nSoil &&
               zSoilOriginal[0] == zSoil[0] && zSoilOriginal[1] == zSoil[1] && zSoilOriginal[2] == zSoil[2] && zSoilOriginal[3] == zSoil[3] &&
               nSnowOriginal == nSnow && shdFacOriginal == shdFac && shdMaxOriginal == shdMax && vegTypeOriginal == vegType && isUrbanOriginal == isUrban &&
               iceOriginal == ice && istOriginal == ist && iscOriginal == isc && smceqOriginal[0] == smceq[0] && smceqOriginal[1] == smceq[1] &&
               smceqOriginal[2] == smceq[2] && smceqOriginal[3] == smceq[3] && iz0tlndOriginal == iz0tlnd && sfcTmpOriginal == sfcTmp &&
               sfcPrsOriginal == sfcPrs && psfcOriginal == psfc && uuOriginal == uu && vvOriginal == vv && q2Original == q2 && qcOriginal == qc &&
               solDnOriginal == solDn && lwDnOriginal == lwDn && prcpOriginal == prcp && tBotOriginal == tBot && co2AirOriginal == co2Air &&
               o2AirOriginal == o2Air && folNOriginal == folN && fIceOldOriginal[0] == fIceOld[0] && fIceOldOriginal[1] == fIceOld[1] &&
               fIceOldOriginal[2] == fIceOld[2] && pblhOriginal == pblh && zLvlOriginal == zLvl);
      
      // Verify that the fraction of the precipitation that falls as snow is between 0 and 1.
      CkAssert(0.0f <= fpIce && 1.0f >= fpIce);
      
      // Verify that the snowfall rate below the canopy is not negative.
      CkAssert(0.0f <= qSnow);
      
      // Verify that the rainfall below the canopy is not epsilon negative.
      // Because it is a derived value it can be slightly negative due to roundoff error.
      // FIXME If it is try to take the water from canLiq or canIce.
      CkAssert(epsilonLessOrEqual(0.0f, rainBelowCanopy));
      
      // FIXME remove
      // It appears that the output value of qsfc is always 0.  Check this.
      CkAssert(0.0f == qsfc);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  return error;
}
