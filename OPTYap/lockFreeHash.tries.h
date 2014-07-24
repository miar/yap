#ifndef _LOCK_FREE_HASH.TRIES.H
#define _LOCK_FREE_HASH.TRIES.H

/*******************************************************************************
 *                            compatibility stuff                              *
 *******************************************************************************/

#define LFHT_NodeEntry(NODE)                           TrNode_entry(NODE)
#define LFHT_NodeNext(NODE)                            TrNode_next(NODE)
#define LFHT_NrLowTagBits                              NumberOfLowTagBits /* 0 (zero) if none */
#define LFHT_USES_REGS                                 USES_REGS          /* BLANC if no TabMalloc */   
#define LFHT_PASS_REGS                                 PASS_REGS          /* BLANC if no TabMalloc */ 
#define LFHT_NODE_ENTRY_STR                            Term
/*******************************************************************************
 *                            LFHT macros                                      *
 *******************************************************************************/
#define LFHT_CELL                                    long
#define LFHT_IsEqualEntry(NODE, ENTRY)               (LFHT_NodeEntry(NODE) == ENTRY)
#define LFHT_IsHashLevel(PTR)                        ((LFHT_CELL)(PTR) & (LFHT_CELL)(0x1))

/////////////////////////////////////////// ok upto here !!!!!



#define SHIFT_SIZE                                     3
#define LFHT_BUCKET_SIZE                               (1 << SHIFT_SIZE)
#define V04_SHIFT_ENTRY(ENTRY, N_SHIFTS)               ((ENTRY) >> ((SHIFT_SIZE * (N_SHIFTS)) + LFHT_NrLowTagBits))
#define V04_HASH_ENTRY(ENTRY, N_SHIFTS)                (V04_SHIFT_ENTRY(ENTRY, N_SHIFTS) & (BASE_HASH_BUCKETS - 1))
#define V04_IS_EMPTY_BUCKET(BUCKET, BASE_BUCKET, STR)  (BUCKET == (STR *) BASE_BUCKET)
#define V04_TAG(PTR)                                   ((long)(PTR) | (long)0x1)
#define V04_UNTAG(PTR)                                 ((long)(PTR) & ~(long)(0x1))


#define V04_GET_HASH_BUCKET(BUCKET, HASH, T, NS, STR)  (BUCKET = (STR **) V04_UNTAG(HASH) + V04_HASH_ENTRY((long)T, NS))
#define V04_GET_PREV_HASH(PREV_HASH, CURR_HASH, STR)   (PREV_HASH = (STR **) *(((STR **) V04_UNTAG(CURR_HASH)) - 1))
#define V04_SET_HASH_BUCKET(BUCKET, V, STR)            (*(BUCKET) = (STR *) V)
#define LFHT_BOOL_CAS(PTR, OLD, NEW)                    __sync_bool_compare_and_swap((PTR), (OLD), (NEW))





#endif /* _LOCK_FREE_HASH.TRIES.H */
