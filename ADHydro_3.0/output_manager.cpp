#include "output_manager.h"
#include "all.h"

OutputManager::OutputManager(FileManager* fileManagerInit) :
fileManager(fileManagerInit)
{
  if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      if (!(NULL != fileManagerInit))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::OutputManager: fileManagerInit must not be NULL.\n");
          ADHYDRO_EXIT;
        }
    }
}

void OutputManager::createFiles()
{
  bool error = false;       // Error flag.
  int  ii;                  // Loop counter.
  int  numberOfOutputFiles; // Total number of output files for the entire run.
  int  outputTime;          // The time point of the next output file to create.

  if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      if (!(0 <= myOutputManagerIndex() && myOutputManagerIndex() < numberOfOutputManagers()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: myOutputManagerIndex must be greater than or equal to zero and less than numberOfOutputManagers.\n");
          error = true;
        }

      if (!(1721425.5 <= referenceDate()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: referenceDate must be on or after 1 CE (1721425.5).\n");
          error = true;
        }

      if (!(1721425.5 <= referenceDate() + (simulationStartTime() / (60.0 * 60.0 * 24.0))))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: the calendar date of simulationStartTime must be on or after 1 CE (1721425.5).\n");
          error = true;
        }

      if (!(0.0 <= simulationDuration()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: simulationDuration must be greater than or equal to zero.\n");
          error = true;
        }

      if (!(0.0 < outputPeriod()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: outputPeriod must be greater than zero.\n");
          error = true;
        }

      if (!(0 < globalNumberOfMeshElements()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: globalNumberOfMeshElements must be greater than zero.\n");
          error = true;
        }

      if (!(0 < numberOfMeshSoilLayers()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: numberOfMeshSoilLayers must be greater than zero.\n");
          error = true;
        }

      if (!(0 < numberOfMeshNeighbors()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: numberOfMeshNeighbors must be greater than zero.\n");
          error = true;
        }

      if (!(0 < globalNumberOfChannelElements()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: globalNumberOfChannelElements must be greater than zero.\n");
          error = true;
        }

      if (!(0 < numberOfChannelNeighbors()))
        {
          ADHYDRO_ERROR("ERROR in OutputManager::createFiles: numberOfChannelNeighbors must be greater than zero.\n");
          error = true;
        }
    }

  if (!error)
    {
      // Calculate the number of output files to create.
      numberOfOutputFiles = std::ceil(simulationDuration() / outputPeriod());

      // If simulationDuration is zero or outputPeriod is infinity then numberOfOutputFiles could come out as zero.
      // In that case, we want to output once at the end of the simulation.
      if (0 == numberOfOutputFiles)
        {
          numberOfOutputFiles = 1;
        }

      if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        {
          ADHYDRO_ASSERT(0 < numberOfOutputFiles);
        }
    }

  // Create all of the output files.  Each file will be created serially by one OutputManager with multiple files handled by multiple OutputManagers.
  for (ii = myOutputManagerIndex() + 1; !error && ii <= numberOfOutputFiles; ii += numberOfOutputManagers())
    {
      // Get the time point for the next output file to create.
      if (ii < numberOfOutputFiles)
        {
          outputTime = simulationStartTime() + outputPeriod() * ii;
        }
      else
        {
          // For the last file use simulationDuration rather than a multiple of outputPeriod.
          // simulationDuration might not be an exact multiple of outputPeriod, and even if it is, this avoids roundoff error.
          outputTime = simulationStartTime() + simulationDuration();
        }

      error = fileManager->createFile(outputTime);
    }

  if (error)
    {
      ADHYDRO_EXIT;
    }
}
