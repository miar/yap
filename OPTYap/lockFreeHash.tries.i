#include "lockFreeHash.tries.h"

#ifdef INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE
/* nonpersistent macros - macros used inside the lfht model */
#define LFHT_STR                          struct subgoal_trie_node
#define LFHT_STR_PTR                      sg_node_ptr
#define LFHT_USES_ARGS                    , LFHT_STR_PTR parent_node LFHT_USES_REGS
#define LFHT_PASS_ARGS                    , parent_node LFHT_PASS_REGS
#define LFHT_ROOT_ADDR                    (&(TrNode_child(parent_node)))
#define LFHT_NodeKey(NODE)                TrNode_entry(NODE)
#define LFHT_NodeNext(NODE)               TrNode_next(NODE)
#define LFHT_GetFirstNode(NODE)           (NODE = (LFHT_STR_PTR) TrNode_child(parent_node))
#define LFHT_FirstNode                    TrNode_child(parent_node)
#define LFHT_NEW_NODE(NODE, KEY, NEXT)    {NEW_SUBGOAL_TRIE_NODE(NODE, KEY, NULL, parent_node, NEXT);}
#define LFHT_FREE_NODE(PTR)               FREE_SUBGOAL_TRIE_NODE(PTR);
#define LFHT_CHECK_INSERT_KEY             subgoal_trie_check_insert_key
#define LFHT_CHECK_INSERT_FIRST_CHAIN     subgoal_trie_check_insert_first_chain
#define LFHT_CHECK_INSERT_BUCKET_ARRAY    subgoal_trie_check_insert_bucket_array
#define LFHT_CHECK_INSERT_BUCKET_CHAIN    subgoal_trie_check_insert_bucket_chain
#define LFHT_ADJUST_CHAIN_NODES           subgoal_trie_adjust_chain_nodes
#define LFHT_INSERT_BUCKET_ARRAY          subgoal_trie_insert_bucket_array
#define LFHT_INSERT_BUCKET_CHAIN          subgoal_trie_insert_bucket_chain
//#define LFHT_ABOLISH_ALL_KEYS             subgoal_trie_abolish_all_keys
//#define LFHT_ABOLISH_CHAIN                subgoal_trie_abolish_chain
//#define LFHT_ABOLISH_BUCKET_ARRAY         subgoal_trie_abolish_bucket_array

static inline LFHT_STR_PTR LFHT_CHECK_INSERT_KEY(LFHT_NODE_KEY_STR key LFHT_USES_ARGS);
static inline LFHT_STR_PTR LFHT_CHECK_INSERT_FIRST_CHAIN(LFHT_STR_PTR chain_node, LFHT_NODE_KEY_STR key, int count_nodes LFHT_USES_ARGS);
static inline LFHT_STR_PTR LFHT_CHECK_INSERT_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash,  LFHT_NODE_KEY_STR key, int n_shifts LFHT_USES_ARGS);
static inline LFHT_STR_PTR LFHT_CHECK_INSERT_BUCKET_CHAIN(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node,  LFHT_NODE_KEY_STR key, int n_shifts, int count_nodes  LFHT_USES_ARGS);
static inline void LFHT_ADJUST_CHAIN_NODES(LFHT_STR_PTR *new_hash, LFHT_STR_PTR chain_node, LFHT_STR_PTR last_node, int n_shifts LFHT_USES_REGS);
static inline void LFHT_INSERT_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node, int n_shifts LFHT_USES_REGS);
static inline void LFHT_INSERT_BUCKET_CHAIN(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node, LFHT_STR_PTR adjust_node, int n_shifts, int count_nodes LFHT_USES_REGS);
//static inline void LFHT_ABOLISH_ALL_KEYS(LFHT_STR_PTR parent_node);
//static inline void LFHT_ABOLISH_CHAIN(LFHT_STR_PTR chain_node, LFHT_STR_PTR * end_chain);
//static inline void LFHT_ABOLISH_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash);
#endif /* INCLUDE_SUBGOAL_LOCK_FREE_HASH_TRIE */

#ifdef INCLUDE_ANSWER_LOCK_FREE_HASH_TRIE

