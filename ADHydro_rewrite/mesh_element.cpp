#include "mesh_element.h"
#include "adhydro.h"
#include "file_manager.h"
#include "surfacewater.h"
#include "groundwater.h"
#include "garto.h"

MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy() :
  SimpleNeighborProxy(0.0, 0.0, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
  region(0),
  neighbor(0),
  reciprocalNeighborProxy(0),
  neighborX(0.0),
  neighborY(0.0),
  neighborZSurface(0.0),
  neighborArea(0.0),
  edgeLength(0.0),
  edgeNormalX(0.0),
  edgeNormalY(0.0),
  neighborManningsN(0.0),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
  // Initialization handled by initialization list.
}

MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, double flowCumulativeShortTermInit,
                                                                     double flowCumulativeLongTermInit, int regionInit, int neighborInit,
                                                                     int reciprocalNeighborProxyInit, double neighborXInit, double neighborYInit,
                                                                     double neighborZSurfaceInit, double neighborAreaInit, double edgeLengthInit,
                                                                     double edgeNormalXInit, double edgeNormalYInit, double neighborManningsNInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
  region(regionInit),
  neighbor(neighborInit),
  reciprocalNeighborProxy(reciprocalNeighborProxyInit),
  neighborX(neighborXInit),
  neighborY(neighborYInit),
  neighborZSurface(neighborZSurfaceInit),
  neighborArea(neighborAreaInit),
  edgeLength(edgeLengthInit),
  edgeNormalX(edgeNormalXInit),
  edgeNormalY(edgeNormalYInit),
  neighborManningsN(neighborManningsNInit),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= regionInit && regionInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: regionInit must be greater than or equal to zero and less than "
              "globalNumberOfRegions.\n");
      CkExit();
    }
  
  if (!(isBoundary(neighborInit) || (0 <= neighborInit && neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: neighborInit must be a boundary condition code or greater than "
              "or equal to zero and less than globalNumberOfMeshElements.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxyInit))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: reciprocalNeighborProxyInit must be greater than or equal to "
              "zero.\n");
      CkExit();
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(0.0 == neighborAreaInit))
        {
          CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: for boundary condition codes neighborAreaInit must be "
                  "zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborAreaInit))
        {
          CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: for non-boundary neighbors neighborAreaInit must be greater "
                  "than zero.\n");
          CkExit();
        }
    }
  
  if (!(0.0 < edgeLengthInit))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: edgeLengthInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!epsilonEqual(1.0, edgeNormalXInit * edgeNormalXInit + edgeNormalYInit * edgeNormalYInit))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: edgeNormalXInit and edgeNormalYInit must be a unit vector.\n");
      CkExit();
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(0.0 == neighborManningsNInit))
        {
          CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: for boundary condition codes neighborManningsNInit must be "
                  "zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborManningsNInit))
        {
          CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::MeshSurfacewaterMeshNeighborProxy: for non-boundary neighbors neighborManningsNInit must be "
                  "greater than zero.\n");
          CkExit();
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void MeshSurfacewaterMeshNeighborProxy::pup(PUP::er &p)
{
  SimpleNeighborProxy::pup(p);
  
  p | region;
  p | neighbor;
  p | reciprocalNeighborProxy;
  p | neighborX;
  p | neighborY;
  p | neighborZSurface;
  p | neighborArea;
  p | edgeLength;
  p | edgeNormalX;
  p | edgeNormalY;
  p | neighborManningsN;
  p | neighborInitialized;
  p | neighborInvariantChecked;
}

bool MeshSurfacewaterMeshNeighborProxy::checkInvariant()
{
  bool error = SimpleNeighborProxy::checkInvariant(); // Error flag.
  
  if (!(0 <= region && region < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: region must be greater than or equal to zero and less than "
              "globalNumberOfRegions.\n");
      error = true;
    }
  
  if (!(isBoundary(neighbor) || (0 <= neighbor && neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: neighbor must be a boundary condition code or greater than or equal to zero and "
              "less than globalNumberOfMeshElements.\n");
      error = true;
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (isBoundary(neighbor))
    {
      if (!(0.0 == neighborArea))
        {
          CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: for boundary condition codes neighborArea must be zero.\n");
          error = true;
        }
    }
  else
    {
      if (!(0.0 < neighborArea))
        {
          CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: for non-boundary neighbors neighborArea must be greater than zero.\n");
          error = true;
        }
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!epsilonEqual(1.0, edgeNormalX * edgeNormalX + edgeNormalY * edgeNormalY))
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: edgeNormalX and edgeNormalY must be a unit vector.\n");
      error = true;
    }
  
  if (isBoundary(neighbor))
    {
      if (!(0.0 == neighborManningsN))
        {
          CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: for boundary condition codes neighborManningsN must be zero.\n");
          error = true;
        }
    }
  else
    {
      if (!(0.0 < neighborManningsN))
        {
          CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: for non-boundary neighbors neighborManningsN must be greater than zero.\n");
          error = true;
        }
    }
  
  if (!neighborInitialized)
    {
      CkError("ERROR in MeshSurfacewaterMeshNeighborProxy::checkInvariant: neighborInitialized must be true before checking invariant.\n");
      error = true;
    }
  
  return error;
}

MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy() :
  SimpleNeighborProxy(0.0, 0.0, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
  region(0),
  neighbor(0),
  reciprocalNeighborProxy(0),
  neighborZBank(0.0),
  neighborZBed(0.0),
  neighborZOffset(0.0),
  edgeLength(0.0),
  neighborBaseWidth(0.0),
  neighborSideSlope(0.0),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
  // Initialization handled by initialization list.
}

MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                                                           double flowCumulativeShortTermInit, double flowCumulativeLongTermInit,
                                                                           int regionInit, int neighborInit, int reciprocalNeighborProxyInit,
                                                                           double neighborZBankInit, double neighborZBedInit, double neighborZOffsetInit,
                                                                           double edgeLengthInit, double neighborBaseWidthInit, double neighborSideSlopeInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
  region(regionInit),
  neighbor(neighborInit),
  reciprocalNeighborProxy(reciprocalNeighborProxyInit),
  neighborZBank(neighborZBankInit),
  neighborZBed(neighborZBedInit),
  neighborZOffset(neighborZOffsetInit),
  edgeLength(edgeLengthInit),
  neighborBaseWidth(neighborBaseWidthInit),
  neighborSideSlope(neighborSideSlopeInit),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= regionInit && regionInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy: regionInit must be greater than or equal to zero and less "
              "than globalNumberOfRegions.\n");
      CkExit();
    }
  
  if (!(0 <= neighborInit && neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy: neighborInit must be greater than or equal to zero and "
              "less than globalNumberOfChannelElements.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxyInit))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy: reciprocalNeighborProxyInit must be greater than or equal "
              "to zero.\n");
      CkExit();
    }
  
  if (!(neighborZBankInit >= neighborZBedInit))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy: neighborZBankInit must be greater than or equal to "
              "neighborZBedInit.\n");
      CkExit();
    }
  
  if (!(0.0 < edgeLengthInit))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy: edgeLengthInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborBaseWidthInit))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy: neighborBaseWidthInit must be greater than or equal to "
              "zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborSideSlopeInit))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy: neighborSideSlopeInit must be greater than or equal to "
              "zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBaseWidthInit || 0.0 < neighborSideSlopeInit))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::MeshSurfacewaterChannelNeighborProxy: one of neighborBaseWidthInit or neighborSideSlopeInit "
              "must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void MeshSurfacewaterChannelNeighborProxy::pup(PUP::er &p)
{
  SimpleNeighborProxy::pup(p);
  
  p | region;
  p | neighbor;
  p | reciprocalNeighborProxy;
  p | neighborZBank;
  p | neighborZBed;
  p | neighborZOffset;
  p | edgeLength;
  p | neighborBaseWidth;
  p | neighborSideSlope;
  p | neighborInitialized;
  p | neighborInvariantChecked;
}

bool MeshSurfacewaterChannelNeighborProxy::checkInvariant()
{
  bool error = SimpleNeighborProxy::checkInvariant(); // Error flag.
  
  if (!(0 <= region && region < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: region must be greater than or equal to zero and less than "
              "globalNumberOfRegions.\n");
      error = true;
    }
  
  if (!(0 <= neighbor && neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: neighbor must be greater than or equal to zero and less than "
              "globalNumberOfChannelElements.\n");
      error = true;
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(neighborZBank >= neighborZBed))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: neighborZBank must be greater than or equal to neighborZBed.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborBaseWidth))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: neighborBaseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSideSlope))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: neighborSideSlope must be greater than or equal to  zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborBaseWidth || 0.0 < neighborSideSlope))
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: one of neighborBaseWidth or neighborSideSlope must be greater than zero.\n");
      error = true;
    }
  
  if (!neighborInitialized)
    {
      CkError("ERROR in MeshSurfacewaterChannelNeighborProxy::checkInvariant: neighborInitialized must be true before checking invariant.\n");
      error = true;
    }
  
  return error;
}

MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy() :
  SimpleNeighborProxy(0.0, 0.0, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
  region(0),
  neighbor(0),
  reciprocalNeighborProxy(0),
  neighborX(0.0),
  neighborY(0.0),
  neighborZSurface(0.0),
  neighborLayerZBottom(0.0),
  neighborArea(0.0),
  edgeLength(0.0),
  edgeNormalX(0.0),
  edgeNormalY(0.0),
  neighborConductivity(0.0),
  neighborPorosity(0.0),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
  // Initialization handled by initialization list.
}

MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, double flowCumulativeShortTermInit,
                                                                   double flowCumulativeLongTermInit, int regionInit, int neighborInit,
                                                                   int reciprocalNeighborProxyInit, double neighborXInit, double neighborYInit,
                                                                   double neighborZSurfaceInit, double neighborLayerZBottomInit, double neighborAreaInit,
                                                                   double edgeLengthInit, double edgeNormalXInit, double edgeNormalYInit,
                                                                   double neighborConductivityInit, double neighborPorosityInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
  region(regionInit),
  neighbor(neighborInit),
  reciprocalNeighborProxy(reciprocalNeighborProxyInit),
  neighborX(neighborXInit),
  neighborY(neighborYInit),
  neighborZSurface(neighborZSurfaceInit),
  neighborLayerZBottom(neighborLayerZBottomInit),
  neighborArea(neighborAreaInit),
  edgeLength(edgeLengthInit),
  edgeNormalX(edgeNormalXInit),
  edgeNormalY(edgeNormalYInit),
  neighborConductivity(neighborConductivityInit),
  neighborPorosity(neighborPorosityInit),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= regionInit && regionInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: regionInit must be greater than or equal to zero and less than "
              "globalNumberOfRegions.\n");
      CkExit();
    }
  
  if (!(isBoundary(neighborInit) || (0 <= neighborInit && neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: neighborInit must be a boundary condition code or greater than "
              "or equal to zero and less than globalNumberOfMeshElements.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxyInit))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: reciprocalNeighborProxyInit must be greater than or equal to "
              "zero.\n");
      CkExit();
    }
  
  if (!(neighborZSurfaceInit >= neighborLayerZBottomInit))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: neighborZSurfaceInit must be greater than or equal to "
              "neighborLayerZBottomInit.\n");
      CkExit();
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(0.0 == neighborAreaInit))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: for boundary condition codes neighborAreaInit must be "
                  "zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborAreaInit))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: for non-boundary neighbors neighborAreaInit must be greater "
                  "than zero.\n");
          CkExit();
        }
    }
  
  if (!(0.0 < edgeLengthInit))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: edgeLengthInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!epsilonEqual(1.0, edgeNormalXInit * edgeNormalXInit + edgeNormalYInit * edgeNormalYInit))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: edgeNormalXInit and edgeNormalYInit must be a unit vector.\n");
      CkExit();
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(0.0 == neighborConductivityInit))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: for boundary condition codes neighborConductivityInit must be "
                  "zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborConductivityInit))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: for non-boundary neighbors neighborConductivityInit must be "
                  "greater than zero.\n");
          CkExit();
        }
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(0.0 == neighborPorosityInit))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: for boundary condition codes neighborPorosityInit must be "
                  "zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborPorosityInit))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::MeshGroundwaterMeshNeighborProxy: for non-boundary neighbors neighborPorosityInit must be "
                  "greater than zero.\n");
          CkExit();
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void MeshGroundwaterMeshNeighborProxy::pup(PUP::er &p)
{
  SimpleNeighborProxy::pup(p);
  
  p | region;
  p | neighbor;
  p | reciprocalNeighborProxy;
  p | neighborX;
  p | neighborY;
  p | neighborZSurface;
  p | neighborLayerZBottom;
  p | neighborArea;
  p | edgeLength;
  p | edgeNormalX;
  p | edgeNormalY;
  p | neighborConductivity;
  p | neighborPorosity;
  p | neighborInitialized;
  p | neighborInvariantChecked;
}

bool MeshGroundwaterMeshNeighborProxy::checkInvariant()
{
  bool error = SimpleNeighborProxy::checkInvariant(); // Error flag.
  
  if (!(0 <= region && region < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: region must be greater than or equal to zero and less than "
              "globalNumberOfRegions.\n");
      error = true;
    }
  
  if (!(isBoundary(neighbor) || (0 <= neighbor && neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements)))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: neighbor must be a boundary condition code or greater than or equal to zero and "
              "less than globalNumberOfMeshElements.\n");
      error = true;
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(neighborZSurface >= neighborLayerZBottom))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: neighborZSurface must be greater than or equal to neighborLayerZBottom.\n");
      error = true;
    }
  
  if (isBoundary(neighbor))
    {
      if (!(0.0 == neighborArea))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: for boundary condition codes neighborArea must be zero.\n");
          error = true;
        }
    }
  else
    {
      if (!(0.0 < neighborArea))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: for non-boundary neighbors neighborArea must be greater than zero.\n");
          error = true;
        }
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!epsilonEqual(1.0, edgeNormalX * edgeNormalX + edgeNormalY * edgeNormalY))
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: edgeNormalX and edgeNormalY must be a unit vector.\n");
      error = true;
    }
  
  if (isBoundary(neighbor))
    {
      if (!(0.0 == neighborConductivity))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: for boundary condition codes neighborConductivity must be zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborConductivity))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: for non-boundary neighbors neighborConductivity must be greater than zero.\n");
          CkExit();
        }
    }
  
  if (isBoundary(neighbor))
    {
      if (!(0.0 == neighborPorosity))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: for boundary condition codes neighborPorosity must be zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborPorosity))
        {
          CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: for non-boundary neighbors neighborPorosity must be greater than zero.\n");
          CkExit();
        }
    }
  
  if (!neighborInitialized)
    {
      CkError("ERROR in MeshGroundwaterMeshNeighborProxy::checkInvariant: neighborInitialized must be true before checking invariant.\n");
      error = true;
    }
  
  return error;
}

MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy() :
  SimpleNeighborProxy(0.0, 0.0, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
  region(0),
  neighbor(0),
  reciprocalNeighborProxy(0),
  neighborZBank(0.0),
  neighborZBed(0.0),
  neighborZOffset(0.0),
  edgeLength(0.0),
  neighborBaseWidth(0.0),
  neighborSideSlope(0.0),
  neighborBedConductivity(0.0),
  neighborBedThickness(0.0),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
  // Initialization handled by initialization list.
}

MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                                                         double flowCumulativeShortTermInit, double flowCumulativeLongTermInit, int regionInit,
                                                                         int neighborInit, int reciprocalNeighborProxyInit, double neighborZBankInit,
                                                                         double neighborZBedInit, double neighborZOffsetInit, double edgeLengthInit,
                                                                         double neighborBaseWidthInit, double neighborSideSlopeInit,
                                                                         double neighborBedConductivityInit, double neighborBedThicknessInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
  region(regionInit),
  neighbor(neighborInit),
  reciprocalNeighborProxy(reciprocalNeighborProxyInit),
  neighborZBank(neighborZBankInit),
  neighborZBed(neighborZBedInit),
  neighborZOffset(neighborZOffsetInit),
  edgeLength(edgeLengthInit),
  neighborBaseWidth(neighborBaseWidthInit),
  neighborSideSlope(neighborSideSlopeInit),
  neighborBedConductivity(neighborBedConductivityInit),
  neighborBedThickness(neighborBedThicknessInit),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= regionInit && regionInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: regionInit must be greater than or equal to zero and less "
              "than globalNumberOfRegions.\n");
      CkExit();
    }
  
  if (!(0 <= neighborInit && neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: neighborInit must be greater than or equal to zero and less "
              "than globalNumberOfChannelElements.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxyInit))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: reciprocalNeighborProxyInit must be greater than or equal "
              "to zero.\n");
      CkExit();
    }
  
  if (!(neighborZBankInit >= neighborZBedInit))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: neighborZBankInit must be greater than or equal to "
              "neighborZBedInit.\n");
      CkExit();
    }
  
  if (!(0.0 < edgeLengthInit))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: edgeLengthInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborBaseWidthInit))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: neighborBaseWidthInit must be greater than or equal to "
              "zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= neighborSideSlopeInit))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: neighborSideSlopeInit must be greater than or equal to "
              "zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBaseWidthInit || 0.0 < neighborSideSlopeInit))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: one of neighborBaseWidthInit or neighborSideSlopeInit "
              "must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBedConductivityInit))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: neighborBedConductivityInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborBedThicknessInit))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::MeshGroundwaterChannelNeighborProxy: neighborBedThicknessInit must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void MeshGroundwaterChannelNeighborProxy::pup(PUP::er &p)
{
  SimpleNeighborProxy::pup(p);
  
  p | region;
  p | neighbor;
  p | reciprocalNeighborProxy;
  p | neighborZBank;
  p | neighborZBed;
  p | neighborZOffset;
  p | edgeLength;
  p | neighborBaseWidth;
  p | neighborSideSlope;
  p | neighborBedConductivity;
  p | neighborBedThickness;
  p | neighborInitialized;
  p | neighborInvariantChecked;
}

bool MeshGroundwaterChannelNeighborProxy::checkInvariant()
{
  bool error = SimpleNeighborProxy::checkInvariant(); // Error flag.
  
  if (!(0 <= region && region < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: region must be greater than or equal to zero and less than "
              "globalNumberOfRegions.\n");
      error = true;
    }
  
  if (!(0 <= neighbor && neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: neighbor must be greater than or equal to zero and less than "
              "globalNumberOfChannelElements.\n");
      error = true;
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(neighborZBank >= neighborZBed))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: neighborZBank must be greater than or equal to neighborZBed.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborBaseWidth))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: neighborBaseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSideSlope))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: neighborSideSlope must be greater than or equal to  zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborBaseWidth || 0.0 < neighborSideSlope))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: one of neighborBaseWidth or neighborSideSlope must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborBedConductivity))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: neighborBedConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborBedThickness))
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: neighborBedThickness must be greater than zero.\n");
      error = true;
    }
  
  if (!neighborInitialized)
    {
      CkError("ERROR in MeshGroundwaterChannelNeighborProxy::checkInvariant: neighborInitialized must be true before checking invariant.\n");
      error = true;
    }
  
  return error;
}

InfiltrationAndGroundwater::VadoseZone::VadoseZone() :
  infiltrationMethod(NO_INFILTRATION), // Dummy values will be overwritten by pup_stl.h code.
  state(NULL)
{
  // Initialization handled by initialization list.
}

