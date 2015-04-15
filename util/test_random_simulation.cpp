#include "element.h"
#include "test_random_simulation.decl.h"
#include <cstdio>

/* I changed the implementation of Element to cut this code out.  We'll have to implement something similar here instead of calling the function in Element that no longer exists.
 * 
  
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

Element::Iterator::~Iterator()
{
  // Do nothing.
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
 */

// Forward declaration to eliminate circular dependency.
class RandomNeighborProxy;

// RandomElement uses RandomNeighborProxy to calculate random flows between
// elements.
class RandomElement : public Element
{
public:

  // Implements the Element::Iterator interface with an iterator into
  // neighbors.
  class Iterator : public Element::Iterator
  {
  public:
    
    // Constructor
    //
    // Parameters:
    //
    // neighbors - vector to create an iterator for.
    Iterator(std::vector<RandomNeighborProxy>& neighbors) :
      it(neighbors.begin()),
      itEnd(neighbors.end())
    {
      // Initialization handled by initialization list.
    }
    
    // Dereference operator.  The body must be defined below the class
    // declaration of RandomNeighborProxy so that the compiler knows it is a
    // subclass of NeighborProxy.
    //
    // Returns: The NeighborProxy that this iterator points to.
    virtual NeighborProxy& operator*();
    
    // Increment operator.
    //
    // Returns: The iterator after incrementing.
    virtual Iterator& operator++()
    {
      ++it;
      
      return *this;
    }
    
    // Returns: true if the iterator is at the end, false otherwise.
    virtual bool atEnd()
    {
      return it == itEnd;
    }
    
    std::vector<RandomNeighborProxy>::iterator it;    // An iterator into neighbors.
    std::vector<RandomNeighborProxy>::iterator itEnd; // An iterator at the end of neighbors.
  }; // End class Iterator.

  // Constructor. Calls Element constructor.  neighbors is initialized to
  // empty.
  RandomElement(double referenceDateInit, double currentTimeInit, double simulationEndTimeInit, double materialInit) :
    Element(referenceDateInit, currentTimeInit, simulationEndTimeInit),
    material(materialInit),
    neighbors()
  {
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    // referenceDateInit, currentTimeInit, and simulationEndTimeInit will be checked by Element constructor.

    if (!(0.0 <= materialInit))
      {
        CkError("ERROR in RandomElement::RandomElement: materialInit must be greater than or equal to zero.\n");
        CkExit();
      }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  }

  // Check invariant conditions.  The body must be defined below the class
  // declaration of RandomNeighborProxy so that this function can access the
  // members of RandomNeighborProxy.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  // Returns: A heap allocated iterator pointing to the first of this element's
  //          neighbors.  The caller is responsible for deleting it.
  virtual Element::Iterator* begin()
  {
    return new Iterator(neighbors);
  }
  
  // Calculate flow rates with neighbors where possible.  A flow rate can be
  // calculated if both this element and its neighbor are synced up at the end
  // of a timestep that ends at the expirationTime of the flow rate.  The body
  // must be defined below the class declaration of RandomNeighborProxy so that
  // this function can access the members of RandomNeighborProxy.
  void calculateNominalFlowRates();
  
  // Attempt to select a new timestep and if a new timestep is selected limit
  // outward flows and send outward flow water to neighbors.  The body must be
  // defined below the class declaration of RandomNeighborProxy so that this
  // function can access the members of RandomNeighborProxy.
  void selectTimestepAndMoveWater();
  
  // If timestepEndTime is in the future and all inward flow water has arrived
  // receive it and advance to the end of the timestep.  The body must be
  // defined below the class declaration of RandomNeighborProxy so that this
  // function can access the members of RandomNeighborProxy.
  void receiveWaterAndAdvanceTime();

  double                           material;  // The amount of material in this container.
  std::vector<RandomNeighborProxy> neighbors; // The list of this element's neighbors.
}; // End class RandomElement.

// In this simulation all elements are stored in a vector on a single process.
// RandomNeighborProxy stores: a pointer to this vector, the index in this
// vector of the neighbor, and the index in the neighbor's neighbors vector of
// the reciprocal neighbor proxy.  Flow rates and expiration times are chosen
// randomly.
class RandomNeighborProxy : public SimpleNeighborProxy
{
public:

