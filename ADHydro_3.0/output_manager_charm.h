#ifndef __OUTPUT_MANAGER_CHARM_H__
#define __OUTPUT_MANAGER_CHARM_H__

#include "file_manager_NetCDF.h"
#include "all.h"

// This is a wrapper to provide a pup routine.  It should be a nested class of OutputManagerCharm, but it can't be because we have to forward declare it before including output_manager_charm.decl.h.
class MeshElementState : public OutputManager::MeshElementState
{
public:

  // Default constructor.  Only needed for pupping.
  MeshElementState() : OutputManager::MeshElementState(0, 0.0, 0, 0) {}

  // Constructor.  Forwarded to base class constructor.
  MeshElementState(size_t elementNumberInit, double currentTimeInit, size_t numberOfSoilLayersInit, size_t numberOfNeighborsInit) :
    OutputManager::MeshElementState(elementNumberInit, currentTimeInit, numberOfSoilLayersInit, numberOfNeighborsInit) {}
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
};

// This is a wrapper to provide a pup routine.  It should be a nested class of OutputManagerCharm, but it can't be because we have to forward declare it before including output_manager_charm.decl.h.
class ChannelElementState : public OutputManager::ChannelElementState
{
public:

  // Default constructor.  Only needed for pupping.
  ChannelElementState() : OutputManager::ChannelElementState(0, 0.0, 0) {}
  
  // Constructor.  Forwarded to base class constructor.
  ChannelElementState(size_t elementNumberInit, double currentTimeInit, size_t numberOfNeighborsInit) :
    OutputManager::ChannelElementState(elementNumberInit, currentTimeInit, numberOfNeighborsInit) {}

  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
};

#include "output_manager_charm.decl.h"

// Implementation of OutputManager for Charm++.
class OutputManagerCharm : public OutputManager, public CBase_OutputManagerCharm
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