InfiltrationAndGroundwater::VadoseZone::VadoseZone(InfiltrationMethodEnum infiltrationMethodInit, void* vadoseZoneStateInit) :
  infiltrationMethod(infiltrationMethodInit),
  state(vadoseZoneStateInit)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (NO_INFILTRATION == infiltrationMethodInit || TRIVIAL_INFILTRATION == infiltrationMethodInit)
    {
      if (!(NULL == vadoseZoneStateInit))
        {
          CkError("ERROR in InfiltrationAndGroundwater::VadoseZone::VadoseZone: if infiltrationMethodInit is NO_INFILTRATION or TRIVIAL_INFILTRATION then "
                  "vadoseZoneStateInit must be NULL.\n");
          CkExit();
        }
    }
  else if (GARTO_INFILTRATION == infiltrationMethodInit)
    {
      if (!(NULL != vadoseZoneStateInit))
        {
          CkError("ERROR in InfiltrationAndGroundwater::VadoseZone::VadoseZone: if infiltrationMethodInit is GARTO_INFILTRATION then vadoseZoneStateInit "
                  "must not be NULL.\n");
          CkExit();
        }
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
      else
        {
          garto_check_invariant((garto_domain*)vadoseZoneStateInit);
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
    }
  else
    {
      CkError("ERROR in InfiltrationAndGroundwater::VadoseZone::VadoseZone: infiltrationMethodInit must be a valid enum value.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

InfiltrationAndGroundwater::VadoseZone::VadoseZone(const VadoseZone &other) :
      infiltrationMethod(NO_INFILTRATION), // Dummy values will be overwritten by assignment operator.
      state(NULL)
{
  *this = other;
}

InfiltrationAndGroundwater::VadoseZone::~VadoseZone()
{
  // Clean up dynamically allocated data.
  if (GARTO_INFILTRATION == infiltrationMethod)
    {
      garto_parameters_dealloc(&(((garto_domain*)state)->parameters));
      garto_domain_dealloc((garto_domain**)&state);
    }
}

InfiltrationAndGroundwater::VadoseZone& InfiltrationAndGroundwater::VadoseZone::operator=(const VadoseZone &other)
{
  int ii; // Loop counter.
  
  // Clean up dynamically allocated data.
  if (GARTO_INFILTRATION == infiltrationMethod)
    {
      garto_parameters_dealloc(&((garto_domain*)state)->parameters);
      garto_domain_dealloc((garto_domain**)&state);
    }
  
  infiltrationMethod = other.infiltrationMethod;
  
  // Make a deep copy of state.
  if (GARTO_INFILTRATION == infiltrationMethod)
    {
      state = new garto_domain;
      
      ((garto_domain*)state)->parameters = new garto_parameters;
      
      ((garto_domain*)state)->parameters->num_bins                    = ((garto_domain*)other.state)->parameters->num_bins;
      ((garto_domain*)state)->parameters->theta_r                     = ((garto_domain*)other.state)->parameters->theta_r;
      ((garto_domain*)state)->parameters->theta_s                     = ((garto_domain*)other.state)->parameters->theta_s;
      ((garto_domain*)state)->parameters->vg_alpha                    = ((garto_domain*)other.state)->parameters->vg_alpha;
      ((garto_domain*)state)->parameters->vg_n                        = ((garto_domain*)other.state)->parameters->vg_n;
      ((garto_domain*)state)->parameters->bc_lambda                   = ((garto_domain*)other.state)->parameters->bc_lambda;
      ((garto_domain*)state)->parameters->bc_psib                     = ((garto_domain*)other.state)->parameters->bc_psib;
      ((garto_domain*)state)->parameters->saturated_conductivity      = ((garto_domain*)other.state)->parameters->saturated_conductivity;
      ((garto_domain*)state)->parameters->effective_capillary_suction = ((garto_domain*)other.state)->parameters->effective_capillary_suction;
      
      ((garto_domain*)state)->top_depth               = ((garto_domain*)other.state)->top_depth;
      ((garto_domain*)state)->bottom_depth            = ((garto_domain*)other.state)->bottom_depth;
      ((garto_domain*)state)->d_theta                 = new double[((garto_domain*)state)->parameters->num_bins + 1];
      ((garto_domain*)state)->surface_front_theta     = new double[((garto_domain*)state)->parameters->num_bins + 1];
      ((garto_domain*)state)->surface_front_depth     = new double[((garto_domain*)state)->parameters->num_bins + 1];
      ((garto_domain*)state)->groundwater_front_theta = new double[((garto_domain*)state)->parameters->num_bins + 1];
      ((garto_domain*)state)->groundwater_front_depth = new double[((garto_domain*)state)->parameters->num_bins + 1];
      ((garto_domain*)state)->initial_water_content   = ((garto_domain*)other.state)->initial_water_content;
      ((garto_domain*)state)->yes_groundwater         = ((garto_domain*)other.state)->yes_groundwater;
      
      for (ii = 0; ii <= ((garto_domain*)state)->parameters->num_bins; ii++)
        {
          ((garto_domain*)state)->d_theta[ii]                 = ((garto_domain*)other.state)->d_theta[ii];
          ((garto_domain*)state)->surface_front_theta[ii]     = ((garto_domain*)other.state)->surface_front_theta[ii];
          ((garto_domain*)state)->surface_front_depth[ii]     = ((garto_domain*)other.state)->surface_front_depth[ii];
          ((garto_domain*)state)->groundwater_front_theta[ii] = ((garto_domain*)other.state)->groundwater_front_theta[ii];
          ((garto_domain*)state)->groundwater_front_depth[ii] = ((garto_domain*)other.state)->groundwater_front_depth[ii];
        }
    }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  else
    {
      // State should have started as NULL or already have been cleaned up.
      CkAssert (NULL == state);
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
  
  return *this;
}

void InfiltrationAndGroundwater::VadoseZone::pup(PUP::er &p)
{
  p | infiltrationMethod;

  if (GARTO_INFILTRATION == infiltrationMethod)
    {
      if (p.isUnpacking())
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          // We should be pupping into a just constructed empty object so there should be no previous dynamically allocated data to clean up.
          CkAssert(NULL == state);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          state = new garto_domain;
        }

      p | *(garto_domain*)state;
    }
  else
    {
      if (p.isUnpacking())
        {
          state = NULL;
        }
    }
}

bool InfiltrationAndGroundwater::VadoseZone::checkInvariant()
{
  bool error = false; // Error flag.
  
  if (NO_INFILTRATION == infiltrationMethod || TRIVIAL_INFILTRATION == infiltrationMethod)
    {
      if (!(NULL == state))
        {
          CkError("ERROR in InfiltrationAndGroundwater::VadoseZone::checkInvariant: if infiltrationMethod is NO_INFILTRATION or TRIVIAL_INFILTRATION then "
                  "state must be NULL.\n");
          error = true;
        }
    }
  else if (GARTO_INFILTRATION == infiltrationMethod)
    {
      if (!(NULL != state))
        {
          CkError("ERROR in InfiltrationAndGroundwater::VadoseZone::checkInvariant: if infiltrationMethod is GARTO_INFILTRATION then state must not be "
                  "NULL.\n");
          error = true;
        }
      else
        {
          // FIXLATER garto_check_invariant is not written to the correct pattern.
          garto_check_invariant((garto_domain*)state);
        }
    }
  else
    {
      CkError("ERROR in InfiltrationAndGroundwater::VadoseZone::checkInvariant: infiltrationMethod must be a valid enum value.\n");
      error = true;
    }
  
  return error;
}

InfiltrationAndGroundwater::InfiltrationAndGroundwater() :
  groundwaterMethod(NO_AQUIFER), // Dummy values will be overwritten by pup_stl.h code.
  soilType(0),
  layerZBottom(0.0),
  slopeX(0.0),
  slopeY(0.0),
  conductivity(0.0),
  porosity(0.0),
  groundwaterHead(0.0),
  groundwaterRecharge(0.0),
  groundwaterError(0.0),
  vadoseZone(),
  meshNeighbors(),
  channelNeighbors()
{
  // Initialization handled by initialization list.
}

InfiltrationAndGroundwater::InfiltrationAndGroundwater(GroundwaterMethodEnum  groundwaterMethodInit, int soilTypeInit, double layerZBottomInit,
                                                       double slopeXInit, double slopeYInit, double conductivityInit, double porosityInit,
                                                       double groundwaterHeadInit, double groundwaterRechargeInit, double groundwaterErrorInit,
                                                       VadoseZone& vadoseZoneInit) :
  groundwaterMethod(groundwaterMethodInit),
  soilType(soilTypeInit),
  layerZBottom(layerZBottomInit),
  slopeX(slopeXInit),
  slopeY(slopeYInit),
  conductivity(conductivityInit),
  porosity(porosityInit),
  groundwaterHead(groundwaterHeadInit),
  groundwaterRecharge(groundwaterRechargeInit),
  groundwaterError(groundwaterErrorInit),
  vadoseZone(vadoseZoneInit),
  meshNeighbors(),
  channelNeighbors()
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(NO_AQUIFER == groundwaterMethodInit || DEEP_AQUIFER == groundwaterMethodInit || SHALLOW_AQUIFER== groundwaterMethodInit))
    {
      CkError("ERROR in InfiltrationAndGroundwater::InfiltrationAndGroundwater: groundwaterMethodInit must be a valid enum value.\n");
      CkExit();
    }
  
  if (NO_INFILTRATION == vadoseZoneInit.infiltrationMethod)
    {
      if (!(NO_AQUIFER == groundwaterMethodInit))
        {
          CkError("ERROR in InfiltrationAndGroundwater::InfiltrationAndGroundwater: if vadoseZoneInit.infiltrationMethod is NO_INFILTRATION "
                  "groundwaterMethodInit must be NO_AQUIFER.\n");
          CkExit();
        }
    }
  else
    {
      if (!(NO_AQUIFER != groundwaterMethodInit))
        {
          CkError("ERROR in InfiltrationAndGroundwater::InfiltrationAndGroundwater: if vadoseZoneInit.infiltrationMethod is not NO_INFILTRATION "
                  "groundwaterMethodInit must not be NO_AQUIFER.\n");
          CkExit();
        }
    }
  
  if (DEEP_AQUIFER == groundwaterMethodInit)
    {
      if (!(groundwaterHeadInit < layerZBottomInit))
        {
          if (2 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in InfiltrationAndGroundwater::InfiltrationAndGroundwater: groundwaterMethodInit is DEEP_AQUIFER, but groundwaterHeadInit is "
                      "not below layerZBottomInit.\n");
            }
        }
    }

  if (!(1 <= soilTypeInit && 19 >= soilTypeInit))
    {
      CkError("ERROR in InfiltrationAndGroundwater::InfiltrationAndGroundwater: soilTypeInit must be greater than or equal to 1 and less than or equal to "
              "19.\n");
      CkExit();
    }
  else if (14 == soilTypeInit)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in InfiltrationAndGroundwater::InfiltrationAndGroundwater: soilTypeInit is 'WATER'.  Should be a waterbody instead.\n");
        }
    }
  else if (16 == soilTypeInit)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in InfiltrationAndGroundwater::InfiltrationAndGroundwater: soilTypeInit is 'OTHER(land-ice)'.  Should be an icemass instead.\n");
        }
    }
  
  if (!(0.0 < conductivityInit))
    {
      CkError("ERROR in InfiltrationAndGroundwater::InfiltrationAndGroundwater: conductivityInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < porosityInit))
    {
      CkError("ERROR in InfiltrationAndGroundwater::InfiltrationAndGroundwater: porosityInit must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (vadoseZoneInit.checkInvariant())
    {
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
}

void InfiltrationAndGroundwater::pup(PUP::er &p)
{
  p | groundwaterMethod;
  p | soilType;
  p | layerZBottom;
  p | slopeX;
  p | slopeY;
  p | conductivity;
  p | porosity;
  p | groundwaterHead;
  p | groundwaterRecharge;
  p | groundwaterError;
  p | vadoseZone;
  p | meshNeighbors;
  p | channelNeighbors;
}

bool InfiltrationAndGroundwater::checkInvariant()
{
  bool                                                       error = false; // Error flag.
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;        // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel;     // Loop iterator.
  
  if (!(NO_AQUIFER == groundwaterMethod || DEEP_AQUIFER == groundwaterMethod || SHALLOW_AQUIFER== groundwaterMethod))
    {
      CkError("ERROR in InfiltrationAndGroundwater::checkInvariant: groundwaterMethod must be a valid enum value.\n");
      error = true;
    }
  
  if (NO_INFILTRATION == vadoseZone.infiltrationMethod)
    {
      if (!(NO_AQUIFER == groundwaterMethod))
        {
          CkError("ERROR in InfiltrationAndGroundwater::checkInvariant: if vadoseZone.infiltrationMethod is NO_INFILTRATION groundwaterMethod must be "
                  "NO_AQUIFER.\n");
          error = true;
        }
    }
  else
    {
      if (!(NO_AQUIFER != groundwaterMethod))
        {
          CkError("ERROR in InfiltrationAndGroundwater::checkInvariant: if vadoseZone.infiltrationMethod is not NO_INFILTRATION groundwaterMethod must not be "
                  "NO_AQUIFER.\n");
          error = true;
        }
    }
  
  if (NO_AQUIFER == groundwaterMethod || DEEP_AQUIFER == groundwaterMethod)
    {
      if (!meshNeighbors.empty())
        {
          CkError("ERROR in InfiltrationAndGroundwater::checkInvariant: if groundwaterMethod is NO_AQUIFER or DEEP_AQUIFER meshNeighbors must be empty.\n");
          error = true;
        }
      
      if (!channelNeighbors.empty())
        {
          CkError("ERROR in InfiltrationAndGroundwater::checkInvariant: if groundwaterMethod is NO_AQUIFER or DEEP_AQUIFER channelNeighbors must be empty.\n");
          error = true;
        }
    }
  
  if (DEEP_AQUIFER == groundwaterMethod)
    {
      if (!(groundwaterHead < layerZBottom))
        {
          if (2 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in InfiltrationAndGroundwater::checkInvariant: groundwaterMethod is DEEP_AQUIFER, but groundwaterHead is not below layerZBottom.\n");
            }
        }
    }

  if (!(1 <= soilType && 19 >= soilType))
    {
      CkError("ERROR in InfiltrationAndGroundwater::checkInvariant: soilType must be greater than or equal to 1 and less than or equal to 19.\n");
      error = true;
    }
  else if (14 == soilType)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in InfiltrationAndGroundwater::checkInvariant: soilType is 'WATER'.  Should be a waterbody instead.\n");
        }
    }
  else if (16 == soilType)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in InfiltrationAndGroundwater::checkInvariant: soilType is 'OTHER(land-ice)'.  Should be an icemass instead.\n");
        }
    }
  
  if (!(0.0 < conductivity))
    {
      CkError("ERROR in InfiltrationAndGroundwater::checkInvariant: conductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < porosity))
    {
      CkError("ERROR in InfiltrationAndGroundwater::checkInvariant: porosity must be greater than zero.\n");
      error = true;
    }
  
  error = vadoseZone.checkInvariant() || error;
  
  for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
    {
      error = (*itMesh).checkInvariant() || error;
    }
  
  for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
    {
      error = (*itChannel).checkInvariant() || error;
    }
  
  return error;
}

bool InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                                                     std::vector<MeshGroundwaterMeshNeighborProxy>::size_type
                                                                                     neighborProxyIndex, double elementX, double elementY,
                                                                                     double elementZSurface, double elementArea,
                                                                                     double elementSurfacewaterDepth, double neighborSurfacewaterDepth,
                                                                                     double neighborGroundwaterHead)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < meshNeighbors.size()))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborProxyIndex must be greater than or equal to "
              "zero and less than meshNeighbors.size().\n");
      error = true;
    }
  else if (!(meshNeighbors[neighborProxyIndex].neighborZSurface >= neighborGroundwaterHead))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborGroundwaterHead must be less than or equal "
              "to meshNeighbors[neighborProxyIndex].neighborZSurface.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: elementSurfacewaterDepth must be greater than or "
              "equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborSurfacewaterDepth must be greater than or "
              "equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      if (isBoundary(meshNeighbors[neighborProxyIndex].neighbor))
        {
          // FIXLATER what to do about inflowHeight?
          error = groundwaterMeshBoundaryFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                                  (BoundaryConditionEnum)meshNeighbors[neighborProxyIndex].neighbor, 0.0,
                                                  meshNeighbors[neighborProxyIndex].edgeLength, meshNeighbors[neighborProxyIndex].edgeNormalX,
                                                  meshNeighbors[neighborProxyIndex].edgeNormalY, layerZBottom, elementArea, slopeX, slopeY, conductivity,
                                                  porosity, groundwaterHead);
        }
      else
        {
          error = groundwaterMeshMeshFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                              meshNeighbors[neighborProxyIndex].edgeLength, elementX, elementY, elementZSurface, layerZBottom, elementArea,
                                              conductivity, porosity, elementSurfacewaterDepth, groundwaterHead, meshNeighbors[neighborProxyIndex].neighborX,
                                              meshNeighbors[neighborProxyIndex].neighborY, meshNeighbors[neighborProxyIndex].neighborZSurface,
                                              meshNeighbors[neighborProxyIndex].neighborLayerZBottom, meshNeighbors[neighborProxyIndex].neighborArea,
                                              meshNeighbors[neighborProxyIndex].neighborConductivity, meshNeighbors[neighborProxyIndex].neighborPorosity,
                                              neighborSurfacewaterDepth, neighborGroundwaterHead);
        }
    }
  
  // Calculate expiration time.
  if (!error)
    {
      meshNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor(double currentTime, double regionalDtLimit,
                                                                                        std::vector<MeshGroundwaterChannelNeighborProxy>::size_type
                                                                                        neighborProxyIndex, double elementZSurface,
                                                                                        double elementSurfacewaterDepth, double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < channelNeighbors.size()))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor: neighborProxyIndex must be greater than or equal "
              "to zero and less than channelNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= elementSurfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor: elementSurfacewaterDepth must be greater than or "
              "equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::calculateNominalFlowRateWithGroundwaterChannelNeighbor: neighborSurfacewaterDepth must be greater than or "
              "equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      // FIXME calculate dtNew
      error = groundwaterMeshChannelFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, channelNeighbors[neighborProxyIndex].edgeLength,
                                             elementZSurface + channelNeighbors[neighborProxyIndex].neighborZOffset,
                                             layerZBottom + channelNeighbors[neighborProxyIndex].neighborZOffset, elementSurfacewaterDepth,
                                             groundwaterHead + channelNeighbors[neighborProxyIndex].neighborZOffset,
                                             channelNeighbors[neighborProxyIndex].neighborZBank, channelNeighbors[neighborProxyIndex].neighborZBed,
                                             channelNeighbors[neighborProxyIndex].neighborBaseWidth, channelNeighbors[neighborProxyIndex].neighborSideSlope,
                                             channelNeighbors[neighborProxyIndex].neighborBedConductivity,
                                             channelNeighbors[neighborProxyIndex].neighborBedThickness, neighborSurfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      channelNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool InfiltrationAndGroundwater::fillInEvapoTranspirationSoilMoistureStruct(double elementZSurface, float zSnso[EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS],
                                                                            EvapoTranspirationSoilMoistureStruct& evapoTranspirationSoilMoisture)
{
  bool   error = false;                                           // Error flag.
  int    ii;                                                      // Loop counter.
  double layerMiddleDepth;                                        // Meters.
  double distanceAboveWaterTable;                                 // Meters.
  double relativeSaturation;                                      // Unitless.
  double waterContent[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS]; // Water content for GARTO_INFILTRATION.
  double soilDepthZ[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS];   // Soil depth in GARTO_INFILTRATION in meters, positive downward.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  for (ii = EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS; ii < EVAPO_TRANSPIRATION_NUMBER_OF_ALL_LAYERS; ii++)
    {
      if (!(zSnso[ii - 1] > zSnso[ii]))
        {
          CkError("ERROR in InfiltrationAndGroundwater::fillInEvapoTranspirationSoilMoistureStruct, soil layer %d: zSnso must be less than the layer above "
                  "it.\n", ii - EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS);
          error = true;
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      if (NO_INFILTRATION == vadoseZone.infiltrationMethod)
        {
          evapoTranspirationSoilMoisture.zwt = 0.0f;

          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
            {
              evapoTranspirationSoilMoisture.smcEq[ii] = porosity * 0.01;
              evapoTranspirationSoilMoisture.sh2o[ii]  = porosity * 0.01;
              evapoTranspirationSoilMoisture.smc[ii]   = porosity * 0.01;
            }
        }
      else if (TRIVIAL_INFILTRATION == vadoseZone.infiltrationMethod)
        {
          evapoTranspirationSoilMoisture.zwt = elementZSurface - groundwaterHead;

          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
            {
              layerMiddleDepth        = 0.5 * (zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] +
                                               zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1]);
              distanceAboveWaterTable = evapoTranspirationSoilMoisture.zwt + layerMiddleDepth; // Plus because layerMiddleDepth is negative.

              // FIXLATER use Van Genutchen or Brooks-Corey?
              if (0.1 >= distanceAboveWaterTable)
                {
                  relativeSaturation = 1.0;
                }
              else
                {
                  relativeSaturation = 1.0 - (log10(distanceAboveWaterTable) + 1.0) * 0.3;

                  if (0.01 > relativeSaturation)
                    {
                      relativeSaturation = 0.01;
                    }
                }

              evapoTranspirationSoilMoisture.smcEq[ii] = porosity * relativeSaturation;
              evapoTranspirationSoilMoisture.sh2o[ii]  = porosity * relativeSaturation;
              evapoTranspirationSoilMoisture.smc[ii]   = porosity * relativeSaturation;
            }
        }
      else if (GARTO_INFILTRATION == vadoseZone.infiltrationMethod)
        {
          evapoTranspirationSoilMoisture.zwt = elementZSurface - groundwaterHead;
          
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
            {
              soilDepthZ[ii] = -zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS]; // soilDepthZ is positive downward.
            }
          
          garto_domain_water_content((garto_domain*)vadoseZone.state, waterContent, soilDepthZ, EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS);
          
          for (ii = 0; ii < EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS; ii++)
            {
              layerMiddleDepth                         = 0.5 * (zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS] +
                                                                zSnso[ii + EVAPO_TRANSPIRATION_NUMBER_OF_SNOW_LAYERS - 1]);
              distanceAboveWaterTable                  = evapoTranspirationSoilMoisture.zwt + layerMiddleDepth; // Plus because layerMiddleDepth is negative.
              evapoTranspirationSoilMoisture.smcEq[ii] = garto_equilibrium_water_content((garto_domain*)vadoseZone.state, distanceAboveWaterTable);
              evapoTranspirationSoilMoisture.sh2o[ii]  = waterContent[ii];
              evapoTranspirationSoilMoisture.smc[ii]   = waterContent[ii];
            }
        }

      // For all methods make the lowest layer water content extend down forever.
      evapoTranspirationSoilMoisture.smcwtd = evapoTranspirationSoilMoisture.smc[EVAPO_TRANSPIRATION_NUMBER_OF_SOIL_LAYERS - 1];
    }
  
  return error;
}

