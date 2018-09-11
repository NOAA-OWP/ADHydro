#include "initialization_manager.h"
#include "adhydro.h"
#include "readonly.h"
#include "initialization_manager.def.h"

#define MESH_ELEMENT_MESH_NEIGHBORS_SIZE (3)

bool InitializationManager::readNodeAndZFiles(const char* nodeFilename, const char* zFilename, size_t* globalNumberOfNodes, size_t* localNumberOfNodes, size_t* localNodeStart, double** nodeX, double** nodeY, double** nodeZ)
{
    bool   error    = false;   // Error flag.
    size_t ii;                 // Loop counter.
    size_t numScanned;         // Used to check that fscanf scanned the correct number of inputs.
    FILE*  nodeFile = NULL;    // The node file to read from.
    FILE*  zFile    = NULL;    // The z file to read from.
    size_t dimension;          // Used to check the dimensions in the files.
    size_t numberOfAttributes; // Used to check the number of attributes in the files.
    size_t boundary;           // Used to check the number of boundary markers in the files.
    size_t numberCheck;        // Used to check numbers that are error checked but otherwise unused.
    size_t index;              // Used to read node numbers.
    double xCoordinate;        // Used to read coordinates from the files.
    double yCoordinate;        // Used to read coordinates from the files.
    double zCoordinate;        // Used to read coordinates from the files.
    
    // Open file.
    nodeFile = fopen(nodeFilename, "r");
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
    {
        if (!(NULL != nodeFile))
        {
            CkError("ERROR in InitializationManager::readNodeAndZFiles: could not open node file %s.\n", nodeFilename);
            error = true;
        }
    }
    
    // Read header.
    if (!error)
    {
        numScanned = fscanf(nodeFile, "%lu %lu %lu %lu", globalNumberOfNodes, &dimension, &numberOfAttributes, &boundary);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(4 == numScanned))
            {
                CkError("ERROR in InitializationManager::readNodeAndZFiles: unable to read header from node file.\n");
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(0 < globalNumberOfNodes && 2 == dimension && 0 == numberOfAttributes && 1 == boundary))
            {
                CkError("ERROR in InitializationManager::readNodeAndZFiles: invalid header in node file.\n");
                error = true;
            }
        }
    }
    
    // Open file.
    if (!error)
    {
        zFile = fopen(zFilename, "r");
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NULL != zFile))
            {
                CkError("ERROR in InitializationManager::readNodeAndZFiles: could not open z file %s.\n", zFilename);
                error = true;
            }
        }
    }
    
    // Read header.
    if (!error)
    {
        numScanned = fscanf(zFile, "%lu %lu", &numberCheck, &dimension);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(2 == numScanned))
            {
                CkError("ERROR in InitializationManager::readNodeAndZFiles: unable to read header from z file.\n");
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(*globalNumberOfNodes == numberCheck && 1 == dimension))
            {
                CkError("ERROR in InitializationManager::readNodeAndZFiles: invalid header in z file.\n");
                error = true;
            }
        }
    }
    
    // Calculate local start and number and allocate arrays.
    if (!error)
    {
        Readonly::localStartAndNumber(*localNodeStart, *localNumberOfNodes, *globalNumberOfNodes, CkNumPes(), CkMyPe());
        
        *nodeX = new double[*localNumberOfNodes];
        *nodeY = new double[*localNumberOfNodes];
        *nodeZ = new double[*localNumberOfNodes];
    }
    
    // Read nodes.
    for (ii = 0; !error && ii < *globalNumberOfNodes; ii++)
    {
        // Read node file.
        numScanned = fscanf(nodeFile, "%lu %lf %lf %*d", &index, &xCoordinate, &yCoordinate);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(3 == numScanned))
            {
                CkError("ERROR in InitializationManager::readNodeAndZFiles: unable to read entry %d from node file.\n", ii);
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(ii == index))
            {
                CkError("ERROR in InitializationManager::readNodeAndZFiles: invalid node number in node file.  %d should be %d.\n", index, ii);
                error = true;
            }
        }
        
        // Read z file.
        if (!error)
        {
            numScanned = fscanf(zFile, "%lu %lf", &numberCheck, &zCoordinate);
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
            {
                if (!(2 == numScanned))
                {
                    CkError("ERROR in InitializationManager::readNodeAndZFiles: unable to read entry %d from z file.\n", ii);
                    error = true;
                }
            }
            
            if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
            {
                if (!(index == numberCheck))
                {
                    CkError("ERROR in InitializationManager::readNodeAndZFiles: invalid node number in z file.  %d should be %d.\n", numberCheck, index);
                    error = true;
                }
            }
        }
        
        // Save values.
        if (!error && *localNodeStart <= index && index < *localNodeStart + *localNumberOfNodes)
        {
            (*nodeX)[index - *localNodeStart] = xCoordinate;
            (*nodeY)[index - *localNodeStart] = yCoordinate;
            (*nodeZ)[index - *localNodeStart] = zCoordinate;
        }
    } // End read nodes.
    
    // Close the files.
    if (NULL != nodeFile)
    {
        fclose(nodeFile);
    }
    
    if (NULL != zFile)
    {
        fclose(zFile);
    }
    
    return error;
}

