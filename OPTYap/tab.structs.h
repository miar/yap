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

typedef enum {false,true} boolean;


/************************************************************************
**                     Table Space Data Structures                     **
************************************************************************/

/**************************
**    subgoal no trie    **
**************************/
#ifdef THREADS_FULL_SHARING_NO_TRIE
typedef struct subgoal_no_trie_pos {
  struct subgoal_frame *subgoal_frame;
  Term entry;
} *subgoal_no_trie_pos;
#endif /* THREADS_FULL_SHARING_NO_TRIE */


/**************************
**      table_entry      **
**************************/

typedef struct table_entry {
#ifdef THREADS
  lockvar lock;  // needed for subgoal_trie and hash_chain fields
#endif
  struct pred_entry *pred_entry;
  Atom pred_atom;
  int pred_arity;
  short pred_flags;
  short execution_mode;  /* combines yap_flags with pred_flags */
#ifdef MODE_DIRECTED_TABLING
  int* mode_directed_array;
#endif /* MODE_DIRECTED_TABLING */
#ifdef THREADS_FULL_SHARING_NO_TRIE
  int* dimension_array;
  subgoal_no_trie_pos subgoal_no_trie;
  //#else /* THREADS_FULL_SHARING_NO_TRIE */ TO INCLUDE LATER
#ifdef THREADS_NO_SHARING
  struct subgoal_trie_node *subgoal_trie[THREADS_NUM_BUCKETS];
#else
  struct subgoal_trie_node *subgoal_trie;
#endif /* THREADS_NO_SHARING */
  struct subgoal_trie_hash *hash_chain;
#endif /* THREADS_FULL_SHARING_NO_TRIE */
  struct table_entry *next;
} *tab_ent_ptr;

#define TabEnt_lock(X)            ((X)->lock)
#define TabEnt_pe(X)              ((X)->pred_entry)
#define TabEnt_atom(X)            ((X)->pred_atom)
#define TabEnt_arity(X)           ((X)->pred_arity)
#define TabEnt_flags(X)           ((X)->pred_flags)
#define TabEnt_mode(X)            ((X)->execution_mode)
#define TabEnt_mode_directed(X)   ((X)->mode_directed_array)
#define TabEnt_dimension_array(X) ((X)->dimension_array)
#define TabEnt_subgoal_no_trie(X) ((X)->subgoal_no_trie)
#define TabEnt_subgoal_trie(X)    ((X)->subgoal_trie)
#define TabEnt_hash_chain(X)      ((X)->hash_chain)
#define TabEnt_next(X)            ((X)->next)


/***********************************************************************
**      subgoal_trie_node, answer_trie_node and global_trie_node      **
***********************************************************************/

typedef struct subgoal_trie_node {
  Term entry;
  struct subgoal_trie_node *parent;
  struct subgoal_trie_node *child;
  struct subgoal_trie_node *next;
#if defined(SUBGOAL_TRIE_LOCK_USING_NODE_FIELD) && !defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL)
  lockvar lock;
#endif 
} *sg_node_ptr;

typedef struct answer_trie_node {
  OPCODE trie_instruction;  /* u.opc */
#ifdef YAPOR
  int or_arg;               /* u.Otapl.or_arg */
#endif /* YAPOR */
  Term entry;
  struct answer_trie_node *parent;
  struct answer_trie_node *child;
  struct answer_trie_node *next;
#if defined(ANSWER_TRIE_LOCK_USING_NODE_FIELD) && !defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL)
  lockvar lock;
#endif 
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
  struct answer_trie_node *intra_invalid_next;
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */
} *ans_node_ptr;


typedef struct global_trie_node {
  Term entry;
  struct global_trie_node *parent;
  struct global_trie_node *child;
  struct global_trie_node *next;
#ifdef GLOBAL_TRIE_LOCK_USING_NODE_FIELD
  lockvar lock;
#endif /* GLOBAL_TRIE_LOCK_USING_NODE_FIELD */
} *gt_node_ptr;

