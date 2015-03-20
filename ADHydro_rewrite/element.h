#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "neighbor_proxy.h"

// Element is an abstract class that implements some of our basic simulation
// architecture.  The architecture is a first-order simulation of moving
// material between containers.  For example, in the ADHydro simulation the
// material is water, and this water can move between various containers such
// as: groundwater, surfacewater, streams, rivers, lakes, etc.  The Element
// class is broken out as an abstract class because it could also be used for
// other simulations of other types of material and containers.
//
// The simulation is called first-order because it uses only the first
// derivative to determine how much material to move between containers.  Each
// pair of neighboring containers calculate a simple linear flow rate, quantity
// per second, and a timestep for which this first derivative flow rate is
// valid.  Then the amount of material equal to the flow rate times the
// timestep is removed from one container and added to the other container.
// Then new flow rates are calculated for the next timestep.
//
// One of the important features of this architecture is that it guarantees
// conservation of material.  When it moves material it always removes a
// quantity of material from one container and adds the same quantity to
// another container.  One issue is that a container may not have enough
// material to satisfy all of its calculated outflows.  The way the
// architecture handles this is by calculating all flows first, and then
// containers that are sending material send messages with the actual amount of
// material that flows.  If a container does not have enough material to
// satisfy all of its calculated outflows it reduces them proportionally and
// sends only the amount of material it has.  A final issue is that material
// might not be precisely conserved due to floating point roundoff error.
//
// Another important feature of this architecture is that it allows different
// timesteps for different containers in the simulation.  Containers with high
// flow rates might need short timesteps to simulate accurately, while
// containers with lower flow rates might be able to use longer timesteps.  It
// is computationally inefficient to force all containers to use the same
// timestep.  The architecture allows neighboring containers to use different
// timesteps by agreeing on flow rates and using accumulators for flow
// quantities.
//
// The Element class represents a container in the simulation.  It stores the
// simulation time that has been reached by that container.  The simulation
// time is moved forward by the following five steps:
//
// Step 1: Calculate nominal flow rates with neighbors.
// Step 2: Select timestep.
// Step 3: Send outflows of material to neighbors.
// Step 4: Receive inflows of material from neighbors.
// Step 5: Advance time.
class Element
{
public:
  
  // There is some generic code for any simulation implemented in the Element
  // class that requires iterating through all of the neighbors of the element.
  // Storing a container of NeighborProxys in Element isn't a good solution
  // because specific simulations are going to subclass NeighborProxy.  Storing
  // a container of pointers to NeighborProxys also isn't a good solution
  // because it has a bad interaction with the pup framework.  Basically, when
  // we are unpacking and need to construct a new object to store the pupped
  // data how do we know which subclass to construct?  The right solution is to
  // have this pure virtual iterator class that gets subclassed by each
  // specific simulation to iterate over the specific NeighborProxy
  // subclass(es) in the simulation.  The actual NeighborProxy subclass objects
  // are stored in the Element subclass, which knows their type and can pup
  // them properly.
  //
  // Subclasses of iterator must iterate over all of an element's neighbors
  // even if an element has multiple types of neighbors in multiple containers.
  //
  // This is a non-standard iterator pattern.  It was done this way so that the
  // actual implementation could come from subclasses.
  class Iterator
  {
  public:
    
    // Destructor.  Needs to be virtual because we will be deleting Iterator
    // pointers to subclass objects.
    virtual ~Iterator();
    
    // Dereference operator.
    //
    // Returns: The NeighborProxy that this iterator points to.
    virtual NeighborProxy& operator*() = 0;
    
    // Increment operator.
    //
    // Returns: The iterator after incrementing.
    virtual Iterator& operator++() = 0;
    
    // Returns: true if the iterator is at the end, false otherwise.
    virtual bool atEnd() = 0;
  }; // End class Iterator.
  
  // Constructor.  timestepEndTime is initialized to be currentTimeInit
  // indicating that a new value for timestepEndTime needs to be selected in
  // step 2 of the simulation.  nextSyncTime is initialized to be
  // simulationEndTimeInit indicating that the element will sync up with other
  // elements at the end of the simulation.
  //
  // Parameters:
  //
  // referenceDateInit     - Julian date to initialize referenceDate.
  // currentTimeInit       - Simulation time in seconds since referenceDateInit
  //                         to initialize currentTime and timestepEndTime.
  // simulationEndTimeInit - Simulation time in seconds since referenceDateInit
  //                         to initialize simulationEndTime and nextSyncTime.
  Element(double referenceDateInit, double currentTimeInit, double simulationEndTimeInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  // Returns: A heap allocated iterator pointing to the first of this element's
  //          neighbors.  The caller is responsible for deleting it.
  virtual Iterator* begin() = 0;
  
  // Returns: true if the flow rate expiration times of all neighbors are
  //          later than currentTime, false othewise.
  bool allFlowRatesCalculated();
  
  // If currentTime is equal to timestepEndTime calculate the minimum of
  // nextSyncTime and the expirationTimes of all neighbors.  If this minimum is
  // later than currentTime select it as the new timestepEndTime.
  //
  // If timestepEndTime is already later than currentTime, or this minimum is
  // equal to currentTime do not select a new timestepEndTime.
  //
  // Returns: true if a new timestepEndTime was selected.
  bool selectTimestep();

  // The following time invariants exist:
  //
  // currentTime <= timestepEndTime <= nextSyncTime <= simulationEndTime
  //
  // For all NeighborProxys:
  //
  // currentTime <= first MaterialTransfer.startTime <
  // first MaterialTransfer.endTime <= next MaterialTransfer.startTime ...
  // last MaterialTransfer.endTime <= expirationTime
  //
  // timestepEndTime <= expirationTime
  double referenceDate;      // Julian date when currentTime is zero.  The current date and time of the simulation is the Julian date equal to
                             // referenceDate + currentTime / (24.0 * 60.0 * 60.0).  Time zone is UTC.
  double currentTime;        // Current simulation time in seconds since referenceDate.
  double timestepEndTime;    // Simulation time at the end of the current timestep in seconds since referenceDate.
  double nextSyncTime;       // Simulation time at the next global sync point in seconds since referenceDate.  Elements may want to sync up globally for
                             // reasons such as outputting the state of all elements at the same simulation time.
  double simulationEndTime;  // Simulation time to end the simulation in seconds since referenceDate.
  
  // Any subclass of Element must include member variable(s) with the
  // NeighborProxys to the Element's neighbors.
}; // End class Element.

#endif // __ELEMENT_H__
