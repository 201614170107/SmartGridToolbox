#ifndef MODEL_DOT_H
#define MODEL_DOT_H

#include <map>
#include <vector>
#include <string>
#include "Common.h"

using std::string;

namespace SmartGridToolbox
{
   class Component;

   /// Simulation environment, containing global simulation data.
   /// Units: All units are internally stored in SI.
   class Model
   {
      public:
         typedef std::map<std::string, Component *> ComponentMap;
         typedef std::vector<Component *> ComponentVec;

      public:
         /// Default constructor.
         Model() : name_("null"),
                   isValid_(false)
         {
            // Empty.
         }

         /// Destructor.
         ~Model();

         const std::string & name() const {return name_;}
         const void setName(const std::string & name) {name_ = name;}

         template<typename T, typename... Args> T & newComponent(Args&&... args)
         {
            T * comp = new T(std::forward<Args>(args)...);
            addGenericComponent(comp);
            return *comp;
         }

         template<typename T> const T * componentNamed(const std::string & name) const
         {
            ComponentMap::const_iterator it = compMap_.find(name);
            return (it == compMap_.end()) ? 0 : dynamic_cast<const T *>(it->second);
         }
         template<typename T> T * componentNamed(const std::string & name)
         {
            return const_cast<T *>((const_cast<const Model *>(this))-> componentNamed<T>(name));
         }

         template<typename T> const T * componentAt(int i) const
         {
            return (i >= 0 && i < compVec_.size()) ? dynamic_cast<T *>(compVec_[i]) : 0;
         }
         template<typename T> T * componentAt(int i)
         {
            return const_cast<T *>((const_cast<const Model *>(this))-> componentAt<T>(i));
         }

         const ComponentVec & components() const {return compVec_;}
         ComponentVec & components() {return compVec_;}

         LatLong getLatLong() {return latLong_;}
         void setLatLong(const LatLong & latLong) {latLong_ = latLong;}

         void validate();

      private:
         void addGenericComponent(Component * comp);

      private:
         std::string name_;
         bool isValid_;

         ComponentMap compMap_;
         ComponentVec compVec_;

         LatLong latLong_;
   };
}

#endif // MODEL_DOT_H
