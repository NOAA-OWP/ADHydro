#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "adhydro_netcdf.h"
#include "region.h"
#include "ReservoirFactory.h"
#include "DiversionFactory.h"


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
                      EvapoTranspirationStateStruct& evapoTranspirationStateInit, InfiltrationAndGroundwater::VadoseZone vadoseZoneInit);
  
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
  
  int                                    elementNumber;
  double                                 surfacewaterDepth;
  double                                 surfacewaterError;
  double                                 groundwaterHead;
  double                                 groundwaterRecharge;
  double                                 groundwaterError;
  double                                 precipitationRate;
  double                                 precipitationCumulativeShortTerm;
  double                                 precipitationCumulativeLongTerm;
  double                                 evaporationRate;
  double                                 evaporationCumulativeShortTerm;
  double                                 evaporationCumulativeLongTerm;
  double                                 transpirationRate;
  double                                 transpirationCumulativeShortTerm;
  double                                 transpirationCumulativeLongTerm;
  EvapoTranspirationStateStruct          evapoTranspirationState;
  InfiltrationAndGroundwater::VadoseZone vadoseZone;
  std::vector<simpleNeighborInfo>        surfacewaterMeshNeighbors;
  std::vector<simpleNeighborInfo>        groundwaterMeshNeighbors;
  std::vector<simpleNeighborInfo>        surfacewaterChannelNeighbors;
  std::vector<simpleNeighborInfo>        groundwaterChannelNeighbors;
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
  static time_t wallclockTimeAtStart;
  
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
  
  // Create all of the instances in the state and display files that will be
  // needed for this run.  This should be called only once after the state and
  // display files have been initially created and only by the file manager for
  // PE zero.  It does serial NetCDF calls, which are faster for dimension
  // changes because it doesn't require synchronization.
  // FIXME keep this or do it another way?
  void NetCDFCreateInstances();
  
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
  
  // Write variables to the geometry file.  geometryInstance must already be
  // set to the instance to write.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID   - If fileOpen is true, the ID of an already open geometry file.
  // fileOpen - Whether fileID refers to an already open geometry file.
  //            Regardless of whether an already open file was passed in or a
  //            file was opened in this method, at the end of this method the
  //            written file is closed and fileOpen is updated to false.
  bool NetCDFWriteGeometry(int& fileID, bool& fileOpen);
  
  // Write variables to the parameter file.  parameterInstance must already be
  // set to the instance to write.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID   - If fileOpen is true, the ID of an already open parameter file.
  // fileOpen - Whether fileID refers to an already open parameter file.
  //            Regardless of whether an already open file was passed in or a
  //            file was opened in this method, at the end of this method the
  //            written file is closed and fileOpen is updated to false.
  bool NetCDFWriteParameter(int& fileID, bool& fileOpen);
  
  // Write variables to the state file.  stateInstance must already be
  // set to the instance to write.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // fileID   - If fileOpen is true, the ID of an already open state file.
  // fileOpen - Whether fileID refers to an already open state file.
  //            Regardless of whether an already open file was passed in or a
  //            file was opened in this method, at the end of this method the
  //            written file is closed and fileOpen is updated to false.
  bool NetCDFWriteState(int& fileID, bool& fileOpen);
  
  // Write variables to the display file.  displayInstance must already be
  // set to the instance to write.
  //
  // Parameters:
  //
  // fileID   - If fileOpen is true, the ID of an already open display file.
  // fileOpen - Whether fileID refers to an already open display file.
  //            Regardless of whether an already open file was passed in or a
  //            file was opened in this method, at the end of this method the
  //            written file is closed and fileOpen is updated to false.
  bool NetCDFWriteDisplay(int& fileID, bool& fileOpen);
  
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
  
  // To break digital dams in the mesh we arbitrarily connect a mesh element to
  // a channel element from the same catchment.  Catchment numbers for streams
  // are stored in the channel element's reach code.  If a stream is pruned we
  // may not find a channel element with that reach code in which case the
  // return value indicates that no connection was made.  The caller should
  // then find the reach code of an unpruned link downstream of the pruned
  // stream and call this function again with that new reach code.
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
  
  // FIXME comment
  bool readForcingData();
  
  // FIXME comment
  double nextForcingDataTime();
  
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
  
  // Forward this message to the appropriate region.
  // FIXME comment
  void sendDiversionReleaseRecipientInitMessage(int element, int neighbor, int neighborRegion, ChannelTypeEnum neighborChannelType, double neighborX,
                                                double neighborY, double neighborZBank, double neighborZBed, double neighborBaseWidth,
                                                double neighborSideSlope);
  
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
  
  // This variable is used for a temporary array with a size of localNumberOfMeshElements.
  double* meshTempArray;
  
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
  int*		   channelStreamOrder;
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
  
  // This variable is used for a temporary array with a size of localNumberOfChannelElements.
  double* channelTempArray;
  
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
  
  // These are used to load forcing data.
  size_t  jultimeSize;         // The size of the allocated array pointed to by jultime.
  double* jultime;             // Array of Julian dates of all instances in forcing file.
  size_t  jultimeNextInstance; // The next instance to use in the forcing file.
  float*  t2;                  // Used to read air temperature at 2m height forcing for mesh elements.
  float*  psfc;                // Used to read surface pressure forcing for mesh elements.
  float*  u;                   // Used to read wind speed U component forcing for mesh elements.
  float*  v;                   // Used to read wind speed V component forcing for mesh elements.
  float*  qVapor;              // Used to read water vapor mixing ratio forcing for mesh elements.
  float*  qCloud;              // Used to read cloud water mixing ratio forcing for mesh elements.
  float*  swDown;              // Used to read downward shortwave flux forcing for mesh elements.
  float*  gLw;                 // Used to read downward longwave flux forcing for mesh elements.
  float*  tPrec;               // Used to read total precipitation forcing for mesh elements.
  float*  tslb;                // Used to read soil temperature at the deepest layer forcing for mesh elements.
  float*  pblh;                // Used to read planetary boundary layer height forcing for mesh elements.
  float*  t2_c;                // Used to read air temperature at 2m height forcing for channel elements.
  float*  psfc_c;              // Used to read surface pressure forcing for channel elements.
  float*  u_c;                 // Used to read wind speed U component forcing for channel elements.
  float*  v_c;                 // Used to read wind speed V component forcing for channel elements.
  float*  qVapor_c;            // Used to read water vapor mixing ratio forcing for channel elements.
  float*  qCloud_c;            // Used to read cloud water mixing ratio forcing for channel elements.
  float*  swDown_c;            // Used to read downward shortwave flux forcing for channel elements.
  float*  gLw_c;               // Used to read downward longwave flux forcing for channel elements.
  float*  tPrec_c;             // Used to read total precipitation forcing for channel elements.
  float*  tslb_c;              // Used to read soil temperature at the deepest layer forcing for channel elements.
  float*  pblh_c;              // Used to read planetary boundary layer height forcing for channel elements.
  
  // Time and simulation control variables.
  double currentTime;         // Current simulation time in seconds since ADHydro::referenceDate.
  int    nextCheckpointIndex; // This multiplied by ADHydro::checkpointPeriod is the next time that a checkpoint will be done.
  int    nextOutputIndex;     // This multiplied by ADHydro::outputPeriod is the next time that an output will be done.
  double simulationEndTime;   // Simulation time to end the simulation in seconds since ADHydro::referenceDate.
  bool   simulationFinished;  // Flag to indicate the simulation is finished.
  bool   sdagCondition;       // Dummy temporary variable for calculating complex SDAG if conditions in a previous serial block.
  
  // These are used for water management.
  ReservoirFactory reservoirFactory; // A singleton factory for creating Reservoir objects.
  DiversionFactory diversionFactory; // A singleton factory for creating Diversion objects.
  
  // NetCDF I/O variables
  MPI_Comm NetCDFMPIComm;     // A duplicate of MPI_COMM_WORLD used instead because of collisions with Charm++ also using MPI.
  int      geometryFileID;    // ID of geometry file.
  bool     geometryFileOpen;  // Whether geometryFileID refers to an open file.
  int      parameterFileID;   // ID of parameter file.
  bool     parameterFileOpen; // Whether parameterFileID refers to an open file.
  int      stateFileID;       // ID of state file.
  bool     stateFileOpen;     // Whether stateFileID refers to an open file.
  int      displayFileID;     // ID of display file.
  bool     displayFileOpen;   // Whether displayFileID refers to an open file.

// FIXME
int maxDepthValuesReceived;
};

#endif // __FILE_MANAGER_H__