#define TrNode_instr(X)              ((X)->trie_instruction)
#define TrNode_or_arg(X)             ((X)->or_arg)
#define TrNode_entry(X)              ((X)->entry)
#define TrNode_parent(X)             ((X)->parent)
#define TrNode_child(X)              ((X)->child)
#define TrNode_sg_fr(X)              ((X)->child)
#define TrNode_sg_ent(X)             ((X)->child)
#define TrNode_next(X)               ((X)->next)
#define TrNode_lock(X)               ((X)->lock)
#define TrNode_expansion(X)          ((X)->expansion)
#define TrNode_intra_invalid_next(X) ((X)->intra_invalid_next)


/******************************
**      answer_ref_node      **
******************************/

#ifdef THREADS_FULL_SHARING
/* used in FTNA_3 */
typedef struct answer_ref_node {
  struct answer_trie_node *entry;  
  struct answer_ref_node *child; 
  struct answer_ref_node *next;
} *ans_ref_ptr;
#endif /* THREADS_FULL_SHARING */

#define RefNode_answer(X)    ((X)->entry)
#define RefNode_next(X)      ((X)->next)
#define RefNode_child(X)     ((X)->child)


/***********************************************************************
**      subgoal_trie_hash, answer_trie_hash and global_trie_hash      **
***********************************************************************/

#define HashBkts_next(X)              ((X)->next)
#define HashBkts_number_of_buckets(X) ((X)->number_of_buckets)
#define HashBkts_buckets(X)           ((X)->buckets)

#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
typedef struct subgoal_trie_hash_buckets {
  struct subgoal_trie_hash_buckets *next;
  int number_of_buckets;
  struct subgoal_trie_node **buckets;
} *sg_hash_bkts_ptr;

/* subgoal_trie_hash */
#define SgHash_num_buckets(X)        (HashBkts_number_of_buckets(((X)->hash_bkts)))
#define SgHash_buckets(X)            (HashBkts_buckets(((X)->hash_bkts)))
#define SgHash_hash_bkts(X)          ((X)->hash_bkts)
#define SgHash_old_hash_bkts(X)      ((X)->old_hash_bkts)
#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL */

typedef struct subgoal_trie_hash {
  /* the first field is used for compatibility **
  ** with the subgoal_trie_node data structure */
  Term mark;    
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
  sg_hash_bkts_ptr old_hash_bkts;
  sg_hash_bkts_ptr hash_bkts;
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03  
  struct subgoal_trie_node expansion_node;
#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03 */
#else
  int number_of_buckets;
  struct subgoal_trie_node **buckets;
#endif
  int number_of_nodes;
  struct subgoal_trie_hash *next;
} *sg_hash_ptr;


#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
typedef struct answer_trie_hash_buckets {
  struct answer_trie_hash_buckets *next;
  int number_of_buckets;
  struct answer_trie_node **buckets;
} *ans_hash_bkts_ptr;

/* answer_trie_hash */
#define AnsHash_num_buckets(X)        (HashBkts_number_of_buckets(((X)->hash_bkts)))
#define AnsHash_buckets(X)            (HashBkts_buckets(((X)->hash_bkts)))
#define AnsHash_hash_bkts(X)          ((X)->hash_bkts)
#define AnsHash_old_hash_bkts(X)      ((X)->old_hash_bkts)
#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL */

typedef struct answer_trie_hash {
  /* the first field is used for compatibility **
  ** with the answer_trie_node data structure  */
  OPCODE mark; // why is OPCODE ?? AND NOT Term
  int number_of_buckets;
  int number_of_nodes;
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
  ans_hash_bkts_ptr old_hash_bkts;
  ans_hash_bkts_ptr hash_bkts;
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02
  struct answer_trie_node *expansion_nodes;
  struct answer_trie_node *unused_expansion_nodes;
#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02 */
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03
  struct answer_trie_node expansion_node;
#endif  /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03 */
#else
  struct answer_trie_node **buckets;
#endif
#ifdef MODE_DIRECTED_TABLING
  struct answer_trie_hash *previous;
#endif /*MODE_DIRECTED_TABLING*/
  struct answer_trie_hash *next;
} *ans_hash_ptr;

