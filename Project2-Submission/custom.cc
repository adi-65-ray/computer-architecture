
/* 
 * Implementation of a Custom branch predictor - group 50
 */

#include "cpu/pred/custom.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"

CustomBP::CustomBP(const CustomBPParams &params)
    : BPredUnit(params),
      globalHistoryReg(params.numThreads, 0),
      globalHistoryBits(params.globalHistoryBits),
      PredictorSize(params.PredictorSize),
      PHTCtrBits(params.PHTCtrBits),
      PHTCounters(PredictorSize, SatCounter8(PHTCtrBits))
{
    #ifdef DEBUG_CUSTOM
    printf("****************Constructor\n");
    #endif
    // To check for the validity of size
    if (!isPowerOf2(PredictorSize))
        fatal("Invalid choice predictor size.\n");

    historyRegisterMask = mask(globalHistoryBits);

    // Creating mark for branch and global adresses according to the predictor sizes
    branchHistoryMask = PredictorSize - 1;
    globalHistoryMask = PredictorSize - 1;

    // This is to create a threshold for determining the prediction
    choiceThreshold = (ULL(1) << (PHTCtrBits - 1)) - 1;
}

/*
 * For an unconditional branch we set its history such that
 * everything is set to taken
 */
void CustomBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    #ifdef DEBUG_CUSTOM
    printf("****************uncondBranch\n");
    #endif

    // modify the global history and final predition of the thread
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->finalPred = true;
    
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, true);
}

void CustomBP::squash(ThreadID tid, void *bpHistory)
{
    #ifdef DEBUG_CUSTOM
    printf("****************squash\n");
    #endif
    
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryReg[tid] = history->globalHistoryReg;

    delete history;
}

bool CustomBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    #ifdef DEBUG_CUSTOM
        printf("****************lookup\n");
    #endif

    // Calculate global index and branch index
    unsigned branchHistoryIdx = (branchAddr >> instShiftAmt) & branchHistoryMask;
    unsigned globalHistoryIdx = globalHistoryReg[tid] & globalHistoryMask;

    // Check if the index is valid
    assert(branchHistoryIdx < PredictorSize);
    assert(globalHistoryIdx < PredictorSize);

    // Perform NAND operation according to the configuration
    unsigned PHTIdx = ~(branchHistoryIdx & globalHistoryIdx) & branchHistoryMask;
    bool finalPrediction = PHTCounters[PHTIdx] > choiceThreshold;

    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->finalPred = finalPrediction;

    bpHistory = static_cast<void*>(history);

    updateGlobalHistReg(tid, finalPrediction);

    return finalPrediction;
}

void CustomBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    #ifdef DEBUG_CUSTOM
            printf("****************btbUpdate\n");
    #endif
    //Update Global History to Not Taken (clear LSB)
    globalHistoryReg[tid] &= (historyRegisterMask & ~ULL(1));
}

void CustomBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed, const StaticInstPtr & inst, Addr corrTarget)
{
    #ifdef DEBUG_CUSTOM
            printf("****************update\n");
    #endif
    // check for a valid bpHistory
    assert(bpHistory);

    BPHistory *history = static_cast<BPHistory*>(bpHistory);

    // We do not update the counters speculatively on a squash.
    // We just restore the global history register.
    if (squashed) {
        #ifdef DEBUG_CUSTOM
        printf("****************update squashed\n");
        #endif

        globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
        return;
    }

    // Calculate global index and branch index
    unsigned branchHistoryIdx = (branchAddr >> instShiftAmt) & branchHistoryMask;
    unsigned globalHistoryIdx = history->globalHistoryReg & globalHistoryMask;

    // Check if the index is valid
    assert(branchHistoryIdx < PredictorSize);
    assert(globalHistoryIdx < PredictorSize);

    // Perform NAND operation according to the configuration
    unsigned PHTIdx = ~(branchHistoryIdx & globalHistoryIdx) & branchHistoryMask;

    /*
    * +------------+---------------+----------+
    * | Predition  | branch status | counter  |
    * +------------+---------------+----------+
    * | Taken      | Taken         |    ++    |
    * | Taken      | Not Taken     |    --    |
    * | Not Taken  | Taken         |    ++    |
    * | Not Taken  | Not Taken     |    --    |
    * +------------+---------------+----------+
    */

    if (history->finalPred == taken) {
        
        if (taken) {
            #ifdef DEBUG_CUSTOM
            printf("****************update counter ++\n");
            #endif

            PHTCounters[PHTIdx]++;
        } else {
            #ifdef DEBUG_CUSTOM
            printf("****************update counter --\n");
            #endif

            PHTCounters[PHTIdx]--;
        }
    } else {

        if (taken) {
            #ifdef DEBUG_CUSTOM
            printf("****************update counter ++\n");
            #endif

            PHTCounters[PHTIdx]++;
        } else {
            #ifdef DEBUG_CUSTOM
            printf("****************update counter --\n");
            #endif

            PHTCounters[PHTIdx]--;
        }
    }

    delete history;
}

/*
* This function is use dot update the global history according to the prediction
*/
void CustomBP::updateGlobalHistReg(ThreadID tid, bool taken)
{   
    // if it's taken then we add 1 else we add 0 in the history
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    
    // Mask the global history to avoid segmenation fault
    globalHistoryReg[tid] &= historyRegisterMask;
}
