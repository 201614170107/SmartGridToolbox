#include <algorithm>
#include <ostream>
#include <sstream>
#include "PowerFlowNR.h"
#include "SparseSolver.h"

namespace SmartGridToolbox
{
   BusNR::BusNR(const std::string & id, BusType type, Phases phases, const UblasVector<Complex> & V,
                const UblasVector<Complex> & Ys, const UblasVector<Complex> & Ic, const UblasVector<Complex> & Sc) :
      id_(id),
      type_(type),
      phases_(phases),
      V_(V),
      S_(Sc),
      Ys_(Ys),
      Ic_(Ic),
      Sc_(Sc)
   {
      assert(V.size() == phases.size());
      assert(Ys.size() == phases.size());
      assert(Ic.size() == phases.size());
      assert(Sc.size() == phases.size());

      for (int i = 0; i < phases.size(); ++i)
      {
         nodes_.push_back(new NodeNR(*this, i));
      }
   }

   BusNR::~BusNR()
   {
      for (auto node : nodes_) delete node;
   }

   NodeNR::NodeNR(BusNR & bus, int phaseIdx) :
      bus_(&bus),
      phaseIdx_(phaseIdx),
      V_(bus.V_(phaseIdx)),
      S_(bus.S_(phaseIdx)),
      Ys_(bus.Ys_(phaseIdx)),
      Ic_(bus.Ic_(phaseIdx)),
      Sc_(bus.Sc_(phaseIdx)),
      idx_(-1)
   {
      // Empty.
   }

   BranchNR::BranchNR(const std::string & id0, const std::string & id1, Phases phases0, Phases phases1,
                      const UblasMatrix<Complex> & Y) :
      nPhase_(phases0.size()),
      ids_{id0, id1},
      phases_{phases0, phases1},
      Y_(Y)
   {
      assert(phases1.size() == nPhase_);
      int nTerm = 2 * nPhase_;
      assert(Y.size1() == nTerm);
      assert(Y.size2() == nTerm);
   }

   PowerFlowNR::~PowerFlowNR()
   {
      for (auto pair : busses_) delete pair.second;
      for (auto branch : branches_) delete branch;
   }

   void PowerFlowNR::addBus(const std::string & id, BusType type, Phases phases, const UblasVector<Complex> & V,
         const UblasVector<Complex> & Y, const UblasVector<Complex> & I, const UblasVector<Complex> & S)
   {
      SGT_DEBUG(debug() << "PowerFlowNR : add bus " << id << std::endl);
      busses_[id] = new BusNR(id, type, phases, V, Y, I, S);
   }

   void PowerFlowNR::addBranch(const std::string & idBus0, const std::string & idBus1, Phases phases0, Phases phases1,
                               const UblasMatrix<Complex> & Y)
   {
      SGT_DEBUG(debug() << "PowerFlowNR : addBranch " << idBus0 << " " << idBus1 << std::endl);
      branches_.push_back(new BranchNR(idBus0, idBus1, phases0, phases1, Y));
   }

   void PowerFlowNR::reset()
   {
      SGT_DEBUG(debug() << "PowerFlowNR : reset." << std::endl);
      busses_ = BusMap();
      branches_ = BranchVec();
   }

