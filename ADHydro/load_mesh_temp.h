#ifndef MESH_H
#define MESH_H

/* For associating mesh edges with channel network links.
 */
typedef struct
{
  // FIXME x and y are unused and z is only used for two things.  Remove them from here and calculate where used?
  double  x;                     // The x coordinate of the center of this edge.
  double  y;                     // The y coordinate of the center of this edge.
  double  z;                     // The z coordinate of the ground surface of the center of this edge.
  int     linkno;                // The linkno in the channel network that this edge is associated with or -1 if this is not a channel edge.
  int     first_element;         // The first element on linkno that this edge is associate with.  Additional elements are sequential after first_element.
  int     num_elements;          // The number of elements on linkno that this edge is associated with.
  double* element_fractions;     // The fraction of this edge touching each associated element on linkno.  element_fractions[1] is for first_element.
                                 // element_fractions[2] is for first_element + 1, etc.  element_fractions should sum to 1.0.
  double  groundwater_flow_rate; // The flow rate in cubic meters per second that will flow between the groundwater in this mesh element and all of the channel
                                 // elements associated with this mesh edge.  When water is moved, the flow rate will be apportioned to the channel elements
                                 // according to element_fractions.  Positive means flow out of the mesh element into the channel.  Negative means flow out of
                                 // the channel into the mesh element.
} channel_edge_type;

/* A mesh_type struct stores the geometry of a mesh.  All arrays use one-based
 * indexing. Units for all values are the same as the units of the coordinates
 * in the .node file.  We always specify coordinates in meters.
 */
typedef struct
{
  int      num_nodes;                // Number of nodes.
  double** nodes_xyz;                // A 2D array of doubles stored as an array of pointers to 1D arrays.
                                     // nodes_xyz[ii][1] is the x coordinate of node ii.
                                     // nodes_xyz[ii][2] is the y coordinate of node ii.
                                     // nodes_xyz[ii][3] is the z coordinate of the ground surface of node ii.
                                     // nodes_xyz[ii][4] is the z coordinate of the bedrock of node ii.
  int      num_elements;             // Number of elements.
  int**    elements;                 // A 2D array of integers stored as an array of pointers to 1D arrays.
                                     // elements[ii][jj] is the node index of the jjth vertex of element ii.
                                     // Elements have three vertices numbered 1 to 3.
  int      num_element_attributes;   // Number of attributes of each element.
  double** element_attributes;       // A 2D array of doubles stored as an array of pointers to 1D arrays.
                                     // element_attributes[ii][jj] is the jjth attribute of element ii.
                                     // If num_element_attributes is zero then element_attributes will be NULL.
                                     // For our purposes elements generally have only one attribute, which is the ID number of the catchment
                                     // that the element is in, but this feature could be used for other things too.
  int**    neighbors;                // A 2D array of integers stored as an array of pointers to 1D arrays.
                                     // neighbors[ii][jj] is the element index of the neighbor across the edge opposite the jjth vertex of element ii
                                     // or a boundary condition code.
  channel_edge_type** channel_edges; // A 2D array of channel_edge_type structs stored as an array of pointers to 1D arrays.
                                     // channel_edges[ii][jj] is for the edge opposite the jjth vertex of element ii.
  double** elements_xyz;             // A 2D array of doubles stored as an array of pointers to 1D arrays.
                                     // elements_xyz[ii][1] is the x coordinate of the center of element ii.
                                     // elements_xyz[ii][2] is the y coordinate of the center of element ii.
                                     // elements_xyz[ii][3] is the z coordinate of the ground surface of the center of element ii.
                                     // elements_xyz[ii][4] is the z coordinate of the bedrock of the center of element ii.
  double** edge_length;              // A 2D array of doubles stored as an array of pointers to 1D arrays.
                                     // edge_length[ii][jj] is the length of the edge opposite the jjth vertex of element ii.
  double** edge_normal_x;            // A 2D array of doubles stored as an array of pointers to 1D arrays.
                                     // edge_normal_x[ii][jj] is the x component of the unit normal vector of the edge opposite the jjth vertex of element ii.
  double** edge_normal_y;            // A 2D array of doubles stored as an array of pointers to 1D arrays.
                                     // edge_normal_y[ii][jj] is the y component of the unit normal vector of the edge opposite the jjth vertex of element ii.
  double*  elements_area;            // A 1D array of doubles.  elements_area[ii] is the area of element ii.
  double   total_area;               // The total area of all mesh elements.
} mesh_type;

/* Read in the geometry of a mesh from files and create a new mesh.
 * Allocate the mesh_type struct and all arrays that it points to.
 * Return TRUE if there is an error, FALSE otherwise.
 *
 * Parameters:
 *
 * mesh          - A pointer passed by reference which will be assigned to point
 *                 to the newly allocated mesh or NULL if there is an error.
 * file_basename - The basename of the triangle files that the mesh will
 *                 be read from.  mesh_read will read the following files:
 *                 file_basename.node, file_basename.z, file_basename.ele,
 *                 file_basename.neigh, and file_basename.edge.
 */
int mesh_read(mesh_type** mesh, char* file_basename);

/* Free memory allocated by mesh_read.
 *
 * Parameters:
 *
 * mesh - A pointer to the mesh passed by reference.
 *        Will be set to NULL after the memory is deallocated.
 */
int mesh_dealloc(mesh_type** mesh);

/* Return TRUE if the mesh invariant is violated, FALSE otherwise.
 * Also print an error message indicating the details of what is wrong.
 *
 * Parameters:
 *
 * mesh - The mesh to check for invariant.
 */
int mesh_check_invariant(mesh_type* mesh);

/* mesh.h needs to include channel.h for struct channel_link, but channel.h
 * includes mesh.h so I'm just doing this incomplete type here to get around
 * the circularity.
 */
typedef struct channel_link channel_link_type;

/* Try to fix various quality problems in the mesh and place no flow boundaries
 * between different catchments.
 *
 * Parameters:
 *
 * mesh     - The mesh to massage.
 * channels - The channel network corresponding to mesh.  Can be passed in as
 *            NULL in which case digital pits will not be fixed by connecting
 *            them to the channel linkno matching their catchment ID.
 */
void mesh_massage(mesh_type* mesh, channel_link_type* channels);

#endif // MESH_H
