#include "RegisterParserPlugins.h"

#include "BranchCompParser.h"
#include "BusCompParser.h"
#include "DgyTransformerParser.h"
#include "NetworkCompParser.h"
#include "OverheadLineParser.h"
#include "SimpleBatteryParser.h"
#include "SimpleBuildingParser.h"
#include "SimpleDcPowerSourceParser.h"
#include "SimpleInverterParser.h"
#include "SinglePhaseTransformerParser.h"
#include "SolarPvParser.h"
#include "WeatherParser.h"
#include "YyTransformerParser.h"
#include "ZipCompParser.h"

namespace SmartGridToolbox
{
   void registerParserPlugins(Parser& p)
   {
      p.registerParserPlugin<BranchCompParser>();
      p.registerParserPlugin<BusCompParser>();
      p.registerParserPlugin<DgyTransformerParser>();
      p.registerParserPlugin<NetworkCompParser>();
      p.registerParserPlugin<OverheadLineParser>();
      p.registerParserPlugin<SimpleBatteryParser>();
      p.registerParserPlugin<SimpleBuildingParser>();
      p.registerParserPlugin<SimpleDcPowerSourceParser>();
      p.registerParserPlugin<SimpleInverterParser>();
      p.registerParserPlugin<SinglePhaseTransformerParser>();
      p.registerParserPlugin<SolarPvParser>();
      p.registerParserPlugin<WeatherParser>();
      p.registerParserPlugin<YyTransformerParser>();
      p.registerParserPlugin<ZipCompParser>();
   }
}