/* nonpersistent macros - macros used inside the lfht model */
#define LFHT_STR                          struct answer_trie_node
#define LFHT_STR_PTR                      ans_node_ptr
#define LFHT_USES_ARGS                    , LFHT_STR_PTR parent_node, int instr LFHT_USES_REGS
#define LFHT_PASS_ARGS                    , parent_node, instr LFHT_PASS_REGS
//#define LFHT_ABOLISH_USES_ARGS            , int mode, int position LFHT_USES_REGS
//#define LFHT_ABOLISH_PASS_ARGS            , mode, position LFHT_PASS_REGS

#define LFHT_ROOT_ADDR                    (&(TrNode_child(parent_node)))
#define LFHT_NodeKey(NODE)                TrNode_entry(NODE)
#define LFHT_NodeNext(NODE)               TrNode_next(NODE)
#define LFHT_GetFirstNode(NODE)           (NODE = (LFHT_STR_PTR) TrNode_child(parent_node))
#define LFHT_FirstNode                    TrNode_child(parent_node)
#define LFHT_NEW_NODE(NODE, KEY, NEXT)    {NEW_ANSWER_TRIE_NODE(NODE, instr, KEY, NULL, parent_node, NEXT);}
#define LFHT_FREE_NODE(PTR)               FREE_ANSWER_TRIE_NODE(PTR)
#define LFHT_CHECK_INSERT_KEY             answer_trie_check_insert_key
#define LFHT_CHECK_INSERT_FIRST_CHAIN     answer_trie_check_insert_first_chain
#define LFHT_CHECK_INSERT_BUCKET_ARRAY    answer_trie_check_insert_bucket_array
#define LFHT_CHECK_INSERT_BUCKET_CHAIN    answer_trie_check_insert_bucket_chain
#define LFHT_ADJUST_CHAIN_NODES           answer_trie_adjust_chain_nodes
#define LFHT_INSERT_BUCKET_ARRAY          answer_trie_insert_bucket_array
#define LFHT_INSERT_BUCKET_CHAIN          answer_trie_insert_bucket_chain
#define LFHT_ABOLISH_ALL_KEYS             free_answer_trie
#define LFHT_ABOLISH_CHAIN                free_answer_trie_chain
#define LFHT_ABOLISH_BUCKET_ARRAY         free_answer_trie_bucket_array

static inline LFHT_STR_PTR LFHT_CHECK_INSERT_KEY(LFHT_NODE_KEY_STR key LFHT_USES_ARGS);
static inline LFHT_STR_PTR LFHT_CHECK_INSERT_FIRST_CHAIN(LFHT_STR_PTR chain_node, LFHT_NODE_KEY_STR key, int count_nodes LFHT_USES_ARGS);
static inline LFHT_STR_PTR LFHT_CHECK_INSERT_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash,  LFHT_NODE_KEY_STR key, int n_shifts LFHT_USES_ARGS);
static inline LFHT_STR_PTR LFHT_CHECK_INSERT_BUCKET_CHAIN(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node,  LFHT_NODE_KEY_STR key, int n_shifts, int count_nodes  LFHT_USES_ARGS);
static inline void LFHT_ADJUST_CHAIN_NODES(LFHT_STR_PTR *new_hash, LFHT_STR_PTR chain_node, LFHT_STR_PTR last_node, int n_shifts LFHT_USES_REGS);
static inline void LFHT_INSERT_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node, int n_shifts LFHT_USES_REGS);
static inline void LFHT_INSERT_BUCKET_CHAIN(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node, LFHT_STR_PTR adjust_node, int n_shifts, int count_nodes LFHT_USES_REGS);


//static inline void LFHT_ABOLISH_ALL_KEYS(LFHT_STR_PTR first_node LFHT_ABOLISH_USES_ARGS);
//static inline void LFHT_ABOLISH_CHAIN(LFHT_STR_PTR chain_node, LFHT_STR_PTR *end_chain LFHT_ABOLISH_USES_ARGS);
//static inline void LFHT_ABOLISH_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash LFHT_ABOLISH_USES_ARGS);
#endif /* INCLUDE_ANSWER_LOCK_FREE_HASH_TRIE */

static inline LFHT_STR_PTR LFHT_CHECK_INSERT_KEY(LFHT_NODE_KEY_STR key LFHT_USES_ARGS) {
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
    return LFHT_CALL_CHECK_INSERT_BUCKET_ARRAY((LFHT_STR_PTR *) first_node, key, 0);
  return LFHT_CALL_CHECK_INSERT_FIRST_CHAIN(first_node, key, 0);
}

