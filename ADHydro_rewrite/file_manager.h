#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "region.h"

// An ElementStateMessage is used for regions to send updated element state to
// file managers for writing out to file.
class ElementStateMessage
{
public:
  
  // Default constructor.  Only needed for pup_stl.h code.
  ElementStateMessage();
  
  // Constructor.
  //
  // All parameters directly initialize member variables.  For description see
  // member variables.
  ElementStateMessage(int elementNumberInit, double surfacewaterDepthInit, double surfacewaterErrorInit, double groundwaterHeadInit,
                      double groundwaterRechargeInit, double groundwaterErrorInit, double precipitationRateInit,
                      double precipitationCumulativeShortTermInit, double precipitationCumulativeLongTermInit, double evaporationRateInit,
                      double evaporationCumulativeShortTermInit, double evaporationCumulativeLongTermInit, double transpirationRateInit,
                      double transpirationCumulativeShortTermInit, double transpirationCumulativeLongTermInit,
                      EvapoTranspirationStateStruct& evapoTranspirationStateInit);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  // Check invariant conditions.
  //
  // Returns: true if the invariant is violated, false otherwise.
  bool checkInvariant();
  
  int                             elementNumber;
  double                          surfacewaterDepth;
  double                          surfacewaterError;
  double                          groundwaterHead;
  double                          groundwaterRecharge;
  double                          groundwaterError;
  double                          precipitationRate;
  double                          precipitationCumulativeShortTerm;
  double                          precipitationCumulativeLongTerm;
  double                          evaporationRate;
  double                          evaporationCumulativeShortTerm;
  double                          evaporationCumulativeLongTerm;
  double                          transpirationRate;
  double                          transpirationCumulativeShortTerm;
  double                          transpirationCumulativeLongTerm;
  EvapoTranspirationStateStruct   evapoTranspirationState;
  // FIXME vadose zone state
  std::vector<simpleNeighborInfo> surfacewaterMeshNeighbors;
  std::vector<simpleNeighborInfo> groundwaterMeshNeighbors;
  std::vector<simpleNeighborInfo> surfacewaterChannelNeighbors;
  std::vector<simpleNeighborInfo> groundwaterChannelNeighbors;
};

// file_manager.decl.h has to be included after ElementStateMessage because it passes parameters of type ElementStateMessage.
#include "file_manager.decl.h"
#include <netcdf.h>

// MESH_ELEMENT_VERTICES_SIZE is always equal to MESH_ELEMENT_MESH_NEIGHBORS_SIZE
#define MESH_ELEMENT_MESH_NEIGHBORS_SIZE (3)
#define MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE (16)
#define CHANNEL_ELEMENT_VERTICES_SIZE (130)
#define XDMF_SIZE (CHANNEL_ELEMENT_VERTICES_SIZE + 2) // channelElementVertices must have two extra values of XDMF metadata.
#define CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE (66)
#define CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE (18)

typedef int       IntArrayMMN[MESH_ELEMENT_MESH_NEIGHBORS_SIZE];          // Fixed size array of ints.       Size is mesh    mesh neighbors.
typedef double    DoubleArrayMMN[MESH_ELEMENT_MESH_NEIGHBORS_SIZE];       // Fixed size array of doubles.    Size is mesh    mesh neighbors.
typedef bool      BoolArrayMMN[MESH_ELEMENT_MESH_NEIGHBORS_SIZE];         // Fixed size array of bools.      Size is mesh    mesh neighbors.
typedef int       IntArrayMCN[MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];       // Fixed size array of ints.       Size is mesh    channel neighbors.
typedef double    DoubleArrayMCN[MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];    // Fixed size array of doubles.    Size is mesh    channel neighbors.
typedef bool      BoolArrayMCN[MESH_ELEMENT_CHANNEL_NEIGHBORS_SIZE];      // Fixed size array of bools.      Size is mesh    channel neighbors.
typedef int       IntArrayXDMF[XDMF_SIZE];                                // Fixed size array of ints.       Size is channel vertices + 2.  This is used only
                                                                          // for channelElementVertices.  See the comment of that variable for explanation.
typedef double    DoubleArrayCV[CHANNEL_ELEMENT_VERTICES_SIZE];           // Fixed size array of doubles.    Size is channel vertices.
typedef bool      BoolArrayCV[CHANNEL_ELEMENT_VERTICES_SIZE];             // Fixed size array of bools.      Size is channel vertices.
typedef int       IntArrayCMN[CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];       // Fixed size array of ints.       Size is channel mesh neighbors.
typedef double    DoubleArrayCMN[CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];    // Fixed size array of doubles.    Size is channel mesh neighbors.
typedef bool      BoolArrayCMN[CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];      // Fixed size array of bools.      Size is channel mesh neighbors.
typedef int       IntArrayCCN[CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];    // Fixed size array of ints.       Size is channel channel neighbors.
typedef double    DoubleArrayCCN[CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE]; // Fixed size array of doubles.    Size is channel channel neighbors.
typedef bool      BoolArrayCCN[CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];   // Fixed size array of bools.      Size is channel channel neighbors.
typedef long long LongLongArrayPair[2];                                   // Fixed size array of long long.  Size is 2.

