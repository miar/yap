/************************************************************************
**                                                                     **
**                   The YapTab/YapOr/OPTYap systems                   **
**                                                                     **
** YapTab extends the Yap Prolog engine to support sequential tabling  **
** YapOr extends the Yap Prolog engine to support or-parallelism       **
** OPTYap extends the Yap Prolog engine to support or-parallel tabling **
**                                                                     **
**                                                                     **
**      Yap Prolog was developed at University of Porto, Portugal      **
**                                                                     **
************************************************************************/

/*********************
**      Macros      **
*********************/
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
#define Init_threads_full_sharing_mode_directed_v02_fields(NODE)	\
        TrNode_intra_invalid_next(NODE) = NULL  
#else
#define Init_threads_full_sharing_mode_directed_v02_fields(NODE)
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */



#ifdef MODE_GLOBAL_TRIE_ENTRY
#define INCREMENT_GLOBAL_TRIE_REFERENCE(ENTRY)                                                          \
        { register gt_node_ptr entry_node = (gt_node_ptr) (ENTRY);                                      \
 	  TrNode_child(entry_node) = (gt_node_ptr) ((unsigned long int) TrNode_child(entry_node) + 1);  \
	}
#define NEW_SUBGOAL_TRIE_NODE(NODE, ENTRY, CHILD, PARENT, NEXT)        \
        INCREMENT_GLOBAL_TRIE_REFERENCE(ENTRY);                        \
        new_subgoal_trie_node(NODE, ENTRY, CHILD, PARENT, NEXT)
#define NEW_ANSWER_TRIE_NODE(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT)  \
        INCREMENT_GLOBAL_TRIE_REFERENCE(ENTRY);                        \
        new_answer_trie_node(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT); \
	Init_threads_full_sharing_mode_directed_v02_fields(NODE)

#define NEW_GLOBAL_TRIE_NODE(NODE, ENTRY, CHILD, PARENT, NEXT)         \
        INCREMENT_GLOBAL_TRIE_REFERENCE(ENTRY);                        \
        new_global_trie_node(NODE, ENTRY, CHILD, PARENT, NEXT)
#else
#define NEW_SUBGOAL_TRIE_NODE(NODE, ENTRY, CHILD, PARENT, NEXT)        \
        new_subgoal_trie_node(NODE, ENTRY, CHILD, PARENT, NEXT)
#define NEW_ANSWER_TRIE_NODE(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT)  \
        new_answer_trie_node(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT); \
  	Init_threads_full_sharing_mode_directed_v02_fields(NODE)

#define NEW_GLOBAL_TRIE_NODE(NODE, ENTRY, CHILD, PARENT, NEXT)         \
        new_global_trie_node(NODE, ENTRY, CHILD, PARENT, NEXT)
#endif /* MODE_GLOBAL_TRIE_ENTRY */


#ifdef MODE_GLOBAL_TRIE_LOOP
#define SUBGOAL_CHECK_INSERT_ENTRY(TAB_ENT, NODE, ENTRY)                           \
        NODE = global_trie_check_insert_entry(NODE, ENTRY PASS_REGS)
#define ANSWER_CHECK_INSERT_ENTRY(SG_FR, NODE, ENTRY, INSTR)                       \
        NODE = global_trie_check_insert_entry(NODE, ENTRY PASS_REGS)
#else
#define SUBGOAL_CHECK_INSERT_ENTRY(TAB_ENT, NODE, ENTRY)                           \
        NODE = subgoal_trie_check_insert_entry(TAB_ENT, NODE, ENTRY PASS_REGS)
#define ANSWER_CHECK_INSERT_ENTRY(SG_FR, NODE, ENTRY, INSTR)	                   \
        NODE = answer_trie_check_insert_entry(SG_FR, NODE, ENTRY, INSTR PASS_REGS)
#endif /* MODE_GLOBAL_TRIE_LOOP */



/************************************************************************
**                 subgoal_trie_check_insert_(gt)_entry                **
************************************************************************/

#ifdef INCLUDE_SUBGOAL_TRIE_CHECK_INSERT
#if !defined(SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL) && !defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL) /* SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL || SUBGOAL_TRIE_LOCK_AT_NODE_LEVEL || ! YAPOR */
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline sg_node_ptr subgoal_trie_check_insert_gt_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#else
static inline sg_node_ptr subgoal_trie_check_insert_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */
  sg_node_ptr child_node;

  LOCK_SUBGOAL_NODE(parent_node);
  child_node = TrNode_child(parent_node);
  if (child_node == NULL) {
    NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, NULL);
    TrNode_child(parent_node) = child_node;
    UNLOCK_SUBGOAL_NODE(parent_node);
    return child_node;
  }

  if (! IS_SUBGOAL_TRIE_HASH(child_node)) {
    int count_nodes = 0;
    do {
      if (TrNode_entry(child_node) == t) {
        UNLOCK_SUBGOAL_NODE(parent_node);
        return child_node;
      }
      count_nodes++;
      child_node = TrNode_next(child_node);
    } while (child_node);
    NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, TrNode_child(parent_node));
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      /* alloc a new hash */
      sg_hash_ptr hash;
      sg_node_ptr chain_node, next_node, *bucket;
      new_subgoal_trie_hash(hash, count_nodes, tab_ent);
      chain_node = child_node;
      do {
        bucket = Hash_buckets(hash) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
        next_node = TrNode_next(chain_node);
        TrNode_next(chain_node) = *bucket;
        *bucket = chain_node;
        chain_node = next_node;
      } while (chain_node);
      TrNode_child(parent_node) = (sg_node_ptr) hash;
    } else {
      TrNode_child(parent_node) = child_node;
    }
    UNLOCK_SUBGOAL_NODE(parent_node);
    return child_node;
  }

  { /* trie nodes with hashing */
    sg_hash_ptr hash;
    sg_node_ptr *bucket;
    int count_nodes = 0;
    hash = (sg_hash_ptr) child_node;
    bucket = Hash_buckets(hash) + HASH_ENTRY(t, Hash_num_buckets(hash));
    child_node = *bucket;
    while (child_node) {
      if (TrNode_entry(child_node) == t) {
        UNLOCK_SUBGOAL_NODE(parent_node);
        return child_node;
      }
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
    NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, *bucket);
    *bucket = child_node;
    Hash_num_nodes(hash)++;
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash) > Hash_num_buckets(hash)) {
      /* expand current hash */
      sg_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
      int num_buckets;
      num_buckets = Hash_num_buckets(hash) * 2;
      ALLOC_BUCKETS(new_hash_buckets, num_buckets);
      old_hash_buckets = Hash_buckets(hash);
      old_bucket = old_hash_buckets + Hash_num_buckets(hash);
      do {
        if (*--old_bucket) {
          chain_node = *old_bucket;
          do {
            bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
            next_node = TrNode_next(chain_node);
            TrNode_next(chain_node) = *bucket;
            *bucket = chain_node;
            chain_node = next_node;
          } while (chain_node);
        }
      } while (old_bucket != old_hash_buckets);
      Hash_buckets(hash) = new_hash_buckets;
      Hash_num_buckets(hash) = num_buckets;
      //      FREE_BUCKETS(old_hash_buckets);
    }
    UNLOCK_SUBGOAL_NODE(parent_node);
    return child_node;
  }
}
#elif defined(SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL)
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline sg_node_ptr subgoal_trie_check_insert_gt_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#else
static inline sg_node_ptr subgoal_trie_check_insert_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */
  sg_node_ptr child_node;
  sg_hash_ptr hash;
#ifdef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
  sg_node_ptr first_node = NULL; 
  int num_buckets = -1;
  sg_node_ptr *bucket = NULL;
 subgoal_trie_check_1:
#endif 

  child_node = TrNode_child(parent_node);
  if (child_node == NULL) {
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
    NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, NULL);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
#ifdef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    if (TRYLOCK_SUBGOAL_NODE(parent_node) != 0)
      goto subgoal_trie_check_1; 
#else
    LOCK_SUBGOAL_NODE(parent_node);
#endif
    if (TrNode_child(parent_node)) {
      sg_node_ptr chain_node = TrNode_child(parent_node);
      if (IS_SUBGOAL_TRIE_HASH(chain_node)) {
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
        FREE_SUBGOAL_TRIE_NODE(child_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
        UNLOCK_SUBGOAL_NODE(parent_node);
        hash = (sg_hash_ptr) chain_node;
        goto subgoal_trie_hash;
      }
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
          FREE_SUBGOAL_TRIE_NODE(child_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_SUBGOAL_NODE(parent_node);
          return chain_node;
        }
        chain_node = TrNode_next(chain_node);
      } while (chain_node);
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = TrNode_child(parent_node);
#else
      NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, TrNode_child(parent_node));
    } else {
      NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, NULL);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
    }
    TrNode_child(parent_node) = child_node;
    UNLOCK_SUBGOAL_NODE(parent_node);
    return child_node;
  } 

#ifdef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
 subgoal_trie_check_2:
#endif 

  if (! IS_SUBGOAL_TRIE_HASH(TrNode_child(parent_node))) {
    int count_nodes = 0;
#ifdef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    sg_node_ptr first_node_tmp = child_node;    
    while (child_node && child_node != first_node) {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
    first_node = first_node_tmp;
#else
    sg_node_ptr first_node = child_node;
    do {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    } while (child_node);
#endif
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
    NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, first_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
#ifdef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    if (TRYLOCK_SUBGOAL_NODE(parent_node) != 0) {
      child_node = TrNode_child(parent_node);
      goto subgoal_trie_check_2; 
    }
#else
    LOCK_SUBGOAL_NODE(parent_node);
#endif
    if (first_node != TrNode_child(parent_node)) {
      sg_node_ptr chain_node = TrNode_child(parent_node);
      if (IS_SUBGOAL_TRIE_HASH(chain_node)) {
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
        FREE_SUBGOAL_TRIE_NODE(child_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
        UNLOCK_SUBGOAL_NODE(parent_node);
        hash = (sg_hash_ptr) chain_node;
        goto subgoal_trie_hash;
      }
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
          FREE_SUBGOAL_TRIE_NODE(child_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_SUBGOAL_NODE(parent_node);
          return chain_node;
        }
        count_nodes++;
        chain_node = TrNode_next(chain_node);
      } while (chain_node != first_node);
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = TrNode_child(parent_node);
#else
      NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, TrNode_child(parent_node));
    } else {
      NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, first_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
    }
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      /* alloc a new hash */
      sg_node_ptr chain_node, next_node, *bucket;
      new_subgoal_trie_hash(hash, count_nodes, tab_ent);
      chain_node = child_node;
      do {
        bucket = Hash_buckets(hash) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
        next_node = TrNode_next(chain_node);
        TrNode_next(chain_node) = *bucket;
        *bucket = chain_node;
        chain_node = next_node;
      } while (chain_node);
      TrNode_child(parent_node) = (sg_node_ptr) hash;
    } else {
      TrNode_child(parent_node) = child_node;
    }
    UNLOCK_SUBGOAL_NODE(parent_node);
    return child_node;
  }

  hash = (sg_hash_ptr) child_node;
subgoal_trie_hash:
  { /* trie nodes with hashing */

#ifdef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    int count_nodes = 0;
    while (num_buckets != Hash_num_buckets(hash)) {
      num_buckets = Hash_num_buckets(hash);
      //      __sync_synchronize();
      bucket = Hash_buckets(hash) + HASH_ENTRY(t, num_buckets);
      child_node = *bucket;
      first_node = NULL;
    } 
    sg_node_ptr first_node_tmp = child_node;
    while (child_node && child_node != first_node) {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
    first_node = first_node_tmp;
#else
    sg_node_ptr *bucket, first_node;
    int num_buckets, count_nodes = 0;

    do {
      num_buckets = Hash_num_buckets(hash);
      //      __sync_synchronize();
      bucket = Hash_buckets(hash) + HASH_ENTRY(t, num_buckets);
      first_node = child_node = *bucket;
    } while (num_buckets != Hash_num_buckets(hash));
    while (child_node) {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
#endif
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
    NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, first_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
#ifdef SUBGOAL_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    if (TRYLOCK_SUBGOAL_NODE(parent_node) != 0){
      child_node = *bucket;	    
      goto subgoal_trie_hash; 
    }
#else
    LOCK_SUBGOAL_NODE(parent_node);
#endif
    if (num_buckets != Hash_num_buckets(hash)) {
      /* the hash has been expanded */ 
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
      FREE_SUBGOAL_TRIE_NODE(child_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
      UNLOCK_SUBGOAL_NODE(parent_node);
      goto subgoal_trie_hash;
    }
    if (first_node != *bucket) {
      sg_node_ptr chain_node = *bucket;
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
          FREE_SUBGOAL_TRIE_NODE(child_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_SUBGOAL_NODE(parent_node);
          return chain_node;
        }
        count_nodes++;
        chain_node = TrNode_next(chain_node);
      } while (chain_node != first_node);
#ifdef SUBGOAL_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = *bucket;
#else
      NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, *bucket);
    } else {
      NEW_SUBGOAL_TRIE_NODE(child_node, t, NULL, parent_node, first_node);
#endif /* SUBGOAL_TRIE_ALLOC_BEFORE_CHECK */
    }
    *bucket = child_node;
    Hash_num_nodes(hash)++;
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash) > Hash_num_buckets(hash)) {
      /* expand current hash */ 
      sg_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
      num_buckets = Hash_num_buckets(hash) * 2;
      ALLOC_BUCKETS(new_hash_buckets, num_buckets);
      old_hash_buckets = Hash_buckets(hash);
      old_bucket = old_hash_buckets + Hash_num_buckets(hash);
      do {
        if (*--old_bucket) {
          chain_node = *old_bucket;
          do {
            bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
            next_node = TrNode_next(chain_node);
            TrNode_next(chain_node) = *bucket;
            *bucket = chain_node;
            chain_node = next_node;
          } while (chain_node);
        }
      } while (old_bucket != old_hash_buckets);
      Hash_buckets(hash) = new_hash_buckets;
      Hash_num_buckets(hash) = num_buckets;
      //      FREE_BUCKETS(old_hash_buckets);
    }
    UNLOCK_SUBGOAL_NODE(parent_node);
    return child_node;
  }
}