double InfiltrationAndGroundwater::evaporate(double unsatisfiedEvaporation, double elementZSurface)
{
  double evaporation = 0.0; // Return value.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= unsatisfiedEvaporation))
    {
      CkError("ERROR in InfiltrationAndGroundwater::evaporate: unsatisfiedEvaporation must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // if (NO_INFILTRATION == vadoseZone.infiltrationMethod) return zero.
  if (TRIVIAL_INFILTRATION == vadoseZone.infiltrationMethod)
    {
      // if (DEEP_AQUIFER == groundwaterMethod) return zero.
      if (SHALLOW_AQUIFER == groundwaterMethod)
        {
          if (groundwaterHead - unsatisfiedEvaporation / porosity >= layerZBottom)
            {
              evaporation      = unsatisfiedEvaporation;
              groundwaterHead -= unsatisfiedEvaporation / porosity;
            }
          else
            {
              evaporation     = (groundwaterHead - layerZBottom) * porosity;
              groundwaterHead = layerZBottom;
            }
        }
    }
  else if (GARTO_INFILTRATION == vadoseZone.infiltrationMethod)
    {
      // FIXLATER get real root depth
      evaporation = garto_evapotranspiration((garto_domain*)vadoseZone.state, unsatisfiedEvaporation, 0.0, elementZSurface - layerZBottom);
    }
    
  return evaporation;
}

double InfiltrationAndGroundwater::transpire(double unsatisfiedTranspiration, double elementZSurface)
{
  double transpiration = 0.0; // Return value.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= unsatisfiedTranspiration))
    {
      CkError("ERROR in InfiltrationAndGroundwater::transpire: unsatisfiedTranspiration must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  // if (NO_INFILTRATION == vadoseZone.infiltrationMethod) return zero.
  if (TRIVIAL_INFILTRATION == vadoseZone.infiltrationMethod)
    {
      // With no vadose zone state transpiration water comes from the same place as evaporation water.
      transpiration = evaporate(unsatisfiedTranspiration, elementZSurface);
    }
  else if (GARTO_INFILTRATION == vadoseZone.infiltrationMethod)
    {
      // FIXLATER get real root depth
      transpiration = garto_evapotranspiration((garto_domain*)vadoseZone.state, 0.0, unsatisfiedTranspiration, elementZSurface - layerZBottom);
    }
  return transpiration;
}

bool InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows(double currentTime, double timestepEndTime, double elementZSurface,
                                                                          double elementArea, double& surfacewaterDepth, Region& region)
{
  bool   error                   = false;                               // Error flag.
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  double dt                      = timestepEndTime - currentTime;       // Seconds
  double groundwaterAvailable    = 0.0;                                 // Groundwater available to satisfy outflows in cubic meters of water.
  double totalOutwardFlowRate    = 0.0;                                 // Sum of all outward flow rates in cubic meters per second.
  double outwardFlowRateFraction = 1.0;                                 // Fraction of all outward flow rates that can be satisfied, unitless.
  double waterSent;                                                     // Cubic meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in InfiltrationAndGroundwater::doInfiltrationAndSendGroundwaterOutflows: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // Do infiltration.
      // if (NO_INFILTRATION == infiltrationMethod) do nothing.
      if (TRIVIAL_INFILTRATION == vadoseZone.infiltrationMethod)
        {
          if (surfacewaterDepth >= conductivity * dt)
            {
              groundwaterRecharge += conductivity * dt;
              surfacewaterDepth   -= conductivity * dt;
            }
          else
            {
              groundwaterRecharge += surfacewaterDepth;
              surfacewaterDepth    = 0.0;
            }
        }
      else if (GARTO_INFILTRATION == vadoseZone.infiltrationMethod)
        {
          garto_timestep((garto_domain*)vadoseZone.state, dt, &surfacewaterDepth, elementZSurface - groundwaterHead, &groundwaterRecharge);
        }

      // Do groundwater outflows and resolve groundwaterRecharge.
      // if (NO_AQUIFER == groundwaterMethod) do nothing.
      if (DEEP_AQUIFER == groundwaterMethod)
        {
          // FIXME send groundwaterRecharge to an aquifer storage bucket.  For now, water just accumulates in groundwaterRecharge.
        }
      else if (SHALLOW_AQUIFER == groundwaterMethod)
        {
          // Limit groundwater outflows.
          if (groundwaterHead > layerZBottom)
            {
              groundwaterAvailable = (groundwaterRecharge + (groundwaterHead - layerZBottom) * porosity) * elementArea;
            }

          for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
            {
              if (0.0 < (*itMesh).nominalFlowRate)
                {
                  totalOutwardFlowRate += (*itMesh).nominalFlowRate;
                }
            }

          for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
            {
              if (0.0 < (*itChannel).nominalFlowRate)
                {
                  totalOutwardFlowRate += (*itChannel).nominalFlowRate;
                }
            }

          if (groundwaterAvailable < totalOutwardFlowRate * dt)
            {
              outwardFlowRateFraction = groundwaterAvailable / (totalOutwardFlowRate * dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
            }

          // Send groundwater outflows taking water from groundwaterRecharge.
          for (itMesh = meshNeighbors.begin(); !error && itMesh != meshNeighbors.end(); ++itMesh)
            {
              if (isBoundary((*itMesh).neighbor) && 0.0 > (*itMesh).nominalFlowRate)
                {
                  // Water for an inflow boundary arrives immediately.
                  error = (*itMesh).insertMaterial(SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, -(*itMesh).nominalFlowRate * dt));
                }
              else if (0.0 < (*itMesh).nominalFlowRate)
                {
                  // Send water for an outflow.
                  waterSent                          = (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction;
                  groundwaterRecharge               -= waterSent / elementArea;
                  (*itMesh).flowCumulativeShortTerm += waterSent;

                  if (!isBoundary((*itMesh).neighbor))
                    {
                      error = region.sendWater((*itMesh).region, RegionMessage(MESH_GROUNDWATER_MESH_NEIGHBOR, (*itMesh).neighbor,
                                                                               (*itMesh).reciprocalNeighborProxy, 0.0, 0.0,
                                                                               SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                                     waterSent)));
                    }
                }
            }

          for (itChannel = channelNeighbors.begin(); !error && itChannel != channelNeighbors.end(); ++itChannel)
            {
              if (0.0 < (*itChannel).nominalFlowRate)
                {
                  // Send water for an outflow.
                  waterSent                             = (*itChannel).nominalFlowRate * dt * outwardFlowRateFraction;
                  groundwaterRecharge                  -= waterSent / elementArea;
                  (*itChannel).flowCumulativeShortTerm += waterSent;

                  error = region.sendWater((*itChannel).region, RegionMessage(CHANNEL_GROUNDWATER_MESH_NEIGHBOR, (*itChannel).neighbor,
                                                                              (*itChannel).reciprocalNeighborProxy, 0.0, 0.0,
                                                                              SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                                    waterSent)));
                }
            }

          // Update groundwaterHead based on net groundwaterRecharge and take or put groundwaterRecharge water back into the domain, or if the domain is full put it
          // back in surfacewaterDepth.
          if (!error)
            {
              // NO_INFILTRATION cannot be used with SHALLOW_AQUIFER.
              if (TRIVIAL_INFILTRATION == vadoseZone.infiltrationMethod)
                {
                  groundwaterHead    += groundwaterRecharge / porosity;
                  groundwaterRecharge = 0.0;

                  if (groundwaterHead < layerZBottom)
                    {
                      // Even though we are limiting outflows, groundwaterHead can go below layerZBottom due to roundoff error.
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
                      CkAssert(epsilonEqual(groundwaterHead, layerZBottom));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

                      groundwaterError += (layerZBottom - groundwaterHead) * porosity;
                      groundwaterHead   = layerZBottom;
                    }
                  else if (groundwaterHead > elementZSurface)
                    {
                      // Exfiltration.
                      surfacewaterDepth += (groundwaterHead - elementZSurface) * porosity;
                      groundwaterHead    = elementZSurface;
                    }
                }
              else if (GARTO_INFILTRATION == vadoseZone.infiltrationMethod)
                {
                  groundwaterHead += groundwaterRecharge / garto_specific_yield((garto_domain*)vadoseZone.state, elementZSurface - groundwaterHead);

                  // Cap groundwaterHead at the surface.  Do not limit groundwaterHead from going below layerZBottom.
                  if (groundwaterHead > elementZSurface)
                    {
                      groundwaterHead = elementZSurface;
                    }

                  if (epsilonLess(0.0, groundwaterRecharge))
                    {
                      // If there is excess water put it immediately into the groundwater front of the GARTO domain.
                      garto_add_groundwater((garto_domain*)vadoseZone.state, &groundwaterRecharge);

                      if (epsilonLess(0.0, groundwaterRecharge))
                        {
                          // Not all of the water could fit in to the GARTO domain because the domain is full.  Put the excess water on the surface and the
                          // groundwater head moves up to the surface.  The real groundwater head is at the top of the surfacewater, but we set groundwaterHead
                          // to be at the surface and add surfacewaterDepth as needed because if we set groundwaterHead to be at the top of the surfacewater we
                          // would need to update it any time the value of surfacewaterDepth changed.
                          surfacewaterDepth   += groundwaterRecharge;
                          groundwaterRecharge  = 0.0;
                          groundwaterHead      = elementZSurface;
                        }
                    }
                  else if (epsilonGreater(0.0, groundwaterRecharge))
                    {
                      // If there is a water deficit take it immediately from the groundwater front of the GARTO domain.
                      garto_take_groundwater((garto_domain*)vadoseZone.state, elementZSurface - groundwaterHead, &groundwaterRecharge);

                      // If there is still a deficit leave it to be resolved next time.  The water table will drop further allowing us to get more water out.
                    }
                }
            }
        }
    }
  
  return error;
}

bool InfiltrationAndGroundwater::allInflowsArrived(double currentTime, double timestepEndTime)
{
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  bool                                                       allArrived = true; // Whether all material has arrived from all neighbors.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in InfiltrationAndGroundwater::allInflowsArrived: currentTime must be less than or equal to timestepEndTime.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call allMaterialHasArrived.  allInflowsArrived shouldn't be called in this situation, but if it is return
  // that all inflows have arrived for the next zero seconds.
  if (currentTime < timestepEndTime)
    {
      for (itMesh = meshNeighbors.begin(); allArrived && itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              allArrived = (*itMesh).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
      
      for (itChannel = channelNeighbors.begin(); allArrived && itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              allArrived = (*itChannel).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
    }
  
  return allArrived;
}

bool InfiltrationAndGroundwater::receiveInflows(double currentTime, double timestepEndTime, double elementArea)
{
  bool error = false;                                                   // Error flag.
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshGroundwaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in InfiltrationAndGroundwater::receiveInflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in InfiltrationAndGroundwater::receiveInflows: elementArea must be greater than zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call getMaterial.  advanceTime shouldn't be called in this situation, but if it is we don't have to do
  // anything to receive inflows for the next zero seconds.
  if (!error && currentTime < timestepEndTime)
    {
      // Receive all inflows.
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              groundwaterRecharge += (*itMesh).getMaterial(currentTime, timestepEndTime) / elementArea;
            }
        }
      
      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              groundwaterRecharge += (*itChannel).getMaterial(currentTime, timestepEndTime) / elementArea;
            }
        }
    }
  
  return error;
}

