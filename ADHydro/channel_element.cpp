#include "channel_element.h"
#include "adhydro.h"
#include "file_manager.h"
#include "surfacewater.h"
#include "groundwater.h"

ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy() :
  SimpleNeighborProxy(0.0, 0.0, false, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
  region(0),
  neighbor(0),
  reciprocalNeighborProxy(0),
  neighborZSurface(0.0),
  neighborZOffset(0.0),
  neighborArea(0.0),
  edgeLength(0.0),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
  // Initialization handled by initialization list.
}

ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, bool inflowOnlyInit,
                                                                           double flowCumulativeShortTermInit, double flowCumulativeLongTermInit,
                                                                           int regionInit, int neighborInit, int reciprocalNeighborProxyInit,
                                                                           double neighborZSurfaceInit, double neighborZOffsetInit, double neighborAreaInit,
                                                                           double edgeLengthInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, inflowOnlyInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
  region(regionInit),
  neighbor(neighborInit),
  reciprocalNeighborProxy(reciprocalNeighborProxyInit),
  neighborZSurface(neighborZSurfaceInit),
  neighborZOffset(neighborZOffsetInit),
  neighborArea(neighborAreaInit),
  edgeLength(edgeLengthInit),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= regionInit && regionInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy: regionInit must be greater than or equal to zero and less"
              " than globalNumberOfRegions.\n");
      CkExit();
    }
  
  if (!(0 <= neighborInit && neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy: neighborInit must be greater than or equal to zero and "
              "less than globalNumberOfMeshElements.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxyInit))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy: reciprocalNeighborProxyInit must be greater than or equal "
              "to zero.\n");
      CkExit();
    }
  
  if (!(0.0 < neighborAreaInit))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy: neighborAreaInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < edgeLengthInit))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy: edgeLengthInit must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void ChannelSurfacewaterMeshNeighborProxy::pup(PUP::er &p)
{
  SimpleNeighborProxy::pup(p);
  
  p | region;
  p | neighbor;
  p | reciprocalNeighborProxy;
  p | neighborZSurface;
  p | neighborZOffset;
  p | neighborArea;
  p | edgeLength;
  p | neighborInitialized;
  p | neighborInvariantChecked;
}

bool ChannelSurfacewaterMeshNeighborProxy::checkInvariant()
{
  bool error = SimpleNeighborProxy::checkInvariant(); // Error flag.
  
  if (!(0 <= region && region < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::checkInvariant: region must be greater than or equal to zero and less than globalNumberOfRegions.\n");
      error = true;
    }
  
  if (!(0 <= neighbor && neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::checkInvariant: neighbor must be greater than or equal to zero and less than globalNumberOfMeshElements.\n");
      error = true;
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::checkInvariant: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < neighborArea))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::checkInvariant: neighborArea must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::checkInvariant: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!neighborInitialized)
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::checkInvariant: neighborInitialized must be true before checking invariant.\n");
      error = true;
    }
  
  return error;
}

ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy() :
  SimpleNeighborProxy(0.0, 0.0, false, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
  region(0),
  neighbor(0),
  reciprocalNeighborProxy(0),
  neighborChannelType(NOT_USED),
  neighborZBank(0.0),
  neighborZBed(0.0),
  neighborLength(0.0),
  neighborBaseWidth(0.0),
  neighborSideSlope(0.0),
  neighborManningsN(0.0),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
  // Initialization handled by initialization list.
}

ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, bool inflowOnlyInit,
                                                                                 double flowCumulativeShortTermInit, double flowCumulativeLongTermInit,
                                                                                 int regionInit, int neighborInit, int reciprocalNeighborProxyInit,
                                                                                 ChannelTypeEnum neighborChannelTypeInit, double neighborZBankInit,
                                                                                 double neighborZBedInit, double neighborLengthInit,
                                                                                 double neighborBaseWidthInit, double neighborSideSlopeInit,
                                                                                 double neighborManningsNInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, inflowOnlyInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
  region(regionInit),
  neighbor(neighborInit),
  reciprocalNeighborProxy(reciprocalNeighborProxyInit),
  neighborChannelType(neighborChannelTypeInit),
  neighborZBank(neighborZBankInit),
  neighborZBed(neighborZBedInit),
  neighborLength(neighborLengthInit),
  neighborBaseWidth(neighborBaseWidthInit),
  neighborSideSlope(neighborSideSlopeInit),
  neighborManningsN(neighborManningsNInit),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= regionInit && regionInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: regionInit must be greater than or equal to "
              "zero and less than globalNumberOfRegions.\n");
      CkExit();
    }
  
  if (!(isBoundary(neighborInit) || (0 <= neighborInit && neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: neighborInit must be a boundary condition code or "
              "greater than or equal to zero and less than globalNumberOfChannelElements.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxyInit))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: reciprocalNeighborProxyInit must be greater than or "
              "equal to zero.\n");
      CkExit();
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(NOT_USED == neighborChannelTypeInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for boundary condition codes "
                  "neighborChannelTypeInit must be NOT_USED.\n");
          CkExit();
        }
    }
  else
    {
      if (!(STREAM == neighborChannelTypeInit || WATERBODY == neighborChannelTypeInit || ICEMASS == neighborChannelTypeInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for non-boundary neighbors "
                  "neighborChannelTypeInit must be STREAM or WATERBODY or ICEMASS.\n");
          CkExit();
        }
    }
  
  if (!(neighborZBankInit >= neighborZBedInit))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: neighborZBankInit must be greater than or equal to "
              "neighborZBedInit.\n");
      CkExit();
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(0.0 == neighborLengthInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for boundary condition codes neighborLengthInit "
                  "must be zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborLengthInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for non-boundary neighbors neighborLengthInit "
                  "must be greater than zero.\n");
          CkExit();
        }
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(0.0 == neighborBaseWidthInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for boundary condition codes "
                  "neighborBaseWidthInit must be zero.\n");
          CkExit();
        }
      
      if (!(0.0 == neighborSideSlopeInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for boundary condition codes "
                  "neighborSideSlopeInit must be zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 <= neighborBaseWidthInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for non-boundary neighbors "
                  "neighborBaseWidthInit must be greater than or equal to zero.\n");
          CkExit();
        }
      
      if (!(0.0 <= neighborSideSlopeInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for non-boundary neighbors "
                  "neighborSideSlopeInit must be greater than or equal to zero.\n");
          CkExit();
        }
      
      if (!(0.0 < neighborBaseWidthInit || 0.0 < neighborSideSlopeInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for non-boundary neighbors "
                  "one of neighborBaseWidthInit or neighborSideSlopeInit must be greater than zero.\n");
          CkExit();
        }
    }
  
  if (isBoundary(neighborInit))
    {
      if (!(0.0 == neighborManningsNInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for boundary condition codes "
                  "neighborManningsNInit must be zero.\n");
          CkExit();
        }
    }
  else
    {
      if (!(0.0 < neighborManningsNInit))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: for non-boundary neighbors "
                  "neighborManningsNInit must be greater than zero.\n");
          CkExit();
        }
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void ChannelSurfacewaterChannelNeighborProxy::pup(PUP::er &p)
{
  SimpleNeighborProxy::pup(p);
  
  p | region;
  p | neighbor;
  p | reciprocalNeighborProxy;
  p | neighborChannelType;
  p | neighborZBank;
  p | neighborZBed;
  p | neighborLength;
  p | neighborBaseWidth;
  p | neighborSideSlope;
  p | neighborManningsN;
  p | neighborInitialized;
  p | neighborInvariantChecked;
}

