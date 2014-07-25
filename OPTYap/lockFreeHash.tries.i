#include "lockFreeHash.tries.h"

#ifdef INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE
#define LFHT_STR                          struct subgoal_trie_node
#define LFHT_STR_PTR                      sg_node_ptr
#define LFHT_USES_ARGS                    , sg_node_ptr parent_node LFHT_USES_REGS
#define LFHT_USES_ARGS1                   sg_node_ptr parent_node LFHT_USES_REGS
#define LFHT_PASS_ARGS                    , parent_node LFHT_PASS_REGS
#define LFHT_ROOT_ADDR                    (&(TrNode_child(parent_node)))
#define LFHT_NEW_NODE(NODE, KEY, NEXT)  {NEW_SUBGOAL_TRIE_NODE(NODE, KEY, NULL, parent_node, NEXT);}
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
#define LFHT_NEW_NODE(NODE, KEY, NEXT)    {NEW_ANSWER_TRIE_NODE(NODE, instr, KEY, NULL, parent_node, NEXT);}
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
static inline LFHT_STR_PTR lfht_check_insert_key(LFHT_NODE_KEY_STR key LFHT_USES_ARGS) {
  LFHT_STR_PTR first_node;
  LFHT_GetFirstNode(first_node);
  if (first_node == NULL) {
    LFHT_STR_PTR new_node;
    LFHT_NEW_NODE(new_node, key, NULL);
    if (LFHT_BoolCAS(LFHT_ROOT_ADDR, NULL, new_node))
      return new_node;
    LFHT_FREE_NODE(new_node);
    LFHT_GetFirstNode(first_node);
  }  
  if (LFHT_IsHashLevel(first_node))
    return lfht_check_insert_bucket_array((LFHT_STR_PTR *) first_node, key, 0 LFHT_PASS_ARGS);
  return lfht_check_insert_first_chain(first_node, key, 0 LFHT_PASS_ARGS);
}

