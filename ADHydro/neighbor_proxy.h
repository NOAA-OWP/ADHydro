#ifndef __NEIGHBOR_PROXY_H__
#define __NEIGHBOR_PROXY_H__

#include "all_charm.h"
#include <list>

// A NeighborProxy represents that an element has a neighbor relationship with
// another element.  It also includes a way to communicate with that neighbor
// and stores some information about material flows to and from that neighbor.
class NeighborProxy
{
public:
  
  // A MaterialTransfer represents some material that has been sent by one
  // element and not yet received by another element.  If the duration of the
  // transfer overlaps more than one timestep at the recipient it is assumed
  // that the transfer proceeds at a constant rate between startTime and
  // endTime.
  class MaterialTransfer
  {
  public:
    
    // Constructor.
    //
    // All parameters directly initialize member variables.  For description
    // see member variables.
    MaterialTransfer(double startTimeInit, double endTimeInit);
    
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
    
    double startTime; // Simulation time in seconds since Element::referenceDate when the transfer starts.
    double endTime;   // Simulation time in seconds since Element::referenceDate when the transfer ends.
    
    // Any subclass of NeighborProxy must include a subclass of
    // MaterialTransfer with member variable(s) to record the quantity of
    // material being transferred.  The representation of the quantity is left
    // to the subclass.  For example, diffusive wave simulations only need to
    // move single-valued materials such as water while dynamic wave
    // simulations need to move multi-valued meterials such as water and
    // momentum.
  }; // End class MaterialTransfer.
  
  // Constructor.
  //
  // All parameters directly initialize member variables.  For description see
  // member variables.
  explicit NeighborProxy(double expirationTimeInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.  NeighborProxy actually has no invariant
  // conditions, but this is included for completeness.  Any subclass of
  // NeighborProxy should check that the time ranges of MaterialTransfers are
  // non-overlapping and go no later than expirationTime.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  double expirationTime;  // Simulation time in seconds since Element::referenceDate when nominal flow rate expires.
  
  // Any subclass of NeighborProxy must include member variable(s) to record
  // the nominal flow rate with the neighbor.  The representation of the
  // nominal flow rate is left to the subclass.  For example, diffusive wave
  // simulations only need to move single-valued materials such as water while
  // dynamic wave simulations need to move multi-valued meterials such as water
  // and momentum.
  
  // Any subclass of NeighborProxy must include member variable(s) to record
  // MaterialTransfers that will be received as Element::currentTime advances.
  
  // Any subclass of NeighborProxy must include member variable(s) to record
  // the identity of the neighbor and a method for communicating with the
  // neighbor.
}; // End class NeighborProxy.

// A SimpleNeighborProxy is a NeighborProxy where material quantities and flows
// are represented as one floating point value (a double variable).  This seems
// like such a generally useful case that it is included here.
class SimpleNeighborProxy : public NeighborProxy
{
public:
  
  // A NeighborProxy::MaterialTransfer with the addition of a variable for flow
  // quantity.
  class MaterialTransfer : public NeighborProxy::MaterialTransfer
  {
  public:
    
    // Default constructor.  Only needed for pup_stl.h code.
    MaterialTransfer();
    
    // Constructor.
    //
    // All parameters directly initialize superclass and member variables.  For
    // description see superclass and member variables.
    MaterialTransfer(double startTimeInit, double endTimeInit, double materialInit);
    
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
    
    double material; // Quantity of material being transferred.
  }; // End class MaterialTransfer.
  
  // Constructor.
  //
  // All parameters directly initialize superclass and member variables.  For
  // description see superclass and member variables.
  //
  // incomingMaterial is initialized to empty.
  SimpleNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, bool inflowOnlyInit, double flowCumulativeShortTermInit,
                      double flowCumulativeLongTermInit);
  
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
  
  // Insert a new MaterialTransfer into incomingMaterial.  This keeps the list
  // sorted and checks that the time range of the newly inserted material is
  // non-overlapping.
  //
  // Returns: true if there is an error, false otherwise.
  bool insertMaterial(MaterialTransfer newMaterial);
  
  // Returns: true if all material flows have arrived between currentTime and
  //          timestepEndTime, false otherwise.  Exit on error.
  bool allMaterialHasArrived(double currentTime, double timestepEndTime);
  
  // Returns: The amount of material that arrived up to timestepEndTime.  The
  //          material is also removed from incomingMaterial.  Exit on error.
  double getMaterial(double currentTime, double timestepEndTime);
  
  // Returns: The amount of material currently held in the incomingMaterial
  //          list.  This is used for mass balance.
  double totalWaterInIncomingMaterial();
  
  double nominalFlowRate;         // Material flow rate in quantity per second.  Positive means flow out of the element into the neighbor.  Negative means flow
                                  // into the element out of the neighbor.  Actual flows may be less than this if the sender does not have enough material to
                                  // satisfy all outflows.
  bool   inflowOnly;              // For some neighbor connections water is forced to flow in only one direction, such as a reservoir releasing water, or water
                                  // being diverted for irrigation.  If inflowOnly is true the element knows that it will only receive water.  The element will
                                  // not calculate a flow rate.  Only the sender will calculate a flow rate, and the receiver will just wait for
                                  // MaterialTransfer messages.  Do not use this for inflow boundary conditions.  Only use it when there is an actual neighbor
                                  // element, but flow will always be in one direction.
  double flowCumulativeShortTerm; // flowCumulativeShortTerm plus flowCumulativeLongTerm together are the cumulative material flow quantity with the neighbor.
  double flowCumulativeLongTerm;  // Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.  Two
                                  // variables are used because over a long simulation this value could become quite large, and the amount of material moved
                                  // each timestep can be quite small so roundoff error becomes a concern.  New flow is added to flowCumulativeShortTerm each
                                  // timestep, and occasionally flowCumulativeShortTerm is added to flowCumulativeLongTerm and flowCumulativeShortTerm is reset
                                  // to zero.
  
  std::list<MaterialTransfer> incomingMaterial; // Material that will be received as Element::currentTime advances.  The list should only be non-empty when
                                                // nominalFlowRate is an inflow.  The list is maintained sorted with the earliest transfers at the front.  All
                                                // transfers in the list must be non-overlapping and be between Element::currentTime and expirationTime.  When
                                                // there are no time gaps in the list all flows have arrived.
  
  // Any subclass of SimpleNeighborProxy must include member variable(s) to
  // record the identity of the neighbor and a method for communicating with
  // the neighbor.
}; // End class SimpleNeighborProxy.

#endif // __NEIGHBOR_PROXY_H__
