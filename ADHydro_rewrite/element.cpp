#include "element.h"

Element::Iterator::~Iterator()
{
  // Do nothing.
}

Element::Element(double referenceDateInit, double currentTimeInit, double simulationEndTimeInit) :
  referenceDate(referenceDateInit),
  currentTime(currentTimeInit),
  timestepEndTime(currentTimeInit),
  nextSyncTime(simulationEndTimeInit),
  simulationEndTime(simulationEndTimeInit)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(1721425.5 <= referenceDateInit))
    {
      CkError("ERROR in Element::Element: referenceDateInit must not be before 1 CE.\n");
      CkExit();
    }
  
  if (!(currentTimeInit <= simulationEndTimeInit))
    {
      CkError("ERROR in Element::Element: simulationEndTimeInit must be greater than or equal to currentTimeInit.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void Element::pup(PUP::er &p)
{
  p | referenceDate;
  p | currentTime;
  p | timestepEndTime;
  p | nextSyncTime;
  p | simulationEndTime;
}

bool Element::checkInvariant()
{
  bool error = false;
  
  if (!(1721425.5 <= referenceDate))
    {
      CkError("ERROR in Element::checkInvariant: referenceDate must not be before 1 CE.\n");
      error = true;
    }
  
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in Element::checkInvariant: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
  
  if (!(timestepEndTime <= nextSyncTime))
    {
      CkError("ERROR in Element::checkInvariant: timestepEndTime must be less than or equal to nextSyncTime.\n");
      error = true;
    }
  
  if (!(nextSyncTime <= simulationEndTime))
    {
      CkError("ERROR in Element::checkInvariant: nextSyncTime must be less than or equal to simulationEndTime.\n");
      error = true;
    }
  
  return error;
}

// FIXME do I need allFlowRatesCalculated?  Maybe I could just call selectTimestep instead.
bool Element::allFlowRatesCalculated()
{
  Iterator* it;                   // Loop iterator.
  bool      allCalculated = true; // Return value.
  
  for (it = begin(); allCalculated && !it->atEnd(); ++*it)
    {
      allCalculated = (currentTime < (**it).expirationTime);
    }
  
  delete it;
  
  return allCalculated;
}

bool Element::selectTimestep()
{
  Iterator* it;                  // Loop iterator.
  bool      newTimestep = false; // return value
  
  if (timestepEndTime == currentTime)
    {
      timestepEndTime = nextSyncTime;

      for (it = begin(); !it->atEnd(); ++*it)
        {
          if (timestepEndTime > (**it).expirationTime)
            {
              timestepEndTime = (**it).expirationTime;
            }
        }

      delete it;
      
      newTimestep = (timestepEndTime > currentTime);
    }
  
  return newTimestep;
}