// The group of file managers acts as an in-memory cache for values that are
// read from and written to NetCDF files.  Reading and writing individual
// values from NetCDF files is too slow.  We need to read and write large
// blocks from the arrays in the NetCDF files.  However, individual values are
// needed by mesh and channel elements.  The file manager group does the block
// reads and writes and makes the individual values available to the elements.
//
// At initialization, each file manager takes ownership of a block of each
// array.  This gives each file manager ownership of the information about a
// set of elements.  Then each file manager sends initialization messages for
// the elements it owns.  At output intervals, each element sends a state
// update back to the file manager that owns it.  When all of the state updates
// have been received the file managers collectively write the arrays to file.
class FileManager : public CBase_FileManager
{
  FileManager_SDAG_CODE
  
public:
  
  // The wallclock time when the simulation starts for printing elapsed time.
  static double wallclockTimeAtStart;
  
  // The simulation time in seconds since ADHydro::referenceDate of the last
  // mass balance reduction to have completed.
  static double massBalanceTime;
  
  // The mass balance value produced the first time the mass balance completes.
  // This is saved as a "should be" value for the rest of the simulation.
  static double massBalanceShouldBe;
  
  // Print out the mass balance check.  This is a reduction target that
  // collects all mass balance values from all regions.  To calculate the
  // mass balance take waterInDomain and add externalFlows and subtract
  // waterError.  This will undo any insertion or removal of water from the
  // "black box" of the simulation domain leaving the amount of water that was
  // present when externalFlows and waterError were both zero.  This value
  // should be invariant except for floating point roundoff error.
  //
  // Parameters:
  //
  // messageTime   - The simulation time in seconds since
  //                 ADHydro::referenceDate when the mass balance was
  //                 collected.
  // waterInDomain - The amount of water in cubic meters in the entire domain.
  //                 Positive means the existance of water.  Must be
  //                 non-negative.
  // externalFlows - The amount of water in cubic meters that has flowed to or
  //                 from external sources and sinks (boundary conditions,
  //                 precipitation, E-T, etc.) for the entire domain.  Positive
  //                 means flow out of the domain.  Negative means flow into
  //                 the domain.
  // waterError    - The amount of water in cubic meters that was created or
  //                 destroyed be error in the entire domain.  Positive means
  //                 water was created.  Negative means water was destroyed.
  static void printOutMassBalance(double messageTime, double waterInDomain, double externalFlows, double waterError);

  // Calculate which file manager owns a given item.  Items are generally mesh
  // elements, although this function can be used to distribute any set of
  // items among file managers.  The first (globalNumberOfItems % CkNumPes())
  // file managers each have (globalNumberOfItems / CkNumPes() + 1) items.
  // The remaining file managers each have (globalNumberOfItems / CkNumPes())
  // items.
  //
  // Returns: the index of the file manager that owns item.
  //
  // Parameters:
  //
  // item                - The index of this item.
  // globalNumberOfItems - The total number of this kind of item.
  static int home(int item, int globalNumberOfItems);
  
  // Calculate which items this file manager owns.  Items are generally mesh
  // elements, although this function can be used to distribute any set of
  // items among file managers.  The first (globalNumberOfItems % CkNumPes())
  // file managers each have (globalNumberOfItems / CkNumPes() + 1) items.
  // The remaining file managers each have (globalNumberOfItems / CkNumPes())
  // items.
  //
  // Parameters:
  //
  // localItemStart      - Scalar passed by reference will be filled in with
  //                       the local start index.
  // localNumberOfItems  - Scalar passed by reference will be filled in with
  //                       the local number of items.
  // globalNumberOfItems - The total number of this kind of item.
  static void localStartAndNumber(int* localItemStart, int* localNumberOfItems, int globalNumberOfItems);
  
  // Constructor.  Initializes the file manager to hold no data and starts the
  // runUntilSimulationEnd function in the SDAG code.
  FileManager();
  
  // Destructor.  Dynamically allocated arrays need to be deleted.
  ~FileManager();
  
  // This function pulls out some duplicate code that is the same for reading
  // the mesh and channels .node and .z files.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // readMesh - If true read the mesh node and z files.  If false read the
  //            channel node and z files.
  bool readNodeAndZFiles(bool readMesh);
  
