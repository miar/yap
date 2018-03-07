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


/************************************
**      Includes & Prototypes      **
************************************/

#include <stdlib.h>
#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#include "opt.mavar.h"

#ifdef LINEAR_TABLING
#include "linear.tab.macros.h"
#endif /*LINEAR TABLING */


#ifdef EXTRA_STATISTICS_CHOICE_POINTS
#define  SgEnt_init_extra_statistics_choice_points(SG_ENT)	       \
  SgEnt_query_number(SG_ENT)= -1;

#else
#define  SgEnt_init_extra_statistics_choice_points(SG_ENT)
#endif /* EXTRA_STATISTICS_CHOICE_POINTS */


#ifdef EXTRA_STATISTICS
#define Extra_stats_ans_trie(NEW_DEPTH)			                \
  Stats_answer_trie_nr_paths++;						\
  Stats_answer_trie_depth_all += NEW_DEPTH;				\
  if (NEW_DEPTH > Stats_answer_trie_depth_max_all)		        \
    Stats_answer_trie_depth_max_all = NEW_DEPTH;			\
  if (NEW_DEPTH < Stats_answer_trie_depth_min_all)		        \
    Stats_answer_trie_depth_min_all = NEW_DEPTH

#define Extra_stats_sg_trie(NEW_DEPTH)			                \
  Stats_subgoal_trie_nr_paths++;					\
  Stats_subgoal_trie_depth_all += NEW_DEPTH;				\
  if (NEW_DEPTH > Stats_subgoal_trie_depth_max_all)		        \
    Stats_subgoal_trie_depth_max_all = NEW_DEPTH;			\
  if (NEW_DEPTH < Stats_subgoal_trie_depth_min_all)		        \
    Stats_subgoal_trie_depth_min_all = NEW_DEPTH

#else
#define Extra_stats_sg_trie(NEW_DEPTH)
#define Extra_stats_ans_trie(NEW_DEPTH)
#endif /* EXTRA_STATISTICS */

#ifdef THREADS
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
static inline void **get_insert_thread_bucket(void **);
#else
static inline void **get_insert_thread_bucket(void **, lockvar *);
#endif
static inline void **get_thread_bucket(void **);
static inline void abolish_thread_buckets(void **);
#if defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03)
static inline int adjust_subgoal_hash_nodes_first_exp(sg_node_ptr, sg_node_ptr *, int);
static inline void adjust_subgoal_hash_nodes(sg_node_ptr, sg_node_ptr *, int);
#endif
#if defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03)
static inline int adjust_answer_hash_nodes_first_exp(ans_node_ptr, ans_node_ptr *, int);
static inline void adjust_answer_hash_nodes(ans_node_ptr, ans_node_ptr *, int);
#endif
#endif /* THREADS */
static inline sg_node_ptr get_insert_subgoal_trie(tab_ent_ptr USES_REGS);
static inline sg_node_ptr get_subgoal_trie(tab_ent_ptr);
static inline sg_node_ptr get_subgoal_trie_for_abolish(tab_ent_ptr USES_REGS);
static inline sg_fr_ptr *get_insert_subgoal_frame_addr(sg_node_ptr USES_REGS);
static inline sg_fr_ptr get_subgoal_frame(sg_node_ptr);
static inline sg_fr_ptr get_subgoal_frame_for_abolish(sg_node_ptr USES_REGS);
#ifdef THREADS_FULL_SHARING
#ifdef THREADS_FULL_SHARING_FTNA_3
static inline void consumer_trie_insert_bucket_chain(ans_ref_ptr *, ans_ref_ptr, ans_ref_ptr, long, int USES_REGS);
static inline void consumer_trie_insert_bucket_array(ans_ref_ptr *, ans_ref_ptr, long USES_REGS);
static inline void consumer_trie_adjust_chain_nodes(ans_ref_ptr *, ans_ref_ptr, ans_ref_ptr, long USES_REGS);
static inline boolean consumer_trie_check_insert_bucket_chain(ans_ref_ptr *, ans_ref_ptr, sg_fr_ptr, ans_node_ptr, long, int USES_REGS);
static inline boolean consumer_trie_check_insert_bucket_array(ans_ref_ptr *, sg_fr_ptr, ans_node_ptr, long USES_REGS);
static inline boolean consumer_trie_check_insert_first_chain(ans_ref_ptr, sg_fr_ptr, ans_node_ptr, int USES_REGS);
static inline boolean consumer_trie_check_insert_node(sg_fr_ptr, ans_node_ptr USES_REGS);
static inline void consumer_trie_free_structs(sg_fr_ptr USES_REGS);
static inline void consumer_trie_free_bucket_array(ans_ref_ptr * USES_REGS);

#endif /* THREADS_FULL_SHARING_FTNA_3 */
#endif /* THREADS_FULL_SHARING */
static inline Int freeze_current_cp(void);
static inline void wake_frozen_cp(Int);
static inline void abolish_frozen_cps_until(Int);
static inline void abolish_frozen_cps_all(void);
static inline void adjust_freeze_registers(void);
static inline void mark_as_completed(sg_fr_ptr USES_REGS);
static inline void unbind_variables(tr_fr_ptr, tr_fr_ptr);
static inline void rebind_variables(tr_fr_ptr, tr_fr_ptr);
static inline void restore_bindings(tr_fr_ptr, tr_fr_ptr);
static inline CELL *expand_auxiliary_stack(CELL *);
static inline void abolish_incomplete_subgoals(choiceptr);
#ifdef YAPOR
static inline void pruning_over_tabling_data_structures(void);
static inline void collect_suspension_frames(or_fr_ptr);
#ifdef TIMESTAMP_CHECK
static inline susp_fr_ptr suspension_frame_to_resume(or_fr_ptr, long);
#else
static inline susp_fr_ptr suspension_frame_to_resume(or_fr_ptr);
#endif /* TIMESTAMP_CHECK */
#endif /* YAPOR */
#ifdef TABLING_INNER_CUTS
static inline void CUT_store_tg_answer(or_fr_ptr, ans_node_ptr, choiceptr, int);
static inline tg_sol_fr_ptr CUT_store_tg_answers(or_fr_ptr, tg_sol_fr_ptr, int);
static inline void CUT_validate_tg_answers(tg_sol_fr_ptr);
static inline void CUT_join_tg_solutions(tg_sol_fr_ptr *, tg_sol_fr_ptr);
static inline void CUT_join_solution_frame_tg_answers(tg_sol_fr_ptr);
static inline void CUT_join_solution_frames_tg_answers(tg_sol_fr_ptr);
static inline void CUT_free_tg_solution_frame(tg_sol_fr_ptr);
static inline void CUT_free_tg_solution_frames(tg_sol_fr_ptr);
static inline tg_sol_fr_ptr CUT_prune_tg_solution_frames(tg_sol_fr_ptr, int);
#endif /* TABLING_INNER_CUTS */
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
static void invalidate_answer_trie(ans_node_ptr, sg_fr_ptr, int USES_REGS);
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */


/*********************************
**      Tabling mode flags      **
*********************************/

#define Flag_Batched            0x0001
#define Flag_Local              0x0002
#define Flags_SchedulingMode    (Flag_Batched | Flag_Local)
#define Flag_ExecAnswers        0x0010
#define Flag_LoadAnswers        0x0020
#define Flags_AnswersMode       (Flag_ExecAnswers | Flag_LoadAnswers)
#define Flag_LocalTrie          0x0100
#define Flag_GlobalTrie         0x0200
#define Flags_TrieMode          (Flag_LocalTrie | Flag_GlobalTrie)

#define Flag_Suspension         0x1000
#define Flag_Linear             0x2000
#define Flags_TablingMode       (Flag_Suspension | Flag_Linear)

#define SetMode_Batched(X)        (X) = ((X) & ~Flags_SchedulingMode) | Flag_Batched
#define SetMode_Local(X)          (X) = ((X) & ~Flags_SchedulingMode) | Flag_Local
#define SetMode_ExecAnswers(X)    (X) = ((X) & ~Flags_AnswersMode) | Flag_ExecAnswers
#define SetMode_LoadAnswers(X)    (X) = ((X) & ~Flags_AnswersMode) | Flag_LoadAnswers
#define SetMode_LocalTrie(X)      (X) = ((X) & ~Flags_TrieMode) | Flag_LocalTrie
#define SetMode_GlobalTrie(X)     (X) = ((X) & ~Flags_TrieMode) | Flag_GlobalTrie
#define SetMode_Suspension(X)     (X) = ((X) & ~Flags_TablingMode) | Flag_Suspension
#define SetMode_Linear(X)         (X) = ((X) & ~Flags_TablingMode) | Flag_Linear

#define IsMode_Batched(X)       ((X) & Flag_Batched)
#define IsMode_Local(X)         ((X) & Flag_Local)
#define IsMode_ExecAnswers(X)   ((X) & Flag_ExecAnswers)
#define IsMode_LoadAnswers(X)   ((X) & Flag_LoadAnswers)
#define IsMode_LocalTrie(X)     ((X) & Flag_LocalTrie)
#define IsMode_GlobalTrie(X)    ((X) & Flag_GlobalTrie)
#define IsMode_Suspension(X)    ((X) & Flag_Suspension)
#define IsMode_Linear(X)        ((X) & Flag_Linear)


/******************************
**      Tabling defines      **
******************************/

/* traverse macros */
#define SHOW_MODE_STRUCTURE        0
#define SHOW_MODE_STATISTICS       1
#define TRAVERSE_MODE_NORMAL       0
#define TRAVERSE_MODE_DOUBLE       1
#define TRAVERSE_MODE_DOUBLE2      2
#define TRAVERSE_MODE_DOUBLE_END   3
#define TRAVERSE_MODE_LONGINT      4
#define TRAVERSE_MODE_LONGINT_END  5
/* do not change order !!! */
#define TRAVERSE_TYPE_SUBGOAL      0
#define TRAVERSE_TYPE_ANSWER       1
#define TRAVERSE_TYPE_GT_SUBGOAL   2
#define TRAVERSE_TYPE_GT_ANSWER    3
/* do not change order !!! */
#define TRAVERSE_POSITION_NEXT     0
#define TRAVERSE_POSITION_FIRST    1
#define TRAVERSE_POSITION_LAST     2

/* mode directed tabling */
#define MODE_DIRECTED_TAGBITS           0xF
#define MODE_DIRECTED_NUMBER_TAGBITS    4
#define MODE_DIRECTED_INDEX             1
#define MODE_DIRECTED_FIRST             2
#define MODE_DIRECTED_ALL               3
#define MODE_DIRECTED_MAX               4
#define MODE_DIRECTED_MIN               5
#define MODE_DIRECTED_LAST              6
#define MODE_DIRECTED_DIM               7
#define MODE_DIRECTED_SUM               8
#define MODE_DIRECTED_DIM_INTEGER       9
#define MODE_DIRECTED_DIM_FLOAT        10
#define MODE_DIRECTED_DIM_BIG_INTEGER  11
#define MODE_DIRECTED_DIM_BIG_FLOAT    12

#define NoTrie_LoadIntegerTerm(ANSWER)     (MkIntTerm((ANSWER)))
#define NoTrie_LoadFloatTerm(ANSWER)       (MkFloatTerm((ANSWER)))
#define NoTrie_LoadBigIntegerTerm(ANSWER)  (Yap_MkBigIntTerm((ANSWER)))

#define MODE_DIRECTED_SET(ARG,MODE)   (((ARG) << MODE_DIRECTED_NUMBER_TAGBITS) + MODE)
#define MODE_DIRECTED_GET_ARG(X)      ((X) >> MODE_DIRECTED_NUMBER_TAGBITS)
#define MODE_DIRECTED_GET_MODE(X)     ((X) & MODE_DIRECTED_TAGBITS)

/* LowTagBits is 3 for 32 bit-machines and 7 for 64 bit-machines */
#define NumberOfLowTagBits         (LowTagBits == 3 ? 2 : 3)
#define MakeTableVarTerm(INDEX)    ((INDEX) << NumberOfLowTagBits)
#define VarIndexOfTableTerm(TERM)  (((unsigned int) (TERM)) >> NumberOfLowTagBits)
#define VarIndexOfTerm(TERM)                                                     \
        ((((CELL) (TERM)) - GLOBAL_table_var_enumerator(0)) / sizeof(CELL))
#define IsTableVarTerm(TERM)                                                     \
        ((CELL) (TERM)) >= GLOBAL_table_var_enumerator(0) &&		 	 \
        ((CELL) (TERM)) <= GLOBAL_table_var_enumerator(MAX_TABLE_VARS - 1)
#ifdef TRIE_COMPACT_PAIRS
#define PairTermMark        NULL
#define CompactPairInit     AbsPair((Term *) 0)
#define CompactPairEndTerm  AbsPair((Term *) (LowTagBits + 1))
#define CompactPairEndList  AbsPair((Term *) (2*(LowTagBits + 1)))
#endif /* TRIE_COMPACT_PAIRS */

/* threads */
#if (_trie_retry_gterm - _trie_do_var + 1) + 1 <= 64  /* 60 (trie instructions) + 1 (ANSWER_TRIE_HASH_MARK) <= 64 */
#define ANSWER_LEAF_NODE_INSTR_BITS   6               /* 2^6 = 64 */
#define ANSWER_LEAF_NODE_INSTR_MASK   0x3F
#endif
#if SIZEOF_INT_P == 4
#define ANSWER_LEAF_NODE_MAX_THREADS  (32 - ANSWER_LEAF_NODE_INSTR_BITS)
#elif SIZEOF_INT_P == 8
#define ANSWER_LEAF_NODE_MAX_THREADS  (64 - ANSWER_LEAF_NODE_INSTR_BITS)
#else
#define ANSWER_LEAF_NODE_MAX_THREADS  OOOOPPS!!! Unknown Pointer Sizeof
#endif /* SIZEOF_INT_P */
#define ANSWER_LEAF_NODE_INSTR_RELATIVE(NODE)  (TrNode_instr(NODE) = TrNode_instr(NODE) - _trie_do_var + 1)
#define ANSWER_LEAF_NODE_INSTR_ABSOLUTE(NODE)  (TrNode_instr(NODE) = (TrNode_instr(NODE) & ANSWER_LEAF_NODE_INSTR_MASK) + _trie_do_var - 1)
#define ANSWER_LEAF_NODE_SET_WID(NODE,WID)     BITMAP_insert(TrNode_instr(NODE), WID + ANSWER_LEAF_NODE_INSTR_BITS)
#define ANSWER_LEAF_NODE_DEL_WID(NODE,WID)     BITMAP_delete(TrNode_instr(NODE), WID + ANSWER_LEAF_NODE_INSTR_BITS)
#define ANSWER_LEAF_NODE_CHECK_WID(NODE,WID)   BITMAP_member(TrNode_instr(NODE), WID + ANSWER_LEAF_NODE_INSTR_BITS)

/* choice points */
#define NORM_CP(CP)                 ((choiceptr)(CP))
#define GEN_CP(CP)                  ((struct generator_choicept *)(CP))
#define CONS_CP(CP)                 ((struct consumer_choicept *)(CP))
#define LOAD_CP(CP)                 ((struct loader_choicept *)(CP))
#ifdef DETERMINISTIC_TABLING
#define DET_GEN_CP(CP)              ((struct deterministic_generator_choicept *)(CP))
#define IS_DET_GEN_CP(CP)           (*(CELL*)(DET_GEN_CP(CP) + 1) <= MAX_TABLE_VARS)
#define IS_BATCHED_NORM_GEN_CP(CP)  (GEN_CP(CP)->cp_dep_fr == NULL)
#define IS_BATCHED_GEN_CP(CP)       (IS_DET_GEN_CP(CP) || IS_BATCHED_NORM_GEN_CP(CP))
#else
#define IS_BATCHED_GEN_CP(CP)       (GEN_CP(CP)->cp_dep_fr == NULL)
#endif /* DETERMINISTIC_TABLING */

/* tagging nodes */
#define TAG_AS_SUBGOAL_LEAF_NODE(NODE)       TrNode_child(NODE) = (sg_node_ptr)((CELL) TrNode_child(NODE) | 0x1)
#define IS_SUBGOAL_LEAF_NODE(NODE)           ((CELL) TrNode_child(NODE) & 0x1)

#define TAG_AS_ANSWER_LEAF_NODE(NODE)        TrNode_parent(NODE) = (ans_node_ptr)((CELL) TrNode_parent(NODE) | 0x1)

#define IS_ANSWER_LEAF_NODE(NODE)            ((CELL) TrNode_parent(NODE) & 0x1)

#define TAG_AS_ANSWER_INVALID_NODE(NODE)     TrNode_parent(NODE) = (ans_node_ptr)((CELL) TrNode_parent(NODE) | 0x2)
#define IS_ANSWER_INVALID_NODE(NODE)         ((CELL) TrNode_parent(NODE) & 0x2)
#define UNTAG_SUBGOAL_NODE(NODE)             ((CELL) (NODE) & ~(0x1))
#define UNTAG_ANSWER_NODE(NODE)              ((CELL) (NODE) & ~(0x3))
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02 
#define TAG_AS_INTRA_ANSWER_INVALID_NODE(NODE)  TrNode_intra_invalid_next(NODE) = (ans_node_ptr)((CELL) TrNode_intra_invalid_next(NODE) | 0x1)
#define IS_INTRA_ANSWER_INVALID_NODE(NODE)      ((CELL) TrNode_intra_invalid_next(NODE) & 0x1)
#define IS_ANSWER_TEMP_NODE(NODE)               ((CELL) TrNode_intra_invalid_next(NODE) & 0x2)
#define UNTAG_ANSWER_TEMP_NODE(NODE)            TrNode_intra_invalid_next(NODE) = (ans_node_ptr)((CELL) TrNode_intra_invalid_next(NODE) & ~(0x2))
#define UNTAG_INTRA_ANSWER_INVALID_NODE(NODE)   ((CELL) (NODE) & ~(0x3))
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */

/* trie hashes */
#define MAX_NODES_PER_TRIE_LEVEL        8  //-> DEFAULT
//#define MAX_NODES_PER_BUCKET            (MAX_NODES_PER_TRIE_LEVEL / 2)
#define MAX_NODES_PER_BUCKET            4

//#define BASE_HASH_BUCKETS               64 //-> DEFAULT
#define BASE_HASH_BUCKETS               8  //-> V04 BASE_HASH_BUCKETS_2  NEW_MIAR

//#define BASE_SG_FR_HASH_BUCKETS         8192 // knapsack 16 threads -> 32768 path right + btree 17 -> 8192
#define HASH_ENTRY(ENTRY, NUM_BUCKETS)  ((((CELL) ENTRY) >> NumberOfLowTagBits) & (NUM_BUCKETS - 1))
#define HASH_ENTRY_SG_FR(ENTRY, NUM_BUCKETS)  ((((CELL) ENTRY) >> 8) & (NUM_BUCKETS - 1))


#define SUBGOAL_TRIE_HASH_MARK          ((Term) MakeTableVarTerm(MAX_TABLE_VARS + 1))
#define IS_SUBGOAL_TRIE_HASH(NODE)      (TrNode_entry(NODE) == SUBGOAL_TRIE_HASH_MARK)
#define ANSWER_TRIE_HASH_MARK           0
#define IS_ANSWER_TRIE_HASH(NODE)       (TrNode_instr(NODE) == ANSWER_TRIE_HASH_MARK)
#define GLOBAL_TRIE_HASH_MARK           ((Term) MakeTableVarTerm(MAX_TABLE_VARS + 1))
#define IS_GLOBAL_TRIE_HASH(NODE)       (TrNode_entry(NODE) == GLOBAL_TRIE_HASH_MARK)
#define HASH_TRIE_LOCK(NODE)            (GLOBAL_trie_locks((((CELL) (NODE)) >> LowTagBits) & (TRIE_LOCK_BUCKETS - 1)))

