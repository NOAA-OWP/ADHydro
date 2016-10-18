#include "file_manager.h"
#include <iomanip>

FileManager::TimePointState::TimePointState(const std::string& directoryInit, double referenceDateInit, double outputTimeInit, size_t globalNumberOfMeshElementsInit, size_t localNumberOfMeshElementsInit,
    size_t localMeshElementStartInit, size_t maximumNumberOfMeshSoilLayersInit,  size_t maximumNumberOfMeshNeighborsInit, size_t globalNumberOfChannelElementsInit,
    size_t localNumberOfChannelElementsInit, size_t localChannelElementStartInit, size_t maximumNumberOfChannelNeighborsInit) :
    directory(directoryInit),
    referenceDate(referenceDateInit),
    outputTime(outputTimeInit),
    globalNumberOfMeshElements(globalNumberOfMeshElementsInit),
    localNumberOfMeshElements(localNumberOfMeshElementsInit),
    localMeshElementStart(localMeshElementStartInit),
    maximumNumberOfMeshSoilLayers(maximumNumberOfMeshSoilLayersInit),
    maximumNumberOfMeshNeighbors(maximumNumberOfMeshNeighborsInit),
    globalNumberOfChannelElements(globalNumberOfChannelElementsInit),
    localNumberOfChannelElements(localNumberOfChannelElementsInit),
    localChannelElementStart(localChannelElementStartInit),
    maximumNumberOfChannelNeighbors(maximumNumberOfChannelNeighborsInit),
    elementsReceived(0)
{
  if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      if (!(1721425.5 <= referenceDate))
        {
          ADHYDRO_ERROR("ERROR in FileManager::TimePointState::TimePointState: referenceDate must be on or after 1 CE (1721425.5).\n");
          ADHYDRO_EXIT;
        }

      if (!(1721425.5 <= referenceDate + (outputTime / (60.0 * 60.0 * 24.0))))
        {
          ADHYDRO_ERROR("ERROR in FileManager::TimePointState::TimePointState: the calendar date of outputTime must be on or after 1 CE (1721425.5).\n");
          ADHYDRO_EXIT;
        }

      if (!(localNumberOfMeshElements <= globalNumberOfMeshElements))
        {
          ADHYDRO_ERROR("ERROR in FileManager::TimePointState::TimePointState: localNumberOfMeshElements must be less than or equal to globalNumberOfMeshElements.\n");
          ADHYDRO_EXIT;
        }

      if (!((0 != localNumberOfMeshElements && localMeshElementStart < globalNumberOfMeshElements) || (0 == localNumberOfMeshElements && 0 == localMeshElementStart)))
        {
          ADHYDRO_ERROR("ERROR in FileManager::TimePointState::TimePointState: localMeshElementStart must be less than globalNumberOfMeshElements or zero if localNumberOfMeshElements is zero.\n");
          ADHYDRO_EXIT;
        }

      if (!(localNumberOfChannelElements <= globalNumberOfChannelElements))
        {
          ADHYDRO_ERROR("ERROR in FileManager::TimePointState::TimePointState: localNumberOfChannelElements must be less than or equal to globalNumberOfChannelElements.\n");
          ADHYDRO_EXIT;
        }

      if (!((0 != localNumberOfChannelElements && localChannelElementStart < globalNumberOfChannelElements) || (0 == localNumberOfChannelElements && 0 == localChannelElementStart)))
        {
          ADHYDRO_ERROR("ERROR in FileManager::TimePointState::TimePointState: localChannelElementStart must be less than globalNumberOfChannelElements or zero if localNumberOfChannelElements is zero.\n");
          ADHYDRO_EXIT;
        }
    }

  if (0 < localNumberOfMeshElements)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      meshStateReceived       = new bool[  localNumberOfMeshElements];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      meshSurfacewaterDepth   = new double[localNumberOfMeshElements];
      meshSurfacewaterCreated = new double[localNumberOfMeshElements];

      if (0 < maximumNumberOfMeshSoilLayers)
        {
          meshGroundwaterHead     = new double[             localNumberOfMeshElements * maximumNumberOfMeshSoilLayers];
          meshGroundwaterRecharge = new double[             localNumberOfMeshElements * maximumNumberOfMeshSoilLayers];
          meshGroundwaterCreated  = new double[             localNumberOfMeshElements * maximumNumberOfMeshSoilLayers];
          meshVadoseZoneState     = new VadoseZoneStateBlob[localNumberOfMeshElements * maximumNumberOfMeshSoilLayers];
          meshRootZoneWater       = new double[             localNumberOfMeshElements * maximumNumberOfMeshSoilLayers];
          meshTotalSoilWater      = new double[             localNumberOfMeshElements * maximumNumberOfMeshSoilLayers];
        }
      else
        {
          meshGroundwaterHead     = NULL;
          meshGroundwaterRecharge = NULL;
          meshGroundwaterCreated  = NULL;
          meshVadoseZoneState     = NULL;
          meshRootZoneWater       = NULL;
          meshTotalSoilWater      = NULL;
        }

      meshPrecipitationRate       = new double[                     localNumberOfMeshElements];
      meshPrecipitationCumulative = new double[                     localNumberOfMeshElements];
      meshEvaporationRate         = new double[                     localNumberOfMeshElements];
      meshEvaporationCumulative   = new double[                     localNumberOfMeshElements];
      meshTranspirationRate       = new double[                     localNumberOfMeshElements];
      meshTranspirationCumulative = new double[                     localNumberOfMeshElements];
      meshEvapoTranspirationState = new EvapoTranspirationStateBlob[localNumberOfMeshElements];
      meshCanopyWaterEquivalent   = new double[                     localNumberOfMeshElements];
      meshSnowWaterEquivalent     = new double[                     localNumberOfMeshElements];

      if (0 < maximumNumberOfMeshNeighbors)
        {
          meshSurfacewaterNeighborsExpirationTime = new double[localNumberOfMeshElements * maximumNumberOfMeshNeighbors];
          meshSurfacewaterNeighborsFlowRate       = new double[localNumberOfMeshElements * maximumNumberOfMeshNeighbors];
          meshSurfacewaterNeighborsFlowCumulative = new double[localNumberOfMeshElements * maximumNumberOfMeshNeighbors];

          if (0 < maximumNumberOfMeshSoilLayers)
            {
              meshGroundwaterNeighborsExpirationTime = new double[localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors];
              meshGroundwaterNeighborsFlowRate       = new double[localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors];
              meshGroundwaterNeighborsFlowCumulative = new double[localNumberOfMeshElements * maximumNumberOfMeshSoilLayers * maximumNumberOfMeshNeighbors];
            }
          else
            {
              meshGroundwaterNeighborsExpirationTime = NULL;
              meshGroundwaterNeighborsFlowRate       = NULL;
              meshGroundwaterNeighborsFlowCumulative = NULL;
            }
        }
      else
        {
          meshSurfacewaterNeighborsExpirationTime = NULL;
          meshSurfacewaterNeighborsFlowRate       = NULL;
          meshSurfacewaterNeighborsFlowCumulative = NULL;
          meshGroundwaterNeighborsExpirationTime  = NULL;
          meshGroundwaterNeighborsFlowRate        = NULL;
          meshGroundwaterNeighborsFlowCumulative  = NULL;
        }
    }
  else
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      meshStateReceived                       = NULL;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      meshSurfacewaterDepth                   = NULL;
      meshSurfacewaterCreated                 = NULL;
      meshGroundwaterHead                     = NULL;
      meshGroundwaterRecharge                 = NULL;
      meshGroundwaterCreated                  = NULL;
      meshPrecipitationRate                   = NULL;
      meshPrecipitationCumulative             = NULL;
      meshEvaporationRate                     = NULL;
      meshEvaporationCumulative               = NULL;
      meshTranspirationRate                   = NULL;
      meshTranspirationCumulative             = NULL;
      meshEvapoTranspirationState             = NULL;
      meshCanopyWaterEquivalent               = NULL;
      meshSnowWaterEquivalent                 = NULL;
      meshVadoseZoneState                     = NULL;
      meshRootZoneWater                       = NULL;
      meshTotalSoilWater                      = NULL;
      meshSurfacewaterNeighborsExpirationTime = NULL;
      meshSurfacewaterNeighborsFlowRate       = NULL;
      meshSurfacewaterNeighborsFlowCumulative = NULL;
      meshGroundwaterNeighborsExpirationTime  = NULL;
      meshGroundwaterNeighborsFlowRate        = NULL;
      meshGroundwaterNeighborsFlowCumulative  = NULL;
    }

  if (0 < localNumberOfChannelElements)
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      channelStateReceived           = new bool[                       localNumberOfChannelElements];
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      channelSurfacewaterDepth       = new double[                     localNumberOfChannelElements];
      channelSurfacewaterCreated     = new double[                     localNumberOfChannelElements];
      channelPrecipitationRate       = new double[                     localNumberOfChannelElements];
      channelPrecipitationCumulative = new double[                     localNumberOfChannelElements];
      channelEvaporationRate         = new double[                     localNumberOfChannelElements];
      channelEvaporationCumulative   = new double[                     localNumberOfChannelElements];
      channelEvapoTranspirationState = new EvapoTranspirationStateBlob[localNumberOfChannelElements];
      channelSnowWaterEquivalent     = new double[                     localNumberOfChannelElements];

      if (0 < maximumNumberOfChannelNeighbors)
        {
          channelSurfacewaterNeighborsExpirationTime = new double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors];
          channelSurfacewaterNeighborsFlowRate       = new double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors];
          channelSurfacewaterNeighborsFlowCumulative = new double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors];
          channelGroundwaterNeighborsExpirationTime  = new double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors];
          channelGroundwaterNeighborsFlowRate        = new double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors];
          channelGroundwaterNeighborsFlowCumulative  = new double[localNumberOfChannelElements * maximumNumberOfChannelNeighbors];
        }
      else
        {
          channelSurfacewaterNeighborsExpirationTime = NULL;
          channelSurfacewaterNeighborsFlowRate       = NULL;
          channelSurfacewaterNeighborsFlowCumulative = NULL;
          channelGroundwaterNeighborsExpirationTime  = NULL;
          channelGroundwaterNeighborsFlowRate        = NULL;
          channelGroundwaterNeighborsFlowCumulative  = NULL;
        }
    }
  else
    {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
      channelStateReceived                       = NULL;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

      channelSurfacewaterDepth                   = NULL;
      channelSurfacewaterCreated                 = NULL;
      channelPrecipitationRate                   = NULL;
      channelPrecipitationCumulative             = NULL;
      channelEvaporationRate                     = NULL;
      channelEvaporationCumulative               = NULL;
      channelEvapoTranspirationState             = NULL;
      channelSnowWaterEquivalent                 = NULL;
      channelSurfacewaterNeighborsExpirationTime = NULL;
      channelSurfacewaterNeighborsFlowRate       = NULL;
      channelSurfacewaterNeighborsFlowCumulative = NULL;
      channelGroundwaterNeighborsExpirationTime  = NULL;
      channelGroundwaterNeighborsFlowRate        = NULL;
      channelGroundwaterNeighborsFlowCumulative  = NULL;
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  clearReceivedFlags();
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
}