#elif defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V01)
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline sg_node_ptr subgoal_trie_check_insert_gt_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#else
static inline sg_node_ptr subgoal_trie_check_insert_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */

  sg_node_ptr child_node, first_node, new_child_node = NULL, *bucket;
  int count_nodes = 0;
  first_node = child_node = (sg_node_ptr) TrNode_child(parent_node);

  if (child_node == NULL || !IS_SUBGOAL_TRIE_HASH(child_node)) {
    while (child_node) {
      if (TrNode_entry(child_node) == t)
	return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
    NEW_SUBGOAL_TRIE_NODE(new_child_node, t, NULL, parent_node, first_node);


    while (!BOOL_CAS(&(TrNode_child(parent_node)), first_node, new_child_node)) {
      sg_node_ptr first_node_tmp;
      first_node_tmp = child_node = (sg_node_ptr) TrNode_child(parent_node);
      if (IS_SUBGOAL_TRIE_HASH(child_node))
	goto subgoal_trie_hash;            
      count_nodes = 0;
      while (child_node && child_node != first_node) {
	if (TrNode_entry(child_node) == t) {
	  FREE_SUBGOAL_TRIE_NODE(new_child_node); 
	  return child_node;
	}
	count_nodes++;
	child_node = TrNode_next(child_node);
      }
      first_node = TrNode_next(new_child_node) = first_node_tmp;
    }
    child_node = new_child_node;    
    count_nodes++;
    
    if (count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      sg_node_ptr chain_node , next_node;
      sg_hash_ptr hash_node;
      new_subgoal_trie_hash_atomic_v01(hash_node, count_nodes);
      if (!BOOL_CAS(&(TrNode_child(parent_node)), child_node, (sg_node_ptr)hash_node)){
	FREE_BUCKETS(SgHash_hash_bkts(hash_node));
	FREE_SUBGOAL_TRIE_HASH(hash_node);
	return child_node;
      }
      // alloc a new hash
      SgHash_init_chain_fields(hash_node, tab_ent);
      chain_node = child_node;
      do {
	bucket = SgHash_buckets(hash_node) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
	next_node = TrNode_next(chain_node);
	TrNode_next(chain_node) = (sg_node_ptr)((long)*bucket & ~(long)0x1);
	*bucket = (sg_node_ptr)((long)chain_node | (long)0x1);
	chain_node = next_node;
      } while (chain_node);
      bucket = SgHash_buckets(hash_node) +  BASE_HASH_BUCKETS;

      do {
	bucket--;
	*bucket = (sg_node_ptr)((long)*bucket & ~(long)0x1);
      } while (bucket != SgHash_buckets(hash_node));
      OPEN_HASH(hash_node); 
    }
    return child_node;
  }

 subgoal_trie_hash:
  {
    int num_buckets = 0; 
    sg_hash_ptr hash_node = (sg_hash_ptr) child_node;
    sg_hash_bkts_ptr hash = SgHash_hash_bkts(hash_node);
    num_buckets = HashBkts_number_of_buckets(hash);
    bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
    child_node = *bucket; 

    while (IS_NEW_HASH_REF(child_node)){
      hash = (sg_hash_bkts_ptr) ((long)(*bucket) & ~(long)0x3);
      num_buckets = HashBkts_number_of_buckets(hash);
      bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
      child_node = *bucket; 
    }

    child_node  = (sg_node_ptr)((long) child_node & ~(long)0x1);
    while (child_node) {
      if (TrNode_entry(child_node) == t) {
	if (new_child_node != NULL)
	  FREE_SUBGOAL_TRIE_NODE(new_child_node); 
	return child_node;    
      }
      child_node = TrNode_next(child_node);    
    }
    
    if (new_child_node == NULL) {
      NEW_SUBGOAL_TRIE_NODE(new_child_node, t, NULL, parent_node, NULL);
    }

    first_node = NULL;    
    
    do {
      count_nodes = 0;
      child_node = *bucket; 
      
      while (IS_NEW_HASH_REF(child_node)){
	hash = (sg_hash_bkts_ptr) ((long)(*bucket) & ~(long)0x3);
	num_buckets = HashBkts_number_of_buckets(hash);
	bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
	child_node = *bucket; 
	first_node = NULL;
      }
      
      child_node  = (sg_node_ptr)((long) child_node &  ~(long)0x1);

      sg_node_ptr first_node_tmp = child_node;
      
      
      while (child_node && child_node != first_node) {
	if (TrNode_entry(child_node) == t){
	  FREE_SUBGOAL_TRIE_NODE(new_child_node);
	  return child_node;	
	}
	count_nodes++;
	child_node = TrNode_next(child_node);
      }
      
      first_node = TrNode_next(new_child_node) = first_node_tmp;
      
    } while(!BOOL_CAS(bucket, first_node, new_child_node));
    child_node = new_child_node;
    count_nodes++; 
    
    Inc_HashNode_num_nodes(hash_node);   
    
    int hash_num_nodes = Hash_num_nodes(hash_node) >> 1;
    if (count_nodes >= MAX_NODES_PER_BUCKET && hash_num_nodes > SgHash_num_buckets(hash_node)) {
      if (BOOL_CAS(&(Hash_num_nodes(hash_node)), (hash_num_nodes << 1), CLOSE_HASH(hash_num_nodes))) {
	// ok for expanding the current hash 
	sg_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
	sg_hash_bkts_ptr new_hash;
	num_buckets = SgHash_num_buckets(hash_node) * 2;
	ALLOC_OPEN_HASH_BUCKETS(new_hash, new_hash_buckets, num_buckets, struct subgoal_trie_node, struct subgoal_trie_hash_buckets);
	old_hash_buckets = SgHash_buckets(hash_node);
	old_bucket = old_hash_buckets + SgHash_num_buckets(hash_node);
	do {
	  --old_bucket; 	  // getting the old_bucket value and closing the bucket
	  chain_node = VAL_CAS(old_bucket,*old_bucket, CLOSE_BUCKET(*old_bucket));
	  while(chain_node) {
	    bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
	    next_node = TrNode_next(chain_node);
	    TrNode_next(chain_node) = (sg_node_ptr) READ_BUCKET_PTR(bucket);
	    *bucket = chain_node;
	    chain_node = next_node;
	  }
	  NEW_HASH_REF(old_bucket, new_hash, struct subgoal_trie_node);	
	} while (old_bucket != old_hash_buckets);

	HashBkts_next(SgHash_hash_bkts(hash_node)) = SgHash_old_hash_bkts(hash_node);
	SgHash_old_hash_bkts(hash_node) = SgHash_hash_bkts(hash_node);

	SgHash_hash_bkts(hash_node) = new_hash;
	OPEN_HASH(hash_node); 
      }
    }     
    return child_node;    
  }
}
#elif defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03)
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline sg_node_ptr subgoal_trie_check_insert_gt_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#else
static inline sg_node_ptr subgoal_trie_check_insert_entry(tab_ent_ptr tab_ent, sg_node_ptr parent_node, Term t USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */

  sg_node_ptr child_node, first_node, new_child_node = NULL, *bucket;
  int count_nodes = 0;
  first_node = child_node = (sg_node_ptr) TrNode_child(parent_node);
  
  if (child_node == NULL || !IS_SUBGOAL_TRIE_HASH(child_node)) {
    while (child_node) {
      if (!IS_SUBGOAL_TRIE_HASH_EXPANSION(child_node)) {
	if (TrNode_entry(child_node) == t) 
	  return child_node;
	count_nodes++;
      }  else {
	// cache coherence (check same code on answer trie)
	__sync_synchronize(); 	   
	child_node = (sg_node_ptr) TrNode_child(parent_node); 	    	
	goto subgoal_trie_hash;
      }
      child_node = TrNode_next(child_node);
    }

    NEW_SUBGOAL_TRIE_NODE(new_child_node, t, NULL, parent_node, first_node);
    
    while (!BOOL_CAS(&(TrNode_child(parent_node)), first_node, new_child_node)) {
      sg_node_ptr first_node_tmp;
      first_node_tmp = child_node = (sg_node_ptr) TrNode_child(parent_node);
      if (IS_SUBGOAL_TRIE_HASH(child_node))
	goto subgoal_trie_hash;            

      while (child_node && child_node != first_node) {
	if (!IS_SUBGOAL_TRIE_HASH_EXPANSION(child_node)) {
	  if (TrNode_entry(child_node) == t) {
	    FREE_SUBGOAL_TRIE_NODE(new_child_node); 
	    return child_node;
	  }
	  count_nodes++;
	} else {
	  // cache coherence (test_large_joins_join2 fails without it)
	  __sync_synchronize(); 	   
	  child_node = (sg_node_ptr) TrNode_child(parent_node); 	    	
	  goto subgoal_trie_hash;
	}
	child_node = TrNode_next(child_node);
      }
      first_node = TrNode_next(new_child_node) = first_node_tmp;
    }    
    child_node = new_child_node;
    count_nodes++;
    
    if (count_nodes == MAX_NODES_PER_TRIE_LEVEL) {
      sg_node_ptr chain_node , next_node, exp_node;
      sg_hash_ptr hash_node;
      struct subgoal_trie_node **new_hash_buckets;
      new_subgoal_trie_hash_atomic_v03(exp_node, hash_node, new_hash_buckets, 0, tab_ent, child_node);
      chain_node = child_node;
      while (!BOOL_CAS(&(TrNode_child(parent_node)), chain_node, (sg_node_ptr)hash_node)) {
	chain_node = TrNode_next(exp_node) = TrNode_child(parent_node);
      }

      // alloc a new hash
      count_nodes = adjust_subgoal_hash_nodes_first_exp(chain_node, new_hash_buckets, 0);
      Add_HashNode_num_nodes_v03(hash_node, count_nodes);

      // remove the the expansion node from all buckets
      bucket = new_hash_buckets + BASE_HASH_BUCKETS;
      do {
	bucket--;
	if (BOOL_CAS(bucket, exp_node, NULL))
	  continue;	
	chain_node = *bucket;
	while(chain_node) {
	  next_node = TrNode_next(chain_node);
	  if (next_node == exp_node) {
	    TrNode_next(chain_node) =  NULL;
	    break;
	  }
	  chain_node = next_node;
	}
      } while (bucket != new_hash_buckets);
      OPEN_SG_HASH_V03(hash_node);
    }
    return child_node;
  }
  
 subgoal_trie_hash:
  {
    int num_buckets = 0;
    sg_hash_ptr hash_node = (sg_hash_ptr) child_node;
    sg_hash_bkts_ptr hash = (sg_hash_bkts_ptr) ((CELL) SgHash_hash_bkts(hash_node) & ~(CELL)0x1);
    num_buckets = HashBkts_number_of_buckets(hash);
    bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
    child_node = *bucket; 
    sg_node_ptr next_node = NULL;
    
    while (IS_NEW_HASH_REF_V03(child_node)){
      hash = (sg_hash_bkts_ptr) ((CELL)(*bucket) & ~(CELL)0x1);
      num_buckets = HashBkts_number_of_buckets(hash);
      bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
      child_node = *bucket; 
    }
    
    int expNodeVisited = 0; 
    while (child_node) {
      if (!IS_SUBGOAL_TRIE_HASH_EXPANSION(child_node)) {
	if (TrNode_entry(child_node) == t) {  
	  if (new_child_node != NULL) 
	    FREE_SUBGOAL_TRIE_NODE(new_child_node);
	  return child_node;    
	}
      }  else {
	if (expNodeVisited == 1)
	  break;
	else
	  expNodeVisited = 1;	
      }       
      child_node = TrNode_next(child_node);    
    }
    
    if (new_child_node == NULL) {
      NEW_SUBGOAL_TRIE_NODE(new_child_node, t, NULL, parent_node, NULL);
    }
    
    first_node = NULL;
    expNodeVisited = 0;
    sg_node_ptr first_node_tmp = NULL;
    do {
      count_nodes = 0;
      child_node = *bucket; 
      
      while (IS_NEW_HASH_REF_V03(child_node)){
	hash = (sg_hash_bkts_ptr) ((CELL)(*bucket) & ~(CELL)0x1);
	num_buckets = HashBkts_number_of_buckets(hash);
	bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
	expNodeVisited = 0;   // lock-freedom - 2
	child_node = *bucket; 
	first_node = NULL;
      }
      
      first_node_tmp = child_node;

      while (child_node != first_node) {
	if (!IS_SUBGOAL_TRIE_HASH_EXPANSION(child_node)) {
	  if (TrNode_entry(child_node) == t) {
	    FREE_SUBGOAL_TRIE_NODE(new_child_node);
	    return child_node;	
	  }	  
	  count_nodes++;
	} else {
	  if (expNodeVisited == 1)
	    break;
	  else
	    expNodeVisited = 1;	
	}
	child_node = TrNode_next(child_node);	  
      }

      if (first_node_tmp && !IS_SUBGOAL_TRIE_HASH_EXPANSION(first_node_tmp))
	first_node = first_node_tmp;	      
      TrNode_next(new_child_node) = first_node_tmp;
    } while(!BOOL_CAS(bucket, first_node_tmp, new_child_node));

    child_node = new_child_node;
    count_nodes++; 
    
    Inc_HashNode_num_nodes_v03(hash_node);

    hash = (sg_hash_bkts_ptr) ((CELL) SgHash_hash_bkts(hash_node) & ~(CELL)0x1);
    
    if (count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash_node) > SgHash_num_buckets(hash_node)) {
      if (BOOL_CAS(&(SgHash_hash_bkts(hash_node)), hash, CLOSE_SG_HASH_V03(hash))) {	
	sg_node_ptr chain_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
	sg_node_ptr exp_node;
	sg_hash_bkts_ptr new_hash;

	old_hash_buckets = HashBkts_buckets(hash);
	int old_number_buckets = HashBkts_number_of_buckets(hash);

	// ok for expanding the current hash 
	num_buckets = old_number_buckets * 2;

	ALLOC_OPEN_HASH_BUCKETS(new_hash, new_hash_buckets, num_buckets, struct subgoal_trie_node, struct subgoal_trie_hash_buckets);
	exp_node = Hash_sg_exp_node(hash_node);
	int i = 0;
	while(i < old_number_buckets) {
	  old_bucket = old_hash_buckets + i;
	  if (BOOL_CAS(old_bucket, NULL, (sg_node_ptr)((CELL)new_hash | (CELL)0x1))) { 
	    i++;
	    continue;
	  }
	  // we have at least one node on the bucket
	  
	  sg_node_ptr *new_bucket_1, *new_bucket_2;
	  new_bucket_1 = new_hash_buckets + i;
	  new_bucket_2 = new_hash_buckets + i + old_number_buckets;
	  *new_bucket_1 = *new_bucket_2 = exp_node;

	  do
	    TrNode_next(exp_node) = first_node = *old_bucket;
	  while(!BOOL_CAS(old_bucket, first_node, (sg_node_ptr)((CELL)new_hash | (CELL)0x1))); 
	  	  
	  // putting the nodes of the old hash in place on the new hash
	  chain_node = first_node;
	  adjust_subgoal_hash_nodes(chain_node, new_hash_buckets, num_buckets); 

	  // removing the link of both buckets to the exp_node
	  if (!BOOL_CAS(new_bucket_1, exp_node, NULL)) { 
	    chain_node = *new_bucket_1;
	    next_node = TrNode_next(chain_node);
	    while(next_node != exp_node) {
	      chain_node = next_node;
	      next_node = TrNode_next(chain_node);
	    }
	    TrNode_next(chain_node) = NULL;	      
	  }

	  if (!BOOL_CAS(new_bucket_2, exp_node, NULL)) { 
	    chain_node = *new_bucket_2;
	    next_node = TrNode_next(chain_node);
	    while(next_node != exp_node) {
	      chain_node = next_node;
	      next_node = TrNode_next(chain_node);
	    }
	    TrNode_next(chain_node) = NULL;	      
	  }	  
	  TrNode_next(exp_node) = NULL;
	  i++;
	}
	
	HashBkts_next(hash) = SgHash_old_hash_bkts(hash_node);
	SgHash_old_hash_bkts(hash_node) = hash;
	SgHash_hash_bkts(hash_node) = new_hash; // open hash
      }
    }
    return child_node;    
  }
}
#endif /* SUBGOAL_TRIE_LOCK_LEVEL */
#endif /* INCLUDE_SUBGOAL_TRIE_CHECK_INSERT */



/************************************************************************
**                 answer_trie_check_insert_(gt)_entry                 **
************************************************************************/

