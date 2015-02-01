#ifndef SIM_NETWORK_COMPONENT_DOT_H
#define SIM_NETWORK_COMPONENT_DOT_H

#include <SgtSim/SimComponent.h>

#include <SgtCore/Bus.h>
#include <SgtCore/CommonBranch.h>
#include <SgtCore/DgyTransformer.h>
#include <SgtCore/Gen.h>
#include <SgtCore/OverheadLine.h>
#include <SgtCore/SinglePhaseTransformer.h>
#include <SgtCore/YyTransformer.h>
#include <SgtCore/Zip.h>

#include <memory>

namespace SmartGridToolbox
{
   class SimBus : public SimComponentAdaptor, public Bus 
   {
      public:
         SimBus(const Bus& bus) :
            Bus(bus)
         {
            // Empty.
         }
   };
   class SimCommonBranch : public SimComponentAdaptor, public CommonBranch
   {
      public:
         SimCommonBranch(const CommonBranch& branch) :
            CommonBranch(branch)
         {
            // Empty.
         }
   };
   class SimDgyTransformer : public SimComponentAdaptor, public DgyTransformer
   {
      public:
         SimDgyTransformer(const DgyTransformer& dgy) :
            DgyTransformer(dgy)
         {
            // Empty.
         }
   };
   class SimGenericBranch : public SimComponentAdaptor, public GenericBranch
   {
      public:
         SimGenericBranch(const GenericBranch& branch) :
            GenericBranch(branch)
         {
            // Empty.
         }
   };
   class SimGenericGen : public SimComponentAdaptor, public GenericGen
   {
      public:
         SimGenericGen(const GenericGen& gen) :
            GenericGen(gen)
         {
            // Empty.
         }
   };
   class SimGenericZip : public SimComponentAdaptor, public GenericZip
   {
      public:
         SimGenericZip(const GenericZip& zip) :
            GenericZip(zip)
         {
            // Empty.
         }
   };
   class SimOverheadLine : public SimComponentAdaptor, public OverheadLine
   {
      public:
         SimOverheadLine(const OverheadLine& ohl) :
            OverheadLine(ohl)
         {
            // Empty.
         }
   };
   class SimSinglePhaseTransformer : public SimComponentAdaptor, public SinglePhaseTransformer
   {
      public:
         SimSinglePhaseTransformer(const SinglePhaseTransformer& trans) :
            SinglePhaseTransformer(trans)
         {
            // Empty.
         }
   };
   class SimYyTransformer : public SimComponentAdaptor, public YyTransformer
   {
      public:
         SimYyTransformer(const YyTransformer& trans) :
            YyTransformer(trans)
         {
            // Empty.
         }
   };
}

#endif // SIM_NETWORK_COMPONENT_DOT_H
