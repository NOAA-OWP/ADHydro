#ifndef OUTPUT_MANAGER_H
#define OUTPUT_MANAGER_H
#include "interperter.h"
#include "unit_test/region.h"
#include "unit_test/element.h"
#include "../../all_charm.h"
#include <unordered_map>
#include "OutputManager.decl.h"
#include "adhydro_boost_types.h"

class OutputManager : public CBase_OutputManager
{
public:
  //Output manager needs access to ADHydro state, which is contained in the mesh and channel elements.
  //A const reference is passed to the output manager to access this state, with an explicit contract
  //not to modify the state.
  OutputManager();
  ~OutputManager();
  void registerRegion(Region& region);
  void unregisterRegion(Region& region);
  double synchronizeOutputTime();

  //In order to double buffer output and allow regions to continue computation while we write output,
  //we will use the charm++ async model.  When an output time step is complete, a region will call on
  //its CkLocalBranch() to run output.  The local output manager will know what to output, collect that data
  //in a local array, and then send an asyc message to itself so it can return control immediatly to the region
  //The message will contain the data needed to write for output, and when the manager recieves this message
  //it can write that on its thread while other threads can continue running the regions.
  //With a little luck, the load balancer will recognize PE's with higher writes and try to reduce the number of
  //regions  that that PE is responsible for.
  
  //TODO/FIXME THINK ABOUT HOW TO SYNCRONIZE TIME STEPS/OUTPUT INTERVALS ACROSS REGIONS OF "INTEREST"
private:
    //Each output manager will keep a spatial index of the entire mesh.
    //The index will be initialized in the construtor based on the last
    //geometry instance in the geometry input file.  
    //If changes are made to the geometry at runtime,
    //then these need to reflected in the index using insert/remove.
    //Local changes can be accumulated on the local output manager branch, then broadcast to all others.
    //If a large number of changes are made, it may be better to simply reconstruct the index to keep the tree well balanced
    //and ensure that query performance is optimal (Perhaps every checkpoint period, the newest geometry is re-read and the 
    //index reconstructed???
  
    //The following variables are used to interface the interperter to
    //ADHydro state.  They are passed by reference to the interperter
    //The spatial indexes are managed soley by the output manager,
    //and thus can be passed as const references, whereas the maps
    //are updated by the interperter based on interperted information,
    //and the reference is upated by the interperter for use by the output manager.
    rtree meshSpatialIndex;
    rtree channelSpatialIndex;
    std::unordered_map<int, bool> meshOutputElements;
    std::unordered_map<int, bool> channelOutputElements;
    
    //These two maps hold elementID, element* pairs.  Each processor
    //registers its region with the output manager, and the manager
    //then knows every local mesh element by ID, and can look up its state
    //by following the pointer at that ID.
    std::unordered_map<int, const Element*> meshElements;
    std::unordered_map<int, const Element*> channelElements;
    //Should use unordered_map for const lookup times, but these are only offically supported in std library
    //with c++11.  The lastet gnu compiler supports most of the c++ 11 features/namespace, but must be compiled with
    //-std=c++11	NOT EXACTLY TRUE, IS WORKING WITH g++ 4.8.5 with no -std flag
    //This IS the latest approved C++ standard, and we probably should move to it.  Most modern compilers will compile c++11
    //and if we run across one that doesn't, they should probably have a solid reason for not upgrading.  In these cases,
    //we could also run with boost or tr1 extensions...or simply revert back to std::map at a performance hit

    
    //A single interperter instance is used throught the life of the output manager.  Calls to 
    //interperter.interpert() cause the spec file to be re-interperted and all values of the
    //interperter interface are updated.
    Interperter interperter;
    //A flag/counter to determine if this output manager in fact must output an elements state
    int numberOfOutputElements;
    double outputInterval;
};


#endif