#ifdef INCLUDE_ANSWER_TRIE_CHECK_INSERT
#if !defined(ANSWER_TRIE_LOCK_AT_WRITE_LEVEL) && !defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL) /* ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL || ANSWER_TRIE_LOCK_AT_NODE_LEVEL || ! YAPOR */
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline ans_node_ptr answer_trie_check_insert_gt_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#else
static inline ans_node_ptr answer_trie_check_insert_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */
  ans_node_ptr child_node;

  TABLING_ERROR_CHECKING(answer_trie_check_insert_(gt)_entry, IS_ANSWER_LEAF_NODE(parent_node));
  LOCK_ANSWER_NODE(parent_node);
  child_node = TrNode_child(parent_node);
  if (child_node == NULL) {
    NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, NULL);
    TrNode_child(parent_node) = child_node;
    UNLOCK_ANSWER_NODE(parent_node);
    return child_node;
  }

  if (! IS_ANSWER_TRIE_HASH(child_node)) {
    int count_nodes = 0;
    do {
      if (TrNode_entry(child_node) == t) {
        UNLOCK_ANSWER_NODE(parent_node);
        return child_node;
      }
      count_nodes++;
      child_node = TrNode_next(child_node);
    } while (child_node);
    NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, TrNode_child(parent_node));
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      /* alloc a new hash */
      ans_hash_ptr hash;
      ans_node_ptr chain_node, next_node, *bucket;
      new_answer_trie_hash(hash, count_nodes, sg_fr);
      chain_node = child_node;
      do {
        bucket = Hash_buckets(hash) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
        next_node = TrNode_next(chain_node);
        TrNode_next(chain_node) = *bucket;
        *bucket = chain_node;
        chain_node = next_node;
      } while (chain_node);
      TrNode_child(parent_node) = (ans_node_ptr) hash;
    } else {
      TrNode_child(parent_node) = child_node;
    }
    UNLOCK_ANSWER_NODE(parent_node);
    return child_node;
  }

  { /* trie nodes with hashing */
    ans_hash_ptr hash;
    ans_node_ptr *bucket;
    int count_nodes = 0;
    hash = (ans_hash_ptr) child_node;
    bucket = Hash_buckets(hash) + HASH_ENTRY(t, Hash_num_buckets(hash));
    child_node = *bucket;
    while (child_node) {
      if (TrNode_entry(child_node) == t) {
        UNLOCK_ANSWER_NODE(parent_node);
        return child_node;
      }
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
    NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, *bucket);
    *bucket = child_node;
    Hash_num_nodes(hash)++;
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash) > Hash_num_buckets(hash)) {
      /* expand current hash */ 
      ans_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
      int num_buckets;
      num_buckets = Hash_num_buckets(hash) * 2;
      ALLOC_BUCKETS(new_hash_buckets, num_buckets);
      old_hash_buckets = Hash_buckets(hash);
      old_bucket = old_hash_buckets + Hash_num_buckets(hash);
      do {
        if (*--old_bucket) {
          chain_node = *old_bucket;
          do {
            bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
            next_node = TrNode_next(chain_node);
            TrNode_next(chain_node) = *bucket;
            *bucket = chain_node;
            chain_node = next_node;
          } while (chain_node);
        }
      } while (old_bucket != old_hash_buckets);
      Hash_buckets(hash) = new_hash_buckets;
      Hash_num_buckets(hash) = num_buckets;
      //      FREE_BUCKETS(old_hash_buckets);
    }
    UNLOCK_ANSWER_NODE(parent_node);
    return child_node;
  }
}
#elif defined(ANSWER_TRIE_LOCK_AT_WRITE_LEVEL)
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline ans_node_ptr answer_trie_check_insert_gt_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#else
static inline ans_node_ptr answer_trie_check_insert_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */
  ans_node_ptr child_node;
  ans_hash_ptr hash;
#ifdef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
  ans_node_ptr first_node = NULL; 
  int num_buckets = -1;
  ans_node_ptr *bucket = NULL;
 answer_trie_check_1:
#endif 
  TABLING_ERROR_CHECKING(answer_trie_check_insert_(gt)_entry, IS_ANSWER_LEAF_NODE(parent_node));
  child_node = TrNode_child(parent_node);
  if (child_node == NULL) {
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
    NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, NULL);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
#ifdef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    if (TRYLOCK_ANSWER_NODE(parent_node) != 0)
      goto answer_trie_check_1; 
#else
    LOCK_ANSWER_NODE(parent_node);
#endif
    if (TrNode_child(parent_node)) {
      ans_node_ptr chain_node = TrNode_child(parent_node);
      if (IS_ANSWER_TRIE_HASH(chain_node)) {
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
        FREE_ANSWER_TRIE_NODE(child_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
        UNLOCK_ANSWER_NODE(parent_node);
        hash = (ans_hash_ptr) chain_node;
        goto answer_trie_hash;
      }
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
          FREE_ANSWER_TRIE_NODE(child_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_ANSWER_NODE(parent_node);
          return chain_node;
        }
        chain_node = TrNode_next(chain_node);
      } while (chain_node);
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = TrNode_child(parent_node);
#else
      NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, TrNode_child(parent_node));
    } else {
      NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, NULL);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
    }
    TrNode_child(parent_node) = child_node;
    UNLOCK_ANSWER_NODE(parent_node);
    return child_node;
  } 

#ifdef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
 answer_trie_check_2:
#endif 

  if (! IS_ANSWER_TRIE_HASH(TrNode_child(parent_node))) {
    int count_nodes = 0;
#ifdef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    ans_node_ptr first_node_tmp = child_node;    
    while (child_node && child_node != first_node) {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
    first_node = first_node_tmp;
#else
    ans_node_ptr first_node = child_node;
    do {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    } while (child_node);
#endif
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
    NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, first_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
#ifdef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    if (TRYLOCK_ANSWER_NODE(parent_node) != 0) {
      child_node = TrNode_child(parent_node);
      goto answer_trie_check_2; 
    }
#else
    LOCK_ANSWER_NODE(parent_node);
#endif
    if (first_node != TrNode_child(parent_node)) {
      ans_node_ptr chain_node = TrNode_child(parent_node);
      if (IS_ANSWER_TRIE_HASH(chain_node)) {
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
        FREE_ANSWER_TRIE_NODE(child_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
        UNLOCK_ANSWER_NODE(parent_node);
        hash = (ans_hash_ptr) chain_node; 
        goto answer_trie_hash;
      }
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
          FREE_ANSWER_TRIE_NODE(child_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_ANSWER_NODE(parent_node);
          return chain_node;
        }
        count_nodes++;
        chain_node = TrNode_next(chain_node);
      } while (chain_node != first_node);
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = TrNode_child(parent_node);
#else
      NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, TrNode_child(parent_node));
    } else {
      NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, first_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
    }
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      /* alloc a new hash */
      ans_node_ptr chain_node, next_node, *bucket;
      new_answer_trie_hash(hash, count_nodes, sg_fr);
      chain_node = child_node;
      do {
        bucket = Hash_buckets(hash) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
        next_node = TrNode_next(chain_node);
        TrNode_next(chain_node) = *bucket;
        *bucket = chain_node;
        chain_node = next_node;
      } while (chain_node);
      TrNode_child(parent_node) = (ans_node_ptr) hash;
    } else {
      TrNode_child(parent_node) = child_node;
    }
    UNLOCK_ANSWER_NODE(parent_node);
    return child_node;
  }

  hash = (ans_hash_ptr) child_node;
answer_trie_hash:
  { /* trie nodes with hashing */

#ifdef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    int count_nodes = 0;
    while (num_buckets != Hash_num_buckets(hash)) {
      num_buckets = Hash_num_buckets(hash);
      //      __sync_synchronize();
      bucket = Hash_buckets(hash) + HASH_ENTRY(t, num_buckets);
      child_node = *bucket;
      first_node = NULL;
    } 
    ans_node_ptr first_node_tmp = child_node;
    while (child_node && child_node != first_node) {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
    first_node = first_node_tmp;
#else
    ans_node_ptr *bucket, first_node;
    int num_buckets, count_nodes = 0;

    do {
      num_buckets = Hash_num_buckets(hash);
      //      __sync_synchronize();
      bucket = Hash_buckets(hash) + HASH_ENTRY(t, num_buckets);
      first_node = child_node = *bucket;
    } while (num_buckets != Hash_num_buckets(hash));
    while (child_node) {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
#endif
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
    NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, first_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
#ifdef ANSWER_TRIE_LOCK_AT_WRITE_LEVEL_USING_TRY_LOCKS
    if (TRYLOCK_ANSWER_NODE(parent_node) != 0) {
      child_node = *bucket;	    
      goto answer_trie_hash; 
    }
#else
    LOCK_ANSWER_NODE(parent_node);
#endif 
    if (num_buckets != Hash_num_buckets(hash)) {
      /* the hash has been expanded */ 
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
      FREE_ANSWER_TRIE_NODE(child_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
      UNLOCK_ANSWER_NODE(parent_node);
      goto answer_trie_hash;
    }
    if (first_node != *bucket) {
      ans_node_ptr chain_node = *bucket;
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
          FREE_ANSWER_TRIE_NODE(child_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_ANSWER_NODE(parent_node);
          return chain_node;
        }
        count_nodes++;
        chain_node = TrNode_next(chain_node);
      } while (chain_node != first_node);
#ifdef ANSWER_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = *bucket;
#else
      NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, *bucket);
    } else {
      NEW_ANSWER_TRIE_NODE(child_node, instr, t, NULL, parent_node, first_node);
#endif /* ANSWER_TRIE_ALLOC_BEFORE_CHECK */
    }
    *bucket = child_node;
    Hash_num_nodes(hash)++;
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash) > Hash_num_buckets(hash)) {
      /* expand current hash */
      ans_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
      num_buckets = Hash_num_buckets(hash) * 2;
      ALLOC_BUCKETS(new_hash_buckets, num_buckets);
      old_hash_buckets = Hash_buckets(hash);
      old_bucket = old_hash_buckets + Hash_num_buckets(hash);
      do {
        if (*--old_bucket) {
          chain_node = *old_bucket;
          do {
            bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
            next_node = TrNode_next(chain_node);
            TrNode_next(chain_node) = *bucket;
            *bucket = chain_node;
            chain_node = next_node;
          } while (chain_node);
        }
      } while (old_bucket != old_hash_buckets);
      Hash_buckets(hash) = new_hash_buckets;
      Hash_num_buckets(hash) = num_buckets;
      //      FREE_BUCKETS(old_hash_buckets);
    }
    UNLOCK_ANSWER_NODE(parent_node);
    return child_node;
  }
}

#elif defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V01)
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline ans_node_ptr answer_trie_check_insert_gt_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#else
static inline ans_node_ptr answer_trie_check_insert_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */
  ans_node_ptr child_node, first_node, new_child_node = NULL, *bucket;
  int count_nodes = 0;
  first_node = child_node = (ans_node_ptr) TrNode_child(parent_node);

  if (child_node == NULL || !IS_ANSWER_TRIE_HASH(child_node)) {
    while (child_node) {
      if (TrNode_entry(child_node) == t)
	return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
    NEW_ANSWER_TRIE_NODE(new_child_node, instr, t, NULL, parent_node, first_node);

    while (!BOOL_CAS(&(TrNode_child(parent_node)), first_node, new_child_node)) {
      ans_node_ptr first_node_tmp;
      first_node_tmp = child_node = (ans_node_ptr) TrNode_child(parent_node);
      if (IS_ANSWER_TRIE_HASH(child_node)) {
	goto answer_trie_hash;            
      }
      count_nodes = 0;
      while (child_node && child_node != first_node) {
	if (TrNode_entry(child_node) == t) {
	  FREE_ANSWER_TRIE_NODE(new_child_node); 
	  return child_node;
	}
	count_nodes++;
	child_node = TrNode_next(child_node);
      }
      first_node = TrNode_next(new_child_node) = first_node_tmp;
    }
    child_node = new_child_node;    
    count_nodes++;
    
    if (count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      ans_node_ptr chain_node , next_node;
      ans_hash_ptr hash_node;
      new_answer_trie_hash_atomic_v01(hash_node, count_nodes);
      if (!BOOL_CAS(&(TrNode_child(parent_node)), child_node, (ans_node_ptr)hash_node)){
	FREE_BUCKETS(AnsHash_hash_bkts(hash_node));
	FREE_ANSWER_TRIE_HASH(hash_node);
	return child_node;
      }
      // alloc a new hash
      AnsHash_init_chain_fields(hash_node, sg_fr);
      chain_node = child_node;
      do {
	bucket = AnsHash_buckets(hash_node) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
	next_node = TrNode_next(chain_node);
	TrNode_next(chain_node) = (ans_node_ptr)((long)*bucket & ~(long)0x1);
	*bucket = (ans_node_ptr)((long)chain_node | (long)0x1);
	chain_node = next_node;
      } while (chain_node);
      bucket = AnsHash_buckets(hash_node) +  BASE_HASH_BUCKETS;

      do {
	bucket--;
	*bucket = (ans_node_ptr)((long)*bucket & ~(long)0x1);
      } while (bucket != AnsHash_buckets(hash_node));
      OPEN_HASH(hash_node); 
    }
    return child_node;
  }

 answer_trie_hash:
  {

    int num_buckets = 0; 
    ans_hash_ptr hash_node = (ans_hash_ptr) child_node;
    ans_hash_bkts_ptr hash = AnsHash_hash_bkts(hash_node);
    num_buckets = HashBkts_number_of_buckets(hash);
    bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
    child_node = *bucket; 
    
    while (IS_NEW_HASH_REF(child_node)){
      hash = (ans_hash_bkts_ptr) ((long)(*bucket) & ~(long)0x3);
      num_buckets = HashBkts_number_of_buckets(hash);
      bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
      child_node = *bucket; 
    }
    
    child_node  = (ans_node_ptr)((long) child_node & ~(long)0x1);
    while (child_node) {
      if (TrNode_entry(child_node) == t) {
	if (new_child_node != NULL)
	  FREE_ANSWER_TRIE_NODE(new_child_node); 
	return child_node;    
      }
      child_node = TrNode_next(child_node);    
    }
    
    if (new_child_node == NULL) {
      NEW_ANSWER_TRIE_NODE(new_child_node, instr, t, NULL, parent_node, NULL);
    }
    
    first_node = NULL;    
    
    do {
      count_nodes = 0;
      child_node = *bucket; 
      
      while (IS_NEW_HASH_REF(child_node)){
        hash = (ans_hash_bkts_ptr) ((long)(*bucket) & ~(long)0x3);
        num_buckets = HashBkts_number_of_buckets(hash);
	bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
	child_node = *bucket; 
	first_node = NULL;
      }
      
      child_node  = (ans_node_ptr)((long) child_node &  ~(long)0x1);
      
      ans_node_ptr first_node_tmp = child_node;
      
      while (child_node && child_node != first_node) {
	if (TrNode_entry(child_node) == t){
	  FREE_ANSWER_TRIE_NODE(new_child_node);
	  return child_node;	
	}
	count_nodes++;
	child_node = TrNode_next(child_node);
      }
      
      first_node = TrNode_next(new_child_node) = first_node_tmp;
      
    } while(!BOOL_CAS(bucket, first_node, new_child_node)); 
    
    child_node = new_child_node;
    count_nodes++; 
    
    Inc_HashNode_num_nodes(hash_node);   
    
    int hash_num_nodes = Hash_num_nodes(hash_node) >> 1;
    if (count_nodes >= MAX_NODES_PER_BUCKET && hash_num_nodes > AnsHash_num_buckets(hash_node)) {
      if (BOOL_CAS(&(Hash_num_nodes(hash_node)), (hash_num_nodes << 1), CLOSE_HASH(hash_num_nodes))) {
	// ok for expanding the current hash 
	ans_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
	ans_hash_bkts_ptr new_hash;
	num_buckets = AnsHash_num_buckets(hash_node) * 2;	
	ALLOC_OPEN_HASH_BUCKETS(new_hash, new_hash_buckets, num_buckets, struct answer_trie_node, struct answer_trie_hash_buckets);
	old_hash_buckets = AnsHash_buckets(hash_node);
	old_bucket = old_hash_buckets + AnsHash_num_buckets(hash_node);
	do {
	  --old_bucket; 	  // getting the old_bucket value and closing the bucket
	  chain_node = VAL_CAS(old_bucket,*old_bucket, CLOSE_BUCKET(*old_bucket));
	  while(chain_node) {
	    bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
	    next_node = TrNode_next(chain_node);
	    TrNode_next(chain_node) = (ans_node_ptr) READ_BUCKET_PTR(bucket);
	    *bucket = chain_node;
	    chain_node = next_node;
	  }
	  NEW_HASH_REF(old_bucket, new_hash, struct answer_trie_node);	
	} while (old_bucket != old_hash_buckets);

	HashBkts_next(AnsHash_hash_bkts(hash_node)) = AnsHash_old_hash_bkts(hash_node);
	AnsHash_old_hash_bkts(hash_node) = AnsHash_hash_bkts(hash_node);

	AnsHash_hash_bkts(hash_node) = new_hash;
	OPEN_HASH(hash_node); 
      }
    } 
    
    return child_node;    
  }
}

