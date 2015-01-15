#include "all.h"

int main(void)
{
  bool        error               = false; // Error flag.
  int         ii, jj;                      // Loop counters;
  const char* meshNodeFilename    = "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.node";
  const char* meshElementFilename = "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.ele";
  FILE*       nodeFile;                    // Input file for mesh nodes.
  FILE*       eleFile;                     // Input file for mesh elements.
  int         numScanned;                  // Used to check that fscanf scanned all of the requested values.
  int         numberOfMeshNodes;           // Size of and number of mesh nodes actually existing in meshNodesX and meshNodesY arrays.
  int         numberOfMeshElements;        // Number of Mesh elements.
  int         dimension;                   // Used to check that the dimension of the mesh element file is 3.
  int         numberOfAttributes;          // Used to check that the number of attributes in the mesh element file is 1.
  int         boundary;                    // For reading the boundary code of a mesh edge.
  double*     meshNodesX          = NULL;  // X coordinates of mesh nodes.
  double*     meshNodesY          = NULL;  // Y coordinates of mesh nodes.
  int         detectedIndex;
  double*     detectedX;
  double*     detectedY;
  int         meshNodeNumber;              // For reading the node number of a mesh node.
  int         meshElementNumber;           // For reading the element number of a mesh element.
  double      xCoordinate;                 // For reading X coordinates of mesh nodes.
  double      yCoordinate;                 // For reading Y coordinates of mesh nodes.
  int         meshVertex[3];               // For reading the vertices of a mesh element.
  int         meshCatchment;               // For reading the catchment of a mesh element.
  double      area;
  bool        tooClose;
  
  // Open the files.
  nodeFile = fopen(meshNodeFilename, "r");
  eleFile  = fopen(meshElementFilename, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != nodeFile))
    {
      fprintf(stderr, "ERROR in main: Could not open mesh node file %s.\n", meshNodeFilename);
      error = true;
    }
  
  if (!(NULL != eleFile))
    {
      fprintf(stderr, "ERROR in main: Could not open mesh element file %s.\n", meshElementFilename);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Read the number of nodes from the file.
  if (!error)
    {
      numScanned = fscanf(nodeFile, "%d %d %d %d", &numberOfMeshNodes, &dimension, &numberOfAttributes, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(4 == numScanned))
        {
          fprintf(stderr, "ERROR in main: Unable to read header from mesh node file %s.\n", meshNodeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfMeshNodes && 2 == dimension && 0 == numberOfAttributes && 1 == boundary))
        {
          fprintf(stderr, "ERROR in main: Invalid header in mesh node file %s.\n", meshNodeFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }
  
  // Allocate arrays.
  if (!error)
    {
      meshNodesX = new double[numberOfMeshNodes];
      meshNodesY = new double[numberOfMeshNodes];
    }

  for (ii = 0; !error && ii < numberOfMeshNodes; ii++)
    {
      // Read node file.
      numScanned = fscanf(nodeFile, "%d %lf %lf %*d", &meshNodeNumber, &xCoordinate, &yCoordinate);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          fprintf(stderr, "ERROR in main: Unable to read entry %d from mesh node file.\n", ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == meshNodeNumber))
        {
          fprintf(stderr, "ERROR in main: Invalid node number in mesh node file.  %d should be %d.\n", meshNodeNumber, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      if (!error)
        {
          meshNodesX[meshNodeNumber] = xCoordinate;
          meshNodesY[meshNodeNumber] = yCoordinate;
        }
    }
  
  // Read the number of mesh elements from the file.
  if (!error)
    {
      numScanned = fscanf(eleFile, "%d %d %d", &numberOfMeshElements, &dimension, &numberOfAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          fprintf(stderr, "ERROR in main: Unable to read header from mesh element file %s.\n", meshElementFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfMeshElements && 3 == dimension && 1 == numberOfAttributes))
        {
          fprintf(stderr, "ERROR in main: Invalid header in mesh element file %s.\n", meshElementFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Allocate arrays.
  if (!error)
    {
      detectedIndex = 0;
      detectedX     = new double[numberOfMeshElements];
      detectedY     = new double[numberOfMeshElements];
    }
  
  // Read the elements.
  for (ii = 0; !error && ii < numberOfMeshElements; ii++)
    {
      numScanned = fscanf(eleFile, "%d %d %d %d %d", &meshElementNumber, &meshVertex[0], &meshVertex[1], &meshVertex[2], &meshCatchment);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(5 == numScanned))
        {
          fprintf(stderr, "ERROR in main: Unable to read entry %d from mesh element file %s.\n", ii, meshElementFilename);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == meshElementNumber))
        {
          fprintf(stderr, "ERROR in main: Invalid element number in mesh element file %s.  %d should be %d.\n", meshElementFilename,
                  meshElementNumber, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Check the element.
      if (!error)
        {
          area = 0.0;

          for (jj = 0; jj < 3; jj++)
            {
              area += meshNodesX[meshVertex[jj]] * (meshNodesY[meshVertex[(jj + 1) % 3]] - meshNodesY[meshVertex[(jj + 2) % 3]]);
            }

          area *= 0.5;
          
          if (0 == meshCatchment || 7500.0 > area)
            {
              // This is a problem triangle.  Report it if there hasn't already been one reported within 100 meters.
              tooClose = false;
              
              for (jj = 0; !tooClose && jj < detectedIndex; jj++)
                {
                  if (100.0 > sqrt((meshNodesX[meshVertex[0]] - detectedX[jj]) * (meshNodesX[meshVertex[0]] - detectedX[jj]) +
                                   (meshNodesY[meshVertex[0]] - detectedY[jj]) * (meshNodesY[meshVertex[0]] - detectedY[jj])))
                    {
                      tooClose = true;
                    }
                }
              
              if (!tooClose)
                {
                  printf("Found problem triangle at (%lf, %lf), catchment = %d, area = %lf.\n", meshNodesX[meshVertex[0]], meshNodesY[meshVertex[0]],
                         meshCatchment, area);
                  
                  detectedX[detectedIndex] = meshNodesX[meshVertex[0]];
                  detectedY[detectedIndex] = meshNodesY[meshVertex[0]];
                  
                  detectedIndex++;
                }
            }
        }
    }

  // Close the files.
  if (NULL != nodeFile)
    {
      fclose(nodeFile);
      nodeFile = NULL;
    }

  if (NULL != eleFile)
    {
      fclose(eleFile);
      eleFile = NULL;
    }
  
  // Deallocate the arrays.
  if (NULL != meshNodesX)
    {
      delete[] meshNodesX;
    }
  
  if (NULL != meshNodesY)
    {
      delete[] meshNodesY;
    }
  
  if (NULL != detectedX)
    {
      delete[] detectedX;
    }
  
  if (NULL != detectedY)
    {
      delete[] detectedY;
    }
  
  return 0;
}
