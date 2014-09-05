#include "SimNetworkParser.h"

#include "SimNetwork.h"
#include "Simulation.h"

namespace SmartGridToolbox
{
   void SimNetworkParser::parse(const YAML::Node& nd, Simulation& into) const
   {
      SGT_DEBUG(debug() << "SimNetwork : parse." << std::endl);

      assertFieldPresent(nd, "id");
      assertFieldPresent(nd, "P_base");

      string id = nd["id"].as<std::string>();
      double PBase = nd["P_base"].as<double>();

      auto ndFreq = nd["freq_Hz"];

      std::unique_ptr<Network> nw = std::unique_ptr<Network>(new Network(id, PBase));
      if (ndFreq)
      {
         nw->setFreq(ndFreq.as<double>());
      }
      into.newSimComponent<SimNetwork>(std::move(nw));
   }
}