#elif defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02)
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline ans_node_ptr answer_trie_check_insert_gt_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#else
static inline ans_node_ptr answer_trie_check_insert_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */

  ans_node_ptr child_node, first_node, new_child_node = NULL, *bucket;
  int count_nodes = 0;
  first_node = child_node = (ans_node_ptr) TrNode_child(parent_node);

  if (child_node == NULL || !IS_ANSWER_TRIE_HASH(child_node)) {
    while (child_node) {
      if (TrNode_entry(child_node) == t) {
	if (IS_ANSWER_TRIE_HASH_EXPANSION(child_node))
	  return TrNode_parent(child_node);
	else
	  return child_node;
      }
      if (!IS_ANSWER_TRIE_HASH_EXPANSION(child_node))
	count_nodes++;
      child_node = TrNode_next(child_node);
    }
    NEW_ANSWER_TRIE_NODE(new_child_node, instr, t, NULL, parent_node, first_node);

    while (!BOOL_CAS(&(TrNode_child(parent_node)), first_node, new_child_node)) {
      ans_node_ptr first_node_tmp;
      first_node_tmp = child_node = (ans_node_ptr) TrNode_child(parent_node);
      if (IS_ANSWER_TRIE_HASH(child_node))
	goto answer_trie_hash;            
      count_nodes = 0;
      while (child_node && child_node != first_node) {
	if (TrNode_entry(child_node) == t) {
	  FREE_ANSWER_TRIE_NODE(new_child_node); 
	  if (IS_ANSWER_TRIE_HASH_EXPANSION(child_node))
	    return TrNode_parent(child_node);
	  else
	    return child_node;
	}
	if (!IS_ANSWER_TRIE_HASH_EXPANSION(child_node))
	  count_nodes++;
	child_node = TrNode_next(child_node);
      }
      first_node = TrNode_next(new_child_node) = first_node_tmp;
    }
    
    child_node = new_child_node; 
        
    count_nodes++;
    
    if (!IS_ANSWER_TRIE_HASH(TrNode_child(parent_node)) && count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      ans_node_ptr chain_node , next_node, exp_nodes;
      ans_hash_ptr hash_node;
      new_answer_trie_hash_exp_nodes(exp_nodes, hash_node, count_nodes, sg_fr, child_node);      
      if (!BOOL_CAS(&(TrNode_child(parent_node)), child_node, (ans_node_ptr)hash_node)) {
	FREE_EXPANSION_NODES(exp_nodes, ans_node_ptr);
	FREE_BUCKETS(AnsHash_hash_bkts(hash_node));			
	FREE_ANSWER_TRIE_HASH(hash_node);
	return child_node;
      } 
      // alloc a new hash
      AnsHash_init_chain_fields(hash_node, sg_fr);
      chain_node = child_node;
      do {
	bucket = AnsHash_buckets(hash_node) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
	next_node = TrNode_next(chain_node);	
	do 
	  TrNode_next(chain_node) = *bucket;
	while (!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
	chain_node = next_node;
      } while (chain_node);
      bucket = AnsHash_buckets(hash_node) +  BASE_HASH_BUCKETS;
      
      do {
	bucket--;
	chain_node = *bucket;
	if (IS_ANSWER_TRIE_HASH_EXPANSION(chain_node)) {
	  if (BOOL_CAS(bucket, chain_node, NULL))
	    continue;
	  else
	    chain_node = *bucket;
	}    
	while(chain_node) {
	  next_node = TrNode_next(chain_node);
	  if (next_node && TrNode_instr(next_node) == ANSWER_TRIE_HASH_EXPANSION_MARK) {
	    TrNode_next(chain_node) =  NULL;
	    break;
	  }
	  chain_node = next_node;
	}
      } while (bucket != AnsHash_buckets(hash_node));
      Hash_unused_exp_nodes(hash_node) = exp_nodes;
      Hash_exp_nodes(hash_node) = Hash_unused_exp_nodes(hash_node); // open hash
    }
    return child_node;
  }
  
 answer_trie_hash:
  {
    int num_buckets = 0;
    ans_hash_ptr hash_node = (ans_hash_ptr) child_node;
    ans_hash_bkts_ptr hash = AnsHash_hash_bkts(hash_node);
    num_buckets = HashBkts_number_of_buckets(hash);
    bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
    child_node = *bucket; 
    
    while (IS_NEW_HASH_REF_V02(child_node)){
      hash = (ans_hash_bkts_ptr) ((CELL)(*bucket) & ~(CELL)0x1);
      num_buckets = HashBkts_number_of_buckets(hash);
      bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
      child_node = *bucket; 
    }
    
    while (child_node) {
      if (TrNode_entry(child_node) == t) {
	if (new_child_node != NULL) 
	  FREE_ANSWER_TRIE_NODE(new_child_node);
	if (IS_ANSWER_TRIE_HASH_EXPANSION(child_node))
	  return TrNode_parent(child_node);
	else
	  return child_node;    
      }
      child_node = TrNode_next(child_node);    
    }
    
    if (new_child_node == NULL) {
      NEW_ANSWER_TRIE_NODE(new_child_node, instr, t, NULL, parent_node, NULL);
    }
    
    first_node = NULL;    
    
    do {
      count_nodes = 0;
      child_node = *bucket; 
      
      while (IS_NEW_HASH_REF_V02(child_node)){
	hash = (ans_hash_bkts_ptr) ((CELL)(*bucket) & ~(CELL)0x1);
	num_buckets = HashBkts_number_of_buckets(hash);
	bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
	child_node = *bucket; 
	first_node = NULL;
      }
      
      ans_node_ptr first_node_tmp = child_node;      
      
      while (child_node && child_node != first_node) {
	if (TrNode_entry(child_node) == t){
	  FREE_ANSWER_TRIE_NODE(new_child_node);
	  if (IS_ANSWER_TRIE_HASH_EXPANSION(child_node))
	    return TrNode_parent(child_node);
	  else
	    return child_node;	
	}
	if (!IS_ANSWER_TRIE_HASH_EXPANSION(child_node))
	  count_nodes++;
	child_node = TrNode_next(child_node);
      }
      
      first_node = TrNode_next(new_child_node) = first_node_tmp;
      
    } while(!BOOL_CAS(bucket, first_node, new_child_node));
    child_node = new_child_node;
    count_nodes++; 
    
    Inc_HashNode_num_nodes_v02(hash_node);
    ans_node_ptr exp_nodes = Hash_exp_nodes(hash_node);

    if (exp_nodes != NULL && count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash_node) > AnsHash_num_buckets(hash_node)) {      
      if (BOOL_CAS(&(Hash_exp_nodes(hash_node)), exp_nodes, NULL)) {
	// ok for expanding the current hash 
	ans_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
	ans_hash_bkts_ptr new_hash;
	num_buckets = AnsHash_num_buckets(hash_node) * 2;
	ALLOC_OPEN_HASH_BUCKETS(new_hash, new_hash_buckets, num_buckets, struct answer_trie_node, struct answer_trie_hash_buckets);
	old_hash_buckets = AnsHash_buckets(hash_node);
	int i = 0;
	
	while(i < AnsHash_num_buckets(hash_node)) {
	  old_bucket = old_hash_buckets + i;
	  ans_node_ptr *new_bucket_1, *new_bucket_2;
	  new_bucket_1 = new_hash_buckets + i;
	  new_bucket_2 = new_hash_buckets + i + AnsHash_num_buckets(hash_node);

	  first_node = NULL;	 
	  // creating the expansion nodes
	  do {
	    ans_node_ptr first_node_tmp;
	    first_node_tmp = chain_node = *old_bucket;
	    while(chain_node != first_node) {	      
	      Hash_unused_exp_nodes(hash_node) = TrNode_next(exp_nodes);
	      TrNode_entry(exp_nodes) = TrNode_entry(chain_node);
	      TrNode_child(exp_nodes) = TrNode_child(chain_node);
	      TrNode_parent(exp_nodes) = chain_node;
	      TrNode_next(exp_nodes)  = *new_bucket_1;
	      *new_bucket_1 = *new_bucket_2 = exp_nodes;
	      if (Hash_unused_exp_nodes(hash_node) != NULL)
                exp_nodes = Hash_unused_exp_nodes(hash_node);
	      else {
                ALLOC_ANSWER_TRIE_NODE(exp_nodes);
                TrNode_instr(exp_nodes) = ANSWER_TRIE_HASH_EXPANSION_MARK;
                TrNode_next(exp_nodes)  = NULL;
		Hash_unused_exp_nodes(hash_node) = exp_nodes;
              }
	      chain_node = TrNode_next(chain_node);
	    }
	    first_node = first_node_tmp;	    
	  } while(!BOOL_CAS(old_bucket, first_node, (ans_node_ptr)((CELL)new_hash | (CELL)0x1)));
	  
	  // putting hash nodes in place if they exist
	  if (first_node) {
	    chain_node = first_node;	  
	    while (chain_node) {
	      first_node = TrNode_next(first_node);
	      bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
	      do 
		TrNode_next(chain_node) = *bucket;
	      while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
	      chain_node = first_node;
	    }
	    
	    // removing the expansion nodes
	    chain_node = *new_bucket_1;
	    if (IS_ANSWER_TRIE_HASH_EXPANSION(chain_node)) {
	      if (BOOL_CAS(new_bucket_1, chain_node, NULL)) {
		// chaining the expansion nodes
		first_node = chain_node;
		while(TrNode_next(chain_node) != NULL)
		  chain_node = TrNode_next(chain_node);		
		TrNode_next(chain_node) = Hash_unused_exp_nodes(hash_node);
		Hash_unused_exp_nodes(hash_node) = first_node;    
		chain_node = NULL;
	      } else
		chain_node = *new_bucket_1;
	    }    
	    
	    while (chain_node) {
	      next_node = TrNode_next(chain_node);
	      if(IS_ANSWER_TRIE_HASH_EXPANSION(next_node)) {
		TrNode_next(chain_node) =  NULL;
		// chaining the expansion nodes
		chain_node = next_node;
		while(TrNode_next(chain_node) != NULL) 
		  chain_node = TrNode_next(chain_node);
		
		TrNode_next(chain_node) = Hash_unused_exp_nodes(hash_node); 
		Hash_unused_exp_nodes(hash_node) = next_node;
		break;
	      }
	      chain_node = next_node;
	    }
	    
	    chain_node = *new_bucket_2;
	    
	    if (IS_ANSWER_TRIE_HASH_EXPANSION(chain_node)) {
	      if (BOOL_CAS(new_bucket_2, chain_node, NULL))
		chain_node = NULL;
	      else
		chain_node = *new_bucket_2;
	    }    
	    
	    while (chain_node) {
	      next_node = TrNode_next(chain_node);
	      if(IS_ANSWER_TRIE_HASH_EXPANSION(next_node)) {
		TrNode_next(chain_node) =  NULL;
		break;
	      }
	      chain_node = next_node;
	    }
	    exp_nodes =  Hash_unused_exp_nodes(hash_node);
	  }
	  i++;
	}
	
	HashBkts_next(AnsHash_hash_bkts(hash_node)) = AnsHash_old_hash_bkts(hash_node);
	AnsHash_old_hash_bkts(hash_node) = AnsHash_hash_bkts(hash_node);

	AnsHash_hash_bkts(hash_node) = new_hash;
	Hash_exp_nodes(hash_node) = Hash_unused_exp_nodes(hash_node); // open hash
      }
    }
    return child_node;    
  }
}

