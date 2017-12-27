#ifndef __REGION_H__
#define __REGION_H__
#include "region.decl.h"
#include "element.h"
#include "main.h"
class Region : public CBase_Region
{
  
public:
  

  Region();
  //Region(){}
  void doSomething();
  void ckAboutToMigrate();
  void ckJustMigrated();
  // Charm++ migration constructor.
  //
  // Parameters:
  //
  // msg - Charm++ migration message.
  Region(CkMigrateMessage* msg);
  
  // Charm++ pack/unpack method.
  //
  // Parameters:
  //
  // p - Pack/unpack processing object.
  void pup(PUP::er &p);
  
  
  // Elements.
  std::map<int, Element>::size_type    numberOfMeshElements;    // During initialization the region waits until it has received this many mesh elements.
  std::map<int, Element>               meshElements;            // A container for the mesh elements.  Map key is element ID number.
  std::map<int, Element>::size_type numberOfChannelElements; // During initialization the region waits until it has received this many channel elements.
  std::map<int, Element>            channelElements;         // A container for the channel elements.  Map key is element ID number.
  
  
};

#endif // __REGION_H__