void InitializationManager::initializeSimulationFromASCIIFiles()
{
    bool    error        = false;    // Error flag.
    size_t  ii, jj;                  // Loop counters.
    size_t  numScanned;              // Used to check that fscanf scanned the correct number of inputs.
    FILE*   eleFile      = NULL;     // The ele      file to read from.
    FILE*   neighFile    = NULL;     // The neigh    file to read from.
    FILE*   landFile     = NULL;     // The land     file to read from.
    FILE*   soilFile     = NULL;     // The soil     file to read from.
    FILE*   chanEleFile  = NULL;     // The chan.ele file to read from.
    size_t  globalNumberOfMeshNodes; // FIXME comment
    size_t  localNumberOfMeshNodes;
    size_t  localMeshNodeStart;
    double* meshNodeX    = NULL;
    double* meshNodeY    = NULL;
    double* meshNodeZ    = NULL;
    size_t  globalNumberOfMeshElements;
    size_t  localNumberOfMeshElements;
    size_t  localMeshElementStart;
    int*    meshVertices;
    int*    meshCatchment;
    int*    meshMeshNeighbors;
    int*    meshVegetationType;
    int*    meshSoilType;
    double* meshSoilDepth;
    size_t  globalNumberOfChannelNodes;
    size_t  localNumberOfChannelNodes;
    size_t  localChannelNodeStart;
    double* channelNodeX = NULL;
    double* channelNodeY = NULL;
    double* channelNodeZ = NULL;
    size_t  globalNumberOfChannelElements; // FIXME comment
    size_t  localNumberOfChannelElements;
    size_t  localChannelElementStart;
    size_t  dimension;                     // Used to check the dimensions in the files.
    size_t  numberOfAttributes;            // Used to check the number of attributes in the files.
    size_t  numberCheck;                   // Used to check numbers that are error checked but otherwise unused.
    size_t  index;                         // Used to read node numbers.
    int     vertex0;                       // Used to read vertices from the files.
    int     vertex1;                       // Used to read vertices from the files.
    int     vertex2;                       // Used to read vertices from the files.
    int     catchment;                     // Used to read catchments from the files.
    int     neighbor0;                     // Used to read neighbors from the files.
    int     neighbor1;                     // Used to read neighbors from the files.
    int     neighbor2;                     // Used to read neighbors from the files.
    int     vegetationType;                // Used to read vegetation type from the files.
    size_t  numberOfSoilLayers;            // Used to read the number of soil layers from the files.
    int     soilTypeReader;                // Used to read multiple layers.
    int     soilType;                      // The final value that will be associated with this element.
    double  soilDepthReader;               // Used to read multiple layers.
    double  soilDepth;                     // The final value that will be associated with this element.
    
    // Read mesh nodes.
    error = readNodeAndZFiles(Readonly::meshNodeFilePath.c_str(), Readonly::meshZFilePath.c_str(), &globalNumberOfMeshNodes, &localNumberOfMeshNodes, &localMeshNodeStart, &meshNodeX, &meshNodeY, &meshNodeZ);
    
    // Open file.
    if (!error)
    {
        eleFile = fopen(Readonly::meshElementFilePath.c_str(), "r");
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NULL != eleFile))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: could not open ele file %s.\n", Readonly::meshElementFilePath.c_str());
                error = true;
            }
        }
    }
    
    // Read header.
    if (!error)
    {
        numScanned = fscanf(eleFile, "%lu %lu %lu", &globalNumberOfMeshElements, &dimension, &numberOfAttributes);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(3 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read header from ele file.\n");
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(0 < globalNumberOfMeshElements && 3 == dimension && 1 == numberOfAttributes))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid header in ele file.\n");
                error = true;
            }
        }
    }
    
    // Open file.
    if (!error)
    {
        neighFile = fopen(Readonly::meshNeighborFilePath.c_str(), "r");
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NULL != neighFile))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: could not open neigh file %s.\n", Readonly::meshNeighborFilePath.c_str());
                error = true;
            }
        }
    }
    
    // Read header.
    if (!error)
    {
        numScanned = fscanf(neighFile, "%lu %lu", &numberCheck, &dimension);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(2 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read header from neigh file.\n");
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(globalNumberOfMeshElements == numberCheck && 3 == dimension))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid header in neigh file.\n");
                error = true;
            }
        }
    }
    
    // Open file.
    if (!error)
    {
        landFile = fopen(Readonly::meshLandFilePath.c_str(), "r");
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NULL != landFile))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: could not open land file %s.\n", Readonly::meshLandFilePath.c_str());
                error = true;
            }
        }
    }
    
    // Read header.
    if (!error)
    {
        numScanned = fscanf(landFile, "%lu", &numberCheck);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(1 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read header from land file.\n");
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(globalNumberOfMeshElements == numberCheck))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid header in land file.\n");
                error = true;
            }
        }
    }
    
    // Open file.
    if (!error)
    {
        soilFile = fopen(Readonly::meshSoilFilePath.c_str(), "r");
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NULL != soilFile))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: could not open soil file %s.\n", Readonly::meshSoilFilePath.c_str());
                error = true;
            }
        }
    }
    
    // Read header.
    if (!error)
    {
        numScanned = fscanf(soilFile, "%lu", &numberCheck);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(1 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read header from soil file.\n");
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(globalNumberOfMeshElements == numberCheck))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid header in soil file.\n");
                error = true;
            }
        }
    }
    
    // Calculate local start and number and allocate arrays.
    if (!error)
    {
        Readonly::localStartAndNumber(localMeshElementStart, localNumberOfMeshElements, globalNumberOfMeshElements, CkNumPes(), CkMyPe());
        
        meshVertices       = new int[   localNumberOfMeshElements * MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
        meshCatchment      = new int[   localNumberOfMeshElements];
        meshMeshNeighbors  = new int[   localNumberOfMeshElements * MESH_ELEMENT_MESH_NEIGHBORS_SIZE];
        meshVegetationType = new int[   localNumberOfMeshElements];
        meshSoilType       = new int[   localNumberOfMeshElements];
        meshSoilDepth      = new double[localNumberOfMeshElements];
    }
    
    // Read mesh elements.
    for (ii = 0; !error && ii < globalNumberOfMeshElements; ii++)
    {
        // Read ele file.
        numScanned = fscanf(eleFile, "%lu %d %d %d %d", &index, &vertex0, &vertex1, &vertex2, &catchment);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(5 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read entry %d from ele file.\n", ii);
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(ii == index))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid element number in ele file.  %d should be %d.\n", index, ii);
                error = true;
            }
            
            if (!(0 <= vertex0 && vertex0 < (int)globalNumberOfMeshNodes))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex0);
                error = true;
            }
            
            if (!(0 <= vertex1 && vertex1 < (int)globalNumberOfMeshNodes))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex1);
                error = true;
            }
            
            if (!(0 <= vertex2 && vertex2 < (int)globalNumberOfMeshNodes))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid vertex number %d in ele file.\n", index, vertex2);
                error = true;
            }
        }
        
        // Read neigh file.
        numScanned = fscanf(neighFile, "%lu %d %d %d", &numberCheck, &neighbor0, &neighbor1, &neighbor2);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(4 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read entry %d from neigh file.\n", ii);
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(index == numberCheck))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid element number in neigh file.  %d should be %d.\n", numberCheck, index);
                error = true;
            }
            
            if (!(isBoundary(neighbor0) || (0 <= neighbor0 && neighbor0 < (int)globalNumberOfMeshElements)))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor0);
                error = true;
            }
            
            if (!(isBoundary(neighbor1) || (0 <= neighbor1 && neighbor1 < (int)globalNumberOfMeshElements)))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor1);
                error = true;
            }
            
            if (!(isBoundary(neighbor2) || (0 <= neighbor2 && neighbor2 < (int)globalNumberOfMeshElements)))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid mesh neighbor number %d in neigh file.\n", index, neighbor2);
                error = true;
            }
        }
        
        // Read land file.
        numScanned = fscanf(landFile, "%lu %d", &numberCheck, &vegetationType);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(2 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read entry %d from land file.\n", ii);
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(index == numberCheck))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid element number in land file.  %d should be %d.\n", numberCheck, index);
                error = true;
            }
            
            if (!(1 <= vegetationType && 27 >= vegetationType))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid vegetation type number %d in land file.\n", index, vegetationType);
                error = true;
            }
        }
        
        // Read soil file.
        numScanned = fscanf(soilFile, "%lu %lu", &numberCheck, &numberOfSoilLayers);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(2 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read entry %d from soil file.\n", ii);
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(index == numberCheck))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid element number in soil file.  %d should be %d.\n", numberCheck, index);
                error = true;
            }
            
            if (!(0 <= numberOfSoilLayers))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid number of soil layers %d in soil file.\n", index, numberOfSoilLayers);
                error = true;
            }
        }
        
        if (!error)
        {
            soilType  = -1;  // If there are no soil layers soilType will be left as -1 as a flag for no soil data, and soilDepth will be left as zero.
            soilDepth = 0.0; // That element will have to get soil type and depth from a neighbor in meshMassage.
            
            // Loop through the soil layers of this element.
            for (jj = 0; jj < numberOfSoilLayers; jj++)
            {
                numScanned = fscanf(soilFile, "%d,%lf", &soilTypeReader, &soilDepthReader);
                
                if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
                {
                    if (!(2 == numScanned))
                    {
                        CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read entry %d soil layer %d from soil file.\n", ii, jj);
                        error = true;
                    }
                }
                
                if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                {
                    if (!((1 <= soilTypeReader && 19 >= soilTypeReader) || -1 == soilTypeReader))
                    {
                        CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: invalid soil type %s in soil file.\n", index, soilTypeReader);
                        error = true;
                    }
                    
                    if (!(0.0 <= soilDepthReader))
                    {
                        CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: mesh element %d: soilDepthReader must be greater than or equal to zero.\n", index);
                        error = true;
                    }
                }
                
                // Only save the first valid soil type.
                if (-1 == soilType)
                {
                    soilType = soilTypeReader;
                }
                
                // Save the sum of soil thicknesses.
                soilDepth += soilDepthReader;
            }
        }
        
        // Save values.
        if (!error && localMeshElementStart <= index && index < localMeshElementStart + localNumberOfMeshElements)
        {
            meshVertices[     (index - localMeshElementStart) * MESH_ELEMENT_MESH_NEIGHBORS_SIZE + 0] = vertex0;
            meshVertices[     (index - localMeshElementStart) * MESH_ELEMENT_MESH_NEIGHBORS_SIZE + 1] = vertex1;
            meshVertices[     (index - localMeshElementStart) * MESH_ELEMENT_MESH_NEIGHBORS_SIZE + 2] = vertex2;
            meshCatchment[     index - localMeshElementStart]                                         = catchment - 2;  // The .ele file stores catchment number plus two because zero and one are used by triangle.
            meshMeshNeighbors[(index - localMeshElementStart) * MESH_ELEMENT_MESH_NEIGHBORS_SIZE + 0] = neighbor0;
            meshMeshNeighbors[(index - localMeshElementStart) * MESH_ELEMENT_MESH_NEIGHBORS_SIZE + 1] = neighbor1;
            meshMeshNeighbors[(index - localMeshElementStart) * MESH_ELEMENT_MESH_NEIGHBORS_SIZE + 2] = neighbor2;
            meshVegetationType[index - localMeshElementStart]                                         = vegetationType;
            meshSoilType[      index - localMeshElementStart]                                         = soilType;
            meshSoilDepth[     index - localMeshElementStart]                                         = soilDepth;
        }
    } // End read mesh elements.
    
    // Close the files.
    if (NULL != eleFile)
    {
        fclose(eleFile);
    }
    
    if (NULL != neighFile)
    {
        fclose(neighFile);
    }
    
    if (NULL != landFile)
    {
        fclose(landFile);
    }
    
    if (NULL != soilFile)
    {
        fclose(soilFile);
    }
    
    if (!error)
    {
        error = readNodeAndZFiles(Readonly::channelNodeFilePath.c_str(), Readonly::channelZFilePath.c_str(), &globalNumberOfChannelNodes, &localNumberOfChannelNodes, &localChannelNodeStart, &channelNodeX, &channelNodeY, &channelNodeZ);
    }
    
    // Open file.
    if (!error)
    {
        chanEleFile = fopen(Readonly::channelElementFilePath.c_str(), "r");
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(NULL != chanEleFile))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: could not open chan.ele file %s.\n", Readonly::channelElementFilePath.c_str());
                error = true;
            }
        }
    }
    
    // Read header.
    if (!error)
    {
        numScanned = fscanf(chanEleFile, "%lu", &globalNumberOfChannelElements);
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_LIBRARY_ERRORS)
        {
            if (!(1 == numScanned))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: unable to read header from chan.ele file.\n");
                error = true;
            }
        }
        
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            if (!(0 < globalNumberOfChannelElements))
            {
                CkError("ERROR in InitializationManager::initializeSimulationFromASCIIFiles: invalid header in chan.ele file.\n");
                error = true;
            }
        }
    }
    
    // Calculate local start and number and allocate arrays.
    if (!error)
    {
        Readonly::localStartAndNumber(localChannelElementStart, localNumberOfChannelElements, globalNumberOfChannelElements, CkNumPes(), CkMyPe());
        
        /*
         *      channelElementVertices            = new int[localNumberOfChannelElements][XDMF_SIZE];
         *      channelChannelType                = new ChannelTypeEnum[localNumberOfChannelElements];
         *      channelReachCode                  = new long long[localNumberOfChannelElements];
         *      channelElementBankFullDepth       = new double[localNumberOfChannelElements];
         *      channelElementLength              = new double[localNumberOfChannelElements];
         *      channelBaseWidth                  = new double[localNumberOfChannelElements];
         *      channelSideSlope                  = new double[localNumberOfChannelElements];
         *      channelMeshNeighbors              = new int[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
         *      channelMeshNeighborsEdgeLength    = new double[localNumberOfChannelElements][CHANNEL_ELEMENT_MESH_NEIGHBORS_SIZE];
         *      channelChannelNeighbors           = new int[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
         *      channelChannelNeighborsDownstream = new bool[localNumberOfChannelElements][CHANNEL_ELEMENT_CHANNEL_NEIGHBORS_SIZE];
         */
    }
    
    contribute(CkCallback(CkCallback::ckExit));
}

