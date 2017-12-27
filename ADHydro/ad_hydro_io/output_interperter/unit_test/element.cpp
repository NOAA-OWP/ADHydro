#include "element.h"
Element::Element():data1(1),data2(2)
{
  
}
Element::~Element(){}
void Element::pup(PUP::er &p)
{
  p|data1;
  p|data2;
}
