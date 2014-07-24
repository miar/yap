#include "lockFreeHash.tries.h"

#ifdef INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE
#define LFHT_STR                   struct subgoal_trie_node
#define LFHT_STR_PTR               sg_node_ptr
#define LFHT_USES_ARGS1            sg_node_ptr parent_node, Term t
#define LFHT_PASS_ARGS             , parent_node, t
#define LFHT_ROOT_ADDR             (&(TrNode_child(parent_node)))
#define LFHT_GET_FIRST_NODE(NODE)  (NODE = (LFHT_STR_PTR) TrNode_child(parent_node))
#define LFHT_NEW_NODE(NODE, NEXT)  {NEW_SUBGOAL_TRIE_NODE(NODE, t, NULL, parent_node, NEXT);}
#define LFHT_FREE_NODE(PTR)        FREE_SUBGOAL_TRIE_NODE(PTR);
#undef LFHT_STR
#undef LFHT_STR_PTR
#undef LFHT_NEW_NODE
#undef LFHT_FREE_NODE
#undef LFHT_USES_ARGS1
#undef LFHT_PASS_ARGS
#undef LFHT_ROOT_ADDR
#undef LFHT_GET_FIRST_NODE
#endif /* INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE */

#ifdef INCLUDE_ANSWER_LOCK_FREE_HASH_TRIE
#define LFHT_STR                   struct answer_trie_node
#define LFHT_STR_PTR               ans_node_ptr
#define LFHT_USES_ARGS1            ans_node_ptr parent_node, Term t, int instr
#define LFHT_PASS_ARGS             , parent_node, t, instr
#define LFHT_ROOT_ADDR             (&(TrNode_child(parent_node)))
#define LFHT_GET_FIRST_NODE(NODE)  (NODE = (LFHT_STR_PTR) TrNode_child(parent_node))
#define LFHT_NEW_NODE(NODE, NEXT)  {NEW_ANSWER_TRIE_NODE(NODE, instr, t, NULL, parent_node, NEXT);}
#define LFHT_FREE_NODE(PTR)        FREE_ANSWER_TRIE_NODE(PTR)
#undef LFHT_STR
#undef LFHT_STR_PTR
#undef LFHT_NEW_NODE
#undef LFHT_FREE_NODE
#undef LFHT_USES_ARGS1
#undef LFHT_PASS_ARGS
#undef LFHT_ROOT_ADDR
#undef LFHT_GET_FIRST_NODE
#endif /* INCLUDE_ANSWER_LOCK_FREE_HASH_TRIE */

####################################################################################
query_replace (lockFreeHash.tries.h  + lockFreeHash.tries.i) :
   struct subgoal_trie_node       ->   LFHT_STR
   sg_node_ptr                    ->   LFHT_STR_PTR
   NEW_SUBGOAL_TRIE_NODE          ->   LFHT_NEW_NODE
   FREE_SUBGOAL_TRIE_NODE(PTR)    ->   LFHT_FREE_NODE
   BASE_HASH_BUCKETS              ->   LFHT_BUCKET_SIZE
   SHIFT_SIZE                     ->   LFHT_SHIFTS
   V04_                           ->   LFHT_
   long                           ->   LFHT_CELL
   TrNode_entry                   ->   LFHT_NODE_ENTRY                      (DO NOT REPLACE ON CONFIGURATION STUFF)
   NumberOfLowTagBits             ->   LFHT_NrLowTagBits                    (DO NOT REPLACE ON CONFIGURATION STUFF)
   BOOL_CAS                       ->   LFHT_BOOL_CAS                        (DO NOT REPLACE ON lockFreeHash.tries.i)
   USES_REGS                      ->   LFHT_USES_REGS                       (DO NOT REPLACE ON CONFIGURATION STUFF)
   PASS_REGS                      ->   LFHT_PASS_REGS                       (DO NOT REPLACE ON CONFIGURATION STUFF)
   Term t                         ->   LFHT_NODE_ENTRY_STR t LFHT_USES_ARGS (DO NOT REPLACE ON CONFIGURATION STUFF)
   PASS_REGS                      ->   LFHT_PASS_ARGS PASS_REGS

####################################################################################

####################################################################################
External dependencies:
   check the memory manager
####################################################################################
                           /* answer_trie_check_insert_entry */
static inline LFHT_STR_PTR lfht_check_insert_entry(LFHT_USES_ARGS1 LFHT_USES_REGS) {
  LFHT_STR_PTR first_node;
  LFHT_GET_FIRST_NODE(first_node);
  if (first_node == NULL) {
    LFHT_STR_PTR new_node;
    LFHT_NEW_NODE(new_node, NULL);
    if (LFHT_BOOL_CAS(LFHT_ROOT_ADDR, NULL, new_node))
      return new_node;
    LFHT_FREE_NODE(new_node);
    LFHT_GET_FIRST_NODE(first_node);
  }  
  if (LFHT_IS_HASH(first_node))
    return lfth_check_insert_bucket_array((LFHT_STR_PTR *) first_node, 0 LFHT_PASS_ARGS LFHT_PASS_REGS);
  return lfht_check_insert_first_chain(first_node, 0 LFHT_PASS_ARGS LFHT_PASS_REGS);
}

/***********************************************************ok upto here **************************/