/* auxiliary stack */
#define STACK_PUSH_UP(ITEM, STACK)          *--(STACK) = (CELL)(ITEM)
#define STACK_POP_UP(STACK)                 *--(STACK)
#define STACK_PUSH_DOWN(ITEM, STACK)        *(STACK)++ = (CELL)(ITEM)
#define STACK_POP_DOWN(STACK)               *(STACK)++
#define STACK_NOT_EMPTY(STACK, STACK_BASE)  (STACK) != (STACK_BASE)
#if defined(YAPOR_COPY) || defined(YAPOR_COW) || defined(YAPOR_SBA)
#define AUX_STACK_CHECK_EXPAND(STACK, STACK_LIMIT)                                  \
        if ((STACK_LIMIT) >= (STACK))                                               \
          Yap_Error(INTERNAL_ERROR, TermNil, "stack full (AUX_STACK_CHECK_EXPAND)")
#else /* YAPOR_THREADS */
#define AUX_STACK_CHECK_EXPAND(STACK, STACK_LIMIT)                                  \
        if ((STACK_LIMIT) >= (STACK))                                               \
          STACK = expand_auxiliary_stack(STACK)
#endif /* YAPOR */
#define STACK_CHECK_EXPAND(STACK, STACK_LIMIT)                                      \
        if ((STACK_LIMIT) >= (STACK) + 4096)                                        \
          Yap_Error(INTERNAL_ERROR, TermNil, "stack full (STACK_CHECK_EXPAND)")



/*************************************
**      Data structures macros      **
*************************************/

#ifdef YAPOR
#define frame_with_suspensions_not_collected(OR_FR)                               \
        (OrFr_nearest_suspnode(OR_FR) == NULL)
#define find_dependency_node(SG_FR, LEADER_CP, DEP_ON_STACK)                      \
        if (SgFr_gen_worker(SG_FR) == worker_id) {                                \
          LEADER_CP = SgFr_gen_cp(SG_FR);                                         \
          DEP_ON_STACK = TRUE;                                                    \
        } else {                                                                  \
          or_fr_ptr aux_or_fr = SgFr_gen_top_or_fr(SG_FR);                        \
          while (! BITMAP_member(OrFr_members(aux_or_fr), worker_id))             \
            aux_or_fr = OrFr_next(aux_or_fr);                                     \
          LEADER_CP = GetOrFr_node(aux_or_fr);                                    \
          DEP_ON_STACK = (LEADER_CP == SgFr_gen_cp(SG_FR));                       \
        }
#define find_leader_node(LEADER_CP, DEP_ON_STACK)                                 \
        { dep_fr_ptr chain_dep_fr = LOCAL_top_dep_fr;                             \
          while (YOUNGER_CP(DepFr_cons_cp(chain_dep_fr), LEADER_CP)) {            \
            if (LEADER_CP == DepFr_leader_cp(chain_dep_fr)) {                     \
              DEP_ON_STACK |= DepFr_leader_dep_is_on_stack(chain_dep_fr);         \
              break;                                                              \
            } else if (YOUNGER_CP(LEADER_CP, DepFr_leader_cp(chain_dep_fr))) {    \
              LEADER_CP = DepFr_leader_cp(chain_dep_fr);                          \
              DEP_ON_STACK = DepFr_leader_dep_is_on_stack(chain_dep_fr);          \
              break;                                                              \
            }                                                                     \
            chain_dep_fr = DepFr_next(chain_dep_fr);                              \
          }                                                                       \
	}
#ifdef TIMESTAMP
#define DepFr_init_timestamp_field(DEP_FR)  DepFr_timestamp(DEP_FR) = 0
#else
#define DepFr_init_timestamp_field(DEP_FR)
#endif /* TIMESTAMP */
#define YAPOR_SET_LOAD(CP_PTR)  SCH_set_load(CP_PTR)
#define SgFr_init_yapor_fields(SG_FR)                                             \
        SgFr_gen_worker(SG_FR) = worker_id;                                       \
        SgFr_gen_top_or_fr(SG_FR) = LOCAL_top_or_fr
#define DepFr_init_yapor_fields(DEP_FR, DEP_ON_STACK, TOP_OR_FR)                  \
        INIT_LOCK_DEP_FR(DEP_FR);                                                 \
        DepFr_leader_dep_is_on_stack(DEP_FR) = DEP_ON_STACK;                      \
        DepFr_top_or_fr(DEP_FR) = TOP_OR_FR;                                      \
        DepFr_init_timestamp_field(DEP_FR)
#else
#define find_dependency_node(SG_FR, LEADER_CP, DEP_ON_STACK)                      \
        LEADER_CP = SgFr_gen_cp(SG_FR);                                           \
        DEP_ON_STACK = TRUE
#define find_leader_node(LEADER_CP, DEP_ON_STACK)                                 \
        { dep_fr_ptr chain_dep_fr = LOCAL_top_dep_fr;                             \
          while (YOUNGER_CP(DepFr_cons_cp(chain_dep_fr), LEADER_CP)) {            \
            if (EQUAL_OR_YOUNGER_CP(LEADER_CP, DepFr_leader_cp(chain_dep_fr))) {  \
              LEADER_CP = DepFr_leader_cp(chain_dep_fr);                          \
              break;                                                              \
            }                                                                     \
            chain_dep_fr = DepFr_next(chain_dep_fr);                              \
          }                                                                       \
	}
#define YAPOR_SET_LOAD(CP_PTR)
#define SgFr_init_yapor_fields(SG_FR)
#define DepFr_init_yapor_fields(DEP_FR, DEP_ON_STACK, TOP_OR_FR)
#endif /* YAPOR */

#if defined(THREADS_SUBGOAL_SHARING) || defined(THREADS_FULL_SHARING)
#define SgEnt_init_next_complete(SG_ENT)	\
  SgEnt_next_complete(SG_ENT) = NULL
#define SgFr_init_next_complete(SG_FR)		\
  SgFr_next_complete(SG_FR) = NULL 
#else
#define SgEnt_init_next_complete(SG_ENT)
#define SgFr_init_next_complete(SG_FR)
#endif

#ifdef MODE_DIRECTED_TABLING
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
#define Init_threads_full_sharing_mode_directed_v02(SG_ENT)   \
        SgEnt_intra_invalid_chain(SG_ENT) = NULL
#else
#define Init_threads_full_sharing_mode_directed_v02(SG_ENT)
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */

#define TabEnt_init_mode_directed_field(TAB_ENT, MODE_ARRAY)  \
        TabEnt_mode_directed(TAB_ENT) = MODE_ARRAY
#define SgEnt_init_mode_directed_fields(SG_ENT, MODE_ARRAY)   \
        SgEnt_invalid_chain(SG_ENT) = NULL;                   \
        SgEnt_mode_directed(SG_ENT) = MODE_ARRAY

#define SgFr_init_mode_directed_fields(SG_FR, MODE_ARRAY)     \
        SgFr_invalid_chain(SG_FR) = NULL;                     \
        SgFr_mode_directed(SG_FR) = MODE_ARRAY

#define AnsHash_init_previous_field(HASH, SG_FR)              \
        if (SgFr_hash_chain(SG_FR))                           \
          Hash_previous(SgFr_hash_chain(SG_FR)) = HASH;       \
        Hash_previous(HASH) = NULL
#else
#define TabEnt_init_mode_directed_field(TAB_ENT, MODE_ARRAY)
#define SgEnt_init_mode_directed_fields(SG_ENT, MODE_ARRAY)
#define SgFr_init_mode_directed_fields(SG_FR, MODE_ARRAY)
#define AnsHash_init_previous_field(HASH, SG_FR)
#endif /* MODE_DIRECTED_TABLING */


#ifdef THREADS_SUBGOAL_COMPLETION_WAIT
#define INIT_LOCK_SG_FR_COMP_WAIT(SG_FR)		\
  INIT_LOCK(SgFr_lock_comp_wait(SG_FR)) ;		\
  pthread_cond_init(&(SgFr_comp_wait(SG_FR)), NULL);
//  SgFr_comp_wait(SG_FR) = PTHREAD_COND_INITIALIZER;

#define LOCK_SG_FR_COMP_WAIT(SG_FR)     LOCK(SgFr_lock_comp_wait(SG_FR))
#define UNLOCK_SG_FR_COMP_WAIT(SG_FR)   UNLOCK(SgFr_lock_comp_wait(SG_FR))
#else
#define INIT_LOCK_SG_FR_COMP_WAIT(SG_FR)
#define LOCK_SG_FR_COMP_WAIT(SG_FR)
#define UNLOCK_SG_FR_COMP_WAIT(SG_FR)
#endif


#if defined(YAPOR) || defined(THREADS_FULL_SHARING)
#define INIT_LOCK_SG_FR(SG_FR)  INIT_LOCK(SgFr_lock(SG_FR))
#define LOCK_SG_FR(SG_FR)       LOCK(SgFr_lock(SG_FR))
#define TRYLOCK_SG_FR(SG_FR)    TRY_LOCK(SgFr_lock(SG_FR))
#define UNLOCK_SG_FR(SG_FR)     UNLOCK(SgFr_lock(SG_FR))
#else
#define INIT_LOCK_SG_FR(SG_FR)
#define LOCK_SG_FR(SG_FR)
#define UNLOCK_SG_FR(SG_FR)
#endif /* YAPOR || THREADS_FULL_SHARING */

#ifdef YAPOR
#define INIT_LOCK_DEP_FR(DEP_FR)    INIT_LOCK(DepFr_lock(DEP_FR))
#define LOCK_DEP_FR(DEP_FR)         LOCK(DepFr_lock(DEP_FR))
#define UNLOCK_DEP_FR(DEP_FR)       UNLOCK(DepFr_lock(DEP_FR))
#define IS_UNLOCKED_DEP_FR(DEP_FR)  IS_UNLOCKED(DepFr_lock(DEP_FR))
#else
#define INIT_LOCK_DEP_FR(DEF_FR)
#define LOCK_DEP_FR(DEP_FR)
#define UNLOCK_DEP_FR(DEP_FR)
#define IS_UNLOCKED_DEP_FR(DEP_FR)
#endif /* YAPOR */

#ifdef THREADS
#define INIT_LOCK_TAB_ENT(TAB_ENT)  INIT_LOCK(TabEnt_lock(TAB_ENT))
#else
#define INIT_LOCK_TAB_ENT(TAB_ENT)
#endif /* SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL || THREADS_NO_SHARING */

#ifdef SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL
#define LOCK_SUBGOAL_TRIE(TAB_ENT)    LOCK(TabEnt_lock(TAB_ENT))
#define UNLOCK_SUBGOAL_TRIE(TAB_ENT)  UNLOCK(TabEnt_lock(TAB_ENT))
#define SgHash_init_chain_fields(HASH, TAB_ENT)       \
       Hash_next(HASH) = TabEnt_hash_chain(TAB_ENT);  \
       TabEnt_hash_chain(TAB_ENT) = HASH
#else
#define LOCK_SUBGOAL_TRIE(TAB_ENT)
#define UNLOCK_SUBGOAL_TRIE(TAB_ENT)
#define SgHash_init_chain_fields(HASH, TAB_ENT)      \
       LOCK(TabEnt_lock(TAB_ENT));                   \
       Hash_next(HASH) = TabEnt_hash_chain(TAB_ENT); \
       TabEnt_hash_chain(TAB_ENT) = HASH;	     \
       UNLOCK(TabEnt_lock(TAB_ENT))
#endif /* SUBGOAL_TRIE_LOCK_AT_ENTRY_LEVEL */

#ifdef ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL
#define LOCK_ANSWER_TRIE(SG_FR)    LOCK_SG_FR(SG_FR)
#define UNLOCK_ANSWER_TRIE(SG_FR)  UNLOCK_SG_FR(SG_FR)
#define AnsHash_init_chain_fields(HASH, SG_FR)     \
        AnsHash_init_previous_field(HASH, SG_FR);  \
        Hash_next(HASH) = SgFr_hash_chain(SG_FR);  \
	SgFr_hash_chain(SG_FR) = HASH
#else
#define LOCK_ANSWER_TRIE(SG_FR)
#define UNLOCK_ANSWER_TRIE(SG_FR)
#define AnsHash_init_chain_fields(HASH, SG_FR)     \
        LOCK_SG_FR(SG_FR);                         \
        AnsHash_init_previous_field(HASH, SG_FR);  \
        Hash_next(HASH) = SgFr_hash_chain(SG_FR);  \
        SgFr_hash_chain(SG_FR) = HASH;             \
        UNLOCK_SG_FR(SG_FR)
#endif /* ANSWER_TRIE_LOCK_AT_ENTRY_LEVEL */


#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
#define TRYLOCK_SUBGOAL_NODE(NODE)
#define LOCK_SUBGOAL_NODE(NODE)
#define UNLOCK_SUBGOAL_NODE(NODE)
#define SgNode_init_lock_field(NODE)
#else
#ifdef SUBGOAL_TRIE_LOCK_USING_NODE_FIELD
#define TRYLOCK_SUBGOAL_NODE(NODE)      TRY_LOCK(TrNode_lock(NODE))
#define LOCK_SUBGOAL_NODE(NODE)         LOCK(TrNode_lock(NODE)) 
#define UNLOCK_SUBGOAL_NODE(NODE)       UNLOCK(TrNode_lock(NODE))
#define SgNode_init_lock_field(NODE)    INIT_LOCK(TrNode_lock(NODE))
#elif defined(SUBGOAL_TRIE_LOCK_USING_GLOBAL_ARRAY)
#define TRYLOCK_SUBGOAL_NODE(NODE)      TRY_LOCK(HASH_TRIE_LOCK(NODE))
#define LOCK_SUBGOAL_NODE(NODE)         LOCK(HASH_TRIE_LOCK(NODE))
#define UNLOCK_SUBGOAL_NODE(NODE)       UNLOCK(HASH_TRIE_LOCK(NODE))
#define SgNode_init_lock_field(NODE)
#else
#define TRYLOCK_SUBGOAL_NODE(NODE)
#define LOCK_SUBGOAL_NODE(NODE) 
#define UNLOCK_SUBGOAL_NODE(NODE)
#define SgNode_init_lock_field(NODE)
#endif /* SUBGOAL_TRIE_LOCK_LEVEL */
#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL */

#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
#define TRYLOCK_ANSWER_NODE(NODE)
#define LOCK_ANSWER_NODE(NODE)
#define UNLOCK_ANSWER_NODE(NODE)
#define AnsNode_init_lock_field(NODE)
#else
#ifdef ANSWER_TRIE_LOCK_USING_NODE_FIELD
#define TRYLOCK_ANSWER_NODE(NODE)        TRY_LOCK(TrNode_lock(NODE))
#define LOCK_ANSWER_NODE(NODE)           LOCK(TrNode_lock(NODE))
#define UNLOCK_ANSWER_NODE(NODE)         UNLOCK(TrNode_lock(NODE))
#define AnsNode_init_lock_field(NODE)    INIT_LOCK(TrNode_lock(NODE))
#elif defined(ANSWER_TRIE_LOCK_USING_GLOBAL_ARRAY)
#define TRYLOCK_ANSWER_NODE(NODE)        TRY_LOCK(HASH_TRIE_LOCK(NODE))
#define LOCK_ANSWER_NODE(NODE)           LOCK(HASH_TRIE_LOCK(NODE))
#define UNLOCK_ANSWER_NODE(NODE)         UNLOCK(HASH_TRIE_LOCK(NODE))
#define AnsNode_init_lock_field(NODE)
#else
#define TRYLOCK_ANSWER_NODE(NODE)
#define LOCK_ANSWER_NODE(NODE)
#define UNLOCK_ANSWER_NODE(NODE)
#define AnsNode_init_lock_field(NODE)
#endif /* ANSWER_TRIE_LOCK_LEVEL */
#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL */

#ifdef GLOBAL_TRIE_LOCK_USING_NODE_FIELD
#define LOCK_GLOBAL_NODE(NODE)        LOCK(TrNode_lock(NODE))
#define UNLOCK_GLOBAL_NODE(NODE)      UNLOCK(TrNode_lock(NODE))
#define GtNode_init_lock_field(NODE)  INIT_LOCK(TrNode_lock(NODE))
#elif defined(GLOBAL_TRIE_LOCK_USING_GLOBAL_ARRAY)
#define LOCK_GLOBAL_NODE(NODE)        LOCK(HASH_TRIE_LOCK(NODE))
#define UNLOCK_GLOBAL_NODE(NODE)      UNLOCK(HASH_TRIE_LOCK(NODE))
#define GtNode_init_lock_field(NODE)
#else
#define LOCK_GLOBAL_NODE(NODE)
#define UNLOCK_GLOBAL_NODE(NODE)
#define GtNode_init_lock_field(NODE)
#endif /* GLOBAL_TRIE_LOCK_LEVEL */



#ifdef LINEAR_TABLING
#define SgFr_linear_tabling_new_sg_fr_fields(SG_FR)     \
  /*ALLOC_ALTERNATIVES_BUCKET(SgFr_loop_alts(SG_FR));*/	\
  SgFr_allocate_drs_looping_structure(SG_FR)

#else /* !LINEAR_TABLING */
#define SgFr_linear_tabling_new_sg_fr_fields(SG_FR)
#define SgFr_init_linear_tabling_fields(SG_FR,TAB_ENT)  \
  SgFr_next(SG_FR) = LOCAL_top_sg_fr;	                \
  LOCAL_top_sg_fr = SG_FR

/* if LINEAR_TABLING defined then 
   SgFr_init_linear_tabling_fields is defined in linear.tab.macros.h 
   I must clean this stuff later */
#endif /* LINEAR_TABLING */




#ifdef THREADS_NO_SHARING
#define TabEnt_init_subgoal_trie_field(TAB_ENT)                           \
        INIT_BUCKETS(&TabEnt_subgoal_trie(TAB_ENT), THREADS_NUM_BUCKETS)
#else
#define TabEnt_init_subgoal_trie_field(TAB_ENT)                           \
        { register sg_node_ptr sg_node;                                   \
          new_subgoal_trie_node(sg_node, 0, NULL, NULL, NULL);            \
          TabEnt_subgoal_trie(TAB_ENT) = sg_node;                         \
	}
#endif /* THREADS_NO_SHARING */
#define DepFr_init_external_field(DEP_FR, IS_EXTERNAL)

#if defined(THREADS_FULL_SHARING) && defined(MODE_DIRECTED_TABLING)
#define Init_mode_directed_full_sharing_fields(SG_ENT)        \
        SgEnt_old_hash_chain(SG_ENT) = NULL;                  \
        Init_threads_full_sharing_mode_directed_v02(SG_ENT)
#else
#define Init_mode_directed_full_sharing_fields(SG_ENT)
#endif


#if defined(THREADS_FULL_SHARING)


#ifdef THREADS_FULL_SHARING_FTNA
#define SgEnt_init_fs_ftna_last_answer(SG_FR)
#define SgFr_init_fs_ftna_last_answer(SG_FR)   \
     SgFr_last_answer(SG_FR) = NULL
#else
#define SgEnt_init_fs_ftna_last_answer(SG_ENT) \
     SgEnt_last_answer(SG_ENT) = NULL
#define SgFr_init_fs_ftna_last_answer(SG_FR)
#endif /* THREADS_FULL_SHARING_FTNA */


#ifdef THREADS_FULL_SHARING_FTNA_3
#define  SgFr_init_fs_ftna_3_fields(SG_FR)   \
  SgFr_cons_ref_ans(SG_FR) = NULL;	     \
  SgFr_cons_ref_first_ans(SG_FR) = NULL;     \
  SgFr_cons_ref_last_ans(SG_FR) = NULL

#define DepFr_init_last_answer_field(DEP_FR, SG_FR)                                               \
        /* start with TrNode_child(DepFr_last_answer(DEP_FR)) ... */                              \
        /* ... pointing to SgEnt_first_answer(SgFr_sg_ent(SG_FR)) */	                          \
        if (SG_FR)                                                                                \
          DepFr_last_answer(DEP_FR) = (ans_ref_ptr) (                                             \
                                 (CELL) (SG_FR) +                                                 \
 	                         (CELL) (&SgFr_cons_ref_first_ans((sg_fr_ptr)DEP_FR)) -           \
				 (CELL) (&TrNode_child((ans_ref_ptr)DEP_FR)));                    \
        else                                                                                      \
          DepFr_last_answer(DEP_FR) = NULL

