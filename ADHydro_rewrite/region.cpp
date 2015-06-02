#include "region.h"
#include "garto.h"

Region::Region(double referenceDateInit, double currentTimeInit, double simulationEndTimeInit) :
  Element(referenceDateInit, currentTimeInit, simulationEndTimeInit)
{
  nextSyncTime = currentTime + 100.0;
  
  if (nextSyncTime > simulationEndTime)
    {
      nextSyncTime = simulationEndTime;
    }
  
  // FIXME only needed for hardcoded mesh
  EvapoTranspirationForcingStruct evapoTranspirationForcingInit;
  EvapoTranspirationStateStruct   evapoTranspirationStateInit;
  
  evapoTranspirationForcingInit.dz8w   = 20.0;
  evapoTranspirationForcingInit.sfcTmp = 300.0;
  evapoTranspirationForcingInit.sfcPrs = 101300.0;
  evapoTranspirationForcingInit.psfc   = 101200.0;
  evapoTranspirationForcingInit.uu     = 0.0;
  evapoTranspirationForcingInit.vv     = 0.0;
  evapoTranspirationForcingInit.q2     = 0.0;
  evapoTranspirationForcingInit.qc     = 0.0;
  evapoTranspirationForcingInit.solDn  = 1000.0;
  evapoTranspirationForcingInit.lwDn   = 300.0;
  evapoTranspirationForcingInit.prcp   = 0.0;
  evapoTranspirationForcingInit.tBot   = 300.0;
  evapoTranspirationForcingInit.pblh   = 10000.0;
  
  evapoTranspirationStateInit.fIceOld[0] = 0.0;
  evapoTranspirationStateInit.fIceOld[1] = 0.0;
  evapoTranspirationStateInit.fIceOld[2] = 0.0;
  evapoTranspirationStateInit.albOld     = 1.0;
  evapoTranspirationStateInit.snEqvO     = 0.0;
  evapoTranspirationStateInit.stc[0]     = 0.0;
  evapoTranspirationStateInit.stc[1]     = 0.0;
  evapoTranspirationStateInit.stc[2]     = 0.0;
  evapoTranspirationStateInit.stc[3]     = 300.0;
  evapoTranspirationStateInit.stc[4]     = 300.0;
  evapoTranspirationStateInit.stc[5]     = 300.0;
  evapoTranspirationStateInit.stc[6]     = 300.0;
  evapoTranspirationStateInit.tah        = 300.0;
  evapoTranspirationStateInit.eah        = 0.0;
  evapoTranspirationStateInit.fWet       = 0.0;
  evapoTranspirationStateInit.canLiq     = 0.0;
  evapoTranspirationStateInit.canIce     = 0.0;
  evapoTranspirationStateInit.tv         = 300.0;
  evapoTranspirationStateInit.tg         = 300.0;
  evapoTranspirationStateInit.iSnow      = 0;
  evapoTranspirationStateInit.zSnso[0]   = 0.0;
  evapoTranspirationStateInit.zSnso[1]   = 0.0;
  evapoTranspirationStateInit.zSnso[2]   = 0.0;
  evapoTranspirationStateInit.zSnso[3]   = -0.05;
  evapoTranspirationStateInit.zSnso[4]   = -0.2;
  evapoTranspirationStateInit.zSnso[5]   = -0.5;
  evapoTranspirationStateInit.zSnso[6]   = -1.0;
  evapoTranspirationStateInit.snowH      = 0.0;
  evapoTranspirationStateInit.snEqv      = 0.0;
  evapoTranspirationStateInit.snIce[0]   = 0.0;
  evapoTranspirationStateInit.snIce[1]   = 0.0;
  evapoTranspirationStateInit.snIce[2]   = 0.0;
  evapoTranspirationStateInit.snLiq[0]   = 0.0;
  evapoTranspirationStateInit.snLiq[1]   = 0.0;
  evapoTranspirationStateInit.snLiq[2]   = 0.0;
  evapoTranspirationStateInit.lfMass     = 100000.0;
  evapoTranspirationStateInit.rtMass     = 100000.0;
  evapoTranspirationStateInit.stMass     = 100000.0;
  evapoTranspirationStateInit.wood       = 200000.0;
  evapoTranspirationStateInit.stblCp     = 200000.0;
  evapoTranspirationStateInit.fastCp     = 200000.0;
  evapoTranspirationStateInit.lai        = 4.6;
  evapoTranspirationStateInit.sai        = 0.6;
  evapoTranspirationStateInit.cm         = 0.002;
  evapoTranspirationStateInit.ch         = 0.002;
  evapoTranspirationStateInit.tauss      = 0.0;
  evapoTranspirationStateInit.deepRech   = 0.0;
  evapoTranspirationStateInit.rech       = 0.0;
  
  // FIXME Create hardcoded mesh.
  switch (thisIndex)
  {
  case 0:
    meshElements.insert(std::pair<int, MeshElement>(0, MeshElement(0, 0, 8, 1,  200.0, 200.0,  2.0, 1.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0,  2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[0].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0,       1, 0, 400.0, 400.0, 4.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.038));
    meshElements[0].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, OUTFLOW, 0,   NAN,   NAN, NAN,      NAN, 600.0,             -1.0,            0.0,             NAN));
    meshElements[0].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0,       1, 0, 400.0, 400.0, 4.0, 3.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[0].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, OUTFLOW, 0,   NAN,   NAN, 0.0, NAN,      NAN, 600.0,             -1.0,            0.0,                 NAN,   NAN));
    meshElements[0].channelNeighbors.push_back(MeshSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 0, 4.5, -5.5, 2.5, 600.0, 1.0, 1.0));
    meshElements[0].underground.channelNeighbors.push_back(MeshGroundwaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 0, 4.5, -5.5, 2.5, 600.0, 1.0, 1.0, 0.000555, 1.0));
    meshElements.insert(std::pair<int, MeshElement>(1, MeshElement(1, 0, 8, 1,  400.0, 400.0,  4.0, 3.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0,  4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[1].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 0, 200.0, 200.0, 2.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.038));
    meshElements[1].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 2, 0, 800.0, 200.0, 8.0, 180000.0, 600.0,             1.0,              0.0,              0.038));
    meshElements[1].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 0, 200.0, 200.0, 2.0, 1.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[1].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 2, 0, 800.0, 200.0, 8.0, 7.0, 180000.0, 600.0,             1.0,              0.0,              4.66E-5, 0.339));
    meshElements.insert(std::pair<int, MeshElement>(2, MeshElement(2, 0, 8, 1,  800.0, 200.0,  8.0, 7.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0,  8.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[2].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 1, 1,  400.0, 400.0,  4.0, 180000.0, 600.0,             -1.0,            0.0,             0.038));
    meshElements[2].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 3, 0, 1000.0, 400.0, 10.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.038));
    meshElements[2].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 1, 1,  400.0, 400.0,  4.0, 3.0, 180000.0, 600.0,             -1.0,            0.0,             4.66E-5, 0.339));
    meshElements[2].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 3, 0, 1000.0, 400.0, 10.0, 9.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[2].channelNeighbors.push_back(MeshSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 1,  4.5, -5.5, -3.5, 300.0, 1.0, 1.0));
    meshElements[2].channelNeighbors.push_back(MeshSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 1, 0, 13.5,  3.5,  5.5, 300.0, 1.0, 1.0));
    meshElements[2].underground.channelNeighbors.push_back(MeshGroundwaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 1,  4.5, -5.5, -3.5, 300.0, 1.0, 1.0, 0.000555, 1.0));
    meshElements[2].underground.channelNeighbors.push_back(MeshGroundwaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 1, 0, 13.5,  3.5,  5.5, 300.0, 1.0, 1.0, 0.000555, 1.0));
    meshElements.insert(std::pair<int, MeshElement>(3, MeshElement(3, 0, 8, 1, 1000.0, 400.0, 10.0, 9.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[3].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 2, 1,  800.0, 200.0,  8.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.038));
    meshElements[3].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 4, 0, 1400.0, 200.0, 14.0, 180000.0, 600.0,             1.0,              0.0,              0.038));
    meshElements[3].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 2, 1,  800.0, 200.0,  8.0,  7.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[3].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 4, 0, 1400.0, 200.0, 14.0, 13.0, 180000.0, 600.0,             1.0,              0.0,              4.66E-5, 0.339));
    channelElements.insert(std::pair<int, ChannelElement>(0, ChannelElement(0, STREAM, 0, 4.5, -5.5, 900.0, 1.0, 1.0, 4.66E-5, 1.0, 0.038, 0.0, 0.0)));
    channelElements[0].meshNeighbors.push_back(ChannelSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 0, 2.0,  2.5, 180000.0, 600.0));
    channelElements[0].meshNeighbors.push_back(ChannelSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 2, 0, 8.0, -3.5, 180000.0, 300.0));
    channelElements[0].channelNeighbors.push_back(ChannelSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1,       1, 0, STREAM, 13.5, 3.5, 900.0, 1.0, 1.0, 0.038));
    channelElements[0].channelNeighbors.push_back(ChannelSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, OUTFLOW, 0, STREAM,  NAN,  NAN,   NAN, NAN, NAN,   NAN));
    channelElements[0].undergroundMeshNeighbors.push_back(ChannelGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 0, 2.0, 1.0,  2.5, 600.0));
    channelElements[0].undergroundMeshNeighbors.push_back(ChannelGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 2, 0, 8.0, 7.0, -3.5, 300.0));
    break;
  case 1:
    meshElements.insert(std::pair<int, MeshElement>(4, MeshElement(4, 1, 8, 1, 1400.0, 200.0, 14.0, 13.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 14.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[4].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 3, 1, 1000.0, 400.0, 10.0, 180000.0, 600.0,             -1.0,            0.0,             0.038));
    meshElements[4].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 5, 0, 1600.0, 400.0, 16.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.038));
    meshElements[4].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 3, 1, 1000.0, 400.0, 10.0,  9.0, 180000.0, 600.0,             -1.0,            0.0,             4.66E-5, 0.339));
    meshElements[4].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 5, 0, 1600.0, 400.0, 16.0, 15.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[4].channelNeighbors.push_back(MeshSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 1, 1, 13.5, 3.5, -0.5, 600.0, 1.0, 1.0));
    meshElements[4].underground.channelNeighbors.push_back(MeshGroundwaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 1, 1, 13.5, 3.5, -0.5, 600.0, 1.0, 1.0, 0.000555, 1.0));
    meshElements.insert(std::pair<int, MeshElement>(5, MeshElement(5, 1, 8, 1, 1600.0, 400.0, 16.0, 15.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 16.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[5].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 4, 1, 1400.0, 200.0, 14.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.038));
    meshElements[5].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 6, 0, 2000.0, 200.0, 20.0, 180000.0, 600.0,             1.0,              0.0,              0.038));
    meshElements[5].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 4, 1, 1400.0, 200.0, 14.0, 13.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[5].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 6, 0, 2000.0, 200.0, 20.0, 19.0, 180000.0, 600.0,             1.0,              0.0,              4.66E-5, 0.339));
    meshElements.insert(std::pair<int, MeshElement>(6, MeshElement(6, 1, 8, 1, 2000.0, 200.0, 20.0, 19.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[6].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 5, 1, 1600.0, 400.0, 16.0, 180000.0, 600.0,             -1.0,            0.0,             0.038));
    meshElements[6].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 7, 0, 2200.0, 400.0, 22.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.038));
    meshElements[6].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 5, 1, 1600.0, 400.0, 16.0, 15.0, 180000.0, 600.0,             -1.0,            0.0,             4.66E-5, 0.339));
    meshElements[6].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 7, 0, 2200.0, 400.0, 22.0, 21.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[6].channelNeighbors.push_back(MeshSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 2, 0, 22.5, 12.5, 2.5, 600.0, 1.0, 1.0));
    meshElements[6].underground.channelNeighbors.push_back(MeshGroundwaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 2, 0, 22.5, 12.5, 2.5, 600.0, 1.0, 1.0, 0.000555, 1.0));
    meshElements.insert(std::pair<int, MeshElement>(7, MeshElement(7, 1, 8, 1, 2200.0, 400.0, 22.0, 21.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 22.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[7].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 6, 1, 2000.0, 200.0, 20.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.038));
    meshElements[7].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 8, 0, 2600.0, 200.0, 26.0, 180000.0, 600.0,             1.0,              0.0,              0.038));
    meshElements[7].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 6, 1, 2000.0, 200.0, 20.0, 19.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[7].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 8, 0, 2600.0, 200.0, 26.0, 25.0, 180000.0, 600.0,             1.0,              0.0,              4.66E-5, 0.339));
    channelElements.insert(std::pair<int, ChannelElement>(1, ChannelElement(1, STREAM, 1, 13.5, 3.5, 900.0, 1.0, 1.0, 4.66E-5, 1.0, 0.038, 0.0, 0.0)));
    channelElements[1].meshNeighbors.push_back(ChannelSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 2, 1,  8.0,  5.5, 180000.0, 300.0));
    channelElements[1].meshNeighbors.push_back(ChannelSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 4, 0, 14.0, -0.5, 180000.0, 600.0));
    channelElements[1].channelNeighbors.push_back(ChannelSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 0, 0, STREAM,  4.5, -5.5, 900.0, 1.0, 1.0, 0.038));
    channelElements[1].channelNeighbors.push_back(ChannelSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 2, 0, STREAM, 22.5, 12.5, 900.0, 1.0, 1.0, 0.038));
    channelElements[1].undergroundMeshNeighbors.push_back(ChannelGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 0, 2, 1,  8.0,  7.0,  5.5, 300.0));
    channelElements[1].undergroundMeshNeighbors.push_back(ChannelGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 4, 0, 14.0, 13.0, -0.5, 600.0));
    channelElements.insert(std::pair<int, ChannelElement>(2, ChannelElement(2, STREAM, 1, 22.5, 12.5, 900.0, 1.0, 1.0, 4.66E-5, 1.0, 0.038, 0.0, 0.0)));
    channelElements[2].meshNeighbors.push_back(ChannelSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 6, 0, 20.0,  2.5, 180000.0, 600.0));
    channelElements[2].meshNeighbors.push_back(ChannelSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 8, 0, 26.0, -3.5, 180000.0, 300.0));
    channelElements[2].channelNeighbors.push_back(ChannelSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 1, 1, STREAM, 13.5,  3.5, 900.0, 1.0, 1.0, 0.038));
    channelElements[2].channelNeighbors.push_back(ChannelSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 3, 0, STREAM, 31.5, 21.5, 900.0, 1.0, 1.0, 0.038));
    channelElements[2].undergroundMeshNeighbors.push_back(ChannelGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 6, 0, 20.0, 19.0,  2.5, 600.0));
    channelElements[2].undergroundMeshNeighbors.push_back(ChannelGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 8, 0, 26.0, 25.0, -3.5, 300.0));
    break;
  case 2:
    meshElements.insert(std::pair<int, MeshElement>( 8, MeshElement( 8, 2, 8, 1, 2600.0, 200.0, 26.0, 25.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 26.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[8].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 7, 1, 2200.0, 400.0, 22.0, 180000.0, 600.0,             -1.0,            0.0,             0.038));
    meshElements[8].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 9, 0, 2800.0, 400.0, 28.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.038));
    meshElements[8].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 7, 1, 2200.0, 400.0, 22.0, 21.0, 180000.0, 600.0,             -1.0,            0.0,             4.66E-5, 0.339));
    meshElements[8].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 9, 0, 2800.0, 400.0, 28.0, 27.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[8].channelNeighbors.push_back(MeshSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 2, 1, 22.5, 12.5, -3.5, 300.0, 1.0, 1.0));
    meshElements[8].channelNeighbors.push_back(MeshSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 3, 0, 31.5, 21.5,  5.5, 300.0, 1.0, 1.0));
    meshElements[8].underground.channelNeighbors.push_back(MeshGroundwaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 2, 1, 22.5, 12.5, -3.5, 300.0, 1.0, 1.0, 0.000555, 1.0));
    meshElements[8].underground.channelNeighbors.push_back(MeshGroundwaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 3, 0, 31.5, 21.5,  5.5, 300.0, 1.0, 1.0, 0.000555, 1.0));
    meshElements.insert(std::pair<int, MeshElement>( 9, MeshElement( 9, 2, 8, 1, 2800.0, 400.0, 28.0, 27.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 28.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[9].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2,  8, 1, 2600.0, 200.0, 26.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.038));
    meshElements[9].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 10, 0, 3200.0, 200.0, 32.0, 180000.0, 600.0,             1.0,              0.0,              0.038));
    meshElements[9].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2,  8, 1, 2600.0, 200.0, 26.0, 25.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[9].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 10, 0, 3200.0, 200.0, 32.0, 31.0, 180000.0, 600.0,             1.0,              0.0,              4.66E-5, 0.339));
    meshElements.insert(std::pair<int, MeshElement>(10, MeshElement(10, 2, 8, 1, 3200.0, 200.0, 32.0, 31.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 32.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::TRIVIAL_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, NULL)));
    meshElements[10].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2,  9, 1, 2800.0, 400.0, 28.0, 180000.0, 600.0,             -1.0,            0.0,             0.038));
    meshElements[10].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 11, 0, 3400.0, 400.0, 34.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.038));
    meshElements[10].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2,  9, 1, 2800.0, 400.0, 28.0, 27.0, 180000.0, 600.0,             -1.0,            0.0,             4.66E-5, 0.339));
    meshElements[10].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 11, 0, 3400.0, 400.0, 34.0, 33.0, 180000.0, 600.0 * sqrt(2.0), sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    meshElements[10].channelNeighbors.push_back(MeshSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 3, 1, 31.5, 21.5, -0.5, 600.0, 1.0, 1.0));
    meshElements[10].underground.channelNeighbors.push_back(MeshGroundwaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 3, 1, 31.5, 21.5, -0.5, 600.0, 1.0, 1.0, 0.000555, 1.0));
    
    // Precipitation only in element 11.
    evapoTranspirationForcingInit.prcp = 10.0;
    
    garto_parameters* parameters; // For creating GARTO domain.
    garto_domain*     domain;     // For creating GARTO domain.
    
    garto_parameters_alloc(&parameters, 5, 4.66E-5, 0.339, 0.01, false, 0.0, 0.0, 1.0, 0.15);
    garto_domain_alloc(&domain, parameters, 0.0, 1.0, true, 0.1, true, 0.5);
    
    meshElements.insert(std::pair<int, MeshElement>(11, MeshElement(11, 2, 8, 1, 3400.0, 400.0, 34.0, 33.0, 180000.0, 1.0, 0.0, 0.7, -1.9, 0.038, 4.66E-5, 0.339, 0.0, 0.0, 33.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, evapoTranspirationForcingInit, evapoTranspirationStateInit, InfiltrationAndGroundwater::GARTO_INFILTRATION, InfiltrationAndGroundwater::SHALLOW_AQUIFER, domain)));
    meshElements[11].meshNeighbors.push_back(MeshSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 10, 1, 3200.0, 200.0, 32.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.038));
    meshElements[11].underground.meshNeighbors.push_back(MeshGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 10, 1, 3200.0, 200.0, 32.0, 31.0, 180000.0, 600.0 * sqrt(2.0), -sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 4.66E-5, 0.339));
    channelElements.insert(std::pair<int, ChannelElement>(3, ChannelElement(3, STREAM, 2, 31.5, 21.5, 900.0, 1.0, 1.0, 4.66E-5, 1.0, 0.038, 0.0, 0.0)));
    channelElements[3].meshNeighbors.push_back(ChannelSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2,  8, 1, 26.0,  5.5, 180000.0, 300.0));
    channelElements[3].meshNeighbors.push_back(ChannelSurfacewaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 10, 0, 32.0, -0.5, 180000.0, 600.0));
    channelElements[3].channelNeighbors.push_back(ChannelSurfacewaterChannelNeighborProxy(currentTime, 0.0, 0.0, 0.0, 1, 2, 1, STREAM, 22.5, 12.5, 900.0, 1.0, 1.0, 0.038));
    channelElements[3].undergroundMeshNeighbors.push_back(ChannelGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2,  8, 1, 26.0, 25.0,  5.5, 300.0));
    channelElements[3].undergroundMeshNeighbors.push_back(ChannelGroundwaterMeshNeighborProxy(currentTime, 0.0, 0.0, 0.0, 2, 10, 0, 32.0, 31.0, -0.5, 600.0));
    break;
  }
  
  regionalDtLimit = 1.0;
  
  thisProxy[thisIndex].runUntilSimulationEnd();
}