bool InfiltrationAndGroundwater::massBalance(double& waterInDomain, double& externalFlows, double& waterError, double elementArea)
{
  bool                                                    error = false; // Error flag.
  std::vector<MeshGroundwaterMeshNeighborProxy>::iterator itMesh;        // Loop iterator.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= waterInDomain))
    {
      CkError("ERROR in InfiltrationAndGroundwater::massBalance: waterInDomain must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      // For NO_INFILTRATION there is no groundwater in the domain.
      if (TRIVIAL_INFILTRATION == vadoseZone.infiltrationMethod)
        {
          if (SHALLOW_AQUIFER == groundwaterMethod && groundwaterHead > layerZBottom)
            {
              waterInDomain += (groundwaterHead - layerZBottom) * porosity * elementArea;
            }
          
          waterInDomain += groundwaterRecharge * elementArea;
        }
      else if (GARTO_INFILTRATION == vadoseZone.infiltrationMethod)
        {
          waterInDomain += garto_total_water_in_domain((garto_domain*)vadoseZone.state) * elementArea;
        }

      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (isBoundary((*itMesh).neighbor))
            {
              externalFlows += (*itMesh).flowCumulativeShortTerm + (*itMesh).flowCumulativeLongTerm;
            }
        }

      waterError += groundwaterError * elementArea;
    }

  return error;
}

