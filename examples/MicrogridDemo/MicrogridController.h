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

#ifndef MICROGRID_DEMO_CONTROLLER_DOT_H
#define MICROGRID_DEMO_CONTROLLER_DOT_H

#include <SgtCore/Parser.h>

#include <SgtSim/Battery.h>
#include <SgtSim/SimBus.h>
#include <SgtSim/SimParser.h>
#include <SgtSim/Simulation.h>

namespace Sgt
{
    class MicrogridController : public SimComponent
    {
        public:
            MicrogridController(const std::string& id) : Component(id) {}

            void setMinP(double P) {minP_ = P;}
            void setMaxP(double P) {maxP_ = P;}

            void setBuildBus(std::shared_ptr<SimBusAbc> bus);
            void setPvBus(std::shared_ptr<SimBusAbc> bus);
            void setBatt(std::shared_ptr<Battery> batt);

        protected:
            virtual void updateState(Time t) override;

        private:

        private:
            std::shared_ptr<SimBusAbc> buildBus_;
            std::shared_ptr<SimBusAbc> pvBus_;
            std::shared_ptr<Battery> batt_;

            double minP_;
            double maxP_;
    };

    class MicrogridControllerParserPlugin : public SimParserPlugin
    {
        public:
            virtual const char* key() override
            {
                return "microgrid_controller";
            }

        public:
            virtual void parse(const YAML::Node& nd, Simulation& sim, const ParserBase& parser) const override;
    };
};

#endif // PV_DEMO_CONTROLLER_DOT_H