#else
#define  SgFr_init_fs_ftna_3_fields(SG_FR)

#define DepFr_init_last_answer_field(DEP_FR, SG_FR)                                               \
        /* start with TrNode_child(DepFr_last_answer(DEP_FR)) ... */                              \
        /* ... pointing to SgEnt_first_answer(SgFr_sg_ent(SG_FR)) */	                          \
        if (SG_FR)                                                                                \
          DepFr_last_answer(DEP_FR) = (ans_node_ptr) (                                            \
                                 (CELL) (SgFr_sg_ent((sg_fr_ptr)SG_FR)) +            \
 	                         (CELL) (&SgEnt_first_answer((sg_ent_ptr)DEP_FR)) -  \
				 (CELL) (&TrNode_child((ans_node_ptr)DEP_FR)));      \
        else                                                                                      \
          DepFr_last_answer(DEP_FR) = NULL

#endif /* THREADS_FULL_SHARING_FTNA_3 */

#else
#define DepFr_init_last_answer_field(DEP_FR, SG_FR)                                               \
        /* start with TrNode_child(DepFr_last_answer(DEP_FR)) ... */                              \
        /* ... pointing to SgFr_first_answer(SG_FR) */                                            \
        if (SG_FR)                                                                                \
          DepFr_last_answer(DEP_FR) = (ans_node_ptr) (                                            \
                                 (CELL) (SG_FR) +                                    \
                                 (CELL) (&SgFr_first_answer((sg_fr_ptr)DEP_FR)) -    \
				 (CELL) (&TrNode_child((ans_node_ptr)DEP_FR)));      \
        else                                                                                      \
          DepFr_last_answer(DEP_FR) = NULL
#endif /* THREADS_FULL_SHARING */

#ifdef THREADS_NO_SUBGOAL_TRIE

#define new_table_entry(TAB_ENT, PRED_ENTRY, ATOM, ARITY, MODE_ARRAY, DIM_ARRAY, SG_FR_MODE_ARRAY, NO_TRIE, NO_TRIE_TERM_TYPE, DIM_ARRAY_LENGTH) \
        ALLOC_TABLE_ENTRY(TAB_ENT);                                    \
        INIT_LOCK_TAB_ENT(TAB_ENT);                                    \
        TabEnt_pe(TAB_ENT) = PRED_ENTRY;                               \
        TabEnt_atom(TAB_ENT) = ATOM;                                   \
        TabEnt_arity(TAB_ENT) = ARITY;                                 \
        TabEnt_flags(TAB_ENT) = 0;                                     \
        SetMode_Batched(TabEnt_flags(TAB_ENT));                        \
        SetMode_ExecAnswers(TabEnt_flags(TAB_ENT));                    \
        SetMode_LocalTrie(TabEnt_flags(TAB_ENT));                      \
        SetMode_Suspension(TabEnt_flags(TAB_ENT));                     \
        TabEnt_mode(TAB_ENT) = TabEnt_flags(TAB_ENT);                  \
        if (IsMode_Local(yap_flags[TABLING_MODE_FLAG]))                \
          SetMode_Local(TabEnt_mode(TAB_ENT));                         \
        if (IsMode_LoadAnswers(yap_flags[TABLING_MODE_FLAG]))          \
          SetMode_LoadAnswers(TabEnt_mode(TAB_ENT));                   \
        if (IsMode_GlobalTrie(yap_flags[TABLING_MODE_FLAG]))           \
          SetMode_GlobalTrie(TabEnt_mode(TAB_ENT));                    \
        if (IsMode_Linear(yap_flags[TABLING_MODE_FLAG]))               \
          SetMode_Linear(TabEnt_mode(TAB_ENT));                        \
        TabEnt_init_mode_directed_field(TAB_ENT, MODE_ARRAY);          \
	TabEnt_dimension_array(TAB_ENT) = DIM_ARRAY;		       \
	TabEnt_dimension_array_length(TAB_ENT) = DIM_ARRAY_LENGTH;     \
	TabEnt_sg_fr_mode_directed(TAB_ENT) = SG_FR_MODE_ARRAY;	       \
        TabEnt_no_subgoal_trie(TAB_ENT) = NO_TRIE;		       \
	TabEnt_no_subgoal_trie_term_type(TAB_ENT) = NO_TRIE_TERM_TYPE; \
        TabEnt_init_subgoal_trie_field(TAB_ENT);                       \
        TabEnt_hash_chain(TAB_ENT) = NULL;                             \
        TabEnt_next(TAB_ENT) = GLOBAL_root_tab_ent;                    \
        GLOBAL_root_tab_ent = TAB_ENT

#else  /* !THREADS_NO_SUBGOAL_TRIE */

#define new_table_entry(TAB_ENT, PRED_ENTRY, ATOM, ARITY, MODE_ARRAY)  \
        ALLOC_TABLE_ENTRY(TAB_ENT);                                    \
        INIT_LOCK_TAB_ENT(TAB_ENT);                                    \
        TabEnt_pe(TAB_ENT) = PRED_ENTRY;                               \
        TabEnt_atom(TAB_ENT) = ATOM;                                   \
        TabEnt_arity(TAB_ENT) = ARITY;                                 \
        TabEnt_flags(TAB_ENT) = 0;                                     \
        SetMode_Batched(TabEnt_flags(TAB_ENT));                        \
        SetMode_ExecAnswers(TabEnt_flags(TAB_ENT));                    \
        SetMode_LocalTrie(TabEnt_flags(TAB_ENT));                      \
        TabEnt_mode(TAB_ENT) = TabEnt_flags(TAB_ENT);                  \
        if (IsMode_Local(yap_flags[TABLING_MODE_FLAG]))                \
          SetMode_Local(TabEnt_mode(TAB_ENT));                         \
        if (IsMode_LoadAnswers(yap_flags[TABLING_MODE_FLAG]))          \
          SetMode_LoadAnswers(TabEnt_mode(TAB_ENT));                   \
        if (IsMode_GlobalTrie(yap_flags[TABLING_MODE_FLAG]))           \
          SetMode_GlobalTrie(TabEnt_mode(TAB_ENT));                    \
        TabEnt_init_mode_directed_field(TAB_ENT, MODE_ARRAY);          \
        TabEnt_init_subgoal_trie_field(TAB_ENT);                       \
        TabEnt_hash_chain(TAB_ENT) = NULL;                             \
        TabEnt_next(TAB_ENT) = GLOBAL_root_tab_ent;                    \
        GLOBAL_root_tab_ent = TAB_ENT


#endif /* THREADS_NO_SUBGOAL_TRIE */



#ifdef THREADS_SUBGOAL_FRAME_BY_WID

#define new_subgoal_entry(SG_ENT)			            \
        { register ans_node_ptr ans_node;                           \
          new_answer_trie_node(ans_node, 0, 0, NULL, NULL, NULL);   \
          ALLOC_SUBGOAL_ENTRY(SG_ENT);                              \
          INIT_LOCK(SgEnt_lock(SG_ENT));		            \
          SgEnt_hash_chain(SG_ENT) = NULL;		 	    \
	  Init_mode_directed_full_sharing_fields(SG_ENT);	    \
          SgEnt_answer_trie(SG_ENT) = ans_node;                     \
          SgEnt_first_answer(SG_ENT) = NULL;                        \
          SgEnt_init_fs_ftna_last_answer(SG_ENT);                   \
	  SgEnt_init_extra_statistics_choice_points(SG_ENT);        \
          SgEnt_sg_ent_state(SG_ENT) = ready;		 	    \
          SgEnt_active_workers(SG_ENT) = 0;                         \
        }
#else /* !THREADS_SUBGOAL_FRAME_BY_WID */

#define new_subgoal_entry(SG_ENT)                                   \
        { register ans_node_ptr ans_node;                           \
          new_answer_trie_node(ans_node, 0, 0, NULL, NULL, NULL);   \
          ALLOC_SUBGOAL_ENTRY(SG_ENT);                              \
          INIT_LOCK(SgEnt_lock(SG_ENT));		            \
          SgEnt_hash_chain(SG_ENT) = NULL;		 	    \
	  Init_mode_directed_full_sharing_fields(SG_ENT);	    \
          SgEnt_answer_trie(SG_ENT) = ans_node;                     \
          SgEnt_first_answer(SG_ENT) = NULL;                        \
          SgEnt_init_fs_ftna_last_answer(SG_ENT);                   \
	  SgEnt_init_extra_statistics_choice_points(SG_ENT);        \
          SgEnt_sg_ent_state(SG_ENT) = ready;		 	    \
          SgEnt_active_workers(SG_ENT) = 0;                         \
          INIT_BUCKETS(&SgEnt_sg_fr(SG_ENT), THREADS_NUM_BUCKETS);  \
        }

#endif /* !THREADS_SUBGOAL_FRAME_BY_WID */

#if defined(THREADS_FULL_SHARING)

#define new_subgoal_frame(SG_FR, SG_ENT)		           \
        { ALLOC_SUBGOAL_FRAME(SG_FR);    	     	           \
          SgFr_linear_tabling_new_sg_fr_fields(SG_FR);		   \
          SgFr_sg_ent(SG_FR) = SG_ENT; 		                   \
          SgFr_state(SG_FR) = ready;                               \
	  SgFr_init_next_complete(SG_FR);    		           \
	  SgFr_init_fs_ftna_3_fields(SG_FR);                       \
	  SgFr_init_fs_ftna_last_answer(SG_FR);                    \
	  SgFr_init_no_sg_trie_fields(SG_FR);			   \
        }

#define init_subgoal_frame(SG_FR, TAB_ENT) 		           \
        { SgFr_state(SG_FR) = evaluating;			   \
 	  SgFr_init_linear_tabling_fields(SG_FR, TAB_ENT);         \
	}
#else /* no multithreading || THREADS_SUBGOAL_SHARING || THREADS_NO_SHARING */

#ifdef THREADS_NO_SUBGOAL_TRIE 
#define new_subgoal_frame(SG_FR, CODE, MODE_ARRAY)		   \
     {   tab_ent_ptr tab_ent = CODE->u.Otapl.te;		   \
	 ALLOC_SUBGOAL_FRAME(SG_FR);				   \
         SgFr_linear_tabling_new_sg_fr_fields(SG_FR);		   \
         SgFr_code(SG_FR) = CODE;                                  \
         SgFr_init_mode_directed_fields(SG_FR, MODE_ARRAY);	   \
         SgFr_state(SG_FR) = ready;                                \
	 SgFr_no_sg_pos(SG_FR) = NULL;				   \
         SgFr_init_next_complete(SG_FR);    		           \
         if (TabEnt_no_subgoal_trie(tab_ent) == NULL) {		   \
	   register ans_node_ptr ans_node;			   \
	   new_answer_trie_node(ans_node, 0, 0, NULL, NULL, NULL); \
	   INIT_LOCK_SG_FR(SG_FR);				   \
	   INIT_LOCK_SG_FR_COMP_WAIT(SG_FR);			   \
	   SgFr_hash_chain(SG_FR) = NULL;			   \
	   SgFr_answer_trie(SG_FR) = ans_node;			   \
	   SgFr_first_answer(SG_FR) = NULL;			   \
	   SgFr_last_answer(SG_FR) = NULL;			   \
	 }							   \
      }
#else /* !THREADS_NO_SUBGOAL_TRIE */

#define new_subgoal_frame(SG_FR, CODE, MODE_ARRAY)		   \
        { register ans_node_ptr ans_node;                          \
          new_answer_trie_node(ans_node, 0, 0, NULL, NULL, NULL);  \
          ALLOC_SUBGOAL_FRAME(SG_FR);                              \
          SgFr_linear_tabling_new_sg_fr_fields(SG_FR);		   \
	  INIT_LOCK_SG_FR(SG_FR);				   \
	  INIT_LOCK_SG_FR_COMP_WAIT(SG_FR);			   \
          SgFr_code(SG_FR) = CODE;                                 \
          SgFr_hash_chain(SG_FR) = NULL;                           \
          SgFr_answer_trie(SG_FR) = ans_node;                      \
          SgFr_first_answer(SG_FR) = NULL;                         \
          SgFr_last_answer(SG_FR) = NULL;                          \
	  SgFr_init_mode_directed_fields(SG_FR, MODE_ARRAY);	   \
          SgFr_state(SG_FR) = ready;                               \
	  SgFr_init_next_complete(SG_FR);    		           \
	}

#endif /* THREADS_NO_SUBGOAL_TRIE */

#define init_subgoal_frame(SG_FR, TAB_ENT)			   \
        { SgFr_init_yapor_fields(SG_FR);                           \
          SgFr_state(SG_FR) = evaluating;                          \
 	  SgFr_init_linear_tabling_fields(SG_FR, TAB_ENT);         \
	}

#endif /* THREADS_FULL_SHARING  */

#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX
#define	DepFr_init_sg_trie_min_max_fields(DEP_FR, SG_FR)	                     \
  if(SG_FR != NULL) /* avoids NULL sg_fr on top dep_fr */ {	                     \
    DepFr_no_sg_pos(DEP_FR) = SgFr_no_sg_pos(SG_FR);	                      	     \
    DepFr_last_consumed_term_type(DEP_FR) = SgFr_mode_directed_term_type(SG_FR);     \
    if (DepFr_last_consumed_term_type(DEP_FR) == MODE_DIRECTED_DIM_INTEGER ||        \
        DepFr_last_consumed_term_type(DEP_FR) == MODE_DIRECTED_DIM_FLOAT)            \
      DepFr_last_term_float(DEP_FR) = 0.0; /* checked that float = 0.0 -> int = 0 */ \
    else								             \
      DepFr_last_term_big_integer(DEP_FR) = NULL;			             \
  }
#else 
#define	DepFr_init_sg_trie_min_max_fields(DEP_FR, SG_FR)
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX) */

#if defined(TIMESTAMP_MODE_DIRECTED_TABLING)
#define	DepFr_init_last_term_field(DEP_FR)   (DepFr_last_term(DEP_FR) = 0.0)
#else
#define	DepFr_init_last_term_field(DEP_FR)
#endif /* TIMESTAMP_MODE_DIRECTED_TABLING */

#define new_dependency_frame(DEP_FR, DEP_ON_STACK, TOP_OR_FR, LEADER_CP, CONS_CP, SG_FR, IS_EXTERNAL, NEXT)  \
        ALLOC_DEPENDENCY_FRAME(DEP_FR);                                                                      \
        DepFr_init_yapor_fields(DEP_FR, DEP_ON_STACK, TOP_OR_FR);                                            \
        DepFr_init_external_field(DEP_FR, IS_EXTERNAL);                                                      \
        DepFr_backchain_cp(DEP_FR) = NULL;                                                                   \
        DepFr_leader_cp(DEP_FR) = NORM_CP(LEADER_CP);                                                        \
        DepFr_cons_cp(DEP_FR) = NORM_CP(CONS_CP);                                                            \
        DepFr_init_last_answer_field(DEP_FR, SG_FR);                                                         \
	DepFr_init_last_term_field(DEP_FR);				                                     \
	DepFr_init_sg_trie_min_max_fields(DEP_FR, SG_FR); 			                             \
        DepFr_next(DEP_FR) = NEXT

#define new_suspension_frame(SUSP_FR, TOP_OR_FR_ON_STACK, TOP_DEP, TOP_SG,             \
                             H_REG, B_REG, TR_REG, H_SIZE, B_SIZE, TR_SIZE)            \
        ALLOC_SUSPENSION_FRAME(SUSP_FR);                                               \
        SuspFr_top_or_fr_on_stack(SUSP_FR) = TOP_OR_FR_ON_STACK;                       \
        SuspFr_top_dep_fr(SUSP_FR) = TOP_DEP;                                          \
        SuspFr_top_sg_fr(SUSP_FR) = TOP_SG;                                            \
        SuspFr_global_reg(SUSP_FR) = (void *) (H_REG);                                 \
        SuspFr_local_reg(SUSP_FR) = (void *) (B_REG);                                  \
        SuspFr_trail_reg(SUSP_FR) = (void *) (TR_REG);                                 \
        ALLOC_BLOCK(SuspFr_global_start(SUSP_FR), H_SIZE + B_SIZE + TR_SIZE, void *);  \
        SuspFr_local_start(SUSP_FR) = SuspFr_global_start(SUSP_FR) + H_SIZE;           \
        SuspFr_trail_start(SUSP_FR) = SuspFr_local_start(SUSP_FR) + B_SIZE;            \
        SuspFr_global_size(SUSP_FR) = H_SIZE;                                          \
        SuspFr_local_size(SUSP_FR) = B_SIZE;                                           \
        SuspFr_trail_size(SUSP_FR) = TR_SIZE;                                          \
        memcpy(SuspFr_global_start(SUSP_FR), SuspFr_global_reg(SUSP_FR), H_SIZE);      \
        memcpy(SuspFr_local_start(SUSP_FR), SuspFr_local_reg(SUSP_FR), B_SIZE);        \
        memcpy(SuspFr_trail_start(SUSP_FR), SuspFr_trail_reg(SUSP_FR), TR_SIZE)

#define new_subgoal_trie_node(NODE, ENTRY, CHILD, PARENT, NEXT)  \
        ALLOC_SUBGOAL_TRIE_NODE(NODE);                           \
        TrNode_entry(NODE) = ENTRY;				 \
	TrNode_child(NODE) = CHILD;	                         \
        TrNode_parent(NODE) = PARENT;                            \
        TrNode_next(NODE) = NEXT;                                \
        SgNode_init_lock_field(NODE)

#define new_answer_trie_node(NODE, INSTR, ENTRY, CHILD, PARENT, NEXT)  \
        ALLOC_ANSWER_TRIE_NODE(NODE);				       \
	/*	printf("new node = %p entry = %d \n", NODE, (int)ENTRY>>3); */ \
        TrNode_instr(NODE) = INSTR;                                    \
        TrNode_entry(NODE) = ENTRY;                                    \
        TrNode_child(NODE) = CHILD;                                    \
        TrNode_parent(NODE) = PARENT;                                  \
        TrNode_next(NODE) = NEXT;                                      \
        AnsNode_init_lock_field(NODE)

#define new_global_trie_node(NODE, ENTRY, CHILD, PARENT, NEXT)  \
        ALLOC_GLOBAL_TRIE_NODE(NODE);                           \
        TrNode_entry(NODE) = ENTRY;                             \
        TrNode_child(NODE) = CHILD;                             \
        TrNode_parent(NODE) = PARENT;                           \
        TrNode_next(NODE) = NEXT;                               \
        GtNode_init_lock_field(NODE)

#define new_answer_ref_node(NODE, ANSWER, NEXT, CHILD)             \
        ALLOC_ANSWER_REF_NODE(NODE);			           \
        RefNode_answer(NODE) = ANSWER;	                           \
        RefNode_next(NODE) = NEXT;                                 \
        RefNode_child(NODE) = CHILD /* or previous (batched mode) */

#define new_subgoal_trie_hash(HASH, NUM_NODES, TAB_ENT)         \
        ALLOC_SUBGOAL_TRIE_HASH(HASH);                          \
        Hash_mark(HASH) = SUBGOAL_TRIE_HASH_MARK;               \
        Hash_num_buckets(HASH) = BASE_HASH_BUCKETS;             \
        ALLOC_BUCKETS(Hash_buckets(HASH), BASE_HASH_BUCKETS);   \
        Hash_num_nodes(HASH) = NUM_NODES

#if defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL) || defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL)

#define CLOSE_ALL_BUCKETS(BUCKET_PTR, NUM_BUCKETS, STR)   \
  { int i; void **init_bucket_ptr;                        \
  init_bucket_ptr = (void **) BUCKET_PTR;                 \
  for (i = NUM_BUCKETS; i != 0; i--)                      \
    *init_bucket_ptr++ = (STR *) 0x1;	                  \
  }

