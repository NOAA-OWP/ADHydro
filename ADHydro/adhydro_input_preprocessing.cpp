#include "adhydro_input_preprocessing.h"
#include "all.h"
#include <netcdf.h>

ADHydroInputPreprocessing::ADHydroInputPreprocessing(CkArgMsg* msg)
{
  bool  error      = false; // Error flag.
  char* nameString = NULL;  // Temporary string for file names.
  int   nameStringSize;     // Size of buffer allocated for nameString.
  int   numPrinted;         // Used to check that snprintf printed the correct nubmer of characters.
  int   numScanned;         // Used to check that scanf scanned the correct nubmer of inputs.
  int   numberOfElements;   // The number of elements in the files.
  int   numberOfNodes;      // The number of nodes in the files.
  int   numberCheck;        // Used to check that the number of elements and nodes match between files.
  int   dimension;          // Used to check the dimensions in the files.
  int   boundary;           // Used to check the number of boundary markers in the files.
  
  nodeFile     = NULL;
  zFile        = NULL;
  elementFile  = NULL;
  neighborFile = NULL;
  
  // Print usage message if number of arguments is incorrect or first argument is any flag.
  if (!(3 == msg->argc && '-' != msg->argv[1][0]))
    {
      CkPrintf("\nUsage:\n\nadhydro_input_preprocessing <input file path and prefix> <output directory>\n\n");
      CkPrintf("E.g.:\n\nadhydro_input_preprocessing ../input/mesh.1 ../output\n\n");
      error = true;
    }
  
  // Allocate space for file name string.
  if (!error)
    {
      nameStringSize = strlen(msg->argv[1]) + strlen(".neigh") + 1; // Longest string concatenated to input file path and prefix is ".neigh".
      nameString     = new char[nameStringSize];
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s.node", msg->argv[1]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)(strlen(msg->argv[1]) + strlen(".node")) == (long long)numPrinted))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: incorrect return value of snprintf when generating node file name.  "
                  "%d should be %d.\n", numPrinted, strlen(msg->argv[1]) + strlen(".node"));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      nodeFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != nodeFile))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: unable to open node file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s.z", msg->argv[1]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)(strlen(msg->argv[1]) + strlen(".z")) == (long long)numPrinted))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: incorrect return value of snprintf when generating z file name.  "
                  "%d should be %d.\n", numPrinted, strlen(msg->argv[1]) + strlen(".z"));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      zFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != zFile))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: unable to open z file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s.ele", msg->argv[1]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)(strlen(msg->argv[1]) + strlen(".ele")) == (long long)numPrinted))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: incorrect return value of snprintf when generating element file name.  "
                  "%d should be %d.\n", numPrinted, strlen(msg->argv[1]) + strlen(".ele"));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      elementFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != elementFile))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: unable to open element file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Create file name.
  if (!error)
    {
      numPrinted = snprintf(nameString, nameStringSize, "%s.neigh", msg->argv[1]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      // Cast to long long to avoid comparison between signed and unsigned without losing range.
      if (!((long long)(strlen(msg->argv[1]) + strlen(".neigh")) == (long long)numPrinted))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: incorrect return value of snprintf when generating neighbor file name.  "
                  "%d should be %d.\n", numPrinted, strlen(msg->argv[1]) + strlen(".neigh"));
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Open file.
  if (!error)
    {
      neighborFile = fopen(nameString, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(NULL != neighborFile))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: unable to open neighbor file %s.\n", nameString);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(nodeFile, "%d %d %d %d", &numberOfNodes, &dimension, &numberOfAttributes, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: unable to read header from node file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfNodes && 2 == dimension && 0 == numberOfAttributes && 1 == boundary))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: invalid header in node file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(zFile, "%d %d", &numberCheck, &dimension);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: unable to read header from z file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(numberOfNodes == numberCheck && 1 == dimension))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: invalid header in z file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(elementFile, "%d %d %d", &numberOfElements, &dimension, &numberOfAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: unable to read header from element file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfElements && 3 == dimension && 0 <= numberOfAttributes))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: invalid header in element file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Read header.
  if (!error)
    {
      numScanned = fscanf(neighborFile, "%d %d", &numberCheck, &dimension);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(2 == numScanned))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: unable to read header from neighbor file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(numberOfElements == numberCheck && 3 == dimension))
        {
          CkError("ERROR in ADHydroInputPreprocessing::ADHydroInputPreprocessing: invalid header in neighbor file.\n");
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Create NetCDF files.
  if (!error)
    {
      fileManagerProxy = CProxy_FileManager::ckNew();

      fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydroInputPreprocessing, filesOpened), thisProxy));

      fileManagerProxy.openFiles(strlen(msg->argv[2]) + 1, msg->argv[2], numberOfElements, numberOfNodes, FILE_MANAGER_CREATE, 0, FILE_MANAGER_CREATE, 0, FILE_MANAGER_CREATE, 0);
    }
  
  // Delete command line argument message.
  delete msg;

  // Delete dynamically allocated string.
  if (NULL != nameString)
    {
      delete[] nameString;
    }
  
  if (error)
    {
      closeFiles();
      CkExit();
    }
}

