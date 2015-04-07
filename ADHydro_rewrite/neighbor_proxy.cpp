#include "neighbor_proxy.h"

NeighborProxy::MaterialTransfer::MaterialTransfer(double startTimeInit, double endTimeInit) :
  startTime(startTimeInit),
  endTime(endTimeInit)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(startTimeInit < endTimeInit))
    {
      CkError("ERROR in NeighborProxy::MaterialTransfer::MaterialTransfer: startTimeInit must be less than endTimeInit.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void NeighborProxy::MaterialTransfer::pup(PUP::er &p)
{
  p | startTime;
  p | endTime;
}

bool NeighborProxy::MaterialTransfer::checkInvariant()
{
  bool error = false; // Error flag.
  
  if (!(startTime < endTime))
    {
      CkError("ERROR in NeighborProxy::MaterialTransfer::checkInvariant: startTime must be less than endTime.\n");
      error = true;
    }
  
  return error;
}

NeighborProxy::NeighborProxy(double expirationTimeInit) :
  expirationTime(expirationTimeInit)
{
  // Initialization handled by initialization list.
}

void NeighborProxy::pup(PUP::er &p)
{
  p | expirationTime;
}

bool NeighborProxy::checkInvariant()
{
  bool error = false; // Error flag.
  
  // No invariant conditions.
  
  return error;
}

SimpleNeighborProxy::MaterialTransfer::MaterialTransfer() :
  NeighborProxy::MaterialTransfer(0.0, 1.0),
  material(0.0)
{
  // Initialization handled by initialization list.
}

SimpleNeighborProxy::MaterialTransfer::MaterialTransfer(double startTimeInit, double endTimeInit, double materialInit) :
  NeighborProxy::MaterialTransfer(startTimeInit, endTimeInit),
  material(materialInit)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= materialInit))
    {
      CkError("ERROR in SimpleNeighborProxy::MaterialTransfer::MaterialTransfer: materialInit must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void SimpleNeighborProxy::MaterialTransfer::pup(PUP::er &p)
{
  NeighborProxy::MaterialTransfer::pup(p);
  
  p | material;
}

bool SimpleNeighborProxy::MaterialTransfer::checkInvariant()
{
  bool error = NeighborProxy::MaterialTransfer::checkInvariant(); // Error flag.
  
  if (!(0.0 <= material))
    {
      CkError("ERROR in SimpleNeighborProxy::MaterialTransfer::checkInvariant: material must be greater than or equal to zero.\n");
      error = true;
    }
  
  return error;
}

SimpleNeighborProxy::SimpleNeighborProxy(double expirationTimeInit, double nominalFlowRateInit) :
  NeighborProxy::NeighborProxy(expirationTimeInit),
  nominalFlowRate(nominalFlowRateInit),
  incomingMaterial()
{
  // Initialization handled by initialization list.
}

void SimpleNeighborProxy::pup(PUP::er &p)
{
  NeighborProxy::pup(p);
  
  p | nominalFlowRate;
  p | incomingMaterial;
}

bool SimpleNeighborProxy::checkInvariant()
{
  bool                                  error = NeighborProxy::checkInvariant(); // Error flag.
  std::list<MaterialTransfer>::iterator it    = incomingMaterial.begin();        // Loop iterator.
  double                                previousEndTime;                         // To check that the elements of incomingMaterial are sorted and
                                                                                 // non-overlapping.
  
  while (it != incomingMaterial.end())
    {
      error           = (*it).checkInvariant() || error;
      previousEndTime = (*it).endTime;
      ++it;
      
      if (it != incomingMaterial.end())
        {
          if (!(previousEndTime <= (*it).startTime))
            {
              CkError("ERROR in SimpleNeighborProxy::checkInvariant: elements of incomingMaterial must be sorted and non-overlapping.\n");
              error = true;
            }
        }
    }
  
  if (!incomingMaterial.empty())
    {
      if (!(0.0 > nominalFlowRate))
        {
          CkError("ERROR in SimpleNeighborProxy::checkInvariant: If incomingMaterial is not empty nominalFlowRate must be an inflow.\n");
          error = true;
        }
      
      if (!(incomingMaterial.back().endTime <= expirationTime))
        {
          CkError("ERROR in SimpleNeighborProxy::checkInvariant: the last endTime in incomingMaterial must be less than or equal to expirationTime.\n");
          error = true;
        }
    }
  
  return error;
}

void SimpleNeighborProxy::insertMaterial(MaterialTransfer newMaterial)
{
  std::list<MaterialTransfer>::reverse_iterator it = incomingMaterial.rbegin(); // Loop iterator.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (newMaterial.checkInvariant())
    {
      CkExit();
    }

  if (!(0.0 > nominalFlowRate))
    {
      CkError("ERROR in SimpleNeighborProxy::insertMaterial: If inserting material nominalFlowRate must be an inflow.\n");
      CkExit();
    }

  if (!(newMaterial.endTime <= expirationTime))
    {
      CkError("ERROR in SimpleNeighborProxy::insertMaterial: newMaterial.endTime must be less than or equal to expirationTime.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  // Find the last element in the list that ends before this new element starts.
  while (it != incomingMaterial.rend() && (*it).endTime > newMaterial.startTime)
    {
      ++it;
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  // Check that the next one in the list does not overlap the new element.
  if (it.base() != incomingMaterial.end())
    {
      if (!(newMaterial.endTime <= (*it.base()).startTime))
        {
          CkError("ERROR in SimpleNeighborProxy::insertMaterial: elements of incomingMaterial must be non-overlapping.\n");
          CkExit();
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)

  incomingMaterial.insert(it.base(), newMaterial);
}

bool SimpleNeighborProxy::allMaterialHasArrived(double currentTime, double timestepEndTime)
{
  std::list<MaterialTransfer>::iterator it;                                      // Loop iterator.
  bool                                  allArrived  = !incomingMaterial.empty(); // Return value.  If incomingMaterial is empty then it hasn't all arrived.
  double                                lastEndTime = currentTime;               // The last endTime for which material has arrived.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime < timestepEndTime))
    {
      CkError("ERROR in SimpleNeighborProxy::allMaterialHasArrived: currentTime must be less than timestepEndTime.\n");
      CkExit();
    }
  
  if (!(incomingMaterial.empty() || currentTime <= incomingMaterial.front().startTime))
    {
      CkError("ERROR in SimpleNeighborProxy::allMaterialHasArrived: currentTime must be less than or equal to the first startTime in incomingMaterial.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  for (it = incomingMaterial.begin(); allArrived && it != incomingMaterial.end() && lastEndTime < timestepEndTime; ++it)
    {
      if (lastEndTime != (*it).startTime)
        {
          // If there is a time gap then it hasn't all arrived.
          allArrived = false;
        }
      else
        {
          // Otherwise, it has all arrived up to the end time of the current MaterialTransfer.
          lastEndTime = (*it).endTime;
        }
    }
  
  // If it hasn't arrived up to timestepEndTime then it hasn't all arrived.
  if (allArrived && lastEndTime < timestepEndTime)
    {
      allArrived = false;
    }
  
  return allArrived;
}

double SimpleNeighborProxy::getMaterial(double currentTime, double timestepEndTime)
{
  double material = 0.0;  // Return value.
  double partialQuantity; // Part of a transfer to get.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime < timestepEndTime))
    {
      CkError("ERROR in SimpleNeighborProxy::getMaterial: currentTime must be less than timestepEndTime.\n");
      CkExit();
    }
  
  if (!(incomingMaterial.empty() || currentTime <= incomingMaterial.front().startTime))
    {
      CkError("ERROR in SimpleNeighborProxy::getMaterial: currentTime must be less than or equal to the first startTime in incomingMaterial.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (!allMaterialHasArrived(currentTime, timestepEndTime))
    {
      CkError("ERROR in SimpleNeighborProxy::getMaterial: allMaterialHasArrived must be true.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  while (!incomingMaterial.empty() && incomingMaterial.front().startTime < timestepEndTime)
    {
      if (incomingMaterial.front().endTime <= timestepEndTime)
        {
          // Get this entire transfer.
          material += incomingMaterial.front().material;
          
          incomingMaterial.pop_front();
        }
      else
        {
          // Get part of this transfer up to timestepEndTime.
          partialQuantity = incomingMaterial.front().material * (timestepEndTime - incomingMaterial.front().startTime) / (incomingMaterial.front().endTime -
                                                                                                                          incomingMaterial.front().startTime);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
          CkAssert(0.0 <= partialQuantity && partialQuantity <= incomingMaterial.front().material);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_INVARIANTS)
          
          material                          += partialQuantity;
          incomingMaterial.front().material -= partialQuantity;
          incomingMaterial.front().startTime = timestepEndTime;
        }
    }
  
  return material;
}