#define ALLOC_CLOSED_HASH_BUCKETS(HASH_BUCKETS, BUCKET_PTR, NUM_BUCKETS, STR, STR_HASH_BKTS) \
  void **alloc_bucket_ptr;					 	                     \
  ALLOC_TRIE_HASH_BUCKETS(HASH_BUCKETS, STR_HASH_BKTS);			                     \
  HashBkts_next(HASH_BUCKETS) = NULL;					                     \
  HashBkts_number_of_buckets(HASH_BUCKETS) = NUM_BUCKETS;		                     \
  ALLOC_BLOCK(alloc_bucket_ptr, NUM_BUCKETS * sizeof(void *), void *);                       \
  CLOSE_ALL_BUCKETS(alloc_bucket_ptr, NUM_BUCKETS, STR);                                     \
  BUCKET_PTR = (void *) alloc_bucket_ptr;				                     \
  HashBkts_buckets(HASH_BUCKETS) = (STR **) alloc_bucket_ptr

#define ALLOC_OPEN_HASH_BUCKETS(HASH_BUCKETS, BUCKET_PTR, NUM_BUCKETS, STR, STR_HASH_BKTS)   \
  void **alloc_bucket_ptr;					 	                     \
  ALLOC_TRIE_HASH_BUCKETS(HASH_BUCKETS, STR_HASH_BKTS);				             \
  HashBkts_next(HASH_BUCKETS) = NULL;					                     \
  HashBkts_number_of_buckets(HASH_BUCKETS) = NUM_BUCKETS;		                     \
  ALLOC_BLOCK(alloc_bucket_ptr, NUM_BUCKETS * sizeof(void *), void *);                       \
  INIT_BUCKETS(alloc_bucket_ptr, NUM_BUCKETS);		  	                             \
  BUCKET_PTR = (STR **) alloc_bucket_ptr;				                     \
  HashBkts_buckets(HASH_BUCKETS) = (STR **) alloc_bucket_ptr

#if defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V01) || defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V01)

#define new_subgoal_trie_hash_atomic_v01(HASH, NUM_NODES)                                                                                                       \
        ALLOC_SUBGOAL_TRIE_HASH(HASH);                                                                                                                          \
        Hash_mark(HASH) = SUBGOAL_TRIE_HASH_MARK;                                                                                                               \
	SgHash_old_hash_bkts(HASH) = NULL; 		                                                                                                        \
        ALLOC_CLOSED_HASH_BUCKETS(SgHash_hash_bkts(HASH), SgHash_buckets(HASH), BASE_HASH_BUCKETS, struct subgoal_trie_node, struct subgoal_trie_hash_buckets); \
        Hash_num_nodes(HASH) = (NUM_NODES << 1) | (int) 1

#define new_answer_trie_hash_atomic_v01(HASH, NUM_NODES)                                                                                                        \
        ALLOC_ANSWER_TRIE_HASH(HASH);                                                                                                                           \
        Hash_mark(HASH) = ANSWER_TRIE_HASH_MARK;                                                                                                                \
	AnsHash_old_hash_bkts(HASH) = NULL;		                                                                                                        \
        ALLOC_CLOSED_HASH_BUCKETS(AnsHash_hash_bkts(HASH), AnsHash_buckets(HASH), BASE_HASH_BUCKETS, struct answer_trie_node, struct answer_trie_hash_buckets); \
        Hash_num_nodes(HASH) = (NUM_NODES << 1) | (int) 1

#endif

#if defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02)

#define ANSWER_TRIE_HASH_EXPANSION_NUM_NODES  10
#define ANSWER_TRIE_HASH_EXPANSION_MARK       (-1)

#define IS_ANSWER_TRIE_HASH_EXPANSION(NODE)   (TrNode_instr(NODE) == ANSWER_TRIE_HASH_EXPANSION_MARK)

#define init_atomic_new_answer_trie_hash(EXP_NODES, HASH, NUM_NODES, CHILD_NODE)	     \
  Hash_num_nodes(HASH) = NUM_NODES;					                     \
  /*create hash expansion nodes */			                                     \
  Hash_exp_nodes(HASH) = Hash_unused_exp_nodes(HASH) = EXP_NODES = NULL;                     \
  ans_node_ptr exp_child_node = CHILD_NODE;		                                     \
  while(exp_child_node != NULL) {			                                     \
    ans_node_ptr new_node;					                             \
    ALLOC_ANSWER_TRIE_NODE(new_node);				                             \
    TrNode_instr(new_node) = ANSWER_TRIE_HASH_EXPANSION_MARK;		                     \
    TrNode_entry(new_node) = TrNode_entry(exp_child_node);		                     \
    TrNode_child(new_node) = TrNode_child(exp_child_node);		                     \
    TrNode_parent(new_node) = exp_child_node;                                                \
    TrNode_next(new_node) = EXP_NODES;                                                       \
    EXP_NODES = new_node;				      	                             \
    exp_child_node = TrNode_next(exp_child_node);			                     \
  }									                     \
  /*alloc open hash buckets pointing to expansion nodes*/		                     \
  void **alloc_bucket_ptr;						                     \
  ALLOC_TRIE_HASH_BUCKETS(AnsHash_hash_bkts(HASH),struct answer_trie_hash_buckets);	     \
  HashBkts_next(AnsHash_hash_bkts(HASH)) = NULL;					     \
  HashBkts_number_of_buckets(AnsHash_hash_bkts(HASH)) = BASE_HASH_BUCKETS;		     \
  ALLOC_BLOCK(alloc_bucket_ptr, BASE_HASH_BUCKETS * sizeof(void *), void *);                 \
  AnsHash_buckets(HASH) = (void *) alloc_bucket_ptr;				             \
  HashBkts_buckets(AnsHash_hash_bkts(HASH)) = (struct answer_trie_node **) alloc_bucket_ptr; \
  void **init_bucket_ptr;				                                     \
  init_bucket_ptr = (void **) alloc_bucket_ptr;	                                             \
  int i;						                                     \
  for (i = 0; i < BASE_HASH_BUCKETS;  i++)	                                             \
    *init_bucket_ptr++ = EXP_NODES


#define new_answer_trie_hash_exp_nodes(EXP_NODES, HASH, NUM_NODES, SG_FR, CHILD_NODE)  \
  ALLOC_ANSWER_TRIE_HASH(HASH);					                       \
  Hash_mark(HASH) = ANSWER_TRIE_HASH_MARK;			                       \
  AnsHash_old_hash_bkts(HASH) = NULL;			                               \
  init_atomic_new_answer_trie_hash(EXP_NODES, HASH, NUM_NODES, CHILD_NODE)
#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02 */

#if defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03) 

#define SUBGOAL_TRIE_HASH_EXPANSION_MARK       ((Term) MakeTableVarTerm(MAX_TABLE_VARS + 1))
#define IS_SUBGOAL_TRIE_HASH_EXPANSION(NODE)   (TrNode_entry(NODE) == SUBGOAL_TRIE_HASH_EXPANSION_MARK)

#define init_atomic_new_subgoal_trie_hash(EXP_NODE, HASH, BUCKETS, NUM_NODES, CHILD_NODE)   	                   \
  Hash_num_nodes(HASH) = NUM_NODES;					                                           \
  /*create hash expansion node */			                                                           \
  EXP_NODE = Hash_sg_exp_node(HASH);			                                                           \
  TrNode_entry(EXP_NODE) = SUBGOAL_TRIE_HASH_EXPANSION_MARK;		                                           \
  TrNode_child(EXP_NODE) = TrNode_parent(EXP_NODE) = NULL;                                                         \
  TrNode_next(EXP_NODE) = CHILD_NODE;                                                                              \
  /*alloc open hash buckets pointing to the expansion node*/		                                           \
  void **alloc_bucket_ptr;						                                           \
  ALLOC_TRIE_HASH_BUCKETS(SgHash_hash_bkts(HASH), struct subgoal_trie_hash_buckets);	                           \
  HashBkts_next(SgHash_hash_bkts(HASH)) = NULL;					                                   \
  HashBkts_number_of_buckets(SgHash_hash_bkts(HASH)) = BASE_HASH_BUCKETS;		                           \
  ALLOC_BLOCK(alloc_bucket_ptr, BASE_HASH_BUCKETS * sizeof(void *), void *);                                       \
  BUCKETS = HashBkts_buckets(SgHash_hash_bkts(HASH)) = (struct subgoal_trie_node **) alloc_bucket_ptr;             \
  SgHash_hash_bkts(HASH) = (sg_hash_bkts_ptr)((CELL) SgHash_hash_bkts(HASH) | (CELL) 0x1);                         \
  void **init_bucket_ptr;				                                                           \
  init_bucket_ptr = (void **) alloc_bucket_ptr;	                                                                   \
  int i;						                                                           \
  for (i = 0; i < BASE_HASH_BUCKETS;  i++)	                                                                   \
    *init_bucket_ptr++ = EXP_NODE


#define new_subgoal_trie_hash_atomic_v03(EXP_NODE, HASH_NODE, BUCKETS, NUM_NODES, TAB_ENT, CHILD_NODE)             \
  ALLOC_SUBGOAL_TRIE_HASH(HASH_NODE);					                                           \
  Hash_mark(HASH_NODE) = SUBGOAL_TRIE_HASH_MARK;			                                           \
  SgHash_old_hash_bkts(HASH_NODE) = NULL;			                                                   \
  init_atomic_new_subgoal_trie_hash(EXP_NODE, HASH_NODE, BUCKETS, NUM_NODES, CHILD_NODE);                          \
  SgHash_init_chain_fields(HASH_NODE, TAB_ENT)

#endif

#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03

#define ANSWER_TRIE_HASH_EXPANSION_MARK       (ANSWER_TRIE_HASH_MARK)
#define IS_ANSWER_TRIE_HASH_EXPANSION(NODE)   (TrNode_instr(NODE) == ANSWER_TRIE_HASH_EXPANSION_MARK) 

#define init_atomic_new_answer_trie_hash(EXP_NODE, HASH, BUCKETS, NUM_NODES, CHILD_NODE)   	                   \
  Hash_num_nodes(HASH) = NUM_NODES;					                                           \
  /*create hash expansion node */			                                                           \
  EXP_NODE = Hash_ans_exp_node(HASH);			                                                           \
  TrNode_instr(EXP_NODE) = ANSWER_TRIE_HASH_EXPANSION_MARK;		                                           \
  TrNode_entry(EXP_NODE) = (Term) NULL ;                                                                           \
  TrNode_child(EXP_NODE) = TrNode_parent(EXP_NODE) = NULL;                                                         \
  TrNode_next(EXP_NODE) = CHILD_NODE;                                                                              \
  /*alloc open hash buckets pointing to the expansion node*/		                                           \
  void **alloc_bucket_ptr;						                                           \
  ALLOC_TRIE_HASH_BUCKETS(AnsHash_hash_bkts(HASH), struct answer_trie_hash_buckets);	                           \
  HashBkts_next(AnsHash_hash_bkts(HASH)) = NULL;					                           \
  HashBkts_number_of_buckets(AnsHash_hash_bkts(HASH)) = BASE_HASH_BUCKETS;		                           \
  ALLOC_BLOCK(alloc_bucket_ptr, BASE_HASH_BUCKETS * sizeof(void *), void *);                                       \
  BUCKETS = HashBkts_buckets(AnsHash_hash_bkts(HASH)) = (struct answer_trie_node **) alloc_bucket_ptr;             \
  AnsHash_hash_bkts(HASH) = (ans_hash_bkts_ptr)((CELL) AnsHash_hash_bkts(HASH) | (CELL) 0x1);                      \
  void **init_bucket_ptr;				                                                           \
  init_bucket_ptr = (void **) alloc_bucket_ptr;	                                                                   \
  int i;						                                                           \
  for (i = 0; i < BASE_HASH_BUCKETS;  i++)	                                                                   \
    *init_bucket_ptr++ = EXP_NODE


#define new_answer_trie_hash_atomic_v03(EXP_NODE, HASH_NODE, BUCKETS, NUM_NODES, SG_FR, CHILD_NODE)                \
  ALLOC_ANSWER_TRIE_HASH(HASH_NODE);					                                           \
  Hash_mark(HASH_NODE) = ANSWER_TRIE_HASH_MARK;			                                                   \
  AnsHash_old_hash_bkts(HASH_NODE) = NULL;			                                                   \
  init_atomic_new_answer_trie_hash(EXP_NODE, HASH_NODE, BUCKETS, NUM_NODES, CHILD_NODE);                           \
  AnsHash_init_chain_fields(HASH_NODE, sg_fr)

#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03 */
#endif

#if defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04) || defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04) || defined(THREADS_FULL_SHARING_FTNA_3)
#define SHIFT_SIZE                                    3

#define V04_SHIFT_ENTRY(ENTRY, N_SHIFTS)               ((ENTRY) >> ((SHIFT_SIZE * (N_SHIFTS)) + NumberOfLowTagBits))
#define V04_HASH_ENTRY(ENTRY, N_SHIFTS)                (V04_SHIFT_ENTRY(ENTRY, N_SHIFTS) & (BASE_HASH_BUCKETS - 1))
#define V04_IS_EMPTY_BUCKET(BUCKET, BASE_BUCKET, STR)  (BUCKET == (STR *) BASE_BUCKET)
/* tag is 0x2 because 0x1 is for leaf node of subgoal tries */

#define V04_TAG(PTR)                                   ((long)(PTR) |  (long)0x2)
#define V04_UNTAG(PTR)                                 ((long)(PTR) & ~(long)(0x2))
#define V04_IS_HASH(PTR)                               ((long)(PTR) & (long)(0x2))


#define V04_IS_EQUAL_ENTRY(X, T)                       (TrNode_entry(X) == T)

#define V04_GET_HASH_BUCKET(BUCKET, HASH, T, NS, STR)  (BUCKET = (STR **) V04_UNTAG(HASH) + V04_HASH_ENTRY((long)T, NS))
#define V04_GET_PREV_HASH(PREV_HASH, CURR_HASH, STR)   (PREV_HASH = (STR **) *(((STR **) V04_UNTAG(CURR_HASH)) - 1))
#define V04_SET_HASH_BUCKET(BUCKET, V, STR)            (*(BUCKET) = (STR *) V)


#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 || ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 || THREADS_FULL_SHARING_FTNA_3 */


#ifndef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
#define init_atomic_new_answer_trie_hash(HASH, NUM_NODES)                     \
  Hash_num_buckets(HASH) = BASE_HASH_BUCKETS;				      \
  ALLOC_BUCKETS(Hash_buckets(HASH), BASE_HASH_BUCKETS);                       \
  Hash_num_nodes(HASH) = NUM_NODES

#endif


#define new_answer_trie_hash(HASH, NUM_NODES, SG_FR)            \
        ALLOC_ANSWER_TRIE_HASH(HASH);                           \
        Hash_mark(HASH) = ANSWER_TRIE_HASH_MARK;                \
        init_atomic_new_answer_trie_hash(HASH, NUM_NODES);      \
        AnsHash_init_chain_fields(HASH, SG_FR)


#define new_global_trie_hash(HASH, NUM_NODES)                   \
        ALLOC_GLOBAL_TRIE_HASH(HASH);                           \
        Hash_mark(HASH) = GLOBAL_TRIE_HASH_MARK;                \
        Hash_num_buckets(HASH) = BASE_HASH_BUCKETS;             \
        ALLOC_BUCKETS(Hash_buckets(HASH), BASE_HASH_BUCKETS);   \
	Hash_num_nodes(HASH) = NUM_NODES

#ifdef LIMIT_TABLING
#define insert_into_global_sg_fr_list(SG_FR)                                 \
        SgFr_previous(SG_FR) = GLOBAL_last_sg_fr;                            \
        SgFr_next(SG_FR) = NULL;                                             \
        if (GLOBAL_first_sg_fr == NULL)                                      \
          GLOBAL_first_sg_fr = SG_FR;                                        \
        else                                                                 \
          SgFr_next(GLOBAL_last_sg_fr) = SG_FR;                              \
        GLOBAL_last_sg_fr = SG_FR
#define remove_from_global_sg_fr_list(SG_FR)                                 \
        if (SgFr_previous(SG_FR)) {                                          \
          if ((SgFr_next(SgFr_previous(SG_FR)) = SgFr_next(SG_FR)) != NULL)  \
            SgFr_previous(SgFr_next(SG_FR)) = SgFr_previous(SG_FR);          \
          else                                                               \
            GLOBAL_last_sg_fr = SgFr_previous(SG_FR);                        \
        } else {                                                             \
          if ((GLOBAL_first_sg_fr = SgFr_next(SG_FR)) != NULL)               \
            SgFr_previous(SgFr_next(SG_FR)) = NULL;                          \
          else                                                               \
            GLOBAL_last_sg_fr = NULL;                                        \
	}                                                                    \
        if (GLOBAL_check_sg_fr == SG_FR)                                     \
          GLOBAL_check_sg_fr = SgFr_previous(SG_FR)
#else
#define insert_into_global_sg_fr_list(SG_FR)
#define remove_from_global_sg_fr_list(SG_FR)
#endif /* LIMIT_TABLING */



/******************************
**      Inline funcions      **
******************************/

#ifdef THREADS

#if defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03)

static inline int adjust_subgoal_hash_nodes_first_exp(sg_node_ptr chain_node, sg_node_ptr *new_hash_buckets, int count_nodes) {
  sg_node_ptr *bucket;
  bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
  if (TrNode_next(chain_node) == NULL) {
    do
      TrNode_next(chain_node) = *bucket;
    while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
    return(count_nodes + 1);  
  }  
  adjust_subgoal_hash_nodes_first_exp(TrNode_next(chain_node), new_hash_buckets, count_nodes);
  do 
    TrNode_next(chain_node) = *bucket;
  while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
  return(count_nodes + 1);  
}

static inline void adjust_subgoal_hash_nodes(sg_node_ptr chain_node, sg_node_ptr *new_hash_buckets, int num_buckets) {
  sg_node_ptr *bucket;
  bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
  if (TrNode_next(chain_node) == NULL) {
    do
      TrNode_next(chain_node) = *bucket;
    while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
    return;  
  }  
  adjust_subgoal_hash_nodes(TrNode_next(chain_node), new_hash_buckets, num_buckets);
  do 
    TrNode_next(chain_node) = *bucket;
  while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
  return;
}
#endif

#if defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03)

static inline int adjust_answer_hash_nodes_first_exp(ans_node_ptr chain_node, ans_node_ptr *new_hash_buckets, int count_nodes) {
  ans_node_ptr *bucket;
  bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), BASE_HASH_BUCKETS);
  if (TrNode_next(chain_node) == NULL) {
    do
      TrNode_next(chain_node) = *bucket;
    while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
    return(count_nodes + 1);  
  }  
  adjust_answer_hash_nodes_first_exp(TrNode_next(chain_node), new_hash_buckets, count_nodes);
  do 
    TrNode_next(chain_node) = *bucket;
  while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
  return(count_nodes + 1);  
}

static inline void adjust_answer_hash_nodes(ans_node_ptr chain_node, ans_node_ptr *new_hash_buckets, int num_buckets) {
  ans_node_ptr *bucket;
  bucket = new_hash_buckets + HASH_ENTRY(TrNode_entry(chain_node), num_buckets);
  if (TrNode_next(chain_node) == NULL) {
    do
      TrNode_next(chain_node) = *bucket;
    while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
    return;  
  }  
  adjust_answer_hash_nodes(TrNode_next(chain_node), new_hash_buckets, num_buckets);
  do 
    TrNode_next(chain_node) = *bucket;
  while(!BOOL_CAS(bucket, TrNode_next(chain_node), chain_node));
  return;
}
#endif