   void PowerFlowNR::validate()
   {
      SGT_DEBUG(debug() << "PowerFlowNR : validate." << std::endl);

      // Make Nodes:
      NodeVec PQNodes = NodeVec();
      NodeVec PVNodes = NodeVec();
      NodeVec SLNodes = NodeVec();
      for (auto & busPair : busses_)
      {
         BusNR & bus = *busPair.second;
         NodeVec * vec = nullptr;
         if (bus.type_ == BusType::PQ)
         {
            vec = &PQNodes;
         }
         else if (bus.type_ == BusType::PV)
         {
            vec = &PVNodes;
         }
         else if (bus.type_ == BusType::SL)
         {
            vec = &SLNodes;
         }
         else
         {
            error() << "Unsupported bus type " << busType2Str(bus.type_) << std::endl;
            abort();
         }
         for (NodeNR * node : bus.nodes_)
         {
            vec->push_back(node);
         }
      }

      // Determine sizes:
      nPQ_ = PQNodes.size();
      nPV_ = PVNodes.size();
      nSL_ = SLNodes.size();
      assert(nSL_ > 0); // TODO: What is correct here?

      // Insert nodes into ordered list of all nodes. Be careful of ordering!
      nodes_ = NodeVec();
      nodes_.reserve(nNode());
      nodes_.insert(nodes_.end(), SLNodes.begin(), SLNodes.end());
      nodes_.insert(nodes_.end(), PQNodes.begin(), PQNodes.end());
      nodes_.insert(nodes_.end(), PVNodes.begin(), PVNodes.end());
      // Index all nodes:
      for (int i = 0; i < nNode(); ++i)
      {
         nodes_[i]->idx_ = i;
      }

      Y_.resize(nNode(), nNode(), false);

      // Branch admittances:
      for (BranchNR * branch : branches_)
      {
         auto it0 = busses_.find(branch->ids_[0]);
         if (it0 == busses_.end())
         {
            error() << "Branch " << branch->ids_[0] << " " << branch->ids_[1] << " contains a non-existent bus "
                    << branch->ids_[0] << std::endl;
            abort();
         }
         auto it1 = busses_.find(branch->ids_[1]);
         if (it1 == busses_.end())
         {
            error() << "Branch " << branch->ids_[0] << " " << branch->ids_[1] << " contains a non-existent bus "
                    << branch->ids_[1] << std::endl;
            abort();
         }
         const BusNR * busses[] = {it0->second, it1->second};
         int nTerm = 2 * branch->nPhase_;

         // There is one link per distinct pair of bus/phase pairs.
         for (int i = 0; i < nTerm; ++i)
         {
            int busIdxI = i / branch->nPhase_; // 0 or 1
            int branchPhaseIdxI = i % branch->nPhase_; // 0 to nPhase of branch.
            const BusNR * busI = busses[busIdxI];
            int busPhaseIdxI = busI->phases_.phaseIndex(branch->phases_[busIdxI][branchPhaseIdxI]);
            const NodeNR * nodeI = busI->nodes_[busPhaseIdxI];
            int idxNodeI = nodeI->idx_;

            // Only count each diagonal element in branch->Y_ once!
            Y_(idxNodeI, idxNodeI) += branch->Y_(i, i) + nodeI->Ys_;

            for (int k = i + 1; k < nTerm; ++k)
            {
               int busIdxK = k / branch->nPhase_; // 0 or 1
               int branchPhaseIdxK = k % branch->nPhase_; // 0 to nPhase of branch.
               const BusNR * busK = busses[busIdxK];
               int busPhaseIdxK = busK->phases_.phaseIndex(branch->phases_[busIdxK][branchPhaseIdxK]);
               const NodeNR * nodeK = busK->nodes_[busPhaseIdxK];
               int idxNodeK = nodeK->idx_;

               Y_(idxNodeI, idxNodeK) += branch->Y_(i, k);
               Y_(idxNodeK, idxNodeI) += branch->Y_(k, i);
            }
         }
      } // Loop over branches.

      // Load quantities.
      Ic_.resize(nNode(), false);
      for (int i = 0; i < nNode(); ++i)
      {
         Ic_(i) = nodes_[i]->Ic_;
      }

      SGT_DEBUG(debug() << "PowerFlowNR : validate complete." << std::endl);
      SGT_DEBUG(printProblem());
   }

   /// Initialize voltages:
   void PowerFlowNR::initV(UblasVector<double> & Vr, UblasVector<double> & Vi) const
   {
      for (int i = 0; i < nNode(); ++i)
      {
         const NodeNR & node = *nodes_[i];
         Vr(i) = node.V_.real();
         Vi(i) = node.V_.imag();
      }
   }

