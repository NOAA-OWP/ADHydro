#include "channel_element.h"
#include "adhydro.h"
#include "surfacewater.h"
#include "groundwater.h"

ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy() :
  SimpleNeighborProxy(0.0, 0.0, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
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

ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                                                           double flowCumulativeShortTermInit, double flowCumulativeLongTermInit,
                                                                           int regionInit, int neighborInit, int reciprocalNeighborProxyInit,
                                                                           double neighborZSurfaceInit, double neighborZOffsetInit, double neighborAreaInit,
                                                                           double edgeLengthInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
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
  // FIXME check that regionInit and neighborInit are less than the number of regions and elements.  I think there will be globals with those sizes.
  if (!(0 <= regionInit))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy: regionInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0 <= neighborInit))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::ChannelSurfacewaterMeshNeighborProxy: neighborInit must be greater than or equal to zero.\n");
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
  
  // FIXME check that region and neighbor are less than the number of regions and elements.  I think there will be globals with those sizes.
  if (!(0 <= region))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::checkInvariant: region must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= neighbor))
    {
      CkError("ERROR in ChannelSurfacewaterMeshNeighborProxy::checkInvariant: neighbor must be greater than or equal to zero.\n");
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
  SimpleNeighborProxy(0.0, 0.0, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
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

ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                                                                 double flowCumulativeShortTermInit, double flowCumulativeLongTermInit,
                                                                                 int regionInit, int neighborInit, int reciprocalNeighborProxyInit,
                                                                                 ChannelTypeEnum neighborChannelTypeInit, double neighborZBankInit,
                                                                                 double neighborZBedInit, double neighborLengthInit,
                                                                                 double neighborBaseWidthInit, double neighborSideSlopeInit,
                                                                                 double neighborManningsNInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
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
  // FIXME check that regionInit and neighborInit are less than the number of regions and elements.  I think there will be globals with those sizes.
  if (!(0 <= regionInit))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: regionInit must be greater than or equal to "
              "zero.\n");
      CkExit();
    }
  
  if (!(isBoundary(neighborInit) || 0 <= neighborInit))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::ChannelSurfacewaterChannelNeighborProxy: neighborInit must be a boundary condition code or "
              "greater than or equal to zero.\n");
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
  
  // FIXME check that region and neighbor are less than the number of regions and elements.  I think there will be globals with those sizes.
  if (!(0 <= region))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: region must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(isBoundary(neighbor) || 0 <= neighbor))
    {
      CkError("ERROR in ChannelSurfacewaterChannelNeighborProxy::checkInvariant: neighbor must be a boundary condition code or greater than or equal to "
              "zero.\n");
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
  SimpleNeighborProxy(0.0, 0.0, 0.0, 0.0), // Dummy values will be overwritten by pup_stl.h code.
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

ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy(double expirationTimeInit, double nominalFlowRateInit,
                                                                         double flowCumulativeShortTermInit, double flowCumulativeLongTermInit, int regionInit,
                                                                         int neighborInit, int reciprocalNeighborProxyInit, double neighborZSurfaceInit,
                                                                         double neighborLayerZBottomInit, double neighborZOffsetInit, double edgeLengthInit) :
  SimpleNeighborProxy(expirationTimeInit, nominalFlowRateInit, flowCumulativeShortTermInit, flowCumulativeLongTermInit),
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
  // FIXME check that regionInit and neighborInit are less than the number of regions and elements.  I think there will be globals with those sizes.
  if (!(0 <= regionInit))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy: regionInit must be greater than or equal to zero.\n");
      CkExit();
    }
  
  if (!(0 <= neighborInit))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::ChannelGroundwaterMeshNeighborProxy: neighborInit must be greater than or equal to zero.\n");
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
  
  // FIXME check that region and neighbor are less than the number of regions and elements.  I think there will be globals with those sizes.
  if (!(0 <= region))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::checkInvariant: region must be greater than or equal to zero.\n");
      error = true;
    }
  
  if (!(0 <= neighbor))
    {
      CkError("ERROR in ChannelGroundwaterMeshNeighborProxy::checkInvariant: neighbor must be greater than or equal to zero.\n");
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
  baseWidth(0.0),
  sideSlope(0.0),
  bedConductivity(0.0),
  bedThickness(0.0),
  manningsN(0.0),
  surfacewaterDepth(0.0),
  surfacewaterError(0.0),
  meshNeighbors(),
  channelNeighbors(),
  undergroundMeshNeighbors()
{
  // Initialization handled by initialization list.
}

ChannelElement::ChannelElement(int elementNumberInit, ChannelTypeEnum channelTypeInit, long long reachCodeInit, double elementXInit, double elementYInit,
                               double elementZBankInit, double elementZBedInit, double elementLengthInit, double baseWidthInit, double sideSlopeInit,
                               double bedConductivityInit, double bedThicknessInit, double manningsNInit, double surfacewaterDepthInit,
                               double surfacewaterErrorInit) :
  elementNumber(elementNumberInit),
  channelType(channelTypeInit),
  reachCode(reachCodeInit),
  elementX(elementXInit),
  elementY(elementYInit),
  elementZBank(elementZBankInit),
  elementZBed(elementZBedInit),
  elementLength(elementLengthInit),
  baseWidth(baseWidthInit),
  sideSlope(sideSlopeInit),
  bedConductivity(bedConductivityInit),
  bedThickness(bedThicknessInit),
  manningsN(manningsNInit),
  surfacewaterDepth(surfacewaterDepthInit),
  surfacewaterError(surfacewaterErrorInit),
  meshNeighbors(),
  channelNeighbors(),
  undergroundMeshNeighbors()
{
#if (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
  // FIXME check that elementNumberInit is less than the number of elements.  I think there will be globals with this size.
  if (!(0 <= elementNumberInit))
    {
      CkError("ERROR in ChannelElement::ChannelElement: elementNumberInit must be greater than or equal to zero.\n");
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
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_PUBLIC_FUNCTIONS_SIMPLE)
}

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
  p | baseWidth;
  p | sideSlope;
  p | bedConductivity;
  p | bedThickness;
  p | manningsN;
  p | surfacewaterDepth;
  p | surfacewaterError;
  p | meshNeighbors;
  p | channelNeighbors;
  p | undergroundMeshNeighbors;
}