typedef struct global_trie_hash {
  /* the first field is used for compatibility **
  ** with the global_trie_node data structure  */
  Term mark;
  int number_of_buckets;
  struct global_trie_node **buckets;
  int number_of_nodes;
#ifdef USE_PAGES_MALLOC
  struct global_trie_hash *next;
#endif /* USE_PAGES_MALLOC */
} *gt_hash_ptr;

#define Hash_mark(X)             ((X)->mark)
#define Hash_num_buckets(X)      ((X)->number_of_buckets)
#define Hash_buckets(X)          ((X)->buckets)
#define Hash_num_nodes(X)        ((X)->number_of_nodes)
#define Hash_previous(X)         ((X)->previous)
#define Hash_next(X)             ((X)->next)

#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03
#define Hash_sg_exp_node(X)          ((sg_node_ptr) &(((struct subgoal_trie_hash *) (X))->expansion_node))
#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V03 */

#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02
#define Hash_exp_nodes(X)        ((X)->expansion_nodes)
#define Hash_unused_exp_nodes(X) ((X)->unused_expansion_nodes)
#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02 */
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03
#define Hash_ans_exp_node(X)          ((ans_node_ptr) &(((struct answer_trie_hash *) (X))->expansion_node))
#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V03 */


/************************************************************************
**                      Execution Data Structures                      **
************************************************************************/

/****************************
**      choice points      **
****************************/

struct generator_choicept {
  struct choicept cp;
  struct dependency_frame *cp_dep_fr;  /* always NULL if batched scheduling */
  struct subgoal_frame *cp_sg_fr;
#ifdef LOW_LEVEL_TRACER
  struct pred_entry *cp_pred_entry;
#endif /* LOW_LEVEL_TRACER */
};

#ifdef DETERMINISTIC_TABLING
struct deterministic_generator_choicept {
  struct deterministic_choicept cp;
  struct subgoal_frame *cp_sg_fr;
#ifdef LOW_LEVEL_TRACER
  struct pred_entry *cp_pred_entry;
#endif /* LOW_LEVEL_TRACER */
};
#endif /* DETERMINISTIC_TABLING */

struct consumer_choicept {
  struct choicept cp;
  struct dependency_frame *cp_dep_fr;
#ifdef LOW_LEVEL_TRACER
  struct pred_entry *cp_pred_entry;
#endif /* LOW_LEVEL_TRACER */
};

struct loader_choicept {
  struct choicept cp;
  struct answer_trie_node *cp_last_answer;
#ifdef LOW_LEVEL_TRACER
  struct pred_entry *cp_pred_entry;
#endif /* LOW_LEVEL_TRACER */
};



/*********************************
**      subgoal_state_flag      **
*********************************/

typedef enum {          /* do not change order !!! */
  incomplete      = 0,  /* INCOMPLETE_TABLING */
  ready           = 1,
  evaluating      = 2,
  complete        = 3,
  complete_in_use = 4,  /* LIMIT_TABLING */
  compiled        = 5,
  compiled_in_use = 6   /* LIMIT_TABLING */
} subgoal_state_flag;



/****************************
**      subgoal_entry      **
****************************/