#elif defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03)
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline ans_node_ptr answer_trie_check_insert_gt_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#else
static inline ans_node_ptr answer_trie_check_insert_entry(sg_fr_ptr sg_fr, ans_node_ptr parent_node, Term t, int instr USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */

  ans_node_ptr child_node, first_node, new_child_node = NULL, *bucket;
  int count_nodes = 0;
  first_node = child_node = (ans_node_ptr) TrNode_child(parent_node);
  
  if (child_node == NULL || !IS_ANSWER_TRIE_HASH(child_node)) {

    while (child_node) {
      if (!IS_ANSWER_TRIE_HASH_EXPANSION(child_node)){
	if (TrNode_entry(child_node) == t) 
	  return child_node;
	count_nodes++;
      }  else {
	// cache coherence (test_large_joins_join2 fails without it)
	__sync_synchronize(); 	   
	child_node = (ans_node_ptr) TrNode_child(parent_node); 	    	
	goto answer_trie_hash;
      }
      child_node = TrNode_next(child_node);
    }

    NEW_ANSWER_TRIE_NODE(new_child_node, instr, t, NULL, parent_node, first_node);
    
    while (!BOOL_CAS(&(TrNode_child(parent_node)), first_node, new_child_node)) {
      ans_node_ptr first_node_tmp;
      first_node_tmp = child_node = (ans_node_ptr) TrNode_child(parent_node);
      if (IS_ANSWER_TRIE_HASH(child_node))
	goto answer_trie_hash;            

      while (child_node && child_node != first_node) {
	if (!IS_ANSWER_TRIE_HASH_EXPANSION(child_node)) {
	  if (TrNode_entry(child_node) == t) {
	    FREE_ANSWER_TRIE_NODE(new_child_node); 
	    return child_node;
	  }
	  count_nodes++;
	} else {
	  
	  // cache coherence (test_large_joins_join2 fails without it)
	  __sync_synchronize(); 	   
	  child_node = (ans_node_ptr) TrNode_child(parent_node);
	  goto answer_trie_hash; 
	  
	  /* This code shows that the expansion node is correct
	     ans_node_ptr expnode1, expnode2;
	     expnode1 = child_node;
	     do {
	     child_node = (ans_node_ptr) TrNode_child(parent_node);
	     if (!IS_ANSWER_TRIE_HASH(child_node))
	     __sync_synchronize(); 	   
	     } while (!IS_ANSWER_TRIE_HASH(child_node));
	     ans_hash_ptr hash_node_lixo = (ans_hash_ptr) child_node; 
	     expnode2 = Hash_ans_exp_node(hash_node_lixo);
	     if (expnode1 != expnode2)
	     printf("expnode1 = %p  expnode2 = %p \n", expnode1, expnode2);
	     goto answer_trie_hash; */	  
	}
	child_node = TrNode_next(child_node);
      }
      first_node = TrNode_next(new_child_node) = first_node_tmp;
    }    
    child_node = new_child_node;
    count_nodes++;
    
    if (count_nodes == MAX_NODES_PER_TRIE_LEVEL) {
      ans_node_ptr chain_node , next_node, exp_node;
      ans_hash_ptr hash_node;
      struct answer_trie_node **new_hash_buckets;
      new_answer_trie_hash_atomic_v03(exp_node, hash_node, new_hash_buckets, 0, sg_fr, child_node);
      chain_node = child_node;
      while (!BOOL_CAS(&(TrNode_child(parent_node)), chain_node, (ans_node_ptr)hash_node)) {
	chain_node = TrNode_next(exp_node) = TrNode_child(parent_node);
      }
      // alloc a new hash
      count_nodes = adjust_answer_hash_nodes_first_exp(chain_node, new_hash_buckets, 0);
      Add_HashNode_num_nodes_v03(hash_node, count_nodes);
      
      // remove the the expansion node from all buckets
      bucket = new_hash_buckets + BASE_HASH_BUCKETS;
      do {
	bucket--;
	if (BOOL_CAS(bucket, exp_node, NULL))
	  continue;	
	chain_node = *bucket;
	while(chain_node) {
	  next_node = TrNode_next(chain_node);
	  if (next_node == exp_node) {
	    TrNode_next(chain_node) =  NULL;
	    break;
	  }
	  chain_node = next_node;
	}
      } while (bucket != new_hash_buckets);
      OPEN_HASH_V03(hash_node);
    }
    return child_node;
  }
  
 answer_trie_hash:
  {
    int num_buckets = 0;
    ans_hash_ptr hash_node = (ans_hash_ptr) child_node; 
    ans_hash_bkts_ptr hash = (ans_hash_bkts_ptr) ((CELL) AnsHash_hash_bkts(hash_node) & ~(CELL)0x1);
    num_buckets = HashBkts_number_of_buckets(hash);
    bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
    child_node = *bucket; 
    ans_node_ptr next_node = NULL;
    
    while (IS_NEW_HASH_REF_V03(child_node)){
      hash = (ans_hash_bkts_ptr) ((CELL)(*bucket) & ~(CELL)0x1);
      num_buckets = HashBkts_number_of_buckets(hash);
      bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
      child_node = *bucket; 
    }
    
    int expNodeVisited = 0; 
    while (child_node) {
      if (!IS_ANSWER_TRIE_HASH_EXPANSION(child_node)) {
	if (TrNode_entry(child_node) == t) {
	  if (new_child_node != NULL) 
	    FREE_ANSWER_TRIE_NODE(new_child_node);
	  return child_node;    
	}
      } else {
	if (expNodeVisited == 1)
	  break;
	else
	  expNodeVisited = 1;	
      }  
      child_node = TrNode_next(child_node);    
    }
    
    if (new_child_node == NULL) {
      NEW_ANSWER_TRIE_NODE(new_child_node, instr, t, NULL, parent_node, NULL);
    }
    
    first_node = NULL;        // oldfFirst
    expNodeVisited = 0;
    ans_node_ptr first_node_tmp = NULL;
    do {
      count_nodes = 0;
      child_node = *bucket; 
      
      while (IS_NEW_HASH_REF_V03(child_node)){
	hash = (ans_hash_bkts_ptr) ((CELL)(*bucket) & ~(CELL)0x1);
	num_buckets = HashBkts_number_of_buckets(hash);
	bucket = HashBkts_buckets(hash) + HASH_ENTRY(t, num_buckets);
	expNodeVisited = 0;   // lock-freedom - 4
	child_node = *bucket; 
	first_node = NULL;
      }

      first_node_tmp = child_node;   // first

      while (child_node != first_node) {
	if (!IS_ANSWER_TRIE_HASH_EXPANSION(child_node)) {
	  if (TrNode_entry(child_node) == t) {
	    FREE_ANSWER_TRIE_NODE(new_child_node);
	    return child_node;	
	  }	  
	  count_nodes++;
	} else {
	  if (expNodeVisited == 1)
	    break;
	  else
	    expNodeVisited = 1;	
	}
	child_node = TrNode_next(child_node);	  
      }      
      if (first_node_tmp && !IS_ANSWER_TRIE_HASH_EXPANSION(first_node_tmp))
	first_node = first_node_tmp;
      TrNode_next(new_child_node) = first_node_tmp;
    } while(!BOOL_CAS(bucket, first_node_tmp, new_child_node));

    child_node = new_child_node;
    count_nodes++; 
    
    Inc_HashNode_num_nodes_v03(hash_node);

    hash = (ans_hash_bkts_ptr) ((CELL) AnsHash_hash_bkts(hash_node) & ~(CELL)0x1);
    
    if (count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash_node) > AnsHash_num_buckets(hash_node)) {
      if (BOOL_CAS(&(AnsHash_hash_bkts(hash_node)), hash, CLOSE_HASH_V03(hash))) {	
	ans_node_ptr chain_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
	ans_node_ptr exp_node;
	ans_hash_bkts_ptr new_hash;

	old_hash_buckets = HashBkts_buckets(hash);
	int old_number_buckets = HashBkts_number_of_buckets(hash);

	// ok for expanding the current hash 
	num_buckets = old_number_buckets * 2;

	ALLOC_OPEN_HASH_BUCKETS(new_hash, new_hash_buckets, num_buckets, struct answer_trie_node, struct answer_trie_hash_buckets);
	exp_node = Hash_ans_exp_node(hash_node);
	int i = 0;
	while(i < old_number_buckets) {
	  old_bucket = old_hash_buckets + i;
	  if (BOOL_CAS(old_bucket, NULL, (ans_node_ptr)((CELL)new_hash | (CELL)0x1))) { 
	    i++;
	    continue;
	  }
	  // we have at least one node on the bucket
	  
	  ans_node_ptr *new_bucket_1, *new_bucket_2;
	  new_bucket_1 = new_hash_buckets + i;
	  new_bucket_2 = new_hash_buckets + i + old_number_buckets;
	  *new_bucket_1 = *new_bucket_2 = exp_node;

	  do
	    TrNode_next(exp_node) = first_node = *old_bucket;
	  while(!BOOL_CAS(old_bucket, first_node, (ans_node_ptr)((CELL)new_hash | (CELL)0x1))); 
	  	  
	  // putting the nodes of the last hash in place on the new hash
	  chain_node = first_node;
	  adjust_answer_hash_nodes(chain_node, new_hash_buckets, num_buckets); 

	  // removing the link of both buckets to the exp_node
	  if (!BOOL_CAS(new_bucket_1, exp_node, NULL)) { 
	    chain_node = *new_bucket_1;
	    next_node = TrNode_next(chain_node);
	    while(next_node != exp_node) {
	      chain_node = next_node;
	      next_node = TrNode_next(chain_node);
	    }
	    TrNode_next(chain_node) = NULL;	      
	  }

	  if (!BOOL_CAS(new_bucket_2, exp_node, NULL)) { 
	    chain_node = *new_bucket_2;
	    next_node = TrNode_next(chain_node);
	    while(next_node != exp_node) {
	      chain_node = next_node;
	      next_node = TrNode_next(chain_node);
	    }
	    TrNode_next(chain_node) = NULL;	      
	  }	  
	  TrNode_next(exp_node) = NULL;
	  i++;
	}
	
	HashBkts_next(hash) = AnsHash_old_hash_bkts(hash_node);
	AnsHash_old_hash_bkts(hash_node) = hash;
	AnsHash_hash_bkts(hash_node) = new_hash; // open hash
      }
    }
    return child_node;    
  }
}

#endif /* ANSWER_TRIE_LOCK_LEVEL */
#endif /* INCLUDE_ANSWER_TRIE_CHECK_INSERT */



/************************************************************************
**                 global_trie_check_insert_(gt)_entry                 **
************************************************************************/

#ifdef INCLUDE_GLOBAL_TRIE_CHECK_INSERT
#ifndef GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL /* GLOBAL_TRIE_LOCK_AT_NODE_LEVEL || ! YAPOR */
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline gt_node_ptr global_trie_check_insert_gt_entry(gt_node_ptr parent_node, Term t USES_REGS) {
#else
static inline gt_node_ptr global_trie_check_insert_entry(gt_node_ptr parent_node, Term t USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */
  gt_node_ptr child_node;
    
  LOCK_GLOBAL_NODE(parent_node);
  child_node = TrNode_child(parent_node);
  if (child_node == NULL) {
    NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, NULL);
    TrNode_child(parent_node) = child_node;
    UNLOCK_GLOBAL_NODE(parent_node);
    return child_node;
  }

  if (! IS_GLOBAL_TRIE_HASH(child_node)) {
    int count_nodes = 0;
    do {
      if (TrNode_entry(child_node) == t) {
        UNLOCK_GLOBAL_NODE(parent_node);
        return child_node;
      }
      count_nodes++;
      child_node = TrNode_next(child_node);
    } while (child_node);
    NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, TrNode_child(parent_node));
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      /* alloc a new hash */
      gt_hash_ptr hash;
      gt_node_ptr chain_node, next_node, *bucket;
      new_global_trie_hash(hash, count_nodes);
      chain_node = child_node;
      do {
        bucket = Hash_buckets(hash) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
        next_node = TrNode_next(chain_node);
        TrNode_next(chain_node) = *bucket;
        *bucket = chain_node;
        chain_node = next_node;
      } while (chain_node);
      TrNode_child(parent_node) = (gt_node_ptr) hash;
    } else {
      TrNode_child(parent_node) = child_node;
    }
    UNLOCK_GLOBAL_NODE(parent_node);
    return child_node;
  }

  { /* trie nodes with hashing */
    gt_hash_ptr hash;
    gt_node_ptr *bucket;
    int count_nodes = 0;
    hash = (gt_hash_ptr) child_node; 
    bucket = Hash_buckets(hash) + HASH_ENTRY(t, Hash_num_buckets(hash));
    child_node = *bucket;
    while (child_node) { 
      if (TrNode_entry(child_node) == t) {
        UNLOCK_GLOBAL_NODE(parent_node);
        return child_node;
      }
      count_nodes++;
      child_node = TrNode_next(child_node);
    } 
    NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, *bucket);
    *bucket = child_node;
    Hash_num_nodes(hash)++;
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash) > Hash_num_buckets(hash)) {
      /* expand current hash */
      gt_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
      int num_buckets;
      num_buckets = Hash_num_buckets(hash) * 2;
      ALLOC_BUCKETS(new_hash_buckets, num_buckets);
      old_hash_buckets = Hash_buckets(hash);
      old_bucket = old_hash_buckets + Hash_num_buckets(hash);
      do {
        if (*--old_bucket) {
          chain_node = *old_bucket;
          do {
            bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
            next_node = TrNode_next(chain_node);
            TrNode_next(chain_node) = *bucket;
            *bucket = chain_node;
            chain_node = next_node;
          } while (chain_node);
        }
      } while (old_bucket != old_hash_buckets);
      Hash_buckets(hash) = new_hash_buckets;
      Hash_num_buckets(hash) = num_buckets;
      //      FREE_BUCKETS(old_hash_buckets);
    }
    UNLOCK_GLOBAL_NODE(parent_node);
    return child_node;
  }
}
#else /* GLOBAL_TRIE_LOCK_AT_WRITE_LEVEL */
#ifdef MODE_GLOBAL_TRIE_ENTRY
static inline gt_node_ptr global_trie_check_insert_gt_entry(gt_node_ptr parent_node, Term t USES_REGS) {
#else
static inline gt_node_ptr global_trie_check_insert_entry(gt_node_ptr parent_node, Term t USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_ENTRY */
  gt_node_ptr child_node;
  gt_hash_ptr hash;

  child_node = TrNode_child(parent_node);
  if (child_node == NULL) {
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
    NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, NULL);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
    LOCK_GLOBAL_NODE(parent_node);
    if (TrNode_child(parent_node)) {
      gt_node_ptr chain_node = TrNode_child(parent_node);
      if (IS_GLOBAL_TRIE_HASH(chain_node)) {
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
        FREE_GLOBAL_TRIE_NODE(child_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
        UNLOCK_GLOBAL_NODE(parent_node);
        hash = (gt_hash_ptr) chain_node;
        goto global_trie_hash;
      }
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
          FREE_GLOBAL_TRIE_NODE(child_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_GLOBAL_NODE(parent_node);
          return chain_node;
        }
        chain_node = TrNode_next(chain_node);
      } while (chain_node);
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = TrNode_child(parent_node);
#else
      NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, TrNode_child(parent_node));
    } else {
      NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, NULL);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
    }
    TrNode_child(parent_node) = child_node;
    UNLOCK_GLOBAL_NODE(parent_node);
    return child_node;
  } 

  if (! IS_GLOBAL_TRIE_HASH(child_node)) {
    gt_node_ptr first_node = child_node;
    int count_nodes = 0;
    do {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    } while (child_node);
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
    NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, first_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
    LOCK_GLOBAL_NODE(parent_node);
    if (first_node != TrNode_child(parent_node)) {
      gt_node_ptr chain_node = TrNode_child(parent_node);
      if (IS_GLOBAL_TRIE_HASH(chain_node)) {
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
        FREE_GLOBAL_TRIE_NODE(child_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
        UNLOCK_GLOBAL_NODE(parent_node);
        hash = (gt_hash_ptr) chain_node; 
        goto global_trie_hash;
      }
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
          FREE_GLOBAL_TRIE_NODE(child_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_GLOBAL_NODE(parent_node);
          return chain_node;
        }
        count_nodes++;
        chain_node = TrNode_next(chain_node);
      } while (chain_node != first_node);
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = TrNode_child(parent_node);
#else
      NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, TrNode_child(parent_node));
    } else {
      NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, first_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
    }
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_TRIE_LEVEL) {
      /* alloc a new hash */
      gt_node_ptr chain_node, next_node, *bucket;
      new_global_trie_hash(hash, count_nodes);
      chain_node = child_node;
      do {
        bucket = Hash_buckets(hash) + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
        next_node = TrNode_next(chain_node);
        TrNode_next(chain_node) = *bucket;
        *bucket = chain_node;
        chain_node = next_node;
      } while (chain_node);
      TrNode_child(parent_node) = (gt_node_ptr) hash;
    } else {
      TrNode_child(parent_node) = child_node;
    }
    UNLOCK_GLOBAL_NODE(parent_node);
    return child_node;
  }

  hash = (gt_hash_ptr) child_node;
