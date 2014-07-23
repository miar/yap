#include "lockFreeHash.tries.h"

#ifdef INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE
#define LFHT_STR          struct subgoal_trie_node
#define LFHT_STR_PTR      sg_node_ptr
#define LFTH_USES_ARGS
#define LFTH_PASS_ARGS

#define LFHT_NEW_NODE(NODE, _NOT_IN_USE, ENTRY, CHILD, PARENT, NEXT)  \
  { NEW_SUBGOAL_TRIE_NODE(NODE, ENTRY, CHILD, PARENT, NEXT); }

#define LFTH_FREE_NODE(PTR)    \
    FREE_SUBGOAL_TRIE_NODE(PTR);

#undef LFHT_STR
#undef LFHT_STR_PTR
#undef LFHT_NEW_NODE
#undef LFTH_FREE_NODE
#undef LFTH_PASS_ARGS
#endif /* INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE */

#ifdef INCLUDE_ANSWER_LOCK_FREE_HASH_TRIE
#define LFHT_STR           struct answer_trie_node
#define LFHT_STR_PTR       ans_node_ptr
#define LFTH_USES_ARGS     ,int instr
#define LFTH_PASS_ARGS     , instr
#define  LFHT_NEW_NODE(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT)    \
  { NEW_ANSWER_TRIE_NODE(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT); }

#define LFTH_FREE_NODE(PTR)
    FREE_ANSWER_TRIE_NODE(PTR);

#undef LFHT_STR
#undef LFHT_STR_PTR
#undef LFHT_NEW_NODE
#undef LFTH_FREE_NODE
#undef LFTH_PASS_ARGS
#endif /* INCLUDE_ANSWER_LOCK_FREE_HASH_TRIE */

####################################################################################
query_replace (lockFreeHash.tries.h  + lockFreeHash.tries.i) :
   struct subgoal_trie_node       ->   LFHT_STR
   sg_node_ptr                    ->   LFTH_STR_PTR
   NEW_SUBGOAL_TRIE_NODE          ->   LFTH_NEW_NODE
   FREE_SUBGOAL_TRIE_NODE(PTR)    ->   LFTH_FREE_NODE
   BASE_HASH_BUCKETS              ->   LFHT_BUCKET_SIZE
   SHIFT_SIZE                     ->   LFHT_SHIFTS
   V04_                           ->   LFHT_
   long                           ->   LFHT_CELL
   TrNode_entry                   ->   LFTH_NODE_ENTRY                      (DO NOT REPLACE ON CONFIGURATION STUFF)
   NumberOfLowTagBits             ->   LFTH_NrLowTagBits                    (DO NOT REPLACE ON CONFIGURATION STUFF)
   BOOL_CAS                       ->   LFTH_BOOL_CAS                        (DO NOT REPLACE ON lockFreeHash.tries.i)
   USES_REGS                      ->   LFTH_USES_REGS                       (DO NOT REPLACE ON CONFIGURATION STUFF)
   PASS_REGS                      ->   LFTH_PASS_REGS                       (DO NOT REPLACE ON CONFIGURATION STUFF)
   Term t                         ->   LFTH_NODE_ENTRY_STR t LFTH_USES_ARGS (DO NOT REPLACE ON CONFIGURATION STUFF)
   PASS_REGS                      ->   LFTH_PASS_ARGS PASS_REGS

####################################################################################


####################################################################################
External dependencies:
   check the memory manager
####################################################################################

static inline sg_node_ptr lfht_trie_check_insert_entry(sg_node_ptr parent_node, Term t USES_REGS) {
  sg_node_ptr child_node;
  child_node = (sg_node_ptr) TrNode_child(parent_node);
  if (child_node == NULL) {
    sg_node_ptr new_child_node;
    NEW_SUBGOAL_TRIE_NODE(new_child_node, t, NULL, parent_node, NULL);
    if (BOOL_CAS(&(TrNode_child(parent_node)), NULL, new_child_node))
      return new_child_node;
    FREE_SUBGOAL_TRIE_NODE(new_child_node);
    child_node = (sg_node_ptr) TrNode_child(parent_node);
  }
  
  if (!V04_IS_HASH(child_node))
    return subgoal_trie_check_insert_first_chain(child_node, parent_node, t, 0 PASS_REGS);
  return subgoal_trie_check_insert_bucket_array((sg_node_ptr *) child_node, parent_node, t, 0  PASS_REGS);
}
