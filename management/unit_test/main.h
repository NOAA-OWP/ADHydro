#ifndef __MAIN_H__
#define __MAIN_H__
#include "element.h"
class Main : public CBase_Main
{
private:
    int doneCount;
    int doneCount1;
    CProxy_Element elementArray;
public:
    Main(CkArgMsg* msg);
    Main(CkMigrateMessage* msg);

    //Entry method
    void done(int id);
};

#endif //__MAIN_H__