   void PowerFlowNR::initS(UblasVector<double> & P, UblasVector<double> & Q) const
   {
      for (int i = 0; i < nNode(); ++i)
      {
         const NodeNR & node = *nodes_[i];
         P(i) = node.S_.real();
         Q(i) = node.S_.imag();
      }
   }

   /// Set the part of J that doesn't update at each iteration.
   /** At this stage, we are treating J as if all busses were PQ. */
   void PowerFlowNR::initJC(UblasCMatrix<double> & JC) const
   {
      const auto G = real(Y_);
      const auto B = imag(Y_);

      const auto GRng = project(G, selPQPVFromAll(), selPQPVFromAll());
      const auto BRng = project(B, selPQPVFromAll(), selPQPVFromAll());

      project(JC, selIrFromf(), selVrFromx()) = -GRng;
      project(JC, selIrFromf(), selViFromx()) =  BRng;
      project(JC, selIiFromf(), selVrFromx()) = -BRng;
      project(JC, selIiFromf(), selViFromx()) = -GRng;
      std::cout << "initJC, JC = " << std::endl;
      for (int i = 0; i < JC.size1(); ++i) std::cout << row(JC, i) << std::endl;
   }

   // At this stage, we are treating f as if all busses were PQ. PV busses will be taken into account later.
   void PowerFlowNR::updatef(UblasVector<double> & f,
                             const UblasVector<double> & Vr, const UblasVector<double> & Vi,
                             const UblasVector<double> & P, const UblasVector<double> & Q,
                             const UblasCMatrix<double> & J) const
   {
      const auto G = real(Y_);
      const auto B = imag(Y_);

      const auto GRng = project(G, selPQPVFromAll(), selAllFromAll());
      const auto BRng = project(B, selPQPVFromAll(), selAllFromAll());

      const auto VrPQPV = project(Vr, selPQPVFromAll());
      const auto ViPQPV = project(Vi, selPQPVFromAll());

      const auto VrPV = project(Vr, selPVFromAll());
      const auto ViPV = project(Vi, selPVFromAll());

      const auto PPQPV = project(P, selPQPVFromAll());
      const auto QPQPV = project(Q, selPQPVFromAll());

      UblasVector<double> M2PQPV = element_prod(VrPQPV, VrPQPV) + element_prod(ViPQPV, ViPQPV);

      project(f, selIrFromf()) = element_div(element_prod(VrPQPV, PPQPV) + element_prod(ViPQPV, QPQPV), M2PQPV)
                               + project(real(Ic_), selPQPVFromAll())
                               - prod(GRng, Vr) + prod(BRng, Vi);
      project(f, selIiFromf()) = element_div(element_prod(ViPQPV, PPQPV) - element_prod(VrPQPV, QPQPV), M2PQPV)
                               + project(imag(Ic_), selPQPVFromAll())
                               - prod(GRng, Vi) - prod(BRng, Vr);
   }