static inline void **get_insert_thread_bucket(void **buckets
#ifndef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
					      , lockvar *buckets_lock
#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL */
					      ) {
  CACHE_REGS

  /* direct bucket */
  if (worker_id < THREADS_DIRECT_BUCKETS)
    return buckets + worker_id;
  

  /*****************NEVER ON THIS POINT FOR NOW  ***/
  /* CHECK THIS -------------- */
  /* ATTENTION : 32 BUCKETS DIRECT + 32 BUCKETS INDIRECT_1 = 64 BUCKETS ORIGINAL STRUCTURE 
                 32 BUCKETS INDIRECT_1 * 32 = 1024 THREADS - CHECK THESIS PHD */


  /* indirect bucket */

  buckets = buckets + THREADS_DIRECT_BUCKETS + (worker_id - THREADS_DIRECT_BUCKETS) / THREADS_DIRECT_BUCKETS;
  if (*buckets)
    return *buckets + (worker_id - THREADS_DIRECT_BUCKETS) % THREADS_DIRECT_BUCKETS;
   
  // insert indirect bucket
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
  void ** buckets_aux;
  ALLOC_BUCKETS(buckets_aux, THREADS_INDIRECT_BUCKETS);
  if (!BOOL_CAS(buckets, NULL, buckets_aux)){  
    FREE_BUCKETS(buckets_aux);    
  }
#else
  LOCK(*buckets_lock);
  if (*buckets == NULL)
    ALLOC_BUCKETS(*buckets, THREADS_INDIRECT_BUCKETS);
  UNLOCK(*buckets_lock);
#endif  //SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL 

  return *buckets + (worker_id - THREADS_DIRECT_BUCKETS) % THREADS_DIRECT_BUCKETS;


}


static inline void **get_thread_bucket(void **buckets) {
  CACHE_REGS

  /* direct bucket */
  if (worker_id < THREADS_DIRECT_BUCKETS)
    return buckets + worker_id;

  /*****************NEVER ON THIS POINT FOR NOW  ***/

  /* indirect bucket */
  buckets = buckets + THREADS_DIRECT_BUCKETS + (worker_id - THREADS_DIRECT_BUCKETS) / THREADS_DIRECT_BUCKETS;
  if (*buckets)
    return *buckets + (worker_id - THREADS_DIRECT_BUCKETS) % THREADS_DIRECT_BUCKETS;
  
  /* empty indirect bucket */
  return buckets; 
}


static inline void abolish_thread_buckets(void **buckets) {

  int i;

  /* abolish indirect buckets */
  buckets += THREADS_NUM_BUCKETS;
  for (i = 0; i < THREADS_INDIRECT_BUCKETS; i++) {
    if (*--buckets) {
      FREE_BUCKETS(*buckets);
      *buckets = NULL;
    }
  }

#if defined(THREADS_SUBGOAL_SHARING)
  /* abolish direct buckets */
  buckets -= THREADS_DIRECT_BUCKETS;
  FREE_BUCKETS(buckets);
#endif /* THREADS_SUBGOAL_SHARING */
}
#endif /* THREADS */


static inline sg_node_ptr get_insert_subgoal_trie(tab_ent_ptr tab_ent USES_REGS) {
#ifdef THREADS_NO_SHARING
  sg_node_ptr *sg_node_addr = (sg_node_ptr *) get_insert_thread_bucket((void **) &TabEnt_subgoal_trie(tab_ent), &TabEnt_lock(tab_ent));
  if (*sg_node_addr == NULL) {
    new_subgoal_trie_node(*sg_node_addr, 0, NULL, NULL, NULL);
  }
  return *sg_node_addr;
#else
  return TabEnt_subgoal_trie(tab_ent);
#endif /* THREADS_NO_SHARING */
}


static inline sg_node_ptr get_subgoal_trie(tab_ent_ptr tab_ent) {
#ifdef THREADS_NO_SHARING
  sg_node_ptr *sg_node_addr = (sg_node_ptr *) get_thread_bucket((void **) &TabEnt_subgoal_trie(tab_ent));
  return *sg_node_addr;
#else
  return TabEnt_subgoal_trie(tab_ent);
#endif /* THREADS_NO_SHARING */
}


static inline sg_node_ptr get_subgoal_trie_for_abolish(tab_ent_ptr tab_ent USES_REGS) {

#ifdef THREADS_NO_SHARING
  sg_node_ptr *sg_node_addr = (sg_node_ptr *) get_thread_bucket((void **) &TabEnt_subgoal_trie(tab_ent));
  sg_node_ptr sg_node = *sg_node_addr;
  *sg_node_addr = NULL;
  if (worker_id == 0)
    abolish_thread_buckets((void **) &TabEnt_subgoal_trie(tab_ent));
  return sg_node;
#else
  return TabEnt_subgoal_trie(tab_ent);
#endif /* THREADS_NO_SHARING */
  

}



#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
static inline sg_fr_ptr *get_insert_subgoal_frame_addr(sg_node_ptr sg_node USES_REGS) {
  sg_fr_ptr *sg_fr_addr = (sg_fr_ptr *) &TrNode_sg_fr(sg_node);

#ifdef THREADS_LOCAL_SG_FR_HASH_BUCKETS
  return sg_fr_addr;
#endif
  
  if (*sg_fr_addr == NULL) {
#if defined(THREADS_SUBGOAL_SHARING)
    void ** new_buckets;
    ALLOC_SG_FR_ARRAY(new_buckets, THREADS_NUM_BUCKETS);
    if (!BOOL_CAS(&(TrNode_sg_fr(sg_node)), NULL, (sg_node_ptr)((CELL)new_buckets | (CELL)0x1)))
      FREE_SG_FR_ARRAY(new_buckets);
#elif defined(THREADS_FULL_SHARING)
    sg_ent_ptr sg_ent;
    new_subgoal_entry(sg_ent);
    if (!BOOL_CAS(&(TrNode_sg_fr(sg_node)), NULL, (sg_node_ptr)((CELL)sg_ent | (CELL)0x1))){
      FREE_ANSWER_TRIE_NODE(SgEnt_answer_trie(sg_ent));
      FREE_SUBGOAL_ENTRY(sg_ent);
    } 
#endif
  }

  sg_fr_addr = (sg_fr_ptr *) get_insert_thread_bucket(
#if defined(THREADS_SUBGOAL_SHARING)
                                                      (void **) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node))
#elif defined(THREADS_FULL_SHARING)
						       (void **) &SgEnt_sg_fr((sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node)))
#endif
						      );
  return sg_fr_addr;
}

#else /* !SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL */

static inline sg_fr_ptr *get_insert_subgoal_frame_addr(sg_node_ptr sg_node USES_REGS) {
  sg_fr_ptr *sg_fr_addr = (sg_fr_ptr *) &TrNode_sg_fr(sg_node);
#ifdef THREADS_LOCAL_SG_FR_HASH_BUCKETS
  return sg_fr_addr;
#endif
#if defined(THREADS_SUBGOAL_SHARING) || defined(THREADS_FULL_SHARING)
  if (*sg_fr_addr == NULL) {
    LOCK_SUBGOAL_NODE(sg_node);
    if (*sg_fr_addr == NULL) {
#if defined(THREADS_SUBGOAL_SHARING)      
      ALLOC_SG_FR_ARRAY(TrNode_sg_fr(sg_node), THREADS_NUM_BUCKETS); 
#elif defined(THREADS_FULL_SHARING)
      sg_ent_ptr sg_ent;
      new_subgoal_entry(sg_ent);
      TrNode_sg_ent(sg_node) = (sg_node_ptr) sg_ent;
#endif
      TAG_AS_SUBGOAL_LEAF_NODE(sg_node);
    }
    UNLOCK_SUBGOAL_NODE(sg_node);
  }
  sg_fr_addr = (sg_fr_ptr *) get_insert_thread_bucket(
#if defined(THREADS_SUBGOAL_SHARING)
                                (void **) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node)),
#elif defined(THREADS_FULL_SHARING)
                                (void **) &SgEnt_sg_fr((sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node))),
#endif
#ifdef SUBGOAL_TRIE_LOCK_USING_NODE_FIELD
                                   &TrNode_lock(sg_node)
#elif defined(SUBGOAL_TRIE_LOCK_USING_GLOBAL_ARRAY)
                                   &HASH_TRIE_LOCK(sg_node)
#endif                           
  );
#endif /* THREADS_SUBGOAL_SHARING || THREADS_FULL_SHARING */
  return sg_fr_addr;
}

#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL */


static inline sg_fr_ptr get_subgoal_frame(sg_node_ptr sg_node) {
#if defined(THREADS_SUBGOAL_SHARING)
#if defined(THREADS_LOCAL_SG_FR_HASH_BUCKETS) || defined(THREADS_SUBGOAL_FRAME_BY_WID)
  return (sg_fr_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node));
#else
  sg_fr_ptr *sg_fr_addr = (sg_fr_ptr *) get_thread_bucket((void **) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node)));
  return *sg_fr_addr;
#endif /* THREADS_LOCAL_SG_FR_HASH_BUCKETS */
#elif defined(THREADS_FULL_SHARING)
  sg_fr_ptr *sg_fr_addr = (sg_fr_ptr *) get_thread_bucket((void **) &SgEnt_sg_fr((sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node))));
  return *sg_fr_addr;
#else
  return (sg_fr_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node));
#endif
}


static inline sg_fr_ptr get_subgoal_frame_for_abolish(sg_node_ptr sg_node USES_REGS) {
#if defined(THREADS_SUBGOAL_SHARING)
#if defined(THREADS_LOCAL_SG_FR_HASH_BUCKETS) || defined(THREADS_SUBGOAL_FRAME_BY_WID)
  return (sg_fr_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node));
#else
  sg_fr_ptr *sg_fr_addr = (sg_fr_ptr *) get_thread_bucket((void **) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node)));
  sg_fr_ptr sg_fr = *sg_fr_addr;
  abolish_thread_buckets((void **) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node)));
  return sg_fr;
#endif /* THREADS_LOCAL_SG_FR_HASH_BUCKETS */
#elif defined(THREADS_FULL_SHARING)
  sg_fr_ptr *sg_fr_addr = (sg_fr_ptr *) get_thread_bucket((void **) &SgEnt_sg_fr((sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node))));
  sg_fr_ptr sg_fr = *sg_fr_addr;
  abolish_thread_buckets((void **) &SgEnt_sg_fr((sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node))));
    /* just creating a dummy sg_fr to get the information of the answer trie */
    /* this sg_fr is removed during the abolish operation */
  if (sg_fr == NULL){
    new_subgoal_frame(sg_fr, (sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node)));
    *sg_fr_addr = sg_fr;
  }    
  return sg_fr;
#else
  return (sg_fr_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node));
#endif
}

static inline Int freeze_current_cp(void) {
  CACHE_REGS
  choiceptr freeze_cp = B;

  B_FZ  = freeze_cp;
  H_FZ  = freeze_cp->cp_h;
  TR_FZ = freeze_cp->cp_tr;
  B = B->cp_b;
  HB = B->cp_h;
  return (LOCAL_LocalBase - (ADDR)freeze_cp);
}


static inline void wake_frozen_cp(Int frozen_offset) {
  CACHE_REGS
  choiceptr frozen_cp = (choiceptr)(LOCAL_LocalBase - frozen_offset);

  restore_bindings(TR, frozen_cp->cp_tr);
  B = frozen_cp;
  TR = TR_FZ;
  TRAIL_LINK(B->cp_tr);
  return;
}


static inline void abolish_frozen_cps_until(Int frozen_offset) {
  CACHE_REGS
  choiceptr frozen_cp = (choiceptr)(LOCAL_LocalBase - frozen_offset);

  B_FZ  = frozen_cp;
  H_FZ  = frozen_cp->cp_h;
  TR_FZ = frozen_cp->cp_tr;
  return;
}


static inline void abolish_frozen_cps_all(void) {
  CACHE_REGS
  B_FZ  = (choiceptr) LOCAL_LocalBase;
  H_FZ  = (CELL *) LOCAL_GlobalBase;
  TR_FZ = (tr_fr_ptr) LOCAL_TrailBase;
  return;
}


static inline void adjust_freeze_registers(void) {
  CACHE_REGS
  B_FZ  = DepFr_cons_cp(LOCAL_top_dep_fr);
  H_FZ  = B_FZ->cp_h;
  TR_FZ = B_FZ->cp_tr;
  return;
}


#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02

#define INVALIDATE_ANSWER_TRIE_LEAF_NODE(NODE, SG_FR)         \
        if (!IS_ANSWER_INVALID_NODE(NODE)) {                  \
	  TrNode_next(NODE) = SgFr_invalid_chain(SG_FR);      \
	  SgFr_invalid_chain(SG_FR) = NODE;                   \
	  TAG_AS_ANSWER_INVALID_NODE(NODE);                   \
	}

#define INVALIDATE_ANSWER_TRIE_NODE(NODE, SG_FR)              \
        TAG_AS_ANSWER_INVALID_NODE(NODE)

