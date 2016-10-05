#ifndef __OUTPUT_MANAGER_CHARM_H__
#define __OUTPUT_MANAGER_CHARM_H__

#include "file_manager_NetCDF.h"
#include "all.h"

// These classes really belong as nested classes of OutputManagerCharm, but there were problems with circular dependencies and being unable to forward declare nested classes.

// This is a wrapper to provide a pup routine.
class MeshElementStateCharm : public MeshElementState
{
public:

  // Constructor.  Forwarded to base class constructor.  No argument constructor needed for pupping.
  MeshElementStateCharm(size_t elementNumberInit = 0, double currentTimeInit = 0.0, size_t numberOfSoilLayersInit = 0, size_t numberOfNeighborsInit = 0) :
    MeshElementState(elementNumberInit, currentTimeInit, numberOfSoilLayersInit, numberOfNeighborsInit) {}

  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
};

// This is a wrapper to provide a pup routine.
class ChannelElementStateCharm : public ChannelElementState
{
public:

  // Constructor.  Forwarded to base class constructor.  No argument constructor needed for pupping.
  ChannelElementStateCharm(size_t elementNumberInit = 0, double currentTimeInit = 0.0, size_t numberOfNeighborsInit = 0) :
    ChannelElementState(elementNumberInit, currentTimeInit, numberOfNeighborsInit) {}

  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
};

#include "output_manager_charm.decl.h"

// Implementation of OutputManager for Charm++.
class OutputManagerCharm : public CBase_OutputManagerCharm, public OutputManager
{
  OutputManagerCharm_SDAG_CODE

public:

  // Constructor.
  //
  // Parameters:
  //
  // fileManagerInit - The subclass of FileManager used determines what type of files are created.
  //                   This is an enum rather than a pointer to an object so that it can be easily sent in a Charm++ message.
  //                   The actual object that gets passed to OutputManager is one of the private member variables below.
  OutputManagerCharm(FileManagerEnum fileManagerInit) : OutputManager(convertFileManagerEnumToFileManagerReference(fileManagerInit)), fileManagerNetCDFInit(*this) { thisProxy[CkMyPe()].runForever(); }

  // OutputManager communication system interface.
  size_t      numberOfOutputManagers();
  size_t      myOutputManagerIndex();
  std::string directory();
  double      referenceDate();
  double      simulationStartTime();
  double      simulationDuration();
  double      outputPeriod();
  size_t      globalNumberOfMeshElements();
  size_t      localNumberOfMeshElements();
  size_t      localMeshElementStart();
  size_t      maximumNumberOfMeshSoilLayers();
  size_t      maximumNumberOfMeshNeighbors();
  size_t      globalNumberOfChannelElements();
  size_t      localNumberOfChannelElements();
  size_t      localChannelElementStart();
  size_t      maximumNumberOfChannelNeighbors();

private:

  // Helper function to error check a FileManagerEnum.
  //
  // Returns: a reference to the correct private member variable for the enum value.
  FileManager& convertFileManagerEnumToFileManagerReference(FileManagerEnum fileManagerInit);

  // FileManager objects to pass to OutputManager.
  FileManagerNetCDF fileManagerNetCDFInit;
};

#endif // __OUTPUT_MANAGER_CHARM_H__
