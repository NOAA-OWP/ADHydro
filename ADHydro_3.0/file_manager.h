#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

// FileManager is a wrapper for file operations used by OutputManager.
// FileManager must be subclassed for each type of file that can be created.
// Currently, only NetCDF files are implemented, but this could be extended.
class FileManager
{
public:

  // The state of the evapotranspiration and vadose zone infiltration simulations are generally complex data structures.
  // We want ADHydro to support using third party modules for those simulations so we don't necessarily have control over the data structure,
  // and the data structure can change if you change to a different module, but we need to save that state in the input/output files as a fixed size blob.
  // These declarations provide fixed size blobs for that data.  Any new simulation module must shoehorn their state into a fixed size blob and add it as an additional union option.
  union EvapoTranspirationStateBlob
  {
    int dummy;
    // FIXME EvapoTranspirationStateStruct NoahMPStateBlob;
  };

  union VadoseZoneStateBlob
  {
    int dummy;
    // FIXME GARTOStateBlob
  };

  // Pure virtual interface to file operations.

  // Create a new empty output file.  This may include setting up file meta-data.
  //
  // OutputManager starts by creating all of the files that it will write over the course of a simulation.
  // Each file is created serially. One OutputManager calls this method on a single processor.
  // All of the files are created at the same time with separate OutputManagers creating separate files to distribute the workload.
  //
  // We do it this way because for NetCDF files we use the serial library to create files and the parallel library to write data into existing files.
  // Parallel file creation requires collective operations so that all OutputManagers agree on the meta-data.
  // Collective operations require all OutputManagers to synchronize at a barrier, which we have found to be inefficient.
  // However, parallel data writes into existing files can be done with independent operations that do not require a barrier.
  // Therefore, the optimal configuration that we have found is multiple simultaneous serial file creation followed by parallel data writes into existing files.
  //
  // Returns: true if there is an error, false otherwise.
  //
  // Parameters:
  //
  // outputTime - Time point to create a file for.  This value can be used for the file name and/or stored inside the file.
  virtual bool createFile(double outputTime) = 0;
};

#endif // __FILE_MANAGER_H__