   // At this stage, we are treating f as if all busses were PQ. PV busses will be taken into account later.
   void PowerFlowNR::updateJ(UblasCMatrix<double> & J, const UblasCMatrix<double> & JC,
                             const UblasVector<double> Vr, const UblasVector<double> Vi,
                             const UblasVector<double> P, const UblasVector<double> Q) const
   {
      // Elements in J that have no non-constant part will be initialized to the corresponding term in JC at the
      // start of the calculation, and will not change. Thus, only set elements that have a non-constant part.

      const auto G = real(Y_);
      const auto B = imag(Y_);

      auto VrPQPV = project(Vr, selPQPVFromAll());
      auto ViPQPV = project(Vr, selPQPVFromAll());
      auto PPQPV = project(P, selPQPVFromAll());
      auto QPQPV = project(Q, selPQPVFromAll());

      auto JIrVr = project(J, selIrFromf(), selVrFromx());
      auto JIrVi = project(J, selIrFromf(), selViFromx());
      auto JIiVr = project(J, selIiFromf(), selVrFromx());
      auto JIiVi = project(J, selIiFromf(), selViFromx());
      
      auto JCIrVr = project(JC, selIrFromf(), selVrFromx());
      auto JCIrVi = project(JC, selIrFromf(), selViFromx());
      auto JCIiVr = project(JC, selIiFromf(), selVrFromx());
      auto JCIiVi = project(JC, selIiFromf(), selViFromx());

      auto JAllQPV = project(J, selAllFromf(), selQPVFromx());
      auto JIrQPV = project(J, selIrFromf(), selQPVFromx());
      auto JIrViPV = project(J, selIrFromf(), selViPVFromx());
      auto JIiQPV = project(J, selIiFromf(), selQPVFromx());
      auto JIiViPV = project(J, selIiFromf(), selViPVFromx());
      
      auto JCAllQPV = project(J, selAllFromf(), selQPVFromx());
      auto JCIrQPV = project(JC, selIrFromf(), selQPVFromx());
      auto JCIrViPV = project(JC, selIrFromf(), selViPVFromx());
      auto JCIiQPV = project(JC, selIiFromf(), selQPVFromx());
      auto JCIiViPV = project(JC, selIiFromf(), selViPVFromx());

      // Reset PV columns:
      JAllQPV = JCAllQPV;

      // Block diagonal:
      for (int i = 0; i < nPQPV(); ++i)
      {
         double PVr_p_QVi = PPQPV(i) * VrPQPV(i) + QPQPV(i) * ViPQPV(i);
         double PVi_m_QVr = PPQPV(i) * ViPQPV(i) - QPQPV(i) * VrPQPV(i);
         double M2 = VrPQPV(i) * VrPQPV(i);
         double M4 = M2 * M2;
         double VrdM4 = VrPQPV(i) / M4;
         double VidM4 = ViPQPV(i) / M4;
         double PdM2 = PPQPV(i) / M2;
         double QdM2 = QPQPV(i) / M2;

         JIrVr(i, i) = JCIrVr(i, i) - (2 * VrdM4 * PVr_p_QVi) + PdM2;
         JIrVi(i, i) = JCIrVi(i, i) - (2 * VidM4 * PVr_p_QVi) + QdM2;
         JIiVr(i, i) = JCIiVr(i, i) - (2 * VrdM4 * PVi_m_QVr) - QdM2;
         JIiVi(i, i) = JCIiVi(i, i) - (2 * VidM4 * PVi_m_QVr) + PdM2;
      }
      std::cout << "updateJ, J = " << std::endl;
      for (int i = 0; i < J.size1(); ++i) std::cout << row(J, i) << std::endl;
   }

   // Modify J and f to take into account PV busses.
   void PowerFlowNR::modifyForPV(UblasCMatrix<double> & J, UblasVector<double> f, 
                                 const UblasVector<double> Vr, const UblasVector<double> Vi,
                                 const UblasVector<double> M2PV)
   {
      auto JAllQPV = project(J, selAllFromf(), selQPVFromx());
      auto JAllViPV = project(J, selAllFromf(), selViPVFromx());
      auto JIrPVQPV = project(J, selIrPVFromf(), selQPVFromx());
      auto JIiPVQPV = project(J, selIiPVFromf(), selQPVFromx());

      const auto VrPV = project(Vr, selPVFromAll());
      const auto ViPV = project(Vi, selPVFromAll());

      for (int k = 0; k < JAllQPV.size2(); ++k)
      {
         auto colAllVrk = column(JAllQPV, k);
         auto colAllVik = column(JAllViPV, k);
         
         // Modify f:
         f += colAllVrk * (0.5 * (M2PV(k) - VrPV(k) * VrPV(k) - ViPV(k) * ViPV(k)) / VrPV(k));

         // Modify Vi column in J:
         colAllVik -= colAllVrk * (ViPV(k) / VrPV(k));

         // Modify Vr column in J:
         for (int i = 0; i < colAllVrk.size(); ++i)
         {
            colAllVrk(i) = 0; // Set to zero rather than erase is OK here, since column is dense anyhow.
         }

         JIrPVQPV(k, k) = ViPV(k) / M2PV(k);
         JIiPVQPV(k, k) = -VrPV(k) / M2PV(k);
      }
   }

