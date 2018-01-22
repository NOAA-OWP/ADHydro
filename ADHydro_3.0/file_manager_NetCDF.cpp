#include "file_manager_NetCDF.h"
#include "readonly.h"
#include <iomanip>
#include <netcdf_par.h>

// Need explicit template instantiation.
template bool FileManagerNetCDF::readVariable(int, const char*, size_t, size_t, size_t, size_t, size_t, bool, float, bool, float**);

template <typename T> bool FileManagerNetCDF::readVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart, size_t numberOfNodesElements,
                                                           size_t fileDimension, size_t memoryDimension, bool repeatLastValue, T defaultValue, bool mandatory, T** variable)
{
    bool   error = false;          // Error flag.
    size_t ii, jj;                 // Loop counters.
    int    ncErrorCode;            // Return value of NetCDF functions.
    int    variableID;             // ID of variable in NetCDF file.
    size_t start[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
    size_t count[NC_MAX_VAR_DIMS]; // For specifying subarrays when reading from NetCDF file.
    T*     tempVariable;           // For remapping arrays when fileDimension is smaller than memoryDimension
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(NULL != variableName))
        {
            CkError("ERROR in FileManagerNetCDF::readVariable: variableName must not be null.\n");
            error = true;
        }
        
        if (!(1 <= numberOfNodesElements))
        {
            CkError("ERROR in FileManagerNetCDF::readVariable: numberOfNodesElements must be greater than or equal to one.\n");
            error = true;
        }
        
        if (!(NULL != variable))
        {
            CkError("ERROR in FileManagerNetCDF::readVariable: variable must not be null.\n");
            error = true;
        }
    }
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    {
        // fileDimenison must be less than or equal to memoryDimension.  Otherwise there is not enough room to read all of the data and it is an error.
        if (!(1 <= fileDimension && fileDimension <= memoryDimension))
        {
            CkError("ERROR in FileManagerNetCDF::readVariable: fileDimension must be greater than or equal to one and less than or equal to memoryDimension for variable %s in NetCDF file.\n", variableName);
            error = true;
        }
    }
    
    if (!error)
    {
        // Get the variable ID.
        ncErrorCode = nc_inq_varid(fileID, variableName, &variableID);
        
        if (!(NC_NOERR == ncErrorCode))
        {
            if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            {
                // If the variable does not exist it is only an error if the variable is mandatory.
                if (mandatory)
                {
                    CkError("ERROR in FileManagerNetCDF::readVariable: unable to get variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
                    error = true;
                }
            }
        }
        else // If the variable does exist get its data.
        {
            // Fill in the start and count of the dimensions.
            start[0] = instance;
            start[1] = nodeElementStart;
            start[2] = 0;
            count[0] = 1;
            count[1] = numberOfNodesElements;
            count[2] = fileDimension;
            
            // Allocate space if needed.
            if (NULL == *variable)
            {
                *variable = new T[numberOfNodesElements * fileDimension];
            }
            
            // Get the variable data.
            ncErrorCode = nc_get_vara(fileID, variableID, start, count, *variable);
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            {
                if (!(NC_NOERR == ncErrorCode))
                {
                    CkError("ERROR in FileManagerNetCDF::readVariable: unable to read variable %s in NetCDF file.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
                    error = true;
                }
            }
            
            // If fileDimenison is less than memoryDimension we need to remap the array and fill in the extra elements.
            if (!error && fileDimension < memoryDimension)
            {
                // Allocate a new array of the right size for memoryDimension.
                tempVariable = new T[numberOfNodesElements * memoryDimension];
                
                for (ii = 0; ii < numberOfNodesElements; ii++)
                {
                    for (jj = 0; jj < fileDimension; jj++)
                    {
                        // Fill in the values up to fileDimension that were read from the file.
                        tempVariable[ii * memoryDimension + jj] = (*variable)[ii * fileDimension + jj];
                    }
                    
                    for(jj = fileDimension; jj < memoryDimension; jj++)
                    {
                        if (repeatLastValue)
                        {
                            // Fill in the rest of the values by repeating the last value read from the file.
                            tempVariable[ii * memoryDimension + jj] = (*variable)[ii * fileDimension + fileDimension - 1];
                        }
                        else
                        {
                            // Fill in the rest of the values with defaultValue.
                            tempVariable[ii * memoryDimension + jj] = defaultValue;
                        }
                    }
                }
                
                // Delete the wrong size array read in from file and set variable to point to the right size array.
                delete[] *variable;
                *variable = tempVariable;
            }
        } // End if the variable does exist get its data.
    } // End if (!error).
    
    return error;
}

bool FileManagerNetCDF::writeState(double checkpointTime, const TimePointState& timePointState)
{
    bool               error    = false;              // Error flag.
    int                ncErrorCode;                   // Return value of NetCDF functions.
    long               year;                          // For adding date and time to fileneme.
    long               month;                         // For adding date and time to fileneme.
    long               day;                           // For adding date and time to fileneme.
    long               hour;                          // For adding date and time to fileneme.
    long               minute;                        // For adding date and time to fileneme.
    double             second;                        // For adding date and time to fileneme.
    std::ostringstream filename;                      // Filename of the file to write.
    int                fileID;                        // NetCDF file handle.
    bool               fileOpen = false;              // Whether the file is open.
    int                intValue;                      // For writing ints to the NetCDF file.
    int                meshElementsDimensionID;       // ID of dimension in NetCDF file.
    int                meshNeighborsDimensionID;      // ID of dimension in NetCDF file.
    int                channelElementsDimensionID;    // ID of dimension in NetCDF file.
    int                channelNeighborsDimensionID;   // ID of dimension in NetCDF file.
    nc_type            EvapoTranspirationStateTypeID; // ID of type in NetCDF file.
    nc_type            VadoseZoneStateTypeID;         // ID of type in NetCDF file.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
        if (!(timePointState.localNumberOfMeshElements + timePointState.localNumberOfChannelElements == timePointState.elementsReceived))
        {
            CkError("ERROR in FileManagerNetCDF::writeState: It is an error to call writeState with a timePointState that has not received all of its data.\n");
            error = true;
        }
    }
    
    // Build filename and create file.
    if (!error)
    {
        julianToGregorian(Readonly::referenceDate + (checkpointTime / ONE_DAY_IN_SECONDS), &year, &month, &day, &hour, &minute, &second);
        
        filename << Readonly::checkpointDirectoryPath << "/state_" << std::setfill('0') << std::setw(4) << year << std::setw(2) << month << std::setw(2) << day
                 << std::setw(2) << hour << std::setw(2) << minute << std::fixed << std::setprecision(0) << std::setw(2) << second << ".nc";
        
        if (0 == CkMyPe() && 1 <= Readonly::verbosityLevel)
        {
            CkPrintf("Writing checkpoint file: %s\n", filename.str().c_str());
        }
        
        ncErrorCode = nc_create_par(filename.str().c_str(), NC_NETCDF4 | NC_MPIIO | NC_WRITE | NC_NOCLOBBER, MPI_COMM_WORLD, MPI_INFO_NULL, &fileID);
        
        if (NC_NOERR == ncErrorCode)
        {
            fileOpen = true;
        }
        else if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            CkError("ERROR in FileManagerNetCDF::writeState: could not create NetCDF file %s.  NetCDF error message: %s.\n", filename.str().c_str(), nc_strerror(ncErrorCode));
            error = true;
        }
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
                CkError("ERROR in FileManagerNetCDF::writeState: could not create attribute geometryInstance.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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
                CkError("ERROR in FileManagerNetCDF::writeState: could not create attribute parameterInstance.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    if (!error)
    {
        ncErrorCode = nc_put_att_double(fileID, NC_GLOBAL, "referenceDate", NC_DOUBLE, 1, &Readonly::referenceDate);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NC_NOERR == ncErrorCode))
            {
                CkError("ERROR in FileManagerNetCDF::writeState: could not create attribute referenceDate.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    if (!error)
    {
        ncErrorCode = nc_put_att_double(fileID, NC_GLOBAL, "currentTime", NC_DOUBLE, 1, &checkpointTime);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NC_NOERR == ncErrorCode))
            {
                CkError("ERROR in FileManagerNetCDF::writeState: could not create attribute currentTime.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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
                CkError("ERROR in FileManagerNetCDF::writeState: could not create dimension meshElements.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                error = true;
            }
        }
        
        if (!error && 0 < timePointState.maximumNumberOfMeshNeighbors)
        {
            ncErrorCode = nc_def_dim(fileID, "meshNeighbors", timePointState.maximumNumberOfMeshNeighbors, &meshNeighborsDimensionID);
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            {
                if (!(NC_NOERR == ncErrorCode))
                {
                    CkError("ERROR in FileManagerNetCDF::writeState: could not create dimension meshNeighbors.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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
                CkError("ERROR in FileManagerNetCDF::writeState: could not create dimension channelElements.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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
                    CkError("ERROR in FileManagerNetCDF::writeState: could not create dimension channelNeighbors.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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
                CkError("ERROR in FileManagerNetCDF::writeState: could not create data type EvapoTranspirationState.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
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
                CkError("ERROR in FileManagerNetCDF::writeState: could not create data type GroundwaterState.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    // Create variables.
    if (0 < timePointState.globalNumberOfMeshElements)
    {
        if (!error)
        {
            error = createVariable(fileID, "meshEvapoTranspirationState", EvapoTranspirationStateTypeID, 1, meshElementsDimensionID, 0, NULL, "Opaque blob of evapotranspiration state in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshSurfaceWater", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Surface water depth in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshSurfaceWaterCreated", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters",
                                   "Surface water created or destroyed in mesh elements as the result of unusual situations.  Positive means water was created.  Negative means water was destroyed.");
        }
        
        if (!error)
        {
            if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            {
                // Assumes GroundwaterModeEnum is 4 bytes when storing as NC_INT.
                CkAssert(4 == sizeof(GroundwaterModeEnum));
            }
            
            error = createVariable(fileID, "meshGroundwaterMode", NC_INT, 1, meshElementsDimensionID, 0, NULL, "Current mode of the groundwater simulation in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshPerchedHead", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Elevation above datum of perched water table in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshSoilWater", VadoseZoneStateTypeID, 1, meshElementsDimensionID, 0, NULL, "Opaque blob of vadose zone state in the soil layer of mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshSoilWaterCreated", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters",
                                   "Soil water created or destroyed in mesh elements as the result of unusual situations.  Positive means water was created.  Negative means water was destroyed.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshAquiferHead", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Elevation above datum of aquifer water table in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshAquiferWater", VadoseZoneStateTypeID, 1, meshElementsDimensionID, 0, NULL, "Opaque blob of vadose zone state in the aquifer layer of mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshAquiferWaterCreated", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters",
                                   "Aquifer water created or destroyed in mesh elements as the result of unusual situations.  Positive means water was created.  Negative means water was destroyed.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshDeepGroundwater", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Deep groundwater in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshPrecipitationRate", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters/second", "Instantaneous precipitation rate in mesh elements as a negative number.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshPrecipitationCumulative", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Cumulative precipitation in mesh elements as a negative number.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshEvaporationRate", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters/second",
                                   "Instantaneous evaporation or condensation rate in mesh elements.  Positive means water removed from the element.  Negative means water added to the element.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshEvaporationCumulative", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters",
                                   "Cumulative evaporation or condensation in mesh elements.  Positive means water removed from the element.  Negative means water added to the element.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshTranspirationRate", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters/second", "Instantaneous transpiration rate in mesh elements as a positive number.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshTranspirationCumulative", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Cumulative transpiration in mesh elements as a positive number.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshCanopyWater", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Water equivalent of total snow and liquid water in canopy in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshSnowWater", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Water equivalent of total snow and liquid water in snow pack in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshRootZoneWater", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Water in root zone in mesh elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "meshTotalGroundwater", NC_DOUBLE, 1, meshElementsDimensionID, 0, "meters", "Total groundwater in mesh elements.");
        }
        
        if (0 < timePointState.maximumNumberOfMeshNeighbors)
        {
            if (!error)
            {
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                {
                    // Assumes NeighborEndpointEnum is 4 bytes when storing as NC_INT.
                    CkAssert(4 == sizeof(NeighborEndpointEnum));
                }
                
                error = createVariable(fileID, "meshNeighborLocalEndpoint", NC_INT, 2, meshElementsDimensionID, meshNeighborsDimensionID, NULL, "How the connection is connected to this element.");
            }
            
            if (!error)
            {
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                {
                    // Assumes NeighborEndpointEnum is 4 bytes when storing as NC_INT.
                    CkAssert(4 == sizeof(NeighborEndpointEnum));
                }
                
                error = createVariable(fileID, "meshNeighborRemoteEndpoint", NC_INT, 2, meshElementsDimensionID, meshNeighborsDimensionID, NULL, "How the connection is connected to the remote neighbor.");
            }
            
            if (!error)
            {
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                {
                    // Assumes size_t is 8 bytes when storing as NC_UINT64.
                    CkAssert(8 == sizeof(size_t));
                }
                
                error = createVariable(fileID, "meshNeighborRemoteElementNumber", NC_UINT64, 2, meshElementsDimensionID, meshNeighborsDimensionID, NULL, "Element number of the remote neighbor.");
            }
            
            if (!error)
            {
                error = createVariable(fileID, "meshNeighborNominalFlowRate", NC_DOUBLE, 2, meshElementsDimensionID, meshNeighborsDimensionID, "meters^3/second",
                                       "Instantaneous flow rate between this element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
            }
            
            if (!error)
            {
                error = createVariable(fileID, "meshNeighborExpirationTime", NC_DOUBLE, 2, meshElementsDimensionID, meshNeighborsDimensionID, "seconds",
                                       "Value of currentTime when flow rate negotiated with neighbor expires.");
            }
            
            if (!error)
            {
                error = createVariable(fileID, "meshNeighborInflowCumulative", NC_DOUBLE, 2, meshElementsDimensionID, meshNeighborsDimensionID, "meters^3",
                                       "Cumulative flow into a mesh element from its neighbor as a negative number.");
            }
            
            if (!error)
            {
                error = createVariable(fileID, "meshNeighborOutflowCumulative", NC_DOUBLE, 2, meshElementsDimensionID, meshNeighborsDimensionID, "meters^3",
                                       "Cumulative flow out of a mesh element into its neighbor as a positive number.");
            }
        }
    }
    
    if (0 < timePointState.globalNumberOfChannelElements)
    {
        if (!error)
        {
            error = createVariable(fileID, "channelEvapoTranspirationState", EvapoTranspirationStateTypeID, 1, channelElementsDimensionID, 0, NULL, "Opaque blob of evapotranspiration state in channel elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "channelSurfaceWater", NC_DOUBLE, 1, channelElementsDimensionID, 0, "meters", "Surface water depth in channel elements.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "channelSurfaceWaterCreated", NC_DOUBLE, 1, channelElementsDimensionID, 0, "meters",
                                   "Surface water created or destroyed in channel elements as the result of unusual situations.  Positive means water was created.  Negative means water was destroyed.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "channelPrecipitationRate", NC_DOUBLE, 1, channelElementsDimensionID, 0, "meters/second", "Instantaneous precipitation rate in channel elements as a negative number.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "channelPrecipitationCumulative", NC_DOUBLE, 1, channelElementsDimensionID, 0, "meters", "Cumulative precipitation in channel elements as a negative number.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "channelEvaporationRate", NC_DOUBLE, 1, channelElementsDimensionID, 0, "meters/second",
                                   "Instantaneous evaporation or condensation rate in channel elements.  Positive means water removed from the element.  Negative means water added to the element.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "channelEvaporationCumulative", NC_DOUBLE, 1, channelElementsDimensionID, 0, "meters",
                                   "Cumulative evaporation or condensation in channel elements.  Positive means water removed from the element.  Negative means water added to the element.");
        }
        
        if (!error)
        {
            error = createVariable(fileID, "channelSnowWater", NC_DOUBLE, 1, channelElementsDimensionID, 0, "meters", "Water equivalent of total snow and liquid water in snow pack in channel elements.");
        }
        
        if (0 < timePointState.maximumNumberOfChannelNeighbors)
        {
            if (!error)
            {
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                {
                    // Assumes NeighborEndpointEnum is 4 bytes when storing as NC_INT.
                    CkAssert(4 == sizeof(NeighborEndpointEnum));
                }
                
                error = createVariable(fileID, "channelNeighborLocalEndpoint", NC_INT, 2, channelElementsDimensionID, channelNeighborsDimensionID, NULL, "How the connection is connected to this element.");
            }
            
            if (!error)
            {
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                {
                    // Assumes NeighborEndpointEnum is 4 bytes when storing as NC_INT.
                    CkAssert(4 == sizeof(NeighborEndpointEnum));
                }
                
                error = createVariable(fileID, "channelNeighborRemoteEndpoint", NC_INT, 2, channelElementsDimensionID, channelNeighborsDimensionID, NULL, "How the connection is connected to the remote neighbor.");
            }
            
            if (!error)
            {
                if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                {
                    // Assumes size_t is 8 bytes when storing as NC_UINT64.
                    CkAssert(8 == sizeof(size_t));
                }
                
                error = createVariable(fileID, "channelNeighborRemoteElementNumber", NC_UINT64, 2, channelElementsDimensionID, channelNeighborsDimensionID, NULL, "Element number of the remote neighbor.");
            }
            
            if (!error)
            {
                error = createVariable(fileID, "channelNeighborNominalFlowRate", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, "meters^3/second",
                                       "Instantaneous flow rate between this element and its neighbor.  Positive means flow out of the element into the neighbor.  Negative means flow into the element out of the neighbor.");
            }
            
            if (!error)
            {
                error = createVariable(fileID, "channelNeighborExpirationTime", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, "seconds",
                                       "Value of currentTime when flow rate negotiated with neighbor expires.");
            }
            
            if (!error)
            {
                error = createVariable(fileID, "channelNeighborInflowCumulative", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, "meters^3",
                                       "Cumulative flow into a channel element from its neighbor as a negative number.");
            }
            
            if (!error)
            {
                error = createVariable(fileID, "channelNeighborOutflowCumulative", NC_DOUBLE, 2, channelElementsDimensionID, channelNeighborsDimensionID, "meters^3",
                                       "Cumulative flow out of a channel element into its neighbor as a positive number.");
            }
        }
    }
    
    // Write variables.
    if (0 < timePointState.localNumberOfMeshElements)
    {
        if (!error)
        {
            error = writeVariable(fileID, "meshEvapoTranspirationState", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshEvapoTranspirationState);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshSurfaceWater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshSurfaceWater);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshSurfaceWaterCreated", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshSurfaceWaterCreated);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshGroundwaterMode", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshGroundwaterMode);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshPerchedHead", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshPerchedHead);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshSoilWater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshSoilWater);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshSoilWaterCreated", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshSoilWaterCreated);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshAquiferHead", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshAquiferHead);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshAquiferWater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshAquiferWater);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshAquiferWaterCreated", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshAquiferWaterCreated);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshDeepGroundwater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshDeepGroundwater);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshPrecipitationRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshPrecipitationRate);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshPrecipitationCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshPrecipitationCumulative);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshEvaporationRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshEvaporationRate);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshEvaporationCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshEvaporationCumulative);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshTranspirationRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshTranspirationRate);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshTranspirationCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshTranspirationCumulative);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshCanopyWater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshCanopyWater);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshSnowWater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshSnowWater);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshRootZoneWater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshRootZoneWater);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "meshTotalGroundwater", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, 0, timePointState.meshTotalGroundwater);
        }
        
        if (0 < timePointState.maximumNumberOfMeshNeighbors)
        {
            if (!error)
            {
                error = writeVariable(fileID, "meshNeighborLocalEndpoint", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors,
                                      timePointState.meshNeighborLocalEndpoint);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "meshNeighborRemoteEndpoint", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors,
                                      timePointState.meshNeighborRemoteEndpoint);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "meshNeighborRemoteElementNumber", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors,
                                      timePointState.meshNeighborRemoteElementNumber);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "meshNeighborNominalFlowRate", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors,
                                      timePointState.meshNeighborNominalFlowRate);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "meshNeighborExpirationTime", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors,
                                      timePointState.meshNeighborExpirationTime);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "meshNeighborInflowCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors,
                                      timePointState.meshNeighborInflowCumulative);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "meshNeighborOutflowCumulative", timePointState.localMeshElementStart, timePointState.localNumberOfMeshElements, timePointState.maximumNumberOfMeshNeighbors,
                                      timePointState.meshNeighborOutflowCumulative);
            }
        }
    }
    
    if (0 < timePointState.localNumberOfChannelElements)
    {
        if (!error)
        {
            error = writeVariable(fileID, "channelEvapoTranspirationState", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, timePointState.channelEvapoTranspirationState);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "channelSurfaceWater", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, timePointState.channelSurfaceWater);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "channelSurfaceWaterCreated", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, timePointState.channelSurfaceWaterCreated);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "channelPrecipitationRate", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, timePointState.channelPrecipitationRate);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "channelPrecipitationCumulative", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, timePointState.channelPrecipitationCumulative);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "channelEvaporationRate", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, timePointState.channelEvaporationRate);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "channelEvaporationCumulative", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, timePointState.channelEvaporationCumulative);
        }
        
        if (!error)
        {
            error = writeVariable(fileID, "channelSnowWater", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, 0, timePointState.channelSnowWater);
        }
        
        if (0 < timePointState.maximumNumberOfChannelNeighbors)
        {
            if (!error)
            {
                error = writeVariable(fileID, "channelNeighborLocalEndpoint", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors,
                                      timePointState.channelNeighborLocalEndpoint);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "channelNeighborRemoteEndpoint", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors,
                                      timePointState.channelNeighborRemoteEndpoint);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "channelNeighborRemoteElementNumber", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors,
                                      timePointState.channelNeighborRemoteElementNumber);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "channelNeighborNominalFlowRate", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors,
                                      timePointState.channelNeighborNominalFlowRate);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "channelNeighborExpirationTime", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors,
                                      timePointState.channelNeighborExpirationTime);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "channelNeighborInflowCumulative", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors,
                                      timePointState.channelNeighborInflowCumulative);
            }
            
            if (!error)
            {
                error = writeVariable(fileID, "channelNeighborOutflowCumulative", timePointState.localChannelElementStart, timePointState.localNumberOfChannelElements, timePointState.maximumNumberOfChannelNeighbors,
                                      timePointState.channelNeighborOutflowCumulative);
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
                CkError("ERROR in FileManagerNetCDF::writeState: could not close NetCDF file %s.  NetCDF error message: %s.\n", filename.str().c_str(), nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    return error;
}

bool FileManagerNetCDF::createVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID1, int dimensionID2, const char* units, const char* comment)
{
    bool error           = false;                        // Error flag.
    int  ncErrorCode;                                    // Return value of NetCDF functions.
    int  dimensionIDs[2] = {dimensionID1, dimensionID2}; // For passing dimension IDs.
    int  variableID;                                     // ID of the variable being created.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
        CkAssert(NULL != variableName && 1 <= numberOfDimensions && 2 >= numberOfDimensions);
    }
    
    // Create the variable.
    ncErrorCode = nc_def_var(fileID, variableName, dataType, numberOfDimensions, dimensionIDs, &variableID);
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    {
        if (!(NC_NOERR == ncErrorCode))
        {
            CkError("ERROR in FileManagerNetCDF::createVariable: could not create variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
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
                CkError("ERROR in FileManagerNetCDF::createVariable: could not create units attribute of variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
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
                CkError("ERROR in FileManagerNetCDF::createVariable: could not create comment attribute of variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    return error;
}

bool FileManagerNetCDF::writeVariable(int fileID, const char* variableName, size_t startDimension1, size_t countDimension1, size_t sizeDimension2, void* data)
{
    bool   error    = false;                             // Error flag.
    int    ncErrorCode;                                  // Return value of NetCDF functions.
    int    variableID;                                   // ID of variable being written.
    size_t start[2] = {startDimension1, 0};              // For specifying subarray to write.
    size_t count[2] = {countDimension1, sizeDimension2}; // For specifying subarray to write.
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
    {
        CkAssert(NULL != variableName && 0 < countDimension1 && NULL != data);
    }
    
    // Get the variable ID.
    ncErrorCode = nc_inq_varid(fileID, variableName, &variableID);
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    {
        if (!(NC_NOERR == ncErrorCode))
        {
            CkError("ERROR in FileManagerNetCDF::writeVariable: could not get ID of variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
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
                CkError("ERROR in FileManagerNetCDF::writeVariable: could not write variable %s.  NetCDF error message: %s.\n", variableName, nc_strerror(ncErrorCode));
                error = true;
            }
        }
    }
    
    return error;
}
