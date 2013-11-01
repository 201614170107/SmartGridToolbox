#include <SmartGridToolbox/Common.h>
#include <SmartGridToolbox/Model.h>
#include <SmartGridToolbox/Parser.h>
#include "RegisterParserPlugins.h"
#include <SmartGridToolbox/Simulation.h>
#include <string>
#include <boost/algorithm/string.hpp>

namespace YAML
{
   using namespace SmartGridToolbox;

   Node convert<Complex>::encode(const Complex & from)
   {
      Node nd;
      nd.push_back(complex2String(from));
      return nd;
   }

   bool convert<Complex>::decode(const Node & nd, Complex & to)
   {
      to = string2Complex(nd.as<std::string>());
      return true;
   }

   Node convert<Phase>::encode(const Phase & from)
   {
      Node nd;
      nd.push_back(phase2Str(from));
      return nd;
   }

   bool convert<Phase>::decode(const Node & nd, Phase & to)
   {
      to = str2Phase(nd.as<std::string>());
      return true;
   }

   Node convert<Phases>::encode(const Phases & from)
   {
      Node nd;
      for (const auto & phasePair : from)
      {
         nd.push_back(phase2Str(phasePair.first)); 
      }
      return nd;
   }
   bool convert<Phases>::decode(const Node & nd, Phases & to)
   {
      if(!nd.IsSequence())
      {
         return false;
      }
      else
      {
         int sz = nd.size();
         to = Phases();
         for (int i = 0; i < sz; ++i)
         {
            to |= nd[i].as<Phase>();
         }
      }
      return true;
   }

   Node convert<BusType>::encode(const BusType & from)
   {
      Node nd;
      nd.push_back(busType2Str(from));
      return nd;
   }

   bool convert<BusType>::decode(const Node & nd, BusType & to)
   {
      to = str2BusType(nd.as<std::string>());
      return true;
   }

   Node convert<Time>::encode(const Time & from)
   {
      Node nd;
      nd.push_back(posix_time::to_simple_string(from));
      return nd;
   }

   bool convert<Time>::decode(const Node & nd, Time & to)
   {
      to = posix_time::duration_from_string(nd.as<std::string>()); 
      return true;
   }

   Node convert<posix_time::ptime>::encode(const posix_time::ptime & from)
   {
      Node nd;
      nd.push_back(posix_time::to_simple_string(from));
      return nd;
   }

   bool convert<posix_time::ptime>::decode(const Node & nd, posix_time::ptime & to)
   {
      to = posix_time::time_from_string(nd.as<std::string>()); 
      return true;
   }

   template<typename T> Node convert<ublas::vector<T>>::encode(const ublas::vector<T> & from)
   {
      Node nd;
      for (const T & val : from) nd.push_back(val);
      return nd;
   }
   template Node convert<ublas::vector<double>>::encode(const ublas::vector<double> & from);
   template Node convert<ublas::vector<Complex>>::encode(const ublas::vector<Complex> & from);

   template<typename T> bool convert<ublas::vector<T>>::decode(const Node & nd, ublas::vector<T> & to)
   {
      if(!nd.IsSequence())
      {
         return false;
      }
      else
      {
         int sz = nd.size();
         to = ublas::vector<T>(sz);
         for (int i = 0; i < sz; ++i)
         {
            to(i) = nd[i].as<T>();
         }
      }
      return true;
   }
   template bool convert<ublas::vector<double>>::decode(const Node & nd, ublas::vector<double> & to);
   template bool convert<ublas::vector<Complex>>::decode(const Node & nd, ublas::vector<Complex> & to);

   template<typename T> Node convert<ublas::matrix<T>>::encode(const ublas::matrix<T> & from)
   {
      Node nd;
      for (int i = 0; i < from.size1(); ++i)
      {
         Node nd1;
         for (int k = 0; k < from.size2(); ++k)
         {
            nd1.push_back(from(i, k));
         }
         nd.push_back(nd1);
      }
      return nd;
   }
   template Node convert<ublas::matrix<double>>::encode(const ublas::matrix<double> & from);
   template Node convert<ublas::matrix<Complex>>::encode(const ublas::matrix<Complex> & from);

   template<typename T> bool convert<ublas::matrix<T>>::decode(const Node & nd, ublas::matrix<T> & to)
   {
      if(!nd.IsSequence())
      {
         return false;
      }
      else
      {
         int nrows = nd.size();
         if (nrows == 0)
         {
            std::cerr << "Matrix has zero rows in yaml." << std::endl;
            return false;
         }
         int ncols = nd[0].size();
         if (ncols == 0)
         {
            std::cerr << "Matrix has zero columns in yaml." << std::endl;
            return false;
         }
         for (int i = 1; i < nrows; ++i)
         {
            if (nd[i].size() != ncols)
            {
               std::cerr << "Ill-formed matrix in yaml." << std::endl;
               return false;
            }
         }
         to = ublas::matrix<T>(nrows, ncols);
         for (int i = 0; i < nrows; ++i)
         {
            for (int k = 0; k < nrows; ++k)
            {
               to(i, k) = nd[i][k].as<T>();
            }
         }
      }
      return true;
   }
   template bool convert<ublas::matrix<double>>::decode(const Node & nd, ublas::matrix<double> & to);
   template bool convert<ublas::matrix<Complex>>::decode(const Node & nd, ublas::matrix<Complex> & to);
}

namespace SmartGridToolbox
{
   std::string ParserState::expandName(const std::string & target) const
   {
      std::string result(target);
      for (const ParserLoop & loop : loops_)
      {
         std::string search = "${" + loop.name_ + "}";
         std::string replace = std::to_string(loop.i_);
         boost::replace_all(result, search, replace);
      }
      return result;
   }