static void invalidate_answer_trie(ans_node_ptr current_node, sg_fr_ptr sg_fr, int position USES_REGS) {
  
  /* when this function is executed, three things happen */
  /* -> the non temporary nodes will not change in the trie, thus no 
     expansion on the tries occurs. This code is safe */
  /* -> temporary nodes will not be added to the trie, thus the 
     worker using this code, can simply avoid it is it finds a temporary
     node */
  /* -> the leaf node passed from temporary to non temporary*/
  if (current_node == NULL || IS_ANSWER_TEMP_NODE(current_node))
    return;

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
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */


static inline void mark_as_completed(sg_fr_ptr sg_fr USES_REGS) {

#ifdef TABLING_EARLY_COMPLETION
    /* as example the test_single02 passes here more than once with tabling_early_completion*/
    /* with early completion a subgoal where the number of substitution variables is zero is completed earlier but it remains on the LOCAL_top_sg_fr, thus passes in the mark_as_complete more that one time */
  if (SgFr_state(sg_fr) >= complete)
    return;  
#endif
  LOCK_SG_FR(sg_fr);

#if defined(THREADS_FULL_SHARING)

#ifdef THREADS_FULL_SHARING_FTNA_3
  if (SgFr_sg_ent_state(sg_fr) < complete) {
    ans_ref_ptr ref_answer = SgFr_cons_ref_first_ans(sg_fr);
    if (ref_answer != NULL) {
      ans_node_ptr last_answer;
      SgFr_first_answer(sg_fr) = last_answer = TrNode_entry(ref_answer);
      TAG_AS_ANSWER_LEAF_NODE(last_answer);
      ref_answer = TrNode_child(ref_answer);
      while(ref_answer) {
	TrNode_child(last_answer) = TrNode_entry(ref_answer);
	last_answer = TrNode_child(last_answer);
	TAG_AS_ANSWER_LEAF_NODE(last_answer);
	ref_answer = TrNode_child(ref_answer);
      }
      SgFr_last_answer(sg_fr) = last_answer;
    }
  }
#endif /* THREADS_FULL_SHARING_FTNA_3 */
  
  SgFr_active_workers(sg_fr)--;
#ifdef MODE_DIRECTED_TABLING
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
  if (SgFr_sg_ent_state(sg_fr) < complete) {
    ans_node_ptr invalid_ans_node;
    invalid_ans_node = SgFr_intra_invalid_chain(sg_fr);
    while (invalid_ans_node) {      
      invalidate_answer_trie(invalid_ans_node, sg_fr, TRAVERSE_POSITION_FIRST PASS_REGS);    
      
      invalid_ans_node = (ans_node_ptr) UNTAG_INTRA_ANSWER_INVALID_NODE(TrNode_intra_invalid_next(invalid_ans_node));      
    }
    SgFr_intra_invalid_chain(sg_fr) = NULL;    
  }
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */

  ans_node_ptr current_node, next_node;
  if (SgFr_invalid_chain(sg_fr)) {
    if (SgFr_sg_ent_state(sg_fr) == complete) {
      if (SgFr_active_workers(sg_fr) == 0) {
	/* free invalid answer nodes */
	current_node = SgFr_invalid_chain(sg_fr);
	SgFr_invalid_chain(sg_fr) = NULL;
	while (current_node) {
	  next_node = TrNode_next(current_node);	
	  ans_node_ptr parent_node, next_parent_node;
	  parent_node = TrNode_parent(current_node);
	  while(IS_ANSWER_INVALID_NODE(parent_node)) {
	    parent_node = (ans_node_ptr) UNTAG_ANSWER_NODE(parent_node);
	    next_parent_node = TrNode_parent(parent_node);
	    FREE_ANSWER_TRIE_NODE(parent_node);
	    parent_node = next_parent_node;
	  }
	  FREE_ANSWER_TRIE_NODE(current_node); // disable flag  use_pages_malloc
	  current_node = next_node;
	}
      }

    } else /* SgFr_sg_ent_state(sg_fr) != complete */ {
      if (SgFr_active_workers(sg_fr) == 0) {
	/* find first valid answer */
	current_node = SgFr_first_answer(sg_fr);
	while (IS_ANSWER_INVALID_NODE(current_node))
	  current_node = TrNode_child(current_node);
	SgFr_first_answer(sg_fr) = current_node;
	/* chain next valid answers */
	next_node = TrNode_child(current_node);
	while (next_node) {
	  if (!IS_ANSWER_INVALID_NODE(next_node)) {
	    TrNode_child(current_node) = next_node;
	    current_node = next_node;   
	  }
	  next_node = TrNode_child(next_node);
	}
	SgFr_last_answer(sg_fr) = current_node;    
	
	/* free invalid answer nodes */
	current_node = SgFr_invalid_chain(sg_fr);
	SgFr_invalid_chain(sg_fr) = NULL;
	
	while (current_node) {
	  next_node = TrNode_next(current_node);	
	  ans_node_ptr parent_node, next_parent_node;
	  parent_node = TrNode_parent(current_node);
	  while(IS_ANSWER_INVALID_NODE(parent_node)) {
	    parent_node = (ans_node_ptr) UNTAG_ANSWER_NODE(parent_node);
	    next_parent_node = TrNode_parent(parent_node);
	    FREE_ANSWER_TRIE_NODE(parent_node);
	    parent_node = next_parent_node;
	  }
	  
	  FREE_ANSWER_TRIE_NODE(current_node);  // disable flag  use_pages_malloc 
	  current_node = next_node;
	}
      } else /* SgFr_active_workers(sg_fr) != 0 */  {
	/* find first valid answer */
	current_node = SgFr_first_answer(sg_fr);
	while (IS_ANSWER_INVALID_NODE(current_node))
	  current_node = TrNode_child(current_node);
	SgFr_first_answer(sg_fr) = current_node;
	/* chain next valid answers */
	next_node = TrNode_child(current_node);
	while (next_node) {
	  if (!IS_ANSWER_INVALID_NODE(next_node)) {
	    TrNode_child(current_node) = next_node;
	    current_node = next_node;   
	  }
	  next_node = TrNode_child(next_node);
	}
	SgFr_last_answer(sg_fr) = current_node;
      }
      TrNode_child(SgFr_last_answer(sg_fr)) = NULL;
    }
  } 

#endif /* MODE_DIRECTED_TABLING */
#else /* !THREADS_FULL_SHARING */

#ifdef MODE_DIRECTED_TABLING
  if (SgFr_invalid_chain(sg_fr)) {
    ans_node_ptr current_node, next_node;
    /* find first valid answer */
    current_node = SgFr_first_answer(sg_fr);
    while (IS_ANSWER_INVALID_NODE(current_node))
      current_node = TrNode_child(current_node);
    SgFr_first_answer(sg_fr) = current_node;
    /* chain next valid answers */
    next_node = TrNode_child(current_node);
    while (next_node) {
      if (! IS_ANSWER_INVALID_NODE(next_node)) {
	TrNode_child(current_node) = next_node;
	current_node = next_node;   
      }
      next_node = TrNode_child(next_node);
    }
    SgFr_last_answer(sg_fr) = current_node;
    /* free invalid answer nodes */
    current_node = SgFr_invalid_chain(sg_fr);
    SgFr_invalid_chain(sg_fr) = NULL;

    while (
#ifdef STUDY_TIMESTAMP_MDT
	   0 && /* don't remove invalid nodes */
#endif /* STUDY_TIMESTAMP_MDT */	   
	   current_node) {
      next_node = TrNode_next(current_node);	
      FREE_ANSWER_TRIE_NODE(current_node);
      current_node = next_node;
    }
  }
#endif /* MODE_DIRECTED_TABLING */
#endif /* THREADS_FULL_SHARING */

#ifdef THREADS_FULL_SHARING
#if defined(THREADS_SUBGOAL_FRAME_BY_WID) && defined(THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE)
  SgFr_state(sg_fr) = complete;
  if (SgFr_sg_ent_state(sg_fr) < complete) {
    SgEnt_sg_fr(SgFr_sg_ent(sg_fr)) = sg_fr;
    SgFr_sg_ent_state(sg_fr) = complete;
  }
#else  /* !THREADS_SUBGOAL_FRAME_BY_WID || !THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE */
  SgFr_sg_ent_state(sg_fr) = SgFr_state(sg_fr) = complete;
#endif /* THREADS_SUBGOAL_FRAME_BY_WID && THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE */
  UNLOCK_SG_FR(sg_fr);
  SgFr_next_complete(sg_fr) = LOCAL_top_sg_fr_complete;
  LOCAL_top_sg_fr_complete = sg_fr;
#else  /* !THREADS_FULL_SHARING */
  SgFr_state(sg_fr) = complete;
#endif /* THREADS_FULL_SHARING */

#ifdef THREADS_SUBGOAL_SHARING
#ifdef THREADS_LOCAL_SG_FR_HASH_BUCKETS
  struct subgoal_trie_node * sg_leaf_node;
  sg_leaf_node = SgFr_sg_leaf_node(sg_fr);
  if (!BOOL_CAS(&(TrNode_sg_fr(sg_leaf_node)), NULL, (sg_node_ptr)((CELL) sg_fr | (CELL)0x1))) {
    SgFr_next_complete(sg_fr) = LOCAL_top_sg_fr_complete;
    LOCAL_top_sg_fr_complete = sg_fr;
  }   
#else /* !THREADS_LOCAL_SG_FR_HASH_BUCKETS */
#ifdef THREADS_SUBGOAL_FRAME_BY_WID
#ifdef THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE
#ifdef THREADS_NO_SUBGOAL_TRIE
  no_subgoal_trie_pos_ptr no_st_pos = SgFr_no_sg_pos(sg_fr);
  if (no_st_pos != NULL) {
    sg_fr_ptr sg_fr_aux;
    do {
      sg_fr_aux = (sg_fr_ptr) SgNoTrie_sg_fr(no_st_pos); 

      if (SgFr_state(sg_fr_aux) >= complete)
        break;    
    } while(!BOOL_CAS(&(SgNoTrie_sg_fr(no_st_pos)), sg_fr_aux, sg_fr));
  } else 
#endif /* THREADS_NO_SUBGOAL_TRIE */
  {
    struct subgoal_trie_node *sg_leaf_node;
    sg_leaf_node = SgFr_sg_leaf_node(sg_fr);  
    sg_fr_ptr sg_fr_aux;
    sg_fr_ptr sg_fr_aux2;
    do {
      sg_fr_aux = (sg_fr_ptr) TrNode_sg_fr(sg_leaf_node);  
      sg_fr_aux2 = (sg_fr_ptr) UNTAG_SUBGOAL_NODE(sg_fr_aux);
      if (sg_fr_aux2 && SgFr_state(sg_fr_aux2) >= complete)
        break;    
    } while(!BOOL_CAS(&(TrNode_sg_fr(sg_leaf_node)), sg_fr_aux, ((CELL) sg_fr | 0x1))); 
  }
#endif /* THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE */

  SgFr_next_complete(sg_fr) = LOCAL_top_sg_fr_complete;
  LOCAL_top_sg_fr_complete = sg_fr;

#else  /* !THREADS_SUBGOAL_FRAME_BY_WID */
#ifdef THREADS_SUBGOAL_FRAME_SHARE_COMPLETE
  sg_fr_ptr *sg_fr_array;
  sg_fr_array = (sg_fr_ptr *) SgFr_sg_fr_array(sg_fr);  
  if (!BOOL_CAS(sg_fr_array, NULL, sg_fr)) 
#endif /* THREADS_SUBGOAL_FRAME_SHARE_COMPLETE */
    {
      SgFr_next_complete(sg_fr) = LOCAL_top_sg_fr_complete;
      LOCAL_top_sg_fr_complete = sg_fr;
    }
#endif /* THREADS_SUBGOAL_FRAME_BY_WID */
#endif /* THREADS_LOCAL_SG_FR_HASH_BUCKETS */
#endif /* THREADS_SUBGOAL_SHARING */
#ifdef THREADS_SUBGOAL_COMPLETION_WAIT
  LOCK_SG_FR_COMP_WAIT(sg_fr); 
  pthread_cond_broadcast(&(SgFr_comp_wait(sg_fr)));
  UNLOCK_SG_FR_COMP_WAIT(sg_fr); 
#endif /* THREADS_SUBGOAL_COMPLETION_WAIT */
  return;
}


static inline void unbind_variables(tr_fr_ptr unbind_tr, tr_fr_ptr end_tr) {
  CACHE_REGS
  TABLING_ERROR_CHECKING(unbind_variables, unbind_tr < end_tr);
  /* unbind loop */
  while (unbind_tr != end_tr) {
    CELL ref = (CELL) TrailTerm(--unbind_tr);
    /* check for global or local variables */
    if (IsVarTerm(ref)) {
      /* unbind variable */
      RESET_VARIABLE(ref);
    } else if (IsPairTerm(ref)) {
      ref = (CELL) RepPair(ref);
      if (IN_BETWEEN(LOCAL_TrailBase, ref, LOCAL_TrailTop)) {
        /* avoid frozen segments */
        unbind_tr = (tr_fr_ptr) ref;
	TABLING_ERROR_CHECKING(unbind_variables, unbind_tr > (tr_fr_ptr) LOCAL_TrailTop);
	TABLING_ERROR_CHECKING(unbind_variables, unbind_tr < end_tr);
      }
#ifdef MULTI_ASSIGNMENT_VARIABLES
    } else {
      CELL *aux_ptr = RepAppl(ref);
      --unbind_tr;
      Term aux_val = TrailVal(unbind_tr);
      *aux_ptr = aux_val;
#endif /* MULTI_ASSIGNMENT_VARIABLES */
    }
  }
  return;
}


static inline void rebind_variables(tr_fr_ptr rebind_tr, tr_fr_ptr end_tr) {
  CACHE_REGS
  TABLING_ERROR_CHECKING(rebind_variables, rebind_tr < end_tr);
  /* rebind loop */
  Yap_NEW_MAHASH((ma_h_inner_struct *)H PASS_REGS);
  while (rebind_tr != end_tr) {
    CELL ref = (CELL) TrailTerm(--rebind_tr);
    /* check for global or local variables */
    if (IsVarTerm(ref)) {
      /* rebind variable */
      *((CELL *)ref) = TrailVal(rebind_tr);
    } else if (IsPairTerm(ref)) {
      ref = (CELL) RepPair(ref);
      if (IN_BETWEEN(LOCAL_TrailBase, ref, LOCAL_TrailTop)) {
        /* avoid frozen segments */
  	rebind_tr = (tr_fr_ptr) ref;
	TABLING_ERROR_CHECKING(rebind_variables, rebind_tr > (tr_fr_ptr) LOCAL_TrailTop);
	TABLING_ERROR_CHECKING(rebind_variables, rebind_tr < end_tr);
      }
#ifdef MULTI_ASSIGNMENT_VARIABLES
    } else {
      CELL *cell_ptr = RepAppl(ref);
      if (!Yap_lookup_ma_var(cell_ptr PASS_REGS)) {
	/* first time we found the variable, let's put the new value */
	*cell_ptr = TrailVal(rebind_tr);
      }
      --rebind_tr;
#endif /* MULTI_ASSIGNMENT_VARIABLES */
    }
  }
  return;
}


static inline void restore_bindings(tr_fr_ptr unbind_tr, tr_fr_ptr rebind_tr) {
  CACHE_REGS
  CELL ref;
  tr_fr_ptr end_tr;

  TABLING_ERROR_CHECKING(restore_variables, unbind_tr < rebind_tr);
  end_tr = rebind_tr;
  Yap_NEW_MAHASH((ma_h_inner_struct *)H PASS_REGS);
  while (unbind_tr != end_tr) {
    /* unbind loop */
    while (unbind_tr > end_tr) {
      ref = (CELL) TrailTerm(--unbind_tr);
      if (IsVarTerm(ref)) {
        RESET_VARIABLE(ref);
      } else if (IsPairTerm(ref)) {
        ref = (CELL) RepPair(ref);
	if (IN_BETWEEN(LOCAL_TrailBase, ref, LOCAL_TrailTop)) {
	  /* avoid frozen segments */
          unbind_tr = (tr_fr_ptr) ref;
	  TABLING_ERROR_CHECKING(restore_variables, unbind_tr > (tr_fr_ptr) LOCAL_TrailTop);
        }
#ifdef MULTI_ASSIGNMENT_VARIABLES
      }	else if (IsApplTerm(ref)) {
	CELL *pt = RepAppl(ref);

	/* AbsAppl means */
	/* multi-assignment variable */
	/* so that the upper cell is the old value */ 
	--unbind_tr;
	if (!Yap_lookup_ma_var(pt PASS_REGS)) {
	  pt[0] = TrailVal(unbind_tr);
	}
#endif /* MULTI_ASSIGNMENT_VARIABLES */
      }
    }
    /* look for end */
    while (unbind_tr < end_tr) {
      ref = (CELL) TrailTerm(--end_tr);
      if (IsPairTerm(ref)) {
        ref = (CELL) RepPair(ref);
	if (IN_BETWEEN(LOCAL_TrailBase, ref, LOCAL_TrailTop)) {
	  /* avoid frozen segments */
  	  end_tr = (tr_fr_ptr) ref;
	  TABLING_ERROR_CHECKING(restore_variables, end_tr > (tr_fr_ptr) LOCAL_TrailTop);
        }
      }
    }
  }
  /* rebind loop */
  while (rebind_tr != end_tr) {
    ref = (CELL) TrailTerm(--rebind_tr);
    if (IsVarTerm(ref)) {
      *((CELL *)ref) = TrailVal(rebind_tr);
    } else if (IsPairTerm(ref)) {
      ref = (CELL) RepPair(ref);
      if (IN_BETWEEN(LOCAL_TrailBase, ref, LOCAL_TrailTop)) {
	/* avoid frozen segments */
        rebind_tr = (tr_fr_ptr) ref;
	TABLING_ERROR_CHECKING(restore_variables, rebind_tr > (tr_fr_ptr) LOCAL_TrailTop);
	TABLING_ERROR_CHECKING(restore_variables, rebind_tr < end_tr);
      }
#ifdef MULTI_ASSIGNMENT_VARIABLES
    } else {
      CELL *cell_ptr = RepAppl(ref);
      /* first time we found the variable, let's put the new value */
      *cell_ptr = TrailVal(rebind_tr);
      --rebind_tr;
#endif /* MULTI_ASSIGNMENT_VARIABLES */
    }
  }
  return;
}


static inline CELL *expand_auxiliary_stack(CELL *stack) {
  CACHE_REGS
  void *old_top = LOCAL_TrailTop;
  INFORMATION_MESSAGE("Expanding trail in 64 Kbytes");
  if (! Yap_growtrail(K64, TRUE)) {  /* TRUE means 'contiguous_only' */
    Yap_Error(OUT_OF_TRAIL_ERROR, TermNil, "stack full (STACK_CHECK_EXPAND)");
    return NULL;
  } else {
    UInt diff = (void *)LOCAL_TrailTop - old_top;
    CELL *new_stack = (CELL *)((void *)stack + diff);
    memmove((void *)new_stack, (void *)stack, old_top - (void *)stack);
    return new_stack;
  }
}


static inline void abolish_incomplete_subgoals(choiceptr prune_cp) {
  CACHE_REGS

#ifdef LINEAR_TABLING    
    while (LOCAL_top_sg_fr && EQUAL_OR_YOUNGER_CP(SgFr_gen_cp(LOCAL_top_sg_fr), prune_cp)) {
      sg_fr_ptr sg_fr;
      sg_fr = LOCAL_top_sg_fr;
      LOCAL_top_sg_fr = SgFr_next(sg_fr);
      int cut_dfn;
#ifdef LINEAR_TABLING_DRE
      if (SgFr_pioneer_frame(sg_fr)!=NULL){
	/*follower subgoal frame--> clear subgoals in max_scc chain up to the pioneer */
	cut_dfn= GET_SGFR_DFN(SgFr_pioneer_frame(sg_fr));
      }else
#endif /*LINEAR_TABLING_DRE*/
	cut_dfn = GET_SGFR_DFN(sg_fr);
      while(LOCAL_max_scc && GET_SGFR_DFN(LOCAL_max_scc) >= cut_dfn){
	sg_fr = LOCAL_max_scc;
	SgFr_state(sg_fr) = incomplete;
	if (LOCAL_max_scc == LOCAL_top_sg_fr_on_branch)
	  LOCAL_top_sg_fr_on_branch = SgFr_next_on_branch(LOCAL_top_sg_fr_on_branch);	  
	LOCAL_max_scc =SgFr_next_on_scc(LOCAL_max_scc);
	
#ifdef THREADS_FULL_SHARING
	LOCK_SG_FR(sg_fr);
	SgFr_active_workers(sg_fr)--;
	UNLOCK_SG_FR(sg_fr);
#endif /* THREADS_FULL_SHARING */
	
#ifdef THREADS_FULL_SHARING_FTNA_3
	if (SgFr_cons_ref_first_ans(sg_fr) == NULL) {
	  /* no answers --> ready */
	  SgFr_state(sg_fr) = ready;
	} else if (TrNode_entry(SgFr_cons_ref_first_ans(sg_fr)) == SgFr_answer_trie(sg_fr)) {
	  /* yes answer --> complete */
	  SgFr_first_answer(sg_fr) = SgFr_last_answer(sg_fr) = SgFr_answer_trie(sg_fr);
	  consumer_trie_free_structs(sg_fr PASS_REGS);
	  SgFr_sg_ent_state(sg_fr) = SgFr_state(sg_fr) = complete;/* assuming load_answers only*/
	}    
#else  /* !THREADS_FULL_SHARING_FTNA_3 */
	if (SgFr_first_answer(sg_fr) == NULL) {
	  /* no answers --> ready */
	  SgFr_state(sg_fr) = ready;
	} else if (SgFr_first_answer(sg_fr) == SgFr_answer_trie(sg_fr)) {
	  /* yes answer --> complete */
#ifdef THREADS_FULL_SHARING
	  SgFr_sg_ent_state(sg_fr) = /* assuming load_answers only */
#endif /* THREADS_FULL_SHARING */
	    SgFr_state(sg_fr) = complete;
	}
#endif  /* THREADS_FULL_SHARING_FTNA_3 */
	else {
	  /* answers --> incomplete/ready */
#ifdef INCOMPLETE_TABLING
	  SgFr_state(sg_fr) = incomplete;
#if defined(MODE_DIRECTED_TABLING) && !defined(THREADS_FULL_SHARING)
	  if (SgFr_invalid_chain(sg_fr)) {
	    ans_node_ptr current_node, next_node;
	    /* find first valid answer */
	    current_node = SgFr_first_answer(sg_fr);
	    while (IS_ANSWER_INVALID_NODE(current_node))
	      current_node = TrNode_child(current_node);
	    SgFr_first_answer(sg_fr) = current_node;
	    /* chain next valid answers */
	    next_node = TrNode_child(current_node);
	    while (next_node) {
	      if (! IS_ANSWER_INVALID_NODE(next_node)) {
		TrNode_child(current_node) = next_node;
		current_node = next_node;   
	      }
	      next_node = TrNode_child(next_node);
	    }
	    SgFr_last_answer(sg_fr) = current_node;
	    /* free invalid answer nodes */
	    current_node = SgFr_invalid_chain(sg_fr);
	    SgFr_invalid_chain(sg_fr) = NULL;
	    while (current_node) {
	      next_node = TrNode_next(current_node);	
	      FREE_ANSWER_TRIE_NODE(current_node);
	      current_node = next_node;
	    }
	  }
#endif /* MODE_DIRECTED_TABLING && !THREADS_FULL_SHARING */
#else  /* !INCOMPLETE_TABLING */
	  SgFr_state(sg_fr) = ready;
#ifdef THREADS_FULL_SHARING_FTNA_3
	  consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
#ifndef THREADS_FULL_SHARING
#ifndef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 
	  free_answer_hash_chain(SgFr_hash_chain(sg_fr) PASS_REGS);
#endif
	  SgFr_hash_chain(sg_fr) = NULL;
	  SgFr_first_answer(sg_fr) = NULL;
	  SgFr_last_answer(sg_fr) = NULL;
	  ans_node_ptr node = TrNode_child(SgFr_answer_trie(sg_fr));
	  TrNode_child(SgFr_answer_trie(sg_fr)) = NULL;
	  free_answer_trie(node, TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);      
#ifdef MODE_DIRECTED_TABLING
	  ans_node_ptr invalid_node = SgFr_invalid_chain(sg_fr);
	  SgFr_invalid_chain(sg_fr) = NULL;
	  /* free invalid answer nodes */
	  while (invalid_node) {
	    node = TrNode_next(invalid_node);	
	    FREE_ANSWER_TRIE_NODE(invalid_node);
	    invalid_node = node;
	  }
#endif /* MODE_DIRECTED_TABLING */
#endif /* !THREADS_FULL_SHARING */      
#endif /* INCOMPLETE_TABLING */
	} 
      }
    }
  return;
}

#else /*!LINEAR_TABLING (SUSPENSION BASED TABLING) */

#ifdef YAPOR
  if (EQUAL_OR_YOUNGER_CP(GetOrFr_node(LOCAL_top_susp_or_fr), prune_cp))
    pruning_over_tabling_data_structures();
#endif /* YAPOR */

  if (EQUAL_OR_YOUNGER_CP(DepFr_cons_cp(LOCAL_top_dep_fr), prune_cp)) {
#ifdef YAPOR
    if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING)
      pruning_over_tabling_data_structures();
#endif /* YAPOR */
    do {
      dep_fr_ptr dep_fr = LOCAL_top_dep_fr;
      LOCAL_top_dep_fr = DepFr_next(dep_fr);
      FREE_DEPENDENCY_FRAME(dep_fr);
    } while (EQUAL_OR_YOUNGER_CP(DepFr_cons_cp(LOCAL_top_dep_fr), prune_cp));
    adjust_freeze_registers();
  }

  while (LOCAL_top_sg_fr && EQUAL_OR_YOUNGER_CP(SgFr_gen_cp(LOCAL_top_sg_fr), prune_cp)) {
    sg_fr_ptr sg_fr;
#ifdef YAPOR
    if (GLOBAL_parallel_mode == PARALLEL_MODE_RUNNING)
      pruning_over_tabling_data_structures();
#endif /* YAPOR */
    sg_fr = LOCAL_top_sg_fr;
    LOCAL_top_sg_fr = SgFr_next(sg_fr);

#ifdef THREADS_FULL_SHARING
    LOCK_SG_FR(sg_fr);
    SgFr_active_workers(sg_fr)--;
    UNLOCK_SG_FR(sg_fr);
#endif /* THREADS_FULL_SHARING */

#ifdef THREADS_FULL_SHARING_FTNA_3
    if (SgFr_cons_ref_first_ans(sg_fr) == NULL) {
      /* no answers --> ready */
      SgFr_state(sg_fr) = ready;
    } else if (TrNode_entry(SgFr_cons_ref_first_ans(sg_fr)) == SgFr_answer_trie(sg_fr)) {
      /* yes answer --> complete */
      SgFr_first_answer(sg_fr) = SgFr_last_answer(sg_fr) = SgFr_answer_trie(sg_fr);
      consumer_trie_free_structs(sg_fr PASS_REGS);
      SgFr_sg_ent_state(sg_fr) = SgFr_state(sg_fr) = complete;/* assuming load_answers only*/
    }    
#else  /* !THREADS_FULL_SHARING_FTNA_3 */
    if (SgFr_first_answer(sg_fr) == NULL) {
      /* no answers --> ready */
      SgFr_state(sg_fr) = ready;
    } else if (SgFr_first_answer(sg_fr) == SgFr_answer_trie(sg_fr)) {
      /* yes answer --> complete */
#ifdef THREADS_FULL_SHARING
      SgFr_sg_ent_state(sg_fr) = /* assuming load_answers only */
#endif /* THREADS_FULL_SHARING */
	SgFr_state(sg_fr) = complete;
    }
#endif  /* THREADS_FULL_SHARING_FTNA_3 */
    else {
      /* answers --> incomplete/ready */
#ifdef INCOMPLETE_TABLING
      SgFr_state(sg_fr) = incomplete;
#if defined(MODE_DIRECTED_TABLING) && !defined(THREADS_FULL_SHARING)
      if (SgFr_invalid_chain(sg_fr)) {
	ans_node_ptr current_node, next_node;
	/* find first valid answer */
	current_node = SgFr_first_answer(sg_fr);
	while (IS_ANSWER_INVALID_NODE(current_node))
	  current_node = TrNode_child(current_node);
	SgFr_first_answer(sg_fr) = current_node;
	/* chain next valid answers */
	next_node = TrNode_child(current_node);
	while (next_node) {
	  if (! IS_ANSWER_INVALID_NODE(next_node)) {
	    TrNode_child(current_node) = next_node;
	    current_node = next_node;   
	  }
	  next_node = TrNode_child(next_node);
	}
	SgFr_last_answer(sg_fr) = current_node;
	/* free invalid answer nodes */
	current_node = SgFr_invalid_chain(sg_fr);
	SgFr_invalid_chain(sg_fr) = NULL;
	while (current_node) {
	  next_node = TrNode_next(current_node);	
	  FREE_ANSWER_TRIE_NODE(current_node);
	  current_node = next_node;
	}
      }
#endif /* MODE_DIRECTED_TABLING && !THREADS_FULL_SHARING */
#else  /* !INCOMPLETE_TABLING */
      SgFr_state(sg_fr) = ready;
#ifdef THREADS_FULL_SHARING_FTNA_3
      consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
#ifndef THREADS_FULL_SHARING
#ifndef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 
    free_answer_hash_chain(SgFr_hash_chain(sg_fr) PASS_REGS);
#endif
      SgFr_hash_chain(sg_fr) = NULL;
      SgFr_first_answer(sg_fr) = NULL;
      SgFr_last_answer(sg_fr) = NULL;
      ans_node_ptr node = TrNode_child(SgFr_answer_trie(sg_fr));
      TrNode_child(SgFr_answer_trie(sg_fr)) = NULL;
      free_answer_trie(node, TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);      
#ifdef MODE_DIRECTED_TABLING
      ans_node_ptr invalid_node = SgFr_invalid_chain(sg_fr);
      SgFr_invalid_chain(sg_fr) = NULL;
      /* free invalid answer nodes */
      while (invalid_node) {
	node = TrNode_next(invalid_node);	
	FREE_ANSWER_TRIE_NODE(invalid_node);
	invalid_node = node;
      }
#endif /* MODE_DIRECTED_TABLING */
#endif /* !THREADS_FULL_SHARING */      
#endif /* INCOMPLETE_TABLING */
    }
#ifdef LIMIT_TABLING
    insert_into_global_sg_fr_list(sg_fr);
#endif /* LIMIT_TABLING */
  }
  return;
}

