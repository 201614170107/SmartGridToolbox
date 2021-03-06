// Copyright 2015 National ICT Australia Limited (NICTA)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "TapChangerParserPlugin.h"

#include "Inverter.h"
#include "SimNetwork.h"
#include "Simulation.h"
#include "TapChanger.h"
#include "Weather.h"

namespace Sgt
{
    void TapChangerParserPlugin::parse(const YAML::Node& nd, Simulation& sim, const ParserBase& parser) const
    {
        assertFieldPresent(nd, "id");
        assertFieldPresent(nd, "sim_network_id");
        assertFieldPresent(nd, "transformer_id");

        assertFieldPresent(nd, "taps");
        assertFieldPresent(nd, "setpoint");
        assertFieldPresent(nd, "tolerance");
        assertFieldPresent(nd, "ctrl_side_idx");
        assertFieldPresent(nd, "winding_idx");
        assertFieldPresent(nd, "ratio_idx");

        std::string id = parser.expand<std::string>(nd["id"]);
        std::string simNetworkId = parser.expand<std::string>(nd["sim_network_id"]);
        std::string transId = parser.expand<std::string>(nd["transformer_id"]);

        std::vector<double> taps = parser.expand<std::vector<double>>(nd["taps"]);
        double setpoint = parser.expand<double>(nd["setpoint"]);
        double tolerance = parser.expand<double>(nd["tolerance"]);
        arma::uword ctrlSideIdx = parser.expand<arma::uword>(nd["ctrl_side_idx"]);
        arma::uword windingIdx = parser.expand<arma::uword>(nd["winding_idx"]);
        arma::uword ratioIdx = parser.expand<arma::uword>(nd["ratio_idx"]);

        bool hasLdc = false;
        Complex ZLdc = 0;
        Complex topFactorLdc = 1.0;
        auto ndZLdc = nd["ldc_impedance"];
        if (ndZLdc)
        {
            hasLdc = true;
            ZLdc = parser.expand<Complex>(ndZLdc);
            auto ndTopFactorLdc = nd["ldc_top_factor"];
            if (ndTopFactorLdc)
            {
                topFactorLdc = parser.expand<Complex>(ndTopFactorLdc);
            }
        }

        ConstSimComponentPtr<SimNetwork> simNetwork = sim.simComponent<SimNetwork>(simNetworkId);
        sgtAssert(simNetwork != nullptr, std::string(key()) + ": sim_network_id = " + simNetworkId + " was not found.");

        auto trans = simNetwork->network().branches()[transId].as<TransformerAbc, true>();
        sgtAssert(trans != nullptr, std::string(key()) + ": transformer_id = " + transId + " was not found.");

        sim.newSimComponent<TapChanger>(id, trans, taps, setpoint, tolerance, ctrlSideIdx, windingIdx, ratioIdx,
                hasLdc, ZLdc, topFactorLdc);
    }
}
