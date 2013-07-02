#ifndef SOLVER_NR_DOT_H
#define SOLVER_NR_DOT_H

#include "Common.h"
#include <vector>

namespace SmartGridToolbox
{
   enum class BusType : int
   {
      PQ = 0,
      PV = 1,
      SL = 2
   };

   struct  BUSDATA
   {
      int type;                        ///< bus type (0=PQ, 1=PV, 2=SWING).
      unsigned char phases;            ///< Phases property. Used for construction of matrices (skip bad entries).
                                       /**  0x01 = C
                                        *   0x02 = B
                                        *   0x04 = A
                                        *   0x08 = D
                                        *   0x10 = Diff phase child
                                        *   0x20 = To side of SPCT
                                        *   0x40 = House present
                                        *   0x80 = Split phase. */

      // The following appear to be A/B/C to ground for wye and AB/BC/CA for delta.
      Complex V[3];                    ///< Bus voltage / phase (wye) or phase pair (delta).
      Complex S[3];                    ///< Constant power / phase (wye) or phase pair (delta).
      Complex Y[3];                    ///< Constant admittance / phase (wye) or phase pair (delta) (impedance loads).
      Complex I[3];                    ///< Constant current / phase (wye) or phase pair (delta).
      std::vector<int> Link_Table;     ///< Table of links that connect to us (for population purposes).
      double PL[3];                    ///< Real power component of total bus load.
      double QL[3];                    ///< Reactive power component of total bus load.
      double PG[3];                    ///< Real power generation at generator bus.
      double QG[3];                    ///< Reactive power generation at generator bus.
      double kv_base;                  ///< kV basis.
      double mva_base;                 ///< MVA basis.
      double Jacob_A[3];               ///< Element a in equation (37).
                                       /**< Used to update the Jacobian matrix at each iteration. */
      double Jacob_B[3];               ///< Element b in equation (38).
                                       /**< Used to update the Jacobian matrix at each iteration. */
      double Jacob_C[3];               ///< Element c in equation (39).
                                       /**< Used to update the Jacobian matrix at each iteration. */
      double Jacob_D[3];               ///< Element d in equation (40).
                                       /**< Used to update the Jacobian matrix at each iteration. */
      unsigned int Matrix_Loc;         ///< Starting idx of object's place in all matrices/equations.
      double max_volt_error;           ///< Maximum voltage error specified for node.
   };
   typedef struct BUSDATA BUSDATA;

   struct BRANCHDATA
   {
      Complex *Yfrom;                  ///< Branch admittance of from side of link.
      Complex *Yto;                    ///< Branch admittance of to side of link.
      Complex *YSfrom;                 ///< Self admittance seen on from side.
      Complex *YSto;                   ///< Self admittance seen on to side.
      unsigned char phases;            ///< Phases property.
                                       /**< Used for construction of matrices. */
      int from;                        ///< Index into bus data.
      int to;                          ///< Index into bus data.
      bool *status;                    ///< Status of the obj, if it is a switch (restoration module usage).
      unsigned char lnk_type;          ///< Type of link the object is.
                                       /**< 0 = UG/OH line, 1 = Triplex line, 2 = switch, 3 = fuse,
                                        *   4 = transformer, 5 = sectionalizer, 6 = recloser. */
      double v_ratio;                  ///< Voltage ratio (V_from/V_to).
   };
   typedef struct BRANCHDATA BRANCHDATA;

   struct Y_NR {
      int row_ind;                     ///< row loc of the element in 6n*6n Y matrix in NR solver.
      int col_ind;                     ///< col location of the element in 6n*6n Y matrix in NR solver.
      double Y_value;                  ///< value of the element in 6n*6n Y matrix in NR solver.
   };
   typedef struct Y_NR Y_NR;

   struct Bus_admit {
      int row_ind;                     ///< Row loc of the element in n*n bus admittance matrix in NR solver.
      int col_ind;                     ///< Col loc of the element in n*n bus admittance matrix in NR solver.
      Complex Y[3][3];                 ///< Complex value of elements in bus admittance matrix in NR solver.
      char size;                       ///< Size of the admittance diagonal.Assumed square, useful for smaller size.
   };
   typedef struct Bus_admit Bus_admit;

   struct NR_SOLVER_VARS {
      double *a_LU;
      double *rhs_LU;
      int *cols_LU;
      int *rows_LU;
   };
   typedef struct NR_SOLVER_VARS NR_SOLVER_VARS;

   int solver_nr(unsigned int bus_count, BUSDATA *bus, unsigned int branch_count, BRANCHDATA *branch,
                 bool *bad_computations);

}

#endif // SOLVER_NR_DOT_H