static inline LFHT_STR_PTR LFHT_CHECK_INSERT_FIRST_CHAIN(LFHT_STR_PTR chain_node, LFHT_NODE_KEY_STR key, int count_nodes LFHT_USES_ARGS) {
  if (LFHT_IsEqualKey(chain_node, key))
    return chain_node;  
  int cn = count_nodes + 1;
  LFHT_STR_PTR chain_next;
  chain_next = LFHT_NodeNext(chain_node); 
  if (chain_next && !LFHT_IsHashLevel(chain_next))
    return LFHT_CALL_CHECK_INSERT_FIRST_CHAIN(chain_next, key, cn); 
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
	LFHT_CALL_ADJUST_CHAIN_NODES(new_hash, LFHT_FirstNode, chain_node, (- 1));
	LFHT_FirstNode = (LFHT_STR_PTR) new_hash;
	return LFHT_CALL_CHECK_INSERT_BUCKET_ARRAY(new_hash, key, 0);
      } else
	LFHT_FreeBuckets(new_hash, bucket, LFHT_STR);
    } else {
      LFHT_STR_PTR new_node;
      LFHT_NEW_NODE(new_node, key, NULL);
      if (LFHT_BoolCAS(&(LFHT_NodeNext(chain_node)), NULL, new_node))
	return new_node;
      LFHT_FREE_NODE(new_node);
    }
    chain_next = LFHT_NodeNext(chain_node);
    if (!LFHT_IsHashLevel(chain_next))
      return LFHT_CALL_CHECK_INSERT_FIRST_CHAIN(chain_next, key, cn);
  }
  // chain_next is refering a deeper hash level. The worker must jump its hash level
  LFHT_STR_PTR *jump_hash, *prev_hash;
  jump_hash = (LFHT_STR_PTR *) chain_next;
  LFHT_GetPreviousHashLevel(prev_hash, jump_hash, LFHT_STR);
  while (prev_hash) {
    jump_hash = prev_hash;
    LFHT_GetPreviousHashLevel(prev_hash, jump_hash, LFHT_STR);
  }
  return LFHT_CALL_CHECK_INSERT_BUCKET_ARRAY(jump_hash, key, 0);
}

static inline LFHT_STR_PTR LFHT_CHECK_INSERT_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash,  LFHT_NODE_KEY_STR key, int n_shifts LFHT_USES_ARGS) {
  LFHT_STR_PTR *bucket;
  LFHT_GetBucket(bucket, curr_hash, key, n_shifts, LFHT_STR);
  if (LFHT_IsEmptyBucket(*bucket, curr_hash, LFHT_STR)) {
    LFHT_STR_PTR new_node;
    LFHT_NEW_NODE(new_node, key, (LFHT_STR_PTR) curr_hash);
    if (LFHT_BoolCAS(bucket, curr_hash, new_node))
      return new_node;
    LFHT_FREE_NODE(new_node);
  }
  LFHT_STR_PTR bucket_next = *bucket;
  if (LFHT_IsHashLevel(bucket_next))
    return LFHT_CALL_CHECK_INSERT_BUCKET_ARRAY((LFHT_STR_PTR *)bucket_next, key, (n_shifts + 1));
  return LFHT_CALL_CHECK_INSERT_BUCKET_CHAIN(curr_hash, bucket_next, key, n_shifts, 0);
}

