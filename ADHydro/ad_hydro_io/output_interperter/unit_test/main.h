#ifndef __MAIN_H__
#define __MAIN_H__
#include "OutputManager.h"
#include "main.decl.h"
class Main : public CBase_Main
{

public:
    Main(CkArgMsg* msg);
    Main(CkMigrateMessage* msg);

    static CProxy_OutputManager outputManager;
    static CProxy_Region	regionProxy;
    static CProxy_Main		mainProxy;
    //Entry method
    int doneCount;
    void done(int id);
};

#endif //__MAIN_H__