bool ChannelElement::checkInvariant()
{
  bool                                                           error = false;     // Error flag.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh; // Loop iterator.
  
  // FIXME check that elementNumberInit is less than the number of elements.  I think there will be globals with this size.
  if (!(0 <= elementNumber))
    {
      CkError("ERROR in ChannelElement::checkInvariant: elementNumber must be greater than or equal to zero.\n");
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

bool ChannelElement::doPointProcessesAndSendOutflows(double referenceDate, double currentTime, double timestepEndTime, Region& region)
{
  bool   error                   = false;                                                           // Error flag.
  std::vector<ChannelSurfacewaterMeshNeighborProxy>::iterator    itMesh;                            // Loop iterator.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;                         // Loop iterator.
  std::vector<ChannelGroundwaterMeshNeighborProxy>::iterator     itUndergroundMesh;                 // Loop iterator.
  double dt                      = timestepEndTime - currentTime;                                   // Seconds.
  double totalOutwardFlowRate    = 0.0;                                                             // Sum of all outward flow rates in cubic meters per
                                                                                                    // second.
  double outwardFlowRateFraction = 1.0;                                                             // Fraction of all outward flow rates that can be
                                                                                                    // satisfied, unitless.
  double area                    = surfacewaterDepth * (baseWidth + sideSlope * surfacewaterDepth); // Wetted cross sectional area of channel in square meters.
  double waterSent;                                                                                 // Cubic meters.
  
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
  
  // FIXME Do point processes for rainfall, snowmelt, and evapo-transpiration without double counting area.
  
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

      if (area * elementLength < totalOutwardFlowRate * dt)
        {
          outwardFlowRateFraction = area * elementLength / (totalOutwardFlowRate * dt);

#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
          CkAssert(0.0 <= outwardFlowRateFraction && 1.0 >= outwardFlowRateFraction);
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
        }

      // Send surfacewater and groundwater outflows taking water from area.
      for (itMesh = meshNeighbors.begin(); !error && itMesh != meshNeighbors.end(); ++itMesh)
        {
          if (0.0 < (*itMesh).nominalFlowRate)
            {
              // Send water for an outflow.
              waterSent                          = (*itMesh).nominalFlowRate * dt * outwardFlowRateFraction;
              area                              -= waterSent / elementLength;
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
          else if (0.0 < (*itChannel).nominalFlowRate)
            {
              // Send water for an outflow.
              waterSent                             = (*itChannel).nominalFlowRate * dt * outwardFlowRateFraction;
              area                                 -= waterSent / elementLength;
              (*itChannel).flowCumulativeShortTerm += waterSent;

              if (!isBoundary((*itChannel).neighbor))
                {
                  error = region.sendWater((*itChannel).region, RegionMessage(CHANNEL_SURFACEWATER_CHANNEL_NEIGHBOR, (*itChannel).neighbor,
                                                                              (*itChannel).reciprocalNeighborProxy, 0.0, 0.0,
                                                                              SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                                    waterSent)));
                }
            }
        }

      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); !error && itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 < (*itUndergroundMesh).nominalFlowRate)
            {
              // Send water for an outflow.
              waterSent                                     = (*itUndergroundMesh).nominalFlowRate * dt * outwardFlowRateFraction;
              area                                         -= waterSent / elementLength;
              (*itUndergroundMesh).flowCumulativeShortTerm += waterSent;

              error = region.sendWater((*itUndergroundMesh).region, RegionMessage(MESH_GROUNDWATER_CHANNEL_NEIGHBOR, (*itUndergroundMesh).neighbor,
                                                                                  (*itUndergroundMesh).reciprocalNeighborProxy, 0.0, 0.0,
                                                                                  SimpleNeighborProxy::MaterialTransfer(currentTime, timestepEndTime,
                                                                                                                        waterSent)));
            }
        }
      
      if (!error)
        {
          // Even though we are limiting outflows, area can go below zero due to roundoff error.
          if (0.0 > area)
            {
#if (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)
              CkAssert(epsilonEqual(0.0, area));
#endif // (DEBUG_LEVEL & DEBUG_LEVEL_INTERNAL_SIMPLE)

              surfacewaterError -= area * elementLength;
              area               = 0.0;
            }

          // Convert cross sectional area back to water depth.
          calculateSurfacewaterDepthFromArea(area);
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
      
      for (itUndergroundMesh = undergroundMeshNeighbors.begin(); allArrived && itUndergroundMesh != undergroundMeshNeighbors.end(); ++itUndergroundMesh)
        {
          if (0.0 > (*itUndergroundMesh).nominalFlowRate)
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
  bool                                                           error = false; // Error flag.
  std::vector<ChannelSurfacewaterChannelNeighborProxy>::iterator itChannel;     // Loop iterator.

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

      for (itChannel = channelNeighbors.begin(); itChannel != channelNeighbors.end(); ++itChannel)
        {
          if (isBoundary((*itChannel).neighbor))
            {
              externalFlows += (*itChannel).flowCumulativeShortTerm + (*itChannel).flowCumulativeLongTerm;
            }
        }

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
