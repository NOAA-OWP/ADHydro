#ifndef __MESH_ELEMENT_H__
#define __MESH_ELEMENT_H__

#include "element.h"
#include "evapo_transpiration.h"

class MeshGroundwaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // FIXME comment and implement
  void sendWater(MaterialTransfer water);
};

class MeshGroundwaterChannelNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // FIXME comment and implement
  void sendWater(MaterialTransfer water);
};

class MeshSurfacewaterMeshNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // FIXME comment and implement
  void sendWater(MaterialTransfer water);
};

class MeshSurfacewaterChannelNeighborProxy : public SimpleNeighborProxy
{
public:
  
  // FIXME comment and implement
  void sendWater(MaterialTransfer water);
};

class InfiltrationAndGroundwater
{
public:
  
  enum InfiltrationMethodEnum
  {
    NO_INFILTRATION,      // This is a situation like exposed bedrock where there is no possibility of infiltration and no groundwater storage.  Almost all
                          // member variables are ignored except that porosity must still have a valid value because Noah-MP doesn't allow layers of zero
                          // thickness or zero porosity so we still have to fill in the soil moisture struct with something.
    TRIVIAL_INFILTRATION, // This method infiltrates surfacewater directly to groundwater at a rate equal to conductivity.  No vadose zone state is modeled.
                          // Whenever vadose zone state is needed it is assumed to be in equilibrium with groundwaterHead.  If groundwaterMethod is
                          // SHALLOW_AQUIFER then quantity of water in the domain is groundwaterRecharge + (groundwaterHead - layerZBottom) * porosity, and no
                          // vadose zone water is counted.  If groundwaterMethod is DEEP_AQUIFER then quantity of water in the domain is groundwaterRecharge.
    GARTO_INFILTRATION,   // Vadose zone water is simulated using Green-Ampt infiltration and redistribution method for surface wetting front, 
                          // and T-O method for groundwater front.
  };
  
  enum GroundwaterMethodEnum
  {
    NO_AQUIFER,      // This method must be used if and only if infiltrationMethod is NO_INFILTRATION.
    DEEP_AQUIFER,    // The water table is assumed to always be below the modeled domain.  If infiltrationMethod is TRIVIAL_INFILTRATION then the member
                     // variable groundwaterHead still needs a valid value because it is used to fill in the soil moisture struct, but its value is never
                     // changed.  Any water that makes it to groundwaterRecharge gets sent immediately to an aquifer storage bucket, which may collect water
                     // from many elements.  There is no lateral flow to neighboring elements.  The meshNeighbors and channelNeighbors vectors must be empty.
    SHALLOW_AQUIFER, // Both vadose zone and water table are modeled.  Water that makes it to groundwaterRecharge stays in the modeled domain.  Water below
                     // groundwaterHead may flow laterally to neighboring elements.
  };
  
  // Fill in the evapo-transpiration soil moisture variables from the current
  // groundwater state.
  //
  // Parameters:
  //
  // elementZSurface - Surface elevation in meters.
  // zSnso           - 1D array of layer bottom depth in meters from snow
  //                   surface of each snow and soil layer.  Specify depths as
  //                   negative numbers.
  // evapoTranspirationSoilMoisture
  //                 - Soil moisture struct passed by reference will be filled
  //                   in based on current groundwater state.
  void fillInEvapoTranspirationSoilMoistureStruct(double elementZSurface, float zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS],
                                                  EvapoTranspirationSoilMoistureStruct& evapoTranspirationSoilMoisture);
  
  // Remove surface evaporation from groundwater.
  //
  // Returns: The amount of water in meters that was able to be supplied.
  //
  // Parameters:
  //
  // unsatisfiedEvaporation - The requested amount of water in meters.
  double evaporate(double unsatisfiedEvaporation);
  
  // Remove transpiration from groundwater.
  //
  // Returns: The amount of water in meters that was able to be supplied.
  //
  // Parameters:
  //
  // unsatisfiedTranspiration - The requested amount of water in meters.
  double transpire(double unsatisfiedTranspiration);
  
  // Perform infiltration, which takes water from surfacewaterDepth and takes
  // or puts water into groundwaterRecharge.  Then limit and send groundwater
  // outflows taking the water from groundwaterRecharge.  Groundwater inflows
  // from last timestep will already be in groundwaterRecharge.  Then update
  // groundwaterHead based on net groundwaterRecharge and take or put
  // groundwaterRecharge water back into the domain, or if the domain is full
  // put it back in surfacewaterDepth.
  //
  // Parameters:
  //
  // currentTime       - Current simulation time in seconds since
  //                     referenceDate.
  // timestepEndTime   - Simulation time at the end of the current timestep in
  //                     seconds since referenceDate.
  // elementZSurface   - Surface elevation in meters.
  // elementArea       - Element area in square meters.
  // surfacewaterDepth - Scalar passed by reference containing the amount of
  //                     surfacewater available for infiltration in meters.
  //                     Will be updated to the new amount of surfacewater
  //                     after infiltration or exfiltration.
  void doInfiltrationAndSendGroundwaterOutflows(double currentTime, double timestepEndTime, double elementZSurface,
                                                double elementArea, double& surfacewaterDepth);
  
  // The methods to use to calculate infiltration and groundwater flow.
  InfiltrationMethodEnum infiltrationMethod;
  GroundwaterMethodEnum  groundwaterMethod;
  
  // Geometric coordinates.
  double layerZBottom; // Elevation in meters.
  
  // Hydraulic parameters.
  double conductivity; // Meters per second.
  double porosity;     // Unitless.
  
  // Water state variables.
  double groundwaterHead;     // Elevation in meters.
  double groundwaterRecharge; // Meters of water.
  double groundwaterError;    // Meters of water.  Positive means water was created.  Negative means water was destroyed.
  
  // Neighbors.
  std::vector<MeshGroundwaterMeshNeighborProxy>    meshNeighbors;
  std::vector<MeshGroundwaterChannelNeighborProxy> channelNeighbors;
};