static inline LFHT_STR_PTR LFHT_CHECK_INSERT_BUCKET_CHAIN(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node,  LFHT_NODE_KEY_STR key, int n_shifts, int count_nodes  LFHT_USES_ARGS) {
  if (LFHT_IsEqualKey(chain_node, key))
    return chain_node;
  int cn = count_nodes + 1;
  LFHT_STR_PTR chain_next;
  chain_next = LFHT_NodeNext(chain_node);
  if (!LFHT_IsHashLevel(chain_next))
    return LFHT_CALL_CHECK_INSERT_BUCKET_CHAIN(curr_hash, chain_next, key, n_shifts, cn);
  
  // chain_next is a hash pointer
  if ((LFHT_STR_PTR *)chain_next == curr_hash) {
    if (cn == LFHT_MAX_NODES_PER_BUCKET) {
      LFHT_STR_PTR *new_hash;
      LFHT_STR_PTR *bucket;
      LFHT_AllocBuckets(new_hash, curr_hash, LFHT_STR);
      new_hash = (LFHT_STR_PTR *) LFHT_TagAsHashLevel(new_hash);
      LFHT_GetBucket(bucket, new_hash, LFHT_NodeKey(chain_node), n_shifts + 1, LFHT_STR);
      LFHT_SetBucket(bucket, chain_node, LFHT_STR);
      if (LFHT_BoolCAS(&(LFHT_NodeNext(chain_node)), curr_hash, new_hash)) {
	LFHT_GetBucket(bucket, curr_hash, key, n_shifts, LFHT_STR);
	LFHT_CALL_ADJUST_CHAIN_NODES(new_hash, *bucket, chain_node, n_shifts);
        LFHT_SetBucket(bucket, new_hash, LFHT_STR);
	return LFHT_CALL_CHECK_INSERT_BUCKET_ARRAY(new_hash, key, (n_shifts + 1));
      } else 
	LFHT_FreeBuckets(new_hash, bucket, LFHT_STR);
    } else {
      LFHT_STR_PTR new_node;
      LFHT_NEW_NODE(new_node, key, (LFHT_STR_PTR) curr_hash);
      if (LFHT_BoolCAS(&(LFHT_NodeNext(chain_node)), curr_hash, new_node))
	return new_node;
      LFHT_FREE_NODE(new_node);
    }
    chain_next = LFHT_NodeNext(chain_node);
    if (!LFHT_IsHashLevel(chain_next))
      return LFHT_CALL_CHECK_INSERT_BUCKET_CHAIN(curr_hash, chain_next, key, n_shifts, cn);
  }
  // chain_next is refering a deeper hash level. The worker must jump its hash level
  LFHT_STR_PTR *jump_hash, *prev_hash;
  jump_hash = (LFHT_STR_PTR *) chain_next;
  LFHT_GetPreviousHashLevel(prev_hash, jump_hash, LFHT_STR);
  while (prev_hash != curr_hash) {
    jump_hash = prev_hash;
    LFHT_GetPreviousHashLevel(prev_hash, jump_hash, LFHT_STR);
  }
  return LFHT_CALL_CHECK_INSERT_BUCKET_ARRAY(jump_hash, key, (n_shifts + 1));
}

static inline void LFHT_ADJUST_CHAIN_NODES(LFHT_STR_PTR *new_hash, LFHT_STR_PTR chain_node, LFHT_STR_PTR last_node, int n_shifts LFHT_USES_REGS) {
  if (chain_node == last_node)
    return;
  LFHT_CALL_ADJUST_CHAIN_NODES(new_hash, LFHT_NodeNext(chain_node), last_node, n_shifts);
  return LFHT_CALL_INSERT_BUCKET_ARRAY(new_hash, chain_node, (n_shifts + 1));   
}

static inline void LFHT_INSERT_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node, int n_shifts LFHT_USES_REGS) {
  LFHT_STR_PTR *bucket;
  LFHT_NodeNext(chain_node) = (LFHT_STR_PTR) curr_hash;
  LFHT_GetBucket(bucket, curr_hash, LFHT_NodeKey(chain_node), n_shifts, LFHT_STR);
  if (LFHT_IsEmptyBucket(*bucket, curr_hash, LFHT_STR))
    if (LFHT_BoolCAS(bucket, curr_hash, chain_node))
      return;
  LFHT_STR_PTR bucket_next = *bucket;
  if (LFHT_IsHashLevel(bucket_next))
    return LFHT_CALL_INSERT_BUCKET_ARRAY((LFHT_STR_PTR *)bucket_next, chain_node, (n_shifts + 1));
  return LFHT_CALL_INSERT_BUCKET_CHAIN(curr_hash, bucket_next, chain_node, n_shifts, 0);
}