FileManager::TimePointState::~TimePointState()
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
  delete[] meshStateReceived;
  delete[] channelStateReceived;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

  delete[] meshSurfacewaterDepth;
  delete[] meshSurfacewaterCreated;
  delete[] meshGroundwaterHead;
  delete[] meshGroundwaterRecharge;
  delete[] meshGroundwaterCreated;
  delete[] meshPrecipitationRate;
  delete[] meshPrecipitationCumulative;
  delete[] meshEvaporationRate;
  delete[] meshEvaporationCumulative;
  delete[] meshTranspirationRate;
  delete[] meshTranspirationCumulative;
  delete[] meshEvapoTranspirationState;
  delete[] meshCanopyWaterEquivalent;
  delete[] meshSnowWaterEquivalent;
  delete[] meshVadoseZoneState;
  delete[] meshRootZoneWater;
  delete[] meshTotalSoilWater;
  delete[] meshSurfacewaterNeighborsExpirationTime;
  delete[] meshSurfacewaterNeighborsFlowRate;
  delete[] meshSurfacewaterNeighborsFlowCumulative;
  delete[] meshGroundwaterNeighborsExpirationTime;
  delete[] meshGroundwaterNeighborsFlowRate;
  delete[] meshGroundwaterNeighborsFlowCumulative;
  delete[] channelSurfacewaterDepth;
  delete[] channelSurfacewaterCreated;
  delete[] channelPrecipitationRate;
  delete[] channelPrecipitationCumulative;
  delete[] channelEvaporationRate;
  delete[] channelEvaporationCumulative;
  delete[] channelEvapoTranspirationState;
  delete[] channelSnowWaterEquivalent;
  delete[] channelSurfacewaterNeighborsExpirationTime;
  delete[] channelSurfacewaterNeighborsFlowRate;
  delete[] channelSurfacewaterNeighborsFlowCumulative;
  delete[] channelGroundwaterNeighborsExpirationTime;
  delete[] channelGroundwaterNeighborsFlowRate;
  delete[] channelGroundwaterNeighborsFlowCumulative;
}

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
void FileManager::TimePointState::clearReceivedFlags()
{
  size_t ii; // Loop counter.

  for (ii = 0; ii < localNumberOfMeshElements; ++ii)
    {
      meshStateReceived[ii] = false;
    }

  for (ii = 0; ii < localNumberOfChannelElements; ++ii)
    {
      channelStateReceived[ii] = false;
    }
}
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)

std::string FileManager::TimePointState::createFilename() const
{
  long               year;     // For adding date and time to fileneme.
  long               month;    // For adding date and time to fileneme.
  long               day;      // For adding date and time to fileneme.
  long               hour;     // For adding date and time to fileneme.
  long               minute;   // For adding date and time to fileneme.
  double             second;   // For adding date and time to fileneme.
  std::ostringstream filename; // For constructing return value.

  julianToGregorian(referenceDate + (outputTime / (60.0 * 60.0 * 24.0)), &year, &month, &day, &hour, &minute, &second);

  filename << directory << "/state_" << std::setfill('0') << std::setw(4) << year << std::setw(2) << month << std::setw(2) << day
      << std::setw(2) << hour << std::setw(2) << minute << std::fixed << std::setprecision(0) << std::setw(2) << second << ".nc";

  return filename.str();
}