global_trie_hash:
  { /* trie nodes with hashing */
    gt_node_ptr *bucket, first_node;
    int num_buckets, count_nodes = 0;

    do {
      num_buckets = Hash_num_buckets(hash);
      //      __sync_synchronize();
      bucket = Hash_buckets(hash) + HASH_ENTRY(t, num_buckets);
      first_node = child_node = *bucket;
    } while (num_buckets != Hash_num_buckets(hash));
    while (child_node) {
      if (TrNode_entry(child_node) == t)
        return child_node;
      count_nodes++;
      child_node = TrNode_next(child_node);
    }
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
    NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, first_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
    LOCK_GLOBAL_NODE(parent_node);
    if (num_buckets != Hash_num_buckets(hash)) {
      /* the hash has been expanded */ 
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
      FREE_GLOBAL_TRIE_NODE(child_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
      UNLOCK_GLOBAL_NODE(parent_node);
      goto global_trie_hash;
    }
    if (first_node != *bucket) {
      gt_node_ptr chain_node = *bucket;
      do {
        if (TrNode_entry(chain_node) == t) {
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
          FREE_GLOBAL_TRIE_NODE(child_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
          UNLOCK_GLOBAL_NODE(parent_node);
          return chain_node;
        }
        count_nodes++;
        chain_node = TrNode_next(chain_node);
      } while (chain_node != first_node);
#ifdef GLOBAL_TRIE_ALLOC_BEFORE_CHECK
      TrNode_next(child_node) = *bucket;
#else
      NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, *bucket);
    } else {
      NEW_GLOBAL_TRIE_NODE(child_node, t, NULL, parent_node, first_node);
#endif /* GLOBAL_TRIE_ALLOC_BEFORE_CHECK */
    }
    *bucket = child_node;
    Hash_num_nodes(hash)++;
    count_nodes++;
    if (count_nodes >= MAX_NODES_PER_BUCKET && Hash_num_nodes(hash) > Hash_num_buckets(hash)) {
      /* expand current hash */ 
      gt_node_ptr chain_node, next_node, *old_bucket, *old_hash_buckets, *new_hash_buckets;
      num_buckets = Hash_num_buckets(hash) * 2;
      ALLOC_BUCKETS(new_hash_buckets, num_buckets);
      old_hash_buckets = Hash_buckets(hash);
      old_bucket = old_hash_buckets + Hash_num_buckets(hash);
      do {
        if (*--old_bucket) {
          chain_node = *old_bucket;
          do {
            bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
            next_node = TrNode_next(chain_node);
            TrNode_next(chain_node) = *bucket;
            *bucket = chain_node;
            chain_node = next_node;
          } while (chain_node);
        }
      } while (old_bucket != old_hash_buckets);
      Hash_buckets(hash) = new_hash_buckets;
      Hash_num_buckets(hash) = num_buckets;
      //      FREE_BUCKETS(old_hash_buckets);
    }
    UNLOCK_GLOBAL_NODE(parent_node);
    return child_node;
  }
}
#endif /* GLOBAL_TRIE_LOCK_LEVEL */
#endif /* INCLUDE_GLOBAL_TRIE_CHECK_INSERT */



/************************************************************************
**             subgoal_search(_global_trie)(_terms)_loop               **
************************************************************************/

#ifdef INCLUDE_SUBGOAL_SEARCH_LOOP
#ifdef MODE_GLOBAL_TRIE_LOOP
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
static inline gt_node_ptr subgoal_search_global_trie_terms_loop(Term t, int *subs_arity_ptr, CELL **stack_vars_ptr, CELL *stack_terms USES_REGS) {
#else
static inline gt_node_ptr subgoal_search_global_trie_loop(Term t, int *subs_arity_ptr, CELL **stack_vars_ptr USES_REGS) {
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
#else
#ifdef MODE_TERMS_LOOP
static inline sg_node_ptr subgoal_search_terms_loop(tab_ent_ptr tab_ent, sg_node_ptr current_node, Term t, int *subs_arity_ptr, CELL **stack_vars_ptr USES_REGS) {
#else
static inline sg_node_ptr subgoal_search_loop(tab_ent_ptr tab_ent, sg_node_ptr current_node, Term t, int *subs_arity_ptr, CELL **stack_vars_ptr USES_REGS) {
#endif /* MODE_TERMS_LOOP */
#endif /* MODE_GLOBAL_TRIE_LOOP */
/************************************************************************
                     ===========
                     |         |
                     |   ...   |
                     |         |
                     -----------
                     |  VAR_N  |  <-- stack_vars
                     -----------           *
                     |   ...   |          /|\
                     -----------           |  subs_arity (N+1)
                     |  VAR_0  |          \|/
                     -----------           *
           YENV -->  |         |
                     -----------
                     |         |
                     |   ...   |
                     |         |
                     ===========
                     |         |
                     |   ...   |
                     |         |
                     -----------
             TR -->  |         |  <-- stack_terms_limit
                     -----------
                     |         |
                     |   ...   |
                     |         |
                     ----------|
                     |  TERM_N |  <-- stack_terms
                     ----------|           *
                     |   ...   |          /|\
                     ----------|           |
                     |  TERM_1 |           |
                     ----------|           |
                     |   NULL  |          \|/
                     ===========           *
 LOCAL_TrailTop -->  |         |
                     -----------
************************************************************************/
#ifdef MODE_GLOBAL_TRIE_LOOP
  gt_node_ptr current_node = GLOBAL_root_gt;
#endif /* MODE_GLOBAL_TRIE_LOOP */
  int subs_arity = *subs_arity_ptr;
  CELL *stack_vars = *stack_vars_ptr;
#if ! defined(MODE_GLOBAL_TRIE_LOOP) || ! defined(GLOBAL_TRIE_FOR_SUBTERMS)
  CELL *stack_terms = (CELL *) LOCAL_TrailTop;
#endif /* ! MODE_GLOBAL_TRIE_LOOP || ! GLOBAL_TRIE_FOR_SUBTERMS */
  CELL *stack_terms_limit = (CELL *) TR;
  AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 1);  /* + 1 because initially we stiil haven't done any STACK_POP_DOWN */
  STACK_PUSH_UP(NULL, stack_terms);

#if defined(MODE_GLOBAL_TRIE_LOOP)
  /* for the global trie, it is safe to skip the IsVarTerm() and IsAtomOrIntTerm() tests in the first iteration */
  goto subgoal_search_loop_non_atomic;
#endif /* MODE_GLOBAL_TRIE_LOOP */

  do {
    if (IsVarTerm(t)) {
      if (IsTableVarTerm(t)) {
	t = MakeTableVarTerm(VarIndexOfTerm(t));
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, t);
      } else {
	if (subs_arity == MAX_TABLE_VARS)
	  Yap_Error(INTERNAL_ERROR, TermNil, "subgoal_search_loop: MAX_TABLE_VARS exceeded");
	STACK_PUSH_UP(t, stack_vars);
	*((CELL *)t) = GLOBAL_table_var_enumerator(subs_arity);
	t = MakeTableVarTerm(subs_arity);
	subs_arity = subs_arity + 1;
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, t);
      }
    } else if (IsAtomOrIntTerm(t)) {
      SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, t);
#ifdef MODE_TERMS_LOOP
    } else {
      gt_node_ptr entry_node;
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
      entry_node = subgoal_search_global_trie_terms_loop(t, &subs_arity, &stack_vars, stack_terms PASS_REGS);
#else
      entry_node = subgoal_search_global_trie_loop(t, &subs_arity, &stack_vars PASS_REGS);
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
      current_node = subgoal_trie_check_insert_gt_entry(tab_ent, current_node, (Term) entry_node PASS_REGS);
#else /* ! MODE_TERMS_LOOP */
    } else 
#if defined(MODE_GLOBAL_TRIE_LOOP)
      /* for the global trie, it is safe to start here in the first iteration */
      subgoal_search_loop_non_atomic:
#endif /* MODE_GLOBAL_TRIE_LOOP */
#ifdef TRIE_COMPACT_PAIRS
    if (IsPairTerm(t)) {
      CELL *aux_pair = RepPair(t);
      if (aux_pair == PairTermMark) {
	t = STACK_POP_DOWN(stack_terms);
	if (IsPairTerm(t)) {
	  aux_pair = RepPair(t);
	  t = Deref(aux_pair[1]);
	  if (t == TermNil) {
	    SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, CompactPairEndList);
	  } else {
	    /* AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 2);                   */
	    /* AUX_STACK_CHECK_EXPAND is not necessary here because the situation of pushing **
	    ** up 3 terms has already initially checked for the CompactPairInit term         */
	    STACK_PUSH_UP(t, stack_terms);
	    STACK_PUSH_UP(AbsPair(PairTermMark), stack_terms);
	  }
	  STACK_PUSH_UP(Deref(aux_pair[0]), stack_terms);
	} else {
	  SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, CompactPairEndTerm);
	  STACK_PUSH_UP(t, stack_terms);
	}
#if defined(MODE_GLOBAL_TRIE_LOOP) && defined(GLOBAL_TRIE_FOR_SUBTERMS)
      } else if (current_node != GLOBAL_root_gt) {
	gt_node_ptr entry_node = subgoal_search_global_trie_terms_loop(t, &subs_arity, &stack_vars, stack_terms PASS_REGS);
	current_node = global_trie_check_insert_gt_entry(current_node, (Term) entry_node PASS_REGS);
#endif /* MODE_GLOBAL_TRIE_LOOP && GLOBAL_TRIE_FOR_SUBTERMS */
      } else {
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, CompactPairInit);
	t = Deref(aux_pair[1]);
	if (t == TermNil) {
	  SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, CompactPairEndList);
	} else {
	  AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 2);
	  STACK_PUSH_UP(t, stack_terms);
	  STACK_PUSH_UP(AbsPair(PairTermMark), stack_terms);
	}
	STACK_PUSH_UP(Deref(aux_pair[0]), stack_terms);
      }
#if defined(MODE_GLOBAL_TRIE_LOOP) && defined(GLOBAL_TRIE_FOR_SUBTERMS)
    } else if (current_node != GLOBAL_root_gt) {
      gt_node_ptr entry_node = subgoal_search_global_trie_terms_loop(t, &subs_arity, &stack_vars, stack_terms PASS_REGS);
      current_node = global_trie_check_insert_gt_entry(current_node, (Term) entry_node PASS_REGS);
#endif /* MODE_GLOBAL_TRIE_LOOP && GLOBAL_TRIE_FOR_SUBTERMS */
#else /* ! TRIE_COMPACT_PAIRS */
#if defined(MODE_GLOBAL_TRIE_LOOP) && defined(GLOBAL_TRIE_FOR_SUBTERMS)
    if (current_node != GLOBAL_root_gt) {
      gt_node_ptr entry_node = subgoal_search_global_trie_terms_loop(t, &subs_arity, &stack_vars, stack_terms PASS_REGS);
      current_node = global_trie_check_insert_gt_entry(current_node, (Term) entry_node PASS_REGS);
    } else 
#endif /* MODE_GLOBAL_TRIE_LOOP && GLOBAL_TRIE_FOR_SUBTERMS */
    if (IsPairTerm(t)) {
      CELL *aux_pair = RepPair(t);
      SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, AbsPair(NULL));
      AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 1);
      STACK_PUSH_UP(Deref(aux_pair[1]), stack_terms);
      STACK_PUSH_UP(Deref(aux_pair[0]), stack_terms);
#endif /* TRIE_COMPACT_PAIRS */
    } else if (IsApplTerm(t)) {
      Functor f = FunctorOfTerm(t);
      if (f == FunctorDouble) {
	union {
	  Term t_dbl[sizeof(Float)/sizeof(Term)];
	  Float dbl;
	} u;
	u.dbl = FloatOfTerm(t);
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, AbsAppl((Term *)f));
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, u.t_dbl[1]);
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, u.t_dbl[0]);
#ifdef MODE_GLOBAL_TRIE_LOOP
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, AbsAppl((Term *)f));
#endif /* MODE_GLOBAL_TRIE_LOOP */
      } else if (f == FunctorLongInt) {
	Int li = LongIntOfTerm(t);
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, AbsAppl((Term *)f));
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, li);
#ifdef MODE_GLOBAL_TRIE_LOOP
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, AbsAppl((Term *)f));
#endif /* MODE_GLOBAL_TRIE_LOOP */
      } else if (f == FunctorDBRef) {
	Yap_Error(INTERNAL_ERROR, TermNil, "subgoal_search_loop: unsupported type tag FunctorDBRef");
      } else if (f == FunctorBigInt) {
	Yap_Error(INTERNAL_ERROR, TermNil, "subgoal_search_loop: unsupported type tag FunctorBigInt");
      } else {
	int i;
	CELL *aux_appl = RepAppl(t);
	SUBGOAL_CHECK_INSERT_ENTRY(tab_ent, current_node, AbsAppl((Term *)f));
	AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + ArityOfFunctor(f) - 1);
	for (i = ArityOfFunctor(f); i >= 1; i--)
	  STACK_PUSH_UP(Deref(aux_appl[i]), stack_terms);
      }
    } else {
      Yap_Error(INTERNAL_ERROR, TermNil, "subgoal_search_loop: unknown type tag");
#endif /* MODE_TERMS_LOOP */
    }
    t = STACK_POP_DOWN(stack_terms);
  } while (t);
  
  *subs_arity_ptr = subs_arity;
  *stack_vars_ptr = stack_vars;
  return current_node;
}
#endif /* INCLUDE_SUBGOAL_SEARCH_LOOP */



/************************************************************************
**               answer_search(_global_trie)(_terms)_loop              **
************************************************************************/