bool ChannelSurfacewaterChannelNeighborProxy::checkInvariant()
{
  bool error = SimpleNeighborProxy::checkInvariant(); // Error flag.
  
  if (!(0 <= region && region < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: region must be greater than or equal to zero and less than "
              "globalNumberOfRegions.\n");
      error = true;
    }
  
  if (!(isBoundary(neighbor) || (0 <= neighbor && neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements)))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: neighbor must be a boundary condition code or greater than or equal to "
              "zero and less than globalNumberOfChannelElements.\n");
      error = true;
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (isBoundary(neighbor))
    {
      if (!(NOT_USED == neighborChannelType))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for boundary condition codes neighborChannelType must be NOT_USED.\n");
          error = true;
        }
    }
  else
    {
      if (!(STREAM == neighborChannelType || WATERBODY == neighborChannelType || ICEMASS == neighborChannelType))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for non-boundary neighbors neighborChannelType must be STREAM or "
                  "WATERBODY or ICEMASS.\n");
          error = true;
        }
    }
  
  if (!(neighborZBank >= neighborZBed))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: neighborZBank must be greater than or equal to neighborZBed.\n");
      error = true;
    }
  
  if (isBoundary(neighbor))
    {
      if (!(0.0 == neighborLength))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for boundary condition codes neighborLength must be zero.\n");
          error = true;
        }
    }
  else
    {
      if (!(0.0 < neighborLength))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for non-boundary neighbors neighborLength must be greater than zero.\n");
          error = true;
        }
    }
  
  if (isBoundary(neighbor))
    {
      if (!(0.0 == neighborBaseWidth))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for boundary condition codes neighborBaseWidth must be zero.\n");
          error = true;
        }
      
      if (!(0.0 == neighborSideSlope))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for boundary condition codes neighborSideSlope must be zero.\n");
          error = true;
        }
    }
  else
    {
      if (!(0.0 <= neighborBaseWidth))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for non-boundary neighbors neighborBaseWidth must be greater than or "
                  "equal to zero.\n");
          error = true;
        }
      
      if (!(0.0 <= neighborSideSlope))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for non-boundary neighbors neighborSideSlope must be greater than or "
                  "equal to zero.\n");
          error = true;
        }
      
      if (!(0.0 < neighborBaseWidth || 0.0 < neighborSideSlope))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for non-boundary neighbors one of neighborBaseWidth or neighborSideSlope "
                  "must be greater than zero.\n");
          error = true;
        }
    }
  
  if (isBoundary(neighbor))
    {
      if (!(0.0 == neighborManningsN))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for boundary condition codes neighborManningsN must be zero.\n");
          error = true;
        }
    }
  else
    {
      if (!(0.0 < neighborManningsN))
        {
          CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: for non-boundary neighbors neighborManningsN must be greater than "
                  "zero.\n");
          error = true;
        }
    }
  
  if (!neighborInitialized)
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: neighborInitialized must be true before checking invariant.\n");
      error = true;
    }
  
  return error;
}

ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy() :
  SimpleNeighborProxy(0.0, 0.0, false, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
  region(0),
  neighbor(0),
  reciprocalNeighborProxy(0),
  neighborZSurface(0.0),
  neighborLayerZBottom(0.0),
  neighborZOffset(0.0),
  edgeLength(0.0),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
  // Initialization handled by initialization list.
}

ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit, bool inflowOnlyInit,
                                                                         double flowCumulativeShortTermInit, double flowCumulativeLongTermInit, int regionInit,
                                                                         int neighborInit, int reciprocalNeighborProxyInit, double neighborZSurfaceInit,
                                                                         double neighborLayerZBottomInit, double neighborZOffsetInit, double edgeLengthInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, inflowOnlyInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
  region(regionInit),
  neighbor(neighborInit),
  reciprocalNeighborProxy(reciprocalNeighborProxyInit),
  neighborZSurface(neighborZSurfaceInit),
  neighborLayerZBottom(neighborLayerZBottomInit),
  neighborZOffset(neighborZOffsetInit),
  edgeLength(edgeLengthInit),
  neighborInitialized(false),
  neighborInvariantChecked(false)
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= regionInit && regionInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy: regionInit must be greater than or equal to zero and less "
              "than globalNumberOfRegions.\n");
      CkExit();
    }
  
  if (!(0 <= neighborInit && neighborInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy: neighborInit must be greater than or equal to zero and less "
              "than globalNumberOfMeshElements.\n");
      CkExit();
    }
  
  if (!(0 <= reciprocalNeighborProxyInit))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy: reciprocalNeighborProxyInit must be greater than or equal "
              "to zero.\n");
      CkExit();
    }
  
  if (!(neighborZSurfaceInit >= neighborLayerZBottomInit))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy: neighborZSurfaceInit must be greater than or equal to "
              "neighborLayerZBottomInit.\n");
      CkExit();
    }
  
  if (!(0.0 < edgeLengthInit))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy: edgeLengthInit must be greater than zero.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

void ChannelGroundwaterMeshNeighborProxy::pup(PUP::er &p)
{
  SimpleNeighborProxy::pup(p);
  
  p | region;
  p | neighbor;
  p | reciprocalNeighborProxy;
  p | neighborZSurface;
  p | neighborLayerZBottom;
  p | neighborZOffset;
  p | edgeLength;
  p | neighborInitialized;
  p | neighborInvariantChecked;
}

bool ChannelGroundwaterMeshNeighborProxy::checkInvariant()
{
  bool error = SimpleNeighborProxy::checkInvariant(); // Error flag.
  
  if (!(0 <= region && region < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfRegions))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::checkInvariant: region must be greater than or equal to zero and less than "
              "globalNumberOfRegions.\n");
      error = true;
    }
  
  if (!(0 <= neighbor && neighbor < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfMeshElements))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::checkInvariant: neighbor must be greater than or equal to zero and less than "
              "globalNumberOfMeshElements.\n");
      error = true;
    }
  
  if (!(0 <= reciprocalNeighborProxy))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::checkInvariant: reciprocalNeighborProxy must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(neighborZSurface >= neighborLayerZBottom))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::checkInvariant: neighborZSurface must be greater than or equal to neighborLayerZBottom.\n");
      error = true;
    }
  
  if (!(0.0 < edgeLength))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::checkInvariant: edgeLength must be greater than zero.\n");
      error = true;
    }
  
  if (!neighborInitialized)
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::checkInvariant: neighborInitialized must be true before checking invariant.\n");
      error = true;
    }
  
  return error;
}