  // Constructor.  If you pass true for noReciprocalYet it won't check
  // reciprocalNeighborProxyInit against the size of the neighbors vector in
  // the neighbor element.  This is because a pair of proxies has to be added
  // one at a time and when you add the first the second won't be there yet so
  // the check will fail.
  RandomNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, std::vector<RandomElement>* elementsInit,
                      std::vector<RandomElement>::size_type neighborInit, std::vector<RandomNeighborProxy>::size_type reciprocalNeighborProxyInit,
                      bool noReciprocalYet = false) :
    SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit),
    elements(elementsInit),
    neighbor(neighborInit),
    reciprocalNeighborProxy(reciprocalNeighborProxyInit)
  {
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    // nominalFlowRateInit and expirationTimeInit will be checked by NeighborProxy constructor.

    if (!(NULL != elementsInit))
      {
        CkError("ERROR in RandomNeighborProxy::RandomNeighborProxy: elementsInit must not be NULL.\n");
        CkExit();
      }
    else if (!(elementsInit->size() > neighborInit))
      {
        CkError("ERROR in RandomNeighborProxy::RandomNeighborProxy: neighborInit must be less than elementsInit->size().\n");
        CkExit();
      }
    else if (!(noReciprocalYet || (*elementsInit)[neighborInit].neighbors.size() > reciprocalNeighborProxyInit))
      {
        CkError("ERROR in RandomNeighborProxy::RandomNeighborProxy: reciprocalNeighborProxyInit must be less than "
                "(*elementsInit)[neighborInit].neighbors.size().\n");
        CkExit();
      }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  }

  // Check invariant conditions.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant()
  {
    bool error = SimpleNeighborProxy::checkInvariant(); // Error flag.  Start by calling base class invariant.

    if (!(NULL != elements))
      {
        CkError("ERROR in RandomNeighborProxy::checkInvariant: elements must not be NULL.\n");
        error = true;
      }
    else if (!(elements->size() > neighbor))
      {
        CkError("ERROR in RandomNeighborProxy::checkInvariant: neighbor must be less than elements->size().\n");
        error = true;
      }
    else if (!((*elements)[neighbor].neighbors.size() > reciprocalNeighborProxy))
      {
        CkError("ERROR in RandomNeighborProxy::checkInvariant: reciprocalNeighborProxy must be less than (*elements)[neighbor].neighbors.size().\n");
        error = true;
      }

    return error;
  }

  // Calculate flow rates with my neighbor if possible.  A flow rate can be
  // calculated if both this element and its neighbor are synced up at the end
  // of a timestep that ends at the expirationTime of the flow rate.  Choose
  // the new flow rate and expiration time randomly.  Also fill the negative
  // of the flow rate into my neighbor's proxy for me.
  void calculateNominalFlowRate(double currentTime)
  {
    // Only calculate a new flow rate if both I and my neighbor have reached the expiration time.
    if (expirationTime == currentTime && expirationTime == (*elements)[neighbor].currentTime)
      {
        // Set expirationTime as a random number of seconds in the future from epsilon to ten plus epsilon.
        //expirationTime += (rand() + 1) * 10.0 / RAND_MAX;
        
        // Select expiration time from a small number of choices that are mutually divisible.
        switch (rand() % 3)
        {
        case 0:
          expirationTime += 1.0;
          break;
        case 1:
          expirationTime += 2.0;
          break;
        case 2:
          expirationTime += 4.0;
          break;
        default:
          CkAssert(false);
        }

        // Set nominalFlowRate as a random number from minus ten to ten.  Have a five percent chance of zero to make sure that case gets tested.
        if (0 == rand() % 20)
          {
            nominalFlowRate = 0.0;
          }
        else
          {
            nominalFlowRate = (rand() - RAND_MAX / 2) * 20.0 / RAND_MAX;
          }

        // Fill the expiration time and negative of the flow rate into my neighbor's proxy for me.
        (*elements)[neighbor].neighbors[reciprocalNeighborProxy].expirationTime  = expirationTime;
        (*elements)[neighbor].neighbors[reciprocalNeighborProxy].nominalFlowRate = -nominalFlowRate;
      }
  }
  
  std::vector<RandomElement>*                 elements;                // The vector of elements in the simulation.
  std::vector<RandomElement>::size_type       neighbor;                // The index of my neighbor in elements.
  std::vector<RandomNeighborProxy>::size_type reciprocalNeighborProxy; // The index of my neighbor's proxy for me.
}; // End class RandomNeighborProxy.

