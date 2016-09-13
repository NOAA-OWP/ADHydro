#ifndef __OUTPUT_MANAGER_H__
#define __OUTPUT_MANAGER_H__

#include "file_manager.h"

// OutputManager contains the generic code for creating ADHydro output files.
// It outputs a separate file for each time point.
// The purpose of this is to make the early time points available for use as soon as possible.
// What type of files are created is wrapped by the FileManager class.
// OutputManager is generic with respect to what communication system is used.
// To support this it has a pure virtual interface for interaction with the communication system.
// Each communication system must subclass OutputManager with a system specific implementation.
// The most likely two candidates are Charm++ or MPI.
// There should be one OutputManager on each processor, i.e. a group in Charm++, or one per rank in MPI.
class OutputManager
{
public:

  // Constructor.
  //
  // Parameters:
  //
  // fileManagerInit - Scalar passed by reference.  The subclass of FileManager used determines what type of files are created.
  //                   The FileManager pointed to must exist for the entire lifetime of the OutputManager,
  //                   but the OutputManager destructor does not delete the FileManager.
  OutputManager(FileManager* fileManagerInit);

  // OutputManager starts by creating all of the files that it will write over the course of a simulation.
  // We do it this way for performance reasons for NetCDF files.
  // We do not call createFiles in the constructor for two reasons.
  // First, it calls the pure virtual interface, and the virtual dispatch tables won't be set up until the subclass constructor runs.
  // Second, it requires knowing referenceDate and simulationStartTime, which might need to be loaded from input files after construction time.
  // You must call createFiles on all OutputManagers and wait until they all return before proceeding with any other OutputManager operations.
  void createFiles();

private:

  // Pure virtual interface to the communication system.

  // Wrappers for communication topology.  Return values must not change over the course of a single run.
  // Indices must be unique and cover all indices from zero to numberOfOutputManagers minus one.
  virtual int numberOfOutputManagers() = 0; // Must be greater than zero.
  virtual int myOutputManagerIndex()   = 0; // Must be greater than or equal to zero and less than numberOfOutputManagers.

  // Wrappers for readonly variables.  Return values must not change over the course of a single run.
  virtual double referenceDate()       = 0; // Julian date.  Must be on or after 1 CE (1721425.5).
  virtual double simulationStartTime() = 0; // Seconds after referenceDate.  Can be positive, negative, or zero, but the calendar date must be on or after 1 CE.
  virtual double simulationDuration()  = 0; // Seconds.  Must be greater than or equal to zero.
  virtual double outputPeriod()        = 0; // Seconds.  Must be greater than zero.

  FileManager* fileManager; // Wrapper for what type of files are created.
};

#endif // __OUTPUT_MANAGER_H__