// FIXME remove
bool initializeHardcodedMap(MapGeometry& geometryData, MapParameters& parameterData, TimePointState& stateData)
{
    size_t ii, jj; // Loop counters.
    
    // Mesh geometry.
    double meshElementX[]    = {0.0, -100.0};
    double meshElementY[]    = {0.0, 0.0};
    double meshElementZ[]    = {0.0, -2.0};
    double meshElementArea[] = {10000.0, 10000.0};
    double meshLatitude[]    = {0.0, 0.0};
    double meshLongitude[]   = {0.0, 0.0};
    double meshSlopeX[]      = {-0.02, -0.02};
    double meshSlopeY[]      = {-0.02, -0.02};
    
    // Mesh parameters.
    size_t meshRegion[]                = {0, 1};
    size_t meshCatchment[]             = {0, 1};
    int    meshVegetationType[]        = {11, 11};
    int    meshGroundType[]            = {2, 2};
    double meshManningsN[]             = {0.16, 0.16};
    bool   meshSoilExists[]            = {true, true};
    double meshImpedanceConductivity[] = {3.38E-6, 3.38E-6};
    bool   meshAquiferExists[]         = {true, true};
    double meshDeepConductivity[]      = {9.74E-7, 9.74E-7};
    
    // Mesh state.
    EvapoTranspirationStateStruct meshEvapoTranspirationState = {{0.0f, 0.0f, 0.0f}, 1.0f, 0.0f, {0.0f, 0.0f, 0.0f, 300.0f, 300.0f, 300.0f, 300.0f}, 300.0f, 2000.0f, 0.0f, 0.0f, 0.0f,
                                                                 300.0f, 300.0f, 0, {0.0f, 0.0f, 0.0f, -0.05f, -0.2f, -0.5f, -1.0f}, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
                                                                 100000.0f, 100000.0f, 100000.0f, 200000.0f, 200000.0f, 200000.0f, 4.6f, 0.6f, 0.002f, 0.002f, 0.0f, 0.0f, 0.0f};
    SimpleVadoseZone              meshSoilWater(0.1, 1.41E-5, 0.421, 0.0426, 0.0, 0.9);
    double                        meshAquiferHead[] = {-0.9, -2.9};
    SimpleVadoseZone              meshAquiferWater(0.8, 4.66E-5, 0.339, 0.0279, 0.0, 0.8);
    
    // Mesh neighbor geometry.
    NeighborEndpointEnum meshNeighborLocalEndpoint[]       = {MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, MESH_SURFACE, MESH_SOIL, MESH_AQUIFER,
                                                              MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, MESH_SURFACE, MESH_SOIL, MESH_AQUIFER};
    NeighborEndpointEnum meshNeighborRemoteEndpoint[]      = {MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE,
                                                              MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE};
    size_t               meshNeighborRemoteElementNumber[] = {1, 1, 1, 0, 0, 0,
                                                              0, 0, 0, 1, 1, 1};
    double               meshNeighborEdgeLength[]          = {100.0, 100.0, 100.0, 100.0, 100.0, 100.0,
                                                              100.0, 100.0, 100.0, 100.0, 100.0, 100.0};
    double               meshNeighborEdgeNormalX[]         = {-1.0, -1.0, -1.0, 0.0, 0.0, 0.0,
                                                               1.0,  1.0,  1.0, 0.0, 0.0, 0.0};
    double               meshNeighborEdgeNormalY[]         = {0.0, 0.0, 0.0, -1.0, -1.0, -1.0,
                                                              0.0, 0.0, 0.0, -1.0, -1.0, -1.0};
    double               meshNeighborZOffset[]             = {0.0, 0.0, 0.0, 10.0, 10.0, 10.0,
                                                              0.0, 0.0, 0.0,  0.0,  0.0,  0.0};
    
    // Mesh neighbor parameters.
    NeighborEndpointEnum meshNeighborLocalEndpointP[]       = {MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, MESH_SURFACE, MESH_SOIL, MESH_AQUIFER,
                                                               MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, MESH_SURFACE, MESH_SOIL, MESH_AQUIFER};
    NeighborEndpointEnum meshNeighborRemoteEndpointP[]      = {MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE,
                                                               MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE};
    size_t               meshNeighborRemoteElementNumberP[] = {1, 1, 1, 0, 0, 0,
                                                               0, 0, 0, 1, 1, 1};
    size_t               meshNeighborRegion[]               = {1, 1, 1, 0, 0, 0,
                                                               0, 0, 0, 1, 1, 1};
    
    // Channel geometry.
    double channelElementX[]        = {0.0, -100.0};
    double channelElementY[]        = {-50.0, -50.0};
    double channelElementZBank[]    = {10.0, -3.0};
    double channelElementZBed[]     = {-2.0, -4.0};
    double channelElementLength[]   = {100.0, 100.0};
    double channelLatitude[]        = {0.0, 0.0};
    double channelLongitude[]       = {0.0, 0.0};
    
    // Channel parameters.
    size_t          channelRegion[]          = {0, 1};
    ChannelTypeEnum channelChannelType[]     = {STREAM, STREAM};
    long long       channelReachCode[]       = {0, 1};
    double          channelBaseWidth[]       = {1.0, 1.0};
    double          channelSideSlope[]       = {1.0, 1.0};
    double          channelManningsN[]       = {0.038, 0.038};
    double          channelBedThickness[]    = {1.0, 1.0};
    double          channelBedConductivity[] = {1.41E-5, 1.41E-5};
    
    // Channel state.
    EvapoTranspirationStateStruct channelEvapoTranspirationState = {{0.0f, 0.0f, 0.0f}, 1.0f, 0.0f, {0.0f, 0.0f, 0.0f, 300.0f, 300.0f, 300.0f, 300.0f}, 300.0f, 2000.0f, 0.0f, 0.0f, 0.0f,
                                                                    300.0f, 300.0f, 0, {0.0f, 0.0f, 0.0f, -0.05f, -0.2f, -0.5f, -1.0f}, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
                                                                    100000.0f, 100000.0f, 100000.0f, 200000.0f, 200000.0f, 200000.0f, 4.6f, 0.6f, 0.002f, 0.002f, 0.0f, 0.0f, 0.0f};
    
    // Channel neighbor geometry.
    NeighborEndpointEnum channelNeighborLocalEndpoint[]       = {CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE, NO_NEIGHBOR,
                                                                 CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE};
    NeighborEndpointEnum channelNeighborRemoteEndpoint[]      = {MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, CHANNEL_SURFACE, NO_NEIGHBOR,
                                                                 MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, CHANNEL_SURFACE, BOUNDARY_OUTFLOW};
    size_t               channelNeighborRemoteElementNumber[] = {0, 0, 0, 1, 1,
                                                                 1, 1, 1, 0, 0};
    double               channelNeighborEdgeLength[]          = {100.0, 100.0, 100.0, 1.0, 1.0,
                                                                 100.0, 100.0, 100.0, 1.0, 1.0,};
    double               channelNeighborEdgeNormalX[]         = {0.0, 0.0, 0.0, -1.0,  1.0,
                                                                 0.0, 0.0, 0.0,  1.0, -1.0};
    double               channelNeighborEdgeNormalY[]         = {1.0, 1.0, 1.0, 0.0, 0.0,
                                                                 1.0, 1.0, 1.0, 0.0, 0.0};
    double               channelNeighborZOffset[]             = {10.0, 10.0, 10.0, 0.0, 0.0,
                                                                  0.0,  0.0,  0.0, 0.0, 0.0};
    
    // Channel neighbor parameters.
    NeighborEndpointEnum channelNeighborLocalEndpointP[]       = {CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE, NO_NEIGHBOR,
                                                                  CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE, CHANNEL_SURFACE};
    NeighborEndpointEnum channelNeighborRemoteEndpointP[]      = {MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, CHANNEL_SURFACE, NO_NEIGHBOR,
                                                                  MESH_SURFACE, MESH_SOIL, MESH_AQUIFER, CHANNEL_SURFACE, BOUNDARY_OUTFLOW};
    size_t               channelNeighborRemoteElementNumberP[] = {0, 0, 0, 1, 1,
                                                                  1, 1, 1, 0, 0};
    size_t               channelNeighborRegion[]               = {0, 0, 0, 1, 1,
                                                                  1, 1, 1, 0, 0};
    
    // Region parameters.
    size_t regionNumberOfMeshElements[]    = {1, 1};
    size_t regionNumberOfChannelElements[] = {1, 1};
    
    // Allocate mesh geometry arrays.
    geometryData.meshElementX    = new double[geometryData.localNumberOfMeshElements];
    geometryData.meshElementY    = new double[geometryData.localNumberOfMeshElements];
    geometryData.meshElementZ    = new double[geometryData.localNumberOfMeshElements];
    geometryData.meshElementArea = new double[geometryData.localNumberOfMeshElements];
    geometryData.meshLatitude    = new double[geometryData.localNumberOfMeshElements];
    geometryData.meshLongitude   = new double[geometryData.localNumberOfMeshElements];
    geometryData.meshSlopeX      = new double[geometryData.localNumberOfMeshElements];
    geometryData.meshSlopeY      = new double[geometryData.localNumberOfMeshElements];
    
    // Allocate mesh parameter arrays.
    parameterData.meshRegion                = new size_t[parameterData.localNumberOfMeshElements];
    parameterData.meshCatchment             = new size_t[parameterData.localNumberOfMeshElements];
    parameterData.meshVegetationType        = new int[   parameterData.localNumberOfMeshElements];
    parameterData.meshGroundType            = new int[   parameterData.localNumberOfMeshElements];
    parameterData.meshManningsN             = new double[parameterData.localNumberOfMeshElements];
    parameterData.meshSoilExists            = new bool[  parameterData.localNumberOfMeshElements];
    parameterData.meshImpedanceConductivity = new double[parameterData.localNumberOfMeshElements];
    parameterData.meshAquiferExists         = new bool[  parameterData.localNumberOfMeshElements];
    parameterData.meshDeepConductivity      = new double[parameterData.localNumberOfMeshElements];
    
    // Allocate mesh neighbor geometry arrays.
    geometryData.meshNeighborLocalEndpoint       = new NeighborEndpointEnum[geometryData.localNumberOfMeshElements * geometryData.maximumNumberOfMeshNeighbors];
    geometryData.meshNeighborRemoteEndpoint      = new NeighborEndpointEnum[geometryData.localNumberOfMeshElements * geometryData.maximumNumberOfMeshNeighbors];
    geometryData.meshNeighborRemoteElementNumber = new size_t[              geometryData.localNumberOfMeshElements * geometryData.maximumNumberOfMeshNeighbors];
    geometryData.meshNeighborEdgeLength          = new double[              geometryData.localNumberOfMeshElements * geometryData.maximumNumberOfMeshNeighbors];
    geometryData.meshNeighborEdgeNormalX         = new double[              geometryData.localNumberOfMeshElements * geometryData.maximumNumberOfMeshNeighbors];
    geometryData.meshNeighborEdgeNormalY         = new double[              geometryData.localNumberOfMeshElements * geometryData.maximumNumberOfMeshNeighbors];
    geometryData.meshNeighborZOffset             = new double[              geometryData.localNumberOfMeshElements * geometryData.maximumNumberOfMeshNeighbors];
    
    // Allocate mesh neighbor parameter arrays.
    parameterData.meshNeighborLocalEndpoint       = new NeighborEndpointEnum[parameterData.localNumberOfMeshElements * parameterData.maximumNumberOfMeshNeighbors];
    parameterData.meshNeighborRemoteEndpoint      = new NeighborEndpointEnum[parameterData.localNumberOfMeshElements * parameterData.maximumNumberOfMeshNeighbors];
    parameterData.meshNeighborRemoteElementNumber = new size_t[              parameterData.localNumberOfMeshElements * parameterData.maximumNumberOfMeshNeighbors];
    parameterData.meshNeighborRegion              = new size_t[              parameterData.localNumberOfMeshElements * parameterData.maximumNumberOfMeshNeighbors];
    
    for (ii = 0; ii < geometryData.localNumberOfMeshElements; ++ii)
    {
        PUP::toMem evapoTranspirationStatePupper(stateData.meshEvapoTranspirationState[ii]);
        PUP::toMem soilWaterPupper(stateData.meshSoilWater[ii]);
        PUP::toMem aquiferWaterPupper(stateData.meshAquiferWater[ii]);
        
        // Fill in mesh geometry arrays.
        geometryData.meshElementX[ii]    = meshElementX[   ii + geometryData.localMeshElementStart];
        geometryData.meshElementY[ii]    = meshElementY[   ii + geometryData.localMeshElementStart];
        geometryData.meshElementZ[ii]    = meshElementZ[   ii + geometryData.localMeshElementStart];
        geometryData.meshElementArea[ii] = meshElementArea[ii + geometryData.localMeshElementStart];
        geometryData.meshLatitude[ii]    = meshLatitude[   ii + geometryData.localMeshElementStart];
        geometryData.meshLongitude[ii]   = meshLongitude[  ii + geometryData.localMeshElementStart];
        geometryData.meshSlopeX[ii]      = meshSlopeX[     ii + geometryData.localMeshElementStart];
        geometryData.meshSlopeY[ii]      = meshSlopeY[     ii + geometryData.localMeshElementStart];
        
        // Fill in mesh parameter arrays.
        parameterData.meshRegion[ii]                = meshRegion[               ii + parameterData.localMeshElementStart];
        parameterData.meshCatchment[ii]             = meshCatchment[            ii + parameterData.localMeshElementStart];
        parameterData.meshVegetationType[ii]        = meshVegetationType[       ii + parameterData.localMeshElementStart];
        parameterData.meshGroundType[ii]            = meshGroundType[           ii + parameterData.localMeshElementStart];
        parameterData.meshManningsN[ii]             = meshManningsN[            ii + parameterData.localMeshElementStart];
        parameterData.meshSoilExists[ii]            = meshSoilExists[           ii + parameterData.localMeshElementStart];
        parameterData.meshImpedanceConductivity[ii] = meshImpedanceConductivity[ii + parameterData.localMeshElementStart];
        parameterData.meshAquiferExists[ii]         = meshAquiferExists[        ii + parameterData.localMeshElementStart];
        parameterData.meshDeepConductivity[ii]      = meshDeepConductivity[     ii + parameterData.localMeshElementStart];
        
        // Fill in mesh state arrays.
        evapoTranspirationStatePupper             | meshEvapoTranspirationState;
        stateData.meshSurfaceWater[ii]            = 0.0;
        stateData.meshSurfaceWaterCreated[ii]     = 0.0;
        stateData.meshGroundwaterMode[ii]         = UNSATURATED_AQUIFER;
        stateData.meshPerchedHead[ii]             = 0.0;
        soilWaterPupper                           | meshSoilWater;
        stateData.meshSoilWaterCreated[ii]        = 0.0;
        stateData.meshAquiferHead[ii]             = meshAquiferHead[ii + stateData.localMeshElementStart];
        aquiferWaterPupper                        | meshAquiferWater;
        stateData.meshAquiferWaterCreated[ii]     = 0.0;
        stateData.meshDeepGroundwater[ii]         = 0.0;
        stateData.meshPrecipitationRate[ii]       = 0.0;
        stateData.meshPrecipitationCumulative[ii] = 0.0;
        stateData.meshEvaporationRate[ii]         = 0.0;
        stateData.meshEvaporationCumulative[ii]   = 0.0;
        stateData.meshTranspirationRate[ii]       = 0.0;
        stateData.meshTranspirationCumulative[ii] = 0.0;
        stateData.meshCanopyWater[ii]             = 0.0;
        stateData.meshSnowWater[ii]               = 0.0;
        stateData.meshRootZoneWater[ii]           = 0.0;
        stateData.meshTotalGroundwater[ii]        = 0.0;
        
        for (jj = 0; jj < geometryData.maximumNumberOfMeshNeighbors; ++jj)
        {
            // Fill in mesh neighbor geometry arrays.
            geometryData.meshNeighborLocalEndpoint[      ii * geometryData.maximumNumberOfMeshNeighbors + jj] = meshNeighborLocalEndpoint[      (ii + geometryData.localMeshElementStart) * geometryData.maximumNumberOfMeshNeighbors + jj];
            geometryData.meshNeighborRemoteEndpoint[     ii * geometryData.maximumNumberOfMeshNeighbors + jj] = meshNeighborRemoteEndpoint[     (ii + geometryData.localMeshElementStart) * geometryData.maximumNumberOfMeshNeighbors + jj];
            geometryData.meshNeighborRemoteElementNumber[ii * geometryData.maximumNumberOfMeshNeighbors + jj] = meshNeighborRemoteElementNumber[(ii + geometryData.localMeshElementStart) * geometryData.maximumNumberOfMeshNeighbors + jj];
            geometryData.meshNeighborEdgeLength[         ii * geometryData.maximumNumberOfMeshNeighbors + jj] = meshNeighborEdgeLength[         (ii + geometryData.localMeshElementStart) * geometryData.maximumNumberOfMeshNeighbors + jj];
            geometryData.meshNeighborEdgeNormalX[        ii * geometryData.maximumNumberOfMeshNeighbors + jj] = meshNeighborEdgeNormalX[        (ii + geometryData.localMeshElementStart) * geometryData.maximumNumberOfMeshNeighbors + jj];
            geometryData.meshNeighborEdgeNormalY[        ii * geometryData.maximumNumberOfMeshNeighbors + jj] = meshNeighborEdgeNormalY[        (ii + geometryData.localMeshElementStart) * geometryData.maximumNumberOfMeshNeighbors + jj];
            geometryData.meshNeighborZOffset[            ii * geometryData.maximumNumberOfMeshNeighbors + jj] = meshNeighborZOffset[            (ii + geometryData.localMeshElementStart) * geometryData.maximumNumberOfMeshNeighbors + jj];
            
            // Fill in mesh neighbor parameter arrays.
            parameterData.meshNeighborLocalEndpoint[      ii * parameterData.maximumNumberOfMeshNeighbors + jj] = meshNeighborLocalEndpointP[      (ii + parameterData.localMeshElementStart) * parameterData.maximumNumberOfMeshNeighbors + jj];
            parameterData.meshNeighborRemoteEndpoint[     ii * parameterData.maximumNumberOfMeshNeighbors + jj] = meshNeighborRemoteEndpointP[     (ii + parameterData.localMeshElementStart) * parameterData.maximumNumberOfMeshNeighbors + jj];
            parameterData.meshNeighborRemoteElementNumber[ii * parameterData.maximumNumberOfMeshNeighbors + jj] = meshNeighborRemoteElementNumberP[(ii + parameterData.localMeshElementStart) * parameterData.maximumNumberOfMeshNeighbors + jj];
            parameterData.meshNeighborRegion[             ii * parameterData.maximumNumberOfMeshNeighbors + jj] = meshNeighborRegion[              (ii + parameterData.localMeshElementStart) * parameterData.maximumNumberOfMeshNeighbors + jj];
            
            // Fill in mesh neighbor state arrays.
            stateData.meshNeighborLocalEndpoint[      ii * stateData.maximumNumberOfMeshNeighbors + jj] = meshNeighborLocalEndpointP[      (ii + stateData.localMeshElementStart) * stateData.maximumNumberOfMeshNeighbors + jj];
            stateData.meshNeighborRemoteEndpoint[     ii * stateData.maximumNumberOfMeshNeighbors + jj] = meshNeighborRemoteEndpointP[     (ii + stateData.localMeshElementStart) * stateData.maximumNumberOfMeshNeighbors + jj];
            stateData.meshNeighborRemoteElementNumber[ii * stateData.maximumNumberOfMeshNeighbors + jj] = meshNeighborRemoteElementNumberP[(ii + stateData.localMeshElementStart) * stateData.maximumNumberOfMeshNeighbors + jj];
            stateData.meshNeighborNominalFlowRate[    ii * stateData.maximumNumberOfMeshNeighbors + jj] = 0.0;
            stateData.meshNeighborExpirationTime[     ii * stateData.maximumNumberOfMeshNeighbors + jj] = Readonly::simulationStartTime;
            stateData.meshNeighborInflowCumulative[   ii * stateData.maximumNumberOfMeshNeighbors + jj] = 0.0;
            stateData.meshNeighborOutflowCumulative[  ii * stateData.maximumNumberOfMeshNeighbors + jj] = 0.0;
        }
    }
    
    // Allocate channel geometry arrays.
    geometryData.channelElementX      = new double[geometryData.localNumberOfChannelElements];
    geometryData.channelElementY      = new double[geometryData.localNumberOfChannelElements];
    geometryData.channelElementZBank  = new double[geometryData.localNumberOfChannelElements];
    geometryData.channelElementZBed   = new double[geometryData.localNumberOfChannelElements];
    geometryData.channelElementLength = new double[geometryData.localNumberOfChannelElements];
    geometryData.channelLatitude      = new double[geometryData.localNumberOfChannelElements];
    geometryData.channelLongitude     = new double[geometryData.localNumberOfChannelElements];
    
    // Allocate channel parameter arrays.
    parameterData.channelRegion          = new size_t[         parameterData.localNumberOfChannelElements];
    parameterData.channelChannelType     = new ChannelTypeEnum[parameterData.localNumberOfChannelElements];
    parameterData.channelReachCode       = new long long[      parameterData.localNumberOfChannelElements];
    parameterData.channelBaseWidth       = new double[         parameterData.localNumberOfChannelElements];
    parameterData.channelSideSlope       = new double[         parameterData.localNumberOfChannelElements];
    parameterData.channelManningsN       = new double[         parameterData.localNumberOfChannelElements];
    parameterData.channelBedThickness    = new double[         parameterData.localNumberOfChannelElements];
    parameterData.channelBedConductivity = new double[         parameterData.localNumberOfChannelElements];
    
    // Allocate channel neighbor geometry arrays.
    geometryData.channelNeighborLocalEndpoint       = new NeighborEndpointEnum[geometryData.localNumberOfChannelElements * geometryData.maximumNumberOfChannelNeighbors];
    geometryData.channelNeighborRemoteEndpoint      = new NeighborEndpointEnum[geometryData.localNumberOfChannelElements * geometryData.maximumNumberOfChannelNeighbors];
    geometryData.channelNeighborRemoteElementNumber = new size_t[              geometryData.localNumberOfChannelElements * geometryData.maximumNumberOfChannelNeighbors];
    geometryData.channelNeighborEdgeLength          = new double[              geometryData.localNumberOfChannelElements * geometryData.maximumNumberOfChannelNeighbors];
    geometryData.channelNeighborEdgeNormalX         = new double[              geometryData.localNumberOfChannelElements * geometryData.maximumNumberOfChannelNeighbors];
    geometryData.channelNeighborEdgeNormalY         = new double[              geometryData.localNumberOfChannelElements * geometryData.maximumNumberOfChannelNeighbors];
    geometryData.channelNeighborZOffset             = new double[              geometryData.localNumberOfChannelElements * geometryData.maximumNumberOfChannelNeighbors];
    
    // Allocate channel neighbor parameter arrays.
    parameterData.channelNeighborLocalEndpoint       = new NeighborEndpointEnum[parameterData.localNumberOfChannelElements * parameterData.maximumNumberOfChannelNeighbors];
    parameterData.channelNeighborRemoteEndpoint      = new NeighborEndpointEnum[parameterData.localNumberOfChannelElements * parameterData.maximumNumberOfChannelNeighbors];
    parameterData.channelNeighborRemoteElementNumber = new size_t[              parameterData.localNumberOfChannelElements * parameterData.maximumNumberOfChannelNeighbors];
    parameterData.channelNeighborRegion              = new size_t[              parameterData.localNumberOfChannelElements * parameterData.maximumNumberOfChannelNeighbors];
    
    for (ii = 0; ii < geometryData.localNumberOfChannelElements; ++ii)
    {
        PUP::toMem evapoTranspirationStatePupper(stateData.channelEvapoTranspirationState[ii]);
        
        // Fill in channel geometry arrays.
        geometryData.channelElementX[ii]      = channelElementX[     ii + geometryData.localChannelElementStart];
        geometryData.channelElementY[ii]      = channelElementY[     ii + geometryData.localChannelElementStart];
        geometryData.channelElementZBank[ii]  = channelElementZBank[ ii + geometryData.localChannelElementStart];
        geometryData.channelElementZBed[ii]   = channelElementZBed[  ii + geometryData.localChannelElementStart];
        geometryData.channelElementLength[ii] = channelElementLength[ii + geometryData.localChannelElementStart];
        geometryData.channelLatitude[ii]      = channelLatitude[     ii + geometryData.localChannelElementStart];
        geometryData.channelLongitude[ii]     = channelLongitude[    ii + geometryData.localChannelElementStart];
        
        // Fill in channel parameter arrays.
        parameterData.channelRegion[ii]          = channelRegion[         ii + parameterData.localChannelElementStart];
        parameterData.channelChannelType[ii]     = channelChannelType[    ii + parameterData.localChannelElementStart];
        parameterData.channelReachCode[ii]       = channelReachCode[      ii + parameterData.localChannelElementStart];
        parameterData.channelBaseWidth[ii]       = channelBaseWidth[      ii + parameterData.localChannelElementStart];
        parameterData.channelSideSlope[ii]       = channelSideSlope[      ii + parameterData.localChannelElementStart];
        parameterData.channelManningsN[ii]       = channelManningsN[      ii + parameterData.localChannelElementStart];
        parameterData.channelBedThickness[ii]    = channelBedThickness[   ii + parameterData.localChannelElementStart];
        parameterData.channelBedConductivity[ii] = channelBedConductivity[ii + parameterData.localChannelElementStart];
        
        // Fill in channel state arrays.
        evapoTranspirationStatePupper                | channelEvapoTranspirationState;
        stateData.channelSurfaceWater[ii]            = 0.0;
        stateData.channelSurfaceWaterCreated[ii]     = 0.0;
        stateData.channelPrecipitationRate[ii]       = 0.0;
        stateData.channelPrecipitationCumulative[ii] = 0.0;
        stateData.channelEvaporationRate[ii]         = 0.0;
        stateData.channelEvaporationCumulative[ii]   = 0.0;
        stateData.channelSnowWater[ii]               = 0.0;
        
        for (jj = 0; jj < geometryData.maximumNumberOfChannelNeighbors; ++jj)
        {
            // Fill in channel neighbor geometry arrays.
            geometryData.channelNeighborLocalEndpoint[ ii * geometryData.maximumNumberOfChannelNeighbors + jj] = channelNeighborLocalEndpoint[      (ii + geometryData.localChannelElementStart) * geometryData.maximumNumberOfChannelNeighbors + jj];
            geometryData.channelNeighborRemoteEndpoint[ii * geometryData.maximumNumberOfChannelNeighbors + jj] = channelNeighborRemoteEndpoint[     (ii + geometryData.localChannelElementStart) * geometryData.maximumNumberOfChannelNeighbors + jj];
            geometryData.channelNeighborRemoteElementNumber[ii*geometryData.maximumNumberOfChannelNeighbors+jj]= channelNeighborRemoteElementNumber[(ii + geometryData.localChannelElementStart) * geometryData.maximumNumberOfChannelNeighbors + jj];
            geometryData.channelNeighborEdgeLength[    ii * geometryData.maximumNumberOfChannelNeighbors + jj] = channelNeighborEdgeLength[         (ii + geometryData.localChannelElementStart) * geometryData.maximumNumberOfChannelNeighbors + jj];
            geometryData.channelNeighborEdgeNormalX[   ii * geometryData.maximumNumberOfChannelNeighbors + jj] = channelNeighborEdgeNormalX[        (ii + geometryData.localChannelElementStart) * geometryData.maximumNumberOfChannelNeighbors + jj];
            geometryData.channelNeighborEdgeNormalY[   ii * geometryData.maximumNumberOfChannelNeighbors + jj] = channelNeighborEdgeNormalY[        (ii + geometryData.localChannelElementStart) * geometryData.maximumNumberOfChannelNeighbors + jj];
            geometryData.channelNeighborZOffset[       ii * geometryData.maximumNumberOfChannelNeighbors + jj] = channelNeighborZOffset[            (ii + geometryData.localChannelElementStart) * geometryData.maximumNumberOfChannelNeighbors + jj];
            
            // Fill in channel neighbor parameter arrays.
            parameterData.channelNeighborLocalEndpoint[ii * parameterData.maximumNumberOfChannelNeighbors + jj] = channelNeighborLocalEndpointP[      (ii + parameterData.localChannelElementStart) * parameterData.maximumNumberOfChannelNeighbors + jj];
            parameterData.channelNeighborRemoteEndpoint[ii* parameterData.maximumNumberOfChannelNeighbors + jj] = channelNeighborRemoteEndpointP[     (ii + parameterData.localChannelElementStart) * parameterData.maximumNumberOfChannelNeighbors + jj];
            parameterData.channelNeighborRemoteElementNumber[ii*parameterData.maximumNumberOfChannelNeighbors+jj]=channelNeighborRemoteElementNumberP[(ii + parameterData.localChannelElementStart) * parameterData.maximumNumberOfChannelNeighbors + jj];
            parameterData.channelNeighborRegion[       ii * parameterData.maximumNumberOfChannelNeighbors + jj] = channelNeighborRegion[              (ii + parameterData.localChannelElementStart) * parameterData.maximumNumberOfChannelNeighbors + jj];
            
            // Fill in channel neighbor state arrays.
            stateData.channelNeighborLocalEndpoint[      ii * stateData.maximumNumberOfChannelNeighbors + jj] = channelNeighborLocalEndpointP[      (ii + stateData.localChannelElementStart) * stateData.maximumNumberOfChannelNeighbors + jj];
            stateData.channelNeighborRemoteEndpoint[     ii * stateData.maximumNumberOfChannelNeighbors + jj] = channelNeighborRemoteEndpointP[     (ii + stateData.localChannelElementStart) * stateData.maximumNumberOfChannelNeighbors + jj];
            stateData.channelNeighborRemoteElementNumber[ii * stateData.maximumNumberOfChannelNeighbors + jj] = channelNeighborRemoteElementNumberP[(ii + stateData.localChannelElementStart) * stateData.maximumNumberOfChannelNeighbors + jj];
            stateData.channelNeighborNominalFlowRate[    ii * stateData.maximumNumberOfChannelNeighbors + jj] = 0.0;
            stateData.channelNeighborExpirationTime[     ii * stateData.maximumNumberOfChannelNeighbors + jj] = Readonly::simulationStartTime;
            stateData.channelNeighborInflowCumulative[   ii * stateData.maximumNumberOfChannelNeighbors + jj] = 0.0;
            stateData.channelNeighborOutflowCumulative[  ii * stateData.maximumNumberOfChannelNeighbors + jj] = 0.0;
        }
    }
    
    // Allocate region parameter arrays.
    parameterData.regionNumberOfMeshElements    = new size_t[parameterData.localNumberOfRegions];
    parameterData.regionNumberOfChannelElements = new size_t[parameterData.localNumberOfRegions];
    
    for (ii = 0; ii < parameterData.localNumberOfRegions; ++ii)
    {
        parameterData.regionNumberOfMeshElements[ii]    = regionNumberOfMeshElements[   ii + parameterData.localRegionStart];
        parameterData.regionNumberOfChannelElements[ii] = regionNumberOfChannelElements[ii + parameterData.localRegionStart];
    }
    
    return false;
}