// FIXME this function is a beast and should be refactored
void ADHydroInputPreprocessing::filesOpened()
{
  int          error = false;                                             // Error flag.
  int          ii, jj, kk;                                                // Loop counters.
  int          ncErrorCode;                                               // Return value of NetCDF functions.
  int          numScanned;                                                // Used to check that scanf scanned the correct nubmer of inputs.
  FileManager* fileManagerLocalBranch = fileManagerProxy.ckLocalBranch(); // For accessing public member variables.
  int          index;                                                     // For reading indices from files.
  int          firstIndex;                                                // Indices can be zero based or one based.  All must be the same.
  double       xCoordinate;                                               // X coordinate of node.
  double       yCoordinate;                                               // Y coordinate of node.
  double       zCoordinate;                                               // Z coordinate of node.
  size_t       netCDFIndex[2];                                            // Index for writing to NetCDF files.
  int          vertex0;                                                   // Node index of element vertex.
  int          vertex1;                                                   // Node index of element vertex.
  int          vertex2;                                                   // Node index of element vertex.
  double       vertex0X;                                                  // X coordinate of vertex.
  double       vertex0Y;                                                  // Y coordinate of vertex.
  double       vertex0ZSurface;                                           // Z surface coordinate of vertex.
  double       vertex0ZBedrock;                                           // Z bedrock coordinate of vertex.
  double       vertex1X;                                                  // X coordinate of vertex.
  double       vertex1Y;                                                  // Y coordinate of vertex.
  double       vertex1ZSurface;                                           // Z surface coordinate of vertex.
  double       vertex1ZBedrock;                                           // Z bedrock coordinate of vertex.
  double       vertex2X;                                                  // X coordinate of vertex.
  double       vertex2Y;                                                  // Y coordinate of vertex.
  double       vertex2ZSurface;                                           // Z surface coordinate of vertex.
  double       vertex2ZBedrock;                                           // Z bedrock coordinate of vertex.
  double       edgeLength0;                                               // Length of element edge.
  double       edgeLength1;                                               // Length of element edge.
  double       edgeLength2;                                               // Length of element edge.
  // FIXME this is only used for initializing groundwater head.  Do we want to do it this way long term?
  double       elementZSurface;                                           // Z surface coordinate of center of element.
  double       coordinate;                                                // For storing coordinates of edges and element.
  double       attribute;                                                 // Element attribute.
  int          catchment;                                                 // Element catchment.  The first attribute, if any, is assumed to be the catchment.
                                                                          // Otherwise, the catchment is set to zero.
  int          neighbor0;                                                 // Element index of element neighbor.
  int          neighbor1;                                                 // Element index of element neighbor.
  int          neighbor2;                                                 // Element index of element neighbor.
  bool         foundIt;                                                   // Flag to indicate element found in neighbor list.
  
  for (ii = 0; !error && ii < fileManagerLocalBranch->numberOfMeshNodes; ii++)
    {
      // Read node file.
      numScanned = fscanf(nodeFile, "%d %lf %lf %*d", &index, &xCoordinate, &yCoordinate);
      
      // Set zero based or one based indices.
      if (0 == ii)
        {
          if (0 == index)
            {
              firstIndex = 0;
            }
          else
            {
              firstIndex = 1;
            }
        }

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read entry %d from node file.\n", ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii + firstIndex == index))
        {
          CkError("ERROR in ADHydroInputPreprocessing::filesOpened: invalid node number in node file.  %d should be %d.\n", index, ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Fill in data.
      if (!error)
        {
          // The netCDF file is zero based.  Use mod rather than minus firstIndex so that only one node gets renumbered, the last one.
          netCDFIndex[0] = index % fileManagerLocalBranch->numberOfMeshNodes;
          
          netCDFIndex[1] = 0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &xCoordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 1;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &yCoordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Read z file.
      if (!error)
        {
          numScanned = fscanf(zFile, "%d %lf", &index, &zCoordinate);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(2 == numScanned))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read entry %d from z file.\n", ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(ii + firstIndex == index))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: invalid node number in z file.  %d should be %d.\n", index, ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      // Fill in data.
      if (!error)
        {
          // The netCDF file is zero based.  Use mod rather than minus firstIndex so that only one node gets renumbered, the last one.
          netCDFIndex[0] = index % fileManagerLocalBranch->numberOfMeshNodes;
          
          netCDFIndex[1] = 2;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &zCoordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          zCoordinate -= 1.0; // FIXME figure out real way to get soil depth.
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeZBedrockCoordinatesVarID, netCDFIndex,
                                           &zCoordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshNodeZBedrockCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  for (ii = 0; !error && ii < fileManagerLocalBranch->numberOfMeshElements; ii++)
    {
      // Read element file.
      numScanned = fscanf(elementFile, "%d %d %d %d", &index, &vertex0, &vertex1, &vertex2);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read entry %d from element file.\n", ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii + firstIndex == index))
        {
          CkError("ERROR in ADHydroInputPreprocessing::filesOpened: invalid element number in element file.  %d should be %d.\n", index, ii + firstIndex);
          error = true;
        }
      
      if (!(firstIndex <= vertex0 && fileManagerLocalBranch->numberOfMeshNodes + firstIndex > vertex0))
        {
          CkError("ERROR in ADHydroInputPreprocessing::filesOpened: vertex index %d out of range in entry %d in element file.\n", vertex0, ii + firstIndex);
          error = true;
        }
      
      if (!(firstIndex <= vertex1 && fileManagerLocalBranch->numberOfMeshNodes + firstIndex > vertex1))
        {
          CkError("ERROR in ADHydroInputPreprocessing::filesOpened: vertex index %d out of range in entry %d in element file.\n", vertex1, ii + firstIndex);
          error = true;
        }
      
      if (!(firstIndex <= vertex2 && fileManagerLocalBranch->numberOfMeshNodes + firstIndex > vertex2))
        {
          CkError("ERROR in ADHydroInputPreprocessing::filesOpened: vertex index %d out of range in entry %d in element file.\n", vertex2, ii + firstIndex);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Read vertex geometry.
      if (!error)
        {
          // Correct vertex index to zero based.
          vertex0 %= fileManagerLocalBranch->numberOfMeshNodes;
          
          netCDFIndex[0] = vertex0;
          
          netCDFIndex[1] = 0;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex0X);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 1;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex0Y);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 2;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex0ZSurface);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeZBedrockCoordinatesVarID, netCDFIndex,
                                           &vertex0ZBedrock);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeZBedrockCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          // Correct vertex index to zero based.
          vertex1 %= fileManagerLocalBranch->numberOfMeshNodes;
          
          netCDFIndex[0] = vertex1;
          
          netCDFIndex[1] = 0;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex1X);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 1;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex1Y);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 2;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex1ZSurface);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeZBedrockCoordinatesVarID, netCDFIndex,
                                           &vertex1ZBedrock);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeZBedrockCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          // Correct vertex index to zero based.
          vertex2 %= fileManagerLocalBranch->numberOfMeshNodes;
          
          netCDFIndex[0] = vertex2;
          
          netCDFIndex[1] = 0;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex2X);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 1;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex2Y);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 2;
          
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeXYZSurfaceCoordinatesVarID, netCDFIndex,
                                           &vertex2ZSurface);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeXYZSurfaceCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          ncErrorCode = nc_get_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNodeZBedrockCoordinatesVarID, netCDFIndex,
                                           &vertex2ZBedrock);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNodeZBedrockCoordinates in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Fill in data.
      if (!error)
        {
          // The netCDF file is zero based.  Use mod rather than minus firstIndex so that only one node gets renumbered, the last one.
          netCDFIndex[0] = index % fileManagerLocalBranch->numberOfMeshElements;
          
          netCDFIndex[1] = 0;
          
          ncErrorCode = nc_put_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshElementNodeIndicesVarID, netCDFIndex, &vertex0);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementNodeIndices in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 1;
          
          ncErrorCode = nc_put_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshElementNodeIndicesVarID, netCDFIndex, &vertex1);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementNodeIndices in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 2;
          
          ncErrorCode = nc_put_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshElementNodeIndicesVarID, netCDFIndex, &vertex2);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementNodeIndices in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate length of edge 0.  Edge 0 goes from vertex 1 to 2 (opposite vertex 0).
      if (!error)
        {
          edgeLength0 = sqrt((vertex1X - vertex2X) * (vertex1X - vertex2X) + (vertex1Y - vertex2Y) * (vertex1Y - vertex2Y));
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 < edgeLength0))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: edge length must be greater than zero for entry %d of element file.\n",
                      ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeLengthVarID, netCDFIndex, &edgeLength0);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeLength in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate length of edge 1.  Edge 1 goes from vertex 2 to 0 (opposite vertex 1).
      if (!error)
        {
          edgeLength1 = sqrt((vertex2X - vertex0X) * (vertex2X - vertex0X) + (vertex2Y - vertex0Y) * (vertex2Y - vertex0Y));
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 < edgeLength1))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: edge length must be greater than zero for entry %d of element file.\n",
                      ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 1;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeLengthVarID, netCDFIndex, &edgeLength1);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeLength in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate length of edge 2.  Edge 2 goes from vertex 0 to 1 (opposite vertex 2).
      if (!error)
        {
          edgeLength2 = sqrt((vertex0X - vertex1X) * (vertex0X - vertex1X) + (vertex0Y - vertex1Y) * (vertex0Y - vertex1Y));
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 < edgeLength2))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: edge length must be greater than zero for entry %d of element file.\n",
                      ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 2;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeLengthVarID, netCDFIndex, &edgeLength2);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeLength in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate X component of unit normal vector of edge 0.  Edge 0 goes from vertex 1 to 2 (opposite vertex 0).
      if (!error)
        {
          coordinate = (vertex2Y - vertex1Y) / edgeLength0;
          
          netCDFIndex[1] = 0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeNormalXVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeNormalX in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate Y component of unit normal vector of edge 0.  Edge 0 goes from vertex 1 to 2 (opposite vertex 0).
      if (!error)
        {
          coordinate = (vertex1X - vertex2X) / edgeLength0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeNormalYVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeNormalY in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate X component of unit normal vector of edge 1.  Edge 1 goes from vertex 2 to 0 (opposite vertex 1).
      if (!error)
        {
          coordinate = (vertex0Y - vertex2Y) / edgeLength1;
          
          netCDFIndex[1] = 1;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeNormalXVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeNormalX in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate Y component of unit normal vector of edge 1.  Edge 1 goes from vertex 2 to 0 (opposite vertex 1).
      if (!error)
        {
          coordinate = (vertex2X - vertex0X) / edgeLength1;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeNormalYVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeNormalY in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate X component of unit normal vector of edge 2.  Edge 2 goes from vertex 0 to 1 (opposite vertex 2).
      if (!error)
        {
          coordinate = (vertex1Y - vertex0Y) / edgeLength2;
          
          netCDFIndex[1] = 2;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeNormalXVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeNormalX in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate Y component of unit normal vector of edge 2.  Edge 2 goes from vertex 0 to 1 (opposite vertex 2).
      if (!error)
        {
          coordinate = (vertex0X - vertex1X) / edgeLength2;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshEdgeNormalYVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeNormalY in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate X coordinate of center of element.
      if (!error)
        {
          coordinate = (vertex0X + vertex1X + vertex2X) / 3.0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshElementXVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementX in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate Y coordinate of center of element.
      if (!error)
        {
          coordinate = (vertex0Y + vertex1Y + vertex2Y) / 3.0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshElementYVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementY in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate Z surface coordinate of center of element.
      if (!error)
        {
          coordinate = (vertex0ZSurface + vertex1ZSurface + vertex2ZSurface) / 3.0;
          
          // FIXME this is only used for initializing groundwater head.  Do we want to do it this way long term?
          elementZSurface = coordinate;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshElementZSurfaceVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementZSurface in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate Z bedrock coordinate of center of element.
      if (!error)
        {
          coordinate = (vertex0ZBedrock + vertex1ZBedrock + vertex2ZBedrock) / 3.0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshElementZBedrockVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementZBedrock in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Calculate area of element.
      if (!error)
        {
          coordinate = (vertex0X * (vertex1Y - vertex2Y) + vertex1X * (vertex2Y - vertex0Y) + vertex2X * (vertex0Y - vertex1Y)) * 0.5;
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(0 < coordinate))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: element area must be greater than zero for entry %d of element file.\n",
                      ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      if (!error)
        {
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshElementAreaVarID, netCDFIndex, &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementArea in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      // Catchment defaults to zero if there are no attributes.
      catchment = 0;
      
      // Read element attributes.
      for (jj = 0; !error && jj < numberOfAttributes; jj++)
        {
          numScanned = fscanf(elementFile, "%lf", &attribute);

    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(1 == numScanned))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read entry %d from element file.\n", ii + firstIndex);
              error = true;
            }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          
          if (!error && 0 == jj)
            {
              catchment = attribute;
            }
        }
      
      // Fill in data.
      if (!error)
        {
          ncErrorCode = nc_put_var1_int(fileManagerLocalBranch->parameterGroupID, fileManagerLocalBranch->meshElementCatchmentVarID, netCDFIndex, &catchment);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementCatchment in NetCDF parameter file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          coordinate = 5.55e-4; // FIXME get real conductivity
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->parameterGroupID, fileManagerLocalBranch->meshElementConductivityVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementConductivity in NetCDF parameter file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          coordinate = 0.5; // FIXME get real porosity
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->parameterGroupID, fileManagerLocalBranch->meshElementPorosityVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementPorosity in NetCDF parameter file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          coordinate = 0.038; // FIXME get real Manning's n
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->parameterGroupID, fileManagerLocalBranch->meshElementManningsNVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementManningsN in NetCDF parameter file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          coordinate = 0.01; // 1 cm of surfacewater.
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshElementSurfacewaterDepthVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementSurfacewaterDepth in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          coordinate = 0.0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshElementSurfacewaterErrorVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementSurfacewaterError in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          coordinate = elementZSurface;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshElementGroundwaterHeadVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementGroundwaterhead in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          coordinate = 0.0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshElementGroundwaterErrorVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementGroundwaterError in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          coordinate = 0.0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshElementGroundwaterRechargeVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshElementGroundwaterRecharge in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      for (jj = 0; !error && jj < 3; jj++)
        {
          netCDFIndex[1] = jj;
          
          coordinate = 0.0;
          
          ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshEdgeSurfacewaterFlowRateVarID, netCDFIndex,
                                           &coordinate);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeSurfacewaterFlowRate in NetCDF state file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          
          if (!error)
            {
              ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshEdgeSurfacewaterCumulativeFlowVarID,
                                               netCDFIndex, &coordinate);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeSurfacewaterCumulativeFlow in NetCDF state "
                          "file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
          
          if (!error)
            {
              ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshEdgeGroundwaterFlowRateVarID,
                                               netCDFIndex, &coordinate);
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeGroundwaterFlowRate in NetCDF state file.  "
                          "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                  error = true;
                }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
          
          if (!error)
            {
              ncErrorCode = nc_put_var1_double(fileManagerLocalBranch->stateGroupID, fileManagerLocalBranch->meshEdgeGroundwaterCumulativeFlowVarID,
                                               netCDFIndex, &coordinate);
              
    #if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
              if (!(NC_NOERR == ncErrorCode))
                {
                  CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshEdgeGroundwaterCumulativeFlow in NetCDF state "
                          "file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                  error = true;
                }
    #endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            }
        }
      
      // Read neighbor file.
      if (!error)
        {
          numScanned = fscanf(neighborFile, "%d %d %d %d", &index, &neighbor0, &neighbor1, &neighbor2);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(4 == numScanned))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read entry %d from neighbor file.\n", ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
          if (!(ii + firstIndex == index))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: invalid element number in neighbor file.  %d should be %d.\n", index, ii + firstIndex);
              error = true;
            }
          
          if (!(isBoundary(neighbor0) || (firstIndex <= neighbor0 && fileManagerLocalBranch->numberOfMeshElements + firstIndex > neighbor0)))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: neighbor index %d out of range in entry %d in neighbor file.\n", neighbor0,
                      ii + firstIndex);
              error = true;
            }
          
          if (!(isBoundary(neighbor1) || (firstIndex <= neighbor1 && fileManagerLocalBranch->numberOfMeshElements + firstIndex > neighbor1)))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: neighbor index %d out of range in entry %d in neighbor file.\n", neighbor1,
                      ii + firstIndex);
              error = true;
            }
          
          if (!(isBoundary(neighbor2) || (firstIndex <= neighbor2 && fileManagerLocalBranch->numberOfMeshElements + firstIndex > neighbor2)))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: neighbor index %d out of range in entry %d in neighbor file.\n", neighbor2,
                      ii + firstIndex);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        }
      
      // Fill in data.
      if (!error)
        {
          // The netCDF file is zero based.  Use mod rather than minus firstIndex so that only one node gets renumbered, the last one.
          netCDFIndex[0] = index % fileManagerLocalBranch->numberOfMeshElements;
          
          netCDFIndex[1] = 0;
          
          // Correct neighbor index to zero based.
          if (!isBoundary(neighbor0))
            {
              neighbor0 %= fileManagerLocalBranch->numberOfMeshElements;
            }
          
          ncErrorCode = nc_put_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNeighborIndicesVarID, netCDFIndex, &neighbor0);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshNeighborIndices in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 1;
          
          // Correct neighbor index to zero based.
          if (!isBoundary(neighbor1))
            {
              neighbor1 %= fileManagerLocalBranch->numberOfMeshElements;
            }
          
          ncErrorCode = nc_put_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNeighborIndicesVarID, netCDFIndex, &neighbor1);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshNeighborIndices in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
      
      if (!error)
        {
          netCDFIndex[1] = 2;
          
          // Correct neighbor index to zero based.
          if (!isBoundary(neighbor2))
            {
              neighbor2 %= fileManagerLocalBranch->numberOfMeshElements;
            }
          
          ncErrorCode = nc_put_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNeighborIndicesVarID, netCDFIndex, &neighbor2);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshNeighborIndices in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }
  
  // Fill in neighbor reciprocal edges.
  for (ii = 0; !error && ii < fileManagerLocalBranch->numberOfMeshElements; ii++)
    {
      for (jj = 0; !error && jj < 3; jj++)
        {
          netCDFIndex[0] = ii;
          netCDFIndex[1] = jj;
          
          ncErrorCode = nc_get_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNeighborIndicesVarID, netCDFIndex, &neighbor0);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (!(NC_NOERR == ncErrorCode))
            {
              CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNeighborIndices in NetCDF geometry file.  "
                      "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          
          // Search for me in my neighbor's neighbor list.
          if (!error && !isBoundary(neighbor0))
            {
              foundIt        = false;
              netCDFIndex[0] = neighbor0;

              for (kk = 0; !error && !foundIt && kk < 3; kk++)
                {
                  netCDFIndex[1] = kk;

                  ncErrorCode = nc_get_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNeighborIndicesVarID, netCDFIndex,
                                                &neighbor1);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                  if (!(NC_NOERR == ncErrorCode))
                    {
                      CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to read variable meshNeighborIndices in NetCDF geometry file.  "
                              "NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                      error = true;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

                  // If my neighbor's kkth neighbor is me then neighbor reciprocal edge is kk.
                  if (!error && neighbor1 == ii)
                    {
                      foundIt        = true;
                      netCDFIndex[0] = ii;
                      netCDFIndex[1] = jj;

                      ncErrorCode = nc_put_var1_int(fileManagerLocalBranch->geometryGroupID, fileManagerLocalBranch->meshNeighborReciprocalEdgeVarID,
                                                    netCDFIndex, &kk);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                      if (!(NC_NOERR == ncErrorCode))
                        {
                          CkError("ERROR in ADHydroInputPreprocessing::filesOpened: unable to write to variable meshNeighborReciprocalEdge in NetCDF geometry "
                                  "file.  NetCDF error message: %s.\n", nc_strerror(ncErrorCode));
                          error = true;
                        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                    }
                }

              if (!error && !foundIt)
                {
                  CkError("ERROR in ADHydroInputPreprocessing::filesOpened: element %d is not in the neighbor list of its neighbor %d.\n", ii, neighbor0);
                  error = true;
                }
            }
        }
    }
  
  // Even if there is an error attempt to close all files.  We can't CkExit here because we need to wait for the fileManagerProxy.closeFiles message to finish.
  closeFiles();
  
  fileManagerProxy.ckSetReductionClient(new CkCallback(CkReductionTarget(ADHydroInputPreprocessing, filesClosed), thisProxy));

  fileManagerProxy.closeFiles();
}