typedef struct subgoal_entry {

#ifdef THREADS_SUBGOAL_COMPLETION_WAIT
  lockvar lock_completion_wait;
  pthread_cond_t completion_wait;
#endif /* THREADS_SUBGOAL_COMPLETION_WAIT */

#if defined(YAPOR) || defined(THREADS_FULL_SHARING) 
  lockvar lock;
#endif /* YAPOR || THREADS_FULL_SHARING */

#ifdef EXTRA_STATISTICS_CHOICE_POINTS
  int query_number;
#endif /* EXTRA_STATISTICS_CHOICE_POINTS */
  yamop *code_of_subgoal;
  struct answer_trie_hash *hash_chain;
#if defined(THREADS_FULL_SHARING) && defined(MODE_DIRECTED_TABLING)
  struct answer_trie_hash *old_hash_chain;
#endif
  struct answer_trie_node *answer_trie;
  struct answer_trie_node *first_answer;
#ifndef THREADS_FULL_SHARING_FTNA 
  struct answer_trie_node *last_answer;
#endif /* THREADS_FULL_SHARING_FTNA */

#ifdef MODE_DIRECTED_TABLING
  int* mode_directed_array;
  struct answer_trie_node *invalid_chain;       /* leaf invalid chain */
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
  struct answer_trie_node *intra_invalid_chain;  
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */
#endif /* MODE_DIRECTED_TABLING */
#ifdef LIMIT_TABLING
  struct subgoal_frame *previous;
#endif /* LIMIT_TABLING */
#ifdef YAPOR
  struct or_frame *top_or_frame_on_generator_branch;
#endif /* YAPOR */
#if defined(YAPOR)
  int generator_worker;
#endif /* YAPOR */
#if defined(THREADS_FULL_SHARING)
  subgoal_state_flag state_flag;
  int active_workers;
#ifdef THREADS_SUBGOAL_FRAME_BY_WID
  struct subgoal_frame *subgoal_frame;
#else
  struct subgoal_frame *subgoal_frame[THREADS_NUM_BUCKETS];
#endif /* THREADS_SUBGOAL_FRAME_BY_WID */
#ifdef USE_PAGES_MALLOC
  struct subgoal_entry * next; 
#endif /*USE_PAGES_MALLOC */
#endif /* THREADS_FULL_SHARING */
}* sg_ent_ptr;

#define SgEnt_lock(X)                    ((X)->lock)
#define SgEnt_query_number(X)            ((X)->query_number)
#define SgEnt_code(X)                    ((X)->code_of_subgoal)
#define SgEnt_tab_ent(X)                 (((X)->code_of_subgoal)->u.Otapl.te)
#define SgEnt_arity(X)                   (((X)->code_of_subgoal)->u.Otapl.s)
#define SgEnt_hash_chain(X)              ((X)->hash_chain)
#define SgEnt_old_hash_chain(X)          ((X)->old_hash_chain)
#define SgEnt_answer_trie(X)             ((X)->answer_trie)
#define SgEnt_first_answer(X)            ((X)->first_answer)
#ifndef THREADS_FULL_SHARING_FTNA 
#define SgEnt_last_answer(X)             ((X)->last_answer)
#endif /* THREADS_FULL_SHARING_FTNA */
#define SgEnt_mode_directed(X)           ((X)->mode_directed_array)
#define SgEnt_invalid_chain(X)           ((X)->invalid_chain)
#define SgEnt_intra_invalid_chain(X)     ((X)->intra_invalid_chain)
#define SgEnt_previous(X)                ((X)->previous)
#define SgEnt_gen_top_or_fr(X)           ((X)->top_or_frame_on_generator_branch)
#define SgEnt_gen_worker(X)              ((X)->generator_worker)
#define SgEnt_sg_ent_state(X)            ((X)->state_flag)
#define SgEnt_active_workers(X)          ((X)->active_workers)
#define SgEnt_sg_fr(X)                   ((X)->subgoal_frame)
#define SgEnt_next(X)                    ((X)->next)


/****************************
**      subgoal_frame      **
****************************/

