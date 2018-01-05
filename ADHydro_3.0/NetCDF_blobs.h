#ifndef __NETCDF_BLOBS_H__
#define __NETCDF_BLOBS_H__

#include "simple_vadose_zone.h"
#include "evapo_transpiration.h"

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

#endif // __NETCDF_BLOBS_H__