Region::Region(CkMigrateMessage* msg) :
  Element(1721425.5, 0.0, 0.0) // Dummy values will be overwritten by pup.
{
  // Do nothing.
}

void Region::sendStateToExternalNeighbors()
{
  std::map<int, std::vector<RegionMessageStruct> >::iterator it; // Loop iterator.
  
  // Loop over all neighbor proxies of all elements.  If the neighbor's region is me it is an internal neighbor, otherwise it is an external neighbor.  For
  // internal neighbors, we always calculate a new nominal flow rate each timestep because we can be sure the neighbors are synced up, and it is inexpensive
  // since it doesn't require any messages.  The new nominal flow rate will be calculated after all messages have gone out to better overlap computation and
  // communication.  Therefore, at this point all we do for internal neighbors is set the nominal flow rate to expired.  For external neighbors, if the nominal
  // flow rate has expired put the element's state in the list of state messages to send to the neighbor's region.
  for (itMesh = meshElements.begin(); itMesh != meshElements.end(); ++itMesh)
    {
      for (itMeshSurfacewaterMeshNeighbor  = (*itMesh).second.meshNeighbors.begin();
           itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          if ((*itMeshSurfacewaterMeshNeighbor).region == thisIndex || isBoundary((*itMeshSurfacewaterMeshNeighbor).neighbor))
            {
              (*itMeshSurfacewaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshSurfacewaterMeshNeighbor).region]
                               .push_back(RegionMessageStruct(MESH_SURFACEWATER_MESH_NEIGHBOR, (*itMeshSurfacewaterMeshNeighbor).neighbor,
                                                              (*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy,
                                                              (*itMesh).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itMeshSurfacewaterChannelNeighbor  = (*itMesh).second.channelNeighbors.begin();
           itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
        {
          if ((*itMeshSurfacewaterChannelNeighbor).region == thisIndex)
            {
              (*itMeshSurfacewaterChannelNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshSurfacewaterChannelNeighbor).region]
                               .push_back(RegionMessageStruct(CHANNEL_SURFACEWATER_MESH_NEIGHBOR, (*itMeshSurfacewaterChannelNeighbor).neighbor,
                                                              (*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy,
                                                              (*itMesh).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itMeshGroundwaterMeshNeighbor  = (*itMesh).second.underground.meshNeighbors.begin();
           itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          if ((*itMeshGroundwaterMeshNeighbor).region == thisIndex || isBoundary((*itMeshGroundwaterMeshNeighbor).neighbor))
            {
              (*itMeshGroundwaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshGroundwaterMeshNeighbor).region]
                               .push_back(RegionMessageStruct(MESH_GROUNDWATER_MESH_NEIGHBOR, (*itMeshGroundwaterMeshNeighbor).neighbor,
                                                              (*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy,
                                                              (*itMesh).second.surfacewaterDepth, (*itMesh).second.underground.groundwaterHead,
                                                              SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
           itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
        {
          if ((*itMeshGroundwaterChannelNeighbor).region == thisIndex)
            {
              (*itMeshGroundwaterChannelNeighbor).expirationTime = currentTime;
            }
          else if ((*itMeshGroundwaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itMeshGroundwaterChannelNeighbor).region]
                               .push_back(RegionMessageStruct(CHANNEL_GROUNDWATER_MESH_NEIGHBOR, (*itMeshGroundwaterChannelNeighbor).neighbor,
                                                              (*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy,
                                                              (*itMesh).second.surfacewaterDepth, (*itMesh).second.underground.groundwaterHead,
                                                              SimpleNeighborProxy::MaterialTransfer()));
            }
        }
    }
  
  for (itChannel = channelElements.begin(); itChannel != channelElements.end(); ++itChannel)
    {
      for (itChannelSurfacewaterMeshNeighbor  = (*itChannel).second.meshNeighbors.begin();
           itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
        {
          if ((*itChannelSurfacewaterMeshNeighbor).region == thisIndex)
            {
              (*itChannelSurfacewaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itChannelSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelSurfacewaterMeshNeighbor).region]
                               .push_back(RegionMessageStruct(MESH_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannelSurfacewaterMeshNeighbor).neighbor,
                                                              (*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy,
                                                              (*itChannel).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itChannelSurfacewaterChannelNeighbor  = (*itChannel).second.channelNeighbors.begin();
           itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          if ((*itChannelSurfacewaterChannelNeighbor).region == thisIndex || isBoundary((*itChannelSurfacewaterChannelNeighbor).neighbor))
            {
              (*itChannelSurfacewaterChannelNeighbor).expirationTime = currentTime;
            }
          else if ((*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelSurfacewaterChannelNeighbor).region]
                               .push_back(RegionMessageStruct(CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannelSurfacewaterChannelNeighbor).neighbor,
                                                              (*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy,
                                                              (*itChannel).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
      
      for (itChannelGroundwaterMeshNeighbor  = (*itChannel).second.undergroundMeshNeighbors.begin();
           itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
        {
          if ((*itChannelGroundwaterMeshNeighbor).region == thisIndex)
            {
              (*itChannelGroundwaterMeshNeighbor).expirationTime = currentTime;
            }
          else if ((*itChannelGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
              outgoingMessages[(*itChannelGroundwaterMeshNeighbor).region]
                               .push_back(RegionMessageStruct(MESH_GROUNDWATER_CHANNEL_NEIGHBOR, (*itChannelGroundwaterMeshNeighbor).neighbor,
                                                              (*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy,
                                                              (*itChannel).second.surfacewaterDepth, 0.0, SimpleNeighborProxy::MaterialTransfer()));
            }
        }
    }
  
  // Send aggregated messages to other regions and clear outgoing message buffers.
  for (it = outgoingMessages.begin(); it != outgoingMessages.end(); ++it)
    {
      if (!(*it).second.empty())
        {
          thisProxy[(*it).first].sendStateMessages(currentTime, regionalDtLimit, (*it).second);
          (*it).second.clear();
        }
    }
  
  // Send myself a message to calculate nominal flow rates of internal neighbors.  This is implemented as a low priority message so that all other regions can
  // send outgoing messages before I calculate to better overlap computation and communication.
  thisProxy[thisIndex].calculateNominalFlowRatesForInternalNeighbors();
}

void Region::handleCalculateNominalFlowRatesForInternalNeighbors()
{
  bool error = false; // Error flag.
  
  // Loop over all neighbor proxies of all elements.  If the neighbor's region is me it is an internal neighbor, otherwise it is an external neighbor.  For
  // internal neighbors, we always calculate a new nominal flow rate each timestep because we can be sure the neighbors are synced up, and it is inexpensive
  // since it doesn't require any messages.  In these loops we calculate the nominal flow rates for internal neighbors.
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      for (itMeshSurfacewaterMeshNeighbor  = (*itMesh).second.meshNeighbors.begin(); !error &&
           itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end(); ++itMeshSurfacewaterMeshNeighbor)
        {
          if (isBoundary((*itMeshSurfacewaterMeshNeighbor).neighbor))
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The boundary condition must not be calculated yet.
              CkAssert((*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              error = (*itMesh).second.calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, regionalDtLimit,
                  itMeshSurfacewaterMeshNeighbor - (*itMesh).second.meshNeighbors.begin(), 0.0);
            }
          else if ((*itMeshSurfacewaterMeshNeighbor).region == thisIndex && (*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(meshElements[(*itMeshSurfacewaterMeshNeighbor).neighbor]
                                    .meshNeighbors[(*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, regionalDtLimit,
                  itMeshSurfacewaterMeshNeighbor - (*itMesh).second.meshNeighbors.begin(),
                  meshElements[(*itMeshSurfacewaterMeshNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  meshElements[(*itMeshSurfacewaterMeshNeighbor).neighbor]
                               .meshNeighbors[(*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy]
                                              .nominalFlowRate = -(*itMeshSurfacewaterMeshNeighbor).nominalFlowRate;
                  meshElements[(*itMeshSurfacewaterMeshNeighbor).neighbor]
                               .meshNeighbors[(*itMeshSurfacewaterMeshNeighbor).reciprocalNeighborProxy]
                                              .expirationTime = (*itMeshSurfacewaterMeshNeighbor).expirationTime;
                }
            }
        }
      
      for (itMeshSurfacewaterChannelNeighbor  = (*itMesh).second.channelNeighbors.begin(); !error &&
           itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end(); ++itMeshSurfacewaterChannelNeighbor)
        {
          if ((*itMeshSurfacewaterChannelNeighbor).region == thisIndex && (*itMeshSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(channelElements[(*itMeshSurfacewaterChannelNeighbor).neighbor]
                                       .meshNeighbors[(*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, regionalDtLimit,
                  itMeshSurfacewaterChannelNeighbor - (*itMesh).second.channelNeighbors.begin(),
                  channelElements[(*itMeshSurfacewaterChannelNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  channelElements[(*itMeshSurfacewaterChannelNeighbor).neighbor]
                                  .meshNeighbors[(*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy]
                                                 .nominalFlowRate = -(*itMeshSurfacewaterChannelNeighbor).nominalFlowRate;
                  channelElements[(*itMeshSurfacewaterChannelNeighbor).neighbor]
                                  .meshNeighbors[(*itMeshSurfacewaterChannelNeighbor).reciprocalNeighborProxy]
                                                 .expirationTime = (*itMeshSurfacewaterChannelNeighbor).expirationTime;
                }
            }
        }
      
      for (itMeshGroundwaterMeshNeighbor  = (*itMesh).second.underground.meshNeighbors.begin(); !error &&
           itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end(); ++itMeshGroundwaterMeshNeighbor)
        {
          if (isBoundary((*itMeshGroundwaterMeshNeighbor).neighbor))
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The boundary condition must not be calculated yet.
              CkAssert((*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.underground.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, regionalDtLimit,
                  itMeshGroundwaterMeshNeighbor - (*itMesh).second.underground.meshNeighbors.begin(), (*itMesh).second.elementX, (*itMesh).second.elementY,
                  (*itMesh).second.elementZSurface, (*itMesh).second.elementArea, (*itMesh).second.surfacewaterDepth, 0.0,
                  (*itMeshGroundwaterMeshNeighbor).neighborZSurface);
            }
          else if ((*itMeshGroundwaterMeshNeighbor).region == thisIndex && (*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor]
                                    .underground.meshNeighbors[(*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.underground.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, regionalDtLimit,
                  itMeshGroundwaterMeshNeighbor - (*itMesh).second.underground.meshNeighbors.begin(), (*itMesh).second.elementX, (*itMesh).second.elementY,
                  (*itMesh).second.elementZSurface, (*itMesh).second.elementArea, (*itMesh).second.surfacewaterDepth,
                  meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor].surfacewaterDepth,
                  meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor].underground.groundwaterHead);

              if (!error)
                {
                  meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor]
                               .underground.meshNeighbors[(*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy]
                                                          .nominalFlowRate = -(*itMeshGroundwaterMeshNeighbor).nominalFlowRate;
                  meshElements[(*itMeshGroundwaterMeshNeighbor).neighbor]
                               .underground.meshNeighbors[(*itMeshGroundwaterMeshNeighbor).reciprocalNeighborProxy]
                                                          .expirationTime = (*itMeshGroundwaterMeshNeighbor).expirationTime;
                }
            }
        }
      
      for (itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin(); !error &&
           itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end(); ++itMeshGroundwaterChannelNeighbor)
        {
          if ((*itMeshGroundwaterChannelNeighbor).region == thisIndex && (*itMeshGroundwaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(channelElements[(*itMeshGroundwaterChannelNeighbor).neighbor]
                                       .undergroundMeshNeighbors[(*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itMesh).second.underground.calculateNominalFlowRateWithGroundwaterChannelNeighbor(currentTime, regionalDtLimit,
                  itMeshGroundwaterChannelNeighbor - (*itMesh).second.underground.channelNeighbors.begin(), (*itMesh).second.elementZSurface,
                  (*itMesh).second.surfacewaterDepth, channelElements[(*itMeshGroundwaterChannelNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  channelElements[(*itMeshGroundwaterChannelNeighbor).neighbor]
                                  .undergroundMeshNeighbors[(*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy]
                                                            .nominalFlowRate = -(*itMeshGroundwaterChannelNeighbor).nominalFlowRate;
                  channelElements[(*itMeshGroundwaterChannelNeighbor).neighbor]
                                  .undergroundMeshNeighbors[(*itMeshGroundwaterChannelNeighbor).reciprocalNeighborProxy]
                                                            .expirationTime = (*itMeshGroundwaterChannelNeighbor).expirationTime;
                }
            }
        }
    }
  
  for (itChannel = channelElements.begin(); !error && itChannel != channelElements.end(); ++itChannel)
    {
      for (itChannelSurfacewaterMeshNeighbor  = (*itChannel).second.meshNeighbors.begin(); !error &&
           itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end(); ++itChannelSurfacewaterMeshNeighbor)
        {
          if ((*itChannelSurfacewaterMeshNeighbor).region == thisIndex && (*itChannelSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(meshElements[(*itChannelSurfacewaterMeshNeighbor).neighbor]
                                    .channelNeighbors[(*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itChannel).second.calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, regionalDtLimit,
                  itChannelSurfacewaterMeshNeighbor - (*itChannel).second.meshNeighbors.begin(),
                  meshElements[(*itChannelSurfacewaterMeshNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  meshElements[(*itChannelSurfacewaterMeshNeighbor).neighbor]
                               .channelNeighbors[(*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy]
                                                 .nominalFlowRate = -(*itChannelSurfacewaterMeshNeighbor).nominalFlowRate;
                  meshElements[(*itChannelSurfacewaterMeshNeighbor).neighbor]
                               .channelNeighbors[(*itChannelSurfacewaterMeshNeighbor).reciprocalNeighborProxy]
                                                 .expirationTime = (*itChannelSurfacewaterMeshNeighbor).expirationTime;
                }
            }
        }
      
      for (itChannelSurfacewaterChannelNeighbor  = (*itChannel).second.channelNeighbors.begin(); !error &&
           itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end(); ++itChannelSurfacewaterChannelNeighbor)
        {
          if (isBoundary((*itChannelSurfacewaterChannelNeighbor).neighbor))
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The boundary condition must not be calculated yet.
              CkAssert((*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itChannel).second.calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, regionalDtLimit,
                  itChannelSurfacewaterChannelNeighbor - (*itChannel).second.channelNeighbors.begin(), 0.0);
            }
          else if ((*itChannelSurfacewaterChannelNeighbor).region == thisIndex && (*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(channelElements[(*itChannelSurfacewaterChannelNeighbor).neighbor]
                                       .channelNeighbors[(*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itChannel).second.calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, regionalDtLimit,
                  itChannelSurfacewaterChannelNeighbor - (*itChannel).second.channelNeighbors.begin(),
                  channelElements[(*itChannelSurfacewaterChannelNeighbor).neighbor].surfacewaterDepth);

              if (!error)
                {
                  channelElements[(*itChannelSurfacewaterChannelNeighbor).neighbor]
                                  .channelNeighbors[(*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy]
                                                    .nominalFlowRate = -(*itChannelSurfacewaterChannelNeighbor).nominalFlowRate;
                  channelElements[(*itChannelSurfacewaterChannelNeighbor).neighbor]
                                  .channelNeighbors[(*itChannelSurfacewaterChannelNeighbor).reciprocalNeighborProxy]
                                                    .expirationTime = (*itChannelSurfacewaterChannelNeighbor).expirationTime;
                }
            }
        }
      
      for (itChannelGroundwaterMeshNeighbor  = (*itChannel).second.undergroundMeshNeighbors.begin(); !error &&
           itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end(); ++itChannelGroundwaterMeshNeighbor)
        {
          if ((*itChannelGroundwaterMeshNeighbor).region == thisIndex && (*itChannelGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // The internal neighbor must not be calculated yet either.
              CkAssert(meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor]
                                    .underground.channelNeighbors[(*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = (*itChannel).second.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, regionalDtLimit,
                  itChannelGroundwaterMeshNeighbor - (*itChannel).second.undergroundMeshNeighbors.begin(),
                  meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor].surfacewaterDepth,
                  meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor].underground.groundwaterHead);

              if (!error)
                {
                  meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor]
                               .underground.channelNeighbors[(*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy]
                                                             .nominalFlowRate = -(*itChannelGroundwaterMeshNeighbor).nominalFlowRate;
                  meshElements[(*itChannelGroundwaterMeshNeighbor).neighbor]
                               .underground.channelNeighbors[(*itChannelGroundwaterMeshNeighbor).reciprocalNeighborProxy]
                                                             .expirationTime = (*itChannelGroundwaterMeshNeighbor).expirationTime;
                }
            }
        }
    }
  
  if (!error)
    {
      // Set up iterators for the incremental scan that detects when all nominal flow rates are calculated because all state messages have arrived.
      itMesh    = meshElements.begin();
      itChannel = channelElements.begin();

      if (itMesh != meshElements.end())
        {
          itMeshSurfacewaterMeshNeighbor    = (*itMesh).second.meshNeighbors.begin();
          itMeshSurfacewaterChannelNeighbor = (*itMesh).second.channelNeighbors.begin();
          itMeshGroundwaterMeshNeighbor     = (*itMesh).second.underground.meshNeighbors.begin();
          itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
        }

      if (itChannel != channelElements.end())
        {
          itChannelSurfacewaterMeshNeighbor    = (*itChannel).second.meshNeighbors.begin();
          itChannelSurfacewaterChannelNeighbor = (*itChannel).second.channelNeighbors.begin();
          itChannelGroundwaterMeshNeighbor     = (*itChannel).second.undergroundMeshNeighbors.begin();
        }

      // The incremental scan also selects the next timestep end time as the minimum of nextSyncTime and all nominal flow rate expiration times.
      timestepEndTime = nextSyncTime;
    }
  
  if (error)
    {
      CkExit();
    }
}

bool Region::allNominalFlowRatesCalculated()
{
  bool allCalculated = true; // Stays true until we find one that is not calculated.
  
  while (allCalculated && itMesh != meshElements.end())
    {
      while (allCalculated && itMeshSurfacewaterMeshNeighbor != (*itMesh).second.meshNeighbors.end())
        {
          if ((*itMeshSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itMeshSurfacewaterMeshNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itMeshSurfacewaterMeshNeighbor).expirationTime)
                {
                  timestepEndTime = (*itMeshSurfacewaterMeshNeighbor).expirationTime;
                }

              ++itMeshSurfacewaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itMeshSurfacewaterChannelNeighbor != (*itMesh).second.channelNeighbors.end())
        {
          if ((*itMeshSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itMeshSurfacewaterChannelNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itMeshSurfacewaterChannelNeighbor).expirationTime)
                {
                  timestepEndTime = (*itMeshSurfacewaterChannelNeighbor).expirationTime;
                }
              
              ++itMeshSurfacewaterChannelNeighbor;
            }
        }
      
      while (allCalculated && itMeshGroundwaterMeshNeighbor != (*itMesh).second.underground.meshNeighbors.end())
        {
          if ((*itMeshGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itMeshGroundwaterMeshNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itMeshGroundwaterMeshNeighbor).expirationTime)
                {
                  timestepEndTime = (*itMeshGroundwaterMeshNeighbor).expirationTime;
                }
              
              ++itMeshGroundwaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itMeshGroundwaterChannelNeighbor != (*itMesh).second.underground.channelNeighbors.end())
        {
          if ((*itMeshGroundwaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itMeshGroundwaterChannelNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itMeshGroundwaterChannelNeighbor).expirationTime)
                {
                  timestepEndTime = (*itMeshGroundwaterChannelNeighbor).expirationTime;
                }
              
              ++itMeshGroundwaterChannelNeighbor;
            }
        }
      
      if (allCalculated)
        {
          ++itMesh;
          
          if (itMesh != meshElements.end())
            {
              itMeshSurfacewaterMeshNeighbor    = (*itMesh).second.meshNeighbors.begin();
              itMeshSurfacewaterChannelNeighbor = (*itMesh).second.channelNeighbors.begin();
              itMeshGroundwaterMeshNeighbor     = (*itMesh).second.underground.meshNeighbors.begin();
              itMeshGroundwaterChannelNeighbor  = (*itMesh).second.underground.channelNeighbors.begin();
            }
        }
    }
  
  while (allCalculated && itChannel != channelElements.end())
    {
      while (allCalculated && itChannelSurfacewaterMeshNeighbor != (*itChannel).second.meshNeighbors.end())
        {
          if ((*itChannelSurfacewaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itChannelSurfacewaterMeshNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itChannelSurfacewaterMeshNeighbor).expirationTime)
                {
                  timestepEndTime = (*itChannelSurfacewaterMeshNeighbor).expirationTime;
                }

              ++itChannelSurfacewaterMeshNeighbor;
            }
        }
      
      while (allCalculated && itChannelSurfacewaterChannelNeighbor != (*itChannel).second.channelNeighbors.end())
        {
          if ((*itChannelSurfacewaterChannelNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itChannelSurfacewaterChannelNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itChannelSurfacewaterChannelNeighbor).expirationTime)
                {
                  timestepEndTime = (*itChannelSurfacewaterChannelNeighbor).expirationTime;
                }
              
              ++itChannelSurfacewaterChannelNeighbor;
            }
        }
      
      while (allCalculated && itChannelGroundwaterMeshNeighbor != (*itChannel).second.undergroundMeshNeighbors.end())
        {
          if ((*itChannelGroundwaterMeshNeighbor).expirationTime == currentTime)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // Internal neighbors should all be calculated.
              CkAssert((*itChannelGroundwaterMeshNeighbor).region != thisIndex);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              
              allCalculated = false;
            }
          else
            {
              if (timestepEndTime > (*itChannelGroundwaterMeshNeighbor).expirationTime)
                {
                  timestepEndTime = (*itChannelGroundwaterMeshNeighbor).expirationTime;
                }
              
              ++itChannelGroundwaterMeshNeighbor;
            }
        }
      
      if (allCalculated)
        {
          ++itChannel;
          
          if (itChannel != channelElements.end())
            {
              itChannelSurfacewaterMeshNeighbor    = (*itChannel).second.meshNeighbors.begin();
              itChannelSurfacewaterChannelNeighbor = (*itChannel).second.channelNeighbors.begin();
              itChannelGroundwaterMeshNeighbor     = (*itChannel).second.undergroundMeshNeighbors.begin();
            }
        }
    }
  
  return allCalculated;
}

void Region::processStateMessages(double senderCurrentTime, double senderRegionalDtLimit, std::vector<RegionMessageStruct>& stateMessages)
{
  bool                                       error = false; // Error flag.
  std::vector<RegionMessageStruct>::iterator it;            // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(senderCurrentTime >= currentTime))
    {
      CkError("ERROR in Region::processStateMessages, region %d: senderCurrentTime must be greater than or equal to currentTime.\n", thisIndex);
      error = true;
    }
  
  if (!(0.0 < senderRegionalDtLimit))
    {
      CkError("ERROR in Region::processStateMessages, region %d: senderRegionalDtLimit must be greater than zero.\n", thisIndex);
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  // FIXME check RegionMessageStruct invariant.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  if (senderCurrentTime > currentTime)
    {
      // This is a message from the future, don't receive it yet.
      thisProxy[thisIndex].sendStateMessages(senderCurrentTime, senderRegionalDtLimit, stateMessages);
    }
  else
    {
      // Use the minimum of my or my neighbor's regionalDtLimit.
      if (senderRegionalDtLimit > regionalDtLimit)
        {
          senderRegionalDtLimit = regionalDtLimit;
        }
      
      // Calculate nominal flow rates.
      for (it = stateMessages.begin(); !error && it != stateMessages.end(); ++it)
        {
          switch ((*it).messageType)
          {
          case MESH_SURFACEWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].meshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case MESH_SURFACEWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case MESH_GROUNDWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].underground.meshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].underground.calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime,
                senderRegionalDtLimit, (*it).recipientNeighborProxyIndex, meshElements[(*it).recipientElementNumber].elementX,
                meshElements[(*it).recipientElementNumber].elementY, meshElements[(*it).recipientElementNumber].elementZSurface,
                meshElements[(*it).recipientElementNumber].elementArea, meshElements[(*it).recipientElementNumber].surfacewaterDepth,
                (*it).senderSurfacewaterDepth, (*it).senderGroundwaterHead);
            break;
          case MESH_GROUNDWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(meshElements[(*it).recipientElementNumber].underground.channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = meshElements[(*it).recipientElementNumber].underground.calculateNominalFlowRateWithGroundwaterChannelNeighbor(currentTime,
                senderRegionalDtLimit, (*it).recipientNeighborProxyIndex, meshElements[(*it).recipientElementNumber].elementZSurface,
                meshElements[(*it).recipientElementNumber].surfacewaterDepth, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_SURFACEWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].meshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterMeshNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].channelNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithSurfacewaterChannelNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth);
            break;
          case CHANNEL_GROUNDWATER_MESH_NEIGHBOR:
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            // The nominal flow rate must be expired.
            CkAssert(channelElements[(*it).recipientElementNumber].undergroundMeshNeighbors[(*it).recipientNeighborProxyIndex].expirationTime == currentTime);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

            error = channelElements[(*it).recipientElementNumber].calculateNominalFlowRateWithGroundwaterMeshNeighbor(currentTime, senderRegionalDtLimit,
                (*it).recipientNeighborProxyIndex, (*it).senderSurfacewaterDepth, (*it).senderGroundwaterHead);
            break;
          }
        }
    }
  
  if (error)
    {
      CkExit();
    }
}

void Region::doPointProcessesAndSendOutflows()
{
  bool                                                       error = false; // Error flag.
  std::map<int, std::vector<RegionMessageStruct> >::iterator it;            // Loop iterator.
  
  
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      error = (*itMesh).second.doPointProcessesAndSendOutflows(referenceDate, currentTime, timestepEndTime, *this);
    }
  
  for (itChannel = channelElements.begin(); !error && itChannel != channelElements.end(); ++itChannel)
    {
      error = (*itChannel).second.doPointProcessesAndSendOutflows(referenceDate, currentTime, timestepEndTime, *this);
    }
  
  if (!error)
    {
      // Send aggregated messages to other regions and clear outgoing message buffers.
      for (it = outgoingMessages.begin(); it != outgoingMessages.end(); ++it)
        {
          if (!(*it).second.empty())
            {
              thisProxy[(*it).first].sendWaterMessages((*it).second);
              (*it).second.clear();
            }
        }
      
      // Set up iterators for the incremental scan that detects when all inflow water messages have arrived.
      itMesh    = meshElements.begin();
      itChannel = channelElements.begin();
    }
  
  if (error)
    {
      CkExit();
    }
}

bool Region::allInflowsArrived()
{
  bool allArrived = true; // Stays true until we find one that has not arrived.
  
  while (allArrived && itMesh != meshElements.end())
    {
      if (!(*itMesh).second.allInflowsArrived(currentTime, timestepEndTime))
        {
          allArrived = false;
        }
      else
        {
          ++itMesh;
        }
    }
  
  while (allArrived && itChannel != channelElements.end())
    {
      if (!(*itChannel).second.allInflowsArrived(currentTime, timestepEndTime))
        {
          allArrived = false;
        }
      else
        {
          ++itChannel;
        }
    }
  
  return allArrived;
}

bool Region::receiveWater(RegionMessageStruct waterMessage)
{
  bool error = false; // Error flag.

  // FIXME error check inputs

  if (!error)
    {
      switch (waterMessage.messageType)
      {
      case MESH_SURFACEWATER_MESH_NEIGHBOR:
        error = meshElements[waterMessage.recipientElementNumber].meshNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                .insertMaterial(waterMessage.water);
        break;
      case MESH_SURFACEWATER_CHANNEL_NEIGHBOR:
        error = meshElements[waterMessage.recipientElementNumber].channelNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                   .insertMaterial(waterMessage.water);
        break;
      case MESH_GROUNDWATER_MESH_NEIGHBOR:
        error = meshElements[waterMessage.recipientElementNumber].underground.meshNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                            .insertMaterial(waterMessage.water);
        break;
      case MESH_GROUNDWATER_CHANNEL_NEIGHBOR:
        error = meshElements[waterMessage.recipientElementNumber].underground.channelNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                               .insertMaterial(waterMessage.water);
        break;
      case CHANNEL_SURFACEWATER_MESH_NEIGHBOR:
        error = channelElements[waterMessage.recipientElementNumber].meshNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                   .insertMaterial(waterMessage.water);
        break;
      case CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR:
        error = channelElements[waterMessage.recipientElementNumber].channelNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                      .insertMaterial(waterMessage.water);
        break;
      case CHANNEL_GROUNDWATER_MESH_NEIGHBOR:
        error = channelElements[waterMessage.recipientElementNumber].undergroundMeshNeighbors[waterMessage.recipientNeighborProxyIndex]
                                                                                              .insertMaterial(waterMessage.water);
        break;
      }
    }

  return error;
}

bool Region::sendWater(int recipientRegion, RegionMessageStruct waterMessage)
{
  bool error = false; // Error flag.

  // FIXME error check inputs

  if (!error)
    {
      if (recipientRegion == thisIndex)
        {
          error = receiveWater(waterMessage);
        }
      else
        {
          outgoingMessages[recipientRegion].push_back(waterMessage);
        }
    }

  return error;
}

void Region::processWaterMessages(std::vector<RegionMessageStruct>& waterMessages)
{
  bool                                       error = false; // Error flag.
  std::vector<RegionMessageStruct>::iterator it;            // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  // FIXME check RegionMessageStruct invariant.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  // Place water in recipient's incoming material list.
  for (it = waterMessages.begin(); !error && it != waterMessages.end(); ++it)
    {
      error = receiveWater(*it);
    }

  if (error)
    {
      CkExit();
    }
}

void Region::receiveInflowsAndAdvanceTime()
{
  bool error = false;
  
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      error = (*itMesh).second.receiveInflows(currentTime, timestepEndTime);
    }
  
  for (itChannel = channelElements.begin(); !error && itChannel != channelElements.end(); ++itChannel)
    {
      error = (*itChannel).second.receiveInflows(currentTime, timestepEndTime);
    }
  
  if (!error)
    {
      // Set regionalDtLimit to twice the previous timestep duration and advance time.
      regionalDtLimit = 2.0 * (timestepEndTime - currentTime);
      currentTime     = timestepEndTime;
    }
  
  if (error)
    {
      CkExit();
    }
}

bool Region::massBalance(double& waterInDomain, double& externalFlows, double& waterError)
{
  bool error = false; // Error flag.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= waterInDomain))
    {
      CkError("ERROR in Region::massBalance: waterInDomain must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  for (itMesh = meshElements.begin(); !error && itMesh != meshElements.end(); ++itMesh)
    {
      error = (*itMesh).second.massBalance(waterInDomain, externalFlows, waterError);
    }
  
  for (itChannel = channelElements.begin(); !error && itChannel != channelElements.end(); ++itChannel)
    {
      error = (*itChannel).second.massBalance(waterInDomain, externalFlows, waterError);
    }

  return error;
}

#include "region.def.h"
