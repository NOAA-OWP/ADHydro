#ifndef __ADHYDRO_H__
#define __ADHYDRO_H__

#include "all.h"
#include "adhydro.decl.h"
#include "output_manager_charm.decl.h"

// An ADHydro object is the main chare of the program.  Execution starts in its constructor.
class ADHydro : public CBase_ADHydro
{
  ADHydro_SDAG_CODE

public:

  // Global readonly variables.  For usage see comments in the example superfile.
  static double                    referenceDate;       // Julian date.
  static double                    simulationStartTime; // Seconds.
  static double                    simulationDuration;  // Seconds.
  static double                    outputPeriod;        // Seconds.
  static int                       verbosityLevel;      // Flag.
  static CProxy_OutputManagerCharm outputManagerProxy;  // Charm group proxy.

  // Constructor.  This is the mainchare constructor where the program starts.
  //
  // Parameters:
  //
  // msg - Charm++ command line arguments message.
  ADHydro(CkArgMsg* msg);
};

#endif // __ADHYDRO_H__
