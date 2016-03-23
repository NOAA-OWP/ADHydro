#include "all.h"
#include <shapefil.h>
#include <netcdf.h>
#include <assert.h>

bool readNetCDFDimensionSize(int fileID, const char* dimensionName, size_t* dimensionSize)
{
  bool error = false; // Error flag.
  int  ncErrorCode;   // Return value of NetCDF functions.
  int  dimensionID;   // ID of dimension in NetCDF file.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  assert(NULL != dimensionName && NULL != dimensionSize);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  
  // Get the dimension ID.
  ncErrorCode = nc_inq_dimid(fileID, dimensionName, &dimensionID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NC_NOERR == ncErrorCode))
    {
      fprintf(stderr, "ERROR in FileManager::readNetCDFDimensionSize: unable to get dimension %s in NetCDF file.  NetCDF error message: %s.\n", dimensionName,
              nc_strerror(ncErrorCode));
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  
  // Get the dimension length.
  if (!error)
    {
      ncErrorCode = nc_inq_dimlen(fileID, dimensionID, dimensionSize);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          fprintf(stderr, "ERROR in FileManager::readNetCDFDimensionSize: unable to get length of dimension %s in NetCDF file.  NetCDF error message: %s.\n",
                  dimensionName, nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

int main(void)
{
  const char* geometryFilename = "/share/CI-WATER_Simulation_Data/small_green_mesh/geometry.nc";
  const char* shpFileBasename  = "/share/CI-WATER_Simulation_Data/small_green_mesh/channel_elements";

  bool       error            = false; // Error flag.
  size_t     ii;                       // Loop counter.
  int        geometryFileID;
  int        ncErrorCode;              // Return value of NetCDF functions.
  bool       shpErrorCode;             // Return value of shapelib functions.
  bool       geometryFileOpen = false;
  size_t     instancesSize;
  size_t     channelElementsSize;
  size_t     channelVerticesSize;
  int        channelVertexXVarID;
  int        channelVertexYVarID;
  int        channelVertexZVarID;
  double*    channelVertexX   = NULL;
  double*    channelVertexY   = NULL;
  double*    channelVertexZ   = NULL;
  size_t     start[NC_MAX_VAR_DIMS];   // For specifying subarrays when reading from NetCDF file.
  size_t     count[NC_MAX_VAR_DIMS];   // For specifying subarrays when reading from NetCDF file.
  size_t     numberOfUniqueVertices;
  SHPHandle  shpFile          = NULL;
  DBFHandle  dbfFile          = NULL;
  SHPObject* shape            = NULL;
  int        elementNumberField;
  size_t     newShapeNumber;

  ncErrorCode = nc_open(geometryFilename, NC_NOWRITE, &geometryFileID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NC_NOERR == ncErrorCode))
    {
      fprintf(stderr, "ERROR in main: unable to open NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  if (!error)
    {
      geometryFileOpen = true;
      
      error = readNetCDFDimensionSize(geometryFileID, "instances", &instancesSize);
    }
  
  if (!error)
    {
      error = readNetCDFDimensionSize(geometryFileID, "channelElements", &channelElementsSize);
    }
  
  if (!error)
    {
      error = readNetCDFDimensionSize(geometryFileID, "channelVerticesSize", &channelVerticesSize);
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(geometryFileID, "channelVertexX", &channelVertexXVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          fprintf(stderr, "ERROR in main: unable to get variable ID of channelVertexX.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(geometryFileID, "channelVertexY", &channelVertexYVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          fprintf(stderr, "ERROR in main: unable to get variable ID of channelVertexY.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      ncErrorCode = nc_inq_varid(geometryFileID, "channelVertexZBank", &channelVertexZVarID);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          fprintf(stderr, "ERROR in main: unable to get variable ID of channelVertexZBank.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      printf("instances = %lu, elements = %lu, vertices = %lu.\n", instancesSize, channelElementsSize, channelVerticesSize);
      
      channelVertexX = new double[channelVerticesSize];
      channelVertexY = new double[channelVerticesSize];
      channelVertexZ = new double[channelVerticesSize];
      
      start[0] = instancesSize - 1;
      start[1] = 0;
      start[2] = 0;
      count[0] = 1;
      count[1] = 1;
      count[2] = channelVerticesSize;
      
      shpFile = SHPCreate(shpFileBasename, SHPT_ARCZ);
      dbfFile = DBFCreate(shpFileBasename);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != shpFile || NULL != dbfFile))
        {
          fprintf(stderr, "ERROR in main: Could not create shp file %s.\n", shpFileBasename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      // In the DBF file create a field called elementNumber of type FTInteger.  Field width is 10 digits with 0 digits after the decimal point
      elementNumberField = DBFAddField(dbfFile, "elementNumber", FTInteger, 10, 0);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (-1 == elementNumberField)
        {
          fprintf(stderr, "ERROR in main: Could not create elementNumber attribute for shp file %s.\n", shpFileBasename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }

  for (ii = 0; !error && ii < channelElementsSize; ++ii)
    {
      if (0 == ii % 10000)
        {
          printf("Getting vertices of channel element %lu.\n", ii);
        }
      
      start[1] = ii;

      ncErrorCode = nc_get_vara_double(geometryFileID, channelVertexXVarID, start, count, channelVertexX);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          fprintf(stderr, "ERROR in main: unable to read variable channelVertexX.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

      if (!error)
        {
          ncErrorCode = nc_get_vara_double(geometryFileID, channelVertexYVarID, start, count, channelVertexY);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              fprintf(stderr, "ERROR in main: unable to read variable channelVertexY.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }

      if (!error)
        {
          ncErrorCode = nc_get_vara_double(geometryFileID, channelVertexZVarID, start, count, channelVertexZ);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              fprintf(stderr, "ERROR in main: unable to read variable channelVertexZBank.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          numberOfUniqueVertices = channelVerticesSize;
          
          while (2 < numberOfUniqueVertices &&
                 channelVertexX[numberOfUniqueVertices - 1] == channelVertexX[numberOfUniqueVertices - 2] && 
                 channelVertexY[numberOfUniqueVertices - 1] == channelVertexY[numberOfUniqueVertices - 2] && 
                 channelVertexZ[numberOfUniqueVertices - 1] == channelVertexZ[numberOfUniqueVertices - 2])
            {
              --numberOfUniqueVertices;
            }
          
          shape = SHPCreateSimpleObject(SHPT_ARCZ, numberOfUniqueVertices, channelVertexX, channelVertexY, channelVertexZ);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NULL != shape))
            {
              fprintf(stderr, "ERROR in main: Could not create shape for channel element %lu.\n", ii);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          newShapeNumber = SHPWriteObject(shpFile, -1, shape);
          
          SHPDestroyObject(shape);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(ii == newShapeNumber))
            {
              fprintf(stderr, "ERROR in main: Wrong shape number. %ld should be %lu.\n", newShapeNumber, ii);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          shpErrorCode = DBFWriteIntegerAttribute(dbfFile, newShapeNumber, elementNumberField, ii);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if(!shpErrorCode)
          {
            fprintf(stderr, "ERROR in main: Failed to write attribute or truncated attribute for shape %ld.\n", newShapeNumber);
            error = true;
          }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  if (NULL != shpFile)
    {
      SHPClose(shpFile);
    }
  
  if (NULL != dbfFile)
    {
      DBFClose(dbfFile);
    }

  delete[] channelVertexX;
  delete[] channelVertexY;
  delete[] channelVertexZ;
  
  // Close the geometry file.
  if (geometryFileOpen)
    {
      ncErrorCode      = nc_close(geometryFileID);
      geometryFileOpen = false;

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NC_NOERR == ncErrorCode))
        {
          fprintf(stderr, "ERROR in main: unable to close NetCDF geometry file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}
