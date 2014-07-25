#ifndef _LOCK_FREE_HASH.TRIES.H
#define _LOCK_FREE_HASH.TRIES.H

/*******************************************************************************
 *                            LFHT macros                                      *
 *******************************************************************************/
#define LFHT_BIT_SHIFT                               3
#define LFHT_BUCKET_ARRAY_SIZE                       (1 << LFHT_BIT_SHIFT)
#define LFHT_MAX_NODES_PER_BUCKET                    4
#define LFHT_CELL                                    long
#define LFHT_IsEqualEntry(NODE, ENTRY)               (LFHT_NodeEntry(NODE) == ENTRY)
#define LFHT_IsHashLevel(PTR)                        ((LFHT_CELL)(PTR) & (LFHT_CELL)(0x1))
#define LFHT_TagAsHashLevel(PTR)             /*v04_tag */               ((LFHT_CELL)(PTR) | (LFHT_CELL)0x1)
#define LFHT_UntagHashLevel(PTR)             /*v04_untag */             ((LFHT_CELL)(PTR) & ~(LFHT_CELL)(0x1))
#define LFHT_SetBucketEntry(BUCKET, V, STR)  /* V04_SET_HASH_BUCKET */  (*(BUCKET) = (STR *) V)
#define LFHT_BoolCAS(PTR, OLD, NEW)          /* LFHT_BOOL_CAS */   __sync_bool_compare_and_swap((PTR), (OLD), (NEW))



/* V04_GET_HASH_BUCKET */
#define LFHT_GetBucketEntry(B, H, E, NS, STR)  (B = (STR **) LFHT_UntagHashLevel(H) + V04_HASH_ENTRY((LFHT_CELL)E, NS))

/*******************************************************************************
 *                            YapTab compatibility stuff                       *
 *******************************************************************************/

#define LFHT_NodeEntry(NODE)                           TrNode_entry(NODE)
#define LFHT_NodeNext(NODE)                            TrNode_next(NODE)
#define LFHT_GetFirstNode(NODE)                        (NODE = (LFHT_STR_PTR) TrNode_child(parent_node))
#define LFHT_NrLowTagBits                              NumberOfLowTagBits /* 0 (zero) if none */
#define LFHT_USES_REGS                                 USES_REGS          /* BLANC if no TabMalloc */   
#define LFHT_PASS_REGS                                 PASS_REGS          /* BLANC if no TabMalloc */ 
#define LFHT_NODE_ENTRY_STR                            Term

/* integrated with TabMalloc. If no TabMalloc, then use malloc */
#define LFHT_MemAllocBuckets(STR)   /* V04_ALLOC_THB */			\
  union trie_hash_buckets *aux;						\
  ALLOC_STRUCT(aux, union trie_hash_buckets, _pages_trie_hash_buckets); \
  STR = aux->hash_buckets

#define LFHT_InitBuckets(BUCKET_PTR, PREV_HASH)                         \
  { int i; void **init_bucket_ptr;                                      \
    *BUCKET_PTR++ = (void *) (PREV_HASH);                               \
    init_bucket_ptr = (void **) BUCKET_PTR;                             \
    for (i = LFHT_BUCKET_ARRAY_SIZE; i != 0; i--)                       \
      *init_bucket_ptr++ = (void *) LFHT_TagAsHashLevel(BUCKET_PTR);    \
  }

#if defined(LFHT_LOCAL_THREAD_BUFFER_FOR_BUCKET_ARRAYS)
#define LFHT_LOCAL_BUCKET_BUFFER           LOCAL_trie_buckets_buffer

#define LFHT_AllocBuckets(BUCKET_PTR, PREV_HASH, STR)	    \
  { void **alloc_bucket_ptr;				    \
    if(LFHT_LOCAL_BUCKET_BUFFER == NULL) {		    \
      LFHT_MemAllocBuckets(alloc_bucket_ptr);		    \
      LFHT_InitBuckets(alloc_bucket_ptr, PREV_HASH);	    \
    } else {						    \
      alloc_bucket_ptr = LFHT_LOCAL_BUCKET_BUFFER;	    \
      LFHT_LOCAL_BUCKET_BUFFER = NULL;			    \
      *alloc_bucket_ptr++ = (void *) (PREV_HASH);	    \
    }							    \
    BUCKET_PTR = (STR **) alloc_bucket_ptr;	            \
  }

#define LFHT_FreeBuckets(PTR, BKT, STR)    /* V04_FREE_TRIE_HASH_BUCKETS */ \
  { LFHT_SetBucketEntry(BKT, PTR, STR);				            \
    LFHT_LOCAL_BUCKET_BUFFER = (((void**)LFHT_UntagHashLevel(PTR)) - 1);    \
  }

#else /* !LFHT_LOCAL_THREAD_BUFFER_FOR_BUCKET_ARRAYS */

#define LFHT_AllocBuckets(BUCKET_PTR, PREV_HASH, STR)	                             \
  { void **alloc_bucket_ptr;						             \
    LFHT_MemAllocBuckets(alloc_bucket_ptr);					     \
    LFHT_InitBuckets(alloc_bucket_ptr, PREV_HASH);                                   \
    BUCKET_PTR = (STR **) alloc_bucket_ptr;                                          \
  }

#define LFHT_FreeBuckets(PTR, BKT, STR)
  /* !! -- commented ... check this later */
  /* V04_FREE_THB(((STR *) V04_UNTAG(PTR)) - 1) 
     FREE_STRUCT((union trie_hash_buckets*)STR, union trie_hash_buckets, _pages_trie_hash_buckets)
     FREE_BLOCK(((ans_node_ptr *) V04_UNTAG(STR)) - 1) */

#endif /* LFHT_LOCAL_THREAD_BUFFER_FOR_BUCKET_ARRAYS */

/////////////////////////////////////////// ok upto HERE !!!!!

#define V04_SHIFT_ENTRY(ENTRY, N_SHIFTS)               ((ENTRY) >> ((SHIFT_SIZE * (N_SHIFTS)) + LFHT_NrLowTagBits))
#define V04_HASH_ENTRY(ENTRY, N_SHIFTS)                (V04_SHIFT_ENTRY(ENTRY, N_SHIFTS) & (BASE_HASH_BUCKETS - 1))
#define V04_IS_EMPTY_BUCKET(BUCKET, BASE_BUCKET, STR)  (BUCKET == (STR *) BASE_BUCKET)


#define V04_GET_PREV_HASH(PREV_HASH, CURR_HASH, STR)   (PREV_HASH = (STR **) *(((STR **) V04_UNTAG(CURR_HASH)) - 1))

#endif /* _LOCK_FREE_HASH.TRIES.H */