typedef struct subgoal_frame {
#ifdef THREADS_SUBGOAL_FRAME_BY_WID
  struct subgoal_frame *next_wid;
#endif /* THREADS_SUBGOAL_FRAME_BY_WID */
#if defined(THREADS_FULL_SHARING)
  struct subgoal_entry *subgoal_entry;
#ifdef THREADS_FULL_SHARING
#ifdef THREADS_FULL_SHARING_FTNA 
  struct answer_trie_node *last_answer;
#endif /* THREADS_FULL_SHARING_FTNA */
#ifdef THREADS_FULL_SHARING_FTNA_3
  struct answer_ref_node *consumer_ref_answer;
  struct answer_ref_node *consumer_ref_first_answer;
  struct answer_ref_node *consumer_ref_last_answer;
#endif /* THREADS_FULL_SHARING_FTNA_3 */
#endif /* THREADS_FULL_SHARING */
#else
  struct subgoal_entry subgoal_entry;
#endif /* THREADS_FULL_SHARING */
  subgoal_state_flag state_flag;
  choiceptr generator_choice_point;
#ifdef INCOMPLETE_TABLING
#ifdef THREADS_FULL_SHARING_FTNA_3
  struct answer_ref_node  *try_answer;
#else
  struct answer_trie_node *try_answer;
#endif /* THREADS_FULL_SHARING_FTNA_3 */
#endif /* INCOMPLETE_TABLING */  
  struct subgoal_frame *next;
#if defined(THREADS_SUBGOAL_SHARING) || defined(THREADS_FULL_SHARING)
  struct subgoal_frame *next_complete;
#ifdef THREADS_LOCAL_SG_FR_HASH_BUCKETS
  struct subgoal_trie_node *sg_leaf_node;
  struct subgoal_frame *next_on_hash;
#else /* !THREADS_LOCAL_SG_FR_HASH_BUCKETS */

#ifdef THREADS_SUBGOAL_FRAME_BY_WID
#ifdef THREADS_SUBGOAL_SHARING
  struct subgoal_trie_node *sg_leaf_node;
#endif /* THREADS_SUBGOAL_SHARING */
  int wid;
#else /* !THREADS_SUBGOAL_FRAME_BY_WID */
#ifdef THREADS_SUBGOAL_SHARING_WITH_PAGES_SG_FR_ARRAY
  struct sg_fr_bkt_array **subgoal_frame_array;
#else /* !THREADS_SUBGOAL_SHARING_WITH_PAGES_SG_FR_ARRAY */
  void **subgoal_frame_array;
#endif /* THREADS_SUBGOAL_SHARING_WITH_PAGES_SG_FR_ARRAY */
#endif /* THREADS_SUBGOAL_FRAME_BY_WID */
#endif /* THREADS_LOCAL_SG_FR_HASH_BUCKETS */
#endif /* THREADS_SUBGOAL_SHARING || THREADS_FULL_SHARING */
} *sg_fr_ptr;

/* subgoal_entry fields */
#if defined(THREADS_FULL_SHARING)
#define SUBGOAL_ENTRY(X)                SgFr_sg_ent(X)->
#else
#define SUBGOAL_ENTRY(X)                (X)->subgoal_entry.
#endif /* THREADS_FULL_SHARING */

