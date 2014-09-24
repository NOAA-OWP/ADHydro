#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

// file_manager.decl.h needs CProxy_MeshElement from mesh_element.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// mesh_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_ChannelElement from channel_element.decl.h.
// channel_element.decl.h needs CProxy_FileManager from file_manager.decl.h and CProxy_MeshElement from mesh_element.decl.h.
// These forward declarations break the circularity.
class CProxy_MeshElement;
class CProxy_ChannelElement;

// Suppress warnings in the The Charm++ autogenerated code.
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "file_manager.decl.h"
#pragma GCC diagnostic warning "-Wsign-compare"

#include "mesh_element.h"
#include "channel_element.h"

typedef int    intarraymmn[MeshElement::meshNeighborsSize];           // Fixed size array of ints.     Size is mesh    mesh neighbors.
typedef bool   boolarraymmn[MeshElement::meshNeighborsSize];          // Fixed size array of bools.    Size is mesh    mesh neighbors.
typedef double doublearraymmn[MeshElement::meshNeighborsSize];        // Fixed size array of doubles.  Size is mesh    mesh neighbors.
typedef int    intarraymcn[MeshElement::channelNeighborsSize];        // Fixed size array of ints.     Size is mesh    channel neighbors.
typedef double doublearraymcn[MeshElement::channelNeighborsSize];     // Fixed size array of doubles.  Size is mesh    channel neighbors.
typedef int    intarrayxdmf[ChannelElement::channelVerticesSize + 2]; // Fixed size array of ints.     Size is channel vertices + 2.  This is used only for
                                                                      // channelElementVertices.  See the comment of that variable for explanation.
typedef double doublearraycvn[ChannelElement::channelVerticesSize];   // Fixed size array of doubles.  Size is channel vertices.
typedef int    intarrayccn[ChannelElement::channelNeighborsSize];     // Fixed size array of ints.     Size is channel channel neighbors.
typedef int    intarraycmn[ChannelElement::meshNeighborsSize];        // Fixed size array of ints.     Size is channel mesh neighbors.
typedef double doublearraycmn[ChannelElement::meshNeighborsSize];     // Fixed size array of doubles.  Size is channel mesh neighbors.

// The group of file managers acts as an in-memory cache for values that are
// read from and written to NetCDF files.  Reading and writing individual
// values from NetCDF files is too slow.  We need to read and write large
// blocks from the arrays in the NetCDF files.  However, individual values are
// needed by mesh and channel elements.  The file manager group does the block
// reads and writes and makes the values available to the elements.
//
// At initialization, each file manager takes ownership of a block of each
// array containing the data for the elements that get assigned to its
// processor by the default Charm++ block mapping of array elements to
// processors.  This way, elements can initialize themselves without any
// message passing by getting a pointer to the file manager local branch and
// accessing its public member variables.
//
// After initialization, array elements might migrate away from the file
// manager local branch that owns their data.  We do not transfer ownership of
// the data between file manager local branches.  Instead, message passing is
// used to update data after initialization.
class FileManager : public CBase_FileManager
{
  FileManager_SDAG_CODE
  
public:

  // Calculate which file manager owns a given item.  The first
  // (globalNumberOfItems % CkNumPes()) file managers each have
  // (globalNumberOfItems / CkNumPes() + 1) items.  The remaining file managers
  // each have (globalNumberOfItems / CkNumPes()) items.
  //
  // Returns: the index of the file manager that owns item.
  //
  // Parameters:
  //
  // item                - The index of this item.
  // globalNumberOfItems - The total number of this kind of item.
  static int home(int item, int globalNumberOfItems);
  
  // Constructor.  Initializes the file manager to hold no data.
  FileManager();
  
  // Destructor.  Dynamically allocated arrays need to be deleted.
  ~FileManager();
  
  // FIXME move handle initialization functions to private
  
  // This function pulls out some duplicate code that is the same for reading
  // the mesh and channels .node and .z files.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // directory    - The directory from which to read the files.
  // fileBasename - The base name not including the directory path of the files
  //                to read.  If readMesh is true readNodeAndZFiles will read
  //                directory/fileBasename.node and directory/fileBasename.z.
  //                If readMesh is false readNodeAndZFiles will read
  //                directory/fileBasename.chan.node and
  //                directory/fileBasename.chan.z.
  // readMesh     - If true read the mesh node and z files.  If false read the
  //                channel node and z files.
  bool readNodeAndZFiles(const char* directory, const char* fileBasename, bool readMesh);
  
