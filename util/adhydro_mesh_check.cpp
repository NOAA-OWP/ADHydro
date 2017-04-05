#include "all.h"
#include <cstdio>
#include <string>

int main(int argc, char** argv)
{
  // Locations of input files.  Edit these and recompile to change which files get read.
  // FIXME make these command line parameters.
  std::string meshNodeFilename;
  std::string meshElementFilename;
  if(argc == 2)
  {
      meshNodeFilename = std::string(argv[1])+std::string("/mesh.1.node");
      meshElementFilename = std::string(argv[1])+std::string("/mesh.1.ele");
  }
  else
  {
      meshNodeFilename    = "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.node";
      meshElementFilename = "/share/CI-WATER_Simulation_Data/small_green_mesh/mesh.1.ele";
  }

  bool    error               = false; // Error flag.
  int     ii, jj;                      // Loop counters;
  FILE*   nodeFile;                    // Input file for mesh nodes.
  FILE*   eleFile;                     // Input file for mesh elements.
  int     numScanned;                  // Used to check that fscanf scanned all of the requested values.
  int     numberOfMeshNodes;           // Size of and number of mesh nodes actually existing in meshNodesX and meshNodesY arrays.
  int     numberOfMeshElements;        // Number of Mesh elements.
  int     dimension;                   // Used to check the dimension of the files.
  int     numberOfAttributes;          // Used to check the number of attributes in the files.
  int     boundary;                    // Used to check the number of boundary codes in the files.
  double* meshNodesX          = NULL;  // X coordinates of mesh nodes.
  double* meshNodesY          = NULL;  // Y coordinates of mesh nodes.
  int     meshNodeNumber;              // For reading the node number of a mesh node.
  int     meshElementNumber;           // For reading the element number of a mesh element.
  double  xCoordinate;                 // For reading X coordinates of mesh nodes.
  double  yCoordinate;                 // For reading Y coordinates of mesh nodes.
  int     detectedIndex;               // Number of detected problem triangles.
  double* detectedX           = NULL;  // X coordinates of detected problem triangles.  Used to exclude larger nearby problem triangles.
  double* detectedY           = NULL;  // Y coordinates of detected problem triangles.  Used to exclude larger nearby problem triangles.
  double* detectedArea        = NULL;  // Areas of detected problem triangles.  Used to exclude larger nearby problem triangles.
  double* detectedAngle       = NULL;  // Angles of detected problem triangles.  Used to exclude larger nearby problem triangles.
  int     meshVertex[3];               // For reading the vertices of a mesh element.
  int     meshCatchment;               // For reading the catchment of a mesh element.
  double  area;                        // For calculating the area of a mesh element.
  double  side[3];                     // For calculating the side lengths of a mesh element.
  double  angle[3];                    // For calculating the angles of a mesh element.
  bool    tooClose;                    // Whether a problem triangle is too close to a previously reported smaller problem triangle.
  
  // Open the files.
  nodeFile = fopen(meshNodeFilename.c_str(), "r");
  eleFile  = fopen(meshElementFilename.c_str(), "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (!(NULL != nodeFile))
    {
      fprintf(stderr, "ERROR in main: Could not open mesh node file %s.\n", meshNodeFilename.c_str());
      error = true;
    }
  
  if (!(NULL != eleFile))
    {
      fprintf(stderr, "ERROR in main: Could not open mesh element file %s.\n", meshElementFilename.c_str());
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
          fprintf(stderr, "ERROR in main: Unable to read header from mesh node file %s.\n", meshNodeFilename.c_str());
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfMeshNodes && 2 == dimension && 0 == numberOfAttributes && 1 == boundary))
        {
          fprintf(stderr, "ERROR in main: Invalid header in mesh node file %s.\n", meshNodeFilename.c_str());
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

  // Read the nodes.
  for (ii = 0; !error && ii < numberOfMeshNodes; ii++)
    {
      numScanned = fscanf(nodeFile, "%d %lf %lf %*d", &meshNodeNumber, &xCoordinate, &yCoordinate);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          fprintf(stderr, "ERROR in main: Unable to read entry %d from mesh node file %s.\n", ii, meshNodeFilename.c_str());
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == meshNodeNumber))
        {
          fprintf(stderr, "ERROR in main: Invalid node number in mesh node file %s.  %d should be %d.\n", meshNodeFilename.c_str(), meshNodeNumber, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Save coordinates.
      if (!error)
        {
          meshNodesX[meshNodeNumber] = xCoordinate;
          meshNodesY[meshNodeNumber] = yCoordinate;
        }
    }
  
  // Read the number of elements from the file.
  if (!error)
    {
      numScanned = fscanf(eleFile, "%d %d %d", &numberOfMeshElements, &dimension, &numberOfAttributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(3 == numScanned))
        {
          fprintf(stderr, "ERROR in main: Unable to read header from mesh element file %s.\n", meshElementFilename.c_str());
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(0 < numberOfMeshElements && 3 == dimension && 1 == numberOfAttributes))
        {
          fprintf(stderr, "ERROR in main: Invalid header in mesh element file %s.\n", meshElementFilename.c_str());
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
      detectedArea  = new double[numberOfMeshElements];
      detectedAngle = new double[numberOfMeshElements];
    }
  
  // Read the elements.
  for (ii = 0; !error && ii < numberOfMeshElements; ii++)
    {
      numScanned = fscanf(eleFile, "%d %d %d %d %d", &meshElementNumber, &meshVertex[0], &meshVertex[1], &meshVertex[2], &meshCatchment);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (!(5 == numScanned))
        {
          fprintf(stderr, "ERROR in main: Unable to read entry %d from mesh element file %s.\n", ii, meshElementFilename.c_str());
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (!(ii == meshElementNumber))
        {
          fprintf(stderr, "ERROR in main: Invalid element number in mesh element file %s.  %d should be %d.\n", meshElementFilename.c_str(), meshElementNumber, ii);
          error = true;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      
      // Check the element.
      if (!error)
        {
          area = 0.0;

          for (jj = 0; jj < 3; jj++)
            {
              area    += meshNodesX[meshVertex[jj]] * (meshNodesY[meshVertex[(jj + 1) % 3]] - meshNodesY[meshVertex[(jj + 2) % 3]]);
              side[jj] = sqrt((meshNodesX[meshVertex[(jj + 1) % 3]] - meshNodesX[meshVertex[(jj + 2) % 3]]) *
                              (meshNodesX[meshVertex[(jj + 1) % 3]] - meshNodesX[meshVertex[(jj + 2) % 3]]) +
                              (meshNodesY[meshVertex[(jj + 1) % 3]] - meshNodesY[meshVertex[(jj + 2) % 3]]) *
                              (meshNodesY[meshVertex[(jj + 1) % 3]] - meshNodesY[meshVertex[(jj + 2) % 3]]));
            }

          area *= 0.5;
          
          for (jj = 0; jj < 3; jj++)
            {
              angle[jj] = acos(((side[jj] * side[jj]) - (side[(jj + 1) % 3] * side[(jj + 1) % 3]) - (side[(jj + 2) % 3] * side[(jj + 2) % 3])) /
                               (-2.0 * side[(jj + 1) % 3] * side[(jj + 2) % 3])) * 180.0 / M_PI; // Convert to degrees.
              
              // We really just want the smallest angle so put that one in angle[0].
              if (angle[0] > angle[jj])
                {
                  angle[0] = angle[jj];
                }
            }
          
          if (0 == meshCatchment || 7500.0 > area || 20.0 > angle[0])
            {
              // This is a problem triangle.  Report it if there hasn't already been a smaller one reported within 100 meters.  Always report triangles with no
              // catchment.
              tooClose = false;
              
              for (jj = 0; 0 != meshCatchment && !tooClose && jj < detectedIndex; jj++)
                {
                  if (100.0 >= sqrt((meshNodesX[meshVertex[0]] - detectedX[jj]) * (meshNodesX[meshVertex[0]] - detectedX[jj]) +
                                    (meshNodesY[meshVertex[0]] - detectedY[jj]) * (meshNodesY[meshVertex[0]] - detectedY[jj])) &&
                      area  >= detectedArea[jj] && angle[0] >= detectedAngle[jj])
                    {
                      tooClose = true;
                    }
                }
              
              if (!tooClose)
                {
                  printf("Found problem triangle at (%lf, %lf), catchment = %d, area = %lf, angle = %lf.\n", meshNodesX[meshVertex[0]],
                         meshNodesY[meshVertex[0]], meshCatchment, area, angle[0]);
                  
                  detectedX[detectedIndex]     = meshNodesX[meshVertex[0]];
                  detectedY[detectedIndex]     = meshNodesY[meshVertex[0]];
                  detectedArea[detectedIndex]  = area;
                  detectedAngle[detectedIndex] = angle[0];
                  
                  detectedIndex++;
                }
            }
        }
    }

  // Close the files.
  if (NULL != nodeFile)
    {
      fclose(nodeFile);
    }

  if (NULL != eleFile)
    {
      fclose(eleFile);
    }
  
  // Deallocate the arrays.
  delete[] meshNodesX;
  delete[] meshNodesY;
  delete[] detectedX;
  delete[] detectedY;
  delete[] detectedArea;
  delete[] detectedAngle;
  
  return 0;
}