ChannelElement::ChannelElement() :
  elementNumber(0), // Dummy values will be overwritten by pup_stl.h code.
  channelType(NOT_USED),
  reachCode(0),
  elementX(0.0),
  elementY(0.0),
  elementZBank(0.0),
  elementZBed(0.0),
  elementLength(0.0),
  latitude(0.0),
  longitude(0.0),
  baseWidth(0.0),
  sideSlope(0.0),
  bedConductivity(0.0),
  bedThickness(0.0),
  manningsN(0.0),
  surfacewaterDepth(0.0),
  surfacewaterError(0.0),
  precipitationRate(0.0),
  precipitationCumulativeShortTerm(0.0),
  precipitationCumulativeLongTerm(0.0),
  evaporationRate(0.0),
  evaporationCumulativeShortTerm(0.0),
  evaporationCumulativeLongTerm(0.0),
  evapoTranspirationForcing(),
  forcingUpdated(false),
  evapoTranspirationState(),
  meshNeighbors(),
  channelNeighbors(),
  undergroundMeshNeighbors(),
  reservoir(NULL),
  reservoirReleaseRecipient(-1),
  diversion(NULL),
  diversionReleaseRecipients(),
  diversionReleaseRecipientsRegions(),
  diversionReleaseRecipientsReciprocalNeighborProxies(),
  diversionReleaseRecipientsInitialized()
{
  // Initialization handled by initialization list.
}

