
/**
 * @file
 * Declaration of a Least Recently Used replacement policy with IPV.
 * The victim is chosen as the highest recency in the vector.
 * @author: Aditya Ray
 */

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_LRU_VARIATION_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_LRU_VARIATION_RP_HH__

#include "mem/cache/replacement_policies/base.hh"

#define ASSOCIATIVITY_IPV   16
#define INVALID             16
//#define DEBUG_LRU

struct LRU_VariationParams;

namespace ReplacementPolicy {

class LRU_Variation : public Base
{
  private:
    // Insertion Promotion Vector
    const int IPV[ASSOCIATIVITY_IPV] = {0,0,1,0,3,0,3,2,1,0,5,1,0,0,4,11};
    // Insertion point
    const int insert_idx = 8;                                                
    int associativity;
    // Number of blocks
    int block_count;
    // to initialise shared recency vector for each set
    std::shared_ptr<std::vector<int>> ptr_new_recency;
    
  protected:
    
    /** LRU_Variation-specific implementation of replacement data. */
    struct LRU_VariationReplData : ReplacementData
    {
        /* recency array keeps track of the usage according to the IPV. */
        std::shared_ptr<std::vector<int>> ptr_recency;
        /* Block number */
        const int block_id;
        /**
         * Default constructor. Invalidate data.
         */
        LRU_VariationReplData(const int block_id, std::shared_ptr<std::vector<int>> ptr_recency);
    };

  public:
    typedef LRU_VariationParams Params;
    LRU_Variation(const Params &p);
    ~LRU_Variation() = default;

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Sets its last touch tick as the starting tick.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                              const override;

    /**
     * Touch an entry to update its replacement data.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Find replacement victim using LRU_Variation timestamps.
     *
     * @param candidates Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                     override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

} // namespace ReplacementPolicy

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_LRU_VARIATION_RP_HH__
