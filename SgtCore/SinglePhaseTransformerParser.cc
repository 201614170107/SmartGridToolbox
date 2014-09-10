#include "SinglePhaseTransformerParser.h"

#include "SinglePhaseTransformer.h"
#include "Network.h"

namespace SmartGridToolbox
{
   void SinglePhaseTransformerParser::parse(const YAML::Node& nd, Network& netw) const
   {
      auto trans = parseSinglePhaseTransformer(nd);

      assertFieldPresent(nd, "bus_0_id");
      assertFieldPresent(nd, "bus_1_id");

      std::string bus0Id = nd["bus_0_id"].as<std::string>();
      std::string bus1Id = nd["bus_1_id"].as<std::string>();

      netw.addArc(std::move(trans), bus0Id, bus1Id);
   }

   std::unique_ptr<SinglePhaseTransformer> SinglePhaseTransformerParser::parseSinglePhaseTransformer(
         const YAML::Node& nd) const
   {
      assertFieldPresent(nd, "id");
      assertFieldPresent(nd, "complex_turns_ratio_01");
      assertFieldPresent(nd, "leakage_impedance");

      const std::string id = nd["id"].as<std::string>();
      Complex a = nd["complex_turns_ratio_01"].as<Complex>();
      Complex ZL = nd["leakage_impedance"].as<Complex>();

      std::unique_ptr<SinglePhaseTransformer> trans(new SinglePhaseTransformer(id, a, ZL));

      return trans;
   }
}