#endif /*LINEAR_TABLING */

#ifdef YAPOR
static inline void pruning_over_tabling_data_structures(void) {
  Yap_Error(INTERNAL_ERROR, TermNil, "pruning over tabling data structures");
  return;
}


static inline void collect_suspension_frames(or_fr_ptr or_fr) {  
  CACHE_REGS
  int depth;
  or_fr_ptr *susp_ptr;

  OPTYAP_ERROR_CHECKING(collect_suspension_frames, IS_UNLOCKED(or_fr));
  OPTYAP_ERROR_CHECKING(collect_suspension_frames, OrFr_suspensions(or_fr) == NULL);

  /* order collected suspension frames by depth */
  depth = OrFr_depth(or_fr);
  susp_ptr = & LOCAL_top_susp_or_fr;
  while (OrFr_depth(*susp_ptr) > depth)
    susp_ptr = & OrFr_nearest_suspnode(*susp_ptr);
  OrFr_nearest_suspnode(or_fr) = *susp_ptr;
  *susp_ptr = or_fr;
  return;
}


static inline
#ifdef TIMESTAMP_CHECK
susp_fr_ptr suspension_frame_to_resume(or_fr_ptr susp_or_fr, long timestamp) {
#else
susp_fr_ptr suspension_frame_to_resume(or_fr_ptr susp_or_fr) {
#endif /* TIMESTAMP_CHECK */
  choiceptr top_cp;
  susp_fr_ptr *susp_ptr, susp_fr;
  dep_fr_ptr dep_fr;

  top_cp = GetOrFr_node(susp_or_fr);
  susp_ptr = & OrFr_suspensions(susp_or_fr);
  susp_fr = *susp_ptr;
  while (susp_fr) {
    dep_fr = SuspFr_top_dep_fr(susp_fr);
    do {
      if (TrNode_child(DepFr_last_answer(dep_fr))) {
        /* unconsumed answers in susp_fr */
        *susp_ptr = SuspFr_next(susp_fr);
        return susp_fr;
      }
#ifdef TIMESTAMP_CHECK
      DepFr_timestamp(dep_fr) = timestamp;
#endif /* TIMESTAMP_CHECK */
      dep_fr = DepFr_next(dep_fr);
#ifdef TIMESTAMP_CHECK
    } while (timestamp > DepFr_timestamp(dep_fr) && YOUNGER_CP(DepFr_cons_cp(dep_fr), top_cp));
#else
    } while (YOUNGER_CP(DepFr_cons_cp(dep_fr), top_cp));
#endif /* TIMESTAMP_CHECK */
    susp_ptr = & SuspFr_next(susp_fr);
    susp_fr = *susp_ptr;
  }
  /* no suspension frame with unconsumed answers */
  return NULL;
}
#endif /* YAPOR */


#ifdef TABLING_INNER_CUTS
static inline void CUT_store_tg_answer(or_fr_ptr or_frame, ans_node_ptr ans_node, choiceptr gen_cp, int ltt) {
  tg_sol_fr_ptr tg_sol_fr, *solution_ptr, next, ltt_next;
  tg_ans_fr_ptr tg_ans_fr;

  solution_ptr = & OrFr_tg_solutions(or_frame);
  while (*solution_ptr && YOUNGER_CP(gen_cp, TgSolFr_gen_cp(*solution_ptr))) {
    solution_ptr = & TgSolFr_next(*solution_ptr);
  }
  if (*solution_ptr && gen_cp == TgSolFr_gen_cp(*solution_ptr)) {
    if (ltt >= TgSolFr_ltt(*solution_ptr)) {
      while (*solution_ptr && ltt > TgSolFr_ltt(*solution_ptr)) {
        solution_ptr = & TgSolFr_ltt_next(*solution_ptr);
      }
      if (*solution_ptr && ltt == TgSolFr_ltt(*solution_ptr)) {
        tg_ans_fr = TgSolFr_first(*solution_ptr);
        if (TgAnsFr_free_slot(tg_ans_fr) == TG_ANSWER_SLOTS) {
          ALLOC_TG_ANSWER_FRAME(tg_ans_fr);
          TgAnsFr_free_slot(tg_ans_fr) = 1;
          TgAnsFr_answer(tg_ans_fr, 0) = ans_node;
          TgAnsFr_next(tg_ans_fr) = TgSolFr_first(*solution_ptr);
          TgSolFr_first(*solution_ptr) = tg_ans_fr;
        } else {
          TgAnsFr_answer(tg_ans_fr, TgAnsFr_free_slot(tg_ans_fr)) = ans_node;
          TgAnsFr_free_slot(tg_ans_fr)++;
        }
        return;
      }
      ltt_next = *solution_ptr;
      next = NULL;
    } else {
      ltt_next = *solution_ptr;
      next = TgSolFr_next(*solution_ptr);
    }
  } else {
    ltt_next = NULL;
    next = *solution_ptr;
  }
  ALLOC_TG_ANSWER_FRAME(tg_ans_fr);
  TgAnsFr_free_slot(tg_ans_fr) = 1;
  TgAnsFr_answer(tg_ans_fr, 0) = ans_node;
  TgAnsFr_next(tg_ans_fr) = NULL;
  ALLOC_TG_SOLUTION_FRAME(tg_sol_fr);
  TgSolFr_gen_cp(tg_sol_fr) = gen_cp;
  TgSolFr_ltt(tg_sol_fr) = ltt;
  TgSolFr_first(tg_sol_fr) = tg_ans_fr;
  TgSolFr_last(tg_sol_fr) = tg_ans_fr;
  TgSolFr_ltt_next(tg_sol_fr) = ltt_next;
  TgSolFr_next(tg_sol_fr) = next;
  *solution_ptr = tg_sol_fr;
  return;
}


static inline tg_sol_fr_ptr CUT_store_tg_answers(or_fr_ptr or_frame, tg_sol_fr_ptr new_solution, int ltt) {
  tg_sol_fr_ptr *old_solution_ptr, next_new_solution;
  choiceptr node, gen_cp;

  old_solution_ptr = & OrFr_tg_solutions(or_frame);
  node = GetOrFr_node(or_frame);
  while (new_solution && YOUNGER_CP(node, TgSolFr_gen_cp(new_solution))) {
    next_new_solution = TgSolFr_next(new_solution);
    gen_cp = TgSolFr_gen_cp(new_solution);
    while (*old_solution_ptr && YOUNGER_CP(gen_cp, TgSolFr_gen_cp(*old_solution_ptr))) {
      old_solution_ptr = & TgSolFr_next(*old_solution_ptr);
    }
    if (*old_solution_ptr && gen_cp == TgSolFr_gen_cp(*old_solution_ptr)) {
      if (ltt >= TgSolFr_ltt(*old_solution_ptr)) {
        tg_sol_fr_ptr *ltt_next_old_solution_ptr;
        ltt_next_old_solution_ptr = old_solution_ptr;
        while (*ltt_next_old_solution_ptr && ltt > TgSolFr_ltt(*ltt_next_old_solution_ptr)) {
          ltt_next_old_solution_ptr = & TgSolFr_ltt_next(*ltt_next_old_solution_ptr);
        }
        if (*ltt_next_old_solution_ptr && ltt == TgSolFr_ltt(*ltt_next_old_solution_ptr)) {
          TgAnsFr_next(TgSolFr_last(*ltt_next_old_solution_ptr)) = TgSolFr_first(new_solution);
          TgSolFr_last(*ltt_next_old_solution_ptr) = TgSolFr_last(new_solution);
          FREE_TG_SOLUTION_FRAME(new_solution);
        } else {
          TgSolFr_ltt(new_solution) = ltt;
          TgSolFr_ltt_next(new_solution) = *ltt_next_old_solution_ptr;
          TgSolFr_next(new_solution) = NULL;
          *ltt_next_old_solution_ptr = new_solution;
	}
      } else {
        TgSolFr_ltt(new_solution) = ltt;
        TgSolFr_ltt_next(new_solution) = *old_solution_ptr;
        TgSolFr_next(new_solution) = TgSolFr_next(*old_solution_ptr);
        *old_solution_ptr = new_solution;
      }
    } else {
      TgSolFr_ltt(new_solution) = ltt;
      TgSolFr_ltt_next(new_solution) = NULL;
      TgSolFr_next(new_solution) = *old_solution_ptr;
      *old_solution_ptr = new_solution;
    }
    old_solution_ptr = & TgSolFr_next(*old_solution_ptr);
    new_solution = next_new_solution;
  }
  return new_solution;
}


static inline void CUT_validate_tg_answers(tg_sol_fr_ptr valid_solutions) {
  tg_ans_fr_ptr valid_answers, free_answer;
  tg_sol_fr_ptr ltt_valid_solutions, free_solution;
  ans_node_ptr first_answer, last_answer, ans_node;
  sg_fr_ptr sg_fr;
  int slots;

  while (valid_solutions) {
    first_answer = last_answer = NULL;
#ifdef DETERMINISTIC_TABLING
    if (IS_DET_GEN_CP(TgSolFr_gen_cp(valid_solutions)))
      sg_fr = DET_GEN_CP(TgSolFr_gen_cp(valid_solutions))->cp_sg_fr;
    else
#endif /* DETERMINISTIC_TABLING */
      sg_fr = GEN_CP(TgSolFr_gen_cp(valid_solutions))->cp_sg_fr;
    ltt_valid_solutions = valid_solutions;
    valid_solutions = TgSolFr_next(valid_solutions);
    do {
      valid_answers = TgSolFr_first(ltt_valid_solutions);
      do {
        slots = TgAnsFr_free_slot(valid_answers);
        do {
          ans_node = TgAnsFr_answer(valid_answers, --slots);
	  LOCK_ANSWER_TRIE(sg_fr);
	  LOCK_ANSWER_NODE(ans_node);
          if (! IS_ANSWER_LEAF_NODE(ans_node)) {
            TAG_AS_ANSWER_LEAF_NODE(ans_node);
            if (first_answer == NULL)
	      first_answer = ans_node;
	    else
              TrNode_child(last_answer) = ans_node;
	    last_answer = ans_node;
	  }
          UNLOCK_ANSWER_NODE(ans_node);	  
	  UNLOCK_ANSWER_TRIE(sg_fr);
        } while (slots);
        free_answer = valid_answers;
        valid_answers = TgAnsFr_next(valid_answers);
        FREE_TG_ANSWER_FRAME(free_answer);
      } while (valid_answers);
      free_solution = ltt_valid_solutions;
      ltt_valid_solutions = TgSolFr_ltt_next(ltt_valid_solutions);
      FREE_TG_SOLUTION_FRAME(free_solution);
    } while (ltt_valid_solutions);
    if (first_answer) {
      LOCK_SG_FR(sg_fr);
      if (SgFr_first_answer(sg_fr) == NULL) {
        SgFr_first_answer(sg_fr) = first_answer;
      } else {
        TrNode_child(SgFr_last_answer(sg_fr)) = first_answer;
      }
      SgFr_last_answer(sg_fr) = last_answer;
      UNLOCK_SG_FR(sg_fr);
    }
  }
  return;
}


static inline void CUT_join_tg_solutions(tg_sol_fr_ptr *old_solution_ptr, tg_sol_fr_ptr new_solution) {
  tg_sol_fr_ptr next_old_solution, next_new_solution;
  choiceptr gen_cp;

  do {
    gen_cp = TgSolFr_gen_cp(new_solution);
    while (*old_solution_ptr && YOUNGER_CP(gen_cp, TgSolFr_gen_cp(*old_solution_ptr))) {
      old_solution_ptr = & TgSolFr_next(*old_solution_ptr);
    }
    if (*old_solution_ptr) {
      next_old_solution = *old_solution_ptr;
      *old_solution_ptr = new_solution;
      CUT_join_solution_frame_tg_answers(new_solution);
      if (gen_cp == TgSolFr_gen_cp(next_old_solution)) {
        tg_sol_fr_ptr free_solution;
        TgAnsFr_next(TgSolFr_last(new_solution)) = TgSolFr_first(next_old_solution);
        TgSolFr_last(new_solution) = TgSolFr_last(next_old_solution);
        free_solution = next_old_solution;
        next_old_solution = TgSolFr_next(next_old_solution);
        FREE_TG_SOLUTION_FRAME(free_solution);
        if (! next_old_solution) {
          if ((next_new_solution = TgSolFr_next(new_solution))) {
            CUT_join_solution_frames_tg_answers(next_new_solution);
	  }
          return;
	}
      }
      gen_cp = TgSolFr_gen_cp(next_old_solution);
      next_new_solution = TgSolFr_next(new_solution);
      while (next_new_solution && YOUNGER_CP(gen_cp, TgSolFr_gen_cp(next_new_solution))) {
        new_solution = next_new_solution;
        next_new_solution = TgSolFr_next(new_solution);
        CUT_join_solution_frame_tg_answers(new_solution);
      }
      old_solution_ptr = & TgSolFr_next(new_solution);
      TgSolFr_next(new_solution) = next_old_solution;
      new_solution = next_new_solution;
    } else {
      *old_solution_ptr = new_solution;
      CUT_join_solution_frames_tg_answers(new_solution);
      return;
    }
  } while (new_solution);
  return;
}


static inline void CUT_join_solution_frame_tg_answers(tg_sol_fr_ptr join_solution) {
  tg_sol_fr_ptr next_solution;

  while ((next_solution = TgSolFr_ltt_next(join_solution))) {
    TgAnsFr_next(TgSolFr_last(join_solution)) = TgSolFr_first(next_solution);
    TgSolFr_last(join_solution) = TgSolFr_last(next_solution);
    TgSolFr_ltt_next(join_solution) = TgSolFr_ltt_next(next_solution);
    FREE_TG_SOLUTION_FRAME(next_solution);
  }
  return;
}


static inline void CUT_join_solution_frames_tg_answers(tg_sol_fr_ptr join_solution) {
  do {
    CUT_join_solution_frame_tg_answers(join_solution);
    join_solution = TgSolFr_next(join_solution);
  } while (join_solution);
  return;
}


static inline void CUT_free_tg_solution_frame(tg_sol_fr_ptr solution) {
  tg_ans_fr_ptr current_answer, next_answer;

  current_answer = TgSolFr_first(solution);
  do {
    next_answer = TgAnsFr_next(current_answer);
    FREE_TG_ANSWER_FRAME(current_answer);
    current_answer = next_answer;
  } while (current_answer);
  FREE_TG_SOLUTION_FRAME(solution);
  return;
}


static inline void CUT_free_tg_solution_frames(tg_sol_fr_ptr current_solution) {
  tg_sol_fr_ptr ltt_solution, next_solution;

  while (current_solution) {
    ltt_solution = TgSolFr_ltt_next(current_solution);
    while (ltt_solution) {
      next_solution = TgSolFr_ltt_next(ltt_solution);
      CUT_free_tg_solution_frame(ltt_solution);
      ltt_solution = next_solution;
    }
    next_solution = TgSolFr_next(current_solution);
    CUT_free_tg_solution_frame(current_solution);
    current_solution = next_solution;
  }
  return;
}


static inline tg_sol_fr_ptr CUT_prune_tg_solution_frames(tg_sol_fr_ptr solutions, int ltt) {
  tg_sol_fr_ptr ltt_next_solution, return_solution;

  if (! solutions) return NULL;
  return_solution = CUT_prune_tg_solution_frames(TgSolFr_next(solutions), ltt);
  while (solutions && ltt > TgSolFr_ltt(solutions)) {
    ltt_next_solution = TgSolFr_ltt_next(solutions);
    CUT_free_tg_solution_frame(solutions);
    solutions = ltt_next_solution;
  }
  if (solutions) {
    TgSolFr_next(solutions) = return_solution;
    return solutions;
  } else {
    return return_solution;
  }
}
#endif /* TABLING_INNER_CUTS */



#ifdef THREADS_FULL_SHARING_FTNA_3_AFS_V04_VERSION

/* USES AFS_V04 CODE AND ALLOCATES THE SAME TYPE OF HASHES */