  // Initialize the file manager from ASCII files.
  //
  // ASCII files do not contain all of the file manager variables so some will
  // be set to defaults in calculateDerivedValues.
  void initializeFromASCIIFiles();
  
  // Open a NetCDF file in read-only mode.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // path   - String containing the file path including filename of the file to
  //          open.
  // fileID - Scalar passed by reference will be filled in with an ID number
  //          generated by the NetCDF library for the file.  Pass this number
  //          in to other NetCDF calls to use this file.
  bool NetCDFOpenForRead(const char* path, int* fileID);
  
  // If path exists open it as a NetCDF file in read-write mode.  If path does
  // not exist create it as a new NetCDF file, which will then be open in
  // read-write mode.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // path    - String containing the file path including filename of the file
  //           to create or open.
  // fileID  - Scalar passed by reference will be filled in with an ID number
  //           generated by the NetCDF library for the file.  Pass this number
  //           in to other NetCDF calls to use this file.
  // created - Scalar passed by reference will be filled in with true if the
  //           file was created and false if it was merely opened.  Can be
  //           passed as NULL in which case it will be ignored.
  bool NetCDFCreateOrOpenForWrite(const char* path, int* fileID, bool* created);
  
  // Create or open the geometry file in read-write mode.  If the file is
  // created all of the structures that should be in the geometry file are
  // created as well.  The path used is ADHydro::adhydroOutputGeometryFilePath.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID - Scalar passed by reference will be filled in with an ID number
  //          generated by the NetCDF library for the file.  Pass this number
  //          in to other NetCDF calls to use this file.
  bool NetCDFCreateOrOpenForWriteGeometry(int* fileID);
  
  // Create or open the parameter file in read-write mode.  If the file is
  // created all of the structures that should be in the parameter file are
  // created as well.  The path used is ADHydro::adhydroOutputParameterFilePath.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID - Scalar passed by reference will be filled in with an ID number
  //          generated by the NetCDF library for the file.  Pass this number
  //          in to other NetCDF calls to use this file.
  bool NetCDFCreateOrOpenForWriteParameter(int* fileID);
  
  
  // Create or open the state file in read-write mode.  If the file is
  // created all of the structures that should be in the state file are
  // created as well.  The path used is ADHydro::adhydroOutputStateFilePath.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID - Scalar passed by reference will be filled in with an ID number
  //          generated by the NetCDF library for the file.  Pass this number
  //          in to other NetCDF calls to use this file.
  bool NetCDFCreateOrOpenForWriteState(int* fileID);
  
  
  // Create or open the display file in read-write mode.  If the file is
  // created all of the structures that should be in the display file are
  // created as well.  The path used is ADHydro::adhydroOutputDisplayFilePath.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID - Scalar passed by reference will be filled in with an ID number
  //          generated by the NetCDF library for the file.  Pass this number
  //          in to other NetCDF calls to use this file.
  bool NetCDFCreateOrOpenForWriteDisplay(int* fileID);
  
  // Create a dimension in a NetCDF file.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID        - The file ID of the NetCDF file.
  // dimensionName - The name of the dimension to create.
  // dimensionSize - The size of the dimension to create.
  // dimensionID   - Scalar passed by reference will be filled in with the ID
  //                 of the created dimension.
  bool NetCDFCreateDimension(int fileID, const char* dimensionName, size_t dimensionSize, int* dimensionID);
  
  // Get the size of a dimension in a NetCDF file.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID        - The file ID of the NetCDF file.
  // dimensionName - The name of the dimension to get the size of.
  // dimensionSize - Scalar passed by reference will be filled in with the size
  //                 of the dimension.
  bool NetCDFReadDimensionSize(int fileID, const char* dimensionName, size_t* dimensionSize);
  
  // Create a variable in a NetCDF file.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID             - The file ID of the NetCDF file.
  // variableName       - The name of the variable to create.
  // dataType           - The type of the variable to create.
  // numberOfDimensions - The number of dimensions of the variable to create.
  // dimensionID0       - The ID of the first dimension of the variable.
  // dimensionID1       - The ID of the second dimension of the variable.
  //                      Ignored if numberOfDimensions is less than two.
  // dimensionID2       - The ID of the third dimension of the variable.
  //                      Ignored if numberOfDimensions is less than three.
  // units              - A units string that will be added as an attribute of
  //                      the variable.  Can be passed as NULL in which case no
  //                      attribute will be added.
  // comment            - A comment string that will be added as an attribute
  //                      of the variable.  Can be passed as NULL in which case
  //                      no attribute will be added.
  bool NetCDFCreateVariable(int fileID, const char* variableName, nc_type dataType, int numberOfDimensions, int dimensionID0, int dimensionID1,
                            int dimensionID2, const char* units, const char* comment);