static inline void LFHT_INSERT_BUCKET_CHAIN(LFHT_STR_PTR *curr_hash, LFHT_STR_PTR chain_node, LFHT_STR_PTR adjust_node, int n_shifts, int count_nodes LFHT_USES_REGS) {
  LFHT_NODE_KEY_STR key = LFHT_NodeKey(adjust_node);
  int cn = count_nodes + 1;
  LFHT_STR_PTR chain_next;
  chain_next = LFHT_NodeNext(chain_node);
  if (!LFHT_IsHashLevel(chain_next))
    return LFHT_CALL_INSERT_BUCKET_CHAIN(curr_hash, chain_next, adjust_node, n_shifts, cn);
  // chain_next is a hash pointer
  if ((LFHT_STR_PTR *)chain_next == curr_hash) {
    if (cn == LFHT_MAX_NODES_PER_BUCKET) {
      LFHT_STR_PTR *new_hash;
      LFHT_STR_PTR *bucket;
      LFHT_AllocBuckets(new_hash, curr_hash, LFHT_STR);
      new_hash = (LFHT_STR_PTR *) LFHT_TagAsHashLevel(new_hash);
      LFHT_GetBucket(bucket, new_hash, LFHT_NodeKey(chain_node), n_shifts + 1, LFHT_STR);
      LFHT_SetBucket(bucket, chain_node, LFHT_STR);
      if (LFHT_BoolCAS(&(LFHT_NodeNext(chain_node)), curr_hash, new_hash)) {
	LFHT_GetBucket(bucket, curr_hash, key, n_shifts, LFHT_STR);
	LFHT_CALL_ADJUST_CHAIN_NODES(new_hash, *bucket, chain_node, n_shifts);
        LFHT_SetBucket(bucket, new_hash, LFHT_STR);
	return LFHT_CALL_INSERT_BUCKET_ARRAY(new_hash, adjust_node, (n_shifts + 1));
      } else
	LFHT_FreeBuckets(new_hash, bucket, LFHT_STR);
    } else {
      LFHT_NodeNext(adjust_node) = (LFHT_STR_PTR) curr_hash;
      if (LFHT_BoolCAS(&(LFHT_NodeNext(chain_node)), curr_hash, adjust_node))
	return;
    }
    chain_next = LFHT_NodeNext(chain_node);
    if (!LFHT_IsHashLevel(chain_next))
      return LFHT_CALL_INSERT_BUCKET_CHAIN(curr_hash, chain_next, adjust_node, n_shifts, cn);
  }
  // chain_next is refering a deeper hash level. The worker must jump its hash level
  LFHT_STR_PTR *jump_hash, *prev_hash;
  jump_hash = (LFHT_STR_PTR *) chain_next;
  LFHT_GetPreviousHashLevel(prev_hash, jump_hash, LFHT_STR);
  while (prev_hash != curr_hash) {
    jump_hash = prev_hash;
    LFHT_GetPreviousHashLevel(prev_hash, jump_hash, LFHT_STR);
  }
  return LFHT_CALL_INSERT_BUCKET_ARRAY(jump_hash, adjust_node, (n_shifts + 1));
}


/* ------------------------------------------------------------------------------------*/
/*                     abolish keys (removes all nodes inside the LFHT)                */
/* ------------------------------------------------------------------------------------*/

#define LFHT_ABOLISH_USES_ARGS            , int mode, int position LFHT_USES_REGS
#define LFHT_ABOLISH_PASS_ARGS            , mode, position LFHT_PASS_REGS

static inline void LFHT_ABOLISH_ALL_KEYS(LFHT_STR_PTR first_node LFHT_ABOLISH_USES_ARGS);
static inline void LFHT_ABOLISH_CHAIN(LFHT_STR_PTR chain_node, LFHT_STR_PTR *end_chain LFHT_ABOLISH_USES_ARGS);
static inline void LFHT_ABOLISH_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash LFHT_ABOLISH_USES_ARGS);

static inline void LFHT_ABOLISH_ALL_KEYS(LFHT_STR_PTR first_node LFHT_ABOLISH_USES_ARGS) {  
if (LFHT_IsHashLevel(first_node)) {
printf("is hash level %p \n", first_node);
    LFHT_CALL_ABOLISH_BUCKET_ARRAY((LFHT_STR_PTR *) first_node);
}else
    LFHT_CALL_ABOLISH_CHAIN(first_node, (LFHT_STR_PTR *)NULL);
  return;
}

 
static inline void LFHT_ABOLISH_CHAIN(LFHT_STR_PTR current_node, LFHT_STR_PTR *end_chain LFHT_ABOLISH_USES_ARGS) {

printf("current_node = %p end_chain %p \n", current_node, end_chain);
  if ((LFHT_STR_PTR *) current_node == end_chain)
    return;
  LFHT_ABOLISH_CHAIN(LFHT_NodeNext(current_node), end_chain , mode, TRAVERSE_POSITION_NEXT PASS_REGS);
/*  if (! IS_ANSWER_LEAF_NODE(current_node)) {  
    int child_mode = TRAVERSE_MODE_NORMAL; // nada
    LFHT_ABOLISH_ALL_KEYS(TrNode_child(current_node), child_mode, TRAVERSE_POSITION_FIRST PASS_REGS);
  }*/
  LFHT_FREE_NODE(current_node);
  return;
}

