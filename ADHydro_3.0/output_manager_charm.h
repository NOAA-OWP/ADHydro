#ifndef __OUTPUT_MANAGER_CHARM_H__
#define __OUTPUT_MANAGER_CHARM_H__

#include "all.h"
#include "output_manager_charm.decl.h"
#include "file_manager_NetCDF.h"

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
  OutputManagerCharm(FileManagerEnum fileManagerInit);

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