   bool PowerFlowNR::solve()
   {
      SGT_DEBUG(debug() << "PowerFlowNR : solve." << std::endl);

      const double tol = 1e-10;
      const int maxiter = 20;

      UblasVector<double> Vr(nNode());
      UblasVector<double> Vi(nNode());
      initV(Vr, Vi);

      UblasVector<double> M2PV = element_prod(project(Vr, selPVFromAll()), project(Vr, selPVFromAll()))
                               + element_prod(project(Vi, selPVFromAll()), project(Vi, selPVFromAll()));

      UblasVector<double> P(nNode());
      UblasVector<double> Q(nNode());
      initS(P, Q);

      UblasCMatrix<double> JC(nVar(), nVar()); ///< The part of J that doesn't update at each iteration.
      initJC(JC);

      UblasVector<double> f(nVar()); ///< Current mismatch function.

      UblasCMatrix<double> J = JC; ///< Jacobian, d f_i / d x_i.
      J = JC; // We only need to redo the elements that we mess with!

      bool wasSuccessful = false;
      for (int i = 0; i < maxiter; ++ i)
      {
         SGT_DEBUG(debug() << "\tIteration = " << i << std::endl);

         updatef(f, Vr, Vi, P, Q, J);
         UblasVector<double> f2 = element_prod(f, f);
         double err = sqrt(*std::max_element(f2.begin(), f2.end()));
         SGT_DEBUG(debug() << "\tf  = " << std::setw(8) << f << std::endl);
         SGT_DEBUG(debug() << "\tError = " << err << std::endl);
         if (err <= tol)
         {
            SGT_DEBUG(debug() << "\tSuccess at iteration " << i << ". Error = " << err << std::endl);
            wasSuccessful = true;
            break;
         }

         updateJ(J, JC, Vr, Vi, P, Q);

         modifyForPV(J, f, Vr, Vi, M2PV);

         UblasVector<double> rhs;

         SGT_DEBUG
         (
            debug() << "\tBefore KLUSolve: Vr  = " << std::setw(8) << Vr << std::endl;
            debug() << "\tBefore KLUSolve: Vi  = " << std::setw(8) << Vi << std::endl;
            debug() << "\tBefore KLUSolve: M^2 = " << std::setw(8)
                    << (element_prod(Vr, Vr) + element_prod(Vi, Vi)) << std::endl;
            debug() << "\tBefore KLUSolve: P   = " << std::setw(8) << P << std::endl;
            debug() << "\tBefore KLUSolve: Q   = " << std::setw(8) << Q << std::endl;
            debug() << "\tBefore KLUSolve: f   = " << std::setw(8) << f << std::endl;
            debug() << "\tBefore KLUSolve: J   = " << std::endl;
            for (int i = 0; i < nVar(); ++i)
            {
               debug() << "\t\t" << std::setw(8) << row(J, i) << std::endl;
            }
         );

         bool ok = KLUSolve(J, -f, rhs);
         SGT_DEBUG(debug() << "\tAfter KLUSolve: ok = " << ok << ", x = " << std::setw(8) << rhs << std::endl);
         if (!ok)
         {
            error() << "KLUSolve failed." << std::endl;
            abort();
         }

         // Update the current values of V from rhs:
         project(Vr, selPQFromAll()) += project(rhs, selVrPQFromx());
         project(Vi, selPQFromAll()) += project(rhs, selViPQFromx());

         // Explicitly deal with the voltage magnitude constraint by updating VrPV by hand.
         auto VrPV = project(Vr, selPVFromAll());
         auto ViPV = project(Vi, selPVFromAll());
         auto DeltaViPV = project(rhs, selViPVFromx());
         auto DeltaVrPV = element_div(
               element_prod(VrPV, VrPV) + element_prod(ViPV, ViPV) - M2PV - 2 * element_prod(ViPV, DeltaViPV),
               2 * VrPV);
         ViPV += DeltaViPV;
         VrPV += DeltaVrPV;

         project(Q, selPVFromAll()) += project(rhs, selQPVFromx());

         SGT_DEBUG(debug() << "\tUpdated Vr  = " << std::setw(8) << Vr << std::endl);
         SGT_DEBUG(debug() << "\tUpdated Vi  = " << std::setw(8) << Vi << std::endl);
         SGT_DEBUG(debug() << "\tUpdated M^2 = " << std::setw(8)
                           << (element_prod(Vr, Vr) + element_prod(Vi, Vi)) << std::endl);
         SGT_DEBUG(debug() << "\tUpdated P   = " << std::setw(8) << P << std::endl);
         SGT_DEBUG(debug() << "\tUpdated Q   = " << std::setw(8) << Q << std::endl);
      }
      if (wasSuccessful)
      {
         UblasVector<Complex> V(nNode());
         UblasVector<Complex> S(nNode());

         for (int i = 0; i < nNode(); ++i)
         {
            V(i) = {Vr(i), Vi(i)};
            S(i) = {P(i), Q(i)};
         }

         // Set the slack power.
         auto SSL = project(S, selSLFromAll());

         auto VSL = project(V, selSLFromAll());
         auto IcSL = project(Ic_, selSLFromAll());

         auto YStar = conj(project(Y_, selSLFromAll(), selAllFromAll()));
         auto VStar = conj(V);
         auto IcStar = conj(project(Ic_, selSLFromAll()));

         SSL = element_prod(VSL, prod(YStar, VStar)) - element_prod(VSL, IcStar);

         // Update nodes and busses.
         for (int i = 0; i < nNode(); ++i)
         {
            NodeNR * node = nodes_[i];
            node->V_ = V(i);
            node->S_ = S(i);
            node->bus_->V_[node->phaseIdx_] = node->V_;
            node->bus_->S_[node->phaseIdx_] = node->S_;
         }
      }
      SGT_DEBUG(debug() << "PowerFlowNR : solve finished. Was successful = " << wasSuccessful << std::endl);
      return wasSuccessful;
   }