class MeshElement : public Element
{
public:
  
  // FIXME what to do about iterator?
  
  // FIXME comment
  bool doPointProcessesAndSendOutflows();
  
  // FIXME comment
  bool allInflowsArrived();
  
  // FIXME comment
  void receiveInflowsAndAdvanceTime();
  
  // Geometric coordinates.
  double elementX;        // Meters.
  double elementY;        // Meters.
  double elementZSurface; // Elevation in meters.
  double elementArea;     // Square meters.
  double latitude;        // Radians.
  double longitude;       // Radians.
  
  // Identification parameters.
  int elementNumber;  // Mesh element ID number of this element.
  int catchment;      // Catchment ID number that this element belongs to.
  int vegetationType; // Vegetation type from Noah-MP VEGPARM.TBL file.
  int soilType;       // Soil type form Noah-MP SOILPARM.TBL file.
  
  // Water state variables.
  double surfacewaterDepth; // Meters.
  double surfacewaterError; // Meters.  Positive means water was created.  Negative means water was destroyed.
  
  // Water flow state variables.
  double precipitationRate;                // Precipitation falling on the mesh element in meters of water per second.  Negative means water added to the
                                           // element.  Must be non-positive.
  double precipitationCumulativeShortTerm; // precipitationCumulativeShortTerm plus precipitationCumulativeLongTerm together are the cumulative precipitation
  double precipitationCumulativeLongTerm;  // that has fallen on the mesh element in meters of water.  Negative means water added to the element.  Must be
                                           // non-positive.  Two variables are used because over a long simulation this value could become quite large, and the
                                           // amount of water added each timestep can be quite small so roundoff error becomes a concern.  New precipitation is
                                           // added to precipitationCumulativeShortTerm each timestep, and occasionally precipitationCumulativeShortTerm is
                                           // added to precipitationCumulativeLongTerm and precipitationCumulativeShortTerm is reset to zero.
  double evaporationRate;                  // Evaporation or condensation on the mesh element in meters of water per second.  Positive means water removed from
                                           // the element.  Negative means water added to the element.
  double evaporationCumulativeShortTerm;   // evaporationCumulativeShortTerm plus evaporationCumulativeLongTerm together are the cumulative evaporation or
  double evaporationCumulativeLongTerm;    // condensation on the mesh element in meters of water.  Positive means water removed from the element.  Negative
                                           // means water added to the element.  See the comment of precipitationCumulativeShortTerm for why two variables are
                                           // used.
  double transpirationRate;                // Transpiration from the mesh element in meters of water per second.  Positive means water removed from the
                                           // element.  Must be non-negative.
  double transpirationCumulativeShortTerm; // transpirationCumulativeShortTerm plus transpirationCumulativeLongTerm together are the cumulative transpiration
  double transpirationCumulativeLongTerm;  // from the mesh element in meters of water.  Positive means water removed from the element.  Must be non-negative.
                                           // See the comment of precipitationCumulativeShortTerm for why two variables are used.
  
  // Evapo-transpiration variables.
  EvapoTranspirationForcingStruct evapoTranspirationForcing; // Input variables that come from forcing data.
  EvapoTranspirationStateStruct   evapoTranspirationState;   // State variables whose values are updated and passed to the next timestep.
  
  // Neighbors.
  std::vector<MeshSurfacewaterMeshNeighborProxy>    meshNeighbors;
  std::vector<MeshSurfacewaterChannelNeighborProxy> channelNeighbors;
  
  // This is the first element of a linked list of objects that represent the
  // infiltration and groundwater layers below ground.  There is a linked list
  // because there can be multiple layers.  This one is an object and not a
  // pointer because the linked list has to have at least one object and if you
  // are doing a simple simulation with just one layer everywhere then you
  // don't have to mess with pointers at all.
  // FIXME currently the code is implemented for only a single layer.
  InfiltrationAndGroundwater underground;
};

#endif // __MESH_ELEMENT_H__