MeshElement::MeshElement() :
  elementNumber(0), // Dummy values will be overwritten by pup_stl.h code.
  catchment(0),
  vegetationType(0),
  elementX(0.0),
  elementY(0.0),
  elementZSurface(0.0),
  elementArea(0.0),
  latitude(0.0),
  longitude(0.0),
  manningsN(0.0),
  surfacewaterDepth(0.0),
  surfacewaterError(0.0),
  precipitationRate(0.0),
  precipitationCumulativeShortTerm(0.0),
  precipitationCumulativeLongTerm(0.0),
  evaporationRate(0.0),
  evaporationCumulativeShortTerm(0.0),
  evaporationCumulativeLongTerm(0.0),
  transpirationRate(0.0),
  transpirationCumulativeShortTerm(0.0),
  transpirationCumulativeLongTerm(0.0),
  evapoTranspirationForcing(),
  evapoTranspirationState(),
  underground(),
  meshNeighbors(),
  channelNeighbors()
{
  vertexX[0] = 0.0;
  vertexX[1] = 0.0;
  vertexX[2] = 0.0;
  vertexY[0] = 0.0;
  vertexY[1] = 0.0;
  vertexY[2] = 0.0;
}

MeshElement::MeshElement(int elementNumberInit, int catchmentInit, int vegetationTypeInit, int soilTypeInit, double vertexXInit[3], double vertexYInit[3],
                         double elementXInit, double elementYInit, double elementZSurfaceInit, double layerZBottomInit, double elementAreaInit,
                         double slopeXInit, double slopeYInit, double latitudeInit, double longitudeInit, double manningsNInit, double conductivityInit,
                         double porosityInit, double surfacewaterDepthInit, double surfacewaterErrorInit, double groundwaterHeadInit,
                         double groundwaterRechargeInit, double groundwaterErrorInit, double precipitationRateInit,
                         double precipitationCumulativeShortTermInit, double precipitationCumulativeLongTermInit, double evaporationRateInit,
                         double evaporationCumulativeShortTermInit, double evaporationCumulativeLongTermInit, double transpirationRateInit,
                         double transpirationCumulativeShortTermInit, double transpirationCumulativeLongTermInit,
                         EvapoTranspirationForcingStruct& evapoTranspirationForcingInit, EvapoTranspirationStateStruct& evapoTranspirationStateInit,
                         InfiltrationAndGroundwater::GroundwaterMethodEnum  groundwaterMethodInit, InfiltrationAndGroundwater::VadoseZone vadoseZoneInit) :
  elementNumber(elementNumberInit),
  catchment(catchmentInit),
  vegetationType(vegetationTypeInit),
  elementX(elementXInit),
  elementY(elementYInit),
  elementZSurface(elementZSurfaceInit),
  elementArea(elementAreaInit),
  latitude(latitudeInit),
  longitude(longitudeInit),
  manningsN(manningsNInit),
  surfacewaterDepth(surfacewaterDepthInit),
  surfacewaterError(surfacewaterErrorInit),
  precipitationRate(precipitationRateInit),
  precipitationCumulativeShortTerm(precipitationCumulativeShortTermInit),
  precipitationCumulativeLongTerm(precipitationCumulativeLongTermInit),
  evaporationRate(evaporationRateInit),
  evaporationCumulativeShortTerm(evaporationCumulativeShortTermInit),
  evaporationCumulativeLongTerm(evaporationCumulativeLongTermInit),
  transpirationRate(transpirationRateInit),
  transpirationCumulativeShortTerm(transpirationCumulativeShortTermInit),
  transpirationCumulativeLongTerm(transpirationCumulativeLongTermInit),
  evapoTranspirationForcing(evapoTranspirationForcingInit),
  evapoTranspirationState(evapoTranspirationStateInit),
  underground(groundwaterMethodInit, soilTypeInit, layerZBottomInit, slopeXInit, slopeYInit, conductivityInit, porosityInit, groundwaterHeadInit,
              groundwaterRechargeInit, groundwaterErrorInit, vadoseZoneInit),
  meshNeighbors(),
  channelNeighbors()
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= elementNumberInit && elementNumberInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements))
    {
      CkError("ERROR in MeshElement::MeshElement: elementNumberInit must be greater than or equal to zero and less than globalNumberOfMeshElements.\n");
      CkExit();
    }

  if (!(1 <= vegetationTypeInit && 27 >= vegetationTypeInit))
    {
      CkError("ERROR in MeshElement::MeshElement: vegetationTypeInit must be greater than or equal to 1 and less than or equal to 27.\n");
      CkExit();
    }
  else if (16 == vegetationTypeInit)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in MeshElement::MeshElement: vegetationTypeInit is 'Water Bodies'.  Should be a waterbody instead.\n");
        }
    }
  else if (24 == vegetationTypeInit)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in MeshElement::MeshElement: vegetationTypeInit is 'Snow or Ice'.  Should be an icemass instead.\n");
        }
    }
  
  if (!(elementZSurfaceInit >= layerZBottomInit))
    {
      CkError("ERROR in MeshElement::MeshElement: elementZSurfaceInit must be greater than or equal to layerZBottomInit.\n");
      CkExit();
    }
  
  if (!(0.0 < elementAreaInit))
    {
      CkError("ERROR in MeshElement::MeshElement: elementAreaInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(-M_PI / 2.0 <= latitudeInit && M_PI / 2.0 >= latitudeInit))
    {
      CkError("ERROR in MeshElement::MeshElement: latitudeInit must be greater than or equal to negative PI over two and less than or equal to PI over "
              "two.\n");
      CkExit();
    }
  
  if (!(-M_PI * 2.0 <= longitudeInit && M_PI * 2.0 >= longitudeInit))
    {
      CkError("ERROR in MeshElement::MeshElement: longitudeInit must be greater than or equal to negative two PI and less than or equal to two PI.\n");
      CkExit();
    }
  
  if (!(0.0 < manningsNInit))
    {
      CkError("ERROR in MeshElement::MeshElement: manningsNInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= surfacewaterDepthInit))
    {
      CkError("ERROR in MeshElement::MeshElement: surfacewaterDepthInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 >= precipitationRateInit))
    {
      CkError("ERROR in MeshElement::MeshElement: precipitationRateInit must be less than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 >= precipitationCumulativeShortTermInit))
    {
      CkError("ERROR in MeshElement::MeshElement: precipitationCumulativeShortTermInit must be less than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 >= precipitationCumulativeLongTermInit))
    {
      CkError("ERROR in MeshElement::MeshElement: precipitationCumulativeLongTermInit must be less than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= transpirationRateInit))
    {
      CkError("ERROR in MeshElement::MeshElement: transpirationRateInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= transpirationCumulativeShortTermInit))
    {
      CkError("ERROR in MeshElement::MeshElement: transpirationCumulativeShortTermInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= transpirationCumulativeLongTermInit))
    {
      CkError("ERROR in MeshElement::MeshElement: transpirationCumulativeLongTermInit must be greater than or equal to zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  if (checkEvapoTranspirationForcingStructInvariant(&evapoTranspirationForcingInit))
    {
      CkExit();
    }
  
  if (checkEvapoTranspirationStateStructInvariant(&evapoTranspirationStateInit))
    {
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_INVARIANTS)
  
  vertexX[0] = vertexXInit[0];
  vertexX[1] = vertexXInit[1];
  vertexX[2] = vertexXInit[2];
  vertexY[0] = vertexYInit[0];
  vertexY[1] = vertexYInit[1];
  vertexY[2] = vertexYInit[2];
}

void MeshElement::pup(PUP::er &p)
{
  p | elementNumber;
  p | catchment;
  p | vegetationType;
  PUParray(p, vertexX, 3);
  PUParray(p, vertexY, 3);
  p | elementX;
  p | elementY;
  p | elementZSurface;
  p | elementArea;
  p | latitude;
  p | longitude;
  p | manningsN;
  p | surfacewaterDepth;
  p | surfacewaterError;
  p | precipitationRate;
  p | precipitationCumulativeShortTerm;
  p | precipitationCumulativeLongTerm;
  p | evaporationRate;
  p | evaporationCumulativeShortTerm;
  p | evaporationCumulativeLongTerm;
  p | transpirationRate;
  p | transpirationCumulativeShortTerm;
  p | transpirationCumulativeLongTerm;
  p | evapoTranspirationForcing;
  p | evapoTranspirationState;
  p | underground;
  p | meshNeighbors;
  p | channelNeighbors;
}

bool MeshElement::checkInvariant()
{
  bool                                                        error = false; // Error flag.
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator    itMesh;        // Loop iterator.
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator itChannel;     // Loop iterator.
  
  if (!(0 <= elementNumber && elementNumber < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements))
    {
      CkError("ERROR in MeshElement::checkInvariant: elementNumber must be greater than or equal to zero and less than globalNumberOfMeshElements.\n");
      error = true;
    }

  if (!(1 <= vegetationType && 27 >= vegetationType))
    {
      CkError("ERROR in MeshElement::checkInvariant: vegetationType must be greater than or equal to 1 and less than or equal to 27.\n");
      error = true;
    }
  else if (16 == vegetationType)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in MeshElement::checkInvariant: vegetationType is 'Water Bodies'.  Should be a waterbody instead.\n");
        }
    }
  else if (24 == vegetationType)
    {
      if (2 <= ADHydro::verbosityLevel)
        {
          CkError("WARNING in MeshElement::checkInvariant: vegetationType is 'Snow or Ice'.  Should be an icemass instead.\n");
        }
    }
  
  if (!(elementZSurface >= underground.layerZBottom))
    {
      CkError("ERROR in MeshElement::checkInvariant: elementZSurface must be greater than or equal to underground.layerZBottom.\n");
      error = true;
    }
  
  if (!(elementZSurface >= underground.groundwaterHead))
    {
      CkError("ERROR in MeshElement::checkInvariant: elementZSurface must be greater than or equal to underground.groundwaterHead.\n");
      error = true;
    }
  
  if (!(0.0 < elementArea))
    {
      CkError("ERROR in MeshElement::checkInvariant: elementArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(-M_PI / 2.0 <= latitude && M_PI / 2.0 >= latitude))
    {
      CkError("ERROR in MeshElement::checkInvariant: latitude must be greater than or equal to negative PI over two and less than or equal to PI over two.\n");
      error = true;
    }
  
  if (!(-M_PI * 2.0 <= longitude && M_PI * 2.0 >= longitude))
    {
      CkError("ERROR in MeshElement::checkInvariant: longitude must be greater than or equal to negative two PI and less than or equal to two PI.\n");
      error = true;
    }
  
  if (!(0.0 < manningsN))
    {
      CkError("ERROR in MeshElement::checkInvariant: manningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in MeshElement::checkInvariant: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 >= precipitationRate))
    {
      CkError("ERROR in MeshElement::checkInvariant: precipitationRate must be less than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 >= precipitationCumulativeShortTerm))
    {
      CkError("ERROR in MeshElement::checkInvariant: precipitationCumulativeShortTerm must be less than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 >= precipitationCumulativeLongTerm))
    {
      CkError("ERROR in MeshElement::checkInvariant: precipitationCumulativeLongTerm must be less than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= transpirationRate))
    {
      CkError("ERROR in MeshElement::checkInvariant: transpirationRate must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= transpirationCumulativeShortTerm))
    {
      CkError("ERROR in MeshElement::checkInvariant: transpirationCumulativeShortTerm must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= transpirationCumulativeLongTerm))
    {
      CkError("ERROR in MeshElement::checkInvariant: transpirationCumulativeLongTerm must be greater than or equal to zero.\n");
      error = true;
    }
  
  error = checkEvapoTranspirationForcingStructInvariant(&evapoTranspirationForcing) || error;
  
  error = checkEvapoTranspirationStateStructInvariant(&evapoTranspirationState) || error;
  
  error = underground.checkInvariant() || error;
  
  for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
    {
      error = (*itMesh).checkInvariant() || error;
    }
  
  for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
    {
      error = (*itChannel).checkInvariant() || error;
    }
  
  return error;
}

bool MeshElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                                       std::vector<MeshSurfacewaterMeshNeighborProxy>::size_type neighborProxyIndex,
                                                                       double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < meshNeighbors.size()))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: neighborProxyIndex must be greater than or equal to zero and less "
              "than meshNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      if (isBoundary(meshNeighbors[neighborProxyIndex].neighbor))
        {
          // FIXLATER what to do about inflowXVelocity, inflowYVelocity, and inflowHeight?
          error = surfacewaterMeshBoundaryFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                                   (BoundaryConditionEnum)meshNeighbors[neighborProxyIndex].neighbor, 0.0, 0.0, 0.0,
                                                   meshNeighbors[neighborProxyIndex].edgeLength, meshNeighbors[neighborProxyIndex].edgeNormalX,
                                                   meshNeighbors[neighborProxyIndex].edgeNormalY, elementArea, surfacewaterDepth);
        }
      else
        {
          error = surfacewaterMeshMeshFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                               meshNeighbors[neighborProxyIndex].edgeLength, elementX, elementY, elementZSurface, elementArea, manningsN,
                                               surfacewaterDepth, meshNeighbors[neighborProxyIndex].neighborX, meshNeighbors[neighborProxyIndex].neighborY,
                                               meshNeighbors[neighborProxyIndex].neighborZSurface, meshNeighbors[neighborProxyIndex].neighborArea,
                                               meshNeighbors[neighborProxyIndex].neighborManningsN, neighborSurfacewaterDepth);
        }
    }
  
  // Calculate expiration time.
  if (!error)
    {
      meshNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool MeshElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor(double currentTime, double regionalDtLimit,
                                                                          std::vector<MeshSurfacewaterChannelNeighborProxy>::size_type neighborProxyIndex,
                                                                          double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < channelNeighbors.size()))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: neighborProxyIndex must be greater than or equal to zero and "
              "less than channelNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in MeshElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      error = surfacewaterMeshChannelFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                              channelNeighbors[neighborProxyIndex].edgeLength,
                                              elementZSurface + channelNeighbors[neighborProxyIndex].neighborZOffset, elementArea, surfacewaterDepth,
                                              channelNeighbors[neighborProxyIndex].neighborZBank, channelNeighbors[neighborProxyIndex].neighborZBed,
                                              channelNeighbors[neighborProxyIndex].neighborBaseWidth, channelNeighbors[neighborProxyIndex].neighborSideSlope,
                                              neighborSurfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      channelNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool MeshElement::doPointProcessesAndSendOutflows(double referenceDate, double currentTime, double timestepEndTime, Region& region)
{
  bool   error                   = false;                                // Error flag.
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator    itMesh;    // Loop iterator.
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator itChannel; // Loop iterator.
  double localSolarDateTime      = referenceDate + (ADHydro::drainDownMode ? ADHydro::drainDownTime : currentTime) / (24.0 * 60.0 * 60.0) +
                                   longitude / (2 * M_PI);               // The time and date to use for the sun angle as a Julian date converted from UTC to
                                                                         // local solar time.  If we are using drainDownMode calendar date and time stands
                                                                         // still at the time specified by ADHydro::drainDownTime.
  long   year;                                                           // For calculating yearlen, julian, and hourAngle.
  long   month;                                                          // For calculating hourAngle.
  long   day;                                                            // For calculating hourAngle.
  long   hour;                                                           // For passing to julianToGregorian, unused.
  long   minute;                                                         // For passing to julianToGregorian, unused.
  double second;                                                         // For passing to julianToGregorian, unused.
  int    yearlen;                                                        // Input to evapoTranspirationSoil in days.
  float  julian;                                                         // Input to evapoTranspirationSoil in days.
  double hourAngle;                                                      // For calculating cosZ.  In radians.
  double declinationOfSun;                                               // For calculating cosZ.  In radians.
  float  cosZ;                                                           // Input to evapoTranspirationSoil, unitless.
  EvapoTranspirationSoilMoistureStruct evapoTranspirationSoilMoisture;   // Input to evapoTranspirationSoil.
  double dt                      = timestepEndTime - currentTime;        // Seconds.
  float  surfacewaterAdd;                                                // Output of evapoTranspirationSoil in millimeters.
  float  evaporationFromCanopy;                                          // Output of evapoTranspirationSoil in millimeters.
  float  evaporationFromSnow;                                            // Output of evapoTranspirationSoil in millimeters.
  float  evaporationFromGround;                                          // Output of evapoTranspirationSoil in millimeters.
  float  transpirationFromVegetation;                                    // Output of evapoTranspirationSoil in millimeters.
  float  waterError;                                                     // Output of evapoTranspirationSoil in millimeters.
  double originalEvapoTranspirationTotalWaterInDomain;                   // For mass balance check.
  double evaporation;                                                    // Meters.
  double transpiration;                                                  // Meters.
  double unsatisfiedEvaporation;                                         // Meters.
  double groundwaterEvaporation;                                         // Meters.
  double totalOutwardFlowRate    = 0.0;                                  // Sum of all outward flow rates in cubic meters per second.
  double outwardFlowRateFraction = 1.0;                                  // Fraction of all outward flow rates that can be satisfied, unitless.
  double waterSent;                                                      // Cubic meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(1721425.5 <= referenceDate))
    {
      CkError("ERROR in MeshElement::doPointProcessesAndSendOutflows: referenceDate must not be before 1 CE.\n");
      error = true;
    }
  
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in MeshElement::doPointProcessesAndSendOutflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  if (!error)
    {
      // Calculate year, month, and day.
      julianToGregorian(localSolarDateTime, &year, &month, &day, &hour, &minute, &second);

      // Determine if it is a leap year.
      if (0 == year % 400)
        {
          yearlen = 366;
        }
      else if (0 == year % 100)
        {
          yearlen = 365;
        }
      else if (0 == year % 4)
        {
          yearlen = 366;
        }
      else
        {
          yearlen = 365;
        }

      // Calculate the ordinal day of the year by subtracting the Julian date of Jan 1 beginning midnight.
      julian = localSolarDateTime - gregorianToJulian(year, 1, 1, 0, 0, 0);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0f <= julian && julian <= yearlen);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // FIXME if this element is shaded set cosZ to zero.

      // FIXME handle slope and aspect effects on solar radiation.

      // The number of "hours" that the sun is east or west of straight overhead.  The value is actually in radians with each hour being Pi/12 radians.
      // Positive means west.  Negative means east.
      hourAngle = (localSolarDateTime - gregorianToJulian(year, month, day, 12, 0, 0)) * 2.0 * M_PI;

      // Calculate cosZ.
      declinationOfSun = -23.44 * M_PI / 180.0 * cos(2.0 * M_PI * (julian + 10.0) / yearlen);
      cosZ             = sin(latitude) * sin(declinationOfSun) + cos(latitude) * cos(declinationOfSun) * cos(hourAngle);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(-1.0f <= cosZ && 1.0f >= cosZ);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

      // If the sun is below the horizon it doesn't matter how far below.  Set cosZ to zero.
      if (0.0f > cosZ)
        {
          cosZ = 0.0f;
        }

      // Fill in the Noah-MP soil moisture struct from groundwater and vadose zone state.
      underground.fillInEvapoTranspirationSoilMoistureStruct(elementZSurface, evapoTranspirationState.zSnso, evapoTranspirationSoilMoisture);

      // Save the original amount of water stored in evapoTranspirationState for mass balance check.
      originalEvapoTranspirationTotalWaterInDomain = evapoTranspirationTotalWaterInDomain(&evapoTranspirationState);
      
      // Do point processes for rainfall, snowmelt, and evapo-transpiration.
      error = evapoTranspirationSoil(vegetationType, underground.soilType, latitude, yearlen, julian, cosZ, dt, sqrt(elementArea), &evapoTranspirationForcing,
                                     &evapoTranspirationSoilMoisture, &evapoTranspirationState, &surfacewaterAdd, &evaporationFromCanopy, &evaporationFromSnow,
                                     &evaporationFromGround, &transpirationFromVegetation, &waterError);
      
      // Because Noah-MP uses single precision floats, its roundoff error is much higher than for doubles.  However, we can calculate the mass error using
      // doubles and account for it in surfacewaterError.  The mass error is the amount of water at the end (water stored in evapoTranspirationState plus water
      // that came out in the form of surfacewaterAdd, evaporationFromCanopy, and evaporationFromSnow minus water error) minus the amount of water at the
      // beginning (water stored in evapoTranspirationState plus water that went in in the form of precipitation).  evaporationFromGround and
      // transpirationFromVegetation are not used in this computation because that water is not taken out by Noah-MP.  It is taken below.
      surfacewaterError += (((double)evapoTranspirationTotalWaterInDomain(&evapoTranspirationState) + (double)surfacewaterAdd + (double)evaporationFromCanopy +
                             (double)evaporationFromSnow - (double)waterError) -
                            (originalEvapoTranspirationTotalWaterInDomain + (double)evapoTranspirationForcing.prcp * dt)) / 1000.0;
    }
  
  if (!error)
    {
      // Move water and record flows for precipitation, evaporation, and transpiration.
      surfacewaterDepth += surfacewaterAdd / 1000.0;
      evaporation        = ((double)evaporationFromCanopy + evaporationFromSnow) / 1000.0;
      transpiration      = 0.0;
      
      // Take evaporationFromGround first from surfacewater, and then if there isn't enough surfacewater from groundwater.  If there isn't enough groundwater
      // print a warning and reduce the quantity of evaporation.
      unsatisfiedEvaporation = evaporationFromGround / 1000.0;
      
      if (surfacewaterDepth >= unsatisfiedEvaporation)
        {
          evaporation       += unsatisfiedEvaporation;
          surfacewaterDepth -= unsatisfiedEvaporation;
        }
      else
        {
          unsatisfiedEvaporation -= surfacewaterDepth;
          evaporation            += surfacewaterDepth;
          surfacewaterDepth       = 0.0;
          groundwaterEvaporation  = underground.evaporate(unsatisfiedEvaporation, elementZSurface);
          unsatisfiedEvaporation -= groundwaterEvaporation;
          evaporation            += groundwaterEvaporation;

          if ((2 <= ADHydro::verbosityLevel && 1.0 < unsatisfiedEvaporation) || (3 <= ADHydro::verbosityLevel && 0.0 < unsatisfiedEvaporation))
            {
              CkError("WARNING in MeshElement::doPointProcessesAndSendOutflows, element %d: unsatisfied evaporation from ground of %le meters.\n",
                      elementNumber, unsatisfiedEvaporation);
            }
        }
      
      // Take transpiration first from groundwater, and then if there isn't enough groundwater from surfacewater.  If there isn't enough surfacewater print a
      // warning and reduce the quantity of transpiration.
      unsatisfiedEvaporation  = transpirationFromVegetation / 1000.0;
      groundwaterEvaporation  = underground.transpire(unsatisfiedEvaporation, elementZSurface);
      unsatisfiedEvaporation -= groundwaterEvaporation;
      transpiration          += groundwaterEvaporation;

      if (surfacewaterDepth >= unsatisfiedEvaporation)
        {
          transpiration     += unsatisfiedEvaporation;
          surfacewaterDepth -= unsatisfiedEvaporation;
        }
      else
        {
          unsatisfiedEvaporation -= surfacewaterDepth;
          transpiration          += surfacewaterDepth;
          surfacewaterDepth       = 0.0;

          if ((2 <= ADHydro::verbosityLevel && 1.0 < unsatisfiedEvaporation) || 3 <= ADHydro::verbosityLevel)
            {
              CkError("WARNING in MeshElement::doPointProcessesAndSendOutflows, element %d: unsatisfied transpiration of %le meters.\n", elementNumber,
                      unsatisfiedEvaporation);
            }
        }
      
      // Record cumulative flows and water error.
      precipitationRate                 = -evapoTranspirationForcing.prcp / 1000.0;
      precipitationCumulativeShortTerm += precipitationRate * dt;
      evaporationRate                   = evaporation / dt;
      evaporationCumulativeShortTerm   += evaporation;
      transpirationRate                 = transpiration / dt;
      transpirationCumulativeShortTerm += transpiration;
      surfacewaterError                += waterError / 1000.0;
      
      // If the roundoff error of adding one timestep's water to CumulativeShortTerm is greater than the roundoff error of adding CumulativeShortTerm to
      // CumulativeLongTerm then move CumulativeShortTerm to CumulativeLongTerm.
      // FIXME implement
      
      // Do point process for infiltration and send groundwater outflows.
      error = underground.doInfiltrationAndSendGroundwaterOutflows(currentTime, timestepEndTime, elementZSurface, elementArea, surfacewaterDepth, region);
    }
  
  if (!error)
    {
      // Limit surfacewater outflows.
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 < (*itMesh).nominalFlowRate)
            {
              totalOutwardFlowRate += (*itMesh).nominalFlowRate;
            }
        }

      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 < (*itChannel).nominalFlowRate)
            {
              totalOutwardFlowRate += (*itChannel).nominalFlowRate;
            }
        }

      if (surfacewaterDepth * elementArea < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = surfacewaterDepth * elementArea / (totalOutwardFlowRate * dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Send surfacewater outflows taking water from surfacewaterDepth.
      for (itMesh = meshNeighbors.begin(); !error && itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (isBoundary((*itMesh).neighbor) && 0.0 > (*itMesh).nominalFlowRate)
            {
              // Water for an inflow boundary arrives immediately.
              error = (*itMesh).insertMaterial(SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, -(*itMesh).nominalFlowRate * dt));
            }
          else if (0.0 < (*itMesh).nominalFlowRate)
            {
              // Send water for an outflow.
              waterSent                          = (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction;
              surfacewaterDepth                 -= waterSent / elementArea;
              (*itMesh).flowCumulativeShortTerm += waterSent;

              if (!isBoundary((*itMesh).neighbor))
                {
                  error = region.sendWater((*itMesh).region, RegionMessage(MESH_SURFACEWATER_MESH_NEIGHBOR, (*itMesh).neighbor,
                                                                           (*itMesh).reciprocalNeighborProxy, 0.0, 0.0,
                                                                           SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                                 waterSent)));
                }
            }
        }

      for (itChannel = channelNeighbors.begin(); !error && itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 < (*itChannel).nominalFlowRate)
            {
              // Send water for an outflow.
              waterSent                             = (*itChannel).nominalFlowRate * dt * outwardFlowRateFraction;
              surfacewaterDepth                    -= waterSent / elementArea;
              (*itChannel).flowCumulativeShortTerm += waterSent;

              error = region.sendWater((*itChannel).region, RegionMessage(CHANNEL_SURFACEWATER_MESH_NEIGHBOR, (*itChannel).neighbor,
                                                                          (*itChannel).reciprocalNeighborProxy, 0.0, 0.0,
                                                                          SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                                waterSent)));
            }
        }

      // Even though we are limiting outflows, surfacewaterDepth can go below zero due to roundoff error.
      if (!error && 0.0 > surfacewaterDepth)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0, surfacewaterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          surfacewaterError -= surfacewaterDepth;
          surfacewaterDepth  = 0.0;
        }
    }
  
  return error;
}

bool MeshElement::allInflowsArrived(double currentTime, double timestepEndTime)
{
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  bool allArrived = underground.allInflowsArrived(currentTime, timestepEndTime); // Whether all material has arrived from all neighbors.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in MeshElement::allInflowsArrived: currentTime must be less than or equal to timestepEndTime.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call allMaterialHasArrived.  allInflowsArrived shouldn't be called in this situation, but if it is return
  // that all inflows have arrived for the next zero seconds.
  if (currentTime < timestepEndTime)
    {
      for (itMesh = meshNeighbors.begin(); allArrived && itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              allArrived = (*itMesh).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
      
      for (itChannel = channelNeighbors.begin(); allArrived && itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              allArrived = (*itChannel).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
    }
  
  return allArrived;
}

bool MeshElement::receiveInflows(double currentTime, double timestepEndTime)
{
  bool                                                        error = false; // Error flag.
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator    itMesh;        // Loop iterator.
  std::vector<MeshSurfacewaterChannelNeighborProxy>::iterator itChannel;     // Loop iterator.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in MeshElement::receiveInflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call getMaterial.  advanceTime shouldn't be called in this situation, but if it is we don't have to do
  // anything to advance time by zero seconds.
  if (!error && currentTime < timestepEndTime)
    {
      // Receive all inflows.
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              surfacewaterDepth += (*itMesh).getMaterial(currentTime, timestepEndTime) / elementArea;
            }
        }
      
      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              surfacewaterDepth += (*itChannel).getMaterial(currentTime, timestepEndTime) / elementArea;
            }
        }
      
      underground.receiveInflows(currentTime, timestepEndTime, elementArea);
    }
  
  return error;
}

bool MeshElement::massBalance(double& waterInDomain, double& externalFlows, double& waterError)
{
  bool                                                     error = false; // Error flag.
  std::vector<MeshSurfacewaterMeshNeighborProxy>::iterator itMesh;        // Loop iterator.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= waterInDomain))
    {
      CkError("ERROR in MeshElement::massBalance: waterInDomain must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      error = underground.massBalance(waterInDomain, externalFlows, waterError, elementArea);
    }
  
  if (!error)
    {
      waterInDomain += surfacewaterDepth * elementArea;
      waterInDomain += (evapoTranspirationTotalWaterInDomain(&evapoTranspirationState) / 1000.0) * elementArea; // Divide by one thousand to convert from
                                                                                                                // millimeters to meters.

      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (isBoundary((*itMesh).neighbor))
            {
              externalFlows += (*itMesh).flowCumulativeShortTerm + (*itMesh).flowCumulativeLongTerm;
            }
        }
      
      externalFlows += (precipitationCumulativeShortTerm + precipitationCumulativeLongTerm) * elementArea;
      externalFlows += (evaporationCumulativeShortTerm   + evaporationCumulativeLongTerm)   * elementArea;
      externalFlows += (transpirationCumulativeShortTerm + transpirationCumulativeLongTerm) * elementArea;

      waterError += surfacewaterError * elementArea;
    }

  return error;
}
