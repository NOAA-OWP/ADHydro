#ifndef __REGION_H__
#define __REGION_H__

#include "mesh_element.h"
#include "channel_element.h"
#include "readonly.h"
#include "region.decl.h"

// A Region is a Charm++ chare object that holds multiple mesh and channel elements and implements some of our basic simulation architecture.
// The architecture is an explicit first-order simulation of moving water between various containers such as: groundwater, surfacewater, streams, lakes, etc.
// Each pair of neighboring containers calculate a simple linear flow rate, cubic meters per second, and an expiration time for how long this flow rate is valid.
// Then the amount of water equal to the flow rate times the timestep is removed from one container and added to the other container.
// Then new flow rates are calculated, etc.
//
// One of the important features of this architecture is that it guarantees conservation of mass.
// When it moves water it always removes a quantity of water from one container and adds the same quantity to another container.
// One issue with mass conservation is that a container may not have enough water to satisfy all of its calculated outflows.
// The way the architecture handles this is by first calculating a nominal flow rate, and then containers that are sending water send messages with the actual amount of water sent.
// If a container does not have enough water to satisfy all of its calculated outflows, it reduces them proportionally and sends only the amount of water it has.
// A final issue with mass conservation is that water might not be precisely conserved due to floating point roundoff error.
// You should expect mass balance errors due to roundoff error to be less than 10^-10 meters times the area in square meters of the simulated watershed.
//
// Another important feature of this architecture is that it allows different timesteps for different containers in the simulation.
// Containers with high flow rates might need short timesteps to simulate accurately, while containers with lower flow rates might be able to use longer timesteps.
// The architecture allows neighboring containers to use different timesteps by agreeing on flow rates and using accumulators for flow quantities.
// In the current implementation, all elements within a single region use the same timestep, but different regions can use different timesteps.
//
// Within a Region, elements can interact by directly accessing each others' public methods.  Only communication between Regions requires Charm++ messages.
//
// Simulation time is moved forward by the following five steps:
//
// Step 1: Calculate nominal flow rates with neighbors.
// Step 2: Select timestep for the Region.
// Step 3: Send outflows of water to neighbors.  Some point processes are also simulated in this step.
// Step 4: Receive inflows of water from neighbors.  Some point processes are also simulated in this step.
// Step 5: Advance time.
class Region : public CBase_Region
{
    Region_SDAG_CODE
    
public:
    
    // Charm++ migration constructor and default constructor.
    //
    // Parameters:
    //
    // msg - Unused migration message.
    inline Region(CkMigrateMessage* msg = NULL) : currentTime(Readonly::simulationStartTime), timestepEndTime(Readonly::simulationStartTime), nextForcingTime(Readonly::simulationStartTime),
                                                  nextCheckpointIndex(1), numberOfMeshElements(0), numberOfChannelElements(0), meshElements(), channelElements(), elementsFinished(0)
    {
        if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
        {
            if (checkInvariant())
            {
                CkExit();
            }
        }
        
        thisProxy[thisIndex].runUntilSimulationEnd();
    }
    
    // Charm++ pack/unpack method.
    //
    // Parameters:
    //
    // p - Pack/unpack processing object.
    inline void pup(PUP::er &p)
    {
        CBase_Region::pup(p);
        __sdag_pup(p);
        
        p | currentTime;
        p | timestepEndTime;
        p | nextForcingTime;
        p | nextCheckpointIndex;
        p | numberOfMeshElements;
        p | numberOfChannelElements;
        p | meshElements;
        p | channelElements;
        p | elementsFinished;
    }
    
    // Check invariant conditions on data.
    //
    // Returns: true if the invariant is violated, false otherwise.
    bool checkInvariant() const;
    
    // Loop over a vector of Messages calling receiveMessage on each one.  Exit on error.
    // This function is necessary because you can't pass a std::vector<SubClass> as a reference to std::vector<SuperClass>
    // the same way you can pass an individual SubClass as a reference to SuperClass.
    //
    // Parameters:
    //
    // messages - The received messages.
    template <typename T> inline void receiveMessages(std::vector<T>& messages)
    {
        typename std::vector<T>::iterator it; // Loop iterator.
        
        // Loop over messages calling receiveMessage for each one.
        for (it = messages.begin(); it != messages.end(); ++it)
        {
            receiveMessage(*it);
        }
    }
    
    // Pass a received Message down to the appropriate element.  Exit on error.
    //
    // Parameters:
    //
    // message - The received message.
    void receiveMessage(Message& message);
    
private:
    
    // Simulation time.
    double       currentTime;         // (s) Current simulation time specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    double       timestepEndTime;     // (s) Simulation time at the end of the current timestep specified as the number of seconds after referenceDate.  Can be negative to specify times before reference date.
    double       nextForcingTime;     // (s) The next simulation time when new forcing data needs to be read.
    size_t       nextCheckpointIndex; // The index of the next checkpoint to output.
    const double simulationEndTime = Readonly::simulationStartTime + Readonly::simulationDuration;
                                      // This is partly for efficiency so we don't do the addition over and over and partly because Charm++ is having trouble parsing Readonly:: in the .ci file.
    
    // Elements in the Region.
    size_t                           numberOfMeshElements;    // For initialization, the Region will wait until it receives this many MeshElements.
    size_t                           numberOfChannelElements; // For initialization, the Region will wait until it receives this many ChannelElements.
    std::map<size_t,    MeshElement> meshElements;            // A map of MeshElements    allowing the Region to find a specific element or iterate over all elements.  Keys are element ID numbers.
    std::map<size_t, ChannelElement> channelElements;         // A map of channelElements allowing the Region to find a specific element or iterate over all elements.  Keys are element ID numbers.
    size_t                           elementsFinished;        // Number of elements finished in the current phase such as initialization, invariant check, receive state, or receive water.
                                                              // This Region is finished when elementsFinished equals meshElements.size() plus channelElements.size().
};

#endif // __REGION_H__
