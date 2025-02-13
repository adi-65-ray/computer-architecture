/**
* @author: Aditya Ray
*/
#include "mem/cache/replacement_policies/lru_variation.hh"

#include <cassert>
#include <memory>

#include "params/LRU_Variation.hh"
#include "sim/core.hh"

namespace ReplacementPolicy {

LRU_Variation::LRU_VariationReplData::LRU_VariationReplData(const int block_id, std::shared_ptr<std::vector<int>> ptr_recency) 
    : block_id(block_id), ptr_recency(ptr_recency) {}

LRU_Variation::LRU_Variation(const Params &p)
  : Base(p), associativity(p.numWays), block_count(0), ptr_new_recency(nullptr)
{
    #ifdef DEBUG_LRU
    printf("\n ******** Constructor ***********");
    #endif
}

void
LRU_Variation::invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    #ifdef DEBUG_LRU
    printf("\n ******** invalidate ***********");
    #endif
    /* storing replacement_data in local variable */
    std::shared_ptr<LRU_VariationReplData> data = std::static_pointer_cast<LRU_VariationReplData>(replacement_data);
    assert(data->ptr_recency != nullptr);// just to make sure we are not dereferencing a nullptr

    std::vector<int> ptr_recency = *(data->ptr_recency);
    int block_id = data->block_id;

    // make the entry invalid
    ptr_recency[block_id] = INVALID;
}

void
LRU_Variation::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    #ifdef DEBUG_LRU
    printf("\n ******** touch ***********");
    #endif

    /* storing replacement_data in local variable */
    std::shared_ptr<LRU_VariationReplData> data = std::static_pointer_cast<LRU_VariationReplData>(replacement_data);
    assert(data->ptr_recency != nullptr); // just to make sure we are not dereferencing a nullptr

    std::vector<int> ptr_recency = *(data->ptr_recency);

    int position = IPV[data->block_id]; 
    int old = ptr_recency[data->block_id];
    // Update last touch 
    for (auto& element : ptr_recency) // Iterate till the end of the vector
    {
        if((element >= position) && (element < old)) // Need to update the position of blocks between new position and old position of the block calling touch
        {   
            element = element + 1; // Move the position of blocks right by 1
            assert(element<=15);
        }
    }
}

void
LRU_Variation::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    #ifdef DEBUG_LRU
    printf("\n ******** reset ***********");
    #endif

    /* storing replacement_data in local variable */
    std::shared_ptr<LRU_VariationReplData> data = std::static_pointer_cast<LRU_VariationReplData>(replacement_data);
    assert(data->ptr_recency != nullptr); // just to make sure we are not dereferencing a nullptr

    std::vector<int> ptr_recency = *(data->ptr_recency);

    // Set last touch timestamp
    for (auto& element : ptr_recency)  // Iterate till the end of the vector
    {   
        // The position of blocks >= 8 needs to be shifted by 1 
        if((element >= insert_idx) && (element != 16)) // Each block will contain 16 as it's initial positon when initialized. To avoid shifting these blocks, index != 16 condition is placed 
        {
           element = element + 1;
        }
    }
    assert(data->block_id>=0 && data->block_id<=15);
    ptr_recency[data->block_id] = insert_idx; // Assign 8 as position to the block calling reset
}

ReplaceableEntry*
LRU_Variation::getVictim(const ReplacementCandidates& candidates) const
{
    #ifdef DEBUG_LRU
    printf("\n ******** Eviction ***********");
    #endif

    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // get shared recency data
    std::shared_ptr<LRU_VariationReplData> data = std::static_pointer_cast<LRU_VariationReplData>(candidates[0]->replacementData);
    assert(data->ptr_recency != nullptr); // just to make sure we are not dereferencing a nullptr

    std::vector<int> ptr_recency = *(data->ptr_recency);
    int maxpos = 0;
    int victim = 0;

    for (int i = 0; i<ptr_recency.size(); i++) 
    {
        // Update victim entry if necessary
        if(maxpos < ptr_recency[i])  // If the position is found to be greater, then that is our victim block
        {
            maxpos = ptr_recency[i]; // Store the position to be referenced again
            victim = i; // Store the index of victim block's index
        }
    }

    return candidates[victim]; // Return the victim block
}

std::shared_ptr<ReplacementData>
LRU_Variation::instantiateEntry()
{
    #ifdef DEBUG_LRU
    printf("\n ******** Instantiate ***********");
    #endif

     // Generate recency vector for every set created
    if (block_count % associativity == 0) {
        #ifdef DEBUG_LRU
        printf("\n ******** create new recency ***********");
        #endif
        ptr_new_recency = std::make_shared<std::vector<int>>(associativity,INVALID);
    }

    #ifdef DEBUG_LRU
    printf("\n ******** create new Relacement Data ***********");
    #endif
    // Create replacement data using current recency instance
    std::shared_ptr<LRU_VariationReplData> entry = std::make_shared<LRU_VariationReplData>(
        (block_count % associativity), ptr_new_recency);
    
    // Update instance block_counter
    block_count++;  

    return std::static_pointer_cast<ReplacementData>(entry);
}

} // namespace ReplacementPolicy