#define SgFr_query_number(X)            (SUBGOAL_ENTRY(X) query_number)
#define SgFr_lock(X)                    (SUBGOAL_ENTRY(X) lock)
#define SgFr_code(X)                    (SUBGOAL_ENTRY(X) code_of_subgoal)
#define SgFr_tab_ent(X)                 ((SUBGOAL_ENTRY(X) code_of_subgoal)->u.Otapl.te)
#define SgFr_arity(X)                   ((SUBGOAL_ENTRY(X) code_of_subgoal)->u.Otapl.s)
#define SgFr_hash_chain(X)              (SUBGOAL_ENTRY(X) hash_chain)
#define SgFr_old_hash_chain(X)          (SUBGOAL_ENTRY(X) old_hash_chain)
#define SgFr_answer_trie(X)             (SUBGOAL_ENTRY(X) answer_trie)
#define SgFr_first_answer(X)            (SUBGOAL_ENTRY(X) first_answer)
#ifdef THREADS_FULL_SHARING_FTNA
#define SgFr_last_answer(X)             ((X)->last_answer)
#else
#define SgFr_last_answer(X)             (SUBGOAL_ENTRY(X) last_answer)
#endif /* THREADS_FULL_SHARING_FTNA */
#define SgFr_mode_directed(X)           (SUBGOAL_ENTRY(X) mode_directed_array)
#define SgFr_invalid_chain(X)           (SUBGOAL_ENTRY(X) invalid_chain) 
#define SgFr_intra_invalid_chain(X)     (SUBGOAL_ENTRY(X) intra_invalid_chain)
#define SgFr_previous(X)                (SUBGOAL_ENTRY(X) previous)
#define SgFr_gen_top_or_fr(X)           (SUBGOAL_ENTRY(X) top_or_frame_on_generator_branch)
#define SgFr_gen_worker(X)              (SUBGOAL_ENTRY(X) generator_worker)
#define SgFr_sg_ent_state(X)            (SUBGOAL_ENTRY(X) state_flag)
#define SgFr_active_workers(X)          (SUBGOAL_ENTRY(X) active_workers)
#define SgFr_lock_comp_wait(X)          (SUBGOAL_ENTRY(X) lock_completion_wait)
#define SgFr_comp_wait(X)               (SUBGOAL_ENTRY(X) completion_wait)

/* subgoal_frame fields */
#define SgFr_sg_fr_array(X)             ((X)->subgoal_frame_array)
#define SgFr_sg_ent(X)                  ((X)->subgoal_entry)
#define SgFr_state(X)                   ((X)->state_flag)
#define SgFr_try_answer(X)              ((X)->try_answer)
#define SgFr_gen_cp(X)                  ((X)->generator_choice_point)
#define SgFr_next(X)                    ((X)->next)
#define SgFr_next_complete(X)           ((X)->next_complete)
#define SgFr_sg_leaf_node(X)            ((X)->sg_leaf_node)
#define SgFr_next_on_hash(X)            ((X)->next_on_hash)
#define SgFr_wid(X)                     ((X)->wid)
#define SgFr_next_wid(X)                ((X)->next_wid)

#define SgFr_cons_ref_ans(X)            ((X)->consumer_ref_answer)
#define SgFr_cons_ref_first_ans(X)      ((X)->consumer_ref_first_answer) 
#define SgFr_cons_ref_last_ans(X)       ((X)->consumer_ref_last_answer) 


/**********************************************************************************************************

  SgFr_lock:                    spin-lock to modify the frame fields.
  SgFr_code                     initial instruction of the subgoal's compiled code.
  SgFr_tab_ent                  a pointer to the corresponding table entry.
  SgFr_arity                    the arity of the subgoal.
  SgFr_hash_chain:              a pointer to the first answer_trie_hash struct.
  SgFr_answer_trie:             a pointer to the top answer trie node.
  SgFr_first_answer:            a pointer to the leaf answer trie node of the first answer.
  SgFr_last_answer:             a pointer to the leaf answer trie node of the last answer.
  SgFr_mode_directed:           a pointer to the mode directed array.
  SgFr_invalid_chain:           a pointer to the first invalid leaf node when using mode directed tabling.
  SgFr_try_answer:              a pointer to the leaf answer trie node of the last tried answer.
                                It is used when a subgoal was not completed during the previous evaluation.
                                Not completed subgoals start by trying the answers already found.
  SgFr_previous:                a pointer to the previous subgoal frame on the chain.
  SgFr_gen_top_or_fr:           a pointer to the top or-frame in the generator choice point branch. 
                                When the generator choice point is shared the pointer is updated 
                                to its or-frame. It is used to find the direct dependency node for 
                                consumer nodes in other workers branches.
  SgFr_gen_worker:              the id of the worker that had allocated the frame.
  SgFr_sg_ent_state:            a flag that indicates the subgoal entry state.
  SgFr_active_workers:          the number of workers evaluating the subgoal.
  SgFr_sg_ent:                  a pointer to the corresponding subgoal entry.
  SgFr_state:                   a flag that indicates the subgoal frame state.
  SgFr_gen_cp:                  a pointer to the correspondent generator choice point.
  SgFr_next:                    a pointer to the next subgoal frame on the chain.

**********************************************************************************************************/