ChannelElement::ChannelElement(int elementNumberInit, ChannelTypeEnum channelTypeInit, long long reachCodeInit, double elementXInit, double elementYInit,
                               double elementZBankInit, double elementZBedInit, double elementLengthInit, double latitudeInit, double longitudeInit,
                               double baseWidthInit, double sideSlopeInit, double bedConductivityInit, double bedThicknessInit, double manningsNInit,
                               double surfacewaterDepthInit, double surfacewaterErrorInit, double precipitationRateInit,
                               double precipitationCumulativeShortTermInit, double precipitationCumulativeLongTermInit, double evaporationRateInit,
                               double evaporationCumulativeShortTermInit, double evaporationCumulativeLongTermInit,
                               EvapoTranspirationForcingStruct& evapoTranspirationForcingInit, EvapoTranspirationStateStruct& evapoTranspirationStateInit) :
  elementNumber(elementNumberInit),
  channelType(channelTypeInit),
  reachCode(reachCodeInit),
  elementX(elementXInit),
  elementY(elementYInit),
  elementZBank(elementZBankInit),
  elementZBed(elementZBedInit),
  elementLength(elementLengthInit),
  latitude(latitudeInit),
  longitude(longitudeInit),
  baseWidth(baseWidthInit),
  sideSlope(sideSlopeInit),
  bedConductivity(bedConductivityInit),
  bedThickness(bedThicknessInit),
  manningsN(manningsNInit),
  surfacewaterDepth(surfacewaterDepthInit),
  surfacewaterError(surfacewaterErrorInit),
  precipitationRate(precipitationRateInit),
  precipitationCumulativeShortTerm(precipitationCumulativeShortTermInit),
  precipitationCumulativeLongTerm(precipitationCumulativeLongTermInit),
  evaporationRate(evaporationRateInit),
  evaporationCumulativeShortTerm(evaporationCumulativeShortTermInit),
  evaporationCumulativeLongTerm(evaporationCumulativeLongTermInit),
  evapoTranspirationForcing(evapoTranspirationForcingInit),
  forcingUpdated(false),
  evapoTranspirationState(evapoTranspirationStateInit),
  meshNeighbors(),
  channelNeighbors(),
  undergroundMeshNeighbors(),
  reservoir(NULL),
  reservoirReleaseRecipient(-1),
  diversion(NULL),
  diversionReleaseRecipients(),
  diversionReleaseRecipientsRegions(),
  diversionReleaseRecipientsReciprocalNeighborProxies(),
  diversionReleaseRecipientsInitialized()
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0 <= elementNumberInit && elementNumberInit < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))
    {
      CkError("ERROR in ChannelElement::ChannelElement: elementNumberInit must be greater than or equal to zero and less than "
              "globalNumberOfChannelElements.\n");
      CkExit();
    }

  if (!(STREAM == channelTypeInit || WATERBODY == channelTypeInit || ICEMASS == channelTypeInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: channelTypeInit must be STREAM or WATERBODY or ICEMASS.\n");
      CkExit();
    }
  
  if (!(elementZBankInit >= elementZBedInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: elementZBankInit must be greater than or equal to elementZBedInit.\n");
      CkExit();
    }
  
  if (!(0.0 < elementLengthInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: elementLength must be greater than zero.\n");
      CkExit();
    }
  
  if (!(-M_PI / 2.0 <= latitudeInit && M_PI / 2.0 >= latitudeInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: latitudeInit must be greater than or equal to negative PI over two and less than or equal to PI over "
              "two.\n");
      CkExit();
    }
  
  if (!(-M_PI * 2.0 <= longitudeInit && M_PI * 2.0 >= longitudeInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: longitudeInit must be greater than or equal to negative two PI and less than or equal to two PI.\n");
      CkExit();
    }
  
  if (!(0.0 <= baseWidthInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: baseWidthInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= sideSlopeInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: sideSlopeInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 < baseWidthInit || 0.0 < sideSlopeInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: one of baseWidthInit or sideSlopeInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < bedConductivityInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: bedConductivityInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < bedThicknessInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: bedThicknessInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 < manningsNInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: manningsNInit must be greater than zero.\n");
      CkExit();
    }
  
  if (!(0.0 <= surfacewaterDepthInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: surfacewaterDepthInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 >= precipitationRateInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: precipitationRateInit must be less than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 >= precipitationCumulativeShortTermInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: precipitationCumulativeShortTermInit must be less than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0.0 >= precipitationCumulativeLongTermInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: precipitationCumulativeLongTermInit must be less than or equal to zero.\n");
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
  
  if (WATERBODY == channelType)
    {
      reservoir = ADHydro::fileManagerProxy.ckLocalBranch()->reservoirFactory.create(reachCode);
    }
  
  diversion = ADHydro::fileManagerProxy.ckLocalBranch()->diversionFactory.create(elementNumber, ADHydro::referenceDate, ADHydro::currentTime);
  
  if (NULL != diversion)
    {
      diversionReleaseRecipients = diversion->getMeshNeighbors();
      diversionReleaseRecipientsRegions.insert(diversionReleaseRecipientsRegions.begin(), diversionReleaseRecipients.size(), -1);
      diversionReleaseRecipientsReciprocalNeighborProxies.insert(diversionReleaseRecipientsReciprocalNeighborProxies.begin(), diversionReleaseRecipients.size(), -1);
      diversionReleaseRecipientsInitialized.insert(diversionReleaseRecipientsInitialized.begin(), diversionReleaseRecipients.size(), false);
      
      // FIXME add to invariant that if diversion is NULL then diversionReleaseRecipients et. al. must be empty.
    }
}

// FIXME destructor to delete reservoir & diversion.

void ChannelElement::pup(PUP::er &p)
{
  p | elementNumber;
  p | channelType;
  p | reachCode;
  p | elementX;
  p | elementY;
  p | elementZBank;
  p | elementZBed;
  p | elementLength;
  p | latitude;
  p | longitude;
  p | baseWidth;
  p | sideSlope;
  p | bedConductivity;
  p | bedThickness;
  p | manningsN;
  p | surfacewaterDepth;
  p | surfacewaterError;
  p | precipitationRate;
  p | precipitationCumulativeShortTerm;
  p | precipitationCumulativeLongTerm;
  p | evaporationRate;
  p | evaporationCumulativeShortTerm;
  p | evaporationCumulativeLongTerm;
  p | evapoTranspirationForcing;
  p | forcingUpdated;
  p | evapoTranspirationState;
  p | meshNeighbors;
  p | channelNeighbors;
  p | undergroundMeshNeighbors;
  p | reservoir;
  p | reservoirReleaseRecipient;
  p | diversion;
  p | diversionReleaseRecipients;
  p | diversionReleaseRecipientsRegions;
  p | diversionReleaseRecipientsReciprocalNeighborProxies;
  p | diversionReleaseRecipientsInitialized;
}

bool ChannelElement::checkInvariant()
{
  bool                                                           error = false;     // Error flag.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh; // Loop iterator.
  
  if (!(0 <= elementNumber && elementNumber < ADHydro::fileManagerProxy.ckLocalBranch()->globalNumberOfChannelElements))
    {
      CkError("ERROR in ChannelElement::checkInvariant: elementNumber must be greater than or equal to zero and less than globalNumberOfChannelElements.\n");
      error = true;
    }

  if (!(STREAM == channelType || WATERBODY == channelType || ICEMASS == channelType))
    {
      CkError("ERROR in ChannelElement::checkInvariant: channelType must be STREAM or WATERBODY or ICEMASS.\n");
      error = true;
    }
  
  if (!(elementZBank >= elementZBed))
    {
      CkError("ERROR in ChannelElement::checkInvariant: elementZBank must be greater than or equal to elementZBed.\n");
      error = true;
    }
  
  if (!(0.0 < elementLength))
    {
      CkError("ERROR in ChannelElement::checkInvariant: elementLength must be greater than zero.\n");
      error = true;
    }
  
  if (!(-M_PI / 2.0 <= latitude && M_PI / 2.0 >= latitude))
    {
      CkError("ERROR in ChannelElement::checkInvariant: latitude must be greater than or equal to negative PI over two and less than or equal to PI over two.\n");
      error = true;
    }
  
  if (!(-M_PI * 2.0 <= longitude && M_PI * 2.0 >= longitude))
    {
      CkError("ERROR in ChannelElement::checkInvariant: longitude must be greater than or equal to negative two PI and less than or equal to two PI.\n");
      error = true;
    }
  
  if (!(0.0 <= baseWidth))
    {
      CkError("ERROR in ChannelElement::checkInvariant: baseWidth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 <= sideSlope))
    {
      CkError("ERROR in ChannelElement::checkInvariant: sideSlope must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 < baseWidth || 0.0 < sideSlope))
    {
      CkError("ERROR in ChannelElement::checkInvariant: one of baseWidth or sideSlope must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < bedConductivity))
    {
      CkError("ERROR in ChannelElement::checkInvariant: bedConductivity must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < bedThickness))
    {
      CkError("ERROR in ChannelElement::checkInvariant: bedThickness must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 < manningsN))
    {
      CkError("ERROR in ChannelElement::checkInvariant: manningsN must be greater than zero.\n");
      error = true;
    }
  
  if (!(0.0 <= surfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::checkInvariant: surfacewaterDepth must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 >= precipitationRate))
    {
      CkError("ERROR in ChannelElement::checkInvariant: precipitationRate must be less than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 >= precipitationCumulativeShortTerm))
    {
      CkError("ERROR in ChannelElement::checkInvariant: precipitationCumulativeShortTerm must be less than or equal to zero.\n");
      error = true;
    }
  
  if (!(0.0 >= precipitationCumulativeLongTerm))
    {
      CkError("ERROR in ChannelElement::checkInvariant: precipitationCumulativeLongTerm must be less than or equal to zero.\n");
      error = true;
    }
  
  error = checkEvapoTranspirationForcingStructInvariant(&evapoTranspirationForcing) || error;
  error = checkEvapoTranspirationStateStructInvariant(&evapoTranspirationState)     || error;
  
  for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
    {
      error = (*itMesh).checkInvariant() || error;
    }
  
  for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
    {
      error = (*itChannel).checkInvariant() || error;
    }
  
  for (itUndergroundMesh = undergroundMeshNeighbors.begin(); itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
    {
      error = (*itUndergroundMesh).checkInvariant() || error;
    }
  
  return error;
}

bool ChannelElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                                          std::vector<ChannelSurfacewaterMeshNeighborProxy>::size_type neighborProxyIndex,
                                                                          double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < meshNeighbors.size()))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: neighborProxyIndex must be greater than or equal to zero and "
              "less than meshNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterMeshNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      error = surfacewaterMeshChannelFlowRate(&meshNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                              meshNeighbors[neighborProxyIndex].edgeLength,
                                              meshNeighbors[neighborProxyIndex].neighborZSurface + meshNeighbors[neighborProxyIndex].neighborZOffset,
                                              meshNeighbors[neighborProxyIndex].neighborArea, neighborSurfacewaterDepth, elementZBank, elementZBed, baseWidth,
                                              sideSlope, surfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      meshNeighbors[neighborProxyIndex].nominalFlowRate *= -1.0; // Use negative of flow rate so that positive means flow out of the channel.
      meshNeighbors[neighborProxyIndex].expirationTime   = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool ChannelElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor(double currentTime, double regionalDtLimit,
                                                                             std::vector<ChannelSurfacewaterChannelNeighborProxy>::size_type
                                                                             neighborProxyIndex, double neighborSurfacewaterDepth)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < channelNeighbors.size()))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: neighborProxyIndex must be greater than or equal to zero and "
              "less than channelNeighbors.size().\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithSurfacewaterChannelNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      if (isBoundary(channelNeighbors[neighborProxyIndex].neighbor))
        {
          // FIXLATER what to do about inflowVelocity and inflowHeight?
          error = surfacewaterChannelBoundaryFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit,
                                                      (BoundaryConditionEnum)channelNeighbors[neighborProxyIndex].neighbor, 0.0, 0.0, elementLength, baseWidth,
                                                      sideSlope, surfacewaterDepth);
        }
      else
        {
          error = surfacewaterChannelChannelFlowRate(&channelNeighbors[neighborProxyIndex].nominalFlowRate, &regionalDtLimit, channelType, elementZBank, elementZBed,
                                                     elementLength, baseWidth, sideSlope, manningsN, surfacewaterDepth,
                                                     channelNeighbors[neighborProxyIndex].neighborChannelType,
                                                     channelNeighbors[neighborProxyIndex].neighborZBank, channelNeighbors[neighborProxyIndex].neighborZBed,
                                                     channelNeighbors[neighborProxyIndex].neighborLength,
                                                     channelNeighbors[neighborProxyIndex].neighborBaseWidth,
                                                     channelNeighbors[neighborProxyIndex].neighborSideSlope,
                                                     channelNeighbors[neighborProxyIndex].neighborManningsN, neighborSurfacewaterDepth);
        }
    }
  
  // Calculate expiration time.
  if (!error)
    {
      channelNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor(double currentTime, double regionalDtLimit,
                                                                         std::vector<ChannelGroundwaterMeshNeighborProxy>::size_type neighborProxyIndex,
                                                                         double neighborSurfacewaterDepth, double neighborGroundwaterHead)
{
  bool error = false; // Error flag.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 < regionalDtLimit))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor: regionalDtLimit must be greater than zero.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < undergroundMeshNeighbors.size()))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborProxyIndex must be greater than or equal to zero and "
              "less than undergroundMeshNeighbors.size().\n");
      error = true;
    }
  else if (!(undergroundMeshNeighbors[neighborProxyIndex].neighborZSurface >= neighborGroundwaterHead))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborGroundwaterHead must be less than or equal to "
              "undergroundMeshNeighbors[neighborProxyIndex].neighborZSurface.\n");
      error = true;
    }
  
  if (!(0.0 <= neighborSurfacewaterDepth))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateWithGroundwaterMeshNeighbor: neighborSurfacewaterDepth must be greater than or equal to "
              "zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      // FIXME figure out how to calculate dtNew
      error = groundwaterMeshChannelFlowRate(&undergroundMeshNeighbors[neighborProxyIndex].nominalFlowRate,
                                             undergroundMeshNeighbors[neighborProxyIndex].edgeLength,
                                             undergroundMeshNeighbors[neighborProxyIndex].neighborZSurface +
                                             undergroundMeshNeighbors[neighborProxyIndex].neighborZOffset,
                                             undergroundMeshNeighbors[neighborProxyIndex].neighborLayerZBottom +
                                             undergroundMeshNeighbors[neighborProxyIndex].neighborZOffset, neighborSurfacewaterDepth,
                                             neighborGroundwaterHead + undergroundMeshNeighbors[neighborProxyIndex].neighborZOffset, elementZBank, elementZBed,
                                             baseWidth, sideSlope, bedConductivity, bedThickness, surfacewaterDepth);
    }
  
  // Calculate expiration time.
  if (!error)
    {
      undergroundMeshNeighbors[neighborProxyIndex].nominalFlowRate *= -1.0; // Use negative of flow rate so that positive means flow out of the channel.
      undergroundMeshNeighbors[neighborProxyIndex].expirationTime   = ADHydro::newExpirationTime(currentTime, regionalDtLimit);
    }
  
  return error;
}

bool ChannelElement::calculateNominalFlowRateForReservoirRelease(double referenceDate, double currentTime, std::vector<ChannelSurfacewaterChannelNeighborProxy>::size_type neighborProxyIndex)
{
  bool   error         = false;                                       // Error flag.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::size_type ii; // Loop iterator.
  double currentInflow = 0.0;                                         // Current flow rate into the reservoir from upstream in cubic meters per second.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(1721425.5 <= referenceDate))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateForReservoirRelease: referenceDate must not be before 1 CE.\n");
      error = true;
    }
  
  if (!(0 <= neighborProxyIndex && neighborProxyIndex < channelNeighbors.size()))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateForReservoirRelease: neighborProxyIndex must be greater than or equal to zero and "
              "less than channelNeighbors.size().\n");
      error = true;
    }
  
  if (!(NULL != reservoir))
    {
      CkError("ERROR in ChannelElement::calculateNominalFlowRateForReservoirRelease: reservoir must not be NULL.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // Calculate nominal flow rate.
  if (!error)
    {
      // Calculate current inflow from neighbor proxies.
      for (ii = 0; ii < channelNeighbors.size(); ++ii)
        {
          if (ii != neighborProxyIndex)
            {
              currentInflow -= channelNeighbors[ii].nominalFlowRate; // Inflows are negative, but the release function takes the total inflow as a positive number.
            }
        }
      
      // FIXME Who does the time zone conversion?
      reservoir->release(currentInflow, surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth) * elementLength, referenceDate,
                         (ADHydro::drainDownMode ? ADHydro::drainDownTime : currentTime),
                         channelNeighbors[neighborProxyIndex].nominalFlowRate, channelNeighbors[neighborProxyIndex].expirationTime);
      
      // FIXME do we want to do this, or leave complete control over expiration time to water management code?
      // FIXME really forcing expiration times to be on standard boundaries is just a performance improvement.
      // FIXME One-way flows don't even need both neighbors to agree on the expiration time.
      //channelNeighbors[neighborProxyIndex].expirationTime = ADHydro::newExpirationTime(currentTime, channelNeighbors[neighborProxyIndex].expirationTime - currentTime);
      
      // FIXME issues with doing this.  How to best set expiration time in drain down mode?
      if (ADHydro::drainDownMode)
        {
          channelNeighbors[neighborProxyIndex].expirationTime = channelNeighbors[neighborProxyIndex].expirationTime - ADHydro::drainDownTime + currentTime;
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 <= channelNeighbors[neighborProxyIndex].nominalFlowRate);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
    }
  
  return error;
}

bool ChannelElement::doPointProcessesAndSendOutflows(double referenceDate, double currentTime, double timestepEndTime, Region& region)
{
  bool   error                   = false;                                           // Error flag.
  size_t ii;                                                                        // Loop counter.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh; // Loop iterator.
  double localSolarDateTime      = referenceDate + (ADHydro::drainDownMode ? ADHydro::drainDownTime : currentTime) / (24.0 * 60.0 * 60.0) +
                                   longitude / (2 * M_PI);                          // The time and date to use for the sun angle as a Julian date converted
                                                                                    // from UTC to local solar time.  If we are using drainDownMode calendar
                                                                                    // date and time stands still at the time specified by
                                                                                    // ADHydro::drainDownTime.
  long   year;                                                                      // For calculating yearlen, julian, and hourAngle.
  long   month;                                                                     // For calculating hourAngle.
  long   day;                                                                       // For calculating hourAngle.
  long   hour;                                                                      // For passing to julianToGregorian, unused.
  long   minute;                                                                    // For passing to julianToGregorian, unused.
  double second;                                                                    // For passing to julianToGregorian, unused.
  int    yearlen;                                                                   // Input to evapoTranspirationSoil in days.
  float  julian;                                                                    // Input to evapoTranspirationSoil in days.
  double hourAngle;                                                                 // For calculating cosZ.  In radians.
  double declinationOfSun;                                                          // For calculating cosZ.  In radians.
  float  cosZ;                                                                      // Input to evapoTranspirationSoil, unitless.
  double dt                      = timestepEndTime - currentTime;                   // Seconds.
  float  surfacewaterAdd;                                                           // Output of evapoTranspirationSoil in millimeters.
  float  evaporationFromSnow;                                                       // Output of evapoTranspirationSoil in millimeters.
  float  evaporationFromGround;                                                     // Output of evapoTranspirationSoil in millimeters.
  float  waterError;                                                                // Output of evapoTranspirationSoil in millimeters.
  double originalEvapoTranspirationTotalWaterInDomain;                              // For mass balance check.
  double evaporation;                                                               // Cubic meters.
  double unsatisfiedEvaporation;                                                    // Cubic meters.
  double waterAvailable;                                                            // Cubic meters of water available for a diversion to take.
  std::vector<std::pair<int, double> >           waterToDivert;                     // Vector of (mesh elementID, cubic meters of water to divert to that mesh element).
  std::vector<std::pair<int, double> >::iterator itDiversion;                       // loop iterator.
  double totalOutwardFlowRate    = 0.0;                                             // Sum of all outward flow rates in cubic meters per second.
  double outwardFlowRateFraction = 1.0;                                             // Fraction of all outward flow rates that can be satisfied, unitless.
  double crossSectionArea        = surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth);
                                                                                    // Wetted cross sectional area of channel in square meters.
  double topArea                 = (baseWidth + 2.0 * sideSlope * surfacewaterDepth) * elementLength;
                                                                                    // surface area of the water top surface in square meters.
  double waterSent;                                                                 // Cubic meters.

// FIXME remove
FILE* hydrographFile;
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(1721425.5 <= referenceDate))
    {
      CkError("ERROR in ChannelElement::doPointProcessesAndSendOutflows: referenceDate must not be before 1 CE.\n");
      error = true;
    }
  
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in ChannelElement::doPointProcessesAndSendOutflows: currentTime must be less than or equal to timestepEndTime.\n");
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

      // Save the original amount of water stored in evapoTranspirationState for mass balance check.
      originalEvapoTranspirationTotalWaterInDomain = evapoTranspirationTotalWaterInDomain(&evapoTranspirationState);

      // Do point processes for rainfall, snowmelt, and evapo-transpiration.
      // FIXME Currently this is double counting area.  Streams overlap with adjacent mesh elements and both evaporate from that area.
      if (ICEMASS == channelType)
        {
          error = evapoTranspirationGlacier(cosZ, dt, &evapoTranspirationForcing, &evapoTranspirationState, &surfacewaterAdd, &evaporationFromSnow,
                                            &evaporationFromGround, &waterError);
        }
      else
        {
          error = evapoTranspirationWater(latitude, yearlen, julian, cosZ, dt, elementLength, surfacewaterDepth * 1000.0, &evapoTranspirationForcing,
                                          &evapoTranspirationState, &surfacewaterAdd, &evaporationFromSnow, &evaporationFromGround, &waterError);
        }
      
      // Because Noah-MP uses single precision floats, its roundoff error is much higher than for doubles.  However, we can calculate the mass error using
      // doubles and account for it in surfacewaterError.  The mass error is the amount of water at the end (water stored in evapoTranspirationState plus water
      // that came out in the form of surfacewaterAdd and evaporationFromSnow minus water error) minus the amount of water at the
      // beginning (water stored in evapoTranspirationState plus water that went in in the form of precipitation).  evaporationFromGround and
      // transpirationFromVegetation are not used in this computation because that water is not taken out by Noah-MP.  It is taken below.  The Noah-MP values
      // are in meters of water thickness.  We need to convert this to cubic meters to store in the channel element's surfacewaterError.
      surfacewaterError += ((((double)evapoTranspirationTotalWaterInDomain(&evapoTranspirationState) + (double)surfacewaterAdd +
                              (double)evaporationFromSnow - (double)waterError) -
                             (originalEvapoTranspirationTotalWaterInDomain + (double)evapoTranspirationForcing.prcp * dt)) / 1000.0) * topArea;
    }
  
  if (!error)
    {
      // Move water and record flows for precipitation, evaporation, and transpiration.
      crossSectionArea += (surfacewaterAdd / 1000.0) * topArea / elementLength;
      evaporation       = (evaporationFromSnow / 1000.0) * topArea;
      
      // Take evaporationFromGround from surfacewater.  If there isn't enough surfacewater print a warning and reduce the quantity of evaporation.
      unsatisfiedEvaporation = (evaporationFromGround / 1000.0) * topArea;
      
      if (crossSectionArea >= unsatisfiedEvaporation / elementLength)
        {
          evaporation      += unsatisfiedEvaporation;
          crossSectionArea -= unsatisfiedEvaporation / elementLength;
        }
      else
        {
          unsatisfiedEvaporation -= crossSectionArea * elementLength;
          evaporation            += crossSectionArea * elementLength;
          crossSectionArea        = 0.0;

          if ((2 <= ADHydro::verbosityLevel && 10.0 < unsatisfiedEvaporation) || (3 <= ADHydro::verbosityLevel && 0.0 < unsatisfiedEvaporation))
            {
              CkError("WARNING in ChannelElement::doPointProcessesAndSendOutflows, element %d: unsatisfied evaporation from ground of %le cubic meters.\n",
                      elementNumber, unsatisfiedEvaporation);
            }
        }
      
      // Record cumulative flows and water error.
      precipitationRate                 = -evapoTranspirationForcing.prcp / 1000.0;
      precipitationCumulativeShortTerm += precipitationRate * topArea * dt;
      evaporationRate                   = evaporation / topArea / dt;
      evaporationCumulativeShortTerm   += evaporation;
      surfacewaterError                += (waterError / 1000.0) * topArea;
      
      // If the roundoff error of adding one timestep's water to CumulativeShortTerm is greater than the roundoff error of adding CumulativeShortTerm to
      // CumulativeLongTerm then move CumulativeShortTerm to CumulativeLongTerm.
      // FIXME implement
    }
  
  if (!error && NULL != diversion)
    {
      // Send water for diversions.
      waterAvailable = crossSectionArea * elementLength;
      
      diversion->divert(&waterAvailable, referenceDate, currentTime, timestepEndTime, waterToDivert);
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      CkAssert(0.0 <= waterAvailable && waterAvailable <= crossSectionArea * elementLength);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      for (itDiversion = waterToDivert.begin(); !error && itDiversion != waterToDivert.end(); ++itDiversion)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(0.0 <= (*itDiversion).second);

          // FIXME invariant check to make sure waterToDivert contains all elements of diversionReleaseRecipients.
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          
          ii = 0;
          
          while (ii < diversionReleaseRecipients.size() && diversionReleaseRecipients[ii] != (*itDiversion).first)
            {
              ++ii;
            }
          
          if (ii < diversionReleaseRecipients.size())
            {
              waterSent = (*itDiversion).second;
              // FIXME how to record cumulative flows?
              
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              // FIXME only need to do this to check that all of the waterSent adds up to the reduction in waterAvailable.
              crossSectionArea -= waterSent / elementLength;
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              error = region.sendWater(diversionReleaseRecipientsRegions[ii],
                                       RegionMessage(MESH_SURFACEWATER_CHANNEL_NEIGHBOR, (*itDiversion).first,
                                                     diversionReleaseRecipientsReciprocalNeighborProxies[ii], 0.0, 0.0,
                                                     SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, waterSent)));
            }
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          else
            {
              CkError("ERROR in ChannelElement::doPointProcessesAndSendOutflows, element %d: diversion release to mesh element %d, which is not in its "
                      "recipients list.\n", elementNumber, (*itDiversion).first);
              error = true;
            }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }
      
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      if (!error)
        {
          // Double check that waterAvailable was reduced by the same amount of water as crossSectionArea.
          CkAssert(epsilonEqual(waterAvailable, crossSectionArea * elementLength));
        }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
      
      if (!error)
        {
          // Use the output of divert as the "official" amount of water left.  This prevents crossSectionArea from going negative due to roundoff error and it
          // means we only need to subtract waterSent from crossSectionArea if we are checking the assertion.
          crossSectionArea = waterAvailable / elementLength;
        }
    }
  
  if (!error)
    {
      // Limit surfacewater and groundwater outflows.
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

      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 < (*itUndergroundMesh).nominalFlowRate)
            {
              totalOutwardFlowRate += (*itUndergroundMesh).nominalFlowRate;
            }
        }

      if (crossSectionArea * elementLength < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = crossSectionArea * elementLength / (totalOutwardFlowRate * dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Send surfacewater and groundwater outflows taking water from crossSectionArea.
      for (itMesh = meshNeighbors.begin(); !error && itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 < (*itMesh).nominalFlowRate)
            {
              // Send water for an outflow.
              waterSent                          = (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction;
              crossSectionArea                  -= waterSent / elementLength;
              (*itMesh).flowCumulativeShortTerm += waterSent;

              error = region.sendWater((*itMesh).region, RegionMessage(MESH_SURFACEWATER_CHANNEL_NEIGHBOR, (*itMesh).neighbor,
                                                                       (*itMesh).reciprocalNeighborProxy, 0.0, 0.0,
                                                                       SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                             waterSent)));
            }
        }

      for (itChannel = channelNeighbors.begin(); !error && itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (isBoundary((*itChannel).neighbor) && 0.0 > (*itChannel).nominalFlowRate)
            {
              // Water for an inflow boundary arrives immediately.
              error = (*itChannel).insertMaterial(SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime, -(*itChannel).nominalFlowRate * dt));
            }
          else if (0.0 < (*itChannel).nominalFlowRate ||
                   (NULL != reservoir && !isBoundary((*itChannel).neighbor) && (*itChannel).neighbor == reservoirReleaseRecipient))
            {
              // Send water for an outflow.  We need to send a water message for one-way-flow neighbor relationships even if the flow rate is zero because the
              // recipient doesn't know what the calculated flow rate is.
              waterSent                             = (*itChannel).nominalFlowRate * dt * outwardFlowRateFraction;
              crossSectionArea                     -= waterSent / elementLength;
              (*itChannel).flowCumulativeShortTerm += waterSent;

              if (!isBoundary((*itChannel).neighbor))
                {
                  error = region.sendWater((*itChannel).region, RegionMessage(CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannel).neighbor,
                                                                              (*itChannel).reciprocalNeighborProxy, 0.0, 0.0,
                                                                              SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                                    waterSent)));
                }
            }

// FIXME remove, save a hydrograph for channel outflow boundaries
if (OUTFLOW == (*itChannel).neighbor ||
         (2546 == elementNumber && 2547 == itChannel->neighbor) ||
         (7429 == elementNumber && 7447 == itChannel->neighbor) ||
         (7462 == elementNumber && 7463 == itChannel->neighbor) ||
         (7510 == elementNumber && 7512 == itChannel->neighbor) ||
         (6926 == elementNumber && 6927 == itChannel->neighbor) ||
         (7152 == elementNumber && 7163 == itChannel->neighbor) ||
         (7646 == elementNumber && 7647 == itChannel->neighbor) ||
         (7490 == elementNumber && 7491 == itChannel->neighbor) ||
         (7373 == elementNumber && 7374 == itChannel->neighbor) ||
         (6173 == elementNumber && 6174 == itChannel->neighbor) ||
         (7409 == elementNumber && 7410 == itChannel->neighbor) ||
         (6303 == elementNumber && 6304 == itChannel->neighbor) ||
         (6309 == elementNumber && 6310 == itChannel->neighbor) ||
         (6740 == elementNumber && 6741 == itChannel->neighbor) ||
         (7553 == elementNumber && 7554 == itChannel->neighbor) ||
         (7608 == elementNumber && 7609 == itChannel->neighbor) ||
         (6992 == elementNumber && 6996 == itChannel->neighbor) ||
         (6935 == elementNumber && 6936 == itChannel->neighbor) ||
         (7041 == elementNumber && 7042 == itChannel->neighbor) ||
         (7209 == elementNumber && 7210 == itChannel->neighbor) ||
         (7062 == elementNumber && 7074 == itChannel->neighbor) ||
         (7334 == elementNumber &&  988 == itChannel->neighbor) ||
         (7342 == elementNumber && 7343 == itChannel->neighbor) ||
         (7055 == elementNumber && 7065 == itChannel->neighbor))
{
char numstr[21];
sprintf(numstr, "%d", elementNumber);
hydrographFile = fopen((ADHydro::adhydroOutputStateFilePath + std::string(".") + std::string(numstr) + std::string(".txt")).c_str(), "a");
fprintf(hydrographFile, "%d,\t%lf,\t%lf,\t%lf\n", elementNumber, currentTime, (*itChannel).nominalFlowRate, outwardFlowRateFraction);
fclose(hydrographFile);
}
        }

      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); !error && itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 < (*itUndergroundMesh).nominalFlowRate)
            {
              // Send water for an outflow.
              waterSent                                     = (*itUndergroundMesh).nominalFlowRate * dt * outwardFlowRateFraction;
              crossSectionArea                             -= waterSent / elementLength;
              (*itUndergroundMesh).flowCumulativeShortTerm += waterSent;

              error = region.sendWater((*itUndergroundMesh).region, RegionMessage(MESH_GROUNDWATER_CHANNEL_NEIGHBOR, (*itUndergroundMesh).neighbor,
                                                                                  (*itUndergroundMesh).reciprocalNeighborProxy, 0.0, 0.0,
                                                                                  SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                                        waterSent)));
            }
        }
      
      if (!error)
        {
          // Even though we are limiting outflows, crossSectionArea can go below zero due to roundoff error.
          if (0.0 > crossSectionArea)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(epsilonEqual(0.0, crossSectionArea));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              surfacewaterError -= crossSectionArea * elementLength;
              crossSectionArea   = 0.0;
            }

          // Convert cross sectional area back to water depth.
          calculateSurfacewaterDepthFromArea(crossSectionArea);
        }
    }
  
  return error;
}