static inline void consumer_trie_insert_bucket_chain(ans_ref_ptr *curr_hash, ans_ref_ptr chain_node, ans_ref_ptr adjust_node, long n_shifts, int count_nodes USES_REGS) { 
  ans_node_ptr t = TrNode_entry(adjust_node);
  int cn = count_nodes + 1;
  ans_ref_ptr chain_next;
  chain_next = TrNode_next(chain_node);
  if (!V04_IS_HASH(chain_next))
    return consumer_trie_insert_bucket_chain(curr_hash, chain_next, adjust_node, n_shifts, cn PASS_REGS);  
  
  // chain_next is a hash pointer
  if ((ans_ref_ptr *)chain_next == curr_hash) {
    if (cn == MAX_NODES_PER_BUCKET) {
      ans_ref_ptr *new_hash;
      ans_ref_ptr *bucket;
      V04_ALLOC_BUCKETS(new_hash, curr_hash, struct answer_ref_node);
      new_hash = (ans_ref_ptr *) V04_TAG(new_hash);
      V04_GET_HASH_BUCKET(bucket, new_hash, TrNode_entry(chain_node), n_shifts + 1, struct answer_ref_node);
      V04_SET_HASH_BUCKET(bucket, chain_node, struct answer_ref_node);
      TrNode_next(chain_node) = (ans_ref_ptr) new_hash;
      V04_GET_HASH_BUCKET(bucket, curr_hash, t, n_shifts, struct answer_ref_node);
      consumer_trie_adjust_chain_nodes(new_hash, *bucket, chain_node, n_shifts PASS_REGS);
      V04_SET_HASH_BUCKET(bucket, new_hash, struct answer_ref_node);
      return consumer_trie_insert_bucket_array(new_hash, adjust_node, (n_shifts + 1) PASS_REGS);
    } else {
      TrNode_next(adjust_node) = (ans_ref_ptr) curr_hash;
      TrNode_next(chain_node) = adjust_node;
    }
  }
  return;
} 

static inline void consumer_trie_insert_bucket_array(ans_ref_ptr *curr_hash, ans_ref_ptr chain_node, long n_shifts USES_REGS) {
  ans_ref_ptr *bucket; 
  TrNode_next(chain_node) = (ans_ref_ptr) curr_hash;
  V04_GET_HASH_BUCKET(bucket, curr_hash, TrNode_entry(chain_node), n_shifts, struct answer_ref_node);
  if (V04_IS_EMPTY_BUCKET(*bucket, curr_hash, struct answer_ref_node)) {
    V04_SET_HASH_BUCKET(bucket, chain_node, struct answer_ref_node);
    return;  
  }
  ans_ref_ptr bucket_next = *bucket;
  if (V04_IS_HASH(bucket_next))
    return consumer_trie_insert_bucket_array((ans_ref_ptr *)bucket_next, chain_node, (n_shifts + 1) PASS_REGS);
  else 
    return consumer_trie_insert_bucket_chain(curr_hash, bucket_next, chain_node, n_shifts, 0 PASS_REGS);
}


static inline void consumer_trie_adjust_chain_nodes(ans_ref_ptr *new_hash, ans_ref_ptr chain_node, ans_ref_ptr last_node, long n_shifts USES_REGS) {
  if (chain_node == last_node)
    return;
  consumer_trie_adjust_chain_nodes(new_hash, TrNode_next(chain_node), last_node, n_shifts PASS_REGS);
  return consumer_trie_insert_bucket_array(new_hash, chain_node, (n_shifts + 1) PASS_REGS);
}

static inline void consumer_trie_check_insert_bucket_chain(ans_ref_ptr *curr_hash, ans_ref_ptr chain_node, sg_fr_ptr sg_fr, ans_node_ptr t, long n_shifts, int count_nodes USES_REGS) {
  if (V04_IS_EQUAL_ENTRY(chain_node, t))
    return;  
  int cn = count_nodes + 1;
  ans_ref_ptr chain_next;
  chain_next = TrNode_next(chain_node);
  if (!V04_IS_HASH(chain_next))
    return consumer_trie_check_insert_bucket_chain(curr_hash, chain_next, sg_fr, t, n_shifts, cn PASS_REGS);  
  
  // chain_next is a hash pointer
  if ((ans_ref_ptr *)chain_next == curr_hash) {
    if (cn == MAX_NODES_PER_BUCKET) {
      ans_ref_ptr *new_hash;
      ans_ref_ptr *bucket;
      V04_ALLOC_BUCKETS(new_hash, curr_hash, struct answer_ref_node);
      new_hash = (ans_ref_ptr *) V04_TAG(new_hash);
      V04_GET_HASH_BUCKET(bucket, new_hash, TrNode_entry(chain_node), n_shifts + 1, struct answer_ref_node);
      V04_SET_HASH_BUCKET(bucket, chain_node, struct answer_ref_node);
      TrNode_next(chain_node) = (ans_ref_ptr) new_hash;
      V04_GET_HASH_BUCKET(bucket, curr_hash, t, n_shifts, struct answer_ref_node);
      consumer_trie_adjust_chain_nodes(new_hash, *bucket, chain_node, n_shifts PASS_REGS);
      V04_SET_HASH_BUCKET(bucket, new_hash, struct answer_ref_node);
      return consumer_trie_check_insert_bucket_array(new_hash, sg_fr, t, (n_shifts + 1) PASS_REGS);
    } else {
      ans_ref_ptr new_node; 
      new_answer_ref_node(new_node, t, (ans_ref_ptr) curr_hash, NULL);
      TrNode_next(chain_node) = new_node;
      TrNode_child(SgFr_cons_ref_last_ans(sg_fr)) = new_node;
      SgFr_cons_ref_last_ans(sg_fr) = new_node;
      return;
    }
  }
}

static inline void consumer_trie_check_insert_bucket_array(ans_ref_ptr *curr_hash, sg_fr_ptr sg_fr, ans_node_ptr t, long n_shifts USES_REGS) {
  ans_ref_ptr *bucket; 
  V04_GET_HASH_BUCKET(bucket, curr_hash, t, n_shifts, struct answer_ref_node);
  if (V04_IS_EMPTY_BUCKET(*bucket, curr_hash, struct answer_ref_node)) {
    ans_ref_ptr new_node; 
    new_answer_ref_node(new_node, t, (ans_ref_ptr) curr_hash, NULL);
    V04_SET_HASH_BUCKET(bucket, new_node, struct answer_ref_node);
    TrNode_child(SgFr_cons_ref_last_ans(sg_fr)) = new_node;
    SgFr_cons_ref_last_ans(sg_fr) = new_node;
    return;
  }
  ans_ref_ptr bucket_next = *bucket;
  if (V04_IS_HASH(bucket_next))
    return consumer_trie_check_insert_bucket_array((ans_ref_ptr *)bucket_next, sg_fr, t, (n_shifts + 1) PASS_REGS);
  else 
    return consumer_trie_check_insert_bucket_chain(curr_hash, bucket_next, sg_fr, t, n_shifts, 0 PASS_REGS);
}


static inline void consumer_trie_check_insert_first_chain(ans_ref_ptr chain_node, sg_fr_ptr sg_fr, ans_node_ptr t, int count_nodes USES_REGS) {
  if (V04_IS_EQUAL_ENTRY(chain_node, t))
    return;
  int cn = count_nodes + 1;
  ans_ref_ptr chain_next;
  chain_next = TrNode_next(chain_node);
  if (chain_next && !V04_IS_HASH(chain_next)) {
    return consumer_trie_check_insert_first_chain(chain_next, sg_fr, t, cn PASS_REGS); 
  }
  
  // chain_next is a hash pointer or the end of the chain
  if (chain_next == NULL) {
    if (cn == MAX_NODES_PER_BUCKET) {
      ans_ref_ptr *new_hash;
      ans_ref_ptr *bucket;
      V04_ALLOC_BUCKETS(new_hash, NULL, struct answer_ref_node);
      new_hash = (ans_ref_ptr *) V04_TAG(new_hash);
      V04_GET_HASH_BUCKET(bucket, new_hash, TrNode_entry(chain_node), NumberOfLowTagBits, struct answer_ref_node);
      V04_SET_HASH_BUCKET(bucket, chain_node, struct answer_ref_node);
      TrNode_next(chain_node) = (ans_ref_ptr) new_hash;
      consumer_trie_adjust_chain_nodes(new_hash, SgFr_cons_ref_ans(sg_fr), chain_node, (NumberOfLowTagBits - 1) PASS_REGS);
      SgFr_cons_ref_ans(sg_fr) = (ans_ref_ptr) new_hash;
      return consumer_trie_check_insert_bucket_array(new_hash, sg_fr, t, NumberOfLowTagBits PASS_REGS);
    } else {
      ans_ref_ptr new_node; 
      new_answer_ref_node(new_node, t, NULL, NULL);
      TrNode_next(chain_node) = new_node;
      TrNode_child(SgFr_cons_ref_last_ans(sg_fr)) = new_node;
      SgFr_cons_ref_last_ans(sg_fr) = new_node;
      return;
    }
  }
}

static inline void consumer_trie_check_insert_node(sg_fr_ptr sg_fr, ans_node_ptr ans_leaf_node USES_REGS) {
  ans_ref_ptr  ref_node;
  ref_node = SgFr_cons_ref_ans(sg_fr);
  if (ref_node == NULL) {
    new_answer_ref_node(ref_node, ans_leaf_node, NULL, NULL);
    SgFr_cons_ref_ans(sg_fr) = SgFr_cons_ref_first_ans(sg_fr) = SgFr_cons_ref_last_ans(sg_fr) = ref_node;
    return;
  }
  if (!V04_IS_HASH(ref_node))
    return consumer_trie_check_insert_first_chain(ref_node, sg_fr, ans_leaf_node, 0 PASS_REGS);
  return consumer_trie_check_insert_bucket_array((ans_ref_ptr *) ref_node, sg_fr, ans_leaf_node, NumberOfLowTagBits PASS_REGS);
}

#endif /* THREADS_FULL_SHARING_FTNA_3_AFS_V04_VERSION */


#ifdef THREADS_FULL_SHARING_FTNA_3
/* USES MODIFIED AFS_V04 CODE AND ALLOCATES THE HASHES WITHOUT THE PREV POINTER*/


static inline void consumer_trie_insert_bucket_chain(ans_ref_ptr *curr_hash, ans_ref_ptr chain_node, ans_ref_ptr adjust_node, long n_shifts, int count_nodes USES_REGS) { 
  ans_node_ptr t = TrNode_entry(adjust_node);
  int cn = count_nodes + 1;
  ans_ref_ptr chain_next;
  chain_next = TrNode_next(chain_node);
  if (chain_next)
    return consumer_trie_insert_bucket_chain(curr_hash, chain_next, adjust_node, n_shifts, cn PASS_REGS);  
  // end of chain
  if (cn == MAX_NODES_PER_BUCKET) {
    ans_ref_ptr *new_hash;
    ans_ref_ptr *bucket;
    FTNA_3_ALLOC_BUCKETS(new_hash, struct answer_ref_node);
    new_hash = (ans_ref_ptr *) V04_TAG(new_hash);
    V04_GET_HASH_BUCKET(bucket, new_hash, TrNode_entry(chain_node), n_shifts + 1, struct answer_ref_node);
    V04_SET_HASH_BUCKET(bucket, chain_node, struct answer_ref_node);
    V04_GET_HASH_BUCKET(bucket, curr_hash, t, n_shifts, struct answer_ref_node);
    consumer_trie_adjust_chain_nodes(new_hash, *bucket, chain_node, n_shifts PASS_REGS);
    V04_SET_HASH_BUCKET(bucket, new_hash, struct answer_ref_node);
    return consumer_trie_insert_bucket_array(new_hash, adjust_node, (n_shifts + 1) PASS_REGS);
  } else {
    TrNode_next(adjust_node) = NULL;
    TrNode_next(chain_node) = adjust_node;
    return;
  }
} 

static inline void consumer_trie_insert_bucket_array(ans_ref_ptr *curr_hash, ans_ref_ptr chain_node, long n_shifts USES_REGS) {
  ans_ref_ptr *bucket; 
  TrNode_next(chain_node) = NULL;
  V04_GET_HASH_BUCKET(bucket, curr_hash, TrNode_entry(chain_node), n_shifts, struct answer_ref_node);
  if (V04_IS_EMPTY_BUCKET(*bucket, NULL, struct answer_ref_node)) {
    V04_SET_HASH_BUCKET(bucket, chain_node, struct answer_ref_node);
    return;  
  }
  ans_ref_ptr bucket_next = *bucket;
  return consumer_trie_insert_bucket_chain(curr_hash, bucket_next, chain_node, n_shifts, 0 PASS_REGS);
}


static inline void consumer_trie_adjust_chain_nodes(ans_ref_ptr *new_hash, ans_ref_ptr chain_node, ans_ref_ptr last_node, long n_shifts USES_REGS) {
  if (chain_node == last_node)
    return;
  consumer_trie_adjust_chain_nodes(new_hash, TrNode_next(chain_node), last_node, n_shifts PASS_REGS);
  return consumer_trie_insert_bucket_array(new_hash, chain_node, (n_shifts + 1) PASS_REGS);
}

static inline boolean consumer_trie_check_insert_bucket_chain(ans_ref_ptr *curr_hash, ans_ref_ptr chain_node, sg_fr_ptr sg_fr, ans_node_ptr t, long n_shifts, int count_nodes USES_REGS) {
  if (V04_IS_EQUAL_ENTRY(chain_node, t))
    return false;  
  int cn = count_nodes + 1;
  ans_ref_ptr chain_next;
  chain_next = TrNode_next(chain_node);
  if (chain_next)
    return consumer_trie_check_insert_bucket_chain(curr_hash, chain_next, sg_fr, t, n_shifts, cn PASS_REGS);  
  
  // end of chain
  
  if (cn == MAX_NODES_PER_BUCKET) {
    ans_ref_ptr *new_hash;
    ans_ref_ptr *bucket;
    FTNA_3_ALLOC_BUCKETS(new_hash, struct answer_ref_node);
    new_hash = (ans_ref_ptr *) V04_TAG(new_hash);
    V04_GET_HASH_BUCKET(bucket, new_hash, TrNode_entry(chain_node), n_shifts + 1, struct answer_ref_node);
    V04_SET_HASH_BUCKET(bucket, chain_node, struct answer_ref_node);
    V04_GET_HASH_BUCKET(bucket, curr_hash, t, n_shifts, struct answer_ref_node);
    consumer_trie_adjust_chain_nodes(new_hash, *bucket, chain_node, n_shifts PASS_REGS);
    V04_SET_HASH_BUCKET(bucket, new_hash, struct answer_ref_node);
    return consumer_trie_check_insert_bucket_array(new_hash, sg_fr, t, (n_shifts + 1) PASS_REGS);
  } else {
    ans_ref_ptr new_node; 
    new_answer_ref_node(new_node, t, NULL, NULL);
    TrNode_next(chain_node) = new_node;
    TrNode_child(SgFr_cons_ref_last_ans(sg_fr)) = new_node;
    SgFr_cons_ref_last_ans(sg_fr) = new_node;
    return true;
  }  
}


static inline boolean consumer_trie_check_insert_bucket_array(ans_ref_ptr *curr_hash, sg_fr_ptr sg_fr, ans_node_ptr t, long n_shifts USES_REGS) {
  ans_ref_ptr *bucket; 
  V04_GET_HASH_BUCKET(bucket, curr_hash, t, n_shifts, struct answer_ref_node);
  if (V04_IS_EMPTY_BUCKET(*bucket, NULL, struct answer_ref_node)) {
    ans_ref_ptr new_node; 
    new_answer_ref_node(new_node, t, NULL, NULL);
    V04_SET_HASH_BUCKET(bucket, new_node, struct answer_ref_node);
    TrNode_child(SgFr_cons_ref_last_ans(sg_fr)) = new_node;
    SgFr_cons_ref_last_ans(sg_fr) = new_node;
    return true;
  }
  ans_ref_ptr bucket_next = *bucket;
  if (V04_IS_HASH(bucket_next))
    return consumer_trie_check_insert_bucket_array((ans_ref_ptr *)bucket_next, sg_fr, t, (n_shifts + 1) PASS_REGS);
  else 
    return consumer_trie_check_insert_bucket_chain(curr_hash, bucket_next, sg_fr, t, n_shifts, 0 PASS_REGS);
}


static inline boolean consumer_trie_check_insert_first_chain(ans_ref_ptr chain_node, sg_fr_ptr sg_fr, ans_node_ptr t, int count_nodes USES_REGS) {
  if (V04_IS_EQUAL_ENTRY(chain_node, t))
    return false;
  int cn = count_nodes + 1;
  ans_ref_ptr chain_next;
  chain_next = TrNode_next(chain_node);
  if (chain_next)
    return consumer_trie_check_insert_first_chain(chain_next, sg_fr, t, cn PASS_REGS); 
    
  if (cn == MAX_NODES_PER_BUCKET) {
    ans_ref_ptr *new_hash;
    ans_ref_ptr *bucket;
    FTNA_3_ALLOC_BUCKETS(new_hash, struct answer_ref_node);   
    new_hash = (ans_ref_ptr *) V04_TAG(new_hash);
    V04_GET_HASH_BUCKET(bucket, new_hash, TrNode_entry(chain_node), 0, struct answer_ref_node);
    V04_SET_HASH_BUCKET(bucket, chain_node, struct answer_ref_node);
    consumer_trie_adjust_chain_nodes(new_hash, SgFr_cons_ref_ans(sg_fr), chain_node, (-1) PASS_REGS);
    SgFr_cons_ref_ans(sg_fr) = (ans_ref_ptr) new_hash;
    return consumer_trie_check_insert_bucket_array(new_hash, sg_fr, t, 0 PASS_REGS);
  } else {
    ans_ref_ptr new_node; 
    new_answer_ref_node(new_node, t, NULL, NULL);
    TrNode_next(chain_node) = new_node;
    TrNode_child(SgFr_cons_ref_last_ans(sg_fr)) = new_node;
    SgFr_cons_ref_last_ans(sg_fr) = new_node;
    return true;
  } 
}

static inline boolean consumer_trie_check_insert_node(sg_fr_ptr sg_fr, ans_node_ptr ans_leaf_node USES_REGS) {
  ans_ref_ptr  ref_node;
  ref_node = SgFr_cons_ref_ans(sg_fr);
  if (ref_node == NULL) {
    new_answer_ref_node(ref_node, ans_leaf_node, NULL, NULL);
    SgFr_cons_ref_ans(sg_fr) = SgFr_cons_ref_first_ans(sg_fr) = SgFr_cons_ref_last_ans(sg_fr) = ref_node;
    return true;
  }
  
  if (!V04_IS_HASH(ref_node))
    return consumer_trie_check_insert_first_chain(ref_node, sg_fr, ans_leaf_node, 0 PASS_REGS);
  return consumer_trie_check_insert_bucket_array((ans_ref_ptr *) ref_node, sg_fr, ans_leaf_node, 0 PASS_REGS);
}


static inline void consumer_trie_free_structs(sg_fr_ptr sg_fr USES_REGS) {
  ans_ref_ptr ref_node;
  ref_node = SgFr_cons_ref_first_ans(sg_fr);
  if (ref_node) {
    do {
      ans_ref_ptr child = TrNode_child(ref_node);
      FREE_ANSWER_REF_NODE(ref_node);
      ref_node = child; 
    } while(ref_node);

    if (V04_IS_HASH(SgFr_cons_ref_ans(sg_fr))) {
      /* has at least one trie hash bucket array */
      consumer_trie_free_bucket_array((ans_ref_ptr *) SgFr_cons_ref_ans(sg_fr) PASS_REGS);
    }
    SgFr_cons_ref_ans(sg_fr) = SgFr_cons_ref_first_ans(sg_fr) = SgFr_cons_ref_last_ans(sg_fr) = NULL;    
  }
  return; 
}

static inline void consumer_trie_free_bucket_array(ans_ref_ptr *curr_hash USES_REGS) {

  ans_ref_ptr *bucket;
  ans_ref_ptr *untag_curr_hash;
  untag_curr_hash = (ans_ref_ptr*) V04_UNTAG(curr_hash);
  bucket = untag_curr_hash + BASE_HASH_BUCKETS;
  do {
    bucket--;
    if (*bucket && V04_IS_HASH(*bucket))
      consumer_trie_free_bucket_array((ans_ref_ptr*)*bucket PASS_REGS);
  } while (bucket != untag_curr_hash);

  FTNA_3_FREE_TRIE_HASH_BUCKETS(untag_curr_hash); 
  return;
}

#endif /* THREADS_FULL_SHARING_FTNA_3 */