/*******************************
**      dependency_frame      **
*******************************/

typedef struct dependency_frame {
#ifdef YAPOR
  lockvar lock;
  int leader_dependency_is_on_stack;
  struct or_frame *top_or_frame;
#ifdef TIMESTAMP_CHECK
  long timestamp;
#endif /* TIMESTAMP_CHECK */
#endif /* YAPOR */
  choiceptr backchain_choice_point;
  choiceptr leader_choice_point;
  choiceptr consumer_choice_point;
#ifdef THREADS_FULL_SHARING_FTNA_3
  struct answer_ref_node *last_consumed_answer;
#else
  struct answer_trie_node *last_consumed_answer;
#endif /* THREADS_FULL_SHARING_FTNA_3 */
#if defined(TIMESTAMP_MODE_DIRECTED_TABLING) || defined(STUDY_TIMESTAMP_MDT)
  Term last_consumed_entry;
#endif /* TIMESTAMP_MODE_DIRECTED_TABLING */
  struct dependency_frame *next;
} *dep_fr_ptr;

#define DepFr_lock(X)                    ((X)->lock)
#define DepFr_leader_dep_is_on_stack(X)  ((X)->leader_dependency_is_on_stack)
#define DepFr_top_or_fr(X)               ((X)->top_or_frame)
#define DepFr_timestamp(X)               ((X)->timestamp)
#define DepFr_external(X)                ((X)->generator_is_external)
#define DepFr_backchain_cp(X)            ((X)->backchain_choice_point)
#define DepFr_leader_cp(X)               ((X)->leader_choice_point)
#define DepFr_cons_cp(X)                 ((X)->consumer_choice_point)
#define DepFr_last_answer(X)             ((X)->last_consumed_answer)
#define DepFr_last_term(X)               ((X)->last_consumed_entry)
#define DepFr_next(X)                    ((X)->next)

/*********************************************************************************************************

  DepFr_lock:                   lock variable to modify the frame fields.
  DepFr_leader_dep_is_on_stack: the generator choice point for the correspondent consumer choice point 
                                is on the worker's stack (FALSE/TRUE).
  DepFr_top_or_fr:              a pointer to the top or-frame in the consumer choice point branch. 
                                When the consumer choice point is shared the pointer is updated to 
                                its or-frame. It is used to update the LOCAL_top_or_fr when a worker 
                                backtracks through answers.
  DepFr_timestamp:              a timestamp used to optimize the search for suspension frames to be 
                                resumed.
  DepFr_external:               the generator choice point is external to the current thread (FALSE/TRUE).
  DepFr_backchain_cp:           a pointer to the nearest choice point with untried alternatives.
                                It is used to efficiently return (backtrack) to the leader node where 
                                we perform the last backtracking through answers operation.
  DepFr_leader_cp:              a pointer to the leader choice point.
  DepFr_cons_cp:                a pointer to the correspondent consumer choice point.
  DepFr_last_answer:            a pointer to the last consumed answer.
  DepFr_next:                   a pointer to the next dependency frame on the chain.  

*********************************************************************************************************/



/*******************************
**      suspension_frame      **
*******************************/