void ADHydroInputPreprocessing::filesClosed()
{
  CkExit();
}

bool ADHydroInputPreprocessing::closeFiles()
{
  bool error = false; // Error flag.
  int  errorCode;     // Return value of file functions.
  
  // Close all open files.
  if (NULL != nodeFile)
    {
      errorCode = fclose(nodeFile);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 == errorCode))
        {
          CkError("ERROR in ADHydroInputPreprocessing::closeFiles: unable to close node file.  Error code: %d.\n", errorCode);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (NULL != zFile)
    {
      errorCode = fclose(zFile);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 == errorCode))
        {
          CkError("ERROR in ADHydroInputPreprocessing::closeFiles: unable to close z file.  Error code: %d.\n", errorCode);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (NULL != elementFile)
    {
      errorCode = fclose(elementFile);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 == errorCode))
        {
          CkError("ERROR in ADHydroInputPreprocessing::closeFiles: unable to close element file.  Error code: %d.\n", errorCode);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (NULL != neighborFile)
    {
      errorCode = fclose(neighborFile);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(0 == errorCode))
        {
          CkError("ERROR in ADHydroInputPreprocessing::closeFiles: unable to close neighbor file.  Error code: %d.\n", errorCode);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  return error;
}

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "adhydro_input_preprocessing.def.h"
#pragma GCC diagnostic warning "-Wunused-variable"
