/* @file
 * Implementation of Custom branch predictor - group 50
 */

#ifndef __CPU_PRED_CUSTOM_BP_HH__
#define __CPU_PRED_CUSTOM_BP_HH__

#include "base/sat_counter.hh"
#include "cpu/pred/bpred_unit.hh"
#include "params/CustomBP.hh"

//#define DEBUG_CUSTOM

class CustomBP : public BPredUnit
{
  public:
    CustomBP(const CustomBPParams &params);
    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
    void squash(ThreadID tid, void *bp_history);
    bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
    void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed, const StaticInstPtr & inst, Addr corrTarget);

  private:
    void updateGlobalHistReg(ThreadID tid, bool taken);

    struct BPHistory {
        unsigned globalHistoryReg;
        bool finalPred;
    };

    std::vector<unsigned> globalHistoryReg;
    unsigned globalHistoryBits;
    unsigned historyRegisterMask;

    unsigned PredictorSize;
    unsigned PHTCtrBits;
    unsigned branchHistoryMask;
    unsigned globalPredictorSize;
    unsigned globalHistoryMask;

    // prediction counters
    std::vector<SatCounter8> PHTCounters;
    
    unsigned choiceThreshold;

};

#endif // __CPU_PRED_CUSTOM_BP_HH__
