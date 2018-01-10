#ifndef __CHECKPOINT_MANAGER_DATA_TYPES_H__
#define __CHECKPOINT_MANAGER_DATA_TYPES_H__

#include "neighbor_proxy.h"
#include "simple_vadose_zone.h"
#include "evapo_transpiration.h"

// The following are some data types that are needed for elements to communicate their state to the checkpoint manager, and to place that data into the input/output files.
// Breaking these types out into a separate header file simplified some include dependencies.

// The states of the vadose zone and evapotranspiration simulations are generally complex data structures.
// We want ADHydro to support using third party modules for those simulations so we don't necessarily have control over the data structures,
// but we need to save their states in the input/output files as fixed size blobs.
// These declarations provide fixed size blobs for that data.  Any new simulation module must shoehorn their state into a fixed size blob,
// generally by providing a pup routine, and update the size definition to make sure it will fit.
#define VADOSE_ZONE_STATE_BLOB_SIZE         (sizeof(SimpleVadoseZone))
#define EVAPO_TRANSPIRATION_STATE_BLOB_SIZE (sizeof(EvapoTranspirationStateStruct))
// FIXME test with these sizes too small

typedef unsigned char VadoseZoneStateBlob[VADOSE_ZONE_STATE_BLOB_SIZE];
typedef unsigned char EvapoTranspirationStateBlob[EVAPO_TRANSPIRATION_STATE_BLOB_SIZE];

PUPbytes(VadoseZoneStateBlob);
PUPbytes(EvapoTranspirationStateBlob);

// A GroundwaterModeEnum describes the current mode of the multi-layer groundwater simulation in a single mesh element.
enum GroundwaterModeEnum
{
    SATURATED_AQUIFER,   // Implies no perched water table.
    UNSATURATED_AQUIFER, // Implies no perched water table.
    PERCHED_WATER_TABLE, // Implies aquifer is not saturated.
    NO_MULTILAYER,       // Implies at least one of soil and/or aquifer doesn't exist.
};

PUPbytes(GroundwaterModeEnum);

// A MeshState is the information about a MeshElement that needs to be output to a state file.
class MeshState
{
public:
    
    // Charm++ pack/unpack method.
    //
    // Parameters:
    //
    // p - Pack/unpack processing object.
    inline void pup(PUP::er &p)
    {
        p | elementNumber;
        p | evapoTranspirationState;
        p | surfaceWater;
        p | surfaceWaterCreated;
        p | groundwaterMode;
        p | perchedHead;
        p | soilWater;
        p | soilWaterCreated;
        p | aquiferHead;
        p | aquiferWater;
        p | aquiferWaterCreated;
        p | deepGroundwater;
        p | precipitationRate;
        p | precipitationCumulative;
        p | evaporationRate;
        p | evaporationCumulative;
        p | transpirationRate;
        p | transpirationCumulative;
        p | canopyWater;
        p | snowWater;
        p | rootZoneWater;
        p | totalGroundwater;
        p | neighbors;
    }
    
    size_t                      elementNumber;
    EvapoTranspirationStateBlob evapoTranspirationState;
    double                      surfaceWater;
    double                      surfaceWaterCreated;
    GroundwaterModeEnum         groundwaterMode;
    double                      perchedHead;
    VadoseZoneStateBlob         soilWater;
    double                      soilWaterCreated;
    double                      aquiferHead;
    VadoseZoneStateBlob         aquiferWater;
    double                      aquiferWaterCreated;
    double                      deepGroundwater;
    double                      precipitationRate;
    double                      precipitationCumulative;
    double                      evaporationRate;
    double                      evaporationCumulative;
    double                      transpirationRate;
    double                      transpirationCumulative;
    double                      canopyWater;
    double                      snowWater;
    double                      rootZoneWater;
    double                      totalGroundwater;
    std::vector<NeighborState>  neighbors;
};

// A ChannelState is the information about a ChannelElement that needs to be output to a state file.
class ChannelState
{
public:
    
    // Charm++ pack/unpack method.
    //
    // Parameters:
    //
    // p - Pack/unpack processing object.
    inline void pup(PUP::er &p)
    {
        p | elementNumber;
        p | evapoTranspirationState;
        p | surfaceWater;
        p | surfaceWaterCreated;
        p | precipitationRate;
        p | precipitationCumulative;
        p | evaporationRate;
        p | evaporationCumulative;
        p | snowWater;
        p | neighbors;
    }
    
    size_t                      elementNumber;
    EvapoTranspirationStateBlob evapoTranspirationState;
    double                      surfaceWater;
    double                      surfaceWaterCreated;
    double                      precipitationRate;
    double                      precipitationCumulative;
    double                      evaporationRate;
    double                      evaporationCumulative;
    double                      snowWater;
    std::vector<NeighborState>  neighbors;
};

#endif // __CHECKPOINT_MANAGER_DATA_TYPES_H__
