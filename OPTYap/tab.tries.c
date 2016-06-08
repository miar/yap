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

#include "Yap.h"
#ifdef TABLING
#include "Yatom.h"
#include "YapHeap.h"
#include "tab.macros.h"

#ifdef _______________________MIG_HERE_________________
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04
static inline void subgoal_trie_insert_bucket_chain(sg_node_ptr *, sg_node_ptr, sg_node_ptr, long, int USES_REGS);
static inline void subgoal_trie_insert_bucket_array(sg_node_ptr *, sg_node_ptr, long USES_REGS);
static inline void subgoal_trie_adjust_chain_nodes(sg_node_ptr *, sg_node_ptr, sg_node_ptr, long USES_REGS);
static inline sg_node_ptr subgoal_trie_check_insert_bucket_chain(sg_node_ptr *, sg_node_ptr, sg_node_ptr, Term, long, int USES_REGS);
static inline sg_node_ptr subgoal_trie_check_insert_bucket_array(sg_node_ptr *, sg_node_ptr, Term, long USES_REGS);
static inline sg_node_ptr subgoal_trie_check_insert_first_chain(sg_node_ptr, sg_node_ptr, Term, int USES_REGS);
#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 */
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04
static inline void answer_trie_insert_bucket_chain(ans_node_ptr *, ans_node_ptr, ans_node_ptr, long, int USES_REGS);
static inline void answer_trie_insert_bucket_array(ans_node_ptr *, ans_node_ptr, long USES_REGS);
static inline void answer_trie_adjust_chain_nodes(ans_node_ptr *, ans_node_ptr, ans_node_ptr, long USES_REGS);
static inline ans_node_ptr answer_trie_check_insert_bucket_chain(ans_node_ptr *, ans_node_ptr, ans_node_ptr, Term, int, long, int USES_REGS);
static inline ans_node_ptr answer_trie_check_insert_bucket_array(ans_node_ptr *, ans_node_ptr, Term, int, long USES_REGS);
static inline ans_node_ptr answer_trie_check_insert_first_chain(ans_node_ptr, ans_node_ptr, Term, int, int USES_REGS);
#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 */
#endif /* _______________________MIG_HERE_________________ */
static inline sg_node_ptr subgoal_trie_check_insert_entry(tab_ent_ptr, sg_node_ptr, Term USES_REGS);
static inline sg_node_ptr subgoal_trie_check_insert_gt_entry(tab_ent_ptr, sg_node_ptr, Term USES_REGS);
static inline ans_node_ptr answer_trie_check_insert_entry(sg_fr_ptr, ans_node_ptr, Term, int USES_REGS);
static inline ans_node_ptr answer_trie_check_insert_gt_entry(sg_fr_ptr, ans_node_ptr, Term, int USES_REGS);
static inline gt_node_ptr global_trie_check_insert_entry(gt_node_ptr, Term USES_REGS);
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
static inline gt_node_ptr global_trie_check_insert_gt_entry(gt_node_ptr, Term USES_REGS);
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */ 
static inline sg_node_ptr subgoal_search_loop(tab_ent_ptr, sg_node_ptr, Term, int *, CELL ** USES_REGS);
static inline sg_node_ptr subgoal_search_terms_loop(tab_ent_ptr, sg_node_ptr, Term, int *, CELL ** USES_REGS);
static inline ans_node_ptr answer_search_loop(sg_fr_ptr, ans_node_ptr, Term, int * USES_REGS);
static inline ans_node_ptr answer_search_terms_loop(sg_fr_ptr, ans_node_ptr, Term, int * USES_REGS);
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
static inline gt_node_ptr subgoal_search_global_trie_terms_loop(Term, int *, CELL **, CELL * USES_REGS);
static inline gt_node_ptr answer_search_global_trie_terms_loop(Term, int *, CELL * USES_REGS);
#else
static inline gt_node_ptr subgoal_search_global_trie_loop(Term, int *, CELL ** USES_REGS);
static inline gt_node_ptr answer_search_global_trie_loop(Term, int * USES_REGS);
#endif /* GLOBAL_TRIE_MODE */ 
static inline CELL *load_answer_loop(ans_node_ptr USES_REGS);
static inline CELL *load_substitution_loop(gt_node_ptr, int *, CELL * USES_REGS);
static inline CELL *exec_substitution_loop(gt_node_ptr, CELL **, CELL * USES_REGS);
#ifdef MODE_DIRECTED_TABLING
#ifdef THREADS_FULL_SHARING_MODE_DIRECTED_V02
static inline ans_node_ptr answer_search_min_max(sg_fr_ptr, ans_node_ptr, Term, int USES_REGS);
#else
static inline ans_node_ptr answer_search_min_max(ans_node_ptr, Term, int USES_REGS);
static void invalidate_answer_trie(ans_node_ptr, sg_fr_ptr, int USES_REGS);
#endif /* THREADS_FULL_SHARING_MODE_DIRECTED_V02 */
#endif /* MODE_DIRECTED_TABLING */

#ifdef YAPOR
#ifdef TABLING_INNER_CUTS
static int update_answer_trie_branch(ans_node_ptr, ans_node_ptr);
#else /* YAPOR && ! TABLING_INNER_CUTS */
static int update_answer_trie_branch(ans_node_ptr);
#endif
#else /* ! YAPOR */
static void update_answer_trie_branch(ans_node_ptr, int);
#endif
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
static void free_global_trie_branch(gt_node_ptr, int USES_REGS);
#else
static void free_global_trie_branch(gt_node_ptr USES_REGS);
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
#ifdef EXTRA_STATISTICS 
static void traverse_subgoal_trie(long, sg_node_ptr, char *, int, int *, int, int USES_REGS);
static void traverse_answer_trie(long, ans_node_ptr, char *, int, int *, int, int, int USES_REGS);
#else
static void traverse_subgoal_trie(sg_node_ptr, char *, int, int *, int, int USES_REGS);
static void traverse_answer_trie(ans_node_ptr, char *, int, int *, int, int, int USES_REGS);
#endif
static void traverse_global_trie(gt_node_ptr, char *, int, int *, int, int USES_REGS);
static void traverse_global_trie_for_term(gt_node_ptr, char *, int *, int *, int *, int USES_REGS);
static inline void traverse_trie_node(Term, char *, int *, int *, int *, int USES_REGS);
static inline void traverse_update_arity(char *, int *, int *);



/*******************************
**      Structs & Macros      **
*******************************/

static struct trie_statistics{

  IOSTREAM *out;
  int show;
  long subgoals;
  long subgoals_incomplete;
  long subgoal_trie_nodes;
  long answers;
#ifdef TABLING_INNER_CUTS
  long answers_pruned;
#endif /* TABLING_INNER_CUTS */
  long answers_true;
  long answers_no;
  long answer_trie_nodes;
  long global_trie_terms;
  long global_trie_nodes;
  long global_trie_references;
}
#ifdef THREADS
 trie_stats[MAX_THREADS];

#define TrStat_out             trie_stats[worker_id].out
#define TrStat_show            trie_stats[worker_id].show
#define TrStat_subgoals        trie_stats[worker_id].subgoals
#define TrStat_sg_incomplete   trie_stats[worker_id].subgoals_incomplete
#define TrStat_sg_nodes        trie_stats[worker_id].subgoal_trie_nodes
#define TrStat_answers         trie_stats[worker_id].answers
#define TrStat_answers_true    trie_stats[worker_id].answers_true
#define TrStat_answers_no      trie_stats[worker_id].answers_no
#define TrStat_answers_pruned  trie_stats[worker_id].answers_pruned
#define TrStat_ans_nodes       trie_stats[worker_id].answer_trie_nodes
#define TrStat_gt_terms        trie_stats[worker_id].global_trie_terms
#define TrStat_gt_nodes        trie_stats[worker_id].global_trie_nodes
#define TrStat_gt_refs         trie_stats[worker_id].global_trie_references
#else  /*!THREADS */
 trie_stats;

#define TrStat_out             trie_stats.out
#define TrStat_show            trie_stats.show
#define TrStat_subgoals        trie_stats.subgoals
#define TrStat_sg_incomplete   trie_stats.subgoals_incomplete
#define TrStat_sg_nodes        trie_stats.subgoal_trie_nodes
#define TrStat_answers         trie_stats.answers
#define TrStat_answers_true    trie_stats.answers_true
#define TrStat_answers_no      trie_stats.answers_no
#define TrStat_answers_pruned  trie_stats.answers_pruned
#define TrStat_ans_nodes       trie_stats.answer_trie_nodes
#define TrStat_gt_terms        trie_stats.global_trie_terms
#define TrStat_gt_nodes        trie_stats.global_trie_nodes
#define TrStat_gt_refs         trie_stats.global_trie_references
#endif /*THREADS */

#if defined(THREADS_SUBGOAL_SHARING) || defined(THREADS_FULL_SHARING)
#define IF_ABOLISH_SUBGOAL_TRIE_SHARED_DATA_STRUCTURES  if (worker_id == 0)
#else
#define IF_ABOLISH_SUBGOAL_TRIE_SHARED_DATA_STRUCTURES
#endif /* THREADS_SUBGOAL_SHARING || THREADS_FULL_SHARING */

#if defined(THREADS_FULL_SHARING)
#define IF_ABOLISH_ANSWER_TRIE_SHARED_DATA_STRUCTURES  if (worker_id == 0)
#else
#define IF_ABOLISH_ANSWER_TRIE_SHARED_DATA_STRUCTURES
#endif /* THREADS_FULL_SHARING */