NeighborProxy& RandomElement::Iterator::operator*()
{
  return *it;
}

bool RandomElement::checkInvariant()
{
  bool                                       error = Element::checkInvariant(); // Error flag.  Start by calling base class invariant.
  bool                                       tempError;                         // Temporary error flag.
  std::vector<RandomNeighborProxy>::iterator it1, it2;                          // Loop iterators.

  if (!(0.0 <= material))
    {
      CkError("ERROR in RandomElement::checkInvariant: material must be greater than or equal to zero.\n");
      error = true;
    }

  for (it1 = neighbors.begin(); it1 != neighbors.end(); ++it1)
    {
      tempError = (*it1).checkInvariant();
      error     = tempError || error;

      // Check tempError here because if (*it1).checkInvariant() fails then it may not be safe to do (*(*it1).elements)[(*it1).neighbor].
      if (!tempError)
        {
          RandomElement& neighbor = (*(*it1).elements)[(*it1).neighbor];

          if (!(&neighbor != this))
            {
              CkError("ERROR in RandomElement::checkInvariant: self neighbors not allowed.\n");
              error = true;
            }
          
          if (!(&(*(*it1).elements)[neighbor.neighbors[(*it1).reciprocalNeighborProxy].neighbor] == this))
            {
              CkError("ERROR in RandomElement::checkInvariant: incorrect reciprocal neighbor.\n");
              error = true;
            }
        }

      for (it2 = neighbors.begin(); it2 != it1; ++it2)
        {
          if (!((*it1).neighbor != (*it2).neighbor))
            {
              CkError("ERROR in RandomElement::checkInvariant: duplicate neighbors not allowed.\n");
              error = true;
            }
        }
      
      if (!((*it1).incomingMaterial.empty() || currentTime <= (*it1).incomingMaterial.front().startTime))
        {
          CkError("ERROR in RandomElement::checkInvariant: currentTime must be less than or equal to the first startTime in incomingMaterial.\n");
          error = true;
        }
      
      if (!(timestepEndTime <= (*it1).expirationTime))
        {
          CkError("ERROR in RandomElement::checkInvariant: timestepEndTime must be less than or equal to expirationTime.\n");
          error = true;
        }
    }

  return error;
}

void RandomElement::calculateNominalFlowRates()
{
  std::vector<RandomNeighborProxy>::iterator it; // Loop iterator.

  for (it = neighbors.begin(); it != neighbors.end(); ++it)
    {
      (*it).calculateNominalFlowRate(currentTime);
    }
}

void RandomElement::selectTimestepAndMoveWater()
{
  std::vector<RandomNeighborProxy>::iterator it;                            // Loop iterator.
  double                                     totalOutwardFlowRate    = 0.0; // Sum of all outward flow rates.
  double                                     outwardFlowRateFraction = 1.0; // Fraction of all outward flows that can be satisfied.
  
  if (selectTimestep())
    {
      for (it = neighbors.begin(); it != neighbors.end(); ++it)
        {
          if (0.0 < (*it).nominalFlowRate)
            {
              totalOutwardFlowRate += (*it).nominalFlowRate;
            }
        }

      // Limit outward flows and take material.
      if (material < totalOutwardFlowRate * (timestepEndTime - currentTime))
        {
          outwardFlowRateFraction = material / (totalOutwardFlowRate * (timestepEndTime - currentTime));
          material                = 0.0;

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }
      else
        {
          material -= totalOutwardFlowRate * (timestepEndTime - currentTime);
        }

      // Send material to recipients.
      for (it = neighbors.begin(); it != neighbors.end(); ++it)
        {
          if (0.0 < (*it).nominalFlowRate)
            {
              (*(*it).elements)[(*it).neighbor].neighbors[(*it).reciprocalNeighborProxy].insertMaterial(
                  RandomNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                      outwardFlowRateFraction * (*it).nominalFlowRate * (timestepEndTime - currentTime)));
            }
        }
    }
}