   bool PowerFlowNR::printProblem()
   {
      debug() << "PowerFlowNR::printProblem()" << std::endl;
      debug() << "\tNodes:" << std::endl;
      for (const NodeNR * nd : nodes_)
      {
         debug() << "\t\tNode:" << std::endl;
         debug() << "\t\t\tId    : " << nd->bus_->id_ << std::endl;
         debug() << "\t\t\tType  : " << nd->bus_->type_ << std::endl;
         debug() << "\t\t\tPhase : " << nd->bus_->phases_[nd->phaseIdx_] << std::endl;
         debug() << "\t\t\tV     : " << nd->V_ << std::endl;
         debug() << "\t\t\tY     : " << nd->Ys_ << std::endl;
         debug() << "\t\t\tI     : " << nd->Ic_ << std::endl;
         debug() << "\t\t\tS     : " << nd->Sc_ << std::endl;
      }
      debug() << "\tBranches:" << std::endl;
      for (const BranchNR * branch : branches_)
      {
         debug() << "\t\tBranch:" << std::endl;
         debug() << "\t\t\tBusses : " << branch->ids_[0] << ", " << branch->ids_[1] << std::endl;
         debug() << "\t\t\tPhases : " << branch->phases_[0] << ", " << branch->phases_[1] << std::endl;
         debug() << "\t\t\tY      :" << std::endl;
         for (int i = 0; i < branch->Y_.size1(); ++i)
         {
            debug() << "\t\t\t\t" << std::setw(16) << row(branch->Y_, i) << std::endl;
         }
      }
      debug() << "\tY:" << std::endl;
      for (int i = 0; i < Y_.size1(); ++i)
      {
         debug() << "\t\t\t\t" << std::setw(16) << row(Y_, i) << std::endl;
      }
   }
}
