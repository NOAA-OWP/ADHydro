#ifndef __CHECKPOINT_MANAGER_H__
#define __CHECKPOINT_MANAGER_H__

#include "time_point_state.h"
#include "readonly.h"
#include "checkpoint_manager.decl.h"

// CheckpointManager is a Charm++ group that outputs checkpoints to state files.  Each time point is output to a separate file.
// The purpose of this is to make the early time points available for use as soon as possible.
// We also believe it might lead to some performance improvements dealing with smaller files.
class CheckpointManager : public CBase_CheckpointManager
{
    CheckpointManager_SDAG_CODE
    
public:
    
    // Constructor.  Sets checkpointData to the proper size filled in with NULLs.
    inline CheckpointManager() : checkpointData(), nextOutputIndex(1), endOutputIndex(1)
    {
        size_t numberOfOutputFiles = std::ceil(Readonly::simulationDuration / Readonly::checkpointPeriod); // Total number of output files for the entire run.
        
        // If checkpointPeriod is infinity then numberOfOutputFiles will be zero.  In that case, we want to output once at the end of the simulation.
        if (0 == numberOfOutputFiles)
        {
            numberOfOutputFiles = 1;
        }
        
        checkpointData.assign(numberOfOutputFiles + 1, NULL);
        
        // FIXME if this OutputManager has no elements for a particular time point then it will never receive a message and it will hang.
        // We need to figure out some way to signal this condition and go on because it doesn't need to wait for data.
        
        thisProxy[CkMyPe()].runUntilSimulationEnd();
    }
    
private:
    
    // FIXME comment
    inline bool readyToOutput()
    {
        bool   ready = true; // Return value.  Stays true until we find something that is not ready.
        size_t ii;           // Loop counter.
        
        // We are going to output the next checkpointGroupSize time points together so we are only ready if they are all ready.
        for (ii = nextOutputIndex; ready && ii < nextOutputIndex + Readonly::checkpointGroupSize && ii < checkpointData.size(); ++ii)
        {
            // If a TimePointState hasn't been created yet it is not ready.  Otherwise, check if all of its state has been received.
            ready = ((NULL != checkpointData[ii]) && (checkpointData[ii]->localNumberOfMeshElements + checkpointData[ii]->localNumberOfChannelElements == checkpointData[ii]->elementsReceived));
        }
        
        return ready;
    }
    
    // FIXME comment
    inline double calculateCheckpointTime(size_t checkpointIndex)
    {
        double checkpointTime; // (s) Return value.
        
        if (checkpointIndex < checkpointData.size() - 1)
        {
            checkpointTime = Readonly::simulationStartTime + Readonly::checkpointPeriod * checkpointIndex;
        }
        else
        {
            // For the last file use simulationDuration rather than a multiple of checkpointPeriod.
            // simulationDuration might not be an exact multiple of checkpointPeriod, and even if it is, this avoids roundoff error.
            checkpointTime = Readonly::simulationStartTime + Readonly::simulationDuration;
        }
        
        return checkpointTime;
    }
    
    std::vector<TimePointState*> checkpointData;  // Sets of data for different time points.  The size of checkpointData is the number of output files plus one and checkpointData[0] is unused.
    size_t                       nextOutputIndex; // The next index in checkpointData to output.  Goes from one to the number of output files.
    size_t                       endOutputIndex;  // Needed by SDAG code to span serial blocks.
};

#endif // __CHECKPOINT_MANAGER_H__