#ifdef INCLUDE_ANSWER_SEARCH_LOOP
#ifdef MODE_GLOBAL_TRIE_LOOP
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
static inline gt_node_ptr answer_search_global_trie_terms_loop(Term t, int *vars_arity_ptr, CELL *stack_terms USES_REGS) {
#else
static inline gt_node_ptr answer_search_global_trie_loop(Term t, int *vars_arity_ptr USES_REGS) {
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
#else
#ifdef MODE_TERMS_LOOP
static inline ans_node_ptr answer_search_terms_loop(sg_fr_ptr sg_fr, ans_node_ptr current_node, Term t, int *vars_arity_ptr USES_REGS) {
#else
static inline ans_node_ptr answer_search_loop(sg_fr_ptr sg_fr, ans_node_ptr current_node, Term t, int *vars_arity_ptr USES_REGS) {
#endif /* MODE_TERMS_LOOP */
#endif /* MODE_GLOBAL_TRIE_LOOP */
/************************************************************************
                     ===========
                     |         |
                     |   ...   |
                     |         |
                     -----------
             TR -->  |  VAR_0  |  <-- stack_vars_base
                     -----------           *
                     |   ...   |          /|\
                     -----------           |   vars_arity (N+1)
                     |  VAR_N  |          \|/
                     -----------           *
                     |         |  <-- stack_terms_limit
                     -----------
                     |         |
                     |   ...   |
                     |         |
                     ----------|
                     |  TERM_N |  <-- stack_terms
                     ----------|           *
                     |   ...   |          /|\
                     ----------|           |
                     |  TERM_1 |           |
                     ----------|           |
                     |   NULL  |          \|/
                     ===========           *
 LOCAL_TrailTop -->  |         |
                     -----------
************************************************************************/
#ifdef MODE_GLOBAL_TRIE_LOOP
  gt_node_ptr current_node = GLOBAL_root_gt;
#endif /* MODE_GLOBAL_TRIE_LOOP */
  int vars_arity = *vars_arity_ptr;
#if ! defined(MODE_GLOBAL_TRIE_LOOP) || ! defined(GLOBAL_TRIE_FOR_SUBTERMS)
  CELL *stack_terms = (CELL *) LOCAL_TrailTop;
#endif /* ! MODE_GLOBAL_TRIE_LOOP || ! GLOBAL_TRIE_FOR_SUBTERMS */
  CELL *stack_vars_base = (CELL *) TR;
#define stack_terms_limit (stack_vars_base + vars_arity)
#ifdef TRIE_COMPACT_PAIRS
  int in_pair = 0;
#else
#define in_pair 0
#endif /* TRIE_COMPACT_PAIRS */
  AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 1);  /* + 1 because initially we stiil haven't done any STACK_POP_DOWN */
  STACK_PUSH_UP(NULL, stack_terms);

#if defined(MODE_GLOBAL_TRIE_LOOP)
  /* for the global trie, it is safe to skip the IsVarTerm() and IsAtomOrIntTerm() tests in the first iteration */
  goto answer_search_loop_non_atomic;
#endif /* MODE_GLOBAL_TRIE_LOOP */

  do {
    if (IsVarTerm(t)) {
      t = Deref(t);
      if (IsTableVarTerm(t)) {
	t = MakeTableVarTerm(VarIndexOfTerm(t));
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, t, _trie_retry_val + in_pair);
      } else {
	if (vars_arity == MAX_TABLE_VARS)
	  Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_loop: MAX_TABLE_VARS exceeded");
	stack_vars_base[vars_arity] = t;
	*((CELL *)t) = GLOBAL_table_var_enumerator(vars_arity);
	t = MakeTableVarTerm(vars_arity);
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, t, _trie_retry_var + in_pair);
	vars_arity = vars_arity + 1;
      }
#ifdef TRIE_COMPACT_PAIRS
      in_pair = 0;
#endif /* TRIE_COMPACT_PAIRS */
    } else if (IsAtomOrIntTerm(t)) {
      ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, t, _trie_retry_atom + in_pair);
#ifdef TRIE_COMPACT_PAIRS
      in_pair = 0;
#endif /* TRIE_COMPACT_PAIRS */
#ifdef MODE_TERMS_LOOP
    } else {
      gt_node_ptr entry_node;
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
      entry_node = answer_search_global_trie_terms_loop(t, &vars_arity, stack_terms PASS_REGS);
#else
      entry_node = answer_search_global_trie_loop(t, &vars_arity PASS_REGS);
#endif /*  GLOBAL_TRIE_FOR_SUBTERMS */
      current_node = answer_trie_check_insert_gt_entry(sg_fr, current_node, (Term) entry_node, _trie_retry_gterm + in_pair PASS_REGS);
#else /* ! MODE_TERMS_LOOP */
    } else 
#if defined(MODE_GLOBAL_TRIE_LOOP)
      /* for the global trie, it is safe to start here in the first iteration */
      answer_search_loop_non_atomic:
#endif /* MODE_GLOBAL_TRIE_LOOP */
#ifdef TRIE_COMPACT_PAIRS
    if (IsPairTerm(t)) {
      CELL *aux_pair = RepPair(t);
      if (aux_pair == PairTermMark) {
	t = STACK_POP_DOWN(stack_terms);
	if (IsPairTerm(t)) {
	  aux_pair = RepPair(t);
	  t = Deref(aux_pair[1]);
	  if (t == TermNil) {
	    ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, CompactPairEndList, _trie_retry_pair);
	  } else {
	    /* AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 2);                   */
	    /* AUX_STACK_CHECK_EXPAND is not necessary here because the situation of pushing **
	    ** up 3 terms has already initially checked for the CompactPairInit term         */
	    STACK_PUSH_UP(t, stack_terms);
	    STACK_PUSH_UP(AbsPair(PairTermMark), stack_terms);
	    in_pair = 4;
	  }
	  STACK_PUSH_UP(Deref(aux_pair[0]), stack_terms);
	} else {
	  ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, CompactPairEndTerm, _trie_retry_null);
	  STACK_PUSH_UP(t, stack_terms);
	}
#if defined(MODE_GLOBAL_TRIE_LOOP) && defined(GLOBAL_TRIE_FOR_SUBTERMS)
      } else if (current_node != GLOBAL_root_gt) {
	gt_node_ptr entry_node = answer_search_global_trie_terms_loop(t, &vars_arity, stack_terms PASS_REGS);
	current_node = global_trie_check_insert_gt_entry(current_node, (Term) entry_node PASS_REGS);
#endif /* MODE_GLOBAL_TRIE_LOOP && GLOBAL_TRIE_FOR_SUBTERMS */
      } else {
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, CompactPairInit, _trie_retry_null + in_pair);
	t = Deref(aux_pair[1]);
	if (t == TermNil) {
	   ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, CompactPairEndList, _trie_retry_pair);
	   in_pair = 0;
	} else {
	  AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 2);
	  STACK_PUSH_UP(t, stack_terms);
	  STACK_PUSH_UP(AbsPair(PairTermMark), stack_terms);
	  in_pair = 4;
	}
	STACK_PUSH_UP(Deref(aux_pair[0]), stack_terms);
      }
#if defined(MODE_GLOBAL_TRIE_LOOP) && defined(GLOBAL_TRIE_FOR_SUBTERMS)
    } else if (current_node != GLOBAL_root_gt) {
      gt_node_ptr entry_node = answer_search_global_trie_terms_loop(t, &vars_arity, stack_terms PASS_REGS);
      current_node = global_trie_check_insert_gt_entry(current_node, (Term) entry_node PASS_REGS);
#endif /* MODE_GLOBAL_TRIE_LOOP && GLOBAL_TRIE_FOR_SUBTERMS */
#else /* ! TRIE_COMPACT_PAIRS */
#if defined(MODE_GLOBAL_TRIE_LOOP) && defined(GLOBAL_TRIE_FOR_SUBTERMS)
    if (current_node != GLOBAL_root_gt) {
      gt_node_ptr entry_node = answer_search_global_trie_terms_loop(t, &vars_arity, stack_terms PASS_REGS);
      current_node = global_trie_check_insert_gt_entry(current_node, (Term) entry_node PASS_REGS);
    } else 
#endif /* MODE_GLOBAL_TRIE_LOOP && GLOBAL_TRIE_FOR_SUBTERMS */
    if (IsPairTerm(t)) {
      CELL *aux_pair = RepPair(t);
      ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, AbsPair(NULL), _trie_retry_pair);
      AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 1);
      STACK_PUSH_UP(Deref(aux_pair[1]), stack_terms);
      STACK_PUSH_UP(Deref(aux_pair[0]), stack_terms);
#endif /* TRIE_COMPACT_PAIRS */
    } else if (IsApplTerm(t)) {
      Functor f = FunctorOfTerm(t);
      if (f == FunctorDouble) {
	union {
	  Term t_dbl[sizeof(Float)/sizeof(Term)];
	  Float dbl;
	} u;
	u.dbl = FloatOfTerm(t);
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, AbsAppl((Term *)f), _trie_retry_null + in_pair);
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, u.t_dbl[1], _trie_retry_extension);
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, u.t_dbl[0], _trie_retry_extension);
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, AbsAppl((Term *)f), _trie_retry_double);
      } else if (f == FunctorLongInt) {
	Int li = LongIntOfTerm (t);
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, AbsAppl((Term *)f), _trie_retry_null + in_pair);
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, li, _trie_retry_extension);
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, AbsAppl((Term *)f), _trie_retry_longint);
      } else if (f == FunctorDBRef) {
	Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_loop: unsupported type tag FunctorDBRef");
      } else if (f == FunctorBigInt) {
	Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_loop: unsupported type tag FunctorBigInt");
      } else {
	int i;
	CELL *aux_appl = RepAppl(t);
	ANSWER_CHECK_INSERT_ENTRY(sg_fr, current_node, AbsAppl((Term *)f), _trie_retry_appl + in_pair);
	AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + ArityOfFunctor(f) - 1);
	for (i = ArityOfFunctor(f); i >= 1; i--)
	  STACK_PUSH_UP(Deref(aux_appl[i]), stack_terms);
      }
#ifdef TRIE_COMPACT_PAIRS
      in_pair = 0;
#endif /* TRIE_COMPACT_PAIRS */
    } else {
      Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_loop: unknown type tag");
#endif /* MODE_TERMS_LOOP */
    }
    t = STACK_POP_DOWN(stack_terms);
  } while (t);

  *vars_arity_ptr = vars_arity;
  return current_node;

#undef stack_terms_limit
#ifndef TRIE_COMPACT_PAIRS
#undef in_pair
#endif /* TRIE_COMPACT_PAIRS */
}
#endif /* INCLUDE_ANSWER_SEARCH_LOOP */



/**************************************************************
**                   answer_search_min_max                   **
**************************************************************/

#ifdef INCLUDE_ANSWER_SEARCH_MODE_DIRECTED

#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02

#define NEW_MODE_DIRECTED_ANS_FIRST_NODE(FIRST_NODE, LAST_NODE, PARENT_NODE, ENTRY, INSTR)  \
        NEW_ANSWER_TRIE_NODE(FIRST_NODE, INSTR, ENTRY, NULL, PARENT_NODE, NULL);            \
        LAST_NODE = FIRST_NODE
  
#define NEW_MODE_DIRECTED_ANS_SECOND_NODE(LAST_NODE, ENTRY, INSTR)              \
        { ans_node_ptr new_node;					        \
	  NEW_ANSWER_TRIE_NODE(new_node, INSTR, ENTRY, NULL, LAST_NODE, NULL);  \
	  TrNode_child(LAST_NODE) = new_node;				        \
	  LAST_NODE = new_node;                                                 \
        }