#define SHOW_TABLE_STR_ARRAY_SIZE  100000
#define SHOW_TABLE_ARITY_ARRAY_SIZE 10000
#define SHOW_TABLE_STRUCTURE(MESG, ARGS...)      \
        if (TrStat_show == SHOW_MODE_STRUCTURE)  \
          Sfprintf(TrStat_out, MESG, ##ARGS)

#define CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(REF,MODE)		                                            \
        if (MODE == TRAVERSE_MODE_NORMAL && IsVarTerm(REF) && REF > VarIndexOfTableTerm(MAX_TABLE_VARS)) {  \
          register gt_node_ptr gt_node = (gt_node_ptr) (REF);	                                            \
          TrNode_child(gt_node) = (gt_node_ptr) ((unsigned long int) TrNode_child(gt_node) - 1);            \
          if (TrNode_child(gt_node) == 0)                                                                   \
            FREE_GLOBAL_TRIE_BRANCH(gt_node,TRAVERSE_MODE_NORMAL);		                            \
        }
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
#define CHECK_DECREMENT_GLOBAL_TRIE_FOR_SUBTERMS_REFERENCE(REF,MODE)	                                    \
        CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(REF,MODE)
#define FREE_GLOBAL_TRIE_BRANCH(NODE,MODE)                                                                  \
        free_global_trie_branch(NODE,MODE PASS_REGS)
#else
#define CHECK_DECREMENT_GLOBAL_TRIE_FOR_SUBTERMS_REFERENCE(REF,MODE)
#define FREE_GLOBAL_TRIE_BRANCH(NODE,MODE)                                                                  \
        free_global_trie_branch(NODE PASS_REGS)
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */



/******************************
**      Local functions      **
******************************/

#define INCLUDE_SUBGOAL_TRIE_CHECK_INSERT  /* subgoal_trie_check_insert_entry */
#define INCLUDE_ANSWER_TRIE_CHECK_INSERT   /* answer_trie_check_insert_entry */
#define INCLUDE_GLOBAL_TRIE_CHECK_INSERT   /* global_trie_check_insert_entry */
#include "tab.tries.i"
#undef INCLUDE_GLOBAL_TRIE_CHECK_INSERT
#undef INCLUDE_ANSWER_TRIE_CHECK_INSERT
#undef INCLUDE_SUBGOAL_TRIE_CHECK_INSERT

#define MODE_GLOBAL_TRIE_ENTRY
#define INCLUDE_SUBGOAL_TRIE_CHECK_INSERT  /* subgoal_trie_check_insert_gt_entry */
#define INCLUDE_ANSWER_TRIE_CHECK_INSERT   /* answer_trie_check_insert_gt_entry */
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
#define INCLUDE_GLOBAL_TRIE_CHECK_INSERT   /* global_trie_check_insert_gt_entry */
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
#include "tab.tries.i"
#undef INCLUDE_GLOBAL_TRIE_CHECK_INSERT
#undef INCLUDE_ANSWER_TRIE_CHECK_INSERT
#undef INCLUDE_SUBGOAL_TRIE_CHECK_INSERT
#undef MODE_GLOBAL_TRIE_ENTRY

#define INCLUDE_SUBGOAL_SEARCH_LOOP        /* subgoal_search_loop */
#define INCLUDE_ANSWER_SEARCH_LOOP         /* answer_search_loop */
#define INCLUDE_LOAD_ANSWER_LOOP           /* load_answer_loop */
#include "tab.tries.i"
#undef INCLUDE_LOAD_ANSWER_LOOP
#undef INCLUDE_ANSWER_SEARCH_LOOP
#undef INCLUDE_SUBGOAL_SEARCH_LOOP

#define MODE_TERMS_LOOP
#define INCLUDE_SUBGOAL_SEARCH_LOOP        /* subgoal_search_terms_loop */
#define INCLUDE_ANSWER_SEARCH_LOOP         /* answer_search_terms_loop */
#include "tab.tries.i"
#undef INCLUDE_ANSWER_SEARCH_LOOP
#undef INCLUDE_SUBGOAL_SEARCH_LOOP
#undef MODE_TERMS_LOOP

#define MODE_GLOBAL_TRIE_LOOP
#define INCLUDE_SUBGOAL_SEARCH_LOOP        /* subgoal_search_global_trie_(terms)_loop */
#define INCLUDE_ANSWER_SEARCH_LOOP         /* answer_search_global_trie_(terms)_loop */
#define INCLUDE_LOAD_ANSWER_LOOP           /* load_substitution_loop */
#include "tab.tries.i"
#undef INCLUDE_LOAD_ANSWER_LOOP
#undef INCLUDE_ANSWER_SEARCH_LOOP
#undef INCLUDE_SUBGOAL_SEARCH_LOOP
#undef MODE_GLOBAL_TRIE_LOOP

#ifdef MODE_DIRECTED_TABLING
#define INCLUDE_ANSWER_SEARCH_MODE_DIRECTED
#include "tab.tries.i"                     /* answer_search_min_max + invalidate_answer_trie */
#undef INCLUDE_ANSWER_SEARCH_MODE_DIRECTED
#endif /* MODE_DIRECTED_TABLING */



static inline CELL *exec_substitution_loop(gt_node_ptr current_node, CELL **stack_vars_ptr, CELL *stack_terms USES_REGS) {
/************************************************************************
                     ===========
                     |         |
                     |   ...   |
                     |         |
                     -----------
           YENV -->  |   N+1   |  <-- stack_vars
                     -----------
                     |  VAR_N  |
                     -----------
                     |   ...   |
                     -----------
                     |  VAR_0  |
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
                     ----------|           |  stack_terms_pair_offset (TRIE_COMPACT_PAIRS)
                     |  TERM_1 |          \|/
                     ===========           *
 LOCAL_TrailTop -->  |         |  <-- stack_terms_base (TRIE_COMPACT_PAIRS)
                     -----------
************************************************************************/
  CELL *stack_vars = *stack_vars_ptr;
  CELL *stack_terms_limit = (CELL *) TR;
#ifdef TRIE_COMPACT_PAIRS
#define stack_terms_base ((CELL *) LOCAL_TrailTop)
  int stack_terms_pair_offset = 0;
#endif /* TRIE_COMPACT_PAIRS */
  Term t = TrNode_entry(current_node);
  current_node = TrNode_parent(current_node);

  do {
    if (IsVarTerm(t)) {
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
      if (t > VarIndexOfTableTerm(MAX_TABLE_VARS)) {
	stack_terms = exec_substitution_loop((gt_node_ptr) t, &stack_vars, stack_terms PASS_REGS);
      } else 
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
      {
	int var_index = VarIndexOfTableTerm(t);
        int vars_arity = *stack_vars;
	t = MkVarTerm();
	if (var_index >= vars_arity) {
	  while (vars_arity < var_index) {
	    *stack_vars-- = 0;
	    vars_arity++;
	  }
	  *stack_vars-- = t;
	  vars_arity++;
	  *stack_vars = vars_arity;
	} else {
	  /* do the same as in macro stack_trie_val_instr() */
	  CELL aux_sub, aux_var, *vars_ptr;
	  vars_ptr = stack_vars + vars_arity - var_index;
	  aux_sub = *((CELL *) t);
	  aux_var = *vars_ptr;
	  if (aux_var == 0) {
	    *vars_ptr = t;
	  } else {
	    if (aux_sub > aux_var) {
	      if ((CELL *) aux_sub <= H) {
		Bind_Global((CELL *) aux_sub, aux_var);
	      } else if ((CELL *) aux_var <= H) {
		Bind_Local((CELL *) aux_sub, aux_var);
	      } else {
		Bind_Local((CELL *) aux_var, aux_sub);
		*vars_ptr = aux_sub;
	      }
	    } else {            
	      if ((CELL *) aux_var <= H) {
		Bind_Global((CELL *) aux_var, aux_sub);
		*vars_ptr = aux_sub;
	      } else if ((CELL *) aux_sub <= H) {
		Bind_Local((CELL *) aux_var, aux_sub);
		*vars_ptr = aux_sub;
	      } else {
		Bind_Local((CELL *) aux_sub, aux_var);
	      }
	    }
	  }
	}
	AUX_STACK_CHECK_EXPAND(stack_terms, stack_terms_limit);
	STACK_PUSH_UP(t, stack_terms);
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
#else
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

  *stack_vars_ptr = stack_vars;
  return stack_terms;

#ifdef TRIE_COMPACT_PAIRS
#undef stack_terms_base
#endif /* TRIE_COMPACT_PAIRS */
}


#ifdef YAPOR
#ifdef TABLING_INNER_CUTS
static int update_answer_trie_branch(ans_node_ptr previous_node, ans_node_ptr current_node) {
  int ltt;
  if (! IS_ANSWER_LEAF_NODE(current_node)) {
    if (TrNode_child(current_node)) {
      TrNode_instr(TrNode_child(current_node)) -= 1;  /* retry --> try */
      update_answer_trie_branch(NULL, TrNode_child(current_node));
      if (TrNode_child(current_node))
        goto update_next_trie_branch;
    }
    /* node belonging to a pruned answer */
    if (previous_node) {
      TrNode_next(previous_node) = TrNode_next(current_node);
      FREE_ANSWER_TRIE_NODE(current_node);
      if (TrNode_next(previous_node)) {
        return update_answer_trie_branch(previous_node, TrNode_next(previous_node));
      } else {
        TrNode_instr(previous_node) -= 2;  /* retry --> trust : try --> do */
        return 0;
      }
    } else {
      TrNode_child(TrNode_parent(current_node)) = TrNode_next(current_node);
      if (TrNode_next(current_node)) {
        TrNode_instr(TrNode_next(current_node)) -= 1;  /* retry --> try */
        update_answer_trie_branch(NULL, TrNode_next(current_node));          
      }
      FREE_ANSWER_TRIE_NODE(current_node);
      return 0;
    }
  }
update_next_trie_branch:
  if (TrNode_next(current_node)) {
    ltt = 1 + update_answer_trie_branch(current_node, TrNode_next(current_node));
  } else {
    TrNode_instr(current_node) -= 2;  /* retry --> trust : try --> do */
    ltt = 1;
  }

  TrNode_or_arg(current_node) = ltt;
  TrNode_instr(current_node) = Yap_opcode(TrNode_instr(current_node));
  return ltt;
}
#else /* YAPOR && ! TABLING_INNER_CUTS */
static int update_answer_trie_branch(ans_node_ptr current_node) {
  int ltt;
  if (! IS_ANSWER_LEAF_NODE(current_node)) {
    TrNode_instr(TrNode_child(current_node)) -= 1;  /* retry --> try */
    update_answer_trie_branch(TrNode_child(current_node));
  }
  if (TrNode_next(current_node)) {
    ltt = 1 + update_answer_trie_branch(TrNode_next(current_node));
  } else {
    TrNode_instr(current_node) -= 2;  /* retry --> trust : try --> do */
    ltt = 1;
  }
  TrNode_or_arg(current_node) = ltt;
  TrNode_instr(current_node) = Yap_opcode(TrNode_instr(current_node));
  return ltt;
}
#endif
#else /* ! YAPOR */
static void update_answer_trie_branch(ans_node_ptr current_node, int position) {
  if (! IS_ANSWER_LEAF_NODE(current_node))
    update_answer_trie_branch(TrNode_child(current_node), TRAVERSE_POSITION_FIRST);  /* retry --> try */
  if (position == TRAVERSE_POSITION_FIRST) {
    ans_node_ptr next = TrNode_next(current_node);
    if (next) {
      while (TrNode_next(next)) {
	update_answer_trie_branch(next, TRAVERSE_POSITION_NEXT);  /* retry --> retry */
	next = TrNode_next(next);
      }
      update_answer_trie_branch(next, TRAVERSE_POSITION_LAST);  /* retry --> trust */
    } else
      position += TRAVERSE_POSITION_LAST;  /* try --> do */
  }
  TrNode_instr(current_node) = Yap_opcode(TrNode_instr(current_node) - position);
  return;
}

#endif /* YAPOR */


#ifdef GLOBAL_TRIE_FOR_SUBTERMS
static void free_global_trie_branch(gt_node_ptr current_node, int mode USES_REGS) {
  Term t = TrNode_entry(current_node);
#else
static void free_global_trie_branch(gt_node_ptr current_node USES_REGS) {
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
  gt_node_ptr parent_node, child_node;

  parent_node = TrNode_parent(current_node);
  child_node  = TrNode_child(parent_node);
  if (IS_GLOBAL_TRIE_HASH(child_node)) {
    gt_hash_ptr hash = (gt_hash_ptr) child_node;
    gt_node_ptr *bucket = Hash_buckets(hash) + HASH_ENTRY(TrNode_entry(current_node), Hash_num_buckets(hash));
    int num_nodes = --Hash_num_nodes(hash);
    child_node = *bucket;
    if (child_node != current_node) {
      while (TrNode_next(child_node) != current_node)
	child_node = TrNode_next(child_node);
      TrNode_next(child_node) = TrNode_next(current_node);
      CHECK_DECREMENT_GLOBAL_TRIE_FOR_SUBTERMS_REFERENCE(t, mode);
      FREE_GLOBAL_TRIE_NODE(current_node);
    } else {
      *bucket = TrNode_next(current_node);
      CHECK_DECREMENT_GLOBAL_TRIE_FOR_SUBTERMS_REFERENCE(t, mode);
      FREE_GLOBAL_TRIE_NODE(current_node);
      if (num_nodes == 0) {
	FREE_BUCKETS(Hash_buckets(hash));
	FREE_GLOBAL_TRIE_HASH(hash);
	if (parent_node != GLOBAL_root_gt) {
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
	  if (mode == TRAVERSE_MODE_NORMAL) {
	    if (IsApplTerm(t)) {
	      Functor f = (Functor) RepAppl(t);
	      if (f == FunctorDouble)
		mode = TRAVERSE_MODE_DOUBLE;
	      else if (f == FunctorLongInt)
		mode = TRAVERSE_MODE_LONGINT;
	      else
		mode = TRAVERSE_MODE_NORMAL;
	    } else
	      mode = TRAVERSE_MODE_NORMAL;
	  } else if (mode == TRAVERSE_MODE_LONGINT)
	    mode = TRAVERSE_MODE_LONGINT_END;
	  else if (mode == TRAVERSE_MODE_DOUBLE)
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
	    mode = TRAVERSE_MODE_DOUBLE2;
	  else if (mode == TRAVERSE_MODE_DOUBLE2)
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	    mode = TRAVERSE_MODE_DOUBLE_END;
	  else
	    mode = TRAVERSE_MODE_NORMAL;
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
	  FREE_GLOBAL_TRIE_BRANCH(parent_node, mode);
	} else
	  TrNode_child(parent_node) = NULL;
      }
    }
  } else if (child_node != current_node) {
    while (TrNode_next(child_node) != current_node)
      child_node = TrNode_next(child_node);
    TrNode_next(child_node) = TrNode_next(current_node);
    CHECK_DECREMENT_GLOBAL_TRIE_FOR_SUBTERMS_REFERENCE(t, mode);
    FREE_GLOBAL_TRIE_NODE(current_node);
  } else if (TrNode_next(current_node) == NULL) {
    CHECK_DECREMENT_GLOBAL_TRIE_FOR_SUBTERMS_REFERENCE(t, mode);
    FREE_GLOBAL_TRIE_NODE(current_node);
    if (parent_node != GLOBAL_root_gt) {
#ifdef GLOBAL_TRIE_FOR_SUBTERMS
      if (mode == TRAVERSE_MODE_NORMAL) {
	if (IsApplTerm(t)) {
	  Functor f = (Functor) RepAppl(t);
	  if (f == FunctorDouble)
	    mode = TRAVERSE_MODE_DOUBLE;
	  else if (f == FunctorLongInt)
	    mode = TRAVERSE_MODE_LONGINT;
	  else
	    mode = TRAVERSE_MODE_NORMAL;
	} else
	  mode = TRAVERSE_MODE_NORMAL;
      } else if (mode == TRAVERSE_MODE_LONGINT)
	mode = TRAVERSE_MODE_LONGINT_END;
      else if (mode == TRAVERSE_MODE_DOUBLE)
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
	mode = TRAVERSE_MODE_DOUBLE2;
      else if (mode == TRAVERSE_MODE_DOUBLE2)
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	mode = TRAVERSE_MODE_DOUBLE_END;
      else
	mode = TRAVERSE_MODE_NORMAL;
#endif /* GLOBAL_TRIE_FOR_SUBTERMS */
      FREE_GLOBAL_TRIE_BRANCH(parent_node, mode);
    } else
      TrNode_child(parent_node) = NULL;
  } else {
    TrNode_child(parent_node) = TrNode_next(current_node);
    CHECK_DECREMENT_GLOBAL_TRIE_FOR_SUBTERMS_REFERENCE(t, mode);
    FREE_GLOBAL_TRIE_NODE(current_node);
  }
  return;
}


#ifdef EXTRA_STATISTICS
 static void traverse_subgoal_trie(long sg_dep, sg_node_ptr current_node, char *str, int str_index, int *arity, int mode, int position USES_REGS) {
#else
 static void traverse_subgoal_trie(sg_node_ptr current_node, char *str, int str_index, int *arity, int mode, int position USES_REGS) {
#endif

  int *current_arity = NULL, current_str_index = 0, current_mode = 0;
  //  printf("traverse - current_node = %p sg_fr = %p \n", current_node, TrNode_sg_fr(current_node));

  /* test if hashing */
  if (IS_SUBGOAL_TRIE_HASH(current_node)) {
    sg_node_ptr *bucket, *last_bucket;
    sg_hash_ptr hash;
    hash = (sg_hash_ptr) current_node;
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
    bucket = SgHash_buckets(hash);
    last_bucket = bucket + SgHash_num_buckets(hash);
#else
    bucket = Hash_buckets(hash);
    last_bucket = bucket + Hash_num_buckets(hash);
#endif
    current_arity = (int *) malloc(sizeof(int) * (arity[0] + 1));
    memcpy(current_arity, arity, sizeof(int) * (arity[0] + 1));
    do {
      if (*bucket) {
#ifdef EXTRA_STATISTICS
        traverse_subgoal_trie(sg_dep, *bucket, str, str_index, arity, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
#else
        traverse_subgoal_trie(*bucket, str, str_index, arity, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
#endif
	memcpy(arity, current_arity, sizeof(int) * (current_arity[0] + 1));
#ifdef TRIE_COMPACT_PAIRS
	if (arity[arity[0]] == -2 && str[str_index - 1] != '[')
	  str[str_index - 1] = ',';
#else
	if (arity[arity[0]] == -1)
	  str[str_index - 1] = '|';
#endif /* TRIE_COMPACT_PAIRS */
      }
    } while (++bucket != last_bucket);
    free(current_arity);
    return;
  }

  /* save current state if first sibling node */
  if (position == TRAVERSE_POSITION_FIRST) {
    current_arity = (int *) malloc(sizeof(int) * (arity[0] + 1));
    memcpy(current_arity, arity, sizeof(int) * (arity[0] + 1));
    current_str_index = str_index;
    current_mode = mode;
  }

  /* process current trie node */
  TrStat_sg_nodes++;
  traverse_trie_node(TrNode_entry(current_node), str, &str_index, arity, &mode, TRAVERSE_TYPE_SUBGOAL PASS_REGS);

  /* show answers ... */
  if (IS_SUBGOAL_LEAF_NODE(current_node)) {

#ifdef THREADS_FULL_SHARING
    sg_fr_ptr sg_fr;
    sg_fr_ptr * sg_fr_addr = (sg_fr_ptr *) get_insert_thread_bucket((void **) &SgEnt_sg_fr((sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_node)))
#ifdef SUBGOAL_TRIE_LOCK_USING_NODE_FIELD
								    , &TrNode_lock(current_node)
#elif defined(SUBGOAL_TRIE_LOCK_USING_GLOBAL_ARRAY)
								    , &HASH_TRIE_LOCK(current_node)
#endif
								    );
    if (worker_id == 0 && *sg_fr_addr == NULL) {
      /* if worker_id = 0 arrived here without sg_fr then we create one. This will help for Test Suite */
      new_subgoal_frame(sg_fr, (sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_node)));
      SgFr_state(sg_fr) = SgFr_sg_ent_state(sg_fr);
      *sg_fr_addr = sg_fr;
    } else
      sg_fr = *sg_fr_addr;
#else
      sg_fr_ptr sg_fr = get_subgoal_frame(current_node);
#endif /* THREADS_FULL_SHARING */
#ifdef THREADS_SUBGOAL_SHARING
	/* just to avoid table differences on the test_suite. */
	if (sg_fr == NULL) {
	  void **buckets;
	  sg_fr_ptr *sg_fr_addr_completed;
	  sg_fr_ptr sg_fr_completed;
	  buckets = (void **) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_node));
	  sg_fr_addr_completed = (sg_fr_ptr *) buckets;
	  sg_fr_completed = *sg_fr_addr_completed;
	  if (sg_fr_completed != NULL)
	    sg_fr = sg_fr_completed;
	}
#endif /* THREADS_SUBGOAL_SHARING */    
    if (sg_fr) {
      Extra_stats_sg_trie(sg_dep);
      TrStat_subgoals++;
      str[str_index] = 0;
      SHOW_TABLE_STRUCTURE("%s.\n", str);
      TrStat_ans_nodes++;
#ifdef EXTRA_STATISTICS
      long ans_dep = 1;
#endif /* EXTRA_STATISTICS */
      if (SgFr_first_answer(sg_fr) == NULL) {
	if (SgFr_state(sg_fr) < complete) {
	  TrStat_sg_incomplete++;
	  SHOW_TABLE_STRUCTURE("    ---> INCOMPLETE\n");
	} else {
	  TrStat_answers_no++;
	  SHOW_TABLE_STRUCTURE("    NO\n");
	}
      } else if (SgFr_first_answer(sg_fr) == SgFr_answer_trie(sg_fr)) {
	TrStat_answers_true++;
	SHOW_TABLE_STRUCTURE("    TRUE\n");
      } else {
	arity[0] = 0;
#ifdef EXTRA_STATISTICS
	traverse_answer_trie(ans_dep, TrNode_child(SgFr_answer_trie(sg_fr)), &str[str_index], 0, arity, 0, TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
#else
	traverse_answer_trie(TrNode_child(SgFr_answer_trie(sg_fr)), &str[str_index], 0, arity, 0, TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
#endif /* EXTRA_STATISTICS */
	if (SgFr_state(sg_fr) < complete) {
	  TrStat_sg_incomplete++;
	  SHOW_TABLE_STRUCTURE("    ---> INCOMPLETE\n");
	}
      }
    }
  } else
  /* ... or continue with child node */
#ifdef EXTRA_STATISTICS
    traverse_subgoal_trie(sg_dep + 1, TrNode_child(current_node), str, str_index, arity, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
#else
    traverse_subgoal_trie(TrNode_child(current_node), str, str_index, arity, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
#endif

  /* restore the initial state and continue with sibling nodes */
  if (position == TRAVERSE_POSITION_FIRST) {
    str_index = current_str_index;
    mode = current_mode;
    current_node = TrNode_next(current_node);
    while (current_node) {
      memcpy(arity, current_arity, sizeof(int) * (current_arity[0] + 1));
#ifdef TRIE_COMPACT_PAIRS
      if (arity[arity[0]] == -2 && str[str_index - 1] != '[')
	str[str_index - 1] = ',';
#else
      if (arity[arity[0]] == -1)
	str[str_index - 1] = '|';
#endif /* TRIE_COMPACT_PAIRS */
#ifdef EXTRA_STATISTICS
      traverse_subgoal_trie(sg_dep, current_node, str, str_index, arity, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
#else
      traverse_subgoal_trie(current_node, str, str_index, arity, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
#endif
      current_node = TrNode_next(current_node);
    }
    free(current_arity);
  }
  return;
}

#ifdef EXTRA_STATISTICS
 static void traverse_answer_trie(long ans_dep, ans_node_ptr current_node, char *str, int str_index, int *arity, int var_index, int mode, int position USES_REGS) {
#else
 static void traverse_answer_trie(ans_node_ptr current_node, char *str, int str_index, int *arity, int var_index, int mode, int position USES_REGS) {
#endif

  int *current_arity = NULL, current_str_index = 0, current_var_index = 0, current_mode = 0;

  /* test if hashing */

  if (IS_ANSWER_TRIE_HASH(current_node)) {
    ans_node_ptr *bucket, *last_bucket;
    ans_hash_ptr hash;

    hash = (ans_hash_ptr) current_node;
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
    bucket = AnsHash_buckets(hash);
    last_bucket = bucket + AnsHash_num_buckets(hash);
#else
    bucket = Hash_buckets(hash);
    last_bucket = bucket + Hash_num_buckets(hash);
#endif

    current_arity = (int *) malloc(sizeof(int) * (arity[0] + 1));
    memcpy(current_arity, arity, sizeof(int) * (arity[0] + 1));
    do {
      if (*bucket) { /* with atomic_locks when a thread arrives here no more expansions will happen on the trie */
#ifdef EXTRA_STATISTICS
        traverse_answer_trie(ans_dep, *bucket, str, str_index, arity, var_index, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
#else
        traverse_answer_trie(*bucket, str, str_index, arity, var_index, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
#endif
	memcpy(arity, current_arity, sizeof(int) * (current_arity[0] + 1));
#ifdef TRIE_COMPACT_PAIRS
	if (arity[arity[0]] == -2 && str[str_index - 1] != '[')
	  str[str_index - 1] = ',';
#else
	if (arity[arity[0]] == -1)
	  str[str_index - 1] = '|';
#endif /* TRIE_COMPACT_PAIRS */
      }
    } while (++bucket != last_bucket);
    free(current_arity);
    return;
  }

  /* save current state if first sibling node */
  if (position == TRAVERSE_POSITION_FIRST) {
    current_arity = (int *) malloc(sizeof(int) * (arity[0] + 1));
    memcpy(current_arity, arity, sizeof(int) * (arity[0] + 1));
    current_str_index = str_index;
    current_var_index = var_index;
    current_mode = mode;
  }

  /* print VAR if starting a term */
  if (arity[0] == 0 && mode == TRAVERSE_MODE_NORMAL) {
    str_index += sprintf(& str[str_index], "    VAR%d: ", var_index);
    var_index++;
  }

  /* process current trie node */
  TrStat_ans_nodes++;
  traverse_trie_node(TrNode_entry(current_node), str, &str_index, arity, &mode, TRAVERSE_TYPE_ANSWER PASS_REGS);
  /* show answer .... */

  if (IS_ANSWER_LEAF_NODE(current_node)) {
    Extra_stats_ans_trie(ans_dep);    
    TrStat_answers++;
    str[str_index] = 0;
    SHOW_TABLE_STRUCTURE("%s\n", str);
  }
#ifdef TABLING_INNER_CUTS
  /* ... or continue with pruned node */
  else if (TrNode_child(current_node) == NULL) {
    TrStat_answers++;
    TrStat_answers_pruned++;
  }
#endif /* TABLING_INNER_CUTS */
  /* ... or continue with child node */
  else

#ifdef EXTRA_STATISTICS
    traverse_answer_trie(ans_dep + 1 , TrNode_child(current_node), str, str_index, arity, var_index, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
#else
    traverse_answer_trie(TrNode_child(current_node), str, str_index, arity, var_index, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
#endif
  /* restore the initial state and continue with sibling nodes */
  if (position == TRAVERSE_POSITION_FIRST) {
    str_index = current_str_index;
    var_index = current_var_index;
    mode = current_mode;
    current_node = TrNode_next(current_node);
    while (current_node) {
      memcpy(arity, current_arity, sizeof(int) * (current_arity[0] + 1));
#ifdef TRIE_COMPACT_PAIRS
      if (arity[arity[0]] == -2 && str[str_index - 1] != '[')
	str[str_index - 1] = ',';
#else
      if (arity[arity[0]] == -1)
	str[str_index - 1] = '|';
#endif /* TRIE_COMPACT_PAIRS */
#ifdef EXTRA_STATISTICS
      traverse_answer_trie(ans_dep, current_node, str, str_index, arity, var_index, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
#else
      traverse_answer_trie(current_node, str, str_index, arity, var_index, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
#endif
      current_node = TrNode_next(current_node);
    }
    free(current_arity);
  }
  return;
}


static void traverse_global_trie(gt_node_ptr current_node, char *str, int str_index, int *arity, int mode, int position USES_REGS) {
  int *current_arity = NULL, current_str_index = 0, current_mode = 0;

  /* test if hashing */
  if (IS_GLOBAL_TRIE_HASH(current_node)) {
    gt_node_ptr *bucket, *last_bucket;
    gt_hash_ptr hash;
    hash = (gt_hash_ptr) current_node;
    bucket = Hash_buckets(hash);
    last_bucket = bucket + Hash_num_buckets(hash);
    current_arity = (int *) malloc(sizeof(int) * (arity[0] + 1));
    memcpy(current_arity, arity, sizeof(int) * (arity[0] + 1));
    do {
      if (*bucket) {
        traverse_global_trie(*bucket, str, str_index, arity, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
	memcpy(arity, current_arity, sizeof(int) * (current_arity[0] + 1));
#ifdef TRIE_COMPACT_PAIRS
	if (arity[arity[0]] == -2 && str[str_index - 1] != '[')
	  str[str_index - 1] = ',';
#else
	if (arity[arity[0]] == -1)
	  str[str_index - 1] = '|';
#endif /* TRIE_COMPACT_PAIRS */
      }
    } while (++bucket != last_bucket);
    free(current_arity);
    return;
  }

  /* save current state if first sibling node */
  if (position == TRAVERSE_POSITION_FIRST) {
    current_arity = (int *) malloc(sizeof(int) * (arity[0] + 1));
    memcpy(current_arity, arity, sizeof(int) * (arity[0] + 1));
    current_str_index = str_index;
    current_mode = mode;
  }

  /* process current trie node */
  TrStat_gt_nodes++;
  traverse_trie_node(TrNode_entry(current_node), str, &str_index, arity, &mode, TRAVERSE_TYPE_GT_SUBGOAL PASS_REGS);

  /* continue with child node ... */
  if (arity[0] != 0 || mode != TRAVERSE_MODE_NORMAL)
    traverse_global_trie(TrNode_child(current_node), str, str_index, arity, mode, TRAVERSE_POSITION_FIRST PASS_REGS);
  /* ... or show term */
  else {
    TrStat_gt_terms++;
    str[str_index] = 0;
    SHOW_TABLE_STRUCTURE("  TERMx%ld: %s\n", (unsigned long int) TrNode_child(current_node), str);
  }

  /* restore the initial state and continue with sibling nodes */
  if (position == TRAVERSE_POSITION_FIRST) {
    str_index = current_str_index;
    mode = current_mode;
    current_node = TrNode_next(current_node);
    while (current_node) {
      memcpy(arity, current_arity, sizeof(int) * (current_arity[0] + 1));
#ifdef TRIE_COMPACT_PAIRS
      if (arity[arity[0]] == -2 && str[str_index - 1] != '[')
	str[str_index - 1] = ',';
#else
      if (arity[arity[0]] == -1)
	str[str_index - 1] = '|';
#endif /* TRIE_COMPACT_PAIRS */
      traverse_global_trie(current_node, str, str_index, arity, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
      current_node = TrNode_next(current_node);
    }
    free(current_arity);
  }

  return;
}


static void traverse_global_trie_for_term(gt_node_ptr current_node, char *str, int *str_index, int *arity, int *mode, int type USES_REGS) {
  if (TrNode_parent(current_node) != GLOBAL_root_gt)
    traverse_global_trie_for_term(TrNode_parent(current_node), str, str_index, arity, mode, type PASS_REGS);
  traverse_trie_node(TrNode_entry(current_node), str, str_index, arity, mode, type PASS_REGS);
  return;
}


static inline void traverse_trie_node(Term t, char *str, int *str_index_ptr, int *arity, int *mode_ptr, int type USES_REGS) {
  int mode = *mode_ptr;
  int str_index = *str_index_ptr;

  /* test the node type */
  if (mode == TRAVERSE_MODE_DOUBLE) {
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
    arity[0]++;
    arity[arity[0]] = (int) t;
    mode = TRAVERSE_MODE_DOUBLE2;
  } else if (mode == TRAVERSE_MODE_DOUBLE2) {
    union {
      Term t_dbl[sizeof(Float)/sizeof(Term)];
      Float dbl;
    } u;
    u.dbl = 0.0;
    u.t_dbl[0] = t;
    u.t_dbl[1] = (Term) arity[arity[0]];
    arity[0]--;
#else /* SIZEOF_DOUBLE == SIZEOF_INT_P */
    union {
      Term t_dbl[sizeof(Float)/sizeof(Term)];
      Float dbl;
    } u;
    u.t_dbl[0] = t;
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
    str_index += sprintf(& str[str_index], "%.15g", u.dbl);
    traverse_update_arity(str, &str_index, arity);
    if (type == TRAVERSE_TYPE_SUBGOAL)
      mode = TRAVERSE_MODE_NORMAL;
    else  /* TRAVERSE_TYPE_ANSWER || TRAVERSE_TYPE_GT_SUBGOAL || TRAVERSE_TYPE_GT_ANSWER */
      mode = TRAVERSE_MODE_DOUBLE_END;
  } else if (mode == TRAVERSE_MODE_DOUBLE_END) {
    mode = TRAVERSE_MODE_NORMAL;
  } else if (mode == TRAVERSE_MODE_LONGINT) {
    Int li = (Int) t;
#if SHORT_INTS
    str_index += sprintf(& str[str_index], "%ld", li);
#else
    str_index += sprintf(& str[str_index], "%d", li);
#endif /* SHORT_INTS */
    traverse_update_arity(str, &str_index, arity);
    if (type == TRAVERSE_TYPE_SUBGOAL)
      mode = TRAVERSE_MODE_NORMAL;
    else  /* TRAVERSE_TYPE_ANSWER || TRAVERSE_TYPE_GT_SUBGOAL || TRAVERSE_TYPE_GT_ANSWER */
      mode = TRAVERSE_MODE_LONGINT_END;
  } else if (mode == TRAVERSE_MODE_LONGINT_END) {
    mode = TRAVERSE_MODE_NORMAL;
  } else if (IsVarTerm(t)) {
    if (t > VarIndexOfTableTerm(MAX_TABLE_VARS)) {
      TrStat_gt_refs++;
      /* (type % 2 + 2): TRAVERSE_TYPE_ANSWER  --> TRAVERSE_TYPE_GT_ANSWER  */
      /* (type % 2 + 2): TRAVERSE_TYPE_SUBGOAL --> TRAVERSE_TYPE_GT_SUBGOAL */
      traverse_global_trie_for_term((gt_node_ptr) t, str, &str_index, arity, &mode, type % 2 + 2 PASS_REGS);
    } else {
      if (type == TRAVERSE_TYPE_SUBGOAL || type == TRAVERSE_TYPE_GT_SUBGOAL)
	str_index += sprintf(& str[str_index], "VAR%d", VarIndexOfTableTerm(t));
      else  /* TRAVERSE_TYPE_ANSWER || TRAVERSE_TYPE_GT_ANSWER */
	str_index += sprintf(& str[str_index], "ANSVAR%d", VarIndexOfTableTerm(t));
      traverse_update_arity(str, &str_index, arity);
    }
  } else if (IsIntTerm(t)) {
#if SHORT_INTS
    str_index += sprintf(& str[str_index], "%ld", IntOfTerm(t));
#else
    str_index += sprintf(& str[str_index], "%d", IntOfTerm(t));
#endif /* SHORT_INTS */
    traverse_update_arity(str, &str_index, arity);
  } else if (IsAtomTerm(t)) {
#ifndef TRIE_COMPACT_PAIRS
    if (arity[arity[0]] == -1 && t == TermNil) {
      str[str_index - 1] = ']';
      arity[0]--;
    } else
#endif /* TRIE_COMPACT_PAIRS */
      str_index += sprintf(& str[str_index], "%s", AtomName(AtomOfTerm(t)));
    traverse_update_arity(str, &str_index, arity);
  } else if (IsPairTerm(t)) {
#ifdef TRIE_COMPACT_PAIRS
    if (t == CompactPairEndList)
      arity[arity[0]] = -1;
    else if (t == CompactPairEndTerm) {
      str[str_index - 1] = '|';
      arity[arity[0]] = -1;
#else
    if (arity[arity[0]] == -1) {
      str[str_index - 1] = ',';
      arity[arity[0]] = -2;
#endif /* TRIE_COMPACT_PAIRS */
    } else {
      str_index += sprintf(& str[str_index], "[");
      arity[0]++;
      arity[arity[0]] = -2;
    }
  } else if (IsApplTerm(t)) {
    Functor f = (Functor) RepAppl(t);
    if (f == FunctorDouble) {
      mode = TRAVERSE_MODE_DOUBLE;
    } else if (f == FunctorLongInt) {
      mode = TRAVERSE_MODE_LONGINT;
    } else if (f == FunctorComma) {
      if (arity[arity[0]] != -3) {
	str_index += sprintf(& str[str_index], "(");
	arity[0]++;
      }
      arity[arity[0]] = -4;
    } else {
      str_index += sprintf(& str[str_index], "%s(", AtomName(NameOfFunctor(f)));
      arity[0]++;
      arity[arity[0]] = ArityOfFunctor(f);
    }
  }

  *mode_ptr = mode;
  *str_index_ptr = str_index;
  return;
}


static inline void traverse_update_arity(char *str, int *str_index_ptr, int *arity) {
  int str_index = *str_index_ptr;
  while (arity[0]) {
    if (arity[arity[0]] > 0) {
      arity[arity[0]]--;
      if (arity[arity[0]] == 0) {
	str_index += sprintf(& str[str_index], ")");
	arity[0]--;
      } else {
	str_index += sprintf(& str[str_index], ",");
	break;
      }
    } else {
      if (arity[arity[0]] == -4) {
	str_index += sprintf(& str[str_index], ",");
	arity[arity[0]] = -3;
	break;
      } else if (arity[arity[0]] == -3) {
	str_index += sprintf(& str[str_index], ")");
	arity[0]--;
      } else if (arity[arity[0]] == -2) {
#ifdef TRIE_COMPACT_PAIRS
	str_index += sprintf(& str[str_index], ",");
#else
	str_index += sprintf(& str[str_index], "|");
	arity[arity[0]] = -1;
#endif /* TRIE_COMPACT_PAIRS */
	break;
      } else if (arity[arity[0]] == -1) {
	str_index += sprintf(& str[str_index], "]");
	arity[0]--;
      }
    }
  }
  *str_index_ptr = str_index;
}



/*******************************
**      Global functions      **
*******************************/
 
#ifdef THREADS_NO_SUBGOAL_TRIE
 sg_fr_ptr subgoal_search_no_trie(yamop *preg, CELL **Yaddr USES_REGS)  {
  tab_ent_ptr tab_ent = preg->u.Otapl.te;
  CELL *stack_vars;
  int i, subs_arity, pred_arity;
  sg_fr_ptr sg_fr;
  
  int *mode_directed, aux_mode_directed[MAX_TABLE_VARS];
  mode_directed = TabEnt_mode_directed(tab_ent);
  int subs_pos = 0;
  stack_vars = *Yaddr;
  subs_arity = 0;
  pred_arity = preg->u.Otapl.s;

  int mode = MODE_DIRECTED_GET_MODE(mode_directed[0]);
  int j = MODE_DIRECTED_GET_ARG(mode_directed[0]) + 1;
  Term t = Deref(XREGS[j]);
  int no_st_index = IntOfTerm(t);

  /*  printf("pred_arity %d subs_arity %d args %d %d %d j %d i %d\n",pred_arity, subs_arity, 
	 IntOfTerm(Deref(XREGS[1])), 
	 IntOfTerm(Deref(XREGS[2])), 
	 IntOfTerm(Deref(XREGS[3])),
	 j, no_st_index); */


  for (i = 1; i < pred_arity; i++) {
    mode = MODE_DIRECTED_GET_MODE(mode_directed[i]);
    j = MODE_DIRECTED_GET_ARG(mode_directed[i]) + 1;    
    t = Deref(XREGS[j]);
    if (mode == MODE_DIRECTED_DIM) {
      // t must be an int otherwise system must give error (to be updated)
      no_st_index = no_st_index * TabEnt_dim_array(tab_ent, i) + IntOfTerm(t);
    } else /* supporting mode == max || mode == min for now ...*/{
      /* t must be a var term - min and max can only have a single var*/
      STACK_PUSH_UP(t, stack_vars);
      subs_arity++; 
      aux_mode_directed[subs_pos++] = MODE_DIRECTED_SET(subs_arity, 
				    MODE_DIRECTED_GET_MODE(mode_directed[i]));
    }
  }
    
  no_subgoal_trie_pos no_st_pos = &(TabEnt_no_subgoal_trie_pos(tab_ent, no_st_index));
  //printf("2 - no_st_pos = %d\n", no_st_index);


  STACK_PUSH_UP(subs_arity, stack_vars);
  *Yaddr = stack_vars++;
  // reset variables 
  while (subs_arity--) {
    Term t = STACK_POP_DOWN(stack_vars);
    RESET_VARIABLE(t);
  }
  
  sg_fr = SgNoTrie_sg_fr(no_st_pos);

  if (sg_fr) {
    if (SgFr_state(sg_fr) >= complete || SgFr_wid(sg_fr) == worker_id)
      return sg_fr;
    mode_directed = SgFr_mode_directed(sg_fr);
    sg_fr = SgFr_next_wid(sg_fr);
    while(sg_fr) {
      if (SgFr_wid(sg_fr) == worker_id)
	return sg_fr;
      sg_fr = SgFr_next_wid(sg_fr);
    }
  } else
    mode_directed = NULL;
  
  
  /* no sg_fr complete for now */
  if (mode_directed == NULL && subs_pos) {
    ALLOC_BLOCK(mode_directed, subs_pos*sizeof(int), int);
    memcpy((void *)mode_directed, (void *)aux_mode_directed, subs_pos*sizeof(int));
  }
  new_subgoal_frame(sg_fr, preg, mode_directed);
  SgFr_wid(sg_fr) = worker_id;
  SgFr_no_sg_pos(sg_fr) = no_st_pos;

  sg_fr_ptr sg_fr_aux;
  do {
    sg_fr_aux = SgNoTrie_sg_fr(no_st_pos);  
    SgFr_next_wid(sg_fr) = sg_fr_aux;
  } while(!BOOL_CAS(&(SgNoTrie_sg_fr(no_st_pos)), sg_fr_aux, sg_fr));

  return sg_fr;
}

#endif /* THREADS_NO_SUBGOAL_TRIE */

sg_fr_ptr subgoal_search(yamop *preg, CELL **Yaddr USES_REGS)  {

  tab_ent_ptr tab_ent = preg->u.Otapl.te;
#ifdef THREADS_NO_SUBGOAL_TRIE
  if (TabEnt_no_subgoal_trie(tab_ent) != NULL)
    return subgoal_search_no_trie(preg, Yaddr PASS_REGS);    
#endif /* THREADS_NO_SUBGOAL_TRIE */

  CELL *stack_vars;
  int i, subs_arity, pred_arity;
  sg_fr_ptr sg_fr;
  sg_node_ptr current_sg_node;

#ifdef MODE_DIRECTED_TABLING
  int *mode_directed, aux_mode_directed[MAX_TABLE_VARS];
  int subs_pos = 0;
#endif /* MODE_DIRECTED_TABLING */

  stack_vars = *Yaddr;
  subs_arity = 0;
  pred_arity = preg->u.Otapl.s;
  current_sg_node = get_insert_subgoal_trie(tab_ent PASS_REGS);
  LOCK_SUBGOAL_TRIE(tab_ent);

#ifdef MODE_DIRECTED_TABLING
  mode_directed = TabEnt_mode_directed(tab_ent);
  if (mode_directed) {
    int old_subs_arity = subs_arity;
    //printf("pred_arity %d subs_arity %d\n",pred_arity, subs_arity);
    for (i = 1; i <= pred_arity; i++) {
      int j = MODE_DIRECTED_GET_ARG(mode_directed[i-1]) + 1;
      current_sg_node = subgoal_search_loop(tab_ent, current_sg_node, Deref(XREGS[j]), &subs_arity, &stack_vars PASS_REGS);
      if (subs_arity != old_subs_arity) {
	if (subs_pos && MODE_DIRECTED_GET_MODE(aux_mode_directed[subs_pos-1]) == MODE_DIRECTED_GET_MODE(mode_directed[i-1])) {
	  /* same mode as before -> use the current entry in the aux_mode_directed[] array */
	  aux_mode_directed[subs_pos-1] += MODE_DIRECTED_SET(subs_arity - old_subs_arity, 0);
	} else {
	  /* new mode -> init a new entry in the aux_mode_directed[] array */
	  aux_mode_directed[subs_pos] = MODE_DIRECTED_SET(subs_arity - old_subs_arity, MODE_DIRECTED_GET_MODE(mode_directed[i-1]));
	  subs_pos++;
	}
	old_subs_arity = subs_arity;
      }
    }
  } else
#endif /* MODE_DIRECTED_TABLING */
  if (IsMode_GlobalTrie(TabEnt_mode(tab_ent))) {
    for (i = 1; i <= pred_arity; i++)
      current_sg_node = subgoal_search_terms_loop(tab_ent, current_sg_node, Deref(XREGS[i]), &subs_arity, &stack_vars PASS_REGS);
  } else {
    for (i = 1; i <= pred_arity; i++)
      current_sg_node = subgoal_search_loop(tab_ent, current_sg_node, Deref(XREGS[i]), &subs_arity, &stack_vars PASS_REGS);
  }

  STACK_PUSH_UP(subs_arity, stack_vars);
  *Yaddr = stack_vars++;
  /* reset variables */
  while (subs_arity--) {
    Term t = STACK_POP_DOWN(stack_vars);
    RESET_VARIABLE(t);
  }

#ifdef THREADS_SUBGOAL_COMPLETION_WAIT
  sg_fr = (sg_fr_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_sg_node)); 
  if (sg_fr == NULL) {
    new_subgoal_frame(sg_fr, preg, NULL);
    SgFr_wid(sg_fr) = worker_id;
    SgFr_sg_leaf_node(sg_fr) = current_sg_node;
    if(BOOL_CAS(&(TrNode_sg_fr(current_sg_node)), NULL, ((CELL) sg_fr | 0x1))) {
      /* no sg_fr complete for now */
      if (subs_pos) {
	ALLOC_BLOCK(mode_directed, subs_pos*sizeof(int), int);
	memcpy((void *)mode_directed, (void *)aux_mode_directed, subs_pos*sizeof(int));
	SgFr_mode_directed(sg_fr) = mode_directed;
      }
      return sg_fr;
    }
    FREE_SUBGOAL_FRAME(sg_fr);
    FREE_ANSWER_TRIE_NODE(SgFr_answer_trie(sg_fr));
    pthread_cond_destroy(&(SgFr_comp_wait(sg_fr)));
    pthread_mutex_destroy(&(SgFr_lock_comp_wait(sg_fr)));
    //__sync_synchronize();
    sg_fr = (sg_fr_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_sg_node)); 
  }
  if (SgFr_state(sg_fr) >= complete || SgFr_wid(sg_fr) == worker_id)
    return sg_fr;

  LOCK_SG_FR_COMP_WAIT(sg_fr); 
  if (SgFr_state(sg_fr) != complete)
    pthread_cond_wait(&(SgFr_comp_wait(sg_fr)), &(SgFr_lock_comp_wait(sg_fr)));    
  UNLOCK_SG_FR_COMP_WAIT(sg_fr); 

  return sg_fr;
#endif /* THREADS_SUBGOAL_COMPLETION_WAIT */


#ifdef THREADS_SUBGOAL_FRAME_BY_WID 
#ifdef THREADS_SUBGOAL_SHARING
  sg_fr = (sg_fr_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_sg_node));  
  
  if (sg_fr) {
    if (
#ifdef THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE
	SgFr_state(sg_fr) >= complete ||
#endif  /* THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE */		
	SgFr_wid(sg_fr) == worker_id)
       return sg_fr;
	mode_directed = SgFr_mode_directed(sg_fr);
    sg_fr = SgFr_next_wid(sg_fr);
    while(sg_fr) {
      if (
	  /*	  #ifdef THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE
          SgFr_state(sg_fr) >= complete ||
	  #endif   THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE */
          SgFr_wid(sg_fr) == worker_id)
	  return sg_fr;
      sg_fr = SgFr_next_wid(sg_fr);
    }
  } else
    mode_directed = NULL;
  
  /* no sg_fr complete for now */
  if (mode_directed == NULL && subs_pos) {
    ALLOC_BLOCK(mode_directed, subs_pos*sizeof(int), int);
    memcpy((void *)mode_directed, (void *)aux_mode_directed, subs_pos*sizeof(int));
  }
  new_subgoal_frame(sg_fr, preg, mode_directed);
  SgFr_wid(sg_fr) = worker_id;
  SgFr_sg_leaf_node(sg_fr) = current_sg_node;

  sg_fr_ptr sg_fr_aux;
  do {
    sg_fr_aux = (sg_fr_ptr) TrNode_sg_fr(current_sg_node);  
    SgFr_next_wid(sg_fr) = (sg_fr_ptr) UNTAG_SUBGOAL_NODE(sg_fr_aux);
  } while(!BOOL_CAS(&(TrNode_sg_fr(current_sg_node)), sg_fr_aux, ((CELL) sg_fr | 0x1)));
  
  return sg_fr;

#else /* THREADS_FULL_SHARING */
  sg_ent_ptr sg_ent;
  sg_ent = (sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_sg_node));
  if (sg_ent == NULL) {
#ifdef MODE_DIRECTED_TABLING
    if (subs_pos) {
      ALLOC_BLOCK(mode_directed, subs_pos*sizeof(int), int);
      memcpy((void *)mode_directed, (void *)aux_mode_directed, subs_pos*sizeof(int));
    } else
      mode_directed = NULL;    
#endif /* MODE_DIRECTED_TABLING */
    new_subgoal_entry(sg_ent);
    SgEnt_init_mode_directed_fields(sg_ent, mode_directed);
    SgEnt_code(sg_ent) = preg;
    SgEnt_sg_fr(sg_ent)= NULL;

    if (!BOOL_CAS(&(TrNode_sg_fr(current_sg_node)), NULL,((CELL)sg_ent | (CELL)0x1))) {
      FREE_ANSWER_TRIE_NODE(SgEnt_answer_trie(sg_ent));
      FREE_SUBGOAL_ENTRY(sg_ent);
#ifdef MODE_DIRECTED_TABLING
      FREE_BLOCK(mode_directed);
#endif /* MODE_DIRECTED_TABLING */
      sg_ent = (sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_sg_node));
    }
  }
  
#ifdef THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE

  if (SgEnt_sg_ent_state(sg_ent) >= complete) 
    return SgEnt_sg_fr(sg_ent);

  sg_fr = SgEnt_sg_fr(sg_ent);

  while(sg_fr) {
    if (SgFr_wid(sg_fr) == worker_id)
      return sg_fr;
    sg_fr = SgFr_next_wid(sg_fr);
  }

  
  if (SgEnt_sg_ent_state(sg_ent) < complete) {
    LOCK(SgEnt_lock(sg_ent));
    if (SgEnt_sg_ent_state(sg_ent) < complete) {
      new_subgoal_frame(sg_fr, sg_ent);
      SgFr_wid(sg_fr) = worker_id;
      SgFr_next_wid(sg_fr) = SgEnt_sg_fr(sg_ent);
      SgEnt_sg_fr(sg_ent) = sg_fr;
      SgFr_active_workers(sg_fr)++;      
      UNLOCK(SgEnt_lock(sg_ent));
      return sg_fr;
    }
    UNLOCK(SgEnt_lock(sg_ent));
  }
  
  return SgEnt_sg_fr(sg_ent);

#else /* !THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE */

  sg_fr = SgEnt_sg_fr(sg_ent);
  while(sg_fr) {
    if (SgFr_wid(sg_fr) == worker_id)
      return sg_fr;    
    sg_fr = SgFr_next_wid(sg_fr);
  }
  new_subgoal_frame(sg_fr, sg_ent);
  SgFr_wid(sg_fr) = worker_id;
  LOCK(SgEnt_lock(sg_ent));  
  SgFr_next_wid(sg_fr) = SgEnt_sg_fr(sg_ent);
  SgEnt_sg_fr(sg_ent) = sg_fr;
  SgFr_active_workers(sg_fr)++;      
  UNLOCK(SgEnt_lock(sg_ent));
  return sg_fr;
#endif /* THREADS_SUBGOAL_FRAME_BY_WID_SHARE_COMPLETE */
  
#endif /* THREADS_SUBGOAL_SHARING */
#else /* !THREADS_SUBGOAL_FRAME_BY_WID */
#ifdef THREADS_LOCAL_SG_FR_HASH_BUCKETS 
  sg_fr_ptr *bucket;
  int sg_fr_hash_key = HASH_ENTRY_SG_FR(current_sg_node, SgFrHashBkts_number_of_buckets(LOCAL_sg_fr_hash_buckets));
  bucket = SgFrHashBkts_buckets(LOCAL_sg_fr_hash_buckets) + sg_fr_hash_key;
  sg_fr = *bucket;
  while(sg_fr) {
    if (SgFr_sg_leaf_node(sg_fr) == current_sg_node)
      return sg_fr;    
    sg_fr = SgFr_next_on_hash(sg_fr);
  }
  sg_fr = (sg_fr_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_sg_node));
  
  if (sg_fr != NULL) 
    return sg_fr;
  
  /* new tabled subgoal */
  if (subs_pos) {
    ALLOC_BLOCK(mode_directed, subs_pos*sizeof(int), int);
    memcpy((void *)mode_directed, (void *)aux_mode_directed, subs_pos*sizeof(int));
  } else
    mode_directed = NULL;
  new_subgoal_frame(sg_fr, preg, mode_directed);
  SgFr_sg_leaf_node(sg_fr) = current_sg_node;
  SgFr_next_on_hash(sg_fr) = *bucket;
  *bucket = sg_fr;
  return sg_fr;
#else /* !THREADS_LOCAL_SG_FR_HASH_BUCKETS */
  sg_fr_ptr *sg_fr_end = get_insert_subgoal_frame_addr(current_sg_node PASS_REGS);
  if (*sg_fr_end == NULL) {
    /* new tabled subgoal */
#if !defined(THREADS_FULL_SHARING)
#ifdef MODE_DIRECTED_TABLING
    if (subs_pos) {
      ALLOC_BLOCK(mode_directed, subs_pos*sizeof(int), int);
      memcpy((void *)mode_directed, (void *)aux_mode_directed, subs_pos*sizeof(int));
    } else
      mode_directed = NULL;
#endif /* MODE_DIRECTED_TABLING */
#ifdef THREADS_SUBGOAL_SHARING
    void **buckets;
    sg_fr_ptr *sg_fr_addr_completed;
    sg_fr_ptr sg_fr_completed;
    buckets = (void **) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(current_sg_node));
    sg_fr_addr_completed = (sg_fr_ptr *) buckets;
    sg_fr_completed = *sg_fr_addr_completed;
    if (sg_fr_completed != NULL)
      sg_fr = sg_fr_completed;
    else {
      new_subgoal_frame(sg_fr, preg, mode_directed);
      SgFr_sg_fr_array(sg_fr) = buckets;
    }
    *sg_fr_end = sg_fr;
#else
    new_subgoal_frame(sg_fr, preg, mode_directed);
    *sg_fr_end = sg_fr;
    TAG_AS_SUBGOAL_LEAF_NODE(current_sg_node);
#endif /* THREADS_SUBGOAL_SHARING */

#else /* THREADS_FULL_SHARING */
    sg_ent_ptr sg_ent = (sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_ent(current_sg_node));
    new_subgoal_frame(sg_fr, sg_ent);
    if (SgEnt_sg_ent_state(sg_ent) == ready) {
      LOCK(SgEnt_lock(sg_ent));
      if (SgEnt_sg_ent_state(sg_ent) == ready) {
        SgEnt_code(sg_ent) = preg;
#ifdef MODE_DIRECTED_TABLING
	if (subs_pos) {
          ALLOC_BLOCK(mode_directed, subs_pos*sizeof(int), int);
          memcpy((void *)mode_directed, (void *)aux_mode_directed, subs_pos*sizeof(int));
        } else
	  mode_directed = NULL;
#endif /* MODE_DIRECTED_TABLING */
	SgEnt_init_mode_directed_fields(sg_ent, mode_directed);
      }
      SgEnt_sg_ent_state(sg_ent) = evaluating;
      UNLOCK(SgEnt_lock(sg_ent));
    }
    *sg_fr_end = sg_fr;
#endif /* !THREADS_FULL_SHARING */
  } else {
    /* repeated tabled subgoal */
    sg_fr = (sg_fr_ptr) UNTAG_SUBGOAL_NODE(*sg_fr_end);
#ifdef LIMIT_TABLING
    if (SgFr_state(sg_fr) <= ready) {  /* incomplete or ready */
      remove_from_global_sg_fr_list(sg_fr);
    }
#endif /* LIMIT_TABLING */
  }
  UNLOCK_SUBGOAL_TRIE(tab_ent);
  return sg_fr;
#endif  /* THREADS_LOCAL_SG_FR_HASH_BUCKETS */
#endif /* !THREADS_SUBGOAL_FRAME_BY_WID */
}


ans_node_ptr answer_search(sg_fr_ptr sg_fr, CELL *subs_ptr USES_REGS) {
#define subs_arity *subs_ptr
  CELL *stack_vars;
  int i, vars_arity;
  ans_node_ptr current_ans_node;

  vars_arity = 0;
  current_ans_node = SgFr_answer_trie(sg_fr);

  if (IsMode_GlobalTrie(TabEnt_mode(SgFr_tab_ent(sg_fr)))) {
    for (i = subs_arity; i >= 1; i--) {
      TABLING_ERROR_CHECKING(answer_search, IsNonVarTerm(subs_ptr[i]));
      current_ans_node = answer_search_terms_loop(sg_fr, current_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
    }
  } else {
    for (i = subs_arity; i >= 1; i--) {
      TABLING_ERROR_CHECKING(answer_search, IsNonVarTerm(subs_ptr[i]));
      current_ans_node = answer_search_loop(sg_fr, current_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
    }
  }

  /* reset variables */
  stack_vars = (CELL *) TR;
  while (vars_arity--) {
    Term t = STACK_POP_DOWN(stack_vars);
    RESET_VARIABLE(t);
  }

  return current_ans_node;
#undef subs_arity
}


#ifdef MODE_DIRECTED_TABLING

#ifdef THREADS_NO_SUBGOAL_TRIE_MIN_MAX
void mode_directed_answer_search_no_trie(sg_fr_ptr sg_fr, CELL *subs_ptr USES_REGS) {
#define subs_arity *subs_ptr
  int *mode_directed;
  CELL *stack_vars;
  int i, j, vars_arity;
  mode_directed = SgFr_mode_directed(sg_fr);
  int mode = MODE_DIRECTED_GET_MODE(mode_directed[0]);
  i = subs_arity;

  no_subgoal_trie_pos no_st_pos = SgFr_no_sg_pos(sg_fr);
  Term term = Deref(subs_ptr[i]);
  Float term_value = (Float) IntOfTerm(term);
  Float no_trie_value = 0;
  Term no_trie_term;
  do {
    no_trie_term = SgNoTrie_ans(no_st_pos);     
    no_trie_value = (Float) IntOfTerm(no_trie_term);
    
    if ((mode == MODE_DIRECTED_MIN && term_value > no_trie_value) || 
	(mode == MODE_DIRECTED_MAX && term_value < no_trie_value))
      return;
  } while(!BOOL_CAS(&(SgNoTrie_ans(no_st_pos)), no_trie_term, term));

  /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX -> HERE 1 */
  printf("value = %d \n", IntOfTerm(SgNoTrie_ans(no_st_pos)));
 
  return;
}
#endif /* THREADS_NO_SUBGOAL_TRIE_MIN_MAX */


#if !defined(THREADS) || defined(THREADS_NO_SHARING) || defined(THREADS_SUBGOAL_SHARING) || defined(THREADS_FULL_SHARING_MODE_DIRECTED_V01)
 
ans_node_ptr mode_directed_answer_search(sg_fr_ptr sg_fr, CELL *subs_ptr USES_REGS) {
#define subs_arity *subs_ptr
  CELL *stack_vars;
  int i, j, vars_arity;
  ans_node_ptr current_ans_node, invalid_ans_node;
  int *mode_directed;

  LOCK_SG_FR(sg_fr); 

  vars_arity = 0;
  current_ans_node = SgFr_answer_trie(sg_fr);
  invalid_ans_node = NULL;
  mode_directed = SgFr_mode_directed(sg_fr);
  j = 0;
  i = subs_arity;
  int mode = MODE_DIRECTED_GET_MODE(mode_directed[j]);
  int n_subs = MODE_DIRECTED_GET_ARG(mode_directed[j]);

  while (i) {

    mode = MODE_DIRECTED_GET_MODE(mode_directed[j]);
    n_subs = MODE_DIRECTED_GET_ARG(mode_directed[j]);
    do {
      TABLING_ERROR_CHECKING(answer_search, IsNonVarTerm(subs_ptr[i]));
      if (mode == MODE_DIRECTED_INDEX || mode == MODE_DIRECTED_ALL) {
	current_ans_node = answer_search_loop(sg_fr, current_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
      } else {
	ans_node_ptr parent_ans_node = current_ans_node;
	if (TrNode_child(current_ans_node) == NULL) {
#ifdef THREADS_FULL_SHARING
	  struct answer_trie_node virtual_ans_node;
	  AnsNode_init_lock_field(&virtual_ans_node);
	  TrNode_parent(&virtual_ans_node) = NULL;
	  TrNode_child(&virtual_ans_node) = NULL;	  
	  current_ans_node = answer_search_loop(sg_fr, &virtual_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
	  TrNode_child(parent_ans_node) = TrNode_child(&virtual_ans_node);
	  TrNode_parent(TrNode_child(&virtual_ans_node)) = parent_ans_node;
#else
	  current_ans_node = answer_search_loop(sg_fr, current_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
	  //	  printf("current_ans_node - 1 => ans_node = %p\n",current_ans_node);
#endif /* THREADS_FULL_SHARING */
	} else if (mode == MODE_DIRECTED_MIN || mode == MODE_DIRECTED_MAX) {
#ifdef TIMESTAMP_MODE_DIRECTED_TABLING
	  current_ans_node = answer_search_min_max(current_ans_node, Deref(subs_ptr[i]), mode PASS_REGS);
	  //	  printf("current_ans_node - 2 => ans_node = %p\n",current_ans_node);
#else /* !TIMESTAMP_MODE_DIRECTED_TABLING */
	  invalid_ans_node = TrNode_child(parent_ans_node);  /* by default, assume a better answer */
	  current_ans_node = answer_search_min_max(current_ans_node, Deref(subs_ptr[i]), mode PASS_REGS);
	  if (invalid_ans_node == TrNode_child(parent_ans_node))  /* worse or equal answer */
	    invalid_ans_node = NULL;
#endif /* TIMESTAMP_MODE_DIRECTED_TABLING */
	} else if (mode == MODE_DIRECTED_FIRST) {
	  current_ans_node = NULL;
	} else {  /* mode == MODE_DIRECTED_LAST */
#ifdef THREADS_FULL_SHARING
	  struct answer_trie_node virtual_ans_node;
	  invalid_ans_node = TrNode_child(parent_ans_node);
	  AnsNode_init_lock_field(&virtual_ans_node);
	  TrNode_parent(&virtual_ans_node) = NULL;
	  TrNode_child(&virtual_ans_node) = NULL;
	  current_ans_node = answer_search_loop(sg_fr, &virtual_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
	  TrNode_child(parent_ans_node) = TrNode_child(&virtual_ans_node);
	  TrNode_parent(TrNode_child(&virtual_ans_node)) = parent_ans_node; 
#else
	  invalid_ans_node = TrNode_child(current_ans_node);
	  TrNode_child(current_ans_node) = NULL;	 
	  current_ans_node = answer_search_loop(sg_fr, current_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
#endif /* THREADS_FULL_SHARING */
	}
      }
      n_subs--;
      i--;
    } while (n_subs && current_ans_node);
    if (current_ans_node == NULL)  /* no answer inserted */
      break;
    j++;
  }


  if (invalid_ans_node)
   invalidate_answer_trie(invalid_ans_node, sg_fr, TRAVERSE_POSITION_FIRST PASS_REGS);

  UNLOCK_SG_FR(sg_fr);
 

  /* reset variables */
  stack_vars = (CELL *) TR;
  while (vars_arity--) {
    Term t = STACK_POP_DOWN(stack_vars);
    RESET_VARIABLE(t);
  } 
  return current_ans_node;
#undef subs_arity
}

#elif defined(THREADS_FULL_SHARING_MODE_DIRECTED_V02)

ans_node_ptr mode_directed_answer_search(sg_fr_ptr sg_fr, CELL *subs_ptr USES_REGS) {
#define subs_arity *subs_ptr
  CELL *stack_vars;
  int i, j, vars_arity;
  ans_node_ptr current_ans_node;
  int *mode_directed;

  vars_arity = 0;
  current_ans_node = SgFr_answer_trie(sg_fr);
  mode_directed = SgFr_mode_directed(sg_fr);
  j = 0;
  i = subs_arity;


  while (i) {
    int mode = MODE_DIRECTED_GET_MODE(mode_directed[j]);   // mode
    int n_subs = MODE_DIRECTED_GET_ARG(mode_directed[j]);  // mode * reps. Example (index,max,max) = 1 * index + 2 * max
    do {
      TABLING_ERROR_CHECKING(answer_search, IsNonVarTerm(subs_ptr[i]));
      if (mode == MODE_DIRECTED_INDEX || mode == MODE_DIRECTED_ALL) {
	current_ans_node = answer_search_loop(sg_fr, current_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
      } else {
	ans_node_ptr parent_ans_node = current_ans_node;
	if (TrNode_child(parent_ans_node) == NULL) {
	  struct answer_trie_node virtual_ans_node;
	  AnsNode_init_lock_field(&virtual_ans_node);
	  TrNode_parent(&virtual_ans_node) = NULL;
	  TrNode_child(&virtual_ans_node) = NULL;	  
	  current_ans_node = answer_search_loop(sg_fr, &virtual_ans_node, Deref(subs_ptr[i]), &vars_arity PASS_REGS);
	  TrNode_parent(current_ans_node) = parent_ans_node;		  
	  if (BOOL_CAS((&(TrNode_child(parent_ans_node))), NULL, current_ans_node)) {
	    n_subs--;
	    i--;
	    continue;
	  } else
	    FREE_ANSWER_TRIE_NODE(current_ans_node);
	}
	
	if (mode == MODE_DIRECTED_MIN || mode == MODE_DIRECTED_MAX) {
	  current_ans_node = answer_search_min_max(sg_fr, parent_ans_node, Deref(subs_ptr[i]), mode PASS_REGS);
	} else if (mode == MODE_DIRECTED_FIRST)
	  current_ans_node = NULL;
      }
      n_subs--;
      i--;
    } while (n_subs && current_ans_node);
    if (current_ans_node == NULL)  // no answer inserted
      break;
    j++;
  }
  
  /* reset variables */
  stack_vars = (CELL *) TR;
  while (vars_arity--) {
    Term t = STACK_POP_DOWN(stack_vars);
    RESET_VARIABLE(t);
  } 
  return current_ans_node;
#undef subs_arity
}

#endif /* THREADS_NO_SHARING || THREADS_SUBGOAL_SHARING || THREADS_FULL_SHARING_MODE_DIRECTED_V01 */
#endif /* MODE_DIRECTED_TABLING */


void load_answer(ans_node_ptr current_ans_node, CELL *subs_ptr USES_REGS) {

#define subs_arity *subs_ptr
  CELL *stack_terms;
  int i;

  TABLING_ERROR_CHECKING(load_answer, H < H_FZ);
  if (subs_arity == 0)
    return;

  stack_terms = load_answer_loop(current_ans_node PASS_REGS);

  for (i = subs_arity; i >= 1; i--) {
    Term t = STACK_POP_DOWN(stack_terms);
    Bind((CELL *) subs_ptr[i], t);
  }
  TABLING_ERROR_CHECKING(load_answer, stack_terms != (CELL *)LOCAL_TrailTop);

  return;
#undef subs_arity
}







CELL *exec_substitution(gt_node_ptr current_node, CELL *aux_stack) {
  CACHE_REGS
#define subs_arity *subs_ptr
  CELL *stack_terms, *subs_ptr;
  Term t;

  ++aux_stack;  /* skip the heap_arity entry */
  stack_terms = exec_substitution_loop(current_node, &aux_stack, (CELL *) LOCAL_TrailTop PASS_REGS);
  *--aux_stack = 0;  /* restore the heap_arity entry */

  subs_ptr = aux_stack + aux_stack[1] + 2;
  t = STACK_POP_DOWN(stack_terms);
  Bind((CELL *) subs_ptr[subs_arity], t);
  TABLING_ERROR_CHECKING(exec_substitution, stack_terms != (CELL *)LOCAL_TrailTop);
  *subs_ptr = subs_arity - 1;

  return aux_stack;
#undef subs_arity
}


void update_answer_trie(sg_fr_ptr sg_fr USES_REGS) {
  ans_node_ptr current_node;

  free_answer_hash_chain(SgFr_hash_chain(sg_fr) PASS_REGS);
  SgFr_hash_chain(sg_fr) = NULL;
  SgFr_state(sg_fr) += 2;  /* complete --> compiled : complete_in_use --> compiled_in_use */

#if defined(THREADS_FULL_SHARING)
  SgFr_sg_ent_state(sg_fr) += 2;  /* complete --> compiled */
  if (IsMode_Batched(TabEnt_mode(SgFr_tab_ent(sg_fr)))) {
    /* cleaning bits used by batched mode and shifting the instruction back to the original place */
    ans_node_ptr leaf_ans_trie_node = SgFr_first_answer(sg_fr);
    while (TrNode_child(leaf_ans_trie_node) != NULL) {
      ANSWER_LEAF_NODE_INSTR_ABSOLUTE(leaf_ans_trie_node);
      leaf_ans_trie_node = TrNode_child(leaf_ans_trie_node);
    }
    ANSWER_LEAF_NODE_INSTR_ABSOLUTE(leaf_ans_trie_node);
  }
#endif /* THREADS_FULL_SHARING */
  current_node = TrNode_child(SgFr_answer_trie(sg_fr));
  if (current_node) {
#ifdef YAPOR
    TrNode_instr(current_node) -= 1;
#ifdef TABLING_INNER_CUTS
    update_answer_trie_branch(NULL, current_node);
#else
    update_answer_trie_branch(current_node);
#endif /* TABLING_INNER_CUTS */
#else /* TABLING */
    update_answer_trie_branch(current_node, TRAVERSE_POSITION_FIRST);
#endif /* YAPOR */
  }
  return;
}


#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04

void subgoal_trie_abolish_chain(sg_node_ptr current_node, sg_node_ptr *end_chain , int mode, int position USES_REGS) { 
    if (! IS_SUBGOAL_LEAF_NODE(current_node)) {
      int child_mode;
      if (mode == TRAVERSE_MODE_NORMAL) {
        Term t = TrNode_entry(current_node);
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
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	child_mode = TRAVERSE_MODE_DOUBLE_END;
      else
	child_mode = TRAVERSE_MODE_NORMAL;
      free_subgoal_trie(TrNode_child(current_node), child_mode, TRAVERSE_POSITION_FIRST PASS_REGS);
    }
#ifndef THREADS_SUBGOAL_FRAME_BY_WID
  else {    
    sg_fr_ptr sg_fr = get_subgoal_frame_for_abolish(current_node PASS_REGS);
    if (sg_fr) {
      ans_node_ptr ans_node;
      ans_node = SgFr_answer_trie(sg_fr);
      if (TrNode_child(ans_node)) {
	free_answer_trie(TrNode_child(ans_node), TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
      }
      FREE_ANSWER_TRIE_NODE(ans_node);
#if defined(THREADS_FULL_SHARING)
      FREE_SUBGOAL_ENTRY(SgFr_sg_ent(sg_fr));
#endif /* THREADS_FULL_SHARING */
#if defined(MODE_DIRECTED_TABLING)
	if (SgFr_invalid_chain(sg_fr)) {
	  ans_node_ptr next_node, invalid_node = SgFr_invalid_chain(sg_fr);
	  SgFr_invalid_chain(sg_fr) = NULL;
	  /* free invalid answer nodes */
	  while (invalid_node) {
	    next_node = TrNode_next(invalid_node);	
	    FREE_ANSWER_TRIE_NODE(invalid_node);
	    invalid_node = next_node;
	  }
	}
#endif /* MODE_DIRECTED_TABLING */
      
#ifdef LIMIT_TABLING
      remove_from_global_sg_fr_list(sg_fr);
#endif /* LIMIT_TABLING */
#if defined(THREADS_FULL_SHARING_FTNA_3) 
      consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
      FREE_SUBGOAL_FRAME(sg_fr);
    }
  }
#endif /* !THREADS_SUBGOAL_FRAME_BY_WID */
    if (position == TRAVERSE_POSITION_FIRST) {
      sg_node_ptr next_node = TrNode_next(current_node);
      CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
      FREE_SUBGOAL_TRIE_NODE(current_node);
      while ((sg_node_ptr *) next_node != end_chain) {
	current_node = next_node;
	next_node = TrNode_next(current_node);
	subgoal_trie_abolish_chain(current_node, end_chain, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
      }
    } else {
      CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
      FREE_SUBGOAL_TRIE_NODE(current_node);
    }
  return;
}

void subgoal_trie_abolish_bucket_array(sg_node_ptr *curr_hash , int mode, int position USES_REGS) {
  int i;
  sg_node_ptr *bucket;
  bucket = (sg_node_ptr *) V04_UNTAG(curr_hash);
  for (i = 0; i < BASE_HASH_BUCKETS ; i++) {
    if (!V04_IS_EMPTY_BUCKET(*bucket, curr_hash, struct subgoal_trie_node)) {
      if (V04_IS_HASH((*bucket))) {
	subgoal_trie_abolish_bucket_array((sg_node_ptr *) *bucket, mode, position PASS_REGS);
      } else {
	subgoal_trie_abolish_chain((sg_node_ptr)*bucket, curr_hash, mode, position PASS_REGS);
      }
    }
    bucket++;
  }
  V04_FREE_BUCKET_ARRAY(curr_hash);
  return;
}

void free_subgoal_trie(sg_node_ptr first_node, int mode, int position USES_REGS) {
  if (V04_IS_HASH(first_node)) {
    subgoal_trie_abolish_bucket_array((sg_node_ptr *) first_node, mode, position PASS_REGS);
  } else
    subgoal_trie_abolish_chain(first_node, (sg_node_ptr *)NULL, mode, position PASS_REGS);
  return;
}

#else /* !SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 */

void free_subgoal_trie(sg_node_ptr current_node, int mode, int position USES_REGS) {
    
  if (IS_SUBGOAL_TRIE_HASH(current_node)) {
    sg_node_ptr *bucket, *last_bucket;
    sg_hash_ptr hash;
    hash = (sg_hash_ptr) current_node;
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
    bucket = SgHash_buckets(hash);
    last_bucket = bucket + SgHash_num_buckets(hash);
#else      
    bucket = Hash_buckets(hash);
    last_bucket = bucket + Hash_num_buckets(hash);
#endif
    do {
      if (*bucket) {
	sg_node_ptr next_node = *bucket;
	do {
	  current_node = next_node;
	  next_node = TrNode_next(current_node);
	  free_subgoal_trie(current_node, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
	} while (next_node);
      }
    } while (++bucket != last_bucket);
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL
    FREE_BUCKETS(SgHash_hash_bkts(hash));
    /* free old hash buckets */      
    sg_hash_bkts_ptr old_hash = SgHash_old_hash_bkts(hash);
    while (old_hash){
      SgHash_old_hash_bkts(hash) = HashBkts_next(SgHash_old_hash_bkts(hash));
      FREE_BUCKETS(old_hash);
      old_hash = SgHash_old_hash_bkts(hash);
    }
#else
    FREE_BUCKETS(Hash_buckets(hash));
#endif  /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL */
    FREE_SUBGOAL_TRIE_HASH(hash);    
    return;
  }
  if (! IS_SUBGOAL_LEAF_NODE(current_node)) {
    int child_mode;
    if (mode == TRAVERSE_MODE_NORMAL) {
      Term t = TrNode_entry(current_node);
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
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
      child_mode = TRAVERSE_MODE_DOUBLE_END;
    else
      child_mode = TRAVERSE_MODE_NORMAL;
    free_subgoal_trie(TrNode_child(current_node), child_mode, TRAVERSE_POSITION_FIRST PASS_REGS);
  }
#ifndef THREADS_SUBGOAL_FRAME_BY_WID
  else {    
    sg_fr_ptr sg_fr = get_subgoal_frame_for_abolish(current_node PASS_REGS);
    if (sg_fr) {
      ans_node_ptr ans_node;
#ifndef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04
      free_answer_hash_chain(SgFr_hash_chain(sg_fr) PASS_REGS);
#endif
      ans_node = SgFr_answer_trie(sg_fr);
      if (TrNode_child(ans_node)) {
	free_answer_trie(TrNode_child(ans_node), TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
      }
      SgFr_hash_chain(sg_fr) = NULL;
      FREE_ANSWER_TRIE_NODE(ans_node);
#if defined(THREADS_FULL_SHARING)
	FREE_SUBGOAL_ENTRY(SgFr_sg_ent(sg_fr));
#endif /* THREADS_FULL_SHARING */
#if defined(MODE_DIRECTED_TABLING)
	if (SgFr_invalid_chain(sg_fr)) {
	  ans_node_ptr next_node, invalid_node = SgFr_invalid_chain(sg_fr);
	  SgFr_invalid_chain(sg_fr) = NULL;
	  /* free invalid answer nodes */
	  while (invalid_node) {
	    next_node = TrNode_next(invalid_node);	
	    FREE_ANSWER_TRIE_NODE(invalid_node);
	    invalid_node = next_node;
	  }
	}
#if defined(THREADS_FULL_SHARING)
	if (SgFr_old_hash_chain(sg_fr)) {
	  struct answer_trie_hash *hash;
	  do {	    
	    hash = SgFr_old_hash_chain(sg_fr);
	    SgFr_old_hash_chain(sg_fr) = Hash_next(SgFr_old_hash_chain(sg_fr));
	    FREE_ANSWER_TRIE_HASH(hash);
	  } while(SgFr_old_hash_chain(sg_fr));
	}
#endif /*THREADS_FULL_SHARING */
#endif /* MODE_DIRECTED_TABLING */
      
#ifdef LIMIT_TABLING
      remove_from_global_sg_fr_list(sg_fr);
#endif /* LIMIT_TABLING */
#if defined(THREADS_FULL_SHARING_FTNA_3) 
      consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
      FREE_SUBGOAL_FRAME(sg_fr);
    }
  }
#endif /* !THREADS_SUBGOAL_FRAME_BY_WID */
  if (position == TRAVERSE_POSITION_FIRST) {
    sg_node_ptr next_node = TrNode_next(current_node);
      CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
      FREE_SUBGOAL_TRIE_NODE(current_node);
    
    while (next_node) {
      current_node = next_node;
      next_node = TrNode_next(current_node);
      free_subgoal_trie(current_node, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
    }
  } else {
    CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
    FREE_SUBGOAL_TRIE_NODE(current_node);    
  }
  return;
}

#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 */

#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04

void answer_trie_abolish_chain(ans_node_ptr current_node, ans_node_ptr *end_chain , int mode, int position USES_REGS) { 
#ifdef TABLING_INNER_CUTS
    if (! IS_ANSWER_LEAF_NODE(current_node) && TrNode_child(current_node)) {
#else
    if (! IS_ANSWER_LEAF_NODE(current_node)) {
#endif /* TABLING_INNER_CUTS */
      int child_mode;
      if (mode == TRAVERSE_MODE_NORMAL) {
        Term t = TrNode_entry(current_node);
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
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	child_mode = TRAVERSE_MODE_DOUBLE_END;
      else
	child_mode = TRAVERSE_MODE_NORMAL;
      free_answer_trie(TrNode_child(current_node), child_mode, TRAVERSE_POSITION_FIRST PASS_REGS);
    }
    if (position == TRAVERSE_POSITION_FIRST) {
      ans_node_ptr next_node = TrNode_next(current_node);
      CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
      FREE_ANSWER_TRIE_NODE(current_node);
      while ((ans_node_ptr *) next_node != end_chain) {
	current_node = next_node;
	next_node = TrNode_next(current_node);
	answer_trie_abolish_chain(current_node, end_chain, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
      }
    } else {
      CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
      FREE_ANSWER_TRIE_NODE(current_node);
    }
  return;
}

/*
A simpler abolish_chain...
void answer_trie_abolish_chain(ans_node_ptr current_node, ans_node_ptr *end_chain , int mode, int position USES_REGS) {
  if ((ans_node_ptr *) current_node == end_chain)
    return;
  answer_trie_abolish_chain(TrNode_next(current_node), end_chain, mode, position PASS_REGS);
  if (! IS_ANSWER_LEAF_NODE(current_node)) {
    int child_mode = 1;
    free_answer_trie(TrNode_child(current_node), child_mode, TRAVERSE_POSITION_FIRST PASS_REGS);
  }
  FREE_ANSWER_TRIE_NODE(current_node);
  return;
}

*/

void answer_trie_abolish_bucket_array(ans_node_ptr *curr_hash , int mode, int position USES_REGS) {
  int i;
  ans_node_ptr *bucket;
  bucket = (ans_node_ptr *) V04_UNTAG(curr_hash);
  for (i = 0; i < BASE_HASH_BUCKETS ; i++) {
    if (!V04_IS_EMPTY_BUCKET(*bucket, curr_hash, struct answer_trie_node)) {
      if (V04_IS_HASH((*bucket))) {
	answer_trie_abolish_bucket_array((ans_node_ptr *) *bucket, mode, position PASS_REGS);
      } else {
	answer_trie_abolish_chain((ans_node_ptr)*bucket, curr_hash, mode, position PASS_REGS);
      }
    }
    bucket++;
  }
  V04_FREE_BUCKET_ARRAY(curr_hash);
  return;
}

void free_answer_trie(ans_node_ptr first_node, int mode, int position USES_REGS) {
  if (V04_IS_HASH(first_node)) {
    answer_trie_abolish_bucket_array((ans_node_ptr *) first_node, mode, position PASS_REGS);
  } else
    answer_trie_abolish_chain(first_node, (ans_node_ptr *)NULL, mode, position PASS_REGS);
  return;
}

#else /* !ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 */

void free_answer_trie(ans_node_ptr current_node, int mode, int position USES_REGS) {

#ifdef TABLING_INNER_CUTS
    if (! IS_ANSWER_LEAF_NODE(current_node) && TrNode_child(current_node)) {
#else
    if (! IS_ANSWER_LEAF_NODE(current_node)) {
#endif /* TABLING_INNER_CUTS */
      int child_mode;
      if (mode == TRAVERSE_MODE_NORMAL) {
        Term t = TrNode_entry(current_node);
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
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
	child_mode = TRAVERSE_MODE_DOUBLE_END;
      else
	child_mode = TRAVERSE_MODE_NORMAL;
      free_answer_trie(TrNode_child(current_node), child_mode, TRAVERSE_POSITION_FIRST PASS_REGS);
    }
    if (position == TRAVERSE_POSITION_FIRST) {
      ans_node_ptr next_node = TrNode_next(current_node);
      CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
      FREE_ANSWER_TRIE_NODE(current_node);
      while (next_node) {
	current_node = next_node;
	next_node = TrNode_next(current_node);
	free_answer_trie(current_node, mode, TRAVERSE_POSITION_NEXT PASS_REGS);
      }
    } else {
      CHECK_DECREMENT_GLOBAL_TRIE_REFERENCE(TrNode_entry(current_node), mode);
      FREE_ANSWER_TRIE_NODE(current_node);
    }
  return;
}

#endif /* !ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 */


void free_answer_hash_chain(ans_hash_ptr hash USES_REGS) {
    while (hash) {
      ans_node_ptr chain_node, *bucket, *last_bucket;
      ans_hash_ptr next_hash;
      
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
      bucket = AnsHash_buckets(hash);
      last_bucket = bucket + AnsHash_num_buckets(hash);
#else      
      bucket = Hash_buckets(hash);
      last_bucket = bucket + Hash_num_buckets(hash);
#endif  /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL */
      
      while (*bucket == NULL)
	bucket++;           
      chain_node = *bucket;
      
      TrNode_child((ans_node_ptr) UNTAG_ANSWER_NODE(TrNode_parent(chain_node))) = chain_node;
      while (++bucket != last_bucket) {
	if (*bucket != NULL) {
	  while (TrNode_next(chain_node)) 
	    chain_node = TrNode_next(chain_node);  

	  
	  TrNode_next(chain_node) = *bucket;
	  chain_node = *bucket ;
	}
      }
      next_hash = Hash_next(hash);    
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL
      FREE_BUCKETS(AnsHash_hash_bkts(hash));
      // free old hash buckets 
      ans_hash_bkts_ptr old_hash = AnsHash_old_hash_bkts(hash);
      while (old_hash){
	AnsHash_old_hash_bkts(hash) = HashBkts_next(AnsHash_old_hash_bkts(hash));
	FREE_BUCKETS(old_hash);
	old_hash = AnsHash_old_hash_bkts(hash);
      }
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V02
      FREE_EXPANSION_NODES(Hash_exp_nodes(hash), ans_node_ptr);
#endif
#else
      FREE_BUCKETS(Hash_buckets(hash));
#endif  /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL */
      FREE_ANSWER_TRIE_HASH(hash);
      hash = next_hash;
    }
  return;
}


/*****************************************************************************************
** all threads abolish their local data structures, and the main thread also abolishes  **
** all shared data structures, if no other thread is running (worker_id == 0).  **
*****************************************************************************************/
void abolish_table(tab_ent_ptr tab_ent USES_REGS) {

  sg_node_ptr sg_node;
#if defined(THREADS)
  if (worker_id == 0) {
#ifdef THREADS_LOCAL_SG_FR_HASH_BUCKETS
    INIT_BUCKETS(SgFrHashBkts_buckets(LOCAL_sg_fr_hash_buckets), SgFrHashBkts_number_of_buckets(LOCAL_sg_fr_hash_buckets));
#endif
    ATTACH_PAGES(_pages_tab_ent);
#ifdef THREADS_SUBGOAL_SHARING_WITH_PAGES_SG_FR_ARRAY
    ATTACH_PAGES(_pages_sg_fr_array);
#endif
#if defined(THREADS_FULL_SHARING)
    ATTACH_PAGES(_pages_sg_ent);
#endif
    ATTACH_PAGES(_pages_sg_fr);
    ATTACH_PAGES(_pages_dep_fr);
    ATTACH_PAGES(_pages_sg_node);
    ATTACH_PAGES(_pages_sg_hash);
    ATTACH_PAGES(_pages_ans_node);    
    ATTACH_PAGES(_pages_ans_hash);
#ifdef THREADS_FULL_SHARING
    ATTACH_PAGES(_pages_ans_ref_node);
#endif  
    ATTACH_PAGES(_pages_gt_node);
    ATTACH_PAGES(_pages_gt_hash);
  } 
#if defined(THREADS_SUBGOAL_SHARING)
  else {
#ifdef THREADS_LOCAL_SG_FR_HASH_BUCKETS
    if (LOCAL_sg_fr_hash_buckets) {
      FREE_BLOCK(SgFrHashBkts_buckets(LOCAL_sg_fr_hash_buckets));
      FREE_BLOCK(LOCAL_sg_fr_hash_buckets);
      LOCAL_sg_fr_hash_buckets = NULL;
    }
#endif
    sg_fr_ptr sg_fr = LOCAL_top_sg_fr_complete;

#ifdef THREADS_SUBGOAL_FRAME_BY_WID    
    if (sg_fr == NULL)
      return;

    sg_fr_ptr sg_fr_last = sg_fr;
    while(SgFr_next_complete(sg_fr_last))
      sg_fr_last = SgFr_next_complete(sg_fr_last);

    sg_fr_ptr remote_sg_fr;
    do {
      remote_sg_fr = REMOTE_top_sg_fr_complete(0);
      SgFr_next_complete(sg_fr_last) = remote_sg_fr;           
    } while(!BOOL_CAS(&(REMOTE_top_sg_fr_complete(0)), remote_sg_fr, sg_fr));
    LOCAL_top_sg_fr_complete = NULL;
    return;
  }
#else /* !THREADS_SUBGOAL_FRAME_BY_WID */      
    while(sg_fr) {      
      sg_fr_ptr next_sg_fr = SgFr_next_complete(sg_fr);
      ans_node_ptr ans_node;
#ifndef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04
      free_answer_hash_chain(SgFr_hash_chain(sg_fr) PASS_REGS);
#endif
      ans_node = SgFr_answer_trie(sg_fr);
      if (TrNode_child(ans_node))
      	free_answer_trie(TrNode_child(ans_node), TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
      SgFr_hash_chain(sg_fr) = NULL;
      FREE_ANSWER_TRIE_NODE(ans_node);
#ifndef THREADS_LOCAL_SG_FR_HASH_BUCKETS
      sg_fr_ptr *sg_fr_addr = (sg_fr_ptr *) get_thread_bucket(SgFr_sg_fr_array(sg_fr));
      *sg_fr_addr = NULL;
#endif
#if defined(THREADS_FULL_SHARING_FTNA_3) 
      consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
      FREE_SUBGOAL_FRAME(sg_fr);	
      sg_fr = next_sg_fr;
    }
    LOCAL_top_sg_fr_complete = NULL;
    return;
  }
#endif /* THREADS_SUBGOAL_FRAME_BY_WID */

#elif defined(THREADS_FULL_SHARING)
  else {
    sg_fr_ptr sg_fr = LOCAL_top_sg_fr_complete;
#ifdef THREADS_SUBGOAL_FRAME_BY_WID    
    if (sg_fr == NULL)
      return;
    sg_fr_ptr sg_fr_last = sg_fr;
    while(SgFr_next_complete(sg_fr_last))
      sg_fr_last = SgFr_next_complete(sg_fr_last);
   
    sg_fr_ptr remote_sg_fr;
    do {
      remote_sg_fr = REMOTE_top_sg_fr_complete(0);
      SgFr_next_complete(sg_fr_last) = remote_sg_fr;           
    } while(!BOOL_CAS(&(REMOTE_top_sg_fr_complete(0)), remote_sg_fr, sg_fr));
 
    LOCAL_top_sg_fr_complete = NULL;
    return;
  }
#else /* !THREADS_SUBGOAL_FRAME_BY_WID */
    while(sg_fr) {      
      sg_fr_ptr next_sg_fr = SgFr_next_complete(sg_fr);
      sg_ent_ptr sg_ent = SgFr_sg_ent(sg_fr);
      sg_fr_ptr *sg_fr_addr = (sg_fr_ptr *) get_thread_bucket((void **) &SgEnt_sg_fr(sg_ent));
      *sg_fr_addr = NULL;
#if defined(THREADS_FULL_SHARING_FTNA_3) 
      consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
      FREE_SUBGOAL_FRAME(sg_fr);
      sg_fr = next_sg_fr;
    }
    LOCAL_top_sg_fr_complete = NULL;
    return;
  }
#endif /* THREADS_SUBGOAL_FRAME_BY_WID */
#endif /* THREADS_SUBGOAL_SHARING */
#endif  /* THREADS */

  sg_node = get_subgoal_trie_for_abolish(tab_ent PASS_REGS);
  if (sg_node) {
    if (TrNode_child(sg_node)) {
      if (TabEnt_arity(tab_ent)) {
          free_subgoal_trie(TrNode_child(sg_node), TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);  
      } 
#ifndef  THREADS_SUBGOAL_FRAME_BY_WID
      else {
        sg_fr_ptr sg_fr = get_subgoal_frame_for_abolish(sg_node PASS_REGS);
        if (sg_fr) {
	  SgFr_hash_chain(sg_fr) = NULL;
	  FREE_ANSWER_TRIE_NODE(SgFr_answer_trie(sg_fr));
#if defined(THREADS_FULL_SHARING)
	  FREE_SUBGOAL_ENTRY(SgFr_sg_ent(sg_fr));
#endif /* THREADS_FULL_SHARING */	  
#ifdef LIMIT_TABLING
	  remove_from_global_sg_fr_list(sg_fr);
#endif /* LIMIT_TABLING */
#if defined(THREADS_FULL_SHARING_FTNA_3) 
	  consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
	  FREE_SUBGOAL_FRAME(sg_fr);
	}
      }
#endif  /* !THREADS_SUBGOAL_FRAME_BY_WID */
      TrNode_child(sg_node) = NULL;
     }
#ifdef THREADS_NO_SHARING
    FREE_SUBGOAL_TRIE_NODE(sg_node);
#endif /* THREADS_NO_SHARING */
  }

#ifdef THREADS_SUBGOAL_FRAME_BY_WID
  /* only wid == 0 is here */
#ifdef THREADS_SUBGOAL_SHARING
  sg_fr_ptr sg_fr = LOCAL_top_sg_fr_complete;
  while (sg_fr) {
    sg_fr_ptr next_sg_fr = SgFr_next_complete(sg_fr);
    ans_node_ptr ans_node;
#ifndef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 
    free_answer_hash_chain(SgFr_hash_chain(sg_fr) PASS_REGS);
#endif
    ans_node = SgFr_answer_trie(sg_fr);
    if (TrNode_child(ans_node))
      free_answer_trie(TrNode_child(ans_node), TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
    SgFr_hash_chain(sg_fr) = NULL;
    FREE_ANSWER_TRIE_NODE(ans_node);
#if defined(THREADS_FULL_SHARING_FTNA_3) 
    consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
    FREE_SUBGOAL_FRAME(sg_fr);	
    sg_fr = next_sg_fr;
  }
  LOCAL_top_sg_fr_complete = NULL;
  
#else  /* THREADS_FULL_SHARING */
  sg_fr_ptr sg_fr = LOCAL_top_sg_fr_complete;
  while (sg_fr) {
    sg_fr_ptr next_sg_fr = SgFr_next_complete(sg_fr);
    if (SgEnt_sg_fr(SgFr_sg_ent(sg_fr)) == sg_fr) {
      ans_node_ptr ans_node;
#ifndef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 
    free_answer_hash_chain(SgFr_hash_chain(sg_fr) PASS_REGS);
#endif
      ans_node = SgFr_answer_trie(sg_fr);
      if (TrNode_child(ans_node))
	free_answer_trie(TrNode_child(ans_node), TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
      FREE_ANSWER_TRIE_NODE(ans_node);      
      FREE_SUBGOAL_ENTRY(SgFr_sg_ent(sg_fr));
    }
#if defined(THREADS_FULL_SHARING_FTNA_3) 
    consumer_trie_free_structs(sg_fr PASS_REGS);
#endif
    FREE_SUBGOAL_FRAME(sg_fr);
    sg_fr = next_sg_fr;
  }
#endif /* THREADS_SUBGOAL_SHARING */
#endif /* THREADS_SUBGOAL_FRAME_BY_WID */
  return;
}


void show_table(tab_ent_ptr tab_ent, int show_mode, IOSTREAM *out) {
  CACHE_REGS

  sg_node_ptr sg_node;
  TrStat_out = out;
  TrStat_show = show_mode;
  TrStat_subgoals = 0;
  TrStat_sg_incomplete = 0;
  TrStat_sg_nodes = 1;
  TrStat_answers = 0;
  TrStat_answers_true = 0;
  TrStat_answers_no = 0;
#ifdef TABLING_INNER_CUTS
  TrStat_answers_pruned = 0;
#endif /* TABLING_INNER_CUTS */
  TrStat_ans_nodes = 0;
  TrStat_gt_refs = 0;
  if (show_mode == SHOW_MODE_STATISTICS)
    Sfprintf(TrStat_out, "Table statistics for predicate '%s", AtomName(TabEnt_atom(tab_ent)));
  else  /* SHOW_MODE_STRUCTURE */
    Sfprintf(TrStat_out, "Table structure for predicate '%s", AtomName(TabEnt_atom(tab_ent)));
#ifdef MODE_DIRECTED_TABLING
  if (TabEnt_mode_directed(tab_ent)) {
    int i, *mode_directed = TabEnt_mode_directed(tab_ent);
    Sfprintf(TrStat_out, "(");
    for (i = 0; i < TabEnt_arity(tab_ent); i++) {
      int mode = MODE_DIRECTED_GET_MODE(mode_directed[i]);
      if (mode == MODE_DIRECTED_INDEX) {
	Sfprintf(TrStat_out, "index");
      } else if (mode == MODE_DIRECTED_FIRST) {
	Sfprintf(TrStat_out, "first");
      } else if (mode == MODE_DIRECTED_ALL) {
	Sfprintf(TrStat_out, "all");
      } else if (mode == MODE_DIRECTED_MAX) {
	Sfprintf(TrStat_out, "max");
      } else if (mode == MODE_DIRECTED_MIN) {
	Sfprintf(TrStat_out, "min");
      } else /* MODE_DIRECTED_LAST */
	Sfprintf(TrStat_out, "last");
      if (i != MODE_DIRECTED_GET_ARG(mode_directed[i]))
	Sfprintf(TrStat_out, "(ARG%d)", MODE_DIRECTED_GET_ARG(mode_directed[i]) + 1);
      if (i + 1 != TabEnt_arity(tab_ent))
	Sfprintf(TrStat_out, ",");
    }
    Sfprintf(TrStat_out, ")'\n");
  } else
#endif /* MODE_DIRECTED_TABLING */
    Sfprintf(TrStat_out, "/%d'\n", TabEnt_arity(tab_ent));
  sg_node = get_subgoal_trie(tab_ent);
  if (sg_node) {
#ifdef EXTRA_STATISTICS
    long sg_dep = 1;
#endif /* EXTRA_STATISTICS */
    if (TrNode_child(sg_node)) {
      if (TabEnt_arity(tab_ent)) {
	char *str = (char *) malloc(sizeof(char) * SHOW_TABLE_STR_ARRAY_SIZE);
	int *arity = (int *) malloc(sizeof(int) * SHOW_TABLE_ARITY_ARRAY_SIZE);
	arity[0] = 1;
	arity[1] = TabEnt_arity(tab_ent);
	int str_index = sprintf(str, "  ?- %s(", AtomName(TabEnt_atom(tab_ent)));
#ifdef EXTRA_STATISTICS
	traverse_subgoal_trie(sg_dep, TrNode_child(sg_node), str, str_index, arity, TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
#else
	traverse_subgoal_trie(TrNode_child(sg_node), str, str_index, arity, TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
#endif
	free(str);
	free(arity);
      } else {
#ifdef THREADS_FULL_SHARING
	/* just to avoid table differences on the test_suite. */
	sg_fr_ptr sg_fr;
#ifdef THREADS_SUBGOAL_FRAME_BY_WID
	sg_fr = SgEnt_sg_fr((sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node)));
#else /* !THREADS_SUBGOAL_FRAME_BY_WID */
	sg_fr_ptr * sg_fr_addr = (sg_fr_ptr *) get_insert_thread_bucket((void **) &SgEnt_sg_fr((sg_ent_ptr) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node)))
#ifdef SUBGOAL_TRIE_LOCK_USING_NODE_FIELD
									, &TrNode_lock(sg_node)
#elif defined(SUBGOAL_TRIE_LOCK_USING_GLOBAL_ARRAY)
									, &HASH_TRIE_LOCK(sg_node)
#endif /* SUBGOAL_TRIE_LOCK_USING_NODE_FIELD */
					);
	sg_fr = *sg_fr_addr;

#endif /* THREADS_SUBGOAL_FRAME_BY_WID */
#else /* !THREADS_FULL_SHARING */
	sg_fr_ptr sg_fr = get_subgoal_frame(sg_node);
#endif /* THREADS_FULL_SHARING */

#if defined(THREADS_SUBGOAL_SHARING) && !defined(THREADS_SUBGOAL_FRAME_BY_WID)
	/* just to avoid table differences on the test_suite. */
	if (sg_fr == NULL) {
	  void **buckets;
	  sg_fr_ptr *sg_fr_addr_completed;
	  sg_fr_ptr sg_fr_completed;
	  buckets = (void **) UNTAG_SUBGOAL_NODE(TrNode_sg_fr(sg_node));
	  sg_fr_addr_completed = (sg_fr_ptr *) buckets;
	  sg_fr_completed = *sg_fr_addr_completed;
	  if (sg_fr_completed != NULL)
	    sg_fr = sg_fr_completed;
	}
#endif /* THREADS_SUBGOAL_SHARING && !THREADS_SUBGOAL_FRAME_BY_WID*/
	if (sg_fr) {
	  TrStat_subgoals++;
	  SHOW_TABLE_STRUCTURE("  ?- %s.\n", AtomName(TabEnt_atom(tab_ent)));
	  TrStat_ans_nodes++;
	  if (SgFr_first_answer(sg_fr) == NULL) {
	    if (SgFr_state(sg_fr) < complete) {
	      TrStat_sg_incomplete++;
	      SHOW_TABLE_STRUCTURE("    ---> INCOMPLETE\n");
	    } else {
	      TrStat_answers_no++;
	      SHOW_TABLE_STRUCTURE("    NO\n");
	    }
	  } else {  /* SgFr_first_answer(sg_fr) == SgFr_answer_trie(sg_fr) */
	    TrStat_answers_true++;
	    SHOW_TABLE_STRUCTURE("    TRUE\n");
	  }
	}
      }
    }
  }
  if (TrStat_subgoals == 0)
    SHOW_TABLE_STRUCTURE("  EMPTY\n");
  if (show_mode == SHOW_MODE_STATISTICS) {
    Sfprintf(TrStat_out, "  Subgoal trie structure\n");
    Sfprintf(TrStat_out, "    Subgoals: %ld (%ld incomplete)\n", TrStat_subgoals, TrStat_sg_incomplete);
    Sfprintf(TrStat_out, "    Subgoal trie nodes: %ld\n", TrStat_sg_nodes);
    Sfprintf(TrStat_out, "  Answer trie structure(s)\n");
#ifdef TABLING_INNER_CUTS
    Sfprintf(TrStat_out, "    Answers: %ld (%ld pruned)\n", TrStat_answers, TrStat_answers_pruned);
#else
    Sfprintf(TrStat_out, "    Answers: %ld\n", TrStat_answers);
#endif /* TABLING_INNER_CUTS */
    Sfprintf(TrStat_out, "    Answers 'TRUE': %ld\n", TrStat_answers_true);
    Sfprintf(TrStat_out, "    Answers 'NO': %ld\n", TrStat_answers_no);
    Sfprintf(TrStat_out, "    Answer trie nodes: %ld\n", TrStat_ans_nodes);
    Sfprintf(TrStat_out, "  Global trie references: %ld\n", TrStat_gt_refs);
  }
  return;
}


void show_global_trie(int show_mode, IOSTREAM *out) {
  CACHE_REGS

  TrStat_out = out;
  TrStat_show = show_mode;
  TrStat_gt_terms = 0;
  TrStat_gt_nodes = 1;
  TrStat_gt_refs = 0;
  if (show_mode == SHOW_MODE_STATISTICS)
    Sfprintf(TrStat_out, "Global trie statistics\n");
  else  /* SHOW_MODE_STRUCTURE */
    Sfprintf(TrStat_out, "Global trie structure\n");
  if (TrNode_child(GLOBAL_root_gt)) {
    char *str = (char *) malloc(sizeof(char) * SHOW_TABLE_STR_ARRAY_SIZE);
    int *arity = (int *) malloc(sizeof(int) * SHOW_TABLE_ARITY_ARRAY_SIZE);
    arity[0] = 0;
    traverse_global_trie(TrNode_child(GLOBAL_root_gt), str, 0, arity, TRAVERSE_MODE_NORMAL, TRAVERSE_POSITION_FIRST PASS_REGS);
    free(str);
    free(arity);
  } else
    SHOW_TABLE_STRUCTURE("  EMPTY\n");
  if (show_mode == SHOW_MODE_STATISTICS) {
    Sfprintf(TrStat_out, "  Terms: %ld\n", TrStat_gt_terms);
    Sfprintf(TrStat_out, "  Global trie nodes: %ld\n", TrStat_gt_nodes);
    Sfprintf(TrStat_out, "  Global trie auto references: %ld\n", TrStat_gt_refs);
  }
  return;
}
#endif /* TABLING */
