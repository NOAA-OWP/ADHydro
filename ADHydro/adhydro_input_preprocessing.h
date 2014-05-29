#ifndef __ADHYDRO_INPUT_PREPROCESSING_H__
#define __ADHYDRO_INPUT_PREPROCESSING_H__

#include "file_manager.h"
#include "adhydro_input_preprocessing.decl.h"

// FIXME document
// Convert triangle files and other files into NetCDF files.
class ADHydroInputPreprocessing : public CBase_ADHydroInputPreprocessing
{
public:
  
  // Constructor.  This is the mainchare constructor where the program starts.
  // Exit on error.
  //
  // Parameters:
  //
  // msg - Charm++ command line arguments message.
  ADHydroInputPreprocessing(CkArgMsg* msg);
  
  // Reduction target for barrier after FileManager opens all files.
  void filesOpened();
  
  // Reduction target for barrier after FileManager closes all files.
  void filesClosed();
  
private:
  
  // Helper function to close all open member variable FILE pointers.
  //
  // Returns: true if there is an error, false otherwise.
  bool closeFiles();
  
  FILE*              nodeFile;           // For opening the .node  file.
  FILE*              zFile;              // For opening the .z     file.
  FILE*              elementFile;        // For opening the .ele   file.
  FILE*              neighborFile;       // For opening the .neigh file.
  CProxy_FileManager fileManagerProxy;   // For creating the NetCDF files.
  int                numberOfAttributes; // For reading the number of attributes from the .node and .ele files.
};

#endif // __ADHYDRO_INPUT_PREPROCESSING_H__
