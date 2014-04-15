#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "mesh.h"
#include "channel.h"
#include "epsilon.h"
#include "memfunc.h"
#include "quantifier.h"
#include "all.h"

/* Read a triangle node file and store the data in mesh.
 * Return TRUE if there is an error, FALSE otherwise.
 * Allocate mesh->nodes_xyz as a 2D array of doubles stored as an array of
 * pointers to 1D arrays.  Allocate each 1D array so that it can hold 4
 * elements with one based indexing to hold the x, y, ground surface z, and
 * bedrock z coordinates, but only fill in the x and y coordinates.
 * Call read_z to fill in the z coordinates.
 *
 * Parameters:
 *
 * mesh     - The mesh in which to store node data.
 * filename - The name of the triangle node file to read.
 */
int read_nodes(mesh_type* mesh, char* filename)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && NULL == mesh->nodes_xyz && NULL != filename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  
  int error = FALSE; // Error flag.
  int ii;            // Loop counter.
  int num_scanned;   // Used to check that fscanf scanned all of the requested values.
  int node_num;      // Used to check that the node indices in the file are one based and sequential.
  int dimension;     // Used to check that the dimension is 2.
  int attributes;    // Used to check that the number of attributes is 0.
  int boundary;      // Used to check that the number of boundary markers is 1.

  // Open the file.
  FILE* node_file = fopen(filename, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (NULL == node_file)
    {
      fprintf(stderr, "ERROR: Could not open node file %s.\n", filename);
      error = TRUE;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Read the number of nodes from the file.
  if (!error)
    {
      num_scanned = fscanf(node_file, "%d %d %d %d", &mesh->num_nodes, &dimension, &attributes, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (4 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from node file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (0 >= mesh->num_nodes || 2 != dimension || 0 != attributes || 1 != boundary)
        {
          fprintf(stderr, "ERROR: Invalid header in node file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Allocate the array.
  if (!error)
    {
      error = dtwo_alloc(&mesh->nodes_xyz, mesh->num_nodes, 4);
    }

  // Fill in the x and y coordinates.
  for (ii = 1; !error && ii <= mesh->num_nodes; ii++)
    {
      num_scanned = fscanf(node_file, "%d %lf %lf %*d", &node_num, &mesh->nodes_xyz[ii][1], &mesh->nodes_xyz[ii][2]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (3 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from node file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (ii != node_num)
        {
          fprintf(stderr, "ERROR: Invalid node number %d in node file %s should be %d.\n", node_num, filename, ii);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Close the file.
  if (NULL != node_file)
    {
      fclose(node_file);
    }

  return error;
}

/* Read a triangle z file and store the data in mesh.
 * Return TRUE if there is an error, FALSE otherwise.
 * Fill in the ground surface z and bedrock z coordinates.
 * The z file isn't actually created by triangle.  We create it, but the file
 * format is patterened after triangle files.  This function assumes
 * mesh->nodes_xyz has already been allocated by read_nodes.
 *
 * Parameters:
 *
 * mesh     - The mesh in which to store node z data.
 * filename - The name of the triangle z file to read.
 */
int read_z(mesh_type* mesh, char* filename)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && NULL != mesh->nodes_xyz && NULL != filename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)

  int error = FALSE; // Error flag.
  int ii;            // Loop counter.
  int num_scanned;   // Used to check that fscanf scanned all of the requested values.
  int node_num;      // Used to check that the node indices in the file are one based and sequential.
  int dimension;     // Used to check that the dimension is 1.

  // FIXME figure out the real spatially varying soil depth.
  double soil_depth = 1.0;

  // Open the file.
  FILE* z_file = fopen(filename, "r");

  if (NULL == z_file)
    {
      // hardcode the values
      double slope_x = 0.05;
      double slope_y = 0.05;
      
      for (ii = 1; ii <= mesh->num_nodes; ii++)
        {
          mesh->nodes_xyz[ii][3] = (slope_x * fabs(mesh->nodes_xyz[ii][1]) + slope_y * fabs(mesh->nodes_xyz[ii][2]));
          mesh->nodes_xyz[ii][4] = mesh->nodes_xyz[ii][3] - soil_depth;
        }

      // FIXME do we want hardcoding to be a real option, or is it just a temporary kludge?
      fprintf(stderr, "WARNING: Could not open z file %s.\n", filename);
      //fprintf(stderr, "ERROR: Could not open z file %s.\n", filename);
      //error = TRUE;
    }

  // Read the number of nodes from the file.
  if (!error && NULL != z_file)
    {
      num_scanned = fscanf(z_file, "%d %d", &node_num, &dimension);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (2 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from z file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (mesh->num_nodes != node_num || 1 != dimension)
        {
          fprintf(stderr, "ERROR: Invalid header in z file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Fill in the z coordinates.
  for (ii = 1; !error && NULL != z_file && ii <= mesh->num_nodes; ii++)
    {
      num_scanned = fscanf(z_file, "%d %lf", &node_num, &mesh->nodes_xyz[ii][3]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (2 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from z file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (ii != node_num)
        {
          fprintf(stderr, "ERROR: Invalid node number %d in z file %s should be %d.\n", node_num, filename, ii);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      if (!error)
        {
          mesh->nodes_xyz[ii][4] = mesh->nodes_xyz[ii][3] - soil_depth;
        }
    }

  // Close the file.
  if (NULL != z_file)
    {
      fclose(z_file);
    }

  return error;
}

/* Read a triangle ele file and store the data in mesh.
 * Return TRUE if there is an error, FALSE otherwise.
 * Allocate mesh->elements as a 2D array of ints stored as an array of pointers
 * to 1D arrays.  Allocate each 1D array so that it can hold 3 elements with
 * one based indexing to hold the three vertices of each element.  Allocate
 * mesh->element_attributes as a 2D array of doubles stored as an array of
 * pointers to 1D arrays.  Allocate each 1D array so that it can hold the
 * number of attributes specified in the ele file with one based indexing.
 * If the number of attributes is zero no array is allocated and
 * mesh->element_attributes is set to NULL.
 *
 * Parameters:
 *
 * mesh     - The mesh in which to store element data.
 * filename - The name of the triangle ele file to read.
 */
int read_elements(mesh_type* mesh, char* filename)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && NULL == mesh->elements && NULL == mesh->element_attributes && NULL != filename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)

  int error = FALSE; // Error flag.
  int ii, jj;        // Loop counters.
  int num_scanned;   // Used to check that fscanf scanned all of the requested values.
  int element_num;   // Used to check that the element indices in the file are one based and sequential.
  int dimension;     // Used to check that the dimension is 3.

  // Open the file.
  FILE* element_file = fopen(filename, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (NULL == element_file)
    {
      fprintf(stderr, "ERROR: Could not open element file %s.\n", filename);
      error = TRUE;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Read the number of elements from the file.
  if (!error)
    {
      num_scanned = fscanf(element_file, "%d %d %d", &mesh->num_elements, &dimension, &mesh->num_element_attributes);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (3 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from element file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (0 >= mesh->num_elements || 3 != dimension || 0 > mesh->num_element_attributes)
        {
          fprintf(stderr, "ERROR: Invalid header in element file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Allocate mesh->elements.
  if (!error)
    {
      error = itwo_alloc(&mesh->elements, mesh->num_elements, 3);
    }

  // Allocate mesh->element_attributes.
  if (!error && 0 < mesh->num_element_attributes)
    {
      error = dtwo_alloc(&mesh->element_attributes, mesh->num_elements, mesh->num_element_attributes);
    }

  // Fill in the vertiex node indices and attributes.
  for (ii = 1; !error && ii <= mesh->num_elements; ii++)
    {
      num_scanned = fscanf(element_file, "%d %d %d %d", &element_num, &mesh->elements[ii][1], &mesh->elements[ii][2], &mesh->elements[ii][3]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (4 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from element file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (ii != element_num)
        {
          fprintf(stderr, "ERROR: Invalid element number %d in element file %s should be %d.\n", element_num, filename, ii);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      for (jj = 1; !error && jj <= mesh->num_element_attributes; jj++)
        {
          num_scanned = fscanf(element_file, "%lf", &mesh->element_attributes[ii][jj]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
          if (1 != num_scanned)
            {
              fprintf(stderr, "ERROR: Error while reading from element file %s.\n", filename);
              error = TRUE;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        }
    }

  // Close the file.
  if (NULL != element_file)
    {
      fclose(element_file);
    }

  return error;
}

/* Read a triangle neigh file and store the data in mesh.
 * Return TRUE if there is an error, FALSE otherwise.
 * Allocate mesh->neighbors as a 2D array of ints stored as an array of
 * pointers to 1D arrays.  Allocate each 1D array so that it can hold 3
 * elements with one based indexing to hold the three neighbors of each
 * element.
 * 
 * Parameters:
 *
 * mesh     - The mesh in which to store element neighbor data.
 * filename - The name of the triangle neigh file to read.
 */
int read_neighbors(mesh_type* mesh, char* filename)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && NULL == mesh->neighbors && NULL != filename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)

  int error = FALSE; // Error flag.
  int ii;            // Loop counter.
  int num_scanned;   // Used to check that fscanf scanned all of the requested values.
  int element_num;   // Used to check that the element indices in the file are one based and sequential.
  int dimension;     // Used to check that the dimension is 3.

  // Open the file.
  FILE* neighbor_file = fopen(filename, "r");

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
  if (NULL == neighbor_file)
    {
      fprintf(stderr, "ERROR: Could not open neighbor file %s.\n", filename);
      error = TRUE;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

  // Read the number of elements from the file.
  if (!error)
    {
      num_scanned = fscanf(neighbor_file, "%d %d", &element_num, &dimension);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (2 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from neighbor file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (mesh->num_elements != element_num || 3 != dimension)
        {
          fprintf(stderr, "ERROR: Invalid header in neighbor file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Allocate the array.
  if (!error)
    {
      error = itwo_alloc(&mesh->neighbors, mesh->num_elements, 3);
    }

  // Fill in the neighbor element indices.
  for (ii = 1; !error && ii <= mesh->num_elements; ii++)
    {
      num_scanned = fscanf(neighbor_file, "%d %d %d %d", &element_num, &mesh->neighbors[ii][1], &mesh->neighbors[ii][2], &mesh->neighbors[ii][3]);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (4 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from neighbor file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (ii != element_num)
        {
          fprintf(stderr, "ERROR: Invalid element number %d in neighbor file %s should be %d.\n", element_num, filename, ii);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Close the file.
  if (NULL != neighbor_file)
    {
      fclose(neighbor_file);
    }

  return error;
}

/* Read a triangle edge file and store the data in mesh.
 * Return TRUE if there is an error, FALSE otherwise.
 * Allocate mesh->channel_edges as a 2d array of channel_edge_type structs
 * stored as an array of pointers to 1D arrays.  Allocate each 1D array so that
 * it can hold 3 elements with one based indexing for the three edges of each
 * element.
 *
 * Parameters:
 *
 * mesh     - The mesh in which to store edge data.
 * filename - The name of the triangle edge file to read.
 */
int read_edges(mesh_type* mesh, char* filename)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && NULL == mesh->channel_edges && NULL != filename);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  
  int error = FALSE; // Error flag.
  int ii, jj;        // Loop counters.
  int num_scanned;   // Used to check that fscanf scanned all of the requested values.
  int num_edges;     // The total number of edges.
  int edge_num;      // Used to check that the edge indices in the file are one based and sequential.
  int boundary;      // Used to check that the number of boundary markers is 1 and to read the boundary marker of each edge.
  int vertex_1;      // The first vertex of the edge.
  int vertex_2;      // The second vertex of the edge.

  // Open the file.
  FILE* edge_file = fopen(filename, "r");

  if (NULL == edge_file)
    {
      // FIXME right now the edge file is optional.  Decide the permanent way to handle this.
      fprintf(stderr, "WARNING: Could not open edge file %s.\n", filename);
      //fprintf(stderr, "ERROR: Could not open edge file %s.\n", filename);
      //error = TRUE;
    }
      
  // Read the number of edges from the file.
  if (!error && NULL != edge_file)
    {
      num_scanned = fscanf(edge_file, "%d %d", &num_edges, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (2 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from edge file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (0 >= num_edges || 1 != boundary)
        {
          fprintf(stderr, "ERROR: Invalid header in edge file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    }

  // Allocate the array.
  if (!error)
    {
      error = vtwo_alloc((void***)&mesh->channel_edges, mesh->num_elements + 1, (3 + 1) * sizeof(channel_edge_type));
    }

  // Initialize the array.  This cannot be combined with the next loop because we must initialize all
  // pointers to NULL before an error happens so that we don't later try to free them.
  if (!error)
    {
      for (ii = 1; ii <= mesh->num_elements; ii++)
        {
          for (jj = 1; jj <= 3; jj++)
            {
              mesh->channel_edges[ii][jj].x                     = 0.0;
              mesh->channel_edges[ii][jj].y                     = 0.0;
              mesh->channel_edges[ii][jj].z                     = 0.0;
              mesh->channel_edges[ii][jj].linkno                = -1;
              mesh->channel_edges[ii][jj].first_element         = -1;
              mesh->channel_edges[ii][jj].num_elements          = 0;
              mesh->channel_edges[ii][jj].element_fractions     = NULL;
              mesh->channel_edges[ii][jj].groundwater_flow_rate = 0.0;
            }
        }
    }

  // Read the edges.
  for (ii = 1; !error && NULL != edge_file && ii <= num_edges; ii++)
    {
      num_scanned = fscanf(edge_file, "%d %d %d %d", &edge_num, &vertex_1, &vertex_2, &boundary);

#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (4 != num_scanned)
        {
          fprintf(stderr, "ERROR: Error while reading from edge file %s.\n", filename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)

#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
      if (ii != edge_num)
        {
          fprintf(stderr, "ERROR: Invalid edge number %d in edge file %s should be %d.\n", edge_num, filename, ii);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

      // If this is a channel network edge store the link index.
      if (!error && 0 > boundary)
        {
          // FIXLATER possible optimization break out of this loop after you find two matching element edges.
          for (jj = 1; jj <= mesh->num_elements; jj++)
            {
              if ((mesh->elements[jj][1] == vertex_1 && mesh->elements[jj][2] == vertex_2) ||
                  (mesh->elements[jj][1] == vertex_2 && mesh->elements[jj][2] == vertex_1))
                {
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  if (-1 != mesh->channel_edges[jj][3].linkno)
                    {
                      fprintf(stderr, "ERROR: Mesh element %d edge %d assigned to two channel links %d and %d.\n",
                              jj, 3, mesh->channel_edges[jj][3].linkno, -boundary);
                      error = TRUE;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

                  mesh->channel_edges[jj][3].linkno = -boundary;
                }
              else if ((mesh->elements[jj][1] == vertex_1 && mesh->elements[jj][3] == vertex_2) ||
                       (mesh->elements[jj][1] == vertex_2 && mesh->elements[jj][3] == vertex_1))
                {
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  if (-1 != mesh->channel_edges[jj][2].linkno)
                    {
                      fprintf(stderr, "ERROR: Mesh element %d edge %d assigned to two channel links %d and %d.\n",
                              jj, 2, mesh->channel_edges[jj][2].linkno, -boundary);
                      error = TRUE;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

                  mesh->channel_edges[jj][2].linkno = -boundary;
                }
              else if ((mesh->elements[jj][2] == vertex_1 && mesh->elements[jj][3] == vertex_2) ||
                       (mesh->elements[jj][2] == vertex_2 && mesh->elements[jj][3] == vertex_1))
                {
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                  if (-1 != mesh->channel_edges[jj][1].linkno)
                    {
                      fprintf(stderr, "ERROR: Mesh element %d edge %d assigned to two channel links %d and %d.\n",
                              jj, 1, mesh->channel_edges[jj][1].linkno, -boundary);
                      error = TRUE;
                    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)

                  mesh->channel_edges[jj][1].linkno = -boundary;
                }
            }
        }
    }

  // Close the file.
  if (NULL != edge_file)
    {
      fclose(edge_file);
    }

  return error;
}

/* Calculate derived geometry values for a single element in mesh.
 *
 * Parameters:
 *
 * mesh    - The mesh in which to store element data.
 * element - The element index to calculate geometry for.
 */
void calculate_single_element_geometry(mesh_type* mesh, int element)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && NULL != mesh->nodes_xyz && NULL != mesh->elements && NULL != mesh->channel_edges && NULL != mesh->elements_xyz &&
         NULL != mesh->edge_length && NULL != mesh->edge_normal_x && NULL != mesh->edge_normal_y && NULL != mesh->elements_area &&
         0 < element && element <= mesh->num_elements);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  
  // X, y, and z coordinates of all vertices of the element.
  int    vertex_1           = mesh->elements[element][1];
  double vertex_1_x         = mesh->nodes_xyz[vertex_1][1];
  double vertex_1_y         = mesh->nodes_xyz[vertex_1][2];
  double vertex_1_ground_z  = mesh->nodes_xyz[vertex_1][3];
  double vertex_1_bedrock_z = mesh->nodes_xyz[vertex_1][4];
  int    vertex_2           = mesh->elements[element][2];
  double vertex_2_x         = mesh->nodes_xyz[vertex_2][1];
  double vertex_2_y         = mesh->nodes_xyz[vertex_2][2];
  double vertex_2_ground_z  = mesh->nodes_xyz[vertex_2][3];
  double vertex_2_bedrock_z = mesh->nodes_xyz[vertex_2][4];
  int    vertex_3           = mesh->elements[element][3];
  double vertex_3_x         = mesh->nodes_xyz[vertex_3][1];
  double vertex_3_y         = mesh->nodes_xyz[vertex_3][2];
  double vertex_3_ground_z  = mesh->nodes_xyz[vertex_3][3];
  double vertex_3_bedrock_z = mesh->nodes_xyz[vertex_3][4];

  // X, y, and z coordinates of the center of the element.
  mesh->elements_xyz[element][1] = (vertex_1_x         + vertex_2_x         + vertex_3_x)         / 3.0;
  mesh->elements_xyz[element][2] = (vertex_1_y         + vertex_2_y         + vertex_3_y)         / 3.0;
  mesh->elements_xyz[element][3] = (vertex_1_ground_z  + vertex_2_ground_z  + vertex_3_ground_z)  / 3.0;
  mesh->elements_xyz[element][4] = (vertex_1_bedrock_z + vertex_2_bedrock_z + vertex_3_bedrock_z) / 3.0;
  
  // X, y, and z coordinates of the center of the edges.
  mesh->channel_edges[element][1].x = (vertex_2_x        + vertex_3_x)        / 2.0;
  mesh->channel_edges[element][1].y = (vertex_2_y        + vertex_3_y)        / 2.0;
  mesh->channel_edges[element][1].z = (vertex_2_ground_z + vertex_3_ground_z) / 2.0;
  mesh->channel_edges[element][2].x = (vertex_1_x        + vertex_3_x)        / 2.0;
  mesh->channel_edges[element][2].y = (vertex_1_y        + vertex_3_y)        / 2.0;
  mesh->channel_edges[element][2].z = (vertex_1_ground_z + vertex_3_ground_z) / 2.0;
  mesh->channel_edges[element][3].x = (vertex_1_x        + vertex_2_x)        / 2.0;
  mesh->channel_edges[element][3].y = (vertex_1_y        + vertex_2_y)        / 2.0;
  mesh->channel_edges[element][3].z = (vertex_1_ground_z + vertex_2_ground_z) / 2.0;
  
  // Length of each edge.
  // Edge 1 goes from vertex 2 to 3 (opposite vertex 1).
  // Edge 2 goes form vertex 3 to 1 (opposite vertex 2).
  // Edge 3 goes form vertex 1 to 2 (opposite vertex 3).
  mesh->edge_length[element][1] = sqrt((vertex_2_x - vertex_3_x) * (vertex_2_x - vertex_3_x) + (vertex_2_y - vertex_3_y) * (vertex_2_y - vertex_3_y));
  mesh->edge_length[element][2] = sqrt((vertex_3_x - vertex_1_x) * (vertex_3_x - vertex_1_x) + (vertex_3_y - vertex_1_y) * (vertex_3_y - vertex_1_y));
  mesh->edge_length[element][3] = sqrt((vertex_1_x - vertex_2_x) * (vertex_1_x - vertex_2_x) + (vertex_1_y - vertex_2_y) * (vertex_1_y - vertex_2_y));

  // Unit normal vector of each edge.
  mesh->edge_normal_x[element][1] = (vertex_3_y - vertex_2_y) / mesh->edge_length[element][1];
  mesh->edge_normal_y[element][1] = (vertex_2_x - vertex_3_x) / mesh->edge_length[element][1];
  mesh->edge_normal_x[element][2] = (vertex_1_y - vertex_3_y) / mesh->edge_length[element][2];
  mesh->edge_normal_y[element][2] = (vertex_3_x - vertex_1_x) / mesh->edge_length[element][2];
  mesh->edge_normal_x[element][3] = (vertex_2_y - vertex_1_y) / mesh->edge_length[element][3];
  mesh->edge_normal_y[element][3] = (vertex_1_x - vertex_2_x) / mesh->edge_length[element][3];

  // Area of element.
  mesh->elements_area[element] = (vertex_1_x * (vertex_2_y - vertex_3_y) + vertex_2_x * (vertex_3_y - vertex_1_y) +
                                  vertex_3_x * (vertex_1_y - vertex_2_y)) * 0.5;
}

/* Calculate derived geometry values for elements in mesh.
 * Return TRUE if there is an error, FALSE otherwise.
 * Allocate and fill in elements_xyz, edge_length, edge_normal_x,
 * edge_normal_y, elements_area, and total_area.
 *
 * Parameters:
 *
 * mesh - The mesh in which to store element data.
 */
int calculate_elements_geometry(mesh_type* mesh)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && NULL == mesh->elements_xyz && NULL == mesh->edge_length && NULL == mesh->edge_normal_x && NULL == mesh->edge_normal_y &&
         NULL == mesh->elements_area && 0.0 == mesh->total_area);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)

  int error = FALSE; // Error flag.
  int ii;            // Loop counter.
  
  // Allocate the arrays.
  error = dtwo_alloc(&mesh->elements_xyz, mesh->num_elements, 4);

  if (!error)
    {
      error = dtwo_alloc(&mesh->edge_length, mesh->num_elements, 3);
    }

  if (!error)
    {
      error = dtwo_alloc(&mesh->edge_normal_x, mesh->num_elements, 3);
    }

  if (!error)
    {
      error = dtwo_alloc(&mesh->edge_normal_y, mesh->num_elements, 3);
    }

  if (!error)
    {
      error = d_alloc(&mesh->elements_area, mesh->num_elements);
    }

  // Calculate the geometry values.
  if (!error)
    {
      for (ii = 1; ii <= mesh->num_elements; ii++)
        {
          calculate_single_element_geometry(mesh, ii);
          
          mesh->total_area += mesh->elements_area[ii];
        }
    } // End if (!error).

  return error;
}

/* Comment in .h file. */
int mesh_read(mesh_type** mesh, char* file_basename)
{
  int error = FALSE;     // Error flag.
  int file_basename_len; // For declaring string variable to hold filename including extension.
  int num_printed;       // Used to check that fprintf printed all of the requested values.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (NULL == mesh)
    {
      fprintf(stderr, "ERROR: mesh must not be NULL\n");
      error = TRUE;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      *mesh = NULL; // Prevent deallocating a random pointer.
    }

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (NULL == file_basename)
    {
      fprintf(stderr, "ERROR: file_basename must not be NULL\n");
      error             = TRUE;
      file_basename_len = 0;
    }
  else
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
    {
      file_basename_len = strlen(file_basename);
    }

  char filename[file_basename_len + strlen(".neigh") + 1];  // Filename including the longest extension ".neigh" and terminating null character.

  if (!error)
    {
      error = v_alloc((void**)mesh, sizeof(mesh_type));
    }

  if (!error)
    {
      (*mesh)->num_nodes              = 0;
      (*mesh)->nodes_xyz              = NULL;
      (*mesh)->num_elements           = 0;
      (*mesh)->elements               = NULL;
      (*mesh)->num_element_attributes = 0;
      (*mesh)->element_attributes     = NULL;
      (*mesh)->neighbors              = NULL;
      (*mesh)->channel_edges          = NULL;
      (*mesh)->elements_xyz           = NULL;
      (*mesh)->edge_length            = NULL;
      (*mesh)->edge_normal_x          = NULL;
      (*mesh)->edge_normal_y          = NULL;
      (*mesh)->elements_area          = NULL;
      (*mesh)->total_area             = 0.0;
    }

  if (!error)
    {
      num_printed = sprintf(filename, "%s.node", file_basename);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (file_basename_len + strlen(".node") != num_printed)
        {
          fprintf(stderr, "ERROR: Error generating filename string from %s and .node.\n", file_basename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      error = read_nodes(*mesh, filename);
    }

  if (!error)
    {
      num_printed = sprintf(filename, "%s.z", file_basename);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (file_basename_len + strlen(".z") != num_printed)
        {
          fprintf(stderr, "ERROR: Error generating filename string from %s and .z.\n", file_basename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      error = read_z(*mesh, filename);
    }

  if (!error)
    {
      num_printed = sprintf(filename, "%s.ele", file_basename);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (file_basename_len + strlen(".ele") != num_printed)
        {
          fprintf(stderr, "ERROR: Error generating filename string from %s and .ele.\n", file_basename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      error = read_elements(*mesh, filename);
    }

  if (!error)
    {
      num_printed = sprintf(filename, "%s.neigh", file_basename);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (file_basename_len + strlen(".neigh") != num_printed)
        {
          fprintf(stderr, "ERROR: Error generating filename string from %s and .neigh.\n", file_basename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      error = read_neighbors(*mesh, filename);
    }

  if (!error)
    {
      num_printed = sprintf(filename, "%s.edge", file_basename);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
      if (file_basename_len + strlen(".edge") != num_printed)
        {
          fprintf(stderr, "ERROR: Error generating filename string from %s and .edge.\n", file_basename);
          error = TRUE;
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    }
  
  if (!error)
    {
      error = read_edges(*mesh, filename);
    }

  if (!error)
    {
      error = calculate_elements_geometry(*mesh);
    }
  
  // Check invariant.
#if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  if (!error)
    {
      error = mesh_check_invariant(*mesh);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_INVARIANTS)
  
  if (error && NULL != mesh && NULL != *mesh)
    {
      mesh_dealloc(mesh);
    }

  return error;
}

/* Comment in .h file. */
int mesh_dealloc(mesh_type** mesh)
{
  int error      = FALSE; // Error flag.
  int temp_error = FALSE; // Error flag.
  int ii, jj;             // Loop counters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (NULL == mesh || NULL == *mesh)
    {
      fprintf(stderr, "ERROR: mesh must not be NULL.\n");
      error = TRUE;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Do not check invariant because it is legal to deallocate a partially allocated channel network.
  
  if (!error)
    {
      if (NULL != (*mesh)->nodes_xyz)
        {
          temp_error = dtwo_dealloc(&(*mesh)->nodes_xyz, (*mesh)->num_nodes, 4);
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->elements)
        {
          temp_error = itwo_dealloc(&(*mesh)->elements, (*mesh)->num_elements, 3);
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->element_attributes)
        {
          temp_error = dtwo_dealloc(&(*mesh)->element_attributes, (*mesh)->num_elements, (*mesh)->num_element_attributes);
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->neighbors)
        {
          temp_error = itwo_dealloc(&(*mesh)->neighbors, (*mesh)->num_elements, 3);
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->channel_edges)
        {
          for (ii = 1; ii <= (*mesh)->num_elements; ii++)
            {
              for (jj = 1; jj <= 3; jj++)
                {
                  if (NULL != (*mesh)->channel_edges[ii][jj].element_fractions)
                    {
                      temp_error = d_dealloc(&(*mesh)->channel_edges[ii][jj].element_fractions, (*mesh)->channel_edges[ii][jj].num_elements);
                      error      = error || temp_error;
                    }
                }
            }
          
          temp_error = vtwo_dealloc((void***)&(*mesh)->channel_edges, (*mesh)->num_elements + 1, (3 + 1) * sizeof(channel_edge_type));
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->elements_xyz)
        {
          temp_error = dtwo_dealloc(&(*mesh)->elements_xyz, (*mesh)->num_elements, 4);
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->edge_length)
        {
          temp_error = dtwo_dealloc(&(*mesh)->edge_length, (*mesh)->num_elements, 3);
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->edge_normal_x)
        {
          temp_error = dtwo_dealloc(&(*mesh)->edge_normal_x, (*mesh)->num_elements, 3);
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->edge_normal_y)
        {
          temp_error = dtwo_dealloc(&(*mesh)->edge_normal_y, (*mesh)->num_elements, 3);
          error      = error || temp_error;
        }

      if (NULL != (*mesh)->elements_area)
        {
          temp_error = d_dealloc(&(*mesh)->elements_area, (*mesh)->num_elements);
          error      = error || temp_error;
        }

      temp_error = v_dealloc((void**)mesh, sizeof(mesh_type));
      error      = error || temp_error;
    }
  
  return error;
}

/* Comment in .h file. */
int mesh_check_invariant(mesh_type* mesh)
{
  int error = FALSE; // Error flag.
  int ii, jj;        // Loop counters.

  if (NULL == mesh)
    {
      fprintf(stderr, "ERROR: mesh must not be NULL.\n");
      error = TRUE;
    }

  if (!error)
    {
      // Process all nodes
      for (ii = 1; ii <= mesh->num_nodes; ii++)
        {
          // Bedrock z coordinate must less than or equal to ground surface z coordinate.
          if (mesh->nodes_xyz[ii][4] > mesh->nodes_xyz[ii][3])
            {
              fprintf(stderr, "ERROR: Mesh node %d has bedrock z coordinate greater than surface z coordinate.\n", ii);
              error = TRUE;
            }
        }
      
      // Process all elements.
      for (ii = 1; ii <= mesh->num_elements; ii++)
        {
          // Bedrock z coordinate must less than or equal to ground surface z coordinate.
          if (mesh->elements_xyz[ii][4] > mesh->elements_xyz[ii][3])
            {
              fprintf(stderr, "ERROR: Mesh element %d has bedrock z coordinate greater than surface z coordinate.\n", ii);
              error = TRUE;
            }
          
          for (jj = 1; jj <= 3; jj++)
            {
              // All element vertices must be valid nodes.
              int node = mesh->elements[ii][jj];

              if (0 >= node || node > mesh->num_nodes)
                {
                  fprintf(stderr, "ERROR: Mesh element %d vertex %d has invalid node index.\n", ii, jj);
                  error = TRUE;
                }

              // All element neighbors must be valid elements or valid boundary conditions.
              int neighbor = mesh->neighbors[ii][jj];

              if (!IS_BOUNDARY(neighbor) && (0 >= neighbor || neighbor > mesh->num_elements || !i_exists(mesh->neighbors[neighbor], 3, i_test_equals, &ii)))
                {
                  fprintf(stderr, "ERROR: Mesh element %d edge %d has invalid neighbor.\n", ii, jj);
                  error = TRUE;
                }
            }
        }
    }
  
  return error;
}

/* Swap the diagonal of a quadrilateral formed by two adjacent triangles.
 * Return whether the swap was able to be performed.  A swap may not be able to
 * be peformed because of a variety of conditions.  Each condition prints a
 * warning, but it is up to the calling routine whether an inability to swap is
 * an error.
 * 
 *              C
 *             /|\
 *  neighbor1 / | \ neighbor4
 *           /  |  \
 *          /   |SSE\
 *       A /    |    \ D
 *         \    |    /
 *          \SE |   /
 *           \  |  /
 *  neighbor2 \ | / neighbor3
 *             \|/
 *              B
 * 
 * swap_element (labeled SE) is triangle ABC.  swap_vertex is vertex A.
 * second_swap_element (labeled SSE) is the neighbor of swap_element opposite
 * swap_vertex.  It is triangle DCB.  second_swap_vertex is vertex D.  The
 * quadrilateral ABDC formed by swap_element and second_swap element will have
 * its diagonal swapped from BC to AD as shown below so that swap_element
 * becomes triangle ABD and second_swap_element becomes triangle DCA.
 * 
 *              C
 *             / \
 *  neighbor1 /   \ neighbor4
 *           /     \
 *          /    SSE\
 *       A /_________\ D
 *         \         /
 *          \SE     /
 *           \     /
 *  neighbor2 \   / neighbor3
 *             \ /
 *              B
 */
int swap_diagonal(mesh_type* mesh, int swap_element, int swap_vertex)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && 0 < swap_element && swap_element <= mesh->num_elements && 1 <= swap_vertex && 3 >= swap_vertex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  
  int ii;                                                               // Loop counter.
  int perform_swap        = TRUE;                                       // Whether the swap can be performed.
  int second_swap_element = mesh->neighbors[swap_element][swap_vertex]; // The element on the other side of the diagonal to swap.
  int second_swap_vertex;                                               // The vertex of second_swap_element opposite of swap_element, vertex D.
  int swap_element_vertex_b;                                            // The vertex index of B in swap_element.
  int swap_element_vertex_c;                                            // The vertex index of C in swap_element.
  int second_swap_element_vertex_c;                                     // The vertex index of C in second_swap_element.
  int second_swap_element_vertex_b;                                     // The vertex index of B in second_swap_element.
  int neighbor1;                                                        // The element index of neighbor1.
  int neighbor3;                                                        // The element index of neighbor3.
  int neighbor1_neighbor;                                               // The neighbor index of neighbor1 that must be changed to second_swap_element.
  int neighbor3_neighbor;                                               // The neighbor index of neighbor3 that must be changed to swap_element.
  
  // Check if second_swap_element exists.
  if (0 >= second_swap_element || second_swap_element > mesh->num_elements)
    {
      fprintf(stderr, "WARNING: swap_element %d swap_vertex %d second_swap_element is not an element.  Swap diagonal aborted.\n", swap_element, swap_vertex);
      perform_swap = FALSE;
    }
  
  // Find second_swap_vertex.
  if (perform_swap)
    {
      second_swap_vertex = 1;

      while (3 > second_swap_vertex && swap_element != mesh->neighbors[second_swap_element][second_swap_vertex])
        {
          second_swap_vertex++;
        }
      
      if (swap_element != mesh->neighbors[second_swap_element][second_swap_vertex])
        {
          fprintf(stderr, "WARNING: swap_element %d second_swap_element %d had no reciprocal neighbor.  Swap diagonal aborted.\n",
                  swap_element, second_swap_element);
          perform_swap = FALSE;
        }
    }
  
  // Abort if there are any channel edges that would have to be reorganized.
  // FIXME we could instead implement the reorganization.
  for (ii = 1; perform_swap && ii <= 3; ii++)
    {
      if (-1 != mesh->channel_edges[swap_element][ii].linkno || -1 != mesh->channel_edges[second_swap_element][ii].linkno)
        {
          fprintf(stderr, "WARNING: swap_element %d or second_swap_element %d has channel edge.  Swap diagonal aborted.\n", swap_element, second_swap_element);
          perform_swap = FALSE;
        }
    }
  
  if (perform_swap)
    {
      // Find the vertex indices of B and C in both elements.
      swap_element_vertex_b        = swap_vertex + 1;
      swap_element_vertex_c        = swap_vertex + 2;
      second_swap_element_vertex_c = second_swap_vertex + 1;
      second_swap_element_vertex_b = second_swap_vertex + 2;
      
      if (3 < swap_element_vertex_b)
        {
          swap_element_vertex_b -= 3;
        }
      
      if (3 < swap_element_vertex_c)
        {
          swap_element_vertex_c -= 3;
        }
      
      if (3 < second_swap_element_vertex_c)
        {
          second_swap_element_vertex_c -= 3;
        }
      
      if (3 < second_swap_element_vertex_b)
        {
          second_swap_element_vertex_b -= 3;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
      // The node ids of vertices B and C should be the same whether they are referenced from swap_element or second_swap_element.
      assert(mesh->elements[swap_element][swap_element_vertex_b] == mesh->elements[second_swap_element][second_swap_element_vertex_b] &&
             mesh->elements[swap_element][swap_element_vertex_c] == mesh->elements[second_swap_element][second_swap_element_vertex_c]);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
      
      // Find the neighbors of the quadrilateral.
      neighbor1 = mesh->neighbors[swap_element][swap_element_vertex_b];
      neighbor3 = mesh->neighbors[second_swap_element][second_swap_element_vertex_c];
      
      // Find swap_element in neighbor1's neighbor list.
      if (0 < neighbor1 && neighbor1 <= mesh->num_elements)
        {
          neighbor1_neighbor = 1;

          while (3 > neighbor1_neighbor && swap_element != mesh->neighbors[neighbor1][neighbor1_neighbor])
            {
              neighbor1_neighbor++;
            }

          if (swap_element != mesh->neighbors[neighbor1][neighbor1_neighbor])
            {
              fprintf(stderr, "WARNING: swap_element %d neighbor1 %d had no reciprocal neighbor.  Swap diagonal aborted.\n", swap_element, neighbor1);
              perform_swap = FALSE;
            }
        }
    }
  
  if (perform_swap)
    {
      // Find second_swap_element in neighbor3's neighbor list.
      if (0 < neighbor3 && neighbor3 <= mesh->num_elements)
        {
          neighbor3_neighbor = 1;

          while (3 > neighbor3_neighbor && second_swap_element != mesh->neighbors[neighbor3][neighbor3_neighbor])
            {
              neighbor3_neighbor++;
            }

          if (second_swap_element != mesh->neighbors[neighbor3][neighbor3_neighbor])
            {
              fprintf(stderr, "WARNING: swap_element %d neighbor3 %d had no reciprocal neighbor.  Swap diagonal aborted.\n", swap_element, neighbor3);
              perform_swap = FALSE;
            }
        }
    }
  
  if (perform_swap)
    {
      // Swap vertices of the diagonal.
      mesh->elements[swap_element][swap_element_vertex_c]               = mesh->elements[second_swap_element][second_swap_vertex];
      mesh->elements[second_swap_element][second_swap_element_vertex_b] = mesh->elements[swap_element][swap_vertex];
      
      // Recalculate element geometry.
      double area_sum = mesh->elements_area[swap_element] + mesh->elements_area[second_swap_element];
      
      calculate_single_element_geometry(mesh, swap_element);
      calculate_single_element_geometry(mesh, second_swap_element);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
      assert(epsilon_equal(area_sum, mesh->elements_area[swap_element] + mesh->elements_area[second_swap_element]));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
      
      if (area_sum * 0.2 > mesh->elements_area[swap_element] || area_sum * 0.2 > mesh->elements_area[second_swap_element])
        {
          fprintf(stderr, "WARNING: Quadrilateral formed by swap_element %d and second_swap_element %d is not sufficiently convex.  Swap diagonal aborted.\n",
                  swap_element, second_swap_element);
          perform_swap = FALSE;
          
          // Put the vertices back.
          mesh->elements[swap_element][swap_element_vertex_c]               = mesh->elements[second_swap_element][second_swap_element_vertex_c];
          mesh->elements[second_swap_element][second_swap_element_vertex_b] = mesh->elements[swap_element][swap_element_vertex_b];
          
          // Recalculate element geometry.
          calculate_single_element_geometry(mesh, swap_element);
          calculate_single_element_geometry(mesh, second_swap_element);
          
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
          assert(epsilon_equal(area_sum, mesh->elements_area[swap_element] + mesh->elements_area[second_swap_element]));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_ASSERTIONS)
        }
    }
  
  if (perform_swap)
    {
      // Update neighbor lists of swap_element and second_swap_element.
      mesh->neighbors[swap_element][swap_vertex]                         = neighbor3;
      mesh->neighbors[swap_element][swap_element_vertex_b]               = second_swap_element;
      mesh->neighbors[second_swap_element][second_swap_vertex]           = neighbor1;
      mesh->neighbors[second_swap_element][second_swap_element_vertex_c] = swap_element;
      
      // Swap swap_element and second_swap_element in the neighbor lists of neighbor1 and neighbor3.
      if (0 < neighbor1 && neighbor1 <= mesh->num_elements)
        {
          mesh->neighbors[neighbor1][neighbor1_neighbor] = second_swap_element;
        }
      
      if (0 < neighbor3 && neighbor3 <= mesh->num_elements)
        {
          mesh->neighbors[neighbor3][neighbor3_neighbor] = swap_element;
        }
    }
  
  return perform_swap;
}

/* The types of fixes that can be applied to digital pits.
 */
typedef enum
{
  NO_FIX,               // Do not attempt to fix.  Just check for the existence of a pit.
  ISOLATED_ELEMENT_FIX, // If the element has no same-catchment neighbors, no outflow boundaries, and no channel edges move it into a neighboring catchment.
  SWAP_DIAGONAL_FIX,    // Attempt to fix by swapping the diagonal with a neighbor.
  CHANNEL_CONNECT_FIX   // Arbitrarily connect the element to the channel linkno matching the catchment ID.
} digital_pit_fix_type;

// FIXME document
int digital_pit_check(mesh_type* mesh, int element, channel_link* channels, digital_pit_fix_type fix)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  assert(NULL != mesh && 0 < element && element <= mesh->num_elements &&
         (NO_FIX == fix || ISOLATED_ELEMENT_FIX == fix || SWAP_DIAGONAL_FIX == fix || CHANNEL_CONNECT_FIX == fix));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_ALL_FUNCTIONS_SIMPLE)
  
  int    ii;                                   // Loop counter.
  int    has_same_catchment_neighbor = FALSE;  // One of the conditions of being a digital pit.
  int    has_lower_neighbor          = FALSE;  // One of the conditions of being a digital pit.
  int    has_outflow_boundary        = FALSE;  // One of the conditions of being a digital pit.
  int    has_channel_edge            = FALSE;  // One of the conditions of being a digital pit.
  int    is_pit                      = FALSE;  // Whether the element is a digital pit.
  double pit_depth                   = 1.0e10; // How deep the digital pit is.
  
  for (ii = 1; ii <= 3; ii++)
    {
      int neighbor = mesh->neighbors[element][ii];
      
      if (0 < neighbor && neighbor <= mesh->num_elements)
        {
          // If there are any element attributes, assume the first attribute is the catchment ID.
          if (0 < mesh->num_element_attributes)
            {
              if (mesh->element_attributes[neighbor][1] == mesh->element_attributes[element][1])
                {
                  has_same_catchment_neighbor = TRUE;
                }
            }
          else // If there are not any element attributes, assume all elements are in the same catchment.
            {
              has_same_catchment_neighbor = TRUE;
            }
          
          if (mesh->elements_xyz[neighbor][3] < mesh->elements_xyz[element][3])
            {
              has_lower_neighbor = TRUE;
            }
          else
            {
              double temp_pit_depth = mesh->elements_xyz[neighbor][3] - mesh->elements_xyz[element][3];
              
              if (pit_depth > temp_pit_depth)
                {
                  pit_depth = temp_pit_depth;
                }
            }
        }
      
      if (OUTFLOW == neighbor)
        {
          has_outflow_boundary = TRUE;
        }
      
      if (-1 != mesh->channel_edges[element][ii].linkno)
        {
          has_channel_edge = TRUE;
        }
    }
  
  if (ISOLATED_ELEMENT_FIX == fix && !has_same_catchment_neighbor && !has_outflow_boundary && !has_channel_edge)
    {
      // With no same-catchment neighbors, no outflow boundaries, and no channel edges an element will be a digital pit.
      // To fix this move the element to a neighboring catchment.
      is_pit = TRUE;

      if (0 < mesh->num_element_attributes)
        {
          int    move_catchment = FALSE;
          double move_catchment_to;
          int    neighbor1      = mesh->neighbors[element][1];
          int    neighbor2      = mesh->neighbors[element][2];
          int    neighbor3      = mesh->neighbors[element][3];
          int    has_neighbor1  = (0 < neighbor1 && neighbor1 <= mesh->num_elements);
          int    has_neighbor2  = (0 < neighbor2 && neighbor2 <= mesh->num_elements);
          int    has_neighbor3  = (0 < neighbor3 && neighbor3 <= mesh->num_elements);

          // If two neighbors share the same catchment move the element into that catchment
          if (has_neighbor1 && has_neighbor2 && mesh->element_attributes[neighbor1][1] == mesh->element_attributes[neighbor2][1])
            {
              move_catchment = TRUE;
              move_catchment_to = mesh->element_attributes[neighbor1][1];
            }
          else if (has_neighbor1 && has_neighbor3 && mesh->element_attributes[neighbor1][1] == mesh->element_attributes[neighbor3][1])
            {
              move_catchment = TRUE;
              move_catchment_to = mesh->element_attributes[neighbor1][1];
            }
          else if (has_neighbor2 && has_neighbor3 && mesh->element_attributes[neighbor2][1] == mesh->element_attributes[neighbor3][1])
            {
              move_catchment = TRUE;
              move_catchment_to = mesh->element_attributes[neighbor2][1];
            }
          else
            {
              // If no two neighbors share the same catchment move the element into the catchment of the lowest neighbor.
              if (has_neighbor1 && (!has_neighbor2 || mesh->elements_xyz[neighbor1][3] <= mesh->elements_xyz[neighbor2][3]) &&
                                   (!has_neighbor3 || mesh->elements_xyz[neighbor1][3] <= mesh->elements_xyz[neighbor3][3]))
                {
                  move_catchment = TRUE;
                  move_catchment_to = mesh->element_attributes[neighbor1][1];
                }
              else if (has_neighbor2 && (!has_neighbor1 || mesh->elements_xyz[neighbor2][3] <= mesh->elements_xyz[neighbor1][3]) &&
                                        (!has_neighbor3 || mesh->elements_xyz[neighbor2][3] <= mesh->elements_xyz[neighbor3][3]))
                {
                  move_catchment = TRUE;
                  move_catchment_to = mesh->element_attributes[neighbor2][1];
                }
              else if (has_neighbor3 && (!has_neighbor1 || mesh->elements_xyz[neighbor3][3] <= mesh->elements_xyz[neighbor1][3]) &&
                                        (!has_neighbor2 || mesh->elements_xyz[neighbor3][3] <= mesh->elements_xyz[neighbor2][3]))
                {
                  move_catchment = TRUE;
                  move_catchment_to = mesh->element_attributes[neighbor3][1];
                }
            }

          if (move_catchment)
            {
              fprintf(stderr, "WARNING: Mesh element %d has no same-catchment neighbors, outflow boundaries, or channel edges.  "
                              "Moving from catchment %.0lf to %.0lf.\n", element, mesh->element_attributes[element][1], move_catchment_to);
              mesh->element_attributes[element][1] = move_catchment_to;
            }
          else
            {
              fprintf(stderr, "WARNING: Mesh element %d has no same-catchment neighbors, outflow boundaries, or channel edges, "
                              "but has no neighboring catchment to move to.\n", element);
            }
        }
    }
  else if (SWAP_DIAGONAL_FIX == fix && !has_lower_neighbor && !has_outflow_boundary && !has_channel_edge)
    {
      // Attempt to fix by swapping the diagonal with a neighbor.  Swapping with a particular neighbor may fail so try each neighbor from lowest elevation to
      // highest until one works or all fail.
      is_pit = TRUE;
      
      int    neighbor1      = mesh->neighbors[element][1];
      int    neighbor2      = mesh->neighbors[element][2];
      int    neighbor3      = mesh->neighbors[element][3];
      int    has_neighbor1  = (0 < neighbor1 && neighbor1 <= mesh->num_elements);
      int    has_neighbor2  = (0 < neighbor2 && neighbor2 <= mesh->num_elements);
      int    has_neighbor3  = (0 < neighbor3 && neighbor3 <= mesh->num_elements);
      int    neighbor_to_swap_with[4];
      double height_of_neighbor[4];
      
      // Sort neighbors by height.  Perform an insertion sort of the three neighbors.
      // FIXLATER This could probably be more elegant.
      if (has_neighbor1)
        {
          neighbor_to_swap_with[1] = 1;
          height_of_neighbor[1]    = mesh->elements_xyz[neighbor1][3];
        }
      else
        {
          neighbor_to_swap_with[1] = -1;
        }
      
      if (has_neighbor2)
        {
          if (-1 == neighbor_to_swap_with[1] || mesh->elements_xyz[neighbor2][3] < height_of_neighbor[1])
            {
              neighbor_to_swap_with[2] = neighbor_to_swap_with[1];
              height_of_neighbor[2]    = height_of_neighbor[1];
              neighbor_to_swap_with[1] = 2;
              height_of_neighbor[1]    = mesh->elements_xyz[neighbor2][3];
            }
          else
            {
              neighbor_to_swap_with[2] = 2;
              height_of_neighbor[2]    = mesh->elements_xyz[neighbor2][3];
            }
        }
      else
        {
          neighbor_to_swap_with[2] = -1;
        }
      
      if (has_neighbor3)
        {
          if (-1 == neighbor_to_swap_with[1] || mesh->elements_xyz[neighbor3][3] < height_of_neighbor[1])
            {
              neighbor_to_swap_with[3] = neighbor_to_swap_with[2];
              height_of_neighbor[3]    = height_of_neighbor[2];
              neighbor_to_swap_with[2] = neighbor_to_swap_with[1];
              height_of_neighbor[2]    = height_of_neighbor[1];
              neighbor_to_swap_with[1] = 3;
              height_of_neighbor[1]    = mesh->elements_xyz[neighbor3][3];
            }
          else if (-1 == neighbor_to_swap_with[2] || mesh->elements_xyz[neighbor3][3] < height_of_neighbor[2])
            {
              neighbor_to_swap_with[3] = neighbor_to_swap_with[2];
              height_of_neighbor[3]    = height_of_neighbor[2];
              neighbor_to_swap_with[2] = 3;
              height_of_neighbor[2]    = mesh->elements_xyz[neighbor3][3];
            }
          else
            {
              neighbor_to_swap_with[3] = 3;
              height_of_neighbor[3]    = mesh->elements_xyz[neighbor3][3];
            }
        }
      else
        {
          neighbor_to_swap_with[3] = -1;
        }

      int swap_with;
      int swap_performed = FALSE;

      for (ii = 1; !swap_performed && -1 != neighbor_to_swap_with[ii] && ii <= 3; ii++)
        {
          swap_with      = mesh->neighbors[element][neighbor_to_swap_with[ii]];
          swap_performed = swap_diagonal(mesh, element, neighbor_to_swap_with[ii]);
        }

      if (swap_performed)
        {
          fprintf(stderr, "WARNING: Diagonal swap performed on digital pit element %d and neighbor %d.\n", element, swap_with);
        }
      else
        {
          fprintf(stderr, "WARNING: Could not perform diagonal swap on digital pit element %d.\n", element);
        }
    }
  else if (CHANNEL_CONNECT_FIX == fix && !has_lower_neighbor && !has_outflow_boundary && !has_channel_edge)
    {
      is_pit = TRUE;
      
      if (0 < mesh->num_element_attributes && NULL != channels)
        {
          // Find the channel link associated with this catchment.
          int linkno = mesh->element_attributes[element][1];
          
          // linkno might have been pruned.  If so, travel downstream to find the closest unpruned link.
          while (0 < linkno && (INTERNAL_BOUNDARY_CONDITION == channels[linkno].type || PRUNED_STREAM == channels[linkno].type ||
                                PRUNED_FAKE == channels[linkno].type || PRUNED_INTERNAL_BOUNDARY_CONDITION == channels[linkno].type))
            {
              linkno = channels[linkno].downstream[1];
            }
          
          if (0 >= linkno || (STREAM != channels[linkno].type && WATERBODY != channels[linkno].type && ICEMASS != channels[linkno].type))
            {
              fprintf(stderr, "WARNING: Invalid channel linkno for element %d.\n", element);
            }
          else
            {
              // Connect the lowest edge of element to linkno.
              int make_connection = TRUE;
              int lowest_edge     = 1;
              
              if (mesh->channel_edges[element][2].z < mesh->channel_edges[element][lowest_edge].z)
                {
                  lowest_edge = 2;
                }
              
              if (mesh->channel_edges[element][3].z < mesh->channel_edges[element][lowest_edge].z)
                {
                  lowest_edge = 3;
                }
              
              // Find the neighbor's edge.
              int neighbor      = mesh->neighbors[element][lowest_edge];
              int has_neighbor  = (0 < neighbor && neighbor <= mesh->num_elements);
              int neighbor_edge = 1;
              
              if (has_neighbor)
                {
                  while (3 > neighbor_edge && element != mesh->neighbors[neighbor][neighbor_edge])
                    {
                      neighbor_edge++;
                    }
                  
                  if (element != mesh->neighbors[neighbor][neighbor_edge])
                    {
                      fprintf(stderr, "WARNING: Element %d neighbor %d has no reciprocal neighbor.  Connection to channel not made.\n", element, neighbor);
                      make_connection = FALSE;
                    }
                  else if (-1 != mesh->channel_edges[neighbor][neighbor_edge].linkno || NULL != mesh->channel_edges[neighbor][neighbor_edge].element_fractions)
                    {
                      fprintf(stderr, "WARNING: Element %d has no channel edge, but neighbor %d does.  Connection to channel not made.\n", element, neighbor);
                      make_connection = FALSE;
                    }
                  else if (d_alloc(&mesh->channel_edges[neighbor][neighbor_edge].element_fractions, 1))
                    {
                      fprintf(stderr, "WARNING: Could not allocate element fractions for element %d neighbor %d.  Connection to channel not made.\n", element, neighbor);
                      make_connection = FALSE;
                    }
                }
              
              if (make_connection && d_alloc(&mesh->channel_edges[element][lowest_edge].element_fractions, 1))
                {
                  fprintf(stderr, "WARNING: Could not allocate element fractions for element %d.  Connection to channel not made.\n", element);
                  make_connection = FALSE;
                  
                  if (has_neighbor)
                    {
                      d_dealloc(&mesh->channel_edges[neighbor][neighbor_edge].element_fractions, 1);
                    }
                }
              
              if (make_connection)
                {
                  fprintf(stderr, "WARNING: Connecting element %d, edge %d and neighbor %d, edge %d in catchment %.0lf to channel linkno %d.\n",
                          element, lowest_edge, neighbor, neighbor_edge, mesh->element_attributes[element][1], linkno);
                  mesh->channel_edges[element][lowest_edge].linkno               = linkno;
                  mesh->channel_edges[element][lowest_edge].first_element        = 1;
                  mesh->channel_edges[element][lowest_edge].num_elements         = 1;
                  mesh->channel_edges[element][lowest_edge].element_fractions[1] = 1.0;
                  
                  if (has_neighbor)
                    {
                      mesh->channel_edges[neighbor][neighbor_edge].linkno               = linkno;
                      mesh->channel_edges[neighbor][neighbor_edge].first_element        = 1;
                      mesh->channel_edges[neighbor][neighbor_edge].num_elements         = 1;
                      mesh->channel_edges[neighbor][neighbor_edge].element_fractions[1] = 1.0;
                    }
                }
            }
        }
    }
  else if (NO_FIX == fix && !has_lower_neighbor && !has_outflow_boundary && !has_channel_edge)
    {
      is_pit = TRUE;
      fprintf(stderr, "WARNING: Mesh element %d is a digital pit of depth %lf.\n", element, pit_depth);
    }
  
  return is_pit;
}

/* Comment in .h file. */
void mesh_massage(mesh_type* mesh, channel_link* channels)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  assert(NULL != mesh);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  int ii, jj; // Loop counters.

  // If there are any element attributes, assume the first attribute is the catchment ID.
  if (0 < mesh->num_element_attributes)
    {
      // Fix any isolated elements.
      for (ii = 1; ii <= mesh->num_elements; ii++)
        {
          digital_pit_check(mesh, ii, channels, ISOLATED_ELEMENT_FIX);
        }
      
      // Place a no flow boundary between neighbors in different catchments with a ridge between.
      for (ii = 1; ii <= mesh->num_elements; ii++)
        {
          for (jj = 1; jj <= 3; jj++)
            {
              int neighbor = mesh->neighbors[ii][jj];

              if (0 < neighbor && neighbor <= mesh->num_elements && mesh->element_attributes[ii][1] != mesh->element_attributes[neighbor][1] &&
                  -1 == mesh->channel_edges[ii][jj].linkno && mesh->elements_xyz[ii][3] < mesh->channel_edges[ii][jj].z &&
                  mesh->elements_xyz[neighbor][3] < mesh->channel_edges[ii][jj].z)
                {
                  mesh->neighbors[ii][jj] = NOFLOW;
                }
            }
        }
    }
  
  // Try to fix any digital pits by swapping diagonals.
  for (ii = 1; ii <= mesh->num_elements; ii++)
    {
      digital_pit_check(mesh, ii, channels, SWAP_DIAGONAL_FIX);
    }
  
  // Swapping diagonals a second time has some positive effect.
  for (ii = 1; ii <= mesh->num_elements; ii++)
    {
      digital_pit_check(mesh, ii, channels, SWAP_DIAGONAL_FIX);
    }
  
  // If there are any element attributes, assume the first attribute is the catchment ID.
  if (0 < mesh->num_element_attributes && NULL != mesh)
    {
      // For the remaining pits just arbitrarily connect them to the channel linkno matching the catchment ID.
      for (ii = 1; ii <= mesh->num_elements; ii++)
        {
          digital_pit_check(mesh, ii, channels, CHANNEL_CONNECT_FIX);
        }
    }
  
  // Check if any digital pits remain.
  int num_digital_pits = 0;
  
  for (ii = 1; ii <= mesh->num_elements; ii++)
    {
      if (digital_pit_check(mesh, ii, channels, NO_FIX))
        {
          num_digital_pits++;
        }
    }
  
  if (0 < num_digital_pits)
    {
      fprintf(stderr, "WARNING: %d digital pits detected.\n", num_digital_pits);
    }
}
