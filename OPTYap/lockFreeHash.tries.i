#include "lockFreeHash.tries.h"

#ifdef INCLUDE_SUBGOAL_TRIE_LOCK_FREE_HASH_TRIE
#define LFH_TRIE_STR      struct subgoal_trie_node
#define LFH_TRIE_STR_PTR  sg_node_ptr

#define NEW_LFH_TRIE_NODE(NODE, _NOT_IN_USE, ENTRY, CHILD, PARENT, NEXT)  \
  { NEW_SUBGOAL_TRIE_NODE(NODE, ENTRY, CHILD, PARENT, NEXT); }

#define FREE_LFH_TRIE_NODE(PTR)
    FREE_SUBGOAL_TRIE_NODE(PTR);

#undef  LFH_TRIE_STR  
#undef  LFH_TRIE_STR_PTR
#undef  NEW_LFH_TRIE_NODE
#undef  FREE_LFH_TRIE_NODE
#endif /* INCLUDE_SUBGOAL_TRIE_LOCK_FREE_HASH_TRIE */


#ifdef INCLUDE_ANSWER_TRIE_LOCK_FREE_HASH_TRIE
#define LFH_TRIE_STR      struct answer_trie_node
#define LFH_TRIE_STR_PTR  ans_node_ptr

#define NEW_LFH_TRIE_NODE(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT)  \
  { NEW_ANSWER_TRIE_NODE(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT); }

#define FREE_LFH_TRIE_NODE(PTR)
    FREE_ANSWER_TRIE_NODE(PTR);

#undef LFH_TRIE_STR  
#undef LFH_TRIE_STR_PTR
#undef NEW_LFH_TRIE_NODE
#undef FREE_LFH_TRIE_NODE
#endif /* INCLUDE_ANSWER_TRIE_LOCK_FREE_HASH_TRIE */


####################################################################################
query_replace (lockFreeHash.tries.h  + lockFreeHash.tries.i) :
   struct subgoal_trie_node       ->   STR
   sg_node_ptr                    ->   STR_PTR
   NEW_SUBGOAL_TRIE_NODE          ->   NEW_LFH_TRIE_NODE
   FREE_SUBGOAL_TRIE_NODE(PTR)    ->   FREE_LFH_TRIE_NODE
   BASE_HASH_BUCKETS              ->   LFH_TRIE_BUCKETS
   SHIFT_SIZE                     ->   LFH_TRIE_SHIFTS
####################################################################################


####################################################################################
External dependencies:
   NumberOfLowTagBits

####################################################################################


#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline sg_node_ptr subgoal_trie_check_insert_gt_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#else
static inline sg_node_ptr subgoal_trie_check_insert_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */
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
