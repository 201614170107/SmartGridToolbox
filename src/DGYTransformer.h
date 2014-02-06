#ifndef D_GY_TRANSFORMER_DOT_H
#define D_GY_TRANSFORMER_DOT_H

#include <SmartGridToolbox/Branch.h>

namespace SmartGridToolbox
{
   class DGYTransformer : public Branch
   {
      public:
         DGYTransformer(const std::string & name, Complex alpha, Complex ZL);
      
      private:
         void recalcY();

      private:
         Complex a_;  ///< Complex turns ratio, n0/n1 where 0 is primary and 1 is secondary.
         Complex YL_; ///< Series leakage impedance.
   };
}

#endif // D_GY_TRANSFORMER_DOT_H