   void assertFieldPresent(const YAML::Node & nd, const std::string & field)
   {
      if (!(nd[field]))
      {
         error() << "Parsing: " << field << " field not present." << std::endl;
         abort();
      }
   };

   void Parser::parse(const std::string & fname, Model & model, Simulation & simulation)
   {
      message() << "Started parsing file " << fname << "." << std::endl;
      mod_ = &model;
      sim_ = &simulation;
      top_ = YAML::LoadFile(fname);
      if (top_.size() == 0)
      {
         error() << "Parsing file " << fname << ". File is empty or doesn't exist." << std::endl;
         abort();
      }

      SGT_DEBUG(debug() << "Parsing global." << std::endl);
      parseGlobal(top_, model, simulation);
      SGT_DEBUG(debug() << "Parsed global." << std::endl);

      const YAML::Node & objsNode = top_["objects"];
      if (objsNode)
      {
         SGT_DEBUG(debug() << "Parsing objects." << std::endl);
         ParserState s;
         parseComponents(objsNode, s, model, false);
         SGT_DEBUG(debug() << "Parsed objects." << std::endl);
      }

      message() << "Finished parsing " << model.name() << "." << std::endl;
      message() << "Start time (local) = " << localTime(simulation.startTime(), model.timezone()) << std::endl;
      message() << "Start time (UTC)   = " << utcTime(simulation.startTime()) << std::endl;
      message() << "End time (local)   = " << localTime(simulation.endTime(), model.timezone()) << std::endl;
      message() << "End time (UTC)     = " << utcTime(simulation.endTime()) << std::endl;
      message() << "timezone           = " << model.timezone()->to_posix_string() << std::endl;
      message() << "lat_long           = " << model.latLong().lat_ << ", " << model.latLong().long_ << "." << std::endl;
   }

   void Parser::postParse()
   {
      const YAML::Node & objsNode = top_["objects"];
      if (objsNode)
      {
         ParserState s;
         parseComponents(objsNode, s, *mod_, true);
         SGT_DEBUG(debug() << "Parsed objects." << std::endl);
      }
      message() << "Parser post-parse " << mod_->name() << "." << std::endl;
   }
   
   Parser::Parser()
   {
      registerParserPlugins(*this);
   }

   void Parser::parseGlobal(const YAML::Node & top, Model & model,
                            Simulation & simulation)
   {
      assertFieldPresent(top, "global");
      const YAML::Node & nodeGlobal = top["global"];

      assertFieldPresent(nodeGlobal, "start_time");
      assertFieldPresent(nodeGlobal, "end_time");

      if (const YAML::Node & nodeConfig = nodeGlobal["configuration_name"])
      {
         model.setName(nodeConfig.as<std::string>());
      }
      else
      {
         model.setName(std::string("null"));
      }

      if (const YAML::Node & nodeTz = nodeGlobal["timezone"])
      {
         try 
         {
            model.setTimezone(local_time::time_zone_ptr(new local_time::posix_time_zone(nodeTz.as<std::string>())));
         }
         catch (...)
         {
            error() << "Couldn't parse timezone " << nodeTz.as<std::string>() << "." << std::endl;
            abort();
         }
      }

      const YAML::Node & nodeStart = nodeGlobal["start_time"];
      try 
      {
         simulation.setStartTime(parseTime(nodeStart, model));
      }
      catch (...)
      {
         error() << "Couldn't parse start date " << nodeStart.as<std::string>() << "." << std::endl;
         abort();
      }

      const YAML::Node & nodeEnd = nodeGlobal["end_time"];
      try 
      {
         simulation.setEndTime(parseTime(nodeEnd, model));
      }
      catch (...)
      {
         error() << "Couldn't parse end date " << nodeEnd.as<std::string>() << "." << std::endl;
         abort();
      }

      if (const YAML::Node & nodeLatLong = nodeGlobal["lat_long"])
      {
         try 
         {
            std::vector<double> llvec = nodeLatLong.as<std::vector<double>>();
            if (llvec.size() != 2)
            {
               throw;
            };
            model.setLatLong({llvec[0], llvec[1]});
         }
         catch (...)
         {
            error() << "Couldn't parse lat_long " << nodeLatLong.as<std::string>() << "." << std::endl;
            abort();
         }
      }

   }

   void Parser::parseComponents(const YAML::Node & node, ParserState & state, Model & model, bool isPostParse)
   {
      message() << "Parsing components. Starting." << std::endl;
      for (const auto & compPair : node)
      {
         std::string nodeType = compPair.first.as<std::string>();
         const YAML::Node & nodeVal = compPair.second;;
         if (nodeType == "loop")
         {
            std::string loopName = nodeVal["name"].as<std::string>();
            int loopCount = nodeVal["count"].as<int>();
            const YAML::Node & loopBody = nodeVal["body"];
            for (state.pushLoop(loopName); state.topLoopVal() < loopCount; state.incrTopLoop())
            {
               parseComponents(loopBody, state, model, isPostParse);
            }
            state.popLoop();
         }
         else
         {
            message() << "Parsing plugin " <<  nodeType << "." << std::endl;
            const ParserPlugin * compParser = componentParser(nodeType);
            if (compParser == nullptr)
            {
               warning() << "I don't know how to parse component " << nodeType << std::endl;
            }
            else if (isPostParse)
            {
               compParser->postParse(nodeVal, model, state);
            }
            else
            {
               compParser->parse(nodeVal, model, state);
            }
         }
      }
      message() << "Parsing components. Completed." << std::endl;
   }
}