  // Initializes the file manager to hold a hardcoded mesh.  When done the file
  // managers contribute to an empty reduction.
  void handleInitializeHardcodedMesh();
  
  // Initializes the file manager from ASCII files.  When done the file
  // managers contribute to an empty reduction.
  //
  // ASCII files contain geometry and parameters only so the water state is
  // initialized to groundwater completely saturated and no surfacewater.
  //
  // Parameters:
  //
  // directorySize    - The size of the array passed in to directory.
  // directory        - The directory from which to read the files.
  // fileBasenameSize - The size of the array passed in to fileBasename.
  // fileBasename     - The base name not including the directory path of the
  //                    files to read.  initializeFromASCIIFiles will read the
  //                    following files: directory/fileBasename.node,
  //                    FIXME etc.
  void handleInitializeFromASCIIFiles(size_t directorySize, const char* directory, size_t fileBasenameSize, const char* fileBasename);
  
  // Initializes the file manager from NetCDF files.  When done the file
  // managers contribute to an empty reduction.
  //
  // Parameters:
  //
  // directorySize - The size of the array passed in to directory.
  // directory     - The directory from which to read the files.
  void handleInitializeFromNetCDFFiles(size_t directorySize, const char* directory);
  
  // If a variable is not available the file managers attempt to derive it from
  // other variables, for example element center coordinates from element
  // vertex coordinates.  If a variable is not available and cannot be derived
  // its array is unallocated and left as NULL.  When done the file managers
  // contribute to an empty reduction.
  void calculateDerivedValues();
  
  // Send requested vertex coordinates.
  //
  // Parameters:
  //
  // requester - File manager requesting vertex coordinates.
  // element   - Element that has vertex.
  // vertex    - Vertex requesting coordinates for.
  // node      - Node index of vertex.
  void getMeshVertexDataMessage(int requester, int element, int vertex, int node);

  // Read forcing data from file and send to mesh elements.  Each file manager
  // reads the data for the mesh elements it owns and sends it on to those mesh
  // elements.
  //
  // Parameters:
  //
  // meshProxy    - The mesh elements to send the forcing data to.
  // channelProxy - The channel elements to send the forcing data to.
  void readForcingData(CProxy_MeshElement meshProxy, CProxy_ChannelElement channelProxy);
  
  // Create NetCDF files for output including creating dimensions and variables
  // within the files.  When done the file managers contribute to an empty
  // reduction.
  //
  // Parameters:
  //
  // directorySize - The size of the array passed in to directory.
  // directory     - The directory in which to create the files.
  void createFiles(size_t directorySize, const char* directory);
  
  // This is a workaround for a problematic interaction when using NetCDF
  // parallel collective I/O with Charm++.  A write to a NetCDF file that
  // resizes an unlimited dimension must be done in collective mode, but doing
  // so in Charm++ causes an MPI error because Charm++ is also using MPI.
  //
  // Instead of calling writeFiles on all file managers, call this function on
  // file manager zero. It opens the files serially and writes one value at the
  // outside corner of every variable to resize the dimensions, and then closes
  // the files.  Then it calls writeFiles on all PEs.  They write the data
  // independently and contribute to an empty reduction.
  //
  // Parameters:
  //
  // directorySize  - The size of the array passed in to directory.
  // directory      - The directory in which to write the files.
  // writeGeometry  - Whether to write a new instance into the geometry file.
  //                  If false, the last exsisting instance is used as the
  //                  instance to write into the state file and it is an error
  //                  if no instance exists.
  // writeParameter - Whether to write a new instance into the parameter file.
  //                  If false, the last exsisting instance is used as the
  //                  instance to write into the state file and it is an error
  //                  if no instance exists.
  // writeState     - Whether to write a new instance into the state file.
  void resizeUnlimitedDimensions(size_t directorySize, const char* directory, bool writeGeometry, bool writeParameter, bool writeState);

