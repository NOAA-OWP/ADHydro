#include "adhydro.h"
#include "initialization_manager.h"
#include "readonly.h"
#include "initialization_manager.def.h"

void InitializationManager::initializeSimulation()
{
    // Initialize Noah-MP.
    if (evapoTranspirationInit(Readonly::noahMPMpTableFilePath.c_str(),  Readonly::noahMPVegParmFilePath.c_str(), Readonly::noahMPSoilParmFilePath.c_str(), Readonly::noahMPGenParmFilePath.c_str()))
    {
        CkExit();
    }
    
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
    
    if (DEBUG_LEVEL & DEBUG_LEVEL_USER_INPUT_SIMPLE)
    {
        if (Readonly::checkInvariant())
        {
            CkExit();
        }
    }
    
    // FIXME load data from file into variables and error check it.
    meshRegion    = new int[Readonly::localNumberOfMeshElements];
    channelRegion = new int[Readonly::localNumberOfChannelElements];
    
    for (size_t ii = 0; ii < Readonly::localNumberOfMeshElements; ++ii)
    {
        meshRegion[ii + Readonly::localMeshElementStart] = ii + Readonly::localMeshElementStart;
    }
    
    for (size_t ii = 0; ii < Readonly::localNumberOfChannelElements; ++ii)
    {
        channelRegion[ii + Readonly::localChannelElementStart] = ii + Readonly::localChannelElementStart;
    }
    
    // FIXME initialize elements from data loaded from file.  For now, hard code a mesh.
    if (0 == CkMyPe())
    {
        ADHydro::regionProxy[0].sendNumberOfElements(1, 1);
        ADHydro::regionProxy[1].sendNumberOfElements(1, 1);
        
        std::map<NeighborConnection, NeighborProxy> neighbors;
        
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SURFACE, 0, MESH_SURFACE,    1), NeighborProxy(1, 100.0, -1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SOIL,    0, MESH_SOIL,       1), NeighborProxy(1, 100.0, -1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_AQUIFER, 0, MESH_AQUIFER,    1), NeighborProxy(1, 100.0, -1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SURFACE, 0, CHANNEL_SURFACE, 0), NeighborProxy(0, 100.0, 0.0, -1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SOIL   , 0, CHANNEL_SURFACE, 0), NeighborProxy(0, 100.0, 0.0, -1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_AQUIFER, 0, CHANNEL_SURFACE, 0), NeighborProxy(0, 100.0, 0.0, -1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        
        ADHydro::regionProxy[0].sendInitializeMeshElement(MeshElement(0, 0, 0.0, 0.0, 0.0, 10000.0, 0.0, 0.0, -0.02, -0.02, 11, 2, 0.16, true, 3.38E-6, true, 9.74E-7, NULL, 0.0, 0.0, UNSATURATED_AQUIFER, 0.0,
                                                                      SimpleVadoseZone(0.1, 1.41E-5, 0.421, 0.0426, 0.0, 0.9), 0.0, -0.9, SimpleVadoseZone(0.8, 4.66E-5, 0.339, 0.0279, 0.0, 0.8), 0.0, 0.0, 0.0, 0.0, 0.0, neighbors));
        
        neighbors.clear();
        
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, MESH_SURFACE,    0), NeighborProxy(0, 100.0, 0.0, 1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, MESH_SOIL   ,    0), NeighborProxy(0, 100.0, 0.0, 1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, MESH_AQUIFER,    0), NeighborProxy(0, 100.0, 0.0, 1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 0, CHANNEL_SURFACE, 1), NeighborProxy(1, 1.0, -1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        
        ADHydro::regionProxy[0].sendInitializeChannelElement(ChannelElement(0, STREAM, 0, 0.0, -50.0, -1.0, -2.0, 100.0, 0.0, 0.0, 1.0, 1.0, 0.038, 1.0, 1.41E-5, NULL, 0.0, 0.0, 0.0, 0.0, neighbors));
        
        neighbors.clear();
        
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SURFACE, 1, MESH_SURFACE,    0), NeighborProxy(0, 100.0, 1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SOIL,    1, MESH_SOIL,       0), NeighborProxy(0, 100.0, 1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_AQUIFER, 1, MESH_AQUIFER,    0), NeighborProxy(0, 100.0, 1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SURFACE, 1, CHANNEL_SURFACE, 1), NeighborProxy(1, 100.0, 0.0, -1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_SOIL   , 1, CHANNEL_SURFACE, 1), NeighborProxy(1, 100.0, 0.0, -1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(MESH_AQUIFER, 1, CHANNEL_SURFACE, 1), NeighborProxy(1, 100.0, 0.0, -1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        
        ADHydro::regionProxy[1].sendInitializeMeshElement(MeshElement(1, 1, -100.0, 0.0, -2.0, 10000.0, 0.0, 0.0, -0.02, -0.02, 11, 2, 0.16, true, 3.38E-6, true, 9.74E-7, NULL, 0.0, 0.0, UNSATURATED_AQUIFER, 0.0,
                                                                      SimpleVadoseZone(0.1, 1.41E-5, 0.421, 0.0426, 0.0, 0.9), 0.0, -1.1, SimpleVadoseZone(0.8, 4.66E-5, 0.339, 0.0279, 0.0, 0.8), 0.0, 0.0, 0.0, 0.0, 0.0, neighbors));
        
        neighbors.clear();
        
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, MESH_SURFACE,     1), NeighborProxy(1, 100.0, 0.0, 1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, MESH_SOIL   ,     1), NeighborProxy(1, 100.0, 0.0, 1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, MESH_AQUIFER,     1), NeighborProxy(1, 100.0, 0.0, 1.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, CHANNEL_SURFACE,  0), NeighborProxy(0, 1.0, 1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        neighbors.insert(std::pair<NeighborConnection, NeighborProxy>(NeighborConnection(CHANNEL_SURFACE, 1, BOUNDARY_OUTFLOW, 0), NeighborProxy(0, 1.0, -1.0, 0.0, 0.0, Readonly::simulationStartTime, 0.0, 0.0)));
        
        ADHydro::regionProxy[1].sendInitializeChannelElement(ChannelElement(1, STREAM, 1, -100.0, -50.0, -3.0, -4.0, 100.0, 0.0, 0.0, 1.0, 1.0, 0.038, 1.0, 1.41E-5, NULL, 0.0, 0.0, 0.0, 0.0, neighbors));
    }
}