#ifdef YAPOR
typedef struct suspension_frame {
  struct or_frame *top_or_frame_on_stack;
  struct dependency_frame *top_dependency_frame;
  struct subgoal_frame *top_subgoal_frame;
  struct suspended_block {
    void *resume_register;
    void *block_start;
    long block_size;
  } global_block, local_block, trail_block;
  struct suspension_frame *next;
} *susp_fr_ptr;
#endif /* YAPOR */

#define SuspFr_top_or_fr_on_stack(X)  ((X)->top_or_frame_on_stack)
#define SuspFr_top_dep_fr(X)          ((X)->top_dependency_frame)
#define SuspFr_top_sg_fr(X)           ((X)->top_subgoal_frame)
#define SuspFr_global_reg(X)          ((X)->global_block.resume_register)
#define SuspFr_global_start(X)        ((X)->global_block.block_start)
#define SuspFr_global_size(X)         ((X)->global_block.block_size)
#define SuspFr_local_reg(X)           ((X)->local_block.resume_register)
#define SuspFr_local_start(X)         ((X)->local_block.block_start)
#define SuspFr_local_size(X)          ((X)->local_block.block_size)
#define SuspFr_trail_reg(X)           ((X)->trail_block.resume_register)
#define SuspFr_trail_start(X)         ((X)->trail_block.block_start)
#define SuspFr_trail_size(X)          ((X)->trail_block.block_size)
#define SuspFr_next(X)                ((X)->next)


#ifdef THREADS_SUBGOAL_SHARING_WITH_PAGES_SG_FR_ARRAY
typedef struct sg_fr_bkt_array {
void * sg_fr_array[THREADS_NUM_BUCKETS];
  struct sg_fr_bkt_array *next;
} *sg_fr_bkt_array_ptr;
#endif


#ifdef THREADS_LOCAL_SG_FR_HASH_BUCKETS
typedef struct subgoal_frame_hash_buckets {
  int number_of_buckets;
  struct subgoal_frame **buckets; 
} *sg_fr_hash_bkts_ptr;

#define SgFrHashBkts_number_of_buckets(X) ((X)->number_of_buckets)
#define SgFrHashBkts_buckets(X)           ((X)->buckets)

#endif /* THREADS_LOCAL_SG_FR_HASH_BUCKETS */

#ifdef THREADS_SUBGOAL_FRAME_BY_WID_
/* NOT IMPLEMENTED YET */
typedef struct subgoal_frame_hash {
  int number_of_nodes;
  struct subgoal_frame *sg_fr_complete;
  sg_fr_hash_bkts_ptr old_hash_bkts;
  sg_fr_hash_bkts_ptr hash_bkts;
  struct subgoal_frame expansion_node;
} *sg_fr_hash_ptr;

typedef struct subgoal_frame_hash_buckets {
  int number_of_buckets;
  struct subgoal_frame **buckets;
  struct subgoal_fr_hash_buckets *next;
} *sg_fr_hash_bkts_ptr;

/* answer_trie_hash */
#define SgFrHash_num_buckets(X)        (HashBkts_number_of_buckets(((X)->hash_bkts)))
#define SgFrHash_buckets(X)            (HashBkts_buckets(((X)->hash_bkts)))
#define SgFrHash_hash_bkts(X)          ((X)->hash_bkts)
#define SgFrHash_old_hash_bkts(X)      ((X)->old_hash_bkts)

#endif /* THREADS_SUBGOAL_FRAME_BY_WID_ */

#if defined(SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04) || defined(ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04) || defined(THREADS_FULL_SHARING_FTNA_3) || defined(LFHT_LOCAL_THREAD_BUFFER_FOR_BUCKET_ARRAYS)

#define BASE_HASH_BUCKETS_2               8   // MUST BE THE SAME AS BASE_HASH_BUCKETS

union trie_hash_buckets {
  void *next;
  void *hash_buckets[BASE_HASH_BUCKETS_2 + 1];
};

union consumer_trie_hash_buckets {
  void *next;
  void *hash_buckets[BASE_HASH_BUCKETS_2];
};

#endif