  // Write data to NetCDF files.  Files must already be created.  When done the
  // file managers contribute to an empty reduction.
  //
  // Parameters:
  //
  // directorySize  - The size of the array passed in to directory.
  // directory      - The directory in which to write the files.
  // writeGeometry  - Whether to write a new instance into the geometry file.
  //                  If false, the last exsisting instance is used as the
  //                  instance to write into the state file and it is an error
  //                  if no instance exists.
  // writeParameter - Whether to write a new instance into the parameter file.
  //                  If false, the last exsisting instance is used as the
  //                  instance to write into the state file and it is an error
  //                  if no instance exists.
  // writeState     - Whether to write a new instance into the state file.
  void writeFiles(size_t directorySize, const char* directory, bool writeGeometry, bool writeParameter, bool writeState);

  int globalNumberOfMeshNodes;       // Number of mesh nodes across all file managers.
  int localMeshNodeStart;            // Index of first mesh node owned by this local branch.
  int localNumberOfMeshNodes;        // Number of mesh nodes owned by this local branch.
  int globalNumberOfMeshElements;    // Number of mesh elements across all file managers.
  int localMeshElementStart;         // Index of first mesh element owned by this local branch.
  int localNumberOfMeshElements;     // Number of mesh elements owned by this local branch.
  int globalNumberOfChannelNodes;    // Number of channel nodes across all file managers.
  int localChannelNodeStart;         // Index of first channel node owned by this local branch.
  int localNumberOfChannelNodes;     // Number of channel nodes owned by this local branch.
  int globalNumberOfChannelElements; // Number of channel elements across all file managers.
  int localChannelElementStart;      // Index of first channel element owned by this local branch.
  int localNumberOfChannelElements;  // Number of channel elements owned by this local branch.
  
  // The following are pointers to dynamically allocated arrays containing the
  // data owned by this local branch.  The pointers can be NULL indicating the
  // data is not available.  Elements must check that the data they need to
  // initialize themselves is available.
  
  // Nodes are a list of points indexed by node number.  A node may be a vertex
  // for multiple elements.  As such, it is not guaranteed that all of an
  // element's nodes are owned by a single local branch.  Therefore, they are
  // not directly used by the elements.  Instead, each local branch caches the
  // coordinates of the vertices of the elements it owns even though this
  // information is redundant.
  double* meshNodeX;
  double* meshNodeY;
  double* meshNodeZSurface;
  double* meshNodeZBedrock;
  
  // This array stores the node indices of the vertices of each element.
  intarraymmn* meshElementVertices;
  
  // These arrays store the coordinates of the vertices of each element.
  // Even this information is not used directly by the elements.  Instead, each
  // local branch calculates element values dervied from the vertex coordinates.
  doublearraymmn* meshVertexX;
  doublearraymmn* meshVertexY;
  doublearraymmn* meshVertexZSurface;
  doublearraymmn* meshVertexZBedrock;
  
  // These arrays store the values used directly by the elements.
  double*         meshElementX;
  double*         meshElementY;
  double*         meshElementZSurface;
  double*         meshElementZBedrock;
  double*         meshElementArea;
  double*         meshElementSlopeX;
  double*         meshElementSlopeY;
  int*            meshCatchment;
  double*         meshConductivity;
  double*         meshPorosity;
  double*         meshManningsN;
  double*         meshSurfacewaterDepth;
  double*         meshSurfacewaterError;
  double*         meshGroundwaterHead;
  double*         meshGroundwaterError;
  intarraymmn*    meshMeshNeighbors;
  boolarraymmn*   meshMeshNeighborsChannelEdge;
  doublearraymmn* meshMeshNeighborsEdgeLength;
  doublearraymmn* meshMeshNeighborsEdgeNormalX;
  doublearraymmn* meshMeshNeighborsEdgeNormalY;
  intarraymcn*    meshChannelNeighbors;
  doublearraymcn* meshChannelNeighborsEdgeLength;
  
  // Nodes are a list of points indexed by node number.  A node may be a vertex
  // for multiple elements.  As such, it is not guaranteed that all of an
  // element's nodes are owned by a single local branch.  Therefore, they are
  // not directly used by the elements.  Instead, each local branch caches the
  // coordinates of the vertices of the elements it owns even though this
  // information is redundant.
  double* channelNodeX;
  double* channelNodeY;
  double* channelNodeZBank;
  double* channelNodeZBed;
  
