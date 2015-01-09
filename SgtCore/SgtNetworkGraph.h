#ifndef SGT_NETWORK_GRAPH_DOT_H
#define SGT_NETWORK_GRAPH_DOT_H

#include <SgtCore/Network.h>
#include <SgtCore/NetworkGraph.h>

namespace SmartGridToolbox
{
   struct SgtGraphNodeInfo
   {
      double x;
      double y;
      double w;
      double h;
      Node* node;
   };
   
   struct SgtGraphArcInfo
   {
      double l;
      Arc* arc;
   };

   using SgtGraphTraits = GraphTraits<SgtGraphNodeInfo, SgtGraphArcInfo>;
   using SgtGraphNode = GraphNode<SgtGraphTraits>;
   using SgtGraphArc = GraphArc<SgtGraphTraits>;

   class SgtNetworkGraph : public NetworkGraph<GraphTraits<SgtGraphNodeInfo, SgtGraphArcInfo>>
   {
      public:
         void create(const Network& nw);
   };
}

#endif // SGT_NETWORK_GRAPH_DOT_H