bool ChannelElement::allInflowsArrived(double currentTime, double timestepEndTime)
{
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh; // Loop iterator.
  bool allArrived = true;                                                           // Whether all material has arrived from all neighbors.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in ChannelElement::allInflowsArrived: currentTime must be less than or equal to timestepEndTime.\n");
      CkExit();
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call allMaterialHasArrived.  allInflowsArrived shouldn't be called in this situation, but if it is return
  // that all inflows have arrived for the next zero seconds.
  if (currentTime < timestepEndTime)
    {
      for (itMesh = meshNeighbors.begin(); allArrived && itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate || (*itMesh).inflowOnly)
            {
              allArrived = (*itMesh).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
      
      for (itChannel = channelNeighbors.begin(); allArrived && itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate || (*itChannel).inflowOnly)
            {
              allArrived = (*itChannel).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
      
      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); allArrived && itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 > (*itUndergroundMesh).nominalFlowRate || (*itUndergroundMesh).inflowOnly)
            {
              allArrived = (*itUndergroundMesh).allMaterialHasArrived(currentTime, timestepEndTime);
            }
        }
    }
  
  return allArrived;
}

bool ChannelElement::receiveInflows(double currentTime, double timestepEndTime)
{
  bool   error = false;                                                             // Error flag.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh; // Loop iterator.
  double area  = surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth);   // Wetted cross sectional area of channel in square meters.
  
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(currentTime <= timestepEndTime))
    {
      CkError("ERROR in ChannelElement::receiveInflows: currentTime must be less than or equal to timestepEndTime.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  
  // If timestepEndTime is not in the future we can't call getMaterial.  advanceTime shouldn't be called in this situation, but if it is we don't have to do
  // anything to receive inflows for the next zero seconds.
  if (!error && currentTime < timestepEndTime)
    {
      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 > (*itMesh).nominalFlowRate)
            {
              area += (*itMesh).getMaterial(currentTime, timestepEndTime) / elementLength;
            }
        }
      
      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (0.0 > (*itChannel).nominalFlowRate)
            {
              area += (*itChannel).getMaterial(currentTime, timestepEndTime) / elementLength;
            }
        }
      
      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 > (*itUndergroundMesh).nominalFlowRate)
            {
              area += (*itUndergroundMesh).getMaterial(currentTime, timestepEndTime) / elementLength;
            }
        }

      // Convert cross sectional area back to water depth.
      calculateSurfacewaterDepthFromArea(area);

      // If in drainDownMode cut off surface water to channel bank depth.
      if (ADHydro::drainDownMode && surfacewaterDepth > (elementZBank - elementZBed))
        {
          surfacewaterDepth = (elementZBank - elementZBed);
        }
    }
  
  return error;
}