  // This array stores the node indices of the vertices of each element, but
  // with some special properties required for display as an XDMF file.
  // We want to display streams as polylines and waterbodies as polygons.
  // In XDMF this requires a mixed topology.  In a mixed topology, each element
  // must store its shape type, either 2 for polyline or 3 for polygon, and
  // number of vertices followed by the node indices of the vertices.  In order
  // for the mixed topology to work with the rectangular arrays of NetCDF files
  // all elements must have the same number of vertices.  Therefore,
  // channelElementVertices[n][0] is the shape type, always 2 or 3,
  // channelElementVertices[n][1] is the number of vertices, always
  // ChannelElement::channelVerticesSize, and the remaining values are the node
  // indices of ther vertices with channelElementVertices[n][2] holding vertex
  // 0, channelElementVertices[n][3] holding vertex 1, etc.  If a shape has
  // fewer vertices than ChannelElement::channelVerticesSize then the last
  // vertex is repeated as necessary.
  intarrayxdmf* channelElementVertices;
  
  // These arrays store the coordinates of the vertices of each element.
  // Currently, this information is not used at all.  Channel element
  // information is derived in preprocessing and there is no code in the file
  // manager for deriving that information from vertices.  That could change in
  // the future.
  doublearraycvn* channelVertexX;
  doublearraycvn* channelVertexY;
  doublearraycvn* channelVertexZBank;
  doublearraycvn* channelVertexZBed;
  
  // These arrays store the values used directly by the elements.
  double*          channelElementX;
  double*          channelElementY;
  double*          channelElementZBank;
  double*          channelElementZBed;
  double*          channelElementLength;
  ChannelTypeEnum* channelChannelType;
  int*             channelPermanentCode;
  double*          channelBaseWidth;
  double*          channelSideSlope;
  double*          channelBedConductivity;
  double*          channelBedThickness;
  double*          channelManningsN;
  double*          channelSurfacewaterDepth;
  double*          channelSurfacewaterError;
  intarrayccn*     channelChannelNeighbors;
  intarraycmn*     channelMeshNeighbors;
  doublearraycmn*  channelMeshNeighborsEdgeLength;
  
  // Time information.  This is only kept up to date when inputting or outputting.
  double          currentTime; // Seconds.
  double          dt;          // Next timestep duration in seconds.
  CMK_REFNUM_TYPE iteration;   // Iteration number to put on all messages this timestep.
  
private:
  
  // Calculate which items this file manager owns.  The first
  // (globalNumberOfItems % CkNumPes()) file managers each have
  // (globalNumberOfItems / CkNumPes() + 1) items.  The remaining file managers
  // each have (globalNumberOfItems / CkNumPes()) items.
  //
  // Parameters:
  //
  // localItemStart      - Scalar passed by reference will be filled in with
  //                       the local start index.
  // localNumberOfItems  - Scalar passed by reference will be filled in with
  //                       the local number of items.
  // globalNumberOfItems - The total number of this kind of item.
  static void localStartAndNumber(int* localItemStart, int* localNumberOfItems, int globalNumberOfItems);
  
  // Returns: true if all vertex information is updated, false otherwise.
  bool allVerticesUpdated();
  
  // Returns: true if all element information is updated, false otherwise.
  bool allElementsUpdated();
  
  // Receive requested vertex coordinates.
  //
  // Parameters:
  //
  // element  - Element that has vertex.
  // vertex   - Vertex that coordinates are for.
  // x        - X coordinate of vertex.
  // y        - Y coordinate of vertex.
  // zSurface - Surface Z coordinate of vertex.
  // zBedrock - Bedrock Z coordinate of vertex.
  void handleMeshVertexDataMessage(int element, int vertex, double x, double y, double zSurface, double zBedrock);
  
  // Calculating derived values had to be split into two functions because file
  // managers may have to enter SDAG code to send and receive messages with
  // vertex coordinates in between.  This function finishes the work of
  // calculateDerivedValues.
  void finishCalculateDerivedValues();
  
  // These arrays are used to record when vertex data and state update messages are received.
  boolarraymmn* meshVertexUpdated;
  bool*         meshElementUpdated;
  bool*         channelElementUpdated;
};

#endif // __FILE_MANAGER_H__
