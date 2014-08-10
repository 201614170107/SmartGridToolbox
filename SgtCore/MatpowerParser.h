#ifndef MATPOWER_PARSER_DOT_H
#define MATPOWER_PARSER_DOT_H

#include <SgtCore/Parser.h>

namespace SmartGridToolbox
{
   /// @brief ParserPlugin that parses Matpower data files.
   class MatpowerParser : public ParserPlugin
   {
      public:
         static constexpr const char* pluginKey()
         {
            return "matpower";
         }

      public:
         virtual void parse(const YAML::Node& nd, Network& netw) const override;
   };
}

#endif // MATPOWER_PARSER_DOT_H