static inline ans_node_ptr answer_search_min_max(sg_fr_ptr sg_fr, ans_node_ptr current_node, Term t, int mode USES_REGS) {
  /* ESTOU AQUIIIIIIIII - INICIO */
   ans_node_ptr child_node, first_child_node;
   Term child_term;
   Float trie_value = 0, term_value = 0;
   
   /* compute the value for the new term (term_value) */
  if (IsAtomOrIntTerm(t))
    term_value = (Float) IntOfTerm(t);
  else if (IsApplTerm(t)) {
    Functor f = FunctorOfTerm(t);
    if (f == FunctorLongInt)
      term_value = (Float) LongIntOfTerm(t);
    else if (f == FunctorDouble)
      term_value = FloatOfTerm(t);
    else
      Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_min_max: invalid arithmetic value");
  }

  /* compute the current value on the trie (trie_value) */
  child_node = first_child_node = TrNode_child(current_node);
  child_term = TrNode_entry(child_node);
  if (IsIntTerm(child_term)) {
    trie_value = (Float) IntOfTerm(child_term);
  } else if (IsApplTerm(child_term)) {
    Functor f = (Functor) RepAppl(child_term);
    child_node = TrNode_child(child_node);
    if (f == FunctorLongInt) {
      trie_value = (Float) TrNode_entry(child_node);
    } else if (f == FunctorDouble) {
      union {
	Term t_dbl[sizeof(Float)/sizeof(Term)];
	Float dbl;
      } u;
      u.t_dbl[0] = TrNode_entry(child_node);
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
      child_node = TrNode_child(child_node);
      u.t_dbl[1] = TrNode_entry(child_node);
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
      trie_value = u.dbl;
    } else
      Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_min_max: invalid arithmetic value");
    child_node = TrNode_child(child_node);
  }
  
  /* worse answer */
  if ((mode == MODE_DIRECTED_MIN && term_value > trie_value) || (mode == MODE_DIRECTED_MAX && term_value < trie_value))
    return NULL;
  /* equal answer */
  if (term_value == trie_value)
    return child_node;

  ans_node_ptr new_first_node = new_last_node = NULL;
  
  if (IsAtomOrIntTerm(t)) {
    NEW_MODE_DIRECTED_ANS_FIRST_NODE(new_first_node, new_last_node, current_node, t, _trie_retry_atom);
  } else if (IsApplTerm(t)) {
    Functor f = FunctorOfTerm(t);
    if (f == FunctorDouble) {
      union {
	Term t_dbl[sizeof(Float)/sizeof(Term)];
	Float dbl;
      } u;
      u.dbl = FloatOfTerm(t);
      NEW_MODE_DIRECTED_FIRST_ANS_NODE(new_first_node, new_last_node, current_node, AbsAppl((Term *)f), _trie_retry_null);
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
      NEW_MODE_DIRECTED_ANS_SECOND_NODE(new_last_node, u.t_dbl[1], _trie_retry_extension);
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
      NEW_MODE_DIRECTED_ANS_SECOND_NODE(new_last_node, u.t_dbl[0], _trie_retry_extension);
      NEW_MODE_DIRECTED_ANS_SECOND_NODE(new_last_node, AbsAppl((Term *)f), _trie_retry_double);
    } else if (f == FunctorLongInt) {
      Int li = LongIntOfTerm(t);
      NEW_MODE_DIRECTED_FIRST_ANS_NODE(new_first_node, new_last_node, current_node, AbsAppl((Term *)f), _trie_retry_null);
      NEW_MODE_DIRECTED_ANS_SECOND_NODE(new_last_node, li, _trie_retry_extension);
      NEW_MODE_DIRECTED_ANS_SECOND_NODE(new_last_node, AbsAppl((Term *)f), _trie_retry_longint);
    }
  }
  
  // invalidate answer before connecting the new answer

  //     I'M HERE !!!!!

  if(!IS_ANSWER_INVALID_NODE(first_child_node)) {
    LOCK_SG_FR(sg_fr);
    if(!IS_ANSWER_INVALID_NODE(first_child_node)) {
      TAG_AS_ANSWER_INVALID_NODE(first_child_node);
      SgFr_mark_invalid_chain(
    }
    UNLOCK_SG_FR(sg_fr);


  }
    

  while (!BOOL_CAS((&(TrNode_child(current_ans_node))), first_child_node, new_first_node)) {
    /* compute again the current value on the trie (trie_value) */
    child_node = first_child_node = TrNode_child(current_node);
    child_term = TrNode_entry(child_node);
    if (IsIntTerm(child_term)) {
      trie_value = (Float) IntOfTerm(child_term);
    } else if (IsApplTerm(child_term)) {
      Functor f = (Functor) RepAppl(child_term);
      child_node = TrNode_child(child_node);
      if (f == FunctorLongInt) {
	trie_value = (Float) TrNode_entry(child_node);
      } else if (f == FunctorDouble) {
	union {
	  Term t_dbl[sizeof(Float)/sizeof(Term)];
	  Float dbl;
	} u;
	u.t_dbl[0] = TrNode_entry(child_node);
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
	child_node = TrNode_child(child_node);
	u.t_dbl[1] = TrNode_entry(child_node);
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	trie_value = u.dbl;
      } else
	Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_min_max: invalid arithmetic value");
      child_node = TrNode_child(child_node);
    }
    
    /* worse answer */
    if ((mode == MODE_DIRECTED_MIN && term_value > trie_value) || (mode == MODE_DIRECTED_MAX && term_value < trie_value)) {
      /* free answer from new_first_node */
      do {
	new_last_node = TrNode_child(new_first_node); 
	FREE_ANSWER_TRIE_NODE(new_first_node);
	new_first_node = new_last_node;
      } while(new_first_node);
      return NULL;
    }
    /* equal answer */
    if (term_value == trie_value) {
      /* free answer from new_first_node */
      do {
	new_last_node = TrNode_child(new_first_node); 
	FREE_ANSWER_TRIE_NODE(new_first_node);
	new_first_node = new_last_node;
      } while(new_first_node);
      return child_node;
    }
    /* better answer */
    // invalidate answer before connecting the new answer

  }

  return new_last_node;
  /* ESTOU AQUIIIIIIIII - FIM */
}

#else /* !THREADS_FULL_SHARING_MODE_DIRECTED_V02 */

#define ANSWER_SAFE_INSERT_ENTRY(PARENT_NODE, ENTRY, INSTR)                       \
        { ans_node_ptr new_node;                                                  \
          NEW_ANSWER_TRIE_NODE(new_node, INSTR, ENTRY, NULL, PARENT_NODE, NULL);  \
	  TrNode_child(PARENT_NODE) = new_node;                                   \
          PARENT_NODE = new_node;                                                 \
	}

static inline ans_node_ptr answer_search_min_max(ans_node_ptr current_node, Term t, int mode USES_REGS) {
  ans_node_ptr child_node;
  Term child_term;
  Float trie_value = 0, term_value = 0;

  /* start by computing the current value on the trie (trie_value) */
  child_node = TrNode_child(current_node);
  child_term = TrNode_entry(child_node);
  if (IsIntTerm(child_term)) {
    trie_value = (Float) IntOfTerm(child_term);
  } else if (IsApplTerm(child_term)) {
    Functor f = (Functor) RepAppl(child_term);
    child_node = TrNode_child(child_node);
    if (f == FunctorLongInt) {
      trie_value = (Float) TrNode_entry(child_node);
    } else if (f == FunctorDouble) {
      union {
	Term t_dbl[sizeof(Float)/sizeof(Term)];
	Float dbl;
      } u;
      u.t_dbl[0] = TrNode_entry(child_node);
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
      child_node = TrNode_child(child_node);
      u.t_dbl[1] = TrNode_entry(child_node);
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
      trie_value = u.dbl;
    } else
      Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_min_max: invalid arithmetic value");
    child_node = TrNode_child(child_node);
  }

  /* then compute the value for the new term (term_value) */
  if (IsAtomOrIntTerm(t))
    term_value = (Float) IntOfTerm(t);
  else if (IsApplTerm(t)) {
    Functor f = FunctorOfTerm(t);
    if (f == FunctorLongInt)
      term_value = (Float) LongIntOfTerm(t);
    else if (f == FunctorDouble)
      term_value = FloatOfTerm(t);
    else
      Yap_Error(INTERNAL_ERROR, TermNil, "answer_search_min_max: invalid arithmetic value");
  }

  /* worse answer */
  if ((mode == MODE_DIRECTED_MIN && term_value > trie_value) || (mode == MODE_DIRECTED_MAX && term_value < trie_value))
    return NULL;
  /* equal answer */
  if (term_value == trie_value)
    return child_node;
  /* better answer */
  if (IsAtomOrIntTerm(t)) {
    ANSWER_SAFE_INSERT_ENTRY(current_node, t, _trie_retry_atom);
  } else if (IsApplTerm(t)) {
    Functor f = FunctorOfTerm(t);
    if (f == FunctorDouble) {
      union {
	Term t_dbl[sizeof(Float)/sizeof(Term)];
	Float dbl;
      } u;
      u.dbl = FloatOfTerm(t);
      ANSWER_SAFE_INSERT_ENTRY(current_node, AbsAppl((Term *)f), _trie_retry_null);
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
      ANSWER_SAFE_INSERT_ENTRY(current_node, u.t_dbl[1], _trie_retry_extension);
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
      ANSWER_SAFE_INSERT_ENTRY(current_node, u.t_dbl[0], _trie_retry_extension);
      ANSWER_SAFE_INSERT_ENTRY(current_node, AbsAppl((Term *)f), _trie_retry_double);
    } else if (f == FunctorLongInt) {
      Int li = LongIntOfTerm(t);
      ANSWER_SAFE_INSERT_ENTRY(current_node, AbsAppl((Term *)f), _trie_retry_null);
      ANSWER_SAFE_INSERT_ENTRY(current_node, li, _trie_retry_extension);
      ANSWER_SAFE_INSERT_ENTRY(current_node, AbsAppl((Term *)f), _trie_retry_longint);
    }
  }
  return current_node;
}
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */

#endif /* INCLUDE_ANSWER_SEARCH_MODE_DIRECTED */



/***************************************************************
**                   invalidate_answer_trie                   **
***************************************************************/

#ifdef INCLUDE_ANSWER_SEARCH_MODE_DIRECTED

#ifdef THREADS_FULL_SHARING

#define INVALIDATE_ANSWER_TRIE_LEAF_NODE(NODE, SG_FR)   \
  if (!IS_ANSWER_INVALID_NODE(NODE)) {                  \
    /*    printf("1-inv node = %p \n",NODE); */		\
    TAG_AS_ANSWER_INVALID_NODE(NODE);			\
    TrNode_next(NODE) = SgFr_invalid_chain(SG_FR);	\
    SgFr_invalid_chain(SG_FR) = NODE;                   \
  }

#define INVALIDATE_ANSWER_TRIE_NODE(NODE, SG_FR)        \
  /* printf("2-inv node = %p \n",NODE); */		\
  TAG_AS_ANSWER_INVALID_NODE(NODE)

#else /* !THREADS_FULL_SHARING */

#define INVALIDATE_ANSWER_TRIE_LEAF_NODE(NODE, SG_FR)   \
      TAG_AS_ANSWER_INVALID_NODE(NODE);                 \
      TrNode_next(NODE) = SgFr_invalid_chain(SG_FR);    \
       SgFr_invalid_chain(SG_FR) = NODE

#define INVALIDATE_ANSWER_TRIE_NODE(NODE, SG_FR)        \
        FREE_ANSWER_TRIE_NODE(NODE)

#endif /* THREADS_FULL_SHARING */


static void invalidate_answer_trie(ans_node_ptr current_node, sg_fr_ptr sg_fr, int position USES_REGS) {
  if (IS_ANSWER_TRIE_HASH(current_node)) {
    ans_hash_ptr hash;
    ans_node_ptr *bucket, *last_bucket;
    hash = (ans_hash_ptr) current_node;
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
    bucket = AnsHash_buckets(hash);
    last_bucket = bucket + AnsHash_num_buckets(hash);
#else      
    bucket = Hash_buckets(hash);
    last_bucket = bucket + Hash_num_buckets(hash);
#endif  /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL */
    do {
      current_node = *bucket;
      if (current_node) {
	ans_node_ptr next_node = TrNode_next(current_node);
	if (IS_ANSWER_LEAF_NODE(current_node)) {
	  INVALIDATE_ANSWER_TRIE_LEAF_NODE(current_node, sg_fr);
	} else {
	  invalidate_answer_trie(TrNode_child(current_node), sg_fr, TRAVERSE_POSITION_FIRST PASS_REGS);
	  INVALIDATE_ANSWER_TRIE_NODE(current_node, sg_fr);
	}
	while (next_node) {
	  current_node = next_node;
	  next_node = TrNode_next(current_node);
	  invalidate_answer_trie(current_node, sg_fr, TRAVERSE_POSITION_NEXT PASS_REGS);
	}
      }
    } while (++bucket != last_bucket); 
    //    LOCK_SG_FR(sg_fr);
    if (Hash_next(hash))
      Hash_previous(Hash_next(hash)) = Hash_previous(hash);
    if (Hash_previous(hash))
      Hash_next(Hash_previous(hash)) = Hash_next(hash);
    else
      SgFr_hash_chain(sg_fr) = Hash_next(hash);
#ifdef THREADS_FULL_SHARING
    Hash_next(hash) = SgFr_old_hash_chain(sg_fr);
    SgFr_old_hash_chain(sg_fr) = hash;
#else    
    FREE_BUCKETS(Hash_buckets(hash));
    FREE_ANSWER_TRIE_HASH(hash);
#endif /* THREADS_FULL_SHARING */
    //    UNLOCK_SG_FR(sg_fr);

  } else {
    if (position == TRAVERSE_POSITION_FIRST) {
      ans_node_ptr next_node = TrNode_next(current_node);
      if (IS_ANSWER_LEAF_NODE(current_node)) {
	INVALIDATE_ANSWER_TRIE_LEAF_NODE(current_node, sg_fr);
      } else {
	invalidate_answer_trie(TrNode_child(current_node), sg_fr, TRAVERSE_POSITION_FIRST PASS_REGS);
	INVALIDATE_ANSWER_TRIE_NODE(current_node, sg_fr);
      }
      while (next_node) {
	current_node = next_node;
	next_node = TrNode_next(current_node);
	invalidate_answer_trie(current_node, sg_fr, TRAVERSE_POSITION_NEXT PASS_REGS);
      }
    } else {
      if (IS_ANSWER_LEAF_NODE(current_node)) {
	INVALIDATE_ANSWER_TRIE_LEAF_NODE(current_node, sg_fr);
      } else {
	invalidate_answer_trie(TrNode_child(current_node), sg_fr, TRAVERSE_POSITION_FIRST PASS_REGS);
	INVALIDATE_ANSWER_TRIE_NODE(current_node, sg_fr);
      }
    }
  }
  return;
}
#endif /* INCLUDE_ANSWER_SEARCH_MODE_DIRECTED */



/************************************************************************
**                   load_(answer|substitution)_loop                   **
************************************************************************/

#ifdef INCLUDE_LOAD_ANSWER_LOOP
#ifdef MODE_GLOBAL_TRIE_LOOP
static inline CELL *load_substitution_loop(gt_node_ptr current_node, int *vars_arity_ptr, CELL *stack_terms USES_REGS) {
#else
static inline CELL *load_answer_loop(ans_node_ptr current_node USES_REGS) {
#endif /* MODE_GLOBAL_TRIE_LOOP */
/************************************************************************
                     ===========
                     |         |
                     |   ...   |
                     |         |
                     -----------
             TR -->  |  VAR_0  |  <-- stack_vars_base
                     -----------           *
                     |   ...   |          /|\
                     -----------           |  vars_arity (N+1)
                     |  VAR_N  |          \|/
                     -----------           *
                     |         |  <-- stack_terms_limit
                     -----------
                     |         |
                     |   ...   |
                     |         |
                     ----------|
                     |  TERM_N |  <-- stack_terms
                     ----------|           *
                     |   ...   |          /|\
                     ----------|           |  stack_terms_pair_offset (TRIE_COMPACT_PAIRS)
                     |  TERM_1 |          \|/
                     ===========           *
 LOCAL_TrailTop -->  |         |  <-- stack_terms_base (TRIE_COMPACT_PAIRS)
                     -----------
************************************************************************/
#ifdef MODE_GLOBAL_TRIE_LOOP
  int vars_arity = *vars_arity_ptr;
#else
  int vars_arity = 0;
  CELL *stack_terms = (CELL *) LOCAL_TrailTop;
#endif /* MODE_GLOBAL_TRIE_LOOP */
  CELL *stack_vars_base = (CELL *) TR;
#define stack_terms_limit (stack_vars_base + vars_arity)
#ifdef TRIE_COMPACT_PAIRS
#define stack_terms_base ((CELL *) LOCAL_TrailTop)
  int stack_terms_pair_offset = 0;
#endif /* TRIE_COMPACT_PAIRS */
  Term t = TrNode_entry(current_node);
#ifdef MODE_GLOBAL_TRIE_LOOP
  current_node = TrNode_parent(current_node);
#else
  current_node = (ans_node_ptr) UNTAG_ANSWER_NODE(TrNode_parent(current_node));
#endif /* MODE_GLOBAL_TRIE_LOOP */

  do {
    if (IsVarTerm(t)) {
#if ! defined(MODE_GLOBAL_TRIE_LOOP) || defined(GLOBAL_TRIE_FOR_SUBTERMS)
      if (t > VarIndexOfTableTerm(MAX_TABLE_VARS)) {
	stack_terms = load_substitution_loop((gt_node_ptr) t, &vars_arity, stack_terms PASS_REGS);
      } else 
#endif /* ! MODE_GLOBAL_TRIE_LOOP || GLOBAL_TRIE_FOR_SUBTERMS */
      { int var_index = VarIndexOfTableTerm(t);
	AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit - vars_arity + var_index + 1);
	if (var_index >= vars_arity) {
	  while (vars_arity < var_index)
	    stack_vars_base[vars_arity++] = 0; 
	  stack_vars_base[vars_arity++] = MkVarTerm(); 
	} else if (stack_vars_base[var_index] == 0)
	  stack_vars_base[var_index] = MkVarTerm(); 
	STACK_PUSH_UP(stack_vars_base[var_index], stack_terms);
      }
    } else if (IsAtomOrIntTerm(t)) {
      AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit);
      STACK_PUSH_UP(t, stack_terms);
    } else if (IsPairTerm(t)) {
#ifdef TRIE_COMPACT_PAIRS
      if (t == CompactPairInit) { 
	Term *stack_aux = stack_terms_base - stack_terms_pair_offset;
	Term head, tail = STACK_POP_UP(stack_aux);
	while (STACK_NOT_EMPTY(stack_aux, stack_terms)) {
	  head = STACK_POP_UP(stack_aux);
	  tail = MkPairTerm(head, tail);
	}
	stack_terms = stack_terms_base - stack_terms_pair_offset;
	stack_terms_pair_offset = (int) STACK_POP_DOWN(stack_terms);
	STACK_PUSH_UP(tail, stack_terms);
      } else {  /* CompactPairEndList / CompactPairEndTerm */
	Term last;
	AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit + 1);
	last = STACK_POP_DOWN(stack_terms);
	STACK_PUSH_UP(stack_terms_pair_offset, stack_terms);
	stack_terms_pair_offset = (int) (stack_terms_base - stack_terms);
	if (t == CompactPairEndList)
	  STACK_PUSH_UP(TermNil, stack_terms);
	STACK_PUSH_UP(last, stack_terms);
      }
#else /* ! TRIE_COMPACT_PAIRS */
      Term head = STACK_POP_DOWN(stack_terms);
      Term tail = STACK_POP_DOWN(stack_terms);
      t = MkPairTerm(head, tail);
      STACK_PUSH_UP(t, stack_terms);
#endif /* TRIE_COMPACT_PAIRS */
    } else if (IsApplTerm(t)) {
      Functor f = (Functor) RepAppl(t);
      if (f == FunctorDouble) {
	union {
	  Term t_dbl[sizeof(Float)/sizeof(Term)];
	  Float dbl;
	} u;
	t = TrNode_entry(current_node);
	current_node = TrNode_parent(current_node);
	u.t_dbl[0] = t;
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
	t = TrNode_entry(current_node);
	current_node = TrNode_parent(current_node);
	u.t_dbl[1] = t;
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	current_node = TrNode_parent(current_node);
	t = MkFloatTerm(u.dbl);
      } else if (f == FunctorLongInt) {
	Int li = TrNode_entry(current_node);
	current_node = TrNode_parent(current_node);
	current_node = TrNode_parent(current_node);
	t = MkLongIntTerm(li);
      } else {
	int f_arity = ArityOfFunctor(f);
	t = Yap_MkApplTerm(f, f_arity, stack_terms);
	stack_terms += f_arity;
      }
      AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit);
      STACK_PUSH_UP(t, stack_terms);
    }

    t = TrNode_entry(current_node);
    current_node = TrNode_parent(current_node);
  } while (current_node);

#ifdef MODE_GLOBAL_TRIE_LOOP
  *vars_arity_ptr = vars_arity;
#endif /* MODE_GLOBAL_TRIE_LOOP */
  return stack_terms;

#undef stack_terms_limit
#ifdef TRIE_COMPACT_PAIRS
#undef stack_terms_base
#endif /* TRIE_COMPACT_PAIRS */
}
#endif /* INCLUDE_LOAD_ANSWER_LOOP */



/***************************
**      Undef Macros      **
***************************/

#undef INCREMENT_GLOBAL_TRIE_REFERENCE
#undef NEW_SUBGOAL_TRIE_NODE
#undef NEW_ANSWER_TRIE_NODE
#undef NEW_GLOBAL_TRIE_NODE
#undef SUBGOAL_CHECK_INSERT_ENTRY
#undef ANSWER_CHECK_INSERT_ENTRY