static inline void LFHT_ABOLISH_BUCKET_ARRAY(LFHT_STR_PTR *curr_hash LFHT_ABOLISH_USES_ARGS) {
  int i;
  LFHT_STR_PTR *bucket;
  bucket = (LFHT_STR_PTR *) LFHT_UntagHashLevel(curr_hash);
  for (i = 0; i < LFHT_BUCKET_ARRAY_SIZE ; i++) {
  if (LFHT_IsHashLevel((*bucket))) {
    printf("is hash level b %p\n", *bucket);
    LFHT_CALL_ABOLISH_BUCKET_ARRAY((LFHT_STR_PTR *) *bucket);
  } else
     LFHT_CALL_ABOLISH_CHAIN((LFHT_STR_PTR)*bucket, curr_hash);
    bucket++;
  }
  LFHT_FreeBuckets(curr_hash, NULL, NULL); 
  return;
}


/*
static inline void LFHT_ABOLISH_CHAIN(LFHT_STR_PTR current_node, LFHT_STR_PTR *end_chain LFHT_ABOLISH_USES_ARGS) {

  if (! IS_ANSWER_LEAF_NODE(current_node)) {  
    int child_mode;
    
    if (mode == TRAVERSE_MODE_NORMAL) {
      LFHT_NODE_KEY_STR t = LFHT_NodeKey(current_node);
      if (IsApplTerm(t)) {
	Functor f = (Functor) RepAppl(t);
	if (f == FunctorDouble)
	  child_mode = TRAVERSE_MODE_DOUBLE;
	else if (f == FunctorLongInt)
	  child_mode = TRAVERSE_MODE_LONGINT;
	else
	  child_mode = TRAVERSE_MODE_NORMAL;
      } else
	  child_mode = TRAVERSE_MODE_NORMAL;
    } else if (mode == TRAVERSE_MODE_LONGINT)
      child_mode = TRAVERSE_MODE_LONGINT_END;
    else if (mode == TRAVERSE_MODE_DOUBLE)
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
      child_mode = TRAVERSE_MODE_DOUBLE2;
    else if (mode == TRAVERSE_MODE_DOUBLE2)
#endif // SIZEOF_DOUBLE x SIZEOF_INT_P 
      child_mode = TRAVERSE_MODE_DOUBLE_END;
    else
      child_mode = TRAVERSE_MODE_NORMAL;
    
    LFHT_ABOLISH_ALL_KEYS(TrNode_child(current_node), child_mode, TRAVERSE_POSITION_FIRST PASS_REGS);
    //    free_answer_trie(TrNode_child(current_node), child_mode, TRAVERSE_POSITION_FIRST PASS_REGS);
  }
    if (position == TRAVERSE_POSITION_FIRST) {
    LFHT_STR_PTR next_node = LFHT_NodeNext(current_node);
    //  CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
    LFHT_FREE_NODE(current_node);
    while ((LFHT_STR_PTR *)next_node != end_chain) {
      current_node = next_node;
      next_node = LFHT_NodeNext(current_node);      
      LFHT_ABOLISH_CHAIN(current_node, end_chain, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
      //free_answer_trie(current_node, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
    }
  } else {
    CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
    LFHT_FREE_NODE(current_node);
  }
  return;
}
*/


#undef LFHT_STR
#undef LFHT_STR_PTR
#undef LFHT_USES_ARGS
#undef LFHT_PASS_ARGS
#undef LFHT_ABOLISH_USES_ARGS
#undef LFHT_ABOLISH_PASS_ARGS
#undef LFHT_ROOT_ADDR
#undef LFHT_NodeKey
#undef LFHT_NodeNext
#undef LFHT_GetFirstNode
#undef LFHT_FirstNode
#undef LFHT_NEW_NODE
#undef LFHT_FREE_NODE
#undef LFHT_CHECK_INSERT_KEY
#undef LFHT_CHECK_INSERT_FIRST_CHAIN
#undef LFHT_CHECK_INSERT_BUCKET_ARRAY
#undef LFHT_CHECK_INSERT_BUCKET_CHAIN
#undef LFHT_ADJUST_CHAIN_NODES
#undef LFHT_INSERT_BUCKET_ARRAY
#undef LFHT_INSERT_BUCKET_CHAIN
#undef LFHT_ABOLISH_ALL_KEYS
#undef LFHT_ABOLISH_CHAIN
#undef LFHT_ABOLISH_BUCKET_ARRAY
