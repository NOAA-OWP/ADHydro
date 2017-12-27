#ifndef __ELEMENT_H__
#define __ELEMENT_H__
#include "pup.h"
class Element
{
private:

public:
    double data1;
  double data2;
    Element();
    ~Element();
    void pup(PUP::er &p);
};

#endif //__Element_H__