  // Read a variable from a NetCDF file.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID                - The file ID of the NetCDF file.
  // variableName          - The name of the variable to read.
  // instance              - The index of the first dimension to read.
  //                         The count of the first dimension is always one.
  //                         This reads one particular instance in time.
  // nodeElementStart      - The index of the second dimension to read.  This
  //                         is ignored if the variable has less than two
  //                         dimensions.
  // numberOfNodesElements - The count of the second dimension to read.  If the
  //                         variable has less than two dimensions this must be
  //                         one. nodeElementStart and numberOfNodesElements
  //                         combine to specify a subset of the nodes or
  //                         elements stored in the variable.
  // fileDimension         - The count of the third dimension to read.  The
  //                         index of the third dimension is always zero.  If
  //                         the variable has less than three dimensions this
  //                         must be one.
  // memoryDimension       - The size of the third dimension in memory.  This
  //                         function can read an array whose third dimension
  //                         in the file is smaller than the desired third
  //                         dimension in memory.  In that case, what gets
  //                         filled in to the extra cells depends on
  //                         repeatLastValue and defaultValue.  It is an error
  //                         if memoryDimension is less than fileDimension.  If
  //                         the variable has less than three dimensions this
  //                         must be one.
  // repeatLastValue       - If there are extra cells to be filled in because
  //                         memoryDimension is greater than fileDimension then
  //                         if repeatLastValue is true the last value in each
  //                         row of the third dimension in the file is repeated
  //                         in the extra cells.  If repeatLastValue is false,
  //                         defaultValue is used instead.
  // defaultValue          - If there are extra cells to be filled in because
  //                         memoryDimension is greater than fileDimension then
  //                         if repeatLastValue is false the extra cells are
  //                         filled in with defaultValue.  If repeatLastValue
  //                         is true defaultValue is ignored.
  // mandatory             - Whether the existence of the variable is
  //                         mandatory.  If true, it is an error if the
  //                         variable does not exist.  If false, this function
  //                         does nothing if the variable does not exist.
  // variable              - A pointer passed by reference.  The pointer (that
  //                         is, *variable) may point to an array of size 1 *
  //                         numberOfNodesElements * fileDimension, which is
  //                         the size of the array that will be read, or it can
  //                         be NULL.  If it is NULL it will be set to point to
  //                         a newly allocated array.  This array, whether
  //                         passed in or newly allocated is filled in with the
  //                         values read from the NetCDF file.  Then, if
  //                         memoryDimension is greater than fileDimension it
  //                         is reallocated to the larger size with new[] and
  //                         delete[].  NOTE: even if you pass in an array, it
  //                         will be deleted and *variable will be set to point
  //                         to a newly allocated array if memoryDimension is
  //                         greater than fileDimension.  Therefore, the array
  //                         you pass in must be allocated with new, but this
  //                         will only happen if memoryDimension is greater
  //                         than fileDimension.  In any case, *variable will
  //                         wind up pointing to an array of size 1 *
  //                         numberOfNodesElements * memoryDimension.
  template <typename T> bool NetCDFReadVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart,
                                                size_t numberOfNodesElements, size_t fileDimension, size_t memoryDimension, bool repeatLastValue,
                                                T defaultValue, bool mandatory, T** variable);
  
  // Read which instances to use in the NetCDF files.
  //
  // Returns: true if there is an error, false otherwise.
  bool NetCDFReadInstances();
  
  // Read variables from the geometry file.  geometryInstance must already be
  // set to the instance to read.
  //
  // Returns: true if there is an error, false otherwise.
  bool NetCDFReadGeometry();
  
  // Read variables from the parameter file.  parameterInstance must already be
  // set to the instance to read.
  //
  // Returns: true if there is an error, false otherwise.
  bool NetCDFReadParameter();
  
  // Read variables from the state file.  stateInstance must already be
  // set to the instance to read.
  //
  // Returns: true if there is an error, false otherwise.
  bool NetCDFReadState();
  
  // Write a variable to a NetCDF file.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID                - The file ID of the NetCDF file.
  // variableName          - The name of the variable to write.
  // instance              - The index of the first dimension to write.
  //                         The count of the first dimension is always one.
  //                         This writes one particular instance in time.
  // nodeElementStart      - The index of the second dimension to write.
  // numberOfNodesElements - The count of the second dimension to write.
  //                         nodeElementStart and numberOfNodesElements combine
  //                         to specify a subset of the nodes or elements
  //                         stored in the variable.  They are ignored if the
  //                         variable has less than two dimensions.
  // memoryDimension       - The count of the third dimension to write.  This
  //                         is ignored if the variable has less than three
  //                         dimensions.
  // variable              - An array of size 1 * numberOfNodesElements *
  //                         memoryDimension which will be written into the
  //                         variable in the file.
  bool NetCDFWriteVariable(int fileID, const char* variableName, size_t instance, size_t nodeElementStart, size_t numberOfNodesElements,
                           size_t memoryDimension, void* variable);
  
  // Write variables to the geometry file.  geometryInstance must already be
  // set to the instance to write.
  //
  // Returns: true if there is an error, false otherwise.
  bool NetCDFWriteGeometry();
  
  // Write variables to the parameter file.  parameterInstance must already be
  // set to the instance to write.
  //
  // Returns: true if there is an error, false otherwise.
  bool NetCDFWriteParameter();
  
  // Write variables to the state file.  stateInstance must already be
  // set to the instance to write.
  //
  // Returns: true if there is an error, false otherwise.
  bool NetCDFWriteState();
  
  // Write variables to the display file.  displayInstance must already be
  // set to the instance to write.
  bool NetCDFWriteDisplay();
  
  // Initialize the file manager from NetCDF files.
  void initializeFromNetCDFFiles();
  
  // Write out file manager data to NetCDF files.
  void writeNetCDFFiles();
  
  // Check if vertex variables need to be updated from node variables and if so
  // send out get vertex data messages.
  void updateVertices();
  
  // Returns: true if all vertex information is updated, false otherwise.
  bool allVerticesUpdated();
  
  // Send requested vertex coordinates.
  //
  // Parameters:
  //
  // requester - File manager requesting vertex coordinates.
  // node      - Node index of vertex.
  void getMeshVertexDataMessage(int requester, int node);
  
  // Send requested vertex coordinates.
  //
  // Parameters:
  //
  // requester - File manager requesting vertex coordinates.
  // node      - Node index of vertex.
  void getChannelVertexDataMessage(int requester, int node);
  
  // Receive requested vertex coordinates.
  //
  // Parameters:
  //
  // node     - Node index of vertex.
  // x        - X coordinate of vertex.
  // y        - Y coordinate of vertex.
  // zSurface - Surface Z coordinate of vertex.
  void handleMeshVertexDataMessage(int node, double x, double y, double zSurface);
  
  // Receive requested vertex coordinates.
  //
  // Parameters:
  //
  // node    - Node index of vertex.
  // x       - X coordinate of vertex.
  // y       - Y coordinate of vertex.
  // zBank   - Bank Z coordinate of vertex.
  void handleChannelVertexDataMessage(int node, double x, double y, double zBank);
  
  // Fix problems with mesh elements having invalid vegetation or soil type.
  // This is a different function from meshMassage because it has to be called
  // further up in calculateDerivedValues before Mannings's N, conductivity,
  // and porosity are determined from vegetation and soil type.
  void meshMassageVegetationAndSoilType();
  
  // To break digital dams in the mesh we arbitrarily connect mesh elements to
  // the lowest channel element from the same catchment.  Catchment numbers for
  // streams are stored in the channel element's reach code.  If a stream is
  // pruned we may not find a channel element with that reach code in which
  // case the return value indicates that no connection was made.  The caller
  // should then find the reach code of an unpruned link downstream of the
  // pruned stream and call this function again with that new reach code.
  //
  // Returns: The channel element number that meshElement was connected to, or
  //          NOFLOW if no connection was made.
  //
  // Parameters:
  //
  // meshElement - The mesh element to connect to a channel element.
  // reachCode   - The reach code of a channel element to connect to.
  int breakMeshDigitalDam(int meshElement, long long reachCode);
  
  // Returns: A channel element immediately downstream of channelElement, or
  //          OUTFLOW if channelElement has an outflow boundary, or NOFLOW if
  //          channelElement has neither.  If channelElement has more than one
  //          qualifying downstream channel element or a downstream channel
  //          element and an outflow boundary the first one encountered is
  //          returned arbitrarily.
  //
  // Parameters:
  //
  // channelElement - The channel element to find a channel element or outflow
  //                  boundary downstream of.
  int findDownstreamChannelElement(int channelElement);
  
  // To break digital dams in the channel network we first search recursively
  // downstream from the dammed channel element to find a channel element lower
  // than the dammed channel element.  All channel elements between the dammed
  // channel element and the lower channel element have their ZBed lowered to
  // be on a straight line slope between the two.
  //
  // Waterbodies are allowed to be dammed so if channelElement is a waterbody
  // then its ZBed is lowered to match dammedChannelElement and it is used as
  // the lower element.  This is also done if downstream of channelElement
  // there is an outflow boundary or if channelElement has no downstream
  // connections at all.
  //
  // The downstream search is done recursively by arbitrarily choosing the
  // first downstream connection of channelElement.  This function could be
  // improved for cases where elements have multiple downstream connections.
  //
  // Returns: The slope from the dammed channel element to the lower channel
  //          element.
  //
  // Parameters:
  //
  // channelElement       - The channel element to check for being the lower
  //                        element or to lower its ZBed if it is not.
  // dammedChannelElement - The original channel element that has the digital
  //                        dam.
  // length               - The length in meters traversed so far to the
  //                        beginning of channelElement.
  double breakChannelDigitalDam(int channelElement, int dammedChannelElement, double length);
  
  // Fix digital dams and similar problems.
  //
  // This function does some things that need information about neighbors.
  // Currently it is set up to run only on one process so that all neighbor
  // information is available locally.  FIXLATER Eventually, it could be set up
  // for distributed operation like the vertex information swap.
  void meshMassage();
  
  // If a variable is not available the file managers attempt to derive it from
  // other variables, for example, element center coordinates can be derived
  // from element vertex coordinates.  If a variable is not available and
  // cannot be derived its array is unallocated and left as NULL.
  void calculateDerivedValues();
  
  // Send mesh and channel element initialization messages to regions.
  //
  // Parameters:
  //
  // regionProxy - A proxy to the chare array of regions.
  void handleSendInitializationMessages(CProxy_Region regionProxy);
  
  // Returns: true if all element information is updated, false otherwise.
  bool allStateUpdated();
  
  // Store the updated state received from a mesh element.
  //
  // Parameters:
  //
  // message - The updated state.
  void handleMeshElementStateMessage(ElementStateMessage& message);
  
  // Store the updated state received from a channel element.
  //
  // Parameters:
  //
  // message - The updated state.
  void handleChannelElementStateMessage(ElementStateMessage& message);
  
  int globalNumberOfRegions;         // Number of regions across all file managers.
  int localRegionStart;              // Index of first region owned by this local branch.
  int localNumberOfRegions;          // Number of regions owned by this local branch.
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
  // data is not available.  The arrays are indexed by a local index, which is
  // globalItemNumber - localItemStart where item can be Region, MeshNode,
  // MeshElement, ChannelNode, or ChannelElement.
  
  // Each region has to know how many element initialization messages to
  // expect.
  int* regionNumberOfMeshElements;
  int* regionNumberOfChannelElements;
  
  // Nodes are a list of points indexed by node number.  A node may be a vertex
  // for multiple elements.  As such, it is not guaranteed that all of an
  // element's nodes are owned by a single local branch.  Therefore, they are
  // not directly used by the elements.  Instead, each local branch caches the
  // coordinates of the vertices of the elements it owns even though this
  // information is redundant.
  double* meshNodeX;
  double* meshNodeY;
  double* meshNodeZSurface;
  
  // This array stores the node indices of the vertices of each element.
  IntArrayMMN* meshElementVertices;
  
  // These arrays store the coordinates of the vertices of each element.
  // Even this information is not used directly by the elements.  Instead, each
  // local branch calculates element values dervied from the vertex coordinates.
  DoubleArrayMMN* meshVertexX;
  DoubleArrayMMN* meshVertexY;
  DoubleArrayMMN* meshVertexZSurface;
  
  // These arrays store the values used directly by the elements.
  int*            meshRegion;
  int*            meshCatchment;
  int*            meshVegetationType;
  int*            meshSoilType;
  bool*           meshAlluvium;
  double*         meshElementX;
  double*         meshElementY;
  double*         meshElementZSurface;
  double*         meshElementSoilDepth;
  double*         meshElementLayerZBottom;
  double*         meshElementArea;
  double*         meshElementSlopeX;
  double*         meshElementSlopeY;
  double*         meshLatitude;
  double*         meshLongitude;
  double*         meshManningsN;
  double*         meshConductivity;
  double*         meshPorosity;
  double*         meshSurfacewaterDepth;
  double*         meshSurfacewaterError;
  double*         meshGroundwaterHead;
  double*         meshGroundwaterRecharge;
  double*         meshGroundwaterError;
  double*         meshPrecipitationRate;
  double*         meshPrecipitationCumulativeShortTerm;
  double*         meshPrecipitationCumulativeLongTerm;
  double*         meshEvaporationRate;
  double*         meshEvaporationCumulativeShortTerm;
  double*         meshEvaporationCumulativeLongTerm;
  double*         meshTranspirationRate;
  double*         meshTranspirationCumulativeShortTerm;
  double*         meshTranspirationCumulativeLongTerm;
  EvapoTranspirationStateStruct*
                  meshEvapoTranspirationState;
  InfiltrationAndGroundwater::GroundwaterMethodEnum*
                  meshGroundwaterMethod;
  InfiltrationAndGroundwater::VadoseZone*
                  meshVadoseZone;
  IntArrayMMN*    meshMeshNeighbors;
  IntArrayMMN*    meshMeshNeighborsRegion;
  BoolArrayMMN*   meshMeshNeighborsChannelEdge;
  DoubleArrayMMN* meshMeshNeighborsEdgeLength;
  DoubleArrayMMN* meshMeshNeighborsEdgeNormalX;
  DoubleArrayMMN* meshMeshNeighborsEdgeNormalY;
  BoolArrayMMN*   meshSurfacewaterMeshNeighborsConnection;
  DoubleArrayMMN* meshSurfacewaterMeshNeighborsExpirationTime;
  DoubleArrayMMN* meshSurfacewaterMeshNeighborsFlowRate;
  DoubleArrayMMN* meshSurfacewaterMeshNeighborsFlowCumulativeShortTerm;
  DoubleArrayMMN* meshSurfacewaterMeshNeighborsFlowCumulativeLongTerm;
  BoolArrayMMN*   meshGroundwaterMeshNeighborsConnection;
  DoubleArrayMMN* meshGroundwaterMeshNeighborsExpirationTime;
  DoubleArrayMMN* meshGroundwaterMeshNeighborsFlowRate;
  DoubleArrayMMN* meshGroundwaterMeshNeighborsFlowCumulativeShortTerm;
  DoubleArrayMMN* meshGroundwaterMeshNeighborsFlowCumulativeLongTerm;
  IntArrayMCN*    meshChannelNeighbors;
  IntArrayMCN*    meshChannelNeighborsRegion;
  DoubleArrayMCN* meshChannelNeighborsEdgeLength;
  BoolArrayMCN*   meshSurfacewaterChannelNeighborsConnection;
  DoubleArrayMCN* meshSurfacewaterChannelNeighborsExpirationTime;
  DoubleArrayMCN* meshSurfacewaterChannelNeighborsFlowRate;
  DoubleArrayMCN* meshSurfacewaterChannelNeighborsFlowCumulativeShortTerm;
  DoubleArrayMCN* meshSurfacewaterChannelNeighborsFlowCumulativeLongTerm;
  BoolArrayMCN*   meshGroundwaterChannelNeighborsConnection;
  DoubleArrayMCN* meshGroundwaterChannelNeighborsExpirationTime;
  DoubleArrayMCN* meshGroundwaterChannelNeighborsFlowRate;
  DoubleArrayMCN* meshGroundwaterChannelNeighborsFlowCumulativeShortTerm;
  DoubleArrayMCN* meshGroundwaterChannelNeighborsFlowCumulativeLongTerm;
  
  // Nodes are a list of points indexed by node number.  A node may be a vertex
  // for multiple elements.  As such, it is not guaranteed that all of an
  // element's nodes are owned by a single local branch.  Therefore, they are
  // not directly used by the elements.  Instead, each local branch caches the
  // coordinates of the vertices of the elements it owns even though this
  // information is redundant.
  double* channelNodeX;
  double* channelNodeY;
  double* channelNodeZBank;
  
  // This array stores the node indices of the vertices of each element, but
  // with some extra metadata required for display as an XDMF file.  We want to
  // display streams as polylines and waterbodies as polygons.  In XDMF this
  // requires a mixed topology.  In a mixed topology, each element must store
  // its shape type, either 2 for polyline or 3 for polygon, and number of
  // vertices followed by the node indices of the vertices.  In order for the
  // mixed topology to work with the rectangular arrays of NetCDF files all
  // elements must have the same number of vertices.  Therefore,
  // channelElementVertices[n][0] is the shape type, always 2 or 3,
  // channelElementVertices[n][1] is the number of vertices, always
  // CHANNEL_ELEMENT_CHANNEL_VERTICES_SIZE, and the remaining values are the
  // node indices of the vertices with channelElementVertices[n][2] holding
  // vertex 0, channelElementVertices[n][3] holding vertex 1, etc.  If a shape
  // has fewer vertices than CHANNEL_ELEMENT_CHANNEL_VERTICES_SIZE then the
  // last vertex is repeated as necessary.
  IntArrayXDMF* channelElementVertices;
  
  // These arrays store the coordinates of the vertices of each element.
  // Even this information is not used directly by the elements.  Instead, each
  // local branch calculates element values dervied from the vertex coordinates.
  DoubleArrayCV* channelVertexX;
  DoubleArrayCV* channelVertexY;
  DoubleArrayCV* channelVertexZBank;
  
  // These arrays store the values used directly by the elements.
  int*             channelRegion;
  ChannelTypeEnum* channelChannelType;
  long long*       channelReachCode;
  double*          channelElementX;
  double*          channelElementY;
  double*          channelElementZBank;
  double*          channelElementBankFullDepth;
  double*          channelElementZBed;
  double*          channelElementLength;
  double*          channelLatitude;
  double*          channelLongitude;
  double*          channelBaseWidth;
  double*          channelSideSlope;
  double*          channelBedConductivity;
  double*          channelBedThickness;
  double*          channelManningsN;
  double*          channelSurfacewaterDepth;
  double*          channelSurfacewaterError;
  double*          channelPrecipitationRate;
  double*          channelPrecipitationCumulativeShortTerm;
  double*          channelPrecipitationCumulativeLongTerm;
  double*          channelEvaporationRate;
  double*          channelEvaporationCumulativeShortTerm;
  double*          channelEvaporationCumulativeLongTerm;
  EvapoTranspirationStateStruct*
                   channelEvapoTranspirationState;
  IntArrayCMN*     channelMeshNeighbors;
  IntArrayCMN*     channelMeshNeighborsRegion;
  DoubleArrayCMN*  channelMeshNeighborsEdgeLength;
  BoolArrayCMN*    channelSurfacewaterMeshNeighborsConnection;
  DoubleArrayCMN*  channelSurfacewaterMeshNeighborsExpirationTime;
  DoubleArrayCMN*  channelSurfacewaterMeshNeighborsFlowRate;
  DoubleArrayCMN*  channelSurfacewaterMeshNeighborsFlowCumulativeShortTerm;
  DoubleArrayCMN*  channelSurfacewaterMeshNeighborsFlowCumulativeLongTerm;
  BoolArrayCMN*    channelGroundwaterMeshNeighborsConnection;
  DoubleArrayCMN*  channelGroundwaterMeshNeighborsExpirationTime;
  DoubleArrayCMN*  channelGroundwaterMeshNeighborsFlowRate;
  DoubleArrayCMN*  channelGroundwaterMeshNeighborsFlowCumulativeShortTerm;
  DoubleArrayCMN*  channelGroundwaterMeshNeighborsFlowCumulativeLongTerm;
  IntArrayCCN*     channelChannelNeighbors;
  IntArrayCCN*     channelChannelNeighborsRegion;
  BoolArrayCCN*    channelChannelNeighborsDownstream;
  BoolArrayCCN*    channelSurfacewaterChannelNeighborsConnection;
  DoubleArrayCCN*  channelSurfacewaterChannelNeighborsExpirationTime;
  DoubleArrayCCN*  channelSurfacewaterChannelNeighborsFlowRate;
  DoubleArrayCCN*  channelSurfacewaterChannelNeighborsFlowCumulativeShortTerm;
  DoubleArrayCCN*  channelSurfacewaterChannelNeighborsFlowCumulativeLongTerm;
  
  // This array stores a lookup table of reach codes for pruned streams.
  // channelPruned[n][0] stores the reach code of a pruned stream.
  // channelPruned[n][1] stores the reach code of an unpruned channel that is
  // downstream of the pruned stream and should be used in place of the pruned
  // stream if needed.  This is only available if you initialize from ASCII
  // files and is only used for meshMassage.
  int                channelPrunedSize;     // The size of the allocated array pointed to by channelPruned.
  int                numberOfChannelPruned; // The number of valid entries in channelPruned.
  LongLongArrayPair* channelPruned;         // The lookup table of reach codes for pruned streams.
  
  // These arrays are used to record when vertex data and state update messages are received.
  std::map< int, std::vector< std::pair< int, int > > > meshNodeLocation;
  std::map< int, std::vector< std::pair< int, int > > > channelNodeLocation;
  BoolArrayMMN*                                         meshVertexUpdated;
  BoolArrayCV*                                          channelVertexUpdated;
  bool*                                                 meshElementUpdated;
  bool*                                                 channelElementUpdated;
  
  // These record which instance to use in NetCDF files.
  size_t geometryInstance;
  bool   geometryInstanceChecked;
  bool   geometryChanged;
  size_t parameterInstance;
  bool   parameterInstanceChecked;
  bool   parameterChanged;
  size_t stateInstance;
  size_t displayInstance;
  
  // Time and simulation control variables.
  double currentTime;         // Current simulation time in seconds since ADHydro::referenceDate.
  int    nextCheckpointIndex; // This multiplied by ADHydro::checkpointPeriod is the next time that a checkpoint will be done.
  int    nextOutputIndex;     // This multiplied by ADHydro::outputPeriod is the next time that an output will be done.
  double simulationEndTime;   // Simulation time to end the simulation in seconds since ADHydro::referenceDate.
  bool   simulationFinished;  // Flag to indicate the simulation is finished.
};

#endif // __FILE_MANAGER_H__