bool ChannelElement::massBalance(double& waterInDomain, double& externalFlows, double& waterError)
{
  bool                                                           error   = false;   // Error flag.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh; // Loop iterator.
  double                                                         topArea = (baseWidth + 2.0 * sideSlope * surfacewaterDepth) * elementLength;
                                                                                    // surface area of the water top surface in square meters.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  if (!(0.0 <= waterInDomain))
    {
      CkError("ERROR in ChannelElement::massBalance: waterInDomain must be greater than or equal to zero.\n");
      error = true;
    }
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)

  if (!error)
    {
      waterInDomain += surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth) * elementLength;
      waterInDomain += (evapoTranspirationTotalWaterInDomain(&evapoTranspirationState) / 1000.0) * topArea; // Divide by one thousand to convert from
                                                                                                            // millimeters to meters.

      for (itMesh = meshNeighbors.begin(); itMesh != meshNeighbors.end(); ++itMesh)
        {
          waterInDomain += (*itMesh).totalWaterInIncomingMaterial();
        }
      
      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (isBoundary((*itChannel).neighbor))
            {
              externalFlows += (*itChannel).flowCumulativeShortTerm + (*itChannel).flowCumulativeLongTerm;
            }
          
          waterInDomain += (*itChannel).totalWaterInIncomingMaterial();
        }

      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          waterInDomain += (*itUndergroundMesh).totalWaterInIncomingMaterial();
        }

      externalFlows += precipitationCumulativeShortTerm + precipitationCumulativeLongTerm;
      externalFlows += evaporationCumulativeShortTerm   + evaporationCumulativeLongTerm;
      
      waterError += surfacewaterError;
    }

  return error;
}

void ChannelElement::calculateSurfacewaterDepthFromArea(double area)
{
  double bOverTwoS; // Temporary value reused in computation.

#if (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)
  CkAssert(0.0 <= area);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PRIVATE_FUNCTIONS_SIMPLE)

  if (0.0 < sideSlope)
    {
      // Trapezoidal or triangular channel.
      bOverTwoS = baseWidth / (2.0 * sideSlope);

      surfacewaterDepth = sqrt(area / sideSlope + bOverTwoS * bOverTwoS) - bOverTwoS;
      
      // For very small areas I wonder if surfacewaterDepth might be able to come out negative due to roundoff error.  I wasn't able to convince myself that it
      // couldn't possibly happen so I'm leaving this in.  I don't add anything to surfacewaterError because I already know that area wasn't negative so it's
      // not really missing water.
      if (0.0 > surfacewaterDepth)
        {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(epsilonEqual(0.0, surfacewaterDepth));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

          surfacewaterDepth = 0.0;
        }
    }
  else
    {
      // Rectangular channel.  By invariant baseWidth and sideSlope can't both be zero.
      surfacewaterDepth = area / baseWidth;
    }
}
