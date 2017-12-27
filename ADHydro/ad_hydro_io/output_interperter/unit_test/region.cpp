#include "region.h"

Region::Region()
{
  meshElements[0] = Element();
  meshElements[1] = Element();
  channelElements[0] = Element();
  channelElements[1] = Element();
  Main::outputManager.ckLocalBranch()->registerRegion(*this);
}

void Region::doSomething()
{
  CkPrintf("Region %d is doing something\n", thisIndex);
  migrateMe((CkMyPe()+1)%CkNumPes());
  Main::mainProxy.done(thisIndex);
}

void Region::ckAboutToMigrate()
{
  CkPrintf("Region %d is about to migrate\n", thisIndex);
  Main::outputManager.ckLocalBranch()->unregisterRegion(*this);
}

void Region::ckJustMigrated()
{
  CkPrintf("Region %d has finished migrating\n", thisIndex);
  Main::outputManager.ckLocalBranch()->registerRegion(*this);
}
Region::Region(CkMigrateMessage* msg)
{
  // Initialization handled by initialization list.
}

void Region::pup(PUP::er &p)
{
  p | numberOfMeshElements;
  p | meshElements;
  p | numberOfChannelElements;
  p | channelElements;

}
#include "region.def.h"