/* answer_trie_check_insert_first_chain*/
static inline LFHT_STR_PTR lfht_check_insert_first_chain(LFHT_STR_PTR chain_node, LFHT_NODE_KEY_STR key, int count_nodes LFHT_USES_ARGS) {
  if (LFHT_IsEqualKey(chain_node, key))
    return chain_node;  
  int cn = count_nodes + 1;
  LFHT_STR_PTR chain_next;
  chain_next = LFHT_NodeNext(chain_node); 
  if (chain_next && !LFHT_IsHashLevel(chain_next))
    return lfht_check_insert_first_chain(chain_next, key, cn LFHT_PASS_ARGS); 
  // chain_next refering the end of the chain or is a hash level pointer
  if (chain_next == NULL) {
    if (cn == LFHT_MAX_NODES_PER_BUCKET) {  
      LFHT_STR_PTR *new_hash;
      LFHT_STR_PTR *bucket;                                     
      LFHT_AllocBuckets(new_hash, NULL, LFHT_STR);
      new_hash = (LFHT_STR_PTR *) LFHT_TagAsHashLevel(new_hash);         
      LFHT_GetBucket(bucket, new_hash, LFHT_NodeKey(chain_node), 0, LFHT_STR);      
      LFHT_SetBucket(bucket, chain_node, LFHT_STR);                  
      if (LFHT_BoolCAS(&(LFHT_NodeNext(chain_node)), NULL, new_hash)) {                
	lfht_adjust_chain_nodes(new_hash, LFHT_FirstNode, chain_node, (- 1) LFHT_PASS_REGS); 
	LFHT_FirstNode = (LFHT_STR_PTR) new_hash;                                                  
	return lfht_check_insert_bucket_array(new_hash, key, 0 LFHT_PASS_ARGS);
      } else 
	LFHT_FreeBuckets(new_hash, bucket, LFHT_STR); 
    } else {      
      LFHT_STR_PTR new_node;                                       
      LFHT_NEW_NODE(new_node, key, NULL);
      if (LFHT_BoolCAS(&(LFHT_NodeNext(chain_node)), NULL, new_node))
	return new_node;
      LFHT_FREE_NODE((new_node);
    }
    chain_next = LFHT_NodeNext(chain_node);
    if (!LFHT_IsHashLevel(chain_next))
      return lfht_check_insert_first_chain(chain_next, key, cn LFHT_PASS_ARGS); 
  }                                                  
  // chain_next is refering a deeper hash level. The worker must jump its hash level
  LFHT_STR_PTR *jump_hash, *prev_hash;
  jump_hash = (LFHT_STR_PTR *) chain_next;                  
  LFHT_GetPreviousHashLevel(prev_hash, jump_hash, LFHT_STR);
  while (prev_hash) {
    jump_hash = prev_hash;
    LFHT_GetPreviousHashLevel(prev_hash, jump_hash, LFHT_STR);
  }
  return lfht_check_insert_bucket_array(jump_hash, key, 0 LFHT_PASS_ARGS);  
} 

/* subgoal_trie_check_insert_bucket_array */
static inline LFHT_STR_PTR lhft_check_insert_bucket_array(LFHT_STR_PTR *curr_hash,  LFHT_NODE_KEY_STR key, int n_shifts LFHT_USES_ARGS) {
  LFHT_STR_PTR *bucket;  
  LFHT_GetBucket((bucket, curr_hash, t, n_shifts, LFHT_STR);   
  if (LFHT_IsEmptyBucket(*bucket, curr_hash, LFHT_STR)) {  
    LFHT_STR_PTR new_node;     
    LFHT_NEW_NODE(new_node, key, (LFHT_STR_PTR) curr_hash);
    if (LFHT_BoolCAS(bucket, curr_hash, new_node))
      return new_node;
    LFHT_FREE_NODE((new_node);
  }
  LFHT_STR_PTR bucket_next = *bucket;
  if (LFHT_IsHashLevel(bucket_next))  
    return lfht_check_insert_bucket_array((LFHT_STR_PTR *)bucket_next, key, (n_shifts + 1) LFHT_PASS_ARGS); 
  return lfht_check_insert_bucket_chain(curr_hash, bucket_next, key, n_shifts, 0 LFHT_PASS_ARGS);
}

/***********************************************************ok upto here **************************/

// HERE

/* subgoal_trie_check_insert_bucket_chain */
static inline sg_node_ptr subgoal_trie_check_insert_bucket_chain(sg_node_ptr *curr_hash, sg_node_ptr chain_node, sg_node_ptr parent_node, Term t, long n_shifts, int count_nodes USES_REGS) {
  if (V04_IS_EQUAL_ENTRY(chain_node, t))
    return chain_node;  
  int cn = count_nodes + 1;
  sg_node_ptr chain_next;
  chain_next = TrNode_next(chain_node);
  if (!V04_IS_HASH(chain_next))
    return subgoal_trie_check_insert_bucket_chain(curr_hash, chain_next, parent_node, t, n_shifts, cn PASS_REGS);  
  
  // chain_next is a hash pointer
  if ((sg_node_ptr *)chain_next == curr_hash) {
    if (cn == MAX_NODES_PER_BUCKET) {
      sg_node_ptr *new_hash;
      sg_node_ptr *bucket;
      V04_ALLOC_BUCKETS(new_hash, curr_hash, struct subgoal_trie_node);
      new_hash = (sg_node_ptr *) V04_TAG(new_hash);
      V04_GET_HASH_BUCKET(bucket, new_hash, TrNode_entry(chain_node), n_shifts + 1, struct subgoal_trie_node);
      V04_SET_HASH_BUCKET(bucket, chain_node, struct subgoal_trie_node);
      if (BOOL_CAS(&TrNode_next(chain_node), curr_hash, new_hash)) {
	V04_GET_HASH_BUCKET(bucket, curr_hash, t, n_shifts, struct subgoal_trie_node);
	subgoal_trie_adjust_chain_nodes(new_hash, *bucket, chain_node, n_shifts PASS_REGS);
	V04_SET_HASH_BUCKET(bucket, new_hash, struct subgoal_trie_node);
	return subgoal_trie_check_insert_bucket_array(new_hash, parent_node, t, (n_shifts + 1) PASS_REGS);
      } else 
	V04_FREE_TRIE_HASH_BUCKETS(new_hash, bucket, struct subgoal_trie_node);  
    } else {
      sg_node_ptr new_node; 
      NEW_SUBGOAL_TRIE_NODE(new_node, t, NULL, parent_node, (sg_node_ptr) curr_hash);
      if (BOOL_CAS(&TrNode_next(chain_node), curr_hash, new_node)) 
	return new_node;      
      FREE_SUBGOAL_TRIE_NODE(new_node);
    }
    chain_next = TrNode_next(chain_node);
    if (!V04_IS_HASH(chain_next))
      return subgoal_trie_check_insert_bucket_chain(curr_hash, chain_next, parent_node, t, n_shifts, cn PASS_REGS);  
  }

  // chain_next is pointig to an hash which is newer than mine. I must jump to the correct hash
  sg_node_ptr *jump_hash, *prev_hash;
  jump_hash = (sg_node_ptr *) chain_next;
  V04_GET_PREV_HASH(prev_hash, jump_hash, struct subgoal_trie_node);
  while (prev_hash != curr_hash) {
    jump_hash = prev_hash;
    V04_GET_PREV_HASH(prev_hash, jump_hash, struct subgoal_trie_node);
  }
  return subgoal_trie_check_insert_bucket_array(jump_hash, parent_node, t, (n_shifts + 1) PASS_REGS);
}
















