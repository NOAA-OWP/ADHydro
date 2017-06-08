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
    inline CheckpointManager() : checkpointData(Readonly::getNumberOfCheckpoints() + 1, NULL), nextCheckpointIndex(1), endOutputIndex(1)
    {
        // FIXME if this OutputManager has no elements for a particular time point then it will never receive a message and it will hang.
        // We need to figure out some way to signal this condition and go on because it doesn't need to wait for data.
        
        thisProxy[CkMyPe()].runUntilSimulationEnd();
    }
    
    // FIXME checkInvariant?
    
private:
    
    // Returns: true if the CheckpointManager has all of the data it needs to output the next checkpoint(s).
    inline bool readyToOutput()
    {
        bool   ready = true; // Return value.  Stays true until we find something that is not ready.
        size_t ii;           // Loop counter.
        
        // We are going to output the next checkpointGroupSize time points together so we are only ready if they are all ready.
        for (ii = nextCheckpointIndex; ready && ii < nextCheckpointIndex + Readonly::checkpointGroupSize && ii < checkpointData.size(); ++ii)
        {
            // If a TimePointState hasn't been created yet it is not ready.  Otherwise, check if all of its state has been received.
            ready = ((NULL != checkpointData[ii]) && (checkpointData[ii]->localNumberOfMeshElements + checkpointData[ii]->localNumberOfChannelElements == checkpointData[ii]->elementsReceived));
        }
        
        return ready;
    }
    
    std::vector<TimePointState*> checkpointData;      // Sets of data for different time points.  The size of checkpointData is the number of checkpoints plus one and checkpointData[0] is unused.
    size_t                       nextCheckpointIndex; // The next index in checkpointData to output.  Goes from one to the number of checkpoints.
    size_t                       endOutputIndex;      // Needed by SDAG code to span serial blocks.
};

#endif // __CHECKPOINT_MANAGER_H__
