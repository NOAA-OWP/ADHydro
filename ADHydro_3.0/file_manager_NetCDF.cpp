#include "file_manager_NetCDF.h"
#include "all.h"
#include <netcdf_par.h>

bool FileManagerNetCDF::writeOutput(const TimePointState& timePointState)
{
  bool        error    = false;              // Error flag.
  int         ncErrorCode;                   // Return value of NetCDF functions.
  std::string filename;                      // Filename of file to create.
  int         fileID;                        // NetCDF file handle.
  bool        fileOpen = false;              // Whether the file is open.
  int         intValue;                      // For writing ints    to the NetCDF file.
  double      doubleValue;                   // For writing doubles to the NetCDF file.
  int         meshElementsDimensionID;       // ID of dimension in NetCDF file.
  int         meshSoilLayersDimensionID;     // ID of dimension in NetCDF file.
  int         meshNeighborsDimensionID;      // ID of dimension in NetCDF file.
  int         channelElementsDimensionID;    // ID of dimension in NetCDF file.
  int         channelNeighborsDimensionID;   // ID of dimension in NetCDF file.
  nc_type     EvapoTranspirationStateTypeID; // ID of type in NetCDF file.
  nc_type     VadoseZoneStateTypeID;         // ID of type in NetCDF file.

  // Build filename and create file.
  filename = timePointState.createFilename();

  ncErrorCode = nc_create_par(filename.c_str(), NC_NETCDF4 | NC_MPIIO | NC_WRITE | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);

  if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    {
      if (!(NC_NOERR == ncErrorCode))
        {
          ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create NetCDF file %s.  NetCDF error message: %s.\n", filename.c_str(), nc_strerror(ncErrorCode));
          error = true;
        }
    }

  if (!error)
    {
      fileOpen = true;
    }

  // Create attributes.
  // FIXME write correct value into geometryInstance and parameterInstance attributes.
  if (!error)
    {
      intValue    = -1;
      ncErrorCode = nc_put_att_int(fileID, NC_GLOBAL, "geometryInstance", NC_INT, 1, &intValue);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create attribute geometryInstance.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  if (!error)
    {
      intValue    = -1;
      ncErrorCode = nc_put_att_int(fileID, NC_GLOBAL, "parameterInstance", NC_INT, 1, &intValue);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create attribute parameterInstance.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  if (!error)
    {
      doubleValue = timePointState.referenceDate;
      ncErrorCode = nc_put_att_double(fileID, NC_GLOBAL, "referenceDate", NC_DOUBLE, 1, &doubleValue);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create attribute referenceDate.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  if (!error)
    {
      doubleValue = timePointState.outputTime;
      ncErrorCode = nc_put_att_double(fileID, NC_GLOBAL, "currentTime", NC_DOUBLE, 1, &doubleValue);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create attribute currentTime.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  // Create dimenisons.
  if (!error && 0 < timePointState.globalNumberOfMeshElements)
    {
      ncErrorCode = nc_def_dim(fileID, "meshElements", timePointState.globalNumberOfMeshElements, &meshElementsDimensionID);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create dimension meshElements.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
        }

      if (!error && 0 < timePointState.maximumNumberOfMeshSoilLayers)
        {
          ncErrorCode = nc_def_dim(fileID, "meshSoilLayers", timePointState.maximumNumberOfMeshSoilLayers, &meshSoilLayersDimensionID);

          if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            {
              if (!(NC_NOERR == ncErrorCode))
                {
                  ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create dimension meshSoilLayers.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                  error = true;
                }
            }
        }

      if (!error && 0 < timePointState.maximumNumberOfMeshNeighbors)
        {
          ncErrorCode = nc_def_dim(fileID, "meshNeighbors", timePointState.maximumNumberOfMeshNeighbors, &meshNeighborsDimensionID);

          if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            {
              if (!(NC_NOERR == ncErrorCode))
                {
                  ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create dimension meshNeighbors.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                  error = true;
                }
            }
        }
    }

  if (!error && 0 < timePointState.globalNumberOfChannelElements)
    {
      ncErrorCode = nc_def_dim(fileID, "channelElements", timePointState.globalNumberOfChannelElements, &channelElementsDimensionID);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create dimension channelElements.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
        }

      if (!error && 0 < timePointState.maximumNumberOfChannelNeighbors)
        {
          ncErrorCode = nc_def_dim(fileID, "channelNeighbors", timePointState.maximumNumberOfChannelNeighbors, &channelNeighborsDimensionID);

          if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            {
              if (!(NC_NOERR == ncErrorCode))
                {
                  ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create dimension channelNeighbors.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                  error = true;
                }
            }
        }
    }

  // Create data types.
  if (!error)
    {
      ncErrorCode = nc_def_opaque(fileID, sizeof(EvapoTranspirationStateBlob), "EvapoTranspirationState", &EvapoTranspirationStateTypeID);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create data type EvapoTranspirationState.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  if (!error)
    {
      ncErrorCode = nc_def_opaque(fileID, sizeof(VadoseZoneStateBlob), "VadoseZoneState", &VadoseZoneStateTypeID);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not create data type VadoseZoneState.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  // Create variables.
  if (0 < timePointState.globalNumberOfMeshElements)
    {
      if (!error)
        {
          error = createVariable(fileID, "meshSurfacewaterDepth", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters", "Surfacewater depth in mesh elements.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshSurfacewaterCreated", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters",
              "Surfacewater created or destroyed in mesh elements as the result of unusual situations including roundoff error.  Positive means water was created.  Negative means water was destroyed.");
        }

      if (0 < timePointState.maximumNumberOfMeshSoilLayers)
        {
          if (!error)
            {
              error = createVariable(fileID, "meshGroundwaterHead", NC_DOUBLE, 2, meshElementsDimensionID, meshSoilLayersDimensionID, 0, "meters", "Z coordinate of water table in mesh elements.");
            }

          if (!error)
            {
              error = createVariable(fileID, "meshGroundwaterRecharge", NC_DOUBLE, 2, meshElementsDimensionID, meshSoilLayersDimensionID, 0, "meters", "Water in groundwater recharge bucket in mesh elements.");
            }

          if (!error)
            {
              error = createVariable(fileID, "meshGroundwaterCreated", NC_DOUBLE, 2, meshElementsDimensionID, meshSoilLayersDimensionID, 0, "meters",
                  "Groundwater created or destroyed in mesh elements as the result of unusual situations including roundoff error.  Positive means water was created.  Negative means water was destroyed.");
            }
        }

      if (!error)
        {
          error = createVariable(fileID, "meshPrecipitationRate", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters/second", "Instantaneous precipitation rate in mesh elements as a negative number.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshPrecipitationCumulative", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters", "Cumulative precipitation in mesh elements as a negative number.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshEvaporationRate", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters/second",
              "Instantaneous evaporation or condensation rate in mesh elements.  Positive means water removed from the element.  Negative means water added to the element.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshEvaporationCumulative", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters",
              "Cumulative evaporation or condensation in mesh elements.  Positive means water removed from the element.  Negative means water added to the element.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshTranspirationRate", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters/second", "Instantaneous transpiration rate in mesh elements as a positive number.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshTranspirationCumulative", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters", "Cumulative transpiration in mesh elements as a positive number.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshEvapoTranspirationState", EvapoTranspirationStateTypeID, 1, meshElementsDimensionID, 0, 0, NULL, "Opaque blob of evapotranspiration state in mesh elements.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshCanopyWaterEquivalent", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters", "Water equivalent of total snow and liquid water in canopy in mesh elements.");
        }

      if (!error)
        {
          error = createVariable(fileID, "meshSnowWaterEquivalent", NC_DOUBLE, 1, meshElementsDimensionID, 0, 0, "meters", "Water equivalent of total snow and liquid water in snow pack in mesh elements.");
        }

      if (0 < timePointState.maximumNumberOfMeshSoilLayers)
        {
          if (!error)
            {
              error = createVariable(fileID, "meshVadoseZoneState", VadoseZoneStateTypeID, 2, meshElementsDimensionID, meshSoilLayersDimensionID, 0, NULL, "Opaque blob of vadose zone state in mesh elements.");
            }

          if (!error)
            {
              error = createVariable(fileID, "meshRootZoneWater", NC_DOUBLE, 2, meshElementsDimensionID, meshSoilLayersDimensionID, 0, "meters", "Water in root zone in mesh elements.  Does not include groundwater recharge.");
            }

          if (!error)
            {
              error = createVariable(fileID, "meshTotalSoilWater", NC_DOUBLE, 2, meshElementsDimensionID, meshSoilLayersDimensionID, 0, "meters", "Total soil water in mesh elements.  Includes groundwater recharge.");
            }
        }

      if (0 < timePointState.maximumNumberOfMeshNeighbors)
        {
          if (!error)
            {
              error = createVariable(fileID, "meshSurfacewaterNeighborsExpirationTime", NC_DOUBLE, 2, meshElementsDimensionID, meshNeighborsDimensionID, 0, "seconds",
                  "Value of currentTime when flow rate negotiated with neighbor expires.");
            }

          if (!error)
            {
              error = createVariable(fileID, "meshSurfacewaterNeighborsFlowRate", NC_DOUBLE, 2, meshElementsDimensionID, meshNeighborsDimensionID, 0, "meters^3/second",
                  "Instantaneous flow rate of surfacewater between a mesh element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
            }

          if (!error)
            {
              error = createVariable(fileID, "meshSurfacewaterNeighborsFlowCumulative", NC_DOUBLE, 2, meshElementsDimensionID, meshNeighborsDimensionID, 0, "meters^3",
                  "Cumulative flow of surfacewater between a mesh element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
            }

          if (0 < timePointState.maximumNumberOfMeshSoilLayers)
            {
              if (!error)
                {
                  error = createVariable(fileID, "meshGroundwaterNeighborsExpirationTime", NC_DOUBLE, 3, meshElementsDimensionID, meshSoilLayersDimensionID, meshNeighborsDimensionID, "seconds",
                      "Value of currentTime when flow rate negotiated with neighbor expires.");
                }

              if (!error)
                {
                  error = createVariable(fileID, "meshGroundwaterNeighborsFlowRate", NC_DOUBLE, 3, meshElementsDimensionID, meshSoilLayersDimensionID, meshNeighborsDimensionID, "meters^3/second",
                      "Instantaneous flow rate of groundwater between a mesh element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
                }

              if (!error)
                {
                  error = createVariable(fileID, "meshGroundwaterNeighborsFlowCumulative", NC_DOUBLE, 3, meshElementsDimensionID, meshSoilLayersDimensionID, meshNeighborsDimensionID, "meters^3",
                      "Cumulative flow of groundwater between a mesh element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
                }
            }
        }
    }

  if (0 < timePointState.globalNumberOfChannelElements)
    {
      if (!error)
        {
          error = createVariable(fileID, "channelSurfacewaterDepth", NC_DOUBLE, 1, channelElementsDimensionID, 0, 0, "meters", "Surfacewater depth in channel elements.");
        }

      if (!error)
        {
          error = createVariable(fileID, "channelSurfacewaterCreated", NC_DOUBLE, 1, channelElementsDimensionID, 0, 0, "meters",
              "Surfacewater created or destroyed in channel elements as the result of unusual situations including roundoff error.  Positive means water was created.  Negative means water was destroyed.");
        }

      if (!error)
        {
          error = createVariable(fileID, "channelPrecipitationRate", NC_DOUBLE, 1, channelElementsDimensionID, 0, 0, "meters/second", "Instantaneous precipitation rate in channel elements as a negative number.");
        }

      if (!error)
        {
          error = createVariable(fileID, "channelPrecipitationCumulative", NC_DOUBLE, 1, channelElementsDimensionID, 0, 0, "meters", "Cumulative precipitation in channel elements as a negative number.");
        }

      if (!error)
        {
          error = createVariable(fileID, "channelEvaporationRate", NC_DOUBLE, 1, channelElementsDimensionID, 0, 0, "meters/second",
              "Instantaneous evaporation or condensation rate in channel elements.  Positive means water removed from the element.  Negative means water added to the element.");
        }

      if (!error)
        {
          error = createVariable(fileID, "channelEvaporationCumulative", NC_DOUBLE, 1, channelElementsDimensionID, 0, 0, "meters",
              "Cumulative evaporation or condensation in channel elements.  Positive means water removed from the element.  Negative means water added to the element.");
        }

      if (!error)
        {
          error = createVariable(fileID, "channelEvapoTranspirationState", EvapoTranspirationStateTypeID, 1, channelElementsDimensionID, 0, 0, NULL, "Opaque blob of evapotranspiration state in channel elements.");
        }

      if (!error)
        {
          error = createVariable(fileID, "channelSnowWaterEquivalent", NC_DOUBLE, 1, channelElementsDimensionID, 0, 0, "meters", "Water equivalent of total snow and liquid water in snow pack in channel elements.");
        }

      if (0 < timePointState.maximumNumberOfChannelNeighbors)
        {
          if (!error)
            {
              error = createVariable(fileID, "channelSurfacewaterNeighborsExpirationTime", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, 0, "seconds",
                  "Value of currentTime when flow rate negotiated with neighbor expires.");
            }

          if (!error)
            {
              error = createVariable(fileID, "channelSurfacewaterNeighborsFlowRate", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, 0, "meters^3/second",
                  "Instantaneous flow rate of surfacewater between a channel element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
            }

          if (!error)
            {
              error = createVariable(fileID, "channelSurfacewaterNeighborsFlowCumulative", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, 0, "meters^3",
                  "Cumulative flow of surfacewater between a channel element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
            }

          if (!error)
            {
              error = createVariable(fileID, "channelGroundwaterNeighborsExpirationTime", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, 0, "seconds",
                  "Value of currentTime when flow rate negotiated with neighbor expires.");
            }

          if (!error)
            {
              error = createVariable(fileID, "channelGroundwaterNeighborsFlowRate", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, 0, "meters^3/second",
                  "Instantaneous flow rate of groundwater between a channel element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
            }

          if (!error)
            {
              error = createVariable(fileID, "channelGroundwaterNeighborsFlowCumulative", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, 0, "meters^3",
                  "Cumulative flow of groundwater between a channel element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
            }
        }
    }

  // Write variables.
  if (0 < timePointState.localNumberOfMeshElements)
    {
      if (!error)
        {
          error = writeVariable(fileID, "meshSurfacewaterDepth", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshSurfacewaterDepth);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshSurfacewaterCreated", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshSurfacewaterCreated);
        }

      if (0 < timePointState.maximumNumberOfMeshSoilLayers)
        {
          if (!error)
            {
              error = writeVariable(fileID, "meshGroundwaterHead", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers, 0,
                  timePointState.meshGroundwaterHead);
            }

          if (!error)
            {
              error = writeVariable(fileID, "meshGroundwaterRecharge", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers, 0,
                  timePointState.meshGroundwaterRecharge);
            }

          if (!error)
            {
              error = writeVariable(fileID, "meshGroundwaterCreated", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers, 0,
                  timePointState.meshGroundwaterCreated);
            }
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshPrecipitationRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshPrecipitationRate);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshPrecipitationCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshPrecipitationCumulative);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshEvaporationRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshEvaporationRate);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshEvaporationCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshEvaporationCumulative);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshTranspirationRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshTranspirationRate);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshTranspirationCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshTranspirationCumulative);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshEvapoTranspirationState", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshEvapoTranspirationState);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshCanopyWaterEquivalent", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshCanopyWaterEquivalent);
        }

      if (!error)
        {
          error = writeVariable(fileID, "meshSnowWaterEquivalent", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, 0, timePointState.meshSnowWaterEquivalent);
        }

      if (0 < timePointState.maximumNumberOfMeshSoilLayers)
        {
          if (!error)
            {
              error = writeVariable(fileID, "meshVadoseZoneState", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers, 0,
                  timePointState.meshVadoseZoneState);
            }

          if (!error)
            {
              error = writeVariable(fileID, "meshRootZoneWater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers, 0,
                  timePointState.meshRootZoneWater);
            }

          if (!error)
            {
              error = writeVariable(fileID, "meshTotalSoilWater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers, 0,
                  timePointState.meshTotalSoilWater);
            }
        }

      if (0 < timePointState.maximumNumberOfMeshNeighbors)
        {
          if (!error)
            {
              error = writeVariable(fileID, "meshSurfacewaterNeighborsExpirationTime", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors, 0,
                  timePointState.meshSurfacewaterNeighborsExpirationTime);
            }

          if (!error)
            {
              error = writeVariable(fileID, "meshSurfacewaterNeighborsFlowRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors, 0,
                  timePointState.meshSurfacewaterNeighborsFlowRate);
            }

          if (!error)
            {
              error = writeVariable(fileID, "meshSurfacewaterNeighborsFlowCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors, 0,
                  timePointState.meshSurfacewaterNeighborsFlowCumulative);
            }

          if (0 < timePointState.maximumNumberOfMeshSoilLayers)
            {
              if (!error)
                {
                  error = writeVariable(fileID, "meshGroundwaterNeighborsExpirationTime", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers,
                      timePointState.maximumNumberOfMeshNeighbors, timePointState.meshGroundwaterNeighborsExpirationTime);
                }

              if (!error)
                {
                  error = writeVariable(fileID, "meshGroundwaterNeighborsFlowRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers,
                      timePointState.maximumNumberOfMeshNeighbors, timePointState.meshGroundwaterNeighborsFlowRate);
                }

              if (!error)
                {
                  error = writeVariable(fileID, "meshGroundwaterNeighborsFlowCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshSoilLayers,
                      timePointState.maximumNumberOfMeshNeighbors, timePointState.meshGroundwaterNeighborsFlowCumulative);
                }
            }
        }
    }

  if (0 < timePointState.localNumberOfChannelElements)
    {
      if (!error)
        {
          error = writeVariable(fileID, "channelSurfacewaterDepth", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, 0, timePointState.channelSurfacewaterDepth);
        }

      if (!error)
        {
          error = writeVariable(fileID, "channelSurfacewaterCreated", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, 0, timePointState.channelSurfacewaterCreated);
        }

      if (!error)
        {
          error = writeVariable(fileID, "channelPrecipitationRate", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, 0, timePointState.channelPrecipitationRate);
        }

      if (!error)
        {
          error = writeVariable(fileID, "channelPrecipitationCumulative", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, 0, timePointState.channelPrecipitationCumulative);
        }

      if (!error)
        {
          error = writeVariable(fileID, "channelEvaporationRate", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, 0, timePointState.channelEvaporationRate);
        }

      if (!error)
        {
          error = writeVariable(fileID, "channelEvaporationCumulative", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, 0, timePointState.channelEvaporationCumulative);
        }

      if (!error)
        {
          error = writeVariable(fileID, "channelEvapoTranspirationState", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, 0, timePointState.channelEvapoTranspirationState);
        }

      if (!error)
        {
          error = writeVariable(fileID, "channelSnowWaterEquivalent", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, 0, timePointState.channelSnowWaterEquivalent);
        }

      if (0 < timePointState.maximumNumberOfChannelNeighbors)
        {
          if (!error)
            {
              error = writeVariable(fileID, "channelSurfacewaterNeighborsExpirationTime", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors, 0,
                  timePointState.channelSurfacewaterNeighborsExpirationTime);
            }

          if (!error)
            {
              error = writeVariable(fileID, "channelSurfacewaterNeighborsFlowRate", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors, 0,
                  timePointState.channelSurfacewaterNeighborsFlowRate);
            }

          if (!error)
            {
              error = writeVariable(fileID, "channelSurfacewaterNeighborsFlowCumulative", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors, 0,
                  timePointState.channelSurfacewaterNeighborsFlowCumulative);
            }

          if (!error)
            {
              error = writeVariable(fileID, "channelGroundwaterNeighborsExpirationTime", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors, 0,
                  timePointState.channelGroundwaterNeighborsExpirationTime);
            }

          if (!error)
            {
              error = writeVariable(fileID, "channelGroundwaterNeighborsFlowRate", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors, 0,
                  timePointState.channelGroundwaterNeighborsFlowRate);
            }

          if (!error)
            {
              error = writeVariable(fileID, "channelGroundwaterNeighborsFlowCumulative", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors, 0,
                  timePointState.channelGroundwaterNeighborsFlowCumulative);
            }
        }
    }

  if (fileOpen)
    {
      ncErrorCode = nc_close(fileID);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeOutput: could not close NetCDF file %s.  NetCDF error message: %s.\n", filename.c_str(), nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  return error;
}

bool FileManagerNetCDF::createVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID1, int dimensionID2, int dimensionID3, const char* units, const char* comment)
{
  bool error           = false;                                      // Error flag.
  int  ncErrorCode;                                                  // Return value of NetCDF functions.
  int  dimensionIDs[3] = {dimensionID1, dimensionID2, dimensionID3}; // For passing dimension IDs.
  int  variableID;                                                   // ID of the variable being created.

  if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
      ADHYDRO_ASSERT(NULL != variableName && 1 <= numberOfDimensions && 3 >= numberOfDimensions);
    }

  // Create the variable.
  ncErrorCode = nc_def_var(fileID, variableName, dataType, numberOfDimensions, dimensionIDs, &variableID);

  if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    {
      if (!(NC_NOERR == ncErrorCode))
        {
          ADHYDRO_ERROR("ERROR in FileManagerNetCDF::createVariable: could not create variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
          error = true;
        }
    }

  // Set units attribute.
  if (!error && NULL != units)
    {
      ncErrorCode = nc_put_att_text(fileID, variableID, "units", strlen(units), units);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::createVariable: could not create units attribute of variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  // Set comment attribute
  if (!error && NULL != comment)
    {
      ncErrorCode = nc_put_att_text(fileID, variableID, "comment", strlen(comment), comment);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::createVariable: could not create comment attribute of variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  return error;
}

bool FileManagerNetCDF::writeVariable(int fileID, const char* variableName, size_t startDimension1, size_t countDimension1, size_t sizeDimension2, size_t sizeDimension3, void* data)
{
  bool   error    = false;                                             // Error flag.
  int    ncErrorCode;                                                  // Return value of NetCDF functions.
  int    variableID;                                                   // ID of variable being written.
  size_t start[3] = {startDimension1, 0, 0};                           // For specifying subarray to write.
  size_t count[3] = {countDimension1, sizeDimension2, sizeDimension3}; // For specifying subarray to write.

  if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
      ADHYDRO_ASSERT(NULL != variableName && 0 < countDimension1 && NULL != data);
    }

  // Get the variable ID.
  ncErrorCode = nc_inq_varid(fileID, variableName, &variableID);

  if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    {
      if (!(NC_NOERR == ncErrorCode))
        {
          ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeVariable: could not get ID of variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
          error = true;
        }
    }

  // Write the variable.
  if (!error)
    {
      ncErrorCode = nc_put_vara(fileID, variableID, start, count, data);

      if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
          if (!(NC_NOERR == ncErrorCode))
            {
              ADHYDRO_ERROR("ERROR in FileManagerNetCDF::writeVariable: could not write variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
              error = true;
            }
        }
    }

  return error;
}