void RandomElement::receiveWaterAndAdvanceTime()
{
  std::vector<RandomNeighborProxy>::iterator it;                       // Loop iterator.
  bool                                       allInflowsArrived = true; // Whether all material has arrived from all neighbors.
  
  // If timestepEndTime is not in the future we are not ready to process a timestep.
  if (currentTime < timestepEndTime)
    {
      // Check if all of the inflow material has arrived.
      for (it = neighbors.begin(); allInflowsArrived && it != neighbors.end(); ++it)
        {
          if (0.0 > (*it).nominalFlowRate)
            {
              allInflowsArrived = (*it).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }

      if (allInflowsArrived)
        {
          // Receive all inflows.
          for (it = neighbors.begin(); it != neighbors.end(); ++it)
            {
              if (0.0 > (*it).nominalFlowRate)
                {
                  material += (*it).getMaterial(currentTime, timestepEndTime);
                }
            }

          // Advance time.
          currentTime = timestepEndTime;
        }
    }
}

// This is a test program to excercise the simulation architecture implemented
// in Element and NeighborProxy.  It creates a given number of RandomElements
// and connects them randomly, and then runs the simulation for a given amount
// of simulation time.
class TestRandomSimulation : public CBase_TestRandomSimulation
{
public:

  // Constructor.  Read command line arguments and call runSimulation.
  //
  // Parameters:
  //
  // msg - Charm++ command line arguments message.
  TestRandomSimulation(CkArgMsg* msg)
  {
    int numScanned; // Used to check that sscanf scanned the correct number of inputs.

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    if (2 > msg->argc)
      {
        CkError("WARNING in TestRandomSimulation::TestRandomSimulation: numberOfElements not specified on command line.  Using 100 elements.\n");
        numberOfElements = 100;
      }
    else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      {
        numScanned = sscanf(msg->argv[1], "%zu", &numberOfElements);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        if (!(1 == numScanned))
          {
            CkError("WARNING in TestRandomSimulation::TestRandomSimulation: unable to read numberOfElements from the command line.  Using 100 elements.\n");
            numberOfElements = 100;
          }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        if (!(0 < numberOfElements))
          {
            CkError("WARNING in main: numberOfElements must be greater than zero.  Using 100 elements.\n");
            numberOfElements = 100;
          }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      }

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    if (3 > msg->argc)
      {
        CkError("WARNING in TestRandomSimulation::TestRandomSimulation: simulationDuration not specified on command line.  Simulating for 1000 seconds.\n");
        simulationDuration = 1000.0;
      }
    else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      {
        numScanned = sscanf(msg->argv[2], "%lf", &simulationDuration);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        if (!(1 == numScanned))
          {
            CkError("WARNING in TestRandomSimulation::TestRandomSimulation: unable to read simulationDuration from the command line.  Simulating for 1000 "
                    "seconds.\n");
            simulationDuration = 1000.0;
          }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        if (!(0.0 <= simulationDuration))
          {
            CkError("WARNING in main: simulationDuration must be greater than or equal to zero.  Simulating for 1000 seconds.\n");
            simulationDuration = 1000.0;
          }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      }

    thisProxy.runSimulation();
  } // End TestRandomSimulation(CkArgMsg* msg).

  // Run the simulation.
  void runSimulation()
  {
    bool                                                       error = false;          // Error flag.
    std::vector<RandomNeighborProxy>::size_type                ii;                     // Loop counter.
    std::vector<RandomElement>::iterator                       it;                     // Loop iterator.
    std::vector<RandomNeighborProxy>::iterator                 itNeighbor;             // Loop iterator.
    std::list<RandomNeighborProxy::MaterialTransfer>::iterator itMaterial;             // Loop iterator.
    std::vector<RandomElement>                                 elements(numberOfElements, RandomElement(1721425.5, 0.0, simulationDuration, 100.0));
    std::vector<RandomNeighborProxy>::size_type                numberOfNeighbors;      // Total number of random neighbor pairs to create.
    std::vector<RandomElement>::size_type                      neighbor1;              // First neighbor of pair.
    std::vector<RandomElement>::size_type                      neighbor2;              // Second neighbor of pair.
    bool                                                       duplicateNeighbor;      // Whether this neighbor pair is a duplicate.
    bool                                                       simulationDone = false; // Flag to indicate all elements have reached simulationEndTime.
    double                                                     totalMaterial;          // Total material in simulation for material balance check.
    double                                                     minCurrentTime;         // Maximum simulation time of all elements.
    double                                                     maxCurrentTime;         // Minimum simulation time of all elements.
    double                                                     averageCurrentTime;     // Average simulation time of all elements.
    int                                                        iteration = 0;          // Number of iterations.

    CkPrintf("In runSimulation, numberOfElements = %d, simulationDuration = %lf.\n", numberOfElements, simulationDuration);

    // Create random connectivity.
    srand(time(NULL));
    
    numberOfNeighbors = numberOfElements * std::log(numberOfElements) / 2.0;

    for (ii = 0; ii < numberOfNeighbors; ++ii)
      {
        neighbor1 = rand() % numberOfElements;
        neighbor2 = rand() % numberOfElements;

        duplicateNeighbor = false;

        // Check for duplicate neighbor.
        for (itNeighbor = elements[neighbor1].neighbors.begin(); !duplicateNeighbor && itNeighbor != elements[neighbor1].neighbors.end(); ++itNeighbor)
          {
            duplicateNeighbor = ((*itNeighbor).neighbor == neighbor2);
          }

        // Check for self neighbor.
        if (!duplicateNeighbor && neighbor1 != neighbor2)
          {
            // Add neighbor relationship.  Pass true for noReciprocalYet for the first proxy because the second proxy doens't exist yet.
            elements[neighbor1].neighbors.push_back(RandomNeighborProxy(0.0, 0.0, &elements, neighbor2, elements[neighbor2].neighbors.size(), true));
            elements[neighbor2].neighbors.push_back(RandomNeighborProxy(0.0, 0.0, &elements, neighbor1, elements[neighbor1].neighbors.size() - 1));
          }
      }

    while (!error && !simulationDone)
      {
        // Step 1: Calculate nominal flow rates with neighbors.
        for (it = elements.begin(); it != elements.end(); ++it)
          {
            (*it).calculateNominalFlowRates();
          }

        // Step 2: Select timestep.
        // Step 3: Send outflows of material to neighbors.
        for (it = elements.begin(); it != elements.end(); ++it)
          {
            (*it).selectTimestepAndMoveWater();
          }

        // Step 4: Receive inflows of material from neighbors.
        // Step 5: Advance time.
        for (it = elements.begin(); it != elements.end(); ++it)
          {
            (*it).receiveWaterAndAdvanceTime();
          }

        // Check if all elements have reached simulationEndTime, do material balance check, report simulation state, and check invariant.
        simulationDone     = true;
        totalMaterial      = 0.0;
        minCurrentTime     = simulationDuration;
        maxCurrentTime     = 0.0;
        averageCurrentTime = 0.0;

        for (it = elements.begin(); !error && it != elements.end(); ++it)
          {
            if ((*it).currentTime < (*it).simulationEndTime)
              {
                simulationDone = false;
              }

            totalMaterial += (*it).material;

            for (itNeighbor = (*it).neighbors.begin(); itNeighbor != (*it).neighbors.end(); ++itNeighbor)
              {
                for (itMaterial = (*itNeighbor).incomingMaterial.begin(); itMaterial != (*itNeighbor).incomingMaterial.end(); ++itMaterial)
                  {
                    totalMaterial += (*itMaterial).material;
                  }
              }

            if (minCurrentTime > (*it).currentTime)
              {
                minCurrentTime = (*it).currentTime;
              }

            if (maxCurrentTime < (*it).currentTime)
              {
                maxCurrentTime = (*it).currentTime;
              }

            averageCurrentTime += (*it).currentTime;
            
            error = (*it).checkInvariant();
          }

        if (!error)
          {
            averageCurrentTime /= numberOfElements;

            CkPrintf("iteration = %d, material balance error = %lg, minCurrentTime = %lf, maxCurrentTime = %lf, averageCurrentTime = %lf.\n", ++iteration,
                     totalMaterial - numberOfElements * 100.0, minCurrentTime, maxCurrentTime, averageCurrentTime);
          }
      } // End while (!error && !simulationDone).

    CkExit();
  } // End void runSimulation().

  std::vector<RandomElement>::size_type numberOfElements;   // The number of elements to create in the simulation.
  double                                simulationDuration; // The simulation time to run the simulation for.
}; // End class TestRandomSimulation.

#include "test_random_simulation.def.h"
