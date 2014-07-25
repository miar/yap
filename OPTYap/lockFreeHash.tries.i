#include "lockFreeHash.tries.h"

#ifdef INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE
#define LFHT_STR                          struct subgoal_trie_node
#define LFHT_STR_PTR                      sg_node_ptr
#define LFHT_USES_ARGS                    , sg_node_ptr parent_node LFHT_USES_REGS
#define LFHT_USES_ARGS1                   sg_node_ptr parent_node LFHT_USES_REGS
#define LFHT_PASS_ARGS                    , parent_node LFHT_PASS_REGS
#define LFHT_ROOT_ADDR                    (&(TrNode_child(parent_node)))
#define LFHT_NEW_NODE(NODE, ENTRY, NEXT)  {NEW_SUBGOAL_TRIE_NODE(NODE, ENTRY, NULL, parent_node, NEXT);}
#define LFHT_FREE_NODE(PTR)               FREE_SUBGOAL_TRIE_NODE(PTR);
#undef LFHT_STR
#undef LFHT_STR_PTR
#undef LFHT_NEW_NODE
#undef LFHT_FREE_NODE
#undef LFHT_USES_ARGS1
#undef LFHT_PASS_ARGS
#undef LFHT_ROOT_ADDR
#endif /* INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE */

#ifdef INCLUDE_ANSWER_LOCK_FREE_HASH_TRIE
#define LFHT_STR                          struct answer_trie_node
#define LFHT_STR_PTR                      ans_node_ptr
#define LFHT_USES_ARGS                    , ans_node_ptr parent_node, int instr LFHT_USES_REGS
#define LFHT_USES_ARGS1                   ans_node_ptr parent_node, int instr LFHT_USES_REGS
#define LFHT_PASS_ARGS                    , parent_node, instr LFHT_PASS_REGS
#define LFHT_ROOT_ADDR                    (&(TrNode_child(parent_node)))
#define LFHT_NEW_NODE(NODE, ENTRY, NEXT)  {NEW_ANSWER_TRIE_NODE(NODE, instr, ENTRY, NULL, parent_node, NEXT);}
#define LFHT_FREE_NODE(PTR)               FREE_ANSWER_TRIE_NODE(PTR)
#undef LFHT_STR
#undef LFHT_STR_PTR
#undef LFHT_NEW_NODE
#undef LFHT_FREE_NODE
#undef LFHT_USES_ARGS1
#undef LFHT_PASS_ARGS
#undef LFHT_ROOT_ADDR
#endif /* INCLUDE_ANSWER_LOCK_FREE_HASH_TRIE */


/* answer_trie_check_insert_entry */
static inline LFHT_STR_PTR lfht_check_insert_entry(LFHT_NODE_ENTRY_STR entry LFHT_USES_ARGS) {
  LFHT_STR_PTR first_node;
  LFHT_GetFirstNode(first_node);
  if (first_node == NULL) {
    LFHT_STR_PTR new_node;
    LFHT_NEW_NODE(new_node, entry, NULL);
    if (LFHT_BoolCAS(LFHT_ROOT_ADDR, NULL, new_node))
      return new_node;
    LFHT_FREE_NODE(new_node);
    LFHT_GetFirstNode(first_node);
  }  
  if (LFHT_IsHashLevel(first_node))
    return lfht_check_insert_bucket_array((LFHT_STR_PTR *) first_node, entry, 0 LFHT_PASS_ARGS);
  return lfht_check_insert_first_chain(first_node, entry, 0 LFHT_PASS_ARGS);
}

/***********************************************************ok upto here **************************/

/* answer_trie_check_insert_first_chain*/
static inline LFHT_STR_PTR lfht_check_insert_first_chain(LFHT_STR_PTR chain_node, LFHT_NODE_ENTRY_STR entry, int count_nodes LFHT_USES_ARGS) {
  if (LFHT_IsEqualEntry(chain_node, entry))
    return chain_node;  
  int cn = count_nodes + 1;
  LFHT_STR_PTR chain_next;
  chain_next = LFHT_NodeNext(chain_node); 

  if (chain_next && !LFHT_IsHashLevel(chain_next))
    return lfht_check_insert_first_chain(chain_next, entry, cn LFHT_PASS_ARGS); 
  
  // chain_next is a hash pointer or the end of the chain
  if (chain_next == NULL) {
    if (cn == LFHT_MAX_NODES_PER_BUCKET) {  
      LFHT_STR_PTR *new_hash;
      LFHT_STR_PTR *bucket;                                     
      LFHT_AllocBuckets(new_hash, NULL, LFHT_STR);
      new_hash = (LFHT_STR_PTR *) LFHT_TagAsHashLevel(new_hash);                      // HERE
      V04_GET_HASH_BUCKET(bucket, new_hash, TrNode_entry(chain_node), 0, LFHT_STR);
      V04_SET_HASH_BUCKET(bucket, chain_node, LFHT_STR);
      if (BOOL_CAS(&TrNode_next(chain_node), NULL, new_hash)) {
	answer_trie_adjust_chain_nodes(new_hash, TrNode_child(parent_node), chain_node, (- 1) PASS_REGS);
	TrNode_child(parent_node) = (ans_node_ptr) new_hash;
	return answer_trie_check_insert_bucket_array(new_hash, parent_node, t, instr, 0 PASS_REGS);
      } else 
	V04_FREE_TRIE_HASH_BUCKETS(new_hash, bucket, LFHT_STR);  
    } else {
      ans_node_ptr new_node; 
      NEW_ANSWER_TRIE_NODE(new_node, instr, t, NULL, parent_node, NULL);
      if (BOOL_CAS(&TrNode_next(chain_node), NULL, new_node)) 
	return new_node;    
      FREE_ANSWER_TRIE_NODE(new_node);
    }
    chain_next = TrNode_next(chain_node);
    if (!V04_IS_HASH(chain_next))
      return answer_trie_check_insert_first_chain(chain_next, parent_node, t, instr, cn PASS_REGS);  
  }
  // chain_next is pointig to an hash which is newer than mine. I must jump to the correct hash
  ans_node_ptr *jump_hash, *prev_hash;
  jump_hash = (ans_node_ptr *) chain_next;
  V04_GET_PREV_HASH(prev_hash, jump_hash, LFHT_STR);
  while (prev_hash != NULL) {
    jump_hash = prev_hash;
    V04_GET_PREV_HASH(prev_hash, jump_hash, LFHT_STR);
  }
  return answer_trie_check_insert_bucket_array(jump_hash, parent_node, t, instr, 0 PASS_REGS);  
} 



















