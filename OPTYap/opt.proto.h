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

#if defined(TABLING) || defined(YAPOR) 
#include "SWI-Stream.h"
#endif /* TABLING || YAPOR */



/*************************
**      opt.init.c      **
*************************/

void Yap_init_global_optyap_data(int, int, int, int);
void Yap_init_local_optyap_data(int);
void Yap_init_root_frames(void);
void itos(int, char *);



/**************************
**      opt.preds.c      **
**************************/

#ifdef YAPOR
void finish_yapor(void);
#endif /* YAPOR */



/**************************
**      tab.tries.c      **
**************************/

#ifdef TABLING
sg_fr_ptr subgoal_search(yamop *, CELL ** USES_REGS);
#ifdef THREADS_NO_SUBGOAL_TRIE
sg_fr_ptr subgoal_search_no_trie(yamop *, CELL ** USES_REGS);
#endif /* THREADS_NO_SUBGOAL_TRIE */
ans_node_ptr answer_search(sg_fr_ptr, CELL * USES_REGS);
#ifdef MODE_DIRECTED_TABLING
ans_node_ptr mode_directed_answer_search(sg_fr_ptr, CELL * USES_REGS);
#endif /* MODE_DIRECTED_TABLING */
void load_answer(ans_node_ptr, CELL * USES_REGS);
CELL *exec_substitution(gt_node_ptr, CELL *);
void update_answer_trie(sg_fr_ptr USES_REGS);
#ifdef SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04
void subgoal_trie_abolish_chain(sg_node_ptr, sg_node_ptr * , int ,int USES_REGS);
void subgoal_trie_abolish_bucket_array(sg_node_ptr *, int, int USES_REGS);
#endif /* SUBGOAL_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 */
void free_subgoal_trie(sg_node_ptr, int, int USES_REGS);
#ifdef ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04
void answer_trie_abolish_chain(ans_node_ptr, ans_node_ptr * , int ,int USES_REGS);
void answer_trie_abolish_bucket_array(ans_node_ptr *, int, int USES_REGS);
#endif /* ANSWER_TRIE_LOCK_AT_ATOMIC_LEVEL_V04 */
void free_answer_trie(ans_node_ptr, int, int USES_REGS);
void free_answer_hash_chain(ans_hash_ptr USES_REGS);
void abolish_table(tab_ent_ptr USES_REGS);
void show_table(tab_ent_ptr, int, IOSTREAM *);
void show_global_trie(int, IOSTREAM *);
#endif /* TABLING */



/*******************************
**      tab.completion.c      **
*******************************/

#ifdef TABLING 
void private_completion(sg_fr_ptr USES_REGS);
#ifdef YAPOR
void public_completion(void);
void complete_suspension_frames(or_fr_ptr);
void suspend_branch(void);
void resume_suspension_frame(susp_fr_ptr, or_fr_ptr);
#endif /* YAPOR */
#endif /* TABLING */



/**************************
**      or.memory.c      **
**************************/

#ifdef YAPOR
void Yap_init_yapor_global_local_memory(void);
void Yap_init_yapor_stacks_memory(UInt, UInt, UInt, int);
void Yap_unmap_yapor_memory(void);
void Yap_remap_yapor_memory(void);
#endif /* YAPOR */



/*******************************
**      or.copy_engine.c      **
*******************************/

#ifdef YAPOR
void make_root_choice_point(void);
void free_root_choice_point(void);
int q_share_work(int p);
int p_share_work(void);
#endif /* YAPOR */



/*****************************
**      or.scheduler.c      **
*****************************/

#ifdef YAPOR
int get_work(void);
#endif /* YAPOR */



/***********************
**      or.cut.c      **
***********************/

#ifdef YAPOR
void prune_shared_branch(choiceptr, int*);
#endif /* YAPOR */
