#include "RegisterComponentParsers.h"
#include "Branch.h"
#include "Bus.h"
#include "Network.h"
#include "TestComponent.h"
#include "ZipToGround.h"

namespace SmartGridToolbox
{
   void registerComponentParsers(Parser & p)
   {
      p.registerComponentParser<BranchParser>();
      p.registerComponentParser<BusParser>();
      p.registerComponentParser<NetworkParser>();
      p.registerComponentParser<TestComponentParser>();
      p.registerComponentParser<ZipToGroundParser>();
   }
}