// A NeighborIndices is a helper class for initializing NeighborProxies.
// The information about a single NeighborProxy is split across the geometry, parameter, and state files.
// The same NeighborProxy will not necessarily occur at the same array index in the different files.
// Instead, the information is tagged with its NeighborConnection in each file.
// A NeighborIndices stores which index in each file goes together.
// Additionally, it helps detect errors like duplicate NeighborConnections and missing data.
class NeighborIndices
{
public:
    inline NeighborIndices() : geometryIndex(-1), parameterIndex(-1), stateIndex(-1) {}
    
    int geometryIndex;  // The array index in the geometry  file for this NeighborProxy, or -1 if the index in the geometry  file has not yet been found.
    int parameterIndex; // The array index in the parameter file for this NeighborProxy, or -1 if the index in the parameter file has not yet been found.
    int stateIndex;     // The array index in the state     file for this NeighborProxy, or -1 if the index in the state     file has not yet been found.
};

bool InitializationManager::initializeSimulation()
{
    bool                                                    error = false; // Error flag.
    size_t                                                  ii, jj;        // Loop counters.
    std::map<NeighborConnection, NeighborIndices>::iterator itNeighbor;    // Loop iterator.
    std::map<NeighborConnection, NeighborIndices>           neighborMap;   // Which indices in the geometry, parameter, and state files go together.
    std::map<NeighborConnection, NeighborProxy>             neighbors;     // Neighbors for a newly initialized element.

    // Initialize Noah-MP.
    error = evapoTranspirationInit(Readonly::noahMPMpTableFilePath.c_str(),  Readonly::noahMPVegParmFilePath.c_str(), Readonly::noahMPSoilParmFilePath.c_str(), Readonly::noahMPGenParmFilePath.c_str());
    
    if (!error)
    {
        // FIXME load sizes from file into variables.
        Readonly::globalNumberOfMeshElements = 2;
        Readonly::maximumNumberOfMeshNeighbors = 6;
        Readonly::globalNumberOfChannelElements = 2;
        Readonly::maximumNumberOfChannelNeighbors = 5;
        Readonly::globalNumberOfRegions = 2;
        
        // Set local range of items.
        Readonly::localStartAndNumber(Readonly::localMeshElementStart,    Readonly::localNumberOfMeshElements,    Readonly::globalNumberOfMeshElements,    CkNumPes(), CkMyPe());
        Readonly::localStartAndNumber(Readonly::localChannelElementStart, Readonly::localNumberOfChannelElements, Readonly::globalNumberOfChannelElements, CkNumPes(), CkMyPe());
        Readonly::localStartAndNumber(Readonly::localRegionStart,         Readonly::localNumberOfRegions,         Readonly::globalNumberOfRegions,         CkNumPes(), CkMyPe());
        
        // Error check sizes and local ranges.
        if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
        {
            error = Readonly::checkInvariant();
        }
    }
    
    if (!error)
    {
        // Allocate storage space of the right size.
        geometryData  = new MapGeometry(Readonly::globalNumberOfMeshElements, Readonly::localNumberOfMeshElements, Readonly::localMeshElementStart, Readonly::maximumNumberOfMeshNeighbors,
                                        Readonly::globalNumberOfChannelElements, Readonly::localNumberOfChannelElements, Readonly::localChannelElementStart, Readonly::maximumNumberOfChannelNeighbors);
        parameterData = new MapParameters(Readonly::globalNumberOfMeshElements, Readonly::localNumberOfMeshElements, Readonly::localMeshElementStart, Readonly::maximumNumberOfMeshNeighbors,
                                          Readonly::globalNumberOfChannelElements, Readonly::localNumberOfChannelElements, Readonly::localChannelElementStart, Readonly::maximumNumberOfChannelNeighbors,
                                          Readonly::globalNumberOfRegions, Readonly::localNumberOfRegions, Readonly::localRegionStart);
        stateData     = new TimePointState(Readonly::globalNumberOfMeshElements, Readonly::localNumberOfMeshElements, Readonly::localMeshElementStart, Readonly::maximumNumberOfMeshNeighbors,
                                           Readonly::globalNumberOfChannelElements, Readonly::localNumberOfChannelElements, Readonly::localChannelElementStart, Readonly::maximumNumberOfChannelNeighbors);
        
        // FIXME read from file instead.
        error = initializeHardcodedMap(*geometryData, *parameterData, *stateData);
    }
    
    // FIXME error check that necessary data is present.
    
    if (!error)
    {
        // Notify regions of how many elements they will be receiving.
        for (ii = 0; ii < Readonly::localNumberOfRegions; ++ii)
        {
            ADHydro::regionProxy[ii + Readonly::localRegionStart].sendNumberOfElements(parameterData->regionNumberOfMeshElements[ii], parameterData->regionNumberOfChannelElements[ii]);
        }
        
        // FIXME this code is kind of messy.  There's lots of near-duplicate code between the mesh and channel sections.
        
        // Initialize mesh elements.
        for (ii = 0; !error && ii < Readonly::localNumberOfMeshElements; ++ii)
        {
            neighborMap.clear();
            neighbors.clear();
            
            // Fill in map of neighbor indices.  There is neighbor data in all three data sources: geometry, parameter, and state.
            // It is not guaranteed that neighbors occur in the same order in all three data sources.
            // Instead, the data in each file is tagged with the NeighborConnection.
            // We need to correlate which indices of each file go with which NeighborConnection.
            for (jj = 0; !error && jj < Readonly::maximumNumberOfMeshNeighbors; ++jj)
            {
                if (NO_NEIGHBOR != geometryData->meshNeighborLocalEndpoint[ ii * Readonly::maximumNumberOfMeshNeighbors + jj] &&
                    NO_NEIGHBOR != geometryData->meshNeighborRemoteEndpoint[ii * Readonly::maximumNumberOfMeshNeighbors + jj])
                {
                    NeighborConnection geometryConnection(geometryData->meshNeighborLocalEndpoint[      ii * Readonly::maximumNumberOfMeshNeighbors + jj],
                                                          ii + Readonly::localMeshElementStart,
                                                          geometryData->meshNeighborRemoteEndpoint[     ii * Readonly::maximumNumberOfMeshNeighbors + jj],
                                                          geometryData->meshNeighborRemoteElementNumber[ii * Readonly::maximumNumberOfMeshNeighbors + jj]);
                    
                    // It is an error to have duplicate neighbor entries.
                    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                    {
                        if (-1 != neighborMap[geometryConnection].geometryIndex)
                        {
                            CkError("ERROR in InitializationManager::initializeSimulation: duplicate neighbor geometry for mesh element %d, local endpoint %d, remote endpoint %d, remote element number %d.\n",
                                    geometryConnection.localElementNumber, geometryConnection.localEndpoint, geometryConnection.remoteEndpoint, geometryConnection.remoteElementNumber);
                            error = true;
                        }
                    }
                    
                    if (!error)
                    {
                        neighborMap[geometryConnection].geometryIndex = jj;
                    }
                }
                
                if (NO_NEIGHBOR != parameterData->meshNeighborLocalEndpoint[ ii * Readonly::maximumNumberOfMeshNeighbors + jj] &&
                    NO_NEIGHBOR != parameterData->meshNeighborRemoteEndpoint[ii * Readonly::maximumNumberOfMeshNeighbors + jj])
                {
                    NeighborConnection parameterConnection(parameterData->meshNeighborLocalEndpoint[      ii * Readonly::maximumNumberOfMeshNeighbors + jj],
                                                           ii + Readonly::localMeshElementStart,
                                                           parameterData->meshNeighborRemoteEndpoint[     ii * Readonly::maximumNumberOfMeshNeighbors + jj],
                                                           parameterData->meshNeighborRemoteElementNumber[ii * Readonly::maximumNumberOfMeshNeighbors + jj]);
                    
                    // It is an error to have duplicate neighbor entries.
                    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                    {
                        if (-1 != neighborMap[parameterConnection].parameterIndex)
                        {
                            CkError("ERROR in InitializationManager::initializeSimulation: duplicate neighbor parameters for mesh element %d, local endpoint %d, remote endpoint %d, remote element number %d.\n",
                                    parameterConnection.localElementNumber, parameterConnection.localEndpoint, parameterConnection.remoteEndpoint, parameterConnection.remoteElementNumber);
                            error = true;
                        }
                    }
                    
                    if (!error)
                    {
                        neighborMap[parameterConnection].parameterIndex = jj;
                    }
                }
                
                if (NO_NEIGHBOR != stateData->meshNeighborLocalEndpoint[ ii * Readonly::maximumNumberOfMeshNeighbors + jj] &&
                    NO_NEIGHBOR != stateData->meshNeighborRemoteEndpoint[ii * Readonly::maximumNumberOfMeshNeighbors + jj])
                {
                    NeighborConnection stateConnection(stateData->meshNeighborLocalEndpoint[      ii * Readonly::maximumNumberOfMeshNeighbors + jj],
                                                       ii + Readonly::localMeshElementStart,
                                                       stateData->meshNeighborRemoteEndpoint[     ii * Readonly::maximumNumberOfMeshNeighbors + jj],
                                                       stateData->meshNeighborRemoteElementNumber[ii * Readonly::maximumNumberOfMeshNeighbors + jj]);
                    
                    // It is an error to have duplicate neighbor entries.
                    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                    {
                        if (-1 != neighborMap[stateConnection].stateIndex)
                        {
                            CkError("ERROR in InitializationManager::initializeSimulation: duplicate neighbor state for mesh element %d, local endpoint %d, remote endpoint %d, remote element number %d.\n",
                                    stateConnection.localElementNumber, stateConnection.localEndpoint, stateConnection.remoteEndpoint, stateConnection.remoteElementNumber);
                            error = true;
                        }
                    }
                    
                    if (!error)
                    {
                        neighborMap[stateConnection].stateIndex = jj;
                    }
                }
            }
            
            // Initialize NeighborProxies.
            for (itNeighbor = neighborMap.begin(); !error && itNeighbor != neighborMap.end(); ++itNeighbor)
            {
                // It is an error for information about a neighbor to exist in some of the three sources, but be missing from any of the other sources.
                if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                {
                    if (-1 == itNeighbor->second.geometryIndex || -1 == itNeighbor->second.parameterIndex || -1 == itNeighbor->second.stateIndex)
                    {
                        CkError("ERROR in InitializationManager::initializeSimulation: missing neighbor neighbor data for mesh element %d, local endpoint %d, remote endpoint %d, remote element number %d.\n",
                                itNeighbor->first.localElementNumber, itNeighbor->first.localEndpoint, itNeighbor->first.remoteEndpoint, itNeighbor->first.remoteElementNumber);
                        error = true;
                    }
                }
                
                if (!error)
                {
                    neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(itNeighbor->first, NeighborProxy(parameterData->meshNeighborRegion[       ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.parameterIndex],
                                                                                                                   geometryData->meshNeighborEdgeLength[    ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.geometryIndex ],
                                                                                                                   geometryData->meshNeighborEdgeNormalX[   ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.geometryIndex ],
                                                                                                                   geometryData->meshNeighborEdgeNormalY[   ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.geometryIndex ],
                                                                                                                   geometryData->meshNeighborZOffset[       ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.geometryIndex ],
                                                                                                                   stateData->meshNeighborNominalFlowRate[  ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.stateIndex    ],
                                                                                                                   stateData->meshNeighborExpirationTime[   ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.stateIndex    ],
                                                                                                                   stateData->meshNeighborInflowCumulative[ ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.stateIndex    ],
                                                                                                                   stateData->meshNeighborOutflowCumulative[ii * Readonly::maximumNumberOfMeshNeighbors + itNeighbor->second.stateIndex    ])));
                }
            }
            
            if (!error)
            {
                PUP::fromMem                  evapotranspirationPuper(stateData->meshEvapoTranspirationState[ii]); // Used to get evapoTranspirationState out of a fixed size blob.
                PUP::fromMem                  soilWaterPuper(         stateData->meshSoilWater[ii]);               // Used to get soilWater               out of a fixed size blob.
                PUP::fromMem                  aquiferWaterPuper(      stateData->meshAquiferWater[ii]);            // Used to get aquiferWater            out of a fixed size blob.
                EvapoTranspirationStateStruct evapoTranspirationState;                                             // For passing to MeshElement constructor.
                SimpleVadoseZone              soilWater;                                                           // For passing to MeshElement constructor.
                SimpleVadoseZone              aquiferWater;                                                        // For passing to MeshElement constructor.
                
                // Recreate the state objects by pupping from fixed size blobs.
                evapotranspirationPuper | evapoTranspirationState;
                soilWaterPuper          | soilWater;
                aquiferWaterPuper       | aquiferWater;
                
                // Send the mesh element to the right region.
                ADHydro::regionProxy[parameterData->meshRegion[ii]].sendInitializeMeshElement(MeshElement(ii + Readonly::localMeshElementStart,
                                                                                                          parameterData->meshCatchment[ii],
                                                                                                          geometryData->meshElementX[ii],
                                                                                                          geometryData->meshElementY[ii],
                                                                                                          geometryData->meshElementZ[ii],
                                                                                                          geometryData->meshElementArea[ii],
                                                                                                          geometryData->meshLatitude[ii],
                                                                                                          geometryData->meshLongitude[ii],
                                                                                                          geometryData->meshSlopeX[ii],
                                                                                                          geometryData->meshSlopeY[ii],
                                                                                                          parameterData->meshVegetationType[ii],
                                                                                                          parameterData->meshGroundType[ii],
                                                                                                          parameterData->meshManningsN[ii],
                                                                                                          parameterData->meshSoilExists[ii],
                                                                                                          parameterData->meshImpedanceConductivity[ii],
                                                                                                          parameterData->meshAquiferExists[ii],
                                                                                                          parameterData->meshDeepConductivity[ii],
                                                                                                          &evapoTranspirationState,
                                                                                                          stateData->meshSurfaceWater[ii],
                                                                                                          stateData->meshSurfaceWaterCreated[ii],
                                                                                                          stateData->meshGroundwaterMode[ii],
                                                                                                          stateData->meshPerchedHead[ii],
                                                                                                          soilWater,
                                                                                                          stateData->meshSoilWaterCreated[ii],
                                                                                                          stateData->meshAquiferHead[ii],
                                                                                                          aquiferWater,
                                                                                                          stateData->meshAquiferWaterCreated[ii],
                                                                                                          stateData->meshDeepGroundwater[ii],
                                                                                                          stateData->meshPrecipitationCumulative[ii],
                                                                                                          stateData->meshEvaporationCumulative[ii],
                                                                                                          stateData->meshTranspirationCumulative[ii],
                                                                                                          neighbors));
            }
        }
        
        // Initialize channel elements.
        for (ii = 0; !error && ii < Readonly::localNumberOfChannelElements; ++ii)
        {
            neighborMap.clear();
            neighbors.clear();
            
            // Fill in map of neighbor indices.  There is neighbor data in all three data sources: geometry, parameter, and state.
            // It is not guaranteed that neighbors occur in the same order in all three data sources.
            // Instead, the data in each file is tagged with the NeighborConnection.
            // We need to correlate which indices of each file go with which NeighborConnection.
            for (jj = 0; !error && jj < Readonly::maximumNumberOfChannelNeighbors; ++jj)
            {
                if (NO_NEIGHBOR != geometryData->channelNeighborLocalEndpoint[ ii * Readonly::maximumNumberOfChannelNeighbors + jj] &&
                    NO_NEIGHBOR != geometryData->channelNeighborRemoteEndpoint[ii * Readonly::maximumNumberOfChannelNeighbors + jj])
                {
                    NeighborConnection geometryConnection(geometryData->channelNeighborLocalEndpoint[      ii * Readonly::maximumNumberOfChannelNeighbors + jj],
                                                          ii + Readonly::localChannelElementStart,
                                                          geometryData->channelNeighborRemoteEndpoint[     ii * Readonly::maximumNumberOfChannelNeighbors + jj],
                                                          geometryData->channelNeighborRemoteElementNumber[ii * Readonly::maximumNumberOfChannelNeighbors + jj]);
                    
                    // It is an error to have duplicate neighbor entries.
                    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                    {
                        if (-1 != neighborMap[geometryConnection].geometryIndex)
                        {
                            CkError("ERROR in InitializationManager::initializeSimulation: duplicate neighbor geometry for channel element %d, local endpoint %d, remote endpoint %d, remote element number %d.\n",
                                    geometryConnection.localElementNumber, geometryConnection.localEndpoint, geometryConnection.remoteEndpoint, geometryConnection.remoteElementNumber);
                            error = true;
                        }
                    }
                    
                    if (!error)
                    {
                        neighborMap[geometryConnection].geometryIndex = jj;
                    }
                }
                
                if (NO_NEIGHBOR != parameterData->channelNeighborLocalEndpoint[ ii * Readonly::maximumNumberOfChannelNeighbors + jj] &&
                    NO_NEIGHBOR != parameterData->channelNeighborRemoteEndpoint[ii * Readonly::maximumNumberOfChannelNeighbors + jj])
                {
                    NeighborConnection parameterConnection(parameterData->channelNeighborLocalEndpoint[      ii * Readonly::maximumNumberOfChannelNeighbors + jj],
                                                           ii + Readonly::localChannelElementStart,
                                                           parameterData->channelNeighborRemoteEndpoint[     ii * Readonly::maximumNumberOfChannelNeighbors + jj],
                                                           parameterData->channelNeighborRemoteElementNumber[ii * Readonly::maximumNumberOfChannelNeighbors + jj]);
                    
                    // It is an error to have duplicate neighbor entries.
                    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                    {
                        if (-1 != neighborMap[parameterConnection].parameterIndex)
                        {
                            CkError("ERROR in InitializationManager::initializeSimulation: duplicate neighbor parameters for channel element %d, local endpoint %d, remote endpoint %d, remote element number %d.\n",
                                    parameterConnection.localElementNumber, parameterConnection.localEndpoint, parameterConnection.remoteEndpoint, parameterConnection.remoteElementNumber);
                            error = true;
                        }
                    }
                    
                    if (!error)
                    {
                        neighborMap[parameterConnection].parameterIndex = jj;
                    }
                }
                
                if (NO_NEIGHBOR != stateData->channelNeighborLocalEndpoint[ ii * Readonly::maximumNumberOfChannelNeighbors + jj] &&
                    NO_NEIGHBOR != stateData->channelNeighborRemoteEndpoint[ii * Readonly::maximumNumberOfChannelNeighbors + jj])
                {
                    NeighborConnection stateConnection(stateData->channelNeighborLocalEndpoint[      ii * Readonly::maximumNumberOfChannelNeighbors + jj],
                                                       ii + Readonly::localChannelElementStart,
                                                       stateData->channelNeighborRemoteEndpoint[     ii * Readonly::maximumNumberOfChannelNeighbors + jj],
                                                       stateData->channelNeighborRemoteElementNumber[ii * Readonly::maximumNumberOfChannelNeighbors + jj]);
                    
                    // It is an error to have duplicate neighbor entries.
                    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                    {
                        if (-1 != neighborMap[stateConnection].stateIndex)
                        {
                            CkError("ERROR in InitializationManager::initializeSimulation: duplicate neighbor state for channel element %d, local endpoint %d, remote endpoint %d, remote element number %d.\n",
                                    stateConnection.localElementNumber, stateConnection.localEndpoint, stateConnection.remoteEndpoint, stateConnection.remoteElementNumber);
                            error = true;
                        }
                    }
                    
                    if (!error)
                    {
                        neighborMap[stateConnection].stateIndex = jj;
                    }
                }
            }
            
            // Initialize NeighborProxies.
            for (itNeighbor = neighborMap.begin(); !error && itNeighbor != neighborMap.end(); ++itNeighbor)
            {
                // It is an error for information about a neighbor to exist in some of the three sources, but be missing from any of the other sources.
                if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
                {
                    if (-1 == itNeighbor->second.geometryIndex || -1 == itNeighbor->second.parameterIndex || -1 == itNeighbor->second.stateIndex)
                    {
                        CkError("ERROR in InitializationManager::initializeSimulation: missing neighbor neighbor data for channel element %d, local endpoint %d, remote endpoint %d, remote element number %d.\n",
                                itNeighbor->first.localElementNumber, itNeighbor->first.localEndpoint, itNeighbor->first.remoteEndpoint, itNeighbor->first.remoteElementNumber);
                        error = true;
                    }
                }
                
                if (!error)
                {
                    neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(itNeighbor->first, NeighborProxy(parameterData->channelNeighborRegion[       ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.parameterIndex],
                                                                                                                   geometryData->channelNeighborEdgeLength[    ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.geometryIndex ],
                                                                                                                   geometryData->channelNeighborEdgeNormalX[   ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.geometryIndex ],
                                                                                                                   geometryData->channelNeighborEdgeNormalY[   ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.geometryIndex ],
                                                                                                                   geometryData->channelNeighborZOffset[       ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.geometryIndex ],
                                                                                                                   stateData->channelNeighborNominalFlowRate[  ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.stateIndex    ],
                                                                                                                   stateData->channelNeighborExpirationTime[   ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.stateIndex    ],
                                                                                                                   stateData->channelNeighborInflowCumulative[ ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.stateIndex    ],
                                                                                                                   stateData->channelNeighborOutflowCumulative[ii * Readonly::maximumNumberOfChannelNeighbors + itNeighbor->second.stateIndex    ])));
                }
            }
            
            if (!error)
            {
                PUP::fromMem                  evapotranspirationPuper(stateData->channelEvapoTranspirationState[ii]); // Used to get evapoTranspirationState out of a fixed size blob.
                EvapoTranspirationStateStruct evapoTranspirationState;                                                // For passing to ChannelElement constructor.
                
                // Recreate the state objects by pupping from fixed size blobs.
                evapotranspirationPuper | evapoTranspirationState;
                
                // Send the channel element to the right region.
                ADHydro::regionProxy[parameterData->channelRegion[ii]].sendInitializeChannelElement(ChannelElement(ii + Readonly::localChannelElementStart,
                                                                                                                   parameterData->channelChannelType[ii],
                                                                                                                   parameterData->channelReachCode[ii],
                                                                                                                   geometryData->channelElementX[ii],
                                                                                                                   geometryData->channelElementY[ii],
                                                                                                                   geometryData->channelElementZBank[ii],
                                                                                                                   geometryData->channelElementZBed[ii],
                                                                                                                   geometryData->channelElementLength[ii],
                                                                                                                   geometryData->channelLatitude[ii],
                                                                                                                   geometryData->channelLongitude[ii],
                                                                                                                   parameterData->channelBaseWidth[ii],
                                                                                                                   parameterData->channelSideSlope[ii],
                                                                                                                   parameterData->channelManningsN[ii],
                                                                                                                   parameterData->channelBedThickness[ii],
                                                                                                                   parameterData->channelBedConductivity[ii],
                                                                                                                   &evapoTranspirationState,
                                                                                                                   stateData->channelSurfaceWater[ii],
                                                                                                                   stateData->channelSurfaceWaterCreated[ii],
                                                                                                                   stateData->meshPrecipitationCumulative[ii],
                                                                                                                   stateData->meshEvaporationCumulative[ii],
                                                                                                                   neighbors));
            }
        }
    }
    
    